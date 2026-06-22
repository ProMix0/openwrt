// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL960x/RTL8198D "LUNA" GMAC datapath driver.
 *
 * The register layout, descriptor format and bring-up sequence were
 * reconstructed from the Realtek re8686/rtl86900 NIC driver (Linux 5.10) and
 * the matching U-Boot poll-mode driver (re8670poll.c).
 *
 * This implements a minimal single-RX-ring + single-TX-ring NAPI datapath for
 * GMAC0 (the CPU port), validated bidirectionally on a Buffalo WSR-1500AX2S.
 *
 * Two things are required and were the main findings:
 *  1. The GMAC IP clock is gated; it must be ungated via the reset controller
 *     before any register access (gmac1/2 reboot the SoC otherwise).
 *  2. Registers are native big-endian; use the __raw_* accessors (no swap) to
 *     match the vendor's volatile-pointer access.
 *
 * The switch core (PHY/port bring-up, isolation, VLANs, FDB) is owned by the
 * rtl960x_dsa switch driver; this driver is purely the CPU-port conduit.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/timer.h>
#include <net/dsa.h>

#include "rtl960x_gmac.h"

/* Driver configuration */
#define ISR_RX_ALL		(ISR_SW_INT | ISR_RX_OK | ISR_RER_RUNT | \
				 ISR_RER_OVF | ISR_RDU)
/* Interrupts that drive NAPI: RX events plus TX completion (TOK). */
#define ISR_NAPI		(ISR_RX_ALL | ISR_TOK)

#define TCR_CONFIG		FIELD_PREP(TCR_IFG, TCR_IFG_NORMAL)

/*
 * This GMAC is a DSA conduit (CPU port): the switch already decides which frames
 * to deliver here (forward/flood/trap), and they carry many destination MACs --
 * the per-port netdev addresses plus anything the switch floods to the CPU on a
 * lookup miss. Accept all unicast (ALLPHYS); MYPHYS alone would silently drop
 * frames the switch delivers that are not addressed to the conduit itself.
 */
#define RCR_DEFAULT		(RCR_ACCEPT_BROADCAST | RCR_ACCEPT_MULTICAST | \
				 RCR_ACCEPT_MYPHYS | RCR_ACCEPT_ALLPHYS)

#define CPUTAGCR_CONFIG		(CPUTAGCR_EN_RX |					\
				 FIELD_PREP(CPUTAGCR_TSIZE, CPUTAGCR_SIZE_8B) |		\
				 FIELD_PREP(CPUTAGCR_SWITCH, CPUTAGCR_FMT_APOLLOPRO) |	\
				 FIELD_PREP(CPUTAGCR_RSIZE_L, CPUTAGCR_SIZE_8B) |	\
				 FIELD_PREP(CPUTAGCR_PROTOCOL_MASK, CPUTAGCR_PROTO_MATCH_ALL) | \
				 FIELD_PREP(CPUTAGCR_PROTOCOL_VAL, CPUTAGCR_PROTO_8370))
#define CPUTAG1CR_CONFIG	CPUTAG1CR_SID

/* RX flow-control assert threshold (free RX descriptors) */
#define RX_PSE_DES_THRES_VAL	8

/*
 * IO_CMD tuning values, validated on HW (they differ from the vendor stock
 * defaults). TX_INT_TRIG_L=7 gates TOK at 28 completions only when
 * TX_PKT_TMR=0, and the count persists across idle time. TX_PKT_TMR is a
 * post-completion delay (~9 us base + ~1.1 us/count; 0xf adds ~16.5 us),
 * too short to coalesce CPU-paced TX, so keep it 0 and use the software
 * backstop for short tails. The RX and TX FIFO fields keep the vendor "apro"
 * encodings.
 */
#define IO_CMD_RX_INT_12PKTS	3
#define IO_CMD_RX_FIFO_64B	2
#define IO_CMD_RX_PKT_TMR_APRO	1
#define IO_CMD_TX_INT_28PKTS	7
#define IO_CMD_TX_PKT_TMR_MAX	0xf	/* The more - the biggest timeout */
#define IO_CMD_TX_FIFO_THRESH_APRO	1
#define IO_CMD_CONFIG		(IO_CMD_RX_ENABLE | IO_CMD_TX_ENABLE |			\
				 FIELD_PREP(IO_CMD_RX_INT_TRIG_L, IO_CMD_RX_INT_12PKTS) |	\
				 FIELD_PREP(IO_CMD_RX_FIFO_THRESH, IO_CMD_RX_FIFO_64B) |	\
				 FIELD_PREP(IO_CMD_RX_PKT_TMR_L, IO_CMD_RX_PKT_TMR_APRO) |	\
				 FIELD_PREP(IO_CMD_TX_INT_TRIG_L, IO_CMD_TX_INT_28PKTS) |	\
				 FIELD_PREP(IO_CMD_TX_PKT_TMR, IO_CMD_TX_PKT_TMR_MAX) |	\
				 FIELD_PREP(IO_CMD_TX_FIFO_THRESH, IO_CMD_TX_FIFO_THRESH_APRO) | \
				 IO_CMD_SHORT_DES_FMT)

#define IO_CMD1_CONFIG		(FIELD_PREP(IO_CMD1_DESC_FMT_EXTRA, IO_CMD1_DESC_FMT_APOLLO) | \
				 IO_CMD1_RX_RING1)

/*
 * DSA glue. The switch tags the ingress port in the descriptor, not inline, so
 * the conduit translates between the descriptor and the rtl_otto trailer tag
 * (DSA_TAG_PROTO_RTL_OTTO) the same way the rtl83xx driver does: on RX the FCS
 * is overwritten with a 4-byte trailer [0x80, port, 0x10, 0x00]; on TX a trailer
 * with that shape selects the egress port.
 *
 * trailer[1] bit6 (0x40) maps to skb->offload_fwd_mark in the rtl_otto tagger:
 * set it when the switch already L2-forwarded the frame to a non-CPU port, so
 * the Linux bridge does not software-forward (and thus duplicate) it.
 */
#define DSA_TRAILER_LEN		4
#define DSA_TAG_OFFLOAD_FWD	0x40

/* Datapath sizing */
#define RX_RING_SIZE		128	/* must be a power of two, 16..256 */
#define TX_RING_SIZE		128
#define RX_BUF_SIZE		1536	/* 0x600, per U-Boot RX_DESC_BUFFER_SIZE */
#define RX_SHIFT		2	/* HW writes the frame at buf + 2 bytes */
#define RX_MAX_FRAME_LEN	(RX_BUF_SIZE - RX_SHIFT)
#define TX_MIN_LEN		60
/* SW backstop for TX completions the coalesced TOK interrupt never raises. */
#define TX_TIMER_DELAY		msecs_to_jiffies(10)

#define RTL960X_GMAC_REGS_DUMP_LEN	0x100

/* The ring strides are the HW descriptor sizes; guard the layout. */
static_assert(sizeof(struct rtl960x_rx_desc) == 16);
static_assert(sizeof(struct rtl960x_tx_desc) == 20);

struct rtl960x_gmac {
	struct net_device *ndev;
	struct device *dev;
	void __iomem *base;
	int irq;
	struct napi_struct rx_napi;
	struct napi_struct tx_napi;
	struct reset_control *rst;

	/* RX ring */
	struct rtl960x_rx_desc *rx_ring;	/* RX_RING_SIZE entries */
	dma_addr_t rx_ring_dma;
	struct sk_buff *rx_skb[RX_RING_SIZE];
	dma_addr_t rx_buf_dma[RX_RING_SIZE];
	u32 rx_head;

	/* TX ring */
	struct rtl960x_tx_desc *tx_ring;	/* TX_RING_SIZE entries */
	dma_addr_t tx_ring_dma;
	struct sk_buff *tx_skb[TX_RING_SIZE];
	u32 tx_head;			/* next slot to fill */
	u32 tx_tail;			/* next slot to reclaim */
	spinlock_t tx_lock;		/* protects tx_head/tx_tail and the ring */
};

/*
 * The GMAC registers are native (big-endian on this SoC); the vendor code
 * accesses them with plain volatile pointers, so use the raw, non-swapping
 * accessors to match.
 */
static inline u32 gmac_r32(struct rtl960x_gmac *g, u32 reg)
{
	return __raw_readl(g->base + reg);
}

static inline void gmac_w32(struct rtl960x_gmac *g, u32 reg, u32 val)
{
	__raw_writel(val, g->base + reg);
}

static inline u16 gmac_r16(struct rtl960x_gmac *g, u32 reg)
{
	return __raw_readw(g->base + reg);
}

static inline void gmac_w16(struct rtl960x_gmac *g, u32 reg, u16 val)
{
	__raw_writew(val, g->base + reg);
}

static inline void gmac_w8(struct rtl960x_gmac *g, u32 reg, u8 val)
{
	__raw_writeb(val, g->base + reg);
}

static inline u8 gmac_r8(struct rtl960x_gmac *g, u32 reg)
{
	return __raw_readb(g->base + reg);
}

static void rtl960x_gmac_stop_hw(struct rtl960x_gmac *g)
{
	gmac_w32(g, GMAC_IO_CMD, 0);
	gmac_w32(g, GMAC_IO_CMD1, 0);
	gmac_w16(g, GMAC_IMR, 0);
	gmac_w32(g, GMAC_IMR0, 0);
	gmac_w16(g, GMAC_ISR, 0xffff);		/* W1C */
	gmac_w32(g, GMAC_ISR1, 0xffffffff);
	usleep_range(10, 20);
}

static void rtl960x_gmac_set_hwaddr(struct rtl960x_gmac *g)
{
	const u8 *a = g->ndev->dev_addr;

	gmac_w32(g, GMAC_IDR0,
		 (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]);
	gmac_w32(g, GMAC_IDR4, (a[4] << 24) | (a[5] << 16));
}

static void rtl960x_gmac_free_rx(struct rtl960x_gmac *g)
{
	int i;

	for (i = 0; i < RX_RING_SIZE; i++) {
		if (!g->rx_skb[i])
			continue;
		dma_unmap_single(g->dev, g->rx_buf_dma[i], RX_BUF_SIZE,
				 DMA_FROM_DEVICE);
		dev_kfree_skb(g->rx_skb[i]);
		g->rx_skb[i] = NULL;
	}
}

static int rtl960x_gmac_alloc_rings(struct rtl960x_gmac *g)
{
	int i;

	g->rx_ring = dma_alloc_coherent(g->dev,
					RX_RING_SIZE * sizeof(*g->rx_ring),
					&g->rx_ring_dma, GFP_KERNEL);
	if (!g->rx_ring)
		return -ENOMEM;

	g->tx_ring = dma_alloc_coherent(g->dev,
					TX_RING_SIZE * sizeof(*g->tx_ring),
					&g->tx_ring_dma, GFP_KERNEL);
	if (!g->tx_ring)
		goto err_free_rx_ring;

	/* Populate RX ring with buffers; HW owns each filled descriptor. */
	for (i = 0; i < RX_RING_SIZE; i++) {
		struct rtl960x_rx_desc *d = &g->rx_ring[i];
		struct sk_buff *skb;
		dma_addr_t dma;

		skb = netdev_alloc_skb(g->ndev, RX_BUF_SIZE);
		if (!skb)
			goto err_free_rx_bufs;

		dma = dma_map_single(g->dev, skb->data, RX_BUF_SIZE,
				     DMA_FROM_DEVICE);
		if (dma_mapping_error(g->dev, dma)) {
			dev_kfree_skb(skb);
			goto err_free_rx_bufs;
		}

		g->rx_skb[i] = skb;
		g->rx_buf_dma[i] = dma;
		d->addr = dma;
		d->opts2 = 0;
		d->opts3 = 0;
		d->opts1 = DESC_OWN | RX_BUF_SIZE |
			   (i == RX_RING_SIZE - 1 ? DESC_EOR : 0);
	}

	/* TX ring starts idle (CPU-owned); only EOR is preset on the last. */
	for (i = 0; i < TX_RING_SIZE; i++) {
		struct rtl960x_tx_desc *d = &g->tx_ring[i];

		d->addr = 0;
		d->opts2 = 0;
		d->opts3 = 0;
		d->opts4 = 0;
		d->opts1 = (i == TX_RING_SIZE - 1) ? DESC_EOR : 0;
	}

	g->rx_head = 0;
	g->tx_head = 0;
	g->tx_tail = 0;
	dma_wmb();

	return 0;

err_free_rx_bufs:
	rtl960x_gmac_free_rx(g);
	dma_free_coherent(g->dev, TX_RING_SIZE * sizeof(*g->tx_ring),
			  g->tx_ring, g->tx_ring_dma);
	g->tx_ring = NULL;
err_free_rx_ring:
	dma_free_coherent(g->dev, RX_RING_SIZE * sizeof(*g->rx_ring),
			  g->rx_ring, g->rx_ring_dma);
	g->rx_ring = NULL;
	return -ENOMEM;
}

static void rtl960x_gmac_free_rings(struct rtl960x_gmac *g)
{
	rtl960x_gmac_free_rx(g);

	if (g->tx_ring) {
		dma_free_coherent(g->dev, TX_RING_SIZE * sizeof(*g->tx_ring),
				  g->tx_ring, g->tx_ring_dma);
		g->tx_ring = NULL;
	}
	if (g->rx_ring) {
		dma_free_coherent(g->dev, RX_RING_SIZE * sizeof(*g->rx_ring),
				  g->rx_ring, g->rx_ring_dma);
		g->rx_ring = NULL;
	}
}

static void rtl960x_gmac_init_hw(struct rtl960x_gmac *g)
{
	rtl960x_gmac_stop_hw(g);

	/*
	 * Keep RXJUMBO set like the vendor driver always does; the accept
	 * cutoff with the bit cleared is unvalidated on this hardware, and
	 * oversize frames are dropped by the FS|LS + length check in the
	 * RX path anyway.
	 */
	gmac_w8(g, GMAC_CMD, CMD_RXCHKSUM | CMD_RXJUMBO);
	gmac_w32(g, GMAC_TCR, TCR_CONFIG);

	/* CPU tag: parsed into the descriptor opts fields, not inline. */
	gmac_w32(g, GMAC_CPUTAGCR, CPUTAGCR_CONFIG);
	gmac_w32(g, GMAC_CPUTAG1CR, CPUTAG1CR_CONFIG);

	/* Program the RX/TX ring bases. */
	gmac_w32(g, GMAC_RXFDP, g->rx_ring_dma);
	gmac_w16(g, GMAC_RXCDO, 0);
	gmac_w8(g, GMAC_RXRINGSIZE, RX_RING_SIZE - 1);
	gmac_w8(g, GMAC_RXCPU_DES_NUM, RX_RING_SIZE - 1);
	gmac_w8(g, GMAC_RX_PSE_DES_THRES, RX_PSE_DES_THRES_VAL);

	gmac_w32(g, GMAC_TXFDP1, g->tx_ring_dma);
	gmac_w16(g, GMAC_TXCDO1, 0);

	rtl960x_gmac_set_hwaddr(g);
	gmac_w32(g, GMAC_RCR, RCR_DEFAULT);

	/* Enable the datapath. */
	gmac_w32(g, GMAC_IO_CMD1, IO_CMD1_CONFIG);
	gmac_w32(g, GMAC_IO_CMD, IO_CMD_CONFIG);

	/* Clear and unmask RX + TX-completion interrupts. */
	gmac_w16(g, GMAC_ISR, 0xffff);
	gmac_w32(g, GMAC_ISR1, 0xffffffff);
	gmac_w16(g, GMAC_IMR, ISR_NAPI);
}

static void rtl960x_gmac_mask_irqs(struct rtl960x_gmac *g, u16 irq)
{
	gmac_w16(g, GMAC_IMR, gmac_r16(g, GMAC_IMR) & ~irq);
}

static void rtl960x_gmac_unmask_irqs(struct rtl960x_gmac *g, u16 irq)
{
	gmac_w16(g, GMAC_IMR, gmac_r16(g, GMAC_IMR) | irq);
}

static int rtl960x_gmac_rx(struct rtl960x_gmac *g, int budget)
{
	struct net_device *ndev = g->ndev;
	bool dsa = netdev_uses_dsa(ndev);
	int done = 0;

	while (done < budget) {
		struct rtl960x_rx_desc *d = &g->rx_ring[g->rx_head];
		bool last = (g->rx_head == RX_RING_SIZE - 1);
		struct sk_buff *skb, *new_skb;
		dma_addr_t new_dma;
		u8 src_port;
		u32 opts1, opts2, opts3;
		int len;

		opts1 = d->opts1;
		if (opts1 & DESC_OWN)		/* still owned by HW */
			break;
		dma_rmb();

		opts2 = d->opts2;
		opts3 = d->opts3;
		src_port = RX_OPTS3_SRC_PORT(opts3);

		/*
		 * This single-buffer datapath only handles whole frames: each
		 * descriptor must be both the first and last segment and carry a
		 * sane length. The bound applies to the raw descriptor length --
		 * that is what the hardware wrote at buf + RX_SHIFT -- before any
		 * FCS adjustment. Reject anything else and rearm the same buffer.
		 * Note no allocation happens on this error path -- a fresh skb is
		 * taken only once the frame is known good, so runts, oversize and
		 * multi-segment descriptors reuse the existing buffer as-is.
		 */
		len = opts1 & DESC_LEN_MASK;
		if ((opts1 & (DESC_FS | DESC_LS)) != (DESC_FS | DESC_LS) ||
		    len < ETH_ZLEN + ETH_FCS_LEN || len > RX_MAX_FRAME_LEN) {
			ndev->stats.rx_errors++;
			goto rearm_same;
		}

		/*
		 * Without DSA, strip the FCS. With DSA, keep the 4 FCS bytes and
		 * overwrite them with the rtl_otto trailer carrying src_port.
		 */
		if (!dsa)
			len -= ETH_FCS_LEN;

		new_skb = netdev_alloc_skb(ndev, RX_BUF_SIZE);
		if (!new_skb) {
			ndev->stats.rx_dropped++;
			goto rearm_same;
		}
		new_dma = dma_map_single(g->dev, new_skb->data, RX_BUF_SIZE,
					 DMA_FROM_DEVICE);
		if (dma_mapping_error(g->dev, new_dma)) {
			dev_kfree_skb(new_skb);
			ndev->stats.rx_dropped++;
			goto rearm_same;
		}

		/* Good frame: swap the filled buffer out for the fresh one. */
		skb = g->rx_skb[g->rx_head];
		dma_unmap_single(g->dev, g->rx_buf_dma[g->rx_head], RX_BUF_SIZE,
				 DMA_FROM_DEVICE);

		skb_reserve(skb, RX_SHIFT);	/* HW wrote the frame at buf + 2 */
		skb_put(skb, len);
		if (dsa) {
			/* overwrite the FCS with the rtl_otto trailer */
			u8 *t = skb->data + len - DSA_TRAILER_LEN;
			u8 tag = src_port;

			/*
			 * If the switch HW-flooded this frame to other ports
			 * (CPU_REASON_FLOOD), mark it offloaded so the Linux
			 * bridge does not forward it again (avoids duplicate
			 * broadcast/multicast). Trapped frames (BCAST_TRAP,
			 * IGMP, ...) keep a different reason and are left for
			 * the bridge/stack.
			 */
			bool fwd = RX_OPTS2_REASON(opts2) == RX_REASON_FLOOD;

			if (fwd)
				tag |= DSA_TAG_OFFLOAD_FWD;

			t[0] = 0x80;
			t[1] = tag;
			t[2] = 0x10;
			t[3] = 0x00;
		}
		skb->protocol = eth_type_trans(skb, ndev);
		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += len;
		napi_gro_receive(&g->rx_napi, skb);

		g->rx_skb[g->rx_head] = new_skb;
		g->rx_buf_dma[g->rx_head] = new_dma;
		d->addr = new_dma;

rearm_same:
		d->opts2 = 0;
		d->opts3 = 0;
		dma_wmb();
		d->opts1 = DESC_OWN | RX_BUF_SIZE | (last ? DESC_EOR : 0);

		g->rx_head = (g->rx_head + 1) % RX_RING_SIZE;
		done++;
	}

	return done;
}

static int rtl960x_gmac_tx_reclaim(struct rtl960x_gmac *g, int budget);

static int rtl960x_gmac_rx_poll(struct napi_struct *napi, int budget)
{
	struct rtl960x_gmac *g = container_of(napi, struct rtl960x_gmac, rx_napi);
	int done;

	done = rtl960x_gmac_rx(g, budget);

	if (done < budget && napi_complete_done(napi, done))
		rtl960x_gmac_unmask_irqs(g, ISR_RX_ALL);

	return done;
}

static int rtl960x_gmac_tx_poll(struct napi_struct *napi, int budget)
{
	struct rtl960x_gmac *g = container_of(napi, struct rtl960x_gmac, tx_napi);
	int done;

	spin_lock_bh(&g->tx_lock);
	done = rtl960x_gmac_tx_reclaim(g, budget);
	if (netif_queue_stopped(g->ndev) &&
	    (g->tx_head + 1) % TX_RING_SIZE != g->tx_tail)
		netif_wake_queue(g->ndev);
	spin_unlock_bh(&g->tx_lock);

	if (done < budget && napi_complete_done(napi, done))
		rtl960x_gmac_unmask_irqs(g, ISR_TOK);

	if (done > budget)
		return budget;
	else
		return done;
}

static irqreturn_t rtl960x_gmac_isr(int irq, void *dev_id)
{
	struct rtl960x_gmac *g = dev_id;
	u16 status = gmac_r16(g, GMAC_ISR);
	status &= gmac_r16(g, GMAC_IMR);

	status &= ISR_NAPI;
	if (!status)
		return IRQ_NONE;

	gmac_w16(g, GMAC_ISR, status);		/* W1C ack */

	rtl960x_gmac_mask_irqs(g);
	napi_schedule_irqoff(&g->napi);

	return IRQ_HANDLED;
}

/* Reclaim TX descriptors the HW has finished with. Caller holds tx_lock. */
static int rtl960x_gmac_tx_reclaim(struct rtl960x_gmac *g, int budget)
{
	int done = 0;

	while (g->tx_tail != g->tx_head && done < budget) {
		struct rtl960x_tx_desc *d = &g->tx_ring[g->tx_tail];
		struct sk_buff *skb;

		if (d->opts1 & DESC_OWN)
			break;

		skb = g->tx_skb[g->tx_tail];
		if (skb) {
			dma_unmap_single(g->dev, d->addr, skb->len,
					 DMA_TO_DEVICE);
			g->ndev->stats.tx_packets++;
			g->ndev->stats.tx_bytes += skb->len;
			dev_consume_skb_any(skb);
			g->tx_skb[g->tx_tail] = NULL;
			done++;
		}
		g->tx_tail = (g->tx_tail + 1) % TX_RING_SIZE;
	}

	return done;
}

static netdev_tx_t rtl960x_gmac_start_xmit(struct sk_buff *skb,
					   struct net_device *ndev)
{
	struct rtl960x_gmac *g = netdev_priv(ndev);
	struct rtl960x_tx_desc *d;
	dma_addr_t dma;
	int dest_port = -1;
	bool last;
	u32 opts1;
	u32 next;
	int len;

	spin_lock_bh(&g->tx_lock);

	next = (g->tx_head + 1) % TX_RING_SIZE;
	if (next == g->tx_tail) {
		/*
		 * Ring full: stop the queue and ask the stack to requeue. The
		 * skb must stay pristine here -- it is retried as-is, so the DSA
		 * trailer (pulled off below) must not be removed before we know
		 * the frame is actually going on the ring.
		 */
		netif_stop_queue(ndev);
		spin_unlock_bh(&g->tx_lock);
		return NETDEV_TX_BUSY;
	}

	/*
	 * DSA egress: a frame from a user port carries the rtl_otto trailer
	 * [0x80, port, 0x10, 0x00]. Pull it off and steer the frame to that
	 * switch port via the TX descriptor instead of normal L2 lookup.
	 */
	if (netdev_uses_dsa(ndev) && skb->len >= DSA_TRAILER_LEN) {
		const u8 *t = skb->data + skb->len - DSA_TRAILER_LEN;

		if (t[0] == 0x80 && t[1] < RTL960X_CPU_PORT &&
		    t[2] == 0x10 && t[3] == 0x00) {
			dest_port = t[1];
			skb_trim(skb, skb->len - DSA_TRAILER_LEN);
		}
	}

	if (skb_put_padto(skb, TX_MIN_LEN)) {
		spin_unlock_bh(&g->tx_lock);
		return NETDEV_TX_OK;		/* skb freed by skb_put_padto */
	}
	len = skb->len;

	dma = dma_map_single(g->dev, skb->data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(g->dev, dma)) {
		spin_unlock_bh(&g->tx_lock);
		ndev->stats.tx_dropped++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	last = (g->tx_head == TX_RING_SIZE - 1);
	d = &g->tx_ring[g->tx_head];
	g->tx_skb[g->tx_head] = skb;
	d->addr = dma;
	if (dest_port >= 0) {
		d->opts2 = TX_OPTS2_CPUTAG | TX_OPTS2_PORTMASK(BIT(dest_port));
		d->opts3 = TX_OPTS3_KEEP | TX_OPTS3_DISLRN | TX_OPTS3_L34_KEEP;
	} else {
		d->opts2 = 0;
		d->opts3 = 0;
	}
	d->opts4 = 0;
	opts1 = DESC_OWN | DESC_FS | DESC_LS | TX_DESC_CRC | len |
		(last ? DESC_EOR : 0);
	dma_wmb();
	d->opts1 = opts1;
	dma_wmb();

	g->tx_head = next;

	/* No room for one more frame: stop now, NAPI wakes us on TX reclaim. */
	if ((next + 1) % TX_RING_SIZE == g->tx_tail)
		netif_stop_queue(ndev);

	/* Kick the TX ring. */
	gmac_w32(g, GMAC_IO_CMD, gmac_r32(g, GMAC_IO_CMD) | IO_CMD_TX_POLL);

	spin_unlock_bh(&g->tx_lock);

	return NETDEV_TX_OK;
}

static void rtl960x_gmac_free_tx(struct rtl960x_gmac *g)
{
	int i;

	for (i = 0; i < TX_RING_SIZE; i++) {
		struct rtl960x_tx_desc *d = &g->tx_ring[i];

		if (!g->tx_skb[i])
			continue;
		dma_unmap_single(g->dev, d->addr, g->tx_skb[i]->len,
				 DMA_TO_DEVICE);
		dev_kfree_skb(g->tx_skb[i]);
		g->tx_skb[i] = NULL;
	}
}

static int rtl960x_gmac_open(struct net_device *ndev)
{
	struct rtl960x_gmac *g = netdev_priv(ndev);
	int ret;

	ret = rtl960x_gmac_alloc_rings(g);
	if (ret)
		return ret;

	ret = request_irq(g->irq, rtl960x_gmac_isr, 0, ndev->name, g);
	if (ret) {
		netdev_err(ndev, "failed to request irq %d: %d\n", g->irq, ret);
		goto err_free_rings;
	}

	napi_enable(&g->rx_napi);
	napi_enable(&g->tx_napi);
	rtl960x_gmac_init_hw(g);

	netif_start_queue(ndev);
	/*
	 * Link state is managed by the switch core, which we don't drive yet;
	 * force the carrier on so the stack will pass traffic for bring-up.
	 */
	netif_carrier_on(ndev);

	netdev_info(ndev, "datapath up (RX ring %d, TX ring %d)\n",
		    RX_RING_SIZE, TX_RING_SIZE);

	return 0;

err_free_rings:
	rtl960x_gmac_free_rings(g);
	return ret;
}

static int rtl960x_gmac_stop(struct net_device *ndev)
{
	struct rtl960x_gmac *g = netdev_priv(ndev);

	printk("stop gmac\n");

	netif_stop_queue(ndev);
	netif_carrier_off(ndev);

	printk("after stop queue\n");
	rtl960x_gmac_stop_hw(g);
	printk("after stop HW\n");

	napi_disable(&g->rx_napi);
	napi_disable(&g->tx_napi);
	free_irq(g->irq, g);

	printk("gmac free\n");
	rtl960x_gmac_free_tx(g);
	rtl960x_gmac_free_rings(g);
	printk("stop gmac done\n");

	return 0;
}

static int rtl960x_gmac_set_mac_address(struct net_device *ndev, void *p)
{
	struct rtl960x_gmac *g = netdev_priv(ndev);
	int ret;

	ret = eth_mac_addr(ndev, p);
	if (ret)
		return ret;

	if (netif_running(ndev))
		rtl960x_gmac_set_hwaddr(g);

	return 0;
}

static const struct net_device_ops rtl960x_gmac_netdev_ops = {
	.ndo_open		= rtl960x_gmac_open,
	.ndo_stop		= rtl960x_gmac_stop,
	.ndo_start_xmit		= rtl960x_gmac_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= rtl960x_gmac_set_mac_address,
};

static int rtl960x_gmac_get_regs_len(struct net_device *ndev)
{
	return RTL960X_GMAC_REGS_DUMP_LEN;
}

static void rtl960x_gmac_get_regs(struct net_device *ndev,
				  struct ethtool_regs *regs, void *p)
{
	struct rtl960x_gmac *g = netdev_priv(ndev);
	u32 *buf = p;
	int i;

	regs->version = 1;
	memset(p, 0, RTL960X_GMAC_REGS_DUMP_LEN);

	for (i = 0; i < RTL960X_GMAC_REGS_DUMP_LEN / sizeof(u32); i++)
		buf[i] = gmac_r32(g, i * sizeof(u32));
}

static int rtl960x_gmac_get_4(struct net_device *ndev) {
	return 4;
}

static int rtl960x_gmac_get_cmd(struct net_device *ndev, struct ethtool_eeprom *eee, u8 *buf) {
	struct rtl960x_gmac *g = netdev_priv(ndev);
	int i;
	for (i = 0; i < eee->len; i++){
		buf[i] = gmac_r8(g, GMAC_IO_CMD + eee->offset + i);
	}
	return 0;
}

static int rtl960x_gmac_set_cmd(struct net_device *ndev, struct ethtool_eeprom *eee, u8 *buf) {
	struct rtl960x_gmac *g = netdev_priv(ndev);
	int i;
	for (i = 0; i < eee->len; i++){
		gmac_w8(g, GMAC_IO_CMD + eee->offset + i, buf[i]);
	}
	return 0;
}

static const struct ethtool_ops rtl960x_gmac_ethtool_ops = {
	.get_regs_len	= rtl960x_gmac_get_regs_len,
	.get_regs	= rtl960x_gmac_get_regs,
	.get_eeprom_len	= rtl960x_gmac_get_4,
	.get_eeprom	= rtl960x_gmac_get_cmd,
	.set_eeprom	= rtl960x_gmac_set_cmd,
};

static void rtl960x_gmac_assert_reset(void *rst)
{
	reset_control_assert(rst);
}

static int rtl960x_gmac_probe(struct platform_device *pdev)
{
	struct rtl960x_gmac *g;
	struct net_device *ndev;
	struct resource *res;
	int irq;
	int ret;

	ndev = devm_alloc_etherdev(&pdev->dev, sizeof(*g));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);
	ndev->netdev_ops = &rtl960x_gmac_netdev_ops;
	ndev->ethtool_ops = &rtl960x_gmac_ethtool_ops;
	ndev->min_mtu = ETH_MIN_MTU;
	ndev->max_mtu = RX_MAX_FRAME_LEN - ETH_HLEN - ETH_FCS_LEN;

	eth_hw_addr_random(ndev);
	netif_carrier_off(ndev);

	g = netdev_priv(ndev);
	g->ndev = ndev;
	g->dev = &pdev->dev;
	spin_lock_init(&g->tx_lock);

	g->base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(g->base))
		return PTR_ERR(g->base);

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return ret;

	/*
	 * Ungate the GMAC IP clock before any register access. GMAC0 is left
	 * enabled by the bootloader, but GMAC1/GMAC2 are gated and reading
	 * their windows while gated hangs the LX bus and reboots the SoC.
	 */
	g->rst = devm_reset_control_get_optional_exclusive(&pdev->dev, NULL);
	if (IS_ERR(g->rst))
		return dev_err_probe(&pdev->dev, PTR_ERR(g->rst),
				     "failed to get reset control\n");
	ret = reset_control_deassert(g->rst);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to ungate GMAC IP clock\n");
	ret = devm_add_action_or_reset(&pdev->dev, rtl960x_gmac_assert_reset,
				       g->rst);
	if (ret)
		return ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;
	g->irq = irq;

	netif_napi_add(ndev, &g->rx_napi, rtl960x_gmac_rx_poll);
	netif_napi_add_tx(ndev, &g->tx_napi, rtl960x_gmac_tx_poll);

	ret = devm_register_netdev(&pdev->dev, ndev);
	if (ret)
		return ret;

	netdev_info(ndev, "RTL960x/LUNA GMAC %pR irq %d\n", res, g->irq);

	return 0;
}

static const struct of_device_id rtl960x_gmac_of_match[] = {
	{ .compatible = "realtek,rtl8198d-gmac" },
	{ .compatible = "realtek,rtl9607c-gmac" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl960x_gmac_of_match);

static struct platform_driver rtl960x_gmac_driver = {
	.probe = rtl960x_gmac_probe,
	.driver = {
		.name = "rtl960x-gmac",
		.of_match_table = rtl960x_gmac_of_match,
	},
};
module_platform_driver(rtl960x_gmac_driver);

MODULE_DESCRIPTION("Realtek RTL960x/LUNA GMAC driver");
MODULE_AUTHOR("Taiga Ogawa");
MODULE_LICENSE("GPL");
