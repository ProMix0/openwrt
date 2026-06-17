// SPDX-License-Identifier: GPL-2.0-only

#include <linux/fwnode.h>
#include <linux/fwnode_mdio.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/types.h>

#define RTMDIO_MAX_PHY				4
#define RTMDIO_PAGE_SELECT			0x1f

/* MDIO bus registers/fields */
#define RTMDIO_RUN				BIT(0)

#define RTMDIO_960X_GPHY_IND_WD			(0x0)
#define RTMDIO_960X_GPHY_IND_CMD		(0x4)
#define   RTMDIO_960X_GPHY_CMD_WREN		BIT(22)
#define   RTMDIO_960X_GPHY_CMD_EN		BIT(21)
#define RTMDIO_960X_GPHY_IND_RD			(0x8)
#define   RTMDIO_960X_GPHY_BUSY			BIT(16)
#define RTMDIO_960X_WRAP_GPHY_MISC		(0x114)
#define   RTMDIO_960X_PHY_PATCH_DONE		BIT(0)
#define RTMDIO_960X_OCP_PHY_BASE		(0xa400)

#define rtmdio_ctrl_from_bus(bus) \
	(((struct rtmdio_chan *)(bus)->priv)->ctrl)

struct rtmdio_port {
	struct device_node *dn;
	int page;
};

struct rtmdio_ctrl {
	struct mutex lock;
	struct regmap *map;
	const struct rtmdio_config *cfg;
};

struct rtmdio_chan {
	struct rtmdio_ctrl *ctrl;
	struct rtmdio_port port[RTMDIO_MAX_PHY];
};

struct rtmdio_config {
	int (*read_phy)(struct mii_bus *bus, u32 addr, u32 page, u32 reg, u32 *val);
	int (*write_phy)(struct mii_bus *bus, u32 addr, u32 page, u32 reg, u32 val);
};

static int rtmdio_960x_internal_run_cmd(struct mii_bus *bus, u32 addr, u32 ocp_addr, u32 cmd)
{
	struct rtmdio_ctrl *ctrl = rtmdio_ctrl_from_bus(bus);
	int ret, val;

	ret = regmap_write(ctrl->map, RTMDIO_960X_GPHY_IND_CMD, cmd | addr << 16 | ocp_addr);

	ret = regmap_read_poll_timeout(ctrl->map, RTMDIO_960X_GPHY_IND_RD, val, !(val & RTMDIO_960X_GPHY_BUSY), 20, 500000);

	return WARN_ONCE(ret, "internal mdio bus access timed out\n");
}

static int rtmdio_960x_write_internal_phy(struct mii_bus *bus, u32 addr, u32 page, u32 reg, u32 val)
{
	struct rtmdio_ctrl *ctrl = rtmdio_ctrl_from_bus(bus);
	u32 ocp_addr = RTMDIO_960X_OCP_PHY_BASE + reg * 2;

	if (reg > 15  && reg < 24)
		ocp_addr = (page << 4) + (reg - 16) * 2;

	regmap_write(ctrl->map, RTMDIO_960X_GPHY_IND_WD, val);

	return rtmdio_960x_internal_run_cmd(bus, addr, ocp_addr, RTMDIO_960X_GPHY_CMD_WREN | RTMDIO_960X_GPHY_CMD_EN);
}

static int rtmdio_960x_read_internal_phy(struct mii_bus *bus, u32 addr, u32 page, u32 reg, u32 *val)
{
	struct rtmdio_ctrl *ctrl = rtmdio_ctrl_from_bus(bus);
	u32 ocp_addr = RTMDIO_960X_OCP_PHY_BASE + reg * 2;
	int err;

	if (reg > 15  && reg < 24)
		ocp_addr = (page << 4) + (reg - 16) * 2;

	err = rtmdio_960x_internal_run_cmd(bus, addr, ocp_addr, RTMDIO_960X_GPHY_CMD_EN);

	if (!err)
		err = regmap_read(ctrl->map, RTMDIO_960X_GPHY_IND_RD, val);
	if (!err)
		*val &= 0xffff;

	return err;
}

static int rtmdio_read(struct mii_bus *bus, int phy, int regnum)
{
	struct rtmdio_ctrl *ctrl = rtmdio_ctrl_from_bus(bus);
	struct rtmdio_chan *priv = (struct rtmdio_chan *)(bus)->priv;
	int err, val;

	if (phy >= RTMDIO_MAX_PHY) return -ENODEV;

	guard(mutex)(&ctrl->lock);
	if (regnum == RTMDIO_PAGE_SELECT)
		return priv->port[phy].page;

	err = (*ctrl->cfg->read_phy)(bus, phy, priv->port[phy].page, regnum, &val);
	pr_debug("rd_PHY(adr=%d, pag=%d, reg=%d) = %d, err = %d\n",
		 phy, priv->port[phy].page, regnum, val, err);
	return err ? err : val;
}

static int rtmdio_write(struct mii_bus *bus, int phy, int regnum, u16 val)
{
	struct rtmdio_ctrl *ctrl = rtmdio_ctrl_from_bus(bus);
	struct rtmdio_chan *priv = (struct rtmdio_chan *)(bus)->priv;
	int err, page;

	if (phy >= RTMDIO_MAX_PHY) return -ENODEV;

	guard(mutex)(&ctrl->lock);
	page = priv->port[phy].page;

	if (regnum == RTMDIO_PAGE_SELECT) {
		priv->port[phy].page = val & GENMASK(11, 0); // 4096 max
		return 0;
	}

	err = (*ctrl->cfg->write_phy)(bus, phy, page, regnum, val);
	pr_debug("wr_PHY(adr=%d, pag=%d, reg=%d, val=%d) err = %d\n",
		 phy, page, regnum, val, err);
	return err;
}

static int rtmdio_probe_bus(struct device *dev, struct rtmdio_ctrl *ctrl)
{
	struct rtmdio_chan *chan;
	struct mii_bus *bus;
	int ret;

	bus = devm_mdiobus_alloc_size(dev, sizeof(*chan));
	if (!bus)
		return -ENOMEM;

	chan = bus->priv;
	chan->ctrl = ctrl;

	bus->name = "Realtek MDIO bus";
	bus->read = rtmdio_read;
	bus->write = rtmdio_write;
	snprintf(bus->id, MII_BUS_ID_SIZE, "realtek-mdio-int");

	ret = devm_of_mdiobus_register(dev, bus, dev->of_node);
	if (ret)
		return dev_err_probe(dev, ret, "cannot register MDIO bus\n");

	return 0;
}

static int rtmdio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtmdio_ctrl *ctrl;
	int ret;

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	ret = devm_mutex_init(dev, &ctrl->lock);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, ctrl);
	ctrl->cfg = (const struct rtmdio_config *)device_get_match_data(dev);
	ctrl->map = syscon_node_to_regmap(pdev->dev.of_node->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	ret = rtmdio_probe_bus(dev, ctrl);
	if (ret)
		return ret;

	/*
	 * PHY_PATCH_DONE enables phy control via SoC. This is required for phy access,
	 * including patching. Must always be set before the phys are probed.
	 */
	regmap_update_bits(ctrl->map, RTMDIO_960X_WRAP_GPHY_MISC,
			   RTMDIO_960X_PHY_PATCH_DONE, RTMDIO_960X_PHY_PATCH_DONE);
	msleep(100);

	return 0;
}

static const struct rtmdio_config rtmdio_960x_int_cfg = {
	.read_phy	= rtmdio_960x_read_internal_phy,
	.write_phy	= rtmdio_960x_write_internal_phy,
};

static const struct of_device_id rtmdio_ids[] = {
	{
		.compatible = "realtek,rtl9607-int-mdio",
		.data = &rtmdio_960x_int_cfg,
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtmdio_ids);

static struct platform_driver rtmdio_driver = {
	.probe = rtmdio_probe,
	.driver = {
		.name = "mdio-rtl9607-int",
		.of_match_table = rtmdio_ids,
	},
};

module_platform_driver(rtmdio_driver);

MODULE_DESCRIPTION("RTL9607C INT-MDIO driver");
MODULE_LICENSE("GPL");
