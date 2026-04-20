/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _RTW_TWT_C_

#include <drv_types.h>

#ifdef CONFIG_RTW_TWT

void _twt_debug_print_info(_adapter *padapter, struct rtw_phl_twt_element twt_element, const char* func_name)
{
	RTW_INFO("[%s]nego_type             = %d\n", func_name, TWT_NEGO_TYPE(twt_element));
	if (TWT_NEGO_TYPE(twt_element) == RTW_PHL_INDIV_TWT ||
	    TWT_NEGO_TYPE(twt_element) == RTW_PHL_WAKE_TBTT_INR) {
	    RTW_INFO("[%s]request               = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.twt_request);
		RTW_INFO("[%s]twt_setup_cmd         = %d\n", func_name, TWT_I_SETUP_CMD(twt_element));

		RTW_INFO("[%s]twt_protection        = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.twt_protection);
		RTW_INFO("[%s]trigger               = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.trigger);
		RTW_INFO("[%s]implicit              = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.implicit);
		RTW_INFO("[%s]flow_type             = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.flow_type);
		RTW_INFO("[%s]twt_flow_id           = %d\n", func_name, TWT_I_FLOW_ID(twt_element));
		RTW_INFO("[%s]twt_wake_int_exp      = %d\n", func_name, twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp);
		RTW_INFO("[%s]target_wake_t_h       = %u\n", func_name, twt_element.info.i_twt_para_set.target_wake_t_h);
		RTW_INFO("[%s]target_wake_t_l       = %u\n", func_name, twt_element.info.i_twt_para_set.target_wake_t_l);
		RTW_INFO("[%s]nom_min_twt_wake_dur  = %d\n", func_name, twt_element.info.i_twt_para_set.nom_min_twt_wake_dur);
		RTW_INFO("[%s]twt_wake_int_mantissa = %d\n", func_name, twt_element.info.i_twt_para_set.twt_wake_int_mantissa);
	}
}

void _twt_fill_default_ul_fixinfo(_adapter *padapter,
			struct sta_info *psta,
			struct rtw_phl_ax_ul_fixinfo *info)
{
	struct rtw_wifi_role_t *wifi_role = padapter->phl_role;
	int i;
	struct rtw_phl_stainfo_t *phl_sta = psta->phl_sta;

	_rtw_memset((u8*)info, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));

	info->cfg.mode = 0;
	info->cfg.storemode = 2;
	info->tf_type = 0;
	info->istwt = 1;
	info->multiport_id = 0;
	info->ulfix_usage = 2;
	info->data_bw = phl_sta->chandef.bw;
	info->data_rate = RTW_DATA_RATE_OFDM48;
	if (phl_sta->chandef.bw == CHANNEL_WIDTH_80)
		info->apep_len = 1000;
	else
		info->apep_len = 200;
#ifdef CONFIG_RTW_TWT_DBG
	if (padapter->twt_cmd_trigger_apep)
		info->apep_len = padapter->twt_cmd_trigger_apep;
#endif
	info->twtgrp_stanum_sel = 0;
	info->store_idx = 0;
	info->sta[0].macid = phl_sta->macid;
	info->sta[0].pref_AC = 0;

	info->ulrua.doppler = 0;
	info->ulrua.ppdu_bw = phl_sta->chandef.bw;
	info->ulrua.gi_ltf = 0;
	info->ulrua.sta_num = 1;
	info->ulrua.grp_id = 0;
	info->ulrua.fix_mode = 1;
	info->ulrua.stbc = 0;
	info->ulrua.rf_gain_fix = 0;
	info->ulrua.rf_gain_idx = 0;
	info->ulrua.tb_t_pe_nom = 2;
	info->ulrua.sta[0].tgt_rssi = rtw_phl_get_sta_rssi(phl_sta);
#ifdef CONFIG_RTW_TWT_DBG
	if (padapter->twt_cmd_trigger_rssi)
		info->ulrua.sta[0].tgt_rssi = padapter->twt_cmd_trigger_rssi;
#endif
	info->ulrua.sta[0].mac_id = phl_sta->macid;
	if (phl_sta->chandef.bw == CHANNEL_WIDTH_80) {
		info->ulrua.sta[0].ru_pos = 0x86;
	} else {
		info->ulrua.sta[0].ru_pos = 0x7a;
	}

	if (phl_sta->asoc_cap.he_ldpc)
		info->ulrua.sta[0].coding = 1; //LDPC
	else
		info->ulrua.sta[0].coding = 0; //BCC

	if ((phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
		info->ulrua.sta[0].rate.mcs = 11;
	else if((phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
		info->ulrua.sta[0].rate.mcs = 9;
	else
		info->ulrua.sta[0].rate.mcs = 7;
#ifdef CONFIG_RTW_TWT_DBG
	if (padapter->twt_cmd_trigger_mcs)
		info->ulrua.sta[0].rate.mcs = padapter->twt_cmd_trigger_mcs;
#endif
	info->ulrua.sta[0].rate.dcm = 0;
	info->ulrua.n_ltf_and_ma = info->ulrua.sta[0].rate.ss = phl_sta->asoc_cap.nss_rx - 1;
	info->ulrua.sta[0].bsr_length = 1000;

	RTW_INFO("[%s][%d]rssi = %d, coding = %d, mcs = %d, ss = %d\n", __func__,
								info->sta[0].macid,
								info->ulrua.sta[0].tgt_rssi,
								info->ulrua.sta[0].coding,
								info->ulrua.sta[0].rate.mcs,
								info->ulrua.sta[0].rate.ss);
}

/* deal with TWT action frames
 * return : _SUCCESS - means TWT action matched.
 *        : _FAIL    - means action code not belongs to TWT feature
 */
u8 rtw_core_twt_on_action(_adapter *padapter,
					u8 *raddr, u8 *pframe_body, u32 frame_len)
{
	u8 ret = _FAIL;
	u8 action_code = 0;

	action_code = pframe_body[1];

	if(!padapter->registrypriv.twt_enable) {
		RTW_INFO("[%s]Not support TWT, but receive TWT action frame from STA!\n", __FUNCTION__);
		return ret;
	}

	switch (action_code) {
		case RTW_WLAN_ACTION_UNPROETCT_S1G_TWTSETUP:
			if(MLME_IS_AP(padapter))
				ret = rtw_core_twt_on_setup_ap(padapter,
							raddr, pframe_body, frame_len);
			break;
		case RTW_WLAN_ACTION_UNPROETCT_S1G_TWTTEARDOWN:
			ret = rtw_core_twt_on_teardown(padapter,
							raddr, pframe_body, frame_len);
			break;
		case RTW_WLAN_ACTION_UNPROETCT_S1G_TWTINFO:
			RTW_INFO("[%s] Current not support TWT information action frame\n", __func__);
			break;
		default:
			RTW_ERR("[%s] unknown action %2x\n", __func__, action_code);
			break;
	}

	return ret;
}

int _twt_fill_default_setup_para(_adapter *padapter, struct rtw_phl_twt_setup_info *twt_setup_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_twt_get_twt_i get_twt_i;
	void *phl = padapter->dvobj->phl;
	u32 twt_l_diff;
	u32 twt_l = 0;
	u32 twt_h = 0;

	/* TODO: consider more default parameters */
	/* TODO: If is TWT REQUEST, should get TWT entry from PHL and find if similar config can be applied*/
	if(twt_setup_info == NULL)
		return _FAIL;

	switch (TWT_NEGO_TYPE(twt_setup_info->twt_element)) {
		case RTW_PHL_INDIV_TWT:
			get_twt_i.id = 0xff;
			get_twt_i.wrole = padapter->phl_role;
			get_twt_i.offset = 1000; /*temp set 1000 for debug log; 100 is enough for less log ver. */
			get_twt_i.tsf_h = &twt_h;
			get_twt_i.tsf_l = &twt_l;
			status = rtw_phl_twt_get_target_wake_time(phl,
							&get_twt_i,
							PHL_CMD_DIRECTLY,
							0);
			if (status != RTW_PHL_STATUS_SUCCESS) {
				twt_h = 0;
				twt_l = 0;
			}
			if (twt_h || twt_l) {
#ifdef WIFI_LOGO_HE_4_56_1
				if (padapter->mlmeextpriv.mlmext_info.is_HE_4_56_1) {
					twt_l_diff = twt_l % (32*1024);
					if (twt_setup_info->twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp == 4)
						twt_l_diff += 16*1024;
				} else
						
#endif
			   	twt_l_diff = twt_l % (16*1024);
			   	RTW_INFO("[%s]before tsf = %u, left = %u\n",__func__, twt_l, twt_l_diff);
			   	twt_l -= twt_l_diff;
			   	RTW_INFO("[%s]after tsf = %u\n",__func__, twt_l);
			}

			if (RTW_PHL_REQUEST_TWT == TWT_I_SETUP_CMD(twt_setup_info->twt_element)) {
				twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_l = twt_l;
				twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_h = twt_h;
			} else if(RTW_PHL_SUGGEST_TWT == TWT_I_SETUP_CMD(twt_setup_info->twt_element)){
				if(twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_h < twt_h) {
					twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_l = twt_l;
					twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_h = twt_h;
				} else if ((twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_h == twt_h) &&
				   (twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_l < twt_l)) {
					twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_l = twt_l;
					twt_setup_info->twt_element.info.i_twt_para_set.target_wake_t_h = twt_h;
				}
			}

			break;
		case RTW_PHL_WAKE_TBTT_INR:
		case RTW_PHL_BCAST_TWT:
		case RTW_PHL_MANAGE_BCAST_TWT:
			break;
		default:
			break;
	}
	return _TRUE;
}

#ifdef CONFIG_XMIT_MGMT_ACK

void rtw_issue_twt_setup_callback(struct rtw_txfb_t *txfb) {
	_adapter *padapter = (_adapter *)txfb->drvpriv;
	struct rtw_twt_sta_setup_cb *setup_cb = (struct rtw_twt_sta_setup_cb *)txfb->ctx;
	struct sta_info *psta = setup_cb->psta;
	int i;
	RTW_INFO("[rtw_issue_twt_setup_callback][%d]txsts = %d, setup_cb->config_id = %d, setup_cb->twt_cmd = %d\n", psta->phl_sta->macid, txfb->txsts, setup_cb->config_id, setup_cb->twt_cmd);
	if (txfb->txsts == TX_STATUS_TX_DONE) {
		if (setup_cb->twt_cmd == RTW_PHL_ACCEPT_TWT) {
			for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
				/* find exist entry */
				if (setup_cb->config_id == psta->twt_config[i].config_id) {
					psta->twt_config[i].acked = 1;
#ifdef CONFIG_RTL_WIFI_OFFLOAD
					psta->twt_en = _TRUE;
					psta->wpseq_lmt = padapter->registrypriv.wifi_mib.pe_twt_lmt;
					if (RTL_WFO_IS_OFLD(padapter->dvobj))
						rtl_wfo_ipc_cfg_sta_info(padapter, psta,
							(PE_ROLE_AOE | PE_ROLE_DOE), RTL_WFO_OP_UPDATE);
#endif
					if (padapter->twt_debug)
						rtw_core_twt_dump_local_setup_info(padapter, psta);
					break;
				}
			}
		}
	}
}
#endif
/* issue action frame: TWT setup frame
 * return : _SUCCESS - management frame send successfully.
 *        : _FAIL    - management frame not send.
 */
u8 rtw_issue_twt_setup(_adapter *padapter, struct sta_info *psta,
						struct rtw_phl_twt_setup_info twt_setup_info,
						enum rtw_phl_setup_cmd twt_cmd,
						u8 twt_config_id)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	u8					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	u8					*mac, *bssid;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	u8 len = 0;
	u8 category = RTW_WLAN_CATEGORY_UNPROTECTED_S1G;
	u8 action = RTW_WLAN_ACTION_UNPROETCT_S1G_TWTSETUP;
	u8 twt_element_id = WLAN_EID_TWT;
	u8 *da = NULL;
#ifdef CONFIG_XMIT_MGMT_ACK
	struct rtw_twt_sta_setup_cb *setup_cb = NULL;
#endif

	if (psta == NULL)
		return _FAIL;
	da = psta->phl_sta->mac_addr;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return _FAIL;

	RTW_INFO("[%s] Going to issue TWT setup resp frame to " MAC_FMT ", diag_token = %d, twt_setup_cmd = %d\n",
		      __func__, MAC_ARG(da), twt_setup_info.dialog_token, twt_cmd);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL) {
		RTW_INFO("[%s] alloc_mgtxmitframe fail\n", __FUNCTION__);
		return _FAIL;
	}

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	if (update_mgntframe_attrib(padapter, pattrib) != _SUCCESS) {
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return _FAIL;
	}

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);
	bssid = cur_network->MacAddress;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_rtw_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, bssid, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(fctrl, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(twt_setup_info.dialog_token), &(pattrib->pktlen));

	/* fill TWT element id */
	pframe = rtw_set_fixed_ie(pframe, 1, &(twt_element_id), &(pattrib->pktlen));

	/* fill TWT element body */
	TWT_I_SETUP_CMD(twt_setup_info.twt_element) = twt_cmd;
	if (twt_cmd >= RTW_PHL_TWT_GROUPING)
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 0;
	else
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 1;
	status = rtw_phl_twt_fill_twt_element(&twt_setup_info.twt_element, pframe + 1, &len);

	/* fill TWT element length */
	*pframe = len;

	pframe += len + 1;
	pattrib->pktlen += len + 1;

	pattrib->last_txcmdsz = pattrib->pktlen;

#ifdef CONFIG_XMIT_MGMT_ACK
	setup_cb = rtw_zmalloc(sizeof(struct rtw_twt_sta_setup_cb));
	if (setup_cb) {
		setup_cb->config_id = twt_config_id;
		setup_cb->psta = psta;
		setup_cb->twt_cmd = twt_cmd;
		pmgntframe->ack_rpt.txfb.ctx = (void*)setup_cb;
		pmgntframe->ack_rpt.txfb.drvpriv = (void*)padapter;
		pmgntframe->ack_rpt.txfb.txfb_cb = rtw_issue_twt_setup_callback;
		pmgntframe->ack_rpt.ctx_buf_len = sizeof(struct rtw_twt_sta_setup_cb);
	}
#endif

	dump_mgntframe(padapter, pmgntframe);
	return _SUCCESS;
}

u8 _twt_check_setup_request_valid(struct rtw_phl_twt_setup_info twt_setup_info)
{
	/* check TWT setup command */
	switch (TWT_I_SETUP_CMD(twt_setup_info.twt_element)) {
		case RTW_PHL_REQUEST_TWT:
		case RTW_PHL_SUGGEST_TWT:
		case RTW_PHL_DEMAND_TWT:
			break;
		default:
			return _FAIL;
	}

	/* check TWT nego type */
	switch (TWT_NEGO_TYPE(twt_setup_info.twt_element)) {
		case RTW_PHL_INDIV_TWT:
			break;
		case RTW_PHL_WAKE_TBTT_INR:
		case RTW_PHL_BCAST_TWT:
		case RTW_PHL_MANAGE_BCAST_TWT:
			RTW_ERR("[%s]Current not support TWT nego type %d\n",
				__func__, TWT_NEGO_TYPE(twt_setup_info.twt_element));
			return _FAIL;
		default:
			return _FAIL;
	}

	/* check TWT request field */
	if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request == 0)
		return _FAIL;


	return _SUCCESS;
}

void _core_twt_update_local_setup_info(_adapter *padapter, struct sta_info *psta,
					struct rtw_phl_twt_setup_info *twt_setup_info,
					u8 twt_config_id)
{
	int i, idx = -1;
	for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
		/* find exist entry */
		if (twt_config_id == psta->twt_config[i].config_id) {
			idx = i;
			break;
		}
		else if (psta->twt_config[i].config_id == TWT_CONFIG_ID_INVALID && idx == -1) {
			idx = i;
		}
	}

	if (idx != -1) {
		_rtw_memcpy(&psta->twt_config[idx].setup_info, twt_setup_info, sizeof(struct rtw_phl_twt_setup_info));
		psta->twt_config[idx].config_id = twt_config_id;
		psta->twt_config[idx].acked = 0;
	} else {
		RTW_ERR("[%s]Add local twt entry id %d failed!", __func__, twt_config_id);
	}
}

void _core_twt_delete_local_setup_info(_adapter *padapter, struct sta_info *psta,
					struct rtw_phl_twt_flow_field *twt_flow_info,
					u8 delete_all)
{
	int i;
	u8 delete_id = 255;

	if (delete_all) {
		for (i = 0; i < MAX_NUM_TWT_CONFIG; i++)
			psta->twt_config[i].config_id = TWT_CONFIG_ID_INVALID;
	} else {
		for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
			if (psta->twt_config[i].config_id == TWT_CONFIG_ID_INVALID)
				continue;
			if (twt_flow_info->nego_type != TWT_NEGO_TYPE(psta->twt_config[i].setup_info.twt_element))
				continue;

			if (twt_flow_info->nego_type == RTW_PHL_INDIV_TWT ||
	   			twt_flow_info->nego_type == RTW_PHL_WAKE_TBTT_INR) {
	   			if (twt_flow_info->info.twt_flow01.teardown_all)
	   				delete_id = 255;
	   			else
	   				delete_id = twt_flow_info->info.twt_flow01.twt_flow_id;

	   			if (TWT_I_FLOW_ID(psta->twt_config[i].setup_info.twt_element) == delete_id ||
	   				delete_id == 255) {
	   				psta->twt_config[i].config_id = TWT_CONFIG_ID_INVALID;
	   			}
	   		} else {
				/* current not support nego type */
	   		}
	   	}
	}
}

u8 _core_twt_sta_trigger(_adapter *padapter, struct sta_info *psta)
{
	int i;
	for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
		if (psta->twt_config[i].config_id == TWT_CONFIG_ID_INVALID)
			continue;

		if (psta->twt_config[i].acked == 0)
			continue;

		if (TWT_NEGO_TYPE(psta->twt_config[i].setup_info.twt_element) != RTW_PHL_INDIV_TWT &&
			TWT_NEGO_TYPE(psta->twt_config[i].setup_info.twt_element) != RTW_PHL_WAKE_TBTT_INR)
			continue;

		if (psta->twt_config[i].setup_info.twt_element.info.i_twt_para_set.req_type.trigger)
			return 1;
	}

	return 0;
}

void rtw_core_twt_dump_local_setup_info(_adapter *padapter, struct sta_info *psta)
{
	int i;

	if (!psta)
		return;

	printk("[%s]macid = %d\n", __func__, psta->phl_sta->macid);
	for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
		if (psta->twt_config[i].config_id == TWT_CONFIG_ID_INVALID)
			continue;
		printk("entry %d\n", i);
		printk("  config_id = %d\n", psta->twt_config[i].config_id);
		printk("  flow_id   = %d\n", TWT_I_FLOW_ID(psta->twt_config[i].setup_info.twt_element));
		printk("  nego_type = %d\n", TWT_NEGO_TYPE(psta->twt_config[i].setup_info.twt_element));
		printk("  trigger   = %d\n", psta->twt_config[i].setup_info.twt_element.info.i_twt_para_set.req_type.trigger);
		printk("  acked     = %d\n", psta->twt_config[i].acked);
	}
}

/* deal with TWT setup frame
 * return : _SUCCESS - twt setup response frame send.
 *        : _FAIL    - twt setup response frame not send.
 */
u8 rtw_core_twt_on_setup_ap(_adapter *padapter, u8 *raddr,
						u8 *twt_element, u32 frame_len)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	void *phl = padapter->dvobj->phl;
	struct sta_info *psta;
	struct rtw_phl_twt_setup_info twt_setup_info;
	struct rtw_phl_ax_ul_fixinfo info;
	u8 twt_config_id = TWT_CONFIG_ID_INVALID;
	u8 ret = _FAIL;
	u16 len = frame_len;
	u8 flow_id = 0;
	u8 x = 0;
	u8 flow_type = 0;
	u8 twt_cnt = 0;

	if (!(wrole->proto_role_cap.twt & BIT(1))) {
		RTW_ERR("[%s]AP does not support TWT responder\n", __func__);
		return _FAIL;
	}

	psta = rtw_get_stainfo(&padapter->stapriv, raddr);
	if (psta == NULL || psta->phl_sta == NULL) {
		RTW_INFO("[%s] STA not exist!\n", __func__);
		goto twt_setup_fail;
	}

	if (!(psta->phl_sta->asoc_cap.twt & BIT(0))) {
		RTW_ERR("[%s]STA does not support TWT requester\n", __func__);
		return _FAIL;
	}

	RTW_INFO("[%s]Recv TWT setup frame: mac_addr = " MAC_FMT ", diag_token = %d, len = %d\n",
		      __func__, MAC_ARG(raddr), twt_element[2], len);

	/* parse TWT setup frame content */
	status = rtw_phl_twt_parse_setup_info(twt_element, len, &twt_setup_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s]rtw_phl_parse_twt_setup_info fail!\n", __func__);
		goto twt_setup_fail;
	}

	if (_twt_check_setup_request_valid(twt_setup_info) == _FAIL) {
		RTW_ERR("[%s]TWT setup frame invalid!\n", __func__);
		goto twt_setup_fail;
	}

	_twt_fill_default_setup_para(padapter, &twt_setup_info);
	if(TWT_NEGO_TYPE(twt_setup_info.twt_element) == RTW_PHL_INDIV_TWT ||
	   TWT_NEGO_TYPE(twt_setup_info.twt_element) == RTW_PHL_WAKE_TBTT_INR)
		flow_id = TWT_I_FLOW_ID(twt_setup_info.twt_element);

	_twt_debug_print_info(padapter, twt_setup_info.twt_element, __func__);
	flow_type = twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type;
#ifdef WIFI_LOGO_HE_4_56_1
	if (padapter->mlmeextpriv.mlmext_info.is_HE_4_56_1)
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = RTW_PHL_UNANNOUNCED_TWT;
#endif
	/* alloc TWT config entry from PHL and enable*/
	status = rtw_phl_twt_alloc_twt_config(phl, wrole,
									twt_setup_info, 1, &twt_config_id);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s]rtw_phl_twt_alloc_twt_config fail!\n", __func__);
		goto issue_twt_reject;
	}

	twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = flow_type;
	ret = rtw_issue_twt_setup(padapter, psta, twt_setup_info, RTW_PHL_ACCEPT_TWT, twt_config_id);
	if (ret == _FAIL) {
		RTW_ERR("[%s]rtw_issue_twt_setup(RTW_PHL_ACCEPT_TWT) fail!\n", __func__);
		goto twt_setup_fail;
	}

	if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger) {
		_twt_fill_default_ul_fixinfo(padapter, psta, &info);
		status = rtw_phl_cmd_set_fw_ul_fixinfo(phl, wrole, &info, PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("[%s]rtw_phl_cmd_set_fw_ul_fixinfo fail!\n", __func__);
			goto issue_twt_reject;
		}
	}

	/* Add STA into TWT entry after sending twt response success*/
	status = rtw_phl_twt_add_sta_info(phl, psta->phl_sta,
									twt_config_id, flow_id);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s]rtw_phl_twt_add_sta_info fail!\n", __func__);
		goto issue_twt_reject;
	}

	_core_twt_update_local_setup_info(padapter, psta, &twt_setup_info, twt_config_id);

#ifdef CONFIG_LMT_TXREQ
	psta->lmt_pending_txreq = padapter->max_tx_ring_cnt >> 3;
#ifdef WIFI_LOGO_HE_4_56_1
	if (padapter->mlmeextpriv.mlmext_info.is_HE_4_56_1) {
		if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp != 4) {
			psta->lmt_pending_txreq = 512;
		} else {
			psta->lmt_pending_txreq = 250;
		}
	}
#endif
#endif

	return _SUCCESS;

issue_twt_reject:

	ret = rtw_issue_twt_setup(padapter, psta, twt_setup_info, RTW_PHL_REJECT_TWT, twt_config_id);
	if (ret == _FAIL)
		RTW_ERR("[%s]rtw_issue_twt_setup(RTW_PHL_REJECT_TWT) fail!\n", __func__);
	return ret;

twt_setup_fail:

	return _FAIL;
}

/* issue action frame: TWT teardown response
 * return : _SUCCESS - management frame send successfully.
 *        : _FAIL    - management frame not send.
 */
u8 rtw_issue_twt_teardown(_adapter *padapter, u8 *da,
						struct rtw_phl_twt_flow_field twt_flow_info)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	u8					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	u8					*mac, *bssid;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	u16 len = 0;
	u8 category = RTW_WLAN_CATEGORY_UNPROTECTED_S1G;
	u8 action = RTW_WLAN_ACTION_UNPROETCT_S1G_TWTTEARDOWN;

	if (da == NULL)
		return _FAIL;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return _FAIL;

	RTW_INFO("[%s] Going to issue TWT teardown frame to " MAC_FMT ", nego_type = %d\n",
		      __func__, MAC_ARG(da), twt_flow_info.nego_type);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL) {
		RTW_INFO("[%s] alloc_mgtxmitframe fail\n", __FUNCTION__);
		return _FAIL;
	}

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	if (update_mgntframe_attrib(padapter, pattrib) != _SUCCESS) {
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return _FAIL;
	}

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);
	bssid = cur_network->MacAddress;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	_rtw_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, bssid, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(fctrl, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));

	/* fill TWT flow body */
	status = rtw_phl_twt_fill_flow_field(&twt_flow_info, pframe, &len);

	pframe += len;
	pattrib->pktlen += len;

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
	return _SUCCESS;
}

/* deal with TWT teardown frame
 * remove STA from specific TWT config
 * delete TWT config if no STA is using
 * return : _SUCCESS - teardown TWT success.
 *        : _FAIL    - teardown TWT fail.
 */
u8 rtw_core_twt_on_teardown(_adapter *padapter, u8 *raddr,
							u8 *pframe, u32 frame_len)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_twt_flow_field twt_flow_info;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	void *phl = padapter->dvobj->phl;
	struct sta_info *psta;
	int id = 0;
	u8 bitmap = 0;
	u8 idx = 0;
	u16 len = frame_len;

	if (!(wrole->proto_role_cap.twt & (BIT(0) | BIT(1)))) {
		RTW_ERR("[%s] AP does not support TWT\n", __func__);
		return _FAIL;
	}

	RTW_INFO("[%s] Recv TWT flow frame, mac_addr = " MAC_FMT "\n",
		      __func__, MAC_ARG(raddr));

	psta = rtw_get_stainfo(&padapter->stapriv, raddr);
	if (psta == NULL || psta->phl_sta == NULL) {
		RTW_INFO("[%s] STA not exist!\n", __func__);
		goto teardown_twt_fail;
	}

	/* parse twt flow frame, start addr is head of TWT flow*/
	status = rtw_phl_twt_parse_flow_field(pframe + 2, len, &twt_flow_info);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s] rtw_phl_parse_twt_flow_field fail!\n", __func__);
		goto teardown_twt_fail;
	}

	/* Delete STA from TWT entry */
	status = rtw_phl_twt_teardown_sta(phl, psta->phl_sta, &twt_flow_info, &bitmap);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s] rtw_phl_twt_teardown_sta fail!\n", __func__);
		goto teardown_twt_fail;
	}

#ifdef CONFIG_RTL_WIFI_OFFLOAD
	/* no limit for this station */
	psta->twt_en = _FALSE;
	psta->wpseq_lmt = 0;
	if (RTL_WFO_IS_OFLD(padapter->dvobj))
		rtl_wfo_ipc_cfg_sta_info(padapter, psta,
			(PE_ROLE_AOE | PE_ROLE_DOE), RTL_WFO_OP_UPDATE);
#endif

	if (bitmap) {
		for (idx = 0; idx < 8; idx++) {
			if(bitmap & BIT(idx)) {
				RTW_INFO("[%s] TWT config idx %d is empty, delete it\n", __func__, idx);
				rtw_phl_twt_free_twt_config(phl, idx);
			}
		}
	}

	_core_twt_delete_local_setup_info(padapter, psta, &twt_flow_info, 0);

#ifdef CONFIG_LMT_TXREQ
	psta->lmt_pending_txreq = 0;
#endif
	if (psta->state & WIFI_SLEEP_STATE) {
		wakeup_sta_to_xmit(padapter, psta);
	}

	rtw_core_twt_ru_ulmacid_cfg(padapter, psta);

	return _SUCCESS;

teardown_twt_fail:

	RTW_ERR("[%s] Teardown TWT fail!\n", __func__);
	return _FAIL;
}

void
rtw_core_twt_inform_announce(_adapter *padapter, struct sta_info *psta)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *phl = padapter->dvobj->phl;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;

	if(!psta || !psta->phl_sta)
		return;

	if (!rtw_core_twt_sta_active(padapter, psta)) {
		return;
	}

	if (!(wrole->proto_role_cap.twt & BIT(1))) {
		RTW_DBG("[%s]AP does not support TWT responder\n", __func__);
		return;
	}

	status = rtw_phl_twt_sta_announce_to_fw(phl, psta->phl_sta->macid);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		/* for debug print*/
		/* RTW_DBG("[%s]rtw_phl_twt_sta_announce_to_fw fail\n", __func__); */
		return;
	}
	return;
}

u32 rtw_core_twt_teardown_sta_all(_adapter *padapter, struct sta_info *psta)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	void *phl = padapter->dvobj->phl;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	u8 bitmap = 0;
	u8 idx = 0;

	if (!psta || !psta->phl_sta) {
		RTW_ERR("[%s] sta_info not exists\n", __func__);
		goto teardown_sta_fail;
	}

	if (!(wrole->proto_role_cap.twt & BIT(1))) {
		RTW_ERR("[%s]AP does not support TWT responder\n", __func__);
		return _FAIL;
	}

	if (!rtw_core_twt_sta_active(padapter, psta))
		return _TRUE;

	_core_twt_delete_local_setup_info(padapter, psta, NULL, 1);

	status = rtw_phl_twt_delete_all_sta_info(phl, psta->phl_sta, &bitmap);
	if (status != RTW_PHL_STATUS_SUCCESS) {
		RTW_ERR("[%s] No TWT config entry is used by STA!\n", __func__);
		goto teardown_sta_fail;
	}
	idx = 0;
	while (bitmap) {
		if (bitmap & BIT(0)) {
				RTW_INFO("[%s] TWT config idx %d is empty, delete it\n", __func__, idx);
				rtw_phl_twt_free_twt_config(phl, idx);
		}
		idx += 1;
		bitmap = bitmap >> 1;
	}
	return _TRUE;

teardown_sta_fail:

	return _FALSE;
}

u32 rtw_core_twt_sta_active(_adapter *padapter, struct sta_info *psta) {
	u8 sum = 0;
	u8 i = 0;

	if (!padapter->registrypriv.twt_enable) {
		return 0;
	}

	for (i = 0; i < MAX_NUM_TWT_CONFIG; i++) {
		if ((psta->twt_config[i].config_id != TWT_CONFIG_ID_INVALID) && psta->twt_config[i].acked)
			sum++;
	}

	return sum;
}

void rtw_core_twt_ru_ulmacid_cfg(_adapter *adapter, struct sta_info *psta)
{
	void *phl = adapter->dvobj->phl;
	struct rtw_phl_ulmacid_set cfg;
	u8 en = 0;

	if (!adapter->registrypriv.twt_enable)
		return;

	en = rtw_core_twt_sta_active(adapter, psta) && _core_twt_sta_trigger(adapter, psta);
	if (!en && psta->twt_update_ulmacid_cfg == 0)
		return;

	_rtw_memset(&cfg, 0, sizeof(struct rtw_phl_ulmacid_set));

	cfg.phl_ul_macid_cfg[0].macid = psta->phl_sta->macid;
	if(en) {
		cfg.phl_ul_macid_cfg[0].ul_su_info_en = 1;
		psta->twt_update_ulmacid_cfg = 1;
	} else {
		cfg.phl_ul_macid_cfg[0].ul_su_info_en = 0;
		psta->twt_update_ulmacid_cfg = 0;
	}

	cfg.phl_ul_macid_cfg[0].ul_su_bw = psta->phl_sta->chandef.bw;
	cfg.phl_ul_macid_cfg[0].ul_su_gi_ltf = psta->cur_rx_gi_ltf;
	cfg.phl_ul_macid_cfg[0].ul_su_doppler_ctrl = 0;
	cfg.phl_ul_macid_cfg[0].ul_su_dcm = 0;
	cfg.phl_ul_macid_cfg[0].ul_su_ss = RTW_GET_RATE_NSS(psta->cur_rx_data_rate);
	cfg.phl_ul_macid_cfg[0].ul_su_mcs = psta->cur_rx_data_rate & 0xf;
	cfg.phl_ul_macid_cfg[0].ul_su_stbc = 0;
	cfg.phl_ul_macid_cfg[0].ul_su_coding = 1;
#ifdef CONFIG_RTW_TWT_DBG
	if (adapter->twt_cmd_trigger_rssi)
		cfg.phl_ul_macid_cfg[0].ul_su_rssi_m = adapter->twt_cmd_trigger_rssi << 1;
	else
#endif
		cfg.phl_ul_macid_cfg[0].ul_su_rssi_m = rtw_phl_get_sta_rssi(psta->phl_sta) << 1;

	if (adapter->mlmeextpriv.mlmext_info.is_HE_4_56_1) {
		cfg.phl_ul_macid_cfg[0].ul_su_gi_ltf = RTW_GILTF_LGI_4XHE32;

		if ((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC11)
			cfg.phl_ul_macid_cfg[0].ul_su_mcs = 11;
		else if((psta->phl_sta->asoc_cap.he_rx_mcs[0] & (BIT(0)|BIT(1))) == HE_MCS_SUPP_MSC0_TO_MSC9)
			cfg.phl_ul_macid_cfg[0].ul_su_mcs = 9;
		else
			cfg.phl_ul_macid_cfg[0].ul_su_mcs = 7;

		cfg.phl_ul_macid_cfg[0].ul_su_ss = psta->phl_sta->asoc_cap.nss_rx - 1;

		if (psta->phl_sta->asoc_cap.he_ldpc)
			cfg.phl_ul_macid_cfg[0].ul_su_coding = 1; //LDPC
		else
			cfg.phl_ul_macid_cfg[0].ul_su_coding = 0; //BCC
	}
	cfg.phl_ul_macid_cfg[0].endcmd = 1;

	if (adapter->twt_debug) {
		printk("[%s][%s]macid %d\n", __func__, en?"ADD":"DEL", cfg.phl_ul_macid_cfg[0].macid);
		printk("ul_su_bw = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_bw);
		printk("ul_su_gi_ltf = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_gi_ltf);
		printk("ul_su_ss = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_ss);
		printk("ul_su_mcs = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_mcs);
		printk("ul_su_coding = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_coding);
		printk("ul_su_rssi_m = %d\n", cfg.phl_ul_macid_cfg[0].ul_su_rssi_m);
	}

	rtw_phl_ru_set_ulmacid_cfg(phl, &cfg);
}


#ifdef CONFIG_RTW_TWT_DBG
void rtw_core_twt_test_cmd_setup(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct sta_info *psta = NULL;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct rtw_phl_twt_setup_info twt_setup_info;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	void *phl = padapter->dvobj->phl;
	u8 twt_config_id = TWT_CONFIG_ID_INVALID;
	u8 flow_id = 0;
	struct rtw_phl_ax_ul_fixinfo info;

	if (!padapter->registrypriv.twt_enable)
		goto issue_twt_reject;

	psta = rtw_get_stainfo_by_macid(pstapriv, padapter->twt_cmd_macid);
	if (!psta)
		goto issue_twt_reject;


	if (!(padapter->registrypriv.twt_dbgmode) &&
		!(psta->phl_sta->asoc_cap.twt & BIT(0))) {
		RTW_ERR("[%s]STA does not support TWT requester\n", __func__);
		return;
	}

	/* initialize a TWT setup info, fill in some parameters*/
	do {
			pmlmeinfo->dialogToken++;
	} while (pmlmeinfo->dialogToken == 0);
	twt_setup_info.dialog_token = pmlmeinfo->dialogToken;
	/* hardcode parameters for individual TWT */
	twt_setup_info.twt_element.info.i_twt_para_set.twt_channel = 0;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_setup_cmd = RTW_PHL_SUGGEST_TWT;
	twt_setup_info.twt_element.twt_ctrl.nego_type = RTW_PHL_INDIV_TWT;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_protection = 0;
	twt_setup_info.twt_element.twt_ctrl.responder_pm_mode = 0;
	twt_setup_info.twt_element.twt_ctrl.twt_info_frame_disable = 1;
	/* update parameters using cmd */
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger = padapter->twt_cmd_trigger;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.implicit = padapter->twt_cmd_implicit;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = padapter->twt_cmd_flow_type;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id = padapter->twt_cmd_flow_id;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp = padapter->twt_cmd_wake_int_exp;
	twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_h = padapter->twt_cmd_target_wake_t_h;
	twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l = padapter->twt_cmd_target_wake_t_l;
	twt_setup_info.twt_element.info.i_twt_para_set.nom_min_twt_wake_dur = padapter->twt_cmd_nom_min_twt_wake_dur;
	twt_setup_info.twt_element.info.i_twt_para_set.twt_wake_int_mantissa = padapter->twt_cmd_twt_wake_int_mantissa;

	_twt_fill_default_setup_para(padapter, &twt_setup_info);
	flow_id = twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id;
	_twt_debug_print_info(padapter,twt_setup_info.twt_element, __func__);

	RTW_INFO("[%s] setup TWT config for STA " MAC_FMT "\n", __func__, MAC_ARG(psta->phl_sta->mac_addr));

	status = rtw_phl_twt_alloc_twt_config(phl, wrole, twt_setup_info, 1, &twt_config_id);
	if (status != RTW_PHL_STATUS_SUCCESS)
		goto issue_twt_reject;
	RTW_INFO("[%s] setup TWT config done\n", __func__);

	if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger) {
		_twt_fill_default_ul_fixinfo(padapter, psta, &info);
		status = rtw_phl_cmd_set_fw_ul_fixinfo(phl, wrole, &info, PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("[%s]rtw_phl_cmd_set_fw_ul_fixinfo fail!\n", __func__);
			goto issue_twt_reject;
		}
	}

	status = rtw_phl_twt_add_sta_info(phl, psta->phl_sta, twt_config_id, flow_id);
	if (status != RTW_PHL_STATUS_SUCCESS)
		goto issue_twt_reject;

	_core_twt_update_local_setup_info(padapter, psta, &twt_setup_info, twt_config_id);

	RTW_INFO("[%s] add STA to TWT config done\n", __func__);

	rtw_issue_twt_setup(padapter, psta, twt_setup_info, RTW_PHL_ACCEPT_TWT, twt_config_id);

	RTW_INFO("[%s] issue TWT setup resp frame done\n", __func__);

#ifdef CONFIG_LMT_TXREQ
	psta->lmt_pending_txreq = padapter->max_tx_ring_cnt >> 3;
	if (padapter->mlmeextpriv.mlmext_info.is_HE_4_56_1) {
		if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp != 4)
			psta->lmt_pending_txreq = 512;
		else
			psta->lmt_pending_txreq = 250;
	}
#endif

	return;

issue_twt_reject:
	RTW_INFO("[%s] setup TWT config error\n", __func__);
	return;
}

void rtw_core_twt_test_cmd_teardown(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct rtw_phl_twt_flow_field twt_flow_info;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	void *phl = padapter->dvobj->phl;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	int i;
	u8 bitmap = 0;
	u8 ret = _FAIL;
	u8 idx = 0;

	if (!padapter->registrypriv.twt_enable)
		goto teardown_twt_fail;

	psta = rtw_get_stainfo_by_macid(pstapriv, padapter->twt_cmd_macid);
	if (!psta || !psta->phl_sta) {
		RTW_ERR("[%s] no STA associated\n", __func__);
		goto teardown_twt_fail;
	}

	RTW_INFO("TWT: %s mac_addr = " MAC_FMT "\n",
		      __func__, MAC_ARG(psta->phl_sta->mac_addr));

	/* set teardown element values */
	twt_flow_info.nego_type = RTW_PHL_INDIV_TWT;
	if (padapter->twt_cmd_teardown_id >= 8) {
		twt_flow_info.info.twt_flow01.teardown_all = 1;
		twt_flow_info.info.twt_flow01.twt_flow_id = 0;

	} else {
		twt_flow_info.info.twt_flow01.teardown_all = 0;
		twt_flow_info.info.twt_flow01.twt_flow_id = padapter->twt_cmd_teardown_id;
	}

	ret = rtw_issue_twt_teardown(padapter, psta->phl_sta->mac_addr, twt_flow_info);
	if(ret == _FAIL) {
		RTW_ERR("[%s] send TWT teardown frame fail\n", __func__);
		goto teardown_twt_fail;
	}

	_core_twt_delete_local_setup_info(padapter, psta, &twt_flow_info, 0);

	/* Delete STA from TWT entry */
	status = rtw_phl_twt_teardown_sta(phl, psta->phl_sta, &twt_flow_info, &bitmap);
	if (status != RTW_PHL_STATUS_SUCCESS)
		goto teardown_twt_fail;

	idx = 0;
	while (bitmap) {
		if (bitmap & BIT(0)) {
				RTW_INFO("[%s] TWT config idx %d is empty, delete it\n", __func__, idx);
				rtw_phl_twt_free_twt_config(phl, idx);
		}
		idx += 1;
		bitmap = bitmap >> 1;
	}
#ifdef CONFIG_LMT_TXREQ
	psta->lmt_pending_txreq = 0;
#endif

	return;

teardown_twt_fail:

	RTW_ERR("TWT: Teardown TWT fail!\n");
	return;
}

void rtw_core_twt_test_cmd_pwrbit(_adapter *padapter, u16 macid, u8 pwrbit)
{
	u8 pkt_type = 1;
	struct sta_info *psta = NULL;
	if(pwrbit)
		pkt_type = 2;
	else
		pkt_type = 1;

	psta = rtw_get_stainfo_by_macid(&padapter->stapriv, macid);
	if (psta) {
		if (pwrbit) {
			//if (!(psta->state & WIFI_SLEEP_STATE)) {
				stop_sta_xmit(padapter, psta);
				//#ifndef CONFIG_RTW_LINK_PHL_MASTER
				// ca5908f2ec3168148928d99d819d8d8c1a5b94c6 cliffwan
				rtw_phl_getpkt(padapter->dvobj->phl,
					padapter->phl_role, macid, pkt_type);
				//#endif /* CONFIG_RTW_LINK_PHL_MASTER */
				RTW_INFO("[rtw_twt_indicate_pwrbit][%d]pkt_type = %d\n", macid, pkt_type);
			//}
		} else {
			//if (psta->state & WIFI_SLEEP_STATE) {
				wakeup_sta_to_xmit(padapter, psta);
				//#ifndef CONFIG_RTW_LINK_PHL_MASTER
				// ca5908f2ec3168148928d99d819d8d8c1a5b94c6 cliffwan
				rtw_phl_getpkt(padapter->dvobj->phl,
					padapter->phl_role, macid, pkt_type);
				//#endif /* CONFIG_RTW_LINK_PHL_MASTER */
				RTW_INFO("[rtw_twt_indicate_pwrbit][%d]pkt_type = %d\n", macid, pkt_type);
			//}
		}
	}
}

void rtw_core_twt_test_cmd_announce(_adapter *padapter, u16 macid)
{
	void *phl = padapter->dvobj->phl;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;

	if (!padapter->registrypriv.twt_enable)
		return;

	psta = rtw_get_stainfo_by_macid(pstapriv, macid);
	if (!psta) {
		RTW_ERR("[%s]STA of macid %d not exist\n", __func__, macid);
		return;
	}

	if (!(padapter->registrypriv.twt_dbgmode) &&
		!(psta->phl_sta->asoc_cap.twt & BIT(0))) {
		RTW_ERR("[%s]STA does not support TWT requester\n", __func__);
		return;
	}

	rtw_core_twt_inform_announce(padapter, psta);
	return;
}

void rtw_core_twt_test_cmd_testcase1(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct sta_info *psta = NULL;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct rtw_phl_twt_setup_info twt_setup_info;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	void *phl = padapter->dvobj->phl;
	u8 twt_config_id = TWT_CONFIG_ID_INVALID;
	u8 flow_id = 0;
	u32 twt_wake_int_exp = 14;
	u32 wait_second = padapter->twt_cmd_test_wait_second;
	struct rtw_phl_ax_ul_fixinfo info;
	

	if (!padapter->registrypriv.twt_enable)
		goto issue_twt_reject;

	psta = rtw_get_stainfo_by_macid(pstapriv, padapter->twt_cmd_test_macid1);
	if (!psta)
		goto issue_twt_reject;


	if (!(padapter->registrypriv.twt_dbgmode) &&
		!(psta->phl_sta->asoc_cap.twt & BIT(0))) {
		RTW_ERR("[%s]STA does not support TWT requester\n", __func__);
		return;
	}

	/* initialize a TWT setup info, fill in some parameters*/
	do {
			pmlmeinfo->dialogToken++;
	} while (pmlmeinfo->dialogToken == 0);
	twt_setup_info.dialog_token = pmlmeinfo->dialogToken;
	/* hardcode parameters for individual TWT */
	twt_setup_info.twt_element.info.i_twt_para_set.twt_channel = 0;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_setup_cmd = RTW_PHL_SUGGEST_TWT;
	twt_setup_info.twt_element.twt_ctrl.nego_type = RTW_PHL_INDIV_TWT;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_protection = 0;
	twt_setup_info.twt_element.twt_ctrl.responder_pm_mode = 0;
	twt_setup_info.twt_element.twt_ctrl.twt_info_frame_disable = 1;

	twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.implicit = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id = 1;
	twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_h = 0;
	twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l = 0;
	twt_setup_info.twt_element.info.i_twt_para_set.nom_min_twt_wake_dur = 255;
	twt_setup_info.twt_element.info.i_twt_para_set.twt_wake_int_mantissa = 30;

	while (wait_second) {
		wait_second = wait_second>>1;
		twt_wake_int_exp++;
	}
	RTW_INFO("twt_wake_int_exp = %d\n", twt_wake_int_exp);
	twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp = twt_wake_int_exp;

	_twt_fill_default_setup_para(padapter, &twt_setup_info);
	flow_id = twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id;
	_twt_debug_print_info(padapter,twt_setup_info.twt_element, __func__);

	RTW_INFO("[%s] setup TWT config for STA " MAC_FMT "\n", __func__, MAC_ARG(psta->phl_sta->mac_addr));

	status = rtw_phl_twt_alloc_twt_config(phl, wrole, twt_setup_info, 1, &twt_config_id);
	if (status != RTW_PHL_STATUS_SUCCESS)
		goto issue_twt_reject;
	RTW_INFO("[%s] setup TWT config done\n", __func__);

	if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger) {
		_twt_fill_default_ul_fixinfo(padapter, psta, &info);
		status = rtw_phl_cmd_set_fw_ul_fixinfo(phl, wrole, &info, PHL_CMD_DIRECTLY, 0);
		if (status != RTW_PHL_STATUS_SUCCESS) {
			RTW_ERR("[%s]rtw_phl_cmd_set_fw_ul_fixinfo fail!\n", __func__);
			goto issue_twt_reject;
		}
	}

	status = rtw_phl_twt_add_sta_info(phl, psta->phl_sta, twt_config_id, flow_id);
	if (status != RTW_PHL_STATUS_SUCCESS)
		goto issue_twt_reject;
	RTW_INFO("[%s] add STA to TWT config done\n", __func__);

	_core_twt_update_local_setup_info(padapter, psta, &twt_setup_info, twt_config_id);

	rtw_issue_twt_setup(padapter, psta, twt_setup_info, RTW_PHL_ACCEPT_TWT, twt_config_id);
	RTW_INFO("[%s] issue TWT setup resp frame done\n", __func__);

#ifdef CONFIG_LMT_TXREQ
	psta->lmt_pending_txreq = padapter->max_tx_ring_cnt >> 3;
#endif
	rtw_core_twt_test_cmd_pwrbit(padapter, psta->phl_sta->macid, 1);

	return;

issue_twt_reject:
	RTW_INFO("[%s] setup TWT config error\n", __func__);
	return;
}

void rtw_core_twt_test_cmd_testcase2(_adapter *padapter)
{
	enum rtw_phl_status status = RTW_PHL_STATUS_SUCCESS;
	struct sta_info *psta1 = NULL;
	struct sta_info *psta2 = NULL;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct rtw_phl_twt_setup_info twt_setup_info;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	void *phl = padapter->dvobj->phl;
	u8 twt_config_id = TWT_CONFIG_ID_INVALID;
	u8 flow_id = 0;
	u32 twt_wake_int_exp = 14;
	u32 wait_second = padapter->twt_cmd_test_wait_second;
	u32 target_wake_t_l_macid1 = 0;
	struct rtw_phl_ax_ul_fixinfo info;

	while (wait_second) {
		wait_second = wait_second>>1;
		twt_wake_int_exp++;
	}
	RTW_INFO("twt_wake_int_exp = %d\n", twt_wake_int_exp);

	if (!padapter->registrypriv.twt_enable)
		goto issue_twt_reject;

	psta1 = rtw_get_stainfo_by_macid(pstapriv, padapter->twt_cmd_test_macid1);
	if (!psta1) {
		printk("[%s]STA macid %d does not exist!\n", __func__, padapter->twt_cmd_test_macid1);
		goto issue_twt_reject;
	}

	psta2 = rtw_get_stainfo_by_macid(pstapriv, padapter->twt_cmd_test_macid2);
	if (!psta2) {
		printk("[%s]STA macid %d does not exist!\n", __func__, padapter->twt_cmd_test_macid2);
		goto issue_twt_reject;
	}

	while(1) {
		if (!(padapter->registrypriv.twt_dbgmode) &&
			!(psta1->phl_sta->asoc_cap.twt & BIT(0))) {
			printk("[%s]STA macid %d does not support TWT requester\n", __func__, padapter->twt_cmd_test_macid1);
			break;
		}

		/* initialize a TWT setup info, fill in some parameters*/
		do {
				pmlmeinfo->dialogToken++;
		} while (pmlmeinfo->dialogToken == 0);
		twt_setup_info.dialog_token = pmlmeinfo->dialogToken;
		/* hardcode parameters for individual TWT */
		twt_setup_info.twt_element.info.i_twt_para_set.twt_channel = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_setup_cmd = RTW_PHL_SUGGEST_TWT;
		twt_setup_info.twt_element.twt_ctrl.nego_type = RTW_PHL_INDIV_TWT;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_protection = 0;
		twt_setup_info.twt_element.twt_ctrl.responder_pm_mode = 0;
		twt_setup_info.twt_element.twt_ctrl.twt_info_frame_disable = 1;

		twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.implicit = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_h = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.nom_min_twt_wake_dur = 255;
		twt_setup_info.twt_element.info.i_twt_para_set.twt_wake_int_mantissa = 30;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp = twt_wake_int_exp;

		_twt_fill_default_setup_para(padapter, &twt_setup_info);
		flow_id = twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id;
		_twt_debug_print_info(padapter,twt_setup_info.twt_element, __func__);
		target_wake_t_l_macid1 = twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l;

		printk("[%s] setup TWT config for STA " MAC_FMT "\n", __func__, MAC_ARG(psta1->phl_sta->mac_addr));

		status = rtw_phl_twt_alloc_twt_config(phl, wrole, twt_setup_info, 1, &twt_config_id);
		if (status != RTW_PHL_STATUS_SUCCESS)
			goto issue_twt_reject;
		printk("[%s] setup TWT config done\n", __func__);

		if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger) {
			_twt_fill_default_ul_fixinfo(padapter, psta1, &info);
			status = rtw_phl_cmd_set_fw_ul_fixinfo(phl, wrole, &info, PHL_CMD_DIRECTLY, 0);
			if (status != RTW_PHL_STATUS_SUCCESS) {
				RTW_ERR("[%s]rtw_phl_cmd_set_fw_ul_fixinfo fail!\n", __func__);
				goto issue_twt_reject;
			}
		}

		status = rtw_phl_twt_add_sta_info(phl, psta1->phl_sta, twt_config_id, flow_id);
		if (status != RTW_PHL_STATUS_SUCCESS)
			goto issue_twt_reject;
		printk("[%s] add STA to TWT config done\n", __func__);

		_core_twt_update_local_setup_info(padapter, psta1, &twt_setup_info, twt_config_id);

		rtw_issue_twt_setup(padapter, psta1, twt_setup_info, RTW_PHL_ACCEPT_TWT, twt_config_id);
		printk("[%s] issue TWT setup resp frame done\n", __func__);

#ifdef CONFIG_LMT_TXREQ
		psta1->lmt_pending_txreq = padapter->max_tx_ring_cnt >> 3;
#endif
		rtw_core_twt_test_cmd_pwrbit(padapter, psta1->phl_sta->macid, 1);

		break;
	}

	while(1) {
		if (!(padapter->registrypriv.twt_dbgmode) &&
			!(psta2->phl_sta->asoc_cap.twt & BIT(0))) {
			printk("[%s]STA macid %d does not support TWT requester\n", __func__, padapter->twt_cmd_test_macid2);
			break;
		}

		/* initialize a TWT setup info, fill in some parameters*/
		do {
				pmlmeinfo->dialogToken++;
		} while (pmlmeinfo->dialogToken == 0);
		twt_setup_info.dialog_token = pmlmeinfo->dialogToken;
		/* hardcode parameters for individual TWT */
		twt_setup_info.twt_element.info.i_twt_para_set.twt_channel = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_request = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_setup_cmd = RTW_PHL_SUGGEST_TWT;
		twt_setup_info.twt_element.twt_ctrl.nego_type = RTW_PHL_INDIV_TWT;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_protection = 0;
		twt_setup_info.twt_element.twt_ctrl.responder_pm_mode = 0;
		twt_setup_info.twt_element.twt_ctrl.twt_info_frame_disable = 1;

		twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.implicit = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.flow_type = 1;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id = 2;
		twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_h = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l = 0;
		twt_setup_info.twt_element.info.i_twt_para_set.nom_min_twt_wake_dur = 255;
		twt_setup_info.twt_element.info.i_twt_para_set.twt_wake_int_mantissa = 30;
		twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_wake_int_exp = twt_wake_int_exp;

		_twt_fill_default_setup_para(padapter, &twt_setup_info);
		if (padapter->twt_cmd_test_overlap && target_wake_t_l_macid1) {
			twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l =  target_wake_t_l_macid1 + 32*1000;
		} else {
			twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l =  target_wake_t_l_macid1 + 500*1000;
		}

		printk("macid1 = %08X, macid2 = %08X\n", target_wake_t_l_macid1, twt_setup_info.twt_element.info.i_twt_para_set.target_wake_t_l);


		flow_id = twt_setup_info.twt_element.info.i_twt_para_set.req_type.twt_flow_id;
		_twt_debug_print_info(padapter,twt_setup_info.twt_element, __func__);

		printk("[%s] setup TWT config for STA " MAC_FMT "\n", __func__, MAC_ARG(psta2->phl_sta->mac_addr));

		status = rtw_phl_twt_alloc_twt_config(phl, wrole, twt_setup_info, 1, &twt_config_id);
		if (status != RTW_PHL_STATUS_SUCCESS)
			goto issue_twt_reject;
		printk("[%s] setup TWT config done\n", __func__);

		if (twt_setup_info.twt_element.info.i_twt_para_set.req_type.trigger) {
			_twt_fill_default_ul_fixinfo(padapter, psta2, &info);
			status = rtw_phl_cmd_set_fw_ul_fixinfo(phl, wrole, &info, PHL_CMD_DIRECTLY, 0);
			if (status != RTW_PHL_STATUS_SUCCESS) {
				RTW_ERR("[%s]rtw_phl_cmd_set_fw_ul_fixinfo fail!\n", __func__);
				goto issue_twt_reject;
			}
		}

		status = rtw_phl_twt_add_sta_info(phl, psta2->phl_sta, twt_config_id, flow_id);
		if (status != RTW_PHL_STATUS_SUCCESS)
			goto issue_twt_reject;
		printk("[%s] add STA to TWT config done\n", __func__);

		_core_twt_update_local_setup_info(padapter, psta2, &twt_setup_info, twt_config_id);

		rtw_issue_twt_setup(padapter, psta2, twt_setup_info, RTW_PHL_ACCEPT_TWT, twt_config_id);
		printk("[%s] issue TWT setup resp frame done\n", __func__);

#ifdef CONFIG_LMT_TXREQ
		psta2->lmt_pending_txreq = padapter->max_tx_ring_cnt >> 3;
#endif
		rtw_core_twt_test_cmd_pwrbit(padapter, psta2->phl_sta->macid, 1);

		break;
	}

	return;

issue_twt_reject:
	RTW_INFO("[%s] setup TWT config error\n", __func__);
	return;
}

void rtw_core_twt_test_cmd_testcase3(_adapter *padapter)
{
	rtw_core_twt_test_cmd_testcase2(padapter);
}

#endif

#endif /* CONFIG_RTW_TWT */
