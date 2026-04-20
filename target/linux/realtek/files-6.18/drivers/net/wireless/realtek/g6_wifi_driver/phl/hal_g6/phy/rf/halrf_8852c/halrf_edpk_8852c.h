/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALRF_EDPK_8852CH__
#define __HALRF_EDPK_8852CH__
#ifdef RF_8852C_SUPPORT
/*--------------------------Define Parameters-------------------------------*/
#define EDPK_VER_8852C 0x6
#define EDPK_RF_PATH_MAX_8852C 2
#define EDPK_KIP_REG_NUM_8852C 6
#define EDPK_BB_REG_NUM_8852C 9
#define EDPK_RF_REG_NUM_8852C 6
#define EDPK_PATH_A_8852C 1
#define EDPK_PATH_B_8852C 1
#define PATH_OFST_8852C 0x100
#define CH_OFST_8852C 0x4
#define PHY_OFST_8852C 0x80
#define EDPK_BY_NCTL_8852C 1
#define EDPK_RELOAD_EN_8852C 0
#define EDPK_REG_DBG 0
#define EDPK_RXSRAM_DBG_8852C 0

#define EDPK_PRE_TX_BB_REG_NUM_8852C 5

/*---------------------------End Define Parameters----------------------------*/

void halrf_edpk_8852c(struct rf_info *rf, enum phl_phy_idx phy_idx, bool force);

// void halrf_edpk_onoff_8852c(struct rf_info *rf, enum rf_path path, bool off);

void halrf_edpk_track_8852c(struct rf_info *rf);

void halrf_edpk_pre_tx_8852c(struct rf_info *rf, enum phl_phy_idx phy,
	u16 cnt, u16 period, s16 pwr, u32 delay);

void halrf_edpk_config_para_8852c(struct rf_info *rf);

#endif
#endif /*  __HALRF_EDPK_8852CH__ */
