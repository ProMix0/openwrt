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
#define _RTW_PE_OFLD_C_
#include <drv_types.h>
/* to avoid mac_addr type conflict, use extern in .h */
/* #include <rtk_fc_api.h> */

#ifdef CONFIG_ETHER_PKT_AGG
struct rtw_pe_ofld_entries rtw_pe_entry = {0};

void rtw_init_pe_entry(struct dvobj_priv *dvobj)
{
	if (!rtw_pe_entry.init) {
		rtw_pe_entry.init = _TRUE;
		_rtw_spinlock_init(&rtw_pe_entry.lock);
	}
}

u32 rtw_iterate_pe_entry(u32 index, u8 *buf, u32 len)
{
	struct rtw_pe_ofld_entry *pe_entry = rtw_pe_entry.entry;
	struct rtw_pe_ofld_entry *pentry;
	u32 ret = 0;

	if (rtw_pe_entry.init == _TRUE) {
		if (index < RTW_PE_ENTRY_NUM) {
			pentry = &pe_entry[index];
			if (pentry->used)
				snprintf(buf, len, "%-2d     %6d     %5d", index+1, pentry->dev_id, pentry->macid);
			else
				buf[0] = 0;
		}

		if (index < RTW_PE_ENTRY_NUM - 1)
			ret = index + 1;
	}
	return ret;
}

void rtw_get_pe_entry_agg_sta_lmt(u8 *buf, u32 len)
{
	if (!rtw_pe_entry.init) {
		buf[0] = 0;
		return;
	}
	snprintf(buf, len, "agg_sta_lmt=%d", rtw_pe_entry.agg_sta_lmt);
}

void rtw_get_pe_entry_num_entry(u8 *buf, u32 len)
{
	if (!rtw_pe_entry.init) {
		buf[0] = 0;
		return;
	}
	snprintf(buf, len, "num_entry=%d", rtw_pe_entry.num_entry);
}

void rtw_get_pe_entry_pe_ofld_mode(u8 *buf, u32 len)
{
	if (!rtw_pe_entry.init) {
		buf[0] = 0;
		return;
	}
	snprintf(buf, len, "pe_ofld_mode=%s", (rtw_pe_entry.pe_ofld_mode == 1) ? "TC MODE" : (rtw_pe_entry.pe_ofld_mode == 2) ? "WMM MODE" : "OFLD OFF");
}

static u32 rtw_alloc_valid_pe_entry(u8 dev_id, u16 macid)
{
	struct rtw_pe_ofld_entry *pe_entry = rtw_pe_entry.entry;
	struct rtw_pe_ofld_entry *pentry;
	u32 i, ret = 0;

	if (rtw_pe_entry.num_entry >= rtw_pe_entry.agg_sta_lmt) {
		return ret;
	}

	for (i = 0; i < rtw_pe_entry.agg_sta_lmt; i++) {
		pentry = &pe_entry[i];
		
		if (!pentry->used) {
			pentry->dev_id = dev_id;
			pentry->macid = macid;
			pentry->used = _TRUE;
			rtw_pe_entry.num_entry++;
			break;
		}
	}

	if (i < rtw_pe_entry.agg_sta_lmt)
		ret = i + 1;
	return ret;
}

static u32 rtw_search_pe_entry(u8 dev_id, u16 macid)
{
	struct rtw_pe_ofld_entry *pe_entry = rtw_pe_entry.entry;
	struct rtw_pe_ofld_entry *pentry;
	u32 i, index = 0;

	for (i = 0; i < rtw_pe_entry.agg_sta_lmt; i++) {
		pentry = &pe_entry[i];

		if (pentry->used &&
			pentry->dev_id == dev_id &&
			pentry->macid == macid) {
			break;
		}
	}

	if (i < rtw_pe_entry.agg_sta_lmt)
		index = i + 1;

	return index;
}

static u32 rtw_free_pe_entry(u8 dev_id, u16 macid)
{
	struct rtw_pe_ofld_entry *pe_entry = rtw_pe_entry.entry;
	struct rtw_pe_ofld_entry *pentry;
	u16 i, ret = 0;

	if (rtw_pe_entry.num_entry == 0) {
		return ret;
	}

	for (i = 0; i < rtw_pe_entry.agg_sta_lmt; i++) {
		pentry = &pe_entry[i];

		if (pentry->used &&
				pentry->dev_id == dev_id &&
				pentry->macid == macid) {
			pentry->dev_id = 0;
			pentry->macid = 0;
			pentry->used = _FALSE;
			rtw_pe_entry.num_entry--;
			break;
		}
	}

	if (i < rtw_pe_entry.agg_sta_lmt)
		ret = i + 1;
	return ret;
}

static bool rtw_allow_pe_ofld(_adapter *padapter)
{
	if (padapter &&
			(is_supported_5g(padapter->registrypriv.band_type) || is_supported_6g(padapter->registrypriv.band_type)))
		return _TRUE;
	return _FALSE;
}

u32 rtw_amsdu_pe_offload_mac_id_set(struct sta_info *psta)
{
	_adapter *adapter = psta->padapter;
	struct dvobj_priv *dvobj;
	u32 index;
	u8 dev_id = 0;
	rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel;
	rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf;
	u8 *pmac_addr;

	if (!adapter || !adapter->pnetdev)
		return 0;

	if (!rtw_allow_pe_ofld(adapter))
		return 0;

	_rtw_spinlock_bh(&rtw_pe_entry.lock);

	if (rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_OFF) {
		_rtw_spinunlock_bh(&rtw_pe_entry.lock);
		return 0;
	}

	dvobj = adapter_to_dvobj(adapter);
#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
	dev_id = dvobj->dev_id;
#endif

	index = rtw_alloc_valid_pe_entry(dev_id, psta->phl_sta->macid);
	if (!index) {
		_rtw_spinunlock_bh(&rtw_pe_entry.lock);
		return 0;
	}

	_rtw_memset(&sta_conf_sel, 0, sizeof(rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t));
	_rtw_memset(&sta_conf, 0, sizeof(rtk_fc_wifi_amsdu_pe_offload_sta_info_t));

	if (MLME_IS_STA(psta->padapter))
		pmac_addr = psta->padapter->mac_addr;
	else
		pmac_addr = psta->phl_sta->mac_addr;

	sta_conf_sel.power_saving = 0;
	sta_conf.power_saving = 0;
#ifdef AMSDU_PE_OFFLD_WIFI_PRI_OFFLD_EN_BITMSK_SUPPORT
	sta_conf_sel.wifi_pri_offld_en_bitmsk = 1;
	if (rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_TC_MODE) {
		sta_conf.wifi_pri_offld_en_bitmsk = 0x09; /* 8'b00001001 */
	} else if (rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_WMM_MODE) {
		sta_conf.wifi_pri_offld_en_bitmsk = 0xff; /* 8'b11111111 */
	}
#endif
	sta_conf_sel.amsdu_pkt_size = 1;
	sta_conf.amsdu_pkt_size = psta->txsc_amsdu_size;
	sta_conf_sel.p_wifi_dev = 1;
	sta_conf.p_wifi_dev = adapter->pnetdev;
	rtk_fc_wifi_amsdu_pe_offload_mac_id_set(index, sta_conf_sel, sta_conf, pmac_addr);

	if (!psta->fc_registered) {
		psta->fc_registered = 1;
		atomic_inc(&dvobj->fc_registered_sta_num);
	} else {
		RTW_ERR("[%s] pst->fc_registered != 0\n", __func__);
	}

	_rtw_spinunlock_bh(&rtw_pe_entry.lock);

	return index;
}

void rtw_amsdu_pe_offload_mac_id_del(struct sta_info *psta)
{
	_adapter *adapter = psta->padapter;
	struct dvobj_priv *dvobj;
	u32 index;
	u8 dev_id = 0;

	if (!psta->fc_registered)
		return;

	if (!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);
#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
	dev_id = dvobj->dev_id;
#endif

	_rtw_spinlock_bh(&rtw_pe_entry.lock);

	index = rtw_free_pe_entry(dev_id, psta->phl_sta->macid);
	if (!index) {
		RTW_ERR("[%s]: index = 0\n", __func__);
		_rtw_spinunlock_bh(&rtw_pe_entry.lock);
		return;
	}

	rtk_fc_wifi_amsdu_pe_offload_mac_id_del(index);
	psta->fc_registered = 0;
	atomic_dec(&dvobj->fc_registered_sta_num);

	_rtw_spinunlock_bh(&rtw_pe_entry.lock);
}

void rtw_amsdu_pe_offload_mac_id_update(struct sta_info *psta)
{
	_adapter *adapter = psta->padapter;
	struct dvobj_priv *dvobj;
	u32 index;
	u8 dev_id = 0;
	rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel = {0};
	rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf = {0};

	if (!adapter)
		return;

	if (!rtw_allow_pe_ofld(adapter))
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (dvobj->tx_mode == 1)
		return;

#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
	dev_id = dvobj->dev_id;
#endif

	_rtw_spinlock_bh(&rtw_pe_entry.lock);

	if (psta->traffic_mode == TRAFFIC_MODE_RX) {
		if (psta->fc_registered) {
			index = rtw_free_pe_entry(dev_id, psta->phl_sta->macid);

			if (index) {
				rtk_fc_wifi_amsdu_pe_offload_mac_id_del(index);
				psta->fc_registered = 0;
				atomic_dec(&dvobj->fc_registered_sta_num);
			} else {
				RTW_ERR("[%s]: index = 0\n", __func__);
			}
		}
	} else if (psta->txsc_amsdu_size) {
		if (psta->fc_registered)
			index = rtw_search_pe_entry(dev_id, psta->phl_sta->macid);
		else
			index = rtw_alloc_valid_pe_entry(dev_id, psta->phl_sta->macid);

		if (index) {
			sta_conf_sel.power_saving = 0;
			sta_conf.power_saving = 0;
			#ifdef AMSDU_PE_OFFLD_WIFI_PRI_OFFLD_EN_BITMSK_SUPPORT
			sta_conf_sel.wifi_pri_offld_en_bitmsk = 1;
			if (rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_TC_MODE) {
				sta_conf.wifi_pri_offld_en_bitmsk = 0x09; /* 8'b00001001 */
			} else if (rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_WMM_MODE) {
				sta_conf.wifi_pri_offld_en_bitmsk = 0xff; /* 8'b11111111 */
			}
			#endif
			sta_conf_sel.amsdu_pkt_size = 1;
			sta_conf.amsdu_pkt_size = psta->txsc_amsdu_size;
			sta_conf_sel.p_wifi_dev = 1;
			sta_conf.p_wifi_dev = adapter->pnetdev;
			rtk_fc_wifi_amsdu_pe_offload_mac_id_set(index, sta_conf_sel, sta_conf, psta->phl_sta->mac_addr);

			if (!psta->fc_registered) {
				psta->fc_registered = 1;
				atomic_inc(&dvobj->fc_registered_sta_num);
			}
		}
	} else {
		if (psta->fc_registered) {
			index = rtw_free_pe_entry(dev_id, psta->phl_sta->macid);

			if (index) {
				rtk_fc_wifi_amsdu_pe_offload_mac_id_del(index);
				psta->fc_registered = 0;
				atomic_dec(&dvobj->fc_registered_sta_num);
			} else {
				RTW_ERR("[%s]: index = 0\n", __func__);
			}
		}
	}

	_rtw_spinunlock_bh(&rtw_pe_entry.lock);
}

static void rtw_reset_pe_entry(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct sta_info *psta = NULL;
	struct rtw_pe_ofld_entry *pe_entry = rtw_pe_entry.entry;
	struct rtw_pe_ofld_entry *pentry;
	u16 i;

	if (!rtw_pe_entry.init) {
		rtw_warn_on(1);
		return;
	}

	rtw_pe_entry.agg_sta_lmt = RTW_PE_ENTRY_NUM;
	rtw_pe_entry.num_entry = 0;

	for (i = 0; i < RTW_PE_ENTRY_NUM; i++) {
		pentry = &pe_entry[i];
		if (pentry->used == _TRUE) {
			rtk_fc_wifi_amsdu_pe_offload_mac_id_del(i+1);
			psta = rtw_get_stainfo_by_macid(&padapter->stapriv, pentry->macid);
			if (psta && psta->fc_registered) {
				psta->fc_registered = 0;
				atomic_dec(&dvobj->fc_registered_sta_num);
			}
		}
		pentry->dev_id = 0;
		pentry->macid = 0;
		pentry->used = _FALSE;
	}
}

static void rtw_pe_ofld_off_set(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	rtw_pe_entry.pe_ofld_mode = RTW_PE_OFLD_OFF;
	rtw_pe_entry.agg_sta_lmt = RTW_PE_OFLD_OFF_AGG_STA_LMT;
	rtk_fc_wifi_amsdu_pe_offload_mode_set(RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64);
}

static void rtw_pe_ofld_tc_mode_set(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	rtw_pe_entry.pe_ofld_mode = RTW_PE_OFLD_TC_MODE;
	rtw_pe_entry.agg_sta_lmt = RTW_PE_OFLD_TC_MODE_AGG_STA_LMT;
	rtk_fc_wifi_amsdu_pe_offload_mode_set(RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_TC_EPP64);
}

static void rtw_pe_ofld_wmm_mode_set(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	rtw_pe_entry.pe_ofld_mode = RTW_PE_OFLD_WMM_MODE;
	rtw_pe_entry.agg_sta_lmt = RTW_PE_OFLD_WMM_MODE_AGG_STA_LMT;
	rtk_fc_wifi_amsdu_pe_offload_mode_set(RTK_FC_WIFI_AMSDU_PE_OFFLOAD_MODE_WMM_EPP64);
}

void rtw_pe_ofld_mode_init(_adapter *padapter)
{
	if (!padapter || !is_primary_adapter(padapter) || !rtw_allow_pe_ofld(padapter))
		return;

	_rtw_spinlock_bh(&rtw_pe_entry.lock);

	RTW_PRINT("[%s]: reset pe_entry\n", __func__);
	rtw_reset_pe_entry(padapter);

	if (padapter->registrypriv.wifi_mib.pe_ofld_mode == RTW_PE_OFLD_TC_MODE) {
		RTW_PRINT("[%s]: set PE OFLD TC mode\n", __func__);
		rtw_pe_ofld_tc_mode_set(padapter);
	} else if (padapter->registrypriv.wifi_mib.pe_ofld_mode == RTW_PE_OFLD_WMM_MODE) {
		RTW_PRINT("[%s]: set PE OFLD WMM mode\n", __func__);
		rtw_pe_ofld_wmm_mode_set(padapter);
	} else {
		RTW_PRINT("[%s]: set PE OFLD OFF\n", __func__);
		rtw_pe_ofld_off_set(padapter);
	}

	_rtw_spinunlock_bh(&rtw_pe_entry.lock);
}

bool rtw_pe_ofld_used(_adapter *padapter)
{
	if (padapter &&
			(is_supported_5g(padapter->registrypriv.band_type) || is_supported_6g(padapter->registrypriv.band_type)) &&
			(rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_TC_MODE || rtw_pe_entry.pe_ofld_mode == RTW_PE_OFLD_WMM_MODE))
		return _TRUE;
	return _FALSE;
}

static void rtw_pe_ofld_check_and_step_to_next_frag(struct sk_buff *pkt, struct sk_buff **frag, u32 skip, u32 *skip_frag_len, u32 *next_skip_frag_len)
{
	u32 frag_len;
	if (skip >= *next_skip_frag_len) {
		if (*frag == NULL)
			*frag = pkt;
		else
			*frag = (*frag == pkt) ? skb_shinfo(pkt)->frag_list : (*frag)->next;

		*skip_frag_len = *next_skip_frag_len;
		if (*frag == NULL) {
			RTW_DBG("%s: no more frage, skip: %u, skip_frag_len: %u, next_skip_frag_len: %u", __func__, skip, *skip_frag_len, *next_skip_frag_len);
			return; /* no more frags */
		}
		frag_len = (*frag)->len - (*frag)->data_len;
		*next_skip_frag_len = *skip_frag_len + frag_len;
		RTW_DBG("%s: frag->len: %u, frag->data_len: %u, frag_len: %u, skip: %u, skip_frag_len: %u, next_skip_frag_len: %u", __func__, (*frag)->len, (*frag)->data_len, frag_len, skip, *skip_frag_len, *next_skip_frag_len);
	}
}

static int rtw_pe_ofld_count_linear_skb_amsdu_subframes(struct sk_buff *pkt, struct pe_ofld_pktinfo_t *pktinfo)
{
	int count = 0;
	struct sk_buff *frag = pkt;
	u32 skip = 0;
	u8 *len_field_low = NULL;
	u8 *len_field_high = NULL;
	u32 msdu_len = 0;
	u32 padding_len = 0;

	if (skb_is_nonlinear(pkt))
		return -1;

	while (skip < pkt->len) {
		/* skip da field */
		skip += ETH_ALEN;
		/* skip sa field */
		skip += ETH_ALEN;
		/* scan and skip length field */
		len_field_high = frag->data + skip;
		skip += 1;
		len_field_low = frag->data + skip;
		skip += 1;
		/* evaluate msdu length and skip msdu field */
		msdu_len = (*len_field_high) & 0xff;
		msdu_len <<= 8;
		msdu_len |= ((*len_field_low) & 0xff);
		skip += msdu_len;
		/* evaluate padding length and skip padding field */
		padding_len = 4 - (skip & 0x3);
		padding_len = (padding_len == 4) ? 0 : padding_len;
		skip += padding_len;
		/* get a amsdu subframe */
		count++;
		/* fill pkt info */
		if (pktinfo) {
			pktinfo->subframes++;
			pktinfo->sfhdr_bytes += 14;
			pktinfo->msdu_bytes += msdu_len;
			pktinfo->pad_bytes += ((skip < pkt->len) ? padding_len : 0);
		}
	}

	if (pktinfo) {
		pktinfo->amsdu_bytes = pkt->len;
	}

	return count;
}

static int rtw_pe_ofld_count_amsdu_subframes(struct sk_buff *pkt, struct pe_ofld_pktinfo_t *pktinfo)
{
	int count = 0;
	struct sk_buff* frag;
	u32 skip = 0;
	u32 msdu_len = 0;
	u32 padding_len = 0;
	u8 *len_field_low;
	u8 *len_field_high;
	u32 skip_frag_len = 0;
	u32 next_skip_frag_len = 0;

	if (!skb_is_nonlinear(pkt)) {
		count = rtw_pe_ofld_count_linear_skb_amsdu_subframes(pkt, pktinfo);
		return count;
	}

	frag = NULL;
	rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
	while (skip < pkt->len) {
		/* skip da field */
		skip += ETH_ALEN;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		/* skip sa field */
		skip += ETH_ALEN;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		/* scan and skip length field */
		len_field_high = frag->data + (skip - skip_frag_len);
		RTW_DBG("%s: len_field_high: %u\n", __func__, (u32) *len_field_high);
		skip += 1;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		len_field_low = frag->data + (skip - skip_frag_len);
		RTW_DBG("%s: len_field_low: %u\n", __func__, (u32) *len_field_low);
		skip += 1;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		/* evaluate msdu length and skip msdu field */
		msdu_len = (*len_field_high) & 0xff;
		msdu_len <<= 8;
		msdu_len |= ((*len_field_low) & 0xff);
		RTW_DBG("%s: msdu_len: %u\n", __func__, msdu_len);
		skip += msdu_len;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		/* evaluate padding length and skip padding field */
		padding_len = 4 - (skip & 0x3);
		padding_len = (padding_len == 4) ? 0 : padding_len;
		RTW_DBG("%s: padding_len: %u\n", __func__, padding_len);
		skip += padding_len;
		rtw_pe_ofld_check_and_step_to_next_frag(pkt, &frag, skip, &skip_frag_len, &next_skip_frag_len);
		RTW_DBG("%s: skip: %u\n", __func__, skip);
		/* get a amsdu subframe */
		count++;
		RTW_DBG("%s: count: %u\n", __func__, count);
		/* fill pkt info */
		if (pktinfo) {
			pktinfo->subframes++;
			pktinfo->sfhdr_bytes += 14;
			pktinfo->msdu_bytes += msdu_len;
			pktinfo->pad_bytes += ((skip < pkt->len) ? padding_len : 0);
		}
	}

	if (pktinfo) {
		pktinfo->amsdu_bytes = pkt->len;
	}

	return count;
}

void rtw_pe_ofld_dbg_pkt_cnt(struct sk_buff *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct dvobj_priv *dvobj = padapter->dvobj;
	int subframe_count = 0;
	struct pe_ofld_pktinfo_t pktinfo;

	_rtw_memset(&pktinfo, 0, sizeof(struct pe_ofld_pktinfo_t));

	subframe_count = rtw_pe_ofld_count_amsdu_subframes(pkt, &pktinfo);
	if (subframe_count <= 0) {
		pxmitpriv->pe_ofld_tx_broken_amsdus++;
		return;
	}
	pxmitpriv->pe_ofld_tx_amsdus++;
	pxmitpriv->pe_ofld_tx_amsdu_subframes += subframe_count;
	pxmitpriv->pe_ofld_tx_amsdu_bytes += pktinfo.amsdu_bytes;
	pxmitpriv->pe_ofld_tx_msdu_bytes += pktinfo.msdu_bytes;
	pxmitpriv->pe_ofld_tx_amsdu_subframe_dist[subframe_count > AMSDU_SUBFRAME_DIST_LEN ? AMSDU_SUBFRAME_DIST_LEN - 1 : subframe_count - 1]++;
	if (pktinfo.amsdu_bytes != (pktinfo.sfhdr_bytes + pktinfo.msdu_bytes + pktinfo.pad_bytes))
		pxmitpriv->pe_ofld_tx_mismatch_amsdus++;
}

void rtw_pe_ofld_dbg_non_amsdu_cnt(struct sk_buff *pkt, _nic_hdl pnetdev)
{
#define CHECK_VW_LEN(len, target) (((len) == (target) - 4) || ((len) == (target) - 8))

	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if (CHECK_VW_LEN(pkt->len, 1518) ||
		CHECK_VW_LEN(pkt->len, 512)) {
		pxmitpriv->pe_ofld_tx_non_amsdu++;
		pxmitpriv->pe_ofld_tx_non_amsdu_bytes += pkt->len;
	}
}

u32 rtw_pe_ofld_parse_msdu_len(struct sk_buff *pkt)
{
	u32 msdu_len;
	if (pkt->cb[_SKB_CB_ETH_AGG] != _PKT_TYPE_AGG_MULTI_PKT)
		return pkt->len;
	msdu_len = *(u8*)(pkt->data + 12) & 0xff;
	msdu_len <<= 8;
	msdu_len |= *(u8*)(pkt->data + 13) & 0xff;
	return msdu_len;
}

void rtw_pe_ofld_set_sta_cnt_frame_sz_need_print_setting(_adapter *padapter, struct sk_buff *pkt)
{
	/* Record STA cnt and frame sz for TC settings */
	if (padapter->dvobj->tx_mode == 1) {
		u32 frame_sz= rtw_pe_ofld_parse_msdu_len(pkt);

		/* categorize frame sz into 88B, 512B, and 1518B */
		if (74 <= frame_sz && frame_sz <= 88)
			frame_sz = 88;
		else if (498 <= frame_sz && frame_sz <= 512)
			frame_sz = 512;
		else if (1504 <= frame_sz && frame_sz <= 1518)
			frame_sz = 1518;

		/* only deal with pkt of 88B, 512B, or 1518B */
		if (frame_sz == 88 || frame_sz == 512 || frame_sz == 1518) {
			if (frame_sz != padapter->tc_setting_frame_sz || padapter->stapriv.asoc_list_cnt != padapter->tc_setting_sta_cnt) {
				padapter->tc_setting_frame_sz = frame_sz;
				padapter->tc_setting_sta_cnt = padapter->stapriv.asoc_list_cnt;
				padapter->need_print_setting = 1;
			}
		}
	}
}
#endif

#if defined(CONFIG_ETHER_PKT_AGG) || defined(CONFIG_RTL_WIFI_OFFLOAD)
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
int rtw_wfo_fc_igmp_learning(_adapter *padapter, struct sk_buff *skb)
{
	int update = 0;
	unsigned short L3_protocol = 0;
	struct sta_priv	*pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	u8 *mac = NULL;
	u32 old_ipmc_num = 0, new_ipmc_num = 0;

	if (!skb || !skb->data)
		return update;

	mac = skb->data + ETH_ALEN;
	psta = rtw_get_stainfo(pstapriv, mac);
#ifdef CONFIG_RTW_A4_STA
	if (padapter->a4_enable && !psta) {
		psta = core_a4_get_fwd_sta(padapter, mac);
	}
#endif

	if (!psta)
		return update;

	old_ipmc_num = psta->ipmc_num;

	L3_protocol = *(unsigned short *)(skb->data + 2*ETH_ALEN);
	if (L3_protocol == __constant_htons(ETH_P_8021Q))
		L3_protocol = *(unsigned short *)(skb->data + 2*ETH_ALEN + 4);

	if (L3_protocol == __constant_htons(ETH_P_IP)) {
		rtw_igmp_type_check(pstapriv, skb);
	} else if (L3_protocol == __constant_htons(ETH_P_IPV6)) {
		rtw_mld_type_check(pstapriv, skb);
	}

	new_ipmc_num = psta->ipmc_num;
	if (old_ipmc_num != new_ipmc_num)
		update = 1;

	return update;
}
#endif

#ifdef CONFIG_TX_MCAST2UNI
void rtw_wfo_fc_get_m2u_macList(_adapter *padapter, struct sk_buff *skb, rtk_egrPreFc_t *param)
{
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	int sta_list[NUM_STA] = {0};
	u8 sta_list_cnt = 0, mac_list_cnt = 0, check_list_num = 0;
	int i;
	int ret = 0;
#if defined(CONFIG_RTW_A4_STA) && defined(RTK_FC_WFO_A4_STA_M2U_REFINE)	
	u8 non_pe_a4_sta_exist = 0;
#endif

	ret = rtw_get_m2u_maclist(padapter, skb, &sta_list[0], &sta_list_cnt);
	if (ret != _FALSE && sta_list_cnt) {
		check_list_num = sta_list_cnt;
		if (check_list_num > MAX_STA_MAC)
			check_list_num = MAX_STA_MAC;
		for (i = 0; i < check_list_num; i++) {
			psta = rtw_get_stainfo_by_offset(pstapriv, sta_list[i]);
			if (psta && psta->phl_sta) {
#if defined(CONFIG_RTW_A4_STA) && defined(RTK_FC_WFO_A4_STA_M2U_REFINE)
				/* Just report one no-ofld A4 STA to FC for one MC packet to driver */
				if (psta->flags & WLAN_STA_A4 && 
					psta->fc_staid >= FC_STA_NUM &&
					non_pe_a4_sta_exist)
					continue;						
#endif
				_rtw_memcpy(param->macList[mac_list_cnt].mac, psta->phl_sta->mac_addr, ETH_ALEN);
				param->macList[mac_list_cnt].ctagif = 0;
				param->macList[mac_list_cnt].cvlan = 0;
				param->macList[mac_list_cnt].cpri_vld = 0;
				param->macList[mac_list_cnt].cpri = 0;
#if defined(CONFIG_RTW_A4_STA) && defined(RTK_FC_WFO_A4_STA_M2U_REFINE)
				if (psta->flags & WLAN_STA_A4) {
					param->macList[mac_list_cnt].is_a4 = 1;
					if (psta->fc_staid >= FC_STA_NUM)
						non_pe_a4_sta_exist = 1;
				}
#endif
				mac_list_cnt++;
			}
		}
		param->macListCnt = mac_list_cnt;
	}
	param->egrPreFc_cbAct |= RTK_FC_CB_MC2UC;
}
#endif

int rtl_wfo_fc_uc2mc_process(_adapter *padapter, struct sk_buff *pkt)
{
	int do_u2m = 0;
	unsigned short l3_protocol = 0;

	if (IP_MCAST_MAC(pkt->data) || ICMPV6_MCAST_MAC(pkt->data))
		return do_u2m;

	if (padapter->registrypriv.wifi_mib.fc_uc2mc_en) {
		l3_protocol = *(unsigned short *)(pkt->data + 2 * ETH_ALEN);
		if (l3_protocol == __constant_htons(ETH_P_8021Q))
			l3_protocol = *(unsigned short *)(pkt->data + 2 * ETH_ALEN + 4);

		if (l3_protocol == __constant_htons(ETH_P_IP))
			do_u2m = rtw_checkUDPandU2M(pkt);
		else if (l3_protocol == __constant_htons(ETH_P_IPV6))
			do_u2m = rtw_checkV6UDPandU2M(pkt);
	}

	return do_u2m;
}

int rtl_wfo_fc_nat25_process(_adapter *padapter, struct net_device *dev, struct sk_buff *skb, bool rx)
{
	u16 protocol;
	u8 *da, *dip, *sa, *sip, *ptr = NULL;
	int do_nat25 = 1;
	int ret = 0;
	void *br_port = NULL;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	extern void rx_dump_skb(struct sk_buff *skb);

	ptr = skb->data + ETH_ALEN * 2;
	protocol = *((unsigned short *)ptr);
	if (protocol == __constant_htons(ETH_P_8021Q)) {
		ptr += 4;
		protocol = *((unsigned short *)ptr);
	}
	#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
	br_port = padapter->pnetdev->br_port;
	#else
	rcu_read_lock();
	br_port = rcu_dereference(padapter->pnetdev->rx_handler_data);
	rcu_read_unlock();
	#endif
	if (padapter->registrypriv.wifi_mib.fc_nat25_en) {
		if (rx) { /* RX: replace DA with STA's MAC address */
#ifdef CONFIG_RTW_A4_STA
			RTW_DBG("DEBUG[%s]: if=%s, a4_enable=%u, "
				"(pmlmeinfo->state&WIFI_FW_ASSOC_SUCCESS)=0x%x, da=%pM, sa=%pM \n",
				__func__, ADPT_ARG(padapter), padapter->a4_enable,
				(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS),
				skb->data, skb->data+ETH_ALEN);
			if (!(padapter->a4_enable && (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)))
#endif
			{
				da = skb->data;
				dip = ptr + 2 + 16;

				if (!br_port)
					return ret;

				if (__constant_htons(ETH_P_EAPOL) == protocol)
					return ret;

				if (da[0] & 1)
					return ret;

				if ((__constant_htons(ETH_P_IP) == protocol) &&
				    	!memcmp(padapter->scdb_ip, dip, 4)) {
					_rtw_memcpy(da, padapter->scdb_mac, ETH_ALEN);
					ret = 1;
				} else {
					if(nat25_handle_frame(padapter, skb) == 0)
						ret = 1;
				}
			}
		} else { /* TX: replace SA with VXD's MAC address */
#ifdef CONFIG_RTW_A4_STA
			if (_TRUE == core_a4_check_tx(padapter, &skb))
				return ret;

			if (padapter->a4_enable &&
				(padapter->mlmeextpriv.mlmext_info.state & WIFI_FW_ASSOC_SUCCESS))
				return ret;
#endif
			if (br_port) {
				ret = 1;
			}
		}
	}

	return ret;
}

#ifdef CONFIG_RTW_A4_STA
int rtl_wfo_fc_lb_check(_adapter *padapter, struct net_device *dev, struct sk_buff *pkt)
{
	int ret = _FALSE;
	u8 *ptr = NULL;

#if (defined(RTK_WLAN_EVENT_INDICATE) || defined(CONFIG_WLAN_EVENT_INDICATE_GENL)) && defined(CONFIG_RTW_MULTI_AP)
	ptr = pkt->data + ETH_ALEN;
	if ((ptr[0] ==0xaa) && (ptr[1] ==0xbb) &&
		(ptr[2] ==0xcc) && (ptr[3] ==padapter->mac_addr[3]) &&
		(ptr[4] ==padapter->mac_addr[4]) && (ptr[5] ==padapter->mac_addr[5])) {

		RTW_INFO("%s: receive WIFI_EZMESH_ARP_LOOP in driver on if:%s, send event!!!\n",
		         __func__, padapter->pnetdev->name);
#if defined(RTK_WLAN_EVENT_INDICATE)
		rtk_wlan_event_indicate(padapter->pnetdev->name,
		                        WIFI_EZMESH_ARP_LOOP,
		                        adapter_mac_addr(padapter),
		                        0);
#endif
#if defined(CONFIG_WLAN_EVENT_INDICATE_GENL)
		rtk_wlan_genl_indicate(padapter->pnetdev->name, WIFI_EZMESH_ARP_LOOP, adapter_mac_addr(padapter), 0);
#endif
		return _TRUE;
	}
#endif
	if (EQ_MAC_ADDR(padapter->br_mac, pkt->data + ETH_ALEN)) {
		RTW_DBG("%s: interface %s, source addr: %pM, equals br0 mac\n",
			__func__, dev->name, pkt->data + ETH_ALEN);
		return _TRUE;
	}

	if (padapter && IS_MCAST(pkt->data)) {
		ret = rtw_check_a4_loop_entry(padapter, pkt->data + ETH_ALEN);
		RTW_DBG("DEBUG[%s]: interface %s, dst addr: %pM, src addr: %pM, mcast loop detect %s\n",
			__func__, dev->name, pkt->data, pkt->data + ETH_ALEN, ret ? "TRUE":"FALSE");
	}

	RTW_DBG("%s: interface %s, dst addr: %pM, src addr: %pM, loop detect %s\n",
		__func__, dev->name, pkt->data, pkt->data + ETH_ALEN, ret ? "TRUE":"FALSE");

	return ret;
}
#endif /* CONFIG_RTW_A4_STA */

int rtl_wfo_fc_ingress_pre_callback(struct net_device *dev, struct sk_buff *skb, rtk_igrPreFc_t *param)
{
	_adapter *padapter = NULL;
	int ret = 0;
	if (!dev || !skb || !skb->data || !param)
		return -1;
	padapter = (_adapter *)rtw_netdev_priv(dev);
	if (MLME_IS_AP(padapter)) {
#ifdef CONFIG_RTW_AP_EXT_SUPPORT
		if (IS_MCAST(skb->data)) {
			ret = rtw_wfo_fc_igmp_learning(padapter, skb);
			if (ret)
				param->igrPreFc_cbAct |= RTK_FC_CB_IGMP_LRN;
		}
#endif
	} else if (MLME_IS_STA(padapter))  {
#ifdef CONFIG_RTW_A4_STA
		if (padapter->a4_enable) {
			ret = rtl_wfo_fc_lb_check(padapter, dev, skb);
			if (ret)
				param->igrPreFc_cbAct |= RTK_FC_CB_LOOPBACK_DETECT_DROP;
		}
#endif
	}
	RTW_DBG("DEBUG[%s]: igrPreFc_cbAct=0x%x\n", __func__, param->igrPreFc_cbAct);
	return 0;
}

int rtl_wfo_fc_ingress_post_callback(struct net_device *dev, struct sk_buff *skb, rtk_igrPostFc_t *param)
{
	_adapter *padapter = NULL;
	int ret = 0;

	if (!dev || !skb || !skb->data || !param)
		return -1;
	padapter = (_adapter *)rtw_netdev_priv(dev);

	if (MLME_IS_STA(padapter)) {
		ret = rtl_wfo_fc_uc2mc_process(padapter, skb);
		if (ret)
			param->igrPostFc_cbAct |= RTK_FC_CB_UC2MC;

#ifdef CONFIG_BR_EXT
		ret = rtl_wfo_fc_nat25_process(padapter, dev, skb, 1);
		if (ret)
			param->igrPostFc_cbAct |= RTK_FC_CB_NAT25_DMAC_TRANS;
#endif
	}
	RTW_DBG("DEBUG[%s]: igrPostFc_cbAct=0x%x\n", __func__, param->igrPostFc_cbAct);
	return 0;
}

int rtl_wfo_fc_egress_pre_callback(struct net_device *dev, struct sk_buff *skb, rtk_egrPreFc_t *param)
{
	_adapter *padapter = NULL;
	int ret = 0;

	if (!dev || !skb || !skb->data || !param)
		return -1;
	padapter = (_adapter *)rtw_netdev_priv(dev);
	if (MLME_IS_AP(padapter)) {
#ifdef CONFIG_TX_MCAST2UNI
		if (IS_MCAST(skb->data))
			rtw_wfo_fc_get_m2u_macList(padapter, skb, param);
#endif
	} else if (MLME_IS_STA(padapter)) {
#ifdef CONFIG_BR_EXT
		ret = rtl_wfo_fc_nat25_process(padapter, dev, skb, 0);
		if (ret) {
			_rtw_memcpy(param->macList[0].mac, GET_MY_HWADDR(padapter), MACADDRLEN);
			param->macListCnt = 1;
			param->egrPreFc_cbAct |= RTK_FC_CB_NAT25_SMAC_TRANS;
		}
#endif
	}
	RTW_DBG("DEBUG[%s]: egrPreFc_cbAct=0x%x\n", __func__, param->egrPreFc_cbAct);
	return 0;
}

void rtl_wfo_fc_block_relay(struct net_device *dev, int block_relay)
{
	rtk_fc_wifi_dev_attr_t attr;

	rtk_fc_wifi_dev_attr_get(dev, &attr);

	if (block_relay)
		attr.block_relay = 1;
	else
		attr.block_relay = 0;

	rtk_fc_wifi_dev_attr_set(dev, attr);
}

void rtl_wfo_fc_mc2uc_enable(struct net_device *dev, int mc2uc_en)
{
	rtk_fc_internal_amsdu_pe_offload_mc2uc(mc2uc_en);
}

void rtl_wfo_fc_callback_hook_update(struct net_device *dev, int up)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(dev);
	rtk_fc_wifi_dev_attr_t attr;

	rtk_fc_wifi_dev_attr_get(dev, &attr);

	if (up) {
		attr.igrPreFc_cb = rtl_wfo_fc_ingress_pre_callback;
		attr.igrPostFc_cb = rtl_wfo_fc_ingress_post_callback;
		attr.egrPreFc_cb = rtl_wfo_fc_egress_pre_callback;
		if (MLME_IS_STA(padapter)) {
			attr.client_mode = 1;
			attr.do_nat25 = 1;
#ifdef CONFIG_RTW_A4_STA
			if (padapter->a4_enable)
				attr.do_nat25 = 0;
#endif
		}
	} else {
		attr.igrPreFc_cb = NULL;
		attr.igrPostFc_cb = NULL;
		attr.egrPreFc_cb = NULL;
		attr.client_mode = 0;
		attr.do_nat25 = 0;
	}

	RTW_PRINT("[%s]: ifname=%s, up=%d, client_mode=%u, do_nat25=%u\n",
		__func__, ADPT_ARG(padapter), up, attr.client_mode, attr.do_nat25);

	rtk_fc_wifi_dev_attr_set(dev, attr);
}

#endif /* defined(CONFIG_ETHER_PKT_AGG) || defined(CONFIG_RTL_WIFI_OFFLOAD) */

