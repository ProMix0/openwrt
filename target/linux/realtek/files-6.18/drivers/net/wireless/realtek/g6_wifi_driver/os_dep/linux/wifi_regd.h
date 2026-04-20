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

#ifndef __WIFI_REGD_H__
#define __WIFI_REGD_H__

void rtw_regd_apply_flags(struct wiphy *wiphy);
int rtw_regd_init(struct wiphy *wiphy);

#ifdef CONFIG_OPENSYNC
int cfg80211_cac_event_async(struct net_device *netdev, const struct cfg80211_chan_def *chandef, enum nl80211_radar_event event);
void rtw_regd_deinit(struct wiphy *wiphy);
#endif

#endif /* __WIFI_REGD_H__ */
