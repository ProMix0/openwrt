/** @file */
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

#ifndef _MAC_AX_FW_CAP_1115E_H_
#define _MAC_AX_FW_CAP_1115E_H_

#include "../../mac_def.h"

#if MAC_AX_1115E_SUPPORT

u32 get_fw_cap_1115e(struct mac_ax_adapter *adapter, struct mac_fw_cap_info *info);

u32 dump_fw_cap_1115e(struct mac_ax_adapter *adapter,  struct mac_fw_cap_info *info,
		      enum mac_fw_feat_sel sel);
#endif /* #if MAC_AX_1115E_SUPPORT */
#endif
