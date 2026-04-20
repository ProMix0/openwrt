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
#define _RTL_WFO_ACL_C_

#include <drv_types.h>

#include <common/rt_error.h>
#include <rt_acl_ext.h>
#include <rt_port_ext.h>

#define RT_ACL_CPU_PORT_LDPID_START	(0x10)

/* RTK FC WFO */
int rtl_wfo_set_amsdu_ofld_mode(u8 mode)
{
	int ret, i;

	if (mode >= AMSDU_OFLD_MAX)
		return _FAIL;

	ret = rtk_fc_wifi_amsdu_pe_offload_mode_set(mode);
	if (ret) {
		RTW_ERR("%s: mode %d error (%d)\n", __func__, mode, ret);
		return _FAIL;
	}

	for (i = 0; i < 8; i++)
		rtk_fc_wifi_amsdu_pe_offload_wifiPri_to_amsduQ_set(i, RTK_FC_WIFI_AMSDU_PE_QUEUE0);

	RTW_INFO("Set amdu ofld mode %d\n", mode);
	return _SUCCESS;
}

int rtl_wfo_get_amsdu_ofld_mode(u8 *mode)
{
	int ret;

	ret = rtk_fc_wifi_amsdu_pe_offload_mode_get(
				(rtk_fc_wifi_amsdu_pe_offload_mode_t *)mode);
	if (ret) {
		RTW_ERR("%s: error (%d)\n", __func__, ret);
		return _FAIL;
	}

	RTW_INFO("Get amdu ofld mode %d\n", *mode);
	return _SUCCESS;
}

int rtl_wfo_get_fc_staid(_adapter *adapter, struct sta_info *psta)
{
	u8 i;
	rtl_wfo_priv_t *wfo_priv;
	struct dvobj_priv *dvobj;

	dvobj = adapter_to_dvobj(adapter);
	wfo_priv = dvobj->wfo_priv;

	if (psta->fc_staid < FC_STA_NUM) {
		RTW_INFO("%s: Already added! FC STA (staid %d macid %d %d band %d)\n",
			__func__, psta->fc_staid, psta->phl_sta->macid,
			wfo_priv->fc_sta[psta->fc_staid].macid,
			wfo_priv->fc_sta[psta->fc_staid].band);
		return _FAIL;
	}

	_rtw_spinlock_bh(&wfo_priv->fc_lock);

	/* Reuse original FC entry */
	for (i = 0; i < FC_STA_NUM; i++) {
		if (wfo_priv->fc_sta[i].macid == psta->phl_sta->macid &&
			wfo_priv->fc_sta[i].band == GET_HAL_SPEC(dvobj)->band_cap) {
			psta->fc_staid = i;
			break;
		}
	}

	/* Find a new FC entry */
	for (i = 0; (psta->fc_staid == 0xff) && (i < FC_STA_NUM); i++) {
		if (wfo_priv->fc_sta[i].macid == 0xff) {
			wfo_priv->fc_sta[i].macid = psta->phl_sta->macid;
			wfo_priv->fc_sta[i].band = GET_HAL_SPEC(dvobj)->band_cap;
			wfo_priv->fc_sta[i].is_sta = _TRUE;
			memcpy(wfo_priv->fc_sta[i].mac, psta->phl_sta->mac_addr, ETH_ALEN);
			psta->fc_staid = i;
			break;
		}
	}
	_rtw_spinunlock_bh(&wfo_priv->fc_lock);

	if (psta->fc_staid >= FC_STA_NUM) {
		RTW_INFO("Get FC STA (staid %d) fail!\n", psta->fc_staid);
	} else {
		RTW_INFO("Get FC STA (staid %d macid %d band %d)\n",
			psta->fc_staid, wfo_priv->fc_sta[psta->fc_staid].macid,
			wfo_priv->fc_sta[psta->fc_staid].band);
	}

	return (psta->fc_staid < FC_STA_NUM) ? _SUCCESS: _FAIL;
}

int _add_staid_to_fc(struct sta_info *psta, u8 *mac_addr, int fc_staid)
{
	rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel;
	rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf;

	_rtw_memset(&sta_conf_sel, 0, sizeof(rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t));
	_rtw_memset(&sta_conf, 0, sizeof(rtk_fc_wifi_amsdu_pe_offload_sta_info_t));

	//sta_conf.wifi_pri_offld_en_bitmsk = 0xff; /* 8'b11111111 */
	sta_conf_sel.amsdu_pkt_size = 1;
	sta_conf.amsdu_pkt_size = psta->txsc_amsdu_size;
	sta_conf_sel.p_wifi_dev = 1;
	sta_conf.p_wifi_dev = psta->padapter->pnetdev;

	RTW_DBG("%s: STA addr %pM, fc_staid = %d\n", __func__, mac_addr, fc_staid);

	/* FC STA ID (0: unspecific, 1-40: designated STA) */
	return rtk_fc_wifi_amsdu_pe_offload_mac_id_set(
			fc_staid + 1, sta_conf_sel, sta_conf, mac_addr);
}

#ifdef CONFIG_RTW_A4_STA
int rtl_wfo_get_a4_fc_staid(_adapter *adapter, struct rtw_a4_db_entry *db, u8 fc_staid)
{
	u8 i;
	rtl_wfo_priv_t *wfo_priv;
	struct dvobj_priv *dvobj;
	struct sta_info *psta = db->psta;

	dvobj = adapter_to_dvobj(adapter);
	wfo_priv = dvobj->wfo_priv;

	RTW_DBG("%s: a4 db addr %px, mac %pM, fc_staid = %d staid_done %d\n", __func__,
			db, db->mac, db->fc_staid, db->staid_done);

	if (_rtw_memcmp(psta->phl_sta->mac_addr, db->mac, ETH_ALEN)) {
		RTW_INFO("%s: A4 station match %pM, assign origin sta fc_staid %d\n",
				__func__, db->mac, psta->fc_staid);
		db->fc_staid = psta->fc_staid;
		return _SUCCESS;
	}

	if (db->staid_done) {
		RTW_INFO("%s: Already added! FC A4 STA (staid %d macid %d %d band %d)\n",
			__func__, db->fc_staid, psta->phl_sta->macid,
			wfo_priv->fc_sta[psta->fc_staid].macid,
			wfo_priv->fc_sta[psta->fc_staid].band);
		return _FAIL;
	}

	_rtw_spinlock_bh(&wfo_priv->fc_lock);

	if (fc_staid < FC_STA_NUM) {
		wfo_priv->fc_sta[fc_staid].macid = psta->phl_sta->macid;
		wfo_priv->fc_sta[fc_staid].band = GET_HAL_SPEC(dvobj)->band_cap;
		wfo_priv->fc_sta[fc_staid].is_sta = _FALSE;
		memcpy(wfo_priv->fc_sta[fc_staid].mac, db->mac, ETH_ALEN);
		db->fc_staid = fc_staid;
		db->staid_done = _TRUE;
	} else {
		/* Find a new FC entry */
		for (i = 0; i < FC_STA_NUM; i++) {
			if (wfo_priv->fc_sta[i].macid == 0xff) {
				wfo_priv->fc_sta[i].macid = psta->phl_sta->macid;
				wfo_priv->fc_sta[i].band = GET_HAL_SPEC(dvobj)->band_cap;
				wfo_priv->fc_sta[i].is_sta = _FALSE;
				memcpy(wfo_priv->fc_sta[i].mac, db->mac, ETH_ALEN);
				db->fc_staid = i;
				db->staid_done = _TRUE;
				break;
			}
		}
	}
	_rtw_spinunlock_bh(&wfo_priv->fc_lock);

	RTW_INFO("%s: Get FC A4 STA %pM (staid %d macid %d band %d)\n", __func__,
		db->mac, db->fc_staid, wfo_priv->fc_sta[psta->fc_staid].macid,
		wfo_priv->fc_sta[psta->fc_staid].band);

	return (db->fc_staid < FC_STA_NUM) ? _SUCCESS: _FAIL;
}

int rtl_wfo_clear_a4_fc_staid(_adapter *adapter, struct rtw_a4_db_entry *db)
{
	rtl_wfo_priv_t *wfo_priv;

	if (db->fc_staid == 0xff)
		return _SUCCESS;

	if (db->fc_staid >= FC_STA_NUM) {
		RTW_ERR("%s: Invalid A4 STA (macid %d staid %d)!\n",
			__func__, db->psta->phl_sta->macid, db->fc_staid);
		return _FAIL;
	}

	RTW_INFO("%s: Clear FC A4 STA %pM (macid %d staid %d) success\n",
		__func__, db->mac, db->psta->phl_sta->macid, db->fc_staid);

	wfo_priv = adapter->dvobj->wfo_priv;
	_rtw_spinlock_bh(&wfo_priv->fc_lock);

	wfo_priv->fc_sta[db->fc_staid].macid = 0xff;
	wfo_priv->fc_sta[db->fc_staid].band = 0xff;
	db->fc_staid = 0xff;
	db->staid_done = 0;

	_rtw_spinunlock_bh(&wfo_priv->fc_lock);

	return _SUCCESS;
}

int rtl_wfo_add_a4_staid_to_fc(struct sta_info *psta, struct rtw_a4_db_entry *db)
{
	int ret;

	if (psta->fc_staid == 0xff)
		return _FAIL;

	if (_rtw_memcmp(psta->phl_sta->mac_addr, db->mac, ETH_ALEN))
		return _SUCCESS;

	ret = _add_staid_to_fc(psta, db->mac, db->fc_staid);
	if (ret) {
		RTW_ERR("%s: Add FC A4 STA (macid %d staid %d) fail (%d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid, ret);
		return _FAIL;
	}

	RTW_INFO("%s: Add FC A4 STA %pM (macid %d fc_staid %d) success\n", __func__,
		db->mac, psta->phl_sta->macid, db->fc_staid);

	return _SUCCESS;
}

int rtl_wfo_del_a4_staid_to_fc(struct rtw_a4_db_entry *db)
{
	int ret;

	if (db->fc_staid == 0xff)
		return _SUCCESS;

	if (db->fc_staid >= FC_STA_NUM) {
		RTW_ERR("%s: Invalid A4 STA (mac %pM, staid %d)!\n",
			__func__, db->mac, db->fc_staid);
		return _FAIL;
	}

	/* FC: 1-40 for the specific mac address, 0 for the unassigned */
	ret = rtk_fc_wifi_amsdu_pe_offload_mac_id_del(db->fc_staid + 1);
	if (ret) {
		RTW_ERR("%s: Delete FC A4 STA (mac %pM, staid %d) fail (%d)!\n",
			__func__, db->mac, db->fc_staid, ret);
		return _FAIL;
	}

	RTW_INFO("%s: Delete FC A4 STA %pM (mac %d staid %d) success\n",
		__func__, db->mac, db->psta->phl_sta->macid, db->fc_staid);

	return _SUCCESS;
}
#endif /* CONFIG_RTW_A4_STA */

int rtl_wfo_add_staid_to_fc(struct sta_info *psta)
{
	int ret;
	u8 *pmac_addr;

	if (psta->fc_staid == 0xff)
		return _FAIL;

	if (psta->fc_staid >= FC_STA_NUM) {
		RTW_ERR("%s: Invalid FC STA (macid %d staid %d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid);
		return _FAIL;
	}

	if (MLME_IS_STA(psta->padapter))
		pmac_addr = psta->padapter->mac_addr;
	else
		pmac_addr = psta->phl_sta->mac_addr;

	ret = _add_staid_to_fc(psta, pmac_addr, psta->fc_staid);
	if (ret) {
		RTW_ERR("%s: Add FC STA (macid %d staid %d) fail (%d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid, ret);
		return _FAIL;
	}

	RTW_INFO("Add FC STA (macid %d staid %d) success\n",
		psta->phl_sta->macid, psta->fc_staid);

	return _SUCCESS;
}

int rtl_wfo_clear_fc_staid(_adapter *adapter, struct sta_info *psta)
{
	rtl_wfo_priv_t *wfo_priv;

	if (psta->fc_staid == 0xff)
		return _SUCCESS;

	if (psta->fc_staid >= FC_STA_NUM) {
		RTW_ERR("%s: Invalid STA (macid %d staid %d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid);
		return _FAIL;
	}

	RTW_INFO("Clear FC STA (macid %d staid %d) success\n",
		psta->phl_sta->macid, psta->fc_staid);

	wfo_priv = adapter->dvobj->wfo_priv;
	_rtw_spinlock_bh(&wfo_priv->fc_lock);

	wfo_priv->fc_sta[psta->fc_staid].macid = 0xff;
	wfo_priv->fc_sta[psta->fc_staid].band = 0xff;
	psta->fc_staid = 0xff;

	_rtw_spinunlock_bh(&wfo_priv->fc_lock);

	return _SUCCESS;
}

int rtl_wfo_del_staid_to_fc(struct sta_info *psta)
{
	int ret;

	if (psta->fc_staid == 0xff)
		return _SUCCESS;

	if (psta->fc_staid >= FC_STA_NUM) {
		RTW_ERR("%s: Invalid STA (macid %d staid %d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid);
		return _FAIL;
	}

	/* FC: 1-40 for the specific mac address, 0 for the unassigned */
	ret = rtk_fc_wifi_amsdu_pe_offload_mac_id_del(psta->fc_staid + 1);
	if (ret) {
		RTW_ERR("%s: Delete FC STA (macid %d staid %d) fail (%d)!\n",
			__func__, psta->phl_sta->macid, psta->fc_staid, ret);
		return _FAIL;
	}

	RTW_INFO("Delete FC STA (macid %d staid %d) success\n",
		psta->phl_sta->macid, psta->fc_staid);

	return _SUCCESS;
}

int rtl_wfo_upd_sta_amsdu_info(struct sta_info *psta)
{
	_adapter *padapter;
	struct dvobj_priv *dvobj;

	if (!psta || !psta->padapter || psta->xps_pe_id >= RTL_WFO_ARM_ID)
		return _FAIL;

	padapter = psta->padapter;
	dvobj = adapter_to_dvobj(padapter);

	if (!RTL_WFO_IS_OFLD(dvobj))
		return _FAIL;

	rtl_wfo_ipc_cfg_sta_info(padapter, psta,
			(PE_ROLE_PPE | PE_ROLE_AOE), RTL_WFO_OP_UPDATE);

	return _SUCCESS;
}

void _rtl_wfo_upd_sta_info(_adapter *adapter, u16 macid,
	u8 rx_cpuid, u8 to_amsdu, u16 wpseq_lmt, bool txsc_update,
	struct sta_info *psta)
{
	const char *cpuid_str[RTL_WFO_PEID_MAX] = {"PE0", "PE1", "PE2", "Driver"};

	if (macid != U16_MAX && macid != psta->phl_sta->macid)
		return;

	/* RX RPS */
	if (rx_cpuid < RTL_WFO_PEID_MAX) {
		psta->rps_pe_id = rx_cpuid;
		RTW_PRINT("macid %d RX will be dispatch to %s\n",
			psta->phl_sta->macid, cpuid_str[rx_cpuid]);
	}

	if (to_amsdu == U8_MAX) {
		/* Update STA info only */
		if (wpseq_lmt != U16_MAX)
			psta->wpseq_lmt = wpseq_lmt;
		rtl_wfo_ipc_cfg_sta_info(adapter, psta,
				(PE_ROLE_PPE | PE_ROLE_DOE | PE_ROLE_AOE),
				RTL_WFO_OP_UPDATE);
		RTW_PRINT("STA (macid %d) info will be dispatch to PE\n",
			psta->phl_sta->macid);
	} else if (to_amsdu) {
		/* FC: to AOE */
		psta->xps_pe_id = rtl_wfo_pe_id_mapping(psta);
		rtl_wfo_get_fc_staid(adapter, psta);
		rtl_wfo_ipc_cfg_sta_info(adapter, psta,
				(PE_ROLE_PPE | PE_ROLE_DOE | PE_ROLE_AOE),
				RTL_WFO_OP_UPDATE);
		rtl_wfo_add_staid_to_fc(psta);
		RTW_PRINT("macid %d TX will be dispatch to AOE (%s)\n",
			psta->phl_sta->macid, cpuid_str[RTL_WFO_AOE_ID]);
	} else if (!to_amsdu) {
		/* FC: to Driver */
		psta->xps_pe_id = RTL_WFO_ARM_ID;
		rtl_wfo_del_staid_to_fc(psta);
		rtl_wfo_clear_fc_staid(adapter, psta);
		rtl_wfo_ipc_cfg_sta_info(adapter, psta,
				(PE_ROLE_PPE | PE_ROLE_DOE | PE_ROLE_AOE),
				RTL_WFO_OP_UPDATE);
		RTW_PRINT("macid %d TX will be dispatch to Host (%s)\n",
			psta->phl_sta->macid, cpuid_str[RTL_WFO_ARM_ID]);
	}

	/* TXSC update */
	if (txsc_update) {
		rtw_core_add_ofld_txsc(adapter,
			psta, psta->phl_sta->mac_addr);
	}
}

int rtl_wfo_upd_sta_info(_adapter *adapter, u16 macid,
	u8 rx_cpuid, u8 to_amsdu, u16 wpseq_lmt, bool txsc_update)
{
	int idx;
	_list *plist, *phead;
	struct sta_priv *pstapriv = &(adapter->stapriv);
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_info *psta;
	struct sta_info *pfirsta;
	unsigned char self_mac[18], bmc_mac[18], mac[18];
	struct mlme_ext_priv *pmlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

	// refer to proc_get_sta_info()
	if(rtw_is_adapter_up(adapter)) {
		if(MLME_IS_STA(adapter) && !(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			pfirsta = rtw_get_stainfo(pstapriv, adapter->phl_role->mac_addr);
		else
			pfirsta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);

		if(pfirsta == NULL)
			return _SUCCESS;
	}
	else
		return _SUCCESS;

	memset(self_mac, 0, sizeof(self_mac));
	memset(bmc_mac, 0, sizeof(bmc_mac));

	if(pfirsta->phl_sta)
		snprintf(self_mac, sizeof(self_mac), MAC_FMT,
			MAC_ARG(pfirsta->phl_sta->mac_addr));
	else
		return _SUCCESS;

	snprintf(bmc_mac, sizeof(bmc_mac), "%s", "ff:ff:ff:ff:ff:ff");
	if (MLME_IS_STA(adapter)) {
		if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) {
				if (strncmp(self_mac, bmc_mac, sizeof(self_mac))) {
					_rtl_wfo_upd_sta_info(adapter, macid, rx_cpuid,
						to_amsdu, wpseq_lmt, txsc_update, pfirsta);
				}
		}
	}
	else {
		_rtw_spinlock_bh(&pstapriv->sta_hash_lock);
		for (idx = 0; idx < NUM_STA; idx++) {
			phead = &(pstapriv->sta_hash[idx]);
			plist = get_next(phead);
			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);

				memset(mac, 0, sizeof(mac));
				if (psta->phl_sta) {
					snprintf(mac, sizeof(mac), MAC_FMT,
						MAC_ARG(psta->phl_sta->mac_addr));
					if (strncmp(mac, self_mac, sizeof(mac)) &&
						strncmp(mac, bmc_mac, sizeof(mac)))
						_rtl_wfo_upd_sta_info(adapter, macid, rx_cpuid,
							to_amsdu, wpseq_lmt, txsc_update, psta);
				}
			}
		}
		_rtw_spinunlock_bh(&pstapriv->sta_hash_lock);
	}

	return _SUCCESS;
}

int rtl_wfo_upd_all_sta_info(_adapter *padapter, bool txsc_only)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta;
	int i;
	_list *plist, *phead;

	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			if (!psta || (psta == padapter->self_sta) ||
				psta->rps_pe_id >= RTL_WFO_ARM_ID)
				continue;

			if (rtw_core_add_ofld_txsc(padapter, psta,
					psta->phl_sta->mac_addr) == FAIL || txsc_only)
				continue;

			if (padapter->dvobj->wfo_en & RTL_WFO_TX_OFLD)
				rtl_wfo_get_fc_staid(padapter, psta);

			rtl_wfo_ipc_cfg_sta_info(padapter, psta,
					(PE_ROLE_PPE | PE_ROLE_DOE | PE_ROLE_AOE),
					RTL_WFO_OP_UPDATE);

			if (padapter->dvobj->wfo_en & RTL_WFO_TX_OFLD)
				rtl_wfo_add_staid_to_fc(psta);
		}
	}

	return _SUCCESS;
}

int rtl_wfo_flush_fc_staid(void)
{
	int ret;

	ret = rtk_fc_wifi_amsdu_pe_offload_mac_id_flush();
	if (ret) {
		RTW_ERR("%s: flush FC STA table fail (%d)!\n", __func__, ret);
		return _FAIL;
	}

	RTW_INFO("Flush FC STA table success\n");

	return _SUCCESS;
}

u8 rtl_wfo_get_devid(struct net_device *dev)
{
	unsigned int wlan_dev_idx;

	rtk_fc_wifi_dev_to_devidx_get(dev, &wlan_dev_idx);

	return (u8)wlan_dev_idx;
}

/* RTK ACL */
void rtl_wfo_redir_txpkt_to_pe(struct txsc_entry *ts_entry, s8 cpuport, s8 cos)
{
	int acl_idx;
	rt_acl_filterAndQos_t rt_acl_entry;

	if (cpuport < 0 || cos < 0)
		return;

	_rtw_memset(&rt_acl_entry, 0, sizeof(rt_acl_filterAndQos_t));
	rt_acl_entry.acl_weight = 100;
	rt_acl_entry.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT;
	rt_acl_entry.filter_fields |= RT_ACL_INGRESS_DMAC_BIT;
	rt_acl_entry.filter_fields |= RT_ACL_INGRESS_SMAC_BIT;
	rt_acl_entry.filter_fields |= RT_ACL_INGRESS_ETHERTYPE_BIT;
	rt_acl_entry.ingress_port_mask = RT_PORT_PHY_PORT_PORT0_BIT |
									RT_PORT_PHY_PORT_PORT1_BIT |
									RT_PORT_PHY_PORT_PORT2_BIT |
									RT_PORT_PHY_PORT_PORT3_BIT |
									RT_PORT_PHY_PORT_PORT4_BIT |
									RT_PORT_PHY_PORT_PON_BIT;
	_rtw_memcpy(rt_acl_entry.ingress_dmac, ts_entry->txsc_ethdr, ETH_ALEN);
	_rtw_memcpy(rt_acl_entry.ingress_smac, ts_entry->txsc_ethdr + ETH_ALEN, ETH_ALEN);
	rt_acl_entry.ingress_ethertype = (ts_entry->txsc_ethdr[ETH_HLEN - 2]) << 8 |
										(ts_entry->txsc_ethdr[ETH_HLEN - 1]);
	rt_acl_entry.action_fields |= RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT;
	rt_acl_entry.action_fields |= RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT;
	rt_acl_entry.action_priority_group_acl_priority = cos;
	rt_acl_entry.action_forward_group_redirect_port_idx = RT_ACL_CPU_PORT_LDPID_START + cpuport;

	if (ts_entry->acl_idx > 0)
		rt_acl_filterAndQos_del(ts_entry->acl_idx);

	if (rt_acl_filterAndQos_add(rt_acl_entry, &acl_idx) != RT_ERR_OK) {
		RTW_ERR("Add ACL rule fail!\n");
		ts_entry->acl_idx = 0;
		return;
	}

	ts_entry->acl_idx = acl_idx;

	RTW_PRINT("Add ACL rule (idx %d DA "MAC_FMT" SA "MAC_FMT" type 0x%04x port 0x%x cos %u).\n",
		ts_entry->acl_idx, MAC_ARG(rt_acl_entry.ingress_dmac),
		MAC_ARG(rt_acl_entry.ingress_smac), rt_acl_entry.ingress_ethertype,
		RT_ACL_CPU_PORT_LDPID_START + cpuport, cos);
}

void rtl_wfo_redir_txpkt_to_host(struct txsc_entry *ts_entry)
{
	if (ts_entry->acl_idx == 0)
		return;

	if (rt_acl_filterAndQos_del(ts_entry->acl_idx) != RT_ERR_OK)
		RTW_ERR("Delete ACL rule (%d) fail!\n", ts_entry->acl_idx);
	else
		RTW_PRINT("Delete ACL rule (%d).\n", ts_entry->acl_idx);

	ts_entry->acl_idx = 0;
}

