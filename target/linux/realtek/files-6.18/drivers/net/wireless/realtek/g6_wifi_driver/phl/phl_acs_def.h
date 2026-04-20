/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#ifndef _PHL_ACS_DEF_H_
#define _PHL_ACS_DEF_H_

/* size of phl_acs_chnl_tbl which include 2G/5G/6G channel */
#define MAX_ACS_INFO 102
#define NHM_RPT_NUM 12

enum rtw_nhm_info_type {
	RTW_INFO_NHM_NOISE,
	RTW_INFO_NHM_CCA_RATIO,
	RTW_INFO_NHM_TX_RATIO,
	RTW_INFO_NHM_RATIO,
	RTW_INFO_NHM_IDLE_RATIO,
};

enum rtw_acs_info_type {
	RTW_ACS_INFO_BAND,
	RTW_ACS_INFO_CHANNEL,
	RTW_ACS_INFO_CLM_RATIO,
	RTW_ACS_INFO_NHM_PWR,
	RTW_ACS_INFO_NHM_CCA_RATIO,
	RTW_ACS_INFO_NHM_TX_RATIO,
	RTW_ACS_INFO_NHM_RATIO,
	RTW_ACS_INFO_NHM_IDLE_RATIO,
	RTW_ACS_INFO_NHM_RPT,
	RTW_ACS_INFO_CHAN_UTIL_CCA,
	RTW_ACS_INFO_CHAN_UTIL_TX,
	RTW_ACS_INFO_CLM_FA_RATIO,
};

struct rtw_acs_info_parm {
	u8 idx;
	enum rtw_acs_info_type type;
	void *info;
};

struct acs_mntr_parm {
	u16 mntr_time;
	bool nhm_incld_cca;
};

struct acs_mntr_rpt {
	u8 clm_ratio;
	u8 nhm_pwr;
	u8 nhm_cca_ratio;
	u8 nhm_tx_ratio;
	u8 nhm_ratio;
	u8 nhm_idle_ratio;
	u8 nhm_rpt[NHM_RPT_NUM];
	u16 nhm_tx_cnt;
	u16 nhm_cca_cnt;
	u16 nhm_idle_cnt;
	u8 clm_fa_ratio;
};

#endif /*_PHL_ACS_DEF_H_*/