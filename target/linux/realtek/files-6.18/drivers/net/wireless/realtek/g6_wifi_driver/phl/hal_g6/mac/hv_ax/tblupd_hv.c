/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
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

#include "tblupd_hv.h"
#include "../mac_ax/tblupd.h"
#include "../mac_ax/fwcmd.h"
#include "../mac_ax/trx_desc.h"
#include <windows.h>

#define FLASH_PAGE_SIZE 1984

u32 mac_plat_auto_test(struct mac_ax_adapter *adapter,
		       struct mac_ax_plat_auto_test *info,
		       enum mac_ax_plat_module test_module)
{
	u32 ret = 0;
	u32 i;
	u32 *src, *dest;
	struct h2c_info h2c_info = {0};
	struct fwcmd_plat_auto_test *tbl;

	h2c_info.agg_en = 0;
	h2c_info.content_len = sizeof(struct fwcmd_plat_auto_test);
	h2c_info.h2c_cat = FWCMD_H2C_CAT_TEST;
	h2c_info.h2c_class = FWCMD_H2C_CL_PLAT_AUTO_TEST;
	h2c_info.h2c_func = test_module;
	h2c_info.rec_ack = 0;
	h2c_info.done_ack = 0;

	tbl = (struct fwcmd_plat_auto_test *)PLTFM_MALLOC(h2c_info.content_len);
	if (!tbl)
		return MACBUFALLOC;
	src = (u32 *)info;
	dest = (u32 *)(&tbl->dword0);
	for (i = 0; i < (sizeof(struct fwcmd_plat_auto_test) / 4); i++)
		*(dest++) = *(src++);

	ret = mac_h2c_common(adapter, &h2c_info, (u32 *)tbl);

	PLTFM_FREE(tbl, h2c_info.content_len);

	return ret;
}

u32 mac_long_run_test(struct mac_ax_adapter *adapter,
		      struct mac_ax_mac_test *info)
{
	u32 ret = 0;
	u32 i;
	u32 *src, *dest;
	struct h2c_info h2c_info = {0};
	struct fwcmd_long_run *tbl;

	h2c_info.agg_en = 0;
	h2c_info.content_len = sizeof(struct fwcmd_long_run);
	h2c_info.h2c_cat = FWCMD_H2C_CAT_TEST;
	h2c_info.h2c_class = FWCMD_H2C_CL_MAC_TEST;
	h2c_info.h2c_func = FWCMD_H2C_FUNC_LONG_RUN;
	h2c_info.rec_ack = 0;
	h2c_info.done_ack = 0;

	tbl = (struct fwcmd_long_run *)PLTFM_MALLOC(h2c_info.content_len);
	if (!tbl)
		return MACBUFALLOC;
	src = (u32 *)info;
	dest = (u32 *)(&tbl->dword0);
	for (i = 0; i < (sizeof(struct fwcmd_long_run) / 4); i++)
		*(dest++) = *(src++);

	ret = mac_h2c_common(adapter, &h2c_info, (u32 *)tbl);

	PLTFM_FREE(tbl, h2c_info.content_len);

	return ret;
}

u32 mac_flash_burn_test(struct mac_ax_adapter *adapter, u8 *fw, u32 len)
{
	u32 ret = 0, residue_len, pkt_len;
	struct h2c_info h2c_info = {0};
	u8 *tbl;

	residue_len = len;
	while (residue_len) {
		if (residue_len >= FLASH_PAGE_SIZE)
			pkt_len = FLASH_PAGE_SIZE;
		else
			pkt_len = residue_len;

		h2c_info.agg_en = 0;
		h2c_info.content_len = pkt_len;
		h2c_info.h2c_cat = FWCMD_H2C_CAT_TEST;
		h2c_info.h2c_class = FWCMD_H2C_CL_PLAT_AUTO_TEST;
		h2c_info.h2c_func = 0x10;
		h2c_info.rec_ack = 0;
		h2c_info.done_ack = 0;

		tbl = (u8 *)PLTFM_MALLOC(h2c_info.content_len);
		if (!tbl)
			return MACBUFALLOC;
		PLTFM_MEMCPY(tbl, fw, pkt_len);
		fw += pkt_len;

		ret = mac_h2c_common(adapter, &h2c_info, (u32 *)tbl);

		PLTFM_FREE(tbl, h2c_info.content_len);
		if (ret)
			goto fail;

		residue_len -= pkt_len;
		// a little delay for flash write
		Sleep(10000);
	}

	return MACSUCCESS;
fail:
	adapter->fw_info.h2c_seq--;

	return ret;
}

