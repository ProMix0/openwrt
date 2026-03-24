// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_pci.h>
#include <linux/platform_device.h>
#include <asm/addrspace.h>

#define RTL_PCI_MISC	(void __iomem *)0xb8000504
#define RTL_IP_SEL		(void __iomem *)0xb8000600
#define RTL_ENABLE_PCIE0 (1<<7)
#define RTL_ENABLE_PCIE1 (1<<6)

struct pcie_para {
	u8 reg;
	u16 value;
};

struct rtl_pci_priv {
	void __iomem *hostcfg_base;
	void __iomem *hostext_base;
	void __iomem *devcfg_base;
	spinlock_t lock;
	struct pci_controller controller;
	const struct pcie_para *phy_param;
	size_t phy_param_size;
	struct gpio_desc *gpiod;
	u32 port;
	u8 bus_number;
};

static inline struct rtl_pci_priv * pci_bus_to_rtl_priv(struct pci_bus *bus)
{
	struct pci_controller *hose;

	hose = (struct pci_controller *) bus->sysdata;
	return container_of(hose, struct rtl_pci_priv, controller);
}

static int rtl_pcie_read(struct pci_bus *bus, unsigned int devfn, int where, int size, unsigned int *val)
{
	struct rtl_pci_priv *ctrl = pci_bus_to_rtl_priv(bus);
	unsigned long flags;
	void __iomem *base;
	u32 data;

	if (ctrl->bus_number == 0xff)
		ctrl->bus_number = bus->number;

	if (bus->number != ctrl->bus_number)
		return PCIBIOS_DEVICE_NOT_FOUND;

	switch (PCI_SLOT(devfn)) {
	case 0:
		base = ctrl->hostcfg_base;
		break;
	case 1:
		base = ctrl->devcfg_base;
		break;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_lock_irqsave(&ctrl->lock, flags);
	iowrite32(PCI_FUNC(devfn), ctrl->hostext_base+0xC);
	switch (size) {
	case 1:
		data = ioread8(base + where);
		break;
	case 2:
		data = ioread16(base + where);
		break;
	case 4:
		data = ioread32(base + where);
		break;
	default:
		spin_unlock_irqrestore(&ctrl->lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&ctrl->lock, flags);

	*val = data;

	return PCIBIOS_SUCCESSFUL;
}

static int rtl_pcie_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
	struct rtl_pci_priv *ctrl = pci_bus_to_rtl_priv(bus);
	unsigned long flags;
	void __iomem *base;

	if (ctrl->bus_number == 0xff)
		ctrl->bus_number = bus->number;

	if (bus->number != ctrl->bus_number)
		return PCIBIOS_DEVICE_NOT_FOUND;

	switch (PCI_SLOT(devfn)) {
	case 0:
		base = ctrl->hostcfg_base;
		break;
	case 1:
		base = ctrl->devcfg_base;
		break;
	default:
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	spin_lock_irqsave(&ctrl->lock, flags);
	iowrite32(PCI_FUNC(devfn), ctrl->hostext_base+0xC);
	switch (size) {
	case 1:
		iowrite8(val, base + where);
		break;
	case 2:
		iowrite16(val, base + where);
		break;
	case 4:
		iowrite32(val, base + where);
		break;
	default:
		spin_unlock_irqrestore(&ctrl->lock, flags);
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}
	spin_unlock_irqrestore(&ctrl->lock, flags);
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops rtl_pci_ops = {
	.read = rtl_pcie_read,
	.write = rtl_pcie_write
};

static const struct pcie_para pcie0_phy_params_revC[] __initconst = {
	{ 0x01, 0xa852 },{ 0x06, 0x0017 },{ 0x08, 0x3591 },{ 0x09, 0x520c },
	{ 0x0a, 0xf670 },{ 0x0b, 0xa90d },{ 0x0d, 0xe720 },{ 0x0e, 0x1010 },
	{ 0x1c, 0x2001 },{ 0x1e, 0x66eb },{ 0x20, 0xd4a4 },{ 0x21, 0x485a },
	{ 0x23, 0x0b66 },{ 0x24, 0x4f0c },{ 0x29, 0xf0f3 },{ 0x2b, 0xa0a1 },
	{ 0x09, 0x500c },{ 0x09, 0x520c },

	{ 0x41, 0xa849 },{ 0x46, 0x0017 },{ 0x48, 0x3591 },{ 0x49, 0x520c },
	{ 0x4a, 0xf650 },{ 0x4b, 0xa90d },{ 0x4d, 0xe720 },{ 0x4e, 0x1010 },
	{ 0x5c, 0x2001 },{ 0x60, 0xd4a6 },{ 0x61, 0x586a },{ 0x63, 0x0b66 },
	{ 0x69, 0xf0f3 },{ 0x6b, 0xa0a1 },{ 0x6f, 0x5046 },
	{ 0x49, 0x500c },{ 0x49, 0x520c }
};

static const struct pcie_para pcie1_phy_params_revC[] __initconst = {
	{ 0x01, 0xa852 },{ 0x06, 0x0017 },{ 0x08, 0x3591 },{ 0x09, 0x520c },
	{ 0x0a, 0xf670 },{ 0x0b, 0xa90d },{ 0x0d, 0xe720 },{ 0x0e, 0x1010 },
	{ 0x1c, 0x2001 },{ 0x1e, 0x66eb },{ 0x20, 0xd4a4 },{ 0x21, 0x485a },
	{ 0x23, 0x0b66 },{ 0x24, 0x4f0c },{ 0x29, 0xf0f3 },{ 0x2b, 0xa0a1 },
	{ 0x09, 0x500c },{ 0x09, 0x520c }
};

static int rtl_pcie_reset(struct rtl_pci_priv *p)
{
	u32 tmp;
	int bits = (p->port) ? (1 << 21) : (1 << 24 | 1 << 30);
	const struct pcie_para *phy = p->phy_param;
	void __iomem *mdiobase = p->hostext_base + 0x0;
	int retry = 0;
	u8 val;

RETRY:
	// 0. Assert PCIE Device Reset
	gpiod_set_raw_value(p->gpiod, 0);

	// 1. PCIE phy mdio reset
	tmp = ioread32(RTL_PCI_MISC);
	tmp &= (p->port ? ~(1 << 14) : ~(1 << 15));
	iowrite32(tmp, RTL_PCI_MISC);

	iowrite32(ioread32(RTL_PCI_MISC) & ~bits, RTL_PCI_MISC);
	iowrite32(ioread32(RTL_PCI_MISC) | bits, RTL_PCI_MISC);

	// 2. PCIE MAC reset
	tmp = (p->port) ? RTL_ENABLE_PCIE1 : RTL_ENABLE_PCIE0;
	iowrite32(ioread32(RTL_IP_SEL) & ~tmp, RTL_IP_SEL);
	iowrite32(ioread32(RTL_IP_SEL) | tmp, RTL_IP_SEL);
	mdelay(10);

	iowrite32(ioread32(RTL_PCI_MISC) | bits, RTL_PCI_MISC);

	iowrite32(0x1, p->hostext_base+0x8);	//bit7 PHY reset=0   bit0 Enable LTSSM=1
	iowrite32(0x81, p->hostext_base+0x8);   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	mdelay(50);

	for (size_t i = 0; i < p->phy_param_size; i++) {
		tmp = ((phy[i].value & 0xffff) << 16) | ((phy[i].reg & 0xff) << 8) | 1;
		iowrite32(tmp, mdiobase);
		mdelay(1);
	}

	mdelay(20); //TPERST#-CLK min 100us

	// PCIE Device Reset
	gpiod_set_raw_value(p->gpiod, 1);

	// wait for LinkUP
	tmp = 10;
	while(--tmp) {
		if((ioread32(p->hostcfg_base + 0x0728) & 0x1f) == 0x11)
			break;

		mdelay(10);
	}

	if (tmp == 0) {
		pr_warn("Warning!! Port %d PCIE Link Failed, State=0x%x retry=%d\n", p->port, ioread32(p->hostcfg_base + 0x0728), retry);

		if (retry < 3) {
			retry ++;
			goto RETRY;
		}

		// tmp - pci known to fail waiting for realtek
		//return -1;
		return -1; //tmp for GPIO check
	}

	mdelay(100); // allow time for CR

	// Enable PCIE host
	iowrite32(0x00100007, p->hostcfg_base + 0x04);
	val = (ioread8(p->hostcfg_base + 0x78) & (~0xe0)) | 0;
	iowrite8(val, p->hostcfg_base + 0x78); // Set MAX_PAYLOAD_SIZE to 128B

	iowrite32(ioread32(p->hostcfg_base + 0x80C) | (1 << 17), p->hostcfg_base + 0x80C);

	const char *str;
	mdelay(1);

	// Read Status
	switch (ioread16(p->hostcfg_base + 0x82) & 0xf) {
	case 1:
		str = "2.5GHz";
		break;
	case 2:
		str = "5.0Ghz";
		break;
	default:
		str = "Unknown";
		break;
	}
	pr_info("Port%d Link@%s\n", p->port, str);

	return 0;
}

static int rtl_pci_probe(struct platform_device *pdev)
{
	struct rtl_pci_priv *pdata;
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;
	int error;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->bus_number = 0xff;

	pdata->hostcfg_base = devm_platform_get_and_ioremap_resource(pdev, 0, NULL);
	if (IS_ERR(pdata->hostcfg_base))
		return PTR_ERR(pdata->hostcfg_base);

	pdata->hostext_base = devm_platform_get_and_ioremap_resource(pdev, 1, NULL);
	if (IS_ERR(pdata->hostext_base))
		return PTR_ERR(pdata->hostext_base);

	pdata->devcfg_base = devm_platform_get_and_ioremap_resource(pdev, 2, NULL);
	if (IS_ERR(pdata->devcfg_base))
		return PTR_ERR(pdata->devcfg_base);

	error = of_property_read_u32(node, "port", &pdata->port);

	if (error) {
		dev_err(dev, "No 'port' property found\n");
		return error;
	}

	if (pdata->port == 0) {
		pdata->phy_param = pcie0_phy_params_revC;
		pdata->phy_param_size = ARRAY_SIZE(pcie0_phy_params_revC);
	}else if (pdata->port == 1) {
		pdata->phy_param = pcie1_phy_params_revC;
		pdata->phy_param_size = ARRAY_SIZE(pcie1_phy_params_revC);
	}else {
		return -EINVAL;
	}

	spin_lock_init(&pdata->lock);

	/* setup reset gpio used by pci */
	// TODO not optional
	pdata->gpiod = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	error = PTR_ERR_OR_ZERO(pdata->gpiod);
	if (error) {
		dev_err(dev, "failed to request gpio: %d\n", error);
		return error;
	}
	gpiod_set_consumer_name(pdata->gpiod, "pci_reset");

	pdata->controller.pci_ops = &rtl_pci_ops;
	pdata->controller.io_resource = devm_kzalloc(dev, sizeof(struct resource), GFP_ATOMIC);
	pdata->controller.mem_resource = devm_kzalloc(dev, sizeof(struct resource), GFP_ATOMIC);

	if (!pdata->controller.io_resource || !pdata->controller.mem_resource)
		return -ENOMEM;

	pci_load_of_ranges(&pdata->controller, node);

	if (rtl_pcie_reset(pdata))
		return -1;

	// TODO do it in dts?
	//insert_resource(&ioport_resource, pdata->controller.io_resource);

	// TODO move to dts, or request dynamically?
	// ioport must include io space for all ports
	// How to do it right - not clear right now
	ioport_resource.start = 0x18c00000;
	ioport_resource.end = 0x19000000 - 1;

	register_pci_controller(&pdata->controller);

	return 0;
}

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

int pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	return of_irq_parse_and_map_pci(dev, slot, pin);
}


static const struct of_device_id rtl_pci_match[] = {
	{ .compatible = "realtek,pci-rtl960x" },
	{},
};

static struct platform_driver rtl_pci_driver = {
	.probe = rtl_pci_probe,
	.driver = {
		.name = "pci-rtl960x",
		.of_match_table = rtl_pci_match,
	},
};

int __init rtl_pci_init(void);
int __init rtl_pci_init(void)
{
	return platform_driver_register(&rtl_pci_driver);
}

arch_initcall(rtl_pci_init);
