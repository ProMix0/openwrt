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

#include "dbg_cmd_8852d.h"

#if MAC_AX_8852D_SUPPORT

static struct check_reg_info check_reg_8852d[] = {
	{R_AX_LTR_DEC_CTRL, 0x7, CHK_REG_INTF_PCIE},
	{R_AX_LTR_LATENCY_IDX0, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_LATENCY_IDX1, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_LATENCY_IDX2, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_LATENCY_IDX3, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_CTRL_0, 0x73, CHK_REG_INTF_PCIE},
	{R_AX_LTR_CTRL_1, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_IDLE_LATENCY, 0xffffffff, CHK_REG_INTF_PCIE},
	{R_AX_LTR_ACTIVE_LATENCY, 0xffffffff, CHK_REG_INTF_PCIE},
};

u32 get_check_reg_8852d(u32 *reg_num, struct check_reg_info **check_reg)
{
	*check_reg = check_reg_8852d;
	*reg_num = sizeof(check_reg_8852d) / sizeof(check_reg_8852d[0]);

	return MACSUCCESS;
}

#endif