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

#include "trx_desc.h"

u32 set_wd_checksum_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_wd_checksum_cfg *config)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val;

	adapter->hw_info->wd_checksum_en = config->sw_fill_wd_checksum_en ? 1 : 0;
	val = MAC_REG_R32(R_AX_WD_CHECKSUM_FUNCTION_ENABLE);
	if (config->host_check_en)
		val |= B_AX_HDT_WD_CHKSUM_EN;
	else
		val &= ~(u32)B_AX_HDT_WD_CHKSUM_EN;
	if (config->cpu_check_en) {
		PLTFM_MSG_ERR("%s: CPU WD CHKSUM is not supported\n", __func__);
		return MACNOTSUP;
	}
	MAC_REG_W32(R_AX_WD_CHECKSUM_FUNCTION_ENABLE, val);

	return MACSUCCESS;
}

u32 get_wd_checksum_cfg(struct mac_ax_adapter *adapter,
			struct mac_ax_wd_checksum_cfg *config)
{
	struct mac_ax_intf_ops *ops = adapter_to_intf_ops(adapter);
	u32 val;

	config->sw_fill_wd_checksum_en = adapter->hw_info->wd_checksum_en ? 1 : 0;
	val = MAC_REG_R32(R_AX_WD_CHECKSUM_FUNCTION_ENABLE);
	config->host_check_en = (val & B_AX_HDT_WD_CHKSUM_EN) ? 1 : 0;
	config->cpu_check_en = (val & B_AX_CDT_WD_CHKSUM_EN) ? 1 : 0;

	return MACSUCCESS;
}
