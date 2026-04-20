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
#include "../halrf_precomp.h"
#ifdef RF_8852C_SUPPORT

/*8852C EDPK ver:0x1 20230811*/

// void _edpk_bkup_bb_8852c(
// 	struct rf_info *rf,
// 	u32 *reg,
// 	u32 reg_bkup[EDPK_BB_REG_NUM_8852C])
// {
// 	u8 i;

// 	for (i = 0; i < EDPK_BB_REG_NUM_8852C; i++) {
// 		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
// 		if (EDPK_REG_DBG)
// 			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup 0x%x = %x\n", reg[i], reg_bkup[i]);
// 	}
// }

void _edpk_reload_kip_8852c(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][EDPK_KIP_REG_NUM_8852C],
	u8 path) 
{
	u8 i;

	for (i = 0; i < EDPK_KIP_REG_NUM_8852C; i++) {
		halrf_wreg(rf, reg[i] + (path << 8), MASKDWORD, reg_bkup[path][i]);
		if (EDPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload 0x%x = %x\n", reg[i] + (path << 8),
				   reg_bkup[path][i]);
	}
}

// void _edpk_reload_bb_8852c(
// 	struct rf_info *rf,
// 	u32 *reg,
// 	u32 reg_bkup[EDPK_BB_REG_NUM_8852C]) 
// {
// 	u8 i;

// 	for (i = 0; i < EDPK_BB_REG_NUM_8852C; i++) {
// 		halrf_wreg(rf, reg[i], MASKDWORD, reg_bkup[i]);
// 		if (EDPK_REG_DBG)
// 			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload 0x%x = %x\n", reg[i],
// 				   reg_bkup[i]);
// 	}
// }

void _edpk_reload_rf_8852c(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][EDPK_RF_REG_NUM_8852C],
	u8 path)
{
	u8 i;

	for (i = 0; i < EDPK_RF_REG_NUM_8852C; i++) {
		halrf_wrf(rf, path, rf_reg[i], MASKRF, rf_bkup[path][i]);
		if (EDPK_REG_DBG)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}

u8 _edpk_one_shot_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	enum dpk_id id)
{

	u16 dpk_cmd = 0x0;
	//u32 r_bff8 = 0x0, r_80fc = 0x0, cnt1 = 0, cnt2 = 0;

	if (id == D_KIP_PRESET) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00002819;
		else 
			dpk_cmd = 0x0000282b;
	} else if (id == D_TXAGC) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00002919;
		else 
			dpk_cmd = 0x0000292b;
	} else if (id == D_SYNC) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00002b19;
		else 
			dpk_cmd = 0x00002b2b;
	} else if (id == LBK_RXIQK) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00000619;
		else 
			dpk_cmd = 0x00000629;
	} else if (id == D_GAIN_LOSS) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00002c19;
		else 
			dpk_cmd = 0x00002c2b;
	} else if (id == D_MDPK_IDL) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00002d19;
		else
			dpk_cmd = 0x00002d2b;
	} else if (id == D_KIP_RESTORE) {
		if (path == RF_PATH_A)
			dpk_cmd = 0x00003119;
		else 
			dpk_cmd = 0x0000312b;
	}
	else	
		dpk_cmd = (u16)((id << 8) | (0x19 + path * 0x12));

	
	halrf_do_one_shot_8852c(rf, path, 0x8000, MASKDWORD, dpk_cmd);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] one-shot for %s = 0x%04x (cnt:%d/%d)\n",
	       id == 0x28 ? "KIP_PRESET" : (id == 0x29 ? "EDPK_TXAGC" :
	       (id == 0x2a ? "EDPK_RXAGC" : (id == 0x2b ? "SYNC" :
	       	(id == 0x2c ? "GAIN_LOSS" : (id == 0x2d ? "MEDPK_IDL" :
		(id == 0x2f ? "EDPK_GAIN_NORM" : (id == 0x31 ? "KIP_RESOTRE" :
		(id == 0x6 ? "LBK_RXIQK" : "Unknown id")))))))), dpk_cmd,
			rf->nctl_ck_times[0], rf->nctl_ck_times[1]);

	if (rf->nctl_ck_times[0] == 2000 || rf->nctl_ck_times[1] == 2000) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] one-shot over 20ms!!!!\n");
		return 1;
	} else
		return 0;
}

// void _edpk_init_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u8 kidx = dpk->cur_idx[path];

// 	dpk->bp[path][kidx].path_ok = 0;
// 	dpk->ov_flag[path] = 0;
// }

void _edpk_information_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 kidx = dpk->cur_idx[path];

	dpk->bp[path][kidx].band = rf->hal_com->band[phy].cur_chandef.band;
	dpk->bp[path][kidx].ch = rf->hal_com->band[phy].cur_chandef.center_ch;
	dpk->bp[path][kidx].bw = rf->hal_com->band[phy].cur_chandef.bw;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d] (PHY%d): TSSI %s/ DBCC %s/ %s/ CH%d/ %s\n",
	       path, dpk->cur_idx[path], phy, rf->is_tssi_mode[path] ? "on" : "off",
	       rf->hal_com->dbcc_en ? "on" : "off",
	       dpk->bp[path][kidx].band == 0 ? "2G" : (dpk->bp[path][kidx].band == 1 ? "5G" : "6G"),
	       dpk->bp[path][kidx].ch,
	       dpk->bp[path][kidx].bw == 0 ? "20M" : (dpk->bp[path][kidx].bw == 1 ? "40M" :
	       (dpk->bp[path][kidx].bw == 2 ? "80M" : "160M")));
}

// void _edpk_rxagc_onoff_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	bool turn_on)
// {
// 	if (path == RF_PATH_A)
// 		halrf_wreg(rf, 0x4730, BIT(31), turn_on);
// 	else
// 		halrf_wreg(rf, 0x4a9c, BIT(31), turn_on);

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RXAGC is %s\n", path,
// 		turn_on ? "turn_on" : "turn_off");
// }

void _edpk_bb_afe_setting_8852c(
	struct rf_info *rf,
	enum rf_path path)
{
	/*1. Keep ADC_fifo reset*/
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x0);
	/*2. BB for IQK DBG mode*/
	halrf_wreg(rf, 0x5670 + (path << 13), MASKDWORD, 0xf801fffd); /*bit13 for gapk_offset*/
	/*3.Set DAC clk*/
	halrf_txck_force_8852c(rf, path, true, DAC_960M);
	/*4. Set ADC clk*/
	halrf_rxck_force_8852c(rf, path, true, ADC_1920M);
	halrf_wreg(rf, 0x5670 + (path << 13), BIT(30) | BIT(29), 0x2);

	halrf_wreg(rf, 0x12a0 | (path <<13), 0xff800000, 0x49);
	halrf_wreg(rf, 0xc0d4 + (path << 8), 0x780, 0x8);
	halrf_wreg(rf, 0xc0d4 + (path << 8), 0x7800, 0x0);
	halrf_wreg(rf, 0xc0c4, 0x003e0000, 0x6);
	halrf_wreg(rf, 0xc0e8 + (path << 8), 0xffff0000, 0x9);
	halrf_wreg(rf, 0xc0e4 + (path << 8), 0x00000030, 0x2);
	halrf_wreg(rf, 0xc0d4 + (path << 8), 0x0c000000, 0x2);
	halrf_wreg(rf, 0xc0d8 + (path << 8), 0x000001e0, 0x4);

	//EDPK force CBW_idx 0 
	halrf_wreg(rf, 0xc0ec | (path << 8), 0x00006000, 0x0);

	halrf_wreg(rf, 0x12b8 + (path << 13), BIT(30), 0x1);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x1f);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x13);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0001);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0041);
	/*5. ADDA fifo rst*/
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x1);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d BB AFE setting\n", path);
}

void _edpk_bb_afe_restore_8852c(
	struct rf_info *rf,
	enum rf_path path)
{

	halrf_wreg(rf, 0x12b8 + (path << 13), BIT(30), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x0);
	halrf_wreg(rf, 0x5670 + (path << 13), MASKDWORD, 0x00000000);
	halrf_wreg(rf, 0x12a0 + (path << 13), 0x000FF000, 0x00); /*[19:12]*/
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x0);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d BB/AFE restore\n", path);
}

void _edpk_tssi_pause_8852c(
	struct rf_info *rf,
	enum rf_path path,
	bool is_pause)
{
	halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), is_pause);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d TSSI %s\n", path,
	       is_pause ? "pause" : "resume");
}

void _edpk_tpg_sel_8852c(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	//enum rf_path rx_path;

	if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x3);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0x0180ff30);
	} else if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_80) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x0);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xffe0fa00);
	} else if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40) {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x2);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xff4009e0);
	} else {
		halrf_wreg(rf, 0x806c, BIT(2) | BIT (1), 0x1);
		halrf_wreg(rf, 0x8068, MASKDWORD, 0xf9f007d0);
	}

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] TPG Select for %s\n",
	       dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160 ? "160M" :
	       (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_80 ? "80M" : 
	       (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40 ? "40M" : "20M")));

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] TPG bw %d\n", dpk->bp[path][kidx].bw);
}

void _edpk_txpwr_bb_force_8852c(
	struct rf_info *rf,
	enum rf_path path,
	bool is_force)
{
	halrf_wreg(rf, 0x56cc + (path << 13), BIT(28), is_force); /*txpwr_bb_force_on*/
	halrf_wreg(rf, 0x580c + (path << 13), BIT(15), is_force); /*txpwr_bb_force_rdy*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d txpwr_bb_force %s\n", path, is_force ? "on" : "off");
}

void _edpk_kip_pwr_clk_onoff_8852c(
	struct rf_info *rf,
	bool turn_on)
{
	if (turn_on) {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080); /*cip power on*/
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x807f030a); /*320M*/
	} else {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
		halrf_wreg(rf, 0x80f4, BIT(18), 0x1);
	}

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] KIP Power/CLK is %s\n", turn_on ? "turn_on" : "turn_off");
}

// void _edpk_tx_on_retore_8852c(struct rf_info *rf) {
// 	halrf_wreg(rf, 0x5890, MASKDWORD, 0x00000400);
// 	halrf_wreg(rf, 0x7890, MASKDWORD, 0x00000002);
// 	halrf_wreg(rf, 0x5880, MASKDWORD, 0x30777777);
// 	halrf_wreg(rf, 0x5884, MASKDWORD, 0x77777477);
// 	halrf_wreg(rf, 0x7880, MASKDWORD, 0x77770347);
// 	halrf_wreg(rf, 0x7884, MASKDWORD, 0x77777777);
// }

void _edpk_kip_control_rfc_8852c(
	struct rf_info *rf,
	enum rf_path path,
	bool ctrl_by_kip)
{
	halrf_wreg(rf, 0x5670 + (path << 13), BIT(1), ctrl_by_kip); /*KIP control RFC*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] RFC is controlled by %s\n", ctrl_by_kip ? "KIP" : "BB");
}

void _edpk_kip_preset_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	/*cip power on*/
	//halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080);	
	/*320M*/
	//halrf_wreg(rf, 0x8088, MASKDWORD, 0x807f030a);

#if 0
	/*must before kip control RFC*/
	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x8078 = 0x%x\n", halrf_rreg(rf, 0x8078, MASKDWORD));

	if (rf->hal_com->cv == CAV)
		halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0x00003F00, 0x01); /*[13:8] thermal slope*/
	else
		halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0x00003F00, 0x0c); /*[13:8] thermal slope*/
#endif
	
	_edpk_kip_control_rfc_8852c(rf, path, true);

	halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), kidx); /*ch0/ch1 selection*/

	//halrf_wreg(rf, 0x8078, 0x000FFFFF, 0x50121); /*[19:0]*/

	_edpk_one_shot_8852c(rf, phy, path, D_KIP_PRESET);

	halrf_wreg(rf, 0x8120, MASKDWORD, 0xce000a08); 
	halrf_wreg(rf, 0x8220, MASKDWORD, 0xce000a08); 
	halrf_wreg(rf, 0x81a0 + (path << 8), 0x00000040, 0x1);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d CIP_for_DPK_nctl\n", path);
}

void _edpk_kip_restore_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	_edpk_one_shot_8852c(rf, phy, path, D_KIP_RESTORE);

	//efem
	halrf_wreg(rf, 0x8120, MASKDWORD, 0x10010000); 
	halrf_wreg(rf, 0x8220, MASKDWORD, 0x10010000); 
	/*CFIR CLK restore*/
	halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
	/*cip power on*/
	halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);

	halrf_wreg(rf, 0x80f4, BIT(18), 0x1);
	
	_edpk_kip_control_rfc_8852c(rf, path, false);
	_edpk_txpwr_bb_force_8852c(rf, path, false);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d restore KIP\n", path);
}

// u8 _edpk_dbm_convert_8852c(
// 	struct rf_info *rf,
// 	u8 dbm)
// {
// 	u8 txagc_cw;

// 	txagc_cw = (dbm - 16) * 4 + 0x40; /*0.25dB/step*/

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] convert %ddBm to 0x%x\n", dbm, txagc_cw);

// 	return txagc_cw;
// }

void _edpk_read_rxsram_8852c(
	struct rf_info *rf)
{
	u32 addr;

	halrf_wreg(rf, 0x80e8, BIT(7), 0x1);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x1);	/*rxsram_ctrl_sel*/
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00020000);	/*rpt_sel*/

	for (addr = 0; addr < 0x200; addr++) {
		halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00010000 | addr);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] RXSRAM[%03d] = 0x%07x\n", addr,
			halrf_rreg(rf, 0x80fc, MASKDWORD));
	}
	halrf_wreg(rf, 0x80e8, BIT(7), 0x0);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x0);	/*rxsram_ctrl_sel*/
}

// void _edpk_rf_reg_query_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	u32 reg)
// {
// #if 1
// 	u32 ori_ctrl = halrf_rreg(rf, 0x5670 + (path << 13), BIT(1));

// 	_edpk_kip_control_rfc_8852c(rf, path, false);

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] RF 0x%x = 0x%x\n", reg,
// 		halrf_rrf(rf, path, reg, MASKRF));

// 	_edpk_kip_control_rfc_8852c(rf, path, (bool)ori_ctrl);
// #endif
// }

void _edpk_kset_query_8852c(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	halrf_wreg(rf, 0x81d4 + (path << 8), 0x003F0000, 0x10);	/*rpt_sel*/

	dpk->cur_k_set = (u8)(halrf_rreg(rf, 0x81fc + (path << 8), 0xE0000000) - 1); /*[31:29]*/

	/*RF_DBG(rf, DBG_RF_DPK, "[EDPK] cur k_set = %d\n", dpk->cur_k_set);*/
}

// void _edpk_para_query_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	u8 kidx)
// {
// 	/*[31:26]:t-meter, [25:16]:txagc_bb, [15:7]:txagc, [6:0]:gs*/
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
// 			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};
// 	u32 para;

// 	para = halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), MASKDWORD);

// 	dpk->bp[path][kidx].txagc_dpk = (para >> 10) & 0x3f;
// 	dpk->bp[path][kidx].ther_dpk = (para >> 26) & 0x3f;

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] thermal/ txagc_RF (K%d) = 0x%x/ 0x%x\n",
// 		dpk->cur_k_set, dpk->bp[path][kidx].ther_dpk, dpk->bp[path][kidx].txagc_dpk);
// #if 0
// 	if (kidx == 0) { /*CH0*/
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K0= 0x%x\n",
// 			halrf_rreg(rf, 0x8190 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K1= 0x%x\n",
// 			halrf_rreg(rf, 0x8194 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K2= 0x%x\n",
// 			halrf_rreg(rf, 0x8198 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K3= 0x%x\n",
// 			halrf_rreg(rf, 0x81a4 + (path << 8), MASKDWORD));
// 	} else { /*CH1*/
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K0= 0x%x\n",
// 			halrf_rreg(rf, 0x81a8 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K1= 0x%x\n",
// 			halrf_rreg(rf, 0x81c4 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K2= 0x%x\n",
// 			halrf_rreg(rf, 0x81c8 + (path << 8), MASKDWORD));
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K3= 0x%x\n",
// 			halrf_rreg(rf, 0x81e8 + (path << 8), MASKDWORD));
// 	}
// #endif
// }

void _edpk_efem_control_8852c(
	struct rf_info *rf, u8 value)
{
	if(value == 53 || value == 54)
	{
		halrf_wreg(rf, 0x5890, BIT(6), 0x1);   // GPIO[6]=1
		halrf_wreg(rf, 0x5890, BIT(7), 0x1);   // GPIO[7]=0 
		halrf_wreg(rf, 0x5890, BIT(10), 0x0);   // GPIO[10]=0
	} else if(value == 51 || value == 52)
	{
		halrf_wreg(rf, 0x5890, BIT(6), 0x1);   // GPIO[6]=1
		halrf_wreg(rf, 0x5890, BIT(7), 0x0);   // GPIO[7]=0 
		halrf_wreg(rf, 0x5890, BIT(10), 0x0);   // GPIO[10]=0
	}
	halrf_wreg(rf, 0x5880, MASKDWORD, 0x77777777); // SW control
	halrf_wreg(rf, 0x5884, MASKDWORD, 0x77777777); // SW control

	if(value == 53 || value == 54)
	{
		halrf_wreg(rf, 0x7890, BIT(1), 0x0);    // GPIO[1]=0
		halrf_wreg(rf, 0x7890, BIT(2), 0x1);    // GPIO[2]=0
		halrf_wreg(rf, 0x7890, BIT(3), 0x1);    // GPIO[3]=1
	} else if(value == 51 || value == 52)
	{
		halrf_wreg(rf, 0x7890, BIT(1), 0x0);    // GPIO[1]=0
		halrf_wreg(rf, 0x7890, BIT(2), 0x0);    // GPIO[2]=0
		halrf_wreg(rf, 0x7890, BIT(3), 0x1);    // GPIO[3]=1
	}
	halrf_wreg(rf, 0x7880, MASKDWORD, 0x77777777); // SW control
	halrf_wreg(rf, 0x7884, MASKDWORD, 0x77777777); // SW control

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] 00c_tx_always_on\n");
}

bool _edpk_sync_check_8852c(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u16 dc_i, dc_q;
	u8 corr_val, corr_idx, rxbb;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/

	corr_idx = (u8)halrf_rreg(rf, 0x80fc, 0x000000ff);
	corr_val = (u8)halrf_rreg(rf, 0x80fc, 0x0000ff00);

	dpk->corr_idx[path][kidx] = corr_idx;
	dpk->corr_val[path][kidx] = corr_val;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x9);	/*rpt_sel*/

	dc_i = (u16)halrf_rreg(rf, 0x80fc, 0x0fff0000); /*[27:16]*/
	dc_q = (u16)halrf_rreg(rf, 0x80fc, 0x00000fff); /*[11:0]*/

	if (dc_i >> 11 == 1)
		dc_i = 0x1000 - dc_i;
	if (dc_q >> 11 == 1)
		dc_q = 0x1000 - dc_q;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d Corr_idx/ Corr_val /DC I/Q, = %d / %d / %d / %d\n",
		path, corr_idx, corr_val, dc_i, dc_q);

	dpk->dc_i[path][kidx] = dc_i;
	dpk->dc_q[path][kidx] = dc_q;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x8);	/*rpt_sel*/

	rxbb = (u8)halrf_rreg(rf, 0x80fc, 0x0000003F);	/*[5:0]*/

	//_edpk_rf_reg_query_8852c(rf, path, 0x00);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x31);	/*rpt_sel*/

	dpk->rxbb_ov[path] = (u8)halrf_rreg(rf, 0x80fc, BIT(8));

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RXBB/ RXAGC_done /RXBB_ovlmt = %d / %d / %d\n",
		path, rxbb, halrf_rreg(rf, 0x80fc, BIT(0)), dpk->rxbb_ov[path]);

#if 0 //ifem
	if ((dc_i > 200) || (dc_q > 200) || (corr_val < 170))
		return true;
#else //efem
	if ((corr_val < 0xbc))
		return true;
#endif
	else
		return false;
}

void _edpk_kip_set_txagc_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 dbm,
	bool set_from_bb)
{	

#if 0 // TBD
	if (set_from_bb)
	 {
		if (dbm >= 24)
			dbm = 24;
		else if (dbm <= 7)
			dbm = 7;
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] set S%d txagc to %ddBm\n", path, dbm);
		halrf_wreg(rf, 0x56cc + (path << 13), 0x0FF80000, dbm << 2); /*[27:19]*/
	}
#endif
	//halrf_wreg(rf, 0x56cc + (path << 13), 0x0ff80000, 0x050); // r_txpwr_bb_force_val
	halrf_wreg(rf, 0x56cc + (path << 13), 0x0ff80000, dbm << 2); // r_txpwr_bb_force_val
	
	_edpk_one_shot_8852c(rf, phy, path, D_TXAGC);

	_edpk_kset_query_8852c(rf, path); //TBD
	//_edpk_rf_reg_query_8852c(rf, path, 0x11);
}

// bool _edpk_kip_set_rxagc_8852c(
// 	struct rf_info *rf,
// 	enum phl_phy_idx phy,
// 	enum rf_path path,
// 	u8 kidx)
// {
// 	_edpk_kip_control_rfc_8852c(rf, path, false);
// 	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
// 	_edpk_kip_control_rfc_8852c(rf, path, true);

// 	_edpk_one_shot_8852c(rf, phy, path, D_RXAGC);//0x2a19
// #if 0
// 	halrf_wreg(rf, 0x80d4, 0x000F0000, 0x8);
// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] set RXBB = 0x%x\n",
// 		halrf_rreg(rf, 0x80fc, 0x0000001F));
// #endif
// 	return _edpk_sync_check_8852c(rf, path, kidx);
// }

// //void _edpk_set_rxbb(struct rf_info *rf, enum rf_path path, u32 val, u32 diff)
void _edpk_set_rxbb(struct rf_info *rf, enum rf_path path, u32 val)
{
	if (path == RF_PATH_A){
	//	halrf_wrf(rf, RF_PATH_B, 0x0, 0x003e0, (val + diff)); //pathB
		halrf_wrf(rf, RF_PATH_B, 0x0, 0x003e0, (val)); //pathB
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d 0x0 = %x 0x5 = %x 0x10005 =%x\n", RF_PATH_B, halrf_rrf(rf, RF_PATH_B, 0x0, 0x003e0), halrf_rrf(rf, RF_PATH_B, 0x5, MASKRF), halrf_rrf(rf, RF_PATH_B, 0x10005, MASKRF));
	} else {
	//	halrf_wrf(rf, RF_PATH_A, 0x0, 0x003e0, (val + diff) ); //pathA
		halrf_wrf(rf, RF_PATH_A, 0x0, 0x003e0, (val) ); //pathA
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d 0x0 = %x 0x5 = %x 0x10005 =%x\n", RF_PATH_A, halrf_rrf(rf, RF_PATH_A, 0x0, 0x003e0), halrf_rrf(rf, RF_PATH_A, 0x5, MASKRF), halrf_rrf(rf, RF_PATH_A, 0x10005, MASKRF));
	}
}


bool _edpk_kip_set_sync_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u32 val)
{
	_edpk_kip_control_rfc_8852c(rf, path, false);

#if 0	
	if (path == RF_PATH_A)
		_edpk_set_rxbb(rf, RF_PATH_B, 0x11);
	//	halrf_wrf(rf, RF_PATH_B, 0x0, 0x003e0, 0x11); //pathB
	else 
		_edpk_set_rxbb(rf, RF_PATH_A, 0x16);
	//	halrf_wrf(rf, RF_PATH_A, 0x0, 0x003e0, 0x16); //pathA
#else
	if (path == RF_PATH_A)
		_edpk_set_rxbb(rf, path, val);
		// halrf_wrf(rf, RF_PATH_B, 0x0, 0x003e0, 0x8); //pathB
	else 
		_edpk_set_rxbb(rf, path, val);
		// halrf_wrf(rf, RF_PATH_A, 0x0, 0x003e0, 0xC); //pathA
#endif

	_edpk_kip_control_rfc_8852c(rf, path, true);

	_edpk_one_shot_8852c(rf, phy, path, D_SYNC);

	return _edpk_sync_check_8852c(rf, path, kidx);
}


void _edpk_lbk_rxiqk_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	u8 cur_rxbb;
	u32 rf_11, reg_81cc;

	enum rf_path rx_path;

	if (path == RF_PATH_A)
		rx_path = RF_PATH_B;
	else 
		rx_path = RF_PATH_A;
	
	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1); /*Rx-IQC switch to MEDPK-mode*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x1); /*RxSRAM_ctrl_sel 0:MEDPK; 1:IQK*/

	_edpk_kip_control_rfc_8852c(rf, path, false);
	/*RF setting*/
	rf_11 = halrf_rrf(rf, path, 0x11, MASKRF);
	cur_rxbb = (u8)halrf_rrf(rf, rx_path, 0x00, MASKRFRXBB);
	reg_81cc = halrf_rreg(rf, 0x81cc + (path << 8), BIT(13) | BIT(12));

	//??
	// RF mode
	halrf_wrf(rf, path, 0x0, 0xf0000, 0x5);

	halrf_wrf(rf, path, 0x11, BIT(1) | BIT(0), 0x0);
	halrf_wrf(rf, path, 0x11, BIT(6) | BIT(5) | BIT(4), 0x1);
	halrf_wrf(rf, path, 0x11, 0x1F000, 0xa); /*[16:12]*/

	
	halrf_wrf(rf, rx_path, 0x00, MASKRFRXBB, 0x1f); /*[9:5]*/

	halrf_wreg(rf, 0x81cc + (path << 8), 0x0000003F, 0x12); /*[5:0]*/
	halrf_wreg(rf, 0x81cc + (path << 8), BIT(13) | BIT(12), 0x3);

	_edpk_kip_control_rfc_8852c(rf, path, true);

	halrf_wreg(rf, 0x802c, MASKDWORD, 0x00250025); /*Rx_tone_idx=0x025 (9.25MHz)*/

	_edpk_one_shot_8852c(rf, phy, path, LBK_RXIQK);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d LBK RXIQC = 0x%x\n", path,
		halrf_rreg(rf, 0x813c + (path << 8), MASKDWORD));

	/*restore*/
	_edpk_kip_control_rfc_8852c(rf, path, false);

	halrf_wrf(rf, path, 0x11, MASKRF, rf_11);
	halrf_wrf(rf, rx_path, 0x00, MASKRFRXBB, cur_rxbb);
	halrf_wreg(rf, 0x81cc + (path << 8), BIT(13) | BIT(12), reg_81cc);

	halrf_wreg(rf, 0x8074, BIT(31), 0x0); /*RxSRAM_ctrl_sel 0:MEDPK; 1:IQK*/
	halrf_wreg(rf, 0x80d0, BIT(21) | BIT(20), 0x0);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(1), 0x1); /*auto*/

	_edpk_kip_control_rfc_8852c(rf, path, true);
}

// void _edpk_get_thermal_8852c(struct rf_info *rf, u8 kidx, enum rf_path path)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
// 			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

// 	dpk->bp[path][kidx].ther_dpk = halrf_get_thermal_8852c(rf, path);

// 	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0xFC000000,
// 			dpk->bp[path][kidx].ther_dpk); /*[31:26]*/

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] thermal@EDPK (by driver)= 0x%x\n", dpk->bp[path][kidx].ther_dpk);
// }

void _edpk_rf_setting_8852c(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	enum rf_path rx_path;

	if (path == RF_PATH_A)
		rx_path = RF_PATH_B;
	else 
		rx_path = RF_PATH_A;
	
	halrf_wrf(rf, path, 0x00, MASKRF, 0x50121 | BIT(rf->hal_com->dbcc_en));
	halrf_wrf(rf, path, 0x10000, MASKRFMODE, RF_DPK);

	halrf_wrf(rf, rx_path, 0x00, MASKRF, 0x50121 | BIT(rf->hal_com->dbcc_en));
	halrf_wrf(rf, rx_path, 0x10000, MASKRFMODE, RF_DPK);

	if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160)
		halrf_wrf(rf, rx_path, 0x8f, 0x00060, 0x0);
	else
		halrf_wrf(rf, rx_path, 0x8f, 0x00060, 0x2);	

	halrf_wrf(rf, path, 0xa3, 0xf0000, 0xd); //IQGEN
	halrf_wrf(rf, rx_path, 0xa3, 0xf0000, 0xd); //IQGEN

	halrf_wrf(rf, rx_path, 0x02, 0x40000, 0x1);
	halrf_wrf(rf, rx_path, 0x02, 0x10000, 0x1);
	halrf_wrf(rf, rx_path, 0xdf, 0x01000, 0x1);
	halrf_wrf(rf, rx_path, 0x9e, 0x00100, 0x1);
	halrf_wrf(rf, rx_path, 0x8a, 0x80000, 0x1);
	halrf_wrf(rf, rx_path, 0x8d, 0x0000f, 0x1);
	halrf_wrf(rf, rx_path, 0x8d, 0x000f0, 0x1);
	halrf_wrf(rf, rx_path, 0x80, 0x00020, 0x0);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RF setting\n", path);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RF 0x0/0x2/0x80/0x8f/0x8a/0x8d/0xa3/0x9e/0xdf/0x10000 = 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x 0x%x/ 0x%x\n",
	       path, 
		   halrf_rrf(rf, path, 0x00, MASKRF),
	       halrf_rrf(rf, path, 0x02, MASKRF),
	       halrf_rrf(rf, path, 0x80, MASKRF),
	       halrf_rrf(rf, path, 0x8f, MASKRF),
		   halrf_rrf(rf, path, 0x8a, MASKRF),
		   halrf_rrf(rf, path, 0x8d, MASKRF),
		   halrf_rrf(rf, path, 0xa3, MASKRF),
	       halrf_rrf(rf, path, 0x9e, MASKRF),
	       halrf_rrf(rf, path, 0xdf, MASKRF),
	       halrf_rrf(rf, path, 0x10000, MASKRF));

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RF setting\n", rx_path);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RF 0x0/0x2/0x80/0x8f/0x8a/0x8d/0xa3/0x9e/0xdf/0x10000 = 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x 0x%x/ 0x%x\n",
	       rx_path, 
		   halrf_rrf(rf, rx_path, 0x00, MASKRF),
	       halrf_rrf(rf, rx_path, 0x02, MASKRF),
	       halrf_rrf(rf, rx_path, 0x80, MASKRF),
	       halrf_rrf(rf, rx_path, 0x8f, MASKRF),
		   halrf_rrf(rf, rx_path, 0x8a, MASKRF),
		   halrf_rrf(rf, rx_path, 0x8d, MASKRF),
		   halrf_rrf(rf, rx_path, 0xa3, MASKRF),
	       halrf_rrf(rf, rx_path, 0x9e, MASKRF),
	       halrf_rrf(rf, rx_path, 0xdf, MASKRF),
	       halrf_rrf(rf, rx_path, 0x10000, MASKRF));
}

// void _edpk_bypass_rxiqc_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path)
// {
// 	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1);
// 	halrf_wreg(rf, 0x813c + (path << 8), MASKDWORD, 0x40000002);
// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Bypass RXIQC\n");
// }

u16 _edpk_dgain_read_8852c(
	struct rf_info *rf)
{
	u16 dgain = 0x0;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/

	dgain = (u16)halrf_rreg(rf, 0x80fc, 0x0FFF0000);	/*[27:16]*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] DGain = 0x%x\n", dgain);

	return dgain;
}

// s8 _edpk_dgain_mapping_8852c(
// 	struct rf_info *rf,
// 	u16 dgain)
// {
// 	u16 bnd[15] = {0xbf1, 0xaa5, 0x97d, 0x875, 0x789,
// 			0x6b7, 0x5fc, 0x556, 0x4c1, 0x43d,
// 			0x3c7, 0x35e, 0x2ac, 0x262, 0x220};
// 	s8 offset = 0;

// 	if (dgain >= bnd[0])
// 		offset = 0x6;
// 	else if ((bnd[0] > dgain) && (dgain >= bnd[1]))
// 		offset = 0x6;
// 	else if ((bnd[1] > dgain) && (dgain >= bnd[2]))
// 		offset = 0x5;
// 	else if ((bnd[2] > dgain) && (dgain >= bnd[3]))
// 		offset = 0x4;
// 	else if ((bnd[3] > dgain) && (dgain >= bnd[4]))
// 		offset = 0x3;
// 	else if ((bnd[4] > dgain) && (dgain >= bnd[5]))
// 		offset = 0x2;
// 	else if ((bnd[5] > dgain) && (dgain >= bnd[6]))
// 		offset = 0x1;
// 	else if ((bnd[6] > dgain) && (dgain >= bnd[7]))
// 		offset = 0x0;
// 	else if ((bnd[7] > dgain) && (dgain >= bnd[8]))
// 		offset = 0xff;
// 	else if ((bnd[8] > dgain) && (dgain >= bnd[9]))
// 		offset = 0xfe;
// 	else if ((bnd[9] > dgain) && (dgain >= bnd[10]))
// 		offset = 0xfd;
// 	else if ((bnd[10] > dgain) && (dgain >= bnd[11]))
// 		offset = 0xfc;
// 	else if ((bnd[11] > dgain) && (dgain >= bnd[12]))
// 		offset = 0xfb;
// 	else if ((bnd[12] > dgain) && (dgain >= bnd[13]))
// 		offset = 0xfa;
// 	else if ((bnd[13] > dgain) && (dgain >= bnd[14]))
// 		offset = 0xf9;
// 	else if (bnd[14] > dgain)
// 		offset = 0xf8;
// 	else
// 		offset = 0x0;

// 	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] DGain offset = %d\n", offset);

// 	return offset;
// }

// u8 _edpk_pas_check_8852c(
// 	struct rf_info *rf)
// {
// 	u8 fail = 0;

// 	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
// 	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
// 	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);

// 	halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00); /*0x80C3*/
// 	if (halrf_rreg(rf, 0x80fc, MASKHWORD) == 0x0800) {
// 		fail = 1;
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] PAS check Fail!!\n");
// 	}
	
// 	return fail;
// }

u8 _edpk_gainloss_read_8852c(
	struct rf_info *rf)
{
	u8 result;

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x6);	/*rpt_sel*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x1);		/*query status*/

	result = (u8)halrf_rreg(rf, 0x80fc, 0x000000F0); /*[7:4]*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x81ac = 0x%x\n", halrf_rreg(rf, 0x81ac, MASKDWORD));

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] tmp GL = %d\n", result);

	return result;
}

u8 _edpk_gainloss_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	
	halrf_wreg(rf, 0x81bc + (path << 8), 0x00800000, 0x1); //[23]eFEM_en, path A

	if (path == RF_PATH_A)
		halrf_wreg(rf, 0x81ac, MASKDWORD, 0x3fa20400); // for 1 section, DPD comp to 18 dBm for linear
	else 
		halrf_wreg(rf, 0x82ac, MASKDWORD, 0x3fca0400);

	_edpk_one_shot_8852c(rf, phy, path, D_GAIN_LOSS);

	//TBD ??
	//_edpk_kip_set_txagc_8852c(rf, phy, path, 0xff, false); /*set new tx power*/

	/*for k3 hw_bug*/
	halrf_wreg(rf, 0x81f0 + (path << 8), 0x0003ffff, 0x0f078);
	halrf_wreg(rf, 0x81f0 + (path << 8), 0xf0000000, 0x0);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] _edpk_gainloss_8852c 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
//	halrf_wreg(rf, 0x81f0 + (path << 8), BIT(23), 0x0080f078);

	return _edpk_gainloss_read_8852c(rf);
}

u8 _edpk_pas_read_8852c(
	struct rf_info *rf,
	u8 is_check)
{
	u8 i;
	u32 val1_i = 0, val1_q = 0, val2_i = 0, val2_q = 0;

	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);

	if (is_check) {
		halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00);
		val1_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val1_i >= 0x800)
			val1_i = 0x1000 - val1_i;
		val1_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val1_q >= 0x800)
			val1_q = 0x1000 - val1_q;
		halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x1f);
		val2_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
		if (val2_i >= 0x800)
			val2_i = 0x1000 - val2_i;
		val2_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
		if (val2_q >= 0x800)
			val2_q = 0x1000 - val2_q;

		if ((val2_i * val2_i + val2_q * val2_q) != 0) /*to avoid BSOD issue*/
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] PAS_delta = 0x%x\n",
				(val1_i * val1_i + val1_q * val1_q) / 
				(val2_i * val2_i + val2_q * val2_q));
	} else {
		for (i = 0; i < 32; i++) {
			halrf_wreg(rf, 0x80c0, MASKBYTE3, i); /*0x80C3*/
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] PAS_Read[%02d]= 0x%08x\n", i,
				   halrf_rreg(rf, 0x80fc, MASKDWORD));
		}
	}

	if ((val1_i * val1_i + val1_q * val1_q) < (val2_i * val2_i + val2_q * val2_q))
		return 2;
	else if ((val1_i * val1_i + val1_q * val1_q) >= ((val2_i * val2_i + val2_q * val2_q) * 8 / 5))
		return 1;
	else
		return 0;
}

u8 _edpk_agc_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u8 init_xdbm,
	u8 loss_only)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 i = 0, tmp_dbm = init_xdbm, tmp_gl_idx = 0;
	u8 tmp_rxbb = 0;
	u8 goout = 0, agc_cnt = 0;
	//s8 offset = 0;
	u16 dgain = 0;
	bool is_fail = false;

	s8 rxbb_diff = 0;
	s32 rxbb_base[2] = {0xC, 0x8};
	enum rf_path rx_path;

	if (path == RF_PATH_A)
		rx_path = RF_PATH_B;
	else 
		rx_path = RF_PATH_A;
	
	
	do {
		switch (i) {
		case 0: /*SYNC and Dgain*/
#if 0 //ifem
			is_fail = _edpk_kip_set_rxagc_8852c(rf, phy, path, kidx);
#else //efem
			is_fail = _edpk_kip_set_sync_8852c(rf, phy, path, kidx, rxbb_base[rx_path]); //5
#endif

			RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d is_fail=%d\n", path, is_fail);
			dgain = _edpk_dgain_read_8852c(rf);

			if (EDPK_RXSRAM_DBG_8852C)
				_edpk_read_rxsram_8852c(rf);

			if (is_fail) {
				goout = 1;
				break;
			}

			if (agc_cnt == 0) {
				if (dgain > dpk->edpk_dgain_thrs) {
					RF_DBG(rf, DBG_RF_DPK, "[EDPK] Dgain (0x%x) > threshold(0x%x), return, SKIP EDPK!!\n", dgain, dpk->edpk_dgain_thrs);
					is_fail = true;
					goout = 1;
					break;
				}
			}

			if (dgain < 0x262) {
				rxbb_diff = -8;				
				rxbb_base[rx_path] = rxbb_base[rx_path] - 8;			
			} else if ((dgain >= 0x262) && (dgain < 0x2ac)) {
				rxbb_diff = -7;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 7;
			} else if ((dgain >= 0x2ac) && (dgain < 0x35e)) {
				rxbb_diff = -6;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 6;
			} else if ((dgain >= 0x35e) && (dgain < 0x3c7)) {
				rxbb_diff = -5;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 5;
			} else if ((dgain >= 0x3c7) && (dgain < 0x43d)) {
				rxbb_diff = -4;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 4;
			} else if ((dgain >= 0x43d) && (dgain < 0x4c1)) {
				rxbb_diff = -3;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 3;
			} else if ((dgain >= 0x4c1) && (dgain < 0x556)) {
				rxbb_diff = -2;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 2;
			} else if ((dgain >= 0x556) && (dgain < 0x5fc)) {
				rxbb_diff = -1;
				rxbb_base[rx_path] = rxbb_base[rx_path] - 1;
			} else if ((dgain >= 0x5fc) && (dgain < 0x6b7)) {
				rxbb_diff = 0;
			} else if ((dgain >= 0x6b7) && (dgain < 0x789)) {
				rxbb_diff = 1;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 1;
			} else if ((dgain >= 0x789) && (dgain < 0x875)) {
				rxbb_diff = 2;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 2;
			} else if ((dgain >= 0x875) && (dgain < 0x97d)) {
				rxbb_diff = 3;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 3;
			} else if ((dgain >= 0x97d) && (dgain < 0xaa5)) {
				rxbb_diff = 4;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 4;
			} else if ((dgain >= 0xaa5) && (dgain < 0xbf1)) {
				rxbb_diff = 5;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 5;
			} else if ((dgain >= 0xbf1)) {
				rxbb_diff = 6;
				rxbb_base[rx_path] = rxbb_base[rx_path] + 6;
			}

			if(rxbb_base[rx_path] > 0x1f)
				rxbb_base[rx_path] = 0x1f;
			if(rxbb_base[rx_path] < 0)
				rxbb_base[rx_path] = 0;

			RF_DBG(rf, DBG_RF_DPK, "[EDPK] dgain = 0x%x,RXBB = 0x%x RxBB diff = %d\n", dgain, rxbb_base[rx_path], rxbb_diff);


			if (dgain > 0x5fc || dgain < 0x556) {			
				//_edpk_set_rxbb(rf, rx_path, tmp, 0);
				_edpk_kip_control_rfc_8852c(rf, path, false);
				_edpk_set_rxbb(rf, path, rxbb_base[rx_path]);
				_edpk_kip_control_rfc_8852c(rf, path, true);
				_edpk_one_shot_8852c(rf, phy, path, D_SYNC);
				dgain = _edpk_dgain_read_8852c(rf);

				RF_DBG(rf, DBG_RF_DPK, "[EDPK]After Re-k dgain = 0x%x\n", dgain);
			}


			//2022/12/02 TBD
			if (agc_cnt == 0) {
			#if 0 //ifem	
				if(dpk->bp[path][kidx].band == BAND_ON_24G)
					_edpk_bypass_rxiqc_8852c(rf, path);
				else
			#endif
					_edpk_lbk_rxiqk_8852c(rf, phy, path);
			}
			i = 1;
			break;

		case 1: /*GAIN_LOSS and idx*/
			tmp_gl_idx = _edpk_gainloss_8852c(rf, phy, path, kidx);
			// _edpk_pas_read_8852c(rf, false);
 
			//???
			if ((_edpk_pas_read_8852c(rf, true) == 2) && (tmp_gl_idx > 0))
				i = 3;
			else if ((tmp_gl_idx == 0 && _edpk_pas_read_8852c(rf, true) == 1) || tmp_gl_idx >= 7)
				i = 2; /*GL > criterion*/
			else if (tmp_gl_idx == 0)
				i = 3; /*GL < criterion*/
			else 
				i = 4;
			break;

		case 2: /*GL > criterion*/
			if (tmp_dbm <= 7) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] Txagc@lower bound!!\n");
			} else {
				if (tmp_dbm - 3 <= 7)
					tmp_dbm = 7;
				else
				tmp_dbm = tmp_dbm - 3;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 Txagc tmp_dbm=%d\n", tmp_dbm);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 before 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
				if(path == RF_PATH_A)
					halrf_wreg(rf, 0x81f0, BIT(23), 0x0);
				else 
					halrf_wreg(rf, 0x82f0, BIT(23), 0x0);

				_edpk_kip_control_rfc_8852c(rf, path, false);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 before path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 before path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 before path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 before path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
				_edpk_kip_control_rfc_8852c(rf, path, true);

				_edpk_kip_set_txagc_8852c(rf, phy, path, tmp_dbm, true);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 after 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
				_edpk_kip_control_rfc_8852c(rf, path, false);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 after path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 after path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 after path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case2 after path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
				_edpk_kip_control_rfc_8852c(rf, path, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 3:	/*GL < criterion*/
			if (tmp_dbm >= 24) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] Txagc@upper bound!!\n");
			} else {
				if (tmp_dbm + 2 >= 24)
					tmp_dbm = 24;
				else
				tmp_dbm = tmp_dbm + 2;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 Txagc tmp_dbm=%d\n", tmp_dbm);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 before 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
				if(path == RF_PATH_A)
					halrf_wreg(rf, 0x81f0, BIT(23), 0x0);
				else 
					halrf_wreg(rf, 0x82f0, BIT(23), 0x0);

				_edpk_kip_control_rfc_8852c(rf, path, false);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 before path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 before path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 before path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 before path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
				_edpk_kip_control_rfc_8852c(rf, path, true);

				_edpk_kip_set_txagc_8852c(rf, phy, path, tmp_dbm, true);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 after 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
				_edpk_kip_control_rfc_8852c(rf, path, false);
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 after path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 after path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 after path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] case3 after path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
				_edpk_kip_control_rfc_8852c(rf, path, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 4:
			if ((rf->phl_com->dev_cap.rfe_type == 5) && (dpk->bp[path][kidx].band != BAND_ON_24G))
				_edpk_kip_set_txagc_8852c(rf, phy, path, tmp_dbm - tmp_gl_idx - 3, true);

			RF_DBG(rf, DBG_RF_DPK, "[EDPK] case 4\n");

			RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x5670=%x 0x7670=%x\n", halrf_rreg(rf, 0x5670, MASKDWORD), halrf_rreg(rf, 0x7670, MASKDWORD));

			_edpk_one_shot_8852c(rf, phy, path, D_TXAGC);// 8 again
			_edpk_kset_query_8852c(rf, path);// 8 again
		
			_edpk_kip_control_rfc_8852c(rf, path, false);
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));

			tmp_rxbb = (u8)halrf_rrf(rf, rx_path, 0x00, MASKRFRXBB);
			if (tmp_rxbb + tmp_gl_idx > 0x1f)
				tmp_rxbb = 0x1f;
			 else
				tmp_rxbb = tmp_rxbb + tmp_gl_idx;

			halrf_wrf(rf, rx_path, 0x00, MASKRFRXBB, tmp_rxbb);
			
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Adjust RXBB (%+d) = 0x%x\n",
				tmp_gl_idx, tmp_rxbb);
			_edpk_kip_control_rfc_8852c(rf, path, true);
			goout = 1;
			break;
		default:
			goout = 1;
			break;
		}	
	} while (!goout && (agc_cnt < 6));

	return is_fail;
}

void _edpk_set_mdpd_para_8852c(
	struct rf_info *rf,
	u8 order)
{
	switch (order) {
	case 0: /*(5,3,1)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x0);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x2);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x4); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x1); /*[30:28] tx_delay_man*/
		break;

	case 1: /*(5,3,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x1);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x1);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		break;

	case 2: /*(5,0,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x2);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x0);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		break;

	case 3: /*(7,3,1)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x3);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x3);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x4); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x1); /*[30:28] tx_delay_man*/
		break;
	default:
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Wrong MDPD order!!(0x%x)\n", order);
		break;
	}

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Set %s for IDL\n", order == 0x0 ? "(5,3,1)" :
		(order == 0x1 ? "(5,3,0)" : (order == 0x2 ? "(5,0,0)" : "(7,3,1)")));
}

// void _edpk_coef_read_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	u8 kidx)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u32 reg, reg_start, reg_stop;

// 	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

// 	reg_start = 0xa500 + kidx * 0x1c0 + path * 0x400 + dpk->cur_k_set * 0x70;

// 	reg_stop = reg_start + 0x70;

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] ===== [Coef of S%d_CH%d_K%d] =====\n",
// 		path, kidx, dpk->cur_k_set);

// 	for (reg = reg_start; reg < reg_stop ; reg += 4) {
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK][coef_r] 0x%x = 0x%08x\n", reg,
// 			   halrf_rreg(rf, reg, MASKDWORD));
// 	}
// 	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);
// }

void _edpk_idl_mpa_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 cnt = 0;
	u16 cnt1;

	halrf_wreg(rf, 0x80a0, BIT(16), 0x1); /*Phase Normalize enable*/

	if (dpk->bp[path][kidx].bw == CHANNEL_WIDTH_160)
		_edpk_set_mdpd_para_8852c(rf, 0x0);	/*force 5,3,1*/
	else 
		_edpk_set_mdpd_para_8852c(rf, 0x2); /*5,0,0*/
#if 0
	if (halrf_rreg(rf, 0x80ec, BIT(8)) == 0x1)
		_edpk_set_mdpd_para_8852c(rf, 0x2); /*5,0,0*/
	else if (halrf_rreg(rf, 0x80ec, BIT(9)) == 0x1)
		_edpk_set_mdpd_para_8852c(rf, 0x1); /*5,3,0*/
	else
	_edpk_set_mdpd_para_8852c(rf, 0x0); /*5,3,1*/
#endif
	halrf_wreg(rf, 0x809c, BIT(8), 0x0);

	for (cnt1 = 0; cnt1 < 1000; cnt1++)
		halrf_delay_us(rf, 1);

	_edpk_one_shot_8852c(rf, phy, path, D_MDPK_IDL);

//---TBD ??
	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/
	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] timing_sync = 0x%x\n", halrf_rreg(rf, 0x80fc, MASKDWORD));
	dpk->dpk_sync[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);

	/*wa for ov issue*/
	halrf_wreg(rf, 0x80d4, 0x003F0000, 0xf);	/*rpt_sel*/
	dpk->ov_flag[path] = (u8)halrf_rreg(rf, 0x80fc, BIT(30));
	while (dpk->ov_flag[path] == 0x1 && cnt < 5) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] ReK due to MEDPK ov!!!\n");
		_edpk_one_shot_8852c(rf, phy, path, D_MDPK_IDL);
		halrf_wreg(rf, 0x80d4, 0x003F0000, 0xf);	/*rpt_sel*/
		dpk->ov_flag[path] = (u8)halrf_rreg(rf, 0x80fc, BIT(30));
		cnt++;
		dpk->rek_cnt[path][0]++;
	}

	if (dpk->ov_flag[path]) {
		_edpk_set_mdpd_para_8852c(rf, 0x2);	/*5,0,0*/
		_edpk_one_shot_8852c(rf, phy, path, D_MDPK_IDL);
	}

	//_edpk_coef_read_8852c(rf, path, kidx);
}

u8 _edpk_order_convert_8852c(
	struct rf_info *rf)
{
	u8 val;

	switch (halrf_rreg(rf, 0x80a0, BIT(1) | BIT(0))) {
	case 0: /*(5,3,1)*/
		val = 0x6;
		break;

	case 1: /*(5,3,0)*/
		val = 0x2;
		break;

	case 2: /*(5,0,0)*/
		val = 0x0;
		break;

	case 3: /*(7,3,1)*/
		val = 0x7;
		break;
	default:
		val = 0xff;
		break;
	}
		
	/*0x80a0 [1:0] = 0x0 => 0x81bc[26:25] = 0x6   //(5,3,1)*/
	/*0x80a0 [1:0] = 0x1 => 0x81bc[26:25] = 0x2   //(5,3,0)*/
	/*0x80a0 [1:0] = 0x2 => 0x81bc[26:25] = 0x0   //(5,0,0)*/
	/*0x80a0 [1:0] = 0x3 => 0x81bc[26:25] = 0x7   //(7,3,1)*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] convert MDPD order to 0x%x\n", val);

	return val;
}

// void _edpk_gs_defalut_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	u8 kidx)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
// 			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

// 	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);
// #if 0
// 	/*CH0*/
// 	halrf_wreg(rf, 0x8190 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
// 	halrf_wreg(rf, 0x8194 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
// 	halrf_wreg(rf, 0x8198 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
// 	halrf_wreg(rf, 0x81a4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
// 	/*CH1*/
// 	halrf_wreg(rf, 0x81a8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
// 	halrf_wreg(rf, 0x81c4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
// 	halrf_wreg(rf, 0x81c8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
// 	halrf_wreg(rf, 0x81e8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
// #endif
// }

void _edpk_gain_normalize_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	bool is_execute)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

	if (is_execute) {
		halrf_wreg(rf, 0x819c + (path << 8), 0x000003FF, 0x200); /*[9:0] pow_cal_start*/
		halrf_wreg(rf, 0x819c + (path << 8), BIT(17) | BIT(16), 0x3); /*pow_cal_len*/

		_edpk_one_shot_8852c(rf, phy, path, D_GAIN_NORM);
	} else
		halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);

	dpk->bp[path][kidx].gs = (u8)halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]======> %s reg_0x%x[6:0] = 0x%x\n", __func__, reg[kidx][dpk->cur_k_set], dpk->bp[path][kidx].gs);
#if 0
	/*CH0*/
	halrf_wreg(rf, 0x8190 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x8194 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x8198 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81a4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
	/*CH1*/
	halrf_wreg(rf, 0x81a8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K0*/
	halrf_wreg(rf, 0x81c4 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K1*/
	halrf_wreg(rf, 0x81c8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K2*/
	halrf_wreg(rf, 0x81e8 + (path << 8), 0x0000007F, 0x5b); /*[6:0], K3*/
#endif
}

void _edpk_on_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

	/*coef load*/
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x1);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x0);

	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
			BIT(26) | BIT(25) | BIT(24), _edpk_order_convert_8852c(rf));

	dpk->bp[path][kidx].path_ok = dpk->bp[path][kidx].path_ok | BIT(dpk->cur_k_set);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d] path_ok = 0x%x\n", path, kidx,
		dpk->bp[path][kidx].path_ok);

#if 0 //ifem
	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000,
			dpk->bp[path][kidx].path_ok);
#else //efem
	halrf_wreg(rf, 0x81bc + (path << 8), 0x80000000, 0x1);

	// pwsf workaround for circuit error
	halrf_wreg(rf, 0x81f0 + (path << 8), 0x003ffff, 0x0000f078);
	halrf_wreg(rf, 0x81f0 + (path << 8), 0xf0000000, 0x0);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] _edpk_on_8852c 0x81f0 = %x 0x82f0 = %x\n", halrf_rreg(rf, 0x81f0, MASKDWORD), halrf_rreg(rf, 0x82f0, MASKDWORD));
#endif 

	_edpk_gain_normalize_8852c(rf, phy, path, kidx, false);
	//_edpk_para_query_8852c(rf, path, kidx);

	if ((rf->phl_com->dev_cap.rfe_type == 5) && (dpk->bp[path][kidx].band != BAND_ON_24G)) {
		dpk->bp[path][kidx].txagc_dpk = dpk->bp[path][kidx].txagc_dpk - 3;
		halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000FC00, dpk->bp[path][kidx].txagc_dpk); /*[15:10]*/
	}
	
}

// bool _edpk_reload_check_8852c(
// 	struct rf_info *rf,
// 	enum phl_phy_idx phy,
// 	enum rf_path path)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	bool is_reload = false;
// 	u8 idx, cur_band, cur_ch;

// 	cur_band = rf->hal_com->band[phy].cur_chandef.band;
// 	cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

// 	for (idx = 0; idx < DPK_BKUP_NUM; idx++) {
// 		if ((cur_band == dpk->bp[path][idx].band) && (cur_ch == dpk->bp[path][idx].ch)) {
// 			halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), idx);
// 			dpk->cur_idx[path] = idx;
// 			is_reload = true;
// 			RF_DBG(rf, DBG_RF_DPK, "[EDPK] reload S%d[%d] success\n", path, idx);
// 		}
// 	}

// 	return is_reload;
// }

// bool _edpk_main_8852c(
// 	struct rf_info *rf,
// 	enum phl_phy_idx phy,
// 	enum rf_path path)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u8 init_xdbm = 15;
// 	u8 kidx = dpk->cur_idx[path];
// 	bool is_fail = false;

// 	_edpk_kip_control_rfc_8852c(rf, path, false);
// 	halrf_rf_direct_cntrl_8852c(rf, path, false); /*switch control to direct write*/
// 	halrf_wrf(rf, path, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/

// 	_edpk_rf_setting_8852c(rf, path, kidx); //01_RF_for_DPK_v1_reg.txt
// 	halrf_set_rx_dck_8852c(rf, phy, path, false); //TBD !!! edpk_set_rx_dck,  //02_RF_RxDCK_S1_20210818_reg.txt


// 	_edpk_kip_pwr_clk_onoff_8852c(rf, true); //03_CIP_for_DPK_nctl_v1_reg.txt
// 	_edpk_kip_preset_8852c(rf, phy, path, kidx); //03_CIP_for_DPK_nctl_v1_reg.txt

	
// 	_edpk_txpwr_bb_force_8852c(rf, path, true);//4
// 	_edpk_kip_set_txagc_8852c(rf, phy, path, init_xdbm, true);//4
// 	_edpk_tpg_sel_8852c(rf, path, kidx);//5
// #if 0
// 	is_fail = _edpk_kip_set_rxagc_8852c(rf, phy, path, kidx);

// 	if (EDPK_RXSRAM_DBG)
// 		_edpk_read_rxsram_8852c(rf);

// 	if (is_fail)
// 		goto _error;

// 	_edpk_dgain_read_8852c(rf);
// 	_edpk_bypass_rxiqc_8852c(rf, path);
// 	_edpk_gainloss_8852c(rf, phy, path, kidx);
// 	_edpk_para_query_8852c(rf, path, kidx);
// #else
// 	is_fail = _edpk_agc_8852c(rf, phy, path, kidx, init_xdbm, false);

// 	if (is_fail)
// 		goto _error;
// #endif
// 	/*_edpk_pas_read_8852c(rf, false);*/
// 	//_edpk_get_thermal_8852c(rf, kidx, path);

// 	_edpk_idl_mpa_8852c(rf, phy, path, kidx);
// 	_edpk_para_query_8852c(rf, path, kidx);

// 	//_edpk_kip_control_rfc_8852c(rf, path, false);
// 	//_edpk_get_thermal_8852c(rf, kidx, path);
// #if 0
// 	_edpk_coef_read_8852c(rf, path, kidx, gain);
// #endif
// 	_edpk_on_8852c(rf, phy, path, kidx); //10_Mdpd_On_K3_v1.txt
// _error:
// 	_edpk_kip_control_rfc_8852c(rf, path, false);
// 	halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d]_K%d %s\n", path, kidx,
// 		dpk->cur_k_set, is_fail ? "need Check" : "is Success");

// 	dpk->dpk_cal_cnt++;

// 	if(!is_fail)
// 		dpk->dpk_ok_cnt++;

// 	return is_fail;
// }

// void _edpk_cal_select_8852c(
// 	struct rf_info *rf,
// 	bool force,
// 	enum phl_phy_idx phy,
// 	u8 kpath)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;

// 	u32 kip_bkup[EDPK_RF_PATH_MAX_8852C][EDPK_KIP_REG_NUM_8852C] = {{0}};
// 	//u32 bb_bkup[EDPK_BB_REG_NUM_8852C] = {0};
// 	u32 rf_bkup[EDPK_RF_PATH_MAX_8852C][EDPK_RF_REG_NUM_8852C] = {{0}};

	// u32 kip_reg[] = {0x813c, 0x8124, 0x8120, 0xc0c4, 0xc0e8, 0xc0d4, 0xc0d8};
	// u32 bb_reg[] = {0x2344, 0x5800, 0x7800};
	// u32 rf_reg[EDPK_RF_REG_NUM_8852C] = {0xdf, 0x5f, 0x8f, 0x97, 0xa3, 0x5, 0x10005};

// 	u8 path;
// 	bool is_fail = true;
// 	bool reloaded[EDPK_RF_PATH_MAX_8852C] = {false};

// 	if ((!phl_is_mp_mode(rf->phl_com)) && EDPK_RELOAD_EN_8852C) {
// 		for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
// 			reloaded[path] = _edpk_reload_check_8852c(rf, phy, path);
// 			if ((reloaded[path] == false) && (dpk->bp[path][0].ch != 0))
// 				dpk->cur_idx[path] = !dpk->cur_idx[path];
// 			else
// 				halrf_edpk_onoff_8852c(rf, path, false);
// 		}
// 	} else {
// 		for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++)
// 			dpk->cur_idx[path] = 0;
// 	}

// 	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
// 		if (kpath & BIT(path)) {
// 			_edpk_bkup_kip_8852c(rf, kip_reg, kip_bkup, path);
// 			_edpk_bkup_rf_8852c(rf, rf_reg, rf_bkup, path);
// 			dpk->cbw_idx[path] = (u8) halrf_rreg(rf, 0xc0ec | (path << 8), 0x00006000);
// 			_edpk_information_8852c(rf, phy, path);
// 			_edpk_init_8852c(rf, path);
// 			if (rf->is_tssi_mode[path])
// 				_edpk_tssi_pause_8852c(rf, path, true);
// 		}
// 	}

// 	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
// 		if (kpath & BIT(path)) {
// 			RF_DBG(rf, DBG_RF_DPK, "[EDPK] ========= S%d[%d] EDPK Start =========\n", path, dpk->cur_idx[path]);
// 			_edpk_rxagc_onoff_8852c(rf, path, false);
// 			halrf_drf_direct_cntrl_8852c(rf, path, false);
// 			_edpk_bb_afe_setting_8852c(rf, path); // only one time //00_BB_AFE_DPK_s01_v1.txt
// 			is_fail = _edpk_main_8852c(rf, phy, path);
// 			halrf_edpk_onoff_8852c(rf, path, is_fail);
// 		}
// 	}

// 	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
// 		if (kpath & BIT(path)) {
// 			_edpk_kip_restore_8852c(rf, phy, path); //R0
// 			_edpk_reload_kip_8852c(rf, kip_reg, kip_bkup, path);
// 			_edpk_reload_rf_8852c(rf, rf_reg, rf_bkup, path);
// 			_edpk_bb_afe_restore_8852c(rf, path); //R2
// 			_edpk_rxagc_onoff_8852c(rf, path, true);
// 			if (rf->is_tssi_mode[path])
// 				_edpk_tssi_pause_8852c(rf, path, false);
// 		}
// 	}
// 	_edpk_kip_pwr_clk_onoff_8852c(rf, false);
// 	_edpk_tx_on_retore_8852c(rf); //R9
// }

// u8 _edpk_bypass_check_8852c(
// 	struct rf_info *rf,
// 	enum phl_phy_idx phy)
// {
// 	struct halrf_fem_info *fem = &rf->fem;

// 	u8 result;

// 	if (rf->hal_com->cv == CAV && (rf->hal_com->band[phy].cur_chandef.band != BAND_ON_24G)) {
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip EDPK due to CAV & not 2G!!\n");
// 		result = 1;
// 	} else if (fem->epa_2g && (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_24G)) {
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip EDPK due to 2G_ext_PA exist!!\n");
// 		result = 1;
// 	} else if (fem->epa_5g && (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_5G)) {
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip EDPK due to 5G_ext_PA exist!!\n");
// 		result = 1;
// 	} else if (fem->epa_6g && (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_6G)) {
// 		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip EDPK due to 6G_ext_PA exist!!\n");
// 		result = 1;
// 	} else if (rf->phl_com->dev_cap.rfe_type > 50) {
// 		result = 1;
// 	} else
// 		result = 0;

// 	return result;
// }

// void _edpk_force_bypass_8852c(
// 	struct rf_info *rf,
// 	enum phl_phy_idx phy)
// {
// 	u8 path, kpath;

// 	kpath = halrf_kpath_8852c(rf, phy);

// 	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
// 		if (kpath & BIT(path))
// 			halrf_edpk_onoff_8852c(rf, path, true);
// 	}
// }

void _edpk_backup_bb_reg_8852c(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_bkup,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
		
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup BB 0x%x = %x\n", reg[i], reg_bkup[i]);
	}
}

void _edpk_bkup_kip_8852c(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][EDPK_KIP_REG_NUM_8852C],
	u8 path)
{
	u8 i;

	for (i = 0; i < EDPK_KIP_REG_NUM_8852C; i++) {
		reg_bkup[path][i] = halrf_rreg(rf, reg[i] + (path << 8), MASKDWORD);
		// if (EDPK_REG_DBG_8852C)
		// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup 0x%x = %x\n", reg[i]+ (path << 8), reg_bkup[path][i]);
	}
}

void _edpk_bkup_rf_8852c(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][EDPK_RF_REG_NUM_8852C],
	u8 path)
{
	u8 i;

	for (i = 0; i < EDPK_RF_REG_NUM_8852C; i++) {
		rf_bkup[path][i] = halrf_rrf(rf, path, rf_reg[i], MASKRF);
		// if (EDPK_REG_DBG_8852C)
		// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup RF S%d 0x%x = %x\n",
		// 		path, rf_reg[i], rf_bkup[path][i]);
	}
}

void _edpk_reload_bb_reg_8852c(
	struct rf_info *rf,
	u32 *reg,
	u32 *reg_backup,
	u8 reg_num)
{
	u8 i;

	for (i = 0; i < reg_num; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_backup[i]);

		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload BB 0x%x = 0x%x\n",
		       reg[i], reg_backup[i]);
	}
}

u8 _edpk_ch_to_idx_8852c(struct rf_info *rf, u8 channel)
{
	u8	channelIndex;

	if (channel >= 1 && channel <= 14)
		channelIndex = channel - 1;
	else if (channel >= 36 && channel <= 64)
		channelIndex = (channel - 36) / 2 + 14;
	else if (channel >= 100 && channel <= 144)
		channelIndex = ((channel - 100) / 2) + 15 + 14;
	else if (channel >= 149 && channel <= 177)
		channelIndex = ((channel - 149) / 2) + 38 + 14;
	else
		channelIndex = 0;

	return channelIndex;
}

void _edpk_hw_tx_8852c(struct rf_info *rf,
			enum phl_phy_idx phy, u8 path, u16 cnt, u16 period, s16 dbm, u32 rate, u8 bw,
			bool enable)
{
	struct rf_pmac_tx_info tx_info = {0};

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	tx_info.ppdu = rate;
	tx_info.mcs = 7;
	tx_info.bw = bw;
	tx_info.nss = 1;
	tx_info.gi = 1;
	tx_info.txagc_cw = 0;
	tx_info.dbm = dbm;
	tx_info.cnt = cnt;
	tx_info.time = 20;
	tx_info.period = period;
	tx_info.length = 0;

	halrf_set_pmac_tx(rf, phy, path, &tx_info, enable, false);
}

void halrf_edpk_pre_tx_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	u16 cnt, u16 period, s16 pwr, u32 delay) 
{
	s8 txagc_base;
	u8 ther_base;
	u8 i;

#if 0
	s8 shift[2] = {-6, 4};
#else
	s8 shift[2] = {2, 2};
#endif

	u32 bb_bkup[EDPK_PRE_TX_BB_REG_NUM_8852C] = {0};
	u32 bb_reg[] = {0x49c0, 0x0d18, 0x0d80, 0x58e8, 0x78e8};
	bool tssi_status = rf->is_tssi_mode[RF_PATH_A];

	enum rf_path path = RF_PATH_A;

	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 kidx = dpk->cur_idx[path];
	//u8 kidx = 0;

	u8 channel = rf->hal_com->band[phy].cur_chandef.center_ch;
//	u8 bw = rf->hal_com->band[phy].cur_chandef.bw;
//	s8 power, power_tmp;
	//s16 xdbm;
	u8 ch_idx = _edpk_ch_to_idx_8852c(rf, channel);
	(void) ch_idx;
//	u8 channel_tmp;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	RF_DBG(rf, DBG_RF_DPK,"[EDPK]%s ================[S%d[%d] (dpk CH %d)]================\n",
			__func__, path, kidx, dpk->bp[path][kidx].ch);
	RF_DBG(rf, DBG_RF_DPK,"[EDPK]ch = %d\n", channel);

	RF_DBG(rf, DBG_RF_DPK,"[EDPK]Before BB bkup,Txpath_0x458c[31:28] = %d, Rxpath_0x49c4[3:0] = %d \n",halrf_rreg(rf, 0x458c, 0xf0000000), halrf_rreg(rf, 0x49c4, 0xf));

	RF_DBG(rf, DBG_RF_DPK,"[EDP]Record TSSI stats: %d\n", tssi_status);

	halrf_hal_bb_backup_info(rf, phy);	//BB Backup
//	_edpk_bkup_bb_8192xb(rf, bb_reg, bb_bkup); //other BB backup
	_edpk_backup_bb_reg_8852c(rf, bb_reg, bb_bkup, EDPK_PRE_TX_BB_REG_NUM_8852C);

/*
	if (bw == CHANNEL_WIDTH_40)
		channel_tmp = channel - 2;
	else if (bw == CHANNEL_WIDTH_80)
		channel_tmp = channel - 6;
	else
		channel_tmp = channel;
*/	
	if (cnt == 0)
		cnt = 4096;
	if (period == 0)
		period = 500;
	if (pwr == 0)
		pwr = 20*4 ;//dbm

	if (delay == 0)
		delay = 15;


	RF_DBG(rf, DBG_RF_DPK,"[EDPK] cnt =%d, period = %d, pwr = %d (/0.25 dbm), delay = %d ms\n", 
		cnt, period, pwr, delay);

	halrf_inn_tssi_enable(rf, phy);
	halrf_inn_tssi_set_efuse_to_de(rf, phy);
	
	//5820/7820 [31] to make sure tssi should be on or off after preTx
	RF_DBG(rf, DBG_RF_DPK,"[EDPK] check tssi, 0x5818[28] = %d, 0x5820[31:30] = %d \n", halrf_rreg(rf, 0x5818, 0x10000000), halrf_rreg(rf, 0x5820, 0xC0000000));
	RF_DBG(rf, DBG_RF_DPK,"[EDPK] check tssi, 0x7818[28] = %d, 0x7820[31:30] = %d \n", halrf_rreg(rf, 0x7818, 0x10000000), halrf_rreg(rf, 0x7820, 0xC0000000));

#if 0
	halrf_wreg(rf, 0x58e8, 0x0000003f, 0x00);
	halrf_wreg(rf, 0x78e8, 0x0000003f, 0x00);
#endif

	_edpk_hw_tx_8852c(rf, phy, RF_PATH_AB, cnt, period, pwr, T_HT_MF, 0, true);	//TX Strat

	halrf_delay_us(rf, delay*1000);
	
	_edpk_hw_tx_8852c(rf, phy, RF_PATH_AB, cnt, period, pwr, T_HT_MF, 0, false);	//TX stop

	for (i = 0; i < 2; i++) {
		path = i;
		
		//txagc
		txagc_base = (s8)(halrf_rreg(rf, 0x1c60 + (path << 13), 0xFF000000) + 0x78 - dpk->pwsf_df[path]); /*[31:24]*/

		RF_DBG(rf, DBG_RF_DPK,"[EDPK_TRK] txagc_base=%x\n", txagc_base);
	
		//read thermal
		
		//ther_base = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000003F); /*[5:0]*/
		ther_base = halrf_get_thermal_8852c(rf, path);

		dpk->bp[path][kidx].txagc_trk_base_edpk = txagc_base;
		dpk->bp[path][kidx].ther_trk_base_edpk = ther_base;
	
		RF_DBG(rf, DBG_RF_DPK,"[EDPK_TRK] path_%d, ther_base = 0x%x, txagc_base = 0x%x\n",
				path, dpk->bp[path][kidx].ther_trk_base_edpk, dpk->bp[path][kidx].txagc_trk_base_edpk);

		dpk->bp[path][kidx].txagc_trk_base_edpk = txagc_base + shift[path];
		
		RF_DBG(rf, DBG_RF_DPK,"[EDPK_TRK] after shift[%d]=%d,  txagc_base = 0x%x\n",
				path, shift[path], dpk->bp[path][kidx].txagc_trk_base_edpk);
		
	}
	
	//Reload BB Reg
//	_edpk_reload_bb_8192xb(rf, bb_reg, bb_bkup);
	_edpk_reload_bb_reg_8852c(rf, bb_reg, bb_bkup, EDPK_PRE_TX_BB_REG_NUM_8852C);
	
	halrf_hal_bb_restore_info(rf, phy);

	//fix bug
	if (!phl_is_mp_mode(rf->phl_com))
		halrf_tx_mode_switch(rf, phy, 0);

	if (!tssi_status) {
		RF_DBG(rf, DBG_RF_DPK,"[EDPK] previous tssi is off, disable tssi after preTx");
		halrf_inn_tssi_disable(rf, phy);
	}

	RF_DBG(rf, DBG_RF_DPK,"[EDPK]after BB restore,Txpath_0x458c[31:28] = %d, Rxpath_0x49c4[3:0] = %d \n",halrf_rreg(rf, 0x458c, 0xf0000000), halrf_rreg(rf, 0x49c4, 0xf));
}

void halrf_edpk_8852c(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	bool force)
{
	u8 path, kidx;
	u8 init_xdbm = 20;
	bool is_fail = false;
	struct halrf_dpk_info *dpk = &rf->dpk;

	enum rf_path rx_path;
	
	u32 kip_bkup[EDPK_RF_PATH_MAX_8852C][EDPK_KIP_REG_NUM_8852C];
	u32 bb_bkup[EDPK_BB_REG_NUM_8852C];
	u32 rf_bkup[EDPK_RF_PATH_MAX_8852C][EDPK_RF_REG_NUM_8852C];

	u32 kip_reg[] = {0xc0e4, 0xc0ec, 0xc0e8, 0xc0d4, 0xc0d8, 0x813c};
	u32 bb_reg[] = {0x5890, 0x5880, 0x5884, 0x7890, 0x7880, 0x7884, 0xc0c4, 0x12a0, 0x32a0};
	u32 rf_reg[EDPK_RF_REG_NUM_8852C] = {0xdf, 0x8f, 0x02, 0xa3, 0x5, 0x10005};

	(void) force;
	// struct halrf_rx_dck_info *rx_dck = &rf->rx_dck;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] ****** EDPK Start test 0913 ******\n");

	_edpk_backup_bb_reg_8852c(rf, bb_reg, bb_bkup, EDPK_BB_REG_NUM_8852C);

	// backup 
	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
		_edpk_bkup_kip_8852c(rf, kip_reg, kip_bkup, path);
		_edpk_bkup_rf_8852c(rf, rf_reg, rf_bkup, path);
		_edpk_information_8852c(rf, phy, path); // edpk related setting

		RF_DBG(rf, DBG_RF_DPK, "[EDPK]double chk bkup value edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), dpk->bnd_dpd[path]);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]double chk bkup value pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), dpk->pwsf_df[path]);
	}	
 
	halrf_edpk_pre_tx_8852c(rf, phy, 0, 0, 0, 0); // pre-tx

	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
		if (rf->is_tssi_mode[path])
			_edpk_tssi_pause_8852c(rf, path, true);
	}
	
		_edpk_efem_control_8852c(rf, rf->phl_com->dev_cap.rfe_type); // 00c.txt

	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
		halrf_wreg(rf, 0x81ac + (path << 8), 0xfc0000, 0x2b); //bnd_dpd
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] edpk bnd_dpd 0x%x[23:18] = 0x%x \n",  0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000));
	}

	halrf_drf_direct_cntrl_8852c(rf, RF_PATH_A, false);
	halrf_rf_direct_cntrl_8852c(rf, RF_PATH_A, false); /*switch control to direct write*/
	halrf_drf_direct_cntrl_8852c(rf, RF_PATH_B, false);
	halrf_rf_direct_cntrl_8852c(rf, RF_PATH_B, false); /*switch control to direct write*/

	for(path = 0; path < EDPK_RF_PATH_MAX_8852C; path++){

		if (path == RF_PATH_A)
			rx_path = RF_PATH_B;
		else 
			rx_path = RF_PATH_A;

		kidx = dpk->cur_idx[path];

		_edpk_bb_afe_setting_8852c(rf, RF_PATH_A); //00_BB_AFE_DPK_s01_v1.txt
		_edpk_bb_afe_setting_8852c(rf, RF_PATH_B); //00_BB_AFE_DPK_s01_v1.txt
		
		halrf_wrf(rf, path, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/
		halrf_wrf(rf, rx_path, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/
		_edpk_rf_setting_8852c(rf, path, kidx); //01_RF_for_DPK_v1_reg.txt

		halrf_set_rx_dck_8852c(rf, phy, rx_path, false); //TBD !!! edpk_set_rx_dck,  //02_RF_RxDCK_S1_20210818_reg.txt
		// halrf_rx_dck_toggle_8852c(rf, path);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d rx dck trigger\n", rx_path);

		_edpk_kip_pwr_clk_onoff_8852c(rf, true); //03_CIP_for_DPK_nctl_v1_reg.txt
		_edpk_kip_preset_8852c(rf, phy, path, kidx); //03_CIP_for_DPK_nctl_v1_reg.txt
	
		_edpk_txpwr_bb_force_8852c(rf, path, true);//4

		_edpk_kip_control_rfc_8852c(rf, path, false);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 before path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 before path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 before path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 before path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
		_edpk_kip_control_rfc_8852c(rf, path, true);

		_edpk_kip_set_txagc_8852c(rf, phy, path, init_xdbm, true);//4

		_edpk_kip_control_rfc_8852c(rf, path, false);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 after path=%d 0x11=%x\n", path, halrf_rrf(rf, path, 0x11, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 after path=%d 0x11=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x11, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 after path=%d 0x10001=%x\n", path, halrf_rrf(rf, path, 0x10001, MASKRF));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 04 after path=%d 0x10001=%x\n", rx_path, halrf_rrf(rf, rx_path, 0x10001, MASKRF));
		_edpk_kip_control_rfc_8852c(rf, path, true);

		_edpk_tpg_sel_8852c(rf, path, kidx);//5

		is_fail = _edpk_agc_8852c(rf, phy, path, kidx, init_xdbm, false); //5,6,7,8

		if (is_fail)
			goto _error;

		// _edpk_kip_set_txagc_8852c(rf, phy, path, init_xdbm, true);
		// _edpk_one_shot_8852c(rf, phy, path, D_TXAGC);// 8 again
		// _edpk_kset_query_8852c(rf, path);// 8 again

		_edpk_idl_mpa_8852c(rf, phy, path, kidx); // 9

		_edpk_on_8852c(rf, phy, path, kidx); //10_Mdpd_On_K3_v1.txt

		_error:
			_edpk_kip_control_rfc_8852c(rf, path, false);
			halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_RX);

			RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d]_K%d %s\n", path, kidx,
				dpk->cur_k_set, is_fail ? "need Check" : "is Success");	
 
		// RF_DBG(rf, DBG_RF_DPK, "[EDPK]  S%d Dgain=%x\n", path, halrf_rreg(rf, 0x80fc, MASKDWORD));	
		_edpk_kip_restore_8852c(rf, phy, path); //R0
	}

	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
		#if 0 //default
		if (rf->phl_com->dev_cap.rfe_type == 53)
				halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, EDPK_PWSF_DF_TYPE53); /*[8:0]*/
		else
			halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, EDPK_PWSF_DF); /*[8:0]*/

		halrf_wreg(rf, 0x81ac + (path << 8), 0xfc0000, 0x36); //bnd_dpd
		#else //morgan 2022/08/18
		halrf_wreg(rf, 0x81ac + (path << 8), 0xfc0000, dpk->bnd_dpd[path]); //bnd_dpd
		halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, dpk->pwsf_df[path]); /*[8:0]*/
		
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]aftr restore edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]aftr restore pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), halrf_rreg(rf, 0x81f0 + (path << 8), 0x1ff));
		#endif

		RF_DBG(rf, DBG_RF_DPK, "[EDPK]after K edpk bnd_dpd 0x%x[23:18] = 0x%x \n",  0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000));
		
		_edpk_reload_rf_8852c(rf, rf_reg, rf_bkup, path); // R1
		_edpk_bb_afe_restore_8852c(rf, path); //R2
		
		if (rf->is_tssi_mode[path])
			_edpk_tssi_pause_8852c(rf, path, false);
		_edpk_reload_kip_8852c(rf, kip_reg, kip_bkup, path); //R9 and R10
			}
	_edpk_reload_bb_reg_8852c(rf, bb_reg, bb_bkup, EDPK_BB_REG_NUM_8852C); //R9 and R10
		
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] EDPK test end 0913\n");

	// RF_DBG(rf, DBG_RF_DPK, "[EDPK] ****** EDPK Start (Ver: 0x%x, Cv: %d, RF_para: %d) ******\n",
	// 	EDPK_VER_8852C, rf->hal_com->cv, halrf_get_radio_ver_from_reg(rf));

	// RF_DBG(rf, DBG_RF_DPK, "[EDPK] Driver mode = %d\n", rf->phl_com->drv_mode);

	// if (_edpk_bypass_check_8852c(rf, phy))
	// 	_edpk_force_bypass_8852c(rf, phy);
	// else
	// 	_edpk_cal_select_8852c(rf, force, phy, halrf_kpath_8852c(rf, phy));

	// if (rx_dck->is_auto_res) /*if auto DCK enabled*/
	// 	halrf_rx_dck_8852c(rf, phy, false);
}

// void halrf_edpk_onoff_8852c(
// 	struct rf_info *rf,
// 	enum rf_path path,
// 	bool off)
// {
// 	struct halrf_dpk_info *dpk = &rf->dpk;
// 	bool off_reverse;
// 	u8 val, kidx = dpk->cur_idx[path];

// 	if (off)
// 		off_reverse = false;
// 	else
// 		off_reverse = true;

// 	val = dpk->is_dpk_enable * off_reverse * dpk->bp[path][kidx].path_ok;

// 	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
// 	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000, val);

// 	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d] EDPK %s !!!\n", path, kidx,
// 		   (val == 0) ? "disable" : "enable");
// }

void halrf_edpk_config_para_8852c(struct rf_info *rf)
{
	u8 path = 0;
	struct halrf_dpk_info *dpk = &rf->dpk;
	
//	RF_DBG(rf, DBG_RF_DPK, "======> %s\n", __func__);

	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {
		dpk->bnd_dpd[path] = halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000);
		dpk->pwsf_df[path] = halrf_rreg(rf, 0x81f0 + (path << 8), 0x1ff);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]bkup edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), dpk->bnd_dpd[path]);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]bkup pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), dpk->pwsf_df[path]);
	}

	dpk->edpk_dgain_thrs = 0xA00;
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]Dgain threshold = 0x%x\n", dpk->edpk_dgain_thrs);

}

void halrf_edpk_track_8852c(
	struct rf_info *rf)
{
	s8 txagc_cur = 0, delta_txagc = 0;
	u8 ther_cur = 0;
	u8 path = 0, kidx = 0;
	u8 pwsf = 0;
	struct halrf_dpk_info *dpk = &rf->dpk;

	RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] edpk tracking test 0907======> %s\n", __func__);
	
	for (path = 0; path < EDPK_RF_PATH_MAX_8852C; path++) {

		kidx = dpk->cur_idx[path];
	
		RF_DBG(rf, DBG_RF_DPK_TRACK,
			"[EDPK_TRK]%s ================[S%d[%d] (CH %d)]================\n",
			__func__, path, kidx, dpk->bp[path][kidx].ch);

		txagc_cur = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), 0xFF000000); /*[31:24]*/

		//read thermal
		//ther_cur = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000003F); /*[5:0]*/
		ther_cur = halrf_get_thermal_8852c(rf, path);

		delta_txagc = txagc_cur - dpk->bp[path][kidx].txagc_trk_base_edpk;

		//limit
		if (delta_txagc > 48) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x and reset delta_txagc to limit 32", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc);

			delta_txagc  = 48;
			
		} else if (delta_txagc < -48) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x and reset delta_txagc to limit -32", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc);

			delta_txagc  = -48;
		}

#if 0 //default
		if (rf->phl_com->dev_cap.rfe_type == 53)
			pwsf = EDPK_PWSF_DF_TYPE53 - delta_txagc;
		else
			pwsf = EDPK_PWSF_DF - delta_txagc;
#else //morgan 2022/08/18
		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] S%d PWSF Default =0x%x",  
			path, dpk->pwsf_df[path]);

			pwsf = (u8)(dpk->pwsf_df[path] - delta_txagc);
#endif

		halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, pwsf);	 //[8:0]

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] ther_cur =0x%x, ther_dpk = 0x%x",  
			ther_cur, dpk->bp[path][kidx].ther_trk_base_edpk);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x, pwsf = 0x%x", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc, pwsf);
	}
}

#endif
