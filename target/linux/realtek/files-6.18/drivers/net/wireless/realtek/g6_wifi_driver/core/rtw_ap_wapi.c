#if defined(CONFIG_RTL_CFG80211_WAPI_SUPPORT)

#include <linux/unistd.h>
#include <linux/etherdevice.h>
#include <drv_types.h>
#include <rtw_ap_wapi.h>

u8 DBG_WAPI_USK_UPDATE_ONCE = 0;
u8 DBG_WAPI_MSK_UPDATE_ONCE = 0;




u32 wapi_debug_component =
	/*				WAPI_INIT	|
	 *				WAPI_API	| */
	 				WAPI_TX	|
	 				WAPI_RX	| 
					WAPI_ERR ; /* always open err flags on */


void rtw_dump_buff(u8 *buff, uint len, char *str)
{
	int i;

	RTW_INFO("\n######Dump %s (len = %d):\n", str, len);

	for (i = 0; i < len; i++) {
		if (i && (i % 16 == 0))
			RTW_INFO("\n");

		RTW_INFO("%02x ", buff[i]);
	}
	RTW_INFO("\n"); 
}

/*	PN1 > PN2, return 1,
 *	else return 0.
 */
u8 rtw_wapi_compare_PN(u8 *PN1, u8 *PN2, u8 len)
{
	u8 i;

	/* overflow case	*/
	if ((PN2[len -1] - PN1[len - 1]) & 0x80)
		return 1;

	for (i = len; i > 0; i--) {
		if (PN1[i-1] == PN2[i-1])
			continue;
		else if(PN1[i-1] > PN2[i-1])
			return 1;
		else
			return 0;
	}
	return 0;
}


/* AddCount: 1 or 2.
 *	If overflow, return 1,
 *	else return 0.
 */

u8 rtw_wapi_increase_PN(u8 *PN, u8 len, u8 AddCount)
{
	u8  i;

	for (i = 0; i < len; i++) {
		if (PN[i] + AddCount <= 0xff) {
			PN[i] += AddCount;
			return 0;
		} else {
			PN[i] += AddCount;
			AddCount = 1;
		}
	}
	return 1;
}

void rtw_wapi_init(_adapter *padapter) 
{
	_rtw_memset(&padapter->wapiApInfo, 0, sizeof(RTL_WAPI_AP_INFO));
}


u8 rtw_wapi_is_wai_packet(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 offset = 0;
	u16 ver;
	u8 type;

	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

	offset = pattrib->hdrlen + LLC_HEADER_SIZE;

	if (*((u16 *)&ptr[offset]) != __constant_htons(ETH_P_WAPI))
		return 0;	
	offset += 2;

	ver = __constant_ntohs(*((u16 *)&ptr[offset]));
	offset += 2;

	if (ptr[offset] == WAI_PROTOCOL) {
		
		WAPI_TRACE(WAPI_RX, "%s: Recv WAI packet (subType = %d)\n", __FUNCTION__,ptr[offset + 1]);
		return 1;
	}

	return 0;
}

int rtw_wapi_is_data_packet(union recv_frame *precv_frame)

{
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

	u8 wlan_hdr_len = pattrib->hdrlen;
	
	u8 part_pn[] = {0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C};

	WLAN_HDR_WAPI_EXT *pwapiExt = (WLAN_HDR_WAPI_EXT *)(ptr + wlan_hdr_len);

	if ((pwapiExt->keyIdx == 0 || pwapiExt->keyIdx == 1) && pwapiExt->reserved == 0) {

		if (memcmp(&pwapiExt->PN[8], part_pn, 8) == 0)
			return 1;
	}
	
	return 0;
}

int rtw_wapi_is_icmp_packet(union recv_frame *precv_frame)

{
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	u8 wlan_hdr_len = pattrib->hdrlen;

	u8 llc[LLC_HEADER_SIZE] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
	u8 ip_hdr[4] = {0x08, 0x00, 0x45, 0x00};


	if (rtw_wapi_is_data_packet(precv_frame) == 0)
		return 0;

	if (memcmp(ptr + wlan_hdr_len + WAPI_EXT_HDR_LEN, llc, LLC_HEADER_SIZE) == 0 && 
			memcmp(ptr + wlan_hdr_len + WAPI_EXT_HDR_LEN + LLC_HEADER_SIZE , ip_hdr, 4) == 0 && 
			*(ptr + wlan_hdr_len + WAPI_EXT_HDR_LEN + LLC_HEADER_SIZE + 11) == 1
		)
		return 1;	
	
	return 0;
}


int rtw_wapi_is_arp_packet(union recv_frame *precv_frame)

{
	u16 *eth_protocol = NULL;
	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	u8 wlan_hdr_len = pattrib->hdrlen;

	if (rtw_wapi_is_data_packet(precv_frame) == 0)
		return 0;

	eth_protocol = (u16 *)(ptr + wlan_hdr_len + WAPI_EXT_HDR_LEN + LLC_HEADER_SIZE);	

	if (*eth_protocol == __constant_htons(ETH_P_ARP)) 
		return 1;
	
	return 0;
}


int rtw_validate_wapi_data_packet(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 pn_len;
	u8 tid, qosIdx = 0;
	struct sta_info *psta = precv_frame->u.hdr.psta;
	struct sta_priv *pstapriv = &padapter->stapriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;	
	RTL_WAPI_AP_INFO *pwapiApInfo = &padapter->wapiApInfo;
	RTL_WAPI_STA_INFO *pwapiStaInfo = NULL;

	u8 *ptr = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	u8 offset = pattrib->hdrlen;
	u8 data_offset = offset + WAPI_EXT_HDR_LEN;
	WLAN_HDR_WAPI_EXT *pwapiExt = (WLAN_HDR_WAPI_EXT *)(ptr + offset);
	u8 keyIdx = pwapiExt->keyIdx;
	u8 *recvPN = pwapiExt->PN;

	u8 *ra = pattrib->ra;
	u8 *ta = pattrib->ta;

	if (rtw_wapi_is_wai_packet(padapter, precv_frame))
		return 1;

	//if (!psta)
		psta = rtw_get_stainfo(pstapriv, ta);

	if (!psta) {
		WAPI_TRACE(WAPI_RX, "%s: psta == NULL\n", __FUNCTION__);
		return 0;
	}
	
	pwapiStaInfo = &psta->wapiStaInfo;

	if (IS_MCAST(ra)) {
		if (pwapiApInfo->wapiMcastEnable == false || 
			keyIdx != pwapiApInfo->keyIdx) {

			WAPI_TRACE(WAPI_RX, "%s: wapiMcastEnable == false or invalid keyIdx (keyIdx = %d pwapiApInfo->keyIdx = %d) \n",
				__FUNCTION__, keyIdx, pwapiApInfo->keyIdx);
			return 0;
		}

		if (psecuritypriv->dot118021XGrpPrivacy == _SMS4_) {
			pn_len = WAPI_PN_LEN;
		} else if (psecuritypriv->dot118021XGrpPrivacy == _GCM_SM4_) {
			pn_len = WAPI_GCM_PN_LEN;
		} else {
			WAPI_TRACE(WAPI_ERR, "%s: Invalid group crypto algorithm!\n", __FUNCTION__);
			return 0;
		}		

		if (!rtw_wapi_compare_PN(recvPN, pwapiApInfo->rxMcastPN, pn_len)) {
			WAPI_TRACE(WAPI_RX, "%s: Invalid multicast recvPN\n", __FUNCTION__);
			return 0;
		}

		_rtw_memcpy(pwapiApInfo->rxMcastPN, recvPN, WAPI_PN_LEN);
		
	} else {
	
		if (pwapiStaInfo->wapiUcastEnable == false || 
			keyIdx != pwapiStaInfo->keyIdx) {			
			WAPI_TRACE(WAPI_RX, "%s: wapiUcastEnable == false or invalid keyIdx (keyIdx = %d pwapiStaInfo->keyIdx = %d) \n",
				__FUNCTION__, keyIdx, pwapiStaInfo->keyIdx);
			return 0;
		}

		if (pattrib->qos) {
			tid = precv_frame->u.hdr.attrib.priority;
			switch (tid) {
			case 0:
			case 3:
				qosIdx = 0;
				break;
			case 1:
			case 2:
				qosIdx = 1;
				break;
			case 4:
			case 5:
				qosIdx = 2;
				break;
			case 6:
			case 7:
				qosIdx = 3;
				break;
			default:
				WAPI_TRACE(WAPI_ERR, "%s: Unknown TID\n", __FUNCTION__);
				break;
			}
		}


		if (MLME_IS_STA(padapter) && (recvPN[0] & 0x1) == 0) {
			WAPI_TRACE(WAPI_RX, "%s: Invalid unicast recvPN (not odd numbers)\n", __FUNCTION__);
			return 0;
		}

		if (MLME_IS_AP(padapter) && (recvPN[0] & 0x1) == 1) {
			WAPI_TRACE(WAPI_RX, "%s: Invalid unicast recvPN (not even numbers)\n", __FUNCTION__);
			return 0;
		}

#if 0
		if (!rtw_wapi_compare_PN(recvPN, pwapiStaInfo->rxUcastPN[qosIdx])) {
			WAPI_TRACE(WAPI_RX, "%s: Invalid unicast recvPN (qosIdx = %d)\n", __FUNCTION__, qosIdx);
			return 0; 
		}
#endif

		_rtw_memcpy(pwapiStaInfo->rxUcastPN[qosIdx], recvPN, WAPI_PN_LEN);
	}	

	return 1;
}


bool rtw_wapi_drop_for_key_absent(_adapter *padapter, u8 *pRA)
{
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	RTL_WAPI_AP_INFO *pwapiApInfo = &padapter->wapiApInfo;
	RTL_WAPI_STA_INFO *pwapiStaInfo = NULL;
	bool bDrop = false;

	if (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI) {

		if (IS_MCAST(pRA)) {
			if (pwapiApInfo->wapiMcastEnable == false) {
				bDrop = true;
				WAPI_TRACE(WAPI_RX, "%s: wapiMcastEnable == false!\n", __FUNCTION__);
				goto exit;
			}
		} else {
			psta = rtw_get_stainfo(pstapriv, pRA);
			if (!psta) {
				bDrop = true;
				WAPI_TRACE(WAPI_ERR, "%s: sta "MAC_FMT" not found!\n", __FUNCTION__, MAC_ARG(pRA));
				goto exit;
			}
			pwapiStaInfo = &psta->wapiStaInfo;

			if (pwapiStaInfo->wapiUcastEnable == false) {
				bDrop = true;
				WAPI_TRACE(WAPI_ERR, "%s: sta "MAC_FMT" wapiUcastEnable == false!\n", __FUNCTION__, MAC_ARG(pRA));
				goto exit;
			}			
		}
	}

exit:
	
	return bDrop;
}

void rtw_wapi_get_iv(_adapter *padapter, u8 *pRA, u8 *IV)
{
	u8 pn_len;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	WLAN_HDR_WAPI_EXT *pwapiExt = NULL;
	RTL_WAPI_AP_INFO *pwapiApInfo = &padapter->wapiApInfo;	
	bool bPNOverflow = false;	
	RTL_WAPI_STA_INFO *pwapiStaInfo = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	pwapiExt = (WLAN_HDR_WAPI_EXT *)IV;

	//WAPI_DATA(WAPI_TX, "wapi_get_iv: pra", pRA, 6);

	if (IS_MCAST(pRA)) {
		if (pwapiApInfo->wapiMcastEnable == false) {
			WAPI_TRACE(WAPI_ERR, "%s: wapiMcastEnable = false!\n", __FUNCTION__);
			return;
		}
		if (pwapiApInfo->keyIdx <= 1) {
			pwapiExt->keyIdx = pwapiApInfo->keyIdx;
			pwapiExt->reserved = 0;

			if (psecuritypriv->dot118021XGrpPrivacy == _SMS4_) {
				pn_len = WAPI_PN_LEN;
			} else if (psecuritypriv->dot118021XGrpPrivacy == _GCM_SM4_) {
				pn_len = WAPI_GCM_PN_LEN;
			} else {
				WAPI_TRACE(WAPI_ERR, "%s: Invalid group crypto algorithm!\n", __FUNCTION__);
				return;
			}				
			bPNOverflow = rtw_wapi_increase_PN(pwapiApInfo->txMcastPN, pn_len, 1);
			if (MLME_IS_AP(padapter) && (bPNOverflow || DBG_WAPI_MSK_UPDATE_ONCE)) {
				//Multicast data PN is overflow, it must trigger multicast key update.
				rtw_cfg80211_indicate_wapi_key_update(padapter, get_bssid(pmlmepriv), NL80211_KEYTYPE_GROUP);
				WAPI_TRACE(WAPI_TX, "%s: Multicast data PN is overflow!\n", __FUNCTION__);
				DBG_WAPI_MSK_UPDATE_ONCE = 0;
			} else {
				_rtw_memcpy(pwapiExt->PN, pwapiApInfo->txMcastPN, WAPI_PN_LEN);
			}
		}
	} else {
		psta = rtw_get_stainfo(pstapriv, pRA);
		if (!psta) {
			WAPI_TRACE(WAPI_ERR, "%s: sta "MAC_FMT" not found!\n", __FUNCTION__, MAC_ARG(pRA));
			return;
		}
		pwapiStaInfo = &psta->wapiStaInfo;

		if (pwapiStaInfo->wapiUcastEnable == false) {
			WAPI_TRACE(WAPI_ERR, "%s: wapiUcastEnable = false!\n", __FUNCTION__);
			return;
		}

		if (pwapiStaInfo->keyIdx <= 1) {
			pwapiExt->keyIdx = pwapiStaInfo->keyIdx;
			pwapiExt->reserved = 0;

			if (psecuritypriv->dot11PrivacyAlgrthm == _SMS4_) {
				pn_len = WAPI_PN_LEN;
			} else if (psecuritypriv->dot11PrivacyAlgrthm == _GCM_SM4_) {
				pn_len = WAPI_GCM_PN_LEN;
			} else {
				WAPI_TRACE(WAPI_ERR, "%s: Invalid unicast crypto algorithm!\n", __FUNCTION__);
				return;
			}
			bPNOverflow = rtw_wapi_increase_PN(pwapiStaInfo->txUcastPN, pn_len, 2);
			if (MLME_IS_AP(padapter) && (bPNOverflow || DBG_WAPI_USK_UPDATE_ONCE)) {
				//Unicast data PN is overflow, it must trigger unicast key update.
				rtw_cfg80211_indicate_wapi_key_update(padapter, psta->phl_sta->mac_addr, NL80211_KEYTYPE_PAIRWISE);
				WAPI_TRACE(WAPI_TX, "%s: Unicast data PN is overflow!\n", __FUNCTION__);
				DBG_WAPI_USK_UPDATE_ONCE = 0;
			} else {
				_rtw_memcpy(pwapiExt->PN, pwapiStaInfo->txUcastPN, WAPI_PN_LEN);

				//RTW_INFO("\n%s:%d pwapiExt->keyIdx=%d\n", __FUNCTION__,__LINE__, pwapiExt->keyIdx);
			}
		}
	}
}


int rtw_wapi_set_key(_adapter *padapter, struct ieee_param *param)
{
	int i, ret = 0;
	u8 alg, keyIdx;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;	
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	RTL_WAPI_STA_INFO *pwapiStaInfo = NULL;
	RTL_WAPI_AP_INFO *pwapiApInfo = &padapter->wapiApInfo;
	
	u8 wapiUcastPNInitValue[] = {0x37, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C};
	u8 wapiMcastPNInitValue[] = {0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C};

	if (MLME_IS_STA(padapter))
		wapiUcastPNInitValue[0] = 0x36;
	
	keyIdx =  param->u.crypt.idx;

	if (strcmp(param->u.crypt.alg, "SMS4") == 0)
		alg = _SMS4_;
	else if (strcmp(param->u.crypt.alg, "GCM-SM4") == 0)
		alg = _GCM_SM4_;
	else 
		return (-EINVAL);

	if (keyIdx < 0 || keyIdx > 1) {
		ret = -EINVAL;
		WAPI_TRACE(WAPI_ERR, "%s: Invalid keyIdx %d!\n", __FUNCTION__, keyIdx);
		goto exit;
	}

	if (param->u.crypt.set_tx == 1) {
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if (!psta) {
			ret = -EINVAL;
			WAPI_TRACE(WAPI_ERR, "%s: sta "MAC_FMT" not found!\n", __FUNCTION__, MAC_ARG(param->sta_addr));
			goto exit;
		}

		pwapiStaInfo = &psta->wapiStaInfo;
		pwapiStaInfo->keyIdx = keyIdx;
		psecuritypriv->dot11PrivacyKeyIndex = keyIdx;
		psecuritypriv->dot11PrivacyAlgrthm = alg;
		psecuritypriv->dot11PrivacyKeyIndex = keyIdx;
		
		_rtw_memcpy(pwapiStaInfo->wapiUcastKey[keyIdx].dataKey, param->u.crypt.key, WAPI_KEY_LEN);		
		_rtw_memcpy(pwapiStaInfo->wapiUcastKey[keyIdx].micKey, param->u.crypt.key + WAPI_KEY_LEN, WAPI_KEY_LEN);
		
		_rtw_memcpy(pwapiStaInfo->txUcastPN, wapiUcastPNInitValue, WAPI_PN_LEN);
		for (i = 0; i < RX_QUEUE_NUM; i++)
			_rtw_memcpy(pwapiStaInfo->rxUcastPN[i], wapiUcastPNInitValue, WAPI_PN_LEN);	

		if (psecuritypriv->sw_encrypt == false || psecuritypriv->sw_decrypt == false) {
			/* set unicast key for ASUE */
			//printk("\n%s: calling rtw_ap_set_sta_key()\n", __FUNCTION__);

			if (MLME_IS_STA(padapter)) {
				_rtw_memcpy(psta->dot118021x_UncstKey.skey, param->u.crypt.key, 32);
				rtw_setstakey_cmd(padapter, psta, UNICAST_KEY, _TRUE);
			}
			else {
				rtw_ap_set_sta_key(padapter, param->sta_addr, alg, param->u.crypt.key, keyIdx, 0);
			}
		}
		pwapiStaInfo->wapiUcastEnable = true;

		psta->ieee8021x_blocked = _FALSE;
		psta->bpairwise_key_installed = _TRUE;

#if 0
		//disable txsc
		pxmitpriv->txsc_enable = 0;
		#ifdef CONFIG_TXSC_AMSDU
		if(pxmitpriv->txsc_enable == 0)
			pxmitpriv->txsc_amsdu_enable = 0;
		#endif
#endif

		//printk("\n%s: alg = %d keyIdx = %d\n", __FUNCTION__, alg, keyIdx);
		//wapi_dump_buf(param->u.crypt.key, param->u.crypt.key_len);
	} else {	
		pwapiApInfo->keyIdx = keyIdx;
		psecuritypriv->dot118021XGrpKeyid = keyIdx;
		psecuritypriv->dot118021XGrpPrivacy = alg;
		
		_rtw_memcpy(pwapiApInfo->wapiMcastKey[keyIdx].dataKey, param->u.crypt.key, WAPI_KEY_LEN);
		_rtw_memcpy(pwapiApInfo->wapiMcastKey[keyIdx].micKey, param->u.crypt.key + WAPI_KEY_LEN, WAPI_KEY_LEN);		

		_rtw_memcpy(pwapiApInfo->txMcastPN, wapiMcastPNInitValue, WAPI_PN_LEN);
		_rtw_memcpy(pwapiApInfo->rxMcastPN, wapiMcastPNInitValue, WAPI_PN_LEN);

		if (psecuritypriv->sw_encrypt == false || psecuritypriv->sw_decrypt == false) {
			/* set rx broadcast key for ASUE */
			
			if (MLME_IS_STA(padapter)) {
				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[keyIdx].skey, param->u.crypt.key, 32);
				rtw_set_key(padapter, psecuritypriv, keyIdx, 1, _TRUE);
			}
			else {
				rtw_ap_set_group_key(padapter, param->u.crypt.key, alg, keyIdx);
			}
		}
		pwapiApInfo->wapiMcastEnable = true;
		psecuritypriv->binstallGrpkey = _TRUE;

		//wapi_dump_buf(param->u.crypt.key, param->u.crypt.key_len);
	}

exit:

	return ret;
}

/*
	If Qos bit 4~6 and 8~15 is not 0, return 1.
*/
int rtw_wapi_check_frame_qos(u8 *pframe, u16 pktlen)
{
	u16 *qos = NULL;
	u8 qos_offset = 0;
	
	if (pframe == NULL || pktlen < WLAN_HDR_A4_QOS_LEN)
		return 0;

	if (get_tofr_ds(pframe) == 3)				
		qos_offset = WLAN_HDR_A4_LEN;
	else 
		qos_offset = WLAN_HDR_A3_LEN;
	
	qos = (u16 *)(pframe + qos_offset);

	if (qos == NULL)
		return 0;

//check whether Qos bit 4~6 and 8~15 is 0
#ifdef CONFIG_LITTLE_ENDIAN
	if (*qos & 0xff70) 
#else
	if (*qos & 0x70ff)
#endif
		return 1;

	return 0;
}
#endif




