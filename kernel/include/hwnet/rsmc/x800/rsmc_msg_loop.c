/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This module is an independent thread that receives
 * and sends netlink messages and interrupts message forwarding.
 * Each module needs to register messages with this module.
 * This module accepts external messages and forwards them through
 * a mapping table to the registered module.
 * Author: linlixin2@huawei.com
 * Create: 2020-10-22
 */

#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netlink.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <uapi/linux/netlink.h>

#include "../module_type.h"
#include "rsmc_msg_loop.h"
#include "rsmc_spi_drv.h"
#include "rsmc_test.h"
#include "rsmc_x800_device.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_NETLINK_HANDLE
HWLOG_REGIST();
MODULE_LICENSE("GPL");

enum channel_status {
	NETLINK_MSG_LOOP_EXIT = 0,
	NETLINK_MSG_LOOP_INIT,
};

#ifdef NETLINK_RSMC
#undef NETLINK_RSMC
#endif
#define NETLINK_RSMC 43
#define NETLINK_BUFFER_MAX_SIZE 4100

#define RSMC_HEART_BEAT_TIMEOUT msecs_to_jiffies(5000)

#define DTS_NODE_HUAWEI_RSMC "huawei_rsmc"
#define DTS_PROP_RSMC_ENABLE "rsmc_enable"

static DEFINE_MUTEX(g_recv_mtx);
static DEFINE_MUTEX(g_send_mtx);

/* module IDs defined for each module */
enum install_model {
	X800_DEVICE = 0,
	MODEL_NUM,
};

/* Message list structure */
struct msg_entity {
	struct list_head head;
	struct msg_head msg;
};

#define MAX_NOR_MSG_LEN 100
#define MAX_FST_MSG_LEN 50

/* The context of the message handle */
struct handle_ctx {
	/* Netlink socket fd */
	struct sock *nlfd;

	/* Save user space progress pid when user space netlink registering. */
	unsigned int native_pid;

	/* Tasks for send messages. */
	struct task_struct *up_task;

	/* Tasks for receive messages. */
	struct task_struct *dn_task;

	/* Channel status */
	int chan_state;

	/* Semaphore of the message sent */
	struct semaphore up_sema;

	spinlock_t up_lock;

	struct semaphore dn_sema;

	spinlock_t dn_lock;

	/* Message processing callback functions */
	msg_process *mod_cb[MODEL_NUM];

	struct timer_list heartbeat_timer;

	volatile unsigned long native_jiffies;

	volatile bool heartbeat_running;

	volatile struct list_head *dn_list[MAX_NOR_MSG_LEN + 4]; // add buffer 4

	volatile int dn_end;

	volatile int dn_start;

	volatile struct list_head *up_list[MAX_NOR_MSG_LEN + 4]; // add buffer 4

	volatile int up_end;

	volatile int up_start;

	volatile struct list_head *up_fast[MAX_NOR_MSG_LEN + 4]; // add buffer 4

	volatile int up_endf;

	volatile int up_startf;
};

static struct handle_ctx g_nl_ctx = {0};

/* mesage map entry index */
enum map_index {
	MAP_KEY_INDEX = 0,
	MAP_VALUE_INDEX,
	MAP_ENTITY_NUM,
};

struct model_map {
	enum install_model model;
	model_reg *reg;
	model_unreg *unreg;
};

const static struct model_map module_init_map[MODEL_NUM] = {
	{X800_DEVICE, x800_device_reg, x800_device_unreg},
};

/* Message mapping table for external modules */
const static u16 cmd_module_map[][MAP_ENTITY_NUM] = {
	{CMD_DN_INIT_REQ, X800_DEVICE},
	{CMD_DN_MODE_SET_REQ, X800_DEVICE},
	{CMD_DN_FREQ_OFFSET_EST_REQ, X800_DEVICE},
	{CMD_DN_TX_REQ, X800_DEVICE},
	{CMD_DN_CHN_INIT_REQ, X800_DEVICE},
	{CMD_DN_CHN_CLOSE_REQ, X800_DEVICE},
	{CMD_DN_CHN_ACQ2TRK_REQ, X800_DEVICE},
	{CMD_DN_CHN_TRK_ADJUST_REQ, X800_DEVICE},
	{CMD_DN_SINGLE_CMD_REQ, X800_DEVICE},
	{CMD_DN_FREQ_OFF_REQ, X800_DEVICE},
	{CMD_INTER_INIT_REQ, X800_DEVICE},
	{CMD_INTER_HB_TIMER_REQ, X800_DEVICE}
};

/* msg.module MODULE_TYPE_FAST_CTRL */
void fast_notify_event(struct list_head *list)
{
	int idx = g_nl_ctx.up_endf;

	if (list == NULL)
		return;
	if (idx >= MAX_FST_MSG_LEN || idx < 0) {
		hwlog_err("%s: idx err", __func__);
		return;
	}
	if ((g_nl_ctx.up_startf + MAX_FST_MSG_LEN - idx) % MAX_FST_MSG_LEN == 1) {
		hwlog_err("%s: msg overlap", __func__);
		return;
	}
	g_nl_ctx.up_fast[idx] = list;
	idx = (idx + 1) % MAX_FST_MSG_LEN;
	g_nl_ctx.up_endf = idx;
	barrier();
	if (g_nl_ctx.up_endf != idx)
		hwlog_err("%s: idx not now", __func__);
	up(&g_nl_ctx.up_sema);
}

void rsmc_nl_inter_up_notify_event(struct msg_head *msg)
{
	struct msg_entity *p = NULL;
	u32 msg_len;
	int msg_idx;
	int ret;

	if (msg == NULL)
		return;
	hwlog_info("%s: enter, type:%d,module:%d,len:%d",
			__func__, msg->type, msg->module, msg->len);
	if (g_nl_ctx.chan_state != NETLINK_MSG_LOOP_INIT) {
		hwlog_err("%s: module not inited", __func__);
		return;
	}
	msg_len = sizeof(struct list_head) + msg->len;
	p = kmalloc(msg_len, GFP_ATOMIC);
	if (p == NULL) {
		hwlog_err("%s: kmalloc failed", __func__);
		return;
	}
	ret = memcpy_s(&p->msg, msg->len, msg, msg->len);
	if (ret != EOK)
		return;

	spin_lock(&g_nl_ctx.up_lock);
	msg_idx = g_nl_ctx.up_end;
	if (msg_idx >= MAX_NOR_MSG_LEN || msg_idx < 0) {
		spin_unlock(&g_nl_ctx.up_lock);
		hwlog_err("%s: idx err", __func__);
		return;
	}
	if ((g_nl_ctx.up_start + MAX_NOR_MSG_LEN - msg_idx) % MAX_NOR_MSG_LEN == 1) {
		spin_unlock(&g_nl_ctx.up_lock);
		hwlog_err("%s: msg overlap", __func__);
		return;
	}
	g_nl_ctx.up_list[msg_idx] = (struct list_head *)p;
	msg_idx = (msg_idx + 1) % MAX_NOR_MSG_LEN;
	g_nl_ctx.up_end = msg_idx;
	barrier();
	if (g_nl_ctx.up_end != msg_idx)
		hwlog_err("%s: idx not new", __func__);
	spin_unlock(&g_nl_ctx.up_lock);

	up(&g_nl_ctx.up_sema);

	hwlog_info("%s: exit", __func__);
}

void rsmc_nl_inter_dn_notify_event(struct msg_head *msg)
{
	struct msg_entity *p = NULL;
	u32 msg_len;
	int msg_idx;
	int ret;

	if (msg == NULL)
		return;
	if (msg->len > NETLINK_BUFFER_MAX_SIZE) {
		hwlog_err("%s: invalid msg len", __func__);
		return;
	}
	hwlog_info("%s: enter, type:%d,module:%d,len:%d",
		__func__, msg->type, msg->module, msg->len);
	if (g_nl_ctx.chan_state != NETLINK_MSG_LOOP_INIT) {
		hwlog_err("%s: module not inited", __func__);
		return;
	}
	msg_len = sizeof(struct list_head) + msg->len;
	p = kmalloc(msg_len, GFP_ATOMIC);
	if (p == NULL) {
		hwlog_err("%s: kmalloc failed", __func__);
		return;
	}
	ret = memcpy_s(&p->msg, msg->len, msg, msg->len);
	if (ret != EOK)
		return;

	spin_lock(&g_nl_ctx.dn_lock);
	msg_idx = g_nl_ctx.dn_end;
	if (msg_idx >= MAX_NOR_MSG_LEN || msg_idx < 0) {
		spin_unlock(&g_nl_ctx.dn_lock);
		hwlog_err("%s: idx err", __func__);
		return;
	}
	if ((g_nl_ctx.dn_start + MAX_NOR_MSG_LEN - msg_idx) % MAX_NOR_MSG_LEN == 1) {
		rsmc_clear_dn_msg_list();
		spin_unlock(&g_nl_ctx.dn_lock);
		hwlog_err("%s: msg overlap", __func__);
		return;
	}
	g_nl_ctx.dn_list[msg_idx] = (struct list_head *)p;
	msg_idx = (msg_idx + 1) % MAX_NOR_MSG_LEN;
	g_nl_ctx.dn_end = msg_idx;
	barrier();
	if (g_nl_ctx.dn_end != msg_idx)
		hwlog_err("%s: idx not new", __func__);
	spin_unlock(&g_nl_ctx.dn_lock);
	up(&g_nl_ctx.dn_sema);

	hwlog_info("%s: exit", __func__);
}

void rsmc_nl_inter_dn_notify_event_nolock(struct msg_head *msg)
{
	struct msg_entity *p = NULL;
	u32 msg_len;
	int msg_idx;
	int ret;
	unsigned long flags;

	if (msg == NULL)
		return;
	hwlog_info("%s: enter, type:%d,module:%d,len:%d",
		__func__, msg->type, msg->module, msg->len);
	if (g_nl_ctx.chan_state != NETLINK_MSG_LOOP_INIT) {
		hwlog_err("%s: module not inited", __func__);
		return;
	}
	msg_len = sizeof(struct list_head) + msg->len;
	p = kmalloc(msg_len, GFP_ATOMIC);
	if (p == NULL) {
		hwlog_err("%s: kmalloc failed", __func__);
		return;
	}
	ret = memcpy_s(&p->msg, msg->len, msg, msg->len);
	if (ret != EOK)
		return;
	spin_lock_irqsave(&g_nl_ctx.dn_lock, flags);
	msg_idx = g_nl_ctx.dn_end;
	if (msg_idx >= MAX_NOR_MSG_LEN || msg_idx < 0) {
		spin_unlock_irqrestore(&g_nl_ctx.dn_lock, flags);
		hwlog_err("%s: idx err", __func__);
		return;
	}
	if ((g_nl_ctx.dn_start + MAX_NOR_MSG_LEN - msg_idx) % MAX_NOR_MSG_LEN == 1) {
		rsmc_clear_dn_msg_list();
		spin_unlock_irqrestore(&g_nl_ctx.dn_lock, flags);
		hwlog_err("%s: msg overlap", __func__);
		return;
	}
	g_nl_ctx.dn_list[msg_idx] = (struct list_head *)p;
	msg_idx = (msg_idx + 1) % MAX_NOR_MSG_LEN;
	g_nl_ctx.dn_end = msg_idx;
	barrier();
	if (g_nl_ctx.dn_end != msg_idx)
		hwlog_err("%s: idx not new", __func__);
	spin_unlock_irqrestore(&g_nl_ctx.dn_lock, flags);
	up(&g_nl_ctx.dn_sema);

	hwlog_info("%s: exit", __func__);
}

void rsmc_clear_dn_msg_list(void)
{
	int i, ret;
	for (i = 0; i < MAX_NOR_MSG_LEN; i++) {
		struct msg_entity *msg = (struct msg_entity *)g_nl_ctx.dn_list[i];
		if (msg != NULL)
			kfree(msg);
	}
	ret = memset_s(g_nl_ctx.dn_list, sizeof(struct list_head *) * MAX_NOR_MSG_LEN,
		0, sizeof(struct list_head *) * MAX_NOR_MSG_LEN);
	if (ret != EOK)
		return;
	g_nl_ctx.dn_start = 0;
	g_nl_ctx.dn_end = 0;
}

void rsmc_process_cmd(struct msg_head *cmd)
{
	int i;

	if (cmd == NULL)
		return;
	hwlog_info("%s: enter, type:%d,module:%d",
		__func__, cmd->type, cmd->module);

	if (cmd->module != MODULE_TYPE_KNL) {
		hwlog_err("%s: module not kernel %d", __func__, cmd->module);
		return;
	}
	for (i = 0; i < sizeof(cmd_module_map) / (sizeof(u16) * MAP_ENTITY_NUM); i++) {
		if (cmd_module_map[i][MAP_KEY_INDEX] != cmd->type)
			continue;
		if (g_nl_ctx.mod_cb[cmd_module_map[i][MAP_VALUE_INDEX]] == NULL)
			break;
		g_nl_ctx.mod_cb[cmd_module_map[i][MAP_VALUE_INDEX]](cmd);
		break;
	}

	hwlog_info("%s: exit", __func__);
}

static void rsmc_heartbeat_process(void)
{
	hwlog_info("%s: heartbeat", __func__);
	g_nl_ctx.native_jiffies = (unsigned long)jiffies;
}

static void rsmc_heartbeat_callback(struct timer_list *t)
{
	unsigned long flags;

	if (!g_nl_ctx.heartbeat_running)
		return;
	if (jiffies - g_nl_ctx.native_jiffies > RSMC_HEART_BEAT_TIMEOUT) {
		struct enable_msg msg = {0};

		msg.head.type = CMD_INTER_INIT_REQ;
		msg.head.module = MODULE_TYPE_KNL;
		msg.head.len = sizeof(struct enable_msg);
		msg.status = 0;
		hwlog_info("%s: heartbeat lost disable x800", __func__);
		spin_lock_irqsave(&g_nl_ctx.dn_lock, flags);
		rsmc_clear_dn_msg_list();
		spin_unlock_irqrestore(&g_nl_ctx.dn_lock, flags);
		rsmc_nl_inter_dn_notify_event_nolock((struct msg_head *)&msg);
	} else {
		struct msg_head msg = {0};

		msg.type = CMD_INTER_HB_TIMER_REQ;
		msg.module = MODULE_TYPE_KNL;
		msg.len = sizeof(struct msg_head);
		rsmc_nl_inter_dn_notify_event_nolock((struct msg_head *)&msg);
	}
}

void rsmc_start_heartbeat(void)
{
	if (g_nl_ctx.heartbeat_running) {
		del_timer_sync(&g_nl_ctx.heartbeat_timer);
	} else {
		g_nl_ctx.heartbeat_running = true;
		rsmc_heartbeat_process();
	}
	timer_setup(&g_nl_ctx.heartbeat_timer,
		rsmc_heartbeat_callback,
		TIMER_IRQSAFE);
	g_nl_ctx.heartbeat_timer.expires = jiffies + HZ;
	add_timer(&g_nl_ctx.heartbeat_timer);
	hwlog_info("%s", __func__);
}

void rsmc_stop_heartbeat(void)
{
	if (g_nl_ctx.heartbeat_running) {
		del_timer_sync(&g_nl_ctx.heartbeat_timer);
		g_nl_ctx.heartbeat_running = false;
	}
	hwlog_info("%s", __func__);
}

void rsmc_netlink_handle_rcv(struct sk_buff *__skb)
{
	struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb = NULL;

	if (g_nl_ctx.chan_state != NETLINK_MSG_LOOP_INIT) {
		hwlog_err("%s: module not inited", __func__);
		return;
	}
	if (__skb == NULL) {
		hwlog_err("%s: __skb null", __func__);
		return;
	}
	skb = skb_get(__skb);
	if (skb == NULL) {
		hwlog_err("%s: skb null", __func__);
		return;
	}
	mutex_lock(&g_recv_mtx);
	if (skb->len < (u32)NLMSG_HDRLEN) {
		hwlog_err("%s: skb len error", __func__);
		goto skb_free;
	}
	nlh = nlmsg_hdr(skb);
	if (nlh == NULL) {
		hwlog_err("%s: nlh = NULL", __func__);
		goto skb_free;
	}
	if ((nlh->nlmsg_len < sizeof(struct nlmsghdr)) ||
		(skb->len < nlh->nlmsg_len)) {
		hwlog_err("%s: nlmsg len error", __func__);
		goto skb_free;
	}
	switch (nlh->nlmsg_type) {
	case NL_MSG_REG:
		g_nl_ctx.native_pid = nlh->nlmsg_pid;
		break;
	case NL_MSG_HEARTBEAT:
		rsmc_heartbeat_process();
		break;
	case NL_MSG_REQ:
		rsmc_nl_inter_dn_notify_event((struct msg_head *)NLMSG_DATA(nlh));
		break;
	default:
		break;
	}

skb_free:
	kfree_skb(skb);
	mutex_unlock(&g_recv_mtx);
}

static struct msg_entity *rsmc_nl_dn_get_msg(void)
{
	struct msg_entity *msg = NULL;
	int idx;

	spin_lock(&g_nl_ctx.dn_lock);
	if (g_nl_ctx.dn_end == g_nl_ctx.dn_start) {
		spin_unlock(&g_nl_ctx.dn_lock);
		return NULL;
	}
	idx = g_nl_ctx.dn_start;
	if (idx >= MAX_NOR_MSG_LEN || idx < 0) {
		spin_unlock(&g_nl_ctx.dn_lock);
		hwlog_err("%s: idx error", __func__);
		return NULL;
	}
	if (g_nl_ctx.dn_list[idx] == NULL) {
		hwlog_err("%s: list is null", __func__);
		msg = NULL;
	} else {
		msg = (struct msg_entity *)g_nl_ctx.dn_list[idx];
		g_nl_ctx.dn_list[idx] = NULL;
	}
	g_nl_ctx.dn_start = (idx + 1) % MAX_NOR_MSG_LEN;
	spin_unlock(&g_nl_ctx.dn_lock);
	return msg;
}

static struct msg_entity *rsmc_nl_up_get_msg(void)
{
	struct msg_entity *msg = NULL;
	int idx;

	spin_lock(&g_nl_ctx.up_lock);
	if (g_nl_ctx.up_end == g_nl_ctx.up_start) {
		spin_unlock(&g_nl_ctx.up_lock);
		return NULL;
	}
	idx = g_nl_ctx.up_start;
	if (idx >= MAX_NOR_MSG_LEN || idx < 0) {
		spin_unlock(&g_nl_ctx.up_lock);
		hwlog_err("%s: idx error", __func__);
		return NULL;
	}
	if (g_nl_ctx.up_list[idx] == NULL) {
		hwlog_err("%s: list is null", __func__);
		msg = NULL;
	} else {
		msg = (struct msg_entity *)g_nl_ctx.up_list[idx];
		g_nl_ctx.up_list[idx] = NULL;
	}
	g_nl_ctx.up_start = (idx + 1) % MAX_NOR_MSG_LEN;
	spin_unlock(&g_nl_ctx.up_lock);
	hwlog_info("%s: %d", __func__, msg->msg.type);
	return msg;
}

static struct msg_entity *rsmc_nl_up_get_fast(void)
{
	struct msg_entity *msg = NULL;
	int idx;

	if (g_nl_ctx.up_endf == g_nl_ctx.up_startf)
		return NULL;
	idx = g_nl_ctx.up_startf;
	if (idx >= MAX_FST_MSG_LEN || idx < 0) {
		hwlog_err("%s: idx error", __func__);
		return NULL;
	}
	if (g_nl_ctx.up_fast[idx] == NULL) {
		msg = NULL;
	} else {
		msg = (struct msg_entity *)g_nl_ctx.up_fast[idx];
		g_nl_ctx.up_fast[idx] = NULL;
	}
	g_nl_ctx.up_startf = (idx + 1) % MAX_FST_MSG_LEN;
	return msg;
}

/* send a message to user space */
int rsmc_nl_up_notify_event(struct msg_head *msg)
{
	int ret;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	void *pdata = NULL;

	if (msg == NULL)
		return -1;

	mutex_lock(&g_send_mtx);
	if (!g_nl_ctx.native_pid || (g_nl_ctx.nlfd == NULL)) {
		hwlog_err("%s: err pid = %d",
			__func__, g_nl_ctx.native_pid);
		ret = -1;
		goto nty_end;
	}

	skb = nlmsg_new(msg->len, GFP_ATOMIC);
	if (skb == NULL) {
		hwlog_err("%s: nlmsg_new fail", __func__);
		ret = -1;
		goto nty_end;
	}

	nlh = nlmsg_put(skb, 0, 0, msg->type, msg->len, 0);
	if (nlh == NULL) {
		hwlog_err("%s: nlmsg_put fail", __func__);
		kfree_skb(skb);
		skb = NULL;
		ret = -1;
		goto nty_end;
	}

	pdata = nlmsg_data(nlh);
	ret = memcpy_s(pdata, msg->len, msg, msg->len);
	if (ret != EOK)
		return ret;

	/* skb will be freed in netlink_unicast */
	ret = netlink_unicast(g_nl_ctx.nlfd, skb,
		g_nl_ctx.native_pid, MSG_DONTWAIT);
	if (ret < 0)
		hwlog_err("%s: netlink_unicast %d fail,errno:%d", __func__, msg->type, ret);

nty_end:
	mutex_unlock(&g_send_mtx);
	return ret;
}

int rsmc_netlink_up_handle_thread(void *data)
{
	struct msg_entity *msg = NULL;

	hwlog_info("%s: enter", __func__);

	while (!kthread_should_stop()) {
		down(&g_nl_ctx.up_sema);
		if (g_nl_ctx.native_pid == 0)
			continue;
		msg = rsmc_nl_up_get_msg();
		while (msg != NULL) {
			rsmc_nl_up_notify_event(&msg->msg);
			kfree(msg);
			msg = rsmc_nl_up_get_msg();
		}
		msg = rsmc_nl_up_get_fast();
		while (msg != NULL) {
			rsmc_nl_up_notify_event(&msg->msg);
			// retry normal message
			msg = rsmc_nl_up_get_msg();
			while (msg != NULL) {
				rsmc_nl_up_notify_event(&msg->msg);
				kfree(msg);
				msg = rsmc_nl_up_get_msg();
			}
			msg = rsmc_nl_up_get_fast();
		}
	}
	return 0;
}

int rsmc_netlink_dn_handle_thread(void *data)
{
	struct msg_entity *msg = NULL;

	hwlog_info("%s: enter", __func__);

	while (!kthread_should_stop()) {
		down(&g_nl_ctx.dn_sema);
		if (g_nl_ctx.native_pid == 0)
			continue;
		msg = rsmc_nl_dn_get_msg();
		while (msg != NULL) {
			rsmc_process_cmd(&msg->msg);
			kfree(msg);
			msg = rsmc_nl_dn_get_msg();
		}
	}
	return 0;
}

/* netlink init function */
int rsmc_netlink_handle_init(void)
{
	struct sched_param param;
	struct netlink_kernel_cfg nb_nl_cfg = {
		.input = rsmc_netlink_handle_rcv,
	};

	g_nl_ctx.nlfd = netlink_kernel_create(&init_net,
		NETLINK_RSMC, &nb_nl_cfg);

	if (g_nl_ctx.nlfd == NULL) {
		hwlog_err("%s: netlink_handle_init failed", __func__);
		return -EINVAL;
	}

	sema_init(&g_nl_ctx.up_sema, 0);
	sema_init(&g_nl_ctx.dn_sema, 0);
	spin_lock_init(&g_nl_ctx.up_lock);
	spin_lock_init(&g_nl_ctx.dn_lock);

	g_nl_ctx.up_task = kthread_run(
		rsmc_netlink_up_handle_thread,
		NULL,
		"rsmc_nl_up_thread");
	if (IS_ERR(g_nl_ctx.up_task)) {
		hwlog_err("%s: failed to create thread", __func__);
		g_nl_ctx.up_task = NULL;
		return -EINVAL;
	}
	param.sched_priority = MAX_USER_RT_PRIO / 2;
	sched_setscheduler(g_nl_ctx.up_task, SCHED_FIFO, &param);

	g_nl_ctx.dn_task = kthread_run(
		rsmc_netlink_dn_handle_thread,
		NULL,
		"rsmc_nl_dn_thread");
	if (IS_ERR(g_nl_ctx.dn_task)) {
		hwlog_err("%s: failed to create thread", __func__);
		g_nl_ctx.dn_task = NULL;
		return -EINVAL;
	}
	param.sched_priority = MAX_USER_RT_PRIO / 2;
	sched_setscheduler(g_nl_ctx.dn_task, SCHED_FIFO, &param);

	g_nl_ctx.chan_state = NETLINK_MSG_LOOP_INIT;

	return 0;
}

/* netlink deinit function */
void rsmc_netlink_handle_exit(void)
{
	if (g_nl_ctx.nlfd && g_nl_ctx.nlfd->sk_socket) {
		sock_release(g_nl_ctx.nlfd->sk_socket);
		g_nl_ctx.nlfd = NULL;
	}

	if (g_nl_ctx.up_task != NULL) {
		kthread_stop(g_nl_ctx.up_task);
		g_nl_ctx.up_task = NULL;
	}
	if (g_nl_ctx.dn_task != NULL) {
		kthread_stop(g_nl_ctx.dn_task);
		g_nl_ctx.dn_task = NULL;
	}
}

int regist_model(model_reg *fun, enum install_model model)
{
	msg_process *fn = NULL;

	if (fun == NULL)
		return -EINVAL;
	if (model > MODEL_NUM || model < 0) {
		hwlog_err("%s: invalid model, model is :%d", __func__, model);
		return -EINVAL;
	}
	fn = fun(rsmc_nl_inter_up_notify_event);
	if (fn == NULL) {
		hwlog_err("%s: fn null:%d", __func__, model);
		return -EINVAL;
	}

	g_nl_ctx.mod_cb[model] = fn;
	return 0;
}

int rsmc_main_thread(void *data)
{
	model_reg *reg_fn = NULL;
	int rtn = 0;
	int model;

	hwlog_info("%s: enter", __func__);
	if (rsmc_netlink_handle_init()) {
		hwlog_err("%s: init netlink_handle module failed", __func__);
		g_nl_ctx.chan_state = NETLINK_MSG_LOOP_EXIT;
		return -EINVAL;
	}

	for (model = 0; model < MODEL_NUM; model++) {
		if (module_init_map[model].model != model) {
			hwlog_err("%s: model init map error", __func__);
			return -EINVAL;
		}
		reg_fn = module_init_map[model].reg;
		if (reg_fn)
			rtn += regist_model(reg_fn, model);
	}

	if (rtn < 0)
		return rtn;
	g_nl_ctx.up_start = 0;
	g_nl_ctx.up_end = 0;
	rtn = memset_s(g_nl_ctx.up_list, sizeof(struct list_head *) * (MAX_NOR_MSG_LEN + 4),
		0, sizeof(struct list_head *) * MAX_NOR_MSG_LEN);
	if (rtn != EOK)
		return -EINVAL;

	g_nl_ctx.up_startf = 0;
	g_nl_ctx.up_endf = 0;
	rtn = memset_s(g_nl_ctx.up_fast, sizeof(struct list_head *) * (MAX_NOR_MSG_LEN + 4),
		0, sizeof(struct list_head *) * MAX_FST_MSG_LEN);
	if (rtn != EOK)
		return -EINVAL;

	rsmc_clear_dn_msg_list();
	g_nl_ctx.heartbeat_running = false;

	init_unit_test();

	hwlog_info("%s: netlink_handle module inited", __func__);
	return 0;
}

int rsmc_init_x800(void)
{
	struct task_struct *task = kthread_run(rsmc_main_thread, NULL, "rsmc_main_thread");
	if (IS_ERR(task)) {
		hwlog_err("%s: failed to create thread", __func__);
		task = NULL;
		return -EINVAL;
	}
	return 0;
}

void rsmc_exit_x800(void)
{
	model_unreg *unreg_fn = NULL;
	int model;

	for (model = 0; model < MODEL_NUM; model++) {
		if (module_init_map[model].model != model) {
			hwlog_err("%s: model init map error", __func__);
			continue;
		}
		unreg_fn = module_init_map[model].unreg;
		if (unreg_fn != NULL)
			unreg_fn(0);
	}
	g_nl_ctx.chan_state = NETLINK_MSG_LOOP_EXIT;
	rsmc_netlink_handle_exit();
}

