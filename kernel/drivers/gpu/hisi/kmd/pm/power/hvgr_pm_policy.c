/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * Description: This file is used when the vulkan jit enable.
 * Author:
 * Create: 2021-02-03
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_pm_policy.h"

#include "hvgr_log_api.h"
#include "hvgr_dm_api.h"

#include "hvgr_pm_api.h"
#include "hvgr_pm_power.h"

bool hvgr_pm_is_need_power_on(struct hvgr_device *gdev, bool gpu_reset)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (hvgr_pm_is_suspending(gdev)) {
		if (gpu_reset)
			return true;
		else
			return false;
	}

	if (atomic_read(&pm_pwr->user_ref) == 0 &&
		atomic_read(&pm_pwr->cur_policy) == HVGR_PM_JOB_DEMAND)
		return false;
	return true;
}

bool hvgr_pm_is_need_power_off(struct hvgr_device *gdev)
{
	int sr_state;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (atomic_read(&pm_pwr->user_ref) != 0)
		return false;

	sr_state = atomic_read(&pm_pwr->sr_state);
	if (atomic_read(&pm_pwr->cur_policy) == HVGR_PM_ALWAYS_ON &&
		(sr_state == HVGR_PM_RESUMEING || sr_state == HVGR_PM_ACTIVE))
		return false;
	return true;
}

ssize_t hvgr_pm_set_policy(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	uint32_t new_policy;
	struct hvgr_device *gdev = dev_get_drvdata(dev);
	struct hvgr_pm_pwr *pm_pwr = NULL;
	const char *policy_name[HVGR_PM_POLICY_MAX] = {
		"always_on",
		"job_demand",
	};

	if (gdev == NULL)
		return -ENODEV;

	pm_pwr = &gdev->pm_dev.pm_pwr;

	for (new_policy = 0; new_policy < HVGR_PM_POLICY_MAX; new_policy++) {
		if (sysfs_streq(policy_name[new_policy], buf))
			break;
	}

	if (unlikely(new_policy == HVGR_PM_POLICY_MAX)) {
		hvgr_err(gdev, HVGR_PM, "power_policy: policy not found\n");
		return -EINVAL;
	}

	if ((uint32_t)atomic_read(&pm_pwr->cur_policy) == new_policy)
		return (ssize_t)count;

	atomic_set(&pm_pwr->cur_policy, (int)new_policy);
	if (new_policy == HVGR_PM_ALWAYS_ON)
		(void)hvgr_pm_power_on_sync(gdev, false);
	else
#ifdef HVGR_GPU_POWER_ASYNC
		hvgr_pm_power_off_async(gdev);
#else
		hvgr_pm_power_off_sync(gdev);
#endif
	return (ssize_t)count;
}

ssize_t hvgr_pm_show_policy(struct device *dev, struct device_attribute *attr, char *const buf)
{
	uint32_t id;
	ssize_t ret = 0;
	struct hvgr_pm_pwr *pm_pwr = NULL;
	struct hvgr_device *gdev = dev_get_drvdata(dev);
	const char *policy_name[HVGR_PM_POLICY_MAX] = {
		"always_on",
		"job_demand",
	};

	if (gdev == NULL)
		return -ENODEV;

	pm_pwr = &gdev->pm_dev.pm_pwr;

	for (id = 0; id < HVGR_PM_POLICY_MAX; id++) {
		if ((uint32_t)atomic_read(&pm_pwr->cur_policy) == id)
			ret += scnprintf(buf + ret,
				PAGE_SIZE - ret, "[%s] ", policy_name[id]);
		else
			ret += scnprintf(buf + ret,
				PAGE_SIZE - ret, "%s ", policy_name[id]);
	}

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "\n");
	return ret;
}

static DEVICE_ATTR(power_policy, S_IRUGO | S_IWUSR, hvgr_pm_show_policy,
	hvgr_pm_set_policy);

#ifdef HVGR_GPU_POWER_ASYNC
#ifdef CONFIG_DFX_DEBUG_FS
ssize_t hvgr_pm_set_delay_poweroff_time(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	uint32_t delay_time;
	int ret;
	struct hvgr_pm_pwr *pm_pwr = NULL;
	struct hvgr_device *gdev = dev_get_drvdata(dev);

	if (gdev == NULL)
		return -ENODEV;

	pm_pwr = &gdev->pm_dev.pm_pwr;
	ret = kstrtouint(buf, 0, &delay_time);
	if (ret)
		return -EINVAL;
	pm_pwr->delay_time = ns_to_ktime(delay_time);
	return (ssize_t)count;
}

ssize_t hvgr_pm_show_delay_poweroff_time(struct device *dev,
	struct device_attribute *attr, char *const buf)
{
	uint32_t ret = 0;
	struct hvgr_pm_pwr *pm_pwr = NULL;
	struct hvgr_device *gdev = dev_get_drvdata(dev);

	if (gdev == NULL)
		return -ENODEV;

	pm_pwr = &gdev->pm_dev.pm_pwr;
	ret += (uint32_t)scnprintf(buf + ret, PAGE_SIZE - ret,
		"Power off delay time: %ld\n", pm_pwr->delay_time);
	return ret;
}

static DEVICE_ATTR(power_off, S_IRUGO | S_IWUSR, hvgr_pm_show_delay_poweroff_time,
	hvgr_pm_set_delay_poweroff_time);
#endif
#endif

void hvgr_pm_policy_init(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (hvgr_is_fpga(gdev) || gdev->subsys_ops->pm_is_always_on())
		atomic_set(&pm_pwr->cur_policy, HVGR_PM_ALWAYS_ON);
	else
		atomic_set(&pm_pwr->cur_policy, HVGR_PM_JOB_DEMAND);

	if (sysfs_create_file(&gdev->dev->kobj, &dev_attr_power_policy.attr))
		hvgr_err(gdev, HVGR_PM, "Create power_policy failed.");
#ifdef HVGR_GPU_POWER_ASYNC
#ifdef CONFIG_DFX_DEBUG_FS
	if (sysfs_create_file(&gdev->dev->kobj, &dev_attr_power_off.attr))
		hvgr_err(gdev, HVGR_PM, "Create power_off delay time failed.");
#endif
#endif
}

void hvgr_pm_policy_term(struct hvgr_device *gdev)
{
	sysfs_remove_file(&gdev->dev->kobj, &dev_attr_power_policy.attr);
#ifdef HVGR_GPU_POWER_ASYNC
#ifdef CONFIG_DFX_DEBUG_FS
	sysfs_remove_file(&gdev->dev->kobj, &dev_attr_power_off.attr);
#endif
#endif
}

bool hvgr_is_always_on(struct hvgr_device *gdev) {
	if (atomic_read(&gdev->pm_dev.pm_pwr.cur_policy) == HVGR_PM_ALWAYS_ON)
		return true;
	return false;
}
