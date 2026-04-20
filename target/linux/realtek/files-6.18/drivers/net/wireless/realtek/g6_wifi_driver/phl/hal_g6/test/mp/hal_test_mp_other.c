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
#define _HAL_TEST_MP_OTHER_C_
#include "../../hal_headers.h"
#include "../../../test/mp/phl_test_mp_def.h"

#ifdef CONFIG_HAL_TEST_MP
enum rtw_hal_status rtw_hal_mp_other_agc_fix_gain(
	struct mp_context *mp, struct mp_other_arg *arg)
{
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_FAILURE;

	PHL_INFO("%s: phy index: %d\n", __FUNCTION__, mp->cur_phy);

	PHL_INFO("%s: en: %d\n", __FUNCTION__, arg->en);
	PHL_INFO("%s: elna_idx: %d\n", __FUNCTION__, arg->elna_idx);
	PHL_INFO("%s: lna_idx: %d\n", __FUNCTION__, arg->lna_idx);
	PHL_INFO("%s: tia_idx: %d\n", __FUNCTION__, arg->tia_idx);
	PHL_INFO("%s: rxbb_idx: %d\n", __FUNCTION__, arg->rxbb_idx);
	PHL_INFO("%s: path: %d\n", __FUNCTION__, arg->path);

	hal_status = rtw_hal_bb_set_agc_fix_gain(mp->hal, arg->en, arg->elna_idx, arg->lna_idx, arg->tia_idx, arg->rxbb_idx, arg->path);

	PHL_INFO("%s: status = %d\n", __FUNCTION__, hal_status);

	return hal_status;
}

#endif /* CONFIG_HAL_TEST_MP */
