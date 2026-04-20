/******************************************************************************
 *
 * Copyright(c) 2009-2010 - 2017 Realtek Corporation.
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

#ifdef CONFIG_IOCTL_CFG80211
void rtw_regd_apply_flags(struct wiphy *wiphy)
{
	struct dvobj_priv *dvobj = wiphy_to_dvobj(wiphy);
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	RT_CHANNEL_INFO *channel_set = rfctl->channel_set;
	u8 max_chan_nums = rfctl->max_chan_nums;

	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	unsigned int i, j;
	u16 channel;
	u32 freq;

	/* all channels disable */
	for (i = 0; i < NUM_NL80211_BANDS; i++) {
		sband = wiphy->bands[i];
		if (!sband)
			continue;
		for (j = 0; j < sband->n_channels; j++) {
			ch = &sband->channels[j];
			ch->flags = IEEE80211_CHAN_DISABLED;
		}
	}

	/* channels apply by channel plans. */
	for (i = 0; i < max_chan_nums; i++) {
		channel = channel_set[i].ChannelNum;
		#if CONFIG_IEEE80211_BAND_6GHZ
		if (channel_set[i].band == BAND_ON_6G)
			continue; /* TODO: wiphy with 6G band */
		else
		#endif
			freq = rtw_ch2freq(channel);
		ch = ieee80211_get_channel(wiphy, freq);
		if (!ch)
			continue;

		/* enable */
		ch->flags = 0;

		if (channel_set[i].flags & RTW_CHF_DFS) {
			/*
			* before integrating with nl80211 flow
			* bypass IEEE80211_CHAN_RADAR when configured with radar detection
			* to prevent from hostapd blocking DFS channels
			*/
#ifndef CONFIG_OPENSYNC
			if (rtw_rfctl_dfs_domain_unknown(rfctl))
#endif
				ch->flags |= IEEE80211_CHAN_RADAR;
		}

		if (channel_set[i].flags & RTW_CHF_NO_IR) {
			#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
			ch->flags |= IEEE80211_CHAN_NO_IBSS | IEEE80211_CHAN_PASSIVE_SCAN;
			#else
			ch->flags |= IEEE80211_CHAN_NO_IR;
			#endif
		}
	}
}

static void rtw_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request)
{
	switch (request->initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
		RTW_INFO("%s: %s\n", __func__, "NL80211_REGDOM_SET_BY_DRIVER");
		break;
	case NL80211_REGDOM_SET_BY_CORE:
		RTW_INFO("%s: %s\n", __func__, "NL80211_REGDOM_SET_BY_CORE");
		break;
	case NL80211_REGDOM_SET_BY_USER:
		RTW_INFO("%s: %s alpha2:%c%c\n", __func__, "NL80211_REGDOM_SET_BY_USER"
			, request->alpha2[0], request->alpha2[1]);
		regulatory_hint(wiphy,request->alpha2);
		rtw_set_country(wiphy_to_adapter(wiphy), request->alpha2);
		break;
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		RTW_INFO("%s: %s\n", __func__, "NL80211_REGDOM_SET_BY_COUNTRY_IE");
		break;
	}

	rtw_regd_apply_flags(wiphy);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
static int rtw_reg_notifier_return(struct wiphy *wiphy, struct regulatory_request *request)
{
	rtw_reg_notifier(wiphy, request);
	return 0;
}
#endif

#ifdef CONFIG_OPENSYNC
struct async_cac_change_evt {
	_list list; /* async_cac_change_list */
	struct net_device *netdev;
	struct cfg80211_chan_def chandef;
	enum nl80211_radar_event event;
};

static void async_cac_change_work_hdl(_workitem *work)
{
	struct rtw_wiphy_data *wiphy_data = container_of(work, struct rtw_wiphy_data, async_cac_change_work);
	struct async_cac_change_evt *evt;
	_list *list, *head = &wiphy_data->async_cac_change_list;
	while (1) {
		_rtw_mutex_lock_interruptible(&wiphy_data->async_cac_change_mutex);
		list = rtw_is_list_empty(head) ? NULL : get_next(head);
		if (list)
			rtw_list_delete(list);
		_rtw_mutex_unlock(&wiphy_data->async_cac_change_mutex);
		if (!list)
			break;
		evt = LIST_CONTAINOR(list, struct async_cac_change_evt, list);
		rtnl_lock();
		cfg80211_cac_event(evt->netdev, &evt->chandef, evt->event, GFP_KERNEL);
		rtnl_unlock();
		rtw_mfree(evt, sizeof(*evt));
	}
}

int cfg80211_cac_event_async(struct net_device *netdev
	, const struct cfg80211_chan_def *chandef, enum nl80211_radar_event event)
{
	_adapter *adapter = rtw_netdev_priv(netdev);
	struct wiphy *wiphy = adapter_to_wiphy(adapter);
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);
	struct async_cac_change_evt *evt;
	evt = rtw_malloc(sizeof(*evt));
	if (!evt)
		return _FAIL;
	_rtw_init_listhead(&evt->list);
	evt->netdev = netdev;
	_rtw_memcpy(&evt->chandef, chandef, sizeof(*chandef));
	evt->event = event;
	_rtw_mutex_lock_interruptible(&wiphy_data->async_cac_change_mutex);
	rtw_list_insert_tail(&evt->list, &wiphy_data->async_cac_change_list);
	_rtw_mutex_unlock(&wiphy_data->async_cac_change_mutex);
	_set_workitem(&wiphy_data->async_cac_change_work);
	return _SUCCESS;
}

static void rtw_regd_async_cac_change_list_free(struct wiphy *wiphy)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);
	struct async_cac_change_evt *evt;
	_list *cur, *head;
	_rtw_mutex_lock_interruptible(&wiphy_data->async_cac_change_mutex);
	head = &wiphy_data->async_cac_change_list;
	cur = get_next(head);
	while ((rtw_end_of_queue_search(head, cur)) == _FALSE) {
		evt = LIST_CONTAINOR(cur, struct async_cac_change_evt, list);
		cur = get_next(cur);
		rtw_list_delete(&evt->list);
		rtw_mfree(evt, sizeof(*evt));
	}
	_rtw_mutex_unlock(&wiphy_data->async_cac_change_mutex);
}
#endif

int rtw_regd_init(struct wiphy *wiphy)
{
	int ret;
#ifdef CONFIG_OPENSYNC
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
	wiphy->reg_notifier = rtw_reg_notifier_return;
#else
	wiphy->reg_notifier = rtw_reg_notifier;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
	wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
	wiphy->flags &= ~WIPHY_FLAG_STRICT_REGULATORY;
	wiphy->flags &= ~WIPHY_FLAG_DISABLE_BEACON_HINTS;
#else
	wiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;
#endif

#ifdef CONFIG_OPENSYNC
#if defined(CONFIG_DFS_MASTER)
	if (wiphy->bands[NL80211_BAND_5GHZ]) {
		wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_DFS_OFFLOAD);
	}
#endif

	_rtw_init_listhead(&wiphy_data->async_cac_change_list);
	_rtw_mutex_init(&wiphy_data->async_cac_change_mutex);
	_init_workitem(&wiphy_data->async_cac_change_work, async_cac_change_work_hdl, NULL);
#endif

	rtw_regd_apply_flags(wiphy);

	ret = wiphy_register(wiphy);
	if (ret == 0) {
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
		/* Do nothing */
		#else
		/* regulatory_hint() should be called after wiphy registered */
#ifdef CONFIG_OPENSYNC
		regulatory_hint(wiphy,"US");
#else
		regulatory_hint(wiphy,"00");
#endif
		#endif
	}
	return ret;
}

#ifdef CONFIG_OPENSYNC
void rtw_regd_deinit(struct wiphy *wiphy)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);

	_cancel_workitem_sync(&wiphy_data->async_cac_change_work);
	rtw_regd_async_cac_change_list_free(wiphy);
	_rtw_mutex_free(&wiphy_data->async_cac_change_mutex);
}
#endif
#endif /* CONFIG_IOCTL_CFG80211 */
