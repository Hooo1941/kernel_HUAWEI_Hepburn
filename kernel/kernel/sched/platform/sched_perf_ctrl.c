// SPDX-License-Identifier: GPL-2.0
/*
 * sched_perf_ctrl.c
 *
 * sched related interfaces defined for perf_ctrl
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/sched/cputime.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#ifdef CONFIG_HW_USERSPACE_THROTTLE
#include <trace/events/sched.h>
#include "sched.h"
#endif

#ifdef CONFIG_RENDER_RT_GLOBAL
#include <linux/cgroup.h>
#include <securec.h>
#endif

#ifdef CONFIG_SCHED_RTG
#include <platform_include/cee/linux/hisi_rtg.h>
#endif
#ifdef CONFIG_SCHED_SMT
#include <linux/sched/smt.h>
#endif

#include <platform_include/cee/linux/sched_perf_ctrl.h>
#include <platform_include/cee/linux/render_rt.h>

#ifdef CONFIG_HECA
#include <linux/platform_drivers/heca.h>
#endif

static struct task_struct *find_process_by_pid(pid_t pid)
{
	return pid ? find_task_by_vpid(pid) : current;
}

#ifdef CONFIG_SCHED_INFO
static int get_schedstat(struct sched_stat *sched_stat_val)
{
	struct task_struct *task = NULL;

	rcu_read_lock();
	task = find_task_by_vpid(sched_stat_val->pid);
	if (task == NULL) {
		rcu_read_unlock();
		pr_err("%s: get task from pid=%d fail\n",
		       __func__, sched_stat_val->pid);
		return -ENOENT;
	}
	get_task_struct(task);
	rcu_read_unlock();

	sched_stat_val->sum_exec_runtime = task_sched_runtime(task);
	sched_stat_val->run_delay = task->sched_info.run_delay;
	sched_stat_val->pcount = task->sched_info.pcount;
	put_task_struct(task);

	return 0;
}

int perf_ctrl_get_sched_stat(void __user *uarg)
{
	struct sched_stat sched_stat_val;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&sched_stat_val, uarg, sizeof(struct sched_stat))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = get_schedstat(&sched_stat_val);
	if (ret != 0) {
		pr_err("%s: fail, ret=%d\n", __func__, ret);
		return ret;
	}

	if (copy_to_user(uarg, &sched_stat_val, sizeof(struct sched_stat))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		ret = -EFAULT;
	}

	return ret;
}
#endif

static struct task_struct *get_pid_leader_task(pid_t pid)
{
	struct task_struct *cur = NULL;
	struct task_struct *leader = NULL;

	cur = find_task_by_vpid(pid);
	if (cur == NULL) {
		pr_err("%s: get task from pid=%d fail\n", __func__, pid);
		return NULL;
	}

	get_task_struct(cur);
	leader = cur->group_leader;
	put_task_struct(cur);
	if (leader == NULL) {
		pr_err("%s: get leader task from pid=%d fail\n", __func__, pid);
		return NULL;
	}

	return leader;
}

int perf_ctrl_get_related_tid(void __user *uarg)
{
	struct related_tid_info *r_t_info = NULL;
	struct task_struct *leader = NULL;
	struct task_struct *pos = NULL;
	int ret = 0;
	int count = 0;

	if (uarg == NULL)
		return -EINVAL;

	r_t_info = kzalloc(sizeof(struct related_tid_info), GFP_KERNEL);
	if (r_t_info == NULL) {
		pr_err("%s: kzalloc fail out of memory\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(r_t_info, uarg, sizeof(struct related_tid_info))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		ret = -EFAULT;
		goto free;
	}

	rcu_read_lock();
	leader = get_pid_leader_task(r_t_info->pid);
	pos = leader;
	if (pos == NULL || leader == NULL) {
		rcu_read_unlock();
		pr_err("%s: get task from pid=%d fail\n",
		       __func__, r_t_info->pid);
		ret = -ENODEV;
		goto free;
	}

	// cppcheck-suppress *
	do {
		if (count >= MAX_TID_COUNT) {
			pr_err("%s: related_tid %d >= MAX_TID_COUNT\n",
			       __func__, count);
			break;
		}
		r_t_info->rel_tid[count++] = pos->pid;
	} while_each_thread(leader, pos);
	rcu_read_unlock();

	r_t_info->rel_count = count;
	if (copy_to_user(uarg, r_t_info, sizeof(struct related_tid_info))) {
		pr_err("%s: copy_to_user fail\n", __func__);
		ret = -EFAULT;
		goto free;
	}

free:
	kfree(r_t_info);
	return ret;
}

#ifdef CONFIG_RENDER_RT_GLOBAL
int perf_ctrl_get_tid_name(void __user *uarg)
{
	struct sched_tid_info tid_info;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&tid_info, uarg, sizeof(struct sched_tid_info)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(tid_info.tid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	strncpy_s(tid_info.tid_name, sizeof(tid_info.tid_name), p->comm, sizeof(p->comm));
	rcu_read_unlock();

	if (copy_to_user(uarg, &tid_info, sizeof(struct sched_tid_info)))
		return -EFAULT;

	return 0;
}

int perf_ctrl_get_cgroup_name(void __user *uarg)
{
	char *name_buf;
	struct sched_tid_info tid_info;
	struct task_struct *p = NULL;
	struct cgroup_subsys_state *css = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&tid_info, uarg, sizeof(struct sched_tid_info)))
		return -EFAULT;

	rcu_read_lock();

	p = find_task_by_vpid(tid_info.tid);
	if (!p) {
		rcu_read_unlock();
		return -ESRCH;
	}

	css = task_css(p, cpu_cgrp_id);
	if (!css) {
		rcu_read_unlock();
		return -ESRCH;
	}
	cgroup_name(css->cgroup, tid_info.cgrp_name, CGRP_NAME_LEN);
	rcu_read_unlock();

	if (copy_to_user(uarg, &tid_info, sizeof(struct sched_tid_info)))
		return -EFAULT;
	return 0;
}
#endif

#ifdef CONFIG_SCHED_RTG
int perf_ctrl_set_task_rtg(void __user *uarg)
{
	struct rtg_group_task task;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&task, uarg, sizeof(struct rtg_group_task))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_id(task.pid, task.grp_id);
	return ret;
}

int perf_ctrl_set_rtg_cpus(void __user *uarg)
{
	struct rtg_cpus cpus;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cpus, uarg, sizeof(struct rtg_cpus))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_preferred_cluster(cpus.grp_id, cpus.cluster_id);
	return ret;
}

int perf_ctrl_set_rtg_freq(void __user *uarg)
{
	struct rtg_freq freqs;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&freqs, uarg, sizeof(struct rtg_freq))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_freq(freqs.grp_id, freqs.freq);
	return ret;
}

int perf_ctrl_set_rtg_freq_update_interval(void __user *uarg)
{
	struct rtg_interval interval;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&interval, uarg, sizeof(struct rtg_interval))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_freq_update_interval(interval.grp_id,
						   interval.interval);
	return ret;
}

int perf_ctrl_set_rtg_util_invalid_interval(void __user *uarg)
{
	struct rtg_interval interval;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&interval, uarg, sizeof(struct rtg_interval))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_util_invalid_interval(interval.grp_id,
						    interval.interval);
	return ret;
}

int perf_ctrl_set_rtg_load_mode(void __user *uarg)
{
	struct rtg_load_mode mode;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&mode, uarg, sizeof(struct rtg_load_mode))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_load_mode(&mode);
	return ret;
}

int perf_ctrl_set_rtg_ed_params(void __user *uarg)
{
	struct rtg_ed_params params;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&params, uarg, sizeof(struct rtg_ed_params))) {
		pr_err("%s: copy_from_user fail\n", __func__);
		return -EFAULT;
	}

	ret = sched_set_group_ed_params(&params);
	return ret;
}

int perf_ctrl_set_task_rtg_min_freq(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_task_by_vpid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	ret = set_task_rtg_min_freq(p, cfg.value);

	put_task_struct(p);
	return ret;
}
#endif

#ifdef CONFIG_HUAWEI_SCHED_VIP
int perf_ctrl_set_vip_prio(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	ret = set_vip_prio(p, cfg.value);

	put_task_struct(p);
	return ret;
}
#endif

#ifdef CONFIG_FRAME_VIP_SCHED
int perf_ctrl_set_frame_vip_prio(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	set_vip_boost(p, cfg.value, VIP_BOOST_FRAME);

	put_task_struct(p);

	return 0;
}
#endif

#ifdef CONFIG_HW_USERSPACE_THROTTLE
int perf_ctrl_set_task_userspace_throttle(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	if (cfg.value < PERF_CTRL_ENABLE_TASK_THROTTLE || cfg.value >= PERF_CTRL_TASK_THROTTLE_MAX)
		return -EINVAL;

	rcu_read_lock();
	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}
	get_task_struct(p);
	rcu_read_unlock();

	if (cfg.value == PERF_CTRL_DISABLE_TASK_THROTTLE) {
		test_and_set_tsk_thread_flag(p, TIF_USERSPACE_UNTHROTTLED);
		test_and_clear_tsk_thread_flag(p, TIF_NEED_THROTTLED);
		sched_unthrottle_userspace_task(p);
	} else if (cfg.value == PERF_CTRL_ENABLE_TASK_THROTTLE) {
		test_and_clear_tsk_thread_flag(p, TIF_USERSPACE_UNTHROTTLED);
	}

	trace_perf(sched_userspace_throttle, p, task_rq(p), cfs_rq_of(&p->se),
			   cfg.value ? OPERATION_DISABLE_TASK_THROTTLE : OPERATION_ENABLE_TASK_THROTTLE);

	put_task_struct(p);
	return 0;
}
#endif

#ifdef CONFIG_HISI_EAS_SCHED
int perf_ctrl_set_favor_small_cap(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	if (cfg.value)
		set_tsk_thread_flag(p, TIF_FAVOR_SMALL_CAP);
	else
		clear_tsk_thread_flag(p, TIF_FAVOR_SMALL_CAP);

	rcu_read_unlock();
	return 0;
}

int perf_ctrl_get_task_pid(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	cfg.value = task_tgid_vnr(p);

	rcu_read_unlock();

	if (copy_to_user(uarg, &cfg, sizeof(struct task_config)))
		return -EFAULT;

	return 0;
}
#endif

#ifdef CONFIG_SCHED_HISI_UTIL_CLAMP
int perf_ctrl_set_task_min_util(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	ret = set_task_min_util(p, cfg.value);

	put_task_struct(p);

	return ret;
}

#ifdef CONFIG_HECA
int perf_ctrl_set_heca_config(void __user *uarg)
{
	return heca_perf_ctrl(uarg);
}
#endif

int perf_ctrl_set_task_max_util(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;
	int ret;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

	ret = set_task_max_util(p, cfg.value);

	put_task_struct(p);

	return ret;
}
#endif

#ifdef CONFIG_SCHED_STAT_YIELD
int perf_ctrl_get_task_yield_time(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	cfg.value = p->cumulative_yield_time;
	rcu_read_unlock();

	if (copy_to_user(uarg, &cfg, sizeof(struct task_config)))
		return -EFAULT;

	return 0;
}
#endif

#ifdef CONFIG_SCHED_SMT
int perf_ctrl_set_task_no_smt(void __user *uarg)
{
	struct task_config cfg;
	struct task_struct *p = NULL;

	if (uarg == NULL)
		return -EINVAL;

	if (copy_from_user(&cfg, uarg, sizeof(struct task_config)))
		return -EFAULT;

	rcu_read_lock();

	p = find_process_by_pid(cfg.pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -ESRCH;
	}

	get_task_struct(p);
	rcu_read_unlock();

#ifdef CONFIG_SCHED_SMT_EXPELLING
	if (cfg.value)
		p->smt_expeller = SMT_EXPELLER;
	else
		p->smt_expeller = SMT_EXPELLEE;
#endif

	put_task_struct(p);

	return 0;
}
#endif
