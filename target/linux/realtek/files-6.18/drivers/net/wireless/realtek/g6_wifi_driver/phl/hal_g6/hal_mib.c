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
#define _HAL_MIB_C_
#include "hal_headers.h"

enum rtw_hal_status rtw_hal_set_lifetime(void *hal, u8 en, u16 to)
{
	return rtw_hal_mac_set_lifetime(hal, en, to);
}

enum rtw_hal_status rtw_hal_get_lifetime(void *hal)
{
	return rtw_hal_mac_get_lifetime(hal);
}

enum rtw_hal_status rtw_hal_set_tx_retry_limit(void *hal, u8 limit)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;

	return rtw_hal_mac_set_tx_retry_limit_all_sta(hal_info->hal_com, limit);
}

enum rtw_hal_status rtw_hal_set_ref_power(void *hal, u8 band, u16 txagc_ref)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	u32 value, result;

	result = rtw_hal_mac_get_pwr_reg(hal_info->hal_com, band, 0xd200, &value);
	if (result)
		return RTW_HAL_STATUS_FAILURE;

	value &= ~0x0ffffc00;
	value |= ((txagc_ref & 0x1ff) << 10);
	value |= ((txagc_ref & 0x1ff) << 19);
	return rtw_hal_mac_set_pwr_reg(hal_info->hal_com, band, 0xd200, value);
}

enum rtw_hal_status
rtw_hal_set_pwr_lmt_en_val(void *hal, u8 band, bool en_val)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_FAILURE;

	hstatus = rtw_hal_mac_set_pwr_lmt_en_val(hal_info->hal_com, band, en_val);
	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
		return RTW_HAL_STATUS_FAILURE;
	}

	hstatus = rtw_hal_mac_write_pwr_limit_en(hal_info->hal_com, band);
	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
		return RTW_HAL_STATUS_FAILURE;
	}

	return hstatus;
}

enum rtw_hal_status
rtw_hal_set_edcca_mode_en_val(void *hal, u8 band, u8 en_val)
{
	struct hal_info_t *hal_info = (struct hal_info_t *)hal;
	enum rtw_hal_status hstatus = RTW_HAL_STATUS_SUCCESS;

	hstatus = rtw_hal_mac_set_edcca_mode_en_val(hal_info->hal_com, band, en_val);
	if (RTW_HAL_STATUS_SUCCESS != hstatus) {
		return RTW_HAL_STATUS_FAILURE;
	}

	return hstatus;
}

#ifdef CONFIG_ENABLE_MAC_KEY_BACKUP
enum rtw_hal_status rtw_hal_backup_key(void *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	rtw_hal_mac_start_h2c_agg(hal);
#endif
	hal_status = rtw_hal_mac_backup_key(hal);
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	if (hal_status == RTW_HAL_STATUS_SUCCESS) {
		hal_status = rtw_hal_mac_stop_h2c_agg(hal, 1);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			PHL_ERR("[%s]rtw_hal_stop_h2c_agg fail\n", __func__);	

	} else {
		rtw_hal_mac_stop_h2c_agg(hal, 0);
	}
#endif
	return hal_status;
}

enum rtw_hal_status rtw_hal_restore_key(void *hal)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	rtw_hal_mac_start_h2c_agg(hal);
#endif
	hal_status = rtw_hal_mac_restore_key(hal);
#ifdef CONFIG_ENABLE_MAC_H2C_AGG
	if (hal_status == RTW_HAL_STATUS_SUCCESS) {
		hal_status = rtw_hal_mac_stop_h2c_agg(hal, 1);
		if (hal_status != RTW_HAL_STATUS_SUCCESS)
			PHL_ERR("[%s]rtw_hal_stop_h2c_agg fail\n", __func__);	
	} else {
		rtw_hal_mac_stop_h2c_agg(hal, 0);
	}
#endif
	return hal_status;
}
#endif /* CONFIG_ENABLE_MAC_KEY_BACKUP */

