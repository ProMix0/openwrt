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
#include "halrf_precomp.h"

/**************halrf.c**************/

void halrf_watchdog(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;

#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_watchdog(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

#if 0
enum rtw_hal_status halrf_chl_rfk_trigger(void *rf_void,
			   enum phl_phy_idx phy_idx,
			   enum rfk_tri_type rfk_tri_typ)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	halrf_inn_chl_rfk_trigger(rf, phy_idx, rfk_tri_typ);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}
#endif

enum rtw_hal_status halrf_iqk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_iqk_trigger(rf, phy_idx, force);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status halrf_dpk_trigger(void *rf_void,
		       enum phl_phy_idx phy_idx,
		       bool force)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_dpk_trigger(rf, phy_idx, force);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return RTW_HAL_STATUS_SUCCESS;
}

enum rtw_hal_status halrf_tssi_trigger(void *rf_void, enum phl_phy_idx phy_idx, bool hwtx_en)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_trigger(rf, phy_idx, hwtx_en);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return RTW_HAL_STATUS_SUCCESS;
}

void halrf_do_tssi_scan(void *rf_void, enum phl_phy_idx phy_idx)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_do_tssi_scan(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_tssi_enable(void *rf_void, enum phl_phy_idx phy_idx)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_enable(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_tssi_disable(void *rf_void, enum phl_phy_idx phy_idx)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_disable(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_dpk_switch(void *rf_void, bool is_enable)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_dpk_switch(rf, is_enable);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_lo_test(void *rf_void, bool is_on, enum rf_path path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_lo_test(rf, is_on, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

s16 halrf_get_power(void *rf_void,
	u8 rf_path, u16 rate, u8 dcm, u8 offset, u8 bandwidth,
	u8 beamforming, u8 channel)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	s16 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_power(rf, rf_path, rate, dcm, offset, bandwidth, beamforming, channel);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

u8 halrf_get_thermal(void *rf_void, u8 rf_path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u8 val;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_thermal(rf, rf_path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

u32 halrf_get_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u32 val;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_tssi_de(rf, phy_idx, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

s32 halrf_get_online_tssi_de(void *rf_void, enum phl_phy_idx phy_idx, u8 path, s32 dbm, s32 puot)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	s32 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_online_tssi_de(rf, phy_idx, path, dbm, puot);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_set_tssi_de_for_tx_verify(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de, u8 path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_tssi_de_for_tx_verify(rf, phy_idx, tssi_de, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_set_tssi_de_offset(void *rf_void, enum phl_phy_idx phy_idx, u32 tssi_de_offset, u8 path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_tssi_de_offset(rf, phy_idx, tssi_de_offset, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_set_tssi_avg_mp(void *rf_void, enum phl_phy_idx phy_idx, s32 xdbm)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_tssi_avg_mp(rf, phy_idx, xdbm);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_set_rx_gain_offset_for_rx_verify(void *rf_void, enum phl_phy_idx phy,
					s8 rx_gain_offset, u8 path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_rx_gain_offset_for_rx_verify(rf, phy,	rx_gain_offset, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_set_power_track(void *rf_void, enum phl_phy_idx phy_idx, u8 value)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_power_track(rf, phy_idx, value);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

u8 halrf_get_power_track(void *rf_void)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u8 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_power_track(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_tssi_get_efuse_ex(void *rf_void, enum phl_phy_idx phy_idx)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_get_efuse_ex(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

bool halrf_tssi_check_efuse_data(void *rf_void, enum phl_phy_idx phy_idx)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_tssi_check_efuse_data(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_bf_config_rf(void *rf_void)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_bf_config_rf(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

/**************halrf_init.c**************/
#if 0
enum rtw_hal_status halrf_dm_init(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;
	enum rtw_hal_status val;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;

	val = halrf_inn_dm_init(rf);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
	
	return val;
}
#endif
/**************halrf_hw_cfg.c**************/

void halrf_config_rf_parameter(void *rf_void, enum phl_phy_idx phy)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_config_rf_parameter(rf, phy);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

/*******************************************/
#if 0
bool halrf_get_efuse_info(void *rf_void, u8 *efuse_map,
		       enum rtw_efuse_info id, void *value, u32 length,
		       u8 autoload_status)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;
	bool val;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
	
	val = halrf_inn_get_efuse_info(rf, efuse_map, id, value, length, autoload_status);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);

	return val;
}
#endif

bool halrf_set_dbcc(void *rf_void, bool dbcc_en)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_set_dbcc(rf, dbcc_en);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_wlan_tx_power_control(void *rf_void, enum phl_phy_idx phy,
	enum phl_pwr_ctrl pwr_ctrl_idx, u32 tx_power_val, bool enable)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_wlan_tx_power_control(rf, phy, pwr_ctrl_idx, tx_power_val, enable);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

bool halrf_wl_tx_power_control(void *rf_void, u32 tx_power_val)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_wl_tx_power_control(rf, tx_power_val);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

enum rtw_hal_status halrf_dpk_tracking(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_dpk_tracking(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return RTW_HAL_STATUS_SUCCESS;
}

u8 halrf_get_default_rfe_type(void *rf_void)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u8 val;
	
#if MUTEX_EN
	bool lock = false;

	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_default_rfe_type(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

u8 halrf_get_default_xtal(void *rf_void)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u8 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_default_xtal(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_psd_init(void *rf_void, enum phl_phy_idx phy,
			u8 path, u8 iq_path, u32 avg, u32 fft)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_psd_init(rf, phy,	path, iq_path, avg, fft);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_psd_restore(void *rf_void, enum phl_phy_idx phy)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_psd_restore(rf, phy);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

u32 halrf_psd_get_point_data(void *rf_void, enum phl_phy_idx phy, s32 point)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u32 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_psd_get_point_data(rf, phy, point);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_psd_query(void *rf_void, enum phl_phy_idx phy,
			u32 point, u32 start_point, u32 stop_point, u32 *outbuf)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_psd_query(rf, phy, point, start_point, stop_point, outbuf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_config_radio_to_fw(void *rf_void)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_config_radio_to_fw(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_disconnect_notify(void *rf_void, struct rtw_chan_def *chandef)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_disconnect_notify(rf, chandef);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

bool  halrf_check_mcc_ch(void *rf_void, struct rtw_chan_def *chandef )
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_check_mcc_ch(rf, chandef );

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

#if 0
void halrf_ctrl_bw_ch(void *rf_void, enum phl_phy_idx phy, u8 central_ch,
				enum band_type band, enum channel_width bw)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
	
	halrf_inn_ctrl_bw_ch(rf, phy, central_ch, band, bw);
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
}
#endif

void halrf_wifi_event_notify(void *rf_void,
			enum phl_msg_evt_id event, enum phl_phy_idx phy_idx)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_wifi_event_notify(rf, event, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_2g_rxant(void *rf_void, enum halrf_ant ant)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_2g_rxant(rf, ant);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

s8 halrf_get_ther_protected_threshold(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	s8 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_get_ther_protected_threshold(rf);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_tssi_set_efuse_to_de(void *rf_void,
					enum phl_phy_idx phy_idx)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_set_efuse_to_de(rf, phy_idx);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_tssi_scan_ch(void *rf_void, enum rf_path path)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_tssi_scan_ch(rf, path);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

void halrf_syn1_onoff(void *rf_void, enum phl_phy_idx phy, u8 path, bool syn1_turn_on)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_syn1_onoff(rf, phy, path, syn1_turn_on);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

u32 halrf_test_event_trigger(void *rf_void,
	enum phl_phy_idx phy, enum halrf_event_idx idx, enum halrf_event_func func)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	u32 val;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	val = halrf_inn_test_event_trigger(rf, phy, idx, func);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif

	return val;
}

void halrf_set_ant_main_or_aux(void *rf_void, enum rf_path path, bool main)
{	
	struct rf_info *rf = (struct rf_info *)rf_void;
	
#if MUTEX_EN
	bool lock = false;
	
	halrf_mutex_lock(rf, &rf->rf_lock);
	lock = true;
#endif

	halrf_inn_set_ant_main_or_aux(rf, path, main);

#if MUTEX_EN
	if (lock)
		halrf_mutex_unlock(rf, &rf->rf_lock);
#endif
}

