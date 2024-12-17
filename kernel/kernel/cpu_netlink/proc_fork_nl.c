/*
 * proc_fork_nl.c
 *
 * iaware fork connector implementation
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
#define MAX_FORK_TIME 30
#define VIP_GROUP_NAME "vip"

#ifdef CONFIG_HW_RTG || IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
#include <linux/sched/hw_rtg/proc_state.h>
#elif (defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_MTK_RTG_SCHED))
#include "hwrtg/proc_state.h"
#endif

#ifdef CONFIG_HW_FORK_THREAD_CHANGE_CGROUP
#include <../kernel/cgroup/cgroup-internal.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/mutex.h>

static struct cgroup *cgroup_for_cpuctl_top_app;
static struct cgroup *cgroup_for_cpuset_top_app;

struct top_app_attach_struct_t {
	struct task_struct *task;
	struct work_struct my_work;
};

static struct workqueue_struct *cgroup_attach_wq;
#endif

static void send_vip_msg(int pid, int tgid)
{
	int dt[SEND_DATA_LEN];

	dt[0] = pid;
	dt[1] = tgid;
	send_to_user(PROC_FORK, SEND_DATA_LEN, dt);
}

#if (defined(CONFIG_HW_VIP_THREAD) && !defined(CONFIG_DISABLE_VIP_FORK_MSG))
int check_vip_status(int cur_pid, int cur_tgid, struct task_struct *task)
{
	pid_t pid = cur_pid;
	pid_t tgid = cur_tgid;
	struct task_struct *parent = NULL;
	struct task_struct *temp_task = task;
	struct task_struct *tg_task = NULL;
	int time = 0;
	int vip = 0;

	if (!task)
		return vip;

	rcu_read_lock();
	do {
		if (pid != tgid) {
			tg_task = find_task_by_pid_ns(tgid, &init_pid_ns);
			if (!tg_task)
				break;
			vip = tg_task->static_vip;
			if (vip == 1)
				break;
		}

		parent = rcu_dereference(
			temp_task->real_parent); /*lint !e1058 !e64*/
		if (!parent)
			break;
		vip = parent->static_vip;
		pid = parent->pid;
		tgid = parent->tgid;

		if (vip == 1)
			break;
		temp_task = parent;
	} while (pid > 1 && time++ < MAX_FORK_TIME);
	rcu_read_unlock();

	return vip;
}

bool is_vip_cgroup(struct task_struct *p)
{
	char proc_cgroup_name[NAME_MAX + 1];
	if (!p)
		return false;

	rcu_read_lock();
	cgroup_name(task_cgroup(p, cpu_cgrp_id),
		proc_cgroup_name, sizeof(proc_cgroup_name));
	rcu_read_unlock();
 
	if (strcmp(proc_cgroup_name, VIP_GROUP_NAME) == 0)
		return true;
	return false;
}
#else
int check_vip_status(int cur_pid, int cur_tgid, struct task_struct *task)
{
	return 0;
}

bool is_vip_cgroup(struct task_struct *p)
{
	return false;
}
#endif

void iaware_proc_fork_connector(struct task_struct *task)
{
	pid_t cur_pid;
	pid_t cur_tgid;

	if (!task)
		return;

	cur_pid = task->pid;
	cur_tgid = task->tgid;

#if IS_ENABLED(CONFIG_COMMON_HW_RTG_FRAME)
	get_task_struct(task);
	if (check_key_aux_comm(task, task->comm))
		send_thread_comm_msg(PROC_AUX_COMM_FORK, cur_pid, cur_tgid);
	put_task_struct(task);
#elif defined(CONFIG_HW_RTG_SCHED) || defined(CONFIG_HW_RTG) || defined(CONFIG_HW_MTK_RTG_SCHED)
	get_task_struct(task);
	if (is_key_aux_comm(task, task->comm))
		send_thread_comm_msg(PROC_AUX_COMM_FORK, cur_pid, cur_tgid);
	put_task_struct(task);
#endif
#ifndef CONFIG_HW_FORK_THREAD_CHANGE_CGROUP
	if (check_vip_status(cur_pid, cur_tgid, task) == 1)
		send_vip_msg(cur_pid, cur_tgid);
#endif
}

#ifdef CONFIG_HW_FORK_THREAD_CHANGE_CGROUP
void record_cgroup_for_cpuctl_top_app(struct cgroup *cgrp, const char *name)
{
	if (cgroup_for_cpuctl_top_app == NULL) {
		if (cgrp == NULL)
			return;
		if (cgrp->subsys[cpu_cgrp_id] == NULL)
			return;
		if (strcmp(name, "top-app") == 0) {
			cgroup_for_cpuctl_top_app = cgrp;
			pr_info("[HW_VIP %s]cgroup_for_cpuctl_top_app: %p\n",
				__func__, cgroup_for_cpuctl_top_app);
		}
	}
}

void clear_cgroup_for_cpuctl_top_app(struct cgroup *cgrp)
{
	if (cgrp == cgroup_for_cpuctl_top_app)
		cgroup_for_cpuctl_top_app = NULL;
}

void record_cgroup_for_cpuset_top_app(struct cgroup *cgrp, const char *name)
{
	if (cgroup_for_cpuset_top_app == NULL) {
		if (cgrp == NULL)
			return;
		if (cgrp->subsys[cpuset_cgrp_id] == NULL)
			return;
		if (strcmp(name, "top-app") == 0) {
			cgroup_for_cpuset_top_app = cgrp;
			pr_info("[HW_VIP %s]cgroup_for_cpuset_top_app: %p\n",
				__func__, cgroup_for_cpuset_top_app);
		}
	}
}

void clear_cgroup_for_cpuset_top_app(struct cgroup *cgrp)
{
	if (cgrp == cgroup_for_cpuset_top_app)
		cgroup_for_cpuset_top_app = NULL;
}

static int attach_cgroup_to_task(struct cgroup *cgrp,
	struct task_struct *task)
{
	int ret = -EINVAL;

	if ((cgrp == NULL) || (task == NULL)) {
		pr_err("[HW_VIP %s]para err, cgrp %p, task %p\n",
			__func__, cgrp, task);
		return ret;
	}

	if (!cgroup_tryget(cgrp)) {
		pr_err("[HW_VIP %s]cgroup_tryget err, cgrp: %p\n", __func__, cgrp);
		return ret;
	}

	mutex_lock(&cgroup_mutex);
	if (cgroup_is_dead(cgrp)) {
		pr_err("[HW_VIP %s]cgroup_is_dead err, cgrp: %p\n", __func__, cgrp);
		goto err_cgroup;
	}

	percpu_down_write(&cgroup_threadgroup_rwsem);
	ret = cgroup_attach_task(cgrp, task, false);
	if (ret != 0)
		pr_err("[HW_VIP %s]cgroup_attach_task err: %d, cgrp: %p\n",
			__func__, ret, cgrp);
	percpu_up_write(&cgroup_threadgroup_rwsem);

err_cgroup:
	mutex_unlock(&cgroup_mutex);
	cgroup_put(cgrp);
	return ret;
}

static void send_msg_to_change_vip_cgroup(struct task_struct *task)
{
	pid_t cur_pid;
	pid_t cur_tgid;

	if (!task) {
		pr_err("[HW_VIP %s]send vip msg err, task is null\n", __func__);
		return;
	}
	cur_pid = task->pid;
	cur_tgid = task->tgid;
	send_vip_msg(cur_pid, cur_tgid);
}

static void top_app_attach_handler(struct work_struct *work)
{
	int ret;
	struct task_struct *task = NULL;
	struct top_app_attach_struct_t *attach_st = NULL;

	if (!work) {
		pr_err("[HW_VIP %s]work para err\n", __func__);
		return;
	}

	attach_st = container_of(work, struct top_app_attach_struct_t, my_work);
	if (!attach_st) {
		pr_err("[HW_VIP %s]get attach struct err\n", __func__);
		return;
	}

	task = attach_st->task;
	kfree(attach_st);
	if (!task) {
		pr_err("[HW_VIP %s]get task addr err\n", __func__);
		return;
	}
	if (task->static_vip != 0) {
		put_task_struct(task);
		return;
	}

	ret = attach_cgroup_to_task(cgroup_for_cpuctl_top_app, task);
	ret |= attach_cgroup_to_task(cgroup_for_cpuset_top_app, task);
	if (ret) {
		send_msg_to_change_vip_cgroup(task);
		pr_err("[HW_VIP %s]attach cgroup failed, seng msg to change cgroup.\n",
			__func__);
	}

	put_task_struct(task);
}

static int __init top_app_attach_init(void)
{
	cgroup_attach_wq = alloc_workqueue("%s", WQ_UNBOUND | WQ_HIGHPRI, 1,
		"top_app_attach_queue");
	if (!cgroup_attach_wq) {
		pr_err("[HW_VIP %s]create workqueue failed\n", __func__);
		return -ENOMEM;
	}

	return 0;
}

static void __exit top_app_attach_exit(void)
{
	flush_workqueue(cgroup_attach_wq);
	destroy_workqueue(cgroup_attach_wq);
	cgroup_attach_wq = NULL;
}

void top_app_cgroup_attach_task(struct task_struct *task)
{
	if (!task) {
		pr_err("[HW_VIP %s]cgroup attach err, para err\n", __func__);
		return;
	}

	if (!cgroup_attach_wq) {
		pr_err("[HW_VIP %s]wq is not init, send msg to change cgroup.\n",
			__func__);
		goto error;
	}

	struct top_app_attach_struct_t *worker_node =
		(struct top_app_attach_struct_t *)
		kmalloc(sizeof(struct top_app_attach_struct_t), GFP_KERNEL);
	if (!worker_node) {
		pr_err("[HW_VIP %s]worker_node alloc err, task: %p\n",
			__func__, task);
		goto error;
	}

	worker_node->task = task;
	INIT_WORK(&worker_node->my_work, top_app_attach_handler);
	queue_work(cgroup_attach_wq, &worker_node->my_work);
	return;

error:
	send_msg_to_change_vip_cgroup(task);
	put_task_struct(task);
}

module_init(top_app_attach_init);
module_exit(top_app_attach_exit);
#endif
