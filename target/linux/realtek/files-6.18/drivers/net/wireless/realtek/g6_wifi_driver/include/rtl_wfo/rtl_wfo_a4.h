/******************************************************************************
 *
 * Copyright(c) 2007 - 2023 Realtek Corporation.
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
#ifndef _RTL_WFO_A4_H_
#define _RTL_WFO_A4_H_

/* A4 IPC */
#define A4_STA		0x0
#define A4_DB		0x1
#define A4_DB_DEL	0x2
#define A4_DB_STATS 	0x3

int rtl_wfo_cfg_a4_sta_info(_adapter *padapter, struct rtw_a4_db_entry *db, u8 fc_staid);
void rtl_wfo_upt_a4_stats(_adapter *padapter, rtl_wfo_ipc_cmd_update_a4_info_t *a4_info);
void rtl_wfo_upt_a4_lb_info(_adapter *padapter, u8 *mac, u32 idx);
void rtl_wfo_del_a4_db(_adapter *padapter, u8 *mac, u8 pe_id, u8 fc_staid);

#endif /* _RTL_WFO_A4_H_ */
