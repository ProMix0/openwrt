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
#ifndef __RTW_LED_H_
#define __RTW_LED_H_

/*@--------------------------[Define] ---------------------------------------*/
#define RTW_LED_BLINK_INTERVAL 	100
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
#define LED_BLINK_WPS_INTERVAL 	200
#endif
#define RTW_LED_ID 				RTW_LED_ID_0
#define RTW_LED_EVENT_DONT_CARE	RTW_LED_EVENT_LENGTH

#ifdef CONFIG_RTW_SW_LED_SHARED
#if defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_RTW_SW_LED_SHARED_USING_5G)
#define RTW_SHARED_LED_ID 1
#elif !defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_RTW_SW_LED_SHARED_USING_2G)
#define RTW_SHARED_LED_ID 1
#else
#define RTW_SHARED_LED_ID 0
#endif
#endif /* CONFIG_RTW_SW_LED_SHARED */

/*@--------------------------[Enum]------------------------------------------*/
typedef enum _LED_CTL_MODE {
	LED_CTL_DOWN,
	LED_CTL_UP_IDLE,
	LED_CTL_BUSY,
	LED_CTL_MANUAL,

	/* currently unused */
	LED_CTL_NO_LINK,
	LED_CTL_LINK,
	LED_CTL_SITE_SURVEY,

	LED_CTL_START_TO_LINK,
	LED_CTL_POWER_OFF,

	LED_CTL_START_WPS,
	LED_CTL_START_WPS_BOTTON,
	LED_CTL_STOP_WPS,
	LED_CTL_STOP_WPS_FAIL,
} LED_CTL_MODE;

#ifdef CONFIG_RTW_SW_LED_SHARED
enum rtw_led_toggle {
	RTW_LED_TOGGLE_OFF,
	RTW_LED_TOGGLE_ON,
};
#endif

/*@--------------------------[Structure]-------------------------------------*/
struct led_priv {
	bool manual_ctrl;
	LED_CTL_MODE mode;
	enum rtw_led_id led_id;
	enum rtw_led_opt manual_opt;
#ifdef CONFIG_WIFI_LED_SHARE_WITH_WPS
	LED_CTL_MODE last_mode;
#endif
#ifdef CONFIG_RTW_SW_LED_SHARED
	_timer led_timer;
	u32	interval;
	u8 toggle; /* enum rtw_led_toggle */
	u8 traffic;
#endif
};

/*@--------------------------[Prptotype]-------------------------------------*/
int	rtw_init_led_priv(_adapter *padapter);
#ifdef CONFIG_RTW_SW_LED_SHARED
extern void rtw_led_config(struct dvobj_priv *dvobj, u8 enable);
extern void rtw_free_led_priv(_adapter *padapter);
#endif
#ifdef CONFIG_RTW_SW_LED
void rtw_led_control(_adapter *adapter, LED_CTL_MODE ctl);
void rtw_led_traffic_update(_adapter *adapter);
#else
#define rtw_led_control(adapter, ctl) do {} while (0)
#define rtw_led_traffic_update(adapter) do {} while (0)
#endif
#define rtw_led_tx_control(adapter, da) do {} while (0)
#define rtw_led_rx_control(adapter, da) do {} while (0)
#define rtw_led_set_iface_en(adapter, en) do {} while (0)
#define rtw_led_set_iface_en_mask(adapter, mask) do {} while (0)
#define rtw_led_set_ctl_en_mask(adapter, ctl_mask) do {} while (0)
#define rtw_led_set_ctl_en_mask_primary(adapter) do {} while (0)
#define rtw_led_set_ctl_en_mask_virtual(adapter) do {} while (0)
#endif

#ifdef CONFIG_RTW_SW_LED_SHARED
extern _adapter *adapter_led_share[CONFIG_IFACE_NUMBER];
#endif
