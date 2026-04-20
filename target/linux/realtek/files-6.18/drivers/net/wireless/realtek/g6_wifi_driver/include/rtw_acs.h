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
#ifndef __RTW_ACS_H_
#define __RTW_ACS_H_


#ifdef CONFIG_RTW_ACS

#define KEEP_PREVIOS_ACS_RESULT_INTERVAL 30000 /* ms */
#define MAX_CHANNEL 165

struct acs_result {
	enum band_type band;
	u8 channel;
	u8 group_idx;
	u8 clm_ratio;
	s8 noise;
	u32 rx_count;
	s8 score;
	bool overlap;
	u8 nhm_rpt[NHM_RPT_NUM];
	u32 nhm_score;
	u16 group_score;
	u16 chan_util_cca;
	u16 chan_util_tx;
	u8 clm_fa_ratio;
};

struct acs_parm {
	u8 num;						/* total num of scan channel */
	u8 acs_idx[MAX_ACS_INFO]; 	/* mapping to phl_acs_chnl_tbl */
};

struct acs_priv {
	struct acs_parm parm;
	struct acs_result result[MAX_ACS_INFO];
	struct acs_result dacs_result[MAX_ACS_INFO];
	u8 best_channel;
	u32 max_nhm_score;
	u8 max_clm_fa;
	u8 use_nhm;
	systime last_acs_time;
	bool exist_clean_channel;
	bool overlap[MAX_CHANNEL + 1];
	u8 dacs_idx;
	u16 dacs_interval; /* 0: use default, otherwise: specific scan interval (s) between two channel */
	u8 scan_mode;	/* according to enum rtw_phl_scan_type, 0: passive scan, 1: active scan */
	u16 duration;	/* 0: use default, otherwise: specific scan time (ms) in a channel */
	u8 ch_list[MAX_ACS_INFO]; /* specific channel list for scan */
	u8 ch_list_num; /* 0: use default, otherwise: use ch_list */
	u8 candidate_ch[MAX_ACS_INFO]; /* specific candidate ch for switch */
	u8 candidate_ch_num; /* 0: use default, otherwise: use candidate_ch */
#if defined(CONFIG_RTW_MULTI_AP) && defined(CONFIG_NSB_MULTI_AP_EVENT)
	u8 switch_ch_triggered; /* 0: switch ch is not triggered, 1: switch ch is triggered */
#endif
};

void acs_parm_init(_adapter *adapter, u8 ch_num, struct phl_scan_channel *ch);
void acs_reset_info(_adapter *adapter);
void acs_select_clean_channel(_adapter	*adapter);
void rtw_select_clean_channel(_adapter *adapter);
void acs_change_bss_chbw(_adapter *adapter, u8 with_csa_ie, s8 csa_cnt);
bool is_acs_ban_channel(_adapter *adapter, u8 channel);
#ifdef CONFIG_RTW_DACS
void rtw_dacs_mnt_result(_adapter *adapter);
void rtw_dacs_mnt_trigger(_adapter *adapter);
#endif /* CONFIG_RTW_DACS */
#endif /* CONFIG_RTW_ACS */

#endif
