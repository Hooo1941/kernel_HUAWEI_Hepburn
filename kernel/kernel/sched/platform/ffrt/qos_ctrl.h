/* SPDX-License-Identifier: GPL-2.0
 * qos_ctrl.h
 *
 * ffrt qos header
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

#ifndef __QOS_CTRL_H
#define __QOS_CTRL_H

#include <linux/list.h>

#define NO_QOS (-1)
#define NR_QOS 18
#define NR_RT_QOS 1
#define MIN_RT_QOS_LEVEL (NR_QOS - NR_RT_QOS)

#define QOS_NUM_MAX 2000

enum qos_manipulate_type {
	QOS_APPLY = 1,
	QOS_LEAVE,
	QOS_OPERATION_CMD_MAX_NR,
};

#define ROOT_UID   0
#define SYSTEM_UID 1000

#define SUPER_UID SYSTEM_UID
#define super_uid(uid) (((uid) == (ROOT_UID)) || ((uid) == (SYSTEM_UID)))

enum auth_err_no {
	ARG_INVALID = 1,
	THREAD_EXITING,
	DIRTY_QOS_POLICY,
	UID_NOT_AUTHORIZED,
	UID_NOT_FOUND,
	PID_DUPLICATE,
	PID_NOT_EXIST,
	INVALID_AUTH,
	QOS_THREAD_NUM_EXCEED_LIMIT,
};


#ifndef CONFIG_QOS_POLICY_MAX_NR
#define QOS_POLICYS_COUNT 5
#else
#define QOS_POLICYS_COUNT CONFIG_QOS_POLICY_MAX_NR
#endif

/*
 * keep match with auth_status
 *
 * range (QOS_POLICY_SYSTEM, QOS_POLICY_MAX_NR) could defined by user
 * use ctrl_qos_policy
 */
enum qos_policy_type {
	QOS_POLICY_DEFAULT = 1,    /* reserved for "NO QOS" */
	QOS_POLICY_SYSTEM  = 2,    /* reserved for ROOT and SYSTEM */
	QOS_POLICY_MAX_NR = QOS_POLICYS_COUNT,
};

/* qos level interval between user space and kernel space */
#define QOS_LEVEL_INTERVAL 1

/* min qos level used in kernel space, begin index for LOOP */
#define QOS_POLICY_MIN_LEVEL (NO_QOS + QOS_LEVEL_INTERVAL)

#define QOS_FLAG_NICE			0x01
#define QOS_FLAG_LATENCY_NICE		0x02
#define QOS_FLAG_UCLAMP			0x04
#define QOS_FLAG_RT			0x08

#define QOS_FLAG_ALL	(QOS_FLAG_NICE			| \
			 QOS_FLAG_LATENCY_NICE		| \
			 QOS_FLAG_UCLAMP		| \
			 QOS_FLAG_RT)

enum qos_ctrl_cmdid {
	QOS_CTRL = 1,
	QOS_POLICY,
	QOS_THREAD_CTRL,
	QOS_CTRL_MAX_NR
};

#define QOS_CTRL_IPC_MAGIG	0xCC

#define QOS_CTRL_BASIC_OPERATION \
	_IOWR(QOS_CTRL_IPC_MAGIG, QOS_CTRL, struct qos_ctrl_data)
#define QOS_CTRL_POLICY_OPERATION \
	_IOWR(QOS_CTRL_IPC_MAGIG, QOS_POLICY, struct qos_policy_datas)
#define QOS_THREAD_CTRL_OPERATION \
	_IOWR(QOS_CTRL_IPC_MAGIG, QOS_THREAD_CTRL, struct thread_attr_ctrl)

struct qos_policy_item {
	int latency_nice;
	int uclamp_min;
	int uclamp_max;
	unsigned long affinity;
	__u8 priority;
	__u8 init_load;
	__u8 prefer_idle;
};

struct thread_attr_ctrl {
	int tid;
	bool priority_set_enable;
	bool affinity_set_enable;
};

struct qos_policy_map {
	rwlock_t lock;
	bool initialized;
	unsigned int policy_flag;
	struct qos_policy_item levels[NR_QOS + 1];
};

struct qos_policy_datas {
	/*
	 * policy_type: id for qos policy, valid from [1, QOS_POLICY_MAX_NR)
	 * policy_flag: control valid sched attr for policy, QOS_FLAG_ALL for whole access
	 * policys:     sched params for specific level qos, minus 1 for matching struct in kerenl
	 */
	int policy_type;
	unsigned int policy_flag;
	struct qos_policy_item policys[NR_QOS + 1];
};

struct qos_ctrl_data {
	int pid;

	/*
	 * type:  operation type, see qos_manipulate_type
	 * level: valid from 1 to NR_QOS. Larger value, more aggressive supply
	 */
	unsigned int type;

	/*
	 * user space level, range from [1, NR_QOS]
	 *
	 * NOTICE!!!:
	 * minus 1 before use in kernel, so the kernel range is [0, NR_QOS)
	 */
	unsigned int level;
	/*
	 * query hwqos and convert it to ffrtqos, only apply in OHOS
	 * SR:SR000I3IBQ
	 */
	int qos;
	int static_qos;
	int dynamic_qos;
	int tag_schedenable;
};

#endif /* __QOS_CTRL_H */
