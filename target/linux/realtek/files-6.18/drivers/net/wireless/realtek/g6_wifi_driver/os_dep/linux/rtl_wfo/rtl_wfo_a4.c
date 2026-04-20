/******************************************************************************
 *
 * Copyright(c) 2007 - 2022 Realtek Corporation.
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
#define _RTL_WFO_A4_C_

#include <drv_types.h>

int rtl_wfo_cfg_a4_sta_info(_adapter *padapter, struct rtw_a4_db_entry *db, u8 fc_staid)
{
	struct sta_info *sta = db->psta;
	int ret = SUCCESS;

	if (sta->rps_pe_id >= RTL_WFO_ARM_ID)
		return FAIL;

	if (padapter->dvobj->wfo_en & RTL_WFO_TX_OFLD)
		rtl_wfo_get_a4_fc_staid(padapter, db, fc_staid);

	rtl_wfo_ipc_cfg_sta_info(padapter, db,
			(PE_ROLE_PPE | PE_ROLE_AOE),
			RTL_WFO_OP_A4_UPDATE);

	if (padapter->dvobj->wfo_en & RTL_WFO_TX_OFLD)
		rtl_wfo_add_a4_staid_to_fc(sta, db);

	return ret;
}

void rtl_wfo_upt_a4_stats(_adapter *padapter, rtl_wfo_ipc_cmd_update_a4_info_t *a4_info)
{
	struct rtw_a4_db_entry *db = NULL;
	db = core_a4_get_source_db_entry(padapter, a4_info->mac);

	if (db) {
		db->tx_count += a4_info->tx_cnt;
		db->rx_count += a4_info->rx_cnt;
		db->tx_bytes += a4_info->tx_bytes;
		db->rx_bytes += a4_info->rx_bytes;
	}
}

void rtl_wfo_del_a4_db(_adapter *padapter, u8 *mac, u8 pe_id, u8 fc_staid)
{
	RTW_DBG("%s: adapter %pM, notify pe %u delete db %pM fc_staid %d\n",
		__func__, padapter->mac_addr, pe_id, mac, fc_staid);
	rtl_wfo_ipc_del_a4_db(padapter, pe_id, mac, fc_staid);
}
