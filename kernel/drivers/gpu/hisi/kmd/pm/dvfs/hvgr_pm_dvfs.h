/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_DVFS_H
#define HVGR_PM_DVFS_H
#include <linux/types.h>
#include "hvgr_pm_defs.h"

#define GPU_THROTTLE_DEVFREQ_ENABLE 1
#define GPU_THROTTLE_DEVFREQ_DISABLE 0
#define GPU_UTIL_SAMPLING_PERIOD_MS 20

#define CL_JOB_WEIGHT (10000 / 50)


#ifdef CONFIG_HVGR_GPU_THROTTLE_DEVFREQ
void hvgr_gpu_devfreq_set_gpu_throttle(unsigned long freq_min,
	unsigned long freq_max, unsigned long *freq);
#endif

int hvgr_pm_metrics_init(struct hvgr_device *gdev);
void hvgr_pm_metrics_term(struct hvgr_device *gdev);

void hvgr_pm_start_timer(struct hvgr_device *gdev);
void hvgr_pm_cancel_timer(struct hvgr_device *gdev);

void hvgr_pm_dvfs_gpu_reset_proc(struct hvgr_device *gdev);

#ifdef CONFIG_GPUFREQ_INDEPDENT
int hvgr_pm_gpufreq_init(struct hvgr_device *gdev);

int hvgr_pm_get_busytime(void);

int hvgr_pm_vsync_hit(void);

int hvgr_pm_get_cl_boost(void);
#endif
#endif
