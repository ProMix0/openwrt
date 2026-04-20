/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#include "halrf_precomp.h"

void halrf_show_rf_ic_info(struct rf_info *rf) 
{
	char *ic_name = NULL;
	
	switch (rf->ic_type) {
#ifdef RF_8852A_SUPPORT
	case RF_RTL8852A:
		ic_name = "RF_RTL8852A";
	break;
#endif
#ifdef RF_8852B_SUPPORT
	case RF_RTL8852B:
		ic_name = "RF_RTL8852B";
	break;
#endif
#ifdef RF_8852C_SUPPORT
	case RF_RTL8852C:
		ic_name = "RF_RTL8852C";
		break;
#endif
#ifdef RF_8852D_SUPPORT
	case RF_RTL8852D:
		ic_name = "RF_RTL8852D";
		break;
#endif
#ifdef RF_8832D_SUPPORT
	case RF_RTL8832D:
		ic_name = "RF_RTL8832D";
		break;
#endif

#ifdef RF_8832BR_SUPPORT
	case RF_RTL8832BR:
		ic_name = "RF_RTL8832BR";
	break;
#endif
#ifdef RF_8192XB_SUPPORT
	case RF_RTL8192XB:
		ic_name = "RF_RTL8192XB";
	break;
#endif
#ifdef RF_8852BP_SUPPORT
	case RF_RTL8852BP:
		ic_name = "RF_RTL8852BP";
	break;
#endif
#ifdef RF_8832CRVU_SUPPORT
	case RF_RTL8832CR_VU:
		ic_name = "RF_RTL8832CR_VU";
		break;
#endif
#ifdef RF_8832BRVT_SUPPORT
	case RF_RTL8832BR_VT:
		ic_name = "RF_RTL8832BR_VT";
	break;
#endif

	default:
		ic_name = "no_info";
	break;
	}

	if (rf->use_sub_did)
		RF_DBG(rf, DBG_RF_INIT, "[HALRF] Use Sub_did=0x%x\n", rf->sub_did);

	RF_DBG(rf, DBG_RF_INIT, "[HALRF] rf->ic_type = %s\n", ic_name);
		

}

void halrf_cmn_info_self_init(struct rf_info *rf)
{
	struct rtw_hal_com_t *hal_i = rf->hal_com;
#ifdef PHL_PLATFORM_AP
	struct dvobj_priv *dvobj = hal_i->drv_priv;
#endif
	
	rf->use_sub_did = false;
	rf->sub_did = 0;

	
#ifdef PHL_PLATFORM_AP
	
	#ifdef CONFIG_RTL8832CRVU
	RF_DBG(rf, DBG_RF_INIT, "[HALRF] RTL8832CRVU is defined !!\n");
	#endif

	#ifdef CONFIG_RTL8832BRVT
	RF_DBG(rf, DBG_RF_INIT, "[HALRF] RTL8832BRVT is defined !!\n");
	#endif

	#ifdef CONFIG_RTL8192XBVS
	RF_DBG(rf, DBG_RF_INIT, "[HALRF] RTL8192XBVS is defined !!\n");
	#endif

	#if defined(CONFIG_RTL8832CRVU) || defined(CONFIG_RTL8832BRVT) || defined(CONFIG_RTL8192XBVS)
	RF_DBG(rf, DBG_RF_INIT, "[HALRF] Sub_did=0x%x\n", dvobj->pci_subdid);
	
	if ((dvobj->pci_subdid == RF_SUBDID_RTL8832CRVU) || (dvobj->pci_subdid == RF_SUBDID_RTL8832BRVT)) {
		rf->use_sub_did = true;
		rf->sub_did = dvobj->pci_subdid;
	}	
	#endif
#endif


	if (hal_i->chip_id == CHIP_WIFI6_8852A)
		rf->ic_type = RF_RTL8852A;
	else if (hal_i->chip_id == CHIP_WIFI6_8834A)
		rf->ic_type = RF_RTL8834A;
	else if (hal_i->chip_id == CHIP_WIFI6_8852B)
		rf->ic_type = RF_RTL8852B;
	else if (hal_i->chip_id == CHIP_WIFI6_8852C)
		rf->ic_type = RF_RTL8852C;

#if defined (RF_8852D_SUPPORT) || defined (RF_8832D_SUPPORT)
	else if (hal_i->chip_id == CHIP_WIFI6_8852D) {
	#ifdef PHL_PLATFORM_AP
		rf->ic_type = RF_RTL8832D;
	#else
		rf->ic_type = RF_RTL8852D;
	#endif
	}
#endif
#ifdef RF_8832BR_SUPPORT
	if (hal_i->chip_id == CHIP_WIFI6_8832BR)
		rf->ic_type = RF_RTL8832BR;
#endif
#ifdef RF_8192XB_SUPPORT
	if (hal_i->chip_id == CHIP_WIFI6_8192XB)
		rf->ic_type = RF_RTL8192XB;
#endif
#ifdef RF_8852BP_SUPPORT
	else if (hal_i->chip_id == CHIP_WIFI6_8852BP)
		rf->ic_type = RF_RTL8852BP;
#endif


#ifdef PHL_PLATFORM_AP
	if (rf->use_sub_did) {
	#ifdef RF_8832CRVU_SUPPORT
		if (rf->sub_did == RF_SUBDID_RTL8832CRVU)
			rf->ic_type = RF_RTL8832CR_VU;
	#endif
	#ifdef RF_8832BRVT_SUPPORT
		if (rf->sub_did == RF_SUBDID_RTL8832BRVT)
			rf->ic_type = RF_RTL8832BR_VT;
	#endif
	}
#endif


	if (rf->ic_type & RF_AX_1SS)
		rf->num_rf_path = 1;
	else if (rf->ic_type & RF_AX_2SS)
		rf->num_rf_path = 2;
	else if (rf->ic_type & RF_AX_3SS)
		rf->num_rf_path = 3;
	else if (rf->ic_type & RF_AX_4SS)
		rf->num_rf_path = 4;
	else
		rf->num_rf_path = 1;

	rf->support_ability = 0xffffffff;
	rf->rf_init_ready = false;
	rf->rf_sys_up_time = 0;
	rf->rf_watchdog_en = true;
	rf->rf_ic_api_en = true;
	/*[Drv Dbg Info]*/
	rf->dbg_component = 0;
	rf->cmn_dbg_msg_period = 2;
	rf->cmn_dbg_msg_cnt = 0;

	//log
	halrf_show_rf_ic_info(rf);

	/*@=== [HALRF Structure] ============================================*/

	//switch (hal_i->chip_id) {
	switch (rf->ic_type) {
#ifdef RF_8852A_SUPPORT
	//case CHIP_WIFI6_8852A:
	case RF_RTL8852A:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8852a;
	break;
#endif
#ifdef RF_8852B_SUPPORT
	//case CHIP_WIFI6_8852B:
	case RF_RTL8852B:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8852b;
	break;
#endif
#ifdef RF_8852C_SUPPORT
	//case CHIP_WIFI6_8852C:
	case RF_RTL8852C:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8852c;
		break;
#endif
#ifdef RF_8832D_SUPPORT
	//case CHIP_WIFI6_8832D:
	case RF_RTL8832D:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8832d;
		break;
#endif
#ifdef RF_8832BR_SUPPORT
	//case CHIP_WIFI6_8832BR:
	case RF_RTL8832BR:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8832br;
	break;
#endif
#ifdef RF_8192XB_SUPPORT
	//case CHIP_WIFI6_8192XB:
	case RF_RTL8192XB:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8192xb;
	break;
#endif
#ifdef RF_8852BP_SUPPORT
	//case CHIP_WIFI6_8852BP:
	case RF_RTL8852BP:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8852bp;
	break;
#endif
#ifdef RF_8832CRVU_SUPPORT
	//case CHIP_WIFI6_8832CRVU:
	case RF_RTL8832CR_VU:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8832crvu;
		break;
#endif
#ifdef RF_8832BRVT_SUPPORT
	//case CHIP_WIFI6_8832BRVT:
	case RF_RTL8832BR_VT:
		rf->rfk_iqk_info = &rf_iqk_hwspec_8832brvt;
	break;
#endif


	default:
	break;
	}
}

void halrf_rfk_self_init(struct rf_info *rf)
{
	struct halrf_iqk_info *iqk_info = &rf->iqk;
	struct halrf_gapk_info *txgapk_info = &rf->gapk;

	u8 path;

	RF_DBG(rf, DBG_RF_RFK, "===> %s\n", __func__);

	/* [TXGAPK init] */
	txgapk_info->is_gapk_init = false;
	
	/*[RXDCK_init*/
	halrf_rx_dck_init(rf);

	/*[IQK init]*/
	iqk_info->is_iqk_init = false;

	/*[TSSI init]*/
	halrf_tssi_init(rf);

	/*[DPK init]*/
	halrf_dpk_init(rf);

	/*[RXBB BW]*/
	for (path = 0; path < KPATH; path++)
		rf->pre_rxbb_bw[path] = 0xff;

	/*[TSSI init]*/
	halrf_op5k_init(rf);
}

void halrf_rfability_init_mp(struct rf_info *rf)
{
	//u64 support_ability = 0;

	switch (rf->ic_type) {
#ifdef RF_8852A_SUPPORT
	case RF_RTL8852A:
		rf->hw_rf_ability |=
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_TSSI_TRK |
			HAL_RF_IQK |
			/*HAL_RF_LCK |*/
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_RXGAINK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			0;
		break;
#endif
#ifdef RF_8852B_SUPPORT
	case RF_RTL8852B:
		rf->hw_rf_ability |=
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			/*HAL_RF_LCK |*/
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			/*HAL_RF_RXGAINK |*/
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			HAL_RF_TX_SHAPE |
			0;
		break;
#endif
#ifdef RF_8852C_SUPPORT
	case RF_RTL8852C:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			HAL_RF_RXDCK_TRACK |
			HAL_RF_EDPK | 
			0;
		break;
#endif
#ifdef RF_8852D_SUPPORT
	case RF_RTL8852D:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			/*HAL_RF_IQK |*/
			/*HAL_RF_LCK |*/
			/*HAL_RF_DPK |*/
			/*HAL_RF_DACK |*/
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			/*HAL_RF_RXDCK |*/
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			/*HAL_RF_RXDCK_TRACK |*/
			0;
		break;
#endif
#ifdef RF_8832D_SUPPORT
	case RF_RTL8832D:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			/*HAL_RF_IQK |*/
			/*HAL_RF_LCK |*/
			/*HAL_RF_DPK |*/
			/*HAL_RF_DACK |*/
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			/*HAL_RF_RXDCK |*/
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			/*HAL_RF_RXDCK_TRACK |*/
			0;
		break;
#endif


#ifdef RF_8832BR_SUPPORT
	case RF_RTL8832BR:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_TX_SHAPE |
			HAL_RF_OP5K_TRACK |
			HAL_RF_OP5K |
			0;
		break;
#endif
#ifdef RF_8192XB_SUPPORT
	case RF_RTL8192XB:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_TX_SHAPE |
			HAL_RF_OP5K_TRACK |
			HAL_RF_OP5K |
			HAL_RF_EDPK | 
			0;
		break;
#endif
#ifdef RF_8852BP_SUPPORT
	case RF_RTL8852BP:
		rf->hw_rf_ability |=
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*HAL_RF_DPK |*/
			HAL_RF_DACK |
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			HAL_RF_RXDCK |
			/*HAL_RF_RXGAINK |*/
			/*HAL_RF_THER_TRIM |*/
			/*HAL_RF_PABIAS_TRIM |*/
			/*HAL_RF_TSSI_TRIM |*/
			/*HAL_RF_XTAL_TRACK |*/
			/*HAL_RF_TX_SHAPE |*/
			HAL_RF_RXDCK_TRACK |
			0;
		break;
#endif
#ifdef RF_8832CRVU_SUPPORT
	case RF_RTL8832CR_VU:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*HAL_RF_DPK |*/
			HAL_RF_DACK |
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			HAL_RF_RXDCK |
			/*HAL_RF_THER_TRIM |*/
			/*HAL_RF_PABIAS_TRIM |*/
			/*HAL_RF_TSSI_TRIM |*/
			/*HAL_RF_TX_SHAPE |*/
			/*HAL_RF_TPE_CTRL |*/
			/*HAL_RF_RXDCK_TRACK |*/
			HAL_RF_TXALCTK |
			0;
		break;
#endif

#ifdef RF_8832BRVT_SUPPORT
	case RF_RTL8832BR_VT:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*HAL_RF_DPK |*/
			HAL_RF_DACK |
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			/*HAL_RF_RXDCK |*/
			/*HAL_RF_THER_TRIM |*/
			/*HAL_RF_PABIAS_TRIM |*/
			/*HAL_RF_TSSI_TRIM |*/
			/*HAL_RF_TX_SHAPE |*/
			/*HAL_RF_TPE_CTRL |*/
			/*HAL_RF_RXDCK_TRACK |*/
			HAL_RF_TXALCTK |
			0;
		break;
#endif

	default:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			/*HAL_RF_IQK |*/
			/*HAL_RF_LCK |*/
			/*HAL_RF_DPK |*/
			/*HAL_RF_DACK |*/
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			0;
		break;
	}
}

void halrf_rfability_init(struct rf_info *rf)
{

	switch (rf->ic_type) {
#ifdef RF_8852A_SUPPORT
	case RF_RTL8852A:
		rf->hw_rf_ability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_TSSI_TRK |
			HAL_RF_IQK |
			/*HAL_RF_LCK |*/
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_RXGAINK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_XTAL_TRACK |
			0;
		break;
#endif
#ifdef RF_8852B_SUPPORT
	case RF_RTL8852B:
		rf->hw_rf_ability |=
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			/*HAL_RF_LCK |*/
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			/*HAL_RF_RXGAINK |*/
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			HAL_RF_TX_SHAPE |
			0;
		break;
#endif
#ifdef RF_8852C_SUPPORT
	case RF_RTL8852C:
		rf->hw_rf_ability |=
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			HAL_RF_RXDCK_TRACK |
			HAL_RF_EDPK | 
			0;
		break;
#endif
#ifdef RF_8852D_SUPPORT
	case RF_RTL8852D:
		rf->hw_rf_ability |=
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_XTAL_TRACK |
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			HAL_RF_RXDCK_TRACK |
			0;
		break;
#endif

#ifdef RF_8832D_SUPPORT
	case RF_RTL8832D:
		rf->hw_rf_ability |=
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			HAL_RF_XTAL_TRACK |
			HAL_RF_TX_SHAPE |
			/*HAL_RF_TPE_CTRL |*/
			HAL_RF_RXDCK_TRACK |
			0;
		break;
#endif

#ifdef RF_8832BR_SUPPORT
	case RF_RTL8832BR:
		rf->hw_rf_ability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			HAL_RF_TX_SHAPE |
			HAL_RF_OP5K_TRACK |
			HAL_RF_OP5K |
			0;
		break;
#endif
#ifdef RF_8192XB_SUPPORT
	case RF_RTL8192XB:
		rf->hw_rf_ability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_THER_TRIM |
			HAL_RF_PABIAS_TRIM |
			HAL_RF_TSSI_TRIM |
			/*HAL_RF_XTAL_TRACK |*/
			HAL_RF_TX_SHAPE |
			HAL_RF_OP5K_TRACK |
			HAL_RF_OP5K |
			HAL_RF_EDPK | 
			0;
		break;
#endif
#ifdef RF_8852BP_SUPPORT
	case RF_RTL8852BP:
		rf->hw_rf_ability |=
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*HAL_RF_DPK |*/
			HAL_RF_DACK |
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			HAL_RF_RXDCK |
			/*HAL_RF_RXGAINK |*/
			/*HAL_RF_THER_TRIM |*/
			/*HAL_RF_PABIAS_TRIM |*/
			/*HAL_RF_TSSI_TRIM |*/
			/*HAL_RF_XTAL_TRACK |*/
			/*HAL_RF_TX_SHAPE |*/
			HAL_RF_RXDCK_TRACK |
			0;
		break;
#endif
#ifdef RF_8832CRVU_SUPPORT
	case RF_RTL8832CR_VU:
		rf->hw_rf_ability |=
		HAL_RF_TX_PWR_TRACK |
		HAL_RF_IQK |
		HAL_RF_LCK |
		/*	HAL_RF_DPK |*/
		HAL_RF_DACK |
		/*	HAL_RF_TXGAPK |*/
		/*	HAL_RF_DPK_TRACK |*/
			HAL_RF_RXDCK |
		/*	HAL_RF_THER_TRIM |*/
		/*	HAL_RF_PABIAS_TRIM |*/
		/*	HAL_RF_TSSI_TRIM |*/
			/*HAL_RF_XTAL_TRACK |*/
		/*	HAL_RF_TX_SHAPE |*/
			/*HAL_RF_TPE_CTRL |*/
		/*	HAL_RF_RXDCK_TRACK |*/
			HAL_RF_TXALCTK |
			0;
		break;
#endif

#ifdef RF_8832BRVT_SUPPORT
	case RF_RTL8832BR_VT:
		rf->hw_rf_ability |=
		/*	HAL_RF_TX_PWR_TRACK |*/
		HAL_RF_IQK |
		HAL_RF_LCK |
		/*	HAL_RF_DPK |*/
		HAL_RF_DACK |
		/*	HAL_RF_TXGAPK |*/
		/*	HAL_RF_DPK_TRACK |*/
		/*	HAL_RF_RXDCK |*/
		/*	HAL_RF_THER_TRIM |*/
		/*	HAL_RF_PABIAS_TRIM |*/
		/*	HAL_RF_TSSI_TRIM |*/
		/*HAL_RF_XTAL_TRACK |*/
		/*	HAL_RF_TX_SHAPE |*/
		/*HAL_RF_TPE_CTRL |*/
		/*	HAL_RF_RXDCK_TRACK |*/
			HAL_RF_TXALCTK |
			0;
		break;
#endif


	default:
		rf->hw_rf_ability =
			/*HAL_RF_TX_PWR_TRACK |*/
			/*HAL_RF_IQK |*/
			/*HAL_RF_LCK |*/
			/*HAL_RF_DPK |*/
			/*HAL_RF_DACK |*/
			/*HAL_RF_TXGAPK |*/
			/*HAL_RF_DPK_TRACK |*/
			0;
		break;
	}
}

void halrf_set_rfability(struct rf_info *rf)
{

	halrf_rfability_init(rf);
	rf->hal_com->dev_hw_cap.rfk_cap = rf->hw_rf_ability;

	RF_DBG(rf, DBG_RF_INIT,
	       "IC = ((0x%x)), mp=%d,  hw_rf_ability = ((0x%x))\n",
	       rf->ic_type, rf->phl_com->drv_mode, rf->hw_rf_ability);
}

void halrf_set_final_rfability(struct rf_info *rf)
{

#if 0
	if (phl_is_mp_mode(rf->phl_com)) {
		halrf_rfability_init_mp(rf);
		rf->support_ability = 
			rf->support_ability & rf->hw_rf_ability;
	} else {

		rf->support_ability =
			rf->phl_com->dev_cap.rfk_cap & rf->hw_rf_ability;
	}
#endif

	rf->support_ability =
		rf->phl_com->dev_cap.rfk_cap & rf->hw_rf_ability;

	RF_DBG(rf, DBG_RF_INIT,
	       "IC = ((0x%x)), mp=%d,  RF_Supportability Init = ((0x%x))\n",
	       rf->ic_type, rf->phl_com->drv_mode, rf->support_ability);
}

void halrf_rfe_init(struct rf_info *rf)
{
	struct halrf_dpk_info *dpk = &rf->dpk;

	u8 rfe_type = rf->phl_com->dev_cap.rfe_type;

	switch (rf->ic_type) {
#ifdef RF_8852A_SUPPORT
	case RF_RTL8852A:
		/*2G FEM check*/
		if (rfe_type == 11 || rfe_type == 12 || rfe_type == 17 ||
		    rfe_type == 18 || rfe_type == 51 || rfe_type == 52 ||
		    rfe_type == 53 || rfe_type == 54) {
			rf->fem.epa_2g = 1;
			rf->fem.elna_2g = 1;
		}
		/*5G FEM check*/
		if (rfe_type == 9 || rfe_type == 10 || rfe_type == 11 ||
		    rfe_type == 12 || rfe_type == 15 || rfe_type == 16 ||
		    rfe_type == 17 || rfe_type == 18 || rfe_type == 37 || 
		    rfe_type == 38 || rfe_type == 51 || rfe_type == 52 ||
		    rfe_type == 53 || rfe_type == 54) {
			rf->fem.epa_5g = 1;
			rf->fem.elna_5g = 1;
		}
		/*6G FEM check*/
		if (rfe_type == 13 || rfe_type == 14 || rfe_type == 15 ||
		    rfe_type == 16 || rfe_type == 17 || rfe_type == 18 ||
		    rfe_type == 37 || rfe_type == 38 || rfe_type == 51 ||
		    rfe_type == 52 || rfe_type == 53 ||
		    rfe_type == 54) {
			rf->fem.epa_6g = 1;
			rf->fem.elna_6g = 1;
		}
		break;
#endif
#ifdef RF_8192XB_SUPPORT
	case RF_RTL8192XB:
		if (rfe_type > 50 && rfe_type < 90)
			dpk->is_edpk = true;
		else
			dpk->is_edpk = false;
		break;
#endif

#ifdef RF_8852C_SUPPORT
	case RF_RTL8852C:
		if (rfe_type > 50 && rfe_type < 90)
			dpk->is_edpk = true;
		else
			dpk->is_edpk = false;
		break;
#endif
	default:
		break;
	}
}

void halrf_rfe_type_gpio_setting(struct rf_info *rf)
{
	u32 band = rf->hal_com->band[HW_PHY_0].cur_chandef.band;

	RF_DBG(rf, DBG_RF_INIT, "======>%s\n", __func__);

	halrf_set_gpio(rf, HW_PHY_0, (u8)band);
}

enum rtw_hal_status halrf_dm_init(void *rf_void)
{
	struct rf_info *rf = (struct rf_info *)rf_void;
	enum rtw_hal_status hal_status = RTW_HAL_STATUS_SUCCESS;

	if (!rf) {
		RF_DBG(rf, DBG_RF_INIT, "[%s] *rf = NULL", __func__);
		return RTW_HAL_STATUS_FAILURE;
	}

	/*Share Xtal*/
	halrf_set_share_xtal(rf);

	halrf_cmd_parser_init(rf);
	halrf_set_final_rfability(rf);
	halrf_rfe_init(rf);
	halrf_rfe_type_gpio_setting(rf);
	halrf_config_nctl_reg(rf);
	halrf_rfk_self_init(rf);

	/*Set Power table ref power*/
	halrf_set_ref_power_to_struct(rf, HW_PHY_0);
	/*Get BW160M power by rate by macid wa*/
	halrf_get_bw160m_wa(rf, HW_PHY_0);

	halrf_rck_trigger(rf, HW_PHY_0);
	//halrf_gapk_save_tx_gain_8852a(rf);
	halrf_dack_trigger(rf, false);
	halrf_rx_dck_trigger(rf, HW_PHY_0, true);
	/* LCTK */
	halrf_txa_lctk_trigger(rf, HW_PHY_0); 
	/*RX Gain K Get efuse*/
	/*halrf_get_efuse_rx_gain_k(rf, HW_PHY_0);*/

	/*Thermal Trim, PA Bias k, TSSI Trim get efuse and set reg*/
	halrf_get_efuse_trim(rf, HW_PHY_0);

	/*TSSI Init*/
	halrf_inn_tssi_get_efuse_ex(rf, HW_PHY_0);
	/*halrf_tssi_get_efuse_ex(rf, HW_PHY_1);*/

	/*Set MAC 0xd220[1]=0  r_txagc_BT_en=0 by Bryant*/
	if (phl_is_mp_mode(rf->phl_com)) {
		/*halrf_wl_tx_power_control(rf, 0xffffffff);*/
		halrf_inn_wlan_tx_power_control(rf, HW_PHY_0, ALL_TIME_CTRL,
			0x0, false);
		halrf_inn_wlan_tx_power_control(rf, HW_PHY_0, GNT_TIME_CTRL,
			0x0, false);
	}

	halrf_fcs_init(rf);

	return hal_status;
}

enum rtw_hal_status halrf_init(struct rtw_phl_com_t *phl_com,
			struct rtw_hal_com_t *hal_com, void **rf_out)
{
	struct rf_info *rf = NULL;

	rf = hal_mem_alloc(hal_com, sizeof(struct rf_info));

	if (!rf)
		return RTW_HAL_STATUS_RF_INIT_FAILURE;

	*rf_out = rf;

	rf->phl_com = phl_com;/*shared memory for all components*/
	rf->hal_com = hal_com;/*shared memory for phl and hal*/

	halrf_dbg_setting_init(rf);
	halrf_cmn_info_self_init(rf);
	
	/*init. HW cap*/
	halrf_set_rfability(rf);
	halrf_mutex_init(rf, &rf->rf_lock);
	return RTW_HAL_STATUS_SUCCESS;
}

void halrf_deinit(struct rtw_phl_com_t *phl_com,
			struct rtw_hal_com_t *hal_com, void *rf)
{
	struct rf_info *halrf = (struct rf_info *)rf;

	/*stop FSM of RF or free memory*/
	PHL_INFO("[PHL] %s - halrf(%p)\n", __func__, halrf);

	halrf_mutex_deinit(halrf, &halrf->rf_lock);

	if(halrf) {
		hal_mem_free(hal_com, halrf, sizeof(struct rf_info));
	}
}
