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
#ifndef _RTL_WFO_ACL_H_
#define _RTL_WFO_ACL_H_

extern int rtk_fc_internal_amsdu_pe_offload_mc2uc(bool enable);

/* RTK FC WFO */
int rtl_wfo_set_amsdu_ofld_mode(u8 mode);
int rtl_wfo_get_amsdu_ofld_mode(u8 *mode);
int rtl_wfo_get_fc_staid(_adapter *adapter, struct sta_info *psta);
int rtl_wfo_add_staid_to_fc(struct sta_info *psta);
#ifdef CONFIG_RTW_A4_STA
int rtl_wfo_get_a4_fc_staid(_adapter *adapter, struct rtw_a4_db_entry *db, u8 fc_staid);
int rtl_wfo_clear_a4_fc_staid(_adapter *adapter, struct rtw_a4_db_entry *db);
int rtl_wfo_add_a4_staid_to_fc(struct sta_info *psta, struct rtw_a4_db_entry *db);
int rtl_wfo_del_a4_staid_to_fc(struct rtw_a4_db_entry *db);
#endif
int rtl_wfo_clear_fc_staid(_adapter *adapter, struct sta_info *psta);
int rtl_wfo_del_staid_to_fc(struct sta_info *psta);
int rtl_wfo_upd_sta_amsdu_info(struct sta_info *psta);
int rtl_wfo_upd_sta_info(_adapter *adapter, u16 macid,
	u8 rx_cpuid, u8 to_amsdu, u16 wpseq_lmt, bool txsc_update);
int rtl_wfo_upd_all_sta_info(_adapter *padapter, bool txsc_only);
int rtl_wfo_flush_fc_staid(void);
u8 rtl_wfo_get_devid(struct net_device *dev);

/* RTK ACL */
void rtl_wfo_redir_txpkt_to_pe(struct txsc_entry *ts_entry, s8 cpuport, s8 cos);
void rtl_wfo_redir_txpkt_to_host(struct txsc_entry *ts_entry);

#endif /* _RTL_WFO_ACL_H_ */

