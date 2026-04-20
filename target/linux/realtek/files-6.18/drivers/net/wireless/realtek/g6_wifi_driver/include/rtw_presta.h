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

#ifndef _RTW_PRESTA_H_
#define _RTW_PRESTA_H_

struct rtw_presta_s *rtw_get_presta(struct sta_priv *pstapriv, const u8 sta_mac[ETH_ALEN]);
struct rtw_presta_s *rtw_retrieve_presta(struct sta_priv *pstapriv, const u8 sta_mac[ETH_ALEN]);
void rtw_init_presta_tbl(struct sta_priv *pstapriv);
void rtw_deinit_presta_tbl(struct sta_priv *pstapriv);
static inline void rtw_reset_presta_tbl(struct sta_priv *pstapriv) {
  rtw_deinit_presta_tbl(pstapriv);
  rtw_init_presta_tbl(pstapriv);
}

#endif /* _RTW_PRESTA_H_ */
