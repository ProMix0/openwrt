/******************************************************************************
 *
 * Copyright(c) 2023 Realtek Corporation.
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
#define _RTL8852DE_OPS_C_
#include "../rtl8852d_hal.h"
#include "rtl8852de.h"

void hal_set_ops_8852de(struct rtw_phl_com_t *phl_com,
			struct hal_info_t *hal)
{
	struct hal_ops_t *ops = hal_get_ops(hal);

	hal_set_ops_8852d(phl_com, hal);

	ops->init_hal_spec = init_hal_spec_8852de;
	ops->hal_get_efuse = hal_get_efuse_8852de;
	ops->hal_init = hal_init_8852de;
	ops->hal_deinit = hal_deinit_8852de;
	ops->hal_start = hal_start_8852de;
	ops->hal_stop = hal_stop_8852de;
#ifdef CONFIG_WOWLAN
	ops->hal_wow_init = hal_wow_init_8852de;
	ops->hal_wow_deinit = hal_wow_deinit_8852de;
#endif /* CONFIG_WOWLAN */
	ops->hal_mp_init = hal_mp_init_8852de;
	ops->hal_mp_deinit = hal_mp_deinit_8852de;

	ops->hal_hci_configure = hal_hci_cfg_8852de;
	ops->init_default_value = hal_init_default_value_8852de;
	ops->init_int_default_value = hal_init_int_default_value_8852de;
	ops->disable_interrupt_isr = hal_disable_int_isr_8852de;
	ops->enable_interrupt = hal_enable_int_8852de;
	ops->disable_interrupt = hal_disable_int_8852de;
	ops->recognize_interrupt = hal_recognize_int_8852de;
	ops->clear_interrupt = hal_clear_int_8852de;
	ops->interrupt_handler = hal_int_hdler_8852de;
	ops->restore_interrupt = hal_restore_int_8852de;
	ops->restore_rx_interrupt = hal_rx_int_restore_8852de;
}

