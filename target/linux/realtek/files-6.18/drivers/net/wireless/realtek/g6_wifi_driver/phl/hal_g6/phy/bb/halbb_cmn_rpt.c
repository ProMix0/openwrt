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

#if 1


void halbb_print_hist_2_buf_u8(struct bb_info *bb, u8 *val, u16 len, char *buf,
			    u16 buf_size)
{
	if (len == PHY_HIST_SIZE) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10], val[11]);
	} else if (len == (PHY_HIST_SIZE - 1)) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10]);
	}
}

void halbb_print_hist_2_buf_s8(struct bb_info *bb, s8 *val, u16 len, char *buf,
			    u16 buf_size)
{
	if (len == PHY_HIST_SIZE) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10], val[11]);
	} else if (len == (PHY_HIST_SIZE - 1)) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10]);
	}
}

void halbb_print_hist_2_buf(struct bb_info *bb, u16 *val, u16 len, char *buf,
			    u16 buf_size)
{
	if (len == PHY_HIST_SIZE) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10], val[11]);
	} else if (len == (PHY_HIST_SIZE - 1)) {
		halbb_snprintf(buf, buf_size,
			       "[%.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d, %.2d]",
			       val[0], val[1], val[2], val[3], val[4],
			       val[5], val[6], val[7], val[8], val[9],
			       val[10]);
	} else if (len == BB_PDP_HIST_SIZE) {
		halbb_snprintf(buf, buf_size,
			       "[%3d, %3d, %3d, %3d, %3d, %3d]",
			       val[0], val[1], val[2], val[3], val[4], val[5]);
	}
}

void halbb_rx_pop_hist(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	u8 pop_idx = 0;
	bool is_cck = cmn_rpt->is_cck_rate;

	if (is_cck)
		pop_idx = bb->bb_physts_i.bb_physts_rslt_0_i.pop_idx_cck;
	else
		pop_idx = bb->bb_physts_i.bb_physts_rslt_1_i.pop_idx;

	if (pop_idx >= POP_HIST_SIZE)
		pop_idx = POP_HIST_SIZE - 1;

	if (is_cck)
		cmn_rpt->bb_physts_pop_i.pop_hist_cck[pop_idx]++;
	else
		cmn_rpt->bb_physts_pop_i.pop_hist_ofdm[pop_idx]++;
}

void halbb_rx_pkt_cnt_rpt_beacon(struct bb_info *bb, struct physts_rxd *desc)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;

	if (desc->user_i[0].is_bcn) {
		cmn_rpt->bb_pkt_cnt_bcn_i.pkt_cnt_beacon++;
		cmn_rpt->bb_pkt_cnt_bcn_i.beacon_phy_rate= desc->data_rate;
	}
}

void halbb_rx_pkt_cnt_rpt_reset(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;

	//halbb_mem_set(bb, cmn_rpt, 0, sizeof(struct bb_cmn_rpt_info));
	cmn_rpt->bb_pkt_cnt_bcn_i.pkt_cnt_beacon = 0;
	halbb_mem_set(bb, &cmn_rpt->bb_pkt_cnt_all_i, 0, sizeof(struct bb_pkt_cnt_cap_info));
	halbb_mem_set(bb, &cmn_rpt->bb_pkt_cnt_su_i, 0, sizeof(struct bb_pkt_cnt_su_info));
	halbb_mem_set(bb, &cmn_rpt->bb_rssi_su_acc_i, 0, sizeof(struct bb_rssi_su_acc_info));
	halbb_mem_set(bb, &cmn_rpt->bb_physts_hist_i, 0, sizeof(struct bb_physts_hist_info));
	halbb_mem_set(bb, &cmn_rpt->bb_physts_acc_i, 0, sizeof(struct bb_physts_acc_info));

	/*[POP cnt reset]*/
	halbb_mem_set(bb, &cmn_rpt->bb_physts_pop_i, 0, sizeof(struct bb_physts_pop_info));
	
	/*[MU]*/
	halbb_mem_set(bb, &cmn_rpt->bb_pkt_cnt_mu_i, 0, sizeof(struct bb_pkt_cnt_mu_info));
	halbb_mem_set(bb, &cmn_rpt->bb_rssi_mu_acc_i, 0, sizeof(struct bb_rssi_mu_acc_info));
	halbb_mem_set(bb, &cmn_rpt->n_user_max, 0, 1);
}

#define CMN_RPT_MU
#ifdef CMN_RPT_MU

u16 halbb_get_plurality_rx_rate_mu(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	u16 max_num_tmp = 0;
	u16 rx_rate_plurality = 0;
	u16 i = 0;
	u16 *pkt_cnt_tmp;
	u8 rate_num_tmp;
	u16 ofst_mode = 0;
	u16 ofst_ss = 0;
	u16 idx = 0;
	bool plurality_is_legacy_rate = true;

	if (pkt_cnt->pkt_cnt_all == 0) {
		return rx_rate_plurality;
	}

	//BB_DBG(bb, DBG_CMN, "cnt_t= (%d)\n", pkt_cnt->pkt_cnt_all);

	/*HT, VHT, HE*/
	if (pkt_cnt->he_pkt_not_zero) {
		pkt_cnt_tmp = pkt_cnt->pkt_cnt_he;
		rate_num_tmp = HE_RATE_NUM;
		ofst_mode = BB_HE_1SS_MCS0;
	} else if (pkt_cnt->vht_pkt_not_zero) {
		pkt_cnt_tmp = pkt_cnt->pkt_cnt_vht;
		rate_num_tmp = VHT_RATE_NUM;
		ofst_mode = BB_VHT_1SS_MCS0;
	} else {
		return rx_rate_plurality;
	}

	for (i = 0; i < rate_num_tmp; i++) {
		if (pkt_cnt_tmp[i] >= max_num_tmp) {
			max_num_tmp = pkt_cnt_tmp[i];
			idx = i;
			plurality_is_legacy_rate = false;
		}
	}

	if (plurality_is_legacy_rate)
		return rx_rate_plurality;

	//BB_DBG(bb, DBG_CMN, "[T]idx_ori= (%d)\n", idx);

	ofst_ss = idx / HE_VHT_NUM_MCS;

	if (ofst_ss >= 0) /*>=2SS*/
		idx -= (ofst_ss * HE_VHT_NUM_MCS);

	//BB_DBG(bb, DBG_CMN, "ofst_ss= (%d), idx=%d\n", ofst_ss, idx);

	rx_rate_plurality = ofst_mode + (ofst_ss << 4) + idx;
	//BB_DBG(bb, DBG_CMN, "[T]rx_rate_plurality= (0x%x), max_num_tmp=%d\n", rx_rate_plurality, max_num_tmp);

	return rx_rate_plurality;
}

void halbb_mu_rate_idx_generate(struct bb_info *bb, struct physts_rxd *desc, struct bb_rate_info *ra_i)
{
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_13_info *psts_13 = &physts->bb_physts_rslt_13_i;
	enum bb_physts_bitmap_t type = physts->bb_physts_rslt_hdr_i.ie_map_type;
	u8 user_idx;

	if (psts_13->n_user != desc->user_num) {
		BB_WARNING("[%s][1] n_user=%d, user_num=%d\n", __func__,
			   psts_13->n_user, desc->user_num);
		return;
	}

	if (desc->user_num > MU_USER_MAX || desc->user_num == 0) {
		BB_WARNING("[%s][2] user_num = %d\n", __func__, desc->user_num);
		return;
	}

	user_idx = desc->user_num - 1;

	ra_i->gi_ltf = desc->gi_ltf;
	ra_i->bw = physts->bb_physts_rslt_1_i.bw_idx;
	ra_i->ss = psts_13->bb_physts_uer_info[user_idx].n_sts + 1;
	ra_i->idx = psts_13->bb_physts_uer_info[user_idx].mcs;

	if (type == VHT_MU || (type == VHT_PKT && physts->frc_mu == 2)) {
		ra_i->mode = BB_VHT_MODE;
		ra_i->rate_idx = GEN_VHT_RATE_IDX(ra_i->ss, ra_i->idx);
	} else if (type == HE_MU || type == TRIG_BASE_PPDU) {
		ra_i->mode = BB_HE_MODE;
		ra_i->rate_idx = GEN_HE_RATE_IDX(ra_i->ss, ra_i->idx);
	} else {
		BB_WARNING("[%s] ie_map_type = %d\n", __func__, type);
		return;
	}

	ra_i->rate_idx_all = ra_i->rate_idx | (((u16)desc->gi_ltf & 0xf) << 12);
}

void halbb_show_rssi_and_rate_distribution_mu(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	struct bb_rssi_mu_acc_info *acc = &cmn_rpt->bb_rssi_mu_acc_i;
	struct bb_rssi_mu_avg_info *avg = &cmn_rpt->bb_rssi_mu_avg_i;
	u8 rssi_avg_tmp = 0;
	u8 rssi_tmp[HALBB_MAX_PATH];
	u16 pkt_cnt_ss = 0;
	u8 i = 0, j =0;
	u8 rate_num = bb->num_rf_path, ss_ofst = 0;

	if (bb->bb_link_i.wlan_mode_bitmap & WLAN_MD_11AX) {
		BB_DBG(bb, DBG_CMN, "[GI_LTF] {4x32/4x08/2x16/2x08/1x16/1x08}={%d, %d, %d, %d, %d, %d}\n",
		       pkt_cnt->gi_ltf_cnt[0], pkt_cnt->gi_ltf_cnt[1],
		       pkt_cnt->gi_ltf_cnt[2], pkt_cnt->gi_ltf_cnt[3],
		       pkt_cnt->gi_ltf_cnt[4], pkt_cnt->gi_ltf_cnt[5]);
	} else {
		BB_DBG(bb, DBG_CMN, "[GI] {LGI:%d, SGI:%d}\n",
		       pkt_cnt->gi_ltf_cnt[0], pkt_cnt->gi_ltf_cnt[1]);
	}

	avg->rssi_t_avg = (u8)HALBB_DIV(acc->rssi_t_avg_acc, pkt_cnt->pkt_cnt_all);
		
	for (i = 0; i < HALBB_MAX_PATH; i++) {
		if (i >= bb->num_rf_path)
			break;

		avg->rssi_t[i] = (u8)HALBB_DIV(acc->rssi_t_acc[i], pkt_cnt->pkt_cnt_all);
	}

	/*@======VHT==========================================================*/
	if (pkt_cnt->vht_pkt_not_zero) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			for (j = 0; j < HE_VHT_NUM_MCS ; j++) {
				pkt_cnt_ss += pkt_cnt->pkt_cnt_vht[ss_ofst + j];
			}

			if (pkt_cnt_ss == 0) {
				rssi_avg_tmp = 0;
				rssi_tmp[0] = 0;
				rssi_tmp[1] = 0;
			} else {
				rssi_avg_tmp = avg->rssi_t_avg >> 1;
				rssi_tmp[0] = avg->rssi_t[0] >> 1;
				rssi_tmp[1] = avg->rssi_t[1] >> 1;
			}

			BB_DBG(bb, DBG_CMN,
				  "*[MU] VHT %d-S RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  rssi_avg_tmp, rssi_tmp[0], rssi_tmp[1],
			          pkt_cnt_ss,
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 11]);

			pkt_cnt_ss = 0;
		}

	}

	/*@======HE==========================================================*/
	if (pkt_cnt->he_pkt_not_zero) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			for (j = 0; j < HE_VHT_NUM_MCS ; j++) {
				pkt_cnt_ss += pkt_cnt->pkt_cnt_he[ss_ofst + j];
			}

			if (pkt_cnt_ss == 0) {
				rssi_avg_tmp = 0;
				rssi_tmp[0] = 0;
				rssi_tmp[1] = 0;
			} else {
				rssi_avg_tmp = avg->rssi_t_avg >> 1;
				rssi_tmp[0] = avg->rssi_t[0] >> 1;
				rssi_tmp[1] = avg->rssi_t[1] >> 1;
			}

			BB_DBG(bb, DBG_CMN,
				  "*[MU] HE %d-SS RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  rssi_avg_tmp, rssi_tmp[0], rssi_tmp[1],
			          pkt_cnt_ss,
				  pkt_cnt->pkt_cnt_he[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 11]);

			pkt_cnt_ss = 0;
		}

	}

	/*@======SC_BW========================================================*/
	
	if (pkt_cnt->sc20_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[MU][Low BW 20M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 11]);
		}
	}

	if (pkt_cnt->sc40_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[MU][Low BW 40M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 11]);
		}
	}

	if (pkt_cnt->sc80_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[MU][Low BW 80M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 11]);
		}
	}
}

void halbb_rx_pkt_mu_cnt_rpt(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	enum channel_width bw_curr;
	u8 band_idx;
	u8 ofst = rate_i->idx;

	if (rate_i->mode <= BB_HT_MODE) {
		BB_WARNING("[%s] mode = %d\n", __func__, rate_i->mode);
		return;
	}
	pkt_cnt->gi_ltf_cnt[desc->gi_ltf]++;

	pkt_cnt->pkt_cnt_all++;
	if (rate_i->ss == 1)
		pkt_cnt->pkt_cnt_1ss++;
	else if (rate_i->ss == 2)
		pkt_cnt->pkt_cnt_2ss++;

	band_idx = (desc->phy_idx == HW_PHY_0) ? 0 : 1;
	bw_curr = bb->hal_com->band[band_idx].cur_chandef.bw;

	rx_bw = bw_curr; /*Will be removed, just for tmp debug using*/

	if (rate_i->ss >= 2)
		ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

	if (rate_i->mode == BB_VHT_MODE) {
		pkt_cnt->vht_pkt_not_zero = true;
		ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);

		if (rx_bw == bw_curr) {
			pkt_cnt->pkt_cnt_vht[ofst]++;
			return;
		}
	} else if (rate_i->mode == BB_HE_MODE) {
		pkt_cnt->he_pkt_not_zero = true;
		ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);

		if (rx_bw == bw_curr) {
			pkt_cnt->pkt_cnt_he[ofst]++;
			return;
		}
	}

	/*SC BW*/
	if (rx_bw != bw_curr) {
		ofst = NOT_GREATER(ofst, LOW_BW_RATE_NUM - 1);

		if (rx_bw == CHANNEL_WIDTH_20) {
			pkt_cnt->pkt_cnt_sc20[ofst]++;
			pkt_cnt->sc20_occur = true;
		} else {
			pkt_cnt->pkt_cnt_sc40[ofst]++;
			pkt_cnt->sc40_occur = true;
		}
	}
}

void halbb_rx_pkt_mu_rssi_statistic(struct bb_info *bb)
{
	struct bb_physts_rslt_hdr_info	*psts_h = &bb->bb_physts_i.bb_physts_rslt_hdr_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rssi_mu_acc_info *rssi_mu_acc = &cmn_rpt->bb_rssi_mu_acc_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u32 *rssi_acc;
	u8 i = 0;

	if (rate_i->mode <= BB_HT_MODE) {
		BB_WARNING("[%s] mode = %d\n", __func__, rate_i->mode);
		return;
	}

	rssi_mu_acc->rssi_t_avg_acc += psts_h->rssi_avg;
	rssi_acc = &rssi_mu_acc->rssi_t_acc[0];
	//BB_DBG(bb, DBG_PHY_STS, "Xt = %d\n", rssi_su_acc->rssi_t_avg_acc);
	
	/*HT/VHT/HE*/
	for (i = 0; i < HALBB_MAX_PATH; i++) {
		if (i >= bb->num_rf_path)
			break;

		rssi_acc[i] += (u32)psts_h->rssi[i];
	}
	//BB_DBG(bb, DBG_PHY_STS, "rssi_acc = %d, %d\n", rssi_acc[0], rssi_acc[1]);
}

#endif

#define CMN_RPT_SU
#ifdef CMN_RPT_SU

void halbb_show_pdp_tmax_hist_su(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	u8 sts = 0, path = 0;
	u16 tmax[6] = {0, 80, 200, 390, 730, 1050};

	if ((pkt_cnt->pkt_cnt_t + pkt_cnt->pkt_cnt_ofdm) > 0) {
		halbb_print_hist_2_buf(bb, tmax, BB_PDP_HIST_SIZE,
				       	  bb->dbg_buf, HALBB_SNPRINT_SIZE);
          	/*mode : the number or value that appears most often in a particular set*/
		BB_DBG(bb, DBG_CMN, "  %-15s %-6s %s\n", "[tmax value]", "(Mode)", bb->dbg_buf);
	}

	for (sts = 0; sts < bb->num_rf_path; sts ++) {
		for (path = 0; path < bb->num_rf_path; path ++) {
			halbb_print_hist_2_buf(bb, hist->tmax_idx_sts_path_hist[sts][path], BB_PDP_HIST_SIZE, bb->dbg_buf,
						HALBB_SNPRINT_SIZE);
			BB_DBG(bb, DBG_CMN, "[tmax_sts%d_path%c] (%4d) %s\n", sts,
				'A' + path, tmax[avg->tmax_sts_path_mode[sts][path]], bb->dbg_buf);
		}
	}

}

void halbb_show_snr_sts_path_hist_su(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	u8 sts = 0, path = 0;

	halbb_print_hist_2_buf_u8(bb, hist_th->evm_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "  %-14s %-6s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
	for (sts = 0; sts < bb->num_rf_path; sts ++) {
		for (path = 0; path < bb->num_rf_path; path ++) {
			halbb_print_hist_2_buf(bb, hist->snr_sts_path_hist[sts][path], BB_HIST_SIZE, bb->dbg_buf,
						HALBB_SNPRINT_SIZE);
			BB_DBG(bb, DBG_CMN, "[snr_sts%d_path%c] (%3d) %s\n", sts,
				'A' + path, avg->snr_sts_path_avg[sts][path], bb->dbg_buf);
		}
	}


}

void halbb_basic_dbg_07_hist_su_per_path(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	u16 valid_cnt = pkt_cnt->pkt_cnt_t + pkt_cnt->pkt_cnt_ofdm;
	u8 i = 0;
	bool print_en = false;
	u8 snr_tmp[4] = {0};

	//BB_DBG(bb, BIT14, "[%s]\n", __func__);

	for (i = 0; i < bb->num_rf_path; i++) {
		//BB_DBG(bb, BIT14, "snr_per_path_acc=%d\n", acc->snr_per_path_acc[i]);

		if (acc->snr_per_path_acc[i] == 0)
			continue;

		avg->snr_per_path_avg[i] = (u8)HALBB_DIV(acc->snr_per_path_acc[i], valid_cnt);
		print_en = true;
		snr_tmp[i] = avg->snr_per_path_avg[i];
	}

	if (print_en) {
		BB_DBG(bb, DBG_CMN, "[SNR path[A:D] = {%d, %d, %d, %d}\n", 
		       snr_tmp[0], snr_tmp[1], snr_tmp[2], snr_tmp[3]);
	}
}

void halbb_basic_dbg_07_hist_su(struct bb_info *bb)
{
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_ch_stb_info *ch_stb = &physts->bb_physts_ch_stb_i;
	struct bb_physts_statics_info *ratio = &ch_stb->channel_change_ratio;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	char buf[HALBB_SNPRINT_SIZE] = {0};
	u16 valid_cnt = pkt_cnt->pkt_cnt_t + pkt_cnt->pkt_cnt_ofdm;
	u8 path = 0;

	if (!(bb->cmn_dbg_msg_component & BB_BASIC_DBG_07_HIST))
		return;
	BB_DBG(bb, DBG_CMN, "====[7. BB Hist]\n");

	halbb_basic_dbg_07_hist_su_per_path(bb);
	/*=== [EVM, SNR] =====================================================*/
	/*Threshold*/

	//BB_DBG(bb, DBG_CMN, "[RESULT ACC] cfo_avg=%d, evm_max=%d, evm_min=%d, cn_avg=%d\n",
	//	acc->cfo_avg_acc, acc->evm_max_acc, acc->evm_min_acc, acc->cn_avg_acc);

	//BB_DBG(bb, DBG_CMN, "valid_cnt=%d\n", valid_cnt);

	halbb_print_hist_2_buf_u8(bb, hist_th->evm_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "  %-8s %-9s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
	/*val*/
	//avg->evm_1ss = (u8)HALBB_DIV(acc->evm_1ss, (pkt_cnt->pkt_cnt_1ss + pkt_cnt->pkt_cnt_ofdm));
	halbb_print_hist_2_buf(bb, hist->evm_1ss, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_1ss]",
	       (avg->evm_1ss >> 2),
	       halbb_show_fraction_num(avg->evm_1ss & 0x3, 2), bb->dbg_buf);

	//avg->evm_max = (u8)HALBB_DIV(acc->evm_max_acc, pkt_cnt->pkt_cnt_2ss);
	halbb_print_hist_2_buf(bb, hist->evm_max_hist, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_max]",
	       (avg->evm_max >> 2),
	       halbb_show_fraction_num(avg->evm_max & 0x3, 2), bb->dbg_buf);

	//avg->evm_min = (u8)HALBB_DIV(acc->evm_min_acc, pkt_cnt->pkt_cnt_2ss);
	halbb_print_hist_2_buf(bb, hist->evm_min_hist, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_min]",
	       (avg->evm_min >> 2),
	       halbb_show_fraction_num(avg->evm_min & 0x3, 2), bb->dbg_buf);


	//avg->snr_avg = (u8)HALBB_DIV(acc->snr_avg_acc, valid_cnt);
	halbb_print_hist_2_buf(bb, hist->snr_avg_hist, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%02d.000)  %s\n", "[SNR_avg]",
	       avg->snr_avg, bb->dbg_buf);

	/*=== [CN] ===========================================================*/
	/*Threshold*/
	halbb_print_hist_2_buf_u8(bb, hist_th->cn_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "  %-8s %-9s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
	/*val*/
	//avg->cn_avg = (u8)HALBB_DIV(acc->cn_avg_acc, pkt_cnt->pkt_cnt_2ss);
	halbb_print_hist_2_buf(bb, hist->cn_avg_hist, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[CN_avg]",
	       (avg->cn_avg >> 1),
	       halbb_show_fraction_num(avg->cn_avg & 0x1, 1), bb->dbg_buf);

	/*=== [CFO] ==========================================================*/
	/*Threshold*/
	halbb_print_hist_2_buf_u8(bb, hist_th->cfo_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "  %-8s %-9s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
	/*val*/
	//avg->cfo_avg = (s16)HALBB_DIV(acc->cfo_avg_acc, valid_cnt);

	halbb_print_sign_frac_digit(bb, avg->cfo_avg, 16, 2, buf, HALBB_SNPRINT_SIZE);
	halbb_print_hist_2_buf(bb, hist->cfo_avg_hist, BB_HIST_SIZE, bb->dbg_buf,
			       HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "%-9s (%s K) %s\n", "[CFO_avg]",
	       buf, bb->dbg_buf);
	BB_DBG(bb, DBG_CMN, "CFO_src: %s\n",
	       (bb->bb_cfo_trk_i.cfo_src == CFO_SRC_FD) ? "FD" : "Preamble");

	/*=== [PDP] ==========================================================*/
	halbb_show_pdp_tmax_hist_su(bb);
	halbb_show_snr_sts_path_hist_su(bb);

	/*=== [sig_val] ==========================================================*/
	/*Threshold*/
	halbb_print_hist_2_buf_s8(bb, hist_th->sig_val_path_hist_th, BB_HIST_TH_SIZE,
				  bb->dbg_buf, HALBB_SNPRINT_SIZE);
	BB_DBG(bb, DBG_CMN, "    %-14s %-6s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
	/*val*/
	//avg->cfo_avg = (s16)HALBB_DIV(acc->cfo_avg_acc, valid_cnt);
	for (path = 0; path < bb->num_rf_path; path++) {
		halbb_print_sign_frac_digit(bb, avg->sig_val_path_avg[path], 8, 2,
					    buf, HALBB_SNPRINT_SIZE);
		halbb_print_hist_2_buf(bb, hist->sig_val_path_hist[path], BB_HIST_SIZE,
				       bb->dbg_buf, HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "[sig_val_path%c]   (%5s) %s\n",
		       'A' + path, buf, bb->dbg_buf);
	}
	BB_DBG(bb, DBG_CMN, "valid_cnt = %d\n", valid_cnt);
	BB_DBG(bb, DBG_CMN, "\n");

	BB_DBG(bb, DBG_CMN, "[TEST] channel_change_idx {now, prev, diff} = {%d,%d,%d}\n",
	       ch_stb->channel_change_idx, ch_stb->channel_change_idx_cmn_rpt,
	       ch_stb->channel_change_idx - ch_stb->channel_change_idx_cmn_rpt);
	BB_DBG(bb, DBG_CMN, "[TEST] change_cnt {rssi, evm, snr, cn, channel} = {%d,%d,%d,%d,%d}\n",
	       ch_stb->rssi_change_cnt, ch_stb->evm_change_cnt,
	       ch_stb->snr_change_cnt, ch_stb->cn_change_cnt,
	       ch_stb->channel_change_cnt);
	BB_DBG(bb, DBG_CMN, "[TEST] channel_change_ratio = (%d/1000), VAR = (%d/1000)\n",
	       ch_stb->channel_change_ratio.val, ch_stb->channel_change_ratio.var);
	BB_DBG(bb, DBG_CMN, "[%s] mean = %d, var = %d, num_sample = %d, round_up = %d, idx = %d\n",
	       __func__, ratio->mean, ratio->var, ratio->num_sample, ratio->round_up, ratio->idx);
}

void halbb_cmn_rpt_get_avg_val(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt_mu = &cmn_rpt->bb_pkt_cnt_mu_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	u16 valid_cnt = pkt_cnt->pkt_cnt_t + pkt_cnt->pkt_cnt_ofdm;
	u8 i = 0, sts = 0, path = 0;
	u16 pkt_cnt_temp = pkt_cnt->pkt_cnt_t;
	u8 mode = 0;
	u16 tmax[6] = {0, 80, 200, 390, 730, 1050};

	avg->evm_1ss = (u8)HALBB_DIV(acc->evm_1ss, (pkt_cnt->pkt_cnt_1ss + pkt_cnt->pkt_cnt_ofdm));
	avg->evm_max = (u8)HALBB_DIV(acc->evm_max_acc, pkt_cnt->pkt_cnt_2ss);
	avg->evm_min = (u8)HALBB_DIV(acc->evm_min_acc, pkt_cnt->pkt_cnt_2ss);
	avg->snr_avg = (u8)HALBB_DIV(acc->snr_avg_acc, valid_cnt);
	avg->cn_avg = (u8)HALBB_DIV(acc->cn_avg_acc, acc->pkt_cnt_cn_valid);
	avg->cfo_avg = (s16)HALBB_DIV(acc->cfo_avg_acc, valid_cnt);

	for (i = 0; i < bb->num_rf_path; i++) {
		avg->sig_val_path_avg[i] = (s8)HALBB_DIV(acc->sig_val_path_acc[i], pkt_cnt->pkt_cnt_2ss);

		if (acc->snr_per_path_acc[i] == 0)
			continue;

		avg->snr_per_path_avg[i] = (u8)HALBB_DIV(acc->snr_per_path_acc[i], valid_cnt);
	}
	pkt_cnt_temp = pkt_cnt->pkt_cnt_t;
	for (sts = 0; sts < bb->num_rf_path; sts++) {
		for (path = 0; path < bb->num_rf_path; path++) {
			avg->tmax_sts_path_mode[sts][path] = halbb_find_mode(bb, hist->tmax_idx_sts_path_hist[sts][path], BB_PDP_HIST_SIZE);
			avg->snr_sts_path_avg[sts][path] = (s8)HALBB_DIV(acc->snr_sts_path_acc[sts][path], pkt_cnt_temp);
		}
	}

	//BB_DBG(bb, DBG_CMN, "evm_1ss=%d, evm_max=%d, evm_min=%d, snr_avg=%d, cn_avg=%d, cfo_avg=%d, snr={%d, %d}\n",
	//       avg->evm_1ss, avg->evm_max, avg->evm_min, avg->snr_avg, avg->cn_avg, avg->cfo_avg,
	//       avg->snr_per_path_avg[0], avg->snr_per_path_avg[1]);

	/*MU*/
	for (i = 0; i < cmn_rpt->n_user_max; i++) {
		if (i >= MU_USER_MAX)
			break;
		avg->evm_1ss_mu_avg[i] = (u8)HALBB_DIV(acc->evm_1ss_mu_acc[i], pkt_cnt_mu->pkt_cnt_1ss);
		avg->evm_max_mu_avg[i] = (u8)HALBB_DIV(acc->evm_max_mu_acc[i], pkt_cnt_mu->pkt_cnt_2ss);
		avg->evm_min_mu_avg[i] = (u8)HALBB_DIV(acc->evm_min_mu_acc[i], pkt_cnt_mu->pkt_cnt_2ss);
		avg->cn_mu_avg[i] = (u8)HALBB_DIV(acc->cn_mu_acc[i], acc->pkt_cnt_cn_mu_valid);
		if (acc->snr_mu_acc[i] != 0)
			avg->snr_mu_avg[i] = (u8)HALBB_DIV(acc->snr_mu_acc[i], pkt_cnt_mu->pkt_cnt_all);
	}

	//BB_DBG(bb, DBG_CMN, "evm_1ss=%d, evm_max=%d, evm_min=%d, snr_avg=%d, cn_avg=%d, cfo_avg=%d, snr={%d, %d}\n",
	//       avg->evm_1ss, avg->evm_max, avg->evm_min, avg->snr_avg, avg->cn_avg, avg->cfo_avg,
	//       avg->snr_per_path_avg[0], avg->snr_per_path_avg[1]);

	/*Normalize channel_change_idx to a ratio*/
	halbb_physts_ch_stability_ratio_statistic(bb, valid_cnt);
}

void halbb_basic_dbg_08_hist_mu(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_mu_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_mu_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_avg_info *avg = &cmn_rpt->bb_physts_avg_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	char buf[HALBB_SNPRINT_SIZE] = {0};
	struct bb_antdiv_info *bb_ant_div = &bb->bb_ant_div_i;
	struct bb_antdiv_cn_info *bb_cn_i = &bb_ant_div->bb_cn_i;
	u16 valid_cnt = pkt_cnt->pkt_cnt_all;
	u8 path = 0, i = 0;

	//halbb_cmn_rpt_get_avg_val(bb);
	for (i = 0; i < cmn_rpt->n_user_max; i++) {
		if (i >= MU_USER_MAX)
			break;
		BB_DBG(bb, DBG_CMN, "[User idx : %d]", i);
		/*=== [EVM, SNR] =====================================================*/
		/*Threshold*/

		//BB_DBG(bb, DBG_CMN, "[RESULT ACC] cfo_avg=%d, evm_max=%d, evm_min=%d, cn_avg=%d\n",
		//	acc->cfo_avg_acc, acc->evm_max_acc, acc->evm_min_acc, acc->cn_avg_acc);

		//BB_DBG(bb, DBG_CMN, "valid_cnt=%d\n", valid_cnt);

		halbb_print_hist_2_buf_u8(bb, hist_th->evm_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
					  HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "  %-8s %-9s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
		/*val*/
		//avg->evm_1ss = (u8)HALBB_DIV(acc->evm_1ss, (pkt_cnt->pkt_cnt_ss[HALBB_1SS] + pkt_cnt->pkt_cnt_ofdm));
		halbb_print_hist_2_buf(bb, hist->evm_1ss_mu_hist[i], BB_HIST_SIZE, bb->dbg_buf,
				       HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_1ss]",
		       (avg->evm_1ss_mu_avg[i] >> 2),
		       halbb_show_fraction_num(avg->evm_1ss_mu_avg[i] & 0x3, 2), bb->dbg_buf);

		//avg->evm_max = (u8)HALBB_DIV(acc->evm_max_acc, pkt_cnt->pkt_cnt_ss[HALBB_2SS]);
		halbb_print_hist_2_buf(bb, hist->evm_max_mu_hist[i], BB_HIST_SIZE, bb->dbg_buf,
				       HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_max]",
		       (avg->evm_max_mu_avg[i] >> 2),
		       halbb_show_fraction_num(avg->evm_max_mu_avg[i] & 0x3, 2), bb->dbg_buf);

		//avg->evm_min = (u8)HALBB_DIV(acc->evm_min_acc, pkt_cnt->pkt_cnt_ss[HALBB_2SS]);
		halbb_print_hist_2_buf(bb, hist->evm_min_mu_hist[i], BB_HIST_SIZE, bb->dbg_buf,
				       HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[EVM_min]",
		       (avg->evm_min_mu_avg[i] >> 2),
		       halbb_show_fraction_num(avg->evm_min_mu_avg[i] & 0x3, 2), bb->dbg_buf);


		//avg->snr_avg = (u8)HALBB_DIV(acc->snr_avg_acc, valid_cnt);
		halbb_print_hist_2_buf(bb, hist->snr_mu_hist[i], BB_HIST_SIZE, bb->dbg_buf,
				       HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[SNR_avg]",
		       avg->snr_mu_avg[i] >> 1,
		       halbb_show_fraction_num(avg->snr_mu_avg[i] & 0x1, 1), bb->dbg_buf);

		/*=== [CN] ===========================================================*/
		/*Threshold*/
		halbb_print_hist_2_buf_u8(bb, hist_th->cn_hist_th, BB_HIST_TH_SIZE, bb->dbg_buf,
					  HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "  %-8s %-9s  %s\n", "[TH]", "(Avg)", bb->dbg_buf);
		/*val*/
		//avg->cn_avg = (u8)HALBB_DIV(acc->cn_avg_acc, acc->pkt_cnt_cn_valid);
		halbb_print_hist_2_buf(bb, hist->cn_mu_hist[i], BB_HIST_SIZE, bb->dbg_buf,
				       HALBB_SNPRINT_SIZE);
		BB_DBG(bb, DBG_CMN, "%-9s (%02d.%03d)  %s\n", "[CN_avg]",
		       (avg->cn_mu_avg[i] >> 1),
		       halbb_show_fraction_num(avg->cn_mu_avg[i] & 0x1, 1), bb->dbg_buf);

		BB_DBG(bb, DBG_CMN, "valid_cnt = %d\n", valid_cnt);
		BB_DBG(bb, DBG_CMN, "\n");
		BB_TRACE( "valid_cnt = %d\n", valid_cnt);
		BB_TRACE( "Ant div valid_cnt = %d\n", bb_cn_i->main_cn_pkt_cnt+bb_cn_i->aux_cn_pkt_cnt );
	}
}

u16 halbb_get_plurality_rx_rate_su(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	u16 max_num_tmp = 0;
	u16 rx_rate_plurality = 0;
	u16 i = 0;
	u16 *pkt_cnt_tmp;
	u8 rate_num_tmp;
	u16 ofst_mode = 0;
	u16 ofst_ss = 0;
	u16 idx = 0;
	bool is_ht_mode = false;
	bool plurality_is_legacy_rate = true;
	
	/*Legacy rate*/
	if (pkt_cnt->pkt_cnt_cck || pkt_cnt->pkt_cnt_ofdm) {
		for (i = 0; i < LEGACY_RATE_NUM; i++) {
			if (pkt_cnt->pkt_cnt_legacy[i] >= max_num_tmp) {
				max_num_tmp = pkt_cnt->pkt_cnt_legacy[i];
				rx_rate_plurality = i;
			}
		}
	}
	//BB_DBG(bb, DBG_CMN, "[LEGACY]rx_rate_plurality= (0x%x), max_num_tmp=%d\n", rx_rate_plurality, max_num_tmp);

	if (pkt_cnt->pkt_cnt_t == 0) {
		return rx_rate_plurality;
	}

	//BB_DBG(bb, DBG_CMN, "cnt_t= (%d)\n", pkt_cnt->pkt_cnt_t);

	/*HT, VHT, HE*/
	if (pkt_cnt->he_pkt_not_zero) {
		pkt_cnt_tmp = pkt_cnt->pkt_cnt_he;
		rate_num_tmp = HE_RATE_NUM;
		ofst_mode = BB_HE_1SS_MCS0;
	} else if (pkt_cnt->vht_pkt_not_zero) {
		pkt_cnt_tmp = pkt_cnt->pkt_cnt_vht;
		rate_num_tmp = VHT_RATE_NUM;
		ofst_mode = BB_VHT_1SS_MCS0;
	} else if (pkt_cnt->ht_pkt_not_zero) {
		pkt_cnt_tmp = pkt_cnt->pkt_cnt_ht;
		rate_num_tmp = HT_RATE_NUM;
		ofst_mode = BB_HT_MCS0;
		is_ht_mode = true;
	} else {
		return rx_rate_plurality;
	}

	for (i = 0; i < rate_num_tmp; i++) {
		if (pkt_cnt_tmp[i] >= max_num_tmp) {
			max_num_tmp = pkt_cnt_tmp[i];
			idx = i;
			plurality_is_legacy_rate = false;
		}
	}

	if (plurality_is_legacy_rate)
		return rx_rate_plurality;

	//BB_DBG(bb, DBG_CMN, "[T]idx_ori= (%d)\n", idx);

	if (!is_ht_mode) {
		ofst_ss = idx / HE_VHT_NUM_MCS;

		if (ofst_ss >= 0) /*>=2SS*/
			idx -= (ofst_ss * HE_VHT_NUM_MCS);

		//BB_DBG(bb, DBG_CMN, "ofst_ss= (%d), idx=%d\n", ofst_ss, idx);
	}

	rx_rate_plurality = ofst_mode + (ofst_ss << 4) + idx;
	//BB_DBG(bb, DBG_CMN, "[T]rx_rate_plurality= (0x%x), max_num_tmp=%d\n", rx_rate_plurality, max_num_tmp);

	return rx_rate_plurality;
}

void halbb_show_rssi_and_rate_distribution_su(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_rssi_su_acc_info *acc = &cmn_rpt->bb_rssi_su_acc_i;
	struct bb_rssi_su_avg_info *avg = &cmn_rpt->bb_rssi_su_avg_i;
	u8 rssi_avg_tmp = 0;
	u8 rssi_tmp[HALBB_MAX_PATH];
	u16 pkt_cnt_ss = 0;
	u8 i = 0, j =0;
	u8 rate_num = bb->num_rf_path, ss_ofst = 0;

	if (bb->bb_link_i.wlan_mode_bitmap & WLAN_MD_11AX) {
		BB_DBG(bb, DBG_CMN, "[GI_LTF] {4x32/4x08/2x16/2x08/1x16/1x08}={%d, %d, %d, %d, %d, %d}\n",
		       pkt_cnt->gi_ltf_cnt[0], pkt_cnt->gi_ltf_cnt[1],
		       pkt_cnt->gi_ltf_cnt[2], pkt_cnt->gi_ltf_cnt[3],
		       pkt_cnt->gi_ltf_cnt[4], pkt_cnt->gi_ltf_cnt[5]);
	} else {
		BB_DBG(bb, DBG_CMN, "[GI] {LGI:%d, SGI:%d}\n",
		       pkt_cnt->gi_ltf_cnt[0], pkt_cnt->gi_ltf_cnt[1]);
	}

	avg->rssi_cck_avg = (u8)HALBB_DIV(acc->rssi_cck_avg_acc, pkt_cnt->pkt_cnt_cck);
	avg->rssi_ofdm_avg = (u8)HALBB_DIV(acc->rssi_ofdm_avg_acc, pkt_cnt->pkt_cnt_ofdm);
	avg->rssi_t_avg = (u8)HALBB_DIV(acc->rssi_t_avg_acc, pkt_cnt->pkt_cnt_t);
		
	for (i = 0; i < HALBB_MAX_PATH; i++) {
		if (i >= bb->num_rf_path)
			break;

		avg->rssi_cck[i] = (u8)HALBB_DIV(acc->rssi_cck_acc[i], pkt_cnt->pkt_cnt_cck);
		avg->rssi_ofdm[i] = (u8)HALBB_DIV(acc->rssi_ofdm_acc[i], pkt_cnt->pkt_cnt_ofdm);
		avg->rssi_t[i] = (u8)HALBB_DIV(acc->rssi_t_acc[i], pkt_cnt->pkt_cnt_t);
		//BB_DBG(bb, DBG_CMN, "*rssi_ofdm_avg %02d =  rssi_ofdm_acc %02d / pkt_cnt_ofdm%02d}\n",
		//	avg->rssi_ofdm_avg, avg->rssi_ofdm[i], acc->rssi_ofdm_acc[i], pkt_cnt->pkt_cnt_ofdm);
	}

	/*@======[Lgcy-non-data]=============================================*/
	BB_DBG(bb, DBG_CMN, "[Lgcy-non-data] {%d, %d, %d, %d | %d, %d, %d, %d, %d, %d, %d, %d} {%d}\n",
	       pkt_cnt->pkt_cnt_legacy_non_data[0], pkt_cnt->pkt_cnt_legacy_non_data[1],
	       pkt_cnt->pkt_cnt_legacy_non_data[2], pkt_cnt->pkt_cnt_legacy_non_data[3],
	       pkt_cnt->pkt_cnt_legacy_non_data[4], pkt_cnt->pkt_cnt_legacy_non_data[5],
	       pkt_cnt->pkt_cnt_legacy_non_data[6], pkt_cnt->pkt_cnt_legacy_non_data[7],
	       pkt_cnt->pkt_cnt_legacy_non_data[8], pkt_cnt->pkt_cnt_legacy_non_data[9],
	       pkt_cnt->pkt_cnt_legacy_non_data[10], pkt_cnt->pkt_cnt_legacy_non_data[11],
	       pkt_cnt->pkt_cnt_else_non_data);

	/*@======CCK=========================================================*/

	BB_DBG(bb, DBG_CMN, "*CCK     RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d}\n",
	       avg->rssi_cck_avg >> 1,
	       avg->rssi_cck[0] >> 1, avg->rssi_cck[1] >> 1,
	       pkt_cnt->pkt_cnt_cck,
	       pkt_cnt->pkt_cnt_legacy[0], pkt_cnt->pkt_cnt_legacy[1],
	       pkt_cnt->pkt_cnt_legacy[2], pkt_cnt->pkt_cnt_legacy[3]);

	/*@======OFDM========================================================*/
	BB_DBG(bb, DBG_CMN, "*OFDM    RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d}\n",
	       avg->rssi_ofdm_avg >> 1,
	       avg->rssi_ofdm[0] >> 1, avg->rssi_ofdm[1] >> 1,
	       pkt_cnt->pkt_cnt_ofdm,
	       pkt_cnt->pkt_cnt_legacy[4], pkt_cnt->pkt_cnt_legacy[5],
	       pkt_cnt->pkt_cnt_legacy[6], pkt_cnt->pkt_cnt_legacy[7],
	       pkt_cnt->pkt_cnt_legacy[8], pkt_cnt->pkt_cnt_legacy[9],
	       pkt_cnt->pkt_cnt_legacy[10], pkt_cnt->pkt_cnt_legacy[11]);

	/*@======HT==========================================================*/
	
	if (pkt_cnt->ht_pkt_not_zero) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = (i << 3);
			for (j = 0; j < HT_NUM_MCS ; j++) {
 				pkt_cnt_ss += pkt_cnt->pkt_cnt_ht[ss_ofst + j];
			}

			if (pkt_cnt_ss == 0) {
				rssi_avg_tmp = 0;
				rssi_tmp[0] = 0;
				rssi_tmp[1] = 0;
			} else {
				rssi_avg_tmp = avg->rssi_t_avg >> 1;
				rssi_tmp[0] = avg->rssi_t[0] >> 1;
				rssi_tmp[1] = avg->rssi_t[1] >> 1;
			}

			BB_DBG(bb, DBG_CMN,
			       "*HT%02d:%02d RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d}\n",
			       (ss_ofst), (ss_ofst + 7),
			       rssi_avg_tmp, rssi_tmp[0], rssi_tmp[1],
			       pkt_cnt_ss,
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 0],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 1],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 2],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 3],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 4],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 5],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 6],
			       pkt_cnt->pkt_cnt_ht[ss_ofst + 7]);

			pkt_cnt_ss = 0;
		}
	}

	/*@======VHT==========================================================*/
	if (pkt_cnt->vht_pkt_not_zero) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			for (j = 0; j < HE_VHT_NUM_MCS ; j++) {
				pkt_cnt_ss += pkt_cnt->pkt_cnt_vht[ss_ofst + j];
			}

			if (pkt_cnt_ss == 0) {
				rssi_avg_tmp = 0;
				rssi_tmp[0] = 0;
				rssi_tmp[1] = 0;
			} else {
				rssi_avg_tmp = avg->rssi_t_avg >> 1;
				rssi_tmp[0] = avg->rssi_t[0] >> 1;
				rssi_tmp[1] = avg->rssi_t[1] >> 1;
			}

			BB_DBG(bb, DBG_CMN,
				  "*VHT %d-S RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  rssi_avg_tmp, rssi_tmp[0], rssi_tmp[1],
			          pkt_cnt_ss,
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_vht[ss_ofst + 11]);

			pkt_cnt_ss = 0;
		}

	}

	/*@======HE==========================================================*/
	if (pkt_cnt->he_pkt_not_zero) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = HE_VHT_NUM_MCS * i;

			for (j = 0; j < HE_VHT_NUM_MCS ; j++) {
				pkt_cnt_ss += pkt_cnt->pkt_cnt_he[ss_ofst + j];
			}

			if (pkt_cnt_ss == 0) {
				rssi_avg_tmp = 0;
				rssi_tmp[0] = 0;
				rssi_tmp[1] = 0;
			} else {
				rssi_avg_tmp = avg->rssi_t_avg >> 1;
				rssi_tmp[0] = avg->rssi_t[0] >> 1;
				rssi_tmp[1] = avg->rssi_t[1] >> 1;
			}

			BB_DBG(bb, DBG_CMN,
				  "*HE %d-SS RSSI:{%02d| %02d,%02d} cnt:{%03d| %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  rssi_avg_tmp, rssi_tmp[0], rssi_tmp[1],
			          pkt_cnt_ss,
				  pkt_cnt->pkt_cnt_he[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_he[ss_ofst + 11]);

			pkt_cnt_ss = 0;
		}

	}
	
	/*@======SC_BW========================================================*/
	
	if (pkt_cnt->sc20_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[Low BW 20M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc20[ss_ofst + 11]);
		}
	}

	if (pkt_cnt->sc40_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[Low BW 40M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc40[ss_ofst + 11]);
		}
	}

	if (pkt_cnt->sc80_occur) {
		for (i = 0; i < rate_num; i++) {
			ss_ofst = 12 * i;

			BB_DBG(bb, DBG_CMN,
				  "*[Low BW 80M] %d-ss MCS[0:11] = {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}\n",
				  (i + 1),
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 0],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 1],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 2],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 3],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 4],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 5],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 6],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 7],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 8],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 9],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 10],
				  pkt_cnt->pkt_cnt_sc80[ss_ofst + 11]);
		}
	}
}

void halbb_rx_pkt_su_non_data_cnt_rpt(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u8 ofst = rate_i->idx;

	if (rate_i->mode == BB_LEGACY_MODE) {
		pkt_cnt->pkt_cnt_legacy_non_data[ofst]++;
	} else {
		pkt_cnt->pkt_cnt_else_non_data++;
	}
}

void halbb_rx_pkt_su_cnt_rpt(struct bb_info *bb, struct physts_rxd *desc, enum channel_width rx_bw)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	enum channel_width bw_curr; /*max bw in current link mode*/
	u8 band_idx;
	u8 ofst = rate_i->idx;

	pkt_cnt->pkt_cnt_all++;

	if (rate_i->mode == BB_LEGACY_MODE) {
		if (cmn_rpt->is_cck_rate)
			pkt_cnt->pkt_cnt_cck++;
		else
			pkt_cnt->pkt_cnt_ofdm++;
	} else {
		pkt_cnt->pkt_cnt_t++;
		if (rate_i->ss == 1)
			pkt_cnt->pkt_cnt_1ss++;
		else if (rate_i->ss == 2)
			pkt_cnt->pkt_cnt_2ss++;

		pkt_cnt->gi_ltf_cnt[desc->gi_ltf]++;
	}

	band_idx = (desc->phy_idx == HW_PHY_0) ? 0 : 1;
	bw_curr = bb->hal_com->band[band_idx].cur_chandef.bw;

	if (rate_i->mode == BB_LEGACY_MODE) {
		pkt_cnt->pkt_cnt_legacy[ofst]++;
		return;
	}

	
	if (rate_i->ss >= 2 && rate_i->mode >= BB_VHT_MODE)
		ofst += (HE_VHT_NUM_MCS * (rate_i->ss - 1));

	if (rate_i->mode == BB_HT_MODE) {
		pkt_cnt->ht_pkt_not_zero = true;
		ofst = NOT_GREATER(ofst, HT_RATE_NUM - 1);

		if (rx_bw == bw_curr) {
			pkt_cnt->pkt_cnt_ht[ofst]++;
			return;
		}

	} else if (rate_i->mode == BB_VHT_MODE) {
		pkt_cnt->vht_pkt_not_zero = true;
		ofst = NOT_GREATER(ofst, VHT_RATE_NUM - 1);

		if (rx_bw == bw_curr) {
			pkt_cnt->pkt_cnt_vht[ofst]++;
			return;
		}

	} else if (rate_i->mode == BB_HE_MODE) {
		pkt_cnt->he_pkt_not_zero = true;
		ofst = NOT_GREATER(ofst, HE_RATE_NUM - 1);

		if (rx_bw == bw_curr) {
			pkt_cnt->pkt_cnt_he[ofst]++;
			return;
		}
	}

	/*SC BW*/
	if (rx_bw != bw_curr) {
		ofst = NOT_GREATER(ofst, LOW_BW_RATE_NUM - 1);

		if (rx_bw == CHANNEL_WIDTH_20) {
			pkt_cnt->pkt_cnt_sc20[ofst]++;
			pkt_cnt->sc20_occur = true;
		} else if (rx_bw == CHANNEL_WIDTH_40) {
			pkt_cnt->pkt_cnt_sc40[ofst]++;
			pkt_cnt->sc40_occur = true;
		} else {
			pkt_cnt->pkt_cnt_sc80[ofst]++;
			pkt_cnt->sc80_occur = true;
		}
	}
}

void halbb_rx_pkt_su_phy_hist_per_path(struct bb_info *bb, u32 physts_bitmap)
{
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_4_to_7_info *psts_r = NULL;
	struct bb_physts_rslt_18_info *psts_18 = &physts->bb_physts_rslt_18_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u8 path = 0, intvl = 0;
	//BB_DBG(bb, BIT14, "[%s] physts_bitmap = 0x%x\n", __func__,physts_bitmap);

	/*if (physts_bitmap & BIT(IE18_DBG_OFDM_FD_CMN)) {
		for (path = 0; path < bb->num_rf_path; path ++) {
			intvl = halbb_find_intrvl(bb, psts_18->tmax_idx_lgy_path[path], hist_th->tmax_idx_hist_th, BB_PDP_HIST_TH_SIZE);
			hist->tmax_idx_lgy_path_hist[path][intvl]++;
		}
	}*/

	if (physts_bitmap & BIT(IE04_CMN_EXT_PATH_A)) {
		psts_r = &physts->bb_physts_rslt_4_i;
		acc->snr_per_path_acc[0] += psts_r->snr_lgy;
		//BB_DBG(bb, BIT14, "snr_acc[0](%d) += %d\n", acc->snr_per_path_acc[0], psts_r->snr_lgy);
		/*sig_val is meaningless for legacy packet*/
		if (rate_i->ss >= 2) {
			intvl = halbb_find_intrvl_sign(bb, psts_r->sig_val_y >> 2, hist_th->sig_val_path_hist_th, BB_HIST_TH_SIZE);
			hist->sig_val_path_hist[0][intvl]++;
			acc->sig_val_path_acc[0] += psts_r->sig_val_y;
		}
	}
#if (defined(HALBB_COMPILE_ABOVE_2SS))
	if (physts_bitmap & BIT(IE05_CMN_EXT_PATH_B)) {
		psts_r = &physts->bb_physts_rslt_5_i;
		acc->snr_per_path_acc[1] += psts_r->snr_lgy;
		//BB_DBG(bb, BIT14, "snr_acc[1](%d) += %d\n", acc->snr_per_path_acc[1], psts_r->snr_lgy);
		/*sig_val is meaningless for legacy packet*/
		if (rate_i->ss >= 2) {
			intvl = halbb_find_intrvl_sign(bb, psts_r->sig_val_y >> 2, hist_th->sig_val_path_hist_th, BB_HIST_TH_SIZE);
			hist->sig_val_path_hist[1][intvl]++;
			acc->sig_val_path_acc[1] += psts_r->sig_val_y;
		}
	}
#endif

#if (defined(HALBB_COMPILE_ABOVE_3SS))
	if (physts_bitmap & BIT(IE06_CMN_EXT_PATH_C)) {
		psts_r = &physts->bb_physts_rslt_6_i;
		acc->snr_per_path_acc[2] += psts_r->snr_lgy;
		/*sig_val is meaningless for legacy packet*/
		if (rate_i->ss >= 3) {
			intvl = halbb_find_intrvl_sign(bb, psts_r->sig_val_y >> 2, hist_th->sig_val_path_hist_th, BB_HIST_TH_SIZE);
			hist->sig_val_path_hist[2][intvl]++;
			acc->sig_val_path_acc[2] += psts_r->sig_val_y;
		}
	}
#endif

#if (defined(HALBB_COMPILE_ABOVE_4SS))
	if (physts_bitmap & BIT(IE07_CMN_EXT_PATH_D)) {
		psts_r = &physts->bb_physts_rslt_7_i;
		acc->snr_per_path_acc[3] += psts_r->snr_lgy;
		/*sig_val is meaningless for legacy packet*/
		if (rate_i->ss >= 4) {
			intvl = halbb_find_intrvl_sign(bb, psts_r->sig_val_y >> 2, hist_th->sig_val_path_hist_th, BB_HIST_TH_SIZE);
			hist->sig_val_path_hist[3][intvl]++;
			acc->sig_val_path_acc[3] += psts_r->sig_val_y;
		}
	}
#endif
}

void halbb_rx_pkt_su_phy_hist_per_sts_path(struct bb_info *bb, u32 physts_bitmap)
{
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_20_info *psts_20 = &physts->bb_physts_rslt_20_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u8 sts = 0, path = 0, intvl = 0 ;

	if (physts_bitmap & BIT(IE20_DBG_OFDM_FD_USER_SEG_0)) {
		for (sts = 0; sts < rate_i->ss; sts ++) {
			for (path = 0; path < bb->num_rf_path; path ++) {
				intvl = halbb_find_intrvl(bb, psts_20->tmax_idx_non_lgy_sts_path[sts][path],
							  hist_th->tmax_idx_hist_th, BB_PDP_HIST_TH_SIZE);
				hist->tmax_idx_sts_path_hist[sts][path][intvl]++;

				intvl = halbb_find_intrvl_sign(bb, psts_20->snr_non_lgy_sts_path[sts][path] >> 1,
							  hist_th->evm_hist_th, BB_HIST_TH_SIZE);
				hist->snr_sts_path_hist[sts][path][intvl]++;
				acc->snr_sts_path_acc[sts][path] += psts_20->snr_non_lgy_sts_path[sts][path] >> 1;
			}
		}
	}
}

void halbb_rx_pkt_su_phy_hist(struct bb_info *bb)
{
	struct bb_physts_rslt_1_info	*psts_1 = &bb->bb_physts_i.bb_physts_rslt_1_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	struct bb_cfo_trk_info *bb_cfo_trk = &bb->bb_cfo_trk_i;
	u16 tmp_u16;
	u8 intvl = 0;
	s16 cfo;

	if (cmn_rpt->is_cck_rate)
		return;

	//BB_DBG(bb, DBG_CMN, "[2]cfo_avg=%d, evm_max=%d, evm_min=%d, cn_avg=%d\n",
	//	psts_1->cfo_avg, psts_1->evm_max, psts_1->evm_min, psts_1->cn_avg);

	if (rate_i->ss == 1) {
		acc->evm_1ss += psts_1->evm_min;
		intvl = halbb_find_intrvl(bb, (psts_1->evm_min >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
		hist->evm_1ss[intvl]++;
	} else {
		/*EVM min/max Histogram*/
		acc->evm_min_acc += psts_1->evm_min;
		intvl = halbb_find_intrvl(bb, (psts_1->evm_min >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
		hist->evm_min_hist[intvl]++;
		
		acc->evm_max_acc += psts_1->evm_max;
		intvl = halbb_find_intrvl(bb, (psts_1->evm_max >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
		hist->evm_max_hist[intvl]++;
		//BB_DBG(bb, DBG_CMN, "evm_max_hist[%d]=%d\n", intvl, hist->evm_max_hist[intvl]);
	}

	/*SNR_avg Histogram*/
	acc->snr_avg_acc += psts_1->snr_avg;
	intvl = halbb_find_intrvl(bb, psts_1->snr_avg, hist_th->evm_hist_th, BB_HIST_TH_SIZE);
	hist->snr_avg_hist[intvl]++;

	/*CN_avg Histogram*/
	if (rate_i->ss == 2)
		acc->cn_avg_acc += psts_1->cn_avg;
	intvl = halbb_find_intrvl(bb, (psts_1->cn_avg >> 1), hist_th->cn_hist_th, BB_HIST_TH_SIZE);
	hist->cn_avg_hist[intvl]++;

	/*CFO_avg Histogram*/
	if (bb_cfo_trk->cfo_src == CFO_SRC_FD)
		cfo = psts_1->cfo_avg;
	else
		cfo = psts_1->cfo_pab_avg;

	tmp_u16 = (u16)ABS_16(cfo);
	acc->cfo_avg_acc += cfo;
	intvl = halbb_find_intrvl(bb, (tmp_u16 >> 2), hist_th->cfo_hist_th, BB_HIST_TH_SIZE);
	hist->cfo_avg_hist[intvl]++;

	//BB_DBG(bb, DBG_CMN, "[ACC] cfo_avg=%d, evm_max=%d, evm_min=%d, cn_avg=%d\n",
	//	acc->cfo_avg_acc, acc->evm_max_acc, acc->evm_min_acc, acc->cn_avg_acc);
}


void halbb_rx_pkt_mu_phy_hist(struct bb_info *bb)
{
	struct bb_physts_rslt_13_info	*psts_13 = &bb->bb_physts_i.bb_physts_rslt_13_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_acc_info *acc = &cmn_rpt->bb_physts_acc_i;
	struct bb_physts_hist_info *hist = &cmn_rpt->bb_physts_hist_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u8 intvl = 0, i = 0;

	//BB_DBG(bb, DBG_CMN, "[2]cfo_avg=%d, evm_max=%d, evm_min=%d, cn_avg=%d\n",
	//	psts_1->cfo_avg, psts_1->evm_max, psts_1->evm_min, psts_1->cn_avg);
	if (cmn_rpt->n_user_max < psts_13->n_user) {
		if (psts_13->n_user >= MU_USER_MAX)
			cmn_rpt->n_user_max = MU_USER_MAX;
		else
			cmn_rpt->n_user_max = psts_13->n_user;
	}
	for (i = 0; i < psts_13->n_user; i++) {
		if (i >= MU_USER_MAX)
			break;
		if (rate_i->ss == 1) {
			acc->evm_1ss_mu_acc[i] += psts_13->bb_physts_uer_info[i].rx_evm_min_seg_cr;
			intvl = halbb_find_intrvl(bb, (psts_13->bb_physts_uer_info[i].rx_evm_min_seg_cr >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
			hist->evm_1ss_mu_hist[i][intvl]++;
		} else {
			/*EVM min/max Histogram*/
			acc->evm_min_mu_acc[i] += psts_13->bb_physts_uer_info[i].rx_evm_min_seg_cr;
			intvl = halbb_find_intrvl(bb, (psts_13->bb_physts_uer_info[i].rx_evm_min_seg_cr >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
			hist->evm_min_mu_hist[i][intvl]++;

			acc->evm_max_mu_acc[i] += psts_13->bb_physts_uer_info[i].rx_evm_max_seg_cr;
			intvl = halbb_find_intrvl(bb, (psts_13->bb_physts_uer_info[i].rx_evm_max_seg_cr >> 2), hist_th->evm_hist_th, BB_HIST_TH_SIZE);
			hist->evm_max_mu_hist[i][intvl]++;
		}

		/*SNR_avg Histogram*/
		acc->snr_mu_acc[i] += psts_13->bb_physts_uer_info[i].snr;
		intvl = halbb_find_intrvl(bb, psts_13->bb_physts_uer_info[i].snr >> 1, hist_th->evm_hist_th, BB_HIST_TH_SIZE);
		hist->snr_mu_hist[i][intvl]++;

		/*CN_avg Histogram*/
		if (rate_i->ss >= 2) {
			if (!((bb->ic_type & BB_IC_AX_SERIES) && (rate_i->gi_ltf > RTW_GILTF_SGI_4XHE08)) &&
			    psts_13->bb_physts_uer_info[i].avg_cn_seg_cr != 0x7F) {
				acc->cn_mu_acc[i] += psts_13->bb_physts_uer_info[i].avg_cn_seg_cr;
				acc->pkt_cnt_cn_mu_valid++;
				intvl = halbb_find_intrvl(bb, (psts_13->bb_physts_uer_info[i].avg_cn_seg_cr >> 1), hist_th->cn_hist_th, BB_HIST_TH_SIZE);
				hist->cn_mu_hist[i][intvl]++;
			}
		}
	}

}
void halbb_rx_pkt_su_rssi_statistic(struct bb_info *bb)
{
	struct bb_physts_rslt_hdr_info	*psts_h = &bb->bb_physts_i.bb_physts_rslt_hdr_i;
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_rssi_su_acc_info *rssi_su_acc = &cmn_rpt->bb_rssi_su_acc_i;
	struct bb_rate_info *rate_i = &cmn_rpt->bb_rate_i;
	u32 *rssi_acc;
	u8 i = 0;

	if (rate_i->mode == BB_LEGACY_MODE) {
		if (cmn_rpt->is_cck_rate) {
			rssi_su_acc->rssi_cck_avg_acc += psts_h->rssi_avg;
			rssi_acc = &rssi_su_acc->rssi_cck_acc[0];
			//BB_DBG(bb, DBG_PHY_STS, "cck = %d\n", rssi_su_acc->rssi_cck_avg_acc);
		} else {
			rssi_su_acc->rssi_ofdm_avg_acc += psts_h->rssi_avg;
			rssi_acc = &rssi_su_acc->rssi_ofdm_acc[0];
			//BB_DBG(bb, DBG_PHY_STS, "ofdm = %d\n", rssi_su_acc->rssi_ofdm_avg_acc);
		}
	} else {
		rssi_su_acc->rssi_t_avg_acc += psts_h->rssi_avg;
		rssi_acc = &rssi_su_acc->rssi_t_acc[0];
		//BB_DBG(bb, DBG_PHY_STS, "Xt = %d\n", rssi_su_acc->rssi_t_avg_acc);
	}

	/*HT/VHT/HE*/
	for (i = 0; i < HALBB_MAX_PATH; i++) {
		if (i >= bb->num_rf_path)
			break;

		rssi_acc[i] += (u32)psts_h->rssi[i];
	}
	//BB_DBG(bb, DBG_PHY_STS, "rssi_acc = %d, %d\n", rssi_acc[0], rssi_acc[1]);
}

//#define ORI_RSSI_FLAG

void halbb_rx_pkt_su_store_in_sta_info(struct bb_info *bb, struct physts_rxd *desc)
{
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_hdr_info	*psts_h = &physts->bb_physts_rslt_hdr_i;
	struct bb_physts_rslt_1_info	*psts_1 = &physts->bb_physts_rslt_1_i;
	struct rtw_phl_stainfo_t *phl_sta;
	struct rtw_rssi_info *rssi_t = NULL;
	u8 ma_fac = 2;
	u8 bb_macid;
	u8 i = 0;

	if (desc->macid_su >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s] macid_su=%d\n", __func__, desc->macid_su);
		return;
	}

	bb_macid = bb->phl2bb_macid_table[desc->macid_su];

	if (bb_macid >= PHL_MAX_STA_NUM) {
		BB_WARNING("[%s] bb_macid=%d\n", __func__, bb_macid);
		return;
	}

	phl_sta = bb->phl_sta_info[bb_macid];

	if (!is_sta_active(phl_sta))
		return;

	if (phl_sta->macid >= PHL_MAX_STA_NUM)
		return;

	if (!phl_sta->hal_sta)
		return;

	rssi_t = &phl_sta->hal_sta->rssi_stat;

	//BB_DBG(bb, DBG_PHY_STS, "desc->macid_su=%d, bb_macid=%d, phl_sta->macid=%d\n",
	//	desc->macid_su, bb_macid, phl_sta->macid);

	//BB_DBG(bb, DBG_PHY_STS, "[pre] psts_h->rssi_avg = %d, rssi_ma=%d\n", psts_h->rssi_avg, rssi_t->rssi_ma);

	if (desc->user_i[0].is_bcn) {
		ma_fac = rssi_t->ma_factor_bcn;
		if (rssi_t->rssi_bcn_ma == 0) {
			rssi_t->rssi_bcn_ma = (s16)(psts_h->rssi_avg << RSSI_MA_H);
			rssi_t->rssi_bcn = (s8)psts_h->rssi_avg;
		} else {
			rssi_t->rssi_bcn_ma = MA_ACC(rssi_t->rssi_bcn_ma, (u16)psts_h->rssi_avg, ma_fac, RSSI_MA_H);
			rssi_t->rssi_bcn = (u8)GET_MA_VAL(rssi_t->rssi_bcn_ma, RSSI_MA_H);
		}

		for (i = 0; i < HALBB_MAX_PATH; i++) {
			if (!(physts->rx_path_en & BIT(i)))
				continue;

			if (rssi_t->rssi_bcn_ma_path[i] == 0) {
				rssi_t->rssi_bcn_ma_path[i] = (u16)(psts_h->rssi[i] << RSSI_MA_H);
			} else {
				rssi_t->rssi_bcn_ma_path[i] = MA_ACC(rssi_t->rssi_bcn_ma_path[i], (u16)psts_h->rssi[i], ma_fac, RSSI_MA_H);
			}
		}

		rssi_t->pkt_cnt_bcn++;
	} else {
		ma_fac = rssi_t->ma_factor;
		if (rssi_t->rssi_ma == 0) {
			rssi_t->rssi_ma = (s16)(psts_h->rssi_avg << RSSI_MA_H);
			rssi_t->rssi = (s8)psts_h->rssi_avg;

			if (!bb->bb_cmn_rpt_i.is_cck_rate)
				rssi_t->snr_ma = (u16)psts_1->snr_avg << RSSI_MA_H;
			//BB_DBG(bb, DBG_BIT21, "[First][macid:%d] snr=%d, rssi_ori=%d, rssi_ma=%d, rssi_ma16=%d\n", bb_macid, rssi_t->snr_ma, psts_h->rssi_avg, rssi_t->rssi, rssi_t->rssi_ma);
		} else {
			rssi_t->rssi_ma = MA_ACC(rssi_t->rssi_ma, (u16)psts_h->rssi_avg, ma_fac, RSSI_MA_H);
			rssi_t->rssi = (u8)GET_MA_VAL(rssi_t->rssi_ma, RSSI_MA_H);

			if (!bb->bb_cmn_rpt_i.is_cck_rate)
				rssi_t->snr_ma = MA_ACC(rssi_t->snr_ma, (u16)psts_1->snr_avg, ma_fac, RSSI_MA_H);
			//BB_DBG(bb, DBG_BIT21, "[NORML][macid:%d] snr=%d, rssi_ori=%d, rssi_ma=%d, rssi_ma16=%d\n", bb_macid, rssi_t->snr_ma, psts_h->rssi_avg, rssi_t->rssi, rssi_t->rssi_ma);
		}

		for (i = 0; i < HALBB_MAX_PATH; i++) {
			if (!(physts->rx_path_en & BIT(i)))
				continue;

			if (rssi_t->rssi_ma_path[i] == 0) {
				rssi_t->rssi_ma_path[i] = (u16)(psts_h->rssi[i] << RSSI_MA_H);
			} else {
				rssi_t->rssi_ma_path[i] = MA_ACC(rssi_t->rssi_ma_path[i], (u16)psts_h->rssi[i], ma_fac, RSSI_MA_H);
			}
		}

		rssi_t->pkt_cnt_data++;
	}

	//BB_DBG(bb, DBG_PHY_STS, "[%d] rssi = %d, rssi_ma=%d\n", bb_macid, rssi_t->rssi, rssi_t->rssi_ma);

	if (bb->bb_cmn_rpt_i.is_cck_rate)
		rssi_t->rssi_cck = psts_h->rssi_avg;
	else
		rssi_t->rssi_ofdm = psts_h->rssi_avg;

	//BB_DBG(bb, DBG_PHY_STS, "[%d] rssi_cck = %d, rssi_ofdm=%d\n", bb_macid, rssi_t->rssi_cck, rssi_t->rssi_ofdm);
}

void halbb_get_rx_pkt_cnt_rpt_su(struct bb_info *bb, struct bb_pkt_cnt_su_info *pkt_cnt_rpt)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;

	halbb_mem_cpy(bb, pkt_cnt_rpt, pkt_cnt, sizeof(struct bb_pkt_cnt_su_info));
}
#endif

void halbb_cmn_rpt_export_physts_avg_rpt(struct bb_info * bb, struct bb_physts_avg_info *info)
{
	halbb_mem_cpy(bb, info, &bb->bb_cmn_rpt_i.bb_physts_avg_i, sizeof(struct bb_physts_avg_info));
}

void halbb_cmn_info_rpt_store_data(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_pkt_cnt_su_info *pkt_cnt = &cmn_rpt->bb_pkt_cnt_su_i;
	struct bb_pkt_cnt_su_store_info *store = &cmn_rpt->bb_pkt_cnt_su_store_i;

	store->ht_pkt_not_zero = pkt_cnt->ht_pkt_not_zero;
	store->vht_pkt_not_zero = pkt_cnt->vht_pkt_not_zero;
	store->he_pkt_not_zero = pkt_cnt->he_pkt_not_zero;
}

void halbb_cmn_info_rpt_reset(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;

	if (physts->hold_type != PSTS_HOLD_DISABLE) {
		BB_DBG(bb, DBG_PHY_STS, "[%s] Hold!\n", __func__);
		return;
	}

	cmn_rpt->bb_pkt_cnt_bcn_i.beacon_cnt_in_period = cmn_rpt->bb_pkt_cnt_bcn_i.pkt_cnt_beacon;
	cmn_rpt->bb_pkt_cnt_bcn_i.pkt_cnt_beacon = 0;

	bb->bb_ch_i.rxsc_l = 0xff;
	bb->bb_ch_i.rxsc_20 = 0xff;
	bb->bb_ch_i.rxsc_40 = 0xff;
	bb->bb_ch_i.rxsc_80 = 0xff;

	halbb_rx_pkt_cnt_rpt_reset(bb);
}

void halbb_cmn_rpt_watchdog(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_link_info *link = &bb->bb_link_i;
	struct rtw_phl_stainfo_t *sta;
	struct rtw_ra_sta_info *ra;
	struct rtw_rate_info *rate_info;
	u32 macid  = link->first_entry_macid;

	if (!link->is_linked)
		return;

	halbb_cmn_rpt_get_avg_val(bb);
	/*Link Info Update*/
	link->rx_rate_plurality = halbb_get_plurality_rx_rate_su(bb);
	link->rx_rate_plurality_mu = halbb_get_plurality_rx_rate_mu(bb);
	link->avg_phy_rate = halbb_rx_avg_phy_rate(bb);
	link->rx_utility = halbb_rx_utility(bb, link->avg_phy_rate, bb->num_rf_path, bb->hal_com->band[bb->bb_phy_idx].cur_chandef.bw);

	/*First STA Info for debug*/
	if (bb->sta_exist[macid]) {
		sta = bb->phl_sta_info[macid];

		if (!is_sta_active(sta))
			return;

		ra = &sta->hal_sta->ra_info;

		if (bb->bb_cmn_hooker->bb_ra_drv_i.is_fw_fix_rate)
			rate_info = &ra->fixed_rt_i;
		else
			rate_info = &ra->rpt_rt_i;

		if (bb->ic_type & BB_IC_AX_SERIES)
			link->tx_rate = (u16)(rate_info->mcs_ss_idx & 0x7f) | ((u16)(rate_info->mode & 0x3) << 7); /*store for debug*/
		else
			link->tx_rate = (u16)(rate_info->mcs_ss_idx & 0xff) | ((u16)(rate_info->mode & 0xf) << 8); /*store for debug*/
	}

	//BB_DBG(bb, DBG_CMN, "tx_rate=0x%x, rx_rate_plurality=0x%x, rx_rate_plurality_mu=0x%x, avg_phy_rate=%d, rx_utility=%d\n",
	//       link->tx_rate, link->rx_rate_plurality, link->rx_rate_plurality_mu, link->avg_phy_rate, link->rx_utility);
}

void halbb_cmn_rpt(struct bb_info *bb, struct physts_rxd *desc, u32 physts_bitmap)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_info	*physts = &bb->bb_physts_i;
	struct bb_physts_rslt_1_info *psts_1 = &physts->bb_physts_rslt_1_i;
	struct bb_physts_rslt_0_info *psts_0 = &physts->bb_physts_rslt_0_i;
	enum channel_width rx_bw = psts_1->bw_idx;

	if (desc->is_su) {
		halbb_rate_idx_parsor(bb, desc->data_rate, (enum rtw_gi_ltf)desc->gi_ltf, &cmn_rpt->bb_rate_i);
	} else if (physts_bitmap & BIT(IE13_DL_MU_DEF)) {
		halbb_mu_rate_idx_generate(bb, desc, &cmn_rpt->bb_rate_i);
	} else {
		BB_DBG(bb, DBG_PHY_STS, "[MU] physts_bitmap error=%d\n", physts_bitmap);
		return;
	}

	cmn_rpt->is_cck_rate = halbb_is_cck_rate(bb, desc->data_rate);
	physts->rx_path_en = (cmn_rpt->is_cck_rate) ? psts_0->rx_path_en_cck : psts_1->rx_path_en;

	if (cmn_rpt->bb_rate_i.mode == BB_LEGACY_MODE)
		bb->bb_ch_i.rxsc_l = psts_1->rxsc;
	else if (psts_1->bw_idx == CHANNEL_WIDTH_20)
		bb->bb_ch_i.rxsc_20 = psts_1->rxsc;
	else if (psts_1->bw_idx == CHANNEL_WIDTH_40)
		bb->bb_ch_i.rxsc_40 = psts_1->rxsc;
	else if (psts_1->bw_idx == CHANNEL_WIDTH_80)
		bb->bb_ch_i.rxsc_80 = psts_1->rxsc;
	else if (psts_1->bw_idx == CHANNEL_WIDTH_160)
		bb->bb_ch_i.rxsc_160 = psts_1->rxsc;

	BB_DBG(bb, DBG_PHY_STS, "is_su = %d\n", desc->is_su);

	if (desc->is_su) {
		if (desc->user_i[0].is_data || desc->user_i[0].is_bcn) {/*@data frame only*/
			halbb_rx_pkt_su_store_in_sta_info(bb, desc);
			halbb_rx_pkt_su_cnt_rpt(bb, desc, rx_bw);
			halbb_rx_pkt_su_rssi_statistic(bb);
			halbb_rx_pkt_su_phy_hist(bb);
			halbb_rx_pkt_su_phy_hist_per_path(bb, physts_bitmap);
			halbb_rx_pkt_su_phy_hist_per_sts_path(bb, physts_bitmap);

			if (desc->user_i[0].is_bcn)
				halbb_rx_pkt_cnt_rpt_beacon(bb, desc);
		} else {
			halbb_rx_pkt_su_non_data_cnt_rpt(bb, desc, rx_bw);
		}
	} else {
		halbb_rx_pkt_mu_cnt_rpt(bb, desc, rx_bw);
		halbb_rx_pkt_mu_rssi_statistic(bb);
		if (physts_bitmap & BIT(IE13_DL_MU_DEF))
			halbb_rx_pkt_mu_phy_hist(bb);
	}
	halbb_rx_pop_hist(bb);
	halbb_idle_time_pwr_physts(bb, desc, cmn_rpt->is_cck_rate);
}

void halbb_physts_hist_init(struct bb_info *bb)
{
	struct bb_cmn_rpt_info	*cmn_rpt = &bb->bb_cmn_rpt_i;
	struct bb_physts_hist_th_info *hist_th = &cmn_rpt->bb_physts_hist_th_i;
	u8 evm_hist_th[BB_HIST_TH_SIZE] = {5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35};
	u8 cn_hist_th[BB_HIST_TH_SIZE] = {2, 3, 4, 5, 6, 8, 10, 12, 14, 16, 18};
	u8 cfo_hist_th[BB_HIST_TH_SIZE] = {1, 5, 10, 15, 20, 30, 60, 90, 120, 150, 200};
	u8 tmax_idx_hist_th[BB_PDP_HIST_TH_SIZE] = {1, 2, 3, 4, 5};
	s8 sig_val_path_hist_th[BB_HIST_TH_SIZE] = {-15, -12, -9, -6, -3, 0, 3, 6, 9, 12, 15};

	halbb_mem_cpy(bb, hist_th->evm_hist_th, evm_hist_th, BB_HIST_TH_SIZE);
	halbb_mem_cpy(bb, hist_th->cn_hist_th, cn_hist_th, BB_HIST_TH_SIZE);
	halbb_mem_cpy(bb, hist_th->cfo_hist_th, cfo_hist_th, BB_HIST_TH_SIZE);
	halbb_mem_cpy(bb, hist_th->tmax_idx_hist_th, tmax_idx_hist_th, BB_PDP_HIST_TH_SIZE);
	halbb_mem_cpy(bb, hist_th->sig_val_path_hist_th, sig_val_path_hist_th, BB_HIST_TH_SIZE);

}

void halbb_cmn_rpt_init(struct bb_info *bb)
{
	BB_DBG(bb, DBG_DBG_API, "%s\n", __func__);

	halbb_physts_hist_init(bb);
	halbb_rx_pkt_cnt_rpt_reset(bb);
}
#endif
