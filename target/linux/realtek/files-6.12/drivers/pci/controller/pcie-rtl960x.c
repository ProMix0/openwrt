// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe host controller driver for Realtek RTL960x SoC
 *
 * Copyright (C) 2024 OpenWrt
 *
 * Based on OEM Realtek driver and pcie-mt7621.c
 */

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/spinlock.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>

#include "../pci.h"

/* Chip revision for PHY parameter selection */
#define CHIP_REV_B			0
#define CHIP_REV_C			1

/* SoC control registers */
#define RTL960X_PCI_MISC_PHYS		0x18000504
#define RTL960X_IP_SEL_PHYS		0x18000600

/*
 * Switch core GPIO enable register offset.
 *
 * FIXME: This is a workaround. RTL9607C requires GPIO to be enabled in switch
 * core register before the GPIO controller (otto-gpio) can use the pin. This
 * should properly be handled by a pinmux/pinctrl driver, not here in the PCIe
 * driver. The OEM SDK handles this in their GPIO driver with a second reg entry.
 *
 * Switch core GPIO enable base = 0x38, +4 per bank:
 *   Bank 0 (GPIO 0-31):  offset 0x38
 *   Bank 1 (GPIO 32-63): offset 0x3C
 */
#define RTL960X_SWCORE_GPIO_EN_BANK1	0x3c

/* IP_SEL register bits */
#define IP_SEL_PCIE0_EN			BIT(7)
#define IP_SEL_PCIE1_EN			BIT(6)

/* Host extension register offsets */
#define HOSTEXT_MDIO			0x00
#define HOSTEXT_PWRCR			0x08
#define HOSTEXT_IPCFG			0x0c

/* Host config register offsets */
#define HOSTCFG_PCIE_CAP		0x70
#define HOSTCFG_LINK_STATUS		0x728
#define HOSTCFG_ENABLE			0x80c

/* Link status */
#define LINK_STATUS_UP			0x11
#define LINK_UP_TIMEOUT_MS		100
#define LINK_RETRY_COUNT		3

/* MDIO write format */
#define MDIO_WRITE(reg, val)		(((val) << 16) | ((reg) << 8) | 1)

extern struct rtl83xx_soc_info soc_info;

struct rtl960x_phy_param {
	u8 reg;
	u16 value;
};

/* PCIE0 Rev B PHY parameters (GEN2) */
static const struct rtl960x_phy_param pcie0_phy_params_revb[] = {
	{ 0x00, 0x4008 }, { 0x01, 0xa812 }, { 0x02, 0x6042 }, { 0x04, 0x5000 },
	{ 0x05, 0x230a }, { 0x06, 0x0011 }, { 0x09, 0x520c }, { 0x0a, 0xc670 },
	{ 0x0b, 0xb905 }, { 0x0d, 0xef16 }, { 0x0e, 0x0000 }, { 0x20, 0x9499 },
	{ 0x21, 0x66aa }, { 0x27, 0x011a },
	{ 0x09, 0x500c }, { 0x09, 0x520c },
	{ 0x40, 0x4008 }, { 0x41, 0xa811 }, { 0x42, 0x6042 }, { 0x44, 0x5000 },
	{ 0x45, 0x230a }, { 0x46, 0x0011 }, { 0x4a, 0xc670 }, { 0x4b, 0xb905 },
	{ 0x4d, 0xef16 }, { 0x4e, 0x0000 }, { 0x4f, 0x000c }, { 0x60, 0x94aa },
	{ 0x61, 0x88ff }, { 0x62, 0x0093 }, { 0x67, 0x011a }, { 0x6f, 0x65bd },
	{ 0x49, 0x500c }, { 0x49, 0x520c },
};

/* PCIE1 Rev B PHY parameters (GEN1) */
static const struct rtl960x_phy_param pcie1_phy_params_revb[] = {
	{ 0x00, 0x8a50 }, { 0x02, 0x26f9 }, { 0x03, 0x6bcd }, { 0x04, 0x8049 },
	{ 0x06, 0x1088 }, { 0x07, 0x52b3 }, { 0x08, 0x5285 }, { 0x09, 0x6300 },
	{ 0x0b, 0x0009 }, { 0x0c, 0x0800 }, { 0x0e, 0x0093 }, { 0x20, 0x0105 },
	{ 0x21, 0x1000 },
};

/* PCIE0 Rev C PHY parameters (GEN2) */
static const struct rtl960x_phy_param pcie0_phy_params_revc[] = {
	{ 0x01, 0xa852 }, { 0x06, 0x0017 }, { 0x08, 0x3591 }, { 0x09, 0x520c },
	{ 0x0a, 0xf670 }, { 0x0b, 0xa90d }, { 0x0d, 0xe720 }, { 0x0e, 0x1010 },
	{ 0x1c, 0x2001 }, { 0x1e, 0x66eb }, { 0x20, 0xd4a4 }, { 0x21, 0x485a },
	{ 0x23, 0x0b66 }, { 0x24, 0x4f0c }, { 0x29, 0xf0f3 }, { 0x2b, 0xa0a1 },
	{ 0x09, 0x500c }, { 0x09, 0x520c },
	{ 0x41, 0xa849 }, { 0x46, 0x0017 }, { 0x48, 0x3591 }, { 0x49, 0x520c },
	{ 0x4a, 0xf650 }, { 0x4b, 0xa90d }, { 0x4d, 0xe720 }, { 0x4e, 0x1010 },
	{ 0x5c, 0x2001 }, { 0x60, 0xd4a6 }, { 0x61, 0x586a }, { 0x63, 0x0b66 },
	{ 0x69, 0xf0f3 }, { 0x6b, 0xa0a1 }, { 0x6f, 0x5046 },
	{ 0x49, 0x500c }, { 0x49, 0x520c },
};

/* PCIE1 Rev C PHY parameters (GEN1) */
static const struct rtl960x_phy_param pcie1_phy_params_revc[] = {
	{ 0x01, 0xa852 }, { 0x06, 0x0017 }, { 0x08, 0x3591 }, { 0x09, 0x520c },
	{ 0x0a, 0xf670 }, { 0x0b, 0xa90d }, { 0x0d, 0xe720 }, { 0x0e, 0x1010 },
	{ 0x1c, 0x2001 }, { 0x1e, 0x66eb }, { 0x20, 0xd4a4 }, { 0x21, 0x485a },
	{ 0x23, 0x0b66 }, { 0x24, 0x4f0c }, { 0x29, 0xf0f3 }, { 0x2b, 0xa0a1 },
	{ 0x09, 0x500c }, { 0x09, 0x520c },
};

/**
 * struct rtl960x_pcie - PCIe port information
 * @dev: pointer to PCIe device
 * @hostcfg_base: host config register base
 * @hostext_base: host extension register base
 * @devcfg_base: device config space base
 * @phy_params: PHY parameter table
 * @phy_param_count: number of PHY parameters
 * @lock: register access lock
 * @port: port number (0 or 1)
 * @reset_gpio: GPIO for device reset
 * @link_up: link status
 * @bus_number: assigned PCI bus number
 */
struct rtl960x_pcie {
	struct device *dev;
	void __iomem *hostcfg_base;
	void __iomem *hostext_base;
	void __iomem *devcfg_base;
	const struct rtl960x_phy_param *phy_params;
	int phy_param_count;
	spinlock_t lock;
	int port;
	struct gpio_desc *reset_gpio;
	bool link_up;
	u8 bus_number;
};

static void __iomem *pci_misc_base;
static void __iomem *ip_sel_base;
static struct regmap *swcore_regmap;
static DEFINE_SPINLOCK(global_lock);
static bool global_regs_mapped;

static inline u32 rtl960x_pci_misc_read(void)
{
	return readl(pci_misc_base);
}

static inline void rtl960x_pci_misc_write(u32 val)
{
	writel(val, pci_misc_base);
}

static inline u32 rtl960x_ip_sel_read(void)
{
	return readl(ip_sel_base);
}

static inline void rtl960x_ip_sel_write(u32 val)
{
	writel(val, ip_sel_base);
}

static int rtl960x_map_soc_regs(void)
{
	unsigned long flags;

	spin_lock_irqsave(&global_lock, flags);
	if (global_regs_mapped) {
		spin_unlock_irqrestore(&global_lock, flags);
		return 0;
	}

	pci_misc_base = ioremap(RTL960X_PCI_MISC_PHYS, 4);
	ip_sel_base = ioremap(RTL960X_IP_SEL_PHYS, 4);

	if (!pci_misc_base || !ip_sel_base) {
		pr_err("rtl960x-pcie: failed to ioremap SoC registers\n");
		if (pci_misc_base)
			iounmap(pci_misc_base);
		if (ip_sel_base)
			iounmap(ip_sel_base);
		spin_unlock_irqrestore(&global_lock, flags);
		return -ENOMEM;
	}

	global_regs_mapped = true;
	spin_unlock_irqrestore(&global_lock, flags);
	return 0;
}

/*
 * Enable PCIe reset GPIOs in switch core via syscon/regmap.
 *
 * FIXME: This is a workaround. The GPIO enable should be handled by a
 * pinmux/pinctrl driver, not here. RTL9607C requires GPIO pins to be
 * enabled in the switch core register before the otto-gpio controller
 * can use them. GPIO 39 (bit 7) and GPIO 40 (bit 8) are PCIe reset pins.
 */
static int rtl960x_enable_reset_gpios(struct device *dev)
{
	struct device_node *np = dev->of_node;

	if (swcore_regmap)
		return 0;

	swcore_regmap = syscon_regmap_lookup_by_phandle(np, "realtek,swcore");
	if (IS_ERR(swcore_regmap)) {
		dev_warn(dev, "failed to get swcore syscon, GPIO enable skipped\n");
		swcore_regmap = NULL;
		return 0;  /* Non-fatal, GPIO might work without it */
	}

	/* Enable GPIO 39 (bit 7) and GPIO 40 (bit 8) in switch core */
	regmap_update_bits(swcore_regmap, RTL960X_SWCORE_GPIO_EN_BANK1,
			   BIT(7) | BIT(8), BIT(7) | BIT(8));

	return 0;
}

static void rtl960x_phy_mdio_write(struct rtl960x_pcie *pcie, u8 reg, u16 val)
{
	writel(MDIO_WRITE(reg, val), pcie->hostext_base + HOSTEXT_MDIO);
	mdelay(1);
}

static void rtl960x_phy_init(struct rtl960x_pcie *pcie)
{
	int i;

	for (i = 0; i < pcie->phy_param_count; i++)
		rtl960x_phy_mdio_write(pcie, pcie->phy_params[i].reg,
				       pcie->phy_params[i].value);
}

static int rtl960x_get_chip_revision(void)
{
	/* Rev C for subtype >= 0x1c (e.g. RTL9607C_VA7), Rev B otherwise */
	return (soc_info.subtype >= 0x1c) ? CHIP_REV_C : CHIP_REV_B;
}

static void rtl960x_select_phy_params(struct rtl960x_pcie *pcie)
{
	int rev = rtl960x_get_chip_revision();

	if (pcie->port == 0) {
		if (rev == CHIP_REV_C) {
			pcie->phy_params = pcie0_phy_params_revc;
			pcie->phy_param_count = ARRAY_SIZE(pcie0_phy_params_revc);
		} else {
			pcie->phy_params = pcie0_phy_params_revb;
			pcie->phy_param_count = ARRAY_SIZE(pcie0_phy_params_revb);
		}
	} else {
		if (rev == CHIP_REV_C) {
			pcie->phy_params = pcie1_phy_params_revc;
			pcie->phy_param_count = ARRAY_SIZE(pcie1_phy_params_revc);
		} else {
			pcie->phy_params = pcie1_phy_params_revb;
			pcie->phy_param_count = ARRAY_SIZE(pcie1_phy_params_revb);
		}
	}
}

static void rtl960x_assert_reset(struct rtl960x_pcie *pcie)
{
	if (pcie->reset_gpio)
		gpiod_set_value_cansleep(pcie->reset_gpio, 1);
}

static void rtl960x_deassert_reset(struct rtl960x_pcie *pcie)
{
	if (pcie->reset_gpio)
		gpiod_set_value_cansleep(pcie->reset_gpio, 0);
}

static void rtl960x_phy_reset(struct rtl960x_pcie *pcie)
{
	unsigned long flags;
	u32 val;
	int rst_bit = (pcie->port == 0) ? 24 : 21;
	int dis_bit = (pcie->port == 0) ? 15 : 14;

	spin_lock_irqsave(&global_lock, flags);
	val = rtl960x_pci_misc_read();

	/* Enable PHY by clearing disable bit */
	val &= ~BIT(dis_bit);
	if (pcie->port == 0)
		val |= BIT(30);
	rtl960x_pci_misc_write(val);

	/* Toggle reset bit */
	val &= ~BIT(rst_bit);
	rtl960x_pci_misc_write(val);
	mb();
	val |= BIT(rst_bit);
	rtl960x_pci_misc_write(val);

	spin_unlock_irqrestore(&global_lock, flags);
	mdelay(1);
}

static void rtl960x_mac_reset(struct rtl960x_pcie *pcie)
{
	unsigned long flags;
	u32 val, bit;

	bit = (pcie->port == 0) ? IP_SEL_PCIE0_EN : IP_SEL_PCIE1_EN;

	spin_lock_irqsave(&global_lock, flags);
	val = rtl960x_ip_sel_read();
	rtl960x_ip_sel_write(val & ~bit);
	mb();
	rtl960x_ip_sel_write(val | bit);
	spin_unlock_irqrestore(&global_lock, flags);
}

static void rtl960x_ltssm_enable(struct rtl960x_pcie *pcie)
{
	writel(0x01, pcie->hostext_base + HOSTEXT_PWRCR);
	mb();
	writel(0x81, pcie->hostext_base + HOSTEXT_PWRCR);
	mdelay(50);
}

static bool rtl960x_wait_link_up(struct rtl960x_pcie *pcie)
{
	u32 val;
	int timeout = LINK_UP_TIMEOUT_MS;

	while (timeout > 0) {
		val = readl(pcie->hostcfg_base + HOSTCFG_LINK_STATUS);
		if ((val & 0x1f) == LINK_STATUS_UP)
			return true;
		mdelay(10);
		timeout -= 10;
	}

	return false;
}

static void rtl960x_enable_host(struct rtl960x_pcie *pcie)
{
	u32 val;
	u16 devctl;

	writel(0x00100007, pcie->hostcfg_base + PCI_COMMAND);

	/* Clear max payload size bits in DEVCTL */
	devctl = readw(pcie->hostcfg_base + HOSTCFG_PCIE_CAP + PCI_EXP_DEVCTL);
	devctl &= ~PCI_EXP_DEVCTL_PAYLOAD;
	writew(devctl, pcie->hostcfg_base + HOSTCFG_PCIE_CAP + PCI_EXP_DEVCTL);

	val = readl(pcie->hostcfg_base + HOSTCFG_ENABLE);
	writel(val | BIT(17), pcie->hostcfg_base + HOSTCFG_ENABLE);
}

static int rtl960x_pcie_hw_init(struct rtl960x_pcie *pcie)
{
	int retry;
	u16 lnksta;

	rtl960x_select_phy_params(pcie);

	for (retry = 0; retry < LINK_RETRY_COUNT; retry++) {
		rtl960x_assert_reset(pcie);
		mdelay(10);

		rtl960x_phy_reset(pcie);
		rtl960x_mac_reset(pcie);
		rtl960x_phy_reset(pcie);
		rtl960x_ltssm_enable(pcie);
		rtl960x_phy_init(pcie);
		mdelay(20);

		rtl960x_deassert_reset(pcie);

		if (rtl960x_wait_link_up(pcie)) {
			pcie->link_up = true;
			break;
		}
	}

	if (!pcie->link_up)
		return -ETIMEDOUT;

	mdelay(100);
	rtl960x_enable_host(pcie);

	lnksta = readw(pcie->hostcfg_base + HOSTCFG_PCIE_CAP + PCI_EXP_LNKSTA);
	dev_info(pcie->dev, "link up, %s\n",
		 pci_speed_string(pcie_link_speed[FIELD_GET(PCI_EXP_LNKSTA_CLS, lnksta)]));

	return 0;
}

static void __iomem *rtl960x_pcie_map_bus(struct pci_bus *bus,
					  unsigned int devfn, int where)
{
	struct rtl960x_pcie *pcie = bus->sysdata;

	if (pcie->bus_number == 0xff)
		pcie->bus_number = bus->number;

	if (bus->number != pcie->bus_number)
		return NULL;

	writel(PCI_FUNC(devfn), pcie->hostext_base + HOSTEXT_IPCFG);

	switch (PCI_SLOT(devfn)) {
	case 0:
		return pcie->hostcfg_base + where;
	case 1:
		return pcie->devcfg_base + where;
	default:
		return NULL;
	}
}

static struct pci_ops rtl960x_pcie_ops = {
	.map_bus = rtl960x_pcie_map_bus,
	.read = pci_generic_config_read,
	.write = pci_generic_config_write,
};

static int rtl960x_pcie_parse_dt(struct rtl960x_pcie *pcie)
{
	struct device *dev = pcie->dev;
	struct platform_device *pdev = to_platform_device(dev);
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hostcfg");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "missing hostcfg resource\n");
		return -EINVAL;
	}
	pcie->hostcfg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->hostcfg_base))
		return dev_err_probe(dev, PTR_ERR(pcie->hostcfg_base),
				     "failed to map hostcfg\n");

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hostext");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(dev, "missing hostext resource\n");
		return -EINVAL;
	}
	pcie->hostext_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->hostext_base))
		return dev_err_probe(dev, PTR_ERR(pcie->hostext_base),
				     "failed to map hostext\n");

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "devcfg");
	if (!res)
		res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!res) {
		dev_err(dev, "missing devcfg resource\n");
		return -EINVAL;
	}
	pcie->devcfg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->devcfg_base))
		return dev_err_probe(dev, PTR_ERR(pcie->devcfg_base),
				     "failed to map devcfg\n");

	pcie->reset_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(pcie->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(pcie->reset_gpio),
				     "failed to get reset GPIO\n");

	if (of_property_read_u32(dev->of_node, "realtek,pcie-port", &pcie->port))
		pcie->port = 0;

	return 0;
}

static int rtl960x_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtl960x_pcie *pcie;
	struct pci_host_bridge *bridge;
	int ret;

	bridge = devm_pci_alloc_host_bridge(dev, sizeof(*pcie));
	if (!bridge)
		return -ENOMEM;

	pcie = pci_host_bridge_priv(bridge);
	pcie->dev = dev;
	pcie->bus_number = 0xff;
	spin_lock_init(&pcie->lock);
	platform_set_drvdata(pdev, pcie);

	ret = rtl960x_map_soc_regs();
	if (ret)
		return ret;

	rtl960x_enable_reset_gpios(dev);

	ret = rtl960x_pcie_parse_dt(pcie);
	if (ret)
		return ret;

	ret = rtl960x_pcie_hw_init(pcie);
	if (ret)
		return ret;

	bridge->sysdata = pcie;
	bridge->ops = &rtl960x_pcie_ops;

	return pci_host_probe(bridge);
}

static const struct of_device_id rtl960x_pcie_of_match[] = {
	{ .compatible = "realtek,rtl960x-pcie" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl960x_pcie_of_match);

static struct platform_driver rtl960x_pcie_driver = {
	.probe = rtl960x_pcie_probe,
	.driver = {
		.name = "rtl960x-pcie",
		.of_match_table = rtl960x_pcie_of_match,
	},
};
builtin_platform_driver(rtl960x_pcie_driver);

MODULE_DESCRIPTION("PCIe host controller driver for Realtek RTL960x SoC");
MODULE_LICENSE("GPL");
