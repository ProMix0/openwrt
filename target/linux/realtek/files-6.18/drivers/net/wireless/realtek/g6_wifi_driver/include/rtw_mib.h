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
#ifndef _RTW_MIB_H_
#define _RTW_MIB_H_

#define RTW_WIFI_MIB_VER_MAJOR "1"
#define RTW_WIFI_MIB_VER_MINOR "0"
#define RTW_WIFI_MIB_VERSION   RTW_WIFI_MIB_VER_MAJOR "." RTW_WIFI_MIB_VER_MINOR

/* flag of sta info */
#define STA_INFO_FLAG_AUTH_OPEN     			0x01
#define STA_INFO_FLAG_AUTH_WEP      			0x02
#define STA_INFO_FLAG_ASOC          			0x04
#define STA_INFO_FLAG_ASLEEP        			0x08

#define _NUM_SBWC			64
#define _NUM_GBWC			64

#define STBC_RX_EN BIT0
#define STBC_TX_EN BIT1

#ifdef CONFIG_24G_256QAM
#define VHT_2G_SURVEY		BIT0
#define VHT_2G_UPT_RA		BIT1
#define VHT_2G_ASOCRSP_IE	BIT2
#define VHT_2G_CHK_RX_RATE	BIT3
#endif /* CONFIG_24G_256QAM */

struct _SBWC_ENTRY {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned int tx_lmt;
	unsigned int rx_lmt;
};

struct _StaBandwidthControl {
	unsigned int count;
	struct _SBWC_ENTRY entry[_NUM_SBWC];
};

struct _GBWC_ENTRY {
	unsigned char mac[MAC_ADDR_LEN];
};

struct _GroupBandwidthControl {
	unsigned int count;
	struct _GBWC_ENTRY entry[_NUM_GBWC];
};

enum qos_prio { BK, BE, VI, VO};
struct _ParaRecord {
	unsigned int	aifsn;
	unsigned int	ecw_min;
	unsigned int	ecw_max;
	unsigned int	txop_limit;
};

struct wifi_mib_priv {
	unsigned char	rtw_mib_version[16];
	unsigned int	rtw_mib_size;
	unsigned int	func_off;
	unsigned int	func_off_prev;
	unsigned char	disable_protection;
	unsigned char	aggregation;
	unsigned char	iapp_enable;
	unsigned int	a4_enable;
	unsigned char	multiap_monitor_mode_disable;
	unsigned char	multiap_bss_type;
	unsigned char	multiap_profile;
	unsigned char	multiap_max_device_reached;
  	unsigned char	multiap_report_rcpi_threshold;
	unsigned char	multiap_report_rcpi_hysteris_margin;
	/* Steering Policy TLV parameters */
	unsigned char   multiap_steering_policy;
	unsigned char	multiap_cu_threshold;
	unsigned char   multiap_rcpi_steering_th;
	/*=================*/
	unsigned char   multiap_change_channel;
	unsigned char   multiap_dfs_ap_mib_channel;
	unsigned char   multiap_ext_cmd;
	unsigned char   multiap_report_fail_assoc;
	unsigned char	multiap_vlan_enable;
	unsigned int	multiap_vlan_id;
	unsigned char	multiap_controller_bh_rssi;
	unsigned char	max_tx_power;
	unsigned int	gbwcmode;
	unsigned int	gbwcthrd_tx;
	unsigned int	gbwcthrd_rx;
	struct _GroupBandwidthControl gbwcEntry;
	unsigned char	telco_selected;
	unsigned int	regdomain;
	unsigned int	dfs_regions;
	unsigned int	led_type;
	unsigned int	lifetime;
	unsigned int	tdma_off_thre;
	unsigned char	tx_retry_limit;
	unsigned int	manual_priority;/* default: 0xff is off */
	unsigned int	opmode;
	unsigned int	autorate;
	unsigned int	fixrate;
	unsigned int	deny_legacy;
	unsigned int	lgyEncRstrct;
	unsigned int	cts2self;
	unsigned int	rtscts;
	unsigned int	coexist;
	unsigned char	ampdu;
	unsigned char	amsdu;
	unsigned char	low_rate_agg_auto;
	unsigned char	crossband_enable;
	unsigned int	monitor_sta_enabled;
	unsigned int	txbf;
	unsigned int	txbfer;
	unsigned int	txbfee;
	unsigned char	txbf_state;
	unsigned char	txbf_apply;
	unsigned int	txbf_mu;
	unsigned char 	txbf_mu_state;
	unsigned char 	txbf_mu_apply;
	unsigned int	txbf_period;
	unsigned int 	txbf_mu_2ss;
	unsigned int 	txbf_mu_1ss;
	unsigned int 	txbf_tp_limit;
	unsigned int 	txbf_mu_amsdu;
	unsigned int 	txbf_csi_rate;
	unsigned char 	txbf_force_regrp;
	unsigned char 	txbf_offload;
	unsigned char 	txbf_auto_snd;
	unsigned char 	txbf_period_fw;
	unsigned char	roaming_switch;
	unsigned char	roaming_qos;
	unsigned char	fail_ratio;
	unsigned char	retry_ratio;
	unsigned char	RSSIThreshold;
	unsigned char	dfgw_mac[6];
	unsigned char	roaming_enable;
	unsigned int	roaming_start_time;
	unsigned char	roaming_rssi_th1;
	unsigned char	roaming_rssi_th2;
	unsigned int	roaming_wait_time;
	unsigned char	band;
	unsigned char   tpc_tx_power;
	unsigned char	rssi_dump;
	unsigned char	totaltp_dump;
	unsigned char	defer_tx_sched;
	unsigned char	defer_tx_cnt;
	unsigned int	defer_tx_tp;
	unsigned int    defertxtimeout;
	unsigned int	txforce;
	/* CONFIG_RTW_AP_BS_DECT */
	unsigned int    bs_detect;
	unsigned int    bs_color;
	unsigned int    bs_tp_thrd;
	/*=================*/
	/* bit0 (1): Dynamic DL+UL grouping
	   bit0 (1) + bit4 (16): Dynamic DL grouping only
	   bit0 (1) + bit5 (32): Dynamic UL grouping only

	   bit1 (2): UL_fix_mode
	   bit3 (8): DL_fix_mode

	   bit2 (4): UL_fix_mode_by_assoc
	   bit6 (64): DL_fix_mode_by_assoc
	*/
	unsigned char   ofdma_enable;
	unsigned char	ldpc;
	unsigned char	stbc; /* bit0: Rx; bit1: Tx */
	unsigned int	mirror_dump;
	unsigned char	mirror_txch;
	unsigned char   bsscolor;
	unsigned char   sr_enable;
	unsigned char   edcca_mode;
	unsigned char   bfrp_mode;
	unsigned char   txbf_mu_rssi_thres;
	unsigned char   txbf_force_ng_cb;
	unsigned char   su_trig_enable;
	unsigned char   en_timer_pfd;
	unsigned char   en_txrpt_pfd;
	unsigned char   set_timer_period;
	unsigned char   timer_period;
	unsigned char   timer_clr_period;

	unsigned char   guest_access;
	unsigned char	block_relay;
	unsigned char   mc2u_disable;
	unsigned char   disable_dfs;
	unsigned char	dfs_bypass_cac;
	unsigned int	dfs_bypass_cac_to;
	unsigned char	powerpercent;
	int             power_reference;
	unsigned char	power_limit;
	struct _StaBandwidthControl sbwcEntry;
	unsigned int    aclmode;
	unsigned int    aclnum;	/*add for H userland no error*/
	unsigned char	scan_backop;
	unsigned char	scan_backop_div;
	unsigned int	scan_backop_dur;
	unsigned int	qos_enable;
	unsigned int	force_qos;
	/*===client mode===*/
	unsigned char	bssid[MAC_ADDR_LEN];
	unsigned char	connect_ssid[32];
	unsigned short	connect_ssid_len;
	unsigned int	connect_ch;
	unsigned char	crossband_pathReady;
	unsigned char	crossband_assoc;
	unsigned char	crossband_prefer;
	unsigned char	acs;
	unsigned char	autoch_1611_enable;
	unsigned char	autoch_3664157_enable;
	/*=================*/
	unsigned int	amsdu_pps;/* PPS */
	unsigned char	dacs;
	unsigned char 	fw_ext_dyn_update;
	unsigned char	low_rssi_sta_block_switch;
	unsigned char	sta_asoc_rssi_th;
	unsigned char	sta_asoc_rssi_th_cnt;
	unsigned int	low_rssi_sta_block_time;
	unsigned char	hiddenAP;
	/* setting value (not operating value) */
	unsigned char   set_channel;
	unsigned char	set_bwmode;
	unsigned char   set_ch_offset;
	/*=================*/
	unsigned char	mc2u_flood_ctrl;
	unsigned char	mc2u_ipv6_logo;
	unsigned short	mc2u_aging_time;
	unsigned int	txq_limit;
	unsigned char 	force_fw_tx;
	unsigned char 	rssi_ru_dump;
	unsigned char	ther_dm;
	unsigned char	dbg;
	unsigned char	del_ther;
	unsigned char	ther_hi;
	unsigned char	ther_low;
	unsigned char	ther_max;
	unsigned int	func_off_by_thermal;
	unsigned int	func_off_by_other;
	struct _ParaRecord ap_manual_edca[4];
	unsigned int	bcnint;
	/* RTW_WKARD_CUSTOM_PWRLMT_EN */
	unsigned char   txpwr_lmt_index;
	/*=================*/
	/* CONFIG_TXSC_AMSDU_TCP_ACK_REFINE */
	unsigned char	tcp_ack_drop_dup;
	/*=================*/
	unsigned int	kick_icverr;
	unsigned char	trx_path;
	/* CMCC_AUTO_DETECTION */
	unsigned int	cmcc_test_mode;
	/*=================*/
	/* CONFIG_DELAY_MEASUREMENT */
	unsigned char	delay_measure;
	/*=================*/
	unsigned char	antdiv;
	/* DEBUG_PHL_RX */
	unsigned char	rx_reorder_disable;
	/*=================*/
	unsigned char	bcn_ignore_edcca;
	/* CONFIG_ETHER_PKT_AGG */
	unsigned char   agg_sta_lmt;
	unsigned char	pe_ofld_dbg;
	unsigned char	dup_skb_mode; /* 0: off, 1: auto, 2: manual */
	unsigned int	dup_skb_thr; /* skbs, cannot be 0 */
	unsigned char	deq_rstrct; /* 0: off, 1: restrict to dequeue only 1 skb */
	unsigned char 	pe_ofld_mode; /* 0: RTW_PE_OFLD_OFF, 1: RTW_PE_OFLD_TC_MODE, 2: RTW_PE_OFLD_WMM_MODE, otherwise: RTW_PE_OFLD_OFF*/
	/*=================*/
	unsigned char   force_sta_wake;
	/* CONFIG_RTL_WIFI_OFFLOAD */
	unsigned char nirx_rsc_step;
	unsigned char amsdu_rsc_step;
	unsigned char wfo_pe_rcvy;
	unsigned char	force_drv_tx;
	unsigned char   force_drv_rx;
	unsigned short	pe_twt_lmt;
	/*=================*/
	/* CONFIG_ETHER_PKT_AGG || CONFIG_RTL_WIFI_OFFLOAD */
	unsigned char fc_nat25_en;
	unsigned char fc_uc2mc_en;
	/*=================*/
	/* CONFIG_SET_SCAN_DENY_TIMER */
	unsigned char	scan_deny_bypass_mode; /* 0: disable, 1: allow, 2: force */
	/*=================*/

	/*===CONFIG_24G_256QAM===*/
	unsigned char	vht_proprietary;
	unsigned int	amsdu_tp_thresh;
	/* RECORD_ENV_STAINFO */
	unsigned char   record_env_stainfo;
	unsigned char wmm_edca_sta_thres;
	unsigned int wmm_edca_amsdu_tp_th; /* mbps */
	unsigned char ppe_thresh_ie_en;   /* 0: disable ppe threshold present ie, 1: enable ppe threshold present ie*/
	unsigned int maxTxFailCnt; /* Fast leave: MAX Tx fail packet count. */
	unsigned int minTxFailCnt; /* Fast leave: MIN Tx fail packet count; this value should be less than maxTxFailCnt. */
	unsigned int txFailSecThr; /* Fast leave: Threshold of Tx Fail Time. (in second) */
	unsigned int sta_expire_to; /* Fast leave: sta expire time. */

	unsigned char mscs;
	unsigned char scs;
	unsigned short	scan_duration;
	unsigned char	scan_dynamic;
	unsigned char qbss_ch_util;
	unsigned char 	fastcon_en;
	unsigned char 	rx_loopdetect_drop;
};
#endif//_RTW_MIB_H_

#ifdef CONFIG_RTW_AP_EXT_SUPPORT
int set_rtw_mib_default_tbl(_adapter *padapter, struct wifi_mib_priv *rtw_priv_mib);
void overwrite_rtw_mib_default_tbl(struct dvobj_priv *dvobj, struct wifi_mib_priv *pmibpriv);
void core_mib_version(_adapter *padapter, char *extra, u32 oper);
void core_mib_size(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_80211AX_HE
#ifdef CONFIG_RTW_TWT
void core_mib_twt_enable(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_fw_tx(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CONFIG_RTW_A4_STA
void core_mib_a4_enable(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CONFIG_RTW_MULTI_AP
void core_mib_multiap_monitor_mode_disable(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_change_channel(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_ext_cmd(_adapter *padapter, char *extra, u32 oper);
void core_mib_multiap_bss_type(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_ampdu(_adapter *padapter, char *extra, u32 oper);
void	core_mib_func_off(_adapter *padapter, char *extra, u32 oper);
#if defined(WIFI6_THER_CTRL)
void	core_mib_func_off_thermal(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_auto_rate(_adapter *padapter, char *extra, u32 oper);
void	core_mib_fix_rate(_adapter *padapter, char *extra, u32 oper);
void	core_mib_deny_legacy(_adapter *padapter, char *extra, u32 oper);
void	core_mib_amsdu(_adapter *padapter, char *extra, u32 oper);
void	core_mib_low_rate_agg_auto(_adapter *padapter, char *extra, u32 oper);
void	core_mib_rssi_dump(_adapter *padapter, char *extra, u32 oper);
void	core_mib_bsscolor(_adapter *padapter, char *extra, u32 oper);
void	core_mib_edcca_mode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_bfrp_mode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_txbf_mu_rssi_thres(_adapter *padapter, char *extra, u32 oper);
void	core_mib_txbf_force_ng_cb(_adapter *padapter, char *extra, u32 oper);
void	core_mib_su_trig_enable(_adapter *padapter, char *extra, u32 oper);
void	core_mib_txbf_auto_snd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_txbf_period_fw(_adapter *padapter, char *extra, u32 oper);
void	core_mib_en_timer_pfd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_en_txrpt_pfd(_adapter *padapter, char *extra, u32 oper);
void	core_mib_set_timer_period(_adapter *padapter, char *extra, u32 oper);
void	core_mib_timer_period(_adapter *padapter, char *extra, u32 oper);
void	core_mib_timer_clr_period(_adapter *padapter, char *extra, u32 oper);

#ifdef CONFIG_RTW_AP_BS_DECT
void    core_mib_bs_detect(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_txforce(_adapter *padapter, char *extra, u32 oper);
void	core_mib_band(_adapter *padapter, char *extra, u32 oper);
void	core_mib_block_relay(_adapter *padapter, char *extra, u32 oper);
void	core_mib_mc2u_disable(_adapter *padapter, char *extra, u32 oper);
void	core_mib_lifetime(_adapter *padapter, char *extra, u32 oper);
void	core_mib_tx_retry_limit(_adapter *padapter, char *extra, u32 oper);
void	core_mib_power_percent(_adapter *padapter, char *extra, u32 oper);
void	core_mib_power_ref(_adapter *padapter, char *extra, u32 oper);
void	core_mib_power_limit(_adapter *padapter, char *extra, u32 oper);
void	core_mib_aclmode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_aclnum(_adapter *padapter, char *extra, u32 oper);
#if defined(CONFIG_VW_REFINE) || defined(CONFIG_ONE_TXQ)
void	core_mib_tx_mode(_adapter *padapter, char *extra, u32 oper);
#endif
void	core_mib_max_tx_power(_adapter *padapter, char *extra, u32 oper);
void	core_mib_channel(_adapter *padapter, char *extra, u32 oper);
void	core_mib_use40M(_adapter *padapter, char *extra, u32 oper);
void	core_mib_2ndchoffset(_adapter *padapter, char *extra, u32 oper);
void	core_mib_encmode(_adapter *padapter, char *extra, u32 oper);
void	core_mib_psk_enable(_adapter *padapter, char *extra, u32 oper);
void	core_mib_wpa_cipher(_adapter *padapter, char *extra, u32 oper);
void	core_mib_wpa2_cipher(_adapter *padapter, char *extra, u32 oper);
void	core_mib_authtype(_adapter *padapter, char *extra, u32 oper);
void	core_mib_multiap_max_device_reached(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_RTW_MIRROR_DUMP
void	core_mib_mirror_dump(_adapter *padapter, char *extra, u32 oper);
void	core_mib_mirror_txch(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_tpc_tx_power(_adapter *padapter, char *extra, u32 oper);
void core_mib_hiddenAP(_adapter *padapter, char *extra, u32 oper);
void core_mib_mc2u_aging_time(_adapter *padapter, char *extra, u32 oper);
void core_mib_bcnint(_adapter *padapter, char *extra, u32 oper);
#ifdef RTW_WKARD_CUSTOM_PWRLMT_EN
void core_mib_set_txpwr_lmt_index(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CTC_QOS_DSCP
void core_mib_ctc_dscp(_adapter *padapter, char *extra, u32 oper);
#endif
void core_mib_atf(_adapter *padapter, char *extra, u32 oper);
void core_mib_fast_leave(_adapter *padapter, char *extra, u32 oper);
void core_mib_inactive_timeout(_adapter *padapter, char *extra, u32 oper);
void core_mib_mscs(_adapter *padapter, char *extra, u32 oper);
void core_mib_scs(_adapter *padapter, char *extra, u32 oper);
void core_mib_qbss_ch_util(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_RTW_MANUAL_EDCA
void core_mib_manual_edca(_adapter *padapter, char *extra, u32 oper);
void set_manual_edca_para(_adapter *padapter);
#endif
void core_mib_trx_path(_adapter *padapter, char *extra, u32 oper);

#ifdef CONFIG_DELAY_MEASUREMENT
void core_mib_delay_measure(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef DEBUG_PHL_RX
void core_mib_rx_reorder_disable(_adapter *padapter, char *extra, u32 oper);
#endif
#ifdef CONFIG_ETHER_PKT_AGG
void core_mib_pe_ofld_dbg(_adapter *padapter, char *extra, u32 oper);
void core_mib_pe_ofld_mode(_adapter *padapter, char *extra, u32 oper);
#endif /* CONFIG_ETHER_PKT_AGG */
void core_mib_force_sta_wake(_adapter *padapter, char *extra, u32 oper);
#ifdef CONFIG_RTL_WIFI_OFFLOAD
void rtl_wfo_mib_init(struct dvobj_priv *dvobj, _adapter *padapter);
void core_mib_force_drv_tx(_adapter *padapter, char *extra, u32 oper);
void core_mib_force_drv_rx(_adapter *padapter, char *extra, u32 oper);
#endif /* CONFIG_RTL_WIFI_OFFLOAD */
#endif

