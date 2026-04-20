/******************************************************************************
 *
 * Copyright(c) 2007 - 2022 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef _RTL_WFO_PRIV_H_
#define _RTL_WFO_PRIV_H_

#define RTL_WFO_IS_OFLD(devobj)	(!!(devobj->wfo_priv))

//#define FBM_USE_KMALLOC_BUF
//#define CONFIG_WFO_NEW_ARCH

/* PE NIRX buffer definition */
#define PE_NIRX_BUF_SZ		(4096)
#define PE_NIRX_BUF_NUM		(4 * 1024 * 1024 / PE_NIRX_BUF_SZ)
#if PE_NIRX_BUF_SZ == 4096
#define PE_NIRX_RES_NUM		(7)
#elif PE_NIRX_BUF_SZ == 8192
#define PE_NIRX_RES_NUM		(11)
#else
#error "wrong PE NIRX buffer size"
#endif

/* AMSDU ofld NIRX buffer definition */
#define AMSDU_NIRX_BUF_SZ	(1024)
#define AMSDU_NIRX_BUF_NUM 	(4 * 1024 * 1024 / AMSDU_NIRX_BUF_SZ)
#if AMSDU_NIRX_BUF_SZ == 128 || AMSDU_NIRX_BUF_SZ == 256
#define AMSDU_NIRX_RES_NUM	(1)
#elif AMSDU_NIRX_BUF_SZ == 512
#define AMSDU_NIRX_RES_NUM	(2)
#elif AMSDU_NIRX_BUF_SZ == 1024 || AMSDU_NIRX_BUF_SZ == 2048
#define AMSDU_NIRX_RES_NUM	(5)
#else
#error "wrong AMSDU Ofld NIRX buffer size"
#endif

#define PE_MEM_BUF_NUM		(1) /* DRAM memory pool */
#define PE_TX_BUF_NUM		(2)

/* PE: rtl_wfo_conf.h */
#if defined(CONFIG_RTL9607F_SERIES)
#define MAX_PE_NUM			(3)
#define WFO_PPE_NUM			(2)	/* number of PPE */
#elif defined(CONFIG_RTL8277B_SERIES) || defined(CONFIG_RTL8277C_SERIES)
#define MAX_PE_NUM 			(2)
#define WFO_PPE_NUM			(1)	/* number of PPE */
#else
#error "Unknown platform."
#endif

/* PE: rtl_wfo_resource.h */
#define PI_RING_RW_PTR_SIZE		4
#define PI_RING_NUM				6
#define DRV_PI_RING_NUM			3
#define DOE_PI_RING_NUM			(PI_RING_NUM * WFO_PPE_NUM + DRV_PI_RING_NUM)

#define FC_STA_NUM				40
#define FC_WMM_VOQ_NUM			4
#define RTL_WFO_MAX_TID_NUM		8

enum rtl_wfo_amsdu_ofld_mode {
	AMSDU_OFLD_40STA_BE = 0,
	AMSDU_OFLD_10STA_WMM = 1,
	AMSDU_OFLD_MAX
};

/* RTL WFO TRX Offload */
#define RTL_WFO_RX_OFLD			BIT0
#define RTL_WFO_TX_OFLD			BIT1
#define RTL_WFO_TX_FULL_OFLD	BIT2

/* definition of HW role */
enum rtl_wfo_pe_id {
	RTL_WFO_PE0_ID = 0,
	RTL_WFO_PE1_ID = 1,
	RTL_WFO_PE2_ID = 2,
	RTL_WFO_ARM_ID = 3, /* must behind all PE */
	RTL_WFO_PEID_MAX,
	RTL_WFO_PEID_ALL = RTL_WFO_PEID_MAX
};

/* definition of SW role */
/* note:
   1. If there are multiple PPE, make sure the HW PE_ID is continuous.
      Thus rps_pe_id can be simply assigned by (PPE_ID + macid % PPE_NUM).
   2. DOE_ID is not necessary to be greater than PPE_ID.
      So don't use code such as 'if (rps_pe_id >= DOE_ID)'.
 */
#define RTL_WFO_PPE_ID		RTL_WFO_PE0_ID	/* Packet Processing Engine */
#define RTL_WFO_DOE_ID		RTL_WFO_PE2_ID	/* DMA Offload Engine */
#define RTL_WFO_AOE_ID		RTL_WFO_PE2_ID	/* AMSDU Offload Engine */

/* WFO new-arch */
#ifdef CONFIG_WFO_NEW_ARCH
#undef WFO_PPE_NUM
#undef PE_MEM_BUF_NUM
#undef RTL_WFO_PPE_ID
#undef RTL_WFO_DOE_ID
#undef RTL_WFO_AOE_ID
#define WFO_PPE_NUM			(1)
#define PE_MEM_BUF_NUM		(2)
#define RTL_WFO_PPE_ID		RTL_WFO_PE0_ID
#define RTL_WFO_DOE_ID		RTL_WFO_PE1_ID
#define RTL_WFO_AOE_ID		RTL_WFO_PE2_ID
#endif

/* WFO resource setting */
#define PE_RX_BUF_NUM		(3 + 1 + 1 * WFO_PPE_NUM)
							/* RXQ * 3 + RPQ * 1 + REORDER * WFO_PPE_NUM */
#define RTL_WFO_RES_NUM		(PE_MEM_BUF_NUM + PE_RX_BUF_NUM + PE_TX_BUF_NUM \
								+ PE_NIRX_RES_NUM + AMSDU_NIRX_RES_NUM)

typedef struct pi_info_s {
	u16 pi_ring_num;
	u8 pi_ring_size;
	u8 pi_ring_dsize;
} pi_info_t;

/* Because pi_info_t is used by both IPC and wfo_dev,
   separate pi_addr from pi_info to save IPC size. */
typedef struct pi_addr_s {
	u32 pi_ring_rw_ptr;
	u32 pi_ring_addr;
} pi_addr_t;

typedef struct rtl_wfo_pe_info_s {
	u8 state;
	u8 pe_role;
	u32 pe_priv_addr;
	u32 *pe_priv;
	u32 last_hb;
} rtl_wfo_pe_info_t;

typedef struct rtl_wfo_pe_resource_s {
	u8 force_init;
	u8 res_pe_id;
	u8 res_order;	/* resource size = PAGE_SIZE(4K) * 2^order */
	u16 res_type;	/* resource type */
	u32 res_addr;	/* address of allocated buffer */
	u32 res_size;	/* total allocated buffer size */
	u32 buf_size;	/* size of each buffer */
	u32 buf_num;	/* number of buffers */
} rtl_wfo_pe_resource_t;

typedef struct fc_sta_s {
	u8 macid;
	u8 band;
	u8 is_sta; /* sta or a4 db */
	u8 mac[ETH_ALEN];
} fc_sta_t;

typedef struct rtl_wfo_dev_s {
	/* device obj info */
	struct dvobj_priv *dvobj;
	u16 total_sta_num;

	/* DOE sw_seq pool */
	u32 sw_seq_addr;	/* DOE's sw_seq base address */
	u16 *doe_sw_seq;	/* 128 STA * 8 TID */

	/* PktInfo ring info */
	pi_info_t pi_info[DOE_PI_RING_NUM];
	pi_addr_t pi_addr[DOE_PI_RING_NUM];
} rtl_wfo_dev_t;

typedef struct rtl_wfo_priv_s {
	/* PE monitor timer */
	_timer hb_timer;
	u8 hb_started;
	u8 hb_hangup_check;

	/* multi-band info */
	u8 wfo_band_cap;	/* 2g/5g/6g enable */
	u8 wfo_band_on;		/* activated band */
	rtl_wfo_dev_t dev[BAND_MAX];

	/* AMSDU Ofld & FC */
	_lock fc_lock;
	u8 amsdu_mode; /* to be removed */
	fc_sta_t fc_sta[FC_STA_NUM];

	/* PE info */
	rtl_wfo_pe_info_t pe_info[MAX_PE_NUM];

	/* Buffer resource */
	rtl_wfo_pe_resource_t resource[RTL_WFO_RES_NUM];
	struct page *res_pages[RTL_WFO_RES_NUM];
	u8 res_state;
	u8 res_num;			/* total number of allocated resources */
	u8 res_nirx_num;	/* number of NIRX_BUF, max=PE_NIRX_RES_NUM */
	u8 res_amsdu_num;	/* number of AMSDU_NIRX_BUF, max=AMSDU_NIRX_RES_NUM */
	u16 next_nirx_th;	/* threshold to allocate more NIRX_BUF */
	u16 next_amsdu_th;	/* threshold to allocate more AMSDU_NIRX_BUF */

#ifdef FBM_USE_KMALLOC_BUF
	void *res_tbl;
#endif
} rtl_wfo_priv_t;

typedef enum {
	PE_START = 0,
	PE_START_WFO,
	PE_SUSPEND_WFO,
	PE_STOP_WFO,
	PE_STOP,
} pe_state_e;

enum rtl_wfo_pe_role {
	PE_ROLE_PPE		= BIT0,
	PE_ROLE_DOE		= BIT1,
	PE_ROLE_AOE		= BIT2,
	PE_ROLE_ALL		= BIT7, /* for IPC, force to send to all PE */
};

typedef enum {
	PE_RES_NOT_ALLOC = 0,
	PE_RES_ALLOC,
	PE_RES_ASSIGNED,
	PE_RES_MAX,
} pe_res_state_e;

void rtl_wfo_init(void);
void rtl_wfo_deinit(void);
int rtl_wfo_supported_band(struct dvobj_priv *dvobj);
void rtl_wfo_priv_init(struct dvobj_priv *dvobj);
void rtl_wfo_priv_deinit(struct dvobj_priv *dvobj);
u8 rtl_wfo_pe_id_mapping(struct sta_info *psta);
void rtl_wfo_pe_id_decision(_adapter *adapter, struct sta_info *psta);
u8 rtl_wfo_set_offload(_adapter *adapter, u8 rx_en, u8 tx_en,
	u8 pe_state);
void rtl_wfo_start_pe(_adapter *padapter);
void rtl_wfo_stop_pe(_adapter *padapter);
void rtl_wfo_restart_drv_trx(struct dvobj_priv *dvobj);
int rtl_wfo_alloc_res(struct dvobj_priv *dvobj);
int rtl_wfo_check_res(_adapter *adapter);

#ifdef FBM_USE_KMALLOC_BUF
enum {
	RESOURCE_NIRX = 0,
	RESOURCE_AMSDU_NIRX,
	RESOURCE_MAX,
};

enum {
	RES_STAGE_0_NOSTA = 0,
	RES_STAGE_1_1STA,
	RES_STAGE_2_2STA,
	RES_STAGE_3_10STA,
	RES_STAGE_4_PON_VW,
	RES_STAGE_MAX
};

enum {
	RES_MV_FROM_IDLE = 0,
	RES_MV_TO_IDLE,
	RES_MV_MAX,
};

#define PON_VW_STA	(0xFA11)
#define ADD_BUFINFO	(0xFADD)

typedef struct {
	u32 sta_num;
	u32 buf_num;
	u32 *buf_info;
	u32 buf_info_sz;
} rtl_wfo_res_t;

typedef struct {
	u32 buf_size;	/* size of each buffer */
	u32 buf_num;	/* number of buffers */
	rtl_wfo_res_t stage[RES_STAGE_MAX];
	u32 rec_max;
	u32 *rec_addr;
	u32 *free_info;
} rtl_wfo_res_tbl_t;

void rtl_wfo_init_res(void);
int rtl_wfo_add_res(struct dvobj_priv *dvobj, u16 type, rtl_wfo_res_t *stage);
u32 rtl_wfo_free_res(struct dvobj_priv *dvobj, u16 res_type, u32 buf_num);
rtl_wfo_res_t *rtl_wfo_check_res_stage(int type, int sta_num);
void rtl_wfo_start_pe_kmem_res(struct dvobj_priv *dvobj);
void rtl_wfo_res_inv(struct dvobj_priv *dvobj, u32 phy_addr_l, int buf_size);
void rtl_wfo_res_wback(struct dvobj_priv *dvobj, u32 phy_addr_l, int buf_size);
#else
int rtl_wfo_add_res(struct dvobj_priv *dvobj, u16 res_type, int order);
#endif /* FBM_USE_KMALLOC_BUF  */
#endif /* _RTL_WFO_PRIV_H_ */

