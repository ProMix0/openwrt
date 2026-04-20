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
#ifndef _RTL_WFO_IPC_H_
#define _RTL_WFO_IPC_H_

#define MAX_WLHDR_SIZE		48 		/* CORE_TXSC_WLHDR_SIZE (WLHDR_SIZE + SNAP_SIZE + 2 + _AES_IV_LEN_) */
#define MAX_WD_CACHE_SIZE	(32+32)	/* WD Body + WD Info */

typedef enum {
	MIB_INT_T,
	MIB_STR_T,
} wfo_mib_type;

struct rtl_wfo_mib_tbl {
	u16 id;
	u8 name[16];
	u8 str[48];
	wfo_mib_type type;
};

/* START - MUST be the same with ca-zephyr-system/rtl_wfo/include/rtl_wfo_ipc.h */
// The number of cmd types must < RE_IPC_MAX_PROCS (0x14, ca_utility\soft_ipc\ca_soft_ipc_priv.h)
typedef enum {
	/* Host --> PE */
	RTL_WFO_IPC_PE_RSVD	= 0x0,
	RTL_WFO_IPC_PE_START,
	RTL_WFO_IPC_PE_STOP,
	RTL_WFO_IPC_PE_MESSAGE,
	/* PE --> Host */
	RTL_WFO_IPC_PE_START_COMPLETE,
	RTL_WFO_IPC_PE_STOP_COMPLETE,
	RTL_WFO_IPC_PE_MESSAGE_COMPLETE,
	RTL_WFO_IPC_PE_HANDLE_RX_PKT,
	RTL_WFO_IPC_PE_UPDATE_A4_INFO,
} rtl_wfo_ipc_pe_e;

typedef struct rlt_wfo_ipc_msg_sz_s {
	/* PE & DOE command */
	u32 cmd_start_size;
	u32 cmd_set_pe_addr_size;
	u32 resource_size;
	u32 cmd_device_info_size;
	u32 cmd_adapter_info_size;
	u32 cmd_sta_info_size;
	u32 cmd_txsc_info_size;
	u32 cmd_rxofld_info_size;
	/* PE & DOE to Driver command */
	u32 cmd_update_a4_info_size;
	/* Response */
	u32 rsp_pe_info_size;
	u32 rsp_ring_addr_size;
	u32 rsp_mib_size;
} rtl_wfo_ipc_msg_sz_t;

/* IPC Message Command */
typedef enum {
	/* Driver to PE & DOE command: 0x0 - 0x5f */
	RTL_WFO_IPC_MSG_CMD_RSVD				= 0x0,
	RTL_WFO_IPC_MSG_CMD_START_WFO,
	RTL_WFO_IPC_MSG_CMD_STOP_WFO,
	RTL_WFO_IPC_MSG_CMD_RESOURCE,
	RTL_WFO_IPC_MSG_CMD_SEND_DEVICE_INFO,
	RTL_WFO_IPC_MSG_CMD_CFG_ADAPTER_INFO,
	RTL_WFO_IPC_MSG_CMD_CFG_STA_INFO,
	RTL_WFO_IPC_MSG_CMD_UPDATE_TXSC_INFO,
	RTL_WFO_IPC_MSG_CMD_UPDATE_RXOFLD_INFO,
	RTL_WFO_IPC_MSG_CMD_GET_RING_ADDR,
	RTL_WFO_IPC_MSG_CMD_SET_PE_PI_ADDR,
	RTL_WFO_IPC_MSG_CMD_DEL_A4_DB,
	RTL_WFO_IPC_MSG_CMD_UPDATE_FC_STA,

	/* PE & DOE to Driver command: 0x60 - 0x7f */
	RTL_WFO_IPC_MSG_CMD_HANDLE_MCAST		= 0x60,
	RTL_WFO_IPC_MSG_CMD_UPDATE_A4_INFO,

	/* MIB: 0x80 - 0xbf */
	RTL_WFO_IPC_MSG_CMD_SET_MIB				= 0x80,
	RTL_WFO_IPC_MSG_CMD_GET_MIB,

	/* Debug & Profile: 0xc0 - 0xff */
	RTL_WFO_IPC_MSG_CMD_GET_STATUS			= 0xc0,
	RTL_WFO_IPC_MSG_CMD_PRIV_INFO,
	RTL_WFO_IPC_MSG_CMD_GET_WMM_STATUS,
	RTL_WFO_IPC_MSG_CMD_DUMP_MEM,
	RTL_WFO_IPC_MSG_CMD_DUMP_DOE_CHAN,
	RTL_WFO_IPC_MSG_CMD_START_PROFILING,
	RTL_WFO_IPC_MSG_CMD_STOP_PROFILING,
	RTL_WFO_IPC_MSG_CMD_GET_MEMMAP,
	RTL_WFO_IPC_MSG_CMD_UART_SELECT,
	RTL_WFO_IPC_MSG_CMD_GET_A4_INFO,

	RTL_WFO_IPC_MSG_CMD_MAX					= 0xff,
} rtl_wfo_ipc_msg_cmd_e;

/* Return status codes */
typedef enum {
	RTL_WFO_SUCCESS = 0,
	RTL_WFO_FAILURE,
	RTL_WFO_INVALID_ARGV,
	RTL_WFO_INVALID_SEQ,
	RTL_WFO_NOT_FOUND,
	RTL_WFO_EXISTS,
	RTL_WFO_INTERNAL_ERROR,
} rtl_wfo_status_e;

/* --------- PE & DOE command --------- */
typedef struct rtl_wfo_buf_recycle_info_s {
	u32 buf_size;
	u32 buf_info;
	u32 buf_info_sz;
	u32 recycle_cnt;
} rtl_wfo_buf_recycle_info_t;

typedef struct rtl_wfo_ipc_cmd_start_s {
	u8 rx_en;
	u8 tx_en;
	u16 wp_max_seq;
	u8 pe_state;
	u8 amsdu_mode;
#ifdef FBM_USE_KMALLOC_BUF
	rtl_wfo_buf_recycle_info_t res[RESOURCE_MAX];
#endif /* FBM_USE_KMALLOC_BUF */
} rtl_wfo_ipc_cmd_start_t;

enum rtl_wfo_op {
	RTL_WFO_OP_ADD = 0,
	RTL_WFO_OP_UPDATE,
	RTL_WFO_OP_DELETE,
	RTL_WFO_OP_A4_UPDATE,
	RTL_WFO_OP_DUMP,
};

typedef struct rtl_wfo_ipc_cmd_set_pe_addr_s {
	pi_addr_t pi_addr[MAX_PE_NUM];
} rtl_wfo_ipc_cmd_set_pe_addr_t;

enum rtl_wfo_res_type {
	RTL_WFO_RESOURCE_RX_BUF				= BIT0,
	RTL_WFO_RESOURCE_RP_BUF				= BIT1,
	RTL_WFO_RESOURCE_REORDER_BUF		= BIT2,
	RTL_WFO_RESOURCE_TX_BUF				= BIT3,
	RTL_WFO_RESOURCE_TX_SKB				= BIT4,
	RTL_WFO_RESOURCE_NIRX_BUF			= BIT5,
	RTL_WFO_RESOURCE_AMSDU_NIRX_BUF		= BIT6,
	RTL_WFO_RESOURCE_MEMPOOL			= BIT10,
	RTL_WFO_RESOURCE_BUF_DUMP			= BIT11,
	RTL_WFO_RESOURCE_BUF_DUMP_VERBOSE	= BIT12,
	RTL_WFO_RESOURCE_SKB_DUMP			= BIT13,
	RTL_WFO_RESOURCE_SKB_DUMP_VERBOSE	= BIT14,
	RTL_WFO_RESOURCE_MEMPOOL_DUMP		= BIT15,
};

typedef struct rtl_wfo_ipc_resource_s {
	u8 force_init;
	u16 res_type;	/* resource type */
	u32 res_addr;	/* address of allocated buffer */
	u32 res_size;	/* total allocated buffer size */
	u32 buf_size;	/* size of each buffer */
	u32 buf_num;	/* number of buffers */
} rtl_wfo_ipc_resource_t;

typedef struct rtl_wfo_ipc_cmd_device_info_s {
	u32 pcie_phy_addr_start;
	u32 chip_id;
	u8 append_fcs;
	u8 accept_icv_err;
} rtl_wfo_ipc_cmd_device_info_t;

typedef struct rtl_wfo_ipc_cmd_adapter_info_s {
	enum rtl_wfo_op op;
	u8 mac_addr[ETH_ALEN];
	u8 br_mac[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	u8 iface_id;
	u8 a4_enable;
	u8 adapter_type;
	u8 wlan_dev_id;
	int fw_state;
	u32 authtype;
	u32 encmode;
} rtl_wfo_ipc_cmd_adapter_info_t;

typedef struct rtl_wfo_ipc_cmd_sta_info_s {
	enum rtl_wfo_op op;
	u8 mac_addr[ETH_ALEN];
	u8 adapter_mac_addr[ETH_ALEN];
	u16 macid;
	u8 active;
	u32 state;
	u8 rps_pe_id;	/* Receive Packet Steering */
	u8 fc_staid;
	u8 agg_en_bitmap;
	u8 tx_amsdu_enable;
	u8 amsdu_max_num;
	u16 amsdu_max_size;
	u32 keytrack;
	u8 twt_en;
	u16 wpseq_lmt;
} rtl_wfo_ipc_cmd_sta_info_t;

typedef struct rtl_wfo_ipc_cmd_txsc_info_s {
	u16 macid;
	u8 txsc_wlhdr[MAX_WLHDR_SIZE];
	u8 wlhdr_len;
	u8 wd_cache[MAX_WD_CACHE_SIZE];
	u8 wd_len;
	u8 wd_seq_offset;
} rtl_wfo_ipc_cmd_txsc_info_t;

typedef struct rtl_wfo_ipc_cmd_rxofld_info_s {
	u16 macid;
	u8 adapter_mac_addr[ETH_ALEN];
	u16 ssn;
	u16 buf_size;
	u8 tid;
} rtl_wfo_ipc_cmd_rxofld_info_t;

typedef struct rtl_wfo_ipc_cmd_del_a4_db_s {
	u8 mac[ETH_ALEN];
	u8 iface_id;
	u8 fc_staid;
} rtl_wfo_ipc_cmd_del_a4_db_t;

typedef struct rtl_wfo_ipc_cmd_upt_fc_sta_s {
	u16 macid;
	u8 fc_staid;
} rtl_wfo_ipc_cmd_upt_fc_sta_t;

typedef struct rtl_wfo_ipc_cmd_update_a4_info_s {
	u8 mac[ETH_ALEN];
	u8 iface_id;
	u8 op;
	u16 macid;
	u16 tx_cnt;
	u16 rx_cnt;
	u32 tx_bytes;
	u32 rx_bytes;
} rtl_wfo_ipc_cmd_update_a4_info_t;

/* --------- MIB --------- */
/* MIB ID Table */
typedef enum {
	RTL_WFO_MIB_MON_INTVL = 0x0,		/* monitor interval (s) */
	RTL_WFO_MIB_MON_BITMAP,				/* monitor bitmap (rtl_wfo_mon_type) */
	RTL_WFO_MIB_LOG_LEVEL,				/* log level 0-6 */
	RTL_WFO_MIB_LOG_COMP,				/* tracing components */
	RTL_WFO_MIB_AMSDU_TO,				/* timeout (ms) */
	RTL_WFO_MIB_AMSDU_MIN_PKT,			/* min pkt count in VOQ */
	RTL_WFO_MIB_AMSDU_BIG_PKT_MAX,		/* max of packets per AMSDU */
	RTL_WFO_MIB_AMSDU_SMALL_PKT_MAX,	/* max of small packets per AMSDU */
	RTL_WFO_MIB_AMSDU_MAX_SIZE,			/* max of packet size per AMSDU */
	RTL_WFO_MIB_AMSDU_AGG,				/* max of AMSDU packets per STA */
	RTL_WFO_MIB_TXREQ_LMT,				/* max number of wp_seq for each ACH */
	RTL_WFO_MIB_TXREQ_LMT_DROP,			/* wp_seq limit drop enable/disable */
	RTL_WFO_MIB_MAX_AGG_NUM,			/* max of TX agg num */
	RTL_WFO_MIB_PE_VOQ_CHK,				/* PE0/1 VOQ qlen check */
	RTL_WFO_MIB_GTR_MAX,				/* max number of rx gather num */
	RTL_WFO_MIB_RFL_MAX,				/* max number of rx refill num */
	RTL_WFO_MIB_SCT_MAX,				/* max number of tx scatter num */
	RTL_WFO_MIB_RCY_MAX,				/* max number of tx recycle num */
	RTL_WFO_MIB_TID_REMAP,				/* remap TID (0,1,4,6) */
	RTL_WFO_MIB_TCP_ACK_OPT,			/* TCP ACK optimization */
	RTL_WFO_MIB_PKT_DUMP,				/* dump wlan hdr, wd, txbd */
	RTL_WFO_MIB_VERSION,				/* get WFO version info */
	RTL_WFO_MIB_SW_DEAMSDU,				/* SW DE-AMSDU */
	RTL_WFO_MIB_REORDER_TO,				/* RX reorder timeout (ms) */
	RTL_WFO_MIB_MAX = 0xffff,
} rtl_wfo_mib_id_e;

typedef struct rtl_wfo_ipc_cmd_set_mib_s {
	u16 id;
	int value;
} rtl_wfo_ipc_cmd_set_mib_t;

typedef struct rtl_wfo_ipc_cmd_get_mib_s {
	u16 id;
} rtl_wfo_ipc_cmd_get_mib_t;

/* --------- Debug & Profile --------- */
typedef struct rtl_wfo_ipc_cmd_dump_mem_s {
	u32 addr;
	u16 len;
} rtl_wfo_ipc_cmd_dump_mem_t;

typedef struct rtl_wfo_ipc_cmd_dump_doe_chan_s {
	u8 ch_idx;
} rtl_wfo_ipc_cmd_dump_doe_chan_t;

/* --------- Response --------- */
typedef struct rtl_wfo_ipc_rsp_pe_info_s {
	u8 pe_role;
	u8 ver_major;
	u8 ver_minor;
	u8 ver_hotfix;
	u32 git_sha1;
	u32 pe_priv_addr;
	u32 doe_sw_seq_addr;
	u32 ic_id;
	rtl_wfo_ipc_msg_sz_t ipc_msg_sz;
	u8 pi_start_idx;
	u8 pi_num;
	u32 pi_ring_rw_ptr;
	u32 pi_ring_addr;
	pi_info_t pi_info[DOE_PI_RING_NUM];
} rtl_wfo_ipc_rsp_pe_info_t;

typedef struct rtl_wfo_ipc_rsp_ring_addr_s {
	u16 rx_ring_size;
	u32 rx_ring_rw_base;
	u32 rx_ring_data_base;
	u16 rp_ring_size;
	u32 rp_ring_rw_base;
	u32 rp_ring_data_base;
	u16 tx_ring_size;
	u32 tx_ring_rw_base;
	u32 tx_ring_data_base;
} rtl_wfo_ipc_rsp_ring_addr_t;

typedef struct rtl_wfo_ipc_rsp_mib_s {
	union {
		int int_val;
		char str_val[64];
	};
} rtl_wfo_ipc_rsp_mib_t;

#define RTL_WFO_IPC_RX_PKT_SIZE	0x100 /* must < PAYLOAD_SIZE - sizeof hdr */
typedef struct rtl_wfo_ipc_rx_pkt_s {
	u8 iface_id;
	u32 len;
	u8 buf[RTL_WFO_IPC_RX_PKT_SIZE];
} rtl_wfo_ipc_rx_pkt_t;

typedef struct rtl_wfo_ipc_msg_s {
	union {
		struct {
			u8 cmd;		// refer to rtl_wfo_ipc_msg_cmd_e
			u8 cmd_seq;
			u16 extra;
		} pe_msg;
		struct {
			u8 ack_seq;
			u8 status;	// reference rtl_wfo_status_e
			u16 extra;
		} pe_msg_complete;
		u32 rsvd;
	} hdr;

	union {
		/* PE & DOE command */
		rtl_wfo_ipc_cmd_start_t cmd_en;
		rtl_wfo_ipc_cmd_set_pe_addr_t cmd_set_pe_addr;
		rtl_wfo_ipc_resource_t resource;
		rtl_wfo_ipc_cmd_device_info_t cmd_device_info;
		rtl_wfo_ipc_cmd_adapter_info_t cmd_adapter_info;
		rtl_wfo_ipc_cmd_sta_info_t cmd_sta_info;
		rtl_wfo_ipc_cmd_txsc_info_t cmd_txsc_info;
		rtl_wfo_ipc_cmd_rxofld_info_t cmd_rxofld_info;
		rtl_wfo_ipc_cmd_del_a4_db_t cmd_del_a4_db;
		rtl_wfo_ipc_cmd_upt_fc_sta_t cmd_upt_fc_sta;

		/* PE & DOE to Driver command */
		rtl_wfo_ipc_cmd_update_a4_info_t cmd_update_a4;

		/* MIB */
		rtl_wfo_ipc_cmd_set_mib_t cmd_set_mib;
		rtl_wfo_ipc_cmd_get_mib_t cmd_get_mib;

		/* Debug & Profile */
		rtl_wfo_ipc_cmd_dump_mem_t cmd_dump_mem;
		rtl_wfo_ipc_cmd_dump_doe_chan_t cmd_dump_doe_chan;

		/* Response */
		rtl_wfo_ipc_rsp_pe_info_t rsp_pe_info;
		rtl_wfo_ipc_rsp_ring_addr_t rsp_ring_addr;
		rtl_wfo_ipc_rsp_mib_t rsp_mib;
		rtl_wfo_ipc_rx_pkt_t rx_pkt;
	} content;
} rtl_wfo_ipc_msg_t;

#define RTL_WFO_IPC_MSG_SIZE	sizeof(rtl_wfo_ipc_msg_t)

/* END - MUST be the same with ca-zephyr-system/rtl_wfo/include/rtl_wfo_ipc.h */

/* PE state */
#define RTL_WFO_PE_INACTIVE		0x00
#define RTL_WFO_PE_ACTIVE		0x01
#define RTL_WFO_PE_RESET		0x02

#ifdef CONFIG_RTL9607F_SERIES
#define RTL_WFO_IPC_CPU_PE0		CA_IPC_CPU_PE0
#define RTL_WFO_IPC_CPU_PE1		CA_IPC_CPU_PE1
#define RTL_WFO_IPC_CPU_PE2		CA_IPC_CPU_PE2
#define RTL_WFO_IPC_CPU_MAX		CA_IPC_CPU_MAX
#elif defined(CONFIG_RTL8277C_SERIES)
#define RTL_WFO_IPC_CPU_PE0		CA_IPC_CPU_PE0
#define RTL_WFO_IPC_CPU_PE1		CA_IPC_CPU_PE0
#define RTL_WFO_IPC_CPU_PE2		CA_IPC_CPU_PE1
#define RTL_WFO_IPC_CPU_MAX		CA_IPC_CPU_MAX
#elif defined(CONFIG_CA8277B_SERIES)
#define RTL_WFO_IPC_CPU_PE0		CA_IPC_CPU_PE1
#define RTL_WFO_IPC_CPU_PE1		CA_IPC_CPU_PE1
#define RTL_WFO_IPC_CPU_PE2		CA_IPC_CPU_PE0
#define RTL_WFO_IPC_CPU_MAX		CA_IPC_CPU_MAX
#endif

int rtl_wfo_ipc_init(struct dvobj_priv *dvobj);
void rtl_wfo_ipc_deinit(struct dvobj_priv *dvobj);

/* Driver to PE & DOE command: 0x0 - 0x5f */
int rtl_wfo_ipc_start_wfo(_adapter *padapter, u8 mode,
	u16 wp_max_seq, u8 pe_state);
int rtl_wfo_ipc_stop_wfo(_adapter *padapter, u8 pe_state);
int rtl_wfo_ipc_set_resource(_adapter *padapter, u8 pe_id,
	rtl_wfo_pe_resource_t *res);
int rtl_wfo_ipc_set_resource_sync(_adapter *padapter, u8 pe_id,
	rtl_wfo_pe_resource_t *res);
int rtl_wfo_ipc_send_device_info(_adapter *padapter);
int rtl_wfo_ipc_cfg_adapter_info(_adapter *padapter, int op);
int rtl_wfo_ipc_update_fc_sta(_adapter *padapter,
	u16 macid, u8 fc_staid, u8 to_role, u8 ppe_pe_id);
int rtl_wfo_ipc_cfg_sta_info(_adapter *padapter,
	void *para, u8 to_role, int op);
int rtl_wfo_ipc_update_txsc_info(_adapter *padapter, struct sta_info *psta,
	u8 *txsc_wlhdr, u8 wlhdr_len, u8 *wd_cache, u8 wd_len, u8 wd_seq_offset);
int rtl_wfo_ipc_update_rxofld_info(_adapter *padapter, struct sta_info *psta,
	u32 ssn, u32 tid, u16 size);
int rtl_wfo_ipc_get_ring_addr(_adapter *padapter,
	rtl_wfo_ipc_rsp_ring_addr_t *ring_info);
int rtl_wfo_ipc_set_pe_addr(int session_id);
int rtl_wfo_ipc_del_a4_db(_adapter *padapter, u8 pe_id, u8 *mac, u8 fc_staid);

/* MIB: 0x80 - 0xbf */
int rtl_wfo_ipc_set_mib(_adapter *padapter, u8 pe_id, char *name, int value);
int rtl_wfo_ipc_get_mib(_adapter *padapter, u8 pe_id, char *name);

/* Debug & Profile: 0xc0 - 0xff */
int rtl_wfo_ipc_get_status(_adapter *padapter, u8 pe_id, u16 reset);
int rtl_wfo_ipc_priv_info(_adapter *padapter);
int rtl_wfo_ipc_get_wmm_status(_adapter *padapter, u16 reset);
int rtl_wfo_ipc_dump_mem(_adapter *padapter, u8 pe_id, u32 addr, u16 len);
int rtl_wfo_ipc_dump_doe_chan(_adapter *padapter, u8 pe_id, u8 ch_idx);
int rtl_wfo_ipc_start_profiling(_adapter *padapter, u8 pe_id);
int rtl_wfo_ipc_stop_profiling(_adapter *padapter, u8 pe_id);
int rtl_wfo_ipc_get_memory_map(_adapter *padapter, u8 pe_id);
int rtl_wfo_ipc_set_uart_selection(_adapter *padapter, u8 uart_sel);
int rtl_wfo_ipc_get_a4_info(_adapter *padapter, u8 pe_id, int op);
void apply_pe_amsdu_para(_adapter *padapter);
void apply_pe_tx_force_para(_adapter *padapter);
void apply_pe_tc_latency_param(_adapter *padapter,
	u8 sta_num, u16 new_pkt_len);
void apply_pe_tc_tput_param(_adapter *padapter,
	u8 sta_num, u16 new_pkt_len);
#endif /* _RTL_WFO_IPC_H_ */
