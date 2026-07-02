/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __RTL960X_GMAC_H
#define __RTL960X_GMAC_H

#include <linux/bits.h>
#include <linux/types.h>

/* GMAC registers (offsets from the per-GMAC base) */
#define GMAC_IDR0		0x00
#define GMAC_IDR4		0x04

#define GMAC_CMD		0x3b
#define CMD_RXCHKSUM		BIT(1)
#define CMD_RXJUMBO		BIT(3)

#define GMAC_IMR		0x3c
#define GMAC_ISR		0x3e
#define ISR_RX_OK		BIT(0)
#define ISR_RER_RUNT		BIT(2)
#define ISR_RER_OVF		BIT(4)
#define ISR_RDU			BIT(5)
#define ISR_TOK			BIT(6)
#define ISR_SW_INT		BIT(10)

#define GMAC_TCR		0x40
#define TCR_IFG			GENMASK(12, 10)
#define TCR_IFG_NORMAL		3

#define GMAC_RCR		0x44
#define RCR_ACCEPT_BROADCAST	BIT(3)
#define RCR_ACCEPT_MULTICAST	BIT(2)
#define RCR_ACCEPT_MYPHYS	BIT(1)
#define RCR_ACCEPT_ALLPHYS	BIT(0)

#define GMAC_CPUTAGCR		0x48
#define CPUTAGCR_EN_RX		BIT(31)
#define CPUTAGCR_TSIZE		GENMASK(30, 27)
#define CPUTAGCR_SWITCH		GENMASK(21, 18)
#define CPUTAGCR_RSIZE_L	GENMASK(17, 16)
#define CPUTAGCR_PROTOCOL_MASK	GENMASK(15, 8)
#define CPUTAGCR_PROTOCOL_VAL	GENMASK(7, 0)
#define CPUTAGCR_SIZE_8B	2
#define CPUTAGCR_FMT_APOLLOPRO	8
#define CPUTAGCR_PROTO_MATCH_ALL	0xff
#define CPUTAGCR_PROTO_8370	0x04

#define GMAC_CONFIG		0x4c

#define GMAC_CPUTAG1CR		0x50
#define CPUTAG1CR_SID		BIT(14)

#define GMAC_MSR		0x58
#define MSR_FORCE_TX		BIT(7)
#define MSR_RXFCE		BIT(6)
#define MSR_TXFCE		BIT(5)

#define GMAC_IMR0		0xd0
#define GMAC_ISR1		0xd8

#define GMAC_TXFDP1		0x1300
#define GMAC_TXCDO1		0x1304

#define GMAC_RXFDP		0x13f0
#define GMAC_RXCDO		0x13f4
#define GMAC_RXRINGSIZE		0x13f6
#define GMAC_RXCPU_DES_NUM	0x1430
#define GMAC_RX_PSE_DES_THRES	0x1432

#define GMAC_IO_CMD		0x1434
#define IO_CMD_TX_POLL		BIT(0)
#define IO_CMD_TX_ENABLE	BIT(4)
#define IO_CMD_RX_ENABLE	BIT(5)
#define IO_CMD_RX_INT_TRIG_L	GENMASK(10, 8)
#define IO_CMD_RX_FIFO_THRESH	GENMASK(12, 11)
#define IO_CMD_RX_PKT_TMR_L	GENMASK(15, 13)
#define IO_CMD_TX_INT_TRIG_L	GENMASK(18, 16)
#define IO_CMD_TX_FIFO_THRESH	GENMASK(20, 19)
#define IO_CMD_TX_PKT_TMR	GENMASK(27, 24)
#define IO_CMD_SHORT_DES_FMT	BIT(30)

#define GMAC_IO_CMD1		0x1438
#define IO_CMD1_RX_RING1	BIT(16)
#define IO_CMD1_DESC_FMT_EXTRA	GENMASK(30, 28)
#define IO_CMD1_DESC_FMT_APOLLO	3

/* Descriptor fields */
#define DESC_OWN		BIT(31)
#define DESC_EOR		BIT(30)
#define DESC_FS			BIT(29)
#define DESC_LS			BIT(28)
#define DESC_LEN_MASK		0xfff
#define TX_DESC_CRC		BIT(23)

/* CPU-tag sideband fields in descriptor opts2/opts3 */
#define RX_OPTS3_SRC_PORT(o3)	(((o3) >> 16) & 0xf)
#define RX_OPTS2_REASON(o2)	(((o2) >> 21) & 0xff)
#define RX_REASON_FLOOD		0xca
#define TX_OPTS2_CPUTAG		BIT(31)
#define TX_OPTS2_PORTMASK(pm)	(((pm) & 0x7ff) << 16)
#define TX_OPTS3_KEEP		BIT(23)
#define TX_OPTS3_DISLRN		BIT(21)
#define TX_OPTS3_L34_KEEP	BIT(17)

#define RTL960X_CPU_PORT	9

/* RX descriptors are four words; TX descriptors include an opts4 word. */
struct rtl960x_rx_desc {
	u32 opts1;	/* own/eor/fs/ls + length */
	u32 addr;	/* DMA buffer address */
	u32 opts2;	/* CPU tag: trap reason */
	u32 opts3;	/* CPU tag: source port */
};

struct rtl960x_tx_desc {
	u32 opts1;	/* own/eor/fs/ls + length */
	u32 addr;	/* DMA buffer address */
	u32 opts2;	/* CPU tag: enable + destination port mask */
	u32 opts3;	/* CPU tag: flags */
	u32 opts4;
};

#endif /* __RTL960X_GMAC_H */
