/*
 * hotplug_thermal.h
 *
 * header of thermal hotplug
 *
 * Copyright (C) 2020-2022 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __THERMAL_HOTPLUG_H
#define __THERMAL_HOTPLUG_H
#include <linux/thermal.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/types.h>

#define DECIMAL	10

enum {
	HOTPLUG_NONE = 0,
	HOTPLUG_NORMAL,
	HOTPLUG_CRITICAL
};

extern struct thermal_hotplug g_thermal_hotplug;

#ifdef CONFIG_THERMAL_GPU_HOTPLUG
struct gpu_hotplug_ops_t {
	gpu_hotplug_t gpu_hotplug;
};
#endif

#ifdef CONFIG_THERMAL_HOTPLUG
#define MAX_MODE_LEN	15
struct hotplug_t {
	struct device *device;
	int cpu_down_threshold;
	int cpu_up_threshold;
	int critical_cpu_down_threshold;
	int critical_cpu_up_threshold;
	int hotplug_status;
	unsigned int cpu_hotplug_mask;
	unsigned int critical_cpu_hotplug_mask;
	unsigned int control_mask; /* diff mask */
	bool need_down;
	bool need_up;
	long current_temp;
#ifdef CONFIG_HOTPLUG_EMULATION
	int cpu_emul_temp;
#endif
	bool cpu_downed;
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	bool gpu_need_down;
	bool gpu_need_up;
	bool gpu_downed;
	int gpu_down_threshold;
	int gpu_up_threshold;
	unsigned int gpu_limit_cores;
	unsigned int gpu_total_cores;
	int gpu_hotplug_status;
	unsigned int ori_gpu_limit_cores;
	unsigned int ori_gpu_total_cores;
	unsigned int critical_gpu_down_threshold;
	unsigned int critical_gpu_up_threshold;
	unsigned int critical_gpu_limit_cores;
	long gpu_current_temp;
#endif
#ifdef CONFIG_GPU_HOTPLUG_EMULATION
	int gpu_emul_temp;
#endif
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
	bool npu_need_down;
	bool npu_need_up;
	bool npu_downed;
	int npu_down_threshold;
	int npu_up_threshold;
	unsigned int npu_limit_cores;
	unsigned int npu_total_cores;
	long npu_current_temp;
#endif
#ifdef CONFIG_NPU_HOTPLUG_EMULATION
	int npu_emul_temp;
#endif
	struct task_struct *hotplug_task;
	struct delayed_work poll_work;
	u32 polling_delay;
	u32 polling_delay_passive;
	spinlock_t hotplug_lock;
	bool disabled;
	bool initialized;
};

struct thermal_hotplug {
	struct class *thermal_class;
	struct hotplug_t hotplug;
};
#endif

int thermal_hotplugs_init(void);
void thermal_hotplugs_exit(void);
#ifdef CONFIG_THERMAL_NPU_HOTPLUG
#define NPU_DEV_ID	0
#endif
#endif
