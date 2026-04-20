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

#include "dbg_cmd_1115e.h"

#if MAC_BE_1115E_SUPPORT

static struct check_reg_info check_reg_1115e[] = {
	{R_BE_HALT_C2H, 0xffffffff, CHK_REG_INTF_USB | CHK_REG_INTF_SDIO | CHK_REG_INTF_PCIE},
};

u32 get_check_reg_1115e(u32 *reg_num, struct check_reg_info **check_reg)
{
	*check_reg = check_reg_1115e;
	*reg_num = sizeof(check_reg_1115e) / sizeof(check_reg_1115e[0]);

	return MACSUCCESS;
}
#endif