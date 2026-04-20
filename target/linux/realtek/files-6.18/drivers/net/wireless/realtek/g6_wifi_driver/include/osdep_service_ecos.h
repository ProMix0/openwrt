/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#ifndef __OSDEP_ECOS_SERVICE_H_
#define __OSDEP_ECOS_SERVICE_H_

#include <stdio.h>
#include <sys/types.h>
#include <asm-generic/io.h>
#include <asm-generic/pci-dma-compat.h>
#include <asm-generic/pci_iomap.h>
#include <asm-generic/uaccess.h>
#include <asm-generic/barrier.h>
#include <asm-generic/checksum.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/timekeeping.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/gfp.h>
#include <linux/wireless.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/workqueue.h>
#include <linux/byteorder/generic.h>
#include <linux/nl80211.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <linux/if_pppox.h>
#include <linux/icmpv6.h>
#include <linux/ipv6.h>
#include <linux/pci.h>
#include <linux/random.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/atalk.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/jiffies.h>
#include <linux/preempt.h>
#include <linux/pci_regs.h>
#include <linux/fs.h>
#include <linux/smp.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include <net/net_namespace.h>
#include <net/rtnetlink.h>
#include <net/iw_handler.h>
#include <net/regulatory.h>
#include <net/ieee80211_radiotap.h>
#include <net/ipx.h>
#include <net/ip6_checksum.h>
#include <cyg/kernel/kapi.h>
#include <cyg/hal/drv_api.h>
#include <cyg/libc/signals/signal.h>

#include <asm/unaligned.h>
#include <linux/timekeeping.h>

#include "if_ether.h"
#include "../../wfo_virt/trx/rtl8198d/rtl8198d_pseudo_trx.h"

struct wlan_to_nic_debug_count{
	unsigned long long send_success;
	unsigned long long send_fail;
};

extern struct wlan_to_nic_debug_count wlan_to_nic_debug_cnt[6];
unsigned long long extport_abnormal;
unsigned long long total_send_fail;
unsigned long long total_send_success;

#if (RTK_MPOOL_DBG==0)
static inline void *_rtw_vmalloc(u32 sz)
{
	void *pbuf;

	pbuf = vmalloc(sz);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

static inline void *_rtw_zvmalloc(u32 sz)
{
	void *pbuf;

	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);

	return pbuf;
}
#else /* RTK_MPOOL_DBG==1 */
#define _rtw_vmalloc(sz)	vmalloc((sz))
#define _rtw_zvmalloc(sz)	vzalloc((sz))
#endif /* RTK_MPOOL_DBG */

static inline void _rtw_vmfree(void *pbuf, u32 sz)
{
	vfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
}

#if (RTK_MPOOL_DBG==0)
static inline void *_rtw_malloc(u32 sz)
{
	void *pbuf = NULL;

	pbuf = kmalloc(sz, ___GFP_MPOOL_NOTSKB);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

static inline void *_rtw_zmalloc(u32 sz)
{
	/*kzalloc in KERNEL_VERSION(2, 6, 14)*/
	void *pbuf = _rtw_malloc(sz);

	if (pbuf != NULL)
		memset(pbuf, 0, sz);

	return pbuf;
}
#else /* RTK_MPOOL_DBG==1 */
#define _rtw_malloc(sz)		kmalloc((sz), ___GFP_MPOOL_NOTSKB)
#define _rtw_zmalloc(sz)	kzalloc((sz), ___GFP_MPOOL_NOTSKB)
#endif /* RTK_MPOOL_DBG */

static inline void _rtw_mfree(void *pbuf, u32 sz)
{
	kfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */

}

/*lock - spinlock*/
typedef	spinlock_t _lock;
static inline void _rtw_spinlock_init(_lock *plock)
{
	spin_lock_init(plock);
}

static inline void _rtw_spinlock_free(_lock *plock)
{
}

static inline void _rtw_spinlock(_lock *plock)
{
#ifdef WKARD_98D
	spin_lock_bh(plock);
#else
	spin_lock(plock);
#endif
}

static inline void _rtw_spinunlock(_lock *plock)
{
#ifdef WKARD_98D
	spin_unlock_bh(plock);
#else
	spin_unlock(plock);
#endif
}

__inline static void _rtw_spinlock_irq(_lock *plock, unsigned long *flags)
{
	spin_lock_irqsave(plock, *flags);
}

__inline static void _rtw_spinunlock_irq(_lock *plock, unsigned long *flags)
{
	spin_unlock_irqrestore(plock, *flags);
}

__inline static void _rtw_spinlock_bh(_lock *plock)
{
	spin_lock_bh(plock);
}

__inline static void _rtw_spinunlock_bh(_lock *plock)
{
	spin_unlock_bh(plock);
}


/*lock - semaphore*/
struct wfo_sem_t
{
    cyg_count32         count;          /* The semaphore count          */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */
	char padding[8];
};

typedef struct wfo_sem_t _sema;
static inline void _rtw_init_sema(_sema *sema, int init_val)
{
	cyg_semaphore_init((cyg_sem_t*)sema, init_val);
}
static inline void _rtw_free_sema(_sema *sema)
{
	cyg_semaphore_destroy((cyg_sem_t*)sema);
}
static inline void _rtw_up_sema(_sema *sema)
{
	cyg_semaphore_post((cyg_sem_t*)sema);
}
static inline u32 _rtw_down_sema(_sema *sema)
{
	return cyg_semaphore_wait((cyg_sem_t*)sema);
}

/*lock - mutex*/
typedef cyg_mutex_t _mutex;
static inline void _rtw_mutex_init(_mutex *pmutex)
{
	cyg_mutex_init(pmutex);
}

static inline void _rtw_mutex_free(_mutex *pmutex)
{
	cyg_mutex_destroy(pmutex);
}
__inline static int _rtw_mutex_lock_interruptible(_mutex *pmutex)
{
	// 1) cyg_mutex_lock will return true after claiming the mutex lock
	//    Its convention is different from Linux/mutex_lock_interruptible.
	// 2) mutex_lock_interruptible return 0 if the mutex has been acquired.
	if (cyg_mutex_lock(pmutex))
		return 0;
	else
		return 1;
}

__inline static int _rtw_mutex_trylock(_mutex *pmutex)
{
	// 1) cyg_mutex_trylock will return true after claiming the mutex lock
	// 2) mutex_trylock returns 1 if the mutex has been acquired successfully,
	//    and 0 on contention.
	if (cyg_mutex_trylock(pmutex))
		return 1;
	else
		return 0;
}

__inline static int _rtw_mutex_lock(_mutex *pmutex)
{
	if (cyg_mutex_lock(pmutex))
		return 0;
	else
		return 1;
}

__inline static void _rtw_mutex_unlock(_mutex *pmutex)
{
	if (!pmutex->locked) {
		printk("%s(): unlock mutex without lock!! bypass!!\n", __func__);
		return;
	}
	cyg_mutex_unlock(pmutex);
}

/*completion*/
typedef struct completion _completion;
static inline void _rtw_init_completion(_completion *comp)
{
	cyg_flag_init(&(comp->flag));
}
static inline unsigned long _rtw_wait_for_comp_timeout(_completion *comp, unsigned long timeout)
{
	return cyg_flag_timed_wait(&(comp->flag), 0x01, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, cyg_current_time()+timeout);
}
static inline void _rtw_wait_for_comp(_completion *comp)
{
	cyg_flag_wait(&(comp->flag), 0x01, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
}

struct	__queue	{
	struct	list_head	queue;
	_lock	lock;
	int qlen;
};

typedef unsigned char	_buffer;

typedef struct	__queue	_queue;


/*list*/
#define LIST_CONTAINOR(ptr, type, member) \
	((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


typedef struct	list_head	_list;
/* Caller must check if the list is empty before calling rtw_list_delete*/
__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

__inline static _list *get_next(_list	*list)
{
	return list->next;
}
__inline static _list	*get_list_head(_queue *queue)
{
	return &(queue->queue);
}
#define rtw_list_first_entry(ptr, type, member) list_first_entry(ptr, type, member)

/* hlist */
typedef struct	hlist_head	rtw_hlist_head;
typedef struct	hlist_node	rtw_hlist_node;
#define rtw_hlist_for_each_entry(pos, head, member) hlist_for_each_entry(pos, head, member)
#define rtw_hlist_for_each_safe(pos, n, head) hlist_for_each_safe(pos, n, head)
#define rtw_hlist_entry(ptr, type, member) hlist_entry(ptr, type, member)
#define rtw_hlist_for_each_entry_safe(pos, np, n, head, member) hlist_for_each_entry_safe(pos, np, n, head, member)
#define rtw_hlist_for_each_entry_rcu(pos, node, head, member) hlist_for_each_entry_rcu(pos, node, head, member)

/* RCU */
typedef struct rcu_head rtw_rcu_head;
#define rtw_rcu_dereference(p) rcu_dereference((p))
#define rtw_rcu_dereference_protected(p, c) rcu_dereference_protected(p, c)
#define rtw_rcu_assign_pointer(p, v) rcu_assign_pointer((p), (v))
#define rtw_rcu_read_lock() rcu_read_lock()
#define rtw_rcu_read_unlock() rcu_read_unlock()

/* rhashtable */
#include "../os_dep/linux/rtw_rhashtable.h"
#include "../os_dep/linux/rtw_hashtable.h"

/*thread*/
//typedef void *_thread_hdl_;
typedef int thread_return;
typedef void *thread_context;
struct thread_hdl{
	_thread_hdl_ thread_handler;
	u8 thread_status;
};
#define THREAD_STATUS_STARTED BIT(0)
#define THREAD_STATUS_STOPPED BIT(1)
#define RST_THREAD_STATUS(t) (t->thread_status = 0)
#define SET_THREAD_STATUS(t, s) 	(t->thread_status |= s)
#define CLR_THREAD_STATUS(t, cl)	(t->thread_status &= ~(cl))
#define CHK_THREAD_STATUS(t, ck) (t->thread_status & ck)

typedef void timer_hdl_return;
typedef void *timer_hdl_context;

static inline void rtw_thread_enter(char *name)
{
	allow_signal(SIGTERM);
}

static inline void rtw_thread_exit(_completion *comp)
{
	complete_and_exit(comp, 0);
}

static inline _thread_hdl_ rtw_thread_start(int (*threadfn)(void *data),
			void *data, const char namefmt[])
{
	_thread_hdl_ _rtw_thread = NULL;

	_rtw_thread = kthread_run(threadfn, data, namefmt);
	if (IS_ERR(_rtw_thread)) {
		WARN_ON(!_rtw_thread);
		_rtw_thread = NULL;
	}
	return _rtw_thread;
}
static inline bool rtw_thread_stop(_thread_hdl_ th)
{

	return kthread_stop(th);
}
static inline void rtw_thread_wait_stop(void)
{
	#if 0
	while (!kthread_should_stop())
		rtw_msleep_os(10);
	#else
	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		//schedule();
		cyg_thread_yield();
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
	#endif
}

static inline void flush_signals_thread(void)
{
	if (signal_pending(current))
		flush_signals(current);
}


typedef unsigned long systime;

/*tasklet*/
#ifndef CONFIG_RTW_OS_HANDLER_EXT
struct wfo_tasklet_struct
{
	struct tasklet_struct *next;
	unsigned long state;
	atomic_t count;
	void (*func)(unsigned long);
	unsigned long data;

#if (PHL_VER_CODE > PHL_VERSION(0001, 0015, 0000, 0000))
	u8 pad[36];
#endif
};
typedef struct wfo_tasklet_struct _tasklet;
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
typedef void (*tasklet_fn_t)(unsigned long);

#ifdef CONFIG_RTW_OS_HANDLER_EXT
enum rtw_handler_state {
	RTW_HDL_STATE_SCHED	/* scheduled for execution */
};

#define RTW_OS_HANDLER_TASKLET	(0)
#define RTW_OS_HANDLER_WORK	(1)

struct rtw_os_handler {
	/* tasklet and work should be first for easier migratiion 
	 * to OS handler model. */
	union {
		struct tasklet_struct task;
		struct work_struct work;
	};
	/* Data specific to tasklet and work */
	union {
		struct {
			tasklet_fn_t func;
			unsigned long data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
			struct __call_single_data csd;
#else
			struct call_single_data csd;
#endif
			unsigned long state;
		} task_data;
		struct {
			void *func;
			void *cntx;
			struct workqueue_struct *wq;
		} work_data;
	};
	/* Common data for OS handler management */
	u8		init;
	u8		type; /* 0: tasklet, 1: work */
	u8		id;
	u8		cpu_id;
	const char 	*name;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	u32		run_on_cpu;
	u32		run;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
};

typedef struct rtw_os_handler _tasklet;
typedef struct rtw_os_handler _workitem;

/* Core API for PHL handler */
struct rtw_phl_handler;
u8 rtw_plfm_init_handler_ext(void *drv_priv,
                             struct rtw_phl_handler *phl_handler);

u8 rtw_plfm_deinit_handler_ext(void *drv_priv,
                               struct rtw_phl_handler *phl_handler);

/* Core API for general OS handler */
struct dvobj_priv;
void rtw_init_os_handler(struct dvobj_priv *dvobj, struct rtw_os_handler *handler);
void rtw_deinit_os_handler(struct dvobj_priv *dvobj, struct rtw_os_handler *handler);

/* List of managed handlers */
enum rtw_core_handler_id {
	RTW_HANDLER_CORE_ONE_TX,
	RTW_HANDLER_CORE_SWQ_TX,
	RTW_HANDLER_CORE_NETIF_RX,
	RTW_HANDLER_CORE_RX_RECYCLE,
	RTW_CORE_HANDLER_NUM,
	/* PHL hanlers */
	RTW_HANDLER_PHL_TX = RTW_CORE_HANDLER_NUM,
	RTW_HANDLER_PHL_RX,
	RTW_HANDLER_PHL_EVT,
	RTW_HANDLER_PHL_SER,
	#ifdef CONFIG_PHL_TEST_SUITE
        RTW_HANDLER_TEST,
	#endif
        RTW_HANDLER_NUM_MAX
};
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

/* tasklet functions */
static inline void rtw_tasklet_init(_tasklet *t, tasklet_fn_t func,
                                    unsigned long data)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	/* Init for un-managed tasklet */
	if (t->init == 0) {
		struct rtw_os_handler *os_handler = (struct rtw_os_handler *)t;
		t->task_data.csd.func = NULL;
		t->cpu_id = WORK_CPU_UNBOUND;
		t->type = RTW_OS_HANDLER_TASKLET;
		t->init = 1;
		t->id = RTW_HANDLER_NUM_MAX;
	}
	/* Always save tasklet function and custom data for PHL use model */
	t->task_data.func = func;
	t->task_data.data = data;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run_on_cpu = 0;
	t->run = 0;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */

#endif /* CONFIG_RTW_OS_HANDLER_EXT */
	tasklet_init((struct tasklet_struct *)t, func, data);
}

static inline void rtw_tasklet_kill(_tasklet *t)
{
	tasklet_kill((struct tasklet_struct *)t);
}

static inline void rtw_tasklet_schedule(_tasklet *t)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if ((t->task_data.csd.func != NULL) && (t->cpu_id != WORK_CPU_UNBOUND)) {
		if (!test_and_set_bit(RTW_HDL_STATE_SCHED, &t->task_data.state)) {
			//smp_call_function_single_async(t->cpu_id, &t->task_data.csd);
			#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
			t->run_on_cpu++;
			#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
		}

		return;
	}
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run++;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

	tasklet_schedule((struct tasklet_struct *)t);
}

static inline void rtw_tasklet_hi_schedule(_tasklet *t)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if ((t->task_data.csd.func != NULL) && (t->cpu_id != WORK_CPU_UNBOUND)) {
		if (!test_and_set_bit(RTW_HDL_STATE_SCHED, &t->task_data.state)) {
			#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
			t->run_on_cpu++;
			#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
			//smp_call_function_single_async(t->cpu_id, &t->task_data.csd);
		}

		return;
	}
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run++;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

	tasklet_hi_schedule((struct tasklet_struct *)t);
}

/*skb_buffer*/
static inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
#ifdef WKARD_98D
	return __dev_alloc_skb(sz, GFP_ATOMIC);
#else
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif
}

static inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

static inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline int _rtw_skb_linearize(struct sk_buff *skb)
{
	return skb_linearize(skb);
}

static inline struct sk_buff *_rtw_pskb_copy(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline u8 *rtw_skb_data(struct sk_buff *pkt)
{
	return pkt->data;
}

static inline u32 rtw_skb_len(struct sk_buff *pkt)
{
	return pkt->len;
}

static inline void *rtw_skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memset(tmp, 0, len);

	return tmp;
}


/*timer*/
#define HZ ((1000000000/CYGNUM_HAL_RTC_NUMERATOR)*CYGNUM_HAL_RTC_DENOMINATOR)
typedef struct rtw_timer_list _timer;
struct rtw_timer_list {
	struct timer_list timer;
	void (*function)(void *);
	void *arg;
};

static inline void timer_hdl(unsigned long cntx)
{
	_timer *ptimer = (_timer *)cntx;

	ptimer->function(ptimer->arg);
}

__inline static void _init_timer(_timer *ptimer, void *pfunc, void *cntx)
{
	ptimer->function = pfunc;
	ptimer->arg = cntx;

	/* setup_timer(ptimer, pfunc,(u32)cntx);	 */
	ptimer->timer.function = timer_hdl;
	ptimer->timer.data = (unsigned long)ptimer;
	init_timer(&ptimer->timer);
}

__inline static void _set_timer(_timer *ptimer, u32 delay_time)
{
	mod_timer(&ptimer->timer , (jiffies + (delay_time * HZ / 1000)));
}

__inline static void _cancel_timer(_timer *ptimer, u8 *bcancelled)
{
	//*bcancelled = del_timer_sync(&ptimer->timer) == 1 ? 1 : 0;
	del_timer_sync(&ptimer->timer);
	*bcancelled = 1;
}

__inline static void _cancel_timer_async(_timer *ptimer)
{
	del_timer(&ptimer->timer);
}

/*work*/
#ifndef CONFIG_RTW_OS_HANDLER_EXT
struct wfo_work_struct
{
	atomic_long_t data;
	struct list_head entry;
	work_func_t func;

#if (PHL_VER_CODE > PHL_VERSION(0001, 0015, 0000, 0000))
	u8 pad[40];
#endif
};
typedef struct wfo_work_struct _workitem;
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

static inline void _init_workitem(_workitem *pwork, void *pfunc, void *cntx)
{
	#ifdef CONFIG_RTW_OS_HANDLER_EXT
	/* Init for un-managed work */
	if (pwork->init == 0) {
		pwork->work_data.wq = NULL;
		pwork->cpu_id = WORK_CPU_UNBOUND;
		pwork->init = 1;
		pwork->id = RTW_HANDLER_NUM_MAX;
	}
	pwork->work_data.func = pfunc;
	pwork->work_data.cntx = cntx;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	pwork->run_on_cpu = 0;
	pwork->run = 0;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	#endif /* CONFIG_RTW_OS_HANDLER_EXT */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
	INIT_WORK((struct work_struct *)pwork, pfunc);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
	INIT_WORK(pwork, pfunc, pwork);
#else
	INIT_TQUEUE(pwork, pfunc, pwork);
#endif
}

__inline static void _set_workitem(_workitem *pwork)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if (pwork->work_data.wq != NULL) {
		queue_work_on(pwork->cpu_id, pwork->work_data.wq,
		              (struct work_struct *)pwork);
		#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
		pwork->run_on_cpu++;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	} else {
		schedule_work((struct work_struct *)pwork);
		#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
		pwork->run++;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	}
#else /* CONFIG_RTW_OS_HANDLER_EXT */
	schedule_work((struct work_struct *)pwork);
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
#else /* LINUX_VERSION_CODE */
	schedule_task(pwork);
#endif /* LINUX_VERSION_CODE */
}

__inline static void _cancel_workitem_sync(_workitem *pwork)
{
	cancel_work_sync((struct work_struct *)pwork);
}
/*
 * Global Mutex: can only be used at PASSIVE level.
 *   */
#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1) { \
			atomic_dec((atomic_t *)&(_MutexCounter));        \
			msleep(10);                          \
		}                                                           \
	}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
	}


typedef	struct	net_device *_nic_hdl;
static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)));
#else
	return netif_queue_stopped(pnetdev);
#endif
}

#ifdef CONFIG_HWSIM
int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb);
#else
static inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
#if defined(APPLY_FC_FASTFWD)
extern int fwdEngine_wifi_rx(struct sk_buff *skb);
enum {
	RE8670_RX_STOP=0,
	RE8670_RX_CONTINUE,
	RE8670_RX_STOP_SKBNOFREE,
	RE8670_RX_END
};
int ret = 0;

	skb->dev = ndev;
	skb->data-=14;
	skb->len+=14;

	ret = fwdEngine_wifi_rx(skb);

	if(ret==RE8670_RX_CONTINUE)
	{
		skb->data+=14;
		skb->len-=14;
	return netif_rx(skb);
}
	else if(ret==RE8670_RX_STOP)
	{
		kfree_skb(skb);
	}

	return 0;
#else
	int err;
	struct net_device *eth_dev = dev_get_by_name(NULL, "eth0");

	if (eth_dev == NULL) {
		printk("%s ERR: Get eth0 netdevice fail\n", __func__);
		goto err;
	}

	skb->dev = ndev;
	err = rtl8198d_wfo_wlandev2portidx(skb->dev,
			&skb->macportid, &skb->macextportid);

	if (err) {
		printk("%s ERR: get extend port id fail\n", __func__);
		goto err;
	}

	//printk("===> Sending SKB from wlan to eth\n");
	err = eth_dev->netdev_ops->ndo_start_xmit(skb, eth_dev);
	if (err != 0) {
		if (skb->macextportid <= 0)
			extport_abnormal++;
		else
			wlan_to_nic_debug_cnt[skb->macextportid-1].send_fail++;
		total_send_fail++;
		return NET_RX_DROP;
	}

	total_send_success++;
	wlan_to_nic_debug_cnt[skb->macextportid-1].send_success++;
	return NET_RX_SUCCESS;
err:
	dev_kfree_skb_any(skb);
	total_send_fail++;
	return NET_RX_DROP;
#endif
}
#endif

#ifdef CONFIG_RTW_NAPI
static inline int _rtw_netif_receive_skb(_nic_hdl ndev, struct sk_buff *skb)
{
	skb->dev = ndev;
	return netif_receive_skb(skb);
}

#ifdef CONFIG_RTW_GRO
static inline gro_result_t _rtw_napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb)
{
	return napi_gro_receive(napi, skb);
}
#endif /* CONFIG_RTW_GRO */
#endif /* CONFIG_RTW_NAPI */

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_wake_all_queues(pnetdev);
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}
static inline void rtw_netif_device_attach(struct net_device *pnetdev)
{
	netif_device_attach(pnetdev);
}
static inline void rtw_netif_device_detach(struct net_device *pnetdev)
{
	netif_device_detach(pnetdev);
}
static inline void rtw_netif_carrier_on(struct net_device *pnetdev)
{
	netif_carrier_on(pnetdev);
}
static inline void rtw_netif_carrier_off(struct net_device *pnetdev)
{
	netif_carrier_off(pnetdev);
}

static inline int rtw_merge_string(char *dst, int dst_len, const char *src1, const char *src2)
{
	int	len = 0;
	len += snprintf(dst + len, dst_len - len, "%s", src1);
	len += snprintf(dst + len, dst_len - len, "%s", src2);

	return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)), (sig), 1)
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
	#define rtw_signal_process(pid, sig) kill_proc((pid), (sig), 1)
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */


/* Suspend lock prevent system from going suspend */
#ifdef CONFIG_WAKELOCK
	#include <linux/wakelock.h>
#endif

/* limitation of path length */
#define PATH_LENGTH_MAX PATH_MAX

/* Atomic integer operations */
#define ATOMIC_T atomic_t
static inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	atomic_set(v, i);
}

static inline int ATOMIC_READ(ATOMIC_T *v)
{
	return atomic_read(v);
}

static inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	atomic_add(i, v);
}
static inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	atomic_sub(i, v);
}

static inline void ATOMIC_INC(ATOMIC_T *v)
{
	atomic_inc(v);
}

static inline void ATOMIC_DEC(ATOMIC_T *v)
{
	atomic_dec(v);
}

static inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	return atomic_add_return(i, v);
}

static inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	return atomic_sub_return(i, v);
}

static inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	return atomic_inc_return(v);
}

static inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	return atomic_dec_return(v);
}

static inline bool ATOMIC_INC_UNLESS(ATOMIC_T *v, int u)
{
	return atomic_add_unless(v, 1, u);
}

#define RTW_SMP_MB()
#define RTW_SMP_RMB()
#define RTW_SMP_WMB()

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adapter) (adapter->pnetdev ? adapter->pnetdev->name : NULL)
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, (adapter->pnetdev ? adapter->pnetdev->name : NULL)
#define DEV_FMT "%s"
#define DEV_ARG(dvobj) \
	({ \
		struct dvobj_priv *d = (dvobj); \
		const char *dev_name; \
		if (d->wiphy) \
			dev_name = wiphy_name(d->wiphy); \
		else \
			dev_name = pci_name(d->pci_data.ppcidev); \
		dev_name; \
	})

#define FUNC_DEV_FMT "%s(%s)"
#define FUNC_DEV_ARG(dvobj)	__func__, DEV_ARG(dvobj)
#define FUNC_DEVID_FMT "%s(%u)"
#define FUNC_DEVID_ARG(dvobj)	__func__, (dvobj)->dev_id

#define rtw_netdev_priv(netdev) (((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv)
struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device *rtw_alloc_etherdev(int sizeof_priv);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(name)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(ndev->nd_net, name)
#else
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(dev_net(ndev), name)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define rtw_get_bridge_ndev_by_name(name) dev_get_by_name(name)
#else
#define rtw_get_bridge_ndev_by_name(name) dev_get_by_name(&init_net, name)
#endif

static inline void rtw_dump_stack(void)
{
	dump_stack();
}
#define rtw_warn_on(condition) WARN_ON(condition)
#define RTW_DIV_ROUND_UP(n, d)	DIV_ROUND_UP(n, d)
#define rtw_sprintf(buf, size, format, arg...) snprintf(buf, size, format, ##arg)

#define STRUCT_PACKED __attribute__ ((packed))

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)) || defined(PLATFORM_ECOS)
typedef struct call_single_data call_single_data_t;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 85))
typedef struct timespec64 _timeval;
#else
typedef struct timeval _timeval;
#endif

static inline void rtw_do_gettimeofday(_timeval *tv)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 85)) && !defined(PLATFORM_ECOS)
	ktime_get_real_ts64(tv);
#else
	do_gettimeofday(tv);
#endif
}

/* Bypass Linux Kernel module related macro */
#define module_param(name, type, perm)
#define module_param_array(name, type, nump, perm)
#define MODULE_PARM_DESC(_parm, desc)
#define MODULE_LICENSE(_license)
#define MODULE_AUTHOR(_author)
#define MODULE_DESCRIPTION(_description)
#define MODULE_VERSION(_version)

#endif /* __OSDEP_ECOS_SERVICE_H_ */
