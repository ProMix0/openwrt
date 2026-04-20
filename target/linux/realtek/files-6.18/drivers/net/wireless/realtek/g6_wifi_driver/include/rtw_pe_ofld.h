#ifndef _RTW_PE_OFLD_H_
#define _RTW_PE_OFLD_H_

#ifdef CONFIG_ETHER_PKT_AGG

#define RTW_PE_OFLD_OFF 0
#define RTW_PE_OFLD_TC_MODE 1
#define RTW_PE_OFLD_WMM_MODE 2
#define RTW_PE_OFLD_TC_MODE_AGG_STA_LMT 40
#define RTW_PE_OFLD_WMM_MODE_AGG_STA_LMT 10
#define RTW_PE_OFLD_OFF_AGG_STA_LMT 0

extern int rtk_fc_internal_amsdu_pe_offload_mc2uc(bool enable);

void rtw_init_pe_entry(struct dvobj_priv *dvobj);
u32 rtw_iterate_pe_entry(u32 index, u8 *buf, u32 len);
void rtw_get_pe_entry_agg_sta_lmt(u8 *buf, u32 len);
void rtw_get_pe_entry_num_entry(u8 *buf, u32 len);
void rtw_get_pe_entry_pe_ofld_mode(u8 *buf, u32 len);
u32 rtw_amsdu_pe_offload_mac_id_set(struct sta_info *psta);
void rtw_amsdu_pe_offload_mac_id_del(struct sta_info *psta);
void rtw_amsdu_pe_offload_mac_id_update(struct sta_info *psta);
void rtw_pe_ofld_mode_init(_adapter *padapter);
bool rtw_pe_ofld_used(_adapter *padapter);



#define RTW_PE_ENTRY_NUM	40

struct rtw_pe_ofld_entry {
	u8		used;
	u8		dev_id;
	u16		macid;
};

struct rtw_pe_ofld_entries {
	_lock	lock;
	bool	init;
	struct rtw_pe_ofld_entry entry[RTW_PE_ENTRY_NUM];
	u8 agg_sta_lmt;
	u8 num_entry;
	u8 pe_ofld_mode;
};

struct pe_ofld_pktinfo_t {
	u32 subframes; /* number of amsdu subframes in amsdu */
	u32 amsdu_bytes; /* length of amsdu included in this packet in byte */
	u32 sfhdr_bytes; /* sum of length of total amsdu subframe headers included in amsdu in byte */
	u32 msdu_bytes; /* sum of length of total msdus included in amsdu in byte */
	u32 pad_bytes; /* sum of length of total paddings included in amsdu in byte */
};

void rtw_pe_ofld_dbg_pkt_cnt(struct sk_buff *pkt, _nic_hdl pnetdev);
void rtw_pe_ofld_dbg_non_amsdu_cnt(struct sk_buff *pkt, _nic_hdl pnetdev);
u32 rtw_pe_ofld_parse_msdu_len(struct sk_buff *pkt);
void rtw_pe_ofld_set_sta_cnt_frame_sz_need_print_setting(_adapter *padapter, struct sk_buff *pkt);

#endif /* CONFIG_ETHER_PKT_AGG */

#if defined(CONFIG_ETHER_PKT_AGG) || defined(CONFIG_RTL_WIFI_OFFLOAD)
void rtl_wfo_fc_block_relay(struct net_device *dev, int block_relay);
void rtl_wfo_fc_mc2uc_enable(struct net_device *dev, int mc2uc_en);
int rtl_wfo_fc_nat25_callback(struct net_device *dev, struct sk_buff *skb, bool rx);
void rtl_wfo_fc_register_uc2mc_proc(void);
void rtl_wfo_fc_callback_hook_update(struct net_device *dev, int up);
#endif /* defined(CONFIG_ETHER_PKT_AGG) || defined(CONFIG_RTL_WIFI_OFFLOAD) */

#endif /* _RTW_PE_OFLD_H_ */

