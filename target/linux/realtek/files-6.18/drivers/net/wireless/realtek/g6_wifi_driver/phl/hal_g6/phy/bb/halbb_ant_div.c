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
#include "halbb_precomp.h"

#ifdef HALBB_ANT_DIV_SUPPORT

bool halbb_antdiv_abort(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;

	/* Early return */
	if (bb->pause_ability & BB_ANT_DIV) {
		BB_DBG(bb, DBG_ANT_DIV, "Return ant diversity pause!\n");
		if (bb_ant_div->antdiv_mode == FIX_MAIN_ANT)
			BB_DBG(bb, DBG_ANT_DIV, "Pause Antenna at (( MAIN ))\n");
		else if (bb_ant_div->antdiv_mode == FIX_AUX_ANT)
			BB_DBG(bb, DBG_ANT_DIV, "Pause Antenna at (( AUX ))\n");
		else
			BB_DBG(bb, DBG_ANT_DIV, "Pause Antenna at pre ANT.\n");
		return true;
	}
	if (phl_is_mp_mode(bb->phl_com)) {
		BB_DBG(bb, DBG_ANT_DIV, "Early return - MP mode\n");
		return true;
	}
	if (!(bb->support_ability & BB_ANT_DIV)) {
		BB_DBG(bb, DBG_ANT_DIV, "Early return - Not support antenna diversity\n");
		return true;
	}
	//if (!(bb_link->is_linked) || !(bb_link->is_one_entry_only)) {
	if (!(bb_link->is_linked)) {	
		BB_DBG(bb, DBG_ANT_DIV, "Early return - is_linked=%d, one_entry_only=%d\n",
			  bb_link->is_linked, bb_link->is_one_entry_only);
		return true;
	}
	return false;
}

void halbb_antdiv_reset_training_stat(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_antdiv_evm_info *bb_evm_i = &bb_ant_div->bb_evm_i;
	struct bb_antdiv_rssi_info *bb_rssi_i = &bb_ant_div->bb_rssi_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;

	halbb_mem_set(bb, bb_rate_i, 0, sizeof(struct bb_antdiv_rate_info));
	halbb_mem_set(bb, bb_cn_i, 0, sizeof(struct bb_antdiv_cn_info));
	halbb_mem_set(bb, bb_evm_i, 0, sizeof(struct bb_antdiv_evm_info));
	halbb_mem_set(bb, bb_rssi_i, 0, sizeof(struct bb_antdiv_rssi_info));

}

void halbb_antdiv_reset(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_antdiv_evm_info *bb_evm_i = &bb_ant_div->bb_evm_i;

	/* Reset stat */
	halbb_antdiv_reset_training_stat(bb);
}

void halbb_antdiv_reg_init(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_antdiv_cr_info *cr = &bb->bb_ant_div_i.bb_antdiv_cr_i;
	struct rtw_phl_com_t *phl = bb->phl_com;
	struct dev_cap_t *dev = &phl->dev_cap;

	/* dis r_ant_train_en */
	halbb_set_reg_cmn(bb, cr->path0_r_ant_train_en, cr->path0_r_ant_train_en_m, 0x0, HW_PHY_0);

	/* r_tx_ant_sel by BB wrapper or r_tx_antidx */
	if (bb_ant_div->tx_ant_by_bbwrapper == true || dev->rfe_type > 50)
		halbb_set_reg_cmn(bb, cr->path0_r_tx_ant_sel, cr->path0_r_tx_ant_sel_m, 0x1, HW_PHY_0);
	else
		halbb_set_reg_cmn(bb, cr->path0_r_tx_ant_sel, cr->path0_r_tx_ant_sel_m, 0x0, HW_PHY_0);

#if (defined(HALBB_COMPILE_AP_SERIES) || defined(HALBB_COMPILE_AP2_SERIES))
	/* r_trsw_tx_extend = 0us */
	halbb_set_reg_cmn(bb, 0x728, 0xf, 0x0, HW_PHY_0);
	/* dis r_hw_antsw_dis_by_gnt_bt */
	halbb_set_reg_cmn(bb, 0x728, BIT(12), 0x0, HW_PHY_0);
	/* enable condition number cal*/
	halbb_set_reg_cmn(bb, cr->sigval_rpt_en, cr->sigval_rpt_en_m, 0x1, HW_PHY_0);
#endif

	/* dis r_bt_force_en */
	halbb_set_reg_cmn(bb, cr->path0_r_bt_force_antidx_en, cr->path0_r_bt_force_antidx_en_m, 0x0, HW_PHY_0);

	/* r_rfsw_ctrl_antenna (Antenna mapping) */
	halbb_set_reg_cmn(bb, cr->path0_r_rfsw_ant_31_0, 0xFFFF, 0x0100, HW_PHY_0);

	/* dis r_BB_SEL_BTG_TRX_S */
	halbb_set_reg_cmn(bb, cr->path0_r_antsel, BIT(21), 0x1, HW_PHY_0);

	/* "antsel" is controlled by HWs*/
	halbb_set_reg_cmn(bb, cr->path0_r_antsel, BIT(16), 0x0, HW_PHY_0);
	/* r_ANT_DIV_SW_2G_S, 2G "CS/CG switching" is controlled by HWs */
	halbb_set_reg_cmn(bb, cr->path0_r_antsel, BIT(23), 0x0, HW_PHY_0);
	/* r_ANT_DIV_SW_5G_S, 5G "CS/CG switching" is controlled by HWs */
	halbb_set_reg_cmn(bb, cr->path0_r_antsel, BIT(25), 0x0, HW_PHY_0);

#if 0
	/* 92XB efem one pin control switch */
	if (bb->ic_sub_type == BB_IC_SUB_TYPE_8192XB_8192XB && dev->rfe_type > 50) {
		rtw_hal_mac_set_gpio_func(bb->hal_com, RTW_MAC_GPIO_WL_RFE_CTRL, 12);
		halbb_gpio_setting(bb, 12, BB_PATH_A, true, ZERO); // default main -> main is high, aux is low
	}
#endif
	/* 32BR one pin control switch */
	/*if (bb->ic_type == BB_RTL8192XB && bb->bb_api_i.band == BAND_ON_5G) {
		halbb_gpio_setting_8192xb(bb, 0, BB_PATH_A, false, ANTSEL_0);
	}*/
}


u8 halbb_ulmacid_cfg_fixed(struct bb_info *bb, u8 ss, u8 rate, u8 bw, u8 ulofdma_en, u8 macid, u8 cont_tx)
{
	u8 ret = RTW_HAL_STATUS_FAILURE;
	u8 len = sizeof(struct rtw_ul_macid_set);
	//struct halbb_ul_macid_set *ulmac_i;
	u8 pkt_len = sizeof(struct halbb_ul_macid_set);
	struct rtw_phl_ax_ul_fixinfo tbl_b;
	u32 *bb_h2c = NULL;
	u8 i = 0;
	bool ret_v = false;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	//extern enum rtw_hal_status rtw_hal_mac_set_fw_ul_fixinfo(void *hal,struct rtw_phl_ax_ul_fixinfo *tbl);

	//BB_DBG(bb, DBG_ANT_DIV, "Trigger Frame Rate =%d\n", rate);
	if (bb_ant_div->tb_antdiv_train_en == 0)
		return ret;

	halbb_mem_set(bb, &tbl_b, 0, sizeof(struct rtw_phl_ax_ul_fixinfo));

	tbl_b.sta[0].macid = macid;
	tbl_b.ulrua.sta[0].mac_id = macid;
	tbl_b.ulrua.sta[0].coding = 1; //LDPC
	tbl_b.ulrua.sta[0].rate.mcs = rate;
	tbl_b.ulrua.sta[0].rate.ss = ss;
	
	tbl_b.ulrua.sta[0].tgt_rssi = 70;
	tbl_b.ulrua.sta_num = 1;
	
	//cfg
	if (ulofdma_en == 1)
		tbl_b.cfg.mode = 0x1;
	else
		tbl_b.cfg.mode = 0x0;


	//continous	TB 0x1, one shot 0x0
	if (cont_tx == 1)
		tbl_b.cfg.storemode = 0x1;
	else
		tbl_b.cfg.storemode = 0x0;

	tbl_b.store_idx = 0x0;
	tbl_b.ulfix_usage = 0x3;
	tbl_b.cfg.interval = 0x5; //ms

	// tf
	tbl_b.data_rate = 0x8;
	tbl_b.data_bw = 0x0;
	tbl_b.gi_ltf = 0x0;
	tbl_b.tf_type = 0x1;

	//common
	tbl_b.ulrua.gi_ltf = 0x0;
	tbl_b.ulrua.n_ltf_and_ma = 0x0;
	tbl_b.apep_len = 0x1;
	
	tbl_b.ulrua.ppdu_bw = bw;
	if (bw == 2) {	
		tbl_b.ulrua.sta[0].ru_pos = RTW_HE_RU996_1*2;
	} else if (bw == 1 ){
		tbl_b.ulrua.sta[0].ru_pos = RTW_HE_RU484_1*2;
	} else if (bw == 0){
		tbl_b.ulrua.sta[0].ru_pos = RTW_HE_RU242_1*2;
	}
//#ifdef CONFIG_RTL8852D
	ret = rtw_hal_mac_set_fw_ul_fixinfo(bb->hal_com->hal_priv, &tbl_b);
//#endif
	return ret;
}

void halbb_antdiv_init(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct rtw_phl_com_t *phl = bb->phl_com;
	struct dev_cap_t *dev = &phl->dev_cap;
	struct bb_physts_rslt_1_info *psts_1 = &bb->bb_physts_i.bb_physts_rslt_1_i;
	enum channel_width rx_bw = psts_1->bw_idx;

	BB_DBG(bb, DBG_ANT_DIV, "%s ======>\n", __func__);
	bb_ant_div->tx_ant_by_bbwrapper = false;
	if (bb->support_ability & BB_ANT_DIV || dev->antdiv_sup == true) {
		/* HW reg. init to set mux & ctrler for antdiv */
		halbb_antdiv_reg_init(bb);
	}

	/* Mode setting*/
	bb_ant_div->antdiv_mode = AUTO_ANT;
	bb_ant_div->pre_antdiv_mode = AUTO_ANT;
	bb_ant_div->antdiv_method = TB_BASED_ANTDIV;
	bb_ant_div->tp_decision_method = TP_HIGHEST_DOMINATION;
	bb_ant_div->evm_decision_method = EVM_LINEAR_AVG;
	bb_ant_div->tb_antdiv_train_rate = 1;
	bb_ant_div->tb_antdiv_train_bw = rx_bw;
	bb_ant_div->tb_antdiv_train_num = 1;
	bb_ant_div->tb_antdiv_train_en = 1;
	bb_ant_div->tb_antdiv_rssi_diff = 8;
	bb_ant_div->tb_antdiv_cn_diff = 1;
	bb_ant_div->tb_antdiv_timeout = 20;
	bb_ant_div->tb_antdiv_train_next = 0;
	bb_ant_div->one_path_CL_pass_WD_lim = 10;
	bb_ant_div->two_path_CL_pass_WD_lim = 3;
	bb_ant_div->one_path_CL_fix_ant_rssi_diff = 30;
	bb_ant_div->two_path_CL_fix_ant_rssi_diff = 10;
	// bb_ant_div->CL_fix_ant_TP_diff = 4; //5:0.03125; 4:0.0625

	bb_ant_div->multista_antdiv_ra_rdy_delay = 10;
	bb_ant_div->multista_tp_diff = 10;

	/* Algorithm parameter setting */
	bb_ant_div->antdiv_period = ANTDIV_PERIOD;
	bb_ant_div->antdiv_train_num = ANTDIV_TRAINING_NUM;
	if (bb->ic_type == BB_RTL8852C)
		//bb_ant_div->antdiv_delay = ANTDIV_DELAY_2;
		bb_ant_div->antdiv_delay = 15;
	else
		bb_ant_div->antdiv_delay = ANTDIV_DELAY;
	bb_ant_div->antdiv_intvl = ANTDIV_INTVL;
	bb_ant_div->tp_diff_th_high = ANTDIV_DEC_TP_HIGH;
	bb_ant_div->tp_diff_th_low = ANTDIV_DEC_TP_LOW;
	bb_ant_div->evm_diff_th = ANTDIV_DEC_EVM;
	bb_ant_div->tp_lb = TP_LOWER_BOUND;

	/* variable init */
	bb_ant_div->antdiv_wd_cnt = 0;
	bb_ant_div->antdiv_training_state_cnt = 0;
	bb_ant_div->get_stats = false;
	bb_ant_div->antdiv_use_ctrl_frame = true;


	bb_ant_div->target_ant = ANTDIV_INIT;
	bb_ant_div->target_ant_evm = ANTDIV_INIT;
	bb_ant_div->target_ant_tp = ANTDIV_INIT;
	bb_ant_div->training_ant = ANTDIV_INIT;
	bb_ant_div->pre_target_ant = ANTDIV_INIT;

	/* use new ant_div code*/
	bb_ant_div->ant_div_new = true;

	//hal_write32(bb->hal_com, 0x2d4, 0x22228fff);
	halbb_antdiv_reset(bb);

	BB_DBG(bb, DBG_INIT, "Init ant_diversity timer");
}

u8 halbb_antdiv_get_targetant(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;

	if (bb_ant_div->multista_antdiv == 1){
		return bb_ant_div->target_ant_tp;
	}else if (bb_ant_div->antdiv_method == TB_BASED_ANTDIV){
		return bb_ant_div->target_ant_cn;
	}else{
		return bb_ant_div->target_ant;
	}
}

void halbb_antdiv_set_ant(struct bb_info *bb, u8 ant)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_antdiv_cr_info *cr = &bb->bb_ant_div_i.bb_antdiv_cr_i;
	struct rtw_hal_com_t *hal = bb->hal_com;
	struct rtw_phl_com_t *phl = bb->phl_com;
	struct dev_cap_t *dev = &phl->dev_cap;
	u8 band = bb->hal_com->band[0].cur_chandef.band;
	u8 default_ant, optional_ant;
	
	if ((bb_ant_div->pre_target_ant != ant) || (bb_ant_div->training_ant != ant) ) {
		BB_DBG(bb, DBG_ANT_DIV, "Set Antenna =%s\n",
			  (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		if(ant == MAIN_ANT) {
			default_ant = ANT1_2G;
			optional_ant = ANT2_2G;
			if (bb->ic_sub_type == BB_IC_SUB_TYPE_8192XB_8192XB && dev->rfe_type == 50) {
				//halbb_write_rf_reg(bb, RF_PATH_A, 0x2, 0x20000, 0x0);
				//halbb_write_rf_reg(bb, RF_PATH_A, 0x2, 0x8000, 0x0);
				rtw_hal_rf_set_ant_main_or_aux(hal->hal_priv, RF_PATH_A, true);
			} else if (bb_ant_div->tx_ant_by_bbwrapper == false) {
				if (bb->ic_sub_type == BB_IC_SUB_TYPE_8192XB_8192XB && dev->rfe_type > 50) {
					halbb_gpio_setting(bb, 12, BB_PATH_A, true, ZERO); // main is high, aux is low
				} else if (bb->ic_type == BB_RTL8852C) {
					halbb_gpio_setting(bb, 11, BB_PATH_A, true, ZERO);
				}
			}
		} else {
			default_ant = ANT2_2G;
			optional_ant = ANT1_2G;
			if (bb->ic_sub_type == BB_IC_SUB_TYPE_8192XB_8192XB && dev->rfe_type == 50) {
				//halbb_write_rf_reg(bb, RF_PATH_A, 0x2, 0x20000, 0x1);
				//halbb_write_rf_reg(bb, RF_PATH_A, 0x2, 0x8000, 0x1);
				rtw_hal_rf_set_ant_main_or_aux(hal->hal_priv, RF_PATH_A, false);
			} else if (bb_ant_div->tx_ant_by_bbwrapper == false) {
				if (bb->ic_sub_type == BB_IC_SUB_TYPE_8192XB_8192XB && dev->rfe_type > 50) {
					halbb_gpio_setting(bb, 12, BB_PATH_A, false, ZERO); // main is high, aux is low
				} else if (bb->ic_type == BB_RTL8852C) {
					halbb_gpio_setting(bb, 11, BB_PATH_A, false, ZERO);
				}
			}
		}

		/* Original Rx antenna */
		halbb_set_reg_cmn(bb, cr->path0_r_antsel, 0x20000, default_ant, HW_PHY_0);
		halbb_set_reg_cmn(bb, cr->path0_r_antsel, 0xf0, default_ant, HW_PHY_0);
		/* Alternative Rx antenna */
		halbb_set_reg_cmn(bb, cr->path0_r_antsel, 0xf00, optional_ant, HW_PHY_0);
		if (bb_ant_div->tx_ant_by_bbwrapper == false)
			/* Tx antenna, same as orig. rx ant. */
			halbb_set_reg_cmn(bb, cr->path0_r_antsel, 0xf000, default_ant, HW_PHY_0);
		#if 0
		else
			halbb_bb_wrap_set_ant(bb, macid, default_ant);
		#endif
		//if (bb->ic_type == BB_RTL8852A)
			//rtw_hal_rf_rx_ant(hal, ant);
		//BB_TRACE("Set Antenna =%s Done\n",(ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

	} else {
		BB_DBG(bb, DBG_ANT_DIV, "Stay in Ori-ant\n");
	}
}

void halbb_set_antdiv_pause_val(struct bb_info *bb, u32 *val_buf, u8 val_len)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_antdiv_cr_info *cr = &bb->bb_ant_div_i.bb_antdiv_cr_i;
	u8 i = 0, pause_result = 0;
	u32 val = 0;

	if (val_len != 1) {
		BB_DBG(bb, DBG_ANT_DIV, "[Error][AntDiv]Need val_len=1\n");
		return;
	}
	BB_DBG(bb, DBG_ANT_DIV, "[%s] len=%d, val[0]=0x%x\n", __func__, val_len, val_buf[0]);

	 if (val_buf[0] == MAIN_ANT) {
	 	BB_DBG(bb, DBG_ANT_DIV, "[Pause Antiv in Main ANT.\n");
		halbb_antdiv_set_ant(bb, MAIN_ANT);
		bb_ant_div->antdiv_mode = FIX_MAIN_ANT;
		bb_ant_div->target_ant = MAIN_ANT;
		bb_ant_div->pre_target_ant = MAIN_ANT;
	} else if (val_buf[0] == AUX_ANT) {
		BB_DBG(bb, DBG_ANT_DIV, "[Pause Antiv in AUX ANT.\n");
		halbb_antdiv_set_ant(bb, AUX_ANT);
		bb_ant_div->antdiv_mode = FIX_AUX_ANT;
		bb_ant_div->target_ant = AUX_ANT;
		bb_ant_div->pre_target_ant = AUX_ANT;
	} else {
		BB_DBG(bb, DBG_ANT_DIV, "[Pause Antiv in pre ANT.\n");
		bb_ant_div->antdiv_mode = AUTO_ANT;
	}
}

void halbb_antdiv_get_rssi(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rssi_info *rssi = &bb_ant_div->bb_rssi_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;

	rssi->rssi_cck_avg = (u8)HALBB_DIV(rssi->rssi_cck_avg_acc, rssi->pkt_cnt_cck);
	rssi->rssi_ofdm_avg = (u8)HALBB_DIV(rssi->rssi_ofdm_avg_acc, rssi->pkt_cnt_ofdm);
	rssi->rssi_t_avg = (u8)HALBB_DIV(rssi->rssi_t_avg_acc, rssi->pkt_cnt_t);


	if (rate_i->mode == BB_LEGACY_MODE) {
		if (cmn_rpt->is_cck_rate) {
			rssi->rssi_final = rssi->rssi_cck_avg;
		} else {
			rssi->rssi_final = rssi->rssi_ofdm_avg;
		}
	} else {
		rssi->rssi_final = rssi->rssi_t_avg;
	}
}


void halbb_antdiv_get_cn_target_ant(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	u32 main_cn, aux_cn;
	u8 target_ant_cn;

	/* CN */
	main_cn =  (u8)HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt);
	aux_cn =  (u8)HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt);

	BB_TRACE("Main    PKT CNT = %d, AVG_CN = (%02d.%03d) \n", bb_cn_i->main_cn_pkt_cnt, main_cn>>1,halbb_show_fraction_num(main_cn & 0x1, 1));
	BB_TRACE("Aux     PKT CNT = %d, AVG_CN =  (%02d.%03d) \n", bb_cn_i->aux_cn_pkt_cnt, aux_cn>>1,halbb_show_fraction_num(aux_cn & 0x1, 1));
	//BB_TRACE("Total   PKT CNT = %d , physts pkt cnt = %d\n", bb_cn_i->aux_cn_pkt_cnt+bb_cn_i->main_cn_pkt_cnt-1, pkt_cnt->pkt_cnt_all);
	//BB_TRACE("Invalid PKT CNT = %d \n", bb_cn_i->cn_invalid_cnt);	
	
	if (bb_cn_i->aux_cn_pkt_cnt == 0)
		//target_ant_cn = MAIN_ANT;
		return;
	else if (bb_cn_i->main_cn_pkt_cnt == 0)
		//target_ant_cn = AUX_ANT;
		return;
	else
		target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);

	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d)\n", "[Main-Ant CN_avg]",
	       (main_cn >> 1),
	       halbb_show_fraction_num(main_cn & 0x1, 1));

	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d)\n", "[Aux-Ant CN_avg]",
	       (aux_cn >> 1),
	       halbb_show_fraction_num(aux_cn & 0x1, 1));

	bb_ant_div->target_ant_cn = target_ant_cn;
	BB_DBG(bb, DBG_ANT_DIV, "CN based TargetAnt= [%s]\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	BB_TRACE("CN based TargetAnt= [%s], total pkt cnt = %d\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT", bb_cn_i->aux_cn_pkt_cnt+bb_cn_i->main_cn_pkt_cnt);
}

void halbb_antdiv_get_highest_mcs_old(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;

	u16 main_max_cnt = 1;
	u16 aux_max_cnt = 1;
	u16 main_max_idx = 0;
	u16 aux_max_idx = 0;
	u8 ss_ofst = 0;
	u8 i;

	BB_DBG(bb, DBG_ANT_DIV, "*Main-Ant CCK    cnt:{%d, %d, %d, %d}\n",
	       bb_rate_i->main_pkt_cnt_legacy[0], bb_rate_i->main_pkt_cnt_legacy[1],
	       bb_rate_i->main_pkt_cnt_legacy[2], bb_rate_i->main_pkt_cnt_legacy[3]);

	BB_DBG(bb, DBG_ANT_DIV, "*Aux-Ant CCK    cnt:{%d, %d, %d, %d}\n",
	       bb_rate_i->aux_pkt_cnt_legacy[0], bb_rate_i->aux_pkt_cnt_legacy[1],
	       bb_rate_i->aux_pkt_cnt_legacy[2], bb_rate_i->aux_pkt_cnt_legacy[3]);

	BB_DBG(bb, DBG_ANT_DIV, "*Main-Ant OFDM    cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
	       bb_rate_i->main_pkt_cnt_legacy[4], bb_rate_i->main_pkt_cnt_legacy[5],
	       bb_rate_i->main_pkt_cnt_legacy[6], bb_rate_i->main_pkt_cnt_legacy[7],
	       bb_rate_i->main_pkt_cnt_legacy[8], bb_rate_i->main_pkt_cnt_legacy[9],
	       bb_rate_i->main_pkt_cnt_legacy[10], bb_rate_i->main_pkt_cnt_legacy[11]);

	BB_DBG(bb, DBG_ANT_DIV, "*Aux-Ant OFDM    cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
	       bb_rate_i->aux_pkt_cnt_legacy[4], bb_rate_i->aux_pkt_cnt_legacy[5],
	       bb_rate_i->aux_pkt_cnt_legacy[6], bb_rate_i->aux_pkt_cnt_legacy[7],
	       bb_rate_i->aux_pkt_cnt_legacy[8], bb_rate_i->aux_pkt_cnt_legacy[9],
	       bb_rate_i->aux_pkt_cnt_legacy[10], bb_rate_i->aux_pkt_cnt_legacy[11]);

	if ((bb_rate_i->main_he_pkt_not_zero == true) || (bb_rate_i->aux_he_pkt_not_zero == true)) {
		for (i = 0; i < HE_RATE_NUM; i++) {

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_he[i] >= main_max_cnt) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_he[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_he[i] >= aux_max_cnt) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_he[i];
					aux_max_idx = i;
				}
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_he[i] > bb_ant_div->tp_lb) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_he[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_he[i] > bb_ant_div->tp_lb) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_he[i];
					aux_max_idx = i;
				}
			}
		}

		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant HE %d-SS cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 11]);

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant HE %d-SS cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 11]);

		}

	} else if ((bb_rate_i->main_vht_pkt_not_zero == true) || (bb_rate_i->aux_vht_pkt_not_zero == true)) {

		for (i = 0; i < VHT_RATE_NUM; i++) {

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_vht[i] >= main_max_cnt) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_vht[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_vht[i] >= aux_max_cnt) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_vht[i];
					aux_max_idx = i;
				}
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_vht[i] > bb_ant_div->tp_lb) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_vht[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_vht[i] > bb_ant_div->tp_lb) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_vht[i];
					aux_max_idx = i;
				}
			}
		}


		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant VHT %d-SS cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 11]);

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant VHT %d-SS cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 11]);
		}

	} else if ((bb_rate_i->main_ht_pkt_not_zero == true) || (bb_rate_i->aux_ht_pkt_not_zero == true)) {
		for (i = 0; i < HT_RATE_NUM; i++) {

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_ht[i] >= main_max_cnt) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_ht[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_ht[i] >= aux_max_cnt) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_ht[i];
					aux_max_idx = i;
				}
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				if (bb_rate_i->main_pkt_cnt_ht[i] > bb_ant_div->tp_lb) {
					main_max_cnt = bb_rate_i->main_pkt_cnt_ht[i];
					main_max_idx = i;
				}
				if (bb_rate_i->aux_pkt_cnt_ht[i] > bb_ant_div->tp_lb) {
					aux_max_cnt = bb_rate_i->aux_pkt_cnt_ht[i];
					aux_max_idx = i;
				}
			}
		}

		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = (i << 3);

			BB_DBG(bb, DBG_ANT_DIV,
			       "*HT%02d:%02d cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 0],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 1],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 2],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 3],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 4],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 5],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 6],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 7]);

			BB_DBG(bb, DBG_ANT_DIV,
			       "*HT%02d:%02d cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 0],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 1],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 2],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 3],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 4],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 5],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 6],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 7]);
		}

	}

	/* Compute all throughput*/
	if (bb_ant_div->tp_decision_method == TP_AVG_DOMINATION) {
		BB_DBG(bb, DBG_ANT_DIV, "Main_tp = %d, Aux_tp = %d\n", 
					 bb_rate_i->main_tp, bb_rate_i->aux_tp);

		if (bb_rate_i->main_tp > bb_rate_i->aux_tp) {
			bb_ant_div->target_ant_tp = MAIN_ANT;
			bb_rate_i->tp_diff = bb_rate_i->main_tp - bb_rate_i->aux_tp;
		} else if (bb_rate_i->main_tp < bb_rate_i->aux_tp) {
			bb_ant_div->target_ant_tp = AUX_ANT;
			bb_rate_i->tp_diff = bb_rate_i->aux_tp - bb_rate_i->main_tp;
		} else {
			bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
			bb_rate_i->no_change_flag = true;
			BB_DBG(bb, DBG_ANT_DIV, "TP based TargetAnt= Pre-TargetAnt\n");
		}

		BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= [%s]\n", 
		(bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		BB_DBG(bb, DBG_ANT_DIV, "TP Confidence= [%d]\n", bb_rate_i->tp_diff);

		return;
	}

	bb_rate_i->main_max_cnt = main_max_cnt;
	bb_rate_i->main_max_idx = main_max_idx;
	bb_rate_i->aux_max_cnt = aux_max_cnt;
	bb_rate_i->aux_max_idx = aux_max_idx;

	/* Decision Tput based target ant using MCS rate instead of phy data rate */
	if ((main_max_cnt > bb_ant_div->tp_lb) || (aux_max_cnt > bb_ant_div->tp_lb)) {
		if (main_max_idx > aux_max_idx) {
			bb_ant_div->target_ant_tp = MAIN_ANT;
			bb_rate_i->tp_diff = 100;
		} else if (main_max_idx < aux_max_idx) {
			bb_ant_div->target_ant_tp = AUX_ANT;
			bb_rate_i->tp_diff = 100;
		} else {
			if (main_max_cnt > aux_max_cnt)
				bb_ant_div->target_ant_tp = MAIN_ANT;
			else if (main_max_cnt < aux_max_cnt)
				bb_ant_div->target_ant_tp = AUX_ANT;
			else {
				bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
				bb_rate_i->no_change_flag = true;
			}
			/* Calc. TP confidence*/
			bb_rate_i->tp_diff = DIFF_2(main_max_cnt, aux_max_cnt);
		}
	} else {
		bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
		bb_rate_i->no_change_flag = true;
		BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= Pre-TargetAnt\n");
	}

	if (bb_rate_i->no_change_flag == true)
		bb_rate_i->tp_diff = 0;

	if (bb_rate_i->tp_diff > 100)
		bb_rate_i->tp_diff = 100;

	BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= [%s]\n", 
		(bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	BB_DBG(bb, DBG_ANT_DIV, "TP Confidence= [%d]\n", bb_rate_i->tp_diff);
}

void halbb_antdiv_get_highest_mcs_new(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_sub_bw_tp_info *bb_sub_bw_tp = &bb_ant_div->bb_sub_bw_tp_i;
	u8 phy = bb->bb_phy_idx == HW_PHY_1 ? 1 : 0;
	enum channel_width main_max_bw = bb->hal_com->band[phy].cur_chandef.bw;
	enum channel_width aux_max_bw = bb->hal_com->band[phy].cur_chandef.bw;

	u16 main_max_cnt = 1;
	u16 aux_max_cnt = 1;
	u16 main_max_idx = 0;
	u16 aux_max_idx = 0;
	u8 ss_ofst = 0;
	u8 i, j;

	Cnt_info cnt_info[4] = {
		{&bb_sub_bw_tp->main_max_tp_20, &bb_sub_bw_tp->aux_max_tp_20, &bb_sub_bw_tp->main_max_cnt_20, &bb_sub_bw_tp->aux_max_cnt_20, &bb_sub_bw_tp->main_max_idx_20, &bb_sub_bw_tp->aux_max_idx_20},
		{&bb_sub_bw_tp->main_max_tp_40, &bb_sub_bw_tp->aux_max_tp_40, &bb_sub_bw_tp->main_max_cnt_40, &bb_sub_bw_tp->aux_max_cnt_40, &bb_sub_bw_tp->main_max_idx_40, &bb_sub_bw_tp->aux_max_idx_40},
		{&bb_sub_bw_tp->main_max_tp_80, &bb_sub_bw_tp->aux_max_tp_80, &bb_sub_bw_tp->main_max_cnt_80, &bb_sub_bw_tp->aux_max_cnt_80, &bb_sub_bw_tp->main_max_idx_80, &bb_sub_bw_tp->aux_max_idx_80},
		{&bb_sub_bw_tp->main_max_tp_160, &bb_sub_bw_tp->aux_max_tp_160, &bb_sub_bw_tp->main_max_cnt_160, &bb_sub_bw_tp->aux_max_cnt_160, &bb_sub_bw_tp->main_max_idx_160, &bb_sub_bw_tp->aux_max_idx_160}
	};

	/*reset sub BW TP info*/
	halbb_mem_set(bb, bb_sub_bw_tp, 0, sizeof(struct bb_sub_bw_tp_info));

	BB_DBG(bb, DBG_ANT_DIV, "*Main-Ant CCK    cnt:{%d, %d, %d, %d}\n",
	       bb_rate_i->main_pkt_cnt_legacy[0], bb_rate_i->main_pkt_cnt_legacy[1],
	       bb_rate_i->main_pkt_cnt_legacy[2], bb_rate_i->main_pkt_cnt_legacy[3]);

	BB_DBG(bb, DBG_ANT_DIV, "*Aux-Ant CCK    cnt:{%d, %d, %d, %d}\n",
	       bb_rate_i->aux_pkt_cnt_legacy[0], bb_rate_i->aux_pkt_cnt_legacy[1],
	       bb_rate_i->aux_pkt_cnt_legacy[2], bb_rate_i->aux_pkt_cnt_legacy[3]);

	BB_DBG(bb, DBG_ANT_DIV, "*Main-Ant OFDM    cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
	       bb_rate_i->main_pkt_cnt_legacy[4], bb_rate_i->main_pkt_cnt_legacy[5],
	       bb_rate_i->main_pkt_cnt_legacy[6], bb_rate_i->main_pkt_cnt_legacy[7],
	       bb_rate_i->main_pkt_cnt_legacy[8], bb_rate_i->main_pkt_cnt_legacy[9],
	       bb_rate_i->main_pkt_cnt_legacy[10], bb_rate_i->main_pkt_cnt_legacy[11]);

	BB_DBG(bb, DBG_ANT_DIV, "*Aux-Ant OFDM    cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
	       bb_rate_i->aux_pkt_cnt_legacy[4], bb_rate_i->aux_pkt_cnt_legacy[5],
	       bb_rate_i->aux_pkt_cnt_legacy[6], bb_rate_i->aux_pkt_cnt_legacy[7],
	       bb_rate_i->aux_pkt_cnt_legacy[8], bb_rate_i->aux_pkt_cnt_legacy[9],
	       bb_rate_i->aux_pkt_cnt_legacy[10], bb_rate_i->aux_pkt_cnt_legacy[11]);

	if ((bb_rate_i->main_he_pkt_not_zero == true) || (bb_rate_i->aux_he_pkt_not_zero == true)) {
		for (i = 0; i < HE_RATE_NUM; i++) {
			bb_sub_bw_tp->COUNT_MAIN_HE[0][i] = bb_rate_i->main_pkt_cnt_he_sc20[i];
			bb_sub_bw_tp->COUNT_MAIN_HE[1][i] = bb_rate_i->main_pkt_cnt_he_sc40[i];
			bb_sub_bw_tp->COUNT_MAIN_HE[2][i] = bb_rate_i->main_pkt_cnt_he_sc80[i];
			bb_sub_bw_tp->COUNT_MAIN_HE[3][i] = bb_rate_i->main_pkt_cnt_he[i];
			bb_sub_bw_tp->COUNT_AUX_HE[0][i] = bb_rate_i->aux_pkt_cnt_he_sc20[i];
			bb_sub_bw_tp->COUNT_AUX_HE[1][i] = bb_rate_i->aux_pkt_cnt_he_sc40[i];
			bb_sub_bw_tp->COUNT_AUX_HE[2][i] = bb_rate_i->aux_pkt_cnt_he_sc80[i];
			bb_sub_bw_tp->COUNT_AUX_HE[3][i] = bb_rate_i->aux_pkt_cnt_he[i];

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				for(j = 0; j < 4; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_HE[j][i] >= main_max_cnt) {
						main_max_cnt = bb_sub_bw_tp->COUNT_MAIN_HE[j][i];
						main_max_idx = i;
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_HE[j][i] >= aux_max_cnt) {
						aux_max_cnt = bb_sub_bw_tp->COUNT_AUX_HE[j][i];
						aux_max_idx = i;
						aux_max_bw = j;
					}
				}
				BB_DBG(bb, DBG_ANT_DIV,
				  "(HE) Main ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  main_max_bw, main_max_idx, main_max_cnt);
				BB_DBG(bb, DBG_ANT_DIV,
				  "(HE) Aux ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  aux_max_bw, aux_max_idx, aux_max_cnt);
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				const int MULTIPLIERS_HE[4] = {113, 226, 474, 947};

				for(j = 0; j < 4; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_HE[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_main_max_cnt) = bb_sub_bw_tp->COUNT_MAIN_HE[j][i];
						*(cnt_info[j].sub_main_max_idx) = i;
						*(cnt_info[j].sub_main_max_tp) = 
							MULTIPLIERS_HE[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_HE[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_aux_max_cnt) = bb_sub_bw_tp->COUNT_AUX_HE[j][i];
						*(cnt_info[j].sub_aux_max_idx) = i;
						*(cnt_info[j].sub_aux_max_tp) = 
							MULTIPLIERS_HE[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						aux_max_bw = j;
					}
				}
			}
		}

		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant HE %d-SS BW-160 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_he[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant HE %d-SS BW-80 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_he_sc80[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant HE %d-SS BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_he_sc40[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant HE %d-SS BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_he_sc20[ss_ofst + 11]);

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant HE %d-SS BW-160 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_he[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant HE %d-SS BW-80 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_he_sc80[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant HE %d-SS BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_he_sc40[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant HE %d-SS BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_he_sc20[ss_ofst + 11]);

		}

	} else if ((bb_rate_i->main_vht_pkt_not_zero == true) || (bb_rate_i->aux_vht_pkt_not_zero == true)) {

		for (i = 0; i < VHT_RATE_NUM; i++) {
			bb_sub_bw_tp->COUNT_MAIN_VHT[0][i] = bb_rate_i->main_pkt_cnt_vht_sc20[i];
			bb_sub_bw_tp->COUNT_MAIN_VHT[1][i] = bb_rate_i->main_pkt_cnt_vht_sc40[i];
			bb_sub_bw_tp->COUNT_MAIN_VHT[2][i] = bb_rate_i->main_pkt_cnt_vht_sc80[i];
			bb_sub_bw_tp->COUNT_MAIN_VHT[3][i] = bb_rate_i->main_pkt_cnt_vht[i];
			bb_sub_bw_tp->COUNT_AUX_VHT[0][i] = bb_rate_i->aux_pkt_cnt_vht_sc20[i];
			bb_sub_bw_tp->COUNT_AUX_VHT[1][i] = bb_rate_i->aux_pkt_cnt_vht_sc40[i];
			bb_sub_bw_tp->COUNT_AUX_VHT[2][i] = bb_rate_i->aux_pkt_cnt_vht_sc80[i];
			bb_sub_bw_tp->COUNT_AUX_VHT[3][i] = bb_rate_i->aux_pkt_cnt_vht[i];

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				for(j = 0; j < 4; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_VHT[j][i] >= main_max_cnt) {
						main_max_cnt = bb_sub_bw_tp->COUNT_MAIN_VHT[j][i];
						main_max_idx = i;
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_VHT[j][i] >= aux_max_cnt) {
						aux_max_cnt = bb_sub_bw_tp->COUNT_AUX_VHT[j][i];
						aux_max_idx = i;
						aux_max_bw = j;
					}
				}
				BB_DBG(bb, DBG_ANT_DIV,
				  "(VHT) Main ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  main_max_bw, main_max_idx, main_max_cnt);
				BB_DBG(bb, DBG_ANT_DIV,
				  "(VHT) Aux ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  aux_max_bw, aux_max_idx, aux_max_cnt);
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				const int MULTIPLIERS_VHT[4] = {100, 208, 450, 900};

				for(j = 0; j < 4; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_VHT[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_main_max_cnt) = bb_sub_bw_tp->COUNT_MAIN_VHT[j][i];
						*(cnt_info[j].sub_main_max_idx) = i;
						*(cnt_info[j].sub_main_max_tp) = MULTIPLIERS_VHT[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_VHT[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_aux_max_cnt) = bb_sub_bw_tp->COUNT_AUX_VHT[j][i];
						*(cnt_info[j].sub_aux_max_idx) = i;
						*(cnt_info[j].sub_aux_max_tp) = MULTIPLIERS_VHT[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						aux_max_bw = j;
					}
				}
			}
		}


		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant VHT %d-SS BW-160 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_vht[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant VHT %d-SS BW-80 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_vht_sc80[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant VHT %d-SS BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_vht_sc40[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Main-Ant VHT %d-SS BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 0],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 1],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 2],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 3],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 4],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 5],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 6],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 7],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 8],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 9],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 10],
				  bb_rate_i->main_pkt_cnt_vht_sc20[ss_ofst + 11]);

			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant VHT %d-SS BW-160 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_vht[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant VHT %d-SS BW-80 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_vht_sc80[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant VHT %d-SS BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_vht_sc40[ss_ofst + 11]);
			BB_DBG(bb, DBG_ANT_DIV,
				  "*Aux-Ant VHT %d-SS BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 0],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 1],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 2],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 3],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 4],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 5],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 6],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 7],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 8],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 9],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 10],
				  bb_rate_i->aux_pkt_cnt_vht_sc20[ss_ofst + 11]);
		}

	} else if ((bb_rate_i->main_ht_pkt_not_zero == true) || (bb_rate_i->aux_ht_pkt_not_zero == true)) {
		for (i = 0; i < HT_RATE_NUM; i++) {
			bb_sub_bw_tp->COUNT_MAIN_HT[0][i] = bb_rate_i->main_pkt_cnt_ht_sc20[i];
			bb_sub_bw_tp->COUNT_MAIN_HT[1][i] = bb_rate_i->main_pkt_cnt_ht[i];
			bb_sub_bw_tp->COUNT_AUX_HT[0][i] = bb_rate_i->aux_pkt_cnt_ht_sc20[i];
			bb_sub_bw_tp->COUNT_AUX_HT[1][i] = bb_rate_i->aux_pkt_cnt_ht[i];

			if (bb_ant_div->tp_decision_method == TP_MAX_DOMINATION) {
				for(j = 0; j < 2; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_HT[j][i] >= main_max_cnt) {
						main_max_cnt = bb_sub_bw_tp->COUNT_MAIN_HT[j][i];
						main_max_idx = i;
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_HT[j][i] >= aux_max_cnt) {
						aux_max_cnt = bb_sub_bw_tp->COUNT_AUX_HT[j][i];
						aux_max_idx = i;
						aux_max_bw = j;
					}
				}
				BB_DBG(bb, DBG_ANT_DIV,
				  "(HT) Main ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  main_max_bw, main_max_idx, main_max_cnt);
				BB_DBG(bb, DBG_ANT_DIV,
				  "(HT) Aux ant MAX MCS CNT in BW-%d MCS-%d, cnt = %d",
				  aux_max_bw, aux_max_idx, aux_max_cnt);
			} else if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
				const int MULTIPLIERS_HT[2] = {100, 208};

				for(j = 0; j < 2; j++) {
					if (bb_sub_bw_tp->COUNT_MAIN_HT[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_main_max_cnt) = bb_sub_bw_tp->COUNT_MAIN_HT[j][i];
						*(cnt_info[j].sub_main_max_idx) = i;
						*(cnt_info[j].sub_main_max_tp) = MULTIPLIERS_HT[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						main_max_bw = j;
					}
					if (bb_sub_bw_tp->COUNT_AUX_HT[j][i] >= bb_ant_div->tp_lb) {
						*(cnt_info[j].sub_aux_max_cnt) = bb_sub_bw_tp->COUNT_AUX_HT[j][i];
						*(cnt_info[j].sub_aux_max_idx) = i;
						*(cnt_info[j].sub_aux_max_tp) = MULTIPLIERS_HT[j] * bb_phy_rate_table[i + LEGACY_RATE_NUM];
						aux_max_bw = j;
					}
				}
			}
		}

		for (i = 0; i < bb->num_rf_path; i++) {
			ss_ofst = (i << 3);

			BB_DBG(bb, DBG_ANT_DIV,
			       "* Main-Ant HT%02d:%02d BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 0],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 1],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 2],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 3],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 4],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 5],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 6],
			       bb_rate_i->main_pkt_cnt_ht[ss_ofst + 7]);

			BB_DBG(bb, DBG_ANT_DIV,
			       "* Main-Ant HT%02d:%02d BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 0],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 1],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 2],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 3],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 4],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 5],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 6],
			       bb_rate_i->main_pkt_cnt_ht_sc20[ss_ofst + 7]);

			BB_DBG(bb, DBG_ANT_DIV,
			       "* Aux-Ant HT%02d:%02d BW-40 cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 0],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 1],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 2],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 3],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 4],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 5],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 6],
			       bb_rate_i->aux_pkt_cnt_ht[ss_ofst + 7]);

			BB_DBG(bb, DBG_ANT_DIV,
			       "* Aux-Ant HT%02d:%02d BW-20 cnt:{%d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 0],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 1],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 2],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 3],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 4],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 5],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 6],
			       bb_rate_i->aux_pkt_cnt_ht_sc20[ss_ofst + 7]);
		}

	}

	//max TP
	if (bb_ant_div->tp_decision_method == TP_HIGHEST_DOMINATION) {
		if((bb_sub_bw_tp->main_max_tp_160 >= bb_sub_bw_tp->main_max_tp_80) && (bb_sub_bw_tp->main_max_tp_160 >= bb_sub_bw_tp->main_max_tp_40) && (bb_sub_bw_tp->main_max_tp_160 >= bb_sub_bw_tp->main_max_tp_20)){
			main_max_cnt = bb_sub_bw_tp->main_max_cnt_160;
			main_max_idx = bb_sub_bw_tp->main_max_idx_160;
		}else if((bb_sub_bw_tp->main_max_tp_80 >= bb_sub_bw_tp->main_max_tp_40) && ((bb_sub_bw_tp->main_max_tp_80 >= bb_sub_bw_tp->main_max_tp_20))){
			main_max_cnt = bb_sub_bw_tp->main_max_cnt_80;
			main_max_idx = bb_sub_bw_tp->main_max_idx_80;
		}else if((bb_sub_bw_tp->main_max_tp_40 >= bb_sub_bw_tp->main_max_tp_20)){
			main_max_cnt = bb_sub_bw_tp->main_max_cnt_40;
			main_max_idx = bb_sub_bw_tp->main_max_idx_40;
		}else{
			main_max_cnt = bb_sub_bw_tp->main_max_cnt_20;
			main_max_idx = bb_sub_bw_tp->main_max_idx_20;
		}
		if((bb_sub_bw_tp->aux_max_tp_160 >= bb_sub_bw_tp->aux_max_tp_80) && (bb_sub_bw_tp->aux_max_tp_160 >= bb_sub_bw_tp->aux_max_tp_40) && (bb_sub_bw_tp->aux_max_tp_160 >= bb_sub_bw_tp->aux_max_tp_20)){
			aux_max_cnt = bb_sub_bw_tp->aux_max_cnt_160;
			aux_max_idx = bb_sub_bw_tp->aux_max_idx_160;
		}else if((bb_sub_bw_tp->aux_max_tp_80 >= bb_sub_bw_tp->aux_max_tp_40) && ((bb_sub_bw_tp->aux_max_tp_80 >= bb_sub_bw_tp->aux_max_tp_20))){
			aux_max_cnt = bb_sub_bw_tp->aux_max_cnt_80;
			aux_max_idx = bb_sub_bw_tp->aux_max_idx_80;
		}else if((bb_sub_bw_tp->aux_max_tp_40 >= bb_sub_bw_tp->aux_max_tp_20)){
			aux_max_cnt = bb_sub_bw_tp->aux_max_cnt_40;
			aux_max_idx = bb_sub_bw_tp->aux_max_idx_40;
		}else{
			aux_max_cnt = bb_sub_bw_tp->aux_max_cnt_20;
			aux_max_idx = bb_sub_bw_tp->aux_max_idx_20;
		}
	}
	/* Compute all throughput*/
	if (bb_ant_div->tp_decision_method == TP_AVG_DOMINATION) {
		
		bb_rate_i->main_tp = HALBB_DIV(bb_rate_i->main_tp, (u32)bb_rate_i->main_pkt_cnt_t);
		bb_rate_i->aux_tp = HALBB_DIV(bb_rate_i->aux_tp, (u32)bb_rate_i->aux_pkt_cnt_t);

		BB_DBG(bb, DBG_ANT_DIV, "Main_tp*100 = %d, Aux_tp*100 = %d\n", 
					 bb_rate_i->main_tp, bb_rate_i->aux_tp);
		BB_DBG(bb, DBG_ANT_DIV, "main_pkt_cnt_t = %d, aux_pkt_cnt_t = %d\n", 
					 bb_rate_i->main_pkt_cnt_t, bb_rate_i->aux_pkt_cnt_t);

		if (bb_rate_i->main_tp > bb_rate_i->aux_tp) {
			bb_ant_div->target_ant_tp = MAIN_ANT;
			bb_rate_i->tp_diff = bb_rate_i->main_tp - bb_rate_i->aux_tp;
		} else if (bb_rate_i->main_tp < bb_rate_i->aux_tp) {
			bb_ant_div->target_ant_tp = AUX_ANT;
			bb_rate_i->tp_diff = bb_rate_i->aux_tp - bb_rate_i->main_tp;
		} else {
			bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
			bb_rate_i->no_change_flag = true;
			BB_DBG(bb, DBG_ANT_DIV, "TP based TargetAnt= Pre-TargetAnt\n");
		}

		BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= [%s]\n", 
		(bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		BB_DBG(bb, DBG_ANT_DIV, "TP Confidence= [%d]\n", bb_rate_i->tp_diff);

		return;
	}
	if (bb_ant_div->tp_decision_method != TP_AVG_DOMINATION) {
		bb_rate_i->main_max_cnt = main_max_cnt;
		bb_rate_i->main_max_idx = main_max_idx;
		bb_rate_i->aux_max_cnt = aux_max_cnt;
		bb_rate_i->aux_max_idx = aux_max_idx;

		/* Decision Tput based target ant using MCS rate instead of phy data rate */
		if ((main_max_cnt > bb_ant_div->tp_lb) || (aux_max_cnt > bb_ant_div->tp_lb)) {
			if (main_max_idx > aux_max_idx) {
				bb_ant_div->target_ant_tp = MAIN_ANT;
				bb_rate_i->tp_diff = 100;
			} else if (main_max_idx < aux_max_idx) {
				bb_ant_div->target_ant_tp = AUX_ANT;
				bb_rate_i->tp_diff = 100;
			} else {
				if (main_max_cnt > aux_max_cnt)
					bb_ant_div->target_ant_tp = MAIN_ANT;
				else if (main_max_cnt < aux_max_cnt)
					bb_ant_div->target_ant_tp = AUX_ANT;
				else {
					bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
					bb_rate_i->no_change_flag = true;
				}
				/* Calc. TP confidence*/
				bb_rate_i->tp_diff = DIFF_2(main_max_cnt, aux_max_cnt);
			}
		} else {
			bb_ant_div->target_ant_tp = bb_ant_div->pre_target_ant;
			bb_rate_i->no_change_flag = true;
			BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= Pre-TargetAnt\n");
		}
	}

	if (bb_rate_i->no_change_flag == true)
		bb_rate_i->tp_diff = 0;

	if (bb_rate_i->tp_diff > 100)
		bb_rate_i->tp_diff = 100;

	BB_DBG(bb, DBG_ANT_DIV, "MCS based TargetAnt= [%s]\n", 
		(bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	BB_DBG(bb, DBG_ANT_DIV, "TP Confidence= [%d]\n", bb_rate_i->tp_diff);
}

void halbb_antdiv_get_highest_mcs(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;

	if (bb_ant_div->ant_div_new == 1)
		halbb_antdiv_get_highest_mcs_new(bb);
	else
		halbb_antdiv_get_highest_mcs_old(bb);

}

void halbb_antdiv_get_evm_target_ant(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_evm_info *bb_evm_i = &bb_ant_div->bb_evm_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	u8 main_2ss_evm_min = 0;
	u8 aux_2ss_evm_min = 0;
	//u8 main_2ss_evm_avg, aux_2ss_evm_avg;
	u8 main_1ss_evm = 0;
	u8 aux_1ss_evm = 0;
	u8 target_ant_evm_1ss = 0, target_ant_evm_2ss = 0;
	u8 decision_evm_ss = 0;
	u8 evm_diff_1ss = 0, evm_diff_2ss = 0;
	bool no_change_flag_1ss = false;
	bool no_change_flag_2ss = false;

	/* 1ss EVM */
	if (bb_ant_div->evm_decision_method == EVM_LINEAR_AVG) {
		/* Modify db to linear (*10)*/
		bb_evm_i->main_evm_1ss = HALBB_DIV(bb_evm_i->main_evm_1ss, 10);
		bb_evm_i->aux_evm_1ss = HALBB_DIV(bb_evm_i->aux_evm_1ss, 10);

		main_1ss_evm = (u8)halbb_convert_to_db((u64)HALBB_DIV(bb_evm_i->main_evm_1ss, 
			(bb_rate_i->main_pkt_cnt_1ss + bb_rate_i->main_pkt_cnt_ofdm)));
		aux_1ss_evm = (u8)halbb_convert_to_db((u64)HALBB_DIV(bb_evm_i->aux_evm_1ss, 
			(bb_rate_i->aux_pkt_cnt_1ss + bb_rate_i->aux_pkt_cnt_ofdm)));

		main_1ss_evm = main_1ss_evm << 2;
		aux_1ss_evm = aux_1ss_evm << 2;
	} else {
		main_1ss_evm =  (u8)HALBB_DIV(bb_evm_i->main_evm_1ss, 
			(bb_rate_i->main_pkt_cnt_1ss + bb_rate_i->main_pkt_cnt_ofdm));
		aux_1ss_evm =  (u8)HALBB_DIV(bb_evm_i->aux_evm_1ss, 
			(bb_rate_i->aux_pkt_cnt_1ss + bb_rate_i->aux_pkt_cnt_ofdm));
	}

	if (main_1ss_evm == aux_1ss_evm) {
		target_ant_evm_1ss = bb_ant_div->pre_target_ant;
		no_change_flag_1ss = true;
	} else if (main_1ss_evm > aux_1ss_evm) {
		target_ant_evm_1ss = MAIN_ANT;
	} else {
		target_ant_evm_1ss = AUX_ANT;
	}

	/* Calc. EVM confindece*/
	evm_diff_1ss = DIFF_2(main_1ss_evm, aux_1ss_evm);

	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d)\n", "[Main-Ant 1ss-EVM_avg]",
	       (main_1ss_evm >> 2),
	       halbb_show_fraction_num(main_1ss_evm & 0x3, 2));

	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d)\n", "[Aux-Ant 1ss-EVM_avg]",
	       (aux_1ss_evm >> 2),
	       halbb_show_fraction_num(aux_1ss_evm & 0x3, 2));

	/* 2ss EVM */
	if ((bb_rate_i->main_pkt_cnt_2ss + bb_rate_i->aux_pkt_cnt_2ss) != 0) {
		if (bb_ant_div->evm_decision_method == EVM_LINEAR_AVG) {

			bb_evm_i->main_evm_min_acc = HALBB_DIV(bb_evm_i->main_evm_min_acc, 10);
			bb_evm_i->aux_evm_min_acc = HALBB_DIV(bb_evm_i->aux_evm_min_acc, 10);

			main_2ss_evm_min = (u8)halbb_convert_to_db((u64)HALBB_DIV(bb_evm_i->main_evm_min_acc,
							      bb_rate_i->main_pkt_cnt_2ss));
			aux_2ss_evm_min = (u8)halbb_convert_to_db((u64)HALBB_DIV(bb_evm_i->aux_evm_min_acc,
							      bb_rate_i->aux_pkt_cnt_2ss));

			main_2ss_evm_min = main_2ss_evm_min << 2;
			aux_2ss_evm_min = aux_2ss_evm_min << 2;

		} else {
			main_2ss_evm_min = (u8)HALBB_DIV(bb_evm_i->main_evm_min_acc,
				bb_rate_i->main_pkt_cnt_2ss);
			aux_2ss_evm_min = (u8)HALBB_DIV(bb_evm_i->aux_evm_min_acc,
				bb_rate_i->aux_pkt_cnt_2ss);
		}

		if (main_2ss_evm_min == aux_2ss_evm_min) {
			target_ant_evm_2ss = bb_ant_div->pre_target_ant;
			no_change_flag_2ss = true;
		} else if (main_2ss_evm_min > aux_2ss_evm_min) {
			target_ant_evm_2ss = MAIN_ANT;
		} else {
			target_ant_evm_2ss = AUX_ANT;
		}

		/* Calc. EVM confindece*/
		evm_diff_2ss = DIFF_2(main_2ss_evm_min, aux_2ss_evm_min);
	}

	/*-----For Debug-----*/
	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d) (%d)\n", "[Main-Ant 2ss-EVM_avg]",
	       (main_2ss_evm_min >> 2),
	       halbb_show_fraction_num(main_2ss_evm_min & 0x3, 2),main_2ss_evm_min);

	BB_DBG(bb, DBG_ANT_DIV, "%-9s (%02d.%03d) (%d)\n", "[Aux-Ant 2ss-EVM_avg]",
	       (aux_2ss_evm_min >> 2),
	       halbb_show_fraction_num(aux_2ss_evm_min & 0x3, 2), aux_2ss_evm_min);
	/*-------------------*/

	if ((bb_rate_i->main_pkt_cnt_2ss + bb_rate_i->aux_pkt_cnt_2ss) != 0) {
		decision_evm_ss = 2;
		bb_evm_i->evm_diff = evm_diff_2ss;
		bb_ant_div->target_ant_evm = target_ant_evm_2ss;
		bb_evm_i->no_change_flag = no_change_flag_2ss;
	} else {
		decision_evm_ss = 1;
		bb_evm_i->evm_diff = evm_diff_1ss;
		bb_ant_div->target_ant_evm = target_ant_evm_1ss;
		bb_evm_i->no_change_flag = no_change_flag_1ss;
	}

	if (bb_evm_i->no_change_flag == true) {
		bb_evm_i->evm_diff = 0;
		BB_DBG(bb, DBG_ANT_DIV, "EVM based TargetAnt= Pre-TargetAnt\n");
	}

	if (bb_evm_i->evm_diff > 100)
		bb_evm_i->evm_diff = 100;

	BB_DBG(bb, DBG_ANT_DIV, "%d-ss EVM based TargetAnt= [%s]\n", 
	decision_evm_ss, (bb_ant_div->target_ant_evm == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	BB_DBG(bb, DBG_ANT_DIV, "EVM Confidence= [%d]\n", bb_evm_i->evm_diff);
}

void halbb_antdiv_training_state(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	u8 next_ant;
	u32 ret = 0;

	if (bb_ant_div->antdiv_training_state_cnt == 0) {
		bb_ant_div->get_stats = false;
		halbb_antdiv_reset_training_stat(bb);
		bb_ant_div->training_ant = bb_ant_div->pre_target_ant;
		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? MAIN_ANT : AUX_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "Next training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		halbb_antdiv_set_ant(bb, next_ant);
		bb_ant_div->training_ant = next_ant;
		//bb_ant_div->pre_target_ant = next_ant;
		bb_ant_div->antdiv_training_state_cnt++;
		BB_DBG(bb, DBG_ANT_DIV, "%s Statistics Interval=%d ms\n",
			((next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"), bb_ant_div->antdiv_intvl);
		bb_ant_div->get_stats = true;
		bb_ant_div->antdiv_timer_i.cb_time= bb_ant_div->antdiv_intvl;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	} else if ((bb_ant_div->antdiv_training_state_cnt % 2) == 0) {
		bb_ant_div->antdiv_training_state_cnt++;
		next_ant = bb_ant_div->training_ant;
		BB_DBG(bb, DBG_ANT_DIV, "%s Statistics Interval=%d ms\n",
			((next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"), bb_ant_div->antdiv_intvl);
		bb_ant_div->get_stats = true;
		bb_ant_div->antdiv_timer_i.cb_time = bb_ant_div->antdiv_intvl;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	} else if ((bb_ant_div->antdiv_training_state_cnt % 2) != 0) {
		bb_ant_div->get_stats = false;
		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "Next training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		halbb_antdiv_set_ant(bb, next_ant);
		bb_ant_div->training_ant = next_ant;
		bb_ant_div->antdiv_training_state_cnt++;
		BB_DBG(bb, DBG_ANT_DIV, "%s Delay=%d ms\n",
			((next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"), bb_ant_div->antdiv_delay);
		bb_ant_div->antdiv_timer_i.cb_time = bb_ant_div->antdiv_delay;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	}
}

void halbb_antdiv_tb_training_state(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u8 ret = 0, i = 0;
	u8 next_ant = 0;
	u32 tb_ppdu_cnt = 0, tb_ppdu_sts = 0, timeout_cnt = 0;

	if (bb_ant_div->antdiv_tb_training_state_cnt == 0) {
		bb_ant_div->time_out_cnt = 0;
		bb_ant_div->training_ant = bb_ant_div->pre_target_ant;
		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? MAIN_ANT : AUX_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "1. training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		bb_ant_div->training_ant = next_ant;
		if (bb_ant_div->training_ant == MAIN_ANT)
		{	if(bb_cn_i->main_cn_pkt_cnt > 0)
			{				
				// if(HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt) <= 0){
				// 	//cable link mode skip next training
				// 	BB_DBG(bb, DBG_ANT_DIV, "Main ant., Cable link CN = %d",HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt));
				// 	bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 4;
				// }else{
					BB_DBG(bb, DBG_ANT_DIV, "Main ant., Skip first State CN = %d",HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt));
					bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 2;
					bb_cn_i->one_path_CL_flag = 0;
				// }
			} else { 
				bb_cn_i->main_cn_avg_acc = 0;
				bb_cn_i->main_cn_pkt_cnt = 0;
				bb_ant_div->antdiv_tb_training_state_cnt++;
				ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
				BB_DBG(bb, DBG_ANT_DIV, "NO packet, main RE Sent Trigger Frame \n");
			}
		} else {
			if(bb_cn_i->aux_cn_pkt_cnt >0)
			{
				// if(HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt) <= 0)
				// {
				// 	BB_DBG(bb, DBG_ANT_DIV, "Aux ant., Cable link CN = %d",HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt));
				// 	bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 4;
				// }else{
					bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 2;
					BB_DBG(bb, DBG_ANT_DIV, "Aux ant., Skip first State CN = %d",HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt));
					bb_cn_i->one_path_CL_flag = 0;
				// }				
			} else {
				bb_cn_i->aux_cn_avg_acc = 0;
				bb_cn_i->aux_cn_pkt_cnt = 0;
				bb_ant_div->antdiv_tb_training_state_cnt++;
				ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
				BB_DBG(bb, DBG_ANT_DIV, "NO packet, aux Sent Trigger Frame \n");
			}
		}	
		bb_ant_div->antdiv_timer_i.cb_time = 1;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	} else if (bb_ant_div->antdiv_tb_training_state_cnt == 2) {
		bb_ant_div->antdiv_tb_training_state_cnt++;
		bb_ant_div->time_out_cnt = 0;

		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "2.training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		halbb_antdiv_set_ant(bb, next_ant);
		bb_ant_div->training_ant = next_ant;
		if (bb_ant_div->training_ant == MAIN_ANT)
		{
			bb_cn_i->main_cn_avg_acc = 0;
			bb_cn_i->main_cn_pkt_cnt = 0;
		} else {
			bb_cn_i->aux_cn_avg_acc = 0;
			bb_cn_i->aux_cn_pkt_cnt = 0;
		}
		//halbb_delay_ms(bb, bb_ant_div->multista_antdiv_ra_rdy_delay);
		ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		BB_DBG(bb, DBG_ANT_DIV, "Sent Trigger Frame \n");
		bb_ant_div->antdiv_timer_i.cb_time = 1;	
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	} else {
		BB_DBG(bb, DBG_ANT_DIV, "0x586c = %x\n", halbb_get_reg(bb, 0x586c, 0x20000));
		tb_ppdu_cnt = 0;
		tb_ppdu_sts = 0;
		timeout_cnt = 0;
		//BB_DBG(bb, DBG_ANT_DIV, "main_cn_pkt_cnt : bb_cn_i->aux_cn_pkt_cnt= %d : %d\n",bb_cn_i->main_cn_pkt_cnt, bb_cn_i->aux_cn_pkt_cnt);
		ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		BB_DBG(bb, DBG_ANT_DIV, "Sent Trigger Frame \n");
		do { /*Polling time always use 1ms, when it exceed 2s, break loop*/
			if (bb_ant_div->training_ant == MAIN_ANT)
			{
				tb_ppdu_sts = bb_cn_i->main_cn_pkt_cnt;
			} else{
				tb_ppdu_sts = bb_cn_i->aux_cn_pkt_cnt;
			}
			//BB_DBG(bb, DBG_ANT_DIV, "tb_ppdu_sts : tb_ppdu_cnt = %d : %d\n",tb_ppdu_sts,tb_ppdu_cnt);
			if (tb_ppdu_sts > bb_ant_div->tb_antdiv_train_num ) {
				timeout_cnt = 0;
				break;
			} else if(tb_ppdu_sts - tb_ppdu_cnt > 0) {
				tb_ppdu_cnt++;
				ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
				BB_DBG(bb, DBG_ANT_DIV, "tb_ppdu_sts=%d,  tb_ppdu_cnt= %d\n", tb_ppdu_sts, tb_ppdu_cnt);
				BB_DBG(bb, DBG_ANT_DIV, "(tb_ppdu_sts - tb_ppdu_cnt > 0) Sent Trigger Frame \n");
			} else {
				halbb_delay_ms(bb, 1);
				//timeout_cnt ++;
				if(timeout_cnt == 5 && tb_ppdu_sts == 0 ) {
					BB_DBG(bb, DBG_ANT_DIV, "Re Sent Trigger Frame \n");
					ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
				}
				timeout_cnt ++;
				continue;
			}
		} while (timeout_cnt < bb_ant_div->tb_antdiv_timeout);	
		BB_DBG(bb, DBG_ANT_DIV, "Time = %dms \n", timeout_cnt);
		bb_ant_div->antdiv_tb_training_state_cnt++;
		
		halbb_tb_based_antdiv(bb);
		//bb_ant_div->antdiv_timer_i.cb_time = 1;
		//halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	}

}

void halbb_antdiv_tb_decision_state(struct bb_info *bb) 
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u32 main_cn, aux_cn;
	u8 target_ant_tb = 0;
	u8 target_ant_cn = 0;

	main_cn =  (u8)HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt);	
	if ( bb_cn_i->main_cn_pkt_cnt > 0) {
		bb_cn_i->record_main_avg_cn = main_cn; 
	} else {
		bb_cn_i->main_cn_pkt_cnt=50;
		main_cn = bb_cn_i->record_main_avg_cn;	
	}

	aux_cn =  (u8)HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt);
	if ( bb_cn_i->aux_cn_pkt_cnt > 0) {
		bb_cn_i->record_aux_avg_cn = aux_cn; 
	} else {
		bb_cn_i->aux_cn_pkt_cnt = 50;
		aux_cn = bb_cn_i->record_aux_avg_cn;
	}

	// only one path cable link
	if ((main_cn == 0) && (aux_cn != 0)) {
		target_ant_cn = MAIN_ANT;
		bb_ant_div->target_ant_cn = target_ant_cn;
		bb_cn_i->one_path_CL_flag = 1;
		BB_DBG(bb, DBG_ANT_DIV, "ONE PATH (MAIN) CABLE LINK CASE \n");
		return;
	}else if ((main_cn != 0) && (aux_cn == 0)) {
		target_ant_cn = AUX_ANT;
		bb_ant_div->target_ant_cn = target_ant_cn;
		bb_cn_i->one_path_CL_flag = 1;
		BB_DBG(bb, DBG_ANT_DIV, "ONE PATH (AUX) CABLE LINK CASE \n");
		return;
	}

	//cable link would skip training state main aux would always CN=0
	if ((main_cn == 0) && (aux_cn == 0)) {
		bb_cn_i->two_path_CL_flag = 1;
		BB_DBG(bb, DBG_ANT_DIV, "TWO PATH CABLE LINK CASE \n");
		if (bb_cn_i->main_rssi_ma > bb_cn_i->aux_rssi_ma) {
			target_ant_cn = MAIN_ANT;
			bb_ant_div->target_ant_cn = target_ant_cn;
		} else if (bb_cn_i->main_rssi_ma > bb_cn_i->aux_rssi_ma) {
			target_ant_cn = AUX_ANT;
			bb_ant_div->target_ant_cn = target_ant_cn;
		} else {
			bb_ant_div->target_ant_cn = bb_ant_div->pre_target_ant;
		}
		return;
		// if (bb_cn_i->aux_cn_pkt_cnt == 50) {
		// 	aux_cn=50;
		// } else if (bb_cn_i->main_cn_pkt_cnt == 50) {
		// 	main_cn=50;
		// }
	}
	
	BB_DBG(bb, DBG_ANT_DIV,"Main    PKT CNT = %d, AVG_CN = (%02d.%03d)\n", bb_cn_i->main_cn_pkt_cnt, main_cn>>1,halbb_show_fraction_num(main_cn & 0x1, 1));
	BB_DBG(bb, DBG_ANT_DIV,"Aux     PKT CNT = %d, AVG_CN = (%02d.%03d)\n", bb_cn_i->aux_cn_pkt_cnt, aux_cn>>1,halbb_show_fraction_num(aux_cn & 0x1, 1));
	BB_DBG(bb, DBG_ANT_DIV,"Main CN vs AUX CN           =  %d : %d, Diff = %d, Min = %d\n", main_cn>>1, aux_cn>>1, DIFF_2(main_cn>>1,aux_cn>>1), MIN_2(main_cn>>1,aux_cn>>1));
	target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((aux_cn >= main_cn) ? MAIN_ANT : AUX_ANT);
	bb_ant_div->target_ant_cn = target_ant_cn;

	if((bb_cn_i->main_rssi <= 30)||(bb_cn_i->aux_rssi <= 30)){
		target_ant_cn = (bb_cn_i->main_rssi >= bb_cn_i->aux_rssi) ? MAIN_ANT: AUX_ANT;
		bb_ant_div->target_ant_cn = target_ant_cn;
		BB_DBG(bb, DBG_ANT_DIV,"RSSI Too low, choose RSSI Bigger One\n");
	} else if(DIFF_2(bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma) > bb_ant_div->tb_antdiv_rssi_diff)
	{
		if((MIN_2(bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma) >=50) && (MAX_2(main_cn>>1,aux_cn>>1) <= 8 )){
			target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
		} else {
			target_ant_cn = (bb_cn_i->main_rssi_ma == bb_cn_i->aux_rssi_ma) ? (bb_ant_div->pre_target_ant) : ((bb_cn_i->aux_rssi_ma >= bb_cn_i->main_rssi_ma) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
			BB_DBG(bb, DBG_ANT_DIV,"RSSI DIFF Too big choose Bigger One\n");
		}
	} else {
		if((DIFF_2(main_cn>>1,aux_cn>>1) <= bb_ant_div->tb_antdiv_cn_diff))
		{
			target_ant_cn = (bb_cn_i->main_rssi_ma == bb_cn_i->aux_rssi_ma) ? (bb_ant_div->pre_target_ant) : ((bb_cn_i->aux_rssi_ma >= bb_cn_i->main_rssi_ma) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
			BB_DBG(bb, DBG_ANT_DIV,"CN DIFF Too small choose Bigger One\n");
		} else {
			target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
		}
	}
}

void halbb_antdiv_get_cn_in_watchdog(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	u8 ret=0;
	u8 i=0;

	BB_DBG(bb, DBG_ANT_DIV, "Training in Watchdog for next\n");
	for (i=0; i < bb_ant_div->tb_antdiv_train_num; i++){
		ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		halbb_delay_ms(bb, 10);
	}

	bb_ant_div->tb_antdiv_train_next = 0;
}

void halbb_tb_based_antdiv(struct bb_info *bb)
{
	//struct rtw_phl_stainfo_t *sta;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct rtw_phl_com_t *phl = bb->phl_com;
	u32 total_tx_tput=0, total_rx_tput=0;
	u32 ant_curr_avg_cn = 0;
	u32 ant_curr_rssi = 0;
	
	if (bb_ant_div->target_ant_cn == MAIN_ANT) {
		ant_curr_avg_cn = HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt);
		ant_curr_rssi = bb_cn_i->main_rssi;

	} else {
		ant_curr_avg_cn = HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt);
		ant_curr_rssi = bb_cn_i->aux_rssi;
	}

	//BB_DBG(bb, DBG_ANT_DIV,"State Cnt = %d",bb_ant_div->antdiv_tb_training_state_cnt);
	if (bb_ant_div->antdiv_tb_training_state_cnt <= 3)
	{	
		if ((ant_curr_avg_cn == 0) && (bb_cn_i->one_path_CL_flag == 1) && (bb_ant_div->one_path_CL_time_cnt <= bb_ant_div->one_path_CL_pass_WD_lim) && (DIFF_2(ant_curr_rssi, bb_ant_div->CL_ant_rssi_perv) <  bb_ant_div->one_path_CL_fix_ant_rssi_diff) ) {
			BB_DBG(bb, DBG_ANT_DIV,"pass train state, one_path_CL_time_cnt= %d, one_path_CL_pass_WD_lim=%d \n", bb_ant_div->one_path_CL_time_cnt, bb_ant_div->one_path_CL_pass_WD_lim);
			bb_ant_div->one_path_CL_time_cnt++;
			bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 4;
			bb_ant_div->antdiv_timer_i.cb_time = 1;
			halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
		} else if ((bb_ant_div->antdiv_tb_training_state_cnt==0) && (ant_curr_avg_cn == 0) && (bb_cn_i->two_path_CL_flag == 1) && (bb_ant_div->two_path_CL_time_cnt <= bb_ant_div->two_path_CL_pass_WD_lim)) {
			BB_DBG(bb, DBG_ANT_DIV,"two_path_CL_time_cnt= %d, two_path_CL_pass_WD_lim=%d \n", bb_ant_div->two_path_CL_time_cnt, bb_ant_div->two_path_CL_pass_WD_lim);
			if (DIFF_2(ant_curr_rssi, bb_ant_div->CL_ant_rssi_perv) < bb_ant_div->two_path_CL_fix_ant_rssi_diff ){
				BB_DBG(bb, DBG_ANT_DIV,"pass train state \n");
				bb_ant_div->two_path_CL_time_cnt++;
				bb_ant_div->antdiv_tb_training_state_cnt = bb_ant_div->antdiv_tb_training_state_cnt + 4;
				bb_ant_div->target_ant_cn = bb_ant_div->pre_target_ant;
				bb_ant_div->antdiv_timer_i.cb_time = 1;
				halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
			} else {
				bb_cn_i->two_path_CL_flag = 0;
				bb_ant_div->two_path_CL_time_cnt = 0;
				halbb_antdiv_tb_training_state(bb);
			}
		} else {
			bb_cn_i->one_path_CL_flag = 0;
			bb_ant_div->one_path_CL_time_cnt = 0;
			bb_cn_i->two_path_CL_flag = 0;
			bb_ant_div->two_path_CL_time_cnt = 0;
			halbb_antdiv_tb_training_state(bb);
		}
		bb_ant_div->CL_ant_rssi_perv = ant_curr_rssi;
	} else {
		if (bb_cn_i->one_path_CL_flag == 1) {
			bb_ant_div->target_ant_cn = bb_ant_div->pre_target_ant;
			BB_DBG(bb, DBG_ANT_DIV,"PASS decision!\n");
			BB_DBG(bb, DBG_ANT_DIV,"only [%s] CL!!\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		} else if (bb_cn_i->two_path_CL_flag == 1) {
			bb_ant_div->target_ant_cn = bb_ant_div->pre_target_ant;
			BB_DBG(bb, DBG_ANT_DIV,"PASS decision!\n");
			BB_DBG(bb, DBG_ANT_DIV,"two path CL!! chose ant=[%s]\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		} else {
			//decision state
			halbb_antdiv_tb_decision_state(bb);
		}
		BB_DBG(bb, DBG_ANT_DIV,"CN based TargetAnt= [%s]\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

		halbb_antdiv_set_ant(bb, bb_ant_div->target_ant_cn);
		//Reset Cnt
		bb_ant_div->antdiv_tb_training_state_cnt = 0;
		bb_cn_i->main_cn_pkt_cnt = 0;
		bb_cn_i->main_cn_avg_acc = 0;
		bb_cn_i->aux_cn_pkt_cnt = 0;
		bb_cn_i->aux_cn_avg_acc = 0;
		bb_ant_div->pre_target_ant = bb_ant_div->target_ant_cn;
		bb_ant_div->training_ant = bb_ant_div->target_ant_cn;

		//Get CN in stable state for next training
		bb_ant_div->tb_antdiv_train_next = 1;
		bb_ant_div->antdiv_timer_i.cb_time = 100;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
	}

}

void halbb_antdiv_decision_state(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_antdiv_evm_info *bb_evm_i = &bb_ant_div->bb_evm_i;

	BB_DBG(bb, DBG_ANT_DIV, "[Decisoin state]\n");

	bb_ant_div->get_stats = false;
	//BB_TRACE("Phy sts get stop\n");
	bb_ant_div->antdiv_training_state_cnt = 0;
	/* Check highest MCS idx of main & aux antenna*/
	halbb_antdiv_get_highest_mcs(bb);

	/* EVM based antenna diversity */
	halbb_antdiv_get_evm_target_ant(bb);

	/* CN based antenna diversity*/
	//halbb_antdiv_get_cn_target_ant(bb);

	/* TP based antenna diversity */
	//halbb_antdiv_get_tp_target_ant(bb);

	/* Final Decision */
	if ((bb_rate_i->main_cnt_all + bb_rate_i->aux_cnt_all) == 0) {
		BB_DBG(bb, DBG_ANT_DIV, "Not enough count, remain previous antenna\n");
		bb_ant_div->target_ant = bb_ant_div->pre_target_ant;
		BB_DBG(bb, DBG_ANT_DIV, "Make decision again ASAP\n");
		bb_ant_div->antdiv_wd_cnt = bb_ant_div->antdiv_period;
		return;
	}

	if (bb_rate_i->no_change_flag && bb_evm_i->no_change_flag) {
		BB_DBG(bb, DBG_ANT_DIV, "No Decision, remain previous antenna\n");
		bb_ant_div->target_ant = bb_ant_div->pre_target_ant;
		BB_DBG(bb, DBG_ANT_DIV, "Make decision again ASAP\n");
		bb_ant_div->antdiv_wd_cnt = bb_ant_div->antdiv_period;
		return;
	}

	if ((bb_ant_div->target_ant_evm == bb_ant_div->target_ant_tp) &&
		(!bb_rate_i->no_change_flag) && (!bb_evm_i->no_change_flag)) {
		BB_DBG(bb, DBG_ANT_DIV, "Decision confidence is enough\n");
		bb_ant_div->target_ant = bb_ant_div->target_ant_evm;
	} else {
		if (bb_rate_i->tp_diff >= bb_ant_div->tp_diff_th_high) {
			BB_DBG(bb, DBG_ANT_DIV, "TP confidence is %d > %d, Decided by MCS based\n"
			,bb_rate_i->tp_diff, bb_ant_div->tp_diff_th_low);
			bb_ant_div->target_ant = bb_ant_div->target_ant_tp;
		} else if (bb_evm_i->evm_diff >= bb_ant_div->evm_diff_th) {
			BB_DBG(bb, DBG_ANT_DIV, "EVM confidence is more than %d, Decided by EVM based\n"
						,bb_ant_div->evm_diff_th);
			bb_ant_div->target_ant = bb_ant_div->target_ant_evm;
		} else if (bb_rate_i->tp_diff >= bb_ant_div->tp_diff_th_low) {
			BB_DBG(bb, DBG_ANT_DIV, "TP confidence is %d > %d, Decided by MCS based\n"
			,bb_rate_i->tp_diff, bb_ant_div->tp_diff_th_low);
			bb_ant_div->target_ant = bb_ant_div->target_ant_tp;
		} else {
			BB_DBG(bb, DBG_ANT_DIV, "Decision confidence is not enough, remain previous antenna\n");
			bb_ant_div->target_ant = bb_ant_div->pre_target_ant;

			BB_DBG(bb, DBG_ANT_DIV, "Make decision again ASAP\n");
			bb_ant_div->antdiv_wd_cnt = bb_ant_div->antdiv_period;
		}
	}

	BB_DBG(bb, DBG_ANT_DIV, "TargetAnt= [%s]\n", (bb_ant_div->target_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

}

void halbb_evm_based_antdiv(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rssi_su_avg_info *avg = &cmn_rpt->bb_rssi_su_avg_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;

	/* Main function */
	if (bb_ant_div->antdiv_training_state_cnt <= ((bb_ant_div->antdiv_train_num << 1) - 2)) {
		BB_DBG(bb, DBG_ANT_DIV, "Ant_diversity training state = %d \n",
					bb_ant_div->antdiv_training_state_cnt);
		halbb_antdiv_training_state(bb);
		return;
	} else {
		halbb_antdiv_decision_state(bb);
		halbb_antdiv_reset_training_stat(bb);
	}

	/* Set new target antenna */

	BB_DBG(bb, DBG_ANT_DIV, "Ant_diversity done\n");
	halbb_antdiv_set_ant(bb, bb_ant_div->target_ant);

	/* Re-assign to next step reqired variable */
	bb_ant_div->target_ant_evm = bb_ant_div->target_ant;
	bb_ant_div->target_ant_tp = bb_ant_div->target_ant;
	bb_ant_div->target_ant_cn = bb_ant_div->target_ant;
	bb_ant_div->pre_target_ant = bb_ant_div->target_ant;

}

# if 0
u32 halbb_tb_based_get_cn_by_macid(struct bb_info *bb, u8 macid)
{
	struct rtw_phl_stainfo_t *sta;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rssi_su_avg_info *avg = &cmn_rpt->bb_rssi_su_avg_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u32 main_cn, aux_cn;
	u8 ret = 0;
	u8 i = 0, j =0;
	u8 main_tb_cnt =0, aux_tb_cnt=0;
	u8 target_ant_cn;
	u32 ant_cn_rlt=0;

	sta = bb->phl_sta_info[macid];
	if (!sta) {
		//BB_DBG(bb, DBG_PWR_CTRL, "NULL PHL STA info\n");
		return 0;
	}
	if (!is_sta_active(sta))
		return 0;

	halbb_antdiv_set_ant(bb, MAIN_ANT);
	bb_ant_div->training_ant = MAIN_ANT;
	bb_cn_i->main_cn_avg_acc = 0;
	bb_cn_i->main_cn_pkt_cnt = 0;
	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, macid, 0);
	do { /*Polling time always use 1ms, when it exceed 2s, break loop*/
		if (main_tb_cnt >= bb_ant_div->tb_antdiv_train_num ) {
			BB_DBG(bb, DBG_ANT_DIV,"Main Report Get, %d ms\n", i);
			//ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, 2, 0);
			break;
		} else if(bb_cn_i->main_cn_pkt_cnt-main_tb_cnt>0) {
			main_tb_cnt++;
			ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, macid, 0);
		} else {
			halbb_delay_ms(bb, 1);
			i++;
			continue;
		}
	} while (i < bb_ant_div->antdiv_delay);	
		/* CN */
	main_cn =  (u8)HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt);
	if ( bb_cn_i->main_cn_pkt_cnt > 0) {
		bb_cn_i->record_main_avg_cn = main_cn; 
	} else {
		main_cn = bb_cn_i->record_main_avg_cn;
		bb_cn_i->main_cn_pkt_cnt=50;
	}
	
	halbb_antdiv_set_ant(bb, AUX_ANT);
	bb_ant_div->training_ant = AUX_ANT;
	bb_cn_i->aux_cn_avg_acc = 0;
	bb_cn_i->aux_cn_pkt_cnt = 0;

	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, macid, 0);
	do { /*Polling time always use 1ms, when it exceed 2s, break loop*/
		if (aux_tb_cnt >= bb_ant_div->tb_antdiv_train_num ) {
			BB_DBG(bb, DBG_ANT_DIV,"Aux Report Get, %d ms\n", j);
			//ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, 2, 0);
			break;
		} else if(bb_cn_i->aux_cn_pkt_cnt-aux_tb_cnt>0) {
			aux_tb_cnt ++;
			ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate,bb_ant_div->tb_antdiv_train_bw, 1, macid, 0);
		} else {
			halbb_delay_ms(bb, 1);
			j++;
			continue;
		}
	} while (j < bb_ant_div->antdiv_delay);

	aux_cn =  (u8)HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt);
	if ( bb_cn_i->aux_cn_pkt_cnt > 0) {
		bb_cn_i->record_aux_avg_cn = aux_cn; 
	} else {
		aux_cn = bb_cn_i->record_aux_avg_cn;
		bb_cn_i->aux_cn_pkt_cnt = 50;
	}
	BB_DBG(bb, DBG_ANT_DIV,"MACID = %d \n", macid);
	BB_DBG(bb, DBG_ANT_DIV,"[%d] Main    PKT CNT = %d, AVG_CN = (%02d.%03d), %d ms \n",macid, bb_cn_i->main_cn_pkt_cnt, main_cn>>1,halbb_show_fraction_num(main_cn & 0x1, 1), i);
	BB_DBG(bb, DBG_ANT_DIV,"[%d] Aux    PKT CNT = %d, AVG_CN = (%02d.%03d), %d ms \n",macid, bb_cn_i->aux_cn_pkt_cnt, aux_cn>>1,halbb_show_fraction_num(aux_cn & 0x1, 1), j);
	target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);
	bb_ant_div->target_ant_cn = target_ant_cn;
	// Return Antenna with CN
	if(bb_ant_div->target_ant_cn == MAIN_ANT) {
		ant_cn_rlt = 0x1000 | main_cn;
	} else {
		ant_cn_rlt =  0x0000 | aux_cn;
	}
	
	return ant_cn_rlt;
}

void halbb_tb_based_multista_antdiv(struct bb_info *bb)
{
	//struct rtw_phl_stainfo_t *sta;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct rtw_phl_com_t *phl = bb->phl_com;
	//u8 i;
	u8 mode = 0;
	//u8 hw_md;
	u32 main_score = 0, aux_score = 0;
	u32 main_score_tput = 0, aux_score_tput = 0;
	u32 main_score_rssi = 0, aux_score_rssi = 0;
	u32 main_score_cn = 100, aux_score_cn = 100;
	u32 main_tx_tput=0, main_rx_tput=0;
	u32 aux_tx_tput=0, aux_rx_tput=0;
	u32 rlt=0;
	u8 target_ant_cn=0;
	u32 total_tput=0;
	
#if 0
	for (i = 1; i <= 4; i++) {
		sta = bb->phl_sta_info[i];
		if (is_sta_active(sta)) {
			mode = sta->wmode;
			hw_md = halbb_hw_mode_mapping(bb, mode, sta);
			if(hw_md & HE_SUPPORT){
				BB_DBG(bb, DBG_ANT_DIV, "Multi Link and support AX");
				BB_DBG(bb, DBG_ANT_DIV, "MACID = %d, Rx Rate = %x, Rx Tput = %d, RSSI = %d", i, sta->stats.rx_rate , sta->stats.rx_moving_average_tp, sta->hal_sta->rssi_stat.rssi_ma >> 5);
				total_tput += sta->stats.rx_moving_average_tp;
				rlt = halbb_tb_based_get_cn_by_macid(bb, i);
				if (rlt & 0xf000) {
					if (sta->stats.rx_rate > 0x199) {
						BB_DBG(bb, DBG_ANT_DIV, "Rx Rate > 2ss MCS9, %x", sta->stats.rx_rate);
						main_score_tput += sta->stats.rx_moving_average_tp*2;
						main_score_rssi += sta->hal_sta->rssi_stat.rssi_ma >> 4;
					} else {
						main_score_tput += sta->stats.rx_moving_average_tp;
						main_score_rssi += sta->hal_sta->rssi_stat.rssi_ma >> 5;
					}
					
					main_score_cn -= (rlt & 0x0fff) >> 1;
					aux_score_cn -= 18;
				} else {
					if (sta->stats.rx_rate > 0x199) {
						BB_DBG(bb, DBG_ANT_DIV, "Rx Rate > 2ss MCS9, %x", sta->stats.rx_rate);
						aux_score_tput += sta->stats.rx_moving_average_tp*2;
						aux_score_rssi += sta->hal_sta->rssi_stat.rssi_ma >> 4;
					} else {
						aux_score_tput += sta->stats.rx_moving_average_tp;
						aux_score_rssi += sta->hal_sta->rssi_stat.rssi_ma >> 5;
					}
					
					main_score_cn -= 18;
					aux_score_cn -= (rlt & 0x0fff) >> 1;

				}
			}
		}
	}
	//BB_DBG(bb, DBG_ANT_DIV, "Main RSSI Score v.s Aux RSSI Score = %d : %d\n",main_score_rssi, aux_score_rssi);
	//BB_DBG(bb, DBG_ANT_DIV, "Main Tput Score v.s Aux Tput Score = %d : %d\n",main_score_tput, aux_score_tput);
	//BB_DBG(bb, DBG_ANT_DIV, "Main Tput Score v.s Aux Tput Score = %d : %d\n",main_score_cn, aux_score_cn);
	//BB_DBG(bb, DBG_ANT_DIV, "Main Score v.s Aux Score = %d : %d\n",main_score, aux_score);
#endif

#ifdef CONFIG_RTL8852D	
	halbb_antdiv_set_ant(bb, MAIN_ANT);
	rtw_hal_bb_set_realtime_tp_timer(bb->hal_com, bb_ant_div->antdiv_intvl);
	halbb_delay_ms(bb, bb_ant_div->antdiv_intvl);
	rtw_hal_bb_get_realtime_tp(phl, &main_tx_tput, &main_rx_tput);
	BB_DBG(bb, DBG_ANT_DIV, "Main Rx Tput = %u Mbps in %d ms\n", main_rx_tput , bb_ant_div->antdiv_intvl);
	halbb_antdiv_set_ant(bb, AUX_ANT);
	rtw_hal_bb_set_realtime_tp_timer(bb->hal_com, bb_ant_div->antdiv_intvl);
	halbb_delay_ms(bb, bb_ant_div->antdiv_intvl);
	rtw_hal_bb_get_realtime_tp(phl, &aux_tx_tput, &aux_rx_tput);
	BB_DBG(bb, DBG_ANT_DIV, "Aux  Rx Tput = %u Mbps in %d ms\n", aux_rx_tput , bb_ant_div->antdiv_intvl);
	if (main_rx_tput == 0){
		main_rx_tput = bb_ant_div->main_rx_tput;
	} else {
		bb_ant_div->main_rx_tput =main_rx_tput;
	}

	if (aux_rx_tput == 0){
		aux_rx_tput = bb_ant_div->aux_rx_tput;
	} else {
		bb_ant_div->aux_rx_tput = aux_rx_tput;
	}

	// Start Decision
	target_ant_cn = (main_rx_tput == aux_rx_tput) ? (bb_ant_div->pre_target_ant) : (((main_rx_tput + main_tx_tput) >= (aux_rx_tput+aux_tx_tput)) ? MAIN_ANT : AUX_ANT);
	bb_ant_div->target_ant_cn = target_ant_cn;
	halbb_antdiv_set_ant(bb, bb_ant_div->target_ant_cn);
	bb_ant_div->pre_target_ant = bb_ant_div->target_ant_cn;
#else
	BB_DBG(bb, DBG_ANT_DIV, "Only 8852DR Support Multi-STA Ant Div = %d\n", target_ant_cn);
#endif


}
#endif
#if 0
void halbb_tb_based_antdiv_old(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rssi_su_avg_info *avg = &cmn_rpt->bb_rssi_su_avg_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u32 main_cn, aux_cn;
	u8 ret = 0;
	u8 i = 0, j =0, k=0;
	u8 main_tb_cnt =0, aux_tb_cnt=0;
	u8 main_rssi_avg =0, aux_rssi_avg=0;
	u8 main_rssi =0, aux_rssi=0;
	u8 target_ant_cn;
	u8 macid=1;


	halbb_antdiv_set_ant(bb, MAIN_ANT);
	bb_ant_div->training_ant = MAIN_ANT;
	bb_cn_i->main_cn_avg_acc = 0;
	bb_cn_i->main_cn_pkt_cnt = 0;
	bb_cn_i->main_rssi = 0;
	bb_cn_i->main_rssi_cnt = 0;
	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
	do { /*Polling time always use 1ms, when it exceed 2s, break loop*/
		if (main_tb_cnt >= bb_ant_div->tb_antdiv_train_num ) {
			break;
		} else if(bb_cn_i->main_cn_pkt_cnt-main_tb_cnt>0) {
			main_tb_cnt++;
			ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		} else {
			halbb_delay_ms(bb, 1);
			i++;
			continue;
		}
	} while (i < bb_ant_div->antdiv_delay);	
	/* CN */
	main_cn =  (u8)HALBB_DIV(bb_cn_i->main_cn_avg_acc, bb_cn_i->main_cn_pkt_cnt);
	if ( bb_cn_i->main_cn_pkt_cnt > 0) {
		bb_cn_i->record_main_avg_cn = main_cn; 
	} else {
		main_cn = bb_cn_i->record_main_avg_cn;
		bb_cn_i->main_cn_pkt_cnt=50;
	}
	
	halbb_antdiv_set_ant(bb, AUX_ANT);
	bb_ant_div->training_ant = AUX_ANT;
	bb_cn_i->aux_cn_avg_acc = 0;
	bb_cn_i->aux_cn_pkt_cnt = 0;
	bb_cn_i->aux_rssi = 0;
	bb_cn_i->aux_rssi_cnt = 0;

	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
	do { /*Polling time always use 1ms, when it exceed 2s, break loop*/
		if (aux_tb_cnt >= bb_ant_div->tb_antdiv_train_num ) {
			//BB_DBG(bb, DBG_ANT_DIV,"Aux Report Get, %d ms\n", j);
			break;
		} else if(bb_cn_i->aux_cn_pkt_cnt-aux_tb_cnt>0) {
			aux_tb_cnt ++;
			ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate,bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		} else {
			halbb_delay_ms(bb, 1);
			j++;
			continue;
		}
	} while (j < bb_ant_div->antdiv_delay);

	aux_cn =  (u8)HALBB_DIV(bb_cn_i->aux_cn_avg_acc, bb_cn_i->aux_cn_pkt_cnt);
	if ( bb_cn_i->aux_cn_pkt_cnt > 0) {
		bb_cn_i->record_aux_avg_cn = aux_cn; 
	} else {
		aux_cn = bb_cn_i->record_aux_avg_cn;
		bb_cn_i->aux_cn_pkt_cnt = 50;
	}

	BB_DBG(bb, DBG_ANT_DIV,"Main    PKT CNT = %d, AVG_CN = (%02d.%03d), %d ms \n", bb_cn_i->main_cn_pkt_cnt, main_cn>>1,halbb_show_fraction_num(main_cn & 0x1, 1), i);
	BB_DBG(bb, DBG_ANT_DIV,"Aux    PKT CNT = %d, AVG_CN = (%02d.%03d), %d ms \n", bb_cn_i->aux_cn_pkt_cnt, aux_cn>>1,halbb_show_fraction_num(aux_cn & 0x1, 1), j);
	BB_DBG(bb, DBG_ANT_DIV,"Main CN vs AUX CN           =  %d : %d, Diff = %d, Min = %d\n", main_cn>>1, aux_cn>>1, DIFF_2(main_cn>>1,aux_cn>>1), MIN_2(main_cn>>1,aux_cn>>1));
	BB_DBG(bb, DBG_ANT_DIV,"Main MA RSSI vs AUX MA RSSI =  %d : %d, Diff = %d, Min = %d\n", bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma, DIFF_2(bb_cn_i->main_rssi_ma,bb_cn_i->aux_rssi_ma), MIN_2(bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma));

	if(DIFF_2(bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma) > bb_ant_div->tb_antdiv_rssi_diff)
	{
		if((MIN_2(bb_cn_i->main_rssi_ma, bb_cn_i->aux_rssi_ma) >=50) && (MAX_2(main_cn>>1,aux_cn>>1) <= 8 )){
			target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
		} else {
			target_ant_cn = (bb_cn_i->main_rssi_ma == bb_cn_i->aux_rssi_ma) ? (bb_ant_div->pre_target_ant) : ((bb_cn_i->aux_rssi_ma >= bb_cn_i->main_rssi_ma) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
			BB_DBG(bb, DBG_ANT_DIV,"RSSI DIFF Too big choose Bigger One\n");
		}
	} else {
		if((DIFF_2(main_cn>>1,aux_cn>>1) <= bb_ant_div->tb_antdiv_cn_diff))
		{
			target_ant_cn = (bb_cn_i->main_rssi_ma == bb_cn_i->aux_rssi_ma) ? (bb_ant_div->pre_target_ant) : ((bb_cn_i->aux_rssi_ma >= bb_cn_i->main_rssi_ma) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
			BB_DBG(bb, DBG_ANT_DIV,"CN DIFF Too small choose Bigger One\n");
		} else {
			target_ant_cn = (main_cn == aux_cn) ? (bb_ant_div->pre_target_ant) : ((main_cn >= aux_cn) ? AUX_ANT : MAIN_ANT);
			bb_ant_div->target_ant_cn = target_ant_cn;
		}
	}
	BB_DBG(bb, DBG_ANT_DIV,"CN based TargetAnt= [%s]\n", (bb_ant_div->target_ant_cn == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
	halbb_antdiv_set_ant(bb, bb_ant_div->target_ant_cn);

}
#endif
void halbb_antdiv_multista_training_state(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;

	u32 total_tx_tput=0, total_rx_tput=0;
	u8 next_ant=0;
	struct rtw_phl_com_t *phl = bb->phl_com;

//#ifdef CONFIG_RTL8852D
	rtw_hal_bb_get_realtime_tp(phl, &total_tx_tput, &total_rx_tput);
//#endif	
	if (bb_ant_div->antdiv_multista_training_state_cnt == 0) {
		bb_ant_div->training_ant = bb_ant_div->pre_target_ant;
		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? MAIN_ANT : AUX_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "1. Next training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		bb_ant_div->training_ant = next_ant;
//#ifdef CONFIG_RTL8852D
		rtw_hal_bb_get_realtime_tp(phl, &total_tx_tput, &total_rx_tput);
//#endif
		if(total_tx_tput > 0 || total_rx_tput > 0 ){
			BB_DBG(bb, DBG_ANT_DIV, "Acc Tx Tput = %d, Rx Tput = %d, in %dms\n",total_tx_tput,total_rx_tput, bb_ant_div->antdiv_intvl);
			bb_ant_div->multista_antdiv_acc=1;
			if(bb_ant_div->training_ant == MAIN_ANT){
				bb_ant_div->main_tx_tput = total_tx_tput;
				bb_ant_div->main_rx_tput = total_rx_tput;
				bb_ant_div->antdiv_multista_training_state_cnt = bb_ant_div->antdiv_multista_training_state_cnt + 2;
			} else {
				bb_ant_div->aux_tx_tput = total_tx_tput;
				bb_ant_div->aux_rx_tput = total_rx_tput;
				bb_ant_div->antdiv_multista_training_state_cnt++;
			}
			bb_ant_div->antdiv_timer_i.cb_time= 1;
			halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
		} else {
			bb_ant_div->antdiv_multista_training_state_cnt++;
//#ifdef CONFIG_RTL8852D
			rtw_hal_bb_set_realtime_tp_timer(bb->hal_com, bb_ant_div->antdiv_intvl);
//#endif
			bb_ant_div->antdiv_timer_i.cb_time= bb_ant_div->antdiv_intvl;
			halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);
		}	
	} else if (bb_ant_div->antdiv_multista_training_state_cnt == 2) {
		bb_ant_div->antdiv_multista_training_state_cnt++;
		next_ant = (bb_ant_div->training_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
		BB_DBG(bb, DBG_ANT_DIV, "2. Next training ant =%s\n",
			  (next_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		halbb_antdiv_set_ant(bb, next_ant);
		bb_ant_div->training_ant = next_ant;
		if(bb_ant_div->multista_antdiv_acc==1){
			bb_ant_div->antdiv_timer_i.cb_time = bb_ant_div->antdiv_intvl*2;
		} else {
			bb_ant_div->antdiv_timer_i.cb_time = bb_ant_div->antdiv_intvl;
		}
		halbb_delay_ms(bb, bb_ant_div->multista_antdiv_ra_rdy_delay);
//#ifdef CONFIG_RTL8852D
		rtw_hal_bb_set_realtime_tp_timer(bb->hal_com, bb_ant_div->antdiv_timer_i.cb_time);
//#endif
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);	
	} else {
		bb_ant_div->get_stats = false;
//#ifdef CONFIG_RTL8852D
		rtw_hal_bb_get_realtime_tp(phl, &total_tx_tput, &total_rx_tput);
//#endif
		if(bb_ant_div->time_out_cnt > 3){
			bb_ant_div->time_out_cnt = 0;
			bb_ant_div->antdiv_multista_training_state_cnt++;
		} else if ((total_tx_tput) == 0 && (total_rx_tput == 0)){
			bb_ant_div->time_out_cnt ++;
			if(bb_ant_div->training_ant == MAIN_ANT){
				bb_ant_div->main_tx_tput = total_tx_tput;
				bb_ant_div->main_rx_tput = total_rx_tput;
			} else {
				bb_ant_div->aux_tx_tput = total_tx_tput;
				bb_ant_div->aux_rx_tput = total_rx_tput;
			}
		} else {
			if(bb_ant_div->training_ant == MAIN_ANT){
				bb_ant_div->main_tx_tput = total_tx_tput;
				bb_ant_div->main_rx_tput = total_rx_tput;
			} else {
				bb_ant_div->aux_tx_tput = total_tx_tput;
				bb_ant_div->aux_rx_tput = total_rx_tput;
			}
			bb_ant_div->get_stats = true;
			bb_ant_div->time_out_cnt = 0;
			bb_ant_div->antdiv_multista_training_state_cnt++;
			BB_DBG(bb, DBG_ANT_DIV, "Tx Tput = %d, Rx Tput = %d, in %dms\n",total_tx_tput,total_rx_tput, bb_ant_div->antdiv_intvl);
		}
		bb_ant_div->antdiv_timer_i.cb_time = 1;
		halbb_cfg_timers(bb, BB_SET_TIMER, &bb_ant_div->antdiv_timer_i);

	}
}

void halbb_tp_based_multista_antdiv(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	u8 target_ant_tput = 0;
	u32 main_trx_tput = 0;
	u32 aux_trx_tput = 0;

	//BB_DBG(bb, DBG_ANT_DIV,"halbb_tp_based_multista_antdiv = %d\n", bb_ant_div->antdiv_multista_training_state_cnt);
	if (bb_ant_div->antdiv_multista_training_state_cnt <= 3)
	{
		halbb_antdiv_multista_training_state(bb);
	} else {
		//decision State
		main_trx_tput = bb_ant_div->main_tx_tput + bb_ant_div->main_rx_tput;
		aux_trx_tput = bb_ant_div->aux_tx_tput + bb_ant_div->aux_rx_tput;
		BB_DBG(bb, DBG_ANT_DIV, "Main Tx Tput = %d, Rx Tput = %d, in %dms\n",bb_ant_div->main_tx_tput ,bb_ant_div->main_rx_tput, bb_ant_div->antdiv_intvl);
		BB_DBG(bb, DBG_ANT_DIV, "Aux  Tx Tput = %d, Rx Tput = %d, in %dms\n",bb_ant_div->aux_tx_tput, bb_ant_div->aux_rx_tput, bb_ant_div->antdiv_intvl);
		if((main_trx_tput == 0) && (aux_trx_tput == 0)){
			target_ant_tput = MAIN_ANT;
		}else if(bb_ant_div->pre_target_ant == MAIN_ANT){
			target_ant_tput = (aux_trx_tput > main_trx_tput + bb_ant_div->multista_tp_diff) ? AUX_ANT : MAIN_ANT;
		} else {
			target_ant_tput = (main_trx_tput > aux_trx_tput + bb_ant_div->multista_tp_diff) ? MAIN_ANT : AUX_ANT;
		}
		// target_ant_tput = (main_trx_tput == aux_trx_tput) ? (bb_ant_div->pre_target_ant) : ((main_trx_tput > aux_trx_tput) ? MAIN_ANT : AUX_ANT);
		bb_ant_div->target_ant_tp = target_ant_tput;
		halbb_antdiv_set_ant(bb, bb_ant_div->target_ant_tp);
		BB_DBG(bb, DBG_ANT_DIV,"Tput based Multi-STA TargetAnt= [%s]\n", (bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");
		//Reset Cnt
		bb_ant_div->antdiv_multista_training_state_cnt = 0;
		bb_ant_div->multista_antdiv_acc = 0;
		bb_ant_div->pre_target_ant = bb_ant_div->target_ant_tp;
//#ifdef CONFIG_RTL8852D
		rtw_hal_bb_set_realtime_tp_timer(bb->hal_com, 500);
//#endif
		BB_DBG(bb, DBG_DCR,"Main vs AUX = %d : %d, Target Ant = %s\n",main_trx_tput, aux_trx_tput,(bb_ant_div->target_ant_tp == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT");

	}
}

void halbb_antenna_diversity(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	struct bb_antdiv_rssi_info *rssi_stat = &bb_ant_div->bb_rssi_i;
	struct rtw_phl_com_t *phl = bb->phl_com;
	struct dev_cap_t *dev = &phl->dev_cap;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct rtw_phl_stainfo_t *sta = NULL;
	
	u8 rssi;
	u8 hw_md;
	u8 mode = 0, k=0;
	u16 macid=1;

	BB_DBG(bb, DBG_ANT_DIV, "%s ======>\n", __func__);
	BB_DBG(bb, DBG_ANT_DIV, "RFE_TYPE = %d\n", dev->rfe_type);
	//BB_TRACE("WD Ant Div Total   PKT CNT = %d \n", bb_cn_i->aux_cn_pkt_cnt+bb_cn_i->main_cn_pkt_cnt);
	halbb_antdiv_get_rssi(bb);
	rssi = rssi_stat->rssi_final >> 1;
	BB_DBG(bb, DBG_ANT_DIV, "RSSI=%d\n", rssi);

	if (halbb_antdiv_abort(bb))
		return;

	/*if (rssi < ANTDIV_RSSI_TH_HIGH) {
		BB_DBG(bb, DBG_ANT_DIV, "RSSI=%d is too low to do Ant_diveristy\n", rssi);
		halbb_antdiv_set_ant(bb, MAIN_ANT);
		return;
	}*/

	if (bb_link->one_entry_tp_active_occur) { // if TP > th., do antdiv per watchdog
		BB_DBG(bb, DBG_ANT_DIV, "TP occur, do antdv immediately\n");
		bb_ant_div->antdiv_wd_cnt = bb_ant_div->antdiv_period;
	}
	if (bb_ant_div->antdiv_wd_cnt < bb_ant_div->antdiv_period) { // else, do antdiv after specific watchdog
		BB_DBG(bb, DBG_ANT_DIV, "Ant-div period = %d, watchdog count = %d\n",
			bb_ant_div->antdiv_period, bb_ant_div->antdiv_wd_cnt);
		bb_ant_div->antdiv_wd_cnt++;
		return;
	} else {
		bb_ant_div->antdiv_wd_cnt = 0;
	}

	/* Fixed-antenna diversity mode */
	if (bb_ant_div->antdiv_mode != AUTO_ANT) {
		BB_DBG(bb, DBG_ANT_DIV, "Fix Antenna at (( %s ))\n",
			  (bb_ant_div->antdiv_mode == FIX_MAIN_ANT) ? "MAIN" : "AUX");
		if (bb_ant_div->antdiv_mode != bb_ant_div->pre_antdiv_mode) {
			if (bb_ant_div->antdiv_mode == FIX_MAIN_ANT) {
				halbb_antdiv_set_ant(bb, MAIN_ANT);
			}
			else if (bb_ant_div->antdiv_mode == FIX_AUX_ANT) {
				halbb_antdiv_set_ant(bb, AUX_ANT);
			}
		}
		bb_ant_div->pre_antdiv_mode = bb_ant_div->antdiv_mode;
		return;
	}
	
	/* Main section of EVM/TP based antenna diversity*/
	//BB_DBG(bb, DBG_ANT_DIV, "RSSI=%d, Ant_diversity start\n, TB Base Ant Div = %d\n", rssi, bb_ant_div->tb_antdiv_en);
	if ((bb_link->is_one_entry_only)) {	
		bb_ant_div->multista_antdiv = 0;
		//Get Active STA and check support AX/BE
		// for (k= 1; k < PHL_MAX_STA_NUM; k++)
		// {	
		// 	sta = bb->phl_sta_info[k];
		// 	if (!sta) {
		// 	//BB_DBG(bb, DBG_PWR_CTRL, "NULL PHL STA info\n");
		// 		continue;
		// 	}
		// 	if (is_sta_active(sta)) {
		// 		macid = k;
		// 		bb_ant_div->tb_antdiv_active_macid = k;
		// 		break;
		// 	}
		// }
		// sta = bb->phl_sta_info[bb_ant_div->tb_antdiv_active_macid];
		sta = bb->phl_sta_info[bb_link->one_entry_macid];
		BB_DBG(bb, DBG_ANT_DIV, "tb_antdiv_active_macid=%d, one_entry_macid=%d\n", 
		       bb_ant_div->tb_antdiv_active_macid, bb_link->one_entry_macid);

		bb_ant_div->tb_antdiv_active_macid = (u8)bb_link->one_entry_macid;
		
		if (!sta)
			return;
		mode = sta->wmode;
		hw_md = halbb_hw_mode_mapping(bb, mode, sta);
		if ((bb_ant_div->antdiv_method == TB_BASED_ANTDIV) && (hw_md & HE_SUPPORT))
		{
			BB_DBG(bb, DBG_ANT_DIV, "One Link and support AX");
			halbb_tb_based_antdiv(bb);
		} else {
			BB_DBG(bb, DBG_ANT_DIV, "One Link but not support AX");
			halbb_evm_based_antdiv(bb);
		}
	} else {
		BB_DBG(bb, DBG_ANT_DIV, "Multi STA Ant Div");
		bb_ant_div->multista_antdiv = 1;
		halbb_tp_based_multista_antdiv(bb);
	}
}

void halbb_antdiv_get_rssi_stat(struct bb_info *bb)
{
	struct bb_physts_rslt_hdr_info	*psts_h = &bb->bb_physts_i.bb_physts_rslt_hdr_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rssi_info *rssi = &bb_ant_div->bb_rssi_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u8 i = 0;

	if (rate_i->mode == BB_LEGACY_MODE) {
		if (cmn_rpt->is_cck_rate) {
			rssi->rssi_cck_avg_acc += psts_h->rssi_avg;
			rssi->pkt_cnt_cck++;
		} else {
			rssi->rssi_ofdm_avg_acc += psts_h->rssi_avg;
			rssi->pkt_cnt_ofdm++;
		}
	} else {
		rssi->rssi_t_avg_acc+= psts_h->rssi_avg;
		rssi->pkt_cnt_t++;
	}
}

void halbb_antdiv_get_evm_stat(struct bb_info *bb)
{
	struct bb_physts_rslt_1_info	*psts_1 = &bb->bb_physts_i.bb_physts_rslt_1_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;

	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_evm_info *bb_evm_i = &bb_ant_div->bb_evm_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;

	/* Only get stats @ training period */
	if (!bb_ant_div->get_stats)
		return;

	/* No EVM info. @ cck rate */
	if (cmn_rpt->is_cck_rate)
		return;

	if (bb_ant_div->evm_decision_method == EVM_LINEAR_AVG) {
		if(bb_ant_div->training_ant == MAIN_ANT) {
			if (rate_i->ss == 1) {
				bb_evm_i->main_evm_1ss += (u32)halbb_db_2_linear((psts_1->evm_min >> 2));
			} else {
				bb_evm_i->main_evm_min_acc += (u32)halbb_db_2_linear((psts_1->evm_min>> 2));

				bb_evm_i->main_evm_max_acc += (u32)halbb_db_2_linear((psts_1->evm_max >> 2));
			}
		} else if(bb_ant_div->training_ant == AUX_ANT) {
			if (rate_i->ss == 1) {
				bb_evm_i->aux_evm_1ss += (u32)halbb_db_2_linear((psts_1->evm_min >> 2));
			} else {
				bb_evm_i->aux_evm_min_acc += (u32)halbb_db_2_linear((psts_1->evm_min >> 2));

				bb_evm_i->aux_evm_max_acc += (u32)halbb_db_2_linear((psts_1->evm_max >> 2));
			}
		}
	} else {
		if(bb_ant_div->training_ant == MAIN_ANT) {
			if (rate_i->ss == 1) {
				bb_evm_i->main_evm_1ss += psts_1->evm_min;
			} else {
				bb_evm_i->main_evm_min_acc += psts_1->evm_min;

				bb_evm_i->main_evm_max_acc += psts_1->evm_max;
			}
		} else if(bb_ant_div->training_ant == AUX_ANT) {
			if (rate_i->ss == 1) {
				bb_evm_i->aux_evm_1ss += psts_1->evm_min;
			} else {
				bb_evm_i->aux_evm_min_acc += psts_1->evm_min;

				bb_evm_i->aux_evm_max_acc += psts_1->evm_max;
			}
		}
	}
}

void halbb_antdiv_get_cn_stat(struct bb_info *bb)
{
	struct bb_physts_rslt_13_info	*psts_13 = &bb->bb_physts_i.bb_physts_rslt_13_i;
	struct bb_physts_rslt_4_to_7_info	*psts_r = &bb->bb_physts_i.bb_physts_rslt_4_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;

	u8 ret = 0;

	//BB_TRACE("cn = %d\n", psts_13->bb_physts_uer_info[0].avg_cn_seg_cr );
	if (psts_13->bb_physts_uer_info[0].n_sts < 1){
		bb_cn_i->cn_invalid_cnt ++;
		//BB_TRACE("rate_i->ss == 0\n");
		return;
	}

	if (psts_13->bb_physts_uer_info[0].avg_cn_seg_cr == 0x7f) {
			//BB_TRACE("CN > 63.5\n");
		bb_cn_i->cn_invalid_cnt ++;
		return;
	}
	//BB_TRACE("Ant = %d, rate %d\n",psts_r->ant_idx, psts_13->bb_physts_uer_info[0].mcs);
	if(bb_ant_div->training_ant == MAIN_ANT) {
		bb_cn_i->main_cn_avg_acc += psts_13->bb_physts_uer_info[0].avg_cn_seg_cr;
		bb_cn_i->main_cn_pkt_cnt ++ ;
		BB_DBG(bb, DBG_ANT_DIV, "main_cn_pkt = %d, pack_CN=%d, main_cn = %d\n", bb_cn_i->main_cn_pkt_cnt, psts_13->bb_physts_uer_info[0].avg_cn_seg_cr, bb_cn_i->main_cn_avg_acc);
		//if(bb_cn_i->main_cn_pkt_cnt <= bb_ant_div->antdiv_train_num){
			//bb_ant_div->tb_antdiv_train_rate = 1;
		//	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		//}	
	} else if(bb_ant_div->training_ant == AUX_ANT) {
		bb_cn_i->aux_cn_avg_acc += psts_13->bb_physts_uer_info[0].avg_cn_seg_cr;
		bb_cn_i->aux_cn_pkt_cnt ++ ;
		BB_DBG(bb, DBG_ANT_DIV, "aux_cn_pkt = %d, pack_CN=%d, aux_cn = %d\n", bb_cn_i->aux_cn_pkt_cnt, psts_13->bb_physts_uer_info[0].avg_cn_seg_cr, bb_cn_i->aux_cn_avg_acc);
		//if(bb_cn_i->aux_cn_pkt_cnt <= bb_ant_div->antdiv_train_num){
			//bb_ant_div->tb_antdiv_train_rate = 3;
		//	ret = halbb_ulmacid_cfg_fixed(bb, 1, bb_ant_div->tb_antdiv_train_rate, bb_ant_div->tb_antdiv_train_bw, 1, bb_ant_div->tb_antdiv_active_macid, 0);
		//}		
	}
}

void halbb_antdiv_get_per_ant_rssi(struct bb_info *bb)
{
	struct bb_physts_rslt_13_info	*psts_13 = &bb->bb_physts_i.bb_physts_rslt_13_i;
	struct bb_physts_rslt_4_to_7_info	*psts_r = &bb->bb_physts_i.bb_physts_rslt_4_i;
	struct bb_physts_rslt_hdr_info	*psts_h = &bb->bb_physts_i.bb_physts_rslt_hdr_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;

	//BB_TRACE("Ant = %d, RSSI = %d\n",bb_ant_div->training_ant,psts_h->rssi[0] >> 1 );

	//psts_r->ant_idx 
	//BB_DBG(bb, DBG_ANT_DIV, "Ant Idx = %d\n", psts_r->ant_idx);
	if(bb_ant_div->training_ant == MAIN_ANT) {
		if (rate_i->mode == BB_LEGACY_MODE) {
			//nothing
		} else {
			bb_cn_i->main_rssi = (psts_h->rssi[0] >> 1);
			//BB_DBG(bb, DBG_ANT_DIV, "main_rssi = %d, main_rssi>>1=%d\n", (psts_h->rssi[0] >> 1), (psts_h->rssi[0] >> 1)>>1);
			bb_cn_i->main_rssi_ma = (bb_cn_i->main_rssi_ma + (psts_h->rssi[0] >> 1))>>1;
			bb_cn_i->main_rssi_cnt ++ ;
		}
	} else if (bb_ant_div->training_ant == AUX_ANT) {
		if (rate_i->mode == BB_LEGACY_MODE) {
			//nothing
		} else {
			bb_cn_i->aux_rssi = (psts_h->rssi[0] >> 1);
			//BB_DBG(bb, DBG_ANT_DIV, "aux_rssi = %d, aux_rssi>>1=%d\n", (psts_h->rssi[0] >> 1), (psts_h->rssi[0] >> 1)>>1);
			bb_cn_i->aux_rssi_ma = (bb_cn_i->aux_rssi_ma + (psts_h->rssi[0] >> 1))>>1;
			bb_cn_i->aux_rssi_cnt ++ ;
		}
	}

}


void halbb_antdiv_get_rate_stat_old(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	//struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;

	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	enum channel_width bw_curr; /*max bw in current link mode*/
	u8 band_idx;
	u8 ofst = rate_i->idx;
	u8 ss_idx = 0;

	/* Only get stats @ training period */
	if (!bb_ant_div->get_stats)
		return;

	/* Data frame only */
	#if 0
	if (bb_ant_div->antdiv_use_ctrl_frame) {
		if (!bb_link->is_match_bssid)
			return;
	}
	#endif

	//BB_DBG(bb, DBG_ANT_DIV, "Rate mode= %d\n", rate_i->mode);
	//BB_DBG(bb, DBG_ANT_DIV, "Training antenna= %d\n", bb_ant_div->training_ant);

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && bb->bb_phy_idx == HW_PHY_1) {
		bw_curr = rx_bw;
	} else
#endif
	{
		band_idx = (desc->phy_idx == HW_PHY_0) ? 0 : 1;
		bw_curr = bb->hal_com->band[band_idx].cur_chandef.bw;
	}

	if(bb_ant_div->training_ant == MAIN_ANT) {
		/* pkt_cnt acc */
		//BB_DBG(bb, DBG_ANT_DIV, "MAIN_ANT rate!\n");
		bb_rate_i->main_cnt_all++;
		if (rate_i->mode == BB_LEGACY_MODE) {
			if (cmn_rpt->is_cck_rate)
				bb_rate_i->main_pkt_cnt_cck++;
			else
				bb_rate_i->main_pkt_cnt_ofdm++;
		} else {
			bb_rate_i->main_pkt_cnt_t++;
			if (rate_i->ss == 1)
				bb_rate_i->main_pkt_cnt_1ss++;
			else if (rate_i->ss == 2)
				bb_rate_i->main_pkt_cnt_2ss++;
		}

		/* rate cnt acc*/
		if (rate_i->mode == BB_LEGACY_MODE) {
			bb_rate_i->main_pkt_cnt_legacy[ofst]++;
			return;
		}


		if (rate_i->ss >= 2 && rate_i->mode >= BB_VHT_MODE)
			ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

		if (rate_i->mode == BB_HT_MODE) {
			bb_rate_i->main_ht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HT_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->main_pkt_cnt_ht[ofst]++;
				/* shift ofst due to mismatch of HT/VHT rate num*/
				ss_idx = ofst / HT_NUM_MCS;
				ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;
				bb_rate_i->main_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_VHT_MODE) {
			bb_rate_i->main_vht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->main_pkt_cnt_vht[ofst]++;
				bb_rate_i->main_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_HE_MODE) {
			bb_rate_i->main_he_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->main_pkt_cnt_he[ofst]++;
				bb_rate_i->main_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		}

	} else if(bb_ant_div->training_ant == AUX_ANT) {
		bb_rate_i->aux_cnt_all++;
		//BB_DBG(bb, DBG_ANT_DIV, "Aux_ANT rate!\n");
		/* pkt_cnt acc */
		if (rate_i->mode == BB_LEGACY_MODE) {
			if (cmn_rpt->is_cck_rate)
				bb_rate_i->aux_pkt_cnt_cck++;
			else
				bb_rate_i->aux_pkt_cnt_ofdm++;
		} else {
			bb_rate_i->aux_pkt_cnt_t++;
			if (rate_i->ss == 1)
				bb_rate_i->aux_pkt_cnt_1ss++;
			else if (rate_i->ss == 2)
				bb_rate_i->aux_pkt_cnt_2ss++;
		}

		if (rate_i->mode == BB_LEGACY_MODE) {
			bb_rate_i->aux_pkt_cnt_legacy[ofst]++;
			return;
		}

		if (rate_i->ss >= 2 && rate_i->mode >= BB_VHT_MODE)
			ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

		if (rate_i->mode == BB_HT_MODE) {
			bb_rate_i->aux_ht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HT_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->aux_pkt_cnt_ht[ofst]++;

				/* shift ofst due to mismatch of HT/VHT rate num*/
				ss_idx = ofst / HT_NUM_MCS;
				ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;
				bb_rate_i->aux_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_VHT_MODE) {
			bb_rate_i->aux_vht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->aux_pkt_cnt_vht[ofst]++;
				bb_rate_i->aux_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_HE_MODE) {
			bb_rate_i->aux_he_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);
			if (rx_bw == bw_curr) {
				bb_rate_i->aux_pkt_cnt_he[ofst]++;
				bb_rate_i->aux_tp +=
					      bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		}
	}
}

void halbb_antdiv_get_rate_stat_new(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	//struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_rate_info *bb_rate_i = &bb_ant_div->bb_rate_i;
	struct bb_link_info *bb_link = &bb->bb_link_i;
	enum channel_width bw_curr; /*max bw in current link mode*/
	u8 band_idx;
	u8 ofst = rate_i->idx;
	u8 ss_idx = 0;

	/* Only get stats @ training period */
	if (!bb_ant_div->get_stats)
		return;

	/* Data frame only */
	#if 0
	if (bb_ant_div->antdiv_use_ctrl_frame) {
		if (!bb_link->is_match_bssid)
			return;
	}
	#endif

	//BB_DBG(bb, DBG_ANT_DIV, "Rate mode= %d\n", rate_i->mode);
	//BB_DBG(bb, DBG_ANT_DIV, "Training antenna= %d\n", bb_ant_div->training_ant);

#ifdef HALBB_DBCC_SUPPORT
	if (bb->hal_com->dbcc_en && bb->bb_phy_idx == HW_PHY_1) {
		bw_curr = rx_bw;
	} else
#endif
	{
		band_idx = (desc->phy_idx == HW_PHY_0) ? 0 : 1;
		bw_curr = bb->hal_com->band[band_idx].cur_chandef.bw;
	}

	if(bb_ant_div->training_ant == MAIN_ANT) {
		/* pkt_cnt acc */
		//BB_DBG(bb, DBG_ANT_DIV, "MAIN_ANT rate!\n");
		bb_rate_i->main_cnt_all++;
		if (rate_i->mode == BB_LEGACY_MODE) {
			if (cmn_rpt->is_cck_rate)
				bb_rate_i->main_pkt_cnt_cck++;
			else
				bb_rate_i->main_pkt_cnt_ofdm++;
		} else {
			bb_rate_i->main_pkt_cnt_t++;
			if (rate_i->ss == 1)
				bb_rate_i->main_pkt_cnt_1ss++;
			else if (rate_i->ss == 2)
				bb_rate_i->main_pkt_cnt_2ss++;
		}

		/* rate cnt acc*/
		if (rate_i->mode == BB_LEGACY_MODE) {
			bb_rate_i->main_pkt_cnt_legacy[ofst]++;
			return;
		}


		if (rate_i->ss >= 2 && rate_i->mode >= BB_VHT_MODE)
			ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

		if (rate_i->mode == BB_HT_MODE) {
			bb_rate_i->main_ht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HT_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->main_pkt_cnt_he_sc20[ofst]++;
					bb_rate_i->main_sc20_occur = true;
					bb_rate_i->main_tp +=
						100 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->main_pkt_cnt_ht[ofst]++;

					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->main_tp +=
						208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				default:
					bb_rate_i->main_pkt_cnt_ht[ofst]++;

					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->main_tp +=
						208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_VHT_MODE) {
			bb_rate_i->main_vht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					bb_rate_i->main_pkt_cnt_vht_sc20[ofst]++;
					bb_rate_i->main_sc20_occur = true;
					bb_rate_i->main_tp +=
					      100 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->main_pkt_cnt_vht_sc40[ofst]++;
					bb_rate_i->main_sc40_occur = true;
					bb_rate_i->main_tp +=
					      208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_80:
					bb_rate_i->main_pkt_cnt_vht_sc80[ofst]++;
					bb_rate_i->main_sc80_occur = true;
					bb_rate_i->main_tp +=
					      450 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_160:
					bb_rate_i->main_pkt_cnt_vht[ofst]++;
					bb_rate_i->main_tp +=
					   	900 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				default:
					bb_rate_i->main_pkt_cnt_vht[ofst]++;
					bb_rate_i->main_tp +=
					   	900 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_HE_MODE) {
			bb_rate_i->main_he_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					bb_rate_i->main_sc20_occur = true;
					bb_rate_i->main_pkt_cnt_he_sc20[ofst]++;
					bb_rate_i->main_tp +=
						113 *bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->main_sc40_occur = true;
					bb_rate_i->main_pkt_cnt_he_sc40[ofst]++;
					bb_rate_i->main_tp +=
							226 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_80:
					bb_rate_i->main_sc80_occur = true;
					bb_rate_i->main_pkt_cnt_he_sc80[ofst]++;
					bb_rate_i->main_tp +=
						474 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];//1.13 * 4.19
				break;
				case CHANNEL_WIDTH_160:
					bb_rate_i->main_pkt_cnt_he[ofst]++;
					bb_rate_i->main_tp +=
						947 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM]; //1.13 * 8.38
				break;
				default:
					bb_rate_i->main_pkt_cnt_he[ofst]++;
					bb_rate_i->main_tp +=
						947 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM]; //1.13 * 8.38
			}
		}

	} else if(bb_ant_div->training_ant == AUX_ANT) {
		bb_rate_i->aux_cnt_all++;
		//BB_DBG(bb, DBG_ANT_DIV, "Aux_ANT rate!\n");
		/* pkt_cnt acc */
		if (rate_i->mode == BB_LEGACY_MODE) {
			if (cmn_rpt->is_cck_rate)
				bb_rate_i->aux_pkt_cnt_cck++;
			else
				bb_rate_i->aux_pkt_cnt_ofdm++;
		} else {
			bb_rate_i->aux_pkt_cnt_t++;
			if (rate_i->ss == 1)
				bb_rate_i->aux_pkt_cnt_1ss++;
			else if (rate_i->ss == 2)
				bb_rate_i->aux_pkt_cnt_2ss++;
		}

		if (rate_i->mode == BB_LEGACY_MODE) {
			bb_rate_i->aux_pkt_cnt_legacy[ofst]++;
			return;
		}

		if (rate_i->ss >= 2 && rate_i->mode >= BB_VHT_MODE)
			ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

		if (rate_i->mode == BB_HT_MODE) {
			bb_rate_i->aux_ht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HT_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->aux_pkt_cnt_he_sc20[ofst]++;
					bb_rate_i->aux_sc20_occur = true;
					bb_rate_i->aux_tp +=
					    100 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->aux_pkt_cnt_ht[ofst]++;

					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->aux_tp +=
						208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				default:
					bb_rate_i->aux_pkt_cnt_ht[ofst]++;

					/* shift ofst due to mismatch of HT/VHT rate num*/
					ss_idx = ofst / HT_NUM_MCS;
					ofst = (ofst % HT_NUM_MCS) + ss_idx * HE_VHT_NUM_MCS;

					bb_rate_i->aux_tp +=
						208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_VHT_MODE) {
			bb_rate_i->aux_vht_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					bb_rate_i->aux_pkt_cnt_vht_sc20[ofst]++;
					bb_rate_i->aux_sc20_occur = true;
					bb_rate_i->aux_tp +=
					      100 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->aux_pkt_cnt_vht_sc40[ofst]++;
					bb_rate_i->aux_sc40_occur = true;
					bb_rate_i->aux_tp +=
					      208 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_80:
					bb_rate_i->aux_pkt_cnt_vht_sc80[ofst]++;
					bb_rate_i->aux_sc80_occur = true;
					bb_rate_i->aux_tp +=
					      450 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_160:
					bb_rate_i->aux_pkt_cnt_vht[ofst]++;
					bb_rate_i->aux_tp +=
						900 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				default:
					bb_rate_i->aux_pkt_cnt_vht[ofst]++;
					bb_rate_i->aux_tp +=
						900 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		} else if (rate_i->mode == BB_HE_MODE) {
			bb_rate_i->aux_he_pkt_not_zero = true;
			ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);
			switch(rx_bw){
				case CHANNEL_WIDTH_20:
					bb_rate_i->aux_sc20_occur = true;
					bb_rate_i->aux_pkt_cnt_he_sc20[ofst]++;
					bb_rate_i->aux_tp +=
						113 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_40:
					bb_rate_i->aux_sc40_occur = true;
					bb_rate_i->aux_pkt_cnt_he_sc40[ofst]++;
					bb_rate_i->aux_tp +=
					      226 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_80:
					bb_rate_i->aux_sc80_occur = true;
					bb_rate_i->aux_pkt_cnt_he_sc80[ofst]++;
					bb_rate_i->aux_tp +=
						474 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				case CHANNEL_WIDTH_160:
					bb_rate_i->aux_pkt_cnt_he[ofst]++;
					bb_rate_i->aux_tp +=
						947 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
				break;
				default:
					bb_rate_i->aux_pkt_cnt_he[ofst]++;
					bb_rate_i->aux_tp +=
						947 * bb_phy_rate_table[ofst + LEGACY_RATE_NUM];
			}
		}
	}
}

void halbb_antdiv_get_rate_stat(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;

	if (bb_ant_div->ant_div_new == 1)
		halbb_antdiv_get_rate_stat_new(bb, desc, rx_bw);
	else
		halbb_antdiv_get_rate_stat_old(bb, desc, rx_bw);

}

void halbb_antdiv_phy_sts(struct bb_info *bb, u32 physts_bitmap,
		       struct physts_rxd *desc) {

	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_1_info *psts_1 = &physts->bb_physts_rslt_1_i;
	struct dev_cap_t *dev = &bb->phl_com->dev_cap;
	struct rtw_phl_stainfo_t *sta;
	struct rtw_cfo_info *cfo_t = NULL;
	enum channel_width rx_bw = psts_1->bw_idx;
	struct bb_cmn_rpt_info *cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	u8 bb_macid;

	halbb_antdiv_get_rssi_stat(bb);
	

	if (!(physts_bitmap & BIT(IE01_CMN_OFDM) &&
	    physts->bb_physts_rslt_hdr_i.ie_map_type >= TRIG_BASE_PPDU)){
		//BB_TRACE("type miss match\n");
		return;
	}
	if(!desc->is_su) {
		if (rate_i->mode <= BB_HT_MODE){

		} else {
			halbb_antdiv_get_cn_stat(bb);
		}
	}
	//if(!desc->is_su)
	//	halbb_antdiv_get_cn_stat(bb);


	if (desc->macid_su >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s] macid_su=%d\n", __func__, desc->macid_su);
		return;
	}

	bb_macid = bb->phl2bb_macid_table[desc->macid_su];

	if (bb_macid >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s] bb_macid=%d\n", __func__, bb_macid);
		return;
	}

	sta = bb->phl_sta_info[bb_macid];
	
	if (!is_sta_active(sta))
		return;

	if (sta->macid >= PHL_MAX_STA_NUM)
		return;

	if (!sta->hal_sta)
		return;

	if ((dev->rfe_type >= 50) && (bb_macid == 0)) /* No need to cnt AP Rx boardcast pkt*/
		return;
	bb_ant_div->tb_antdiv_active_macid = bb_macid;
	halbb_antdiv_get_per_ant_rssi(bb);
	halbb_antdiv_get_rate_stat(bb, desc, rx_bw);
	halbb_antdiv_get_evm_stat(bb);

}
void halbb_antdiv_dbg(struct bb_info *bb, char input[][16], u32 *_used,
			      char *output, u32 *_out_len)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cr_info *cr = &bb->bb_ant_div_i.bb_antdiv_cr_i;

	char help[] = "-h";
	u32 var[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u32 ret = 0;

	if ((_os_strcmp(input[1], help) == 0)) {
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set Antenna Diversity Mode: {1} {0}: Auto-mode {1}: Fix main ant. {2}: Fix aux ant.\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set Antenna Diversity Method: {2} {0}: EVM based {1}: CN based {2}: TB based\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set training period: {3} {num of watchdog} (How much watchdog to do one ant-div)\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set training state number: {4} {num}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set training interval: {5} {ms} (how long for one training state)\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set training delay: {6} {ms} (how long to wait RA stable)\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Set decision threshold: {7} {TP high th.} {TP low th.} {EVM th.}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "TP lower bound: {8} {th.}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "TP decision method: {9} {0}: Max cnt domination {1}: highest rate domination {2}: Avg. TP domination\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "EVM decision method: {10} {0}:Linear avg. {1}: dB avg.\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "New ant_div method enable: {11} {0}:old code. {1}: new code: extend to sub BW MCS.\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "TX ant set method: {18} {0}:by BB r_tx_antidx {1}: by BB wrapper.\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "Show all parameter: {100}\n");
		BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			 "[ONLY DEBUG] {101} 0x586c[16]={0,1}\n");
	} else {
		HALBB_SCAN(input[1], DCMD_DECIMAL, &var[0]);

		if (var[0] == 1) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_mode = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div Mode=%d\n", bb_ant_div->antdiv_mode);
			if (bb_ant_div->antdiv_mode == FIX_MAIN_ANT) {
				halbb_antdiv_set_ant(bb, MAIN_ANT);
				bb_ant_div->target_ant = MAIN_ANT;
				bb_ant_div->pre_target_ant = MAIN_ANT;
			} else if (bb_ant_div->antdiv_mode == FIX_AUX_ANT) {
				halbb_antdiv_set_ant(bb, AUX_ANT);
				bb_ant_div->target_ant = AUX_ANT;
				bb_ant_div->pre_target_ant = AUX_ANT;
			} else if (bb_ant_div->antdiv_mode == AUTO_ANT)
				halbb_antdiv_set_ant(bb, bb_ant_div->pre_target_ant);
		} else if (var[0] == 2) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_method = var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training method=%d\n", bb_ant_div->antdiv_method);
		} else if (var[0] == 3) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_period = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div period=%d watchdog\n", bb_ant_div->antdiv_period);
		} else if (var[0] == 4) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_train_num = var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training state num=%d\n", bb_ant_div->antdiv_train_num);
		} else if (var[0] == 5) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_intvl = var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training interval=%d\n", bb_ant_div->antdiv_intvl);
		} else if (var[0] == 6) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->antdiv_delay = var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training delay=%d\n", bb_ant_div->antdiv_delay);
		} else if (var[0] == 7) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			HALBB_SCAN(input[3], DCMD_DECIMAL, &var[2]);
			HALBB_SCAN(input[4], DCMD_DECIMAL, &var[3]);
			bb_ant_div->tp_diff_th_high = (u16)var[1];
			bb_ant_div->tp_diff_th_low = (u16)var[2];
			bb_ant_div->evm_diff_th = (u8)var[3];

			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Decision threshold: {TP high = %d} {TP low = %d} {EVM = %d}\n",
				 bb_ant_div->tp_diff_th_high,
				 bb_ant_div->tp_diff_th_low,
				 bb_ant_div->evm_diff_th);
		} else if (var[0] == 8) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->tp_lb = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "TP lower bound=%d\n", bb_ant_div->tp_lb);
		} else if (var[0] == 9) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->tp_decision_method = (u8)var[1];
			BB_DBG(bb, DBG_ANT_DIV, "TP decision method=(( %d ))\n",
						bb_ant_div->tp_decision_method);
		} else if (var[0] == 10) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->evm_decision_method = (u8)var[1];
			BB_DBG(bb, DBG_ANT_DIV, "EVM decision method=(( %d ))\n",
						bb_ant_div->evm_decision_method);
		} else if (var[0] == 11) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->ant_div_new = (bool)var[1];
			BB_DBG(bb, DBG_ANT_DIV, "New ant_div method enable =(( %d ))\n",
						bb_ant_div->ant_div_new);
		} else if  (var[0] == 12) {		
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			HALBB_SCAN(input[3], DCMD_DECIMAL, &var[2]);
			HALBB_SCAN(input[4], DCMD_DECIMAL, &var[3]);
			HALBB_SCAN(input[5], DCMD_DECIMAL, &var[4]);
			HALBB_SCAN(input[6], DCMD_DECIMAL, &var[5]);
			HALBB_SCAN(input[7], DCMD_DECIMAL, &var[6]);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
					"Fixed Trigger Frame SS=%d, MCS Rate = %d, BW = %d, enable = %d, macid = %d, cont tx = %d\n",
					(u8)var[1], (u8)var[2], (u8)var[3], (u8)var[4], (u8)var[5], (u8)var[6]);
			ret = halbb_ulmacid_cfg_fixed(bb, (u8)var[1], (u8)var[2], (u8)var[3], (u8)var[4], (u8)var[5], (u8)var[6]);
		} else if  (var[0] == 13) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			HALBB_SCAN(input[3], DCMD_DECIMAL, &var[2]);
			bb_ant_div->tb_antdiv_train_rate = (u8)var[1];
			bb_ant_div->tb_antdiv_train_bw = (u8)var[2];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
					"TB Ant Div Training Rate =%d, BW = %d\n",
					(u8)var[1], (u8)var[2]);
		} else if  (var[0] == 14) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->tb_antdiv_train_num = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
					"TB Ant Div Training Num %d\n", (u8)var[1]);
		} else if  (var[0] == 15) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->tb_antdiv_train_en = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"TB Ant Div Training State %d\n", (u8)var[1]);	
		} else if  (var[0] == 16) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			HALBB_SCAN(input[3], DCMD_DECIMAL, &var[2]);
			bb_ant_div->tb_antdiv_rssi_diff = (u8)var[1];
			bb_ant_div->tb_antdiv_cn_diff = (u8)var[2];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"TB Ant Div Decision RSSI Diff = %d, CN Diff = %d\n",bb_ant_div->tb_antdiv_rssi_diff, bb_ant_div->tb_antdiv_cn_diff);	
		} else if  (var[0] == 17) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->multista_antdiv_ra_rdy_delay  = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"Muti-STA RA RDY Delay = %d\n", bb_ant_div->multista_antdiv_ra_rdy_delay);
		} else if  (var[0] == 18) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->tx_ant_by_bbwrapper = (bool)var[1];
			if (bb_ant_div->tx_ant_by_bbwrapper)
				halbb_set_reg_cmn(bb, cr->path0_r_tx_ant_sel, cr->path0_r_tx_ant_sel_m, 0x1, HW_PHY_0);
			else
				halbb_set_reg_cmn(bb, cr->path0_r_tx_ant_sel, cr->path0_r_tx_ant_sel_m, 0x0, HW_PHY_0);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"tx_ant_by_bbwrapper = %d\n", bb_ant_div->tx_ant_by_bbwrapper);
		} else if (var[0] == 19) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->one_path_CL_pass_WD_lim = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"one path CL case: max of WD that can be skipped = %d\n", bb_ant_div->one_path_CL_pass_WD_lim);
		} else if (var[0] == 20) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->two_path_CL_pass_WD_lim = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"two path CL case: max of WD that can be skipped = %d\n", bb_ant_div->two_path_CL_pass_WD_lim);
		} else if (var[0] == 21) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->one_path_CL_fix_ant_rssi_diff = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"RSSI diff at one path CL = %d\n", bb_ant_div->one_path_CL_fix_ant_rssi_diff);
		} else if (var[0] == 22) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->two_path_CL_fix_ant_rssi_diff = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"RSSI diff at two path CL = %d\n", bb_ant_div->two_path_CL_fix_ant_rssi_diff);
		} else if (var[0] == 23) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			bb_ant_div->multista_tp_diff = (u8)var[1];
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"multista TP diff = %d\n", bb_ant_div->multista_tp_diff);
		} else if (var[0] == 100) {
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div Mode = {%d}\n", bb_ant_div->antdiv_mode);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training method = {%d}\n", bb_ant_div->antdiv_method);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "TP decision method = {%d}\n", bb_ant_div->tp_decision_method);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "EVM decision method = {%d}\n", bb_ant_div->evm_decision_method);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div period = {#%d-watchdog}\n", bb_ant_div->antdiv_period);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training state num = {%d}\n", bb_ant_div->antdiv_train_num);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training interval = {%d}\n", bb_ant_div->antdiv_intvl);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Ant-Div training delay = {%d}\n", bb_ant_div->antdiv_delay);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Tx ant set by BB wrapper = {%d}\n", bb_ant_div->tx_ant_by_bbwrapper);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "Decision threshold: {TP high = %d} {TP low = %d} {EVM = %d}\n",
				 bb_ant_div->tp_diff_th_high,
				 bb_ant_div->tp_diff_th_low,
				 bb_ant_div->evm_diff_th);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "TP lower bound = {%d}\n", bb_ant_div->tp_lb);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "New ant_div method enable = {%d}\n", bb_ant_div->ant_div_new);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "TB Ant_Div Para : Training Num : %d, Training rate = %d, Training BW = %d\n",
				 bb_ant_div->tb_antdiv_train_num,
				 bb_ant_div->tb_antdiv_train_rate,
				 bb_ant_div->tb_antdiv_train_bw);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "TB Ant_Div Para : RSSI Diff : %d, CN Diff = %d\n",
				 bb_ant_div->tb_antdiv_rssi_diff,
				 bb_ant_div->tb_antdiv_cn_diff);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"Muti-STA RA RDY Delay = %d\n", bb_ant_div->multista_antdiv_ra_rdy_delay);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"one path CL case: max of WD that can be skipped = %d\n", bb_ant_div->one_path_CL_pass_WD_lim);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"two path CL case: max of WD that can be skipped = %d\n", bb_ant_div->two_path_CL_pass_WD_lim);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"RSSI diff at one path CL = %d\n", bb_ant_div->one_path_CL_fix_ant_rssi_diff);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"RSSI diff at two path CL = %d\n", bb_ant_div->two_path_CL_fix_ant_rssi_diff);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				"multista TP diff = %d\n", bb_ant_div->multista_tp_diff);
			// BB_DBG_CNSL(out_len, used, output + used, out_len - used,
			// 	"TP diff at CL = %d\n", bb_ant_div->CL_fix_ant_TP_diff);
		} else if (var[0] == 101) {
			HALBB_SCAN(input[2], DCMD_DECIMAL, &var[1]);
			halbb_set_reg_cmn(bb, cr->path0_r_antsel, BIT(16), var[1], HW_PHY_0);
			BB_DBG_CNSL(out_len, used, output + used, out_len - used,
				 "[ONLY DEBUG] 0x1586c[16] = %d\n", var[1]);

		}
	}

	*_used = used;
	*_out_len = out_len;
}

void halbb_antdiv_io_en(struct bb_info *bb)
{
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u8 ret=0;

	//BB_DBG(bb, DBG_ANT_DIV, "Callback bb_ant_div->multista_antdiv = %d", bb_ant_div->multista_antdiv);
	if(bb_ant_div->multista_antdiv == 1) {
		halbb_tp_based_multista_antdiv(bb);
	} else if ((bb_ant_div->antdiv_method == TB_BASED_ANTDIV) && (bb_ant_div->multista_antdiv == 0)) {
		if(bb_ant_div->antdiv_tb_training_state_cnt == 0)
		{
			if(bb_ant_div->tb_antdiv_train_next == 1)
			{
				//Reset Cnt
				bb_cn_i->main_cn_pkt_cnt = 0;
				bb_cn_i->main_cn_avg_acc = 0;
				bb_cn_i->aux_cn_pkt_cnt = 0;
				bb_cn_i->aux_cn_avg_acc = 0;
				halbb_antdiv_get_cn_in_watchdog(bb);
			}
		}else{
			halbb_tb_based_antdiv(bb);
		}
	}else
		halbb_evm_based_antdiv(bb);

	
}

void halbb_antdiv_callback(void *context)
{
	struct bb_info *bb = (struct bb_info *)context;
	struct halbb_timer_info *timer = &bb->bb_ant_div_i.antdiv_timer_i;

	//BB_DBG(bb, DBG_ANT_DIV, "[%s]===>\n", __func__);
	timer->timer_state = BB_TIMER_IDLE;

	if (bb->phl_com->hci_type == RTW_HCI_PCIE)
		halbb_antdiv_io_en(bb);
	else
		rtw_hal_cmd_notify(bb->phl_com, MSG_EVT_NOTIFY_BB, (void *)(&timer->event_idx), bb->bb_phy_idx);
}

void halbb_antdiv_timer_init(struct bb_info *bb)
{
	struct halbb_timer_info *timer = &bb->bb_ant_div_i.antdiv_timer_i;

	BB_DBG(bb, DBG_ANT_DIV, "[%s]\n", __func__);

	timer->event_idx = BB_EVENT_TIMER_ANTDIV;
	timer->timer_state = BB_TIMER_IDLE;

	halbb_init_timer(bb, &timer->timer_list, halbb_antdiv_callback, bb, "halbb_antdiv_timer");
}

void halbb_cr_cfg_antdiv_init(struct bb_info *bb)
{
	struct bb_antdiv_cr_info *cr = &bb->bb_ant_div_i.bb_antdiv_cr_i;

	switch (bb->cr_type) {
	#ifdef HALBB_COMPILE_AP_SERIES
	case BB_AP:
		cr->path0_r_ant_train_en = PATH0_R_ANT_TRAIN_EN_A;
		cr->path0_r_ant_train_en_m = PATH0_R_ANT_TRAIN_EN_A_M;
		cr->path0_r_tx_ant_sel = PATH0_R_TX_ANT_SEL_A;
		cr->path0_r_tx_ant_sel_m = PATH0_R_TX_ANT_SEL_A_M;
		cr->path0_r_rfe_buf_en = PATH0_R_RFE_BUF_EN_A;
		cr->path0_r_rfe_buf_en_m = PATH0_R_RFE_BUF_EN_A_M;
		cr->path0_r_lnaon_agc = PATH0_R_LNAON_AGC_A;
		cr->path0_r_lnaon_agc_m = PATH0_R_LNAON_AGC_A_M;
		cr->path0_r_trsw_bit_bt = PATH0_R_TRSW_BIT_BT_A;
		cr->path0_r_trsw_bit_bt_m = PATH0_R_TRSW_BIT_BT_A_M;
		cr->path0_r_trsw_s = PATH0_R_TRSW_S_A;
		cr->path0_r_trsw_s_m = PATH0_R_TRSW_S_A_M;
		cr->path0_r_trsw_o = PATH0_R_TRSW_O_A;
		cr->path0_r_trsw_o_m = PATH0_R_TRSW_O_A_M;
		cr->path0_r_trswb_o = PATH0_R_TRSWB_O_A;
		cr->path0_r_trswb_o_m = PATH0_R_TRSWB_O_A_M;
		cr->path0_r_bt_force_antidx = PATH0_R_BT_FORCE_ANTIDX_A;
		cr->path0_r_bt_force_antidx_m = PATH0_R_BT_FORCE_ANTIDX_A_M;
		cr->path0_r_bt_force_antidx_en = PATH0_R_BT_FORCE_ANTIDX_EN_A;
		cr->path0_r_bt_force_antidx_en_m = PATH0_R_BT_FORCE_ANTIDX_EN_A_M;
		cr->path0_r_ant_module_rfe_opt = PATH0_R_ANT_MODULE_RFE_OPT_A;
		cr->path0_r_ant_module_rfe_opt_m = PATH0_R_ANT_MODULE_RFE_OPT_A_M;
		cr->path0_r_rfsw_tr = PATH0_R_RFSW_TR_A;
		cr->path0_r_rfsw_tr_m = PATH0_R_RFSW_TR_A_M;
		cr->path0_r_antsel = PATH0_R_ANTSEL_A;
		cr->path0_r_antsel_m = PATH0_R_ANTSEL_A_M;
		cr->path0_r_rfsw_ant_31_0 = PATH0_R_RFSW_ANT_31_0__A;
		cr->path0_r_rfsw_ant_31_0_m = PATH0_R_RFSW_ANT_31_0__A_M;
		cr->path0_r_rfsw_ant_63_32 = PATH0_R_RFSW_ANT_63_32__A;
		cr->path0_r_rfsw_ant_63_32_m = PATH0_R_RFSW_ANT_63_32__A_M;
		cr->path0_r_rfsw_ant_95_64 = PATH0_R_RFSW_ANT_95_64__A;
		cr->path0_r_rfsw_ant_95_64_m = PATH0_R_RFSW_ANT_95_64__A_M;
		cr->path0_r_rfsw_ant_127_96 = PATH0_R_RFSW_ANT_127_96__A;
		cr->path0_r_rfsw_ant_127_96_m = PATH0_R_RFSW_ANT_127_96__A_M;
		break;

	#endif
	#ifdef HALBB_COMPILE_CLIENT_SERIES
	case BB_CLIENT:
		cr->path0_r_ant_train_en = PATH0_R_ANT_TRAIN_EN_C;
		cr->path0_r_ant_train_en_m = PATH0_R_ANT_TRAIN_EN_C_M;
		cr->path0_r_tx_ant_sel = PATH0_R_TX_ANT_SEL_C;
		cr->path0_r_tx_ant_sel_m = PATH0_R_TX_ANT_SEL_C_M;
		cr->path0_r_rfe_buf_en = PATH0_R_RFE_BUF_EN_C;
		cr->path0_r_rfe_buf_en_m = PATH0_R_RFE_BUF_EN_C_M;
		cr->path0_r_lnaon_agc = PATH0_R_LNAON_AGC_C;
		cr->path0_r_lnaon_agc_m = PATH0_R_LNAON_AGC_C_M;
		cr->path0_r_trsw_bit_bt = PATH0_R_TRSW_BIT_BT_C;
		cr->path0_r_trsw_bit_bt_m = PATH0_R_TRSW_BIT_BT_C_M;
		cr->path0_r_trsw_s = PATH0_R_TRSW_S_C;
		cr->path0_r_trsw_s_m = PATH0_R_TRSW_S_C_M;
		cr->path0_r_trsw_o = PATH0_R_TRSW_O_C;
		cr->path0_r_trsw_o_m = PATH0_R_TRSW_O_C_M;
		cr->path0_r_trswb_o = PATH0_R_TRSWB_O_C;
		cr->path0_r_trswb_o_m = PATH0_R_TRSWB_O_C_M;
		cr->path0_r_bt_force_antidx = PATH0_R_BT_FORCE_ANTIDX_C;
		cr->path0_r_bt_force_antidx_m = PATH0_R_BT_FORCE_ANTIDX_C_M;
		cr->path0_r_bt_force_antidx_en = PATH0_R_BT_FORCE_ANTIDX_EN_C;
		cr->path0_r_bt_force_antidx_en_m = PATH0_R_BT_FORCE_ANTIDX_EN_C_M;
		cr->path0_r_ant_module_rfe_opt = PATH0_R_ANT_MODULE_RFE_OPT_C;
		cr->path0_r_ant_module_rfe_opt_m = PATH0_R_ANT_MODULE_RFE_OPT_C_M;
		cr->path0_r_rfsw_tr = PATH0_R_RFSW_TR_C;
		cr->path0_r_rfsw_tr_m = PATH0_R_RFSW_TR_C_M;
		cr->path0_r_antsel = PATH0_R_ANTSEL_C;
		cr->path0_r_antsel_m = PATH0_R_ANTSEL_C_M;
		cr->path0_r_rfsw_ant_31_0 = PATH0_R_RFSW_ANT_31_0__C;
		cr->path0_r_rfsw_ant_31_0_m = PATH0_R_RFSW_ANT_31_0__C_M;
		cr->path0_r_rfsw_ant_63_32 = PATH0_R_RFSW_ANT_63_32__C;
		cr->path0_r_rfsw_ant_63_32_m = PATH0_R_RFSW_ANT_63_32__C_M;
		cr->path0_r_rfsw_ant_95_64 = PATH0_R_RFSW_ANT_95_64__C;
		cr->path0_r_rfsw_ant_95_64_m = PATH0_R_RFSW_ANT_95_64__C_M;
		cr->path0_r_rfsw_ant_127_96 = PATH0_R_RFSW_ANT_127_96__C;
		cr->path0_r_rfsw_ant_127_96_m = PATH0_R_RFSW_ANT_127_96__C_M;
		break;
	#endif
	#ifdef HALBB_COMPILE_AP2_SERIES
	case BB_AP2:
		cr->path0_r_ant_train_en = PATH0_R_ANT_TRAIN_EN_A2;
		cr->path0_r_ant_train_en_m = PATH0_R_ANT_TRAIN_EN_A2_M;
		cr->path0_r_tx_ant_sel = PATH0_R_TX_ANT_SEL_A2;
		cr->path0_r_tx_ant_sel_m = PATH0_R_TX_ANT_SEL_A2_M;
		cr->path0_r_rfe_buf_en = PATH0_R_RFE_BUF_EN_A2;
		cr->path0_r_rfe_buf_en_m = PATH0_R_RFE_BUF_EN_A2_M;
		cr->path0_r_lnaon_agc = PATH0_R_LNAON_AGC_A2;
		cr->path0_r_lnaon_agc_m = PATH0_R_LNAON_AGC_A2_M;
		cr->path0_r_trsw_bit_bt = PATH0_R_TRSW_BIT_BT_A2;
		cr->path0_r_trsw_bit_bt_m = PATH0_R_TRSW_BIT_BT_A2_M;
		cr->path0_r_trsw_s = PATH0_R_TRSW_S_A2;
		cr->path0_r_trsw_s_m = PATH0_R_TRSW_S_A2_M;
		cr->path0_r_trsw_o = PATH0_R_TRSW_O_A2;
		cr->path0_r_trsw_o_m = PATH0_R_TRSW_O_A2_M;
		cr->path0_r_trswb_o = PATH0_R_TRSWB_O_A2;
		cr->path0_r_trswb_o_m = PATH0_R_TRSWB_O_A2_M;
		cr->path0_r_bt_force_antidx = PATH0_R_BT_FORCE_ANTIDX_A2;
		cr->path0_r_bt_force_antidx_m = PATH0_R_BT_FORCE_ANTIDX_A2_M;
		cr->path0_r_bt_force_antidx_en = PATH0_R_BT_FORCE_ANTIDX_EN_A2;
		cr->path0_r_bt_force_antidx_en_m = PATH0_R_BT_FORCE_ANTIDX_EN_A2_M;
		cr->path0_r_ant_module_rfe_opt = PATH0_R_ANT_MODULE_RFE_OPT_A2;
		cr->path0_r_ant_module_rfe_opt_m = PATH0_R_ANT_MODULE_RFE_OPT_A2_M;
		cr->path0_r_rfsw_tr = PATH0_R_RFSW_TR_A2;
		cr->path0_r_rfsw_tr_m = PATH0_R_RFSW_TR_A2_M;
		cr->path0_r_antsel = PATH0_R_ANTSEL_A2;
		cr->path0_r_antsel_m = PATH0_R_ANTSEL_A2_M;
		cr->path0_r_rfsw_ant_31_0 = PATH0_R_RFSW_ANT_31_0__A2;
		cr->path0_r_rfsw_ant_31_0_m = PATH0_R_RFSW_ANT_31_0__A2_M;
		cr->path0_r_rfsw_ant_63_32 = PATH0_R_RFSW_ANT_63_32__A2;
		cr->path0_r_rfsw_ant_63_32_m = PATH0_R_RFSW_ANT_63_32__A2_M;
		cr->path0_r_rfsw_ant_95_64 = PATH0_R_RFSW_ANT_95_64__A2;
		cr->path0_r_rfsw_ant_95_64_m = PATH0_R_RFSW_ANT_95_64__A2_M;
		cr->path0_r_rfsw_ant_127_96 = PATH0_R_RFSW_ANT_127_96__A2;
		cr->path0_r_rfsw_ant_127_96_m = PATH0_R_RFSW_ANT_127_96__A2_M;
		cr->sigval_rpt_en = SIGVAL_RPT_EN_A2;
		cr->sigval_rpt_en_m = SIGVAL_RPT_EN_A2_M;
		break;

	#endif

	default:
		break;
	}

}


#endif
