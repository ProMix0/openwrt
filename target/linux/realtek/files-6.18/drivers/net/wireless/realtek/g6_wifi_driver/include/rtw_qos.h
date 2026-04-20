/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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


#ifndef _RTW_QOS_H_
#define _RTW_QOS_H_

#define DRV_CFG_UAPSD_VO 	BIT0
#define DRV_CFG_UAPSD_VI 	BIT1
#define DRV_CFG_UAPSD_BK 	BIT2
#define DRV_CFG_UAPSD_BE 	BIT3

#define WMM_IE_UAPSD_VO 	BIT0
#define WMM_IE_UAPSD_VI 	BIT1
#define WMM_IE_UAPSD_BK 	BIT2
#define WMM_IE_UAPSD_BE 	BIT3

#define WMM_TID0 	BIT0
#define WMM_TID1 	BIT1
#define WMM_TID2 	BIT2
#define WMM_TID3 	BIT3
#define WMM_TID4 	BIT4
#define WMM_TID5 	BIT5
#define WMM_TID6 	BIT6
#define WMM_TID7 	BIT7

#define AP_SUPPORTED_UAPSD BIT7
/* TC = Traffic Category,  TID0~7 represents TC */
#define BIT_MASK_TID_TC 0xff
/* TS = Traffic Stream,  TID8~15 represents TS */
#define BIT_MASK_TID_TS 0xff00
#define ALL_TID_TC_SUPPORTED_UAPSD 0xff
#define DSCP_TABLE_SIZE 64

struct	qos_priv	{

	unsigned int	  qos_option;	/* bit mask option: u-apsd, s-apsd, ts, block ack...		 */
	u8 uapsd_enable;

#ifdef CONFIG_WMMPS_STA
	/* uapsd (unscheduled automatic power-save delivery) = a kind of wmmps */
	u8 uapsd_max_sp_len;
	/* declare uapsd_tid as a bitmap for the uapsd setting of TID 0~15 */
	u16 uapsd_tid;
	/* declare uapsd_tid_delivery_enabled as a bitmap for the delivery-enabled setting of TID 0~7 */
	u8 uapsd_tid_delivery_enabled;
	/* declare uapsd_tid_trigger_enabled as a bitmap for the trigger-enabled setting of TID 0~7 */
	u8 uapsd_tid_trigger_enabled;
	/* declare uapsd_ap_supported to record whether the connected ap  supports uapsd or not */
	u8 uapsd_ap_supported;
#endif /* CONFIG_WMMPS_STA */	

};

void rtw_init_dscp_table(_adapter *padapter);
// Robust AV Streaming

#ifdef CONFIG_QOS_MANAGEMENT

#define SET_EXT_CAPABILITY_ELE_SCS(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE(((u8 *)(_pEleStart))+6, 6, 1, (_val))

#define GET_EXT_CAPABILITY_ELE_SCS(_pEleStart) \
	LE_BITS_TO_1BYTE((_pEleStart) + 6, 6, 1)

#define SET_EXT_CAPABILITY_ELE_MSCS(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE(((u8 *)(_pEleStart))+10, 5, 1, (_val))

#define GET_EXT_CAPABILITY_ELE_MSCS(_pEleStart) \
	LE_BITS_TO_1BYTE((_pEleStart) + 10, 5, 1)

enum _ip_protocol {
	IPP_TCP = 0x06,
	IPP_UDP = 0x11,
	_IPP_MAX = 0xFF,
};

struct rtw_phl2_tclas_element_data_s {
	u8 type;
	union {
		struct rtw_phl_tclas_type0 t0;
		struct rtw_phl_tclas_type1 t1;
		struct rtw_phl_tclas_type2 t2;
		struct rtw_phl_tclas_type3 t3;
		struct rtw_phl_tclas_type4 t4;
		struct rtw_phl_tclas_type5 t5;
		struct rtw_phl_tclas_type6 t6;
		struct rtw_phl_tclas_type7 t7;
		struct rtw_phl_tclas_type8 t8;
		struct rtw_phl_tclas_type9 t9;
		struct rtw_phl_tclas_type10 t10;
	} u;
};

struct rtw_phl2_mscs_desc_data_s {
	u8 up;
	struct hlist_node hnode;
	u8 up_control_bmp;
	u8 up_control_limit;
	u32 stream_timeout;
	systime last_updated;
	u8 tclas_mask_elem_len;
	struct rtw_phl2_tclas_element_data_s tclas_mask_elem[MAX_TCLAS_ELE];
};

static inline void rtw_phl2_mscs_desc_data_free(struct rtw_phl2_mscs_desc_data_s *s) {
	rtw_mfree(s, sizeof(struct rtw_phl2_mscs_desc_data_s));
}

static inline void rtw_phl2_mscs_desc_data_copy(struct rtw_phl2_mscs_desc_data_s *dst, const struct rtw_phl2_mscs_desc_data_s *src) {
	memcpy(dst, src, sizeof(struct rtw_phl2_mscs_desc_data_s));
}

static inline struct rtw_phl2_mscs_desc_data_s *rtw_phl2_mscs_desc_data_copynew(const struct rtw_phl2_mscs_desc_data_s *s) {
	struct rtw_phl2_mscs_desc_data_s *res = (struct rtw_phl2_mscs_desc_data_s *)rtw_malloc(sizeof(struct rtw_phl2_mscs_desc_data_s));
	if(res)
		rtw_phl2_mscs_desc_data_copy(res, s);
	return res;
}

struct rtw_mscs_desc_s {
	u8 req_type;
	u16 mscs_status;
	struct rtw_phl2_mscs_desc_data_s template;
};

struct rtw_phl2_scs_desc_data_s {
	u8 up;
	/* TCLAS Elements */
	u8 tclas_len;
	struct rtw_phl2_tclas_element_data_s tclas[MAX_TCLAS_ELE];
	/* TCLAS Processing Element */
	u8 processing;
};

static inline void rtw_phl2_scs_desc_data_copy(struct rtw_phl2_scs_desc_data_s *dst, const struct rtw_phl2_scs_desc_data_s *src) {
	memcpy(dst, src, sizeof(struct rtw_phl2_scs_desc_data_s));
}

static inline struct rtw_phl2_scs_desc_data_s *rtw_phl2_scs_desc_data_copynew(const struct rtw_phl2_scs_desc_data_s *s) {
	struct rtw_phl2_scs_desc_data_s *res = (struct rtw_phl2_scs_desc_data_s *)rtw_malloc(sizeof(struct rtw_phl2_scs_desc_data_s));
	rtw_phl2_scs_desc_data_copy(res, s);
	return res;
}

struct rtw_scs_desc_s {
	u8 req_type;
	u8 scsid;
	/* Intra-Access Category Priority Element */
	struct rtw_phl_intra_ac_priority iacp;
	struct rtw_phl2_scs_desc_data_s rule;
};

enum _mscs_rec_ret {
	QOS_MSCS_REC_UPDATE,
	QOS_MSCS_REC_NEW,
	QOS_MSCS_REC_IGNORE,
};
#define QOS_MSCS_MIN_REC_PKTLEN 500
#define QOS_MSCS_TBL_BITL 8
#define QOS_MSCS_TBL_SZ (1 << QOS_MSCS_TBL_BITL)

u16 mscs_parse_descriptor(struct rtw_mscs_desc_s *data, u8 *ele_start, u16 ele_len);
u16 scs_parse_descriptor(struct rtw_scs_desc_s *data, u8 *ele_start, u16 ele_len);
void mscs_htbl_destroy_table(struct hlist_head *htbl, u8 htbl_bitl);
void mscs_htbl_init_table(struct hlist_head *htbl, u8 htbl_bitl);
u8 mscs_rec_pkt(struct hlist_head *htbl, u8 htbl_bitl, const struct rtw_phl2_mscs_desc_data_s *template, const struct sk_buff *pkt1);
u8 mscs_retrieve_priority(struct hlist_head *htbl, u8 htbl_bitl, const struct rtw_phl2_mscs_desc_data_s *template, const struct sk_buff *pkt2);
u8 scs_match_rule(const struct rtw_phl2_scs_desc_data_s *rule, const struct sk_buff *pkt2);

static inline void mscs_htbl_reinit_table(struct hlist_head *htbl, u8 htbl_bitl) {
	mscs_htbl_destroy_table(htbl, htbl_bitl);
	mscs_htbl_init_table(htbl, htbl_bitl);
}
#endif

#define SET_EXT_CAPABILITY_ELE_QOS_MAP(_pEleStart, _val) \
	SET_BITS_TO_LE_1BYTE(((u8 *)(_pEleStart))+4, 0, 1, _val)

#define GET_EXT_CAPABILITY_ELE_QOS_MAP(_pEleStart) \
	LE_BITS_TO_1BYTE((_pEleStart) + 4, 0, 1)

unsigned char *rtw_qos_append_qos_map_ie(unsigned char *pframe, _adapter *padapter, u32 *len);


enum _rtw_qos_action_type {
  qos_action_ADDTS_Request,
  qos_action_ADDTS_Response,
  qos_action_DELTS,
  qos_action_Schedule,
  qos_action_QoS_Map_Configure,
  qos_action_ADDTS_Reserve_Request,
  qos_action_ADDTS_Reserve_Response,
};

struct qos_txmgmt {
	u8 raddr[6];
	u8 qos_action;
};
u8 rtw_qos_issue_action(_adapter *padapter, struct qos_txmgmt *ptxmgmt);
u8 rtw_qos_issue_action_allsta(_adapter *padapter, struct qos_txmgmt ptxmgmt);


struct dscp_txmgmt {
	u8 raddr[6];
	u8 oui_subtyp;
	u8 dialog_token;
	u8 request_control;
	u8 qos_mgmt_elem_len;
	u8 *qos_mgmt_elem;
};

u8 rtw_dscp_issue_action(_adapter *padapter, struct dscp_txmgmt *ptxmgmt);
u8 rtw_core_vsp_qos_on_action(_adapter *padapter,  u8* frame_body, u32 frame_body_len, struct sta_info *psta);


#endif /* _RTL871X_QOS_H_ */
