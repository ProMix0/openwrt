/******************************************************************************
 *
 * Copyright(c) 2020 Realtek Corporation. All rights reserved.
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
 ******************************************************************************/

#include "fwcmd_hv.h"
#include "../mac_ax/fwcmd.h"
#include "../mac_ax/trx_desc.h"


u32 hv_ptn_h2c_common(struct mac_ax_adapter* adapter,
	struct rtw_g6_h2c_hdr* hdr, u32* pvalue)
{
#define HV_PTN_H2C_MAX_LEN 60
	u32 ret = 0;

	struct h2c_info h2c_info = {0};

	h2c_info.agg_en = 0;
	h2c_info.content_len = hdr->content_len;
	h2c_info.h2c_cat = FWCMD_H2C_CAT_TEST;
	h2c_info.h2c_class = FWCMD_H2C_CL_FW_AUTO_TEST;
	h2c_info.h2c_func = hdr->h2c_func;
	h2c_info.rec_ack = 0;
	h2c_info.done_ack = 0;

	ret = mac_h2c_common(adapter, &h2c_info, (u32 *)pvalue);

	return ret;
}

u32 c2h_lps_onoff_rpt(struct mac_ax_adapter *adapter,
	u16 len, u8 *buf)
{
	struct fwcmd_lps_onoff_test *rpt;
	u32 type;
	u32 ret;

	rpt = (struct fwcmd_lps_onoff_test *)buf;
	type = le32_to_cpu(rpt->dword0);
	ret = le32_to_cpu(rpt->dword1);
	PLTFM_MSG_TRACE("LPS test type (%d), result (%d).\n", type, ret);

    if (TRUE == ret) {
        return MACSUCCESS;
    } else {
        return MACFWTESTFAIL;
    }
}

