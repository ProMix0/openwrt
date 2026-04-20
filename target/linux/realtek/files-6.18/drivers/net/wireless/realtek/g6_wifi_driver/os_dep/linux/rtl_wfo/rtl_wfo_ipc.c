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
#define _RTL_WFO_IPC_C_

#include <drv_types.h>
#include <ca_types.h>
#include "../phl/phl_headers.h"
#include <ca_soft_ipc_pri.h>

extern rtl_wfo_priv_t g_wfo_priv;
static u8 ipc_cmd_seq = 0;

static u8 rtl_wfo_get_ipc_id(u8 pe_id)
{
	if (pe_id == RTL_WFO_PE0_ID)
		return RTL_WFO_IPC_CPU_PE0;
	else if (pe_id == RTL_WFO_PE1_ID)
		return RTL_WFO_IPC_CPU_PE1;
	else if (pe_id == RTL_WFO_PE2_ID)
		return RTL_WFO_IPC_CPU_PE2;
	else {
		RTW_ERR("%s: invalid pe id %d\n", __func__, pe_id);
		return RTL_WFO_IPC_CPU_MAX;
	}
}

static u8 rtl_wfo_get_pe_id(u8 ipc_id)
{
	if (ipc_id == RTL_WFO_IPC_CPU_PE0)
		return RTL_WFO_PE0_ID;
	else if (ipc_id == RTL_WFO_IPC_CPU_PE1)
		return RTL_WFO_PE1_ID;
	else if (ipc_id == RTL_WFO_IPC_CPU_PE2)
		return RTL_WFO_PE2_ID;
	else {
		RTW_ERR("%s: invalid ipc id %d\n", __func__, ipc_id);
		return RTL_WFO_PEID_MAX;
	}
}

static u8 rtl_wfo_session_to_band(ca_ipc_session_id_t session_id)
{
	if (session_id == CA_IPC_SESSION_RTL_WFO_2G)
		return BAND_ON_24G;
	else if (session_id == CA_IPC_SESSION_RTL_WFO_5G)
		return BAND_ON_5G;
	else if (session_id == CA_IPC_SESSION_RTL_WFO_6G)
		return BAND_ON_6G;
	else
		return BAND_MAX;
}

static ca_ipc_session_id_t _rtl_wfo_dvobj_to_session(struct dvobj_priv *dvobj)
{
	if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_2G)
		return CA_IPC_SESSION_RTL_WFO_2G;
	else if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_5G)
		return CA_IPC_SESSION_RTL_WFO_5G;
	else if (GET_HAL_SPEC(dvobj)->band_cap == BAND_CAP_6G)
		return CA_IPC_SESSION_RTL_WFO_6G;
	else
		RTW_ERR("IPC session for band cap (%d) is not assigned\n",
				GET_HAL_SPEC(dvobj)->band_cap);

	return CA_IPC_SESSION_MAX;
}

static ca_ipc_session_id_t rtl_wfo_dvobj_to_session(struct dvobj_priv *dvobj)
{
	if (rtl_wfo_supported_band(dvobj))
		return _rtl_wfo_dvobj_to_session(dvobj);
	else
		RTW_ERR("Not supported band cap (%d)\n", GET_HAL_SPEC(dvobj)->band_cap);

	return CA_IPC_SESSION_MAX;
}

static int rtl_wfo_get_ipc_session(_adapter *padapter, u8 ipc_pe_id)
{
	struct dvobj_priv *dvobj;
	int session_id;

	if (ipc_pe_id < CA_IPC_CPU_PE0 || ipc_pe_id >= CA_IPC_CPU_MAX) {
		RTW_ERR("%s: invalid IPC cpu id (%d)\n", __func__, ipc_pe_id);
		return 0;
	}

	dvobj = adapter_to_dvobj(padapter);
	session_id = rtl_wfo_dvobj_to_session(dvobj);
	if (session_id < CA_IPC_SESSION_MAX)
		return session_id;
	else
		return 0;
}

void rtl_wfo_get_ipc_msg_size(rtl_wfo_ipc_msg_sz_t *sz)
{
	/* PE & DOE command */
	sz->cmd_start_size = sizeof(rtl_wfo_ipc_cmd_start_t);
	sz->cmd_set_pe_addr_size = sizeof(rtl_wfo_ipc_cmd_set_pe_addr_t);
	sz->resource_size = sizeof(rtl_wfo_ipc_resource_t);
	sz->cmd_device_info_size = sizeof(rtl_wfo_ipc_cmd_device_info_t);
	sz->cmd_adapter_info_size = sizeof(rtl_wfo_ipc_cmd_adapter_info_t);
	sz->cmd_sta_info_size = sizeof(rtl_wfo_ipc_cmd_sta_info_t);
	sz->cmd_txsc_info_size = sizeof(rtl_wfo_ipc_cmd_txsc_info_t);
	sz->cmd_rxofld_info_size = sizeof(rtl_wfo_ipc_cmd_rxofld_info_t);
	/* PE & DOE to Driver command */
	sz->cmd_update_a4_info_size = sizeof(rtl_wfo_ipc_cmd_update_a4_info_t);
	/* Response */
	sz->rsp_pe_info_size = sizeof(rtl_wfo_ipc_rsp_pe_info_t);
	sz->rsp_ring_addr_size = sizeof(rtl_wfo_ipc_rsp_ring_addr_t);
	sz->rsp_mib_size = sizeof(rtl_wfo_ipc_rsp_mib_t);
}

int rtl_wfo_cmp_ipc_msg_size(rtl_wfo_ipc_msg_sz_t *drv, rtl_wfo_ipc_msg_sz_t *pe)
{
	int result = 0;

#define COMPARE_STRUCT_SZ(X, result) \
do { \
	if ((drv->X) != (pe->X)) { \
		RTW_ERR("=======================================\n"); \
		RTW_ERR("\t" "Driver (%08u) != PE (%08u), %s\n", \
					drv->X, pe->X, #X); \
		RTW_ERR("=======================================\n"); \
		result = -1; \
	} \
} while(0)

	/* PE & DOE command */
	COMPARE_STRUCT_SZ(cmd_start_size, result);
	COMPARE_STRUCT_SZ(cmd_set_pe_addr_size, result);
	COMPARE_STRUCT_SZ(resource_size, result);
	COMPARE_STRUCT_SZ(cmd_device_info_size, result);
	COMPARE_STRUCT_SZ(cmd_adapter_info_size, result);
	COMPARE_STRUCT_SZ(cmd_sta_info_size, result);
	COMPARE_STRUCT_SZ(cmd_txsc_info_size, result);
	COMPARE_STRUCT_SZ(cmd_rxofld_info_size, result);
	/* PE & DOE to Driver command */
	COMPARE_STRUCT_SZ(cmd_update_a4_info_size, result);
	/* Response */
	COMPARE_STRUCT_SZ(rsp_pe_info_size, result);
	COMPARE_STRUCT_SZ(rsp_ring_addr_size, result);
	COMPARE_STRUCT_SZ(rsp_mib_size, result);

	return result;
}

int pe_start_complete_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no,
	ca_uint16_t trans_id, void *msg_data, ca_uint16_t *msg_size)
{
	rtl_wfo_ipc_msg_t *msg = (rtl_wfo_ipc_msg_t *)msg_data;
	rtl_wfo_ipc_rsp_pe_info_t *rsp;
	rtl_wfo_pe_info_t *wfo_pe_info;
	rtl_wfo_dev_t *wfo_dev = NULL;
	pi_info_t *dev_pi_info = NULL, *pi_info = NULL;
	pi_addr_t *dev_pi_addr = NULL;
	u32 pi_ring_rw_ptr, pi_ring_addr;
	u8 band, pe_id, i, active_pe_num;
	struct dvobj_priv *dvobj;
	rtl_wfo_ipc_msg_sz_t drv_ipc_msg_sz;
#ifdef CONFIG_OPENSYNC
	_adapter *padapter = NULL;
#endif

	RTW_INFO("%s: session_id %d cpu_id %d ack_seq %d status %d\n",
		__func__, peer.session_id, peer.cpu_id,
		msg->hdr.pe_msg_complete.ack_seq, msg->hdr.pe_msg_complete.status);

	if (msg->hdr.pe_msg_complete.status != RTL_WFO_SUCCESS)
		return 0;

	rsp = &msg->content.rsp_pe_info;

	RTW_PRINT("RTL WFO v%d.%d.%d (sha1 %08x) session %d cpu_id %d role 0x%x\n",
		rsp->ver_major, rsp->ver_minor, rsp->ver_hotfix, rsp->git_sha1,
		peer.session_id, peer.cpu_id, rsp->pe_role);
	RTW_PRINT("PI rw_ptr 0x%08x addr 0x%08x pe_priv 0x%08x sw_seq 0x%08x\n",
		rsp->pi_ring_rw_ptr, rsp->pi_ring_addr, rsp->pe_priv_addr,
		rsp->doe_sw_seq_addr);

	if (peer.cpu_id > CA_IPC_CPU_MAX) {
		RTW_ERR("%s: CPU ID (%d) > %d\n", __func__, peer.cpu_id, CA_IPC_CPU_MAX);
		return 0;
	}

	band = rtl_wfo_session_to_band(peer.session_id);
	if (!(g_wfo_priv.wfo_band_cap & BIT(band)))
		return 0;

	pe_id = rtl_wfo_get_pe_id(peer.cpu_id);
	if ((band < BAND_MAX) && (pe_id < RTL_WFO_ARM_ID)) {
		wfo_dev = &g_wfo_priv.dev[band];
		wfo_pe_info = &g_wfo_priv.pe_info[pe_id];
		dvobj = g_wfo_priv.dev[band].dvobj;
		if (!dvobj) {
			RTW_ERR("Band %d: NULL WFO devobj\n", band);
			return 0;
		}

		rtl_wfo_get_ipc_msg_size(&drv_ipc_msg_sz);
		if (rtl_wfo_cmp_ipc_msg_size(&drv_ipc_msg_sz, &rsp->ipc_msg_sz) != 0) {
			RTW_ERR("CPU ID (%d): IPC msg mismatch!!!\n", peer.cpu_id);
			return 0;
		}

		if (!(BIT(dvobj->ic_id) & rsp->ic_id)) {
			RTW_ERR("CPU ID (%d): IC supported mismatch!!!\n", peer.cpu_id);
			RTW_ERR("=======================================\n");
			RTW_ERR("[PE] IC supported list:");
			for (i = 0; i < MAX_IC_ID; i++) {
				if (rsp->ic_id & (1 << i))
					RTW_ERR(" %d", i);
			}
			RTW_ERR("\n");
			RTW_ERR("=======================================\n");
			RTW_ERR("[Driver] supported IC: %d", dvobj->ic_id);
			RTW_ERR("=======================================\n");
			return 0;
		}

		if (wfo_pe_info->state == RTL_WFO_PE_INACTIVE ||
				wfo_pe_info->state == RTL_WFO_PE_RESET) {
			wfo_pe_info->state = RTL_WFO_PE_ACTIVE;
			wfo_pe_info->pe_role = rsp->pe_role;
			wfo_pe_info->pe_priv_addr = rsp->pe_priv_addr;
		}

		/* handle PktInfo ring */
		if (rsp->pi_num &&
			(rsp->pi_start_idx + rsp->pi_num <= DOE_PI_RING_NUM)) {
			dev_pi_info = &wfo_dev->pi_info[rsp->pi_start_idx];
			dev_pi_addr = &wfo_dev->pi_addr[rsp->pi_start_idx];
			pi_ring_rw_ptr = rsp->pi_ring_rw_ptr;
			pi_ring_addr = rsp->pi_ring_addr;
			for (i = 0; i < rsp->pi_num; i++) {
				pi_info = &rsp->pi_info[i];
				dev_pi_info[i].pi_ring_num = pi_info->pi_ring_num;
				dev_pi_info[i].pi_ring_size = pi_info->pi_ring_size;
				dev_pi_info[i].pi_ring_dsize = pi_info->pi_ring_dsize;
				dev_pi_addr[i].pi_ring_rw_ptr = pi_ring_rw_ptr;
				dev_pi_addr[i].pi_ring_addr = pi_ring_addr;
				pi_ring_rw_ptr += PI_RING_RW_PTR_SIZE;
				pi_ring_addr += (pi_info->pi_ring_num * \
						(pi_info->pi_ring_size + pi_info->pi_ring_dsize));
			}
		}

		/* for DOE, save sw_seq info */
		if (wfo_pe_info->pe_role & PE_ROLE_DOE)
			wfo_dev->sw_seq_addr = rsp->doe_sw_seq_addr;
	}

	/* All PEs ready and set PE info to DOE */
	active_pe_num = 0;
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if (g_wfo_priv.pe_info[pe_id].state == RTL_WFO_PE_ACTIVE)
			active_pe_num++;
	}
	if (active_pe_num == MAX_PE_NUM)
		rtl_wfo_ipc_set_pe_addr(peer.session_id);

#ifdef CONFIG_OPENSYNC
	padapter = dvobj_get_primary_adapter(dvobj);

	rtl_wfo_ipc_set_mib(padapter, MAX_PE_NUM, "sw_deamsdu", 1);
#endif

	return 0;
}

int pe_stop_complete_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no,
	ca_uint16_t trans_id, void *msg_data, ca_uint16_t *msg_size)
{
	rtl_wfo_ipc_msg_t *msg = (rtl_wfo_ipc_msg_t *)msg_data;
	u8 pe_id, band, i;
	rtl_wfo_pe_info_t *wfo_pe_info;
	rtl_wfo_dev_t *wfo_dev = NULL;

	RTW_INFO("%s: session_id %d cpu_id %d ack_seq %d status %d\n",
		__func__, peer.session_id, peer.cpu_id,
		msg->hdr.pe_msg_complete.ack_seq, msg->hdr.pe_msg_complete.status);

	if (msg->hdr.pe_msg_complete.status != RTL_WFO_SUCCESS)
		return 0;

	band = rtl_wfo_session_to_band(peer.session_id);
	pe_id = rtl_wfo_get_pe_id(peer.cpu_id);
	if ((band < BAND_MAX) && (pe_id < RTL_WFO_ARM_ID)) {
		wfo_dev = &g_wfo_priv.dev[band];
		wfo_pe_info = &g_wfo_priv.pe_info[pe_id];

		/* PE info */
		wfo_pe_info->state = RTL_WFO_PE_INACTIVE;
		wfo_pe_info->pe_priv_addr = 0;
		if (wfo_pe_info->pe_priv)
			iounmap(wfo_pe_info->pe_priv);
		wfo_pe_info->pe_priv = NULL;

		/* device info */
		wfo_dev->sw_seq_addr = 0;
		if (wfo_dev->doe_sw_seq)
			iounmap(wfo_dev->doe_sw_seq);
		wfo_dev->doe_sw_seq = NULL;
		for (i = 0; i < DOE_PI_RING_NUM; i++) {
			wfo_dev->pi_addr[i].pi_ring_rw_ptr = 0xeeeeeeee;
			wfo_dev->pi_addr[i].pi_ring_addr = 0xeeeeeeee;
		}
	}

	return 0;
}

int pe_message_complete_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no,
	ca_uint16_t trans_id, void *msg_data, ca_uint16_t *msg_size)
{
	rtl_wfo_ipc_msg_t *msg = (rtl_wfo_ipc_msg_t *)msg_data;

	RTW_INFO("%s: session_id %d cpu_id %d ack_seq %d status %d\n",
		__func__, peer.session_id, peer.cpu_id,
		msg->hdr.pe_msg_complete.ack_seq, msg->hdr.pe_msg_complete.status);
	return 0;
}

int pe_handle_rx_pkt_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no,
	ca_uint16_t trans_id, void *msg_data, ca_uint16_t *msg_size)
{
	rtl_wfo_ipc_msg_t *msg = (rtl_wfo_ipc_msg_t *)msg_data;
	struct dvobj_priv *dvobj = NULL;
	_adapter *padapter = NULL;
	struct sk_buff *skb;
	u8 band;

	RTW_INFO("%s: session_id %d cpu_id %d cmd %d cmd_seq %d\n",
		__func__, peer.session_id, peer.cpu_id,
		msg->hdr.pe_msg.cmd, msg->hdr.pe_msg.cmd_seq);

	RTW_INFO("%s: ifid %d len %d buf %p\n",
		__func__, msg->content.rx_pkt.iface_id, msg->content.rx_pkt.len,
		msg->content.rx_pkt.buf);

	band = rtl_wfo_session_to_band(peer.session_id);
	if (band < BAND_MAX)
		dvobj = g_wfo_priv.dev[band].dvobj;
	if (!dvobj)
		return 1;

	padapter = rtw_get_iface_by_id(dvobj->padapters[IFACE_ID0],
									msg->content.rx_pkt.iface_id);
	if (!padapter)
		return 2;

	skb = alloc_skb(msg->content.rx_pkt.len, GFP_ATOMIC);
	if (!skb)
		return 3;

	skb_put(skb, msg->content.rx_pkt.len);
	memcpy(skb->data, msg->content.rx_pkt.buf, msg->content.rx_pkt.len);
	rtw_process_u2mc(padapter, skb);
	kfree_skb(skb);

	return 0;
}

#ifdef CONFIG_RTW_A4_STA
int pe_update_a4_cb(ca_ipc_addr_t peer, ca_uint16_t msg_no,
	ca_uint16_t trans_id, void *msg_data, ca_uint16_t *msg_size)
{
	rtl_wfo_ipc_msg_t *msg = (rtl_wfo_ipc_msg_t *)msg_data;
	rtl_wfo_ipc_cmd_update_a4_info_t *a4_info = &msg->content.cmd_update_a4;
	struct dvobj_priv *dvobj;
	_adapter *padapter;
	struct sta_priv *pstapriv;
	struct sta_info *psta;
	u8 band;

	RTW_INFO("%s: session_id %d cpu_id %d cmd %d cmd_seq %d\n",
		__func__, peer.session_id, peer.cpu_id,
		msg->hdr.pe_msg.cmd, msg->hdr.pe_msg.cmd_seq);

	RTW_INFO("%s: ifid %d macid %d mac %pM\n", __func__,
			a4_info->iface_id, a4_info->macid, a4_info->mac);

	band = rtl_wfo_session_to_band(peer.session_id);
	dvobj = g_wfo_priv.dev[band].dvobj;
	if (!dvobj)
		return 1;

	padapter = rtw_get_iface_by_id(dvobj->padapters[IFACE_ID0],
									a4_info->iface_id);
	if (!padapter)
		return 2;

	pstapriv = &(padapter->stapriv);
	psta = rtw_get_stainfo_by_macid(pstapriv, a4_info->macid);
	if (!psta || psta->padapter != padapter || psta == padapter->self_sta)
		return 3;

	RTW_DBG("%s: adapter ifid %d, mac %pM, sta macid %d mac %pM\n",
		__func__, padapter->iface_id, padapter->mac_addr,
		psta->phl_sta->macid, psta->phl_sta->mac_addr);

	switch (a4_info->op) {
	case A4_STA:
		core_a4_upt_sta_list(padapter, psta);
		break;
	case A4_DB:
	case A4_DB_STATS:
		//memcpy(mac, a4_info->mac, ETH_ALEN);
		RTW_DBG("\tA4 DB mac %pM op %d\n", a4_info->mac, a4_info->op);
		core_a4_upt_source_db(padapter, psta, a4_info->mac);
		rtl_wfo_upt_a4_stats(padapter, a4_info);
		break;
	case A4_DB_DEL:
		//memcpy(mac, a4_info->mac, ETH_ALEN);
		RTW_DBG("\tA4_DB_DEL mac %pM\n", a4_info->mac);
		a4_del_source_db_entry(padapter, a4_info->mac, _FALSE);
		break;
	}

	return 0;
}
#endif /* CONFIG_RTW_A4_STA */

static ca_ipc_msg_handle_t rtl_wfo_ipc_msg[] = {
	{ .msg_no = RTL_WFO_IPC_PE_START_COMPLETE, .proc = pe_start_complete_cb },
	{ .msg_no = RTL_WFO_IPC_PE_STOP_COMPLETE, .proc = pe_stop_complete_cb },
	{ .msg_no = RTL_WFO_IPC_PE_MESSAGE_COMPLETE, .proc = pe_message_complete_cb },
	{ .msg_no = RTL_WFO_IPC_PE_HANDLE_RX_PKT, .proc = pe_handle_rx_pkt_cb },
#ifdef CONFIG_RTW_A4_STA
	{ .msg_no = RTL_WFO_IPC_PE_UPDATE_A4_INFO, .proc = pe_update_a4_cb },
#endif
};

int rtl_wfo_ipc_init(struct dvobj_priv *dvobj)
{
	int session_id, rc = CA_IPC_EINVAL;

	session_id = _rtl_wfo_dvobj_to_session(dvobj);
	if (session_id < CA_IPC_SESSION_MAX) {
		rc = ca_ipc_msg_handle_register(session_id, rtl_wfo_ipc_msg,
				sizeof(rtl_wfo_ipc_msg) / sizeof(ca_ipc_msg_handle_t));

		if (rc != CA_IPC_OK) {
			RTW_ERR("RTL WFO IPC (%d) registration failed (%d)\n",
					session_id, rc);
		} else {
			RTW_PRINT("RTL WFO IPC (%d) created\n", session_id);
			if (rtl_wfo_supported_band(dvobj))
				rtl_wfo_priv_init(dvobj);
		}
	} else {
		RTW_ERR("unsupported band (%d) or create IPC (%d) failed (%d)!\n",
				GET_HAL_SPEC(dvobj)->band_cap, session_id, rc);
	}

	return rc;
}

void rtl_wfo_ipc_deinit(struct dvobj_priv *dvobj)
{
	int session_id, rc = CA_IPC_EINVAL;

	session_id = _rtl_wfo_dvobj_to_session(dvobj);
	if (session_id < CA_IPC_SESSION_MAX) {
		rc = ca_ipc_msg_handle_unregister(session_id);
		if (rc != CA_IPC_OK) {
			RTW_ERR("RTL WFO IPC (%d) unregistration failed (%d)\n",
					session_id, rc);
		} else {
			RTW_PRINT("RTL WFO IPC (%d) deleted\n", session_id);
			if (rtl_wfo_supported_band(dvobj))
				rtl_wfo_priv_deinit(dvobj);
		}
	} else {
		RTW_ERR("RTL WFO IPC deletion (%d) failed (%d)!\n",
			session_id, rc);
	}
}

static int rtl_wfo_ipc_send(int dest_cpu_id, int session_id,
	int msg_no, void *msg_data, unsigned short msg_size, int type)
{
	int result = CA_IPC_EINVAL;
	ca_ipc_pkt_t ipc_pkt;

	if (g_wfo_priv.pe_info[dest_cpu_id - 1].state != RTL_WFO_PE_ACTIVE)
		return result;

	memset(&ipc_pkt, 0, sizeof(ca_ipc_pkt_t));

	ipc_pkt.dst_cpu_id = dest_cpu_id;
	ipc_pkt.session_id = session_id;
	ipc_pkt.msg_no = msg_no;
	ipc_pkt.priority = CA_IPC_PRIO_HIGH;
	ipc_pkt.msg_data = msg_data;
	ipc_pkt.msg_size = msg_size;

	if (type == CA_IPC_SYNC_MSG)
		result = ca_ipc_msg_sync_send(&ipc_pkt, msg_data, &msg_size);
	else if (type == CA_IPC_ASYN_MSG)
		result = ca_ipc_msg_async_send(&ipc_pkt);

	return result;
}

int rtl_wfo_ipc_start_wfo(_adapter *padapter, u8 ofld_en,
	u16 wp_max_seq, u8 pe_state)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg;
	u8 pe_id, ipc_pe_id;
	rtl_wfo_ipc_cmd_start_t *pcmd;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
#ifdef CONFIG_WFO_NEW_ARCH
	int pe_start_seq[MAX_PE_NUM] =
			{RTL_WFO_PPE_ID, RTL_WFO_AOE_ID, RTL_WFO_DOE_ID};
#else
	int pe_start_seq[MAX_PE_NUM] =
			{RTL_WFO_PPE_ID, RTL_WFO_PPE_ID + 1, RTL_WFO_DOE_ID};
#endif

	// Start PEs in sequence
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(pe_start_seq[pe_id]);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session failed!\n", __func__);
			return CA_IPC_EINVAL;
		}

		memset(&msg, 0, RTL_WFO_IPC_MSG_SIZE);
		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_START_WFO;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

		pcmd = &msg.content.cmd_en;
		pcmd->rx_en = ((ofld_en & RTL_WFO_RX_OFLD) == RTL_WFO_RX_OFLD);
		pcmd->tx_en = ((ofld_en & RTL_WFO_TX_OFLD) == RTL_WFO_TX_OFLD);
		if (pcmd->tx_en && (ofld_en & RTL_WFO_TX_FULL_OFLD))
			pcmd->tx_en = 2;
		pcmd->wp_max_seq = wp_max_seq;
		pcmd->pe_state = pe_state;
		pcmd->amsdu_mode = AMSDU_OFLD_40STA_BE;

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE, &msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_SYNC_MSG);
		if (result != CA_IPC_OK) {
			RTW_ERR(ADPT_FMT": start PE%d failed (%d)!\n",
				ADPT_ARG(padapter), pe_start_seq[pe_id], result);
		} else {
			RTW_INFO(ADPT_FMT": PE%d start (rx_en %d tx_en %d "
				"wp_max_seq %d pe_state %d amsdu_mode %d)\n",
				ADPT_ARG(padapter), pe_start_seq[pe_id], pcmd->rx_en,
				pcmd->tx_en, pcmd->wp_max_seq, pcmd->pe_state,
				pcmd->amsdu_mode);
		}
	}
	g_wfo_priv.wfo_band_on |= GET_HAL_SPEC(dvobj)->band_cap;

	return result;
}

int rtl_wfo_ipc_stop_wfo(_adapter *padapter, u8 pe_state)
{
	int result, session_id, pe_id;
	rtl_wfo_ipc_msg_t msg;
	u8 ipc_pe_id;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
#ifdef CONFIG_WFO_NEW_ARCH
	int pe_stop_seq[MAX_PE_NUM] =
			{RTL_WFO_DOE_ID, RTL_WFO_AOE_ID, RTL_WFO_PPE_ID};
#else
	int pe_stop_seq[MAX_PE_NUM] =
			{RTL_WFO_DOE_ID, RTL_WFO_PPE_ID + 1, RTL_WFO_PPE_ID};
#endif
#ifdef FBM_USE_KMALLOC_BUF
	extern rtl_wfo_res_tbl_t res_tbl[];
	rtl_wfo_res_tbl_t *tbl;
	u32 res_addr, phy_addr, *buf_info, buf_info_sz,
		recycle_cnt[RESOURCE_MAX] = { 0 };
	int i;
#endif

#ifdef FBM_USE_KMALLOC_BUF
	extern rtl_wfo_res_tbl_t res_tbl[];
	rtl_wfo_res_tbl_t *tbl;
	u32 res_addr, phy_addr, *buf_info, buf_info_sz,
		recycle_cnt[RESOURCE_MAX] = { 0 };
#endif

	g_wfo_priv.wfo_band_on &=
			~(GET_HAL_SPEC(dvobj)->band_cap);

	// Stop PEs in sequence
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(pe_stop_seq[pe_id]);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session failed!\n", __func__);
			return CA_IPC_EINVAL;
		}

		memset(&msg, 0, RTL_WFO_IPC_MSG_SIZE);
		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_STOP_WFO;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
		msg.content.cmd_en.rx_en = 0;
		msg.content.cmd_en.tx_en = 0;
		msg.content.cmd_en.pe_state = pe_state;

#ifdef FBM_USE_KMALLOC_BUF
		for (i = 0; i < RESOURCE_MAX; i++) {
			tbl = &res_tbl[i];
			buf_info = tbl->free_info;
			buf_info_sz = tbl->rec_max * sizeof(u32);

			memset(buf_info, 0, buf_info_sz);
			phy_addr = (u32)virt_to_phys(buf_info);
			rtl_wfo_res_wback(padapter->dvobj, phy_addr, buf_info_sz);

			msg.content.cmd_en.res[i].buf_size = tbl->buf_size;
			msg.content.cmd_en.res[i].buf_info = phy_addr;
			msg.content.cmd_en.res[i].buf_info_sz = buf_info_sz;
		}
#endif

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE, &msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_SYNC_MSG);

		if (result != CA_IPC_OK)
			RTW_ERR(ADPT_FMT": stop PE%d failed (%d)!\n",
				ADPT_ARG(padapter), pe_stop_seq[pe_id], result);
		else
			RTW_INFO(ADPT_FMT": PE%d stop\n",
				ADPT_ARG(padapter), pe_stop_seq[pe_id]);

#ifdef FBM_USE_KMALLOC_BUF
		for (i = 0; i < RESOURCE_MAX; i++) {
			rtl_wfo_buf_recycle_info_t *res = &msg.content.cmd_en.res[i];
			tbl = &res_tbl[i];

			if (res->recycle_cnt)
				recycle_cnt[i] +=
					rtl_wfo_free_res(padapter->dvobj, i, res->recycle_cnt);
		}
#endif
	}

#ifdef FBM_USE_KMALLOC_BUF
	RTW_PRINT("Dram Recycle: NIRX %d AMSDU_NIRX %d\n",
		recycle_cnt[0], recycle_cnt[1]);
#endif

	return 0;
}

int rtl_wfo_ipc_set_resource(_adapter *padapter, u8 pe_id,
	rtl_wfo_pe_resource_t *res)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
	rtl_wfo_ipc_resource_t *pcmd;

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_RESOURCE;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
	pcmd = &msg.content.resource;
	pcmd->force_init = res->force_init;
	pcmd->res_type = res->res_type;
	pcmd->res_addr = res->res_addr;
	pcmd->res_size = res->res_size;
	pcmd->buf_size = res->buf_size;
	pcmd->buf_num = res->buf_num;

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Set PE/DOE (%d) resource fail (%d)!\n",
			ADPT_ARG(padapter), pe_id, result);
	else
		RTW_INFO(ADPT_FMT": Resource (%d 0x%08x %d) to PE/DOE (%d)\n",
			ADPT_ARG(padapter), res->res_type, res->res_addr,
			res->res_size, pe_id);

	return 0;
}

int rtl_wfo_ipc_send_device_info(_adapter *padapter)
{
	int i, ret = CA_IPC_OK, result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_device_info_t *pcmd;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct rtw_phl_com_t *phl_com = GET_HAL_DATA(dvobj);
	PPCI_DATA pci_data = dvobj_to_pci(dvobj);
	struct pci_dev *pdev = pci_data->ppcidev;
	unsigned long pmem_start, pmem_flags;
	u8 pe_id, ipc_pe_id;

	for (i = 0 ; i < 6; i++) {
		pmem_start = pci_resource_start(pdev, i);
		pmem_flags = pci_resource_flags(pdev, i);
		if (pmem_flags & IORESOURCE_MEM)
			break;
	}
	if (i == 6) {
		RTW_ERR("pci memory-io not found!\n");
		return CA_E_NOT_FOUND;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_SEND_DEVICE_INFO;

	pcmd = &msg.content.cmd_device_info;
	pcmd->pcie_phy_addr_start = pmem_start;
	pcmd->chip_id = padapter->dvobj->ic_id;
	pcmd->append_fcs = phl_com->append_fcs;
	pcmd->accept_icv_err = phl_com->accept_icv_err;

	RTW_PRINT("pcie_phy_addr_start 0x%08x chip_id %d\n",
		pcmd->pcie_phy_addr_start, pcmd->chip_id);

	/* send device info to PPE and DOE */
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if (g_wfo_priv.pe_info[pe_id].pe_role & \
			(PE_ROLE_PPE | PE_ROLE_DOE)) {
			ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
			session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
			if (session_id) {
				msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
				result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
							RTL_WFO_IPC_PE_MESSAGE, &msg,
							RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
				if (result != CA_IPC_OK) {
					RTW_ERR("Send device info to PE(%d) fail (%d)!\n",
						pe_id, result);
					ret = result;
				}
			}
		}
	}

	return ret;
}

int rtl_wfo_ipc_cfg_adapter_info(_adapter *padapter, int op)
{
	int ret = CA_IPC_OK, result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_adapter_info_t *pcmd;
	u8 pe_id, ipc_pe_id;

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_CFG_ADAPTER_INFO;
	pcmd = &msg.content.cmd_adapter_info;
	pcmd->op = op;
	pcmd->iface_id = padapter->iface_id;
	pcmd->wlan_dev_id = rtl_wfo_get_devid(padapter->pnetdev);

	switch (op) {
	case RTL_WFO_OP_ADD:
	case RTL_WFO_OP_UPDATE:
		memcpy(pcmd->mac_addr, padapter->mac_addr, ETH_ALEN);
#ifdef CONFIG_BR_EXT
		memcpy(pcmd->br_mac, padapter->br_mac, ETH_ALEN);
#endif
#ifdef CONFIG_RTW_A4_STA
		pcmd->a4_enable = padapter->a4_enable;
#endif
		pcmd->adapter_type = padapter->adapter_type;
		pcmd->fw_state = padapter->mlmepriv.fw_state;
		pcmd->authtype = padapter->securitypriv.dot11AuthAlgrthm;
		pcmd->encmode = padapter->securitypriv.dot11PrivacyAlgrthm;
		if (MLME_IS_STA(padapter)) {
			memcpy(pcmd->bssid, get_bssid(&padapter->mlmepriv), ETH_ALEN);
			RTW_INFO("MLME_IS_STA bssid "MAC_FMT"\n",
					MAC_ARG(get_bssid(&padapter->mlmepriv)));
		}

		RTW_INFO(ADPT_FMT": [%s adapter] id %d adapter_type %d mac_addr "MAC_FMT
			" fw_state 0x%x authtype %d encmode %d\n",
			ADPT_ARG(padapter), (pcmd->op ? "Update" : "Add"), pcmd->iface_id,
			pcmd->adapter_type, MAC_ARG(pcmd->mac_addr), pcmd->fw_state,
			pcmd->authtype, pcmd->encmode);
		break;
	case RTL_WFO_OP_DELETE:
		RTW_INFO("Del adapter: iface_id %d\n", pcmd->iface_id);
		break;
	case RTL_WFO_OP_DUMP:
		RTW_INFO("Dump adapter info\n");
		break;
	default:
		RTW_ERR("%s: unknown op code (%d)\n", __func__, op);
		return CA_IPC_EINVAL;
	}

	/* send adapter info to PPE */
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if (g_wfo_priv.pe_info[pe_id].pe_role & PE_ROLE_PPE) {
			ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
			session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
			if (session_id) {
				msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
				result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
							RTL_WFO_IPC_PE_MESSAGE, &msg,
							RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
				if (result != CA_IPC_OK) {
					RTW_ERR(ADPT_FMT": config(%d) adapter info to PE(%d) fail (%d)!\n",
						ADPT_ARG(padapter), op, pe_id, result);
					ret = result;
				}
			}
		}
	}

	return ret;
}

#ifdef CONFIG_RTW_A4_STA
int rtl_wfo_ipc_del_a4_db(_adapter *padapter, u8 pe_id, u8 *mac, u8 fc_staid)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_del_a4_db_t *pcmd;
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_DEL_A4_DB;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	pcmd = &msg.content.cmd_del_a4_db;
	memcpy(pcmd->mac, mac, ETH_ALEN);
	pcmd->iface_id = padapter->iface_id;
	pcmd->fc_staid = fc_staid;
	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
			RTL_WFO_IPC_PE_MESSAGE, &msg,
			RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);

	if (result != CA_IPC_OK)
		RTW_ERR("Send delete A4 DB to PE (%d) fail (%d)!\n", pe_id, result);

	return result;
}

int rtl_wfo_ipc_get_a4_info(_adapter *padapter, u8 pe_id, int op)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_adapter_info_t *pcmd;
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_A4_INFO;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	pcmd = &msg.content.cmd_adapter_info;
	pcmd->op = op;
	pcmd->iface_id = padapter->iface_id;
	pcmd->wlan_dev_id = rtl_wfo_get_devid(padapter->pnetdev);

	switch (op) {
		case RTL_WFO_OP_DUMP:
			RTW_INFO("Dump A4 adapter info\n");
			break;
		default:
			RTW_ERR("%s: unknown op code (%d)\n", __func__, op);
			return CA_IPC_EINVAL;
	}

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
			RTL_WFO_IPC_PE_MESSAGE, &msg,
			RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR("configure(%d) A4 adapter info to PE (%d) fail (%d)!\n",
				op, pe_id, result);

	return result;
}
#endif /* CONFIG_RTW_A4_STA */

int rtl_wfo_ipc_update_fc_sta(_adapter *padapter,
	u16 macid, u8 fc_staid, u8 to_role, u8 ppe_pe_id)
{
	int result = CA_IPC_EINVAL, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_upt_fc_sta_t *pcmd;
	u8 pe_id, ipc_pe_id;

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_UPDATE_FC_STA;
	pcmd = &msg.content.cmd_upt_fc_sta;
	pcmd->macid = macid;
	pcmd->fc_staid = fc_staid;

	/* If to_role is PE_ROLE_ALL, force to send IPC to all PEs.
	   Otherwise, for PPE-only role, IPC should be sent only to the PE
	   that matches ppe_pe_id. */
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if ((g_wfo_priv.pe_info[pe_id].pe_role & to_role) ||
			(to_role == PE_ROLE_ALL)) {
			/* For PPE, need to check if rps_pe_id matches. */
			if ((to_role != PE_ROLE_ALL) &&
				(g_wfo_priv.pe_info[pe_id].pe_role == PE_ROLE_PPE) &&
				(pe_id != ppe_pe_id))
				continue;

			ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
			session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
			if (session_id) {
				msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
				result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
								RTL_WFO_IPC_PE_MESSAGE, &msg,
								RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
				if (result != CA_IPC_OK)
					RTW_ERR(ADPT_FMT": Update FC_STA to PE (%d) fail (%d)!\n",
						ADPT_ARG(padapter), pe_id, result);
			}
		}
	}
	return result;
}

int rtl_wfo_ipc_cfg_sta_info(_adapter *padapter,
	void *para, u8 to_role, int op)
{
	int result = CA_IPC_EINVAL, session_id, *macid;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_sta_info_t *pcmd;
	u8 pe_id, ipc_pe_id;
	struct sta_info *psta = (struct sta_info *)para;
#ifdef CONFIG_RTW_A4_STA
	struct rtw_a4_db_entry *db;
#endif

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_CFG_STA_INFO;

	pcmd = &msg.content.cmd_sta_info;
	pcmd->op = op;

	switch (op) {
	case RTL_WFO_OP_ADD:
	case RTL_WFO_OP_UPDATE:
		pcmd->macid = psta->phl_sta->macid;
		memcpy(pcmd->adapter_mac_addr, padapter->mac_addr, ETH_ALEN);

		pcmd->active = psta->phl_sta->active;
		pcmd->state = psta->state;
		pcmd->rps_pe_id = psta->rps_pe_id;
		pcmd->fc_staid = psta->fc_staid;

		pcmd->agg_en_bitmap = psta->htpriv.agg_enable_bitmap;
		pcmd->tx_amsdu_enable = psta->htpriv.tx_amsdu_enable;
		pcmd->amsdu_max_num = psta->txsc_amsdu_num;
		pcmd->amsdu_max_size = (u16)psta->txsc_amsdu_size;
		pcmd->keytrack = ATOMIC_READ(&psta->keytrack);
		pcmd->twt_en = psta->twt_en;
		pcmd->wpseq_lmt =  psta->wpseq_lmt;

		if (op == RTL_WFO_OP_ADD) {
			memcpy(pcmd->mac_addr, psta->phl_sta->mac_addr, ETH_ALEN);
		}
		RTW_INFO(ADPT_FMT": op %d to_role 0x%x addr "MAC_FMT" active %d "
				"state 0x%08x macid %d rps_pe_id %d fc_staid %d "
				"agg_en 0x%02x amsdu (en %d max %d size %d)\n",
			ADPT_ARG(padapter), op, to_role, MAC_ARG(psta->phl_sta->mac_addr),
			pcmd->active, pcmd->state, pcmd->macid, pcmd->rps_pe_id,
			pcmd->fc_staid, pcmd->agg_en_bitmap, pcmd->tx_amsdu_enable,
			pcmd->amsdu_max_num, pcmd->amsdu_max_size);
		break;
	case RTL_WFO_OP_DELETE:
		pcmd->macid = psta->phl_sta->macid;
		pcmd->fc_staid = psta->fc_staid;
		memcpy(pcmd->adapter_mac_addr, padapter->mac_addr, ETH_ALEN);

		RTW_INFO(ADPT_FMT": op %d to_role 0x%x macid %d fc_staid %d\n",
			ADPT_ARG(padapter), op, to_role, pcmd->macid, pcmd->fc_staid);
		break;
	case RTL_WFO_OP_A4_UPDATE:
#ifdef CONFIG_RTW_A4_STA
		db = (struct rtw_a4_db_entry *)para;
		psta = db->psta;
		pcmd->macid = psta->phl_sta->macid;
		memcpy(pcmd->adapter_mac_addr, padapter->mac_addr, ETH_ALEN);
		pcmd->rps_pe_id = psta->rps_pe_id;

		/* Update A4 db related info */
		pcmd->fc_staid = db->fc_staid;

		RTW_PRINT(ADPT_FMT": op %d to_role 0x%x addr %pM "
			"macid %d rps_pe_id %d fc_staid %d\n",
			ADPT_ARG(padapter), op, to_role, db->mac,
			pcmd->macid, pcmd->rps_pe_id, db->fc_staid);
#endif
		break;
	case RTL_WFO_OP_DUMP:
		macid = (int *)para;
		pcmd->macid = (u16)*macid;
		break;
	default:
		RTW_ERR("%s: unknown op code (%d)\n", __func__, op);
		return CA_IPC_EINVAL;
	}

	/* If to_role is PE_ROLE_ALL, force to send IPC(OP_DUMP) to all PEs.
	   Otherwise, for PPE-only role, IPC should be sent only to the PE
	   that matches STA's rps_pe_id. */
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if ((g_wfo_priv.pe_info[pe_id].pe_role & to_role) ||
			(to_role == PE_ROLE_ALL)) {
			/* For PPE, need to check if ppe_pe_id matches. */
			if ((to_role != PE_ROLE_ALL) &&
				(g_wfo_priv.pe_info[pe_id].pe_role == PE_ROLE_PPE) &&
				(pe_id != psta->ppe_pe_id))
				continue;

			ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
			session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
			if (session_id) {
				msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
				result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
								RTL_WFO_IPC_PE_MESSAGE, &msg,
								RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
				if (result != CA_IPC_OK)
					RTW_ERR(ADPT_FMT": configure(%d) STA info to PE (%d) fail (%d)!\n",
						ADPT_ARG(padapter), op, pe_id, result);
			}
		}
	}

	return result;
}

int rtl_wfo_ipc_update_txsc_info(_adapter *padapter, struct sta_info *psta,
	u8 *txsc_wlhdr, u8 wlhdr_len, u8 *wd_cache, u8 wd_len, u8 wd_seq_offset)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_txsc_info_t *pcmd;
	struct txsc_entry *ts_entry;
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(psta->xps_pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	if (wd_len > MAX_WD_CACHE_SIZE) {
		RTW_ERR("%s: invalid wd_len %d\n", __func__, wd_len);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_UPDATE_TXSC_INFO;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	pcmd = &msg.content.cmd_txsc_info;
	pcmd->macid = psta->phl_sta->macid;

	pcmd->wlhdr_len = wlhdr_len;
	memcpy(pcmd->txsc_wlhdr, txsc_wlhdr, wlhdr_len);
	pcmd->wd_len = wd_len;
	memcpy(pcmd->wd_cache, wd_cache, pcmd->wd_len);
	pcmd->wd_seq_offset = wd_seq_offset;

	RTW_INFO(ADPT_FMT": macid %d wlhdr_len %d wd_len %d wd_seq_offset %d\n",
		ADPT_ARG(padapter), pcmd->macid, pcmd->wlhdr_len,
		pcmd->wd_len, pcmd->wd_seq_offset);
	RTW_DBG_DUMP("txsc_wlhdr", pcmd->txsc_wlhdr, pcmd->wlhdr_len);
	RTW_DBG_DUMP("wd_cache", pcmd->wd_cache, pcmd->wd_len);

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Update txsc entry to PE (%d) fail (%d)!\n",
			ADPT_ARG(padapter), psta->xps_pe_id, result);

	return result;
}

int rtl_wfo_ipc_update_rxofld_info(_adapter *padapter,
	struct sta_info *psta, u32 ssn, u32 tid, u16 size)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_rxofld_info_t *pcmd;
	u8 to_pe = rtl_wfo_pe_id_mapping(psta);
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(to_pe);

	if (tid >= RTL_WFO_MAX_TID_NUM) {
		RTW_ERR("%s: invalid TID %d!\n", __func__, tid);
		return CA_IPC_EINVAL;
	}

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_UPDATE_RXOFLD_INFO;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	pcmd = &msg.content.cmd_rxofld_info;
	memcpy(pcmd->adapter_mac_addr, padapter->mac_addr, ETH_ALEN);
	pcmd->macid = psta->phl_sta->macid;
	pcmd->ssn = ssn;
	pcmd->buf_size = size;
	pcmd->tid = tid;

	RTW_INFO(ADPT_FMT": to_pe %d adapter "MAC_FMT" macid %d ssn %d buf_size %d tid %d\n",
		ADPT_ARG(padapter), to_pe, MAC_ARG(pcmd->adapter_mac_addr),
		pcmd->macid, pcmd->ssn, pcmd->buf_size, pcmd->tid);

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Send RX ofld info to PE (%d) fail (%d)!\n",
				ADPT_ARG(padapter), psta->rps_pe_id, result);

	return result;
}

int rtl_wfo_ipc_get_ring_addr(_adapter *padapter,
	rtl_wfo_ipc_rsp_ring_addr_t *ring_info)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_rsp_ring_addr_t *rsp;
	u8 pe_id, ipc_pe_id;

	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if (g_wfo_priv.pe_info[pe_id].pe_role & PE_ROLE_DOE)
			break;
	}
	ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_RING_ADDR;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_SYNC_MSG);
	if (result != CA_IPC_OK) {
		RTW_ERR("Get ring addr from (%d) fail (%d)!\n", pe_id, result);
		return result;
	}

	/* Response from DOE */
	rsp = &msg.content.rsp_ring_addr;
	_rtw_memcpy(ring_info, rsp, sizeof(rtl_wfo_ipc_rsp_ring_addr_t));

	RTW_INFO("rw_base rx 0x%08x rp 0x%08x tx 0x%08x\n",
		rsp->rx_ring_rw_base, rsp->rp_ring_rw_base, rsp->tx_ring_rw_base);
	RTW_INFO("data_base rx 0x%08x rp 0x%08x tx 0x%08x\n",
		rsp->rx_ring_data_base, rsp->rp_ring_data_base, rsp->tx_ring_data_base);
	RTW_INFO("ring_sz rx %u rp %u tx %u\n",
		rsp->rx_ring_size, rsp->rp_ring_size, rsp->tx_ring_size);

	return result;
}

int rtl_wfo_ipc_set_pe_addr(int session_id)
{
	int result = CA_IPC_OK;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_set_pe_addr_t *pcmd;
	u8 i, ipc_pe_id, band;
	rtl_wfo_dev_t *wfo_dev;
	pi_addr_t *pi_addr;

	band = rtl_wfo_session_to_band(session_id);
	if (band >= BAND_MAX)
		return CA_IPC_EINVAL;
	wfo_dev = &g_wfo_priv.dev[band];

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_SET_PE_PI_ADDR;

	pcmd = &msg.content.cmd_set_pe_addr;
	for (i = 0; i < WFO_PPE_NUM; i++) {
		pi_addr = &wfo_dev->pi_addr[i * PI_RING_NUM];
		pcmd->pi_addr[i].pi_ring_rw_ptr = pi_addr->pi_ring_rw_ptr;
		pcmd->pi_addr[i].pi_ring_addr = pi_addr->pi_ring_addr;

		RTW_INFO("pe_info[%d] pe_rw_ptr 0x%08x bd 0x%08x\n",
			i, pi_addr->pi_ring_rw_ptr, pi_addr->pi_ring_addr);
	}

	/* send adapter info to DOE | PPE */
	for (i = 0; i < MAX_PE_NUM; i++) {
		if (g_wfo_priv.pe_info[i].pe_role & (PE_ROLE_DOE | PE_ROLE_PPE)) {
			ipc_pe_id = rtl_wfo_get_ipc_id(i);
			msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
			result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
							RTL_WFO_IPC_PE_MESSAGE, &msg,
							RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
			if (result != CA_IPC_OK)
				RTW_ERR("Set pe_info to PE%d (role 0x%x) fail (%d)!\n",
					i, g_wfo_priv.pe_info[i].pe_role, result);
		}
	}

	return result;
}

static struct rtl_wfo_mib_tbl wfo_mib_tbl[] = {
	{RTL_WFO_MIB_MON_INTVL, "mon", "<sec>", MIB_INT_T},
	{RTL_WFO_MIB_MON_BITMAP, "mon_bm", "<B0-B2: PE|STA|ADPTR>", MIB_INT_T},
	{RTL_WFO_MIB_LOG_LEVEL, "log", "<level>", MIB_INT_T},
	{RTL_WFO_MIB_LOG_COMP, "log_comp", "<bitmap>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_TO, "amsdu_to", "<us>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_MIN_PKT, "amsdu_min", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_BIG_PKT_MAX, "amsdu_bpkt_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_SMALL_PKT_MAX, "amsdu_spkt_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_MAX_SIZE, "amsdu_size", "<size>", MIB_INT_T},
	{RTL_WFO_MIB_AMSDU_AGG, "amsdu_agg", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_TXREQ_LMT, "txreq_lmt", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_TXREQ_LMT_DROP, "txreq_lmt_drop", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_MAX_AGG_NUM, "max_agg_num", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_PE_VOQ_CHK, "pe_voq_chk", "<1|0>", MIB_INT_T},
	{RTL_WFO_MIB_GTR_MAX, "gather_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_RFL_MAX, "refill_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_SCT_MAX, "scatter_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_RCY_MAX, "recycle_max", "<num>", MIB_INT_T},
	{RTL_WFO_MIB_TID_REMAP, "tid_remap", "<1|0>", MIB_INT_T},
	{RTL_WFO_MIB_TCP_ACK_OPT, "tcp_ack_opt", "<1|0>", MIB_INT_T},
	{RTL_WFO_MIB_PKT_DUMP, "pkt_dump", "<B0-B5: TX|RX|WP_RPT|NI_TX|NI_RX_AGG|NI_TX_AGG>", MIB_INT_T},
	{RTL_WFO_MIB_VERSION, "version", "", MIB_STR_T},
	{RTL_WFO_MIB_SW_DEAMSDU, "sw_deamsdu", "<1|0>", MIB_INT_T},
	{RTL_WFO_MIB_REORDER_TO, "reorder_to", "<ms>", MIB_INT_T},
	/* keep RTL_WFO_MIB_MAX at the end of this table */
	{RTL_WFO_MIB_MAX, "keep_this_last", "", MIB_INT_T}
};

int rtl_wfo_ipc_set_mib(_adapter *padapter, u8 pe_id, char *name, int value)
{
	int result, session_id, i, start, end;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id;
	rtl_wfo_ipc_cmd_set_mib_t *setmib;
	struct rtl_wfo_mib_tbl *pmib = NULL;

	/* MIB name-id mapping */
	for (i = 0; i < ARRAY_SIZE(wfo_mib_tbl); i++) {
		if (!strcmp(name, wfo_mib_tbl[i].name)) {
			pmib = &wfo_mib_tbl[i];
			break;
		}
	}
	if (!pmib) {
		RTW_PRINT("Usage: wfo setmib|sm <name> <value> [pe_id]\n");
		RTW_PRINT("[MIB table]\n");
		for (i = 0; i < ARRAY_SIZE(wfo_mib_tbl); i++) {
			RTW_PRINT("\t%s %s\n", wfo_mib_tbl[i].name, wfo_mib_tbl[i].str);
		}
		return -1;
	}

	if ((pe_id >= 0) && (pe_id < MAX_PE_NUM)) {
		start = pe_id;
		end = pe_id + 1;
	} else {
		start = 0;
		end = MAX_PE_NUM;
	}

	for (i = start; i < end; i++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(i);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session fail!\n", __func__);
			return CA_IPC_EINVAL;
		}

		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_SET_MIB;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
		setmib = &msg.content.cmd_set_mib;
		setmib->id = pmib->id;
		setmib->value = value;

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE, &msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
		if (result != CA_IPC_OK) {
			RTW_ERR(ADPT_FMT": PE%d setmib %s fail (%d)!\n",
				ADPT_ARG(padapter), i, name, result);
			return result;
		}

		if (pmib->type == MIB_INT_T)
			RTW_PRINT(ADPT_FMT": PE%d setmib %s (%d 0x%x)\n",
				ADPT_ARG(padapter), i, name, setmib->value, setmib->value);
	}

	return 0;
}

int rtl_wfo_ipc_get_mib(_adapter *padapter, u8 pe_id, char *name)
{
	int result, session_id, i, start, end;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id;
	rtl_wfo_ipc_rsp_mib_t *rsp;
	rtl_wfo_ipc_cmd_get_mib_t *getmib;
	struct rtl_wfo_mib_tbl *pmib = NULL;

	/* MIB name-id mapping */
	for (i = 0; i < ARRAY_SIZE(wfo_mib_tbl); i++) {
		if (!strcmp(name, wfo_mib_tbl[i].name)) {
			pmib = &wfo_mib_tbl[i];
			break;
		}
	}
	if (!pmib) {
		RTW_PRINT("Usage: wfo getmib|gm <name>\n");
		RTW_PRINT("[MIB table]\n");
		for (i = 0; i < ARRAY_SIZE(wfo_mib_tbl); i++) {
			RTW_PRINT("\t%s %s\n", wfo_mib_tbl[i].name, wfo_mib_tbl[i].str);
		}
		return -1;
	}

	if ((pe_id >= 0) && (pe_id < MAX_PE_NUM)) {
		start = pe_id;
		end = pe_id + 1;
	} else {
		start = 0;
		end = MAX_PE_NUM;
	}

	for (i = start; i < end; i++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(i);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session fail!\n", __func__);
			return CA_IPC_EINVAL;
		}

		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_MIB;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
		getmib = &msg.content.cmd_get_mib;
		getmib->id = pmib->id;

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE, &msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_SYNC_MSG);
		if (result != CA_IPC_OK) {
			RTW_ERR(ADPT_FMT": PE%d getmib (%s) fail (%d)!\n",
				ADPT_ARG(padapter), i, name, result);
			return result;
		}

		/* Response from PE/DOE */
		rsp = &msg.content.rsp_mib;
		if (pmib->type == MIB_INT_T)
			RTW_PRINT(ADPT_FMT": PE%d getmib %s (%d 0x%x)\n",
				ADPT_ARG(padapter), i, name, rsp->int_val, rsp->int_val);
		else if (pmib->type == MIB_STR_T)
			RTW_PRINT(ADPT_FMT": PE%d getmib %s (%s)\n",
				ADPT_ARG(padapter), i, name, rsp->str_val);
	}
	return 0;
}

int rtl_wfo_ipc_get_status(_adapter *padapter, u8 pe_id, u16 reset)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_STATUS;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
	msg.hdr.pe_msg.extra = reset;

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Get PE (%d) status fail (%d)!\n",
			ADPT_ARG(padapter), pe_id, result);

	return 0;
}

int rtl_wfo_ipc_priv_info(_adapter *padapter)
{
	int i, result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id;

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_PRIV_INFO;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	for (i = 0; i < MAX_PE_NUM; i++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(i);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session fail!\n", __func__);
			return CA_IPC_EINVAL;
		}

		rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	}

	return 0;
}

int rtl_wfo_ipc_get_wmm_status(_adapter *padapter, u16 reset)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 pe_id, ipc_pe_id;

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_WMM_STATUS;
	msg.hdr.pe_msg.extra = reset;

	/* send device info to DOE and AOE */
	for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++) {
		if (g_wfo_priv.pe_info[pe_id].pe_role & (PE_ROLE_DOE | PE_ROLE_AOE)) {
			ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
			session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
			if (session_id) {
				msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
				result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
							RTL_WFO_IPC_PE_MESSAGE, &msg,
							RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
				if (result != CA_IPC_OK)
					RTW_ERR(ADPT_FMT": Get PE (%d) wmm status fail (%d)!\n",
						ADPT_ARG(padapter), pe_id, result);
			}
		}
	}

	return 0;
}

int rtl_wfo_ipc_dump_mem(_adapter *padapter, u8 pe_id, u32 addr, u16 len)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_dump_mem_t *pcmd;
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_DUMP_MEM;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	pcmd = &msg.content.cmd_dump_mem;
	pcmd->addr = addr;
	pcmd->len = len;

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Dump PE/DOE (%d) memory fail (%d)!\n",
			ADPT_ARG(padapter), pe_id, result);

	return result;
}

int rtl_wfo_ipc_dump_doe_chan(_adapter *padapter, u8 pe_id, u8 ch_idx)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_dump_doe_chan_t *cmd;
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_DUMP_DOE_CHAN;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	cmd = &msg.content.cmd_dump_doe_chan;
	cmd->ch_idx = ch_idx;
	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE,	&msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Dump DOE (%d) channel info fail (%d)!\n",
			ADPT_ARG(padapter), pe_id, result);

	return result;
}

int rtl_wfo_ipc_start_profiling(_adapter *padapter, u8 pe_id)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_set_mib_t *cmd;
	u8 ipc_pe_id, start, end;

	if ((pe_id >= 0) && (pe_id < MAX_PE_NUM)) {
		start = pe_id;
		end = pe_id + 1;
	} else {
		start = 0;
		end = MAX_PE_NUM;
	}

	for (pe_id = start; pe_id < end; pe_id++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session fail!\n", __func__);
			return CA_IPC_EINVAL;
		}

		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_START_PROFILING;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE,	&msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
		if (result != CA_IPC_OK) {
			RTW_ERR(ADPT_FMT" PE%d start profiling fail (%d)!\n",
				ADPT_ARG(padapter), pe_id, result);
			return result;
		}
	}

	return 0;
}

int rtl_wfo_ipc_stop_profiling(_adapter *padapter, u8 pe_id)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	rtl_wfo_ipc_cmd_set_mib_t *cmd;
	u8 ipc_pe_id, start, end;

	if ((pe_id >= 0) && (pe_id < MAX_PE_NUM)) {
		start = pe_id;
		end = pe_id + 1;
	} else {
		start = 0;
		end = MAX_PE_NUM;
	}

	for (pe_id = start; pe_id < end; pe_id++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session fail!\n", __func__);
			return CA_IPC_EINVAL;
		}

		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_STOP_PROFILING;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE,	&msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
		if (result != CA_IPC_OK) {
			RTW_ERR(ADPT_FMT": PE%d stop profiling fail (%d)!\n",
				ADPT_ARG(padapter), pe_id, result);
			return result;
		}
	}

	return 0;
}

int rtl_wfo_ipc_get_memory_map(_adapter *padapter, u8 pe_id)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);

	session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
	if (!session_id) {
		RTW_ERR("%s: get IPC session fail!\n", __func__);
		return CA_IPC_EINVAL;
	}

	msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_GET_MEMMAP;
	msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;

	result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
				RTL_WFO_IPC_PE_MESSAGE, &msg,
				RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
	if (result != CA_IPC_OK)
		RTW_ERR(ADPT_FMT": Get PE (%d) memory map fail (%d)!\n",
			ADPT_ARG(padapter), pe_id, result);

	return 0;
}

int rtl_wfo_ipc_set_uart_selection(_adapter *padapter, u8 uart_sel)
{
	int result, session_id;
	rtl_wfo_ipc_msg_t msg = {0};
	u8 ipc_pe_id, pe_id;


	for (pe_id = 0; pe_id < RTL_WFO_ARM_ID; pe_id++) {
		ipc_pe_id = rtl_wfo_get_ipc_id(pe_id);
		session_id = rtl_wfo_get_ipc_session(padapter, ipc_pe_id);
		if (!session_id) {
			RTW_ERR("%s: get IPC session failed!\n", __func__);
			return CA_IPC_EINVAL;
		}

		msg.hdr.pe_msg.cmd = RTL_WFO_IPC_MSG_CMD_UART_SELECT;
		msg.hdr.pe_msg.cmd_seq = ipc_cmd_seq++;
		msg.hdr.pe_msg.extra = uart_sel + 1; /* 0:ARM, 1~3:PE */

		result = rtl_wfo_ipc_send(ipc_pe_id, session_id,
					RTL_WFO_IPC_PE_MESSAGE, &msg,
					RTL_WFO_IPC_MSG_SIZE, CA_IPC_ASYN_MSG);
		if (result != CA_IPC_OK)
			RTW_ERR(ADPT_FMT": UART1 to (%d), IPC to (%d) fail (%d)!\n",
				ADPT_ARG(padapter), uart_sel, pe_id, result);
	}

	return 0;
}

#ifdef CONFIG_VW_REFINE
void apply_pe_tc_latency_param(_adapter *padapter,
	u8 sta_num, u16 new_pkt_len)
{
	u8 tx_hwagg, amsdu_min, amsdu_agg;
	u8 amsdu_bpkt_max, amsdu_spkt_max;

	if (sta_num == 16) {
		if (new_pkt_len > 1500) {
			tx_hwagg = 127;
			amsdu_min = 14;
			amsdu_agg = 1;
			amsdu_bpkt_max = 1;
			amsdu_spkt_max = 40;
		} else if (new_pkt_len > 510) {
			tx_hwagg = 63;
			amsdu_min = 6;
			amsdu_agg = 1;
			amsdu_bpkt_max = 0;
			amsdu_spkt_max = 6;
		} else {
			tx_hwagg = 63;
			amsdu_min = 20;
			amsdu_agg = 2;
			amsdu_bpkt_max = 0;
			amsdu_spkt_max = 10;
		}
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "amsdu_agg", amsdu_agg);
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "amsdu_min", amsdu_min);
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "max_agg_num", tx_hwagg);
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "amsdu_bpkt_max", amsdu_bpkt_max);
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "amsdu_spkt_max", amsdu_spkt_max);
	}
}

void apply_pe_tc_tput_param(_adapter *padapter,
	u8 sta_num, u16 new_pkt_len)
{
	u8 tx_hwagg;
	u32 edca_val = 0;

	if (sta_num == 5) {
		if (new_pkt_len > 1500) {
			tx_hwagg = 127;
			edca_val = 0x1125;
		} else if (new_pkt_len > 510) {
			tx_hwagg = 63;
			edca_val = 0x1125;
		} else {
			tx_hwagg = 36;
			edca_val = 0x1125;
		}
	} else if (sta_num == 10) {
		if (new_pkt_len > 1500) {
			tx_hwagg = 76;
			edca_val = 0x1125;
		} else if (new_pkt_len > 510) {
			tx_hwagg = 63;
			edca_val = 0x1125;
		} else {
			tx_hwagg = 36;
			edca_val = 0x1125;
		}
	} else if (sta_num == 20) {
		if (new_pkt_len > 1500) {
			tx_hwagg = 72;
			edca_val = 0x1104;
		} else {
			tx_hwagg = 35;
			edca_val = 0x1123;
		}
	} else if (sta_num == 40) {
		if (new_pkt_len > 1500) {
			tx_hwagg = 72;
			edca_val = 0x1104;
		} else {
			tx_hwagg = 35;
			edca_val = 0x1123;
		}
	}
	if (sta_num == 5 || sta_num == 10 || sta_num == 20 || sta_num == 40) {
		if (padapter->registrypriv.manual_edca == 0)
			padapter->registrypriv.manual_edca = 1;
		rtl_wfo_ipc_set_mib(padapter, RTL_WFO_DOE_ID, "max_agg_num", tx_hwagg);
		rtw_hw_set_edca(padapter, 0, edca_val);
	}
}

void apply_pe_amsdu_para(_adapter *padapter)
{
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);
	u8 sta_num, amsdu_min = 0, pe_id;
	u32 amsdu_to = 0;

	sta_num = get_dvobj_total_sta_num(pdvobj);
	if (padapter->vw_enable == VW_MODE_LATENCY) {
		if (sta_num == 16) {
			amsdu_to = 4;
			for (pe_id = 0; pe_id < MAX_PE_NUM; pe_id++)
				rtl_wfo_ipc_set_mib(padapter, pe_id, "amsdu_to", amsdu_to);
		}
	} else if (padapter->vw_enable == VW_MODE_TP) {
		if (sta_num == 5) {
			amsdu_to = 30000;
			amsdu_min = 128;
		} else if (sta_num == 10) {
			amsdu_to = 40000;
			amsdu_min = 94;
		} else if (sta_num == 20) {
			amsdu_to = 64000;
			amsdu_min = 74;
		} else if (sta_num == 40) {
			amsdu_to = 30000;
			amsdu_min = 50;
		}
		if (amsdu_to > 0 && amsdu_min > 0) {
			rtl_wfo_ipc_set_mib(padapter, RTL_WFO_AOE_ID, "amsdu_to", amsdu_to);
			rtl_wfo_ipc_set_mib(padapter, RTL_WFO_AOE_ID, "amsdu_min", amsdu_min);
			rtl_wfo_ipc_set_mib(padapter, RTL_WFO_AOE_ID, "amsdu_spkt_max", 40);
		} else if (sta_num == 1) {
			rtl_wfo_ipc_set_mib(padapter, RTL_WFO_AOE_ID, "amsdu_spkt_max", 40);
		}
	}
}

void apply_pe_tx_force_para(_adapter *padapter)
{
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	struct stainfo_stats *pstats;
	_list *plist, *phead;
	u16 new_pkt_len = 0;
	u8 i, sta_num, pkt_len_changed = 0;
	rtl_wfo_priv_t *wfo_priv = pdvobj->wfo_priv;

	sta_num = get_dvobj_total_sta_num(pdvobj);
	for (i = 0; i < NUM_STA; i++) {
		phead = &(pstapriv->sta_hash[i]);
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
			psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
			plist = get_next(plist);

			if (!psta || (psta == padapter->self_sta) ||
				psta->rps_pe_id >= RTL_WFO_ARM_ID)
				continue;

			pstats = &psta->sta_stats;
			if (pstats->last_tx_pkt_len != pstats->tx_pkt_len)
				pkt_len_changed++;
		}
	}
	if ((padapter->vw_enable == VW_MODE_TP ||
		padapter->vw_enable == VW_MODE_LATENCY) && pkt_len_changed == sta_num) {
		for (i = 0; i < NUM_STA; i++) {
			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);

				if (!psta || (psta == padapter->self_sta) ||
					psta->rps_pe_id >= RTL_WFO_ARM_ID)
					continue;

				pstats = &psta->sta_stats;
				if (pstats->last_tx_pkt_len != pstats->tx_pkt_len)
					pstats->last_tx_pkt_len = pstats->tx_pkt_len;
				if (new_pkt_len == 0)
					new_pkt_len = pstats->tx_pkt_len;
			}
		}
		if (padapter->vw_enable == VW_MODE_TP)
			apply_pe_tc_tput_param(padapter, sta_num, new_pkt_len);
		else if (padapter->vw_enable == VW_MODE_LATENCY)
			apply_pe_tc_latency_param(padapter, sta_num, new_pkt_len);
	}
}
#endif

