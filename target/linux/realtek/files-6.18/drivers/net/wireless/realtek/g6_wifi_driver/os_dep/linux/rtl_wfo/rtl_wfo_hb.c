#define _RTL_WFO_HB_C_

#include <linux/sched.h>
#include <linux/kmod.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#include <drv_types.h>

static int monitor_pid = -1;

/* Generic Netlink Family Name */
#define RTL_WFO_GENL_EVENT			"WFO_EVENT"
#define RTL_WFO_GENL_EVENT_GROUP	"WFO_EVENT_GRP"

enum {
	RTL_WFO_EVENT_ATTR_UNSPEC,
	RTL_WFO_EVENT_ATTR_MSG,
	RTL_WFO_EVENT_ATTR_MAX,
};
#define RTL_WFO_EVENT_ATTR_MAX_NUM (RTL_WFO_EVENT_ATTR_MAX - 1)

enum {
	RTL_WFO_CMD_UNSPEC,
	RTL_WFO_CMD_EVENT,
	RTL_WFO_CMD_MAX,
};
#define RTL_WFO_CMD_MAX_NUM (RTL_WFO_CMD_MAX - 1)

static int rtl_wfo_event_recv(struct sk_buff *skb, struct genl_info *info);
/* genl command ops definition */
static struct genl_ops rtl_wfo_event_ops[] = {
	{
		.cmd = RTL_WFO_CMD_EVENT,
		.flags = 0,
		.doit = rtl_wfo_event_recv,
	},
};

/* genl group definition */
static struct genl_multicast_group rtl_wfo_event_indicate_grp[] = {
	{
		.name = RTL_WFO_GENL_EVENT_GROUP,
	},
};

/* genl family definition */
static struct genl_family rtl_wfo_genl_event_family = {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	.id = GENL_ID_GENERATE,
	#endif
	.hdrsize = 0,
	.name = RTL_WFO_GENL_EVENT,
	.version = 1,
	.maxattr = RTL_WFO_EVENT_ATTR_MAX_NUM,
	.ops = rtl_wfo_event_ops,
	.n_ops = ARRAY_SIZE(rtl_wfo_event_ops),
	.mcgrps = rtl_wfo_event_indicate_grp,
	.n_mcgrps = ARRAY_SIZE(rtl_wfo_event_indicate_grp),
};

static int get_genl_grp_idx_by_name(struct genl_family *family, const char *name){
	int i = 0;

	for (i=0; i < family->n_mcgrps; i++){
		RTW_DBG("group %d name = %s\n", i, (family->mcgrps+i)->name);
		if(!strcmp((family->mcgrps+i)->name, name))
			return i;
	}
	return -1;
}

static int rtl_wfo_event_send(int event_id, char *data, int data_len)
{
	struct sk_buff *skb;
	int rc = 0, grp_idx = -1;
	void *head = NULL;
	size_t genl_payload = 0;
	char netlink_data[128] = {0};
	int gfp_flag = GFP_ATOMIC;

	RTW_DBG("Enter %s \n", __func__);

	memcpy(netlink_data, &event_id, sizeof(int));
	memcpy(netlink_data + sizeof(int), data, data_len);
	netlink_data[sizeof(int) + data_len] = '\0';
	genl_payload = nla_total_size(sizeof(netlink_data)); /* total length of attribute including padding */
	RTW_DBG("[%s] genl payload len=%zu\n", __func__, genl_payload);

	/* create a new netlink msg */
	skb = genlmsg_new(genl_payload, gfp_flag);
	if (!skb) {
		RTW_INFO("Failed to alloc skb\n");
		return -ENOMEM;
	}

	/* Add a new netlink message to an skb */
	genlmsg_put(skb, 0, 0, &rtl_wfo_genl_event_family, gfp_flag, RTL_WFO_CMD_EVENT);

	/* add a netlink attribute to a socket buffer */
	if ((rc = nla_put(skb, RTL_WFO_EVENT_ATTR_MSG, data_len+sizeof(int), netlink_data)) != 0) {
		RTW_INFO("nla_put fail\n");
		goto nlmsg_fail;
	}

	head = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));
	genlmsg_end(skb, head);

	grp_idx = get_genl_grp_idx_by_name(&rtl_wfo_genl_event_family, RTL_WFO_GENL_EVENT_GROUP);
	if (-1 == grp_idx){
		RTW_INFO("get group offset Failed(%s)\n", RTL_WFO_GENL_EVENT_GROUP);
		rc = -EINVAL;
		goto nlmsg_fail;
	}

	//RTW_DBG"Sent to group (%d) message\n", grp_idx);
	rc = genlmsg_multicast(&rtl_wfo_genl_event_family, skb, 0, grp_idx, gfp_flag); // for first group
	if (rc < 0) {
		RTW_INFO("Failed to multicast skb: ret=%d\n", rc);
		return rc;
	}

	return 0;

nlmsg_fail:
	genlmsg_cancel(skb, head);
	nlmsg_free(skb);

	return rc;
}

void rtl_wfo_notify_event(u32 event_id)
{
	if (monitor_pid <= 0)
		return;

	switch (event_id) {
	case RTL_WFO_EVENT_RESET_PE:
		rtl_wfo_event_send(event_id, "[RTL_WFO] PE RESET!", strlen("[RTL_WFO] PE RESET!"));
		break;
	case RTL_WFO_EVENT_DUMP_PE_LOG:
		rtl_wfo_event_send(event_id, "[RTL_WFO] DUMP PE INFO!", strlen("[RTL_WFO] DUMP PE INFO!"));
		break;
	case RTL_WFO_EVENT_REBOOT:
		rtl_wfo_event_send(event_id, "[RTL_WFO] System Reboot!", strlen("[RTL_WFO] System Reboot!"));
		break;
	case RTL_WFO_EVENT_STOP:
		rtl_wfo_event_send(event_id, "[RTL WFO] PE STOP!", strlen("[RTL WFO] PE STOP!"));
		break;
	default:
		RTW_ERR("unknown or unused wfo event %d!!\n", event_id);
	}
}

static int rtl_wfo_event_recv(struct sk_buff *skb, struct genl_info *info)
{
	struct nlmsghdr *nlh = NULL;
	struct genlmsghdr *genlhdr = NULL;
	struct nlattr *nla = NULL;
	int i = 0, nla_len = 0;

	RTW_DBG("Enter %s\n", __func__);

	if (skb == NULL) {
		RTW_ERR("skb is NULL\n");
		return 0;
	}
	nlh = (struct nlmsghdr *)skb->data;

	RTW_DBG("%s: received message from pid %d: %s\n", __func__, nlh->nlmsg_pid, (char *)NLMSG_DATA(nlh));

	monitor_pid = nlh->nlmsg_pid;

	genlhdr = nlmsg_data(nlh);
	if (rtl_wfo_genl_event_family.hdrsize) {
		RTW_ERR("[%s:%d] parser user specific header heae.\n", __func__, __LINE__);
	}

	nla = genlmsg_data(genlhdr) + rtl_wfo_genl_event_family.hdrsize;
	nla_len = genlmsg_len(genlhdr) - rtl_wfo_genl_event_family.hdrsize; 		  //len of attributes

	RTW_DBG("%s: received message from pid %d, genl_cmd=%d\n", __func__, nlh->nlmsg_pid, genlhdr->cmd);
	for (i = 0; nla_ok(nla, nla_len); nla = nla_next(nla, &nla_len), ++i) {
		RTW_DBG("%s: [%d] nla_type=%d nla data=%s\n", __func__, i, nla->nla_type, (char *)nla_data(nla));
	}

	return 1;
}

int rtl_wfo_genl_event_init(void)
{
	int ret = 0;

	monitor_pid = -1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)) && !defined(CPTCFG_VERSION)
	ret = genl_register_family_with_ops_groups(&rtl_wfo_genl_event_family,
							rtl_wfo_event_ops, rtl_wfo_event_indicate_grp);
#else
	ret = genl_register_family(&rtl_wfo_genl_event_family);
#endif
	if (0 != ret) {
		RTW_ERR("[%s] GENL register failed (%d) !! \n", __func__, ret);
	}

	RTW_INFO("%s: register GENL %s successfully\n", __func__, RTL_WFO_GENL_EVENT);
	return 0;
}

void rtl_wfo_genl_event_deinit(void)
{
	genl_unregister_family(&rtl_wfo_genl_event_family);
	RTW_INFO("%s: unregister GENL %s successfully\n", __func__, RTL_WFO_GENL_EVENT);
}

void rtl_wfo_remap_hb_pe_vaddr(struct dvobj_priv *dvobj)
{
	rtl_wfo_priv_t *wfo_priv = dvobj->wfo_priv;
	u32 i, *ptr;
	rtl_wfo_pe_info_t *pe_info;

	for (i = 0; i < MAX_PE_NUM; i++) {
		if (wfo_priv->pe_info[i].state != RTL_WFO_PE_ACTIVE)
			continue;
		pe_info = &(wfo_priv->pe_info[i]);

		if (pe_info->pe_priv == NULL) {
			if (pe_info->pe_priv_addr >= 0x90000000) {
				ptr = ioremap(pe_info->pe_priv_addr, 8);
				if (ptr) {
					RTW_DBG("WFO: PE%d priv addr 0x%08x size 8: %px\n",
						i, pe_info->pe_priv_addr, ptr);
					pe_info->pe_priv = ptr;
				} else {
					RTW_ERR("WFO: fail to ioremap PE%d priv addr 0x%08x\n",
						i, pe_info->pe_priv_addr);
				}
			} else {
				RTW_ERR("WFO: unknown PE%d priv_addr %08x\n",
					i, pe_info->pe_priv_addr);
				break;
			}
		}
	}
}

void rlt_wfo_hb_timer_hdl(void *param)
{
	struct dvobj_priv *devobj = (struct dvobj_priv *)param;
	rtl_wfo_priv_t *wfo_priv = devobj->wfo_priv;
	_adapter *padapter = dvobj_get_primary_adapter(devobj);
	u32 i, *ptr, reset_pe = 0, is_hangup = 0;
	rtl_wfo_pe_info_t *pe_info;

	if (!wfo_priv)
		return;

	for (i = 0; i < MAX_PE_NUM; i++) {
		if (padapter->registrypriv.wifi_mib.wfo_pe_rcvy &&
				wfo_priv->pe_info[i].state == RTL_WFO_PE_RESET) {
			RTW_ERR("[WFO] Restart PE fail! Starting sytem reboot now....\n");
			rtl_wfo_notify_event(RTL_WFO_EVENT_REBOOT);
			return;
		}

		if (wfo_priv->pe_info[i].state != RTL_WFO_PE_ACTIVE)
			continue;

		pe_info = &(wfo_priv->pe_info[i]);
		ptr = pe_info->pe_priv;

		if (!ptr) {
			RTW_ERR("[WFO] PE(%d) pe_info->pe_priv is NULL!!", i);
			return;
		}

		RTW_DBG("pe_state %d heartbeat %d %d monitor_pid %d\n",
			ptr[0], ptr[1], pe_info->last_hb, monitor_pid);

		if (!wfo_priv->hb_hangup_check) {
			if (HB_DELTA(ptr[1], pe_info->last_hb) <= HB_DELTA_RANGE &&
					ptr[0] == PE_START_WFO)
				is_hangup = 1;
			pe_info->last_hb = ptr[1];
		} else {
			if (ptr[1] != pe_info->last_hb)
				pe_info->last_hb = ptr[1];
			else if (ptr[0] == PE_START_WFO) {
				reset_pe = 1;
				wfo_priv->pe_info[i].state = RTL_WFO_PE_RESET;
			}
		}
	}

	/* In checking round, if all PEs hb continuously update, change back to normal round */
	if (wfo_priv->hb_hangup_check && !reset_pe)
		wfo_priv->hb_hangup_check = 0;

	/* Mark next round to checking round */
	if (is_hangup)
		wfo_priv->hb_hangup_check = 1;

	/* Detect PE hangup */
	if (reset_pe) {
		if (padapter->registrypriv.wifi_mib.wfo_pe_rcvy) {
			wfo_priv->res_state = PE_RES_ALLOC;
			RTW_DBG("[RTL_WFO] PE Hangup! Restarting...\n");
			/* trigger vboot to restart WFO PEs */
			rtl_wfo_notify_event(RTL_WFO_EVENT_RESET_PE);
			wfo_priv->hb_hangup_check = 0;

			return;
		} else {
			rtl_wfo_notify_event(RTL_WFO_EVENT_DUMP_PE_LOG);
		}
	}

	/* Restart interfaces and stop PE */
	if (devobj->wfo_trg_stop_pe) {
		for (i = 0; i < BAND_CAP_BIT_NUM; i++) {
			if (wfo_priv->dev[i].dvobj &&
					((wfo_priv->wfo_band_cap >> i) & BIT0)) {
				wfo_priv->dev[i].dvobj->wfo_trg_stop_pe = 0;
				wfo_priv->dev[i].dvobj->wfo_pe_stopped = 1;
			}
		}
		rtl_wfo_notify_event(RTL_WFO_EVENT_STOP);
	}

	_set_timer(&wfo_priv->hb_timer, HEARTBEAT_MON_INTERVAL_MS);
}

