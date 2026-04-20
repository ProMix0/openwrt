/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
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

#include <drv_types.h>
#include <rtw_rm_fsm.h>
#include "_hal_rate.h"
#include "../phl/phl_headers.h"

#if defined(CONFIG_RTW_MULTI_AP) || defined(CONFIG_WLAN_DE_SUPPORT)
enum tlv_len {
	RTW_TLV_LEN_NULL = 0,	/* 0 octet */
	RTW_TLV_LEN_1 = 1,		/* 1 octet */
	RTW_TLV_LEN_2 = 2,		/* 2 octet */
	RTW_TLV_LEN_3 = 3,		/* 3 octet */
	RTW_TLV_LEN_4 = 4,		/* 4 octet */
	RTW_TLV_LEN_5 = 5,		/* 5 octet */
	RTW_TLV_LEN_6 = 6,		/* 6 octet */
	RTW_TLV_LEN_7 = 7,		/* 7 octet */
	RTW_TLV_LEN_8 = 8,		/* 8 octet */

	RTW_TLV_LEN_MAX
};

#ifdef CONFIG_RTW_MULTI_AP
#define BLOCK_MAC			0x00
#define UNBLOCK_MAC			0x01
#define _MEASUREMENT_REPORT_IE_		39

//set association control block list
struct assoc_control_block_list {
	unsigned char	addr[MAC_ADDR_LEN];
	unsigned short	timer;
};

#ifdef DEBUG_MAP_NL
static struct sock *rtw_map_nl_sock = NULL;
u32	rtw_map_user_pid = 0;
static u32	msg_drop_cnt = 0;
#endif

unsigned char MULTI_AP_OUI[] = {0x50, 0x6F, 0x9A, 0x1B};

//netlink event
#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE   (0)   // 0x00
#define MASK_MSCS_REQUEST_EVENT                                 (1)   // 0x01
#define MASK_SCS_REQUEST_EVENT                                  (2)   // 0x02
#define MASK_BTM_RESPONSE_EVENT                                 (64)  // 0x40
#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN    (128) // 0x80
#define MAP_FRAGMENTIZED_DATA                                   (240) // 0xF0
#define MAP_CHANNEL_CHANGE_NOTIFICATION_MESSAGE                 (254) // 0xFE
#define MAP_GENERAL_NETLINK_MESSAGE                             (255) // 0xFF

#ifdef CONFIG_NSB_MULTI_AP_EVENT
#define MASK_CHANNEL_CHANGE_EVENT                               (1) //0x01
#define MASK_RADAR_DETECT_EVENT                               	(2) //0x02

struct map_event_msg_ch_change {
	u8 netlink_msg_type;
	u8 bssid[MAC_ADDR_LEN];
	u8 reserved[6];
	u8 event_type;
	u8 reason;
	u8 old_ch;
	u8 old_bw;
	u8 old_opclass;
	u8 ch;
	u8 bw;
	u8 opclass;
	u8 hwmode;
};

struct map_event_msg_radar_etect {
	u8 netlink_msg_type;
	u8 bssid[MAC_ADDR_LEN];
	u8 reserved[6];
	u8 event_type;
	u8 ch;
};
#endif

//TLVs
#define TLV_TYPE_AP_HT_CAPABILITIES                             (134) // 0x86
#define TLV_TYPE_AP_VHT_CAPABILITIES                            (135) // 0x87
#define TLV_TYPE_AP_HE_CAPABILITIES                             (136) // 0x88
#define TLV_TYPE_METRIC_REPORT_POLICY                           (138) // 0x8A
#define TLV_TYPE_AP_METRICS                                     (148) // 0x94
#define TLV_TYPE_STA_MAC_ADDRESS_TYPE                           (149) // 0x95
#define TLV_TYPE_ASSOCIATED_STA_LINK_METRICS                    (150) // 0x96
#define TLV_TYPE_BEACON_METRICS_RESPONSE                        (154) // 0x9A
#define TLV_TYPE_AP_CAPABILITY                                  (161) // 0xA1
#define TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS                   (162) // 0xA2

#if defined (CONFIG_RTW_MULTI_AP_R2) && defined (CONFIG_RTW_OPCLASS_CHANNEL_SCAN)
#define TLV_TYPE_CHANNEL_SCAN_RESULT                            (167) // 0xA7
#endif

#ifdef CONFIG_RTW_MULTI_AP_R2
//CAC TLVs
#define TLV_TYPE_CAC_COMPLETION_REPORT                          (175) // 0xAF
#define TLV_TYPE_CAC_STATUS_REPORT                              (177) // 0xB1
#endif

#ifdef CONFIG_RTW_MULTI_AP_R3
#define TLV_TYPE_AP_WIFI_6_CAPABILITIES					        (170) // 0xAA
#define TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT            (176) // 0xB0
#define TLV_TYPE_SERVICE_PRIORITIZATION_RULE                    (185) // 0xB9
#define TLV_TYPE_DSCP_MAPPING_TABLE                             (186) // 0xBA

#ifdef CONFIG_RTW_MULTI_AP_R4
#define TLV_TYPE_SPATIAL_REUSE_REQUEST					        (216) // 0xD8
#define TLV_TYPE_SPATIAL_REUSE_REPORT					        (217) // 0xD9
#define TLV_TYPE_QOS_MANAGEMENT_POLICY					       (219) // 0xDB
#define TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR					       (220) // 0xDC
#endif  /* CONFIG_RTW_MULTI_AP_R4 */

#endif


#ifdef CONFIG_RTW_MULTI_AP_R2
//Profile 2 TLVs
#define TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION                (191) // 0xBF
#define TLV_TYPE_TUNNELED                                       (194) // 0xC2
#define TLV_TYPE_AP_EXTENDED_METRICS                            (199) // 0xC7
#define TLV_TYPE_EXTENDED_ASSOCIATED_STA_LINK_METRIC            (200) // 0xC8
#define TLV_TYPE_STATUS_CODE                                    (201) // 0xC9
#define TLV_TYPE_REASON_CODE                                    (202) // 0xCA
// CAC Completion Action
#define CAC_COMPL_ACTION_MASK                              0b00011000
#define CAC_COMPL_ACTION_RETURN                            0b00001000
#define CAC_COMPL_ACTION_REMAIN                            0b00000000
//CAC max payload length
#define CAC_MAX_PAYLOAD                                          1024
#endif

#define RTW_MAP_MAX_NETLINK_MSG_SIZE					(4096)


/*
	Multi-AP internal function declaration
	naming: _core_map_xxx
	note: only use in rtw_multi_ap.c
*/
void _core_map_get_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	int offset=0;

	result_buf[0] = TLV_TYPE_AP_CAPABILITY;
	offset++;

	result_buf[offset] = 1;
	offset++;

	result_buf[offset] = 128;
	offset++;

	*len += offset;
}

void _core_map_get_ht_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	u8 val = 0;
	u8 rf_type;
	u32 offset = 0;
	struct dvobj_priv *dvobj = NULL;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;

	//not suuport ht
	if (!(pmibpriv->band & WLAN_MD_11N)) {
		*(result_buf+offset) = TLV_TYPE_AP_HT_CAPABILITIES;
		offset++;

		*(result_buf+offset) = 0;
		offset++;

		*len += offset;
		RTW_INFO("%s %u Not support HT wireless mode:%u\n\n", __FUNCTION__, __LINE__, pmibpriv->band);
		return;
	}

	dvobj = adapter_to_dvobj(padapter);
	rf_type = GET_HAL_RFPATH(dvobj);

	//TX spatial stream support
	if (rf_type == RF_2T2R || rf_type == RF_2T3R || rf_type == RF_2T4R || rf_type == RF_4T4R)
		val |= BIT6;
	if (rf_type == RF_3T3R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT7;

	//RX spatial stream support
	if (rf_type == RF_1T2R || rf_type == RF_2T2R || rf_type == RF_2T4R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT4;
	if (rf_type == RF_3T3R || rf_type == RF_2T3R || rf_type == RF_4T4R)
		val |= BIT5;

	//Short GI Support for 20MHz
	if (padapter->mlmepriv.htpriv.sgi_20m)
		val |= BIT3;

	//Short GI Support for 40MHz
	if (padapter->mlmepriv.htpriv.sgi_40m)
		val |= BIT2;

	//HT support for 40MHz
	//if(padapter->mlmepriv.htpriv.)
		val |= BIT1;

	*(result_buf+offset) = TLV_TYPE_AP_HT_CAPABILITIES;
	offset++;

	*(result_buf+offset) = 7;
	offset++;

	_rtw_memcpy(result_buf+offset, adapter_mac_addr(padapter), MAC_ADDR_LEN);
	offset += MAC_ADDR_LEN;

	*(result_buf+offset) = val;
	offset++;

	*len += offset;
}

void _core_map_get_vht_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	u8 val = 0, val2 = 0;
	u8 rf_type;
	u32 offset = 0;
	struct dvobj_priv *dvobj = NULL;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;

	//not support vht
	if (!(pmibpriv->band & WLAN_MD_11AC)) {
		*(result_buf+offset) = TLV_TYPE_AP_VHT_CAPABILITIES;
		offset++;

		*(result_buf+offset) = 0;
		offset++;

		*len += offset;
		RTW_INFO("%s %u Not support VHT wireless mode:%u\n\n", __FUNCTION__, __LINE__, pmibpriv->band);
		return;
	}

	dvobj = adapter_to_dvobj(padapter);
	rf_type = GET_HAL_RFPATH(dvobj);

	//TX spatial stream support (bits 7-5)
	if (rf_type == RF_2T2R || rf_type == RF_2T3R || rf_type == RF_2T4R || rf_type == RF_4T4R)
		val |= BIT5;
	if (rf_type == RF_3T3R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT6;

	//Now the maximum number of supported Tx spatial streams is 4, so bit7 of val is 0.

	//RX spatial stream support (bits 4-2)
	if (rf_type == RF_1T2R || rf_type == RF_2T2R || rf_type == RF_2T4R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT2;
	if (rf_type == RF_3T3R || rf_type == RF_2T3R || rf_type == RF_4T4R)
		val |= BIT3;

	//Now the maximum number of supported Rx spatial streams is 4, so bit4 of val is 0.

	//Short GI Support for 80MHz
	if (padapter->mlmepriv.vhtpriv.sgi_80m || padapter->mlmepriv.vhtpriv.sgi_160m)
		val |= BIT1;

	//Short GI Support for 160MHZ and 80MHZ+80MHZ
	//val |= BIT0;

	//VHT support for 80+80MHz
	//val2 |= BIT7

	//VHT support for 160 MHz
	//val2 |= BIT6

#if defined(CONFIG_BEAMFORMING) && defined(TX_BEAMFORMING)
	//SU Beamformer Capable
	if (TEST_FLAG(VHT_BF_CAP(padapter), BEAMFORMING_VHT_BEAMFORMER_ENABLE))
		val2 |= BIT5;

	//MU Beamformer Capable
	if (TEST_FLAG(VHT_BF_CAP(padapter), BEAMFORMING_VHT_MU_MIMO_AP_ENABLE))
		val2 |= BIT4;
#endif

	*(result_buf+offset) = TLV_TYPE_AP_VHT_CAPABILITIES;
	offset++;

	*(result_buf+offset) = 12;
	offset++;

	_rtw_memcpy(result_buf+offset, adapter_mac_addr(padapter), MAC_ADDR_LEN);
	offset += MAC_ADDR_LEN;

	if (padapter->phl_role != NULL) {
		//vht tx MCS
		_rtw_memcpy(result_buf+offset, &padapter->phl_role->proto_role_cap.vht_tx_mcs , 2);
		offset += 2;

		//vht rx MCS 
		_rtw_memcpy(result_buf+offset, &padapter->phl_role->proto_role_cap.vht_rx_mcs , 2);
		offset += 2;
	} else {
		//vht tx MCS 
		*((u16 *)(result_buf+offset)) = 0;
		offset += 2;

		//vht rx MCS 
		*((u16 *)(result_buf+offset)) = 0;
		offset += 2;
	}

	_rtw_memcpy(result_buf+offset, &val, 1);
	offset++;

	_rtw_memcpy(result_buf+offset, &val2, 1);
	offset++;

	*len += offset;
}

void _core_map_get_he_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	u8 val = 0, val2 = 0;
	u8 rf_type;
	u32 offset = 0;
	struct dvobj_priv *dvobj = NULL;
	struct wifi_mib_priv *pmibpriv = &padapter->registrypriv.wifi_mib;

	//not support he
	if (!(pmibpriv->band & WLAN_MD_11AX)) {
		*(result_buf+offset) = TLV_TYPE_AP_HE_CAPABILITIES;
		offset++;

		*(result_buf+offset) = 0;
		offset++;

		*len += offset;
		RTW_INFO("%s %u Not support HE wireless mode:%u\n\n", __FUNCTION__, __LINE__, pmibpriv->band);
		return;
	}

	dvobj = adapter_to_dvobj(padapter);
	rf_type = GET_HAL_RFPATH(dvobj);

	//TX spatial stream support (bits 7-5)
	if (rf_type == RF_2T2R || rf_type == RF_2T3R || rf_type == RF_2T4R || rf_type == RF_4T4R)
		val |= BIT5;
	if (rf_type == RF_3T3R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT6;

	//Now the maximum number of supported Tx spatial streams is 4, so bit7 of val is 0.

	//RX spatial stream support (bits 4-2)
	if (rf_type == RF_1T2R || rf_type == RF_2T2R || rf_type == RF_2T4R || rf_type == RF_3T4R || rf_type == RF_4T4R)
		val |= BIT2;
	if (rf_type == RF_3T3R || rf_type == RF_2T3R || rf_type == RF_4T4R)
		val |= BIT3;

	//Now the maximum number of supported Rx spatial streams is 4, so bit4 of val is 0.

	//HE support for 80+80 MHz
	//val |= BIT1;

	//HE support for 160 MHz
	//val |= BIT0;

	//SU beamformer capable
	val2 |= BIT7;

	//MU beamformer capable
	val2 |= BIT6;

	//UL MU-MIMO capable
	//val2 |= BIT5;

	//UL MU-MIMO + OFDMA capable
	//val2 |= BIT4;

	//DL MU-MIMO + OFDMA capable
	//val2 |= BIT3;

	//UL OFDMA capable
	if (padapter->registrypriv.wifi_mib.ofdma_enable & ul_fix_mode)
		val2 |= BIT2;

	//DL OFDMA capable
	if (padapter->registrypriv.wifi_mib.ofdma_enable & dl_fix_mode)
		val2 |= BIT3;

	*(result_buf+offset) = TLV_TYPE_AP_HE_CAPABILITIES;
	offset++;

	*(result_buf+offset) = 13; //Variable
	offset++;

	_rtw_memcpy(result_buf+offset, adapter_mac_addr(padapter), MAC_ADDR_LEN);
	offset += MAC_ADDR_LEN;

	//Length of supported HE MCS field
	*(result_buf+offset) = 4;
	offset++;

	if (padapter->phl_role != NULL) {
		_rtw_memcpy(result_buf+offset, padapter->phl_role->proto_role_cap.he_tx_mcs,
			HE_CAP_ELE_SUPP_MCS_LEN_TX_80M);
		offset += 2;

		_rtw_memcpy(result_buf+offset, padapter->phl_role->proto_role_cap.he_rx_mcs,
			HE_CAP_ELE_SUPP_MCS_LEN_RX_80M);
		offset += 2;
	} else {
		*((u16 *)(result_buf+offset)) = 0;
		offset += 2;

		*((u16 *)(result_buf+offset)) = 0;
		offset += 2;
	}

	_rtw_memcpy(result_buf+offset, &val, 1);
	offset++;

	_rtw_memcpy(result_buf+offset, &val2, 1);
	offset++;

	*len += offset;
}

#ifdef CONFIG_RTW_MULTI_AP_R3
#define IS_AP 0
#define IS_STA 1
#define NUMBER_OF_TRAFFIC_IDENTIFIERS 8
int map_get_ap_wifi6_cap_fill_role(_adapter *padapter, u8 *tlv_content, int *offset, u16 maximum_limit, u8 roleFlag) // 0 for Ap, 1 for Sta
{
	u8 agent_role_octet    = 0;
	u8 su_beamformer_octet = 0;
	u8 rts_octet           = 0;

	/*BIT 7-6 set to 0 for Ap role, 1 for Sta Role*/
	if (IS_STA == roleFlag) {
		agent_role_octet |= BIT6;
	}
	if (padapter->phl_role->proto_role_cap.sgi_160) {
		/*Support 160 MHz*/
		agent_role_octet |= BIT5;
		/*Support 80+80 MHZ, in _adapter definition, sgi_160 represents both these 2 features*/
		agent_role_octet |= BIT4;

		/*MCS NSS length will be 12 in this case*/
		agent_role_octet |= BIT3;
	}
#ifdef CONFIG_RTW_MULTI_AP_LOGO // This is only for passing 4.6.1 160 and 80+80 mcsnss accessing issue in logo test.
	if (!(padapter->phl_role->proto_role_cap.sgi_160)) {
		/*Support 160 MHz*/
		agent_role_octet |= BIT5;
		/*Support 80+80 MHZ, in _adapter definition, sgi_160 represents both these 2 features*/
		agent_role_octet |= BIT4;

		/*MCS NSS length will be 12 in this case*/
		agent_role_octet |= BIT3;
	}
#endif
	/*Length of MCS NSS, value 4 bit is always set, the spec 17.2.72 assumes support 80 MHz, so no check is made for sgi_80,
	  MCS NSS Length also assumes support for 80 MHz, as it can only be 4, 8, or 12, 0 is not an option*/
	agent_role_octet |= BIT2;
	/*Fill in from Agent-Role to MCS-NSS-Length*/
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memcpy((tlv_content + *offset), &agent_role_octet, 1);
	(*offset)++;

	/*This is for MCS NSS*/
	if ((*offset) + 2 > maximum_limit)
		return -ENOMEM;
	_rtw_memcpy(tlv_content + *offset, padapter->phl_role->proto_role_cap.he_rx_mcs, 2);
	*offset += 2;

	if ((*offset) + 2 > maximum_limit)
		return -ENOMEM;
	_rtw_memcpy(tlv_content + *offset, padapter->phl_role->proto_role_cap.he_tx_mcs, 2);
	*offset += 2;

	/*160 MHz*/
	if (agent_role_octet & BIT5) {
		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs) + 2, 2);
		*offset += 2;

		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs) + 2, 2);
		*offset += 2;
	}

	/*80+80 MHz*/
	if (agent_role_octet & BIT4) {
		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs) + 4, 2);
		*offset += 2;

		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs) + 4, 2);
		*offset += 2;
	}

#ifdef CONFIG_RTW_MULTI_AP_LOGO // This is only for passing 4.6.1 160 and 80+80 mcsnss accessing issue in logo test.
	/*160 MHz*/
	if (!(agent_role_octet & BIT5)) {
		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs), 2);
		*offset += 2;

		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs), 2);
		*offset += 2;
	}

	/*80+80 MHz*/
	if (!(agent_role_octet & BIT4)) {
		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs), 2);
		*offset += 2;

		if ((*offset) + 2 > maximum_limit)
			return -ENOMEM;
		_rtw_memcpy(tlv_content + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs), 2);
		*offset += 2;
	}
#endif

	if (padapter->phl_role->proto_role_cap.he_su_bfmr) {
		/*support for SU Beamformer*/
		su_beamformer_octet |= BIT7;
	}
	if (padapter->phl_role->proto_role_cap.he_su_bfme) {
		/*support for SU Beamformee*/
		su_beamformer_octet |= BIT6;
	}
	/*support for MU beamformer Status is not found in proto_tole_cap*/
	/*support for Beamformee STS <= 80 MHz not found*/
	if (padapter->phl_role->proto_role_cap.bfme_sts_greater_80mhz) {
		/*support Beamformee STS >= 80 MHz*/
		su_beamformer_octet |= BIT3;
	}
	/*support for UL MU-MIMO not found*/
	if (padapter->registrypriv.wifi_mib.ofdma_enable & BIT(1)) {
		/*support UL OFDMA*/
		su_beamformer_octet |= BIT1;
	}
	if (padapter->registrypriv.wifi_mib.ofdma_enable & BIT(0)) {
		/*support DL OFDMA*/
		su_beamformer_octet |= BIT0;
	}
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memcpy((tlv_content + *offset), &su_beamformer_octet, 1);
	(*offset)++;

	/*octet 15, max number of users supported per DL/UL MU-MIMO TX in an AP role not found*/
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memset((tlv_content + *offset), 0, 1);
	(*offset)++;
	/*octet 16, max number of users supported per DL OFDMA TX in an AP role not found*/
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memset((tlv_content + *offset), 0, 1);
	(*offset)++;
	/*octet 17, max number of users supported per UL OFDMA RX in an AP role not found*/
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memset((tlv_content + *offset), 0, 1);
	(*offset)++;

	/*RTS not found in g6, but found in rtl8192fe, so support*/
	rts_octet |= BIT7;
	/*MU RTS is found in g6, phl/hal_g6, so support*/
	rts_octet |= BIT6;
	/*Multi_bssid found in g6/phl, so support*/
	rts_octet |= BIT5;
	/*MU EDCA found in g6/core, so support*/
	rts_octet |= BIT4;
#ifdef CONFIG_RTW_TWT
	rts_octet |= BIT3;
	rts_octet |= BIT2;
#endif
	if ((*offset) + 1 > maximum_limit)
		return -ENOMEM;
	_rtw_memcpy((tlv_content + *offset), &rts_octet, 1);
	(*offset)++;

	return 0;
}

int map_get_ap_wifi6_capabilities(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	_adapter          *iface;
	u8                 isSta = 0;
	u8                 isAp  = 0;
	u8                 i;
	u8                *tlv_length_ptr;
	struct dvobj_priv *dvobj;

	isAp  = MLME_IS_AP(padapter) ? 1 : 0;
	isSta = MLME_IS_STA(padapter) ? 1 : 0;

	dvobj = adapter_to_dvobj(padapter);
	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		isAp  = MLME_IS_AP(iface) ? (isAp | 1) : (isAp | 0);
		isSta = MLME_IS_STA(iface) ? (isSta | 1) : (isSta | 0);
	}

	/*Octet index 0 in result_buff is the tlv type, which is TLV_TYPE_AP_WIFI_6_CAPABILITIES*/
	/* CHECKING IS PREEMPTIVE, meaning check everytime before writing */
	if ((*processed_bytes) + 1 > maximum_limit)
		goto abort;
	*(tlv_content + *processed_bytes) = TLV_TYPE_AP_WIFI_6_CAPABILITIES;
	(*processed_bytes)++;
	/*Octet index 1 and 2 in result_buff is the tlvLength */
	/*tlv_length will be determined later*/
	if ((*processed_bytes) + 1 > maximum_limit)
		goto abort;
	*(tlv_content + *processed_bytes) = 0;
	(*processed_bytes)++;

	if ((*processed_bytes) + 1 > maximum_limit)
		goto abort;
	*(tlv_content + *processed_bytes) = 0;
	(*processed_bytes)++;

	/*Octet index 3 to 8 in result_buff is the RUID which is the MAC*/
	if ((*processed_bytes) + MACADDRLEN > maximum_limit)
		goto abort;
	_rtw_memcpy(tlv_content + *processed_bytes, adapter_mac_addr(padapter), MACADDRLEN);
	*processed_bytes += MACADDRLEN;
	/*Octet index 9 in result_buff is number of roles*/
	if ((*processed_bytes) + 1 > maximum_limit)
		goto abort;
	*(tlv_content + *processed_bytes) = (isAp + isSta);
	(*processed_bytes)++;

	if (isAp) {
		if (map_get_ap_wifi6_cap_fill_role(padapter, tlv_content, processed_bytes, maximum_limit, IS_AP))
			goto abort;
	}
	if (isSta) {
		if (map_get_ap_wifi6_cap_fill_role(padapter, tlv_content, processed_bytes, maximum_limit, IS_STA))
			goto abort;
	}
	tlv_length_ptr = tlv_content + 1;
	RTW_PUT_BE16(tlv_length_ptr, (*processed_bytes) - 3);
	return 0;

abort:
	printk("[ERROR] abort ap wifi 6 capability tlv get. \n");
	return -ENOMEM;
}

int map_get_assoc_wifi6_sta_status_report(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	struct sta_info *psta                = NULL;
	struct sta_priv *pstapriv            = &padapter->stapriv;
	u8               sta_mac[MACADDRLEN] = { 0 };
	int              i;
	u8              *tlv_length_ptr;

	_rtw_memcpy(sta_mac, adapter_mac_addr(padapter), MACADDRLEN);
	psta = rtw_get_stainfo(pstapriv, sta_mac);
	if (psta) {
		u8 *ptr = tlv_content;
		u8  tid = 0;
		/* First byte is tlvType */
		if (*processed_bytes + 1 > maximum_limit)
			goto abort;
		*ptr = TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT;
		ptr++;
		(*processed_bytes)++;
		/* Second and third byte is tlvLength. */
		if (*processed_bytes + 2 > maximum_limit)
			goto abort;
		_rtw_memset(ptr, 0, 2);
		ptr += 2;
		(*processed_bytes) += 2;

		/* Followed by 6 bytes of Mac addr of associated STA */
		if (*processed_bytes + MACADDRLEN > maximum_limit)
			goto abort;
		_rtw_memcpy(ptr, sta_mac, MACADDRLEN);
		ptr += MACADDRLEN;
		(*processed_bytes) += MACADDRLEN;
		/* n is hard coded to be 8, and it should be */
		if (*processed_bytes + 1 > maximum_limit)
			goto abort;
		*ptr = NUMBER_OF_TRAFFIC_IDENTIFIERS;
		ptr++;
		(*processed_bytes)++;
		/* n repetition of the TID and Queue size for this TID */
		for (i = 0; i < NUMBER_OF_TRAFFIC_IDENTIFIERS; i++) {
			if (*processed_bytes + 1 > maximum_limit)
				goto abort;
			*ptr = tid;
			ptr++;
			(*processed_bytes)++;
			tid++;

			/* TO DO: queue size is hardcoded for now, needs change. */
			if (*processed_bytes + 1 > maximum_limit)
				goto abort;
			*ptr = 100;
			ptr++;
			(*processed_bytes)++;
		}
		/* now set the tlv_length */
		tlv_length_ptr = tlv_content + 1;
		RTW_PUT_BE16(tlv_length_ptr, (*processed_bytes) - 3);
	} else {
		RTW_INFO("%s - STA %02x%02x%02x%02x%02x%02x not found\n", __FUNCTION__,
		         sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		return -EIO;
	}
	return 0;

abort:
	printk("[ERROR] abort associated wifi6 stations status report tlv get. \n");
	return -ENOMEM;
}
#endif // CONFIG_RTW_MULTI_AP_R3

#ifdef CONFIG_RTW_MULTI_AP_R4
int map_get_spatial_reuse_report(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	u8 				   sr_valid_octet = 0;
	u8                *tlv_length_ptr;
	/*Octet index 0 in result_buff is the tlv type, which is TLV_TYPE_SPATIAL_REUSE_REPORT*/
	/* the total process_bytes is 40 */
	if ((*processed_bytes) + 40 > maximum_limit)
		goto abort;

	*(tlv_content + *processed_bytes) = TLV_TYPE_SPATIAL_REUSE_REPORT;
	(*processed_bytes)++;
	/*Octet index 1 and 2 in result_buff is the tlvLength */
	/*tlv_length will be determined later*/
	*(tlv_content + *processed_bytes) = 0;
	(*processed_bytes)++;
	*(tlv_content + *processed_bytes) = 0;
	(*processed_bytes)++;

	/*Octet index 3 to 8 in result_buff is the RUID which is the MAC*/
	_rtw_memcpy(tlv_content + *processed_bytes, adapter_mac_addr(padapter), MACADDRLEN);
	*processed_bytes += MACADDRLEN;

	/*Octet index 9 in result_buff is BSS_COLOR(bits5-0)*/
	/*BIT 6 for BSS_COLOR is Partial BSS Color, harcode to 0*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.bss_color, 1);
	(*processed_bytes)++;

	/*Octet index 10 in result_buff is SR INFO*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.valid_byte, 1);
	(*processed_bytes)++;

	/*Non-SRG OBSSPD Max Offset*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.non_srg_max_offset, 1);
	(*processed_bytes)++;

	/*SRG OBSSPD Min Offset*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.srg_min_offset, 1);
	(*processed_bytes)++;

	/*SRG OBSSPD Max Offset*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.srg_max_offset, 1);
	(*processed_bytes)++;

	/*SRG BSS Color Bitmap*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.bss_color_bitmap, 8);
	(*processed_bytes) += 8;
	/*SRG Partial BSSID Bitmap*/
	_rtw_memcpy((tlv_content + *processed_bytes), &padapter->registrypriv.partial_bssid_bitmap, 8);
	(*processed_bytes) += 8;
	/*Neighbor BSSColorInUse Bitmap*/
	_rtw_memset((tlv_content + *processed_bytes), 0, 8);
	(*processed_bytes) += 8;
	/*Reserved bytes*/
	_rtw_memset((tlv_content + *processed_bytes), 0, 2);
	(*processed_bytes) += 2;

	tlv_length_ptr = tlv_content + 1;
	RTW_PUT_BE16(tlv_length_ptr, (*processed_bytes) - 3);
	return 0;

abort:
	printk("[ERROR] abort spatial reuse report tlv get. \n");
	return -ENOMEM;
}

int map_set_spatial_reuse_request(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	/* the total process_bytes is 31 */
	if ((*processed_bytes + 31) > maximum_limit)
		goto abort;
	// Skipping TLV type(len=1), TLV length(len=2) and BSSID(len=6)
	*processed_bytes += 9;

	/*BSS_COLOR(bits5-0)*/
	_rtw_memcpy(&padapter->registrypriv.bss_color, (tlv_content + 9), 1);
	_rtw_memcpy(&padapter->phl_role->proto_role_cap.bsscolor, (tlv_content + 9), 1);
	*processed_bytes += 1;

	/*Valid byte*/
	_rtw_memcpy(&padapter->registrypriv.valid_byte, (tlv_content + 10), 1);
	*processed_bytes += 1;

	/*SRG max offset*/
	_rtw_memcpy(&padapter->registrypriv.non_srg_max_offset, (tlv_content + 11), 1);
	*processed_bytes += 1;

	/*SRG max offset*/
	_rtw_memcpy(&padapter->registrypriv.srg_min_offset, (tlv_content + 12), 1);
	*processed_bytes += 1;

	/*SRG max offset*/
	_rtw_memcpy(&padapter->registrypriv.srg_max_offset, (tlv_content + 13), 1);
	*processed_bytes += 1;

	/*SRG BSS color bitmap*/
	_rtw_memcpy(&padapter->registrypriv.bss_color_bitmap, (tlv_content + 14), 8);
	*processed_bytes += 8;

	/*SRG Partial bssid bitmap*/
	_rtw_memcpy(&padapter->registrypriv.partial_bssid_bitmap, (tlv_content + 22), 8);
	*processed_bytes += 8;
	return 0;

abort:
	printk("[ERROR] abort spatial reuse request tlv set. \n");
	return -ENOMEM;
}
#endif // CONFIG_RTW_MULTI_AP_R4

#ifdef CONFIG_RTW_MULTI_AP_R3
int map_set_service_prioritization_rule(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	u16       tlv_length;
	u8        add_remove;
	u8        output;
	u8        always_match;
	_adapter *primary_adapter;

	if ((*processed_bytes + 3) > maximum_limit)
		goto abort;
	tlv_length = RTW_GET_BE16(tlv_content + 1);
	if (tlv_length > maximum_limit)
		goto abort;
	*processed_bytes += 3;

	if ((*processed_bytes + 5) > maximum_limit)
		goto abort;
	add_remove = (*(tlv_content + 7)) & BIT7;
	*processed_bytes += 5;

	if ((*processed_bytes + 2) > maximum_limit)
		goto abort;
	output = *(tlv_content + 9);
	*processed_bytes += 2;

	if ((*processed_bytes + 1) > maximum_limit)
		goto abort;
	always_match = (*(tlv_content + 10)) & BIT7;
	*processed_bytes += 1;
	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);

	if (map_set_service_prioritization_rule_apply(primary_adapter, add_remove, output, always_match)) // set SP info for root interface only.
		return -1;
	return 0;

abort:
	printk("[ERROR] abort service prioritization tlv set. \n");
	return -ENOMEM;
}

int map_set_service_prioritization_rule_apply(_adapter *padapter, u8 add_remove, u8 output, u8 always_match)
{
	if (add_remove) {
		if (always_match) {
			padapter->service_priority_output = output;
			padapter->service_priority_enabled = 1;
			printk("[g6] Service prioritization rule set, output : [%u] \n", output);
		}
	} else {
		if (always_match) {
			padapter->service_priority_enabled = 0;
			// padapter -> service_priority_output = output; //lazy execution is okay.
			printk("[g6] Service prioritization rule removed \n");
		}
	}
	return 0;
}

#define DSCP_TABLE_LENGTH 64
int map_set_dscp_mapping_table(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	u16 tlv_length;
	_adapter *         primary_adapter;

	if ((*processed_bytes + 3) > maximum_limit)
		goto abort;
	tlv_length = RTW_GET_BE16(tlv_content + 1);
	if (tlv_length > maximum_limit)
		goto abort;
	*processed_bytes += 3;

	if ((*processed_bytes + 64) > maximum_limit)
		goto abort;

	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);

	if (!primary_adapter->dscp_pcp_table) {
		primary_adapter->dscp_pcp_table = (u8 *)kmalloc(DSCP_TABLE_LENGTH, GFP_ATOMIC);
	}

	_rtw_memcpy(primary_adapter->dscp_pcp_table, tlv_content + 3, DSCP_TABLE_LENGTH);
	primary_adapter->dscp_pcp_table_enabled = 1;
	*processed_bytes += 64;

#ifdef CONFIG_QOS_MANAGEMENT
	rtw_qos_issue_action_allsta(padapter, (struct qos_txmgmt) {
		.qos_action = qos_action_QoS_Map_Configure,
	});
#endif

	printk("[g6] DSCP_PCP table set \n");

	return 0;

abort:
	printk("[ERROR] abort dscp mapping table tlv set. \n");
	return -ENOMEM;
}

int map_clear_qos_mgmt_disallow_list(_adapter *padapter)
{
	_adapter *primary_adapter;

	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
	primary_adapter->mscs_disallow_list_count = 0;
	if (primary_adapter->mscs_disallow_list) kfree(primary_adapter->mscs_disallow_list);
	primary_adapter->mscs_disallow_list = NULL;
	primary_adapter->scs_disallow_list_count = 0;
	if (primary_adapter->scs_disallow_list) kfree(primary_adapter->scs_disallow_list);
	primary_adapter->scs_disallow_list = NULL;

	return 0;
}

int map_set_qos_mgmt_ctrl(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	int ret = -1;
	u8 *p = tlv_content;
	u16 tlv_length, ele_len;
	u8 *client_mac;
	struct dscp_txmgmt ptxmgmt;

#ifdef CONFIG_QOS_MANAGEMENT
	if ((*processed_bytes + 3) > maximum_limit)
		goto abort;

	tlv_length = RTW_GET_BE16(p + 1);
	if (tlv_length > maximum_limit)
		goto abort;
	*processed_bytes += 3;
	p += 3;

	*processed_bytes += 2;
	p += 2;//qmid

	if (!_rtw_memcmp(padapter->mac_addr, p, ETH_ALEN))
		goto abort;
	*processed_bytes += 6;
	p += 6;//bssid

	client_mac = p;
	*processed_bytes += 6;
	p += 6;//client mac

	ele_len = tlv_length - 6 - 6 - 2;

	switch (*p) {
		case WLAN_EID_SCS_DESCRIPTOR:
		if (process_scs_req_ctrl(padapter, p, ele_len, client_mac) == SCS_REQ_SUCCESS)
			ret = 0;
		break;
		case WLAN_EID_EXTENSION:
		switch (*(p + 2)) {
			case WLAN_EID_EXT_MSCS_DESCRIPTOR:
			if (process_mscs_req_ctrl(padapter, p, ele_len, client_mac) == SCS_REQ_SUCCESS)
				ret = 0;
			break;
			default:
			break;
		}
		break;
		case WLAN_EID_VENDOR_SPECIFIC: //qos management
		ptxmgmt = (struct dscp_txmgmt){
			.oui_subtyp = RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_REQ,
			.qos_mgmt_elem_len = ele_len,
			.qos_mgmt_elem = p,
		};
		_rtw_memcpy(ptxmgmt.raddr, client_mac, 6);
		ret = rtw_dscp_issue_action(padapter, &ptxmgmt);
		break;
		default:
		break;
	}
#endif
abort:
	return ret;
}

int map_set_qos_mgmt_disallow_list(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	_adapter *primary_adapter;
	u8 *p = tlv_content;
	u16 tlv_length;

	if ((*processed_bytes + 3) > maximum_limit)
		goto abort;

	tlv_length = RTW_GET_BE16(p + 1);
	if (tlv_length > maximum_limit)
		goto abort;
	*processed_bytes += 3;
	p += 3;

	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);

	if (*processed_bytes + 1 > maximum_limit)
		goto abort;
	primary_adapter->mscs_disallow_list_count = *p;
	*processed_bytes += 1;
	p += 1;

	if (*processed_bytes + primary_adapter->mscs_disallow_list_count > maximum_limit)
		goto abort;
	if (primary_adapter->mscs_disallow_list == NULL)
		primary_adapter->mscs_disallow_list = (u8 *)kmalloc(primary_adapter->mscs_disallow_list_count * ETH_ALEN, GFP_ATOMIC);
	krealloc(primary_adapter->mscs_disallow_list, primary_adapter->mscs_disallow_list_count * ETH_ALEN, GFP_ATOMIC);
	_rtw_memcpy(primary_adapter->mscs_disallow_list, p, primary_adapter->mscs_disallow_list_count * ETH_ALEN);
	*processed_bytes += primary_adapter->mscs_disallow_list_count * ETH_ALEN;
	p += primary_adapter->mscs_disallow_list_count * ETH_ALEN;

	if (*processed_bytes + 1 > maximum_limit)
		goto abort;
	primary_adapter->scs_disallow_list_count = *p;
	*processed_bytes += 1;
	p += 1;

	if (*processed_bytes + primary_adapter->scs_disallow_list_count > maximum_limit)
		goto abort;
	if (primary_adapter->scs_disallow_list == NULL)
		primary_adapter->scs_disallow_list = (u8 *)kmalloc(primary_adapter->scs_disallow_list_count * ETH_ALEN, GFP_ATOMIC);
	krealloc(primary_adapter->scs_disallow_list, primary_adapter->scs_disallow_list_count * ETH_ALEN, GFP_ATOMIC);
	_rtw_memcpy(primary_adapter->scs_disallow_list, p, primary_adapter->scs_disallow_list_count * ETH_ALEN);
	*processed_bytes += primary_adapter->scs_disallow_list_count * ETH_ALEN;
	p += primary_adapter->scs_disallow_list_count * ETH_ALEN;

	return 0;

abort:
	printk("[ERROR] abort qos mgmt disallow list set. \n");
	return -ENOMEM;
}

void _support_map_wifi6_ap_cap_fill_role(_adapter *padapter, u8 *result_buf, unsigned int *offset, u8 roleFlag) //0 for Ap, 1 for Sta
{
	u8 agent_role_octet    = 0;
	u8 su_beamformer_octet = 0;
	u8 rts_octet           = 0;

	/*BIT 7-6 set to 0 for Ap role, 1 for Sta Role*/
	if (IS_STA == roleFlag) {
		agent_role_octet |= BIT6;
	}
	if (padapter->phl_role->proto_role_cap.sgi_160) {
		/*Support 160 MHz*/
		agent_role_octet |= BIT5;
		/*Support 80+80 MHZ, in _adapter definition, sgi_160 represents both these 2 features*/
		agent_role_octet |= BIT4;

		/*MCS NSS length will be 12 in this case*/
		agent_role_octet |= BIT3;
	}
#ifdef CONFIG_RTW_MULTI_AP_LOGO // This is only for passing 4.6.1 160 and 80+80 mcsnss accessing issue in logo test.
	if (!(padapter->phl_role->proto_role_cap.sgi_160)) {
		/*Support 160 MHz*/
		agent_role_octet |= BIT5;
		/*Support 80+80 MHZ, in _adapter definition, sgi_160 represents both these 2 features*/
		agent_role_octet |= BIT4;

		/*MCS NSS length will be 12 in this case*/
		agent_role_octet |= BIT3;
	}
#endif
	/*Length of MCS NSS, value 4 bit is always set, the spec 17.2.72 assumes support 80 MHz, so no check is made for sgi_80,
	  MCS NSS Length also assumes support for 80 MHz, as it can only be 4, 8, or 12, 0 is not an option*/
	agent_role_octet |= BIT2;
	/*Fill in from Agent-Role to MCS-NSS-Length*/
	_rtw_memcpy((result_buf + *offset), &agent_role_octet, 1);
	(*offset)++;

	/*This is for MCS NSS*/
	_rtw_memset(result_buf + *offset, 0, 12);
	_rtw_memcpy(result_buf + *offset, padapter->phl_role->proto_role_cap.he_rx_mcs, 2);
	*offset += 2;
	_rtw_memcpy(result_buf + *offset, padapter->phl_role->proto_role_cap.he_tx_mcs, 2);
	*offset += 2;

	/*160 MHz*/
	if (agent_role_octet & BIT5) {
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs) + 2, 2);
		*offset += 2;
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs) + 2, 2);
		*offset += 2;
	}

	/*80+80 MHz*/
	if (agent_role_octet & BIT4) {
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs) + 4, 2);
		*offset += 2;
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs) + 4, 2);
		*offset += 2;
	}

#ifdef CONFIG_RTW_MULTI_AP_LOGO // This is only for passing 4.6.1 160 and 80+80 mcsnss accessing issue in logo test.
	/*160 MHz*/
	if (!(agent_role_octet & BIT5)) {
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs), 2);
		*offset += 2;
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs), 2);
		*offset += 2;
	}

	/*80+80 MHz*/
	if (!(agent_role_octet & BIT4)) {
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_rx_mcs), 2);
		*offset += 2;
		_rtw_memcpy(result_buf + *offset, (padapter->phl_role->proto_role_cap.he_tx_mcs), 2);
		*offset += 2;
	}
#endif

	if (padapter->phl_role->proto_role_cap.he_su_bfmr) {
		/*support for SU Beamformer*/
		su_beamformer_octet |= BIT7;
	}
	if (padapter->phl_role->proto_role_cap.he_su_bfme) {
		/*support for SU Beamformee*/
		su_beamformer_octet |= BIT6;
	}
	/*support for MU beamformer Status is not found in proto_tole_cap*/
	/*support for Beamformee STS <= 80 MHz not found*/
	if (padapter->phl_role->proto_role_cap.bfme_sts_greater_80mhz) {
		/*support Beamformee STS >= 80 MHz*/
		su_beamformer_octet |= BIT3;
	}
	/*support for UL MU-MIMO not found*/
	if (padapter->registrypriv.wifi_mib.ofdma_enable & BIT(1)) {
		/*support UL OFDMA*/
		su_beamformer_octet |= BIT1;
	}
	if (padapter->registrypriv.wifi_mib.ofdma_enable & BIT(0)) {
		/*support DL OFDMA*/
		su_beamformer_octet |= BIT0;
	}
	_rtw_memcpy((result_buf + *offset), &su_beamformer_octet, 1);
	(*offset)++;

	/*octet 15, max number of users supported per DL/UL MU-MIMO TX in an AP role not found*/
	_rtw_memset((result_buf + *offset), 0, 1);
	(*offset)++;
	/*octet 16, max number of users supported per DL OFDMA TX in an AP role not found*/
	_rtw_memset((result_buf + *offset), 0, 1);
	(*offset)++;
	/*octet 17, max number of users supported per UL OFDMA RX in an AP role not found*/
	_rtw_memset((result_buf + *offset), 0, 1);
	(*offset)++;

	/*RTS not found in g6, but found in rtl8192fe, so support*/
	rts_octet |= BIT7;
	/*MU RTS is found in g6, phl/hal_g6, so support*/
	rts_octet |= BIT6;
	/*Multi_bssid found in g6/phl, so support*/
	rts_octet |= BIT5;
	/*MU EDCA found in g6/core, so support*/
	rts_octet |= BIT4;
#ifdef CONFIG_RTW_TWT
	rts_octet |= BIT3;
	rts_octet |= BIT2;
#endif
	_rtw_memcpy((result_buf + *offset), &rts_octet, 1);
	(*offset)++;
}

void _core_map_get_wifi6_ap_cap(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	_adapter *         iface;
	u8                 isSta  = 0;
	u8                 isAp   = 0;
	unsigned int *     offset = len;
	u8                 i;
	struct dvobj_priv *dvobj;

	isAp  = MLME_IS_AP(padapter) ? 1 : 0;
	isSta = MLME_IS_STA(padapter) ? 1 : 0;

	dvobj = adapter_to_dvobj(padapter);
	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		isAp  = MLME_IS_AP(iface) ? (isAp | 1) : (isAp | 0);
		isSta = MLME_IS_STA(iface) ? (isSta | 1) : (isSta | 0);
	}

	/*Octet index 0 in result_buff is the tlv type, which is TLV_TYPE_AP_WIFI_6_CAPABILITIES*/
	*(result_buf + *offset) = TLV_TYPE_AP_WIFI_6_CAPABILITIES;
	(*offset)++;
	/*Octet index 1 and 2 in result_buff is the tlvLength */
	/*tlv_length will be determined later*/
	*(result_buf + *offset) = 0;
	(*offset)++;
	*(result_buf + *offset) = 0;
	(*offset)++;

	/*Octet index 3 to 8 in result_buff is the RUID which is the MAC*/
	_rtw_memcpy(result_buf + *offset, adapter_mac_addr(padapter), MACADDRLEN);
	*offset += MACADDRLEN;
	/*Octet index 9 in result_buff is number of roles*/
	*(result_buf + *offset) = (isAp + isSta);
	(*offset)++;

	if (isAp) {
		_support_map_wifi6_ap_cap_fill_role(padapter, result_buf, offset, IS_AP);
	}
	if (isSta) {
		_support_map_wifi6_ap_cap_fill_role(padapter, result_buf, offset, IS_STA);
	}

	/*Now set the tlv_length,*/
	RTW_PUT_BE16(result_buf + 1, (*offset) - 3);

}

#endif //CONFIG_RTW_MULTI_AP_R3

#ifdef CONFIG_RTW_MULTI_AP_R3
/* TO DO: this is hard coded, need to find the related info in _adapter */
void _core_map_get_assoc_wifi6_sta_status_report(_adapter *padapter, u8 *tmpbuf, u8 *result, unsigned int *len)
{
       struct sta_info *psta                = NULL;
       struct sta_priv *pstapriv            = &padapter->stapriv;
       u8               sta_mac[MACADDRLEN] = { 0 };
       int              i;
       int              offset = 0;

       _rtw_memcpy(sta_mac, &tmpbuf[1], MACADDRLEN);
       psta = rtw_get_stainfo(pstapriv, sta_mac);
       if (psta) {
               u8 *ptr = result;
               u8  tid = 0;
               /* First byte is tlvType */
               *ptr = TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT;
               ptr++;
               offset++;
               /* Second and third byte is tlvLength. */
               _rtw_memset(ptr, 0, 2);
               ptr += 2;
               offset += 2;

               /* Followed by 6 bytes of Mac addr of associated STA */
               _rtw_memcpy(ptr, sta_mac, MACADDRLEN);
               ptr += MACADDRLEN;
               offset += MACADDRLEN;
               /* n is hard coded to be 8, and it should be */
               *ptr = NUMBER_OF_TRAFFIC_IDENTIFIERS;
               ptr++;
               offset++;
               /* n repetition of the TID and Queue size for this TID */
               for (i = 0; i < NUMBER_OF_TRAFFIC_IDENTIFIERS; i++) {
                       *ptr = tid;
                       ptr++;
                       offset++;
                       tid++;

                       /* TO DO: queue size is hardcoded for now, needs change. */
                       *ptr = 100;
                       ptr++;
                       offset++;
               }
			   RTW_PUT_BE16(tmpbuf + 1, offset - 3);
       } else {
               RTW_INFO("%s - STA %02x%02x%02x%02x%02x%02x not found\n", __FUNCTION__,
                        sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
               _rtw_memset(tmpbuf, 0, 26);
               offset = 26;
       }

       *len = offset;
}
#endif // CONFIG_RTW_MULTI_AP_R3

void _core_map_get_ap_metric(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	int data_len = 25;
	u16 tlv_len = 22;
	u16 sta_nr = 0;
	u8 ch_util, esp_ie = 0;
	u8 *p = NULL;

	p = result_buf;

	//TLV Type
	*p = TLV_TYPE_AP_METRICS;
	p++;

	//Length
	_rtw_memcpy(p, &tlv_len, sizeof(u16));
	p+= RTW_TLV_LEN_2;

	//The BSSID of the BSS
	_rtw_memcpy(p, padapter->mac_addr, MAC_ADDR_LEN);
	p += RTW_TLV_LEN_6;

	// The Channel utilization
	*p = rtw_get_ch_utilization(padapter);
	p++;

	//The Number of STAs current associated with this BSS
	sta_nr = padapter->stapriv.asoc_list_cnt;
	_rtw_memcpy(p, &sta_nr, sizeof(u16));
	p+= RTW_TLV_LEN_2;

	//Estimated Service Parameter Information Field
	esp_ie |= (BIT7 | BIT6 | BIT5 | BIT4);

	*p = esp_ie;
	p++;

	//ESP AC=BE
	*p = 0xf9;
	*(p+1) = 0xff;
	*(p+2) = 0x00;
	p += RTW_TLV_LEN_3;

	//ESP AC=BK
	*p = 0xf8;
	*(p+1) = 0xff;
	*(p+2) = 0x00;
	p += RTW_TLV_LEN_3;

	//ESP AC=VO
	*p = 0xfb;
	*(p+1) = 0xff;
	*(p+2) = 0x00;
	p += RTW_TLV_LEN_3;

	//ESP AC=VI
	*p = 0xfa;
	*(p+1) = 0xff;
	*(p+2) = 0x00;
	p += RTW_TLV_LEN_3;

	*len = data_len;
}

#ifdef CONFIG_RTW_MULTI_AP_R2
void _core_map_get_extended_ap_metric(_adapter *padapter, u8 *result_buf, unsigned int *len)
{
	int data_len = 33;
	u16 tlv_len = 30;
	u8 *p = NULL;
	u32 temp;

	struct recv_priv  *precvpriv = &padapter->recvpriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	p = result_buf;

	//TLV Type
	*p = TLV_TYPE_AP_EXTENDED_METRICS;
	p++;

	//Length
	_rtw_memcpy(p, &tlv_len, sizeof(u16));
	p+= RTW_TLV_LEN_2;

	//The BSSID of the BSS
	_rtw_memcpy(p, padapter->mac_addr, MACADDRLEN);
	p += RTW_TLV_LEN_6;

	// UnicastBytesSent
	temp = ((pxmitpriv->tx_uc_bytes > 0xffffffff) ? 0xffffffff : (pxmitpriv->tx_uc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	// UnicastBytesReceived
	temp = ((precvpriv->rx_uc_bytes > 0xffffffff) ? 0xffffffff : (precvpriv->rx_uc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	// MulticastBytesSent
	temp = ((pxmitpriv->tx_mc_bytes > 0xffffffff) ? 0xffffffff : (pxmitpriv->tx_mc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	// MulticastBytesReceived
	temp = ((precvpriv->rx_mc_bytes > 0xffffffff) ? 0xffffffff : (precvpriv->rx_mc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	// BroadcastBytesSent
	temp = ((pxmitpriv->tx_bc_bytes > 0xffffffff) ? 0xffffffff : (pxmitpriv->tx_bc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	// BroadcastBytesReceived
	temp = ((precvpriv->rx_bc_bytes > 0xffffffff) ? 0xffffffff : (precvpriv->rx_bc_bytes & 0xffffffff));
	_rtw_memcpy(p, &temp, 4);
	p += RTW_TLV_LEN_4;

	*len = data_len;
}
#endif

u8 _core_map_rssi_to_rcpi(u8 rssi)
{
	//convert per 100 to per 220
	return ( 2 * (10 + rssi ));
}

void _core_map_get_assoc_sta_link_metric(_adapter *padapter, u8 *buf, u8 *result, unsigned int *len)
{
	int data_len = 0;
	u16 tlv_len = 0;
	u8 sta_mac[6] = {0};
	u8 *p;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	int tx_rate=0, rx_rate=0;
	unsigned char tmp_rate[20] = {0};

	_rtw_memcpy(sta_mac, &buf[1], MACADDRLEN);

	psta = rtw_get_stainfo(pstapriv, sta_mac);

	if(psta)
	{
		p = result;
		*p = TLV_TYPE_ASSOCIATED_STA_LINK_METRICS;
		p += RTW_TLV_LEN_3; //Offset for TLV Type and Length

		_rtw_memcpy(p, sta_mac, MACADDRLEN);
		p += RTW_TLV_LEN_6;

		*p = RTW_TLV_LEN_1; //Number of bssid reported for this STA
		p++;

		_rtw_memcpy(p, padapter->mac_addr, MACADDRLEN);
		p += RTW_TLV_LEN_6;

		_rtw_memset(p, 0, RTW_TLV_LEN_4); //time delta
		p += RTW_TLV_LEN_4;

		get_current_rate(psta, 1, &tx_rate, tmp_rate);

		get_current_rate(psta, 0, &rx_rate, tmp_rate);

		_rtw_memcpy(p, &tx_rate, RTW_TLV_LEN_4); //tx rate
		p += RTW_TLV_LEN_4;

		_rtw_memcpy(p, &rx_rate, RTW_TLV_LEN_4); //rx rate
		p += RTW_TLV_LEN_4;

		*p = _core_map_rssi_to_rcpi(rtw_phl_get_sta_rssi(psta->phl_sta)); //rssi converted to rcpi
		p++;

		data_len = 26;
		tlv_len = data_len;

		_rtw_memcpy(&result[1], &tlv_len, sizeof(u16));

		*len = (data_len + RTW_TLV_LEN_3); //For TLV and length
	}
	else{
		RTW_INFO("%s - STA %02x%02x%02x%02x%02x%02x not found\n", __FUNCTION__,
			sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		_rtw_memset(buf, 0, 29);
		*len = 29;
	}
}

void _core_map_get_assoc_sta_traffic_stats(_adapter *padapter, u8 *buf, u8 *result, unsigned int *len)
{
	unsigned short tlv_len = 0;
	unsigned char sta_mac[6] = {0};
	unsigned char *p = result;
	u32 temp = 0;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;

	_rtw_memcpy(sta_mac, &buf[1], MAC_ADDR_LEN);
	//RTW_INFO("%s - %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
	psta = rtw_get_stainfo(pstapriv, sta_mac);
	if (psta)
	{
		*p = TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS;
		p += RTW_TLV_LEN_3; //Offset for TLV Type and Length

		_rtw_memcpy(p, sta_mac, MAC_ADDR_LEN);
		p += RTW_TLV_LEN_6;

		temp = ((psta->sta_stats.tx_bytes > 0xffffffff) ? 0xffffffff : (psta->sta_stats.tx_bytes & 0xffffffff));
		_rtw_memcpy(p, &temp, 4); //tx bytes
		p += RTW_TLV_LEN_4;

		temp = ((psta->sta_stats.rx_bytes > 0xffffffff) ? 0xffffffff : (psta->sta_stats.rx_bytes & 0xffffffff));
		_rtw_memcpy(p, &temp, 4); //rx bytes
		p += RTW_TLV_LEN_4;

		temp = ((psta->sta_stats.tx_pkts > 0xffffffff) ? 0xffffffff : (psta->sta_stats.tx_pkts & 0xffffffff));
		_rtw_memcpy(p, &temp, 4); //tx pkts
		p += RTW_TLV_LEN_4;

		temp = ((psta->sta_stats.rx_data_pkts > 0xffffffff) ? 0xffffffff : (psta->sta_stats.rx_data_pkts & 0xffffffff));
		_rtw_memcpy(p, &temp, 4); //rx pkts
		p += RTW_TLV_LEN_4;

		_rtw_memcpy(p, &psta->sta_stats.tx_fail_cnt, 4); //tx fail
		p += RTW_TLV_LEN_4;

		temp = 0;
		_rtw_memcpy(p, &temp, 4); //rx fail
		p += RTW_TLV_LEN_4;

		_rtw_memcpy(p, &temp, 4); //retransmission
		p += RTW_TLV_LEN_4;

		tlv_len = 34;

		_rtw_memcpy(&result[1], &tlv_len, sizeof(unsigned short));
		*len = (tlv_len + RTW_TLV_LEN_3); //For TLV and length
	}
	else {
		_rtw_memset(p, 0, 37);
		*len = 37;
	}
}

#ifdef CONFIG_RTW_MULTI_AP_R2
void _core_map_get_extended_assoc_sta_link_metric(_adapter *padapter, u8 *buf, u8 *result, unsigned int *len)
{
	u16 data_len = 0;
	u16 tlv_len = 0;
	u8 sta_mac[6] = {0};
	u8 *p;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	int tx_rate=0, rx_rate=0;
	unsigned char tmp_rate[20] = {0};

	_rtw_memcpy(sta_mac, &buf[1], MACADDRLEN);
	//RTW_INFO("%s - %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);

	p = result;
	*p = TLV_TYPE_EXTENDED_ASSOCIATED_STA_LINK_METRIC;

	data_len += RTW_TLV_LEN_3;	//Offset for TLV Type and Length

	_rtw_memcpy(p + data_len, sta_mac, MACADDRLEN);
	data_len += RTW_TLV_LEN_6;

	psta = rtw_get_stainfo(pstapriv, sta_mac);
	if (psta)
	{
		*(p + data_len) = RTW_TLV_LEN_1; //Number of bssid reported for this STA
		data_len++;

		_rtw_memcpy(p + data_len, padapter->mac_addr, MACADDRLEN);
		data_len += RTW_TLV_LEN_6;

		get_current_rate(psta, 1, &tx_rate, tmp_rate);

		get_current_rate(psta, 0, &rx_rate, tmp_rate);

		_rtw_memcpy(p + data_len, &tx_rate, RTW_TLV_LEN_4); //Last data downlink rate
		data_len += RTW_TLV_LEN_4;

		_rtw_memcpy(p + data_len, &rx_rate, RTW_TLV_LEN_4); //Last data uplink rate
		data_len += RTW_TLV_LEN_4;

		_rtw_memset(p + data_len, 0, RTW_TLV_LEN_4); //Todo: utilization receive
		data_len += RTW_TLV_LEN_4;

		_rtw_memset(p + data_len, 0, RTW_TLV_LEN_4); //Todo: utilization transmit
		data_len += RTW_TLV_LEN_4;

		tlv_len = data_len - 3;
		_rtw_memcpy(&result[1], &tlv_len, sizeof(u16));

		*len = (data_len); //For TLV and length
	} else {
		RTW_INFO("%s - STA %02x%02x%02x%02x%02x%02x not found\n", __FUNCTION__,
			sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		*(p + data_len) = RTW_TLV_LEN_1; //Number of bssid reported for this STA
		data_len++;

		tlv_len = data_len - 3;
		_rtw_memcpy(&result[1], &tlv_len, sizeof(u16));

		*len = (data_len); //For TLV and length
	}
}

void _core_map_set_channel(_adapter *padapter, int channel, u8 cac_oper)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	int i;
	u16 ifbmp = 0;
	u8 bw = padapter->mlmeextpriv.cur_bwmode;
	u8 offset = padapter->mlmeextpriv.cur_ch_offset;

	if (channel == padapter->mlmeextpriv.cur_channel) {
		return;
	}

	ifbmp &= (1 << dvobj->iface_nums) - 1;
	for (i = 0; i < dvobj->iface_nums; i++) {
		if (!dvobj->padapters[i]) {
			continue;
		}

		if (CHK_MLME_STATE(dvobj->padapters[i], WIFI_AP_STATE) && dvobj->padapters[i]->netif_up) {
			ifbmp |= BIT(i);
		}
	}

	if (ifbmp) {
		rtw_change_bss_chbw_cmd(padapter, cac_oper, ifbmp, 0, channel, bw, offset);
	}
}

void _core_map_update_cac_status(_adapter *padapter)
{
	u8 send_buf_cac_status_report[CAC_MAX_PAYLOAD] = {0};
	u16 data_len_cac_status_report = 0;
	u8 offset_cac_status_report = 0;
	u8 *p;

	data_len_cac_status_report += 6; //radio_id
	data_len_cac_status_report += 1; //status
	data_len_cac_status_report += 1; //channel
	data_len_cac_status_report += 1; //op_class

	send_buf_cac_status_report[offset_cac_status_report++] = TLV_TYPE_CAC_STATUS_REPORT;

	p = &send_buf_cac_status_report[offset_cac_status_report];
	_rtw_memcpy(p, &data_len_cac_status_report, 2);
	offset_cac_status_report += 2;

	//radio_id
	p = &send_buf_cac_status_report[offset_cac_status_report];
	_rtw_memcpy(p, padapter->mac_addr, MACADDRLEN);
	offset_cac_status_report += 6;

	//status{0: ongoing CAC channel, 1: available channel, 2: non-occupancy channel}
	if (padapter->dvobj->rf_ctl.radar_detected) {
		send_buf_cac_status_report[offset_cac_status_report++] = 0;
	} else {
		send_buf_cac_status_report[offset_cac_status_report++] = 1;
	}

	//channel & op_class
	send_buf_cac_status_report[offset_cac_status_report++] = padapter->mlmeextpriv.cur_channel;
	send_buf_cac_status_report[offset_cac_status_report++] = padapter->multiap_cac_op_class;

	core_map_nl_event_send(send_buf_cac_status_report, data_len_cac_status_report + 3);
}

void _core_map_update_cac_results(_adapter *padapter)
{
	u8 send_buf_cac_completion_report[CAC_MAX_PAYLOAD] = {0};
	u16 data_len_cac_completion_report = 0;
	u8 offset_cac_completion_report = 0;
	u8 *p;

	data_len_cac_completion_report += 1; //radio_nr
	data_len_cac_completion_report += 6; //radio_id
	data_len_cac_completion_report += 1; //op_class
	data_len_cac_completion_report += 1; //channel
	data_len_cac_completion_report += 1; //flags
	data_len_cac_completion_report += 1; //pairs_nr

	send_buf_cac_completion_report[offset_cac_completion_report++] = TLV_TYPE_CAC_COMPLETION_REPORT;

	p = &send_buf_cac_completion_report[offset_cac_completion_report];
	RTW_PUT_BE16(p, data_len_cac_completion_report);
	offset_cac_completion_report += 2;

	send_buf_cac_completion_report[offset_cac_completion_report++] = 1; //radio_nr

	//radio_id
	p = &send_buf_cac_completion_report[offset_cac_completion_report];
	_rtw_memcpy(p, padapter->mac_addr, MACADDRLEN);
	offset_cac_completion_report += 6;

	//op_class & channel
	send_buf_cac_completion_report[offset_cac_completion_report++] = padapter->multiap_cac_op_class;
	send_buf_cac_completion_report[offset_cac_completion_report++] = padapter->mlmeextpriv.cur_channel;

	/* TODO: set flags & pairs_nr */
	send_buf_cac_completion_report[offset_cac_completion_report++] = 0;
	send_buf_cac_completion_report[offset_cac_completion_report++] = 0;

	core_map_nl_event_send(send_buf_cac_completion_report, data_len_cac_completion_report + 3);
}

void _core_map_cac_result_report(_adapter *padapter)
{
	_core_map_update_cac_status(padapter);
	_core_map_update_cac_results(padapter);

	if (padapter->multiap_cac_channel_hold) {
		_core_map_set_channel(padapter, padapter->multiap_cac_channel_hold, 0);
	}
}

void _core_map_init_cac(_adapter *adapter)
{
	adapter->multiap_cac_channel_hold = 0;
	adapter->multiap_cac_op_class = 0;
	rtw_init_timer(&adapter->multiap_cac_timer,
			_core_map_cac_result_report, adapter);
}

void _core_map_deinit_cac(_adapter *adapter)
{
	_cancel_timer_ex(&adapter->multiap_cac_timer);
}

#ifdef CONFIG_RTW_MULTI_AP_R3
void core_map_deinit_service_prioritization(_adapter *padapter)
{
	padapter->dscp_pcp_table_enabled = false;
	if (padapter->dscp_pcp_table) {
		kfree(padapter->dscp_pcp_table);
	}
}
#endif /* CONFIG_RTW_MULTI_AP_R3 */

#endif

int core_map_get_ap_capability(_adapter *padapter, u8 *tmpbuf, u8* result)
{
	int len = 0;

	RTW_INFO("%s\n", __FUNCTION__);

	if(*tmpbuf == 0)
		_core_map_get_ap_cap(padapter, result, &len);
	else if(*tmpbuf == 1)
		_core_map_get_ht_ap_cap(padapter, result, &len);
	else if(*tmpbuf == 2)
		_core_map_get_vht_ap_cap(padapter, result, &len);
	else if(*tmpbuf == 3)
		_core_map_get_he_ap_cap(padapter, result, &len);
#ifdef CONFIG_RTW_MULTI_AP_R3
	else if(*tmpbuf == 4)
		_core_map_get_wifi6_ap_cap(padapter, result, &len);
#endif
	else
		return len;
	return len;
}

int core_map_get_metric(_adapter *padapter, u8 *tmpbuf, u8* result)
{
	int len = 0;

	RTW_INFO("%s\n", __FUNCTION__);
	if(*tmpbuf == 0)
		_core_map_get_ap_metric(padapter, result, &len);
	else if(*tmpbuf == 1)
		_core_map_get_assoc_sta_link_metric(padapter, tmpbuf, result, &len);
	else if(*tmpbuf == 2)
		_core_map_get_assoc_sta_traffic_stats(padapter, tmpbuf, result, &len);
#ifdef CONFIG_RTW_MULTI_AP_R2
	else if(*tmpbuf == 3)
		_core_map_get_extended_ap_metric(padapter, result, &len);
#endif
	else
		return len;
	return len;
}

int core_map_get_client_capability(_adapter *padapter, u8 *tmpbuf, u8* result)
{
	int len = 0;
	u8      macaddr[MAC_ADDR_LEN];
	u16     u8_max_len = 256;
	_list   *phead, *plist;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;

	RTW_INFO("%s\n", __FUNCTION__);
	_rtw_memcpy(macaddr, tmpbuf, MAC_ADDR_LEN);
	psta = rtw_get_stainfo(pstapriv, macaddr);
	if (psta && psta->assoc_req_length > 0) {
		result[0] = 0;
		if (psta->assoc_req_length >= u8_max_len) {
			result[1] = 0;
			_rtw_memcpy(&result[2], &psta->assoc_req_length, sizeof(psta->assoc_req_length));
			_rtw_memcpy(&result[4], psta->assoc_req_body, psta->assoc_req_length);
			len = (psta->assoc_req_length + 4);
		} else {
			result[1] = psta->assoc_req_length;
			_rtw_memcpy(&result[2], psta->assoc_req_body, psta->assoc_req_length);
			len = (psta->assoc_req_length + 2);
		}
	} else {
		result[0] = 1;
		result[1] = 0;
		len = 2;
	}
	return len;
}
#endif

/* rtw_get_multi_ap_ext */
u8 core_map_get_multi_ap_ie(const u8 *ies, int ies_len, u8 *profile, u16 *vlan_id)
{
	u8 *ie;
	u32 ielen;
	u8 val = 0;

	RTW_PRINT("[%s]ies_len = %d\n", __func__, ies_len);
	{
		u32 len = 0;
		u8 buf[100] = {0}, n = 0;
		for (len = 0; len < ies_len; len++) {
			n += _sprintf(buf + n, "%02x ", ies[len]);
			if (19 == len % 20) {
				RTW_PRINT("%s\n", buf);
				n = _sprintf(buf, "%s", "");
			}
		}
		RTW_PRINT("%s\n", buf);
	}

	ie = rtw_get_ie_ex(ies, ies_len, WLAN_EID_VENDOR_SPECIFIC, 
		MULTI_AP_OUI, 4, NULL, &ielen);

	RTW_DBG("\n");

	if (ielen < 9)
		goto exit;

	if (MULTI_AP_SUB_ELEM_TYPE != ie[6])
		goto exit;

	RTW_DBG("\n");

	val = ie[8];

	RTW_DBG("val = 0x%x \n", val);

	if(profile && (val & (MULTI_AP_FRONTHAUL_BSS | MULTI_AP_BACKHAUL_BSS | MULTI_AP_BACKHAUL_STA)))
		*profile = MAP_PROFILE_ONE;

#ifdef CONFIG_RTW_MULTI_AP_R2
	if(profile && (ielen > 9) && (MULTI_AP_PROFILE_SUB_ELEM_TYPE == ie[9]))
	{
		// profile is filled at byte 12 of multi ap ie, if there is a profile sub element
		*profile = ie[11];
		RTW_DBG("Multi-AP profile = 0x%x\n", *profile);
	}

	if(vlan_id && (*profile >= MAP_PROFILE_TWO) && (ielen > 12) && (MULTI_AP_802_1_Q_SUB_ELEM_TYPE == ie[12]))
	{
		// vlan id is filled at byte 15 & 16(little endian) of multi ap ie, if there is a default 8021.Q sub element
		*vlan_id = RTW_GET_LE16(&ie[14]);
		RTW_DBG("Multi-AP vlan id = %d\n", *vlan_id);
	}
#endif

exit:

	return val;
}

/* rtw_set_multi_ap_ie_ext */
u8 *core_map_append_multi_ap_ie(u8 *pbuf, uint *frlen, u8 val, u8 profile, u16 primary_vid)
{
	u8 cont_len = 7;

	u8 *cont_len_pos = NULL;

	RTW_DBG("val = 0x%x \n", val);

	*pbuf++ = WLAN_EID_VENDOR_SPECIFIC;
	cont_len_pos = pbuf;
	*pbuf++ = cont_len;
	_rtw_memcpy(pbuf, MULTI_AP_OUI, 4);
	pbuf += 4;
	*pbuf++ = MULTI_AP_SUB_ELEM_TYPE;
	*pbuf++ = 1; /* len */
	*pbuf++ = val;

	if(profile >= 2) {
		*pbuf++ = MULTI_AP_PROFILE_SUB_ELEM_TYPE;
		*pbuf++ = 1;
		*pbuf++ = profile;
		cont_len += 3;
	}

	RTW_DBG("profile = 0x%x \n", profile);

	if( profile >= 2 && primary_vid && (val & MULTI_AP_BACKHAUL_BSS)) {
		RTW_DBG("primary_vid = 0x%d \n", primary_vid);
		*pbuf++ = MULTI_AP_802_1_Q_SUB_ELEM_TYPE;
		*pbuf++ = 2;
		RTW_PUT_LE16(pbuf, primary_vid);
		pbuf += 2;
		cont_len += 4;
	}

	*cont_len_pos = cont_len;

	if (frlen)
		*frlen = *frlen + (cont_len + 2);

	return pbuf;
}

void core_map_check_sta_ie(_adapter *adapter, struct sta_info *sta, u8 *ies, int ies_len)
{
	u8 bss_type = 0, profile = 0;
	u16 primary_vid = 0;

	if(!adapter || !sta || !ies || (ies_len <= 0))
	{
		RTW_WARN("input param invalid!\n");
		return;
	}

	sta->flags &= ~WLAN_STA_MULTI_AP;
	bss_type = core_map_get_multi_ap_ie(ies, ies_len, &profile, &primary_vid);

	if (bss_type & MULTI_AP_FRONTHAUL_BSS
		|| bss_type & MULTI_AP_BACKHAUL_BSS
		|| bss_type & MULTI_AP_BACKHAUL_STA)
	{
		if (MAP_MODE_BACKHAL_AP & adapter->multi_ap_mode)
			sta->flags |= WLAN_STA_MULTI_AP | WLAN_STA_A4;
		else if (MAP_MODE_FRONT_AP & adapter->multi_ap_mode)
			sta->flags |= WLAN_STA_MULTI_AP;

		sta->multiap_profile = profile;
#if defined(CONFIG_RTW_MULTI_AP_R2)
		if((GET_MAP_PROFILE(adapter) >= MAP_PROFILE_TWO) && (GET_MAP_BSS_TYPE(adapter) == MULTI_AP_BACKHAUL_STA))
			adapter->registrypriv.wifi_mib.multiap_vlan_id = primary_vid;
#endif
	}

	RTW_DBG("sta->flags=0x%x \n", sta->flags);
	RTW_DBG("sta->multiap_profile=0x%x \n", sta->multiap_profile);
}

int core_map_update_assoc_control(_adapter *padapter, u8 *tmpbuf)
{
	u8 control;
	_adapter *padpt = padapter;
	struct assoc_control_block_list block_entry;
	unsigned int timeout = 0;

	_rtw_memset(&block_entry, 0, sizeof(struct assoc_control_block_list));
	_rtw_memcpy(&block_entry.addr, tmpbuf, MAC_ADDR_LEN);
	_rtw_memcpy(&block_entry.timer, tmpbuf+6, 2);
	_rtw_memcpy(&control, tmpbuf+8, 1);

	RTW_INFO("[%s][%d]mac(%2X:%2X:%2X:%2X:%2X:%2X) time:%u control:%u\n",
		__FUNCTION__, __LINE__,
		block_entry.addr[0], block_entry.addr[1], block_entry.addr[2],
		block_entry.addr[3], block_entry.addr[4], block_entry.addr[5],
		block_entry.timer, control);

	timeout = block_entry.timer * 1000;
	_rtw_memcpy(&control, tmpbuf + RTW_TLV_LEN_8, RTW_TLV_LEN_1);

	if (control == BLOCK_MAC) {
		return rtw_blacklist_add(&padpt->black_list, block_entry.addr, timeout);
	}
	else if (control == UNBLOCK_MAC) {
		return rtw_blacklist_del(&padpt->black_list, block_entry.addr);
	}
	else {
		RTW_WARN("[rtw_multiap]Invalid control code: %02x!\n", control);
	}
	return 0;
}

int core_map_update_steering_policy(_adapter *padapter, u8 *tmpbuf)
{
	u8 steering_policy = tmpbuf[0];
	u8 channel_utilization_th = tmpbuf[1];
	u8 rcpi_steering_th = tmpbuf[2];

	padapter->registrypriv.wifi_mib.multiap_steering_policy = steering_policy;
	padapter->registrypriv.wifi_mib.multiap_cu_threshold = channel_utilization_th;
	padapter->registrypriv.wifi_mib.multiap_rcpi_steering_th = rcpi_steering_th;

	RTW_INFO("[%s] steering_policy=%u, channel_utilization_th=%u, rcpi_steering_th=%u\n",
		__FUNCTION__, steering_policy, channel_utilization_th, rcpi_steering_th);

	return 0;
}

int core_map_update_metric_policy(_adapter *padapter, u8 *tmpbuf)
{
	u8 rcpi_threshold = tmpbuf[0];
	u8 rcpi_hysteris_margin = tmpbuf[1];
	u8 ru_threshold = tmpbuf[2];

	padapter->registrypriv.wifi_mib.multiap_report_rcpi_threshold = rcpi_threshold;
	padapter->registrypriv.wifi_mib.multiap_report_rcpi_hysteris_margin = rcpi_hysteris_margin;
	padapter->registrypriv.wifi_mib.multiap_cu_threshold = ru_threshold;

	RTW_INFO("[%s] rcpi_threshold=%u, rcpi_hysteris_margin=%u, ru_threshold=%u\n",
		__FUNCTION__, rcpi_threshold, rcpi_hysteris_margin, ru_threshold);

	return 0;
}

int core_map_update_bss(_adapter *padapter, u8 *buffer, u16 lens, int *processed)
{
	int i = 0;
	u8 total_bss = 0;
	u16 lens_count;

	total_bss = buffer[0];

	lens_count = total_bss * 6 + 1;

	if (lens_count != lens) {
		RTW_ERR("[Debugging] total length not match! \n");
		return 1;
	}

#ifdef CONFIG_RTW_MULTI_AP_DFS_EN
	if (padapter->map_neighbor_bss_tbl) {
		if (padapter->map_neighbor_bss_tbl[0] != total_bss) {
			_rtw_memcpy(padapter->map_neighbor_bss_tbl, buffer, lens);
		} else {
			for (i = 1; i < total_bss*MACADDRLEN; i+=MACADDRLEN) {
				if (!_rtw_memcmp(buffer + i*MACADDRLEN, padapter->map_neighbor_bss_tbl + i*MACADDRLEN, MACADDRLEN)) {
					_rtw_memcpy(padapter->map_neighbor_bss_tbl, buffer, lens);
					break;
				}
			}
		}
	}
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */

	return 0;
}

int core_map_get_asso_sta_metric(_adapter *padapter, u8 *tmpbuf, u8* result)
{
	int len = 0;

	RTW_INFO("%s\n", __FUNCTION__);
	if(*tmpbuf == 0)
		_core_map_get_assoc_sta_link_metric(padapter, tmpbuf, result, &len);
	else if(*tmpbuf == 1)
		_core_map_get_assoc_sta_traffic_stats(padapter, tmpbuf, result, &len);
#ifdef CONFIG_RTW_MULTI_AP_R2
	else if(*tmpbuf == 2)
		_core_map_get_extended_assoc_sta_link_metric(padapter, tmpbuf, result, &len);
#endif
#ifdef CONFIG_RTW_MULTI_AP_R3
	else if(*tmpbuf == 3)
		_core_map_get_assoc_wifi6_sta_status_report(padapter, tmpbuf, result, &len);
#endif
	else
		return len;
	return len;
}

#ifdef DEBUG_MAP_NL
void _core_map_nl_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	unsigned char *message;

	RTW_INFO("\n");

	if (!skb) {
		RTW_ERR("Error, NULL skb \n");
		return;
	}
	
	nlh = (struct nlmsghdr *)skb->data;

	if (!memcmp(NLMSG_DATA(nlh), RTW_MAP_PREFIX, sizeof(RTW_MAP_PREFIX))) {
		rtw_map_user_pid = nlh->nlmsg_pid;
		RTW_WARN("get MAP NLMSG, rtw_map_user_pid = %d \n", rtw_map_user_pid);
	}
}

#ifdef PLATFORM_LINUX
void _core_map_init_nl(_adapter *adapter)
{
	struct netlink_kernel_cfg cfg = {
		.input = _core_map_nl_rcv,
	};

	RTW_DBG("rtw_map_nl_sock = %p ++ \n", rtw_map_nl_sock);

	if (rtw_map_nl_sock) {
		RTW_DBG("rtw_map_nl_sock is ready = %p \n", rtw_map_nl_sock);
		return;
	}
	
	rtw_map_nl_sock = netlink_kernel_create(&init_net, NETLINK_RTW_MAP, &cfg);
	
	if (!rtw_map_nl_sock)
		RTW_DBG("error!! create rtw_map_nl_sock = NULL \n");

	RTW_DBG("rtw_map_nl_sock = %p -- \n", rtw_map_nl_sock);
}

void _core_map_deinit_nl(_adapter *adapter)
{
	RTW_DBG("\n");

	if(rtw_map_nl_sock)
	{
		netlink_kernel_release(rtw_map_nl_sock);
		rtw_map_nl_sock = NULL;
	}

	rtw_map_user_pid = 0;
}
#endif

void _core_map_nl_event_send_fragment(u8 *data, u32 data_len)
{

#ifdef RTW_COMMON_NETLINK
	rtw_netlink_send(data, data_len, NL_COMM_MAP);
#else
#if defined(PLATFORM_ECOS)
	extern void wfo_radio_cmd_sender_core_map_nl_event_send_fragment(struct net_device *ndev, void *data, u32 msg_len);
	struct net_device *ndev = NULL;
	struct dev_map_tbl_s *map = get_radio_map(0);

	if (map == NULL) {
		_dbgdump("In %s, get_radio_map(0) is NULL\n", __func__);
		return;
	}

	ndev = map->ndev;

	if (ndev == NULL) {
		_dbgdump("In %s, net device NULL\n", __func__);
		return;
	}

	wfo_radio_cmd_sender_core_map_nl_event_send_fragment(ndev, data, data_len);
#else
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;
	s32 err = 0; 
	const u8 *fn = " ";

	RTW_INFO("\n");

	if (!rtw_map_nl_sock) {
		RTW_WARN("error!! rtw_map_nl = NULL \n");
		goto msg_fail;
	}

	if(rtw_map_user_pid == 0)
	{
		RTW_WARN("error!!, rtw_map_user_pid is 0\n");
		err = -ENOBUFS;
		fn  = "rtw_map_user_pid = 0";
		goto msg_fail;
	}

	skb = alloc_skb(NLMSG_SPACE(data_len), GFP_ATOMIC);

	if (!skb) {
		err = -ENOBUFS;
		fn  = "alloc_skb";
		goto msg_fail;
	}

	nlh = nlmsg_put(skb, 0, 0, 0, data_len, 0);

	if (!nlh) {
		err = -ENOBUFS;
		fn  = "nlmsg_put";
		goto msg_fail_skb;
	}

	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	memcpy(NLMSG_DATA(nlh), data, data_len);

	err = netlink_unicast(rtw_map_nl_sock, skb, rtw_map_user_pid, MSG_DONTWAIT);

	if (err < 0) {
		fn = "nlmsg_unicast";
		goto msg_fail;
	}

	return;

msg_fail_skb:
	if (skb)
		kfree_skb(skb);

msg_fail:

	RTW_WARN("msg_fail \n");

	if (msg_drop_cnt < 3) { //eric-map ??
		msg_drop_cnt ++;

		RTW_WARN("Drop: pid=%d len=%d, %s: err=%d\n", 
			rtw_map_user_pid, data_len, fn, err);
	}

	return;
#endif
#endif
}
EXPORT_SYMBOL(_core_map_nl_event_send_fragment);

static unsigned char message_id = 0;
#define RTW_MAP_FRAGMENTIZED_PACKET_LEN			(RTW_MAP_MAX_PAYLOAD_SZ - 4)
void core_map_nl_event_send(u8 *data, u32 data_len)
{
	int fragment_sequence_nr = 0;
	int data_sent = 0;
	unsigned char *packet;

	if (data_len > RTW_MAP_MAX_NETLINK_MSG_SIZE) {
		RTW_DBG("Drop message size more than %u\n", RTW_MAP_MAX_NETLINK_MSG_SIZE);
		return;
	}
	if (data_len <= RTW_MAP_MAX_PAYLOAD_SZ) {
		_core_map_nl_event_send_fragment(data, data_len);
		return;
	}
	//fragmentation support
	packet = rtw_zmalloc(RTW_MAP_MAX_PAYLOAD_SZ);
	if (packet == NULL)
		return;
	//add 4 bytes of header to the packet to support the implementation of fragmentation
	packet[0] = MAP_FRAGMENTIZED_DATA;
	packet[1] = message_id;
	while (data_sent < data_len) {
		packet[3] = fragment_sequence_nr;
		fragment_sequence_nr++;
		if (data_len - data_sent <= RTW_MAP_FRAGMENTIZED_PACKET_LEN) {
			packet[2] = 1; //last fragment
			_rtw_memcpy(packet + 4, data + data_sent, data_len-data_sent);
			_core_map_nl_event_send_fragment(packet, data_len - data_sent + 4);
			data_sent += data_len - data_sent;
		} else {
			packet[2] = 0; //not the last fragment
			_rtw_memcpy(packet + 4, data + data_sent, RTW_MAP_FRAGMENTIZED_PACKET_LEN);
			_core_map_nl_event_send_fragment(packet, RTW_MAP_MAX_PAYLOAD_SZ);
			data_sent += RTW_MAP_FRAGMENTIZED_PACKET_LEN;
		}
		_rtw_memset(packet + 2, 0, RTW_MAP_MAX_PAYLOAD_SZ - 2); // reserve packet[0] and packet[1]
	}
	if (packet)
		rtw_mfree(packet, RTW_MAP_MAX_PAYLOAD_SZ);
	message_id++;
	return;
}

#ifdef CONFIG_RTW_80211K
void core_map_report_beacon_metrics(struct sta_info *psta)
{
	int data_len = 0;
	unsigned short tlv_len = 0;
	unsigned char i, bm_report_info_size, beacon_report_nr;
	unsigned char *send_buf;
	unsigned char *pbuf;

	// send_buf need to be limited, incase EM daemon send bcn metric resp fail caused by tlv stream oversize
	send_buf = rtw_zmalloc(RTW_MAP_MAX_BCN_RPT_SZ);
	if(!send_buf){
		RTW_ERR("alloc send_buf fail!\n");
		goto beacon_fail;
	}
	_rtw_memset(send_buf, 0, RTW_MAP_MAX_BCN_RPT_SZ);

	bm_report_info_size = sizeof(struct dot11k_beacon_measurement_report_info);
	beacon_report_nr = 0;

	send_buf[0] = TLV_TYPE_BEACON_METRICS_RESPONSE;

	_rtw_memcpy(&send_buf[3], psta->phl_sta->mac_addr, MAC_ADDR_LEN);

	send_buf[9] = 0; //Reserved

	send_buf[10] = 0; // beacon_report_nr, save later

	data_len += 8;

	pbuf = &send_buf[11];

	_rtw_spinlock_bh(&psta->lock);
	for(i = psta->beacon_report_start_index; i < psta->rm_beacon_rpt_num; i++) {
		if (psta->rm_beacon_rpt_list[i] == NULL)
			continue;
		if (data_len + 5 + bm_report_info_size + psta->rm_beacon_rpt_list[i]->subelements_len > RTW_MAP_MAX_BCN_RPT_SZ)
		{
			break;
		}

		if (psta->rm_beacon_rpt_mode[i] != 0) {
			RTW_INFO("RCPI DEBUG DRIVER:%d, bcn_rpt[%d].mode:%d, skip!!!\n", __LINE__,i, psta->rm_beacon_rpt_mode[i]);
			psta->beacon_report_start_index += 1;
			continue;
		}

		*pbuf = _MEASUREMENT_REPORT_IE_;
		pbuf++;
		data_len++;

		*pbuf = psta->rm_beacon_rpt_len[i];
		pbuf++;
		data_len++;

		*pbuf = psta->rm_becaon_meas_token[i];
		pbuf++;
		data_len++;

		*pbuf = psta->rm_beacon_rpt_mode[i];
		pbuf++;
		data_len++;

		*pbuf = bcn_rep; //BEACON MEASUREMENT TYPE
		pbuf++;
		data_len++;

		if((psta->rm_beacon_rpt_status == MEASUREMENT_INCAPABLE) || psta->rm_beacon_rpt_status == MEASUREMENT_REFUSED)
		{
			_rtw_spinunlock_bh(&psta->lock);
			goto beacon_fail;
		}

		_rtw_memcpy(pbuf, &(psta->rm_beacon_rpt_list[i]->info), bm_report_info_size);
		pbuf += bm_report_info_size;
		data_len += bm_report_info_size;

		if(psta->rm_beacon_rpt_len[i] > 29
			&& data_len + psta->rm_beacon_rpt_list[i]->subelements_len <= RTW_MAP_MAX_BCN_RPT_SZ) {
			_rtw_memcpy(pbuf, psta->rm_beacon_rpt_list[i]->subelements, psta->rm_beacon_rpt_list[i]->subelements_len);
			pbuf += psta->rm_beacon_rpt_list[i]->subelements_len;
			data_len += psta->rm_beacon_rpt_list[i]->subelements_len;
		}

		beacon_report_nr += 1;
		psta->beacon_report_start_index += 1;
	}
	_rtw_spinunlock_bh(&psta->lock);

	send_buf[10] = beacon_report_nr;

	tlv_len = data_len;
	_rtw_memcpy(&send_buf[1], &tlv_len, sizeof(unsigned short));

	data_len += 3;

	core_map_nl_event_send(send_buf, data_len);

	if(psta->rm_beacon_rpt_num >= RTW_MAX_RADIO_RPT_NUM) {
		RTW_WARN("[%s %d] rm_beacon_rpt_num clear!\n", __FUNCTION__, __LINE__);
		psta->rm_beacon_rpt_num = 0;
	}

beacon_fail:
	if(send_buf)
		rtw_mfree(send_buf, RTW_MAP_MAX_BCN_RPT_SZ);
}
#endif /* CONFIG_RTW_80211K */
#endif

#ifdef DEBUG_MAP_UNASSOC
u8 core_map_check_state(_adapter *adapter, u32 state)
{
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return 0;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return 0;

	if (dvobj->map_measure_state & BIT(state))
		return 1;
	else
		return 0;
}

void _core_map_set_state(struct dvobj_priv *dvobj, u32 state)
{
	if(dvobj)
		dvobj->map_measure_state |= BIT(state);
}

void _core_map_clear_state(struct dvobj_priv *dvobj, u32 state)
{
	if (dvobj)
		dvobj->map_measure_state &= (~ BIT(state));
}

struct unassoc_sta_info *_core_map_get_unassoc_sta(struct dvobj_priv *dvobj, u8 *mac)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	u8 found_unassoc_sta = 0;
	u8 i = 0;

	if (!dvobj)
		return NULL;

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);

	for (i = 0; i < MAX_UNASSOC_STA_NUM; i++) {
		unassoc_sta = &(dvobj->map_unassoc_sta[i]);
		if (unassoc_sta->used && !memcmp(mac, unassoc_sta->mac, ETH_ALEN)) {
			found_unassoc_sta = 1;
			break;
		}
	}

	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);

	if (found_unassoc_sta)
		return unassoc_sta;
	else
		return NULL;
}

struct unassoc_sta_info *_core_map_add_unassoc_sta(_adapter *adapter, u8 *mac)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	struct dvobj_priv *dvobj = NULL;
	u32 i = 0;
	u8 max_timeout_idx = 0;

	if(!adapter)
		return NULL;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return NULL;

	unassoc_sta = _core_map_get_unassoc_sta(dvobj, mac);

	if (unassoc_sta)
		return unassoc_sta;

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);

	for (i = 0; i < MAX_UNASSOC_STA_NUM; i++) {
		unassoc_sta = &(dvobj->map_unassoc_sta[i]);
		if(unassoc_sta->used == 0)
			break;
		else {
			if (dvobj->map_unassoc_sta[max_timeout_idx].timeout < unassoc_sta->timeout) {
				max_timeout_idx = i;
			}
			if (i == (MAX_UNASSOC_STA_NUM - 1)) {
				//without empty slot, replace the one with the largest expired time
				unassoc_sta = &(dvobj->map_unassoc_sta[max_timeout_idx]);
			}
		}
	}

	// init a new unassoc_sta
	_rtw_memcpy(unassoc_sta->mac, mac, ETH_ALEN);
	unassoc_sta->used = 1;
	unassoc_sta->rssi = 0;
	unassoc_sta->timeout = 0;
	unassoc_sta->query = 0;

	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);
	return unassoc_sta;
}

void _core_map_del_unassoc_sta(_adapter *adapter, u8 *mac)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

	unassoc_sta = _core_map_get_unassoc_sta(dvobj, mac);

	if (!unassoc_sta)
		return;

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);
	unassoc_sta->used = 0;
	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);
}

void _core_map_del_all_unassoc_sta(struct dvobj_priv *dvobj)
{
	u32 i = 0;

	if (!dvobj)
		return;

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);
	for (i = 0; i < MAX_UNASSOC_STA_NUM; i++) {
		dvobj->map_unassoc_sta[i].used = 0;
	}
	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);
}

void core_map_unassoc_sta_expire(_adapter *adapter)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	struct dvobj_priv *dvobj = NULL;
	u32 i = 0;

	if(!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);
	for (i = 0; i < MAX_UNASSOC_STA_NUM; i++) {
		unassoc_sta = &(dvobj->map_unassoc_sta[i]);
		if(unassoc_sta->used == 1){
			if (unassoc_sta->timeout > MAX_UNASSOC_STA_TIMEOUT) {
				unassoc_sta->used = 0;
			} else
				unassoc_sta->timeout++;
		}
	}
	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);

}

void core_map_update_unassoc_sta_metric(_adapter *adapter, union recv_frame *prframe)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	u8 rssi = pattrib->phy_info.signal_strength;
	u8 *ta = get_addr2_ptr(prframe->u.hdr.rx_data);

	unassoc_sta = _core_map_add_unassoc_sta(adapter, ta);

	if (!unassoc_sta
		|| !rssi)
		return;

	unassoc_sta->rssi = rssi;
	unassoc_sta->timeout = 0;

#if 0
	RTW_DBG("[%pM] update rssi = %d \n",
		unassoc_sta->mac, unassoc_sta->rssi);
#endif
}

void _core_map_send_unassoc_metric_response(struct dvobj_priv *dvobj)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	u32 i = 0, sta_num = 0, data_len = 0;
	u16 tlv_len = 0;
	u8 report_buf[MAX_UNASSOC_STA_NUM * 12 + 5] = {0};
	u8 channel;
	u8 *p;

	if (!dvobj)
		return;

	RTW_INFO("===Send_unassoc_metric_response===\n");
	RTW_INFO("jiffies = %lu ++ \n", jiffies);

	channel = dvobj->map_measure_channel;
	data_len = 2;

	p = &report_buf[5];

	_rtw_spinlock_bh(&dvobj->map_unassoc_sta_used_lock);
	RTW_INFO("Dump queried unassoc STAs:\n");
	for (i = 0; i < MAX_UNASSOC_STA_NUM; i++) {
		unassoc_sta = &(dvobj->map_unassoc_sta[i]);
		if (!unassoc_sta->used || !unassoc_sta->query )
			continue;
		RTW_INFO("[%d][used=%d][query=%d][%pM][timeout=%d] rssi=%d \n", 
			i, unassoc_sta->used, unassoc_sta->query, unassoc_sta->mac, unassoc_sta->timeout, unassoc_sta->rssi);
		if (!unassoc_sta->rssi)
			continue;

		if (p > report_buf + (MAX_UNASSOC_STA_NUM -1) * 12 + 5) {
			RTW_INFO("report buf full, stop adding unassoc_sta info\n");
			break;
		}

		memcpy(p, unassoc_sta->mac, ETH_ALEN);
		p += 6;

		*p = channel;
		p++;

		memset(p, 0 , 4);
		p+= 4;

		*p = _core_map_rssi_to_rcpi(unassoc_sta->rssi);
		p++;

		sta_num++;
		data_len += 12;
		unassoc_sta->query = 0;
	}

	RTW_INFO("Loop fin \n");

	_rtw_spinunlock_bh(&dvobj->map_unassoc_sta_used_lock);

	report_buf[0] = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE;

	tlv_len = data_len;
	memcpy(&report_buf[1], &tlv_len, 2);

	report_buf[3] = dvobj->map_measure_op_class;
	report_buf[4] = sta_num;

	data_len += 3;

	RTW_INFO("\n");

	core_map_nl_event_send(report_buf, data_len);
}

void core_map_report_unassoc_metric(_adapter *adapter)
{
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

	_core_map_send_unassoc_metric_response(dvobj);
	rtw_hw_set_rx_mode(adapter, PHL_RX_MODE_NORMAL);

	if (0 != dvobj->map_measure_channel && rtw_get_oper_ch(adapter) != dvobj->map_measure_channel) {
		rtw_hw_set_ch_bw(adapter,
			rtw_get_oper_ch(adapter),
			rtw_get_oper_bw(adapter),
			rtw_get_oper_choffset(adapter),
			_TRUE,
			PHL_CMD_NO_WAIT);
	}
	dvobj->map_measure_channel = 0;

	_core_map_clear_state(dvobj, MAP_STATE_MEASURE_UNASSOC);
}

s32 core_map_ioctl_get_unassoc_metric(_adapter *adapter, u8 *data)
{
	struct unassoc_sta_info *unassoc_sta = NULL;
	u32 i, sta_num, data_len = 0;
	unsigned char *p;
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return 0;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return 0;

	RTW_INFO("\n");
	sta_num = data[2];

	if (sta_num)
		p = &data[3];

	RTW_INFO("Receive an unassoc_sta_metric_query, channel = %d op_class = %d, sta_num = %d\n",
		dvobj->map_measure_channel, dvobj->map_measure_op_class, sta_num);

	for (i = 0; i < sta_num; i++) {
		unassoc_sta = _core_map_get_unassoc_sta(dvobj, p);
		RTW_INFO("unassoc_sta_metric_query for sta[%d][%pM] \n", i, unassoc_sta->mac);
		if (unassoc_sta)
			unassoc_sta->query = 1;
		p += ETH_ALEN;
	}

	if (core_map_check_state(adapter, MAP_STATE_MEASURE_UNASSOC)) {
		RTW_INFO("Under MAP_STATE_MEASURE_UNASSOC state, reply current records.\n");
		_core_map_send_unassoc_metric_response(dvobj);
		return 1;
	}

	if (data[1]!= 0 && !rtw_is_same_band(data[1], rtw_get_oper_ch(adapter))) {
		RTW_INFO("unassoc_sta_metric_query request CH(%d) and current CH(%d) not in the same band, drop request!\n",
		data[1], rtw_get_oper_ch(adapter));
		return 1;
	}

	_core_map_set_state(dvobj, MAP_STATE_MEASURE_UNASSOC);

	dvobj->map_measure_op_class = data[0];
	dvobj->map_measure_channel = data[1];

	if (0 != dvobj->map_measure_channel && rtw_get_oper_ch(adapter) != dvobj->map_measure_channel) {
		RTW_INFO("channel mismatch, oper = %d & req = %d, switch to req channel\n",
			rtw_get_oper_ch(adapter), dvobj->map_measure_channel);

		rtw_hw_set_ch_bw(adapter,
			dvobj->map_measure_channel,
			CHANNEL_WIDTH_20,
			CHAN_OFFSET_NO_EXT,
			_TRUE,
			PHL_CMD_NO_WAIT);
	}

	rtw_hw_set_rx_mode(adapter, PHL_RX_MODE_SNIFFER);

	RTW_INFO("jiffies = %lu -- \n", jiffies);

	_set_timer(&dvobj->map_unassoc_sta_timer, MULTI_AP_UNASSOC_STA_TIMEOUT);

	return 1;
}

void _core_map_init_unassoc_sta(_adapter *adapter)
{
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

	_core_map_del_all_unassoc_sta(dvobj);
	rtw_init_timer(&dvobj->map_unassoc_sta_timer, core_map_report_unassoc_metric, adapter);
}

void _core_map_deinit_unassoc_sta(struct dvobj_priv *dvobj)
{
	if(dvobj)
		_cancel_timer_ex(&dvobj->map_unassoc_sta_timer);
}

void core_map_deinit_unassoc_sta(_adapter *adapter)
{
	struct dvobj_priv *dvobj = NULL;

	if(!adapter || !is_primary_adapter(adapter))
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

	_core_map_deinit_unassoc_sta(dvobj);

	if (!core_map_check_state(adapter, MAP_STATE_MEASURE_UNASSOC))
		return;

	_core_map_del_all_unassoc_sta(dvobj);

	rtw_hw_set_rx_mode(adapter, PHL_RX_MODE_NORMAL);

	if (0 != dvobj->map_measure_channel && rtw_get_oper_ch(adapter) != dvobj->map_measure_channel) {
		rtw_hw_set_ch_bw(adapter,
						rtw_get_oper_ch(adapter),
						rtw_get_oper_bw(adapter),
						rtw_get_oper_choffset(adapter),
						_TRUE,
						PHL_CMD_NO_WAIT);
	}
	dvobj->map_measure_channel = 0;
	_core_map_clear_state(dvobj, MAP_STATE_MEASURE_UNASSOC);
}
#endif

#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
void _core_map_init_nbor_tbl(_adapter *adapter)
{
	adapter->map_neighbor_bss_tbl = (u8 *) rtw_vmalloc((100*MACADDRLEN + 1));
}

void _core_map_deinit_nbor_tbl(_adapter *adapter)
{
	if (adapter->map_neighbor_bss_tbl)
			rtw_vmfree(adapter->map_neighbor_bss_tbl, (100*MACADDRLEN + 1));
}
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
void core_map_cfg_init(_adapter *adapter)
{
	if(!adapter)
		return;

#if defined(DEBUG_MAP_NL) && !defined(RTW_COMMON_NETLINK)
#ifdef PLATFORM_LINUX
	if (is_primary_adapter(adapter))
		_core_map_init_nl(adapter);
#endif
#endif
#ifdef DEBUG_MAP_UNASSOC
	if (is_primary_adapter(adapter))
		_core_map_init_unassoc_sta(adapter);
#endif
#ifdef CONFIG_RTW_MULTI_AP_R2
	_core_map_init_cac(adapter);
#endif
	_rtw_init_queue(&adapter->black_list);
#if (CONFIG_RTW_MULTI_AP_DFS_EN) && (CONFIG_DFS)
	_core_map_init_nbor_tbl(adapter);
#endif /* CONFIG_RTW_MULTI_AP_DFS_EN */
}

void core_map_cfg_free(_adapter *adapter)
{
	struct dvobj_priv *dvobj = NULL;

	if(!adapter)
		return;

	dvobj = adapter_to_dvobj(adapter);

	if (!dvobj)
		return;

#if defined(DEBUG_MAP_NL) && !defined(RTW_COMMON_NETLINK)
#ifdef PLATFORM_LINUX
	if (is_primary_adapter(adapter))
		_core_map_deinit_nl(adapter);
#endif
#endif
#ifdef DEBUG_MAP_UNASSOC
	if (is_primary_adapter(adapter))
		_core_map_deinit_unassoc_sta(dvobj);
#endif
#ifdef CONFIG_RTW_MULTI_AP_R2
	_core_map_deinit_cac(adapter);
#ifdef CONFIG_RTW_MULTI_AP_R3
	core_map_deinit_service_prioritization(adapter);
#endif
#endif
	rtw_blacklist_flush(&adapter->black_list);
	_rtw_deinit_queue(&adapter->black_list);
}

#ifdef CONFIG_RTW_MULTI_AP_R2
//Send Client Disassociation Stats event
void core_map_send_sta_disassoc_event(struct sta_info *psta, u16 reason)
{	
	u16 data_len = 0;
	u16 tlv_len = 0;
	u8 *p = NULL;
	u8 msg_buf[256] = {0};
	u32 tmp = 0;
	u64 stats = 0;

	if (psta == NULL)
		return;

	msg_buf[0] = 0x13;
	
	p = &msg_buf[3];
	
	_rtw_memcpy(p, psta->phl_sta->mac_addr, ETH_ALEN);
	p += RTW_TLV_LEN_6;
	data_len += RTW_TLV_LEN_6;
	
	_rtw_memcpy(p, &reason, sizeof(u16));
	p += RTW_TLV_LEN_2;	
	data_len += RTW_TLV_LEN_2;		

	//Associated STA Traffic Stats TLV
	*p = TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS;
	p += RTW_TLV_LEN_1;

	tlv_len = RTW_TLV_LEN_6 + RTW_TLV_LEN_4 * 7;
	_rtw_memcpy(p, &tlv_len, sizeof(u16));
	p += RTW_TLV_LEN_2;

	_rtw_memcpy(p, psta->phl_sta->mac_addr, ETH_ALEN);
	p += RTW_TLV_LEN_6;

	_rtw_memcpy(p, &psta->sta_stats.tx_bytes, 4); //tx bytes
	p += RTW_TLV_LEN_4;

	_rtw_memcpy(p, &psta->sta_stats.rx_bytes, 4); //rx bytes
	p += RTW_TLV_LEN_4;

	_rtw_memcpy(p, &psta->sta_stats.tx_pkts, 4); //tx pkts 
	p += RTW_TLV_LEN_4;
	
	stats = sta_rx_pkts(psta);
	_rtw_memcpy(p, &stats, 4); //rx pkts
	p += RTW_TLV_LEN_4;

	_rtw_memcpy(p, &psta->sta_stats.tx_fail_cnt, 4); //tx fail
	p += RTW_TLV_LEN_4;

	_rtw_memcpy(p, &tmp, 4); //rx fail
	p += RTW_TLV_LEN_4;

	_rtw_memcpy(p, &psta->sta_stats.tx_retry_cnt, 4); //retransmission
	p += RTW_TLV_LEN_4;

	data_len += (1 + RTW_TLV_LEN_2 + RTW_TLV_LEN_6 + RTW_TLV_LEN_4 * 7);

	_rtw_memcpy(&msg_buf[1], &data_len, sizeof(u16));

	data_len += 3; //For message type + overall length
	
	core_map_nl_event_send(msg_buf, data_len);	
}

//Send Client Failed Connection event
void core_map_send_sta_failed_conn_event(struct sta_info *psta, u16 status, u16 reason)
{	
	u16 data_len = 0;
	u8 *p = NULL;
	u8 msg_buf[256] = {0};

	if (psta == NULL)
		return;

	msg_buf[0] = 0x12;

	p = &msg_buf[3];	
	
	_rtw_memcpy(p, psta->phl_sta->mac_addr, ETH_ALEN);
	p += RTW_TLV_LEN_6;
	data_len += RTW_TLV_LEN_6;

	_rtw_memcpy(p, &status, sizeof(u16));
	p += RTW_TLV_LEN_2;
	data_len += RTW_TLV_LEN_2;

	_rtw_memcpy(p, &reason, sizeof(u16));
	p += RTW_TLV_LEN_2;
	data_len += RTW_TLV_LEN_2;

	_rtw_memcpy(&msg_buf[1], &data_len, sizeof(u16));	

	data_len += 3; //For message type + overall length

	core_map_nl_event_send(msg_buf, data_len);		
}
void core_map_send_tunneled_message(_adapter *adapter, unsigned char* mac_address, unsigned char message_type, unsigned short payload_length, unsigned char *payload)
{
	u16 total_len = 0;
	u16 max_size = RTW_MAP_MAX_PAYLOAD_SZ;
	MULTI_AP_TUNNEL_MSG_STRU *multi_ap_tunnel = NULL;

	total_len = sizeof(MULTI_AP_TUNNEL_MSG_STRU) + payload_length;
	if( total_len > max_size ){
		RTW_WARN("tunneled message total len is too long[%d].\n", total_len);
		return;
	}

	multi_ap_tunnel = (MULTI_AP_TUNNEL_MSG_STRU *)rtw_zmalloc(max_size);
	if (multi_ap_tunnel == NULL){
		RTW_WARN("multi_ap_tunnel malloc fail.\n");
		return;
	}

	multi_ap_tunnel->tlv_type = TLV_TYPE_TUNNELED;
	multi_ap_tunnel->tlv_len = total_len - 3;
	memcpy(multi_ap_tunnel->macaddr, mac_address, ETH_ALEN);
	multi_ap_tunnel->msg_type = message_type;
	multi_ap_tunnel->frame_body_len = payload_length;
	memcpy(multi_ap_tunnel->frame_body, payload, payload_length);

	core_map_nl_event_send((u8 *)multi_ap_tunnel, total_len);

	rtw_mfree(multi_ap_tunnel, max_size);
	return;
}

u8 core_map_tx_vlan_process(_adapter *padapter, struct sk_buff *pskb, struct sta_info *psta)
{
	u8 insert_primary_vid = 0;

	if(!padapter || !pskb || !psta)
	{
		RTW_INFO("[%s %d] input null pointer\n", __FUNCTION__, __LINE__);
		return MAP_TX_CONTINUE;
	}

	if((GET_MAP_PROFILE(padapter) >= MAP_PROFILE_TWO) && (GET_MAP_VLAN_ID(padapter)))
	{
		//Add vlan tag for outgoing ingress 1905 packets from backhaul sta associated to VLAN configured backhaul AP
		if((GET_MAP_BSS_TYPE(padapter) == MULTI_AP_BACKHAUL_STA)
		&& (check_fwstate(&padapter->mlmepriv, WIFI_STATION_STATE) == _TRUE)
		&& (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_1905)))
			insert_primary_vid = 1;

		//Add vlan tag for EAPOL key packets on backhaul bss/sta if primary vid set
		if(((GET_MAP_BSS_TYPE(padapter) == MULTI_AP_BACKHAUL_BSS) || (GET_MAP_BSS_TYPE(padapter) == MULTI_AP_BACKHAUL_STA))
		&& (psta->multiap_profile >= MAP_PROFILE_TWO)
		&& is_eapol_key_pkt(pskb, 0))
			insert_primary_vid = 1;

		if(insert_primary_vid)
		{
			if(skb_cow_head(pskb, VLAN_TAG_LEN) < 0)
			{
				RTW_ERR("[%s %d] skb_cow_head() failed!\n", __FUNCTION__, __LINE__);
				return MAP_TX_DROP;
			}

			memmove(pskb->data-VLAN_TAG_LEN, pskb->data, ETH_ALEN<<1);
			skb_push(pskb, VLAN_TAG_LEN);
			*((unsigned short*)(pskb->data+(ETH_ALEN<<1))) = __constant_htons(ETH_P_8021Q);
			*((unsigned short*)(pskb->data+(ETH_ALEN<<1)+2)) = __constant_htons(GET_MAP_VLAN_ID(padapter));
		}
	}

	//drop pkt with vlan tag in fronthaul bss tx
	if((GET_MAP_PROFILE(padapter) >= MAP_PROFILE_TWO) && (GET_MAP_BSS_TYPE(padapter) == MULTI_AP_FRONTHAUL_BSS))
	{
		if ((PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021Q))
		|| (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021AD)))
		{
			RTW_INFO("[%s %d] Drop pkt with vlan tag in fronthaul bss tx.\n", __FUNCTION__, __LINE__);
			return MAP_TX_DROP;
		}
	}

	return MAP_TX_CONTINUE;
}

u8 core_map_rx_vlan_process(_adapter *padapter, struct sk_buff **ppskb)
{
	struct sk_buff *pskb = NULL, *pskb2 = NULL;

	if(!padapter || !ppskb || !(*ppskb))
	{
		RTW_INFO("[%s %d] input null pointer\n", __FUNCTION__, __LINE__);
		return MAP_RX_CONTINUE;
	}

	pskb = *ppskb;

	//drop vlan pkts received on fronthaul bss
	if(GET_MAP_VLAN_ENABLE(padapter) &&  (GET_MAP_BSS_TYPE(padapter) == MULTI_AP_FRONTHAUL_BSS))
	{
		if ((PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021Q))
		||	(PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021AD)))
		{
			RTW_INFO("[%s %d] Drop vlan pkt in fronthaul bss rx!\n", __FUNCTION__, __LINE__);
			return MAP_RX_DROP;
		}
	}

	//remove vlan tag in backhaul EAPOL KEY packets
	if((GET_MAP_BSS_TYPE(padapter) == MULTI_AP_BACKHAUL_BSS) || (GET_MAP_BSS_TYPE(padapter) == MULTI_AP_BACKHAUL_STA))
	{
		if ((PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021Q)) && is_eapol_key_pkt(pskb, 1))
		{
			if (skb_cloned(pskb))
			{
				pskb2 = skb_copy(pskb, GFP_ATOMIC);
				if (pskb2 == NULL)
				{
					RTW_ERR("[%s %d] Drop pkt due to skb_copy() failed!\n", __FUNCTION__, __LINE__);
					return MAP_RX_DROP;
				}

				rtw_skb_free(pskb);
				pskb = pskb2;
				*ppskb = pskb;
			}

			memmove(pskb->data + VLAN_TAG_LEN, pskb->data, ETH_ALEN<<1);
			skb_pull(pskb, VLAN_TAG_LEN);
		}
	}

	return MAP_RX_CONTINUE;
}

#ifdef CONFIG_RTW_MULTI_AP_R3
u8 core_map_tx_set_vlan_pcp(_adapter *padapter, struct sk_buff *pskb, struct sta_info *psta)
{
	u8 vlan_offset = 0;
	u8 dscp_value = 0;
	u8 pcp_value = 0;
	u8 *pcp_byte = NULL;
	_adapter *primary_adapter = NULL;
	u8 i = 0;

	if(!padapter || !pskb || !psta)
	{
		RTW_INFO("[%s %d] input null pointer\n", __FUNCTION__, __LINE__);
		return MAP_TX_CONTINUE;
	}

	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);

	if (!primary_adapter) {
		RTW_WARN("[g6] primary_adapter is NULL \n");
	}
	if (primary_adapter->service_priority_enabled != 1 || primary_adapter->dscp_pcp_table_enabled != 1) {
		return MAP_TX_CONTINUE;
	}

	if (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021Q) || (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021AD))) {
		vlan_offset = (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_8021AD))? 8 : 4;
		dscp_value = (*(pskb->data+(ETH_ALEN<<1) + vlan_offset + 2 + 1)) >> 2;
		if (primary_adapter->service_priority_output < 8) {
			pcp_value = primary_adapter->service_priority_output;
		} else if (primary_adapter->service_priority_output == 8) {
			pcp_value = (primary_adapter->dscp_pcp_table)[dscp_value];
		} else {
			printk("[g6] invalid service priority output \n");
		}
		pcp_byte = pskb->data+(ETH_ALEN<<1) + vlan_offset - 4 + 2;
		*pcp_byte |= pcp_value << 5;
		pskb->cb[_SKB_CB_PRIORITY] = pcp_value;
	}

	return MAP_TX_CONTINUE;
}

void core_map_tx_set_qos_priority(_adapter *padapter, struct sk_buff *pskb, s32 *UserPriority)
{
	_adapter * primary_adapter = NULL;
	u8 dscp_value = 0;

	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
	if (PKT_ETH_TYPE(pskb) == __constant_htons(ETH_P_IP)) {
		if (primary_adapter->service_priority_enabled == 1) {
			if (primary_adapter->service_priority_output < 8) {
				*UserPriority = primary_adapter->service_priority_output;
			} else if (primary_adapter->service_priority_output == 8) {
				if (primary_adapter->dscp_pcp_table_enabled == 1) {
					dscp_value = (*(pskb->data+(ETH_ALEN<<1) + 2 + 1)) >> 2;
					*UserPriority = (primary_adapter->dscp_pcp_table)[dscp_value];
				}
			} else {
				printk("[g6] invalid service priority output \n");
			}
		}
	}
}
#endif

void core_map_send_association_status_notification(_adapter *padapter, u8 association_allowance_status)
{
	u8 send_buf[256] = {0};
	u16 data_len = 0;

	send_buf[0] = TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION;
	_rtw_memcpy(&send_buf[3], adapter_mac_addr(padapter), ETH_ALEN);
	send_buf[9] = association_allowance_status;
	data_len = (ETH_ALEN + 1);
	_rtw_memcpy(&send_buf[1], &data_len, sizeof(u16));
	data_len += 3;

	core_map_nl_event_send(send_buf, data_len);
	return;
}

void core_map_trigger_cac(_adapter *padapter, char *tmpbuf)
{
	u8 multiap_cac_channel_nr = tmpbuf[1];
	u8 multiap_cac_channel = tmpbuf[2];
	u8 multiap_cac_completion_oper = tmpbuf[3];
	padapter->multiap_cac_op_class = tmpbuf[4];

	if (1 != multiap_cac_channel_nr) {
		RTW_INFO("CAC Request TLV contains multiple channels.\n");
		return;
	}

	if (CAC_COMPL_ACTION_RETURN == (multiap_cac_completion_oper & CAC_COMPL_ACTION_MASK)){
		padapter->multiap_cac_channel_hold = padapter->mlmeextpriv.cur_channel;
	} else {
		padapter->multiap_cac_channel_hold = 0;
	}

	_core_map_set_channel(padapter, multiap_cac_channel, 0);
	//report CAC result to daemon after 30 seconds
	_set_timer(&padapter->multiap_cac_timer, MULTI_AP_CAC_TIMEOUT);
}

void core_map_terminate_cac(_adapter *padapter)
{
	_cancel_timer_ex(&padapter->multiap_cac_timer);
	if (0 != padapter->multiap_cac_channel_hold) {
		_core_map_set_channel(padapter, padapter->multiap_cac_channel_hold, RTW_CMDF_DIRECTLY);
	}
}
#endif /* defined(CONFIG_RTW_MULTI_AP_R2) */

#if defined(CONFIG_RTW_MULTI_AP_R2) && defined(CONFIG_RTW_OPCLASS_CHANNEL_SCAN)

int core_map_update_channel_scan_result(_adapter * padapter)
{
	u8* send_buf = NULL;
	unsigned short int data_len = 0;
	u8 i, j;
	u32 buf_pt_offset	= 0;
	u8 drop_data = 0;
	u8 channel_nr = 0;
	unsigned short int new_data_len = 0;

	if (!padapter->opclass_sync_result || padapter->multiap_nl_send)
		return _FAIL;

	data_len += 1; //TLV_TYPE_CHANNEL_SCAN_RESULT
	data_len += 2; //data_len
	data_len += 1; //band
	data_len += 1; //channel_nr
	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		new_data_len = data_len;
		data_len += 1; //channel
		data_len += 1; //scan_status
		data_len += 31; //timestamp
		data_len += 1; //channel_utilization
		data_len += 1; //noise
		data_len += 2; //neighbor_nr
		for (j = 0; j < padapter->opclass_scan_result->channels[i].neighbor_nr; j++) {
			data_len += 6; //bssid
			data_len += 1; //ssid_length
			data_len += padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length; //ssid
			data_len += 1; //signal strength
			data_len += 1; //channel_band_width
			data_len += 1; //channel_utilization
			data_len += 2; //station_count
			if (data_len > RTW_MAP_MAX_NETLINK_MSG_SIZE) {
				drop_data = 1;
				break;
			}
		}
		if (drop_data == 1)
			break;
	}
	if (drop_data == 1) {
		data_len = new_data_len;
		RTW_DBG("Channel scan result inaccurate.");
	}

	//write into buffer
	send_buf = (u8*)_rtw_malloc(data_len * sizeof(u8));
	if (send_buf == NULL)
		return _FAIL;

	send_buf[buf_pt_offset++]	= TLV_TYPE_CHANNEL_SCAN_RESULT;
	_rtw_memcpy(&send_buf[buf_pt_offset], &data_len, 2); //data_len
	buf_pt_offset += 2;

	if (NULL != strstr(padapter->pnetdev->name, "wlan")) {
		send_buf[buf_pt_offset++]	= padapter->pnetdev->name[4] - '0';
	} else {
		send_buf[buf_pt_offset++]	= 0xFF; //error
	}
	if (drop_data == 0) {
		send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channel_nr;
		channel_nr			= padapter->opclass_scan_result->channel_nr;
	} else {
		send_buf[buf_pt_offset++]	= i;
		channel_nr			= i;
	}
	for (i = 0; i < padapter->opclass_scan_result->channel_nr; i++) {
		if (i == channel_nr)
			break;
		send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].channel; //channel
		send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].scan_status; //scan_status
		_rtw_memcpy(&send_buf[buf_pt_offset], padapter->opclass_scan_result->channels[i].timestamp, 31);//timestamp
		buf_pt_offset += 31;
		if(padapter->opclass_scan_result->channels[i].channel_utilization < 1) {
			padapter->opclass_scan_result->channels[i].channel_utilization = 1;
		}
		send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].channel_utilization; //channel_utilization
		send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].noise; //noise
		_rtw_memcpy(&send_buf[buf_pt_offset], &padapter->opclass_scan_result->channels[i].neighbor_nr, 2);//neighbor_nr
		buf_pt_offset += 2;
		for (j = 0; j < padapter->opclass_scan_result->channels[i].neighbor_nr; j++) {
			_rtw_memcpy(&send_buf[buf_pt_offset], padapter->opclass_scan_result->channels[i].neighbors[j].bssid, 6); //neighbor_nr
			buf_pt_offset += 6;
			send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length; //ssid_length
			_rtw_memcpy(&send_buf[buf_pt_offset], padapter->opclass_scan_result->channels[i].neighbors[j].ssid, padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length); //ssid
			buf_pt_offset += padapter->opclass_scan_result->channels[i].neighbors[j].ssid_length;
			send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].signal_strength; //signal_strength
			send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].channel_band_width; //channel_band_width
			send_buf[buf_pt_offset++]	= padapter->opclass_scan_result->channels[i].neighbors[j].channel_utilization; //channel_utilization
			_rtw_memcpy(&send_buf[buf_pt_offset], &padapter->opclass_scan_result->channels[i].neighbors[j].station_count, 2); //station_count
			buf_pt_offset += 2;
		}
	}

	core_map_nl_event_send(send_buf, data_len);

	if (send_buf)
		rtw_mfree(send_buf, data_len);

func_quit:
	padapter->multiap_nl_send = 1;
	return _SUCCESS;
}
#endif

void core_map_ch_util_trigger(_adapter * padapter)
{
	//send the bssid of the triggerred ch util
	int data_len = 0;
	u8 send_buf[256] = { 0 };
	u8 ch_utilization;
	u8 ch_threshold;

	ch_utilization = rtw_get_ch_utilization(padapter);
	ch_threshold = padapter->registrypriv.wifi_mib.multiap_cu_threshold;

	//If the latest channel utilization has crossed the ch utilization threshold with respect to last measured
	if(((padapter->multiap_last_cu < ch_threshold) && (ch_utilization >= ch_threshold)) ||
		((padapter->multiap_last_cu >= ch_threshold) && (ch_utilization < ch_threshold)) ||
		(ch_threshold != 0 && ch_utilization > (padapter->multiap_last_cu + (10 * ch_threshold)))){
		send_buf[0] = TLV_TYPE_METRIC_REPORT_POLICY;
		send_buf[1] = 3;	//ch util trigger
		memcpy(&send_buf[2], adapter_mac_addr(padapter), ETH_ALEN);
		data_len += 8;
		core_map_nl_event_send(send_buf, data_len);
		padapter->multiap_last_cu = ch_utilization;
	}
}

void core_map_ap_sta_rssi_trigger(_adapter * padapter, struct sta_info *psta)
{
	int           data_len      = 0;
	unsigned char send_buf[256] = { 0 };
	unsigned char current_rssi;
	unsigned char rssi_threshold, hysteresis_margin;

	current_rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
	if (padapter->registrypriv.wifi_mib.multiap_report_rcpi_threshold < 20) {
		rssi_threshold = 0;
	} else {
		rssi_threshold = (padapter->registrypriv.wifi_mib.multiap_report_rcpi_threshold / 2 - 10);
	}

	hysteresis_margin = padapter->registrypriv.wifi_mib.multiap_report_rcpi_hysteris_margin;

	//If hysteresis margin is not set (0), use value no higher than 5 as recommended by spec
	if (hysteresis_margin == 0) {
		hysteresis_margin = 2;
	}

	RTW_DBG("%s %u hm:%u rh:%u cr:%u lr:%u\n", __FUNCTION__, __LINE__,
		hysteresis_margin, rssi_threshold,
		current_rssi, psta->last_rssi);

	if (rssi_threshold) {
		if (!psta->last_rssi)
			psta->last_rssi = rtw_phl_get_sta_rssi(psta->phl_sta);

		//If the latest station rssi has crossed the rssi threshold with respect to last measured including hysteresis margin
		if (((psta->last_rssi <= rssi_threshold) && (current_rssi > rssi_threshold) && (current_rssi - psta->last_rssi >= hysteresis_margin)) ||
			((psta->last_rssi >= rssi_threshold) && (current_rssi < rssi_threshold) && (psta->last_rssi - current_rssi >= hysteresis_margin))) {

			send_buf[0] = TLV_TYPE_METRIC_REPORT_POLICY; //unassoc rssi trigger
			send_buf[1] = 1;

			_rtw_memcpy(&send_buf[2], psta->phl_sta->mac_addr, MAC_ADDR_LEN);

			data_len = 8;

			core_map_nl_event_send(send_buf, data_len);

			psta->last_rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
		}
	}
}

void core_map_send_btm_response_notify(_adapter *padapter, u8 bssid[ETH_ALEN], u8 mac[ETH_ALEN], u8 target_bssid[ETH_ALEN], u8 status)
{
	int data_len = 0;
	u8 send_buf[256] = { 0 };

	send_buf[0] = MAP_GENERAL_NETLINK_MESSAGE;
	data_len += 1;

	// The MAC address of the client.
	_rtw_memcpy(send_buf + data_len, mac, ETH_ALEN);
	data_len += ETH_ALEN;

	// The BSSID of the BSS
	_rtw_memcpy(send_buf + data_len, bssid, ETH_ALEN);
	data_len += ETH_ALEN;

	send_buf[data_len] = MASK_BTM_RESPONSE_EVENT;
	data_len += sizeof(unsigned char);

	_rtw_memcpy(send_buf + data_len, target_bssid, ETH_ALEN);
	data_len += ETH_ALEN;

	send_buf[data_len] = status;
	data_len += sizeof(unsigned char);

	core_map_nl_event_send(send_buf, data_len);
}

void core_map_send_client_notify(unsigned char event, unsigned char mac[ETH_ALEN], unsigned char bssid[ETH_ALEN])
{
	int           data_len                                                  = 0;
	unsigned char send_buf[1 + MACADDRLEN + MACADDRLEN + sizeof(unsigned char)] = { 0 };

	send_buf[0] = MAP_GENERAL_NETLINK_MESSAGE;
	data_len += 1;

	//  The MAC address of the client.
	_rtw_memcpy(send_buf + data_len, mac, MACADDRLEN);
	data_len += MACADDRLEN;

	//  The BSSID of the BSS
	_rtw_memcpy(send_buf + data_len, bssid, MACADDRLEN);
	data_len += MACADDRLEN;

	send_buf[data_len] = event;
	data_len += sizeof(unsigned char);

	core_map_nl_event_send(send_buf, data_len);
}

static inline void core_map_send_client_notify_frame(unsigned char event, unsigned char mac[ETH_ALEN], unsigned char bssid[ETH_ALEN], u8* frame_body, u32 frame_body_len)
{
	int data_len = 0;
	// 1 + MACADDRLEN + MACADDRLEN + sizeof(unsigned char) + frame_body_len
	unsigned char send_buf[256] = { 0 };

	send_buf[0] = MAP_GENERAL_NETLINK_MESSAGE;
	data_len += 1;

	//  The MAC address of the client.
	_rtw_memcpy(send_buf + data_len, mac, MACADDRLEN);
	data_len += MACADDRLEN;

	//  The BSSID of the BSS
	_rtw_memcpy(send_buf + data_len, bssid, MACADDRLEN);
	data_len += MACADDRLEN;

	send_buf[data_len] = event;
	data_len += sizeof(unsigned char);
	
	_rtw_memcpy(send_buf + data_len, frame_body, frame_body_len);
	data_len += frame_body_len;

	core_map_nl_event_send(send_buf, data_len);
}

#ifdef CONFIG_RTW_MULTI_AP_R4
void core_map_send_ras_mscs_req_notify(_adapter *padapter, u8 bssid[ETH_ALEN], u8 mac[ETH_ALEN], u8* frame_body, u32 frame_body_len) {
	core_map_send_client_notify_frame(MASK_MSCS_REQUEST_EVENT, mac, bssid, frame_body, frame_body_len);
}

void core_map_send_ras_scs_req_notify(_adapter *padapter, u8 bssid[ETH_ALEN], u8 mac[ETH_ALEN], u8* frame_body, u32 frame_body_len) {
	core_map_send_client_notify_frame(MASK_SCS_REQUEST_EVENT, mac, bssid, frame_body, frame_body_len);
}
#endif

void core_map_send_client_join_notify(unsigned char bssid[ETH_ALEN], unsigned char mac[ETH_ALEN])
{
	RTW_INFO("[Multi-AP] client join event: MAC %02x:%02x:%02x:%02x:%02x:%02x BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
	core_map_send_client_notify(MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN, mac, bssid);
}

void core_map_send_client_leave_notify(unsigned char bssid[ETH_ALEN], unsigned char mac[ETH_ALEN])
{
	RTW_INFO("[Multi-AP] client leave event: MAC %02x:%02x:%02x:%02x:%02x:%02x BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
	core_map_send_client_notify(MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE, mac, bssid);
}

void core_map_send_channel_change_notify()
{
	int data_len    = 0;
	u8  send_buf[1] = { 0 };

	send_buf[0] = MAP_CHANNEL_CHANGE_NOTIFICATION_MESSAGE;
	data_len += 1;

	core_map_nl_event_send(send_buf, data_len);
}

/*
 *	result will be written to tlv_content. It currently hold TLV_TYPE as the first octets only.
 *	the return format is:
 *		Field:         			Length:				Value:
 *		type            		1 octet				TLV_TYPE.
 *		parameter length		2 octets			k
 *		payload         		k octets			retrieved info.
 */
int core_map_get_general(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	int ret = -1;
	switch (*tlv_content) {
#ifdef CONFIG_RTW_MULTI_AP_R3
	case TLV_TYPE_AP_WIFI_6_CAPABILITIES:
		ret = map_get_ap_wifi6_capabilities(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
	case TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT:
		ret = map_get_assoc_wifi6_sta_status_report(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
#endif
#ifdef CONFIG_RTW_MULTI_AP_R4
		case TLV_TYPE_SPATIAL_REUSE_REPORT:
			ret = map_get_spatial_reuse_report(padapter, tlv_content, maximum_limit, processed_bytes);
			break;
#endif /* CONFIG_RTW_MULTI_AP_R4 */
	default:
		printk("[Error] g6 driver does not implement map get general id: %u \n", *tlv_content);
		break;
	}
	return ret;
}

/*
 *	tlv_content contains tlv infomation to set the device.
 *	current format is:
 *		Field:         				Length:				Value:
 *		type            			1 octet				TLV_TYPE
 *		parameter length			2 octets  			k
 *		payload         			k octets			setting info.
 */
int core_map_set_general(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	int ret = -1;
	switch (*tlv_content) {
#ifdef CONFIG_RTW_MULTI_AP_R3
	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE:
		ret = map_set_service_prioritization_rule(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
	case TLV_TYPE_DSCP_MAPPING_TABLE:
		ret = map_set_dscp_mapping_table(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
#endif
#ifdef CONFIG_RTW_MULTI_AP_R4
	case TLV_TYPE_SPATIAL_REUSE_REQUEST:
		ret = map_set_spatial_reuse_request(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
	case TLV_TYPE_QOS_MANAGEMENT_POLICY:
		ret = map_set_qos_mgmt_disallow_list(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
	case TLV_TYPE_QOS_MANAGEMENT_DESCRIPTOR:
		ret = map_set_qos_mgmt_ctrl(padapter, tlv_content, maximum_limit, processed_bytes);
		break;
#endif
	default:
		printk("[Error] g6 driver does not implement map set general id: %u \n", *tlv_content);
		break;
	}
	return ret;
}

int core_map_get_link_metric(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	int ret = -1;
	u8 sta_mac[MAC_ADDR_LEN];
	u16 offset       = 0;
	u16 payload_len  = 0;
	u16 u16temp      = 0;
	u32 u32temp      = 0;
	u32 tx_fail      = 0;
	u32 tx_fail_mgmt = 0;

	struct sta_info *psta       = NULL;
	struct sta_priv *pstapriv   = &padapter->stapriv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	struct dvobj_priv *dvobj    = adapter_to_dvobj(padapter);

	_rtw_memcpy(sta_mac, &tlv_content[2], MAC_ADDR_LEN);
	psta = rtw_get_stainfo(pstapriv, sta_mac);

	_rtw_memset(tlv_content, 0, maximum_limit);
	offset += 2; //reserve for payload len

	if (rtw_is_adapter_up(padapter)) {
		if (psta) {
			_rtw_memcpy((tlv_content + offset), adapter_mac_addr(padapter), ETH_ALEN);
			offset += RTW_TLV_LEN_6;

			_rtw_memcpy((tlv_content + offset), sta_mac, ETH_ALEN);
			offset += RTW_TLV_LEN_6;

			u32temp = 0;
			_rtw_memcpy((tlv_content + offset), &u32temp, 4);
			offset += RTW_TLV_LEN_4;

			u32temp = ((pxmitpriv->tx_pkts > 0xffffffff) ? 0xffffffff : (pxmitpriv->tx_pkts & 0xffffffff));
			_rtw_memcpy((tlv_content + offset), &u32temp, 4);
			offset += RTW_TLV_LEN_4;

			rtw_phl_get_hw_cnt_tx_fail(dvobj->phl, &tx_fail, &tx_fail_mgmt);
			u32temp = ((tx_fail > 0xffffffff) ? 0xffffffff : (tx_fail & 0xffffffff));
			_rtw_memcpy((tlv_content + offset), &u32temp, 4);
			offset += RTW_TLV_LEN_4;

			u16temp = psta->cur_tx_data_rate;
			_rtw_memcpy((tlv_content + offset), &u16temp, 2);
			offset += RTW_TLV_LEN_2;
			_rtw_memcpy((tlv_content + offset), &u16temp, 2);
			offset += RTW_TLV_LEN_2;

			u16temp = 100;
			_rtw_memcpy((tlv_content + offset), &u16temp, 2);
			offset += RTW_TLV_LEN_2;

			u32temp = ((precvpriv->rx_pkts > 0xffffffff) ? 0xffffffff : (precvpriv->rx_pkts & 0xffffffff));
			_rtw_memcpy((tlv_content + offset), &u32temp, 4);
			offset += RTW_TLV_LEN_4;

			u32temp = ((precvpriv->rx_errors > 0xffffffff) ? 0xffffffff : (precvpriv->rx_errors & 0xffffffff));
			_rtw_memcpy((tlv_content + offset), &u32temp, 4);
			offset += RTW_TLV_LEN_4;

			*(tlv_content + offset) = rtw_phl_get_sta_rssi(psta->phl_sta);
			offset++;
		}
	}

	*processed_bytes = offset;
	payload_len      = (offset - 2);
	_rtw_memcpy(tlv_content, &payload_len, 2);
	ret = 0;

/*
	if(payload_len) {
		for(i = 0; i < 41; i++) {
			printk("%02x ", tlv_content[i]);
		}
	}
*/
	return ret;
}

/*
 *	current format is:
 *		Field:          			Length:				Value:
 *		1 layer         			1 octet				k
 *		2 layer         			6 octets			client mac address
 *		2 layer         			8 octets			link time
 *		2 layer         			1 octets			profile
 *		Total Size: 2 + k(6 + 8 + 1)
 */
int core_map_get_sta_info(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes) {
	int ret     = -1;
	int sta_num = 0;
	u8 *p = NULL;
	u8 i, sta_size;
	u32 link_time = 0;
	struct list_head *phead, *plist;
	struct sta_priv *pstapriv   = &padapter->stapriv;
	struct sta_info *psta       = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	u8 *mybssid                 = get_bssid(pmlmepriv);

	_rtw_memset(tlv_content, 0, maximum_limit);
	sta_size = (6 + 4+ 1);
	p        = tlv_content;
	p++; //reserve the first byte for sta number
	*processed_bytes = 1;

	if (MLME_IS_STA(padapter)) {
		if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
			|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE) {
			if (!_rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN)) {
				psta = rtw_get_stainfo(pstapriv, mybssid);
				if(psta && psta->phl_sta && !is_broadcast_mac_addr(psta->phl_sta->mac_addr)) {
					if((*processed_bytes + sta_size) <= maximum_limit) {
						_rtw_memcpy(p, psta->phl_sta->mac_addr, ETH_ALEN);
						p += ETH_ALEN;

						link_time = psta->link_time;
						_rtw_memcpy(p, &link_time, 4);
						p += 4;

						*p = psta->multiap_profile;
						p++;

						sta_num++;
						*processed_bytes += sta_size;
					} else { //overflow to return error
						RTW_ERR("[%s]Buffer is not enough\n", __FUNCTION__);
						goto fail;
					}
				}
			}
		}
	} else {
		_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
		phead = &pstapriv->asoc_list;
		plist = phead->next;
		while ((plist != phead)) {
			if((*processed_bytes + sta_size) > maximum_limit) {//overflow to return error
				RTW_ERR("[%s]Buffer is not enough\n", __FUNCTION__);
				_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
				goto fail;
			}
			psta = list_entry(plist, struct sta_info, asoc_list);
			_rtw_memcpy(p, psta->phl_sta->mac_addr, ETH_ALEN);
			p += ETH_ALEN;

			link_time = psta->link_time; //u32 to u64 to avoid memcpy garbage
			_rtw_memcpy(p, &link_time, 4);
			p += 4;

			*p = psta->multiap_profile;
			p++;

			sta_num++;
			*processed_bytes += sta_size;
			plist = plist->next;
		}
		_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
	}
	*tlv_content = sta_num;
	ret          = 0;

#if 0 //debug
	p = tlv_content;
	RTW_INFO("===>(%s)%u len:%d sta_num = %u\n",
		padapter->pnetdev->name, MLME_IS_STA(padapter), *processed_bytes, sta_num);
	for(i = 0; i < *processed_bytes; i++) {
		RTW_INFO("%02X ", *p);
		p++;
	}
#endif

	return ret;
fail:
	*tlv_content     = 0;
	*processed_bytes = 0;
	return ret;
}

// tlv_content currently holds 2+6n bytes, first byte is whether allow station mode, next byte is number of stations, say n.
// Following is 6n bytes, each 6 byte correspond to mac addr of 1 station.
// After processing, tlv_content should have 1 + n bytes, the first byte is number of sta, which should be unchanged.
// Following byte each correspond to rssi value of a station. 0 value means either sta not found or rssi is indeed 0.
int core_map_get_clients_rssi(_adapter *padapter, u8 *tlv_content, u16 maximum_limit, int *processed_bytes)
{
	int ret     = -1;
	int sta_num = 0;
	u8 total_rssi_needed = 0; // u8 should be enough to hold max number of stations' rssi.
	u8 *p = NULL;
	u8 i, sta_rssi_size;
	u8 current_sta_rssi = 0;
	struct list_head *phead, *plist;
	struct sta_priv *pstapriv   = &padapter->stapriv;
	struct sta_info *psta       = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	u8 *mybssid                 = get_bssid(pmlmepriv);
	u8 allow_sta_mode;
	u8 client_mac[ETH_ALEN] = {0};

	_rtw_memcpy(&allow_sta_mode, tlv_content, 1);
	_rtw_memcpy(&total_rssi_needed, tlv_content + 1, 1);

	sta_rssi_size = 1;

	// Reserve the first 1 byte, indicating number of rssi that follows.
	// Following bytes each contains 1 rssi.
	p = tlv_content + 1;
	*processed_bytes = 1;

	if (MLME_IS_STA(padapter)) {
		if (allow_sta_mode) {
			if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE) == _TRUE
				|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE) {
				if (!_rtw_memcmp(mybssid, "\x0\x0\x0\x0\x0\x0", ETH_ALEN)) {
					psta = rtw_get_stainfo(pstapriv, mybssid);
					if(psta && psta->phl_sta && !is_broadcast_mac_addr(psta->phl_sta->mac_addr) && (_rtw_memcmp(client_mac, psta->phl_sta->mac_addr, ETH_ALEN) == _TRUE)) {
						if((*processed_bytes + sta_rssi_size) <= maximum_limit) {
							*p = rtw_phl_get_sta_rssi(psta->phl_sta);
							p += 1;

							sta_num++;
							*processed_bytes += sta_rssi_size;
						} else { //overflow to return error
							RTW_ERR("[%s]Buffer is not enough\n", __FUNCTION__);
							goto fail;
						}
					}
				}
			}
		} else {
			RTW_ERR("AP is in station mode, does not allow map rssi query. \n");
			goto fail;
		}
	} else {
		for (i = 0; i < total_rssi_needed; i++) {
			_rtw_memcpy(client_mac, tlv_content + 2 + i * ETH_ALEN, ETH_ALEN);
			psta = rtw_get_stainfo(pstapriv, client_mac);
			if (psta && psta->phl_sta && (_rtw_memcmp(client_mac, psta->phl_sta->mac_addr, ETH_ALEN) == _TRUE)) {
				current_sta_rssi = rtw_phl_get_sta_rssi(psta->phl_sta);
			} else {
				RTW_INFO("Map get client rssi - STA %02x%02x%02x%02x%02x%02x get info error. \n",
					client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5]);
				current_sta_rssi = 0;
			}
			if((*processed_bytes + sta_rssi_size) > maximum_limit) {
				RTW_ERR("[%s]Buffer is not enough\n", __FUNCTION__);
				goto fail;
			}
			*p = current_sta_rssi;
			p += 1;

			sta_num++;
			*processed_bytes += sta_rssi_size;
		}
	}
	*tlv_content = sta_num;
	ret          = 0;

	return ret;
fail:
	*tlv_content     = 0;
	*processed_bytes = 0;
	return ret;
}

#ifdef CONFIG_RTW_ACS
int core_map_acs_trigger(_adapter *padapter)
{
	u8 _status = _FALSE;
	u8 ssc_chk;

	if (!is_primary_adapter(padapter))
		goto exit;

	ssc_chk = rtw_sitesurvey_condition_check(padapter, _FALSE);
	if (ssc_chk != SS_ALLOW)
		goto exit;

	rtw_ps_deny(padapter, PS_DENY_SCAN);
	if (_FAIL == rtw_pwr_wakeup(padapter))
		goto cancel_ps_deny;
	if (!rtw_is_adapter_up(padapter)) {
		RTW_INFO("scan abort!! adapter cannot use\n");
		goto cancel_ps_deny;
	}

	padapter->multiap_acs_trigger = 1;
	acs_reset_info(padapter);
	_status = rtw_set_acs_sitesurvey(padapter);

	return _status;

cancel_ps_deny:
	rtw_ps_deny_cancel(padapter, PS_DENY_SCAN);
exit:
	return _FALSE;
}

#define MAP_ACS_CANDI_CHANNEL_NUM 6
u8 acs_candidate_ch[MAP_ACS_CANDI_CHANNEL_NUM] = {1,6,11,36,52,149};

int is_map_acs_candidate_channel(_adapter *padapter, u8 channel)
{
	int i;
	for (i = 0; i < MAP_ACS_CANDI_CHANNEL_NUM; i++) {
		if (channel == acs_candidate_ch[i])
			return true;
	}

	return false;
}

#endif

#ifdef CONFIG_NSB_MULTI_AP_EVENT
void core_map_send_client_event(_adapter *adapter, u8* mac_address, u8 msg_type, u8 rssi, u16 frame_len, u8 *frame)
{
	u16 total_len = 0;
	u16 max_size = RTW_MAP_MAX_PAYLOAD_SZ;
	MULTI_AP_CLI_EVENT_STRU *multi_cli_event = NULL;
	
	if(rtw_map_user_pid == 0)
		return;

	total_len = sizeof(MULTI_AP_CLI_EVENT_STRU) + frame_len;
	if( total_len > max_size ){
		RTW_WARN("tunneled message total len is too long[%d].\n", total_len);
		return;
	}

	multi_cli_event = (MULTI_AP_CLI_EVENT_STRU *)rtw_zmalloc(max_size);
	if (multi_cli_event == NULL){
		RTW_WARN("multi_ap_tunnel malloc fail.\n");
		return;
	}

	multi_cli_event->tlv_type = TLV_TYPE_TUNNELED;
	multi_cli_event->tlv_len = total_len - 3;
	memcpy(multi_cli_event->macaddr, mac_address, ETH_ALEN);
	multi_cli_event->msg_type = msg_type;
	multi_cli_event->rssi = rssi;
	multi_cli_event->frame_len = frame_len;
	memcpy(multi_cli_event->frame, frame, frame_len);

	core_map_nl_event_send((u8 *)multi_cli_event, total_len);

	rtw_mfree(multi_cli_event, max_size);
}

void core_map_send_ch_change_event(u8 *bssid, u8 reason, u8 old_ch, u8 old_bw, u8 old_opclass, u8 ch, u8 bw, u8 opclass, u8 hwmode)
{
	struct map_event_msg_ch_change map_event_msg_ch_change;
	
	if(rtw_map_user_pid == 0)
		return;

	map_event_msg_ch_change.netlink_msg_type = MAP_GENERAL_NETLINK_MESSAGE;
	memcpy(map_event_msg_ch_change.bssid, bssid, MAC_ADDR_LEN);
	map_event_msg_ch_change.event_type = MASK_CHANNEL_CHANGE_EVENT;
	map_event_msg_ch_change.reason = reason;
	map_event_msg_ch_change.old_ch = old_ch;
	map_event_msg_ch_change.old_bw = old_bw;
	map_event_msg_ch_change.old_opclass = old_opclass;
	map_event_msg_ch_change.ch = ch;
	map_event_msg_ch_change.bw = bw;
	map_event_msg_ch_change.opclass = opclass;
	map_event_msg_ch_change.hwmode = hwmode;
	
	core_map_nl_event_send((u8 *)&map_event_msg_ch_change, sizeof(struct map_event_msg_ch_change));
}

void core_map_radar_detect_event(u8 *bssid, u8 ch)
{
	struct map_event_msg_radar_etect map_event_msg_radar_etect;
	
	if(rtw_map_user_pid == 0)
		return;

	map_event_msg_radar_etect.netlink_msg_type = MAP_GENERAL_NETLINK_MESSAGE;
	memcpy(map_event_msg_radar_etect.bssid, bssid, MAC_ADDR_LEN);
	map_event_msg_radar_etect.event_type = MASK_RADAR_DETECT_EVENT;
	map_event_msg_radar_etect.ch = ch;
	
	core_map_nl_event_send((u8 *)&map_event_msg_radar_etect, sizeof(struct map_event_msg_radar_etect));
}
#endif

u32 core_map_get_bh_ssid(struct dvobj_priv *dvobj, char **ssid)
{
	int i;
	struct mlme_priv *pmlmepriv;
	struct wlan_network *cur_network;

	*ssid = NULL;
	for (i = 0; i < dvobj->iface_nums; i++) {
		if (dvobj->padapters[i]->multi_ap_mode == MAP_MODE_BACKHAL_AP) {
			pmlmepriv = &(dvobj->padapters[i]->mlmepriv);
			cur_network = &(pmlmepriv->cur_network);
			*ssid = cur_network->network.Ssid.Ssid;
			return cur_network->network.Ssid.SsidLength;
		}
	}

	return 0;
}

u8 get_vsie_matched(_adapter *padapter, WLAN_BSSID_EX *network)
{
	u8 *ie;
	u32 ielen = 0;
	u8 val = 0;
	u32 remain_len = network->IELength - _BEACON_IE_OFFSET_;
	u8 *search_ie = network->IEs + _BEACON_IE_OFFSET_;
	u8 check_oui[] = {0x00, 0xe0, 0x4c, 0x1};

	while (1) {
		ie = rtw_get_ie_ex(search_ie, remain_len, WLAN_EID_VENDOR_SPECIFIC,
			check_oui, 4, NULL, &ielen);

		if (!ie)
			break;
		else if (ielen == 6)
			return true;

		search_ie = ie + ielen;
		remain_len = network->IELength - (search_ie - network->IEs);
	}
	return false;
}

u32 _core_map_check_bh_sta_exist(struct dvobj_priv *dvobj)
{
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (dvobj->padapters[i]->multi_ap_mode == MAP_MODE_BACKHAL_STA)
			return true;
	}

	return false;
}

void core_map_reorder_survey_result(_adapter *adapter, u8 lock)
{
	struct	mlme_priv	*pmlmepriv = &(adapter->mlmepriv);
	_list	*plist, *phead;
	struct	wlan_network	*pnetwork = NULL;
	struct	wlan_network	*selected_pnetwork = NULL;
	char *ssid_bh = NULL;
	u32 ssid_bh_len = 0;
#ifdef RTW_MI_SHARE_BSS_LIST
	_queue *queue = &adapter->dvobj->scanned_queue;
	u32 num_of_scanned = adapter->dvobj->num_of_scanned;
#else
	_queue *queue = &pmlmepriv->scanned_queue;
	u32 num_of_scanned = pmlmepriv->num_of_scanned;
#endif

	/* only reoder survey info for MAP BH STA */
	if (adapter->multi_ap_mode != MAP_MODE_BACKHAL_STA)
		return;

	if (!adapter->registrypriv.wifi_mib.multiap_controller_bh_rssi)
		return;

	ssid_bh_len = core_map_get_bh_ssid(adapter->dvobj, &ssid_bh);

	RTW_INFO("[%s]Controller mac: "MAC_FMT"\n",
		__func__,
		MAC_ARG(adapter->dvobj->map_controller_bh_mac));

	if (lock)
		_rtw_spinunlock_bh(&pmlmepriv->lock);

	phead = get_list_head(queue);
	plist = get_next(phead);

	while(1) {
		if (ssid_bh_len == 0)
			break;
		if (rtw_end_of_queue_search(phead, plist) == _TRUE)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (get_vsie_matched(adapter, &pnetwork->network) &&
			pnetwork->network.Rssi >= adapter->registrypriv.wifi_mib.multiap_controller_bh_rssi &&
			ssid_bh_len == pnetwork->network.Ssid.SsidLength &&
			memcmp(ssid_bh, pnetwork->network.Ssid.Ssid, ssid_bh_len) == 0) {
			selected_pnetwork = pnetwork;
			memcpy(adapter->dvobj->map_controller_bh_mac, pnetwork->network.MacAddress, 6);
			RTW_INFO("[%s]Update controller bh mac "MAC_FMT"\n",
				__func__,
				MAC_ARG(pnetwork->network.MacAddress));
			break;
		} else if (memcmp(adapter->dvobj->map_controller_bh_mac, pnetwork->network.MacAddress, 6) == 0) {
			if (pnetwork->network.Rssi >= adapter->registrypriv.wifi_mib.multiap_controller_bh_rssi) {
				selected_pnetwork = pnetwork;
				RTW_INFO("[%s]Controller bh mac matched "MAC_FMT"\n",
					__func__,
					MAC_ARG(pnetwork->network.MacAddress));
			}
			if (pnetwork->network.Ssid.SsidLength == 0) {
				pnetwork->network.Ssid.SsidLength = ssid_bh_len;
				memcpy(pnetwork->network.Ssid.Ssid, ssid_bh, ssid_bh_len);
				RTW_INFO("[%s]Copy controller bh SSID %s to entry "MAC_FMT"\n",
					__func__, pnetwork->network.Ssid.Ssid,
					MAC_ARG(pnetwork->network.MacAddress));
			}
		}

		plist = get_next(plist);
	}

	if (selected_pnetwork) {
		RTW_INFO("[%s]Reorder BSSID = "MAC_FMT" to first entry, rssi = %lu\n",
			__func__,
			MAC_ARG(selected_pnetwork->network.MacAddress),
			selected_pnetwork->network.Rssi);
		selected_pnetwork->network.Rssi = 100;
		selected_pnetwork->network.PhyInfo.SignalStrength = 100;

		list_del(&selected_pnetwork->list);
		list_add(&selected_pnetwork->list, phead);
	}

	if (lock)
		_rtw_spinlock_bh(&pmlmepriv->lock);
}

#endif /* CONFIG_RTW_MULTI_AP */

