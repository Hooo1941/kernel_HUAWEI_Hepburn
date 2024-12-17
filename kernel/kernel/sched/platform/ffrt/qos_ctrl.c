// SPDX-License-Identifier: GPL-2.0
/*
 * qos_ctrl.c
 *
 * ffrt qos ioctl
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
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <uapi/linux/sched/types.h>
#include <securec.h>
#include "qos_ctrl.h"
#include <trace/events/ffrt.h>

typedef long (*qos_ctrl_func)(struct file *file, void __user *uarg);

static long ctrl_qos_operation(struct file *file, void __user *uarg);
static long ctrl_qos_policy(struct file *file, void __user *uarg);
static long ctrl_thread_attr(struct file *file, void __user *uarg);

static qos_ctrl_func g_func_array[QOS_CTRL_MAX_NR] = {
	NULL, /* reserved */
	ctrl_qos_operation,
	ctrl_qos_policy,
	ctrl_thread_attr,
};

static struct qos_policy_map qos_policy_array[QOS_POLICY_MAX_NR];

static long ffrt_sched_set_latency_sensitive(struct task_struct *task, u8 prefer_idle)
{
	if (prefer_idle)
		set_tsk_thread_flag(task, TIF_LATENCY_SENSITIVE);
	return 0;
}

static long ffrt_sched_set_ign_initial_load(struct task_struct *task, u8 init_load)
{
	if (init_load)
		set_tsk_thread_flag(task, TIF_IGN_INITIAL_LOAD);
	return 0;
}

/* This function must be called when p is valid. That means the p's refcount must exist */
static int sched_set_task_qos_attr(struct task_struct *p, int level, int status)
{
	struct qos_policy_item *item = NULL;
	struct qos_policy_map *policy_map = NULL;
	struct sched_attr attr = {0};
	int ret;

	read_lock(&qos_policy_array[status].lock);
	if (!qos_policy_array[status].initialized) {
		pr_err("[QOS_CTRL] dirty qos policy, pid=%d, uid=%d, status=%d\n",
		       p->pid, p->cred->uid.val, status);
		read_unlock(&qos_policy_array[status].lock);
		return -DIRTY_QOS_POLICY;
	}

	policy_map = &qos_policy_array[status];
	item = &policy_map->levels[level];

	attr.sched_policy = -1;
	attr.sched_flags = SCHED_FLAG_KEEP_PARAMS;

	if (policy_map->policy_flag & QOS_FLAG_LATENCY_NICE) {
		attr.sched_flags |= SCHED_FLAG_LATENCY_NICE;
		attr.sched_latency_nice = item->latency_nice;
	}

	if (policy_map->policy_flag & QOS_FLAG_UCLAMP) {
		attr.sched_flags |= SCHED_FLAG_UTIL_CLAMP;
		attr.sched_util_min = (unsigned int)(item->uclamp_min);
		attr.sched_util_max = (unsigned int)(item->uclamp_max);
	}

	read_unlock(&qos_policy_array[status].lock);

	if (unlikely(p->flags & PF_EXITING)) {
		pr_info("[QOS_CTRL] dying task, no need to set qos\n");
		return -THREAD_EXITING;
	}

	trace_perf(ffrt_qos_attr, level, (u64)(p->pid), attr.sched_latency_nice, attr.sched_util_min, attr.sched_util_max,
	item->affinity, item->priority, item->init_load, item->prefer_idle);

	ret = sched_setattr_nocheck(p, &attr);
	if (ret) {
		pr_err("[QOS_CTRL] set qos_attr failed\n");
		return ret;
	}
	ffrt_sched_set_ign_initial_load(p, item->init_load);
	return ffrt_sched_set_latency_sensitive(p, item->prefer_idle);
}

static inline bool same_uid(struct task_struct *dude, struct task_struct *bro)
{
	return uid_eq(task_uid(dude), task_uid(bro));
}

static inline bool super_user(struct task_struct *p)
{
	return super_uid(task_uid(p).val);
}

/*
 * judge permission for changing other tasks' qos
 */
static bool can_change_qos(struct task_struct *p)
{
	if (p != current && !same_uid(current, p) && !super_user(current)) {
		pr_err("[QOS_CTRL] %d apply for others not permit\n", p->pid);
		return false;
	}

	return true;
}

static int qos_apply(struct qos_ctrl_data *data)
{
	int level = (int)data->level;
	struct task_struct *p;
	int pid = data->pid;
	int ret;

	if (level > NR_QOS || level <= NO_QOS) {
		pr_err("[QOS_CTRL] no this qos level, qos apply failed\n");
		ret = -ARG_INVALID;
		goto out;
	}

	p = find_get_task_by_vpid((pid_t)pid);
	if (unlikely(!p)) {
		pr_err("[QOS_CTRL] no matching task for this pid, qos apply failed\n");
		ret = -ESRCH;
		goto out;
	}

	if (unlikely(p->flags & PF_EXITING)) {
		pr_info("[QOS_CTRL] dying task, no need to set qos\n");
		ret = -THREAD_EXITING;
		goto out_put_task;
	}

	if (!can_change_qos(p)) {
		pr_err("[QOS_CTRL] apply for others not permit\n");
		ret = -ARG_INVALID;
		goto out_put_task;
	}

	ret = sched_set_task_qos_attr(p, level, QOS_POLICY_SYSTEM);
	if (ret)
		pr_err("[QOS_CTRL] set qos_level %d for thread %d on status %d failed\n", level, p->pid, QOS_POLICY_SYSTEM);

out_put_task:
	put_task_struct(p);
out:
	return ret;
}

static int qos_leave(struct qos_ctrl_data *data)
{
	unsigned int level = data->level;
	struct task_struct *p;
	int pid = data->pid;
	int ret;

	p = find_get_task_by_vpid((pid_t)pid);
	if (!p) {
		pr_err("[QOS_CTRL] no matching task for this pid, qos apply failed\n");
		ret = -ESRCH;
		goto out;
	}

	if (unlikely(p->flags & PF_EXITING)) {
		pr_info("[QOS_CTRL] dying task, no need to set qos\n");
		ret = -THREAD_EXITING;
		goto out_put_task;
	}

	if (!can_change_qos(p)) {
		pr_err("[QOS_CTRL] apply for others not permit\n");
		ret = -ARG_INVALID;
		goto out_put_task;
	}

	/*
	 * NO NEED to judge whether current status is AUTH_STATUS_DISABLE.
	 * In the auth destoring context, the removing of thread's sched attr was protected by
	 * auth->mutex, QOS_POLICY_DEFAULT will never appear here.
	 *
	 * The second param level means nothing, actually you can use any valid level here, cause the
	 * policy matching QOS_POLICY_DEFAULT has default parameters for all qos level, which can
	 * keep a powerful thread to behave like a ordinary thread.
	 */
	ret = sched_set_task_qos_attr(p, (int)level, QOS_POLICY_DEFAULT);
	if (ret)
		pr_err("[QOS_CTRL] set qos_level %d for thread %d to default failed\n", level, p->pid);

out_put_task:
	put_task_struct(p);
out:
	return ret;
}

typedef int (*qos_manipulate_func)(struct qos_ctrl_data *data);

static qos_manipulate_func qos_func_array[QOS_OPERATION_CMD_MAX_NR] = {
	NULL,
	qos_apply,  // 1
	qos_leave,
};

static long do_qos_manipulate(struct qos_ctrl_data *data)
{
	long ret = 0;
	unsigned int type = data->type;

	if (qos_func_array[type])
		ret = (long)(*qos_func_array[type])(data);

	return ret;
}

static bool valid_qos_operation(struct qos_ctrl_data *data)
{
	unsigned int type = data->type;
	unsigned int level = data->level;

	if (type >= QOS_OPERATION_CMD_MAX_NR) {
		pr_err("[QOS_CTRL] CMD_ID_QOS_MANIPULATE type not valid\n");
		goto out_failed;
	}

	if (level > NR_QOS) {
		pr_err("[QOS_CTRL] CMD_ID_QOS_MANIPULATE level not valid\n");
		goto out_failed;
	}

	return true;
out_failed:
	return false;
}

static long ctrl_qos_operation(struct file *file __maybe_unused, void __user *uarg)
{
	struct qos_ctrl_data qos_data;

	if (copy_from_user(&qos_data, uarg, sizeof(qos_data))) {
		pr_err("[QOS_CTRL] CMD_ID_QOS_APPLY copy data failed\n");
		return -ARG_INVALID;
	}

	if (!valid_qos_operation(&qos_data))
		return -ARG_INVALID;


	return do_qos_manipulate(&qos_data);
}

static inline bool valid_latency_nice(int latency_nice)
{
	return latency_nice >= MIN_LATENCY_NICE && latency_nice <= MAX_LATENCY_NICE;
}

static inline bool valid_uclamp(int uclamp_min, int uclamp_max)
{
	if (uclamp_min > uclamp_max)
		return false;
	if (uclamp_max > SCHED_CAPACITY_SCALE)
		return false;

	return true;
}

static bool valid_qos_flag(unsigned int qos_flag)
{
	return !(qos_flag & ~QOS_FLAG_ALL);
}

static bool valid_qos_item(struct qos_policy_datas *datas)
{
	int i;
	int policy_type = datas->policy_type;
	struct qos_policy_item *data = NULL;

	if (policy_type <= 0 || policy_type >= QOS_POLICY_MAX_NR) {
		pr_err("[QOS_CTRL] not valid qos policy type, policy change failed\n");
		goto out_failed;
	}

	if (!valid_qos_flag(datas->policy_flag)) {
		pr_err("[QOS_CTRL] not valid qos flag, policy change failed\n");
		goto out_failed;
	}

	/* check user space qos polcicy data, level 0 reserved */
	for (i = 0; i <= NR_QOS; ++i) {
		data = &datas->policys[i];

		if (!valid_latency_nice(data->latency_nice)) {
			pr_err("[QOS_CTRL] invalid latency_nice, policy change failed\n");
			goto out_failed;
		}

		if (!valid_uclamp(data->uclamp_min, data->uclamp_max)) {
			pr_err("[QOS_CTRL] invalid uclamp, policy change failed\n");
			goto out_failed;
		}
	}

	return true;

out_failed:
	pr_err("[QOS_CTRL] not valid qos policy params\n");
	return false;
}

static void qos_policy_change(struct qos_policy_item *data, int policy_type, int level)
{
	struct qos_policy_item *item = NULL;

	item = &qos_policy_array[policy_type].levels[level];
	item->latency_nice = data->latency_nice;
	item->uclamp_min = data->uclamp_min;
	item->uclamp_max = data->uclamp_max;
	item->affinity = data->affinity;
	item->priority = data->priority;
	item->init_load = data->init_load;
	item->prefer_idle = data->prefer_idle;
}

static long do_qos_policy_change(struct qos_policy_datas *datas)
{
	long ret = 0;
	int i;
	struct qos_policy_item *data = NULL;
	int policy_type = datas->policy_type;

	if (!valid_qos_item(datas))
		goto out_failed;

	write_lock(&qos_policy_array[policy_type].lock);
	for (i = QOS_POLICY_MIN_LEVEL; i <= NR_QOS; ++i) {
		/* user space policy params */
		data = &datas->policys[i];
		qos_policy_change(data, policy_type, i);
	}
	qos_policy_array[policy_type].policy_flag = datas->policy_flag;
	qos_policy_array[policy_type].initialized = true;
	write_unlock(&qos_policy_array[policy_type].lock);

	return ret;

out_failed:
	return -ARG_INVALID;
}

static long ctrl_qos_policy(struct file *file __maybe_unused, void __user *uarg)
{
	struct qos_policy_datas policy_datas;

	if (copy_from_user(&policy_datas, uarg, sizeof(policy_datas))) {
		pr_err("[QOS_CTRL] CMD_ID_QOS_APPLY copy data failed\n");
		return -ARG_INVALID;
	}

	return do_qos_policy_change(&policy_datas);
}

static long do_ctrl_thread_attr(struct thread_attr_ctrl *ctrl_data)
{
	int tid = ctrl_data->tid;
	bool priority_set_enable = ctrl_data->priority_set_enable;
	bool affinity_set_enable = ctrl_data->affinity_set_enable;
	struct task_struct *p;

	p = find_get_task_by_vpid((pid_t)tid);
	if (unlikely(!p)) {
		pr_err("[QOS_CTRL] no matching task for this pid, set thread attr failed\n");
		return -ESRCH;
	}

	if (unlikely(p->flags & PF_EXITING)) {
		pr_info("[QOS_CTRL] dying task, no need to set thread attr\n");
		put_task_struct(p);
		return -THREAD_EXITING;
	}

	if (priority_set_enable) {
		clear_tsk_thread_flag(p, TIF_NO_USER_SETPRIO);
	} else {
		set_tsk_thread_flag(p, TIF_NO_USER_SETPRIO);
	}

	if (affinity_set_enable) {
		clear_tsk_thread_flag(p, TIF_NO_SETAFFINITY);
	} else {
		set_tsk_thread_flag(p, TIF_NO_SETAFFINITY);
	}

	put_task_struct(p);
	return 0;
}

static long ctrl_thread_attr(struct file *file __maybe_unused, void __user *uarg)
{
	struct thread_attr_ctrl ctrl_data;

	if (copy_from_user(&ctrl_data, uarg, sizeof(ctrl_data))) {
		pr_err("[QOS_CTRL] QOS_THREAD_CTRL copy data failed\n");
		return -ARG_INVALID;
	}

	return do_ctrl_thread_attr(&ctrl_data);
}

long do_qos_ctrl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	unsigned int func_cmd = _IOC_NR(cmd);

	if (uarg == NULL) {
		pr_err("%s: invalid user uarg\n", __func__);
		return -EINVAL;
	}

	if (_IOC_TYPE(cmd) != QOS_CTRL_IPC_MAGIG) {
		pr_err("%s: qos ctrl magic fail, TYPE=%d\n",
		       __func__, _IOC_TYPE(cmd));
		return -EINVAL;
	}

	if (func_cmd >= QOS_CTRL_MAX_NR) {
		pr_err("%s: qos ctrl cmd error, cmd:%d\n",
		       __func__, _IOC_TYPE(cmd));
		return -EINVAL;
	}

	if (g_func_array[func_cmd])
		return (*g_func_array[func_cmd])(file, uarg);

	return -EINVAL;
}

static void init_qos_policy_array(void)
{
	int i;

	/* index 0 reserved */
	for (i = 1; i < QOS_POLICY_MAX_NR; ++i)
		rwlock_init(&qos_policy_array[i].lock);

	pr_info("[QOS_CTRL] lock in qos policy initialized\n");
}

static int __init init_qos_ctrl(void)
{
	init_qos_policy_array();
	return 0;
}

/* module entry points */
module_init(init_qos_ctrl);
