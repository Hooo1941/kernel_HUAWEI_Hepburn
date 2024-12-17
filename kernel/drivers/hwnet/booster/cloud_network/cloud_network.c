/*
 * cloud_network.c
 *
 * cloud network module implementation
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

#include "cloud_network.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <securec.h>
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gfp.h>
#include <net/xfrm.h>
#include <linux/jhash.h>
#include <linux/rtnetlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <uapi/linux/netfilter/nf_nat.h>

#include "netlink_handle_boosterd.h"
#include "cloud_network_utils.h"
#include "cloud_network_param.h"
#include "cloud_network_device.h"
#include "cloud_network_session.h"
#include "cloud_network_option.h"
#include "cloud_network_detect.h"

#define CLOUD_NETWORK_MODULE "cloud_network_module"

#define DTS_NODE_HUAWEI_CLOUD_NETWORK "huawei,cloud_network"
#define DTS_PROP_CLOUD_NETWORK_ENABLE "cloud_network_enable"

static int cmd_init(const struct netlink_msg_head *msg);
static int cmd_deinit(const struct netlink_msg_head *msg);
static int cmd_tx_rx_ctl(const struct netlink_msg_head *msg);
static int cmd_updata_all_cloud_param(const struct netlink_msg_head *msg);
static int cmd_updata_cloud_key_param(const struct netlink_msg_head *msg);
static int cmd_updata_cloud_compass_param(const struct netlink_msg_head *msg);
static int cmd_updata_cloud_app_param(const struct netlink_msg_head *msg);
static int cmd_updata_link_info(const struct netlink_msg_head *msg);
static int cmd_ims_monitor_start(const struct netlink_msg_head *msg);
static int cmd_ims_monitor_stop(const struct netlink_msg_head *msg);
static int cmd_compass_monitor_start(const struct netlink_msg_head *msg);
static int cmd_compass_monitor_stop(const struct netlink_msg_head *msg);

struct netlink_ctrl {
	struct list_queue receive_queue;
	struct work_struct receive_handle_work;
	struct list_queue report_queue;
	struct work_struct report_handle_work;
	struct tasklet_struct report_handle_tasklet;
};

typedef int (*msg_handle_func)(const struct netlink_msg_head *msg);
struct msg_handle {
	u32 msg_type;
	msg_handle_func handler;
};

static struct netlink_ctrl *netlink_mgr = NULL;
static netlink_event_report netlink_cmd_report_cb = NULL;
static const struct msg_handle netlink_cmd_receive_dispatch_tbl[] = {
	{ INIT, cmd_init },
	{ DEINIT, cmd_deinit },
	{ TX_RX_CTL, cmd_tx_rx_ctl },
	{ CLOUD_PARAM_ALL_UPDATE, cmd_updata_all_cloud_param },
	{ CLOUD_PARAM_KEY_UPDATE, cmd_updata_cloud_key_param },
	{ CLOUD_PARAM_COMPASS_UPDATE, cmd_updata_cloud_compass_param },
	{ CLOUD_PARAM_APP_UPDATE, cmd_updata_cloud_app_param },
	{ LINK_INFO_UPDATE, cmd_updata_link_info },
	{ IMS_MONITOR_START, cmd_ims_monitor_start },
	{ IMS_MONITOR_STOP, cmd_ims_monitor_stop },
	{ COMPASS_AVAILABLE_MONITOR_START, cmd_compass_monitor_start },
	{ COMPASS_AVAILABLE_MONITOR_STOP, cmd_compass_monitor_stop }
};

static int cmd_init(const struct netlink_msg_head *msg)
{
	int ret = ERROR;
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_init_result *msg_rept = NULL;

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}
	log_info("netlink cmd init");
	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_init_result), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}
	ret = netlink_cmd_init_param_handle(msg);

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_init_result);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = INIT_RESULT;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_init_result);

	msg_rept = (struct rept_init_result *)(msg_header->data);
	msg_rept->result = ret;
	msg_rept->reason = 0;
	log_info("event report init result %d", ret);
	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return ret;
}

static int cmd_deinit(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd deinit");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_deinit_param_handle(msg);
	return SUCCESS;
}

static int cmd_tx_rx_ctl(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd tx rx ctl");

	if (!is_cloud_network_enable()) {
		log_err("cloud network not inited, error");
		return ERROR;
	}
	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_tx_rx_ctl_handle(msg);
	return SUCCESS;
}

static int cmd_updata_all_cloud_param(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd updata all cloud param");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_param_all_handle(msg);
	return SUCCESS;
}

static int cmd_updata_cloud_key_param(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd cloud key config");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_param_key_handle(msg);
	return SUCCESS;
}

static int cmd_updata_cloud_compass_param(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd cloud compass config");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_param_compass_handle(msg);
	return SUCCESS;
}

static int cmd_updata_cloud_app_param(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd updata cloud app param");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_param_app_handle(msg);
	return SUCCESS;
}

static int cmd_updata_link_info(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd updata link info");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_param_link_handle(msg);
	return SUCCESS;
}

static int cmd_ims_monitor_start(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd ims app monitor start");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_ims_monitor_start_handle(msg);
	return SUCCESS;
}

static int cmd_ims_monitor_stop(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd ims app monitor stop");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_ims_monitor_stop_handle(msg);
	return SUCCESS;
}

static int cmd_compass_monitor_start(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd compass monitor start");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_compass_monitor_start_handle(msg);
	return SUCCESS;
}

static int cmd_compass_monitor_stop(const struct netlink_msg_head *msg)
{
	log_info("netlink cmd compass monitor stop");

	if (unlikely(msg == NULL)) {
		log_err("msg NULL Pointer, error");
		return ERROR;
	}

	netlink_cmd_cloud_compass_monitor_stop_handle(msg);
	return SUCCESS;
}

static void netlink_event_report_exec(struct list_queue *report_queue)
{
	struct list_node *node = NULL;

	if (unlikely(report_queue == NULL)) {
		log_err("work NULL Pointer, error");
		return;
	}

	while (list_queue_empty(report_queue) == 0) {
		node = dequeue_list_queue(report_queue);
		if (unlikely(node == NULL)) {
			log_err("dequeue a null node, error");
			break;
		}
		log_info("type %u msg_len %u", ((struct netlink_head *)(&(node->data)))->type,
			((struct netlink_head *)(&(node->data)))->msg_len);
		if (netlink_cmd_report_cb)
			netlink_cmd_report_cb((struct netlink_head *)(node->data), TASKLET_EXEC);
		list_node_free(node);
		node = NULL;
	}
	return;
}

static void netlink_event_report_tasket(unsigned long arg)
{
	struct netlink_ctrl *ctrl = NULL;

	if (unlikely((struct netlink_ctrl *)arg == NULL)) {
		log_err("netlink_ctrl is NULL Pointer, error");
		return;
	}
	ctrl = (struct netlink_ctrl *)arg;
	netlink_event_report_exec(&(ctrl->report_queue));
	return;
}

static void netlink_event_report_work(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;

	if (unlikely(work == NULL)) {
		log_err("work NULL Pointer, error");
		return;
	}
	ctrl = container_of(work, struct netlink_ctrl, report_handle_work);
	netlink_event_report_exec(&(ctrl->report_queue));
	return;
}

void netlink_report(const struct netlink_head *msg, enum exec_type type)
{
	struct list_node *node = NULL;

	if (unlikely((netlink_mgr == NULL) || (msg == NULL))) {
		log_err("NULL Pointer, error");
		return;
	}

	node = list_node_alloc((void *)msg, msg->msg_len);
	if (unlikely(node == NULL)) {
		log_err("alloc node failed, error");
		return;
	}
	enqueue_list_queue(&(netlink_mgr->report_queue), node);
	log_info("type %u msg_len %u", msg->type, msg->msg_len);

	if (type == TASKLET_EXEC) {
		tasklet_schedule(&(netlink_mgr->report_handle_tasklet));
	} else {
		schedule_work(&(netlink_mgr->report_handle_work));
	}
	return;
}

static void netlink_cmd_receive_dispatch(const struct netlink_msg_head *msg)
{
	if (unlikely(msg == NULL)) {
		log_err("msg_rev is NULL, error");
		return;
	}

	if (likely(msg->type < RECEIVE_CMD_TYPE_NUM_MAX)) {
		log_info("msg type %u", msg->type);
		if (msg->type == netlink_cmd_receive_dispatch_tbl[msg->type].msg_type &&
			netlink_cmd_receive_dispatch_tbl[msg->type].handler != NULL) {
			if (netlink_cmd_receive_dispatch_tbl[msg->type].handler(msg) != SUCCESS)
				log_err("msg->type %u id is invalid, error", msg->type);
		}
	} else {
		log_err("msg type %u id is invalid, error", msg->type);
	}

	return;
}

static void netlink_cmd_receive_handle(struct work_struct *work)
{
	struct netlink_ctrl *ctrl = NULL;
	struct list_node *node = NULL;

	if (unlikely(work == NULL)) {
		log_err("work is NULL, error");
		return;
	}

	ctrl = container_of(work, struct netlink_ctrl, receive_handle_work);
	while (list_queue_empty(&(ctrl->receive_queue)) == 0) {
		node = dequeue_list_queue(&(ctrl->receive_queue));
		if (node == NULL) {
			log_err("dequeue a null node");
			break;
		}
		netlink_cmd_receive_dispatch((struct netlink_msg_head *)(&(node->data)));
		list_node_free(node);
		node = NULL;
	}
	return;
}

void cloud_net_netlink_receive(const struct netlink_head *msg)
{
	struct list_node *node = NULL;

	if (unlikely(netlink_mgr == NULL || msg == NULL ||
		(msg->msg_len - sizeof(struct netlink_msg_head) < 0))) {
		log_err("msg is error, type %u msg_len %u", msg->type, msg->msg_len);
		return;
	}

	log_info("receive netlink message type %u msg_len %u", msg->type, msg->msg_len);
	switch (msg->type) {
	case CLOUD_NETWORK_EVENT:
		node = list_node_alloc((void *)((char *)msg + sizeof(struct netlink_head)), msg->msg_len - sizeof(struct netlink_head));
		if (unlikely(node == NULL)) {
			log_err("alloc node failed, error");
			return;
		}
		enqueue_list_queue(&(netlink_mgr->receive_queue), node);
		schedule_work(&(netlink_mgr->receive_handle_work));
		break;
	default:
		log_err("invalid cloud network netlink msg type %u", msg->type);
		break;
	}
	return;
}

void cloud_net_register_report(netlink_event_report fn)
{
	if (unlikely(fn == NULL))
		return;

	log_info("cloud nework register netlink report");
	netlink_cmd_report_cb = fn;
	return;
}

static int netlink_init(void)
{
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

	init_list_queue(&(netlink_mgr->receive_queue));
	INIT_WORK(&(netlink_mgr->receive_handle_work), netlink_cmd_receive_handle);
	init_list_queue(&(netlink_mgr->report_queue));
	INIT_WORK(&(netlink_mgr->report_handle_work), netlink_event_report_work);
	tasklet_init(&(netlink_mgr->report_handle_tasklet),
		netlink_event_report_tasket, (unsigned long)netlink_mgr);
	return SUCCESS;
}

static void netlink_exit(void)
{
	log_info("netlink exit");

	if (unlikely(netlink_mgr == NULL)) {
		log_err("netlink has exited");
		return;
	}

	cancel_work_sync(&(netlink_mgr->receive_handle_work));
	cancel_work_sync(&(netlink_mgr->report_handle_work));
	clean_list_queue(&(netlink_mgr->receive_queue));
	clean_list_queue(&(netlink_mgr->report_queue));
	kfree(netlink_mgr);
	netlink_mgr = NULL;
	return;
}

bool cloud_network_is_support(void)
{
	struct device_node *node = of_find_compatible_node(NULL, NULL,
		DTS_NODE_HUAWEI_CLOUD_NETWORK);
	if (node == NULL) {
		log_info("not find huawei,cloud_network dts node, not suport");
		return false;
	}
	if (of_property_read_bool(node, DTS_PROP_CLOUD_NETWORK_ENABLE)) {
		log_info("find cloud_network_enable dts probe, is suport");
		return true;
	}
	log_info("not find cloud_network_enable dts probe, not suport");
	return false;
}

static int __init cloud_network_init(void)
{
	int ret;

	log_info("cloud network module init");

	if (!cloud_network_is_support())
		return -EINVAL;

	ret = netlink_init();
	ret += cloud_network_param_init();
	ret += cloud_network_device_init();
	ret += cloud_network_session_init();
	ret += cloud_network_option_init();
	ret += cloud_network_detect_init();
	return ret;
}

static void __exit cloud_network_exit(void)
{
	log_info("cloud network module exit");

	if (!cloud_network_is_support())
		return;

	netlink_exit();
	cloud_network_param_exit();
	cloud_network_device_exit();
	cloud_network_session_exit();
	cloud_network_option_exit();
	cloud_network_detect_exit();
	return;
}

module_init(cloud_network_init);
module_exit(cloud_network_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(CLOUD_NETWORK_MODULE);
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
