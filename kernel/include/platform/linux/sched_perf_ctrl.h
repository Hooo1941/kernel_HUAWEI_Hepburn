/*
 * sched_perf_ctrl.h
 *
 * sched perf ctrl header file

 * Copyright (c) 2020, Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SCHED_PERF_CTRL_H
#define __SCHED_PERF_CTRL_H

#include <linux/version.h>
#include <linux/sched.h>

struct sched_stat {
	pid_t pid;
	unsigned long long sum_exec_runtime;
	unsigned long long run_delay;
	unsigned long pcount;
};

struct sched_task_config {
	pid_t pid;
	unsigned int value;
};

struct sched_tid_info {
	pid_t tid;
	char tid_name[TASK_COMM_LEN];
};

#if IS_ENABLED(CONFIG_SCHED_INFO) && IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
int perf_ctrl_get_sched_stat(void __user *uarg);
#else
static inline int perf_ctrl_get_sched_stat(void __user *uarg)
{
	return -EFAULT;
}
#endif

static inline int perf_ctrl_set_task_util(void __user *uarg)
{
	return 0;
}

#if IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
int perf_ctrl_get_related_tid(void __user *uarg);
int perf_ctrl_get_tid_name(void __user *uarg);
#else
static inline int perf_ctrl_get_related_tid(void __user *uarg)
{
	return -EFAULT;
}
static inline int perf_ctrl_get_tid_name(void __user *uarg)
{
	return -EFAULT;
}
#endif

#if IS_ENABLED(CONFIG_HUAWEI_SCHED_VIP) && IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
int perf_ctrl_set_vip_prio(void __user *uarg);
#else
static inline int perf_ctrl_set_vip_prio(void __user *uarg)
{
	return -EFAULT;
}
#endif

#if IS_ENABLED(CONFIG_HW_FAVOR_SMALL_CAP) && IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
int perf_ctrl_set_favor_small_cap(void __user *uarg);
#else
static inline int perf_ctrl_set_favor_small_cap(void __user *uarg)
{
	return -EFAULT;
}
#endif

#if IS_ENABLED(CONFIG_SCHED_TASK_UTIL_CLAMP) && IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
int perf_ctrl_set_task_min_util(void __user *uarg);
int perf_ctrl_set_task_max_util(void __user *uarg);
#else
static inline int perf_ctrl_set_task_min_util(void __user *uarg)
{
	return -EFAULT;
}

static inline int perf_ctrl_set_task_max_util(void __user *uarg)
{
	return -EFAULT;
}
#endif

#if IS_ENABLED(CONFIG_HUAWEI_SCHED_STAT_YIELD) && IS_ENABLED(CONFIG_SCHED_PERF_CTRL)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
void sched_stat_yield_init(void *unused, struct task_struct *p);
void sched_stat_yield_update(void *unused, struct task_struct *unused2);
#endif
int perf_ctrl_get_task_yield_time(void __user *uarg);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static inline void sched_stat_yield_init(void *unused, struct task_struct *p)
{
}
static inline void sched_stat_yield_update(void *unused, struct task_struct *unused2)
{
}
#endif
static inline int perf_ctrl_get_task_yield_time(void __user *uarg)
{
	return -EFAULT;
}
#endif

#endif
