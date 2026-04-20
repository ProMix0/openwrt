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
#define _PHL_TEST_MP_OTHER_C_
#include "../../phl_headers.h"
#include "phl_test_mp_def.h"
#include "../../hal_g6/test/mp/hal_test_mp_api.h"

#ifdef CONFIG_PHL_TEST_MP
enum rtw_phl_status phl_mp_other_agc_fix_gain(
	struct mp_context *mp, struct mp_other_arg *arg){

	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	hal_status = rtw_hal_mp_other_agc_fix_gain(mp, arg);

	/* Record the result */
	arg->cmd_ok = true;
	arg->status = hal_status;

	/* Transfer to report */
	mp->rpt = arg;
	mp->rpt_len = sizeof(struct mp_other_arg);
	mp->buf = NULL;
	mp->buf_len = 0;

	return RTW_PHL_STATUS_SUCCESS;
}

enum rtw_phl_status mp_other(struct mp_context *mp, struct mp_other_arg *arg)
{
	enum rtw_phl_status phl_status = RTW_PHL_STATUS_FAILURE;

	switch(arg->cmd){
	case MP_OTHER_CMD_SET_AGC_FIX_GAIN:
		PHL_INFO("%s: CMD = MP_RX_CMD_FILTER_RX_RESET\n", __FUNCTION__);
		phl_status = phl_mp_other_agc_fix_gain(mp, arg);
		break;
	default:
		PHL_WARN("%s: CMD NOT RECOGNIZED\n", __FUNCTION__);
		break;
	}

	return phl_status;
}
#endif /* CONFIG_PHL_TEST_MP */
