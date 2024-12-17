/*
 * netlink_handle_boosterd.h
 *
 * receive and report netlink messages for boosterD service kernel module implementation
 * the interface file of this module
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

#ifndef _NETLINK_HANDLE_BOOSTERD_H_
#define _NETLINK_HANDLE_BOOSTERD_H_

#include <linux/types.h>
#include <linux/spinlock.h>

enum exec_type {
	TASKLET_EXEC = 0,
	WORK_EXEC = 1,
	EXEC_TYPE_NUM_MAX
};

enum report_msg_type {
	CLOUD_NETWORK_REPORT = 0,
	REPORT_TYPE_NUM_MAX
};

enum receive_msg_type {
	CLOUD_NETWORK_EVENT = 0,
	RECEIVE_TYPE_NUM_MAX
};

enum module_id {
	CLOUD_NETWORK = 0,
	MODULE_ID_NUM_MAX
};

struct netlink_head {
	u32 type;
	u32 msg_len; /* sizeof(netlink_head) + length of data */
	char data[0];
};

/* the interface provided by the netlink_handle_boosterd module for netlink event report */
typedef void (*netlink_event_report)(const struct netlink_head *msg,
	enum exec_type type);

/* the interface provided by Caller module for register netlink_event_report func */
typedef void (*register_netlink_event_report_func)(netlink_event_report fn);
/* the interface provided by Caller module for netlink event receive */
typedef void (*netlink_cmd_receive)(const struct netlink_head *msg);
#endif /* _NETLINK_HANDLE_BOOSTERD_H_ */
