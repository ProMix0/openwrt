#ifndef __RTW_AP_WAPI_H__
#define __RTW_AP_WAPI_H__


#define	ETH_P_WAPI		0x88B4
#define WAPI_VERSION	1
#define WAI_PROTOCOL	1


#define	WAPI_KEY_LEN		16
#define WAPI_PN_LEN			16
#define WAPI_GCM_PN_LEN		12
#define	WAPI_EXT_HDR_LEN	18
#define WAPI_MIC_LEN		16


#define	RX_QUEUE_NUM		4


#define WAPI_DEBUG



#ifdef WAPI_DEBUG

/* WAPI trace debug */
extern u32 wapi_debug_component;

static inline void wapi_dump_buf(u8 *buf, u32 len)
{
	u32 i;
	printk("-----------------Len %d----------------\n", len);
	for (i = 0; i < len; i++)
		printk(KERN_CONT"%2.2x-", *(buf + i));
	printk("\n");
}

#define WAPI_TRACE(component, x, args...) \
	do { if (wapi_debug_component & (component)) \
			printk(x, ##args);\
	} while (0);

#define WAPI_DATA(component, x, buf, len) \
	do { if (wapi_debug_component & (component)) { \
			printk("%s:\n", x);\
			wapi_dump_buf((buf), (len)); } \
	} while (0);

#define RT_ASSERT_RET(_Exp)								\
	if (!(_Exp)) {									\
		printk("RTWLAN: ");					\
		printk("Assertion failed! %s,%s, line=%d\n", \
		       #_Exp, __FUNCTION__, __LINE__);          \
		return;						\
	}
#define RT_ASSERT_RET_VALUE(_Exp, Ret)								\
	if (!(_Exp)) {									\
		printk("RTWLAN: ");					\
		printk("Assertion failed! %s,%s, line=%d\n", \
		       #_Exp, __FUNCTION__, __LINE__);          \
		return Ret;						\
	}

#else

#define RT_ASSERT_RET(_Exp) do {} while (0)
#define RT_ASSERT_RET_VALUE(_Exp, Ret) do {} while (0)
#define WAPI_TRACE(component, x, args...) do {} while (0)
#define WAPI_DATA(component, x, buf, len) do {} while (0)
#endif



enum WAPI_DEBUG_LEVEL {
	WAPI_INIT = 1,
	WAPI_API = BIT(1),
	WAPI_TX = BIT(2),
	WAPI_RX = BIT(3),
	WAPI_MLME = BIT(4),
	WAPI_IOCTL = BIT(5),
	WAPI_ERR = BIT(31)
};



typedef struct _WLAN_HDR_WAPI_EXTENSION {
	u8 keyIdx;
	u8 reserved;
	u8 PN[WAPI_PN_LEN];
} WLAN_HDR_WAPI_EXT;



typedef struct _wapi_key {
	u8 dataKey[WAPI_KEY_LEN];
	u8 micKey[WAPI_KEY_LEN];
} wapiKey;


typedef struct _wapi_ap_info {
	bool wapiMcastEnable;
	u8 txMcastPN[WAPI_PN_LEN];
	u8 rxMcastPN[WAPI_PN_LEN];

	u8 keyIdx;
	wapiKey wapiMcastKey[2];
	
} RTL_WAPI_AP_INFO;

typedef struct _wapi_sta_info {
	bool wapiUcastEnable;
	u8 txUcastPN[WAPI_PN_LEN];
	u8 rxUcastPN[RX_QUEUE_NUM][WAPI_PN_LEN];

	u8 keyIdx;
	wapiKey wapiUcastKey[2];
	
} RTL_WAPI_STA_INFO;




void rtw_dump_buff(u8 *buff, uint len, char *str);
void rtw_wapi_init(_adapter *padapter) ;
u8 rtw_wapi_compare_PN(u8 *PN1, u8 *PN2, u8 len);
u8 rtw_wapi_increase_PN(u8 *PN, u8 len , u8 AddCount);
u8 rtw_wapi_is_wai_packet(_adapter *adapter, union recv_frame *precv_frame);
int rtw_validate_wapi_data_packet(_adapter *padapter, union recv_frame *precv_frame);

bool rtw_wapi_drop_for_key_absent(_adapter *padapter, u8 *pRA);
void rtw_wapi_get_iv(_adapter *padapter, u8 *pRA, u8 *IV);
int rtw_wapi_set_key(_adapter *padapter, struct ieee_param *param);

int rtw_wapi_is_data_packet(union recv_frame *precv_frame);
int rtw_wapi_is_icmp_packet(union recv_frame *precv_frame);
int rtw_wapi_is_arp_packet(union recv_frame *precv_frame);




u32	rtw_sms4_encrypt(_adapter *padapter, u8 *pxmitframe);
u32	rtw_sms4_decrypt(_adapter *padapter, u8 *precvframe);

u32	rtw_gcm_sm4_encrypt(_adapter *padapter, u8 *pxmitframe);
u32	rtw_gcm_sm4_decrypt(_adapter *padapter, u8 *precvframe);

int rtw_wapi_check_frame_qos(u8 *pframe, u16 pktlen);

#endif


