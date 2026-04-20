/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __HALRF_H__
#define __HALRF_H__

/*@--------------------------[Define] ---------------------------------------*/

/*H2C cmd ID*/
/*Class 8*/
/*Class 9*/
/*Class a*/
#define FWCMD_H2C_BACKUP_RFK 0
#define FWCMD_H2C_RELOAD_RFK 1
#define FWCMD_H2C_GET_MCCCH 2
#define FWCMD_H2C_DPK_OFFLOAD 3
#define FWCMD_H2C_IQK_OFFLOAD 4

/*@--------------------------[Enum]------------------------------------------*/
enum halrf_func_idx {
	RF00_PWR_TRK = 0,
	RF01_IQK = 1,
	RF02_LCK = 2,
	RF03_DPK = 3,
	RF04_TXGAPK = 4,
	RF05_DACK = 5,
	RF06_DPK_TRK = 6,
	RF07_2GBAND_SHIFT = 7,
	RF08_RXDCK = 8,
	RF09_RFK = 9,
	RF10_RF_INIT = 10,
	RF11_RF_POWER = 11,
	RF12_RXGAINK = 12,
	RF13_THER_TRIM = 13,
	RF14_PABIAS_TRIM = 14,
	RF15_TSSI_TRIM = 15,
	RF16_PSD = 16,
	RF17_TSSI_TRK = 17,
	RF18_XTAL_TRK = 18,
	RF19_TX_SHAPE = 19,
	RF20_OP5K_TRK = 20,
	RF21_OP5K = 21,
	RF22_TPE_CTRL = 22,
	RF23_RXDCK_TRK = 23,
	RF24_EDPK = 24,
	RF25_TXALCTK = 25
};

enum halrf_rf_mode {
	RF_SHUT_DOWN = 0x0,
	RF_STANDBY = 0x1,
	RF_TX = 0x2,
	RF_RX = 0x3,
	RF_TXIQK = 0x4,
	RF_DPK = 0x5,
	RF_RXK1 = 0x6,
	RF_RXK2 = 0x7
};

enum halrf_rfe_src_sel {
	HALRF_PAPE_RFM		= 0,
	HALRF_GNT_BT_INV	= 1,
	HALRF_LNA0N		= 2,
	HALRF_LNAON_RFM		= 3,
	HALRF_TRSW_RFM		= 4,
	HALRF_TRSW_RFM_B	= 5,
	HALRF_GNT_BT		= 6,
	HALRF_ZERO		= 7,
	HALRF_ANTSEL_0		= 8,
	HALRF_ANTSEL_1		= 9,
	HALRF_ANTSEL_2		= 0xa,
	HALRF_ANTSEL_3		= 0xb,
	HALRF_ANTSEL_4		= 0xc,
	HALRF_ANTSEL_5		= 0xd,
	HALRF_ANTSEL_6		= 0xe,
	HALRF_ANTSEL_7		= 0xf
};

/*@=[HALRF supportability]=======================================*/
enum halrf_ability {
	HAL_RF_TX_PWR_TRACK = BIT(RF00_PWR_TRK),
	HAL_RF_IQK = BIT(RF01_IQK),
	HAL_RF_LCK = BIT(RF02_LCK),
	HAL_RF_DPK = BIT(RF03_DPK),
	HAL_RF_TXGAPK = BIT(RF04_TXGAPK),
	HAL_RF_DACK = BIT(RF05_DACK),
	HAL_RF_DPK_TRACK = BIT(RF06_DPK_TRK),
	HAL_2GBAND_SHIFT = BIT(RF07_2GBAND_SHIFT),
	HAL_RF_RXDCK = BIT(RF08_RXDCK),
	HAL_RF_RXGAINK = BIT(RF12_RXGAINK),
	HAL_RF_THER_TRIM = BIT(RF13_THER_TRIM),
	HAL_RF_PABIAS_TRIM = BIT(RF14_PABIAS_TRIM),
	HAL_RF_TSSI_TRIM = BIT(RF15_TSSI_TRIM),
	HAL_RF_TSSI_TRK = BIT(RF17_TSSI_TRK),
	HAL_RF_XTAL_TRACK = BIT(RF18_XTAL_TRK),
	HAL_RF_TX_SHAPE = BIT(RF19_TX_SHAPE),
	HAL_RF_OP5K_TRACK = BIT(RF20_OP5K_TRK),
	HAL_RF_OP5K = BIT(RF21_OP5K),
	HAL_RF_TPE_CTRL = BIT(RF22_TPE_CTRL),
	HAL_RF_RXDCK_TRACK = BIT(RF23_RXDCK_TRK),
	HAL_RF_EDPK = BIT(RF24_EDPK),
	HAL_RF_TXALCTK = BIT(RF25_TXALCTK)
};

/*@=[HALRF Debug Component]=====================================*/
enum halrf_dbg_comp {
	DBG_RF_TX_PWR_TRACK = BIT(RF00_PWR_TRK),
	DBG_RF_IQK = BIT(RF01_IQK),
	DBG_RF_LCK = BIT(RF02_LCK),
	DBG_RF_DPK = BIT(RF03_DPK),
	DBG_RF_TXGAPK = BIT(RF04_TXGAPK),
	DBG_RF_DACK = BIT(RF05_DACK),
	DBG_RF_DPK_TRACK = BIT(RF06_DPK_TRK),
	DBG_RF_RXDCK = BIT(RF08_RXDCK),
	DBG_RF_RFK = BIT(RF09_RFK),
	DBG_RF_INIT = BIT(RF10_RF_INIT),
	DBG_RF_POWER = BIT(RF11_RF_POWER),
	DBG_RF_RXGAINK = BIT(RF12_RXGAINK),
	DBG_RF_THER_TRIM = BIT(RF13_THER_TRIM),
	DBG_RF_PABIAS_TRIM = BIT(RF14_PABIAS_TRIM),
	DBG_RF_TSSI_TRIM = BIT(RF15_TSSI_TRIM),
	DBG_RF_PSD = BIT(RF16_PSD),
	DBG_RF_XTAL_TRACK = BIT(RF18_XTAL_TRK),
	DBG_RF_OP5K_TRACK = BIT(RF20_OP5K_TRK),
	DBG_RF_OP5K = BIT(RF21_OP5K),
	DBG_RF_FW = BIT(28),
	DBG_RF_MP = BIT(29),
	DBG_RF_TMP = BIT(30),
	DBG_RF_CHK = BIT(31),
	DBG_RF_TXALCTK = BIT(RF25_TXALCTK)
};

 
/*@--------------------------[Structure]-------------------------------------*/

struct rfk_location {
	enum band_type cur_band;
	enum channel_width cur_bw;
	u8 cur_ch;
};

struct halrf_fem_info {
	u8 elna_2g;		/*@with 2G eLNA  NO/Yes = 0/1*/
	u8 elna_5g;		/*@with 5G eLNA  NO/Yes = 0/1*/
	u8 elna_6g;		/*@with 6G eLNA  NO/Yes = 0/1*/
	u8 epa_2g;		/*@with 2G ePA    NO/Yes = 0/1*/
	u8 epa_5g;		/*@with 5G ePA    NO/Yes = 0/1*/
	u8 epa_6g;		/*@with 6G ePA    NO/Yes = 0/1*/
};

#define OP5K_RESET_CNT_DATA	16
#define OP5K_RESET_CNT_ZERO_IDX	2
#define	OP5K_THERMAL_NUM	2
#define	OP5K_AVG_THERMAL_NUM	2
#define OP5K_THER_THRESHOLD	6

struct halrf_op5k_info {
	u32 rst_cnt[MAX_RF_PATH][OP5K_RESET_CNT_DATA];
	u32 rst_cnt_zero[MAX_RF_PATH][OP5K_RESET_CNT_ZERO_IDX];
	s32 rst_cnt_final[MAX_RF_PATH];
	u32 op5k_backup[MAX_RF_PATH];
	u32 op5k_default[MAX_RF_PATH];
	u8 thermal_base_op5k[MAX_RF_PATH];
	u8 thermal_op5k[MAX_RF_PATH];
	u8 thermal_op5k_avg[MAX_RF_PATH][OP5K_THERMAL_NUM];
	u8 thermal_op5k_avg_index;
	bool op5k_progress;
	bool op5k_ther_chk;
	bool op5k_set_ch_chk;
	u8 record_bw;
};

#if 1 /* all rf operation usage (header) */

/* clang-format on */
#define RF_PATH_MAX_NUM (8)
#define RF_TASK_RECORD_MAX_TIMES (16)
#define RF_BACKUP_MAC_REG_MAX_NUM (16)
#define RF_BACKUP_BB_REG_MAX_NUM (16)
#define RF_BACKUP_RF_REG_MAX_NUM (16)
#define RF_BACKUP_KIP_REG_MAX_NUM (16)


struct halrf_iqk_ops {
	u8 (*iqk_kpath)(struct rf_info *rf, enum phl_phy_idx phy_idx);	
	bool (*iqk_mcc_page_sel)(struct rf_info *rf, enum phl_phy_idx phy,  u8 path);
	void (*iqk_get_ch_info)(struct rf_info *rf, enum phl_phy_idx phy,  u8 path);	
	void (*iqk_preset)(struct rf_info *rf, u8 path);	
	void (*iqk_macbb_setting)(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
	void (*iqk_start_iqk)(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);
	void (*iqk_restore)(struct rf_info *rf, u8 path);
	void (*iqk_afebb_restore)(struct rf_info *rf, enum phl_phy_idx phy_idx, u8 path);	
};

struct rfk_iqk_info {
	struct halrf_iqk_ops *rf_iqk_ops;
	u8 rf_max_path_num;
	u32 rf_iqk_version;
	u8 rf_iqk_ch_num;
	u8 rf_iqk_path_num;

	const u32 *backup_mac_reg;
	u32 backup_mac_reg_num;
	const u32 *backup_bb_reg;
	u32 backup_bb_reg_num;
	const u32 *backup_rf_reg;
	u32 backup_rf_reg_num;
};
/* clang-format off */
#endif /* all rf operation usage (header) */

#ifdef HALRF_CONFIG_FW_IO_OFLD_SUPPORT
struct halrf_fw_offload {
	enum rtw_mac_src_cmd_ofld src;
	enum rtw_mac_cmd_type_ofld type;
	u8 lc;
	enum rtw_mac_rf_path rf_path;
	u16 offset;
	u16 id;
	u32 value;
	u32 mask;
};
#endif

struct halrf_rx_dck_info {
	bool is_afe;
	bool is_rxdck_track_en;
	struct rfk_location loc[KPATH]; /*max RF path*/
	u32 rxdck_time;
	bool is_auto_res;
	u8 ther_rxdck[KPATH];
	u8 rek_cnt[KPATH];
};

struct halrf_mcc_info {
	u8 ch[2];  
	u8 band[2];  
	u8 table_idx;  
	
	bool is_init;
};

struct halrf_txa_lctk_info {
	u32 version;
	u32 txalctk_time;
};

struct rf_info {
	struct rtw_phl_com_t	*phl_com;
	struct rtw_hal_com_t	*hal_com;
	/*[Common Info]*/
	u32			ic_type;
	u8			num_rf_path;
	u16 		sub_did;
	bool		use_sub_did;
	/*[System Info]*/
	bool			rf_init_ready;
	u32			rf_sys_up_time;
	bool			rf_watchdog_en;
	bool			rf_ic_api_en;
	/*[DM Info]*/
	u32			support_ability;
	u32			hw_rf_ability;
	u32			manual_support_ability;
	/*[FW Info]*/
	u32			fw_dbg_component;
	/*[Drv Dbg Info]*/
	u32			dbg_component;
	u8			cmn_dbg_msg_period;
	u8			cmn_dbg_msg_cnt;
	/*[BTC / RFK Info ]*/
	bool 			rfk_is_processing;
	bool			is_bt_iqk_timeout;
	bool			is_chl_rfk;
	u32			rfk_total_time;
	/*[initial]*/
	u8 		pre_rxbb_bw[KPATH];
	/*[TSSI Info]*/
	bool		is_tssi_mode[KPATH]; /*S0/S1*/
	u8		tssi_slope_type[KPATH];
	/*[Thermal]*/
	bool		is_thermal_trigger;
	u8		cur_ther_s0;
	u8		cur_ther_s1;
	/*LCK*/
	u8		lck_ther_s0;
	u8		lck_ther_s1;
	/*[Do Coex]*/
	bool		is_coex;
	/*[watchdog]*/
	bool		is_watchdog_stop;
	/*[thermal rek indictor]*/	
	bool rfk_do_thr_rek;
	/*reg check*/
	u32	rfk_reg[KIP_REG];
	u32	rfc_reg[2][10];
	u32	rfk_check_fail_count;
	/*fast channel switch*/
	u8	ther_init;
	u32	fcs_rfk_ok_map;
	u8	pre_chl_idx;
	u8	pre_ther_idx;
	/* [Check NCTL Done status Read Times] */
	u32 nctl_ck_times[2];  /* 0xbff8 0x80fc*/
	u32	fw_ofld_enable;
	u32 init_rf_reg_time;
	u32 set_ch_bw_time;
	_os_mutex rf_lock;
	u32 fw_ofld_start;

	/*@=== [HALRF Structure] ============================================*/
	struct halrf_pwr_track_info	pwr_track;
	struct halrf_tssi_info		tssi;
	struct halrf_xtal_info		xtal_track;
	struct halrf_iqk_info		iqk;
	struct halrf_dpk_info	dpk;
	struct halrf_rx_dck_info rx_dck;
	struct halrf_dack_info	dack;
	struct halrf_gapk_info	gapk;
	struct halrf_pwr_info	pwr_info;
	struct halrf_radio_info	radio_info;
	struct halrf_fem_info	fem;
	struct rf_dbg_cmd_info	rf_dbg_cmd_i;
	struct halrf_kfree_info	kfree_info;
	struct halrf_psd_data	psd;
	struct rfk_location		iqk_loc[2];	/*S0/S1*/
	struct rfk_location		dpk_loc[2];	/*S0/S1*/
	struct rfk_location		gapk_loc[2];	/*S0/S1*/
	struct rfk_iqk_info	*rfk_iqk_info;
	struct halrf_op5k_info op5k_info;
	struct halrf_txa_lctk_info txa_lctk;
#ifdef HALRF_CONFIG_FW_IO_OFLD_SUPPORT
	struct halrf_fw_offload fwofld;
#endif
	struct halrf_mcc_info mcc_info;
};

/*@--------------------------[Prptotype]-------------------------------------*/

/**************halrf.c**************/

void halrf_inn_watchdog(void *rf_void);
//enum rtw_hal_status halrf_inn_chl_rfk_trigger(void *rf_void, enum phl_phy_idx phy_idx, enum rfk_tri_type rfk_tri_typ);
enum rtw_hal_status halrf_inn_iqk_trigger(void *rf_void, enum phl_phy_idx phy_idx, bool force);
enum rtw_hal_status halrf_inn_dpk_trigger(void *rf_void, enum phl_phy_idx phy_idx, bool force);
enum rtw_hal_status halrf_inn_tssi_trigger(void *rf_void, enum phl_phy_idx phy_idx, bool hwtx_en);

void halrf_inn_do_tssi_scan(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_inn_tssi_enable(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_inn_tssi_disable(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_inn_dpk_switch(void *rf_void, bool is_enable);
void halrf_inn_lo_test(void *rf_void, bool is_on, enum rf_path path);
s16 halrf_inn_get_power(void *rf_void,
	u8 rf_path, u16 rate, u8 dcm, u8 offset, u8 bandwidth,
	u8 beamforming, u8 channel);
u8 halrf_inn_get_thermal(void *rf_void, u8 rf_path);
u32 halrf_inn_get_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path);
s32 halrf_inn_get_online_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path, s32 dbm, s32 puot);
void halrf_inn_set_tssi_de_for_tx_verify(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de, u8 path);
void halrf_inn_set_tssi_de_offset(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de_offset, u8 path);
void halrf_inn_set_tssi_avg_mp(void *rf_void,
					enum phl_phy_idx phy_idx, s32 xdbm);
void halrf_inn_set_rx_gain_offset_for_rx_verify(void *rf_void,
					enum phl_phy_idx phy,
					s8 rx_gain_offset, u8 path);
void halrf_inn_set_power_track(void *rf_void, enum phl_phy_idx phy_idx, u8 value);
u8 halrf_inn_get_power_track(void *rf_void);
void halrf_inn_tssi_get_efuse_ex(void *rf_void, enum phl_phy_idx phy_idx);
bool halrf_inn_tssi_check_efuse_data(void *rf_void, enum phl_phy_idx phy_idx);
void halrf_inn_bf_config_rf(void *rf_void);

/**************halrf_init.c**************/

//enum rtw_hal_status halrf_inn_dm_init(void *rf_void);

/**************halrf_hw_cfg.c**************/

void halrf_inn_config_rf_parameter(void *rf_void, enum phl_phy_idx phy);

/*******************************************/

//bool halrf_inn_get_efuse_info(void *rf_void, u8 *efuse_map,
//		       enum rtw_efuse_info id, void *value, u32 length,
//		       u8 autoload_status);
bool halrf_inn_set_dbcc(void *rf_void, bool dbcc_en);
void halrf_inn_wlan_tx_power_control(void *rf_void, enum phl_phy_idx phy,
	enum phl_pwr_ctrl pwr_ctrl_idx, u32 tx_power_val, bool enable);
bool halrf_inn_wl_tx_power_control(void *rf_void, u32 tx_power_val);
enum rtw_hal_status halrf_inn_dpk_tracking(void *rf_void);
u8 halrf_inn_get_default_rfe_type(void *rf_void);
u8 halrf_inn_get_default_xtal(void *rf_void);
void halrf_inn_psd_init(void *rf_void, enum phl_phy_idx phy,
			u8 path, u8 iq_path, u32 avg, u32 fft);
void halrf_inn_psd_restore(void *rf_void, enum phl_phy_idx phy);
u32 halrf_inn_psd_get_point_data(void *rf_void, enum phl_phy_idx phy, s32 point);
void halrf_inn_psd_query(void *rf_void, enum phl_phy_idx phy,
			u32 point, u32 start_point, u32 stop_point, u32 *outbuf);
void halrf_inn_config_radio_to_fw(void *rf_void);

void halrf_inn_disconnect_notify(void *rf_void, struct rtw_chan_def *chandef );
bool  halrf_inn_check_mcc_ch(void *rf_void, struct rtw_chan_def *chandef );
//void halrf_inn_ctrl_bw_ch(void *rf_void, enum phl_phy_idx phy, u8 central_ch,
//				enum band_type band, enum channel_width bw);
void halrf_inn_wifi_event_notify(void *rf_void,
			enum phl_msg_evt_id event, enum phl_phy_idx phy_idx);
void halrf_inn_2g_rxant(void *rf_void, enum halrf_ant ant);
s8 halrf_inn_get_ther_protected_threshold(void *rf_void);
void halrf_inn_tssi_set_efuse_to_de(void *rf_void,
					enum phl_phy_idx phy_idx);
void halrf_inn_tssi_scan_ch(void *rf_void, enum rf_path path);
void halrf_inn_syn1_onoff(void *rf_void, enum phl_phy_idx phy, u8 path, bool syn1_turn_on);
u32 halrf_inn_test_event_trigger(void *rf_void,
		enum phl_phy_idx phy, enum halrf_event_idx idx, enum halrf_event_func func);
void halrf_inn_set_ant_main_or_aux(void *rf_void, enum rf_path path, bool main);

#endif

