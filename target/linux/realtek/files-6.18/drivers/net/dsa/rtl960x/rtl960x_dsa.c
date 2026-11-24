// SPDX-License-Identifier: GPL-2.0-only
/*
 * DSA switch driver for the Realtek RTL9607C family switch core.
 *
 * The RTL9607C family of SoCs has an embedded 11 port switch controller.
 * rtl960x_dsa_phylink_get_caps() describes the ports this driver supports.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/if_bridge.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/soc/realtek/otto_table.h>
#include <net/dsa.h>

#include "rtl960x_dsa.h"
#include "rtl960x_l2.h"
#include "rtl960x_vlan.h"

/* Forced MAC ability of a port, used to force the CPU ports' link up */
#define RTL960X_FORCE_P_ABLTY_REG(_port)	(0x1cc + (_port) * 4)
#define   RTL960X_FORCE_P_ABLTY_NWAY		BIT(7)
#define   RTL960X_FORCE_P_ABLTY_TX_PAUSE	BIT(6)
#define   RTL960X_FORCE_P_ABLTY_RX_PAUSE	BIT(5)
#define   RTL960X_FORCE_P_ABLTY_LINK		BIT(4)
#define   RTL960X_FORCE_P_ABLTY_DUPLEX		BIT(2)
#define   RTL960X_FORCE_P_ABLTY_SPEED_MASK	GENMASK(1, 0)
#define   RTL960X_FORCE_P_ABLTY_SPEED23_MASK	GENMASK(13, 12)
#define   RTL960X_FORCE_P_ABLTY_SPEED_1000	2
#define RTL960X_ABLTY_FORCE_MODE_REG(_port)	(0x238 + (_port) * 4)
#define   RTL960X_ABLTY_FORCE_MODE_ENABLE	0xbfff

/*
 * SerDes (SDS) programming for port 6 ("HSG0"), ported 1:1 from the vendor
 * SDK's _rtl9607c_lan_sds0_modeV3_set() (chip rev C, SGMII_Patch_20211214 /
 * 2500Base-X_Patch_20211214).
 * Register offsets are in the switch0 register space, identical to the SDK's
 * rtk_rtl9607c_reg_list.c (offset, 4-byte per-index stride for arrayed regs).
 *
 * The "common IP enable" registers live in the soc peripheral space instead
 * (SDK 0xB8000600 / 0xB8000508) and are handled via ioremap in probe().
 */
#define RTL960X_WRAP_GPHY_MISC			0x114
#define   RTL960X_PATCH_PHY_DONE		BIT(0)
#define RTL960X_CPU_PLL_CTRL_DUMMY		0x354
#define   RTL960X_SPDSEL_SGMII_USB3_MASK	GENMASK(31, 30)
#define   RTL960X_SGMII_SEL_USB3		BIT(29)
#define RTL960X_SDS_CFG_REG(_sds)		(0x270 + (_sds) * 4)
#define   RTL960X_CFG_COMBO_SDS_EN		BIT(5)
#define   RTL960X_CFG_SDS_MODE_MASK		GENMASK(4, 0)
#define RTL960X_UTP_FIBER_AUTODET_REG(_sds)	(0x2c0 + (_sds) * 4)
#define   RTL960X_SDS_PHY_MODE			BIT(10)
#define   RTL960X_SDS_CMD_STOP_GLI_CLK		BIT(8)
#define RTL960X_LINE_RATE_2500M_0		0x2d96c
#define   RTL960X_EN_RATE_2500M			BIT(19)
#define   RTL960X_RATE_MASK			GENMASK(18, 0)
#define   RTL960X_RATE_2500M_VAL		0x4ffff /* SDK value, semantics undocumented */
#define RTL960X_WSDS_HSG0_ANA_00		0x41000
#define   RTL960X_CFG_MDIO_RSTB			BIT(3)
#define RTL960X_WSDS_HSG0_DIG_01		0x41034
#define   RTL960X_CFG_SDS_MDX_REQ		BIT(15)
#define RTL960X_WSDS_HSG0_DIG_02		0x41038
#define RTL960X_HSG0_FIB_REG0			0x41c00
#define   RTL960X_FP_CFG_FIB_PDOWN		BIT(11)
#define RTL960X_HSG0_FIB_EXT_REG19		0x41e4c
#define   RTL960X_FEP_CFG_TX_MODE		GENMASK(15, 14)

/* Port 6 is the first (index 1) HSGMII-capable SerDes, "HSG0" */
#define RTL960X_SDS_HSG0			1

/* CFG_SDS_MODE field codes (RTL960X_CFG_SDS_MODE_MASK) */
#define RTL960X_SDS_MODE_SGMII			0x2
#define RTL960X_SDS_MODE_2500BASEX		0x16
#define RTL960X_SDS_MODE_RESET			0x1f

/* "Enable common IP" physical addresses (SDK 0xB8000600 / 0xB8000508) */
#define RTL960X_IP_COMMON_ADDR		0x18000600
#define   RTL960X_IP_COMMON_BIT		BIT(29)
#define RTL960X_IP_EN_ADDR		0x18000508
#define   RTL960X_IP_EN_BITS		(BIT(27) | BIT(28))

/*
 * Preferred CPU port for the user ports when the device tree describes more
 * than one. Port 7 can also be a CPU port (GMAC2) but is muxed with the
 * second HSGMII SerDes, so keep GMAC0 on port 9 as the default.
 */
#define RTL960X_DEFAULT_CPU_PORT		9

/*
 * Port isolation registers - one egress portmask per port. Bits 28:11 are
 * the extension ports of MAC7/9/10, which the driver leaves clear.
 */
#define RTL960X_PORT_ISOLATION_REG(_port)	(0x27000 + (_port) * 4)
#define   RTL960X_PORT_ISOLATION_MASK		GENMASK(10, 0)

/* MSTP port state registers - one per port, a 2-bit state per MSTI */
#define RTL960X_MSTI_CTRL_REG(_port)		(0x1704c + (_port) * 4)
#define   RTL960X_MSTI_CTRL_PORT_STATE_OFFSET(_msti)	((_msti) << 1)
#define   RTL960X_MSTI_CTRL_PORT_STATE_MASK(_msti) \
		(0x3 << RTL960X_MSTI_CTRL_PORT_STATE_OFFSET(_msti))

/*
 * Per-port L2 learning limit, two 12-bit counts per register. A limit of 0
 * disables learning; the register maximum 0xfff disables the limit.
 */
#define RTL960X_LUT_LRN_LIMITNO_REG(_port)	(0x17008 + ((_port) / 2) * 4)
#define   RTL960X_LUT_LRN_LIMITNO_SHIFT(_port)	(((_port) % 2) * 12)
#define   RTL960X_LUT_LRN_LIMITNO_MASK(_port)	(GENMASK(11, 0) << \
					 RTL960X_LUT_LRN_LIMITNO_SHIFT(_port))
/* What to do with a frame once the port's limit is reached, 2 bits per port */
#define RTL960X_LUT_LEARN_OVER_CTRL_REG		0x1c010
#define   RTL960X_LUT_LEARN_OVER_ACT_MASK(_port)	(0x3 << ((_port) * 2))
#define   RTL960X_LUT_LEARN_OVER_ACT_FORWARD	0	/* forward, do not learn */

/* Per port MIB statistics */
#define RTL960X_STAT_PORT_MIB_BASE		(0x32000)
#define RTL960X_STAT_PORT_MIB(x)		(RTL960X_STAT_PORT_MIB_BASE + (x) * 0x80)
/* Each define is an offset of RTL960X_STAT_PORT_MIB */
#define   RTL960X_IF_OUT_BROADCAST_PKTS		(0x64)
#define   RTL960X_IF_OUT_MULTICAST_PKTS		(0x60)
#define   RTL960X_IF_OUT_UCAST_PKTS		(0x5c)
#define   RTL960X_DOT1D_TP_PORT_IN_DISCARDS	(0x58)
#define   RTL960X_ETHER_STATS_TX_PKTS_1519_TO_MAX_OCTETS	(0x50)
#define   RTL960X_IF_OUT_DISCARDS		(0x4c)
#define   RTL960X_DOT3_OUT_PAUSE_FRAMES		(0x48)
#define   RTL960X_DOT3_STATS_EXCESSIVE_COLLISIONS	(0x44)
#define   RTL960X_ETHER_STATS_COLLISIONS	(0x40)
#define   RTL960X_DOT3_STATS_LATE_COLLISIONS	(0x3c)
#define   RTL960X_DOT3_STATS_DEFERRED_TRANSMISSIONS	(0x38)
#define   RTL960X_DOT3_STATS_MULTIPLE_COLLISION_FRAMES	(0x34)
#define   RTL960X_DOT3_STATS_SINGLE_COLLISION_FRAMES	(0x30)
#define   RTL960X_IF_OUT_OCTETS			(0x28) /* 64 bits */
#define   RTL960X_ETHER_STATS_TX_PKTS_1024_TO_1518_OCTETS	(0x24)
#define   RTL960X_ETHER_STATS_TX_PKTS_512_TO_1023_OCTETS	(0x20)
#define   RTL960X_ETHER_STATS_TX_PKTS_256_TO_511_OCTETS	(0x1c)
#define   RTL960X_ETHER_STATS_TX_PKTS_128_TO_255_OCTETS	(0x18)
#define   RTL960X_ETHER_STATS_TX_PKTS_65_TO_127_OCTETS	(0x14)
#define   RTL960X_ETHER_STATS_TX_PKTS_64_OCTETS	(0x10)
#define   RTL960X_ETHER_STATS_TX_OVERSIZE_PKTS	(0xc)
#define   RTL960X_ETHER_STATS_TX_UNDERSIZE_PKTS	(0x8)
#define   RTL960X_ETHER_STATS_TX_BROADCAST_PKTS	(0x4)
#define   RTL960X_ETHER_STATS_TX_MULTICAST_PKTS	(0x0)
#define STAT_RX_OFFSET	(0x600)
#define   RTL960X_ETHER_STATS_RX_PKTS_1024_TO_1518_OCTETS	(STAT_RX_OFFSET + 0x54)
#define   RTL960X_ETHER_STATS_RX_PKTS_512_TO_1023_OCTETS	(STAT_RX_OFFSET + 0x50)
#define   RTL960X_ETHER_STATS_RX_PKTS_256_TO_511_OCTETS	(STAT_RX_OFFSET + 0x4c)
#define   RTL960X_ETHER_STATS_RX_PKTS_128_TO_255_OCTETS	(STAT_RX_OFFSET + 0x48)
#define   RTL960X_ETHER_STATS_RX_PKTS_65_TO_127_OCTETS	(STAT_RX_OFFSET + 0x44)
#define   RTL960X_ETHER_STATS_RX_PKTS_64_OCTETS	(STAT_RX_OFFSET + 0x40)
#define   RTL960X_ETHER_STATS_RX_OVERSIZE_PKTS	(STAT_RX_OFFSET + 0x3c)
#define   RTL960X_ETHER_STATS_RX_UNDERSIZE_PKTS	(STAT_RX_OFFSET + 0x38)
#define   RTL960X_ETHER_STATS_RX_PKTS_1519_TO_MAX_OCTETS	(STAT_RX_OFFSET + 0x30)
#define   RTL960X_IF_IN_BROADCAST_PKTS		(STAT_RX_OFFSET + 0x2c)
#define   RTL960X_IF_IN_MULTICAST_PKTS		(STAT_RX_OFFSET + 0x28)
#define   RTL960X_ETHER_STATS_DROP_EVENTS	(STAT_RX_OFFSET + 0x24)
#define   RTL960X_IF_IN_UCAST_PKTS		(STAT_RX_OFFSET + 0x20)
#define   RTL960X_ETHER_STATS_JABBERS		(STAT_RX_OFFSET + 0x1c)
#define   RTL960X_ETHER_STATS_FRAGMENTS		(STAT_RX_OFFSET + 0x18)
#define   RTL960X_DOT3_CONTROL_IN_UNKNOWN_OPCODES	(STAT_RX_OFFSET + 0x14)
#define   RTL960X_DOT3_IN_PAUSE_FRAMES		(STAT_RX_OFFSET + 0x10)
#define   RTL960X_DOT3_STATS_SYMBOL_ERRORS	(STAT_RX_OFFSET + 0xc)
#define   RTL960X_ETHER_STATS_CRC_ALIGN_ERRORS	(STAT_RX_OFFSET + 0x8)
#define   RTL960X_IF_IN_OCTETS			(STAT_RX_OFFSET + 0x0) /* 64 bits */
#define RTL960X_STAT_PORT_OAM_MIB(x)		(RTL960X_STAT_PORT_MIB_BASE + (x) * 0x8)
#define STAT_OAM_OFFSET	(0xc00)
#define   RTL960X_OUT_OAM_PDU_PKTS	(STAT_OAM_OFFSET + 0x0)
#define   RTL960X_IN_OAM_PDU_PKTS	(STAT_OAM_OFFSET + 0x4)

#define RTL960X_STAT_CTRL		0x34000
#define RTL960X_STAT_PORT_RESET		0x34014
#define RTL960X_STAT_RESET		0x34018
#define   STAT_RESET_STATUS		BIT(3)
#define RTL960X_STAT_RESET_CONFIG	0x3401c
#define   STAT_RESET_CMD		BIT(5)

#define RTL960X_MAX_PACKET_LENGTH	0x3fff

struct rtl960x_mib_counter {
	u32 offset;
	u32 length;
	const char *name;
};

#define MIB_DESC(_o, _l, _n)	\
	{			\
		.offset = (_o),	\
		.length = (_l),	\
		.name = (_n),	\
	}

static const struct rtl960x_mib_counter rtl960x_mib_counters[] = {
	MIB_DESC(RTL960X_ETHER_STATS_TX_MULTICAST_PKTS, 1, "etherStatsTxMulticastPkts"),
	MIB_DESC(RTL960X_ETHER_STATS_TX_BROADCAST_PKTS, 1, "etherStatsTxBroadcastPkts"),
	MIB_DESC(RTL960X_ETHER_STATS_TX_UNDERSIZE_PKTS, 1, "etherStatsTxUndersizePkts"),
	MIB_DESC(RTL960X_ETHER_STATS_TX_OVERSIZE_PKTS, 1, "etherStatsTxOversizePkts"),
	MIB_DESC(RTL960X_ETHER_STATS_COLLISIONS, 1, "etherStatsCollisions"),
	MIB_DESC(RTL960X_IF_OUT_DISCARDS, 1, "ifOutDiscards"),
	MIB_DESC(RTL960X_DOT1D_TP_PORT_IN_DISCARDS, 1, "dot1dTpPortInDiscards"),
	MIB_DESC(RTL960X_ETHER_STATS_DROP_EVENTS, 1, "etherStatsDropEvents"),
	MIB_DESC(RTL960X_ETHER_STATS_CRC_ALIGN_ERRORS, 1, "etherStatsCrcAlignErrors"),
};

enum rtl960x_stp_state {
	RTL960X_STP_STATE_DISABLED = 0,
	RTL960X_STP_STATE_BLOCKING = 1,
	RTL960X_STP_STATE_LEARNING = 2,
	RTL960X_STP_STATE_FORWARDING = 3,
};

static int rtl960x_cpu_port_setup(struct rtl960x_dsa *priv, int port)
{
	int ret;

	/* Force the link to the GMAC up at 1000/full */
	ret = regmap_write(priv->map, RTL960X_FORCE_P_ABLTY_REG(port),
			   RTL960X_FORCE_P_ABLTY_NWAY |
			   RTL960X_FORCE_P_ABLTY_LINK |
			   RTL960X_FORCE_P_ABLTY_DUPLEX |
			   FIELD_PREP(RTL960X_FORCE_P_ABLTY_SPEED_MASK,
				      RTL960X_FORCE_P_ABLTY_SPEED_1000));
	if (ret)
		return ret;

	return regmap_write(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(port),
			    0xffff);
}

static int rtl960x_port_set_isolation(struct rtl960x_dsa *priv, int port,
				      u32 mask)
{
	return regmap_write(priv->map, RTL960X_PORT_ISOLATION_REG(port), mask);
}

static int rtl960x_port_add_isolation(struct rtl960x_dsa *priv, int port,
				      u32 mask)
{
	return regmap_set_bits(priv->map, RTL960X_PORT_ISOLATION_REG(port),
			       mask);
}

static int rtl960x_port_remove_isolation(struct rtl960x_dsa *priv, int port,
					 u32 mask)
{
	return regmap_clear_bits(priv->map, RTL960X_PORT_ISOLATION_REG(port),
				 mask);
}

static int rtl960x_port_mib_reset(struct rtl960x_dsa *priv, u32 pmask)
{
	u32 val;
	int ret;

	ret = regmap_set_bits(priv->map, RTL960X_STAT_PORT_RESET, pmask);
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->map, RTL960X_STAT_RESET_CONFIG, STAT_RESET_CMD);
	if (ret)
		return ret;

	return regmap_read_poll_timeout(priv->map, RTL960X_STAT_RESET, val,
					!FIELD_GET(STAT_RESET_STATUS, val),
					10, 10000);
}

static void rtl960x_mib_counter_read(struct rtl960x_dsa *priv, int port, u32 offset,
				     u32 length, u64 *mibvalue)
{
	u32 low, high, high1, reg = RTL960X_STAT_PORT_MIB(port) + offset;

	if (length == 2) {
		regmap_read(priv->map, reg + 4, &high);
		regmap_read(priv->map, reg, &low);
		*mibvalue = low;
		regmap_read(priv->map, reg + 4, &high1);
		if (high != high1) {
			regmap_read(priv->map, reg, &low);
			*mibvalue = low;
		}
		*mibvalue |= (u64)high1 << 32;
	} else {
		regmap_read(priv->map, reg, &low);
		*mibvalue = low;
	}
}

static void rtl960x_get_ethtool_stats(struct dsa_switch *ds, int port, u64 *data)
{
	struct rtl960x_dsa *priv = ds->priv;
	const struct rtl960x_mib_counter *mib;
	int i;

	for (i = 0; i < ARRAY_SIZE(rtl960x_mib_counters); i++) {
		mib = &rtl960x_mib_counters[i];

		rtl960x_mib_counter_read(priv, port, mib->offset, mib->length, data + i);
	}
}

static void rtl960x_get_strings(struct dsa_switch *ds, int port, u32 stringset, u8 *data)
{
	if (stringset != ETH_SS_STATS)
		return;

	for (int i = 0; i < ARRAY_SIZE(rtl960x_mib_counters); i++)
		ethtool_puts(&data, rtl960x_mib_counters[i].name);
}

static int rtl960x_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
	if (sset != ETH_SS_STATS)
		return -EOPNOTSUPP;

	return ARRAY_SIZE(rtl960x_mib_counters);
}

static void rtl960x_get_phy_stats(struct dsa_switch *ds, int port,
				  struct ethtool_eth_phy_stats *phy_stats)
{
	struct rtl960x_dsa *priv = ds->priv;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_SYMBOL_ERRORS, 1,
				 &phy_stats->SymbolErrorDuringCarrier);
}

static void rtl960x_get_mac_stats(struct dsa_switch *ds, int port,
				  struct ethtool_eth_mac_stats *mac_stats)
{
	struct rtl960x_dsa *priv = ds->priv;
	u64 data;

	/* The RTL960X exposes MIB objects, which we have to translate into
	 * IEEE 802.3 Managed Objects. This is not always completely faithful,
	 * but we try out best. See RFC 3635 for a detailed treatment of the
	 * subject.
	 */

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_UCAST_PKTS, 1,
				 &mac_stats->FramesTransmittedOK);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_SINGLE_COLLISION_FRAMES, 1,
				 &mac_stats->SingleCollisionFrames);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_MULTIPLE_COLLISION_FRAMES, 1,
				 &mac_stats->MultipleCollisionFrames);

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_UCAST_PKTS, 1,
				 &mac_stats->FramesReceivedOK);

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_OCTETS, 2,
				 &mac_stats->OctetsTransmittedOK);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_DEFERRED_TRANSMISSIONS, 1,
				 &mac_stats->FramesWithDeferredXmissions);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_LATE_COLLISIONS, 1,
				 &mac_stats->LateCollisions);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_EXCESSIVE_COLLISIONS, 1,
				 &mac_stats->FramesAbortedDueToXSColls);

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_OCTETS, 2,
				 &mac_stats->OctetsReceivedOK);

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_MULTICAST_PKTS, 1,
				 &mac_stats->MulticastFramesXmittedOK);
	mac_stats->FramesTransmittedOK += mac_stats->MulticastFramesXmittedOK;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_BROADCAST_PKTS, 1,
				 &mac_stats->BroadcastFramesXmittedOK);
	mac_stats->FramesTransmittedOK += mac_stats->BroadcastFramesXmittedOK;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_OUT_PAUSE_FRAMES, 1,
				 &data);
	mac_stats->FramesTransmittedOK += data;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_DISCARDS, 1, &data);
	mac_stats->FramesTransmittedOK -= data;

	mac_stats->OctetsTransmittedOK -= 18 * mac_stats->FramesTransmittedOK;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_MULTICAST_PKTS, 1,
				 &mac_stats->MulticastFramesReceivedOK);
	mac_stats->FramesReceivedOK += mac_stats->MulticastFramesReceivedOK;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_BROADCAST_PKTS, 1,
				 &mac_stats->BroadcastFramesReceivedOK);
	mac_stats->FramesReceivedOK += mac_stats->BroadcastFramesReceivedOK;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_IN_PAUSE_FRAMES, 1,
				 &data);
	mac_stats->FramesReceivedOK += data;

	mac_stats->OctetsReceivedOK -= 18 * mac_stats->FramesReceivedOK;
}

static void rtl960x_get_ctrl_stats(struct dsa_switch *ds, int port,
				   struct ethtool_eth_ctrl_stats *ctrl_stats)
{
	struct rtl960x_dsa *priv = ds->priv;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_CONTROL_IN_UNKNOWN_OPCODES, 1,
				   &ctrl_stats->UnsupportedOpcodesReceived);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_OUT_PAUSE_FRAMES, 1,
				   &ctrl_stats->MACControlFramesTransmitted);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_IN_PAUSE_FRAMES, 1,
				   &ctrl_stats->MACControlFramesReceived);
}

static const struct ethtool_rmon_hist_range rtl960x_rmon_ranges[] = {
	{ 0, 64 },
	{ 65, 127 },
	{ 128, 255 },
	{ 256, 511 },
	{ 512, 1023 },
	{ 1024, 1518 },
	{ 1519, RTL960X_MAX_PACKET_LENGTH },
	{ }
};

static void rtl960x_get_rmon_stats(struct dsa_switch *ds, int port,
				   struct ethtool_rmon_stats *rmon_stats,
				   const struct ethtool_rmon_hist_range **ranges)
{
	struct rtl960x_dsa *priv = ds->priv;

	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_UNDERSIZE_PKTS, 1,
				 &rmon_stats->undersize_pkts);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_OVERSIZE_PKTS, 1,
				 &rmon_stats->oversize_pkts);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_FRAGMENTS, 1,
				 &rmon_stats->fragments);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_JABBERS, 1,
				 &rmon_stats->jabbers);

	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_64_OCTETS, 1,
				 &rmon_stats->hist[0]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_65_TO_127_OCTETS, 1,
				 &rmon_stats->hist[1]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_128_TO_255_OCTETS, 1,
				 &rmon_stats->hist[2]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_256_TO_511_OCTETS, 1,
				 &rmon_stats->hist[3]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_512_TO_1023_OCTETS, 1,
				 &rmon_stats->hist[4]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_1024_TO_1518_OCTETS, 1,
				 &rmon_stats->hist[5]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_RX_PKTS_1519_TO_MAX_OCTETS, 1,
				 &rmon_stats->hist[6]);

	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_64_OCTETS, 1,
				 &rmon_stats->hist_tx[0]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_65_TO_127_OCTETS, 1,
				 &rmon_stats->hist_tx[1]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_128_TO_255_OCTETS, 1,
				 &rmon_stats->hist_tx[2]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_256_TO_511_OCTETS, 1,
				 &rmon_stats->hist_tx[3]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_512_TO_1023_OCTETS, 1,
				 &rmon_stats->hist_tx[4]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_1024_TO_1518_OCTETS, 1,
				 &rmon_stats->hist_tx[5]);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_TX_PKTS_1519_TO_MAX_OCTETS, 1,
				 &rmon_stats->hist_tx[6]);

	*ranges = rtl960x_rmon_ranges;
}

static void rtl960x_get_pause_stats(struct dsa_switch *ds, int port,
				    struct ethtool_pause_stats *pause_stats)
{
	struct rtl960x_dsa *priv = ds->priv;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_OUT_PAUSE_FRAMES, 1,
				       &pause_stats->tx_pause_frames);

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_IN_PAUSE_FRAMES, 1,
				       &pause_stats->rx_pause_frames);
}

static void rtl960x_get_stats64(struct dsa_switch *ds, int port,
				struct rtnl_link_stats64 *s)
{

	struct rtl960x_dsa *priv = ds->priv;
	u64 data;

	/* MIB counter doesn't provide a FramesTransmittedOK but instead
	 * provide stats for Unicast, Broadcast and Multicast frames separately.
	 * To simulate a global frame counter, read Unicast and addition Multicast
	 * and Broadcast later
	 */

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_UCAST_PKTS, 1,
				 &s->rx_packets);
	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_MULTICAST_PKTS, 1,
				 &s->multicast);
	s->rx_packets += s->multicast;
	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_BROADCAST_PKTS, 1,
				 &data);
	s->rx_packets += data;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_UCAST_PKTS, 1,
				 &s->tx_packets);
	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_MULTICAST_PKTS, 1,
				 &data);
	s->tx_packets += data;
	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_BROADCAST_PKTS, 1,
				 &data);
	s->tx_packets += data;

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_IN_OCTETS, 2,
				 &s->rx_bytes);
	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_OCTETS, 2,
				 &s->tx_bytes);

	/* if{In,Out}Octets includes FCS - remove it */
	s->rx_bytes -= 4 * s->rx_packets;
	s->tx_bytes -= 4 * s->tx_packets;
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_DROP_EVENTS, 1,
				 &s->rx_dropped);

	rtl960x_mib_counter_read(priv, port, RTL960X_IF_OUT_DISCARDS, 1,
				 &s->tx_dropped);

	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_COLLISIONS, 1,
				 &s->collisions);

	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_FRAGMENTS, 1,
				 &s->rx_length_errors);
	rtl960x_mib_counter_read(priv, port, RTL960X_ETHER_STATS_JABBERS, 1,
				 &data);
	s->rx_length_errors += data;

	s->rx_errors = s->rx_length_errors;

	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_EXCESSIVE_COLLISIONS, 1,
				 &s->tx_aborted_errors);
	rtl960x_mib_counter_read(priv, port, RTL960X_DOT3_STATS_LATE_COLLISIONS, 1,
				 &s->tx_window_errors);

	s->tx_errors = s->tx_aborted_errors + s->tx_window_errors;
}

/*
 * A learn limit of 0 disables learning; the over-limit action is
 * forward-without-learning, so the port keeps forwarding.
 */
static int rtl960x_port_set_learning(struct rtl960x_dsa *priv, int port,
				     bool enable)
{
	int ret;

	ret = regmap_update_bits(priv->map,
				 RTL960X_LUT_LEARN_OVER_CTRL_REG,
				 RTL960X_LUT_LEARN_OVER_ACT_MASK(port),
				 RTL960X_LUT_LEARN_OVER_ACT_FORWARD);
	if (ret)
		return ret;

	return regmap_assign_bits(priv->map, RTL960X_LUT_LRN_LIMITNO_REG(port),
				  RTL960X_LUT_LRN_LIMITNO_MASK(port), enable);
}

static void rtl960x_port_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl960x_dsa *priv = ds->priv;
	int ret;

	ret = rtl960x_l2_flush(priv, port);
	if (ret)
		dev_err(priv->dev, "failed to fast age on port %d: %pe\n",
			port, ERR_PTR(ret));
}

static void rtl960x_port_stp_state_set(struct dsa_switch *ds, int port,
				       u8 state)
{
	struct rtl960x_dsa *priv = ds->priv;
	enum rtl960x_stp_state val;
	int msti;

	switch (state) {
	case BR_STATE_DISABLED:
		val = RTL960X_STP_STATE_DISABLED;
		break;
	case BR_STATE_BLOCKING:
	case BR_STATE_LISTENING:
		val = RTL960X_STP_STATE_BLOCKING;
		break;
	case BR_STATE_LEARNING:
		val = RTL960X_STP_STATE_LEARNING;
		break;
	case BR_STATE_FORWARDING:
		val = RTL960X_STP_STATE_FORWARDING;
		break;
	default:
		dev_err(priv->dev, "invalid STP state: %u\n", state);
		return;
	}

	/*
	 * A VLAN entry's FID field doubles as its MSTI: standalone ports and
	 * VLAN-aware bridges use FID 0, VLAN-unaware bridges FID 1. MST is not
	 * offloaded, so mirror the port's single STP state into every MSTI.
	 */
	for (msti = 0; msti < RTL960X_NUM_MSTI; msti++)
		regmap_update_bits(priv->map, RTL960X_MSTI_CTRL_REG(port),
				   RTL960X_MSTI_CTRL_PORT_STATE_MASK(msti),
				   field_prep(RTL960X_MSTI_CTRL_PORT_STATE_MASK(msti),
					      val));
}

static enum dsa_tag_protocol rtl960x_dsa_get_tag_protocol(struct dsa_switch *ds,
							  int port,
							  enum dsa_tag_protocol mp)
{
	return DSA_TAG_PROTO_RTL_OTTO;
}

static int rtl960x_dsa_setup(struct dsa_switch *ds)
{
	struct rtl960x_dsa *priv = ds->priv;
	u32 cpu_ports = 0, user_ports = 0;
	struct dsa_port *dp;
	int ret;

	/*
	 * Start with all ports completely isolated and not learning, the CPU
	 * and unused ports included, as rtl8365mb does. Learning is enabled
	 * per user port by the bridge's BR_LEARNING flag; a standalone port
	 * must not learn, or a destination it learned on another port would
	 * be blocked by isolation instead of reaching the CPU for the software
	 * bridge. Unknown destinations are flooded within the ingress port's
	 * isolation group, which is how CPU-bound traffic reaches the CPU.
	 */
	dsa_switch_for_each_port(dp, ds) {
		ret = rtl960x_port_set_isolation(priv, dp->index, 0);
		if (ret)
			return ret;

		ret = rtl960x_port_set_learning(priv, dp->index, false);
		if (ret)
			return ret;

		if (dsa_port_is_cpu(dp))
			cpu_ports |= BIT(dp->index);
	}

	/* User ports forward only to the CPU ports */
	dsa_switch_for_each_user_port(dp, ds) {
		ret = rtl960x_port_set_isolation(priv, dp->index, cpu_ports);
		if (ret)
			return ret;

		user_ports |= BIT(dp->index);
	}

	/* CPU ports forward to every user port */
	dsa_switch_for_each_cpu_port(dp, ds) {
		ret = rtl960x_cpu_port_setup(priv, dp->index);
		if (ret)
			return ret;

		ret = rtl960x_port_set_isolation(priv, dp->index, user_ports);
		if (ret)
			return ret;
	}

	/* One reserved VID per offloaded bridge; DSA numbers them for us. */
	ds->max_num_bridges = RTL960X_NUM_BRIDGE_VIDS;

	ds->assisted_learning_on_cpu_port = true;

	ret = rtl960x_port_mib_reset(priv, user_ports | cpu_ports);
	if (ret)
		return ret;

	/* Every user port starts standalone (see rtl960x_vlan_setup). */
	return rtl960x_vlan_setup(ds);
}

static int rtl960x_port_bridge_join(struct dsa_switch *ds, int port,
				    struct dsa_bridge bridge,
				    bool *tx_fwd_offload,
				    struct netlink_ext_ack *extack)
{
	struct rtl960x_dsa *priv = ds->priv;
	struct dsa_port *dp;
	u32 mask = 0;
	int ret;

	dev_dbg(priv->dev, "bridge %d join port %d\n", bridge.num, port);

	/*
	 * The port forwards as standalone until here. Block it before the
	 * isolation groups are opened, so nothing crosses into the bridge
	 * before its VLANs are replayed; DSA then syncs the bridge port's STP
	 * state, and the bridge enables the port. A join DSA rolls back later
	 * is put back into forwarding by rtl960x_port_bridge_leave(); a
	 * failure below restores it here, as DSA does not call leave then.
	 */
	rtl960x_port_stp_state_set(ds, port, BR_STATE_BLOCKING);

	/*
	 * Add this port to the isolation group of every other port
	 * offloading this bridge.
	 */
	dsa_switch_for_each_bridge_member(dp, ds, bridge.dev) {
		if (dp->index == port)
			continue;

		ret = rtl960x_port_add_isolation(priv, dp->index, BIT(port));
		if (ret)
			goto undo_isolation;

		mask |= BIT(dp->index);
	}

	/* Add those ports to the isolation group of this port */
	ret = rtl960x_port_add_isolation(priv, port, mask);
	if (ret)
		goto undo_isolation;

	ret = rtl960x_vlan_port_bridge_join(ds, port, bridge, extack);
	if (ret)
		goto undo_self_isolation;

	return 0;

undo_self_isolation:
	rtl960x_port_remove_isolation(priv, port, mask);

undo_isolation:
	dsa_switch_for_each_user_port(dp, ds) {
		if (mask & BIT(dp->index))
			rtl960x_port_remove_isolation(priv, dp->index,
						      BIT(port));
	}
	rtl960x_port_stp_state_set(ds, port, BR_STATE_FORWARDING);

	return ret;
}

static void rtl960x_port_bridge_leave(struct dsa_switch *ds, int port,
				      struct dsa_bridge bridge)
{
	struct rtl960x_dsa *priv = ds->priv;
	struct dsa_port *dp;
	u32 mask = 0;
	int ret;

	dev_dbg(priv->dev, "bridge %d leave port %d\n", bridge.num, port);

	/*
	 * Remove this port from the isolation group of every other port
	 * offloading this bridge.
	 */
	dsa_switch_for_each_bridge_member(dp, ds, bridge.dev) {
		if (dp->index == port)
			continue;

		ret = rtl960x_port_remove_isolation(priv, dp->index, BIT(port));
		if (ret)
			dev_err(priv->dev,
				"failed to remove port %d from isolation group of port %d: %pe\n",
				port, dp->index, ERR_PTR(ret));

		mask |= BIT(dp->index);
	}

	/* Remove those ports from the isolation group of this port */
	ret = rtl960x_port_remove_isolation(priv, port, mask);
	if (ret)
		dev_err(priv->dev,
			"failed to remove isolation group of port %d: %pe\n",
			port, ERR_PTR(ret));

	/*
	 * On a normal leave the bridge disables the port, which fast-ages it,
	 * and DSA then clears BR_LEARNING and puts the port back into
	 * forwarding. A join that DSA rolls back after inheriting the bridge
	 * port's flags and (disabled) STP state leaves without any of that,
	 * so restore the standalone state here: learning off, the port's
	 * dynamic entries flushed, the standalone VLAN state, and forwarding
	 * last, once the port is standalone again.
	 */
	ret = rtl960x_port_set_learning(priv, port, false);
	if (ret)
		dev_err(priv->dev, "failed to disable learning on port %d: %pe\n",
			port, ERR_PTR(ret));
	rtl960x_port_fast_age(ds, port);
	rtl960x_vlan_port_bridge_leave(ds, port, bridge);
	rtl960x_port_stp_state_set(ds, port, BR_STATE_FORWARDING);
}

static int rtl960x_port_pre_bridge_flags(struct dsa_switch *ds, int port,
					 struct switchdev_brport_flags flags,
					 struct netlink_ext_ack *extack)
{
	/* Flooding follows the hardware defaults (everything is flooded). */
	if (flags.mask & ~BR_LEARNING)
		return -EINVAL;

	return 0;
}

static int rtl960x_port_bridge_flags(struct dsa_switch *ds, int port,
				     struct switchdev_brport_flags flags,
				     struct netlink_ext_ack *extack)
{
	struct rtl960x_dsa *priv = ds->priv;

	if (flags.mask & BR_LEARNING)
		return rtl960x_port_set_learning(priv, port,
						 !!(flags.val & BR_LEARNING));

	return 0;
}

static void rtl960x_dsa_phylink_get_caps(struct dsa_switch *ds, int port,
					 struct phylink_config *config)
{
	switch (port) {
	/* Ports connected to the integrated GPHYs. There is no MII pinout. */
	case 0 ... 3:
		__set_bit(PHY_INTERFACE_MODE_INTERNAL,
			  config->supported_interfaces);
		/*
		 * GMII is the default interface mode for phylib, so we have
		 * to support it for ports with integrated PHY.
		 */
		__set_bit(PHY_INTERFACE_MODE_GMII,
			  config->supported_interfaces);

		config->mac_capabilities = MAC_SYM_PAUSE | MAC_ASYM_PAUSE |
					   MAC_10 | MAC_100 | MAC_1000FD;
		break;

	/*
	 * CPU ports connected to GMAC0 and GMAC1, forced to 1000/full without
	 * flow control in rtl960x_cpu_port_setup().
	 */
	case 9:
	case 10:
		__set_bit(PHY_INTERFACE_MODE_INTERNAL,
			  config->supported_interfaces);

		config->mac_capabilities = MAC_1000FD;
		break;

	case 6:
		__set_bit(PHY_INTERFACE_MODE_SGMII,
			  config->supported_interfaces);
// For now
//		__set_bit(PHY_INTERFACE_MODE_1000BASEX,
//			  config->supported_interfaces);
		// TODO HSGMII?
		// With this capability set, PHY don't communicate over SGMII!
		__set_bit(PHY_INTERFACE_MODE_2500BASEX,
			  config->supported_interfaces);

		/*
		 * Autoneg is out-of-band for both interfaces: the
		 * phylib-managed RTL8221B never requires in-band status, so
		 * under MLO_AN_PHY phylink resolves PHYLINK_PCS_NEG_OUTBAND
		 * and the serial stream carries no link ability to decode.
		 * No phylink_pcs is registered; the MAC link state comes
		 * solely from the forced ability written in mac_link_up.
		 */
		config->mac_capabilities = MAC_SYM_PAUSE | MAC_ASYM_PAUSE |
					   MAC_10 | MAC_100 | MAC_1000FD | MAC_2500FD;
		break;

	/*
	 * Port 5 (PON SerDes) and ports 6 and 7 (HSGMII SerDes) need SerDes
	 * configuration this driver does not implement; port 7 as the GMAC2
	 * CPU port is also not supported yet. Ports 4 and 8 are unused. Leave
	 * supported_interfaces empty so phylink rejects them rather than
	 * accepting a mode this driver never configures.
	 */
	default:
		break;
	}
}

static struct dsa_port *
rtl960x_dsa_preferred_default_local_cpu_port(struct dsa_switch *ds)
{
	struct dsa_port *cpu_dp = dsa_to_port(ds, RTL960X_DEFAULT_CPU_PORT);

	if (dsa_port_is_cpu(cpu_dp))
		return cpu_dp;

	return NULL;
}

/*
 * SGMII (1G) analog SerDes tuning table for the HSG0 SerDes (port 6), chip
 * rev C. This is the exact ANA + PAGE1 write sequence from the SDK's
 * _rtl9607c_lan_sds0_modeV3_set() SGMII case (SGMII_Patch_20211214).
 */
struct rtl960x_sds_ana {
	u16 cmd;
	u16 data;
};

static const struct rtl960x_sds_ana rtl960x_sgmii_ana[] = {
	{ 0xc001, 0xac68 },
	{ 0xc003, 0x276d },
	{ 0xc004, 0x5000 },
	{ 0xc005, 0x6313 },
	{ 0xc006, 0x001c },
	{ 0xc009, 0x521c },
	{ 0xc00a, 0xb628 },
	{ 0xc00d, 0xe714 },
	{ 0xc100, 0x50a2 },
	{ 0xc101, 0x886a },
	{ 0xc102, 0x0053 },
	{ 0xc103, 0x2b66 },
	{ 0xc104, 0x450c },
	{ 0xc109, 0xf0f2 },
	{ 0xc10b, 0x80a1 },
};

/*
 * 2500BASE-X (2.5G) analog SerDes tuning table for the HSG0 SerDes (port 6),
 * chip rev C. This is the exact ANA + PAGE1 write sequence from the SDK's
 * _rtl9607c_lan_sds0_modeV3_set() 2500BASEX case (2500Base-X_Patch_20211214).
 * Ten of the 15 values are the same as the SGMII table above; 0xc001, 0xc005,
 * 0xc100, 0xc101 and 0xc103 differ.
 */
static const struct rtl960x_sds_ana rtl960x_2500basex_ana[] = {
	{ 0xc001, 0xac4c },
	{ 0xc003, 0x276d },
	{ 0xc004, 0x5000 },
	{ 0xc005, 0x63db },
	{ 0xc006, 0x001c },
	{ 0xc009, 0x521c },
	{ 0xc00a, 0xb628 },
	{ 0xc00d, 0xe714 },
	{ 0xc100, 0x50a5 },
	{ 0xc101, 0x888a },
	{ 0xc102, 0x0053 },
	{ 0xc103, 0x8b66 },
	{ 0xc104, 0x450c },
	{ 0xc109, 0xf0f2 },
	{ 0xc10b, 0x80a1 },
};

/* Analog indirect-access window: write data -> DIG_02, cmd -> DIG_01,
 * then poll DIG_01[15] (CFG_SDS_MDX_REQ) until the SerDes is idle.
 */
static int rtl960x_sds_indirect_write(struct rtl960x_dsa *priv, u16 cmd, u16 data)
{
	u32 val;
	int ret;

	ret = regmap_write(priv->map, RTL960X_WSDS_HSG0_DIG_02, data);
	if (ret)
		return ret;

	ret = regmap_write(priv->map, RTL960X_WSDS_HSG0_DIG_01, cmd);
	if (ret)
		return ret;

	return regmap_read_poll_timeout(priv->map, RTL960X_WSDS_HSG0_DIG_01,
					val, !(val & RTL960X_CFG_SDS_MDX_REQ),
					10, 10000);
}

/*
 * Program the HSG0 SerDes (port 6) for the requested phylink interface, SGMII
 * or 2500BASE-X, following the SDK's rev-C (_modeV3_set) link-down /
 * clock-gate protection sequence. Both cases share the protection preamble
 * and the ANA doorbell mechanism and differ only in the CFG_SDS_MODE code,
 * the CPU_PLL_CTRL_DUMMY speed bits, the analog tuning table, and the
 * BASE-X-only fiber-PMA / 2500M line-rate tail; the SDK does not touch the
 * SDS_PHY_MODE field for BASE-X ("the MAC/PHY mode is not used"). For the
 * initial proof-of-concept this writes the "common IP enable" bits via a
 * direct ioremap and drives the whole sequence from mac_config.
 */
static int rtl960x_sds0_setup(struct rtl960x_dsa *priv, phy_interface_t iface)
{
	bool basex = iface == PHY_INTERFACE_MODE_2500BASEX;
	const struct rtl960x_sds_ana *ana;
	u32 abltry, force_mode, val;
	int i, n, ret;

	ana = basex ? rtl960x_2500basex_ana : rtl960x_sgmii_ana;
	n = basex ? ARRAY_SIZE(rtl960x_2500basex_ana) :
		    ARRAY_SIZE(rtl960x_sgmii_ana);

	/* Enable common IP (SDK ioal_socMem32 0xB8000600 + 0xB8000508) */
	val = readl(priv->ip_common);
	writel(val | RTL960X_IP_COMMON_BIT, priv->ip_common);

	val = readl(priv->ip_en);
	writel(val | RTL960X_IP_EN_BITS, priv->ip_en);

	/* Force port 6 MAC link down, saving the previous ability/state for
	 * the restore at the end (SDK macForceAbility_get/set + state set).
	 */
	ret = regmap_read(priv->map, RTL960X_FORCE_P_ABLTY_REG(6), &abltry);
	if (ret)
		return ret;
	ret = regmap_read(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(6),
			  &force_mode);
	if (ret)
		return ret;

	ret = regmap_clear_bits(priv->map, RTL960X_FORCE_P_ABLTY_REG(6),
				RTL960X_FORCE_P_ABLTY_LINK);
	if (ret)
		goto out;
	/* 0xbfff == ENABLED in dal_rtl9607c_port_macForceAbilityState_set */
	ret = regmap_write(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(6), 0xbfff);
	if (ret)
		goto out_restore;

	/* Stop the SerDes clock before reprogramming */
	ret = regmap_set_bits(priv->map,
			      RTL960X_UTP_FIBER_AUTODET_REG(RTL960X_SDS_HSG0),
			      RTL960X_SDS_CMD_STOP_GLI_CLK);
	if (ret)
		goto out_restore;
	/* SDK osal_time_mdelay(100); mac_config runs in process context,
	 * so sleep instead of spinning for 100 ms.
	 */
	usleep_range(100000, 120000);

	/* Release MDIO reset + enable the combo SDS */
	ret = regmap_set_bits(priv->map, RTL960X_WSDS_HSG0_ANA_00,
			      RTL960X_CFG_MDIO_RSTB);
	if (ret)
		goto out_restore;

	ret = regmap_set_bits(priv->map, RTL960X_SDS_CFG_REG(RTL960X_SDS_HSG0),
			      RTL960X_CFG_COMBO_SDS_EN);
	if (ret)
		goto out_restore;

	/* SGMII or 2500BASE-X mode */
	ret = regmap_update_bits(priv->map, RTL960X_SDS_CFG_REG(RTL960X_SDS_HSG0),
				 RTL960X_CFG_SDS_MODE_MASK,
				 basex ? RTL960X_SDS_MODE_2500BASEX
				       : RTL960X_SDS_MODE_SGMII);
	if (ret)
		goto out_restore;

	/* CPU_PLL_CTRL_DUMMY: APHY speed selection for SerDes port 6. SGMII
	 * runs the PLL/4 path with the USB3 select set (SDK SGMII case);
	 * 2500BASE-X needs PLL/2 (SPDSEL=1) with SGMII_SEL_USB3=0 (SDK 2500
	 * case).
	 */
	ret = regmap_update_bits(priv->map, RTL960X_CPU_PLL_CTRL_DUMMY,
				 RTL960X_SPDSEL_SGMII_USB3_MASK,
				 basex ? FIELD_PREP(RTL960X_SPDSEL_SGMII_USB3_MASK, 1)
				       : 0);
	if (ret)
		goto out_restore;
	ret = regmap_update_bits(priv->map, RTL960X_CPU_PLL_CTRL_DUMMY,
				 RTL960X_SGMII_SEL_USB3,
				 basex ? 0 : RTL960X_SGMII_SEL_USB3);
	if (ret)
		goto out_restore;

	/* Analog SerDes tuning */
	for (i = 0; i < n; i++) {
		ret = rtl960x_sds_indirect_write(priv, ana[i].cmd, ana[i].data);
		if (ret)
			goto out_restore;
	}

	/* Reset SDS, then re-assert the mode */
	ret = regmap_update_bits(priv->map, RTL960X_SDS_CFG_REG(RTL960X_SDS_HSG0),
				 RTL960X_CFG_SDS_MODE_MASK,
				 RTL960X_SDS_MODE_RESET);
	if (ret)
		goto out_restore;
	ret = regmap_update_bits(priv->map, RTL960X_SDS_CFG_REG(RTL960X_SDS_HSG0),
				 RTL960X_CFG_SDS_MODE_MASK,
				 basex ? RTL960X_SDS_MODE_2500BASEX
				       : RTL960X_SDS_MODE_SGMII);
	if (ret)
		goto out_restore;

	/* 2500BASE-X only, after the mode is re-asserted: power up the fiber
	 * PMA (FP_CFG_FIB_PDOWN=0) and disable the TX silent start
	 * (FEP_CFG_TX_MODE=0). The SGMII case writes neither register.
	 */
	if (basex) {
		ret = regmap_update_bits(priv->map, RTL960X_HSG0_FIB_REG0,
					 RTL960X_FP_CFG_FIB_PDOWN, 0);
		if (ret)
			goto out_restore;
		ret = regmap_update_bits(priv->map, RTL960X_HSG0_FIB_EXT_REG19,
					 RTL960X_FEP_CFG_TX_MODE, 0);
		if (ret)
			goto out_restore;
	}

	/* Restore the SerDes clock */
	ret = regmap_clear_bits(priv->map,
				RTL960X_UTP_FIBER_AUTODET_REG(RTL960X_SDS_HSG0),
				RTL960X_SDS_CMD_STOP_GLI_CLK);
	if (ret)
		goto out_restore;
	/* SDK osal_time_mdelay(100); sleep in process context instead of
	 * busy-spinning 100 ms with interrupts enabled.
	 */
	usleep_range(100000, 120000);

	/* Switch ready, PHY patch done */
	ret = regmap_set_bits(priv->map, RTL960X_WRAP_GPHY_MISC,
			      RTL960X_PATCH_PHY_DONE);
	if (ret)
		goto out_restore;

	if (basex) {
		/* 2.5G line rate: enable the override with the SDK rate */
		ret = regmap_update_bits(priv->map, RTL960X_LINE_RATE_2500M_0,
					 RTL960X_EN_RATE_2500M | RTL960X_RATE_MASK,
					 RTL960X_EN_RATE_2500M |
					 FIELD_PREP(RTL960X_RATE_MASK,
						    RTL960X_RATE_2500M_VAL));
	} else {
		/* SGMII is 1G, keep the 2500M line-rate override disabled */
		ret = regmap_clear_bits(priv->map, RTL960X_LINE_RATE_2500M_0,
					RTL960X_EN_RATE_2500M);
	}
	if (ret)
		goto out_restore;

	/* SGMII_MAC: SerDes plays the MAC role (0 == MAC, 1 == PHY). The
	 * field is not used in 2500BASE-X, so the SDK leaves it alone there
	 * and so do we.
	 */
	if (!basex) {
		ret = regmap_clear_bits(priv->map,
					RTL960X_UTP_FIBER_AUTODET_REG(RTL960X_SDS_HSG0),
					RTL960X_SDS_PHY_MODE);
		if (ret)
			goto out_restore;
	}

out_restore:
	/* MAC link restore: put back the pre-switch ability and force state */
	regmap_write(priv->map, RTL960X_FORCE_P_ABLTY_REG(6), abltry);
	regmap_write(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(6), force_mode);
out:
	return ret;
}

/*
 * Hazard (documented, mitigation deferred): the RTL8221B PHY driver mutates
 * phydev->interface in rtl822xb_update_interface() to mirror its SerDes side.
 * A later config_init() replay -- suspend/resume via phy_init_hw, or a PHY
 * re-attach -- recomputes the PHY's SERDES_OPTION from that mutated value:
 * after a 2500BASE-X period it locks the PHY into 2500BASEX-only, the dual
 * auto-adapt mode dies and the WAN stays down until the PHY is re-probed.
 * Untested on target; settle the mitigation after a pm_suspend cycle there.
 */
static void rtl960x_dsa_mac_config(struct phylink_config *config,
				   unsigned int mode,
				   const struct phylink_link_state *state)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl960x_dsa *priv = dp->ds->priv;
	int ret;

	if (dp->index != 6)
		return;

	if (state->interface != PHY_INTERFACE_MODE_SGMII &&
	    state->interface != PHY_INTERFACE_MODE_2500BASEX){
	    printk("%s:%d INVALID MAC MODE!\n", __func__ ,__LINE__);
		return;
		}


	if (
	    state->interface != PHY_INTERFACE_MODE_2500BASEX){
	    printk("%s:%d skip SGMII\n", __func__ ,__LINE__);
		return;
		}

	/* phylink calls mac_config only when the resolved interface changed
	 * (plain link flaps never reach it), but reprogram on any difference
	 * from the interface last programmed so a return to SGMII is not
	 * skipped; cache it only on success so a failure retries on the next
	 * event.
	 */
	if (priv->port6_ifmode == state->interface){
	    printk("%s:%d skip reconfiguring - mode is the same\n", __func__ ,__LINE__);
		return;
		}

	ret = rtl960x_sds0_setup(priv, state->interface);
	if (ret)
		dev_err(priv->dev,
			"failed to configure port 6 SerDes for %s: %pe\n",
			state->interface == PHY_INTERFACE_MODE_2500BASEX ?
				"2500BASE-X" : "SGMII", ERR_PTR(ret));
	else{
		priv->port6_ifmode = state->interface;
		dev_info(priv->dev,
			"configured port 6 SerDes for %s\n",
			state->interface == PHY_INTERFACE_MODE_2500BASEX ?
				"2500BASE-X" : "SGMII");
		}
}

static void rtl960x_dsa_mac_link_down(struct phylink_config *config,
				      unsigned int mode,
				      phy_interface_t interface)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl960x_dsa *priv = dp->ds->priv;
	printk("\n%s:%d Enter!\n", __func__, __LINE__);
	printk("Config: dev %s\n", dp->name);
	if (dp->index != 6)
		return;
	regmap_write(priv->map, RTL960X_FORCE_P_ABLTY_REG(6), 0);
	regmap_write(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(6), 0xbfff);
}

static void rtl960x_dsa_mac_link_up(struct phylink_config *config,
				    struct phy_device *phydev,
				    unsigned int mode,
				    phy_interface_t interface,
				    int speed, int duplex,
				    bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl960x_dsa *priv = dp->ds->priv;
	u32 sdk_speed, abltry;
	int ret;

	/*
	 * The internal GPHYs and the switch MAC sync speed/duplex in hardware,
	 * so there is nothing to program for them. Port 6 is the SGMII SerDes
	 * MAC, which does not follow the PHY link state on its own, so force its
	 * MAC ability to the phylink-resolved link parameters. This matches what
	 * the SDK does in dal_rtl9607c_port_macForceAbility_set + state set.
	 */
	if (dp->index != 6)
		return;

	/* mac_config() caches port6_ifmode only on success; against a failed
	 * or mode-mismatched SerDes program do not advertise link, because
	 * phylink cannot see a void mac_config failure and would leave the
	 * carrier up with no data path.
	 */
	if (priv->port6_ifmode != interface){
	printk("%s:%d weird mode mismatch skip!\n", __func__, __LINE__);
		return;
		}

	switch (speed) {
	case SPEED_10:
		sdk_speed = 0;		/* PORT_SPEED_10M */
		break;
	case SPEED_100:
		sdk_speed = 1;		/* PORT_SPEED_100M */
		break;
	case SPEED_2500:
		sdk_speed = 5;		/* PORT_SPEED_2G5 */
		break;
	case SPEED_1000:
	default:
		sdk_speed = 2;		/* PORT_SPEED_1000M */
		break;
	}

	/* SPEED01_ABLTY <-> speed[1:0], SPEED23_ABLTY <-> speed[3:2] */
	abltry = FIELD_PREP(RTL960X_FORCE_P_ABLTY_SPEED_MASK, sdk_speed & 0x3) |
		 FIELD_PREP(RTL960X_FORCE_P_ABLTY_SPEED23_MASK, sdk_speed >> 2) |
		 RTL960X_FORCE_P_ABLTY_LINK;
	/* The SDK hardcodes full-duplex here (2500BASE-X and SGMII run the
	 * serial side full-duplex); the pause cells follow the phylink
	 * resolved state.
	 */
	abltry |= RTL960X_FORCE_P_ABLTY_DUPLEX;
	if (tx_pause)
		abltry |= RTL960X_FORCE_P_ABLTY_TX_PAUSE;
	if (rx_pause)
		abltry |= RTL960X_FORCE_P_ABLTY_RX_PAUSE;

	ret = regmap_write(priv->map, RTL960X_FORCE_P_ABLTY_REG(6), abltry);
	if (ret)
		dev_err(priv->dev,
			"failed to force port 6 MAC ability: %pe\n",
			ERR_PTR(ret));

	ret = regmap_write(priv->map, RTL960X_ABLTY_FORCE_MODE_REG(6),
			   RTL960X_ABLTY_FORCE_MODE_ENABLE);
	if (ret)
		dev_err(priv->dev,
			"failed to enable port 6 force mode: %pe\n",
			ERR_PTR(ret));
}

static const struct phylink_mac_ops rtl960x_dsa_phylink_mac_ops = {
	.mac_config	= rtl960x_dsa_mac_config,
	.mac_link_down	= rtl960x_dsa_mac_link_down,
	.mac_link_up	= rtl960x_dsa_mac_link_up,
};

static const struct dsa_switch_ops rtl960x_dsa_ops = {
	.get_tag_protocol	= rtl960x_dsa_get_tag_protocol,
	.setup			= rtl960x_dsa_setup,
	.preferred_default_local_cpu_port = rtl960x_dsa_preferred_default_local_cpu_port,
	.phylink_get_caps	= rtl960x_dsa_phylink_get_caps,
	.port_bridge_join	= rtl960x_port_bridge_join,
	.port_bridge_leave	= rtl960x_port_bridge_leave,
	.port_pre_bridge_flags	= rtl960x_port_pre_bridge_flags,
	.port_bridge_flags	= rtl960x_port_bridge_flags,
	.port_stp_state_set	= rtl960x_port_stp_state_set,
	.port_fast_age		= rtl960x_port_fast_age,
	.port_vlan_filtering	= rtl960x_port_vlan_filtering,
	.port_vlan_add		= rtl960x_port_vlan_add,
	.port_vlan_del		= rtl960x_port_vlan_del,
	.port_fdb_add		= rtl960x_port_fdb_add,
	.port_fdb_del		= rtl960x_port_fdb_del,
	.port_fdb_dump		= rtl960x_port_fdb_dump,
	.get_strings		= rtl960x_get_strings,
	.get_ethtool_stats	= rtl960x_get_ethtool_stats,
	.get_sset_count		= rtl960x_get_sset_count,
	.get_eth_phy_stats	= rtl960x_get_phy_stats,
	.get_eth_mac_stats	= rtl960x_get_mac_stats,
	.get_eth_ctrl_stats	= rtl960x_get_ctrl_stats,
	.get_rmon_stats		= rtl960x_get_rmon_stats,
	.get_stats64		= rtl960x_get_stats64,
	.get_pause_stats	= rtl960x_get_pause_stats,
};

static int rtl960x_dsa_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rtl960x_dsa *priv;
	struct dsa_switch *ds;
	int ret;

	ret = otto_table_loaded();
	if (ret)
		return dev_err_probe(dev, ret, "table access not available\n");

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	ds = devm_kzalloc(dev, sizeof(*ds), GFP_KERNEL);
	if (!ds)
		return -ENOMEM;

	priv->dev = dev;
	priv->ds = ds;
	/* The port 6 SerDes has not been programmed yet */
	priv->port6_ifmode = PHY_INTERFACE_MODE_NA;

	ret = devm_mutex_init(dev, &priv->l2_lock);
	if (ret)
		return ret;

	priv->map = syscon_node_to_regmap(dev->of_node);
	if (IS_ERR(priv->map))
		return dev_err_probe(dev, PTR_ERR(priv->map),
				     "failed to get switch-core regmap\n");

	/*
	 * PoC: map the "common IP enable" registers directly. These live in
	 * the soc peripheral space (see RTL960X_IP_COMMON/IP_EN_ADDR), not
	 * in the switch0 regmap.
	 */
	priv->ip_common = devm_ioremap(dev, RTL960X_IP_COMMON_ADDR, 0x4);
	if (!priv->ip_common)
		return dev_err_probe(dev, -ENOMEM,
				     "failed to map IP common registers\n");

	priv->ip_en = devm_ioremap(dev, RTL960X_IP_EN_ADDR, 0x4);
	if (!priv->ip_en)
		return dev_err_probe(dev, -ENOMEM,
				     "failed to map IP enable registers\n");

	ds->dev = dev;
	ds->num_ports = RTL960X_NUM_PORTS;
	ds->ops = &rtl960x_dsa_ops;
	ds->phylink_mac_ops = &rtl960x_dsa_phylink_mac_ops;
	ds->priv = priv;

	platform_set_drvdata(pdev, priv);

	return dsa_register_switch(ds);
}

static void rtl960x_dsa_remove(struct platform_device *pdev)
{
	struct rtl960x_dsa *priv = platform_get_drvdata(pdev);

	if (priv)
		dsa_unregister_switch(priv->ds);
}

static void rtl960x_dsa_shutdown(struct platform_device *pdev)
{
	struct rtl960x_dsa *priv = platform_get_drvdata(pdev);

	if (priv)
		dsa_switch_shutdown(priv->ds);

	platform_set_drvdata(pdev, NULL);
}

static const struct of_device_id rtl960x_dsa_of_match[] = {
	{ .compatible = "realtek,rtl9607c-switch" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl960x_dsa_of_match);

static struct platform_driver rtl960x_dsa_driver = {
	.probe = rtl960x_dsa_probe,
	.remove = rtl960x_dsa_remove,
	.shutdown = rtl960x_dsa_shutdown,
	.driver = {
		.name = "rtl960x-switch",
		.of_match_table = rtl960x_dsa_of_match,
	},
};
module_platform_driver(rtl960x_dsa_driver);

MODULE_DESCRIPTION("Realtek RTL9607C family DSA switch driver");
MODULE_AUTHOR("Taiga Ogawa");
MODULE_LICENSE("GPL");
