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
#ifndef __HALRF_EDPK_8192XBH__
#define __HALRF_EDPK_8192XBH__
#ifdef RF_8192XB_SUPPORT
/*--------------------------Define Parameters-------------------------------*/
#define EDPK_VER_8192XB 0x0b
#define EDPK_RF_PATH_MAX_8192XB 2
#define EDPK_KIP_REG_NUM_8192XB 6
//#define EDPK_BB_REG_NUM_8192XB 3
#define EDPK_BB_REG_NUM_8192XB 6

#define EDPK_PRE_TX_BB_REG_NUM_8192XB 5

#define EDPK_RF_REG_NUM_8192XB 4
#define EDPK_PATH_A_8192XB 1
#define EDPK_PATH_B_8192XB 1
#define EDPK_RELOAD_EN_8192XB 0
#define EDPK_REG_DBG_8192XB 1
#define EDPK_RXSRAM_DBG_8192XB 0
#define EDPK_COEF_DBG_8192XB 0



#define EDPK_PWSF_DF_8192XB 0x78
#define EDPK_BND_DF_8192XB 0x30

//=== boundary by RFE and ID ====
#define EDPK_RFE51_ID0_BND_8192XB 0x30
#define EDPK_RFE51_ID1_BND_8192XB 0x30
//-----
#define EDPK_RFE52_ID0_BND_8192XB 0x30
//---------
#define EDPK_RFE53_ID0_BND_8192XB 0x30
#define EDPK_RFE53_ID1_BND_8192XB 0x36
#define EDPK_RFE53_ID2_BND_8192XB 0x20
//-------
#define EDPK_RFE54_ID0_BND_8192XB 0x36
//========


//=== PWSF default by RFE and ID ====
#define EDPK_RFE51_ID0_PWSF_DF_8192XB 0x78
#define EDPK_RFE51_ID1_PWSF_DF_8192XB 0x78
//-----
#define EDPK_RFE52_ID0_PWSF_DF_8192XB 0x78
//---------
#define EDPK_RFE53_ID0_PWSF_DF_8192XB 0x78
#define EDPK_RFE53_ID1_PWSF_DF_8192XB 0x60
#define EDPK_RFE53_ID2_PWSF_DF_8192XB 0x78
//-------
#define EDPK_RFE54_ID0_PWSF_DF_8192XB 0x78
//=========




/*---------------------------End Define Parameters----------------------------*/

void halrf_edpk_8192xb(struct rf_info *rf, enum phl_phy_idx phy_idx, bool force);

void halrf_edpk_onoff_8192xb(struct rf_info *rf, enum rf_path path, bool off);

void halrf_edpk_track_8192xb(struct rf_info *rf);

void halrf_edpk_pre_tx_8192xb(struct rf_info *rf, enum phl_phy_idx phy,
	u16 cnt, u16 period, s16 pwr, u32 delay);

void halrf_edpk_config_para_8192xb(struct rf_info *rf);

void halrf_edpk_set_dgain_thres_8192xb(struct rf_info *rf, u32 val);


#endif
#endif /*  __HALRF_EDPK_8192XBH__ */
