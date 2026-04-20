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
#define _PHL_MIB_C_
#include "phl_headers.h"

#ifdef CONFIG_LIFETIME_FEATURE
enum rtw_phl_status
rtw_phl_set_lifetime(void *phl, u8 enable, u16 timeout)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);

	hsts = rtw_hal_set_lifetime(phl_info->hal, enable, timeout);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
rtw_phl_get_lifetime(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	void *drv_priv = phl_to_drvpriv(phl_info);

	hsts = rtw_hal_get_lifetime(phl_info->hal);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}
#endif

enum rtw_phl_status
phl_set_tx_retry_limit_hdl(void *phl, u8 limit)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;

	hsts = rtw_hal_set_tx_retry_limit(phl_info->hal, limit);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}

enum rtw_phl_status
rtw_phl_cmd_set_tx_retry_limit(void *phl, struct rtw_wifi_role_t *wifi_role,
								u8 limit,
								enum phl_cmd_type cmd_type,
								u32 cmd_timeout)
{

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

#ifdef CONFIG_CMD_DISP
	if (cmd_type == PHL_CMD_DIRECTLY) {
		psts = phl_set_tx_retry_limit_hdl(phl_info, limit);
		goto _exit;
	}

	psts = phl_cmd_enqueue(phl_info,
			wifi_role->hw_band,
			MSG_EVT_SET_TX_RTY_LMT,
			(u8*)&limit, sizeof(u8),
			NULL,
			cmd_type, cmd_timeout);

	if (is_cmd_failure(psts)) {
		/* Send cmd success, but wait cmd fail*/
		psts = RTW_PHL_STATUS_FAILURE;
	} else if (psts != RTW_PHL_STATUS_SUCCESS) {
		/* Send cmd fail */
		psts = RTW_PHL_STATUS_FAILURE;
	}

#else
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,"%s CMD_DISP not supported, call directly\n", __func__);
	psts = phl_set_tx_retry_limit_hdl(phl_info, limit);
#endif

_exit:
	return psts;
}

enum rtw_phl_status
phl_set_pwr_lmt_en_val_hdl(void *phl, u8 *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct phl_pwr_lmt_param *p = (struct phl_pwr_lmt_param*)param;

	hsts = rtw_hal_set_pwr_lmt_en_val(phl_info->hal, p->band, p->en_val);
	if (RTW_HAL_STATUS_SUCCESS != hsts) {
		return RTW_HAL_STATUS_FAILURE;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_cmd_set_pwr_lmt_en_val(void *phl, struct rtw_wifi_role_t *wifi_role,
								bool en_val,
								enum phl_cmd_type cmd_type,
								u32 cmd_timeout)
{

	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_pwr_lmt_param *parm;
	u32 parm_len = sizeof(struct phl_pwr_lmt_param);

	parm = _os_kmem_alloc(phl_to_drvpriv(phl_info), parm_len);
	if (parm == NULL) {
		PHL_ERR("%s: failed to allocate parm\n", __func__);
		return RTW_PHL_STATUS_RESOURCE;
	}
	parm->band = wifi_role->hw_band;
	parm->en_val = en_val;

#ifdef CONFIG_CMD_DISP
	if (cmd_type == PHL_CMD_DIRECTLY) {
		psts = phl_set_pwr_lmt_en_val_hdl(phl_info, (u8 *)parm);
		_os_kmem_free(phl_to_drvpriv(phl_info), parm, parm_len);
		goto _exit;
	}

	psts = phl_cmd_enqueue(phl_info,
			wifi_role->hw_band,
			MSG_EVT_SET_PWR_LMT_EN,
			(u8 *)parm, parm_len,
			NULL,
			cmd_type, cmd_timeout);

	if (is_cmd_failure(psts)) {
		/* Send cmd success, but wait cmd fail*/
		psts = RTW_PHL_STATUS_FAILURE;
	} else if (psts != RTW_PHL_STATUS_SUCCESS) {
		/* Send cmd fail */
		psts = RTW_PHL_STATUS_FAILURE;
	}

#else
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,"%s CMD_DISP not supported, call directly\n", __func__);
	psts = phl_set_pwr_lmt_en_val_hdl(phl_info, (u8 *)parm);
#endif

_exit:
	return psts;
}

static void
_phl_cmd_set_edcca_mode_done(void *drv_priv,
						u8 *cmd,
						u32 cmd_len,
						enum rtw_phl_status status)
{
	struct phl_edcca_mode_param *param = (struct phl_edcca_mode_param *)cmd;
	if (param) {
		_os_kmem_free(drv_priv, param, cmd_len);
	}
}

enum rtw_phl_status
phl_set_edcca_mode_en_val_hdl(void *phl, u8 *param)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct phl_edcca_mode_param *p = (struct phl_edcca_mode_param*)param;

	hsts = rtw_hal_set_edcca_mode_en_val(phl_info->hal, p->band, p->resp_ack_chk_cca_en);
	if (hsts != RTW_HAL_STATUS_SUCCESS) {
		return RTW_PHL_STATUS_FAILURE;
	}

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status
rtw_phl_cmd_set_edcca_mode_en_val(void *phl, struct rtw_wifi_role_t *wifi_role,
								u8 en_val,
								enum phl_cmd_type cmd_type,
								u32 cmd_timeout)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	void *drv = phl_to_drvpriv(phl_info);
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;
	struct phl_edcca_mode_param *parm;
	u32 parm_len = sizeof(struct phl_edcca_mode_param);

#ifdef CONFIG_CMD_DISP
	if (cmd_type == PHL_CMD_DIRECTLY) {
		if (rtw_hal_set_edcca_mode_en_val(phl_info->hal, wifi_role->hw_band, en_val) != RTW_HAL_STATUS_SUCCESS) {
			psts = RTW_PHL_STATUS_FAILURE;
		} else
			psts = RTW_PHL_STATUS_SUCCESS;
		goto _exit;
	}

	parm = _os_kmem_alloc(phl_to_drvpriv(phl_info), parm_len);
	parm->band = wifi_role->hw_band;
	parm->resp_ack_chk_cca_en = en_val;

	psts = phl_cmd_enqueue(phl_info,
			wifi_role->hw_band,
			MSG_EVT_SET_EDCCA_MODE_EN,
			(u8 *)parm, parm_len,
			_phl_cmd_set_edcca_mode_done,
			cmd_type, cmd_timeout);

	if ((false == is_cmd_enqueue(psts)) && (RTW_PHL_STATUS_SUCCESS != psts)) {
		if (parm)
			_os_kmem_free(drv, parm, parm_len);
	}

#else
	PHL_TRACE(COMP_PHL_DBG, _PHL_DEBUG_,"%s CMD_DISP not supported, call directly\n", __func__);
	if (rtw_hal_set_edcca_mode_en_val(phl_info->hal, wifi_role->hw_band, en_val) != RTW_HAL_STATUS_SUCCESS) {
		psts = RTW_PHL_STATUS_FAILURE;
	} else
		psts = RTW_PHL_STATUS_SUCCESS;
#endif

_exit:
	return psts;
}

#ifdef POWER_PERCENT_ADJUSTMENT
enum rtw_phl_status
rtw_phl_set_ref_power(void *phl, u8 band, int txagc_ref)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	struct hal_info_t *hal_info = (struct hal_info_t *)phl_info->hal;
	int diff_level = txagc_ref;
	s8 ofst_bw = 0, ofst_mode = 0;

	if (diff_level > 14)
		diff_level = 14;
	else if (diff_level < -16)
		diff_level = -16;
	else
		diff_level = txagc_ref;
	ofst_mode = (diff_level / 2) + (diff_level & 1);
	ofst_bw = (diff_level / 2);

	hsts = _phl_set_power_offset(phl, band, ofst_mode, ofst_bw);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}
#endif /* POWER_PERCENT_ADJUSTMENT */

enum rtw_phl_status
rtw_phl_set_dig_mode(void *phl, struct rtw_wifi_role_t *wifi_role,
								u8 dig_op_mode)
{
	// enum dig_op_mode
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;
	enum rtw_phl_status psts = RTW_PHL_STATUS_FAILURE;

	hsts = rtw_hal_set_dig_mode(phl_info->hal, dig_op_mode);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		goto fail;

	return RTW_PHL_STATUS_SUCCESS;

fail:
	return RTW_PHL_STATUS_FAILURE;
}

u8 rtw_phl_antdiv_get_targetant(void *phl)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;

	return rtw_hal_antdiv_get_targetant(phl_info->hal);
}

enum rtw_phl_status rtw_phl_antdiv_pause(void *phl, u8 target_ant, u8 pause)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;

	hsts = rtw_hal_antdiv_pause(phl_info->hal, target_ant, pause);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		return RTW_PHL_STATUS_FAILURE;
	else
		return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status rtw_phl_sr_enable(void *phl, u8 sr_en)
{
	struct phl_info_t *phl_info = (struct phl_info_t *)phl;
	enum rtw_hal_status hsts = RTW_HAL_STATUS_FAILURE;

	hsts = rtw_hal_sr_enable(phl_info->hal, sr_en);
	if (RTW_HAL_STATUS_SUCCESS != hsts)
		return RTW_PHL_STATUS_FAILURE;
	else
		return RTW_PHL_STATUS_SUCCESS;
}


