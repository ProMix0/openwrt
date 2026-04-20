/******************************************************************************
 *
 * Copyright(c) 2007 - 2022 Realtek Corporation.
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
#define _RTL_WFO_PRIV_C_

#include <drv_types.h>
#include "../phl/phl_headers.h"

rtl_wfo_priv_t g_wfo_priv;

void rtl_wfo_init(void)
{
	int i, j;
	extern uint rtw_wfo_band;

	/* AMSDU Ofld & FC */
	_rtw_spinlock_init(&g_wfo_priv.fc_lock);
	for (i = 0; i < FC_STA_NUM; i++) {
		g_wfo_priv.fc_sta[i].macid = 0xff;
		g_wfo_priv.fc_sta[i].band = 0xff;
	}
	rtl_wfo_flush_fc_staid();

	/* PE & DOE info */
	for (i = 0; i < MAX_PE_NUM; i++) {
		g_wfo_priv.pe_info[i].state = RTL_WFO_PE_INACTIVE;
		g_wfo_priv.pe_info[i].pe_role = 0;
		g_wfo_priv.pe_info[i].pe_priv_addr = 0;
		g_wfo_priv.pe_info[i].pe_priv = NULL;
		g_wfo_priv.pe_info[i].last_hb = 0;
	}

	/* device info */
	for (i = 0; i < BAND_MAX; i++) {
		g_wfo_priv.dev[i].dvobj = NULL;
		g_wfo_priv.dev[i].total_sta_num = 0;
		g_wfo_priv.dev[i].sw_seq_addr = 0;
		g_wfo_priv.dev[i].doe_sw_seq = NULL;
		for (j = 0; j < DOE_PI_RING_NUM; j++) {
			g_wfo_priv.dev[i].pi_addr[j].pi_ring_rw_ptr = 0xeeeeeeee;
			g_wfo_priv.dev[i].pi_addr[j].pi_ring_addr = 0xeeeeeeee;
		}
	}

	/* WFO supported band */
#ifdef CONFIG_WFO_NEW_ARCH
	g_wfo_priv.wfo_band_cap = rtw_wfo_band;
#else
	g_wfo_priv.wfo_band_cap = BAND_CAP_5G;
#endif
	g_wfo_priv.hb_started = 0;

	/* Buffer resource */
	for (i = 0; i < RTL_WFO_RES_NUM; i++) {
		_rtw_memset(&g_wfo_priv.resource[i], 0, sizeof(rtl_wfo_pe_resource_t));
		g_wfo_priv.res_pages[i] = NULL;
	}
	g_wfo_priv.res_state = PE_RES_NOT_ALLOC;
	g_wfo_priv.res_num = 0;
	g_wfo_priv.res_nirx_num = 0;
	g_wfo_priv.res_amsdu_num = 0;
	g_wfo_priv.next_nirx_th = 0;
	g_wfo_priv.next_amsdu_th = 0;

#ifdef FBM_USE_KMALLOC_BUF
	rtl_wfo_init_res();
#endif

	RTW_PRINT("RTL WFO inited (supported band 0x%x)\n", g_wfo_priv.wfo_band_cap);
}

void rtl_wfo_deinit(void)
{
	_rtw_spinlock_free(&g_wfo_priv.fc_lock);
	RTW_PRINT("RTL WFO deinited\n");
}

int rtl_wfo_supported_band(struct dvobj_priv *dvobj)
{
	return !!(g_wfo_priv.wfo_band_cap & GET_HAL_SPEC(dvobj)->band_cap);
}

void rtl_wfo_priv_init(struct dvobj_priv *dvobj)
{
	int rc;
	rtl_wfo_priv_t *wfo_priv = &g_wfo_priv;
	rtl_wfo_dev_t *wfo_dev = NULL;

	if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_2G)
		wfo_dev = &wfo_priv->dev[BAND_ON_24G];
	else if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_5G)
		wfo_dev = &wfo_priv->dev[BAND_ON_5G];
	else if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_6G)
		wfo_dev = &wfo_priv->dev[BAND_ON_5G];

	if (wfo_dev) {
		wfo_dev->dvobj = dvobj;
		dvobj->wfo_priv = wfo_priv;
		dvobj->wfo_dev = wfo_dev;

		if (wfo_priv->res_state == PE_RES_NOT_ALLOC) {
			rc = rtl_wfo_alloc_res(dvobj);
			if (rc < 0) {
				RTW_ERR("WFO resource allocate failed (%d)!\n", rc);
			} else {
				wfo_priv->res_state = PE_RES_ALLOC;
				RTW_INFO("RTL WFO resource allocated\n");
			}
		}
	}

	if (!g_wfo_priv.hb_started)
		rtw_init_timer(&(wfo_priv->hb_timer), rlt_wfo_hb_timer_hdl, dvobj);
	g_wfo_priv.hb_started++;

	RTW_INFO("RTL WFO priv inited\n");
}

void rtl_wfo_priv_deinit(struct dvobj_priv *dvobj)
{
	int i;
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;

	g_wfo_priv.hb_started--;
	if (!g_wfo_priv.hb_started)
		_cancel_timer_ex(&wfo_priv->hb_timer);

	if (wfo_priv->res_state > PE_RES_NOT_ALLOC) {
		for (i = 0; i < wfo_priv->res_num; i++) {
			if (wfo_priv->res_pages[i])
				__free_pages(wfo_priv->res_pages[i],
							wfo_priv->resource[i].res_order);
			else
				RTW_INFO("wfo pages[%d] not allocated!\n", i);
		}
		wfo_priv->res_state = PE_RES_NOT_ALLOC;
		wfo_priv->res_num = 0;
		wfo_priv->res_nirx_num = 0;
		wfo_priv->res_amsdu_num = 0;
		RTW_INFO("RTL WFO resource free\n");
	}

	dvobj->wfo_priv = NULL;
	RTW_INFO("RTL WFO priv de-inited\n");
}

u8 rtl_wfo_pe_id_mapping(struct sta_info *psta)
{
	return (RTL_WFO_PPE_ID + psta->phl_sta->macid % WFO_PPE_NUM);
}

void rtl_wfo_pe_id_decision(_adapter *padapter, struct sta_info *psta)
{
	u32 enc_alg = padapter->securitypriv.dot11PrivacyAlgrthm;

	if ((enc_alg == _NO_PRIVACY_) ||
			(MLME_IS_STA(padapter) && (enc_alg & _AES_)) ||
			(psta->wpa_pairwise_cipher & WPA_CIPHER_CCMP) ||
			(psta->wpa2_pairwise_cipher & WPA_CIPHER_CCMP) ||
			(psta->akm_suite_type & WLAN_AKM_TYPE_SAE)) {
		psta->rps_pe_id = psta->xps_pe_id =
			psta->ppe_pe_id = rtl_wfo_pe_id_mapping(psta);
	} else {
		psta->rps_pe_id = psta->xps_pe_id =
			psta->ppe_pe_id = RTL_WFO_ARM_ID;
	}

	if (padapter->registrypriv.wifi_mib.force_drv_rx)
		psta->rps_pe_id = RTL_WFO_ARM_ID;

	if (padapter->registrypriv.wifi_mib.force_drv_tx)
		psta->xps_pe_id = RTL_WFO_ARM_ID;
}

u8 rtl_wfo_set_offload(_adapter *adapter, u8 rx_en, u8 tx_en, u8 pe_state)
{
	void *phl = adapter->dvobj->phl;
	enum rtw_phl_status sts = RTW_PHL_STATUS_FAILURE;
	rtl_wfo_ipc_rsp_ring_addr_t ring_info;
	u8 ofld_en = 0;
	int result;
	u16 pe_wp_max_seq = (tx_en) ? (WP_RESERVED_SEQ - MAX_RSVD_SEQ - 1) : 0;
	/* [Drv] (WP_RESERVED_SEQ - MAX_RSVD_SEQ) ~ (WP_RESERVED_SEQ-1): #2817 ~ #4094 */
	/* [PE] #1 ~ (WP_RESERVED_SEQ - MAX_RSVD_SEQ - 1): #1 ~ #2816 */

	result = rtl_wfo_ipc_get_ring_addr(adapter, &ring_info);
	if (result) {
		RTW_ERR("ERROR: rtl_wfo_ipc_get_ring_addr fail(%d)\n", result);
		return ofld_en;
	}

	sts = rtw_phl_check_ring_info(phl,
			ring_info.rx_ring_rw_base,
			ring_info.rx_ring_data_base,
			ring_info.rx_ring_size,
			ring_info.rp_ring_rw_base,
			ring_info.rp_ring_data_base,
			ring_info.rp_ring_size,
			ring_info.tx_ring_rw_base,
			ring_info.tx_ring_data_base,
			ring_info.tx_ring_size);
	if (sts != RTW_PHL_STATUS_SUCCESS)
		return ofld_en;

	if (rx_en) {
		extern void phl_rx_stop_pcie(struct phl_info_t *phl_info);
		phl_rx_stop_pcie(phl);
		rtw_phl_set_offload(adapter->dvobj->phl, RTL_WFO_RX_OFLD, rx_en);
		ofld_en |= RTL_WFO_RX_OFLD;
#ifdef WFO_DYNAMIC_RXBUF
		rtw_phl_release_rx_buffer_pre(phl);
#endif
	}

	if (tx_en) {
		rtw_phl_set_offload(adapter->dvobj->phl, RTL_WFO_TX_OFLD, tx_en);
		ofld_en |= (RTL_WFO_TX_OFLD | RTL_WFO_TX_FULL_OFLD);
	}

	if (!adapter->dvobj->wfo_priv->wfo_band_on) {
		/* Set AMSDU Ofld mode to FC */
		rtl_wfo_set_amsdu_ofld_mode(AMSDU_OFLD_40STA_BE);
	}

	/* Send IPC to DOE & PE to enter PE_START_WFO state */
	result = rtl_wfo_ipc_start_wfo(adapter, ofld_en, pe_wp_max_seq, pe_state);
	if (result) {
		RTW_ERR("ERROR: rtl_wfo_ipc_start_wfo fail (%d)!!!\n", result);
		ofld_en = 0;
	}

	if (rx_en) {
#ifdef WFO_DYNAMIC_RXBUF
		/* free busy rx buffer after rtl_wfo_ipc_start_wfo */
		rtw_phl_release_rx_buffer_post(phl);
#endif
	}

	return ofld_en;
}

void rtl_wfo_start_pe(_adapter *padapter)
{
	u8 rx_en, tx_en, ofld_en;
	int i, active_pe_num = 0;

	if (padapter->dvobj->wfo_en) {
		RTW_WARN("WFO already started\n");
		return;
	}

	if (!rtl_wfo_supported_band(adapter_to_dvobj(padapter)))
		return;

	for (i = 0; i < MAX_PE_NUM; i++) {
		if (g_wfo_priv.pe_info[i].state == RTL_WFO_PE_ACTIVE)
			active_pe_num++;
		else
			RTW_ERR("WFO PE%d not active (%d)!!!\n", i, g_wfo_priv.pe_info[i].state);
	}
	if (active_pe_num != MAX_PE_NUM) {
		RTW_ERR("RTL WFO not started!!! active PE num: %d/%d\n", active_pe_num, MAX_PE_NUM);
		goto wfo_fail;
	}

	/* set TRX offload mode */
	tx_en = 2; /* 1: BE-only, 2: all ACQ */
	rx_en = 1;

	/* TX Offload disabled once TX Fragment enabled */
	if (padapter->xmitpriv.frag_len < MAX_FRAG_THRESHOLD)
		tx_en = 0;

#ifdef FBM_USE_KMALLOC_BUF
	rtl_wfo_start_pe_kmem_res(padapter->dvobj);
#endif /* FBM_USE_KMALLOC_BUF */

	RTW_PRINT("[WFO]............................................STARTING\n");

	ofld_en = rtl_wfo_set_offload(padapter, rx_en, tx_en, PE_START_WFO);
	padapter->dvobj->wfo_en = ofld_en;
	if (!ofld_en) {
		RTW_ERR("[%s] WFO enable failed\n", __func__);
		rtl_wfo_notify_event(RTL_WFO_EVENT_DUMP_PE_LOG);
		rtl_wfo_restart_drv_trx(padapter->dvobj);
	} else {
		rtw_core_set_gt3(padapter, 1, 1000); /* unit: us */
		RTW_PRINT("__      __ ____  ___     ___   __   __\n");
		RTW_PRINT("\\ \\    / /| ___|/   \\   /   \\ |   \\ | |\n");
		RTW_PRINT(" \\ \\/\\/ / | __||  O  | |  O  ||  |\\\\| |\n");
		RTW_PRINT("  \\_/\\_/  |_|   \\___/   \\___/ |__| \\__|\n");
		RTW_PRINT("\n");
		RTW_PRINT("[WFO]............................................DONE\n");

		return;
	}

wfo_fail:
	RTW_ERR("__      __ ____  ___     ___   ____  ____\n");
	RTW_ERR("\\ \\    / /| ___|/   \\   /   \\ | ___|| ___|\n");
	RTW_ERR(" \\ \\/\\/ / | __||  O  | |  O  || __| | __|\n");
	RTW_ERR("  \\_/\\_/  |_|   \\___/   \\___/ |_|   |_|\n");
	RTW_ERR("\n");
	RTW_ERR("[WFO]............................................FAIL\n");
}

void rtl_wfo_stop_pe(_adapter *padapter)
{
	if (!padapter->dvobj->wfo_en) {
		RTW_WARN("WFO already stopped\n");
		return;
	}

	if (!rtl_wfo_supported_band(adapter_to_dvobj(padapter)))
		return;

	rtl_wfo_ipc_stop_wfo(padapter, PE_SUSPEND_WFO);
	rtw_phl_set_offload(padapter->dvobj->phl,
		RTL_WFO_RX_OFLD | RTL_WFO_TX_OFLD, 0);
	padapter->dvobj->wfo_en = 0;
#ifdef WFO_DYNAMIC_RXBUF
	rtw_phl_refill_rx_buffer(padapter->dvobj->phl);
#endif
}

void rtl_wfo_restart_drv_trx(struct dvobj_priv *dvobj)
{
	rtl_wfo_flush_fc_staid();
	dvobj->wfo_not_start_pe = 1;
#ifdef WFO_DYNAMIC_RXBUF
	rtw_phl_refill_rx_buffer(dvobj->phl);
#endif
	phl_wfo_drv_trx_rcvy(dvobj->phl);
}

int rtl_wfo_alloc_res(struct dvobj_priv *dvobj)
{
	int i;
	_adapter *adapter = dvobj_get_primary_adapter(dvobj);
	struct wifi_mib_priv *mib = &adapter->registrypriv.wifi_mib;
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;
	PPCI_DATA pci_data = dvobj_to_pci(dvobj);
	struct pci_dev *pdev = pci_data->ppcidev;
	u32 phy_addr_h = 0;
	dma_addr_t bus_addr = 0;
	/* note: if modify resource array, remember to sync the change to RTL_WFO_RES_NUM. */
	rtl_wfo_pe_resource_t resource[] = {
		/* MEM Pool, 2M (AOE) */
		{1, RTL_WFO_AOE_ID, 9, RTL_WFO_RESOURCE_MEMPOOL, 0, 0, 0, 0},
#ifdef CONFIG_WFO_NEW_ARCH
		/* MEM Pool, 256K (DOE) */
		{1, RTL_WFO_DOE_ID, 6, RTL_WFO_RESOURCE_MEMPOOL, 0, 0, 0, 0},
#endif
		/* RXQ, 4M + 4M + 2M, 12288B x (320 + 320 + 128) - DOE */
		{1, RTL_WFO_DOE_ID, 10, RTL_WFO_RESOURCE_RX_BUF, 0, 0, 12288, 340},
		{0, RTL_WFO_DOE_ID, 10, RTL_WFO_RESOURCE_RX_BUF, 0, 0, 12288, 340},
		{0, RTL_WFO_DOE_ID, 9, RTL_WFO_RESOURCE_RX_BUF, 0, 0, 12288, 170},
		/* RPQ, 256K, (512B + 128B + 384B) x 256 - DOE */
		{1, RTL_WFO_DOE_ID, 6, RTL_WFO_RESOURCE_RP_BUF, 0, 0, 1024, 256},
#ifdef CONFIG_WFO_NEW_ARCH
		/* REORDER, 2M, (8B x 1024 AMPDU_SZ + 64B) x (4 TID x 63 STA) - PPE */
		{1, RTL_WFO_PPE_ID, 9, RTL_WFO_RESOURCE_REORDER_BUF, 0, 0, 8256, 252},
#else
		/* REORDER, 1M, (8B x 256 AMPDU_SZ + 64B) x (8 TID x 64 STA) - PPE0 */
		{1, RTL_WFO_PPE_ID, 8, RTL_WFO_RESOURCE_REORDER_BUF, 0, 0, 2112, 256},
#if (WFO_PPE_NUM > 1)
		/* REORDER, 1M, (8B x 256 AMPDU_SZ + 64B) x (8 TID x 64 STA) - PPE1 */
		{1, (RTL_WFO_PPE_ID + 1), 8, RTL_WFO_RESOURCE_REORDER_BUF, 0, 0, 2112, 256},
#endif
#endif
		/* ACH0-3, 256K + 8K, (64B + 128B + 64B(aligned)) x (1024 + 32) - DOE */
		{1, RTL_WFO_DOE_ID, 6, RTL_WFO_RESOURCE_TX_BUF, 0, 0, 256, 1024},
		{0, RTL_WFO_DOE_ID, 1, RTL_WFO_RESOURCE_TX_BUF, 0, 0, 256, 32},
#ifdef FBM_USE_KMALLOC_BUF
		/* no need to pre-allocate NIRX_BUF and AMSDU_NIRX_BUF */
#else
		/* NIRX - DOE */
#if PE_NIRX_BUF_SZ == 4096
		/* 4M, 4096B x 1024 (WPSEQ-11K: 2816 x 3) */
		{1, RTL_WFO_DOE_ID, 10, RTL_WFO_RESOURCE_NIRX_BUF, 0, 0, PE_NIRX_BUF_SZ, PE_NIRX_BUF_NUM},
#elif PE_NIRX_BUF_SZ == 8192
		/* 4M, 8192B x 512 (WPSEQ-11K: 2816 x 2) */
		{1, RTL_WFO_DOE_ID, 10, RTL_WFO_RESOURCE_NIRX_BUF, 0, 0, PE_NIRX_BUF_SZ, PE_NIRX_BUF_NUM},
#endif /* end - PE_NIRX_BUF_SZ */
		/* AMSDU_NIRX - AOE */
#if AMSDU_NIRX_BUF_SZ == 128 || AMSDU_NIRX_BUF_SZ == 256
		/* 4M, 128B x 32768 or 256B x 16384 */
		{1, RTL_WFO_AOE_ID, 10, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 0, 0, AMSDU_NIRX_BUF_SZ, AMSDU_NIRX_BUF_NUM}
#elif AMSDU_NIRX_BUF_SZ == 512
		/* 4M, 512B x 8192 */
		{1, RTL_WFO_AOE_ID, 10, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 0, 0, AMSDU_NIRX_BUF_SZ, AMSDU_NIRX_BUF_NUM},
#elif AMSDU_NIRX_BUF_SZ == 1024 || AMSDU_NIRX_BUF_SZ == 2048
		/* 4M, 1024B x 4096 or 2048B x 2048 */
		{1, RTL_WFO_AOE_ID, 10, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 0, 0, AMSDU_NIRX_BUF_SZ, AMSDU_NIRX_BUF_NUM},
#endif /* end - AMSDU_NIRX_BUF_SZ */
#endif /* end - FBM_USE_KMALLOC_BUF */
	};

	for (i = 0; i < ARRAY_SIZE(resource); i++) {
		wfo_priv->res_pages[i] = alloc_pages(GFP_KERNEL | __GFP_ZERO,
											resource[i].res_order);
		if (!wfo_priv->res_pages[i])
			break;
		resource[i].res_addr = __pa(page_address(wfo_priv->res_pages[i]));
		resource[i].res_size = (1 << (resource[i].res_order + PAGE_SHIFT));
		// WB zero memory
		pci_get_bus_addr(pdev, __va(resource[i].res_addr), &bus_addr,
							resource[i].res_size, DMA_TO_DEVICE);
		if (!dma_mapping_error(&(pdev->dev), bus_addr)) {
			_os_cache_wback(dvobj, &resource[i].res_addr, &phy_addr_h,
							resource[i].res_size, DMA_TO_DEVICE);
		} else {
			RTW_ERR("dma_mapping_error 0x%llx\n", bus_addr);
			break;
		}
	}

	if (i < ARRAY_SIZE(resource)) {
		for (i = 0; i < ARRAY_SIZE(resource); i++) {
			if (wfo_priv->res_pages[i]) {
				__free_pages(wfo_priv->res_pages[i], resource[i].res_order);
				wfo_priv->res_pages[i] = NULL;
			}else {
				RTW_ERR("alloc pages[%d] failed!\n", i);
			}
		}
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(resource); i++) {
		_rtw_memcpy(&wfo_priv->resource[i], &resource[i],
					sizeof(rtl_wfo_pe_resource_t));
		if (resource[i].res_type == RTL_WFO_RESOURCE_NIRX_BUF)
			wfo_priv->res_nirx_num++;
		else if (resource[i].res_type == RTL_WFO_RESOURCE_AMSDU_NIRX_BUF)
			wfo_priv->res_amsdu_num++;
	}
	wfo_priv->res_num = i;
	wfo_priv->next_nirx_th = 1;
	wfo_priv->next_amsdu_th = 1;

	return 0;
}

#ifdef FBM_USE_KMALLOC_BUF
#define MBYTES(X) ((X)*1024*1024)
/*
			 NIRX	 AMSDU
	idle:	 4M(1)	 4M(1)
	1sta:	10M(3)	 5M(2)
	2sta:	16M(5)	 5M(2)
	10sta:	24M(7)	 8M(4)

	PON_VW: 26M(7)	20M(5)
*/
rtl_wfo_res_tbl_t res_tbl[RESOURCE_MAX] = {
	/* RESOURCE_NIRX */
	{
		.buf_size = PE_NIRX_BUF_SZ, .buf_num = 0,
		.stage = {
			{ .sta_num =  0, .buf_num = (MBYTES( 4)/PE_NIRX_BUF_SZ) },
			{ .sta_num =  1, .buf_num = (MBYTES(10)/PE_NIRX_BUF_SZ) },
			{ .sta_num =  2, .buf_num = (MBYTES(16)/PE_NIRX_BUF_SZ) },
			{ .sta_num = 10, .buf_num = (MBYTES(24)/PE_NIRX_BUF_SZ) },

			/* PON_VW */
			{ .sta_num = PON_VW_STA, .buf_num = (MBYTES(26)/PE_NIRX_BUF_SZ) },
		},
	},

	/* RESOURCE_AMSDU_NIRX */
	{
		.buf_size = AMSDU_NIRX_BUF_SZ, .buf_num = 0,
		.stage = {
			{ .sta_num =  0, .buf_num = (MBYTES(4)/AMSDU_NIRX_BUF_SZ) },
			{ .sta_num =  1, .buf_num = (MBYTES(5)/AMSDU_NIRX_BUF_SZ) },
			{ .sta_num =  2, .buf_num = (MBYTES(5)/AMSDU_NIRX_BUF_SZ) },
			{ .sta_num = 10, .buf_num = (MBYTES(8)/AMSDU_NIRX_BUF_SZ) },

			/* PON_VW */
			{ .sta_num = PON_VW_STA, .buf_num = (MBYTES(20)/AMSDU_NIRX_BUF_SZ) },
		},
	},
};

void rtl_wfo_init_res(void)
{
	int i, k, buf_num;
	rtl_wfo_res_tbl_t *tbl;

	g_wfo_priv.res_tbl = &res_tbl;
	for (i = 0; i < RESOURCE_MAX; i++) {
		tbl = &res_tbl[i];
		for (tbl->rec_max = 0, k = 0; k < RES_STAGE_MAX; k++) {
			buf_num = ((k == 0) ? tbl->stage[k].buf_num : \
				(tbl->stage[k].buf_num - tbl->stage[k-1].buf_num));
			tbl->stage[k].buf_info_sz = buf_num * sizeof(u32);
			tbl->stage[k].buf_info =
				rtw_zmalloc(tbl->stage[k].buf_info_sz);
			tbl->rec_max =
				((tbl->stage[k].buf_num > tbl->rec_max) ? \
				tbl->stage[k].buf_num : tbl->rec_max);
		}
		tbl->rec_addr =
			rtw_zmalloc(tbl->rec_max * sizeof(u32));
		tbl->free_info =
			rtw_zmalloc(tbl->rec_max * sizeof(u32));

		RTW_DBG("%s: res_type %d rec_max %u buf_num %u\n",
			__func__, i, tbl->rec_max, tbl->buf_num);
	}
}

void rtl_wfo_start_pe_kmem_res(struct dvobj_priv *dvobj)
{
	rtl_wfo_res_t *stage = NULL;
	int i;
	for (i = 0; i < RESOURCE_MAX; i++) {
		stage = rtl_wfo_check_res_stage(i, RES_STAGE_0_NOSTA);
		if (stage) {
			rtl_wfo_add_res(dvobj, i, stage);
		}
	}
}

inline void rtl_wfo_res_wback(struct dvobj_priv *dvobj,
	u32 phy_addr_l, int buf_size)
{
	u32 phy_addr_h = 0;
	_os_cache_wback(dvobj,
		&phy_addr_l, &phy_addr_h, buf_size, DMA_TO_DEVICE);
}

inline void rtl_wfo_res_inv(struct dvobj_priv *dvobj,
	u32 phy_addr_l, int buf_size)
{
	u32 phy_addr_h = 0;
	_os_cache_inv(dvobj,
		&phy_addr_l, &phy_addr_h, buf_size, DMA_FROMDEVICE);
}


int rtl_wfo_add_res(struct dvobj_priv *dvobj,
	u16 res_type, rtl_wfo_res_t *stage)
{
	u32 i, phy_addr, rec, *buf_info = stage->buf_info;
	u32 buf_num, buf_cur, alloc_num = 0;
	rtl_wfo_res_tbl_t *tbl = NULL;
	rtl_wfo_pe_resource_t res;
	void *p;

	if (res_type != RESOURCE_NIRX &&
		res_type != RESOURCE_AMSDU_NIRX) {
		RTW_ERR("res_type %d is not supported!\n", res_type);
		return 0;
	}

	tbl = &res_tbl[res_type];
	buf_cur = tbl->buf_num;
	memset(stage->buf_info, 0, stage->buf_info_sz);

	if (stage->sta_num == ADD_BUFINFO) {
		buf_num = stage->buf_num;
	} else {
		buf_num =
			(tbl->buf_num < stage->buf_num) ? \
			(stage->buf_num - tbl->buf_num) : 0;
	}

	for (i = 0; i < buf_num; i++) {
		p = rtw_zmalloc(tbl->buf_size);
		if (p) {
			if ((u64)p % (tbl->buf_size)) {
				RTW_ERR("NOT %d alignment!\n", tbl->buf_size);
				rtw_mfree(p, tbl->buf_size);
				break;
			}
		} else {
			RTW_ERR("failed to allocate page\n");
			break;
		}

		phy_addr = (u32)virt_to_phys(p);
		for (rec = 0; rec < tbl->rec_max; rec++) {
			if (tbl->rec_addr[rec] == 0) {
				tbl->rec_addr[rec] = phy_addr;
				rtl_wfo_res_wback(dvobj, phy_addr, tbl->buf_size);
				*(buf_info + i) = phy_addr;
				alloc_num++;
				RTW_DBG("%s: i %u addr 0x%08x\n",
					__func__, i, (u32)virt_to_phys(p));
				break;
			}
		}
		if (rec == tbl->rec_max) {
			RTW_DBG("%s: no available rec_addr!\n", __func__);
		}
	}

	if (alloc_num) {
		tbl->buf_num += alloc_num;

		res.force_init = (stage->sta_num == 0) ? 1 : 0;
		res.res_pe_id = RTL_WFO_DOE_ID;
		res.res_type = (res_type == RESOURCE_NIRX) ? \
			RTL_WFO_RESOURCE_NIRX_BUF : RTL_WFO_RESOURCE_AMSDU_NIRX_BUF;
		res.res_addr = (u32)virt_to_phys(buf_info);
		res.res_size = tbl->buf_size*alloc_num;
		res.buf_size = tbl->buf_size;
		res.buf_num = alloc_num;

		rtl_wfo_res_wback(dvobj, res.res_addr, stage->buf_info_sz);
		rtl_wfo_ipc_set_resource(dvobj_get_primary_adapter(dvobj),
			res.res_pe_id, &res);
	}

	RTW_PRINT("RES_ADD: res_type %d add %u buf %u->%u "
		"init %d stage{ sta %u buf %u}, %s\n",
		res_type, alloc_num, buf_cur, tbl->buf_num,
		res.force_init, stage->sta_num, stage->buf_num,
		(alloc_num == buf_num)?"done.":"fail!");

	return buf_num;
}

u32 rtl_wfo_free_res(struct dvobj_priv *dvobj, u16 res_type, u32 buf_num)
{
	int i, rec;
	rtl_wfo_res_tbl_t *tbl = &res_tbl[res_type];
	u32 *buf_info = tbl->free_info;
	u32 phy_addr, buf_cur = tbl->buf_num;
	void *p;

	if (res_type != RESOURCE_NIRX &&
		res_type != RESOURCE_AMSDU_NIRX) {
		RTW_ERR("res_type %d is not supported!\n", res_type);
		return 0;
	}

	phy_addr = (u32)virt_to_phys(buf_info);
	rtl_wfo_res_inv(dvobj, phy_addr, tbl->rec_max * sizeof(u32));

	for (i = 0; i < buf_num; i++) {
		phy_addr = (u32)(*(buf_info+i));
		if (phy_addr) {
			p = phys_to_virt((phys_addr_t)phy_addr);
			for (rec = 0; rec < tbl->rec_max; rec++) {
				if (tbl->rec_addr[rec] == phy_addr) {
					rtw_mfree(p, tbl->buf_size);
					tbl->buf_num--;
					tbl->rec_addr[rec] = 0;
					break;
				}
			}
			if (rec == tbl->rec_max) {
				RTW_PRINT("RES_FREE: NOT FOUND!! res_type %d  addr 0x%08x\n",
					res_type, phy_addr);
			}
		} else {
			RTW_PRINT("RES_FREE: NULL VALUE!! res_type %d idx %d/%u\n",
				phy_addr, i, buf_num);
		}
	}

	RTW_PRINT("RES_FREE: res_type %d free %u buf %u->%u, %s\n",
		res_type, buf_num, buf_cur, tbl->buf_num,
		(buf_cur-buf_num == tbl->buf_num)?"done.":"fail!!");

	return buf_cur-tbl->buf_num;
}

rtl_wfo_res_t *rtl_wfo_check_res_stage(int type, int sta_num)
{
	rtl_wfo_res_tbl_t *tbl = &res_tbl[type];
	rtl_wfo_res_t *stage;
	u32 i, idx = 0;

	for (i = 0; i < RES_STAGE_MAX; i++) {
		if (tbl->stage[i].sta_num > sta_num)
			break;
		idx = i;
	}
	stage = &tbl->stage[idx];
	return stage;
}

int __rtl_wfo_check_res(_adapter *adapter, struct dvobj_priv *dvobj)
{
	struct wifi_mib_priv *mib = &adapter->registrypriv.wifi_mib;
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;
	rtl_wfo_res_t *stage;
	int i, total_sta_num = 0, vw_mode = 0;

	for (i = 0; i < BAND_MAX; i++)
		total_sta_num += wfo_priv->dev[i].total_sta_num;

	if (total_sta_num) {
#if defined(CONFIG_VW_REFINE) && defined(CONFIG_PON_VW_FULL_MEM)
		if (adapter->vw_enable == VW_MODE_TP) {
			vw_mode = 1;
		}
#endif /* CONFIG_VW_REFINE & CONFIG_PON_VW_FULL_MEM */

		for (i = 0; i < RESOURCE_MAX; i++) {
			stage = rtl_wfo_check_res_stage(i,
						vw_mode ? PON_VW_STA : total_sta_num);
			if (stage) {
				rtl_wfo_add_res(dvobj, i, stage);
			}
		}
	}

	return 0;
}
#else /* !FBM_USE_KMALLOC_BUF */
int rtl_wfo_add_res(struct dvobj_priv *dvobj, u16 res_type, int order)
{
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;
	PPCI_DATA pci_data = dvobj_to_pci(dvobj);
	struct pci_dev *pdev = pci_data->ppcidev;
	u32 phy_addr_h = 0;
	dma_addr_t bus_addr = 0;
	rtl_wfo_pe_resource_t resource = {0};
	struct page *res_pages;
	int res_idx = wfo_priv->res_num;

	if ((res_type != RTL_WFO_RESOURCE_NIRX_BUF) &&
			(res_type != RTL_WFO_RESOURCE_AMSDU_NIRX_BUF)) {
		RTW_ERR("not support type(0x%08x)! NIRX and AMSDU_NIRX only.\n",
				res_type);
		return -EINVAL;
	}

	if ((res_type == RTL_WFO_RESOURCE_NIRX_BUF) &&
			(wfo_priv->res_nirx_num >= PE_NIRX_RES_NUM)) {
		RTW_ERR("exceed max number of NIRX resource (%d)\n",
				PE_NIRX_RES_NUM);
		return -ENOSPC;
	} else if ((res_type == RTL_WFO_RESOURCE_AMSDU_NIRX_BUF) &&
			(wfo_priv->res_amsdu_num >= AMSDU_NIRX_RES_NUM)) {
		RTW_ERR("exceed max number of AMSDU resource (%d)\n",
				AMSDU_NIRX_RES_NUM);
		return -ENOSPC;
	}

	res_pages = alloc_pages(GFP_ATOMIC | __GFP_ZERO, order);
	if (!res_pages) {
		RTW_ERR("failed to allocate page (order %d)\n", order);
		return -ENOMEM;
	}

	resource.res_addr = __pa(page_address(res_pages));
	resource.res_size = (1 << (order + PAGE_SHIFT));
	// WB zero memory
	pci_get_bus_addr(pdev, __va(resource.res_addr),
		&bus_addr, resource.res_size, DMA_TO_DEVICE);
	if (!dma_mapping_error(&(pdev->dev), bus_addr)) {
		_os_cache_wback(dvobj,
			&resource.res_addr, &phy_addr_h,
			resource.res_size, DMA_TO_DEVICE);
	} else {
		RTW_ERR("dma_mapping_error 0x%llx\n", bus_addr);
		__free_pages(res_pages, order);
		return -1;
	}

	resource.res_order = order;
	resource.res_type = res_type;
	if (res_type == RTL_WFO_RESOURCE_NIRX_BUF) {
		resource.res_pe_id = RTL_WFO_DOE_ID;
		resource.buf_size = PE_NIRX_BUF_SZ;
		wfo_priv->res_nirx_num++;
		if (wfo_priv->res_nirx_num == PE_NIRX_RES_NUM)
			wfo_priv->next_nirx_th = NUM_STA_TOTAL_LIMIT + 1;
	} else if (res_type == RTL_WFO_RESOURCE_AMSDU_NIRX_BUF) {
		resource.res_pe_id = RTL_WFO_AOE_ID;
		resource.buf_size = AMSDU_NIRX_BUF_SZ;
		wfo_priv->res_amsdu_num++;
		if (wfo_priv->res_amsdu_num == AMSDU_NIRX_RES_NUM)
			wfo_priv->next_amsdu_th = NUM_STA_TOTAL_LIMIT + 1;
	}
	resource.buf_num = resource.res_size / resource.buf_size;
	rtl_wfo_ipc_set_resource(dvobj_get_primary_adapter(dvobj),
			resource.res_pe_id, &resource);

	_rtw_memcpy(&wfo_priv->resource[res_idx], &resource, sizeof(resource));
	wfo_priv->res_num++;

	return 0;
}

int __rtl_wfo_check_res(_adapter *adapter, struct dvobj_priv *dvobj)
{
	struct wifi_mib_priv *mib = &adapter->registrypriv.wifi_mib;
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;
	int i, total_sta_num = 0;
	int vw_mode = 0;

#if defined(CONFIG_VW_REFINE) && defined(CONFIG_PON_VW_FULL_MEM)
	if (adapter->vw_enable == VW_MODE_TP) {
		vw_mode = 1;
	}
#endif /* CONFIG_VW_REFINE & CONFIG_PON_VW_FULL_MEM */

	for (i = 0; i < BAND_MAX; i++)
		total_sta_num += wfo_priv->dev[i].total_sta_num;

	if (total_sta_num >= wfo_priv->next_nirx_th) {
		/*
			PAGE_SIZE			4096
			PE_NIRX_BUF_SZ		4096

				 MEM_SZ	  SEG_NUM
			idle:	 4M			1
			1sta:	10M			3
			2sta:	16M			5
			10sta:	24M			7
			PON_VW: 26M			7
		*/

		if (vw_mode == 1) {
			if (total_sta_num == 1) {
				wfo_priv->next_nirx_th += 1;
				for (i = 1; i < PE_NIRX_RES_NUM; i++) {
					if (i == (PE_NIRX_RES_NUM - 1))
						rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 9);
					else
						rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 10);
				}
			}
		} else {
			if (total_sta_num == 1) {
				wfo_priv->next_nirx_th += 1;
				rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 10);
				rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 9);
			} else {
				wfo_priv->next_nirx_th += mib->nirx_rsc_step;
				if (total_sta_num == 2) {
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 10);
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 9);
				} else if (total_sta_num == 10) {
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 10);
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_NIRX_BUF, 10);
				}
			}
		}
	}

	if (total_sta_num >= wfo_priv->next_amsdu_th) {
		/*
			PAGE_SIZE			4096
			AMSDU_NIRX_BUF_SZ	1024

				  MEM_SZ   SEG_NUM
			idle:	 4M			1
			1sta:	 5M			2
			2sta:	 5M			2
			10sta:	 8M			4
			PON_VW: 20M			5
		*/

		if (vw_mode == 1) {
			if (total_sta_num == 1) {
				wfo_priv->next_amsdu_th += 1;
				for (i = 1; i < AMSDU_NIRX_RES_NUM; i++) {
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 10);
				}
			}
		} else {
			if (total_sta_num == 1) {
				wfo_priv->next_amsdu_th += 1;
				rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 8);
			} else {
				wfo_priv->next_amsdu_th += mib->amsdu_rsc_step;
				if (total_sta_num == 2) {
					/* already 5M */
				} else if (total_sta_num == 10) {
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 9);
					rtl_wfo_add_res(dvobj, RTL_WFO_RESOURCE_AMSDU_NIRX_BUF, 8);
				}
			}
		}
	}

	return 0;
}
#endif /* FBM_USE_KMALLOC_BUF */

int rtl_wfo_check_res(_adapter *adapter)
{
	struct dvobj_priv *dvobj = adapter->dvobj;

	if (is_supported_5g(adapter->registrypriv.band_type)) {
		dvobj->wfo_priv->dev[BAND_ON_5G].total_sta_num =
				get_dvobj_total_sta_num(dvobj);
	} else if (is_supported_24g(adapter->registrypriv.band_type)) {
		dvobj->wfo_priv->dev[BAND_ON_24G].total_sta_num =
				get_dvobj_total_sta_num(dvobj);
	}

	return __rtl_wfo_check_res(adapter, dvobj);
}

