/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_pm_dfx.h"
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <securec.h>

#include "hvgr_debugfs_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_driver.h"

ssize_t hvgr_pm_set_statistics(void *data, const char *buf, size_t count)
{
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;
	bool is_enable = false;

	if (sysfs_streq("enable", buf)) {
		pm_dfx->is_enable = true;
		return (ssize_t)count;
	}

	if (sysfs_streq("disable", buf)) {
		pm_dfx->is_enable = false;
		return (ssize_t)count;
	}

	if (sysfs_streq("clear", buf)) {
		is_enable = pm_dfx->is_enable;
		(void)memset_s(&gdev->pm_dev.pm_dfx, sizeof(*pm_dfx), 0, sizeof(*pm_dfx));
		pm_dfx->is_enable = is_enable;

		return (ssize_t)count;
	}

	return -EINVAL;
}

ssize_t hvgr_pm_show_statistics(void *data, char * const buf)
{
	ssize_t ret = 0;
	uint64_t avage_buck_off_time = 0;
	uint64_t avage_buck_on_time = 0;
	uint64_t avage_core_off_time = 0;
	uint64_t avage_core_on_time = 0;
	uint64_t avage_gpu_off_time = 0;
	uint64_t avage_gpu_on_time = 0;
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (pm_dfx->pm_buck_off_times != 0)
		avage_buck_off_time =
			pm_dfx->pm_total_buck_off_time / pm_dfx->pm_buck_off_times;

	if (pm_dfx->pm_buck_on_times != 0)
		avage_buck_on_time =
			pm_dfx->pm_total_buck_on_time / pm_dfx->pm_buck_on_times;

	if (pm_dfx->pm_core_off_times != 0)
		avage_core_off_time =
			pm_dfx->pm_total_core_off_time / pm_dfx->pm_core_off_times;

	if (pm_dfx->pm_core_on_times != 0)
		avage_core_on_time =
			pm_dfx->pm_total_core_on_time / pm_dfx->pm_core_on_times;

	if (pm_dfx->pm_gpu_off_times != 0)
		avage_gpu_off_time =
			pm_dfx->pm_total_gpu_off_time / pm_dfx->pm_gpu_off_times;

	if (pm_dfx->pm_gpu_on_times != 0)
		avage_gpu_on_time =
			pm_dfx->pm_total_gpu_on_time / pm_dfx->pm_gpu_on_times;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"PM static status: %s\n", pm_dfx->is_enable ? "enable" : "disable");

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"\t on_times  off_times  avage_on_time(us)  avage_off_time(us)  max_on_time(us)%s",
		"  max_off_time(us)  min_on_time(us)  min_off_time(us)\n");

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"buck:\t %8d   %8d           %8lu            %8lu         %8lu          %8lu         %8lu          %8lu\n",
		pm_dfx->pm_buck_on_times, pm_dfx->pm_buck_off_times,
		avage_buck_on_time / HVGR_CONVERT_TO_MILLISECOND,
		avage_buck_off_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_buck_on_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_buck_off_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_buck_on_min_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_buck_off_min_time / HVGR_CONVERT_TO_MILLISECOND);

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"core:\t %8d   %8d           %8lu            %8lu         %8lu          %8lu         %8lu          %8lu\n",
		pm_dfx->pm_core_on_times, pm_dfx->pm_core_off_times,
		avage_core_on_time / HVGR_CONVERT_TO_MILLISECOND,
		avage_core_off_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_core_on_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_core_off_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_core_on_min_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_core_off_min_time / HVGR_CONVERT_TO_MILLISECOND);

	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"gpu:\t %8d   %8d           %8lu            %8lu         %8lu          %8lu         %8lu          %8lu\n",
		pm_dfx->pm_gpu_on_times, pm_dfx->pm_gpu_off_times,
		avage_gpu_on_time / HVGR_CONVERT_TO_MILLISECOND,
		avage_gpu_off_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_gpu_on_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_gpu_off_max_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_gpu_on_min_time / HVGR_CONVERT_TO_MILLISECOND,
		pm_dfx->pm_gpu_off_min_time / HVGR_CONVERT_TO_MILLISECOND);

	return ret;
}

void hvgr_pm_dfx_gpu_on_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;

	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_gpu_on_times++;
	pm_dfx->pm_total_gpu_on_time += diff_time;

	if (pm_dfx->pm_gpu_on_max_time < diff_time)
		pm_dfx->pm_gpu_on_max_time = diff_time;

	if (pm_dfx->pm_gpu_on_min_time == 0 ||
		pm_dfx->pm_gpu_on_min_time > diff_time)
		pm_dfx->pm_gpu_on_min_time = diff_time;
}

void hvgr_pm_dfx_gpu_off_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_gpu_off_times++;
	pm_dfx->pm_total_gpu_off_time += diff_time;

	if (pm_dfx->pm_gpu_off_max_time < diff_time)
		pm_dfx->pm_gpu_off_max_time = diff_time;

	if (pm_dfx->pm_gpu_off_min_time == 0 ||
		pm_dfx->pm_gpu_off_min_time > diff_time)
		pm_dfx->pm_gpu_off_min_time = diff_time;
}

void hvgr_pm_dfx_buck_on_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_buck_on_times++;
	pm_dfx->pm_total_buck_on_time += diff_time;

	if (pm_dfx->pm_buck_on_max_time < diff_time)
		pm_dfx->pm_buck_on_max_time = diff_time;

	if (pm_dfx->pm_buck_on_min_time == 0 ||
		pm_dfx->pm_buck_on_min_time > diff_time)
		pm_dfx->pm_buck_on_min_time = diff_time;
}

void hvgr_pm_dfx_buck_off_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_buck_off_times++;
	pm_dfx->pm_total_buck_off_time += diff_time;

	if (pm_dfx->pm_buck_off_max_time < diff_time)
		pm_dfx->pm_buck_off_max_time = diff_time;

	if (pm_dfx->pm_buck_off_min_time == 0 ||
		pm_dfx->pm_buck_off_min_time > diff_time)
		pm_dfx->pm_buck_off_min_time = diff_time;
}

void hvgr_pm_dfx_core_off_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_core_off_times++;
	pm_dfx->pm_total_core_off_time += diff_time;

	if (pm_dfx->pm_core_off_max_time < diff_time)
		pm_dfx->pm_core_off_max_time = diff_time;

	if (pm_dfx->pm_core_off_min_time == 0 ||
		pm_dfx->pm_core_off_min_time > diff_time)
		pm_dfx->pm_core_off_min_time = diff_time;
}

void hvgr_pm_dfx_core_on_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_core_on_times++;
	pm_dfx->pm_total_core_on_time += diff_time;

	if (pm_dfx->pm_core_on_max_time < diff_time)
		pm_dfx->pm_core_on_max_time = diff_time;

	if (pm_dfx->pm_core_on_min_time == 0 ||
		pm_dfx->pm_core_on_min_time > diff_time)
		pm_dfx->pm_core_on_min_time = diff_time;
}

void hvgr_pm_dfx_gpu_reset_statistics(struct hvgr_device *gdev, uint64_t time)
{
	uint64_t diff_time;
	struct hvgr_pm_dfx *pm_dfx = &gdev->pm_dev.pm_dfx;

	if (!pm_dfx->is_enable)
		return;
	diff_time = (uint64_t)ktime_to_ns(ktime_get()) - time;
	pm_dfx->pm_gpu_reset_times++;
	pm_dfx->pm_total_gpu_reset_time += diff_time;

	if (pm_dfx->pm_gpu_reset_max_time < diff_time)
		pm_dfx->pm_gpu_reset_max_time = diff_time;

	if (pm_dfx->pm_gpu_reset_min_time == 0 ||
		pm_dfx->pm_gpu_reset_min_time > diff_time)
		pm_dfx->pm_gpu_reset_min_time = diff_time;
}

uint64_t hvgr_pm_dfx_get_curtime(struct hvgr_device *gdev)
{
	if (gdev->pm_dev.pm_dfx.is_enable)
		return (uint64_t)ktime_to_ns(ktime_get());
	return 0;
}

void hvgr_pm_dump_reg(struct hvgr_device *gdev, enum HVGR_LOG_LEVEL level)
{
	if (!hvgr_log_is_enable(level, HVGR_PM))
		return;
	hvgr_dump_registers(gdev);
}

hvgr_debugfs_read_write_declare(fops_pm_statistics, hvgr_pm_show_statistics,
	hvgr_pm_set_statistics);

void hvgr_pm_dfx_init(struct hvgr_device *gdev)
{
	if (gdev->pm_dev.pm_dir == NULL)
		return;

#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_debugfs_register(gdev, "pm_statistics", gdev->pm_dev.pm_dir, &fops_pm_statistics);
#endif
}
