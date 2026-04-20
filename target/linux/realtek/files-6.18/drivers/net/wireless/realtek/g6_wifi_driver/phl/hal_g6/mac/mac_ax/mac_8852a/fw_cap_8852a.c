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
#include "fw_cap_8852a.h"

#if MAC_AX_8852A_SUPPORT

u32 get_fw_cap_8852a(struct mac_ax_adapter *adapter, struct mac_fw_cap_info *info)
{
	return MACSUCCESS;
}

u32 dump_fw_cap_8852a(struct mac_ax_adapter *adapter,  struct mac_fw_cap_info *info,
		      enum mac_fw_feat_sel sel)
{
	return MACSUCCESS;
}
#endif /* MAC_AX_8852A_SUPPORT */
