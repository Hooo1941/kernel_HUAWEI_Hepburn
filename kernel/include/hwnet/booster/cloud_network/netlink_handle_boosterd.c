/*
 * netlink_handle_boosterd.c
 *
 * receive and report netlink messages for boosterD service kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "netlink_handle_boosterd.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/interrupt.h>
#include <uapi/linux/netlink.h>
#include <linux/moduleparam.h>
#include <linux/gfp.h>
#include <securec.h>

#include "cloud_network_utils.h"

#ifdef CONFIG_HW_CLOUD_NETWORK
#include "cloud_network.h"
#endif

#define NETLINK_HANDLE_BOOSTERERD_MODULE "netlink_handle_boosterd"
#define NETLINK_MSG_LEN_MAX 4096
#define STATUS_DISABLE 0
#define STATUS_ENABLE 1

#ifndef NETLINK_BOOSTERD
#define NETLINK_BOOSTERD 47
#endif

enum nl_msg_header_type {
	SET_USER_PID = 0,
	NETLINK_CMD_MSG = 1,
	NL_MSG_HEADER_NTYPE_NUM_MAX
};

struct netlink_ctrl {
	struct sock *dev_sock;
	unsigned int user_pid;
	atomic_t status;
	struct list_queue receive_queue;
	struct work_struct receive_work;
	struct list_queue report_queue;
	struct work_struct report_work;
	struct tasklet_struct report_tasklet;
};

struct module_handle_netlink_msg_func {
	enum module_id module;
	register_netlink_event_report_func register_report_func;
	netlink_cmd_receive receive_func;
};

struct receive_msg_type_module_match {
	enum receive_msg_type msg_type;
	enum module_id module;
};

static const struct module_handle_netlink_msg_func module_handle_tbl[] = {
#ifdef CONFIG_HW_CLOUD_NETWORK
	{ CLOUD_NETWORK, cloud_net_register_report, cloud_net_netlink_receive },
#endif
};

static const struct receive_msg_type_module_match receive_msg_type_module_tbl[] = {
#ifdef CONFIG_HW_CLOUD_NETWORK
	{ CLOUD_NETWORK_EVENT, CLOUD_NETWORK },
#endif
};

static struct netlink_ctrl *netlink_mgr = NULL;

static void netlink_msg_report(const struct netlink_head *msg, enum exec_type type)
{
	struct list_node *node = NULL;

	if (unlikely((netlink_mgr == NULL) || (msg == NULL) ||
		atomic_read(&(netlink_mgr->status)) == STATUS_DISABLE)) {
		log_err("NULL Pointer or status is disable, error");
		return;
	}

	node = list_node_alloc((void *)msg, msg->msg_len);
	if (unlikely(node == NULL)) {
		log_err("alloc node failed");
		return;
	}
	enqueue_list_queue(&(netlink_mgr->report_queue), node);
	log_info("type %u msg_len %u", msg->type, msg->msg_len);
	if (type == TASKLET_EXEC) {
		tasklet_schedule(&(netlink_mgr->report_tasklet));
	} else {
		schedule_work(&(netlink_mgr->report_work));
	}
	return;
}

static void netlink_send_unicast_data(struct netlink_ctrl *ctrl,
	struct list_node *node)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	unsigned int msg_size;
	int ret;
	int sec_ret;

	if (unlikely((ctrl == NULL) || (node == NULL))) {
		log_err("NULL Pointer, error");
		return;
	}

	msg_size = (unsigned int) (NLMSG_SPACE((int) node->len));
	skb = alloc_skb(msg_size, GFP_ATOMIC);
	if (skb == NULL) {
		log_err("alloc skb failed");
		return;
	}
	nlh = nlmsg_put(skb, 0, 0, 0, msg_size - sizeof(*nlh), 0);
	if (nlh == NULL) {
		log_err("nlh is NULL Pointer");
		dev_kfree_skb_any(skb);
		return;
	}
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;
	sec_ret = memcpy_s(NLMSG_DATA(nlh), node->len, node->data, node->len);
	if (sec_ret != 0) {
		log_err("memcpy node->data fail");
		dev_kfree_skb_any(skb);
		return;
	}
	log_info("type %u msg_len %u", ((struct netlink_head *)(&(node->data)))->type,
		((struct netlink_head *)(&(node->data)))->msg_len);
	ret = netlink_unicast(ctrl->dev_sock, skb, ctrl->user_pid, 0);
	if (ret < 0)
		log_err("msg send failed!");
	return;
}

static void netlink_msg_report_exec(struct netlink_ctrl *ctrl,
	struct list_queue *report_queue)
{
	struct list_node *node = NULL;

	if (unlikely((ctrl == NULL) || (report_queue == NULL))) {
		log_err("wifi_audio_netlink is NULL Pointer");
		return;
	}
	while (list_queue_empty(report_queue) == 0) {
		node = dequeue_list_queue(report_queue);
		if (node == NULL) {
			log_err("dequeue a null node");
			break;
		}
		netlink_send_unicast_data(ctrl, node);
		list_node_free(node);
		node = NULL;
	}
	return;
}

static void netlink_msg_report_tasket(unsigned long arg)
{
	struct netlink_ctrl *ctrl = NULL;

	if (unlikely((struct netlink_ctrl *)arg == NULL)) {
		log_err("wifi_audio_netlink is NULL Pointer");
		return;
	}
	ctrl = (struct netlink_ctrl *)arg;
	netlink_msg_report_exec(ctrl, &(ctrl->report_queue));
	return;
}

static void netlink_msg_report_work(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;

	if (unlikely(work == NULL)) {
		log_err("work NULL Pointer");
		return;
	}
	ctrl = container_of(work, struct netlink_ctrl, report_work);
	netlink_msg_report_exec(ctrl, &(ctrl->report_queue));
	return;
}

static void netlink_cmd_receive_dispatch(const struct netlink_head *msg)
{
	int module_id;

	if (unlikely(msg == NULL)) {
		log_err("msg_rev is NULL, error");
		return;
	}

	log_info("id %d, len %u", msg->type, msg->msg_len);
	if (msg->type < RECEIVE_TYPE_NUM_MAX) {
		module_id = receive_msg_type_module_tbl[msg->type].module;
		log_info("type %d, module_id %u", msg->type, module_id);
		if (module_id >= 0 && module_id < MODULE_ID_NUM_MAX &&
			module_handle_tbl[module_id].receive_func != NULL) {
			log_info("module_id %u handle the msg", module_id);
			module_handle_tbl[module_id].receive_func(msg);
		}
	}
	return;
}

static void netlink_msg_receive_work(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;

	if (unlikely(work == NULL)) {
		log_err("work is NULL, error");
		return;
	}

	ctrl = container_of(work, struct netlink_ctrl, receive_work);
	while (list_queue_empty(&(ctrl->receive_queue)) == 0) {
		node = dequeue_list_queue(&(ctrl->receive_queue));
		if (node == NULL) {
			log_err("dequeue a null node");
			break;
		}
		netlink_cmd_receive_dispatch((struct netlink_head *)(&(node->data)));
		list_node_free(node);
		node = NULL;
	}
	return;
}

static void netlink_msg_set(struct netlink_ctrl *pctrl,
	const struct netlink_head *msg_rev, unsigned int msg_len)
{
	struct list_node *node = NULL;

	if (unlikely((msg_rev == NULL) || (pctrl == NULL) ||
		(msg_len > NETLINK_MSG_LEN_MAX))) {
		log_err("NULL Pointer or msg_len is over, error");
		return;
	}

	node = list_node_alloc((void *)msg_rev, msg_len);
	if (unlikely(node == NULL)) {
		log_err("alloc node failed, error");
		return;
	}
	enqueue_list_queue(&(pctrl->receive_queue), node);
	schedule_work(&(pctrl->receive_work));
	return;
}

static void netlink_msg_receive(struct sk_buff *netlink_skb)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;

	if (unlikely(netlink_skb == NULL || netlink_mgr == NULL)) {
		log_err("NULL pointer, error");
		return;
	}

	skb = skb_get(netlink_skb);
	if (unlikely(skb->len < NLMSG_SPACE(0))) {
		log_err("skb->len isn't enough");
		dev_kfree_skb_any(skb);
		return;
	}

	nlh = nlmsg_hdr(skb);
	if (unlikely(nlh == NULL)) {
		log_err("nlh is NULL");
		dev_kfree_skb_any(skb);
		return;
	}

	if (unlikely(nlh->nlmsg_len >
		(NETLINK_MSG_LEN_MAX + sizeof(struct nlmsghdr)))) {
		log_err("nlmsg_len is over");
		dev_kfree_skb_any(skb);
		return;
	}
	log_info("msg header type %u pid %u", nlh->nlmsg_type, nlh->nlmsg_pid);
	switch (nlh->nlmsg_type) {
	case SET_USER_PID:
		/* Store the user space daemon pid */
		netlink_mgr->user_pid = nlh->nlmsg_pid;
		atomic_set(&(netlink_mgr->status), STATUS_ENABLE);
		break;
	case NETLINK_CMD_MSG:
		netlink_msg_set(netlink_mgr,
			(struct netlink_head *)NLMSG_DATA(nlh),
			nlh->nlmsg_len - sizeof(struct nlmsghdr));
		break;
	default:
		break;
	}
	dev_kfree_skb_any(skb);
	return;
}

static void module_handle_tbl_init(void)
{
	for (unsigned int i = 0; i < (sizeof(module_handle_tbl) /
		sizeof(module_handle_tbl[0])); i++) {
		if (module_handle_tbl[i].register_report_func != NULL) {
			log_info("module index %u register report function", i);
			module_handle_tbl[i].register_report_func(netlink_msg_report);
		}
	}
	return;
}

static int netlink_init(void)
{
	struct netlink_kernel_cfg netlink_cfg = {
		.groups = 0,
		.flags = 0,
		.input = netlink_msg_receive,
		.cb_mutex = NULL,
		.bind = NULL,
		.unbind = NULL,
		.compare = NULL,
	};

	log_info("netlink init");

	if (unlikely(netlink_mgr != NULL)) {
		log_err("netlink has inited");
		clean_list_queue(&(netlink_mgr->receive_queue));
		clean_list_queue(&(netlink_mgr->report_queue));
		return SUCCESS;
	}

	netlink_mgr = (struct netlink_ctrl *)kmalloc(sizeof(struct netlink_ctrl),
		GFP_ATOMIC);
	if (unlikely(netlink_mgr == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	netlink_mgr->dev_sock = netlink_kernel_create(&init_net,
		NETLINK_BOOSTERD, &netlink_cfg);
	if (unlikely(!netlink_mgr->dev_sock)) {
		log_err("netlink kernel create error\n");
		kfree(netlink_mgr);
		netlink_mgr = NULL;
		return ERROR;
	}
	atomic_set(&(netlink_mgr->status), STATUS_DISABLE);
	init_list_queue(&(netlink_mgr->receive_queue));
	INIT_WORK(&(netlink_mgr->receive_work), netlink_msg_receive_work);
	init_list_queue(&(netlink_mgr->report_queue));
	INIT_WORK(&(netlink_mgr->report_work), netlink_msg_report_work);
	tasklet_init(&(netlink_mgr->report_tasklet), netlink_msg_report_tasket,
		(unsigned long)netlink_mgr);
	return SUCCESS;
}

static void netlink_exit(void)
{
	log_info("netlink exit");
	if (unlikely(netlink_mgr == NULL)) {
		log_err("netlink has exited");
		return;
	}

	tasklet_kill(&(netlink_mgr->report_tasklet));
	cancel_work_sync(&(netlink_mgr->receive_work));
	cancel_work_sync(&(netlink_mgr->report_work));
	clean_list_queue(&(netlink_mgr->receive_queue));
	clean_list_queue(&(netlink_mgr->report_queue));
	if (likely(netlink_mgr->dev_sock != NULL))
		netlink_kernel_release(netlink_mgr->dev_sock);

	kfree(netlink_mgr);
	netlink_mgr = NULL;
	return;
}

static int __init netlink_handle_boosterd_init(void)
{
	int ret;

	log_info("netlink handle boosterd module init");

	if (!cloud_network_is_support())
		return -EINVAL;

	ret = netlink_init();
	module_handle_tbl_init();
	return ret;
}

static void __exit netlink_handle_boosterd_exit(void)
{
	log_info("netlink handle boosterd module exit");

	if (!cloud_network_is_support())
		return;

	netlink_exit();
	return;
}

module_init(netlink_handle_boosterd_init);
module_exit(netlink_handle_boosterd_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(NETLINK_HANDLE_BOOSTERERD_MODULE);
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
