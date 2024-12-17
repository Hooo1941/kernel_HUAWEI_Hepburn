/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
#include <linux/thermal.h>
#endif

#include "hvgr_pm_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_sch_api.h"

#include "hvgr_pm_defs.h"
#include "hvgr_pm_driver.h"
#include "hvgr_pm_core_mask.h"


static ssize_t hvgr_pm_set_available_cores(struct hvgr_device *gdev,
	uint32_t core_mask)
{
	uint32_t present_mask;
	int ret = 0;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	mutex_lock(&pm_pwr->core_mask_lock);
	mutex_lock(&pm_pwr->power_lock);
	present_mask = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	if ((core_mask & present_mask) != core_mask || core_mask == 0) {
		hvgr_err(gdev, HVGR_PM,
			"Set core mask: invalid new core mask.");
		ret = -EINVAL;
		goto out;
	}

	if (pm_pwr->core_mask_desire == core_mask)
		goto out;

	if (pm_pwr->cur_status == HVGR_PM_POWER_OFF) {
		pm_pwr->core_mask_cur =
			pm_pwr->core_mask_desire = core_mask;
		goto out;
	}

	mutex_unlock(&pm_pwr->power_lock);
	/* Suspend kick job to GPU */

	ret = hvgr_sch_suspend(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM,
			"Job suspend failed, core_mask is stopped.");
		mutex_unlock(&pm_pwr->core_mask_lock);
		return -EINVAL;
	}

	if (hvgr_pm_request_gpu(gdev, false) == false) {
		hvgr_err(gdev, HVGR_PM,
			"request gpu fail, core_mask is stopped.");
		hvgr_sch_resume(gdev);
		mutex_unlock(&pm_pwr->core_mask_lock);
		return -EINVAL;
	}

	/* Do core_mask */
	mutex_lock(&pm_pwr->power_lock);
	if (pm_pwr->cur_status == HVGR_PM_GPU_READY) {
		hvgr_datan_ct_disable_by_power(gdev, true);
		pm_pwr->core_mask_desire = core_mask;
		hvgr_pm_driver_set_core_mask(gdev);
		pm_pwr->core_mask_cur = pm_pwr->core_mask_desire;
		hvgr_datan_ct_enable_by_power(gdev);
	}
	mutex_unlock(&pm_pwr->power_lock);
	/* Resume kick job to GPU */
	hvgr_sch_resume(gdev);

	hvgr_pm_release_gpu(gdev);
	mutex_unlock(&pm_pwr->core_mask_lock);
	return 0;

out:
	mutex_unlock(&pm_pwr->power_lock);
	mutex_unlock(&pm_pwr->core_mask_lock);
	return ret;
}

static uint32_t hvgr_pm_core_num_to_bitmap(uint32_t core_num, uint32_t present_cores)
{
	uint32_t i;
	uint32_t num = 0;
	uint32_t pos = (uint32_t)fls(present_cores);

	for (i = 0; i < pos; i++) {
		num += ((present_cores >> i) & (uint32_t)1);
		if (num == core_num)
			return hvgr_get_bit_mask(present_cores, 0, i + (uint32_t)1);
	}

	return 0;
}

static uint32_t hvgr_pm_bitmap_to_core_num(uint32_t bitmap)
{
	uint32_t i;
	uint32_t core_num = 0;
	uint32_t pos = (uint32_t)fls(bitmap);

	for (i = 0; i < pos; i++)
		core_num += ((bitmap >> i) & (uint32_t)1);

	return core_num;
}

ssize_t hvgr_pm_show_core_mask(void *data, char * const buf)
{
	ssize_t ret = 0;
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"Current core mask : 0x%X\n",
		pm_pwr->core_mask_cur);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"Available core mask : 0x%X\n",
		pm_pwr->present_cores[HVGR_PM_CORE_GPC]);
	return ret;
}

ssize_t hvgr_pm_set_core_mask_inner(void *data, const char *buf, size_t count)
{
	uint32_t new_mask;
	int ret;
	struct hvgr_device *gdev = (struct hvgr_device *)data;

	ret = kstrtouint(buf, 0, &new_mask);
	if (ret)
		return -EINVAL;
#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
	if (atomic_read(&(gdev->pm_dev.pm_pwr.hang_core_proc_flag)) != 0) {
		hvgr_info(gdev, HVGR_PM, "Now gpu hang set core mask, can not set core mask 0x%x",
			new_mask);
		return -EINVAL;
	}
#endif
	if (hvgr_pm_set_available_cores(gdev, new_mask))
		return -EINVAL;
	return (ssize_t)count;
}

ssize_t hvgr_pm_gpu_hang_cores_control(struct hvgr_device *gdev,
	uint32_t core_mask)
{
	return hvgr_pm_set_available_cores(gdev, core_mask);
}

void hvgr_pm_gpu_thermal_cores_control(u64 core_num)
{
	struct hvgr_device *gdev;
	uint32_t bitmap;
	uint32_t present_cores;

	gdev = hvgr_get_device();
	if (gdev == NULL)
		return;

	present_cores = gdev->pm_dev.pm_pwr.present_cores[HVGR_PM_CORE_GPC];
	if ((uint32_t)core_num >= hvgr_pm_bitmap_to_core_num(present_cores))
		bitmap = present_cores;
	else
		bitmap = hvgr_pm_core_num_to_bitmap((uint32_t)core_num, present_cores);

	(void)hvgr_pm_set_available_cores(gdev, bitmap);
}

static ssize_t show_core_mask(struct device *dev,
	struct device_attribute *attr, char * const buf)
{
	struct hvgr_device *gdev = dev_get_drvdata(dev);

	if (gdev == NULL)
		return -ENODEV;
	return hvgr_pm_show_core_mask(gdev, buf);
}

static ssize_t set_core_mask(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hvgr_device *gdev = dev_get_drvdata(dev);

	if (gdev == NULL)
		return -ENODEV;
	return hvgr_pm_set_core_mask(gdev, buf, count);
}

static DEVICE_ATTR(core_mask, S_IRUGO | S_IWUSR, show_core_mask, set_core_mask);

void hvgr_core_mask_init(struct hvgr_device *gdev)
{
#ifdef CONFIG_THERMAL_GPU_HOTPLUG
	gpu_hotplug_callback_register(hvgr_pm_gpu_thermal_cores_control);
#endif
	if (sysfs_create_file(&gdev->dev->kobj, &dev_attr_core_mask.attr))
		hvgr_err(gdev, HVGR_PM, "Create core_mask failed.");
}

void hvgr_core_mask_term(struct hvgr_device *gdev)
{
	sysfs_remove_file(&gdev->dev->kobj, &dev_attr_core_mask.attr);
}
