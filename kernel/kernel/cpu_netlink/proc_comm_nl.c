/*
 * proc_comm_nl.c
 *
 * iaware comm connector implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#include <cpu_netlink/cpu_netlink.h>
#include <linux/cgroup.h>
#define BINDER_THREAD_NAME "Binder:"
#define  BACKGROUND_GROUP_NAME "background"

#ifdef CONFIG_HW_RTG || IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
#include <linux/sched/hw_rtg/proc_state.h>
#elif (defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_MTK_RTG_SCHED))
#include "hwrtg/proc_state.h"
#endif

#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED) || IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
const char* mali_names[] = {"gpu-work-client", "gpu-mem-pool", "gpu-work-server",
	"mali-utility-wo", "mali-mem-purge", "mali-cmar-backe",
	"mali-cpu-comman", "mali-compiler", "mali-event-hand"};
#endif

#define IAWARE_COMM_MSG_LEN 2

void send_thread_comm_msg(int num, int pid, int tgid)
{
	int dt[IAWARE_COMM_MSG_LEN];

	dt[0] = pid;
	dt[1] = tgid;
	send_to_user(num, IAWARE_COMM_MSG_LEN, dt);
}

#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED) || IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
static bool is_mali_thread(const char* comm)
{
	int length = sizeof(mali_names) / sizeof(mali_names[0]);
	for (int i = 0; i < length; i++) {
		if (strstr(comm, mali_names[i]) != NULL)
			return true;
	}
	return false;
}
#endif

int iaware_proc_comm_connector(struct task_struct *task, const char *comm)
{
	int sock_num = 0;
	char proc_cgroup_name[NAME_MAX + 1];

	if (!task || !comm)
		return sock_num;

	if (strstr(comm, BINDER_THREAD_NAME)) {
		rcu_read_lock();
		cgroup_name(task_cgroup(task, cpuset_cgrp_id), proc_cgroup_name, sizeof(proc_cgroup_name));
		rcu_read_unlock();
		if (strstr(proc_cgroup_name, BACKGROUND_GROUP_NAME))
			sock_num = PROC_COMM;
	}
#if defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED) || IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
	else if (is_mali_thread(comm)) {
		sock_num = PROC_MALI_COMM;
	}
#endif
#if IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
	else if (check_key_aux_comm(task, comm)) {
		sock_num = PROC_AUX_COMM;
	} else if (check_key_aux_comm(task, task->comm)) {
		sock_num = PROC_AUX_COMM_REMOVE;
	}
#elif defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED)
	else if (is_key_aux_comm(task, comm)) {
		sock_num = PROC_AUX_COMM;
	} else if (is_key_aux_comm(task, task->comm)) {
		sock_num = PROC_AUX_COMM_REMOVE;
	}
#endif

	return sock_num;
}

void iaware_send_comm_msg(struct task_struct *task, int sock_num)
{
	int pid;
	int tgid;

	if (!task || !sock_num)
		return;

	get_task_struct(task);
	pid = task->pid;
	tgid = task->tgid;
	put_task_struct(task);
	send_thread_comm_msg(sock_num, pid, tgid);
}
