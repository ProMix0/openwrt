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
#ifndef _RTL_WFO_PEHT_H_
#define _RTL_WFO_PEHT_H_

#define HEARTBEAT_MON_INTERVAL_MS	(1000)
/* PE update heartbeat every 100ms */
#define HB_DELTA_RANGE (HEARTBEAT_MON_INTERVAL_MS/100/2)
#define HB_DELTA(a,b) ((a>b)?a-b:b-a)


typedef enum {
	RTL_WFO_EVENT_MSG_RSVD = 0,
	RTL_WFO_EVENT_ALLOC_RES,
	RTL_WFO_EVENT_START,
	RTL_WFO_EVENT_RESTART,
	RTL_WFO_EVENT_STOP,
	RTL_WFO_EVENT_RESET_PE,
	RTL_WFO_EVENT_PE_STATE_CHANGED,
	RTL_WFO_EVENT_RESTART_PE_WFO,
	RTL_WFO_EVENT_SYNC_STA_PE_WFO,
	RTL_WFO_EVENT_DUMP_PE_LOG,
	RTL_WFO_EVENT_REBOOT,
} rtl_wfo_event_e;

int rtl_wfo_genl_event_init(void);
void rtl_wfo_genl_event_deinit(void);
void rtl_wfo_notify_event(u32 event_id);
void rlt_wfo_hb_timer_hdl(void *param);
void rtl_wfo_remap_hb_pe_vaddr(struct dvobj_priv *dvobj);
#endif
