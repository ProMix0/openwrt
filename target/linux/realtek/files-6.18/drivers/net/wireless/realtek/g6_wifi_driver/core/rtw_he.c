/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
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
#define _RTW_HE_C

#include <drv_types.h>

#ifdef CONFIG_80211AX_HE

void	rtw_he_use_default_setting(_adapter *padapter)
{
	/* CONFIG_80211AX_HE_TODO */
}

static void rtw_he_set_asoc_cap_supp_mcs(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start, u8 supp_mcs_len)
{
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *role_cap = &(wrole->proto_role_cap);
	int nss = 0, nss_tx = 0, nss_rx = 0;
	u8 mcs_from_role = HE_MSC_NOT_SUPP;
	u8 mcs_from_ie = HE_MSC_NOT_SUPP;
	u8 mcs_val_rx = HE_MSC_NOT_SUPP;
	u8 mcs_val_tx = HE_MSC_NOT_SUPP;

	if(phl_sta->chandef.bw == CHANNEL_WIDTH_160){
		_rtw_memset(phl_sta->asoc_cap.he_rx_mcs, HE_MSC_NOT_SUPP_BYTE, HE_CAP_ELE_SUPP_MCS_LEN_RX_80M + HE_CAP_ELE_SUPP_MCS_LEN_RX_160M);
		_rtw_memset(phl_sta->asoc_cap.he_tx_mcs, HE_MSC_NOT_SUPP_BYTE, HE_CAP_ELE_SUPP_MCS_LEN_TX_80M + HE_CAP_ELE_SUPP_MCS_LEN_TX_160M);
	}else{
	_rtw_memset(phl_sta->asoc_cap.he_rx_mcs, HE_MSC_NOT_SUPP_BYTE, HE_CAP_ELE_SUPP_MCS_LEN_RX_80M);
	_rtw_memset(phl_sta->asoc_cap.he_tx_mcs, HE_MSC_NOT_SUPP_BYTE, HE_CAP_ELE_SUPP_MCS_LEN_TX_80M);
	}
	/* only deal with <= 80MHz now */
	for (nss = 1; nss <= 8; nss++) {

		mcs_val_rx = HE_MSC_NOT_SUPP;
		mcs_val_tx = HE_MSC_NOT_SUPP;

		switch (nss) {
		case 1:
			mcs_from_role = GET_HE_CAP_MCS_1SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_1SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_1SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_1SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_1SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_1SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);

			if(phl_sta->chandef.bw == CHANNEL_WIDTH_160){
				mcs_from_role = GET_HE_CAP_MCS_1SS(role_cap->he_tx_mcs);
				mcs_from_ie = GET_HE_CAP_RX_MCS_160MHZ_1SS(ele_start);
				if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
					mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

				mcs_from_role = GET_HE_CAP_MCS_1SS(role_cap->he_rx_mcs);
				mcs_from_ie = GET_HE_CAP_TX_MCS_160MHZ_1SS(ele_start);
				if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
					mcs_val_tx = mcs_from_ie;

				SET_HE_CAP_160M_MCS_1SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
				SET_HE_CAP_160M_MCS_1SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);

			}
			break;
		case 2:
			mcs_from_role = GET_HE_CAP_MCS_2SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_2SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_2SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_2SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_2SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_2SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);

			if(phl_sta->chandef.bw == CHANNEL_WIDTH_160){
				mcs_from_role = GET_HE_CAP_MCS_2SS(role_cap->he_tx_mcs);
				mcs_from_ie = GET_HE_CAP_RX_MCS_160MHZ_2SS(ele_start);
				if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
					mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

				mcs_from_role = GET_HE_CAP_MCS_2SS(role_cap->he_rx_mcs);
				mcs_from_ie = GET_HE_CAP_TX_MCS_160MHZ_2SS(ele_start);
				if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
					mcs_val_tx = mcs_from_ie;

				SET_HE_CAP_160M_MCS_2SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
				SET_HE_CAP_160M_MCS_2SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);

			}
			break;
		case 3:
			mcs_from_role = GET_HE_CAP_MCS_3SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_3SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_3SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_3SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_3SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_3SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		case 4:
			mcs_from_role = GET_HE_CAP_MCS_4SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_4SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_4SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_4SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_4SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_4SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		case 5:
			mcs_from_role = GET_HE_CAP_MCS_5SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_5SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_5SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_5SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_5SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_5SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		case 6:
			mcs_from_role = GET_HE_CAP_MCS_6SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_6SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_6SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_6SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_6SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_6SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		case 7:
			mcs_from_role = GET_HE_CAP_MCS_7SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_7SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_7SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_7SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_7SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_7SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		case 8:
			mcs_from_role = GET_HE_CAP_MCS_8SS(role_cap->he_tx_mcs);
			mcs_from_ie = GET_HE_CAP_RX_MCS_LESS_THAN_80MHZ_8SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_rx = (mcs_from_role < mcs_from_ie) ? mcs_from_role : mcs_from_ie;

			mcs_from_role = GET_HE_CAP_MCS_8SS(role_cap->he_rx_mcs);
			mcs_from_ie = GET_HE_CAP_TX_MCS_LESS_THAN_80MHZ_8SS(ele_start);
			if ((mcs_from_role != HE_MSC_NOT_SUPP) && (mcs_from_ie != HE_MSC_NOT_SUPP))
				mcs_val_tx = mcs_from_ie;

			SET_HE_CAP_MCS_8SS(phl_sta->asoc_cap.he_rx_mcs, mcs_val_rx);
			SET_HE_CAP_MCS_8SS(phl_sta->asoc_cap.he_tx_mcs, mcs_val_tx);
			break;
		}

		if (mcs_val_rx != HE_MSC_NOT_SUPP)
			nss_rx++;

		if (mcs_val_tx != HE_MSC_NOT_SUPP)
			nss_tx++;
	}

	phl_sta->asoc_cap.nss_rx = nss_rx;
	phl_sta->asoc_cap.nss_tx = nss_tx;
}

static void rtw_he_set_asoc_cap_ppe_thre(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start)
{
	u8 nsts, rumsk, i, j, offset, shift;
	u16 ppe8, ppe16;

	if (phl_sta->asoc_cap.pkt_padding != 3)
		return;

	nsts = GET_HE_CAP_PPE_NSTS(ele_start);
	rumsk = GET_HE_CAP_PPE_PU_IDX_BITMASK(ele_start);
	shift = 7;

	for (i = 0; i <= nsts; i ++) {
		for (j = 0; j < 4; j++) {
			if (rumsk & (BIT(0) << j)) {
				offset = shift / 8;
#ifdef PLATFORM_ECOS
				ppe16 = le16_to_cpu(get_unaligned_be16((ele_start + offset)));
				ppe16 = (ppe16 >> (shift % 8)) & BIT_LEN_MASK_16(3);
#else
				ppe16 = LE_BITS_TO_2BYTE(ele_start + offset, shift % 8, 3);
#endif
				shift += 3;
				offset = shift / 8;
#ifdef PLATFORM_ECOS
				ppe8 = le16_to_cpu(get_unaligned_be16((ele_start + offset)));
				ppe8 = (ppe8 >> (shift % 8)) & BIT_LEN_MASK_16(3);
#else
				ppe8 = LE_BITS_TO_2BYTE(ele_start + offset, shift % 8, 3);
#endif
				shift += 3;
				phl_sta->asoc_cap.ppe_thr[i][j] = ((ppe16 & 0x07) | ((ppe8 & 0x07) << 3));
			} else {
				phl_sta->asoc_cap.ppe_thr[i][j] = 0;
			}
		}
		/* For 1+9 smart phone not carry padding for BW 160M issue
		If 160M is not carried, follow 80M's setting.
		*/
		if (!(rumsk & (BIT(0) << 3)) && (rumsk & (BIT(0) << 2)))
			phl_sta->asoc_cap.ppe_thr[i][3] = phl_sta->asoc_cap.ppe_thr[i][2];
	}
}

static void update_sta_he_mac_cap_apmode(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start)
{
	/* CONFIG_80211AX_HE_TODO - we may need to refer to role_cap when setting some of asoc_cap  */
#if 0
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *role_cap = &(wrole->proto_role_cap);
#endif

	phl_sta->asoc_cap.htc_rx = GET_HE_MAC_CAP_HTC_HE_SUPPORT(ele_start);
	phl_sta->asoc_cap.twt = GET_HE_MAC_CAP_TWT_REQUESTER_SUPPORT(ele_start);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_TWT_RESPONDER_SUPPORT(ele_start)) << 1);
	phl_sta->asoc_cap.trig_padding = GET_HE_MAC_CAP_TRI_FRAME_PADDING_DUR(ele_start);
	phl_sta->asoc_cap.all_ack = GET_HE_MAC_CAP_ALL_ACK_SUPPORT(ele_start);
	phl_sta->asoc_cap.a_ctrl = GET_HE_MAC_CAP_TRS_SUPPORT(ele_start);
	phl_sta->asoc_cap.a_ctrl |= ((GET_HE_MAC_CAP_BRS_SUPPORT(ele_start)) << 1);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_BC_TWT_SUPPORT(ele_start)) << 2);
	phl_sta->asoc_cap.a_ctrl |= ((GET_HE_MAC_CAP_OM_CTRL_SUPPORT(ele_start)) << 2);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_FLEX_TWT_SCHED_SUPPORT(ele_start)) << 3);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_PSR_RESPONDER(ele_start)) << 4);
	phl_sta->asoc_cap.ops = GET_HE_MAC_CAP_OPS_SUPPORT(ele_start);
	phl_sta->asoc_cap.amsdu_in_ampdu =
		GET_HE_MAC_CAP_AMSDU_NOT_UNDER_BA_IN_ACK_EN_AMPDU(ele_start);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_HE_SUB_CH_SELECTIVE_TX(ele_start)) << 5);
	phl_sta->asoc_cap.ht_vht_trig_rx =
		GET_HE_MAC_CAP_HT_VHT_TRIG_FRAME_RX(ele_start);
}

static void update_sta_he_phy_cap_apmode(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta,
                                         u8 *ele_start, u8 *supp_mcs_len)
{
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
        struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct protocol_cap_t *role_cap = &(wrole->proto_role_cap);
	struct role_cap_t *cap = &(wrole->cap);
	enum channel_width previous_bw = phl_sta->chandef.bw;

	if (phl_sta->chandef.band == BAND_ON_24G) {
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(0))		// 40MHz in 2.4G supported
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_40) ?
			wrole->chandef.bw : CHANNEL_WIDTH_40;
	} else if (phl_sta->chandef.band == BAND_ON_5G) {
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(1))		// 40 & 80MHz in 5G supported
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_80) ?
			wrole->chandef.bw : CHANNEL_WIDTH_80;
		else
			phl_sta->chandef.bw = CHANNEL_WIDTH_20;

		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(2)){		// 160MHz in 5G supported
			*supp_mcs_len += 4;
				phl_sta->chandef.bw = CHANNEL_WIDTH_160;
		}
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(3))		// 160/80+80MHz in 5G supported
			*supp_mcs_len += 4;
	}

	phl_sta->asoc_cap.he_242tone_5g = GET_HE_PHY_CAP_SUPPORT_242tone_5G(ele_start) ? 1 : 0;
	phl_sta->asoc_cap.he_242tone_2g = GET_HE_PHY_CAP_SUPPORT_242tone_2G(ele_start) ? 1 : 0;

	if (phl_sta->chandef.band == BAND_ON_24G) {
		if (previous_bw < phl_sta->chandef.bw) {
			RTW_INFO("[%s]Going to assign BW = %d, previus assign BW = %d, force down BW\n",
					__FUNCTION__, phl_sta->chandef.bw, previous_bw);
			phl_sta->chandef.bw = previous_bw;
		}
	}

	if (phl_sta->chandef.bw > pmlmeext->cur_bwmode)
		phl_sta->chandef.bw = pmlmeext->cur_bwmode;

	phl_sta->asoc_cap.he_ldpc = (GET_HE_PHY_CAP_LDPC_IN_PAYLOAD(ele_start) & role_cap->he_ldpc);

	if (phl_sta->asoc_cap.er_su) {
		phl_sta->asoc_cap.ltf_gi = (BIT(RTW_GILTF_2XHE16) |
			BIT(RTW_GILTF_2XHE08) | BIT(RTW_GILTF_1XHE16) |
			(GET_HE_PHY_CAP_NDP_4X_LTF_3_POINT_2_GI(ele_start) ?
			BIT(RTW_GILTF_LGI_4XHE32) : 0) |
			(GET_HE_PHY_CAP_ERSU_PPDU_4X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_SGI_4XHE08) : 0) |
			(GET_HE_PHY_CAP_ERSU_PPDU_1X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_1XHE08) : 0));
	} else {
		phl_sta->asoc_cap.ltf_gi = (BIT(RTW_GILTF_2XHE16) |
			BIT(RTW_GILTF_2XHE08) | BIT(RTW_GILTF_1XHE16) |
			(GET_HE_PHY_CAP_NDP_4X_LTF_3_POINT_2_GI(ele_start) ?
			BIT(RTW_GILTF_LGI_4XHE32) : 0) |
			(GET_HE_PHY_CAP_SU_MU_PPDU_4X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_SGI_4XHE08) : 0) |
			(GET_HE_PHY_CAP_SU_PPDU_1X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_1XHE08) : 0));
	}

	phl_sta->asoc_cap.stbc_he_tx = (GET_HE_PHY_CAP_STBC_TX_LESS_THAN_80MHZ(ele_start) & role_cap->stbc_he_rx);
	phl_sta->asoc_cap.stbc_he_rx = (GET_HE_PHY_CAP_STBC_RX_LESS_THAN_80MHZ(ele_start) & role_cap->stbc_he_tx);
	phl_sta->asoc_cap.doppler_tx = (GET_HE_PHY_CAP_DOPPLER_TX(ele_start) & role_cap->doppler_rx);
	phl_sta->asoc_cap.doppler_rx = (GET_HE_PHY_CAP_DOPPLER_RX(ele_start) & role_cap->doppler_tx);

	phl_sta->asoc_cap.dcm_max_const_tx =
		GET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_TX(ele_start);
	if (phl_sta->asoc_cap.dcm_max_const_tx > role_cap->dcm_max_const_rx)
		phl_sta->asoc_cap.dcm_max_const_tx = role_cap->dcm_max_const_rx;

	phl_sta->asoc_cap.dcm_max_nss_tx = (GET_HE_PHY_CAP_DCM_MAX_NSS_TX(ele_start) & role_cap->dcm_max_nss_rx);

	phl_sta->asoc_cap.dcm_max_const_rx =
		GET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_RX(ele_start);
	if (phl_sta->asoc_cap.dcm_max_const_rx > role_cap->dcm_max_const_tx)
		phl_sta->asoc_cap.dcm_max_const_rx = role_cap->dcm_max_const_tx;

	phl_sta->asoc_cap.dcm_max_nss_rx = (GET_HE_PHY_CAP_DCM_MAX_NSS_RX(ele_start) & role_cap->dcm_max_nss_tx);

	phl_sta->asoc_cap.partial_bw_su_er =
		GET_HE_PHY_CAP_RX_PARTIAL_BW_SU_IN_20MHZ_MUPPDU(ele_start);
	phl_sta->asoc_cap.he_su_bfmr = GET_HE_PHY_CAP_SU_BFER(ele_start);
	phl_sta->asoc_cap.he_su_bfme = GET_HE_PHY_CAP_SU_BFEE(ele_start);
	phl_sta->asoc_cap.he_mu_bfmr = GET_HE_PHY_CAP_MU_BFER(ele_start);
	phl_sta->asoc_cap.bfme_sts =
		GET_HE_PHY_CAP_BFEE_STS_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.bfme_sts_greater_80mhz =
		GET_HE_PHY_CAP_BFEE_STS_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.num_snd_dim =
		GET_HE_PHY_CAP_NUM_SND_DIMEN_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.num_snd_dim_greater_80mhz =
		GET_HE_PHY_CAP_NUM_SND_DIMEN_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.ng_16_su_fb = GET_HE_PHY_CAP_NG_16_SU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.ng_16_mu_fb = GET_HE_PHY_CAP_NG_16_MU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.cb_sz_su_fb =
		GET_HE_PHY_CAP_CODEBOOK_4_2_SU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.cb_sz_mu_fb =
		GET_HE_PHY_CAP_CODEBOOK_7_5_MU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_su_bfm_fb =
		GET_HE_PHY_CAP_TRIG_SUBF_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_mu_bfm_fb =
		GET_HE_PHY_CAP_TRIG_MUBF_PARTIAL_BW_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_cqi_fb = GET_HE_PHY_CAP_TRIG_CQI_FEEDBACK(ele_start);
	phl_sta->asoc_cap.partial_bw_su_er =
		GET_HE_PHY_CAP_PARTIAL_BW_EXT_RANGE(ele_start);
	phl_sta->asoc_cap.pwr_bst_factor =
		GET_HE_PHY_CAP_PWR_BOOST_FACTOR_SUPPORT(ele_start);
	phl_sta->asoc_cap.max_nc = GET_HE_PHY_CAP_MAX_NC(ele_start);
	phl_sta->asoc_cap.stbc_tx_greater_80mhz =
		(GET_HE_PHY_CAP_STBC_TX_GREATER_THAN_80MHZ(ele_start) & role_cap->stbc_rx_greater_80mhz);
	phl_sta->asoc_cap.stbc_rx_greater_80mhz =
		(GET_HE_PHY_CAP_STBC_RX_GREATER_THAN_80MHZ(ele_start) & role_cap->stbc_tx_greater_80mhz);
	phl_sta->asoc_cap.dcm_max_ru = GET_HE_PHY_CAP_DCM_MAX_RU(ele_start);
	phl_sta->asoc_cap.long_sigb_symbol =
		GET_HE_PHY_CAP_LONGER_THAN_16_HESIGB_OFDM_SYM(ele_start);
	phl_sta->asoc_cap.non_trig_cqi_fb =
		GET_HE_PHY_CAP_NON_TRIGGER_CQI_FEEDBACK(ele_start);
	phl_sta->asoc_cap.tx_1024q_ru =
		(GET_HE_PHY_CAP_TX_1024_QAM_LESS_THAN_242_TONE_RU(ele_start) & role_cap->rx_1024q_ru);
	phl_sta->asoc_cap.rx_1024q_ru =
		(GET_HE_PHY_CAP_RX_1024_QAM_LESS_THAN_242_TONE_RU(ele_start) & role_cap->tx_1024q_ru);
	phl_sta->asoc_cap.fbw_su_using_mu_cmprs_sigb =
		GET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_CMP_SIGB(ele_start);
	phl_sta->asoc_cap.fbw_su_using_mu_non_cmprs_sigb =
		GET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_NONCMP_SIGB(ele_start);

	if (GET_HE_PHY_CAP_PPE_THRESHOLD_PRESENT(ele_start))
		phl_sta->asoc_cap.pkt_padding = 3;
	else
		phl_sta->asoc_cap.pkt_padding = GET_HE_PHY_CAP_NOMINAL_PACKET_PADDING(ele_start);
}

static void update_sta_he_supp_mcs_apmode(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start, u8 supp_mcs_len)
{
	rtw_he_set_asoc_cap_supp_mcs(padapter, phl_sta, ele_start, supp_mcs_len);
}

static void update_sta_he_ppe_thre_apmode(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start)
{
	rtw_he_set_asoc_cap_ppe_thre(padapter, phl_sta, ele_start);
}

void	update_sta_he_info_apmode(_adapter *padapter, void *sta)
{
	struct sta_info	*psta = (struct sta_info *)sta;
	struct rtw_phl_stainfo_t *phl_sta = psta->phl_sta;
	struct he_priv	*phepriv_sta = &psta->hepriv;
	u8 *ele_start = NULL;
	u8 supp_mcs_len = 4;

	if (phepriv_sta->he_option == _FALSE)
		return;

	ele_start = &(phepriv_sta->he_cap[1]);
	update_sta_he_mac_cap_apmode(padapter, phl_sta, ele_start);

	ele_start += HE_CAP_ELE_MAC_CAP_LEN;
	update_sta_he_phy_cap_apmode(padapter, phl_sta, ele_start, &supp_mcs_len);

	ele_start += HE_CAP_ELE_PHY_CAP_LEN;
	update_sta_he_supp_mcs_apmode(padapter, phl_sta, ele_start, supp_mcs_len);

	ele_start += supp_mcs_len;
	update_sta_he_ppe_thre_apmode(padapter, phl_sta, ele_start);
}

void	update_hw_he_param(_adapter *padapter)
{
	/* CONFIG_80211AX_HE_TODO */
}

static void HE_mac_caps_handler(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start)
{
	phl_sta->asoc_cap.htc_rx = GET_HE_MAC_CAP_HTC_HE_SUPPORT(ele_start);
	phl_sta->asoc_cap.twt = GET_HE_MAC_CAP_TWT_REQUESTER_SUPPORT(ele_start);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_TWT_RESPONDER_SUPPORT(ele_start)) << 1);
	phl_sta->asoc_cap.trig_padding = GET_HE_MAC_CAP_TRI_FRAME_PADDING_DUR(ele_start);
	phl_sta->asoc_cap.all_ack = GET_HE_MAC_CAP_ALL_ACK_SUPPORT(ele_start);
	phl_sta->asoc_cap.a_ctrl = GET_HE_MAC_CAP_TRS_SUPPORT(ele_start);
	phl_sta->asoc_cap.a_ctrl |= ((GET_HE_MAC_CAP_BRS_SUPPORT(ele_start)) << 1);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_BC_TWT_SUPPORT(ele_start)) << 2);
	phl_sta->asoc_cap.a_ctrl |= ((GET_HE_MAC_CAP_OM_CTRL_SUPPORT(ele_start)) << 2);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_FLEX_TWT_SCHED_SUPPORT(ele_start)) << 3);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_PSR_RESPONDER(ele_start)) << 4);
	phl_sta->asoc_cap.ops = GET_HE_MAC_CAP_OPS_SUPPORT(ele_start);
	phl_sta->asoc_cap.amsdu_in_ampdu =
		GET_HE_MAC_CAP_AMSDU_NOT_UNDER_BA_IN_ACK_EN_AMPDU(ele_start);
	phl_sta->asoc_cap.twt |= ((GET_HE_MAC_CAP_HE_SUB_CH_SELECTIVE_TX(ele_start)) << 5);
	phl_sta->asoc_cap.ht_vht_trig_rx =
		GET_HE_MAC_CAP_HT_VHT_TRIG_FRAME_RX(ele_start);
}

static void HE_phy_caps_handler(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start, u8 *supp_mcs_len)
{
	struct rtw_wifi_role_t 	*wrole = padapter->phl_role;

	if (phl_sta->chandef.band == BAND_ON_24G) {
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(0))
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_40) ?
			wrole->chandef.bw : CHANNEL_WIDTH_40;
	} else if (phl_sta->chandef.band == BAND_ON_5G) {
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(1))
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_80) ?
			wrole->chandef.bw : CHANNEL_WIDTH_80;
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(2)) {
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_160) ?
			wrole->chandef.bw : CHANNEL_WIDTH_160;
			*supp_mcs_len += 4;
		}
		if (GET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(ele_start) & BIT(3)) {
			phl_sta->chandef.bw = (wrole->chandef.bw < CHANNEL_WIDTH_80_80) ?
			wrole->chandef.bw : CHANNEL_WIDTH_80_80;
			*supp_mcs_len += 4;
		}
	}
	phl_sta->asoc_cap.he_ldpc = GET_HE_PHY_CAP_LDPC_IN_PAYLOAD(ele_start);
	if (phl_sta->asoc_cap.er_su) {
		phl_sta->asoc_cap.ltf_gi = (BIT(RTW_GILTF_2XHE16) |
			BIT(RTW_GILTF_2XHE08) | BIT(RTW_GILTF_1XHE16) |
			(GET_HE_PHY_CAP_NDP_4X_LTF_3_POINT_2_GI(ele_start) ?
			BIT(RTW_GILTF_LGI_4XHE32) : 0) |
			(GET_HE_PHY_CAP_ERSU_PPDU_4X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_SGI_4XHE08) : 0) |
			(GET_HE_PHY_CAP_ERSU_PPDU_1X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_1XHE08) : 0));
	} else {
		phl_sta->asoc_cap.ltf_gi = (BIT(RTW_GILTF_2XHE16) |
			BIT(RTW_GILTF_2XHE08) | BIT(RTW_GILTF_1XHE16) |
			(GET_HE_PHY_CAP_NDP_4X_LTF_3_POINT_2_GI(ele_start) ?
			BIT(RTW_GILTF_LGI_4XHE32) : 0) |
			(GET_HE_PHY_CAP_SU_MU_PPDU_4X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_SGI_4XHE08) : 0) |
			(GET_HE_PHY_CAP_SU_PPDU_1X_LTF_0_POINT_8_GI(ele_start) ?
			BIT(RTW_GILTF_1XHE08) : 0));
	}
	phl_sta->asoc_cap.stbc_he_tx = GET_HE_PHY_CAP_STBC_TX_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.stbc_he_rx = GET_HE_PHY_CAP_STBC_RX_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.doppler_tx = GET_HE_PHY_CAP_DOPPLER_TX(ele_start);
	phl_sta->asoc_cap.doppler_rx = GET_HE_PHY_CAP_DOPPLER_RX(ele_start);
	phl_sta->asoc_cap.dcm_max_const_tx =
		GET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_TX(ele_start);
	phl_sta->asoc_cap.dcm_max_nss_tx = GET_HE_PHY_CAP_DCM_MAX_NSS_TX(ele_start);
	phl_sta->asoc_cap.dcm_max_const_rx =
		GET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_RX(ele_start);
	phl_sta->asoc_cap.dcm_max_nss_rx = GET_HE_PHY_CAP_DCM_MAX_NSS_RX(ele_start);
	phl_sta->asoc_cap.partial_bw_su_er =
		GET_HE_PHY_CAP_RX_PARTIAL_BW_SU_IN_20MHZ_MUPPDU(ele_start);
	phl_sta->asoc_cap.he_su_bfmr = GET_HE_PHY_CAP_SU_BFER(ele_start);
	phl_sta->asoc_cap.he_su_bfme = GET_HE_PHY_CAP_SU_BFEE(ele_start);
	phl_sta->asoc_cap.he_mu_bfmr = GET_HE_PHY_CAP_MU_BFER(ele_start);
	phl_sta->asoc_cap.bfme_sts =
		GET_HE_PHY_CAP_BFEE_STS_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.bfme_sts_greater_80mhz =
		GET_HE_PHY_CAP_BFEE_STS_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.num_snd_dim =
		GET_HE_PHY_CAP_NUM_SND_DIMEN_LESS_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.num_snd_dim_greater_80mhz =
		GET_HE_PHY_CAP_NUM_SND_DIMEN_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.ng_16_su_fb = GET_HE_PHY_CAP_NG_16_SU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.ng_16_mu_fb = GET_HE_PHY_CAP_NG_16_MU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.cb_sz_su_fb =
		GET_HE_PHY_CAP_CODEBOOK_4_2_SU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.cb_sz_mu_fb =
		GET_HE_PHY_CAP_CODEBOOK_7_5_MU_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_su_bfm_fb =
		GET_HE_PHY_CAP_TRIG_SUBF_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_mu_bfm_fb =
		GET_HE_PHY_CAP_TRIG_MUBF_PARTIAL_BW_FEEDBACK(ele_start);
	phl_sta->asoc_cap.trig_cqi_fb = GET_HE_PHY_CAP_TRIG_CQI_FEEDBACK(ele_start);
	phl_sta->asoc_cap.partial_bw_su_er =
		GET_HE_PHY_CAP_PARTIAL_BW_EXT_RANGE(ele_start);
	phl_sta->asoc_cap.pwr_bst_factor =
		GET_HE_PHY_CAP_PWR_BOOST_FACTOR_SUPPORT(ele_start);
	phl_sta->asoc_cap.max_nc = GET_HE_PHY_CAP_MAX_NC(ele_start);
	phl_sta->asoc_cap.stbc_tx_greater_80mhz =
		GET_HE_PHY_CAP_STBC_TX_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.stbc_rx_greater_80mhz =
		GET_HE_PHY_CAP_STBC_RX_GREATER_THAN_80MHZ(ele_start);
	phl_sta->asoc_cap.dcm_max_ru = GET_HE_PHY_CAP_DCM_MAX_RU(ele_start);
	phl_sta->asoc_cap.long_sigb_symbol =
		GET_HE_PHY_CAP_LONGER_THAN_16_HESIGB_OFDM_SYM(ele_start);
	phl_sta->asoc_cap.non_trig_cqi_fb =
		GET_HE_PHY_CAP_NON_TRIGGER_CQI_FEEDBACK(ele_start);
	phl_sta->asoc_cap.tx_1024q_ru =
		GET_HE_PHY_CAP_TX_1024_QAM_LESS_THAN_242_TONE_RU(ele_start);
	phl_sta->asoc_cap.rx_1024q_ru =
		GET_HE_PHY_CAP_RX_1024_QAM_LESS_THAN_242_TONE_RU(ele_start);
	phl_sta->asoc_cap.fbw_su_using_mu_cmprs_sigb =
		GET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_CMP_SIGB(ele_start);
	phl_sta->asoc_cap.fbw_su_using_mu_non_cmprs_sigb =
		GET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_NONCMP_SIGB(ele_start);

	if (GET_HE_PHY_CAP_PPE_THRESHOLD_PRESENT(ele_start))
		phl_sta->asoc_cap.pkt_padding = 3;
	else
		phl_sta->asoc_cap.pkt_padding = GET_HE_PHY_CAP_NOMINAL_PACKET_PADDING(ele_start);
}

static void HE_supp_mcs_handler(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start, u8 supp_mcs_len)
{
	rtw_he_set_asoc_cap_supp_mcs(padapter, phl_sta, ele_start, supp_mcs_len);
}

static void HE_ppe_thre_handler(_adapter *padapter, struct rtw_phl_stainfo_t *phl_sta, u8 *ele_start)
{
	rtw_he_set_asoc_cap_ppe_thre(padapter, phl_sta, ele_start);
}

void HE_caps_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct rtw_wifi_role_t 	*wrole = padapter->phl_role;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct sta_priv 		*pstapriv = &padapter->stapriv;
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	struct sta_info 		*psta = NULL;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u8 *ele_start = (&(pIE->data[0]) + 1);
	u8 supp_mcs_len = 4;

	if (pIE == NULL)
		return;

	if (phepriv->he_option == _FALSE)
		return;

	psta = rtw_get_stainfo(pstapriv, cur_network->MacAddress);
	if (psta == NULL)
		return;
	if (psta->phl_sta == NULL)
		return;

	phl_sta = psta->phl_sta;

	/* HE MAC Caps */
	HE_mac_caps_handler(padapter, phl_sta, ele_start);
	ele_start += HE_CAP_ELE_MAC_CAP_LEN;

	/* HE PHY Caps */
	HE_phy_caps_handler(padapter, phl_sta, ele_start, &supp_mcs_len);
	ele_start += HE_CAP_ELE_PHY_CAP_LEN;

	/* HE Supp MCS Set */
	HE_supp_mcs_handler(padapter, phl_sta, ele_start, supp_mcs_len);
	ele_start += supp_mcs_len;

	/* HE PPE Thresholds */
	HE_ppe_thre_handler(padapter, phl_sta, ele_start);

	pmlmeinfo->HE_enable = 1;
}

void HE_operation_handler(_adapter *padapter, PNDIS_802_11_VARIABLE_IEs pIE)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct sta_priv 		*pstapriv = &padapter->stapriv;
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network);
	struct sta_info 		*psta = NULL;
	struct rtw_phl_stainfo_t *phl_sta = NULL;
	u8 *ele_start = (&(pIE->data[0]) + 1);

	if (pIE == NULL)
		return;

	if (phepriv->he_option == _FALSE)
		return;

	psta = rtw_get_stainfo(pstapriv, cur_network->MacAddress);
	if (psta == NULL)
		return;
	if (psta->phl_sta == NULL)
		return;

	phl_sta = psta->phl_sta;

	phl_sta->asoc_cap.er_su = !GET_HE_OP_PARA_ER_SU_DISABLE(ele_start);
}

static int rtw_build_he_mac_caps(struct protocol_cap_t *proto_cap, u8 *pbuf)
{
	/* Set HE MAC Capabilities Information */

	int info_len = HE_CAP_ELE_MAC_CAP_LEN;

	if (proto_cap->htc_rx)
		SET_HE_MAC_CAP_HTC_HE_SUPPORT(pbuf, 1);

	if (proto_cap->twt & BIT(0))
		SET_HE_MAC_CAP_TWT_REQUESTER_SUPPORT(pbuf, 1);

	if (proto_cap->twt & BIT(1))
		SET_HE_MAC_CAP_TWT_RESPONDER_SUPPORT(pbuf, 1);

	if (proto_cap->trig_padding)
		SET_HE_MAC_CAP_TRI_FRAME_PADDING_DUR(pbuf,
			proto_cap->trig_padding);

	if (proto_cap->all_ack)
		SET_HE_MAC_CAP_ALL_ACK_SUPPORT(pbuf, 1);

	if (proto_cap->htc_rx && (proto_cap->a_ctrl & BIT(0)))
		SET_HE_MAC_CAP_TRS_SUPPORT(pbuf, 1);

	if (proto_cap->a_ctrl & BIT(1))
		SET_HE_MAC_CAP_BRS_SUPPORT(pbuf, 1);

	if (proto_cap->twt & BIT(2))
		SET_HE_MAC_CAP_BC_TWT_SUPPORT(pbuf, 1);

	if (proto_cap->htc_rx && (proto_cap->a_ctrl & BIT(2)))
		SET_HE_MAC_CAP_OM_CTRL_SUPPORT(pbuf, 1);

	SET_HE_MAC_CAP_MAX_AMPDU_LEN_EXP_EXT(pbuf, 2);

	if (proto_cap->twt & BIT(3))
		SET_HE_MAC_CAP_FLEX_TWT_SCHED_SUPPORT(pbuf, 1);

	if (proto_cap->twt & BIT(4))
		SET_HE_MAC_CAP_PSR_RESPONDER(pbuf, 1);

	if (proto_cap->ops)
		SET_HE_MAC_CAP_OPS_SUPPORT(pbuf, 1);

	if (proto_cap->amsdu_in_ampdu)
		SET_HE_MAC_CAP_AMSDU_NOT_UNDER_BA_IN_ACK_EN_AMPDU(pbuf, 1);

	if (proto_cap->twt & BIT(5))
		SET_HE_MAC_CAP_HE_SUB_CH_SELECTIVE_TX(pbuf, 1);

	if (proto_cap->ht_vht_trig_rx)
		SET_HE_MAC_CAP_HT_VHT_TRIG_FRAME_RX(pbuf, 1);

	return info_len;
}

static int rtw_build_he_phy_caps(_adapter *padapter,struct protocol_cap_t *proto_cap,
                                 struct role_cap_t *role_cap,
                                 struct hal_spec_t *hal_spec,
                                 u8 *pbuf)
{
	/* struct rtw_chan_def *chan_def = &(wrole->chandef); */
	/* Set HE PHY Capabilities Information */
	int info_len = HE_CAP_ELE_PHY_CAP_LEN;
	struct rtw_phl_com_t *phl_com = padapter->dvobj->phl_com;
#ifdef CONFIG_RTW_MULTI_DEV_MULTI_BAND
	struct dev_cap_t *dev_cap = &phl_com->dev_cap;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	_adapter *primary_adapter = GET_PRIMARY_ADAPTER(padapter);
	u8 cw_cap = 0;
	if (dev_cap->band_sup & BAND_CAP_5G) {
		if ((dev_cap->bw_sup & BW_CAP_80_80M) && (REGSTY_BW_5G(pregistrypriv) & BW_CAP_80_80M))
			cw_cap |= BIT(3);
		if ((dev_cap->bw_sup & BW_CAP_160M) && (REGSTY_BW_5G(pregistrypriv) & BW_CAP_160M))
			cw_cap |= BIT(2);
		if ((dev_cap->bw_sup & BW_CAP_80M || dev_cap->bw_sup & BW_CAP_40M) && 
			(REGSTY_BW_5G(pregistrypriv) & BW_CAP_160M || 
			 REGSTY_BW_5G(pregistrypriv) & BW_CAP_80M || 
			 REGSTY_BW_5G(pregistrypriv) & BW_CAP_40M))
			cw_cap |= BIT(1);
	}
	if ((dev_cap->band_sup & BAND_CAP_2G) && (dev_cap->bw_sup & BW_CAP_40M)
           	&& (REGSTY_BW_2G(pregistrypriv) & BW_CAP_40M))
		cw_cap |= BIT(0);

	SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, cw_cap);

#else /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */
#if 1
	SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, (BIT(0) | BIT(1)));
#else
	u8 bw_cap = 0;

	if (phy_cap->bw_sup & BW_CAP_40M)
		bw_cap |= BIT(0);
	if (phy_cap->bw_sup & BW_CAP_80M)
		bw_cap |= BIT(1);

	if (chan_def->band == BAND_ON_24G) {
		if (chan_def->bw == CHANNEL_WIDTH_40)
			SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, BIT(0));
	} else if (chan_def->band == BAND_ON_5G) {
		if (chan_def->bw == CHANNEL_WIDTH_80)
			SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, BIT(1));
		else if (chan_def->bw == CHANNEL_WIDTH_160)
			SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, (BIT(1) | BIT(2)));
		else if (chan_def->bw == CHANNEL_WIDTH_80_80)
			SET_HE_PHY_CAP_SUPPORT_CHAN_WIDTH_SET(pbuf, (BIT(1) | BIT(3)));
	}
#endif
#endif /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */

	SET_HE_PHY_CAP_DEVICE_CLASS(pbuf, HE_DEV_CLASS_A);

	if (proto_cap->he_ldpc)
		SET_HE_PHY_CAP_LDPC_IN_PAYLOAD(pbuf, 1);
#ifdef RTW_WKARD_BEACON
	DBGP("force enable RX ldpc\n"); //shall enable LDPC for Intel AX200
	SET_HE_PHY_CAP_LDPC_IN_PAYLOAD(pbuf, 1);
#endif


	SET_HE_PHY_CAP_SU_PPDU_1X_LTF_0_POINT_8_GI(pbuf, 1);

	SET_HE_PHY_CAP_NDP_4X_LTF_3_POINT_2_GI(pbuf, 1);

	if (proto_cap->stbc_he_tx)
		SET_HE_PHY_CAP_STBC_TX_LESS_THAN_80MHZ(pbuf, 1);
#ifdef RTW_WKARD_BEACON
	DBGP("no RX stbc \n");
#else
	if (proto_cap->stbc_he_rx)
		SET_HE_PHY_CAP_STBC_RX_LESS_THAN_80MHZ(pbuf, 1);
#endif

	if (proto_cap->doppler_tx)
		SET_HE_PHY_CAP_DOPPLER_TX(pbuf, 1);

	if (proto_cap->doppler_rx)
		SET_HE_PHY_CAP_DOPPLER_RX(pbuf, 1);

	if (proto_cap->dcm_max_const_tx)
		SET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_TX(pbuf,
			proto_cap->dcm_max_const_tx);

	if (proto_cap->dcm_max_nss_tx)
		SET_HE_PHY_CAP_DCM_MAX_NSS_TX(pbuf, 1);

	if (proto_cap->dcm_max_const_rx)
		SET_HE_PHY_CAP_DCM_MAX_CONSTELLATION_RX(pbuf,
			proto_cap->dcm_max_const_rx);

	if (proto_cap->dcm_max_nss_rx)
		SET_HE_PHY_CAP_DCM_MAX_NSS_RX(pbuf, 1);

	if (proto_cap->partial_bw_su_in_mu)
		SET_HE_PHY_CAP_RX_PARTIAL_BW_SU_IN_20MHZ_MUPPDU(pbuf, 1);

	if (proto_cap->he_su_bfmr)
		SET_HE_PHY_CAP_SU_BFER(pbuf, 1);

	if (proto_cap->he_su_bfme)
		SET_HE_PHY_CAP_SU_BFEE(pbuf, 1);

	if (proto_cap->he_mu_bfmr)
		SET_HE_PHY_CAP_MU_BFER(pbuf, 1);

	if (proto_cap->bfme_sts)
		SET_HE_PHY_CAP_BFEE_STS_LESS_THAN_80MHZ(pbuf,
			proto_cap->bfme_sts);

	if (proto_cap->bfme_sts_greater_80mhz)
		SET_HE_PHY_CAP_BFEE_STS_GREATER_THAN_80MHZ(pbuf,
			proto_cap->bfme_sts_greater_80mhz);

	if (proto_cap->num_snd_dim)
		SET_HE_PHY_CAP_NUM_SND_DIMEN_LESS_THAN_80MHZ(pbuf,
			proto_cap->num_snd_dim);

	if (proto_cap->num_snd_dim_greater_80mhz)
		SET_HE_PHY_CAP_NUM_SND_DIMEN_GREATER_THAN_80MHZ(pbuf,
			proto_cap->num_snd_dim_greater_80mhz);

	if (proto_cap->ng_16_su_fb)
		SET_HE_PHY_CAP_NG_16_SU_FEEDBACK(pbuf, 1);

	if (proto_cap->ng_16_mu_fb)
		SET_HE_PHY_CAP_NG_16_MU_FEEDBACK(pbuf, 1);

	if (proto_cap->cb_sz_su_fb)
		SET_HE_PHY_CAP_CODEBOOK_4_2_SU_FEEDBACK(pbuf, 1);

	if (proto_cap->cb_sz_mu_fb)
		SET_HE_PHY_CAP_CODEBOOK_7_5_MU_FEEDBACK(pbuf, 1);

	if (proto_cap->trig_su_bfm_fb)
		SET_HE_PHY_CAP_TRIG_SUBF_FEEDBACK(pbuf, 1);

	if (proto_cap->trig_mu_bfm_fb)
		SET_HE_PHY_CAP_TRIG_MUBF_PARTIAL_BW_FEEDBACK(pbuf, 1);

	if (proto_cap->trig_cqi_fb)
		SET_HE_PHY_CAP_TRIG_CQI_FEEDBACK(pbuf, 1);

	if (proto_cap->partial_bw_su_er)
		SET_HE_PHY_CAP_PARTIAL_BW_EXT_RANGE(pbuf, 1);

	if (primary_adapter->registrypriv.wifi_mib.ppe_thresh_ie_en)
		SET_HE_PHY_CAP_PPE_THRESHOLD_PRESENT(pbuf, 1);

	if (proto_cap->pwr_bst_factor)
		SET_HE_PHY_CAP_PWR_BOOST_FACTOR_SUPPORT(pbuf, 1);

	SET_HE_PHY_CAP_SU_MU_PPDU_4X_LTF_0_POINT_8_GI(pbuf, 1);

	if (proto_cap->max_nc)
		SET_HE_PHY_CAP_MAX_NC(pbuf, proto_cap->max_nc);

	if (proto_cap->stbc_tx_greater_80mhz)
		SET_HE_PHY_CAP_STBC_TX_GREATER_THAN_80MHZ(pbuf, 1);

	if (proto_cap->stbc_rx_greater_80mhz)
		SET_HE_PHY_CAP_STBC_RX_GREATER_THAN_80MHZ(pbuf, 1);

	SET_HE_PHY_CAP_SUPPORT_242_TONE_RU_24G(pbuf, 1);
	SET_HE_PHY_CAP_SUPPORT_242_TONE_RU_5G(pbuf, 1);

	SET_HE_PHY_CAP_ERSU_PPDU_4X_LTF_0_POINT_8_GI(pbuf, 1);
	SET_HE_PHY_CAP_ERSU_PPDU_1X_LTF_0_POINT_8_GI(pbuf, 1);

	if (proto_cap->dcm_max_ru)
		SET_HE_PHY_CAP_DCM_MAX_RU(pbuf, proto_cap->dcm_max_ru);

	if (proto_cap->long_sigb_symbol)
		SET_HE_PHY_CAP_LONGER_THAN_16_HESIGB_OFDM_SYM(pbuf, 1);

	if (proto_cap->non_trig_cqi_fb)
		SET_HE_PHY_CAP_NON_TRIGGER_CQI_FEEDBACK(pbuf, 1);

	if (proto_cap->tx_1024q_ru)
		SET_HE_PHY_CAP_TX_1024_QAM_LESS_THAN_242_TONE_RU(pbuf, 1);

	if (proto_cap->rx_1024q_ru)
		SET_HE_PHY_CAP_RX_1024_QAM_LESS_THAN_242_TONE_RU(pbuf, 1);

	if (proto_cap->fbw_su_using_mu_cmprs_sigb)
		SET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_CMP_SIGB(pbuf, 1);

	if (proto_cap->fbw_su_using_mu_non_cmprs_sigb)
		SET_HE_PHY_CAP_RX_FULLBW_SU_USE_MUPPDU_NONCMP_SIGB(pbuf, 1);

	if (proto_cap->pkt_padding)
		SET_HE_PHY_CAP_NOMINAL_PACKET_PADDING(pbuf,
			proto_cap->pkt_padding);

	return info_len;
}

static int rtw_build_he_supp_mcs(_adapter *padapter,struct protocol_cap_t *proto_cap, u8 *pbuf)
{

	 /* struct rtw_chan_def *chan_def = &(wrole->chandef); */

	/* Set HE Supported MCS and NSS Set */

	int info_len = 4;
	struct rtw_phl_com_t *phl_com = padapter->dvobj->phl_com;
	struct dev_cap_t *dev_cap = &phl_com->dev_cap;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;

	_rtw_memset(pbuf, HE_MSC_NOT_SUPP_BYTE, info_len);
	_rtw_memcpy(pbuf, proto_cap->he_rx_mcs, HE_CAP_ELE_SUPP_MCS_LEN_RX_80M);
	_rtw_memcpy(pbuf + 2, proto_cap->he_tx_mcs, HE_CAP_ELE_SUPP_MCS_LEN_TX_80M);
	if (dev_cap->band_sup & BAND_CAP_5G) {
		if (REGSTY_BW_5G(pregistrypriv) & BW_CAP_160M) {
			_rtw_memcpy(pbuf + 4, proto_cap->he_rx_mcs + 2, HE_CAP_ELE_SUPP_MCS_LEN_RX_160M);
			_rtw_memcpy(pbuf + 6, proto_cap->he_tx_mcs + 2, HE_CAP_ELE_SUPP_MCS_LEN_TX_160M);
			info_len += 4;
		}
	}

	RTW_PRINT("[%s] info_len:%d\n", __func__, info_len);	
	return info_len;
}

static int rtw_build_he_ppe_thre(_adapter *adapter, struct protocol_cap_t *proto_cap, u8 *pbuf)
{
	/* Set HE PPE Thresholds (optional) */
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct rtw_phl_com_t *phl_com = dvobj->phl_com;
	struct dev_cap_t *dev_cap = &phl_com->dev_cap;
	struct registry_priv *pregistrypriv = &adapter->registrypriv;
	u8 nsts = 0, rumsk = 0, i, j, offset, shift;
	u8 ppe8, ppe16;
	u8 bw_mode;
	int info_len = 0;
	
	nsts = (GET_HAL_TX_NSS(adapter_to_dvobj(adapter)) - 1);
	SET_HE_CAP_PPE_NSTS(pbuf, nsts);
	
	if (dev_cap->band_sup & BAND_CAP_5G)
		bw_mode = REGSTY_BW_5G(pregistrypriv);
	else
		bw_mode = REGSTY_BW_2G(pregistrypriv);
	
	if (bw_mode & BW_CAP_160M) {
		rumsk = (HE_CAP_ELE_PPE_RU_INDEX_BITMASK_242 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_484 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_996 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_2_996);
	} else if (bw_mode & BW_CAP_80M) {
		rumsk = (HE_CAP_ELE_PPE_RU_INDEX_BITMASK_242 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_484 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_996);
	} else if (bw_mode & BW_CAP_40M) {
		rumsk = (HE_CAP_ELE_PPE_RU_INDEX_BITMASK_242 |
				HE_CAP_ELE_PPE_RU_INDEX_BITMASK_484);
	} else if (bw_mode & BW_CAP_20M) {
		rumsk = HE_CAP_ELE_PPE_RU_INDEX_BITMASK_242;
	}
	SET_HE_CAP_PPE_PU_IDX_BITMASK(pbuf, rumsk);
	shift = 7;

	for (i = 0; i <= nsts; i ++) {
		for (j = 0; j <= CHANNEL_WIDTH_160; j++) {
			if (rumsk & (BIT(0) << j)) {
				offset = shift / 8;

				ppe16 = HE_CAP_ELE_PPE_CONSTELLATION_INDEX_BPSK;
				ppe8 = HE_CAP_ELE_PPE_CONSTELLATION_INDEX_NONE;

				proto_cap->ppe_thr[i][j] = ((ppe16 & 0x07) | ((ppe8 & 0x07) << 3));
				SET_BITS_TO_LE_2BYTE(pbuf + offset, shift % 8, 6, proto_cap->ppe_thr[i][j]);
				shift += 6;
			}
		}
	}

	info_len = (shift / 8) + (shift % 8? 1: 0);
	return info_len;
}

static int rtw_build_ppe_test(u8 *pbuf_phy, u8 *pbuf, WLAN_BSSID_EX *pbss_network)
{
	int info_len = 0;

#ifdef WIFI_LOGO_HE_4_5_3
	if (pbss_network->Ssid.SsidLength == 8
		&& _rtw_memcmp(pbss_network->Ssid.Ssid, "HE-4.5.3", 8))
	{
		unsigned char ppe[6] = {0x39, 0x1c, 0xc7, 0x71, 0x1c, 0x07};

		info_len = 6;

		SET_HE_PHY_CAP_PPE_THRESHOLD_PRESENT(pbuf_phy, 1);
		_rtw_memcpy(pbuf, ppe, info_len);
	}
#endif

	return info_len;
}

u32 rtw_get_dft_he_cap_ie(_adapter *padapter, struct phy_cap_t *phy_cap,
		struct protocol_cap_t *proto_cap, u8 *pbuf)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct he_priv	*phepriv = &pmlmepriv->hepriv;
	u32 he_cap_total_len = 0, len = 0;
	u8* pcap_start = phepriv->he_cap;
	u8* pcap = pcap_start;
	struct role_cap_t default_cap;

	_rtw_memset(pcap, 0, HE_CAP_ELE_MAX_LEN);

	/* Ele ID Extension */
	*pcap++ = WLAN_EID_EXTENSION_HE_CAPABILITY;

	/* HE MAC Caps */
	pcap += rtw_build_he_mac_caps(proto_cap, pcap);

	/* HE PHY Caps */
	rtw_phl_get_dft_cap(padapter->dvobj->phl, 0, &default_cap);
	pcap += rtw_build_he_phy_caps(padapter,proto_cap,
	                              &default_cap,
	                              phl_get_ic_spec(padapter->dvobj->phl_com),
	                              pcap);

	/* HE Supported MCS and NSS Set */
	pcap += rtw_build_he_supp_mcs(padapter,proto_cap, pcap);

	/* HE PPE Thresholds (optional) */
	pcap += rtw_build_he_ppe_thre(padapter, proto_cap, pcap);

	he_cap_total_len = (pcap - pcap_start);

	pbuf = rtw_set_ie(pbuf, WLAN_EID_EXTENSION, he_cap_total_len, pcap_start, &len);

	return len;
}

u32 rtw_build_he_cap_ie(_adapter *padapter, u8 *pbuf)
{
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *proto_cap = &(wrole->proto_role_cap);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct he_priv	*phepriv = &pmlmepriv->hepriv;
	_adapter *primary_adapter = GET_PRIMARY_ADAPTER(padapter);
	WLAN_BSSID_EX *pbss_network = (WLAN_BSSID_EX *)&pmlmepriv->cur_network.network;
	u32 he_cap_total_len = 0, len = 0;
	u8* pcap_start = phepriv->he_cap;
	u8* pcap = pcap_start;
	u8 *pcap_phy;
	u8 *pcap_ppe;
	struct hal_spec_t *hal_spec = phl_get_ic_spec(padapter->dvobj->phl_com);

	_rtw_memset(pcap, 0, HE_CAP_ELE_MAX_LEN);

#if 0 // def CONFIG_RTW_MULTI_DEV_MULTI_BAND
	DBGP("chnl_width=%d +++\n", proto_cap->chnl_width);

	if (BAND_ON_24G == rtw_get_phyband_on_dev(padapter))
		proto_cap->chnl_width = CHANNEL_WIDTH_40;
	else
		proto_cap->chnl_width = CHANNEL_WIDTH_80;

	DBGP("chnl_width=%d ---\n", proto_cap->chnl_width);

#endif /* CONFIG_RTW_MULTI_DEV_MULTI_BAND */

	/* Ele ID Extension */
	*pcap++ = WLAN_EID_EXTENSION_HE_CAPABILITY;

	/* HE MAC Caps */
	pcap += rtw_build_he_mac_caps(proto_cap, pcap);

	/* HE PHY Caps */
	pcap_phy = pcap;
	pcap += rtw_build_he_phy_caps(padapter,proto_cap,
	                              &wrole->cap,
	                              hal_spec,
	                              pcap);

	/* HE Supported MCS and NSS Set */
	pcap += rtw_build_he_supp_mcs(padapter,proto_cap, pcap);

	/* HE PPE Thresholds (optional) */
	pcap_ppe = pcap;
	if (primary_adapter->registrypriv.wifi_mib.ppe_thresh_ie_en)
		pcap_ppe += rtw_build_he_ppe_thre(padapter, proto_cap, pcap);
	else
		pcap_ppe += rtw_build_ppe_test(pcap_phy, pcap_ppe, pbss_network);
	pcap = pcap_ppe;

	he_cap_total_len = (pcap - pcap_start);

	pbuf = rtw_set_ie(pbuf, WLAN_EID_EXTENSION, he_cap_total_len, pcap_start, &len);

	return len;
}

u32 rtw_restructure_he_ie(_adapter *padapter, u8 *in_ie, u8 *out_ie, uint in_len, uint *pout_len)
{
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct he_priv	*phepriv = &pmlmepriv->hepriv;
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	u32	ielen;
	u8 *out_he_op_ie, *vht_op_ie, *he_cap_ie, *he_op_ie;
	u8 he_cap_eid_ext = WLAN_EID_EXTENSION_HE_CAPABILITY;
	u8 he_op_eid_ext = WLAN_EID_EXTENSION_HE_OPERATION;

	rtw_he_use_default_setting(padapter);

	if (is_supported_5g(regsty->band_type) && rtw_hw_chk_band_cap(dvobj, BAND_CAP_5G))
	{
		vht_op_ie = rtw_get_ie(in_ie + 12, WLAN_EID_VHT_OPERATION, &ielen, in_len - 12);
		if (!vht_op_ie || ielen != VHT_OP_IE_LEN)
			goto exit;
	}

	he_cap_ie = rtw_get_ie_ex(in_ie + 12, in_len - 12, WLAN_EID_EXTENSION, &he_cap_eid_ext, 1, NULL, &ielen);
	if (!he_cap_ie || (ielen > (HE_CAP_ELE_MAX_LEN + 2)))
		goto exit;
	he_op_ie = rtw_get_ie_ex(in_ie + 12, in_len - 12, WLAN_EID_EXTENSION, &he_op_eid_ext, 1, NULL, &ielen);
	if (!he_op_ie || (ielen > (HE_OPER_ELE_MAX_LEN + 2)))
		goto exit;

	*pout_len += rtw_build_he_cap_ie(padapter, out_ie + *pout_len);

	phepriv->he_option = _TRUE;

exit:
	return phepriv->he_option;
}

static int rtw_build_he_oper_params(_adapter *padapter, u8 *pbuf)
{
	/* Set HE Operation Parameters */

	int info_len = HE_OPER_PARAMS_LEN;

	SET_HE_OP_PARA_DEFAULT_PE_DUR(pbuf, 0x4);

	return info_len;
}

static int rtw_build_he_oper_bss_color_info(_adapter *padapter, u8 *pbuf)
{
	/* Set BSS Color Information */
	int info_len = HE_OPER_BSS_COLOR_INFO_LEN;
	struct rtw_wifi_role_t *wrole = padapter->phl_role;
	struct protocol_cap_t *proto_cap = &(wrole->proto_role_cap);

	SET_HE_OP_BSS_COLOR_INFO_BSS_COLOR(pbuf, proto_cap->bsscolor);

#ifdef CONFIG_RTW_AP_BS_DECT
	SET_HE_OP_BSS_COLOR_INFO_BSS_COLOR_DISABLE(pbuf, proto_cap->bc_disabled);
#endif
	return info_len;
}

static int rtw_build_he_oper_basic_mcs_set(_adapter *padapter, u8 *pbuf)
{
	/* Set Basic HE-MCS and NSS Set */

	int info_len = HE_OPER_BASIC_MCS_LEN;

	_rtw_memset(pbuf, HE_MSC_NOT_SUPP_BYTE, info_len);

 	#if 0 // ori
	SET_HE_OP_BASIC_MCS_1SS(pbuf, HE_MCS_SUPP_MSC0_TO_MSC11);
	SET_HE_OP_BASIC_MCS_2SS(pbuf, HE_MCS_SUPP_MSC0_TO_MSC11);
	#else
	/* refer to 11AX Broadcom TestAP and ASUS_1800 */
	/* Fix AX200 1x1 will not send HE in assoc_req */
	SET_HE_OP_BASIC_MCS_1SS(pbuf, HE_MCS_SUPP_MSC0_TO_MSC7);
	//SET_HE_OP_BASIC_MCS_2SS(pbuf, HE_MCS_SUPP_MSC0_TO_MSC11);
	#endif

	return info_len;
}

static int rtw_build_vht_oper_info(_adapter *padapter, u8 *pbuf)
{
	/* Set VHT Operation Information (optional) */

	int info_len = 0;

	return info_len;
}

static int rtw_build_max_cohost_bssid_ind(_adapter *padapter, u8 *pbuf)
{
	/* Set Max Co-Hosted BSSID Indicator (optional) */

	int info_len = 0;

	return info_len;
}

static int rtw_build_6g_oper_info(_adapter *padapter, u8 *pbuf)
{
	/* Set 6GHz Operation Information (optional) */

	int info_len = 0;

	return info_len;
}

static int rtw_build_mu_edca_QoS_info(_adapter *padapter, u8 *pbuf)
{
	int info_len = MU_EDCA_QoS_INFO_LEN;

	_rtw_memset(pbuf, 0, info_len);

	SET_MU_EDCA_QoS_INFO(pbuf, 0x1);

	return info_len;
}

static int rtw_build_mu_edca_MUAC_BE(_adapter *padapter, u8 *pbuf)
{
	int info_len = MU_EDCA_MUAC_BE_LEN;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(pdvobjpriv);
	struct ru_grp_table *rugrptable = &phl_com->rugrptable[padapter->phl_role->id];
	struct ru_common_ctrl *ru_ctrl = &rugrptable->ru_ctrl;

	_rtw_memset(pbuf, 0, info_len);

	SET_MU_EDCA_AIC_AIFSN(pbuf, 0X0);
	//SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xff);
	//SET_MU_EDCA_TIMER(pbuf, 0xff);
	SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xa4);
	//SET_MU_EDCA_TIMER(pbuf, 0x08);
	SET_MU_EDCA_TIMER(pbuf, ru_ctrl->mu_edca);

	return info_len;
}

static int rtw_build_mu_edca_MUAC_BK(_adapter *padapter, u8 *pbuf)
{
	int info_len = MU_EDCA_MUAC_BK_LEN;

	_rtw_memset(pbuf, 0, info_len);

	SET_MU_EDCA_AIC_AIFSN(pbuf, 0x20);
	//SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xff);
	//SET_MU_EDCA_TIMER(pbuf, 0xff);
	SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xa4);
	SET_MU_EDCA_TIMER(pbuf, 0x08);

	return info_len;
}

static int rtw_build_mu_edca_MUAC_VI(_adapter *padapter, u8 *pbuf)
{
	int info_len = MU_EDCA_MUAC_VI_LEN;

	_rtw_memset(pbuf, 0, info_len);

	SET_MU_EDCA_AIC_AIFSN(pbuf, 0x40);
	//SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xff);
	//SET_MU_EDCA_TIMER(pbuf, 0xff);
	SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0x43);
	SET_MU_EDCA_TIMER(pbuf, 0x08);

	return info_len;
}

static int rtw_build_mu_edca_MUAC_VO(_adapter *padapter, u8 *pbuf)
{
	int info_len = MU_EDCA_MUAC_VO_LEN;

	_rtw_memset(pbuf, 0, info_len);

	SET_MU_EDCA_AIC_AIFSN(pbuf, 0x60);
	//SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0xff);
	//SET_MU_EDCA_TIMER(pbuf, 0xff);
	SET_MU_EDCA_ECW_MIN_MAX(pbuf, 0x32);
	SET_MU_EDCA_TIMER(pbuf, 0x08);

	return info_len;
}

u32	rtw_build_he_operation_ie(_adapter *padapter, u8 *pbuf)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
	u32 he_oper_total_len = 0, len = 0;
	u8* poper_start = phepriv->he_op;
	u8* poper = poper_start;

	_rtw_memset(poper, 0, HE_OPER_ELE_MAX_LEN);

	/* Ele ID Extension */
	*poper++ = WLAN_EID_EXTENSION_HE_OPERATION;

	/* HE Oper Params */
	poper += rtw_build_he_oper_params(padapter, poper);

	/* BSS Color Info */
	poper += rtw_build_he_oper_bss_color_info(padapter, poper);

	/* Basic MCS and NSS Set */
	poper += rtw_build_he_oper_basic_mcs_set(padapter, poper);

	/* VHT Oper Info */
	poper += rtw_build_vht_oper_info(padapter, poper);

	/* Max Co-Hosted BSSID Indicator */
	poper += rtw_build_max_cohost_bssid_ind(padapter, poper);

	/* 6G Oper Info */
	poper += rtw_build_6g_oper_info(padapter, poper);

	he_oper_total_len = (poper - poper_start);

	pbuf = rtw_set_ie(pbuf, WLAN_EID_EXTENSION, he_oper_total_len, poper_start, &len);

	return len;
}

u32	rtw_build_mu_edca_ie(_adapter *padapter, u8 *pbuf)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct he_priv		*phepriv = &pmlmepriv->hepriv;
	u32 he_oper_total_len = 0, len = 0;
	u8* poper_start = phepriv->mu_edca;
	u8* poper = poper_start;

	_rtw_memset(poper, 0, MU_EDCA_ELE_MAX_LEN);

	/* Ele ID Extension */
	*poper++ = WLAN_EID_EXTENSION_MU_EDCA;

	/* MU EDCA QoS info */
	poper += rtw_build_mu_edca_QoS_info(padapter, poper);

	/* MU AC BE */
	poper += rtw_build_mu_edca_MUAC_BE(padapter, poper);

	/* MU AC BK */
	poper += rtw_build_mu_edca_MUAC_BK(padapter, poper);

	/* MU AC VI */
	poper += rtw_build_mu_edca_MUAC_VI(padapter, poper);

	/* MU AC VO */
	poper += rtw_build_mu_edca_MUAC_VO(padapter, poper);

	he_oper_total_len = (poper - poper_start);

	pbuf = rtw_set_ie(pbuf, WLAN_EID_EXTENSION, he_oper_total_len, poper_start, &len);

	return len;
}

void HEOnAssocRsp(_adapter *padapter)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct vht_priv		*pvhtpriv = &pmlmepriv->vhtpriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	ht_AMPDU_len;

	if (!pmlmeinfo->VHT_enable)
		return;

	if (!pmlmeinfo->HE_enable)
		return;

	RTW_INFO("%s\n", __FUNCTION__);

	/* AMPDU related settings here ? */
}

void rtw_he_ies_attach(_adapter *padapter, WLAN_BSSID_EX *pnetwork)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 he_cap_eid_ext = WLAN_EID_EXTENSION_HE_CAPABILITY;
	u8 cap_len, operation_len, mu_edca_len;
	uint len = 0;
	sint ie_len = 0;
	u8 *p = NULL;

	p = rtw_get_ie_ex(pnetwork->IEs + _BEACON_IE_OFFSET_, pnetwork->IELength - _BEACON_IE_OFFSET_,
		WLAN_EID_EXTENSION, &he_cap_eid_ext, 1, NULL, &ie_len);
	if (p && ie_len > 0)
		return;

	rtw_he_use_default_setting(padapter);

	cap_len = rtw_build_he_cap_ie(padapter, pnetwork->IEs + pnetwork->IELength);
	pnetwork->IELength += cap_len;

	operation_len = rtw_build_he_operation_ie(padapter, pnetwork->IEs + pnetwork->IELength);
	pnetwork->IELength += operation_len;

	mu_edca_len = rtw_build_mu_edca_ie(padapter, pnetwork->IEs + pnetwork->IELength);
	pnetwork->IELength += mu_edca_len;

	pmlmepriv->hepriv.he_option = _TRUE;
}

void rtw_he_ies_detach(_adapter *padapter, WLAN_BSSID_EX *pnetwork)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 he_cap_eid_ext = WLAN_EID_EXTENSION_HE_CAPABILITY;
	u8 he_op_eid_ext = WLAN_EID_EXTENSION_HE_OPERATION;
	u8 he_mu_edca_eid_ext = WLAN_EID_EXTENSION_MU_EDCA;

	rtw_remove_bcn_ie_ex(padapter, pnetwork, WLAN_EID_EXTENSION, &he_cap_eid_ext, 1);
	rtw_remove_bcn_ie_ex(padapter, pnetwork, WLAN_EID_EXTENSION, &he_op_eid_ext, 1);
	rtw_remove_bcn_ie_ex(padapter, pnetwork, WLAN_EID_EXTENSION, &he_mu_edca_eid_ext, 1);

	pmlmepriv->hepriv.he_option = _FALSE;
}

u8 rtw_he_htc_en(_adapter *padapter, struct sta_info *psta)
{
#if 1
	return 0;
#else
	/* CONFIG_80211AX_HE_TODO */
#endif
}

void rtw_he_fill_htc(_adapter *padapter, struct pkt_attrib *pattrib, u32 *phtc_buf)
{
	SET_HE_VAR_HTC(phtc_buf);

	/* CONFIG_80211AX_HE_TODO */
}

u8 rtw_he_shortest_supported_giltf(_adapter *padapter, struct sta_info *psta)
{
	u8 supp_giltf = 0;
	u8 giltf_level = 0;

	if (psta->phl_sta->wrole)
		supp_giltf = psta->phl_sta->wrole->proto_role_cap.ltf_gi;

	if (supp_giltf & (BIT(RTW_GILTF_SGI_4XHE08) | BIT(RTW_GILTF_2XHE08) | BIT(RTW_GILTF_1XHE08)))
		giltf_level = 2;
	else if (supp_giltf & (BIT(RTW_GILTF_2XHE16) | BIT(RTW_GILTF_1XHE16)))
		giltf_level = 1;
	else
		giltf_level = 0;

	return giltf_level;
}

const u16 HE_MCS_DATA_RATE[4][3][48] =
{
	{	{14, 29, 43, 58, 87, 117, 131, 146, 175, 195, 219, 243,
		 29, 58, 87, 117, 175, 234, 263, 292, 351, 390, 438, 487,
		 43, 87, 131, 175, 263, 351, 394, 438, 526, 585, 658, 731,
		 58, 117, 175, 234, 351, 468, 526, 585, 702, 780, 877, 975},			// 3.2, 20MHz

		{16, 32, 48, 65, 97, 130, 146, 162, 195, 216, 243, 270,
		 32, 65, 97, 130, 195, 260, 292, 325, 390, 433, 487, 541,
		 48, 97, 146, 195, 292, 390, 438, 487, 585, 650, 731, 812,
		 65, 130, 195, 260, 390, 520, 585, 650, 780, 866, 975, 1083},			// 1.6, 20MHz

		{17, 34, 52, 69, 103, 138, 155, 172, 207, 229, 258, 287,
		 34, 69, 103, 138, 207, 275, 310, 344, 413, 459, 516, 574,
		 52, 103, 155, 206, 310, 413, 465, 516, 620, 688, 774, 860,
		 69, 138, 207, 275, 413, 551, 619, 688, 826, 918, 1032, 1147}			// 0.8, 20MHz
	},

	{	{29, 58, 87, 117, 175, 234, 263, 292, 351, 390, 438, 487,
		 58, 117, 175, 234, 351, 468, 526, 585, 702, 780, 877, 975,
		 87, 175, 263, 351, 526, 702, 789, 877, 1053, 1170, 1316, 1462,
		 117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, 1755, 1950},		// 3.2, 40MHz

		{32, 65, 97, 130, 195, 260, 292, 325, 390, 433, 487, 541,
		 65, 130, 195, 260, 390, 520, 585, 650, 780, 866, 975, 1083,
		 97, 195, 292, 390, 585, 780, 877, 975, 1170, 1300, 1462, 1625,
		 130, 260, 390, 520, 780, 1040, 1170, 1300, 1560, 1733, 1970, 2166},		// 1.6, 40MHz

		{34, 69, 103, 138, 207, 275, 310, 344, 413, 459, 516, 574,
		 69, 138, 207, 275, 413, 551, 619, 688, 826, 918, 1032, 1147,
		 103, 206, 310, 413, 620, 826, 929, 1032, 1239, 1376, 1549, 1721,
		 138, 275, 413, 551, 826, 1101, 1239, 1377, 1652, 1835, 2065, 2294}		// 0.8, 40MHz
	},

	{	{61, 122, 183, 245, 367, 490, 551, 612, 735, 816, 918, 1020,
		 122, 245, 367, 490, 735, 980, 1102, 1225, 1470, 1633, 1837, 2041,
		 183, 367, 551, 735, 1102, 1470, 1653, 1837, 2205, 2450, 2756, 3062,
		 245, 490, 735, 980, 1470, 1960, 2205, 2450, 2940, 3266, 3675, 4083},		// 3.2, 80MHz

		{68, 136, 204, 272, 408, 544, 612, 680, 816, 907, 1020, 1134,
		 136, 272, 408, 544, 816, 1088, 1225, 1361, 1633, 1814, 2041, 2268,
		 204, 408, 612, 816, 1225, 1633, 1837, 2041, 2450, 2722, 3062, 3402,
		 272, 544, 816, 1088, 1633, 2177, 2450, 2722, 3266, 3629, 4083, 4537},		// 1.6, 80MHz

		{72, 144, 216, 288, 432, 577, 649, 721, 865, 961, 1081, 1201,
		 144, 288, 432, 577, 865, 1153, 1297, 1441, 1729, 1922, 2162, 2402,
		 216, 432, 649, 865, 1297, 1730, 1946, 2162, 2594, 2882, 3243, 3603,
		 288, 577, 865, 1153, 1729, 2306, 2594, 2882, 3459, 3843, 4324, 4804}		// 0.8, 80MHz
	},

	{	{122, 245, 367, 490, 735, 980, 1102, 1225, 1470, 1633, 1837, 2041,
		 245, 490, 735, 980, 1470, 1960, 2205, 2450, 2940, 3266, 3675, 4083,
		 367, 735, 1102, 1470, 2205, 2940, 3307, 3675, 4410, 4900, 5512, 6125,
		 490, 980, 1470, 1960, 2940, 3920, 4410, 4900, 5880, 6533, 7350, 8166},	// 3.2, 160MHz

		{136, 272, 408, 544, 816, 1088, 1225, 1361, 1633, 1814, 2041, 2268,
		 272, 544, 816, 1088, 1633, 2177, 2450, 2722, 3266, 3629, 4083, 4537,
		 408, 816, 1225, 1633, 2450, 3266, 3675, 4083, 4900, 5444, 6125, 6805,
		 544, 1088, 1633, 2177, 3266, 4355, 4900, 5444, 6533, 7259, 8166, 9074},	// 1.6, 160MHz

		{144, 288, 432, 577, 865, 1211, 1297, 1441, 1729, 1922, 2162, 2402,
		 288, 577, 865, 1153, 1729, 2396, 2594, 2882, 3459, 3843, 4324, 4804,
		 432, 865, 1297, 1729, 2594, 3555, 3891, 4324, 5188, 5765, 6485, 7206,
		 577, 1153, 1729, 2306, 3459, 4689, 5188, 5765, 6918, 7686, 8647, 9608}		// 0.8, 160MHz
	}
};

u16 rtw_he_mcs_to_data_rate(u8 bw, u8 gi_ltf, struct he_priv *hepriv)
{
	u8 *pbuf = hepriv->he_cap + 1 + HE_CAP_ELE_MAC_CAP_LEN + HE_CAP_ELE_PHY_CAP_LEN;
	u8 nss, mcs, mcs_idx = 0;
	u16 supp_mcs;
	
	if (gi_ltf > 2)
		gi_ltf = 0;

	if (bw >= CHANNEL_WIDTH_80_80)
		return 0;

	/* lookup for tx mcs */
	if (bw == CHANNEL_WIDTH_160)
		pbuf += (4 + 2);
	else
		pbuf += 2;

	supp_mcs = le16_to_cpu(*(u16 *)pbuf);

	for (nss = 8; nss > 0; nss--) {
		mcs = (supp_mcs >> (2 * (nss - 1))) & (BIT(0) | BIT(1));
		if (mcs != 3) {
			mcs_idx = 12 * (((nss >> 1) > 0) ? (nss >> 1) - 1 : 0);
			switch (mcs) {
			case 0:
				mcs_idx += 7;
				break;
			case 1:
				mcs_idx += 9;
				break;
			case 2:
				mcs_idx += 11;
				break;
			}
			break;
		}
	}

	if (!mcs_idx)
		mcs_idx = 7;

	return HE_MCS_DATA_RATE[bw][gi_ltf][mcs_idx];
}

#endif /* CONFIG_80211AX_HE */

