/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_pm_dvfs.h"
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/version.h>

#ifdef CONFIG_GPUFREQ_INDEPDENT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <platform_include/cee/linux/gpufreq_v2.h>
#else
#include <linux/platform_drivers/gpufreq_v2.h>
#endif
#endif

#include <linux/spinlock.h>
#include <linux/device.h>
#include <securec.h>

#include "hvgr_assert.h"
#include "hvgr_dm_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_driver.h"
#include "hvgr_pm_api.h"

/*
 * caller needs to hold pm_dvfs->dvfs_lock before calling this
 * function
 */
static void hvgr_pm_accumulate_gpu_run_time(struct hvgr_device *gdev,
	ktime_t now)
{
	ktime_t diff;
	uint32_t ns_time;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	diff = ktime_sub(now, pm_dvfs->time_stamp);
	if (ktime_to_ns(diff) < 0)
		return;

	ns_time = (uint32_t)ktime_to_ns(diff) >> HVGR_PM_TIME_SHIFT;

	if (pm_dvfs->gpu_active)
		pm_dvfs->cur.time_busy += ns_time;
	else
		pm_dvfs->cur.time_idle += ns_time;

	if (pm_dvfs->cl_job_flag)
		pm_dvfs->cur.cl_time += ns_time;
	pm_dvfs->time_stamp = now;
}


void hvgr_pm_set_cl_job_flag(struct hvgr_device *gdev, ktime_t now, bool status)
{
	unsigned long flags;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	hvgr_pm_accumulate_gpu_run_time(gdev, now);
	pm_dvfs->cl_job_flag = status;
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

void hvgr_pm_change_gpu_active_status(struct hvgr_device *gdev, ktime_t now, bool status)
{
	unsigned long flags;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	hvgr_pm_accumulate_gpu_run_time(gdev, now);
	pm_dvfs->gpu_active = status;
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

void hvgr_pm_dvfs_gpu_reset_proc(struct hvgr_device *gdev)
{
	unsigned long flags;
	ktime_t now;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	now = ktime_get();
	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	hvgr_pm_accumulate_gpu_run_time(gdev, now);
	pm_dvfs->gpu_active = false;
	pm_dvfs->cl_job_flag = false;
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

static void hvgr_pm_calc_gpu_utilisation(struct hvgr_device *gdev)
{
	uint32_t time_busy, time_idle, cl_time;
	uint32_t cl_rate, gpu_util;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	time_busy = pm_dvfs->cur.time_busy - pm_dvfs->last.time_busy;
	time_idle = pm_dvfs->cur.time_idle - pm_dvfs->last.time_idle;
	cl_time = pm_dvfs->cur.cl_time - pm_dvfs->last.cl_time;

	hvgr_info(gdev, HVGR_PM, "busy time = %u, cl time = %u, idle time = %u.",
		time_busy, cl_time, time_idle);
	gpu_util = (100 * time_busy) / max(time_busy + time_idle, 1U);
	/*
	 * time_busy indicates the running time of the job on the GPU, and cl job is a subset of
	 * the jobs supported by the GPU. Therefore, cl_time cannot be greater than time_busy.
	 * If cl_time is greater than time_busy, the system is running incorrectly.
	 */
	if (cl_time > time_busy)
		hvgr_err(gdev, HVGR_PM,
			"CL time is greater than busy time, cl_time = %d, busy_time = %d.",
			cl_time, time_busy);

	cl_rate = (100 * cl_time) / max(time_busy, 1u);
	cl_rate = cl_rate > 100 ? 100 : cl_rate;
	/*
	 * One way to speed up cl jobs is to increase the GPU frequency. Increasing the GPU load
	 * indirectly increases the GPU frequency. The following expression is actually a
	 * cl-weighted GPU load calculation: gpu_util = gpu_util * (1 + 0.01 * cl_rate)
	 */
	gpu_util = gpu_util * (CL_JOB_WEIGHT * cl_rate + 100 * (100 - cl_rate));
	gpu_util /= 10000;
	gpu_util = gpu_util > 100 ? 100 : gpu_util;

	pm_dvfs->cl_boost = cl_rate != 0;
	pm_dvfs->utilisation = (int)gpu_util;
	hvgr_info(gdev, HVGR_PM, "cl boost = %d,  utilisation = %d.",
		pm_dvfs->cl_boost, pm_dvfs->utilisation);
}

void hvgr_pm_get_dvfs_action(struct hvgr_device *gdev)
{
	ktime_t now;
	unsigned long flags;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	now = ktime_get();
	hvgr_pm_accumulate_gpu_run_time(gdev, now);
	hvgr_pm_calc_gpu_utilisation(gdev);
	pm_dvfs->last = pm_dvfs->cur;
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

#ifdef CONFIG_GPUFREQ_INDEPDENT
int hvgr_pm_get_busytime(void)
{
	struct hvgr_device *gdev = hvgr_get_device();

	if (gdev == NULL)
		return 0;
	hvgr_pm_get_dvfs_action(gdev);
	return gdev->pm_dev.pm_dvfs.utilisation;
}

int hvgr_pm_vsync_hit(void)
{
	return 0;
}

int hvgr_pm_get_cl_boost(void)
{
	struct hvgr_device *gdev = hvgr_get_device();

	if (gdev == NULL)
		return 0;
	return gdev->pm_dev.pm_dvfs.cl_boost;
}

int hvgr_pm_gpufreq_init(struct hvgr_device *gdev)
{
	int ret;

	ret = gpufreq_register_callback(GF_CB_BUSYTIME, hvgr_pm_get_busytime);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM, "Get_busytime callback register failed.");
		return ret;
	}

	ret = gpufreq_register_callback(GF_CB_VSYNC_HIT, hvgr_pm_vsync_hit);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM, "Get_vsync_hit callback register failed.");
		return ret;
	}

	ret = gpufreq_register_callback(GF_CB_CL_BOOST, hvgr_pm_get_cl_boost);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM, "Get_cl_boost callback register failed.");
		return ret;
	}
	return gpufreq_start();
}
#else
enum hrtimer_restart hvgr_pm_util_callback(struct hrtimer *timer)
{
	struct hvgr_device *gdev;
	struct hvgr_pm_dvfs *pm_dvfs = NULL;
	unsigned long freq = 150;

	gdev = container_of(timer, struct hvgr_device,
		pm_dev.pm_dvfs.util_timer);

	if (gdev == NULL)
		return 0;
	hvgr_pm_get_dvfs_action(gdev);
#ifdef CONFIG_HVGR_GPU_THROTTLE_DEVFREQ
	hvgr_gpu_devfreq_set_gpu_throttle(100, 200, &freq);
#endif
	hvgr_info(gdev, HVGR_PM, "Thro freq = %lu", freq);
	pm_dvfs = &gdev->pm_dev.pm_dvfs;
	hrtimer_start(&pm_dvfs->util_timer, pm_dvfs->delay_time, HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}
#endif /* CONFIG_GPUFREQ_INDEPDENT */

#ifdef CONFIG_HVGR_GPU_THROTTLE_DEVFREQ
void hvgr_pm_set_force_freq(struct hvgr_device *gdev, unsigned long freq, bool low)
{
	unsigned long flags;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	pm_dvfs->thro_freq_flag = true;

	hvgr_info(gdev, HVGR_PM, "freq = %lu, thro_freq = %lu.",
		freq, pm_dvfs->thro_freq);

	if (freq > pm_dvfs->thro_freq)
		pm_dvfs->thro_freq = freq;

	if (low)
		pm_dvfs->thro_freq = freq;

	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

unsigned long hvgr_pm_trans_thro_hint_to_freq(struct hvgr_device *gdev, uint32_t thro_hint)
{
	unsigned long flags;
	unsigned long thro_freq;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	if (thro_hint == HVGR_THRO_LOW)
		thro_freq = pm_dvfs->min_freq;
	else if (thro_hint == HVGR_THRO_HIGH)
		thro_freq = pm_dvfs->max_freq;
	else if (thro_hint == HVGR_THRO_MEDIUM)
		thro_freq = (pm_dvfs->min_freq + pm_dvfs->max_freq) >> 1;
	else
		thro_freq = 0;

	hvgr_info(gdev, HVGR_PM, "thro_hint = %u, thro_freq = %lu.", thro_hint, thro_freq);

	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
	return thro_freq;
}

void hvgr_gpu_devfreq_set_gpu_throttle(unsigned long freq_min,
	unsigned long freq_max, unsigned long *freq)
{
	unsigned long flags;
	int thro_enable;
	unsigned long final_freq;
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_pm_dvfs *pm_dvfs = NULL;

	if (gdev == NULL)
		return;

	pm_dvfs = &gdev->pm_dev.pm_dvfs;
	thro_enable = atomic_read(&pm_dvfs->thro_enable);

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);

	if (thro_enable == GPU_THROTTLE_DEVFREQ_DISABLE) {
		pm_dvfs->thro_freq = 0;
		pm_dvfs->pre_thro_freq = 0;
		pm_dvfs->thro_freq_flag = false;
		goto out;
	}

	if (pm_dvfs->thro_freq_flag == true) {
		final_freq = pm_dvfs->thro_freq;
		pm_dvfs->pre_thro_freq = pm_dvfs->thro_freq;
		pm_dvfs->thro_freq_flag = false;
	} else {
		final_freq = pm_dvfs->pre_thro_freq;
	}

	pm_dvfs->thro_freq = 0;

	/*
	 * If the frequency is locked, forcible frequency adjustment does not
	 * take effect.
	 */
	if (freq_min == freq_max)
		goto out;

	/* Update frequency threshold */
	if (freq_min != pm_dvfs->min_freq)
		pm_dvfs->min_freq = freq_min;

	if (freq_max != pm_dvfs->max_freq)
		pm_dvfs->max_freq = freq_max;

	if (final_freq == freq_min || final_freq == freq_max)
		*freq = final_freq;

	hvgr_info(gdev, HVGR_PM,
		"set gpu throttle, freq_min = %lu, freq_max = %lu. freq = %lu",
		freq_min, freq_max, *freq);
out:
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
}

ssize_t hvgr_pm_thro_read(void *data, char * const buf)
{
	uint32_t cur_freq;
	uint32_t pre_freq;
	ssize_t ret = 0;
	unsigned long flags;
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;
	int thro_status = atomic_read(&pm_dvfs->thro_enable);

	spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
	cur_freq = (uint32_t)pm_dvfs->thro_freq;
	pre_freq = (uint32_t)pm_dvfs->pre_thro_freq;
	spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Status: %s\n",
		thro_status == 1 ? "enable" : "disable");
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Cur thro freq: %d\n", cur_freq);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Pre thro freq: %d\n", pre_freq);
	return ret;
}

ssize_t hvgr_pm_thro_write(void *data, char * const buf,
	size_t count)
{
	int thro_enable;
	int cur_thro_enable;
	unsigned long flags;
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	if ((kstrtoint(buf, 0, &thro_enable) != 0) ||
		(thro_enable < 0))
		return -EINVAL;

	if (thro_enable != GPU_THROTTLE_DEVFREQ_DISABLE &&
		thro_enable != GPU_THROTTLE_DEVFREQ_ENABLE)
		return -EINVAL;

	cur_thro_enable = atomic_read(&pm_dvfs->thro_enable);
	if (thro_enable != cur_thro_enable) {
		atomic_set(&pm_dvfs->thro_enable, thro_enable);

		spin_lock_irqsave(&pm_dvfs->dvfs_lock, flags);
		pm_dvfs->thro_freq = 0;
		pm_dvfs->pre_thro_freq = 0;
		pm_dvfs->thro_freq_flag = false;
		spin_unlock_irqrestore(&pm_dvfs->dvfs_lock, flags);
	}
	return (ssize_t)count;
}

hvgr_debugfs_read_write_declare(fops_gpu_throttle, hvgr_pm_thro_read,
	hvgr_pm_thro_write);
#endif

ssize_t hvgr_pm_show_gpu_util(struct device *dev, struct device_attribute *attr, char *const buf)
{
	struct hvgr_device *gdev = dev_get_drvdata(dev);

	if (gdev == NULL)
		return -ENODEV;

	return (ssize_t)scnprintf(buf, PAGE_SIZE, "Gpu utilisation : %d\n",
		gdev->pm_dev.pm_dvfs.utilisation);
}

static DEVICE_ATTR(gpu_utilisation, S_IRUGO, hvgr_pm_show_gpu_util, NULL);

int hvgr_pm_metrics_init(struct hvgr_device *gdev)
{
	struct hvgr_pm_dvfs *pm_dvfs = &gdev->pm_dev.pm_dvfs;

	(void)memset_s(pm_dvfs, sizeof(*pm_dvfs), 0, sizeof(*pm_dvfs));
#ifdef CONFIG_HVGR_GPU_THROTTLE_DEVFREQ
	atomic_set(&pm_dvfs->thro_enable, GPU_THROTTLE_DEVFREQ_ENABLE);
#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_debugfs_register(gdev, "gpu_throttle", gdev->pm_dev.pm_dir, &fops_gpu_throttle);
#endif
#endif
	pm_dvfs->time_stamp = ktime_get();
	spin_lock_init(&pm_dvfs->dvfs_lock);

#ifdef CONFIG_GPUFREQ_INDEPDENT
	if (likely(gdev->subsys_ops->gpufreq_callback_register != NULL))
		gdev->subsys_ops->gpufreq_callback_register(gdev);
#else
	pm_dvfs->delay_time = ns_to_ktime(GPU_UTIL_SAMPLING_PERIOD_MS * 1000000);
	hrtimer_init(&pm_dvfs->util_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pm_dvfs->util_timer.function = hvgr_pm_util_callback;
	hvgr_pm_start_timer(gdev);
#endif
	if (sysfs_create_file(&gdev->dev->kobj, &dev_attr_gpu_utilisation.attr))
		hvgr_err(gdev, HVGR_PM, "Create gpu_utilisation failed.");

	return 0;
}

void hvgr_pm_metrics_term(struct hvgr_device *gdev)
{
	sysfs_remove_file(&gdev->dev->kobj, &dev_attr_gpu_utilisation.attr);
#ifdef CONFIG_GPUFREQ_INDEPDENT
	gpufreq_term();
#else
	hvgr_pm_cancel_timer(gdev);
#endif
}

void hvgr_pm_start_timer(struct hvgr_device *gdev)
{
	if (gdev->pm_dev.pm_dvfs.timer_running)
		return;

	hrtimer_start(&gdev->pm_dev.pm_dvfs.util_timer,
		gdev->pm_dev.pm_dvfs.delay_time, HRTIMER_MODE_REL);
	gdev->pm_dev.pm_dvfs.timer_running = true;
}

void hvgr_pm_cancel_timer(struct hvgr_device *gdev)
{
	if (!gdev->pm_dev.pm_dvfs.timer_running)
		return;

	(void)hrtimer_cancel(&gdev->pm_dev.pm_dvfs.util_timer);
	gdev->pm_dev.pm_dvfs.timer_running = false;
}
