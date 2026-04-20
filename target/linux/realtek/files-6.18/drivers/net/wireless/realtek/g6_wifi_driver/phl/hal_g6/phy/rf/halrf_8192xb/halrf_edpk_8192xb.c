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
#ifdef RF_8192XB_SUPPORT

/*8192XB EDPK ver:0x8 202301512*/

void _edpk_bkup_kip_8192xb(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][EDPK_KIP_REG_NUM_8192XB],
	u8 path)
{
	u8 i;
	//RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	for (i = 0; i < EDPK_KIP_REG_NUM_8192XB; i++) {
		reg_bkup[path][i] = halrf_rreg(rf, reg[i] + (path << 8), MASKDWORD);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup 0x%x = %x\n", reg[i]+ (path << 8), reg_bkup[path][i]);
	}
}

#if 0
void _edpk_bkup_bb_8192xb(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[EDPK_BB_REG_NUM_8192XB])
{
	u8 i;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	for (i = 0; i < EDPK_BB_REG_NUM_8192XB; i++) {
		reg_bkup[i] = halrf_rreg(rf, reg[i], MASKDWORD);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup 0x%x = %x\n", reg[i], reg_bkup[i]);
	}
}
#endif

void _edpk_backup_bb_reg_8192xb(
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



void _edpk_bkup_rf_8192xb(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][EDPK_RF_REG_NUM_8192XB],
	u8 path)
{
	u8 i;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	for (i = 0; i < EDPK_RF_REG_NUM_8192XB; i++) {
		if (rf_reg[i] == 0x5)
			continue;
		rf_bkup[path][i] = halrf_rrf(rf, path, rf_reg[i], MASKRF);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Backup RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}

void _edpk_reload_kip_8192xb(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[][EDPK_KIP_REG_NUM_8192XB],
	u8 path) 
{
	u8 i;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	for (i = 0; i < EDPK_KIP_REG_NUM_8192XB; i++) {
		halrf_wreg(rf, reg[i] + (path << 8), MASKDWORD, reg_bkup[path][i]);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload 0x%x = %x\n", reg[i] + (path << 8),
				   reg_bkup[path][i]);
	}
}

#if 0
void _edpk_reload_bb_8192xb(
	struct rf_info *rf,
	u32 *reg,
	u32 reg_bkup[EDPK_BB_REG_NUM_8192XB]) 
{
	u8 i;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	for (i = 0; i < EDPK_BB_REG_NUM_8192XB; i++) {
		halrf_wreg(rf, reg[i], MASKDWORD, reg_bkup[i]);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload 0x%x = %x\n", reg[i],
				   reg_bkup[i]);
	}
}
#endif

void _edpk_reload_bb_reg_8192xb(
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


void _edpk_reload_rf_8192xb(
	struct rf_info *rf,
	u32 *rf_reg,
	u32 rf_bkup[][EDPK_RF_REG_NUM_8192XB],
	u8 path)
{
	u8 i;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	for (i = 0; i < EDPK_RF_REG_NUM_8192XB; i++) {
		if (rf_reg[i] == 0x5)
			continue;
		halrf_wrf(rf, path, rf_reg[i], MASKRF, rf_bkup[path][i]);
		if (EDPK_REG_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Reload RF S%d 0x%x = %x\n",
				path, rf_reg[i], rf_bkup[path][i]);
	}
}

void _edpk_hw_tx_8192xb(struct rf_info *rf,
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

u8 _edpk_ch_to_idx_8192xb(struct rf_info *rf, u8 channel)
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


void halrf_edpk_set_dgain_thres_8192xb(struct rf_info *rf, u32 val){

	struct halrf_dpk_info *dpk = &rf->dpk;

	dpk->edpk_dgain_thrs = val;
}


void halrf_edpk_pre_tx_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	u16 cnt, u16 period, s16 pwr, u32 delay) 
{
	enum rf_path path = RF_PATH_A;

	struct halrf_dpk_info *dpk = &rf->dpk;
	u8 kidx = dpk->cur_idx[path];
	//u8 kidx = 0;

	u8 channel = rf->hal_com->band[phy].cur_chandef.center_ch;
//	u8 bw = rf->hal_com->band[phy].cur_chandef.bw;
//	s8 power, power_tmp;
	//s16 xdbm;
	u8 ch_idx = _edpk_ch_to_idx_8192xb(rf, channel);
//	u8 channel_tmp;
	s8 txagc_base;
#if 0
	s8 shift[2] = {-6, 4};
#else
	s8 shift[2] = {0, 0};
#endif
	u8 ther_base;
	u8 i;

	u32 bb_bkup[EDPK_PRE_TX_BB_REG_NUM_8192XB] = {0};
	u32 bb_reg[] = {0x49c0, 0x0d18, 0x0d80, 0x58e8, 0x78e8};
	bool tssi_status = rf->is_tssi_mode[RF_PATH_A];

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	RF_DBG(rf, DBG_RF_DPK,"[EDPK]%s ================[S%d[%d] (dpk CH %d)]================\n",
			__func__, path, kidx, dpk->bp[path][kidx].ch);
	RF_DBG(rf, DBG_RF_DPK,"[EDPK]ch = %d\n", channel);

	RF_DBG(rf, DBG_RF_DPK,"[EDPK]Before BB bkup,Txpath_0x458c[31:28] = %d, Rxpath_0x49c4[3:0] = %d \n",halrf_rreg(rf, 0x458c, 0xf0000000), halrf_rreg(rf, 0x49c4, 0xf));

	RF_DBG(rf, DBG_RF_DPK,"[EDP]Record TSSI stats: %d\n", tssi_status);

	halrf_hal_bb_backup_info(rf, phy);	//BB Backup
//	_edpk_bkup_bb_8192xb(rf, bb_reg, bb_bkup); //other BB backup
	_edpk_backup_bb_reg_8192xb(rf, bb_reg, bb_bkup, EDPK_PRE_TX_BB_REG_NUM_8192XB);

/*
	if (bw == CHANNEL_WIDTH_40)
		channel_tmp = channel - 2;
	else if (bw == CHANNEL_WIDTH_80)
		channel_tmp = channel - 6;
	else
		channel_tmp = channel;
*/	
	if (cnt == 0)
		cnt = 1000;
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
	RF_DBG(rf, DBG_RF_DPK,"[EDPK] check tssi, 0x5818[28] = %d, 0x5820[31] = %d \n", halrf_rreg(rf, 0x5818, 0x10000000), halrf_rreg(rf, 0x5820, 0x80000000));
	RF_DBG(rf, DBG_RF_DPK,"[EDPK] check tssi, 0x7818[28] = %d, 0x7820[31] = %d \n", halrf_rreg(rf, 0x7818, 0x10000000), halrf_rreg(rf, 0x7820, 0x80000000));

#if 0
	halrf_wreg(rf, 0x58e8, 0x0000003f, 0x00);
	halrf_wreg(rf, 0x78e8, 0x0000003f, 0x00);
#endif

	_edpk_hw_tx_8192xb(rf, phy, RF_PATH_AB, cnt, period, pwr, T_HT_MF, 0, true);	//TX Strat

	halrf_delay_us(rf, delay*1000);
	
	_edpk_hw_tx_8192xb(rf, phy, RF_PATH_AB, cnt, period, pwr, T_HT_MF, 0, false);	//TX stop

	for (i = 0; i < 2; i++) {
		path = i;
		
		//txagc
		txagc_base = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), 0xFF000000); /*[31:24]*/
	
		//read thermal
		
		//ther_base = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000003F); /*[5:0]*/
		ther_base = halrf_get_thermal_8192xb(rf, path);


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
	_edpk_reload_bb_reg_8192xb(rf, bb_reg, bb_bkup, EDPK_PRE_TX_BB_REG_NUM_8192XB);
	
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





u8 _edpk_one_shot_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	enum dpk_id id)
{
	u8 phy_map;
	u16 dpk_cmd = 0x0, cnt1 = 0, cnt2 = 0;
	u32 r_bff8 = 0x0, r_80fc = 0x0;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	phy_map = (BIT(phy) << 4) | BIT(path);

	dpk_cmd = (u16)((id << 8) | (0x19 + path * 0x12));
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_START);
#endif
	halrf_wreg(rf, 0x8000, MASKDWORD, dpk_cmd);

	r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
	while (r_bff8 != 0x55 && cnt1 < 2000) {
		halrf_delay_us(rf, 10);
		r_bff8 = halrf_rreg(rf, 0xbff8, MASKBYTE0);
		cnt1++;
	}

	halrf_delay_us(rf, 1);

	r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
	while (r_80fc != 0x8000 && cnt2 < 2000) {
		halrf_delay_us(rf, 1);
		r_80fc = halrf_rreg(rf, 0x80fc, MASKLWORD);
		cnt2++;
	}

	halrf_wreg(rf, 0x8010, MASKBYTE0, 0x0);
#if 0
	halrf_btc_rfk_ntfy(rf, phy_map, RF_BTC_DPK, RFK_ONESHOT_STOP);
#endif
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] one-shot for %s = 0x%04x (cnt:%d/%d)\n",
	       id == 0x06 ? "LBK_RXIQK" : (id == 0x28 ? "KIP_PRESET" : (id == 0x29 ? "DPK_TXAGC" :
	       (id == 0x2a ? "DPK_RXAGC" : (id == 0x2b ? "SYNC" :
	       	(id == 0x2c ? "GAIN_LOSS" : (id == 0x2d ? "MDPK_IDL" :
		(id == 0x2f ? "DPK_GAIN_NORM" : (id == 0x31 ? "KIP_RESOTRE" :
		"Unknown id")))))))), dpk_cmd, cnt1, cnt2);

	if (cnt1 == 2000 || cnt2 == 2000) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] one-shot timeout!!!!\n");
		return 1;
	} else
		return 0;
}


void _edpk_init_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 kidx = dpk->cur_idx[path];
	u8 k_set;

	dpk->bp[path][kidx].path_ok = 0;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	

	for (k_set = 0; k_set < 4; k_set++) {
		dpk->edpk_rek_cnt[path][k_set] = 0;

		dpk->edpk_rek_chk[path][k_set][0] = 0;
		dpk->edpk_rek_chk[path][k_set][1] = 0;
		dpk->edpk_rek_chk[path][k_set][2] = 0;
		dpk->edpk_rek_chk[path][k_set][3] = 0;
		dpk->edpk_rek_chk[path][k_set][4] = 0;
	}

	
}

void _edpk_information_8192xb(
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
	       dpk->bp[path][kidx].bw == 0 ? "20M" : (dpk->bp[path][kidx].bw == 1 ? "40M" : "80M"));
}

void _edpk_rxagc_onoff_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	bool turn_on)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	if (path == RF_PATH_A)
		halrf_wreg(rf, 0x4730, BIT(31), turn_on);
	else
		halrf_wreg(rf, 0x4a9c, BIT(31), turn_on);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RXAGC is %s\n", path,
		turn_on ? "turn_on" : "turn_off");
}

void _edpk_bb_afe_setting_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	/*1. Keep ADC_fifo reset*/
	halrf_wreg(rf, 0x20fc, BIT(16 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x0);
	halrf_wreg(rf, 0x20fc, BIT(24 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x0);
	/*2. BB for IQK DBG mode*/
	halrf_wreg(rf, 0x5670 + (path << 13), MASKDWORD, 0xd801dffd); /*bit13 for gapk_offset*/
	/*3.Set DAC clk*/
	halrf_txck_force_8192xb(rf, path, true, DAC_960M);
	/*4. Set ADC clk*/
	halrf_rxck_force_8192xb(rf, path, true, ADC_1920M);
	//halrf_wreg(rf, 0x5670 + (path << 13), BIT(30) | BIT(29), 0x2);

	halrf_wreg(rf, 0xc0d4 + (path << 8), BIT(27) | BIT(26), 0x0);
	halrf_wreg(rf, 0xc0d8 + (path << 8), 0x000001E0, 0x7); /*[8:5]*/
	halrf_wreg(rf, 0xc0ec + (path << 8), BIT(14) | BIT(13), 0x0); /*force CBW_idx 0*/
	halrf_wreg(rf, 0x12b8 + (path << 13), BIT(30), 0x1);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x1f);
	halrf_wreg(rf, 0x030c, MASKBYTE3, 0x13);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0001);
	halrf_wreg(rf, 0x032c, MASKHWORD, 0x0041);
	/*5. ADDA fifo rst*/
	halrf_wreg(rf, 0x20fc, BIT(20 + path), 0x1);
	halrf_wreg(rf, 0x20fc, BIT(28 + path), 0x1);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d BB/AFE setting\n", path);
}

void _edpk_bb_afe_restore_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
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

void _edpk_tssi_pause_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	bool is_pause)
{
	halrf_wreg(rf, 0x5818 + (path << 13), BIT(30), is_pause);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d TSSI %s\n", path,
	       is_pause ? "pause" : "resume");
}

void _edpk_efem_control_8192xb(
	struct rf_info *rf,
	bool is_tx_on)
{
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	if (is_tx_on) {/*rfe_type 5354*/
	//	halrf_wreg(rf, 0x5890, MASKDWORD, 0x00000840);
		halrf_wreg(rf, 0x5890, BIT(6), 0x1);   // GPIO[6]=1
		halrf_wreg(rf, 0x5890, BIT(7), 0x0);   // GPIO[7]=0 
		halrf_wreg(rf, 0x5890, BIT(11), 0x1);   // GPIO[11]=1
		halrf_wreg(rf, 0x5880, MASKDWORD, 0x77777777); // SW control
		halrf_wreg(rf, 0x5884, MASKDWORD, 0x77777777); // SW control

		
	//	halrf_wreg(rf, 0x7890, MASKDWORD, 0x0000000a);
		halrf_wreg(rf, 0x7890, BIT(1), 0x1);    // GPIO[1]=1
		halrf_wreg(rf, 0x7890, BIT(2), 0x0);    // GPIO[2]=0
		halrf_wreg(rf, 0x7890, BIT(3), 0x1);    // GPIO[3]=1
		halrf_wreg(rf, 0x7880, MASKDWORD, 0x77777777); // SW control
		halrf_wreg(rf, 0x7884, MASKDWORD, 0x77777777); // SW control

		RF_DBG(rf, DBG_RF_DPK, "[EDPK] eFEM Tx fix TX mode\n");
	} else {
#if 0
		halrf_wreg(rf, 0x5890, MASKDWORD, 0x000000c0);
		halrf_wreg(rf, 0x5880, MASKDWORD, 0x43777777);
		halrf_wreg(rf, 0x5884, MASKDWORD, 0x77770777);
		halrf_wreg(rf, 0x7890, MASKDWORD, 0x0000000c);
		halrf_wreg(rf, 0x7880, MASKDWORD, 0x77773407);
		halrf_wreg(rf, 0x7884, MASKDWORD, 0x77777777);
#endif
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] eFEM Tx back to HW Control by restore Reg\n");
	}
}

void _edpk_tpg_sel_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

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
}

void _edpk_txpwr_bb_force_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	bool is_force)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	halrf_wreg(rf, 0x56cc + (path << 13), BIT(28), is_force); /*txpwr_bb_force_on*/
	halrf_wreg(rf, 0x580c + (path << 13), BIT(15), is_force); /*txpwr_bb_force_rdy*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d txpwr_bb_force %s\n", path, is_force ? "on" : "off");
}

void _edpk_kip_pwr_clk_onoff_8192xb(
	struct rf_info *rf,
	bool turn_on)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	if (turn_on) {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000080); /*cip power on*/
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x807f030a); /*320M*/
	} else {
		halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
		halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);
		halrf_wreg(rf, 0x80f4, BIT(18), 0x1);
	}

	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] KIP Power/CLK is %s\n", turn_on ? "turn_on" : "turn_off");
}

void _edpk_kip_control_rfc_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	bool ctrl_by_kip)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	halrf_wreg(rf, 0x5670 + (path << 13), BIT(1), ctrl_by_kip); /*KIP control RFC*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RFC is controlled by %s\n",
		path, ctrl_by_kip ? "KIP" : "BB");
}

void _edpk_kip_preset_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	/*must before kip control RFC*/
	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x8078 = 0x%x\n", halrf_rreg(rf, 0x8078, MASKDWORD));

	//halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0x00003F00, 0x01); /*[13:8] slope*/

	_edpk_kip_control_rfc_8192xb(rf, path, true);
	halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), kidx); /*ch0/ch1 selection*/

	_edpk_one_shot_8192xb(rf, phy, path, D_KIP_PRESET);
}

void _edpk_kip_restore_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	_edpk_one_shot_8192xb(rf, phy, path, D_KIP_RESTORE);

	/*cip power on*/
	//halrf_wreg(rf, 0x8008, MASKDWORD, 0x00000000);
	/*CFIR CLK restore*/
	//halrf_wreg(rf, 0x8088, MASKDWORD, 0x80000000);

	//_edpk_kip_control_rfc_8192xb(rf, path, false);
	_edpk_txpwr_bb_force_8192xb(rf, path, false);
#if 0
	if (rf->hal_com->cv > 0x0) /*hw txagc_offset*/
		halrf_wreg(rf, 0x81c8 + (path << 8), BIT(15), 0x1);
#endif
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d restore KIP\n", path);
}

u8 _edpk_dbm_convert_8192xb(
	struct rf_info *rf,
	u8 dbm)
{
	u8 txagc_cw;


//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	txagc_cw = (dbm - 16) * 4 + 0x40; /*0.25dB/step*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] convert %ddBm to 0x%x\n", dbm, txagc_cw);

	return txagc_cw;
}

void _edpk_read_rxsram_8192xb(
	struct rf_info *rf)
{
	u32 addr;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80e8, BIT(7), 0x1);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x1);	/*rxsram_ctrl_sel*/
	halrf_wreg(rf, 0x80d4, MASKDWORD, 0x00020000);	/*rpt_sel*/

	for (addr = 0; addr < 0x200; addr++) {
		halrf_wreg(rf, 0x80d8, MASKDWORD, 0x00010000 | addr);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] RXSRAM[%03d] = 0x%08x\n", addr,
			halrf_rreg(rf, 0x80fc, MASKDWORD));
	}
	halrf_wreg(rf, 0x80e8, BIT(7), 0x0);	/*web_iqrx*/
	halrf_wreg(rf, 0x8074, BIT(31), 0x0);	/*rxsram_ctrl_sel*/
}

void _edpk_rf_reg_query_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u32 reg)
{
	

#if 1
	u32 ori_ctrl = halrf_rreg(rf, 0x5670 + (path << 13), BIT(1));

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	_edpk_kip_control_rfc_8192xb(rf, path, false);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] query RF 0x%x = 0x%x\n", reg,
		halrf_rrf(rf, path, reg, MASKRF));

	_edpk_kip_control_rfc_8192xb(rf, path, (bool)ori_ctrl);
#endif
}

void _edpk_kset_query_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x81d4 + (path << 8), 0x003F0000, 0x10);	/*rpt_sel*/

	dpk->cur_k_set = (u8)(halrf_rreg(rf, 0x81fc + (path << 8), 0xE0000000) - 1); /*[31:29]*/

	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] cur k_set = %d\n", dpk->cur_k_set);
}

void _edpk_para_query_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	/*[31:26]:t-meter, [25:16]:txagc_bb, [15:7]:txagc, [6:0]:gs*/
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};
	u32 para;
//	u8 ref = 0;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	para = halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), MASKDWORD);

	dpk->bp[path][kidx].txagc_dpk = (para >> 10) & 0x3f;
	dpk->bp[path][kidx].ther_dpk = (para >> 26) & 0x3f;

//	ref = (u8)halrf_rreg(rf, 0x81ac + (path << 8), 0x00fc0000); /*[23:18] */

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] thermal/ txagc_RF (K%d) = 0x%x/ 0x%x\n",
		dpk->cur_k_set, dpk->bp[path][kidx].ther_dpk, dpk->bp[path][kidx].txagc_dpk);

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x%x = 0x%x\n", 0x81ac + (path << 8), ref);

	

//	if (dpk->bp[path][kidx].txagc_dpk < ref) {
//		halrf_wreg(rf, 0x81ac + (path << 8), 0x00fc0000, dpk->bp[path][kidx].txagc_dpk);		
//		RF_DBG(rf, DBG_RF_DPK, "[EDPK]!!! 0x%x[23:18] = txagc_dpk = 0%x\n", 0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0x00fc0000));
//	}

#if 0
	if (kidx == 0) { /*CH0*/
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K0= 0x%x\n",
			halrf_rreg(rf, 0x8190 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K1= 0x%x\n",
			halrf_rreg(rf, 0x8194 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K2= 0x%x\n",
			halrf_rreg(rf, 0x8198 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K3= 0x%x\n",
			halrf_rreg(rf, 0x81a4 + (path << 8), MASKDWORD));
	} else { /*CH1*/
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K0= 0x%x\n",
			halrf_rreg(rf, 0x81a8 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K1= 0x%x\n",
			halrf_rreg(rf, 0x81c4 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K2= 0x%x\n",
			halrf_rreg(rf, 0x81c8 + (path << 8), MASKDWORD));
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] CH0_K3= 0x%x\n",
			halrf_rreg(rf, 0x81e8 + (path << 8), MASKDWORD));
	}
#endif
}

bool _edpk_sync_check_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u16 dc_i, dc_q;
	u8 corr_val, corr_idx, rxbb;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

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

	//_edpk_rf_reg_query_8192xb(rf, path, 0x00);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x31);	/*rpt_sel*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RXBB/ RXAGC_done /RXBB_ovlmt = %d / %d / %d\n",
		path, rxbb,
		halrf_rreg(rf, 0x80fc, BIT(0)),
		halrf_rreg(rf, 0x80fc, BIT(8)));

	if ((dc_i > 200) || (dc_q > 200) || (corr_val < 160))
		return true;
	else
		return false;
}

void _edpk_kip_set_txagc_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 dbm,
	bool set_from_bb)
{	

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	if (set_from_bb) {
		if (dbm >= 30)
			dbm = 30;
		else if (dbm <= 7)
			dbm = 7;
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] set S%d txagc to %ddBm\n", path, dbm);
		halrf_wreg(rf, 0x56cc + (path << 13), 0x0FF80000, dbm << 2); /*[27:19]*/
	}

	_edpk_one_shot_8192xb(rf, phy, path, D_TXAGC);
	_edpk_kset_query_8192xb(rf, path);
	//_edpk_rf_reg_query_8192xb(rf, path, 0x11);
}

bool _edpk_kip_set_rxagc_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	_edpk_kip_control_rfc_8192xb(rf, path, false);
	halrf_wreg(rf, 0x8078, 0x000FFFFF, halrf_rrf(rf, path, 0x00, MASKRF)); /*[19:0]*/
	_edpk_kip_control_rfc_8192xb(rf, path, true);

	_edpk_one_shot_8192xb(rf, phy, path, D_RXAGC);
#if 0
	halrf_wreg(rf, 0x80d4, 0x000F0000, 0x8);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] set RXBB = 0x%x\n",
		halrf_rreg(rf, 0x80fc, 0x0000001F));
#endif
	return _edpk_sync_check_8192xb(rf, path, kidx);
}

void _edpk_lbk_rxiqk_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	bool is_bypass)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 i, cur_rxbb;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1);

	if (is_bypass) {
		halrf_wreg(rf, 0x813c + (path << 8), MASKDWORD, 0x40000002);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Bypass RXIQC\n");
		return;
	}

	halrf_wreg(rf, 0x8074, BIT(31), 0x1); /*RxSRAM_ctrl_sel 0:MDPK; 1:IQK*/

	_edpk_kip_control_rfc_8192xb(rf, path, false);

	cur_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);

	/*RF setting*/
	halrf_wrf(rf, path, 0xdc, BIT(9) | BIT(8), 0x3);
	halrf_wrf(rf, path, 0x1f, BIT(17) | BIT(16), halrf_rrf(rf, path, 0x17, BIT(11) | BIT(10)));
	halrf_wrf(rf, path, 0x1f, 0x003FF, halrf_rrf(rf, path, 0x17, 0x003FF)); /*[9:0]*/
	halrf_wrf(rf, path, 0x00, MASKRFMODE, 0xd);
	halrf_wrf(rf, path, 0x84, 0xF0000, 0xa); /*[19:16]*/

	if (cur_rxbb >= 0xe)
		halrf_wrf(rf, path, 0x84, BIT(13) | BIT(12), 0x0); /*TXG_ATT*/
	else if (cur_rxbb <= 0xa)
		halrf_wrf(rf, path, 0x84, BIT(13) | BIT(12), 0x2); /*TXG_ATT*/
	else
		halrf_wrf(rf, path, 0x84, BIT(13) | BIT(12), 0x1); /*TXG_ATT*/

	halrf_wrf(rf, path, 0x1e, MASKRF, 0x80014); /*POW IQKPLL, 9.25MHz offset for IQKPLL*/

	for (i = 0; i < 10; i++)
		halrf_delay_us(rf, 10); /*IQKPLL's settling time*/

	_edpk_kip_control_rfc_8192xb(rf, path, true);

	halrf_wreg(rf, 0x802c, 0x0FFF0000, 0x025); /*[27:16] Rx_tone_idx=0x025 (9.25MHz)*/

	_edpk_one_shot_8192xb(rf, phy, path, LBK_RXIQK);

	dpk->dpk_rxiqc[path] = halrf_rreg(rf, 0x813c + (path << 8), MASKDWORD);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d LBK RXIQC = 0x%x\n", path, dpk->dpk_rxiqc[path]);

	_edpk_kip_control_rfc_8192xb(rf, path, false);

	halrf_wrf(rf, path, 0xdc, BIT(9) | BIT(8), 0x0);
	halrf_wreg(rf, 0x8074, BIT(31), 0x0); /*RxSRAM_ctrl_sel 0:MDPK; 1:IQK*/
	halrf_wreg(rf, 0x80d0, BIT(21) | BIT(20), 0x0);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(1), 0x1); /*auto*/
	halrf_wrf(rf, path, 0x00, MASKRFMODE, 0x5);

	_edpk_kip_control_rfc_8192xb(rf, path, true);
}

void _edpk_get_thermal_8192xb(struct rf_info *rf, u8 kidx, enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

 	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	dpk->bp[path][kidx].ther_dpk = halrf_get_thermal_8192xb(rf, path);

	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0xFC000000,
			dpk->bp[path][kidx].ther_dpk); /*[31:26]*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] thermal@DPK (by Driver)= 0x%x\n", dpk->bp[path][kidx].ther_dpk);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x%x = 0x%x\n", reg[kidx][dpk->cur_k_set] + (path << 8),
		halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), MASKDWORD));
}

void _edpk_rf_setting_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wrf(rf, path, 0x00, MASKRF, 0x50241);
	halrf_wrf(rf, path, 0x10000, MASKRFMODE, RF_DPK);
	halrf_wrf(rf, path, 0x01, BIT(18) | BIT(17) | BIT(16), 0x7); /*TSSI curve*/
	/*TIA*/
	halrf_wrf(rf, path, 0xdf, BIT(12), 0x1);
	halrf_wrf(rf, path, 0x9e, BIT(8), 0x1); /*0:26dB, 1:20dB*/
	/*eDPD*/
	halrf_wrf(rf, path, 0x02, BIT(17), 0x1); /*DEBUG_BB_SEL_AUX_RXG*/
	halrf_wrf(rf, path, 0x02, BIT(15), 0x1); /*BB_SEL_AUX_RXG_int*/
	/*att*/
#if 0
	halrf_wrf(rf, path, 0x82, BIT(10) | BIT(9) | BIT(8), 0x0); /*ATT 0~7: 0 ~ -8 dB*/
	halrf_wrf(rf, path, 0x82, BIT(6) | BIT(5) | BIT(4), 0x3); /*R1  0~3 : -38/-34/-28/-22 dB*/
	halrf_wrf(rf, path, 0x82, BIT(3) | BIT(2), 0x3); /*R2  0~3 : -12/-8/-4/0 dB*/
#else //morgan
	halrf_wrf(rf, path, 0x82, BIT(10) | BIT(9) | BIT(8), 0x4); /*ATT 0~7: 0 ~ -8 dB*/
	halrf_wrf(rf, path, 0x82, BIT(6) | BIT(5) | BIT(4), 0x2); /*R1	0~3 : -38/-34/-28/-22 dB*/
	halrf_wrf(rf, path, 0x82, BIT(3) | BIT(2), 0x2); /*R2  0~3 : -12/-8/-4/0 dB*/
#endif
	halrf_wrf(rf, path, 0x80, BIT(2), 0x0); /*POW_EDPD_IQK*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d RF 0x0/0x2/0x80/0x82/0x9e/0xdf/0x1a/0x1001a = 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x/ 0x%x\n",
	       path, halrf_rrf(rf, path, 0x00, MASKRF),
	       halrf_rrf(rf, path, 0x02, MASKRF),
	       halrf_rrf(rf, path, 0x80, MASKRF),
	       halrf_rrf(rf, path, 0x82, MASKRF),
	       halrf_rrf(rf, path, 0x9e, MASKRF),
	       halrf_rrf(rf, path, 0xdf, MASKRF),
	       halrf_rrf(rf, path, 0x1a, MASKRF),
	       halrf_rrf(rf, path, 0x1001a, MASKRF));
}

void _edpk_bypass_rxiqc_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	halrf_wreg(rf, 0x81a0 + (path << 8), BIT(7), 0x1);
	halrf_wreg(rf, 0x813c + (path << 8), MASKDWORD, 0x40000002);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Bypass RXIQC\n");
}

u16 _edpk_dgain_read_8192xb(
	struct rf_info *rf)
{
	u16 dgain = 0x0;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/

	dgain = (u16)halrf_rreg(rf, 0x80fc, 0x0FFF0000);	/*[27:16]*/

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] DGain = 0x%x\n", dgain);

	return dgain;
}

s8 _edpk_dgain_mapping_8192xb(
	struct rf_info *rf,
	u16 dgain)
{
	u16 bnd[15] = {0xbf1, 0xaa5, 0x97d, 0x875, 0x789,
			0x6b7, 0x5fc, 0x556, 0x4c1, 0x43d,
			0x3c7, 0x35e, 0x2ac, 0x262, 0x220};
	s8 offset = 0;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if (dgain >= bnd[0])
		offset = 0x6;
	else if ((bnd[0] > dgain) && (dgain >= bnd[1]))
		offset = 0x6;
	else if ((bnd[1] > dgain) && (dgain >= bnd[2]))
		offset = 0x5;
	else if ((bnd[2] > dgain) && (dgain >= bnd[3]))
		offset = 0x4;
	else if ((bnd[3] > dgain) && (dgain >= bnd[4]))
		offset = 0x3;
	else if ((bnd[4] > dgain) && (dgain >= bnd[5]))
		offset = 0x2;
	else if ((bnd[5] > dgain) && (dgain >= bnd[6]))
		offset = 0x1;
	else if ((bnd[6] > dgain) && (dgain >= bnd[7]))
		offset = 0x0;
	else if ((bnd[7] > dgain) && (dgain >= bnd[8]))
		offset = 0xff;
	else if ((bnd[8] > dgain) && (dgain >= bnd[9]))
		offset = 0xfe;
	else if ((bnd[9] > dgain) && (dgain >= bnd[10]))
		offset = 0xfd;
	else if ((bnd[10] > dgain) && (dgain >= bnd[11]))
		offset = 0xfc;
	else if ((bnd[11] > dgain) && (dgain >= bnd[12]))
		offset = 0xfb;
	else if ((bnd[12] > dgain) && (dgain >= bnd[13]))
		offset = 0xfa;
	else if ((bnd[13] > dgain) && (dgain >= bnd[14]))
		offset = 0xf9;
	else if (bnd[14] > dgain)
		offset = 0xf8;
	else
		offset = 0x0;

	//RF_DBG(rf, DBG_RF_DPK, "[EDPK] DGain offset = %d\n", offset);

	return offset;
}

u8 _edpk_pas_check_8192xb(
	struct rf_info *rf)
{
	u8 fail = 0;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);

	halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00); /*0x80C3*/
	if (halrf_rreg(rf, 0x80fc, MASKHWORD) == 0x0800) {
		fail = 1;
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] PAS check Fail!!\n");
	}
	
	return fail;
}

u8 _edpk_gainloss_read_8192xb(
	struct rf_info *rf, enum rf_path path)
{
	u8 result;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x6);	/*rpt_sel*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x1);		/*query status*/

	result = (u8)halrf_rreg(rf, 0x80fc, 0x000000F0); /*[7:4]*/


	RF_DBG(rf, DBG_RF_DPK, "[EDPK] tmp GL = %d\n", result);

	return result;
}

u8 _edpk_gainloss_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	
	halrf_wreg(rf, 0x81bc + (path << 8), BIT(23), 0x1); /*eFEM_en*/

	_edpk_one_shot_8192xb(rf, phy, path, D_GAIN_LOSS);
	_edpk_kip_set_txagc_8192xb(rf, phy, path, 0xff, false); /*set new tx power*/

	/*for hw_bug*/
	#if 0
	halrf_wreg(rf, 0x81f0 + (path << 8), MASKDWORD, 0x0000f078); /*[17:0]*/
	#else //20220819 morgan

//	halrf_wreg(rf, 0x81ac + (path << 8), 0xfc0000, dpk->bnd_dpd[path]); //bnd_dpd
	halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, dpk->pwsf_df[path]); /*[8:0]*/


	RF_DBG(rf, DBG_RF_DPK, "[EDPK]AFTER get gainloss, FIX HW BUG, rewrites, S%d \n", path);
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]NO-write, just read edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000));
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]aftr write pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), halrf_rreg(rf, 0x81f0 + (path << 8), 0x1ff));
	#endif

	return _edpk_gainloss_read_8192xb(rf, path);
}

u8 _edpk_pas_read_8192xb(
	struct rf_info *rf,
	u8 is_check)
{
	u8 i;
	u32 val1_i = 0, val1_q = 0, val2_i = 0, val2_q = 0;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

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
	else if ((val1_i * val1_i + val1_q * val1_q) >= ((val2_i * val2_i + val2_q * val2_q) * 3))
		return 1;
	else
		return 0;
}

void _edpk_pa_scan(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	u8 i = 0;
	u32 tmp = 0;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	halrf_wreg(rf, 0x80d4, 0x00ff0000, 0x06);
	halrf_wreg(rf, 0x80bc, 0x00004000, 0x0);
	halrf_wreg(rf, 0x80c0, 0x00ff0000, 0x08);

	for (i = 0; i <= 0x1f; i++) {
		halrf_wreg(rf, 0x80c0, 0xff000000, i);
 		tmp = halrf_rreg(rf, 0x80fc, MASKDWORD);
		RF_DBG(rf, DBG_RF_DPK, "PA_SCAN S%d, i=%d, 0x80fc = 0x%x\n",path, i, tmp);
	}


}



u8 _edpk_agc_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u8 init_xdbm,
	u8 loss_only)
{
	u8 i = 0, tmp_dbm = init_xdbm, tmp_gl_idx = 0;
	u8 tmp_rxbb = 0,limited_rxbb = 0;
	u8 goout = 0, agc_cnt = 0;
	s8 offset = 0;
	u16 dgain = 0;
	bool is_fail = false;

	struct halrf_dpk_info *dpk = &rf->dpk;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);
	
	do {
		switch (i) {
		case 0: /*SYNC and Dgain*/
			is_fail = _edpk_kip_set_rxagc_8192xb(rf, phy, path, kidx);

			if (EDPK_RXSRAM_DBG_8192XB)
				_edpk_read_rxsram_8192xb(rf);

			if (is_fail) {
				goout = 1;
				break;
			}

			dgain = _edpk_dgain_read_8192xb(rf);

			if (dgain > dpk->edpk_dgain_thrs) {
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] Dgain (0x%x) > threshold(0x%x), return, SKIP EDPK!!\n", dgain, dpk->edpk_dgain_thrs);
				is_fail = true;
				goout = 1;
				break;
			}

			i = 2;
			break;

		case 1: /*Gain adjustment*/
			_edpk_kip_control_rfc_8192xb(rf, path, false);
			tmp_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);
			offset = _edpk_dgain_mapping_8192xb(rf, dgain);

			if (tmp_rxbb + offset > 0x1f) {
				tmp_rxbb = 0x1f;
				limited_rxbb = 1;
			} else if (tmp_rxbb + offset < 0) {
				tmp_rxbb = 0;
				limited_rxbb = 1;
			} else
				tmp_rxbb = tmp_rxbb + offset;

			halrf_wrf(rf, path, 0x00, MASKRFRXBB, tmp_rxbb);
			
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Adjust RXBB (%+d) = 0x%x\n", offset, tmp_rxbb);
			_edpk_kip_control_rfc_8192xb(rf, path, true);
#if 0
			if (offset != 0 || agc_cnt == 0) {
				if (rf->hal_com->band[phy].cur_chandef.bw < 2)
					_edpk_lbk_rxiqk_8192xb(rf, phy, path, true);
				else
					_edpk_lbk_rxiqk_8192xb(rf, phy, path, false);
			}
#endif
			if ((dgain > 1922) || (dgain < 342))
				i = 0;
			else
				i = 2;

			agc_cnt++;
			break;

		case 2: /*GAIN_LOSS and idx*/
			tmp_gl_idx = _edpk_gainloss_8192xb(rf, phy, path, kidx);
			//_edpk_pas_read_8192xb(rf, false);

			if ((_edpk_pas_read_8192xb(rf, true) == 2) && (tmp_gl_idx > 0))
				i = 4;
			else if ((tmp_gl_idx == 0 && _edpk_pas_read_8192xb(rf, true) == 1) || tmp_gl_idx >= 4)
				i = 3; /*GL > criterion*/
			else if (tmp_gl_idx == 0)
				i = 4; /*GL < criterion*/
			else 
				i = 5;

			//20220818 after gain loss read pa scan for debug
			//20220822 morgan remove
			//_edpk_pa_scan(rf, phy, path);			
			break;

		case 3: /*GL > criterion*/
			if (tmp_dbm <= 7) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] Txagc@lower bound!!\n");
			} else {
				if (tmp_dbm - 3 <= 7)
					tmp_dbm = 7;
				else
				tmp_dbm = tmp_dbm - 3;
				_edpk_kip_set_txagc_8192xb(rf, phy, path, tmp_dbm, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 4:	/*GL < criterion*/
			if (tmp_dbm >= 30) {
				goout = 1;
				RF_DBG(rf, DBG_RF_DPK, "[EDPK] Txagc@upper bound!!\n");
			} else {
				if (tmp_dbm + 2 >= 30)
					tmp_dbm = 30;
				else
				tmp_dbm = tmp_dbm + 2;
				_edpk_kip_set_txagc_8192xb(rf, phy, path, tmp_dbm, true);
			}
			i = 0;
			agc_cnt++;
			break;

		case 5:
			_edpk_kip_control_rfc_8192xb(rf, path, false);
			tmp_rxbb = (u8)halrf_rrf(rf, path, 0x00, MASKRFRXBB);
			if (tmp_rxbb + tmp_gl_idx > 0x1f)
				tmp_rxbb = 0x1f;
			 else
				tmp_rxbb = tmp_rxbb + tmp_gl_idx;

			 halrf_wrf(rf, path, 0x00, MASKRFRXBB, tmp_rxbb);
			
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] Adjust RXBB (%+d) = 0x%x\n",
				tmp_gl_idx, tmp_rxbb);
			_edpk_kip_control_rfc_8192xb(rf, path, true);
			goout = 1;
			break;
		default:
			goout = 1;
			break;
		}	
	} while (!goout && (agc_cnt < 6));

	return is_fail;
}

void _edpk_set_mdpd_para_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx,
	u8 order)
{
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	switch (order) {
	case 0: /*(5,3,1)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x0);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x2);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
				BIT(26) | BIT(25) | BIT(24), 0x6);
		break;

	case 1: 
	#if 1 //default
		/*(5,3,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x1);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x1);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
				BIT(26) | BIT(25) | BIT(24), 0x2);
	#else //morgan 0818  /*(5,0,0)*/
		halrf_wreg(rf, 0x80a0, BIT(1) | BIT(0), 0x2);
		halrf_wreg(rf, 0x809c, BIT(10) | BIT(9), 0x0);
		halrf_wreg(rf, 0x80a0, 0x00001F00, 0x0); /*[12:8] phase normalize tap*/
		halrf_wreg(rf, 0x8070, 0x70000000, 0x0); /*[30:28] tx_delay_man*/
		halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
				BIT(26) | BIT(25) | BIT(24), 0x0);
	#endif		
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

void _edpk_read_timingsync0_8192xb(
	struct rf_info *rf,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/
	dpk->dpk_sync[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x9);	/*rpt_sel*/
	dpk->dpk_dciq[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);
}

void _edpk_get_coef_for_avg_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx,
	bool is_first)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 addr[20] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12,
		       13, 14, 16, 17, 19, 20, 22, 23, 25, 26
		      };
	u32 coef;
	u8 i, avg_num = 12;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if(dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40)
		avg_num = 20;

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

	for (i = 0; i < avg_num; i++) {
		coef = halrf_rreg(rf, 0xA5E0 + path * 0x400 + addr[i] * 4, MASKDWORD);

		if (EDPK_COEF_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] get 0x%x = 0x%08x\n", 0xA5E0 + path * 0x400 + addr[i] * 4, coef);

		dpk->dpk_coef_i[is_first][i] = (coef >> 16) & 0x7ff;
		dpk->dpk_coef_q[is_first][i] = coef & 0x7ff;

		if ((dpk->dpk_coef_i[is_first][i] >> 10) != 0)
			dpk->dpk_coef_i[is_first][i] |= 0xF800;

		if ((dpk->dpk_coef_q[is_first][i] >> 10) != 0)
			dpk->dpk_coef_q[is_first][i] |= 0xF800;
	}

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);
}

void _edpk_avg_coef_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 addr[20] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12,
		        13, 14, 16, 17, 19, 20, 22, 23, 25, 26
		      };
	u32 avg_i, avg_q;
	u8 i, avg_num = 12;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if(dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40)
		avg_num = 20;

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

	for (i = 0; i < avg_num; i++) {
		if (dpk->rek_cnt[path][0] == 5 && dpk->rek_cnt[path][1] < 5) {/*avg0 fail*/
			avg_i = dpk->dpk_coef_i[1][i] & 0x7ff;
			avg_q = dpk->dpk_coef_q[1][i] & 0x7ff;
		} else if (dpk->rek_cnt[path][1] == 5 && dpk->rek_cnt[path][0] < 5) {/*avg1 fail*/
			avg_i = dpk->dpk_coef_i[0][i] & 0x7ff;
			avg_q = dpk->dpk_coef_q[0][i] & 0x7ff;
		} else {
			avg_i = ((dpk->dpk_coef_i[0][i] + dpk->dpk_coef_i[1][i])) >> 1 & 0x7ff;
			avg_q = ((dpk->dpk_coef_q[0][i] + dpk->dpk_coef_q[1][i])) >> 1 & 0x7ff;
		}
			
		halrf_wreg(rf, 0xA5E0 + path * 0x400 + addr[i] * 4, MASKDWORD, (avg_i << 16) | avg_q);

		if (EDPK_COEF_DBG_8192XB) {
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] coef_i[0][%02d] / coef_i[1][%02d] / coef_i_avg[%02d] = 0x%04x / 0x%04x / 0x%04x\n",
				i, i, i, dpk->dpk_coef_i[0][i], dpk->dpk_coef_i[1][i], avg_i);
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] coef_q[0][%02d] / coef_q[1][%02d] / coef_q_avg[%02d] = 0x%04x / 0x%04x / 0x%04x\n",
				i, i, i, dpk->dpk_coef_q[0][i], dpk->dpk_coef_q[1][i],avg_q);
		}
	}

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);
}

u8 _edpk_coef_check_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 addr[20] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12,
		       13, 14, 16, 17, 19, 20, 22, 23, 25, 26
		      };
	u8 k, fail = false, chk_num = 12;
	u32 coef;
	u16 coef_i, coef_q;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if(dpk->bp[path][kidx].bw == CHANNEL_WIDTH_40)
		chk_num = 20;

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

	for (k = 0; k < chk_num; k++) {
		coef = halrf_rreg(rf, 0xA5E0 + path * 0x400 + addr[k] * 4, MASKDWORD);

		if (EDPK_COEF_DBG_8192XB)
			RF_DBG(rf, DBG_RF_DPK, "[EDPK][CHK] 0x%x = 0x%08x \n", 0xA5E0 + path * 0x400 + addr[k] * 4, coef);

		coef_i = (coef >> 16) & 0x7ff;
		coef_q = coef & 0x7ff;

		if ((coef_i >> 10 == 0) && (coef_i > 0x380)) { /*>0*/
			fail = true;
			break;
		} else if ((coef_i >> 10 != 0) && (coef_i < 0x480)) { /*<0*/
			fail = true;
			break;
		} else if ((coef_q >> 10 == 0) && (coef_q > 0x380)) { /*>0*/
			fail = true;
			break;
		} else if ((coef_q >> 10 != 0) && (coef_q < 0x480)) { /*<0*/
			fail = true;
			break;
		}
	}
	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);

	if (fail)
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] coef check fail (0x%x/0x%x)\n", coef_i, coef_q);

	return fail;
}

u8 _edpk_ldl_phase_check_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 fail;
	u16 val_i = 0, val_q = 0;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80d4, MASKBYTE2, 0x06); /*0x80d6, ctrl_out_Kpack*/
	halrf_wreg(rf, 0x80bc, BIT(14), 0x0);	/*query status*/
	halrf_wreg(rf, 0x80c0, MASKBYTE2, 0x08);
	halrf_wreg(rf, 0x80c0, MASKBYTE3, 0x00); /*0x80C3*/

	val_i = halrf_rreg(rf, 0x80fc, MASKHWORD);
//	if (val_i >= 0x800)
//		val_i = 0x1000 - val_i;

	val_q = halrf_rreg(rf, 0x80fc, MASKLWORD);
	if (val_q >= 0x800)
		val_q = 0x1000 - val_q;

	if ((val_i <= 0x7ff) && ((val_i * 3 / 4) > val_q))
		fail = false;
	else
		fail = true;

	if (fail)
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] LDL phase check fail (0x%x/0x%x)\n", val_i, val_q);

	return fail;
}

bool _edpk_rek_check_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	u8 rek_cnt)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 corr_idx;
	bool result = false;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0x0);	/*rpt_sel*/
	dpk->dpk_sync[path] = halrf_rreg(rf, 0x80fc, MASKDWORD);
	corr_idx = dpk->dpk_sync[path] & 0xff; /*[7:0]*/

	if (HALRF_ABS(corr_idx - 16, dpk->corr_idx[path][kidx]) >= 2) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] corr_idx diff(%d, %d) >=2 happen!!!\n",
			dpk->corr_idx[path][kidx], corr_idx);
		dpk->edpk_rek_chk[path][dpk->cur_k_set][rek_cnt] = 0x1;
	}

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0xf);	/*rpt_sel*/
	dpk->ov_flag[path] = (u8)halrf_rreg(rf, 0x80fc, BIT(30));

	if (dpk->ov_flag[path]) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] MDPK OV happen!!!\n");
		dpk->edpk_rek_chk[path][dpk->cur_k_set][rek_cnt] = 0x3;
	}

	if (dpk->edpk_rek_chk[path][dpk->cur_k_set][rek_cnt] != 0)
		result = true;
	else {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] ReK check OK!!\n");
		result = false;
	}

	return result;
}

u8 _edpk_c_check_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kcnt)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 rpt12, rpt13;
	u16 addr[20] = {0x196, 0x197, 0x198, 0x19a, 0x19b,
		       0x19c, 0x19e, 0x19f, 0x1a0, 0x1a2,
		       0x1a3, 0x1a4, 0x1a6, 0x1a7, 0x1a9,
		       0x1aa, 0x1ac, 0x1ad, 0x1af, 0x1b0};
	//u8 i;
	bool is_fail = false;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80e8, BIT(6), 0x0);
	halrf_wreg(rf, 0x80d8, BIT(17), 0x1);

	halrf_wreg(rf, 0x80d8, 0x000001FF, addr[0]); /*[8:0]*/

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0xc); /*rpt_sel*/
	rpt12 = halrf_rreg(rf, 0x80fc, MASKDWORD);

	halrf_wreg(rf, 0x80d4, 0x003F0000, 0xd); /*rpt_sel*/
	rpt13 = halrf_rreg(rf, 0x80fc, MASKDWORD);

	if ((rpt12 >> 31) != 0)
		rpt12 = 0x80000000 - (rpt12 & 0x7fffffff);
	if ((rpt13 >> 31) != 0)
		rpt13 = 0x80000000 - (rpt13 & 0x7fffffff);

	if (rpt12 > 0x3333333 || rpt12 > 0x3333333)
		is_fail = false;
	else
		is_fail = true;

	dpk->c_chk[path] = is_fail;
#if 0
	for (i = 0; i < 20; i++) {
		halrf_wreg(rf, 0x80d8, 0x000001FF, addr[i]); /*[8:0]*/

		halrf_wreg(rf, 0x80d4, 0x003F0000, 0xc); /*rpt_sel*/
		rpt12 = halrf_rreg(rf, 0x80fc, MASKDWORD);
		dpk->rc_mtx[path][kcnt][i][0] = rpt12;

		halrf_wreg(rf, 0x80d4, 0x003F0000, 0xd); /*rpt_sel*/
		rpt13 = halrf_rreg(rf, 0x80fc, MASKDWORD);
		dpk->rc_mtx[path][kcnt][i][1] = rpt13;
#if 0
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] 0x%03x | 0x%08x 0x%08x 0x%08x\n",
			addr, rpt12, rpt13, rpt14);
#endif
	}
#endif
	halrf_wreg(rf, 0x80d8, BIT(17), 0x0);

	return is_fail;
}

void _edpk_coef_read_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx,
	bool is_first)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	//u32 reg, reg_start, reg_stop;
	u8 addr;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00010000);

	for (addr = 0; addr < 28; addr++)
		dpk->dpk_coef[path][is_first][addr] = halrf_rreg(rf, 0xA5E0 + path * 0x400 + addr * 4, MASKDWORD);
#if 0
	reg_start = 0xa500 + kidx * 0x1c0 + path * 0x400 + dpk->cur_k_set * 0x70;

	reg_stop = reg_start + 0x70;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] ===== [Coef of S%d_CH%d_K%d =====\n",
		path, kidx, dpk->cur_k_set);

	for (reg = reg_start; reg < reg_stop ; reg += 4) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK][coef_r] 0x%x = 0x%08x\n", reg,
			   halrf_rreg(rf, reg, MASKDWORD));
	}
#endif
	halrf_wreg(rf, 0x81d8 + (path << 8), MASKDWORD, 0x00000000);
}

bool _edpk_idl_mpa_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	bool is_rek;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, 0x80a0, BIT(16), 0x1); /*Phase Normalize enable*/

	if (halrf_rreg(rf, 0x80ec, BIT(8)) == 0x1)
		_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x2); /*5,0,0*/
	else if (halrf_rreg(rf, 0x80ec, BIT(9)) == 0x1)
		_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x1); /*5,3,0*/
	else if (halrf_rreg(rf, 0x80ec, BIT(10)) == 0x1)
		_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x0); /*5,3,1*/
	else if (dpk->bp[path][kidx].bw >= CHANNEL_WIDTH_40) 
		_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x2); /*5,0,0*/
		//_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x1); /*5,3,0*/
		//2022/0822 morgan
	else
		_edpk_set_mdpd_para_8192xb(rf, path, kidx, 0x2); /*5,0,0*/

	halrf_wreg(rf, 0x809c, BIT(8), 0x0);

	_edpk_one_shot_8192xb(rf, phy, path, D_MDPK_IDL);

	if (halrf_rreg(rf, 0x80ec, BIT(11)) == 0) {
		_edpk_one_shot_8192xb(rf, phy, path, 0x33);
		_edpk_one_shot_8192xb(rf, phy, path, D_MDPK_LDL);
	}

	is_rek = _edpk_rek_check_8192xb(rf, phy, path, kidx, dpk->edpk_rek_cnt[path][dpk->cur_k_set]);

	while (is_rek == true && dpk->edpk_rek_cnt[path][dpk->cur_k_set] < 5) {
		_edpk_one_shot_8192xb(rf, phy, path, D_MDPK_IDL);
		if (halrf_rreg(rf, 0x80ec, BIT(11)) == 0) {
			_edpk_one_shot_8192xb(rf, phy, path, 0x33);
			_edpk_one_shot_8192xb(rf, phy, path, D_MDPK_LDL);
		}
		is_rek = _edpk_rek_check_8192xb(rf, phy, path, kidx, dpk->edpk_rek_cnt[path][dpk->cur_k_set]);
		dpk->edpk_rek_cnt[path][dpk->cur_k_set]++;
	}

	if (dpk->edpk_rek_cnt[path][dpk->cur_k_set] == 5)
		return true; /*fail*/
	else
		return false;
}

u8 _edpk_order_convert_8192xb(
	struct rf_info *rf)
{
	u8 val;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

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

void _edpk_gs_defalut_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);
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

void _edpk_gain_normalize_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx,
	bool is_execute)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 reg[2][4] = {{0x8190, 0x8194, 0x8198, 0x81a4},
			 {0x81a8, 0x81c4, 0x81c8, 0x81e8}};

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if (is_execute) {
		halrf_wreg(rf, 0x819c + (path << 8), 0x000003FF, 0x200); /*[9:0] pow_cal_start*/
		halrf_wreg(rf, 0x819c + (path << 8), BIT(17) | BIT(16), 0x3); /*pow_cal_len*/

		_edpk_one_shot_8192xb(rf, phy, path, D_GAIN_NORM);
	} else
		halrf_wreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F, 0x5b);

	dpk->bp[path][kidx].gs = (u8)halrf_rreg(rf, reg[kidx][dpk->cur_k_set] + (path << 8), 0x0000007F);
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

void _edpk_on_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path,
	u8 kidx)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	/*coef load*/
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x1);
	halrf_wreg(rf, 0x81dc + (path << 8), BIT(16), 0x0);

	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2),
			BIT(26) | BIT(25) | BIT(24), _edpk_order_convert_8192xb(rf));

	dpk->bp[path][kidx].path_ok = dpk->bp[path][kidx].path_ok | BIT(dpk->cur_k_set);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d] path_ok = 0x%x\n", path, kidx,
		dpk->bp[path][kidx].path_ok);

	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000,
			dpk->bp[path][kidx].path_ok);

	_edpk_gain_normalize_8192xb(rf, phy, path, kidx, false);
	//_edpk_para_query_8192xb(rf, path, kidx);

	halrf_wreg(rf, 0x81b4 + (path << 8) + (kidx << 2), 0x07FC0000, 0x76); /*[26:18] k2*/
}

bool _edpk_reload_check_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	bool is_reload = false;
	u8 idx, cur_band, cur_ch;

//	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	cur_band = rf->hal_com->band[phy].cur_chandef.band;
	cur_ch = rf->hal_com->band[phy].cur_chandef.center_ch;

	for (idx = 0; idx < DPK_BKUP_NUM; idx++) {
		if ((cur_band == dpk->bp[path][idx].band) && (cur_ch == dpk->bp[path][idx].ch)) {
			halrf_wreg(rf, 0x8104 + (path << 8), BIT(8), idx);
			dpk->cur_idx[path] = idx;
			is_reload = true;
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] reload S%d[%d] success\n", path, idx);
		}
	}

	return is_reload;
}

void halrf_edpk_config_para_8192xb(struct rf_info *rf)
{
	u8 path = 0;
	struct halrf_dpk_info *dpk = &rf->dpk;
	
//	RF_DBG(rf, DBG_RF_DPK, "======> %s\n", __func__);

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		dpk->bnd_dpd[path] = halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000);
		dpk->pwsf_df[path] = halrf_rreg(rf, 0x81f0 + (path << 8), 0x1ff);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]bkup edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), dpk->bnd_dpd[path]);
		RF_DBG(rf, DBG_RF_DPK, "[EDPK]bkup pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), dpk->pwsf_df[path]);
	}

	dpk->edpk_dgain_thrs = 0xA00;
	RF_DBG(rf, DBG_RF_DPK, "[EDPK]Dgain threshold = 0x%x\n", dpk->edpk_dgain_thrs);

}

bool _edpk_main_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	enum rf_path path)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 init_xdbm = 24;
	u8 kidx = dpk->cur_idx[path];
	bool is_fail = false;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	_edpk_kip_control_rfc_8192xb(rf, path, false);
	halrf_rf_direct_cntrl_8192xb(rf, path, false); /*switch control to direct write*/
	halrf_wrf(rf, path, 0x10005, MASKRF, 0x03ffd); /*only keep BB control TX_POWER*/

	_edpk_rf_setting_8192xb(rf, path, kidx);
	//halrf_set_rx_dck_8192xb(rf, phy, path, false); /*****due to hw issue****/

	_edpk_kip_pwr_clk_onoff_8192xb(rf, true);
	_edpk_kip_preset_8192xb(rf, phy, path, kidx);
	_edpk_txpwr_bb_force_8192xb(rf, path, true);
	_edpk_kip_set_txagc_8192xb(rf, phy, path, init_xdbm, true);
	_edpk_tpg_sel_8192xb(rf, path, kidx);
#if 0
	is_fail = _edpk_kip_set_rxagc_8192xb(rf, phy, path, kidx);

	if (EDPK_RXSRAM_DBG_8192XB)
		_edpk_read_rxsram_8192xb(rf);

	if (is_fail)
		goto _error;

	_edpk_dgain_read_8192xb(rf);

	if (rf->hal_com->band[phy].cur_chandef.bw < 2)
		_edpk_lbk_rxiqk_8192xb(rf, phy, path, true);
	else
		_edpk_lbk_rxiqk_8192xb(rf, phy, path, false);

	_edpk_gainloss_8192xb(rf, phy, path, kidx);
	//_edpk_para_query_8192xb(rf, path, kidx);
#else
	is_fail = _edpk_agc_8192xb(rf, phy, path, kidx, init_xdbm, false);

	if (is_fail)
		goto _error;
#endif
	/*_edpk_pas_read_8192xb(rf, false);*/
	//_edpk_get_thermal_8192xb(rf, kidx, path);

	//_edpk_para_query_8192xb(rf, path, kidx); //for edpk

	is_fail = _edpk_idl_mpa_8192xb(rf, phy, path, kidx);

	_edpk_para_query_8192xb(rf, path, kidx);

	if (is_fail)
		goto _error;

	//_edpk_kip_control_rfc_8192xb(rf, path, false);
	//_edpk_get_thermal_8192xb(rf, kidx, path);
#if 0
	_edpk_coef_read_8192xb(rf, path, kidx, gain);
#endif
	_edpk_on_8192xb(rf, phy, path, kidx);
_error:
	_edpk_kip_control_rfc_8192xb(rf, path, false);
	halrf_wrf(rf, path, 0x00, MASKRFMODE, RF_STANDBY);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d]_K%d %s\n", path, kidx,
		dpk->cur_k_set, is_fail ? "need Check" : "is Success");

	return is_fail;
}

void _edpk_cal_select_8192xb(
	struct rf_info *rf,
	bool force,
	enum phl_phy_idx phy,
	u8 kpath)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u32 kip_bkup[EDPK_RF_PATH_MAX_8192XB][EDPK_KIP_REG_NUM_8192XB] = {{0}};
	u32 bb_bkup[EDPK_BB_REG_NUM_8192XB] = {0};
	u32 rf_bkup[EDPK_RF_PATH_MAX_8192XB][EDPK_RF_REG_NUM_8192XB] = {{0}};

	u32 kip_reg[] = {0x813c, 0x8124, 0x8120, 0xc0d4, 0xc0d8, 0xc0ec};
	u32 bb_reg[] = {0x5890, 0x5880, 0x5884, 0x7890, 0x7880, 0x7884};
	u32 rf_reg[EDPK_RF_REG_NUM_8192XB] = {0xde, 0xdf, 0x5, 0x10005};

	u8 path;
	bool is_fail = true;
	bool reloaded[EDPK_RF_PATH_MAX_8192XB] = {false};

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if ((!phl_is_mp_mode(rf->phl_com)) && EDPK_RELOAD_EN_8192XB) {
		for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
			reloaded[path] = _edpk_reload_check_8192xb(rf, phy, path);
			if ((reloaded[path] == false) && (dpk->bp[path][0].ch != 0))
				dpk->cur_idx[path] = !dpk->cur_idx[path];
			else
				halrf_edpk_onoff_8192xb(rf, path, false);
		}
	} else {
		for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++)
			dpk->cur_idx[path] = 0;
	}

//	_edpk_bkup_bb_8192xb(rf, bb_reg, bb_bkup);
	_edpk_backup_bb_reg_8192xb(rf, bb_reg, bb_bkup, EDPK_BB_REG_NUM_8192XB);

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		if (kpath & BIT(path)) {
			_edpk_bkup_kip_8192xb(rf, kip_reg, kip_bkup, path);
			_edpk_bkup_rf_8192xb(rf, rf_reg, rf_bkup, path);
			_edpk_information_8192xb(rf, phy, path);		
			_edpk_init_8192xb(rf, path);
		//	if (rf->is_tssi_mode[path])
		//		_edpk_tssi_pause_8192xb(rf, path, true);
#if 0
			dpk->bnd_dpd[path] = halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000);
			dpk->pwsf_df[path] = halrf_rreg(rf, 0x81f0 + (path << 8), 0x1ff);
#endif

			RF_DBG(rf, DBG_RF_DPK, "[EDPK]double chk bkup value edpk bnd_dpd 0x%x[23:18] = 0x%x \n", 0x81ac + (path << 8), dpk->bnd_dpd[path]);
			RF_DBG(rf, DBG_RF_DPK, "[EDPK]double chk bkup value pwsf_df 0x%x[8:0] = 0x%x \n", 0x81f0 + (path << 8), dpk->pwsf_df[path]);

		}
	}

	halrf_edpk_pre_tx_8192xb(rf, phy, 0, 0, 0, 0); 

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		if (kpath & BIT(path)) {
			if (rf->is_tssi_mode[path])
				_edpk_tssi_pause_8192xb(rf, path, true);
			}
	}
	
	_edpk_efem_control_8192xb(rf, true);

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		halrf_wreg(rf, 0x81ac + (path << 8), 0xfc0000, 0x2b); //bnd_dpd
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] edpk bnd_dpd 0x%x[23:18] = 0x%x \n",  0x81ac + (path << 8), halrf_rreg(rf, 0x81ac + (path << 8), 0xfc0000));
	}

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		if (kpath & BIT(path)) {
			RF_DBG(rf, DBG_RF_DPK, "[EDPK] ========= S%d[%d] DPK Start =========\n", path, dpk->cur_idx[path]);
			_edpk_rxagc_onoff_8192xb(rf, path, false);
			halrf_drf_direct_cntrl_8192xb(rf, path, false);
			_edpk_bb_afe_setting_8192xb(rf, path);
			is_fail = _edpk_main_8192xb(rf, phy, path);
			halrf_edpk_onoff_8192xb(rf, path, is_fail);
		}
	}

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		if (kpath & BIT(path)) {
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
			
			_edpk_kip_restore_8192xb(rf, phy, path);
			_edpk_reload_kip_8192xb(rf, kip_reg, kip_bkup, path);
			_edpk_reload_rf_8192xb(rf, rf_reg, rf_bkup, path);
			halrf_wrf(rf, path, 0x5, MASKRF, 0x1);
			_edpk_bb_afe_restore_8192xb(rf, path);
			_edpk_rxagc_onoff_8192xb(rf, path, true);
			if (rf->is_tssi_mode[path])
				_edpk_tssi_pause_8192xb(rf, path, false);
		}
	}
	_edpk_kip_pwr_clk_onoff_8192xb(rf, false);

	_edpk_efem_control_8192xb(rf, false);
//	_edpk_reload_bb_8192xb(rf, bb_reg, bb_bkup);
	_edpk_reload_bb_reg_8192xb(rf, bb_reg, bb_bkup, EDPK_BB_REG_NUM_8192XB);

}

u8 _edpk_bypass_check_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy)
{
	struct halrf_fem_info *fem = &rf->fem;

	u8 result;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if (rf->hal_com->band[phy].cur_chandef.band == BAND_ON_5G) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip DPK due to 5G!!\n");
		result = 1;
	} else if (rf->phl_com->dev_cap.rfe_type < 51) {
		RF_DBG(rf, DBG_RF_DPK, "[EDPK] Skip DPK due to RFE type < 51\n");
		result = 1;
	} else
		result = 0;

	return result;
}

void _edpk_force_bypass_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy)
{
	u8 path, kpath;

	kpath = halrf_kpath_8192xb(rf, phy);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {
		if (kpath & BIT(path))
			halrf_edpk_onoff_8192xb(rf, path, true);
	}
}

void halrf_edpk_8192xb(
	struct rf_info *rf,
	enum phl_phy_idx phy,
	bool force)
{

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);	
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] ****** eDPK Start (Ver: 0x%x, Cv: %d, RFE: %d, RF_para: %d) ******\n",
		EDPK_VER_8192XB, rf->hal_com->cv, rf->phl_com->dev_cap.rfe_type, halrf_get_radio_ver_from_reg(rf));
	RF_DBG(rf, DBG_RF_DPK, "[EDPK] Driver mode = %d\n", rf->phl_com->drv_mode);

	if (_edpk_bypass_check_8192xb(rf, phy))
		_edpk_force_bypass_8192xb(rf, phy);
	else
		_edpk_cal_select_8192xb(rf, force, phy, halrf_kpath_8192xb(rf, phy));
}

void halrf_edpk_onoff_8192xb(
	struct rf_info *rf,
	enum rf_path path,
	bool off)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	bool off_reverse;
	u8 val, kidx = dpk->cur_idx[path];

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]#####======> %s\n", __func__);

	if (off)
		off_reverse = false;
	else
		off_reverse = true;
 
	val = dpk->is_dpk_enable * off_reverse * dpk->bp[path][kidx].path_ok;

	RF_DBG(rf, DBG_RF_DPK, "[EDPK]is_dpk_enable = %d, off_reverse =%d,path_ok[path%d][kidx%d]=%d \n", 
		dpk->is_dpk_enable, off_reverse, path, kidx, dpk->bp[path][kidx].path_ok);

	/*MDPD enable, [31:28] = [k3,k2,k1,k0]*/
	halrf_wreg(rf, 0x81bc + (path << 8) + (kidx << 2), 0xf0000000, val);

	RF_DBG(rf, DBG_RF_DPK, "[EDPK] S%d[%d] DPK %s !!!\n", path, kidx,
		   (val == 0) ? "disable" : "enable");
}

void halrf_edpk_track_8192xb(
	struct rf_info *rf)
{
	struct halrf_dpk_info *dpk = &rf->dpk;
	s8 txagc_cur = 0, delta_txagc = 0;
	u8 ther_cur = 0;
	u8 path = 0, kidx = 0;
	u8 pwsf = 0;

	RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK]#####======> %s\n", __func__);
	
	for (path = 0; path < EDPK_RF_PATH_MAX_8192XB; path++) {

		kidx = dpk->cur_idx[path];
	
		RF_DBG(rf, DBG_RF_DPK_TRACK,
			"[EDPK_TRK]%s ================[S%d[%d] (CH %d)]================\n",
			__func__, path, kidx, dpk->bp[path][kidx].ch);

		txagc_cur = (s8)halrf_rreg(rf, 0x1c60 + (path << 13), 0xFF000000); /*[31:24]*/

		//read thermal
		//ther_cur = (u8)halrf_rreg(rf, 0x81fc + (path << 8), 0x0000003F); /*[5:0]*/
		ther_cur = halrf_get_thermal_8192xb(rf, path);

		delta_txagc = txagc_cur - dpk->bp[path][kidx].txagc_trk_base_edpk;

		

		//limit
		if (delta_txagc > 32) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x and reset delta_txagc to limit 32", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc);

			delta_txagc  = 32;
			
		} else if (delta_txagc < -32) {
			RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x and reset delta_txagc to limit -32", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc);

			delta_txagc  = -32;
		}

#if 0 //default
		if (rf->phl_com->dev_cap.rfe_type == 53)
			pwsf = EDPK_PWSF_DF_TYPE53 - delta_txagc;
		else
			pwsf = EDPK_PWSF_DF - delta_txagc;
#else //morgan 2022/08/18
		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] S%d PWSF Default =0x%x",  
			path, dpk->pwsf_df[path]);

			pwsf = dpk->pwsf_df[path] - delta_txagc;
#endif

		halrf_wreg(rf, 0x81f0 + (path << 8), 0x1ff, pwsf);	 //[8:0]

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] ther_cur =0x%x, ther_dpk = 0x%x",  
			ther_cur, dpk->bp[path][kidx].ther_trk_base_edpk);

		RF_DBG(rf, DBG_RF_DPK_TRACK, "[EDPK_TRK] txagc_cur = 0x%x, txagc_dpk = 0x%x, delta_txagc = 0x%x, pwsf = 0x%x", 
			txagc_cur, dpk->bp[path][kidx].txagc_trk_base_edpk, delta_txagc, pwsf);

	}
}

#endif
