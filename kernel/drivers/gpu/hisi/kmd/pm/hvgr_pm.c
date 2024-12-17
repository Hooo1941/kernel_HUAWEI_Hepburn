/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pm.h>

#include "hvgr_log_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_pm_policy.h"
#include "hvgr_pm_power.h"
#include "hvgr_pm_gpu_reset.h"
#include "hvgr_pm_core_mask.h"
#include "hvgr_pm_driver.h"
#include "hvgr_pm_dvfs.h"
#include "hvgr_pm_dfx.h"

ssize_t hvgr_pm_set_core_mask(struct hvgr_device *gdev,
	const char *buf, size_t count)
{
	return hvgr_pm_set_core_mask_inner((void *)gdev, buf, count);
}

bool hvgr_pm_request_gpu(struct hvgr_device *gdev, bool gpu_reset)
{
	hvgr_pm_user_ref_add(gdev);
	return hvgr_pm_power_on_sync(gdev, gpu_reset);
}

void hvgr_pm_release_gpu(struct hvgr_device *gdev)
{
	hvgr_pm_user_ref_dec(gdev);
#ifdef HVGR_GPU_POWER_ASYNC
	hvgr_pm_power_off_async(gdev);
#else
	hvgr_pm_power_off_sync(gdev);
#endif
}

uint32_t hvgr_pm_get_availiable_cores(const struct hvgr_device *gdev)
{
	return gdev->pm_dev.pm_pwr.core_mask_cur;
}

static int hvgr_pm_soft_init(struct hvgr_device *gdev)
{
	int ret;

	gdev->pm_dev.pm_dir = hvgr_debugfs_create_dir("pm",
		gdev->root_debugfs_dir);
	gdev->pm_dev.gdev = gdev;
	ret = hvgr_pm_power_soft_init(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM, "pm power soft init failed");
		return ret;
	}
	ret = hvgr_pm_gpu_reset_soft_init(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_PM, "pm gpu reset init failed");
		return ret;
	}
	hvgr_core_mask_init(gdev);
	(void)hvgr_pm_metrics_init(gdev);
	hvgr_pm_dfx_init(gdev);
	return 0;
}

static void hvgr_pm_soft_term(struct hvgr_device *gdev)
{
	hvgr_pm_power_soft_term(gdev);

	if (!IS_ERR_OR_NULL(gdev->pm_dev.pm_reset.reset_workq))
		destroy_workqueue(gdev->pm_dev.pm_reset.reset_workq);

	hvgr_pm_metrics_term(gdev);
#ifdef CONFIG_DFX_DEBUG_FS
	if (!IS_ERR_OR_NULL(gdev->pm_dev.pm_dir))
		debugfs_remove_recursive(gdev->pm_dev.pm_dir);
#endif
	hvgr_core_mask_term(gdev);
}

int hvgr_pm_gpu_access_enable(struct hvgr_device * const gdev)
{
	if (hvgr_pm_soft_init(gdev) != 0) {
		hvgr_err(gdev, HVGR_PM, "hvgr_pm_soft_init failed");
		return -1;
	}

	if (hvgr_pm_driver_hw_access_enable(gdev) != 0) {
		hvgr_pm_soft_term(gdev);
		hvgr_err(gdev, HVGR_PM, "hvgr_pm_driver_hw_access_enable failed");
		return -1;
	}
	return 0;
}

int hvgr_pm_gpu_access_disable(struct hvgr_device * const gdev)
{
	hvgr_pm_driver_hw_access_disable(gdev);
	return 0;
}

void hvgr_pm_gpu_access_term(struct hvgr_device * const gdev)
{
	hvgr_pm_soft_term(gdev);
}

#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
static uint32_t hvgr_pm_select_one_core_mask(struct hvgr_pm_pwr *pm_pwr, uint32_t cores)
{
	uint32_t core_pos;

	if (atomic_read(&(pm_pwr->hang_core_proc_flag)) != 0 && (pm_pwr->core_mask_cur != 0))
		cores &= ~(pm_pwr->core_mask_cur | (pm_pwr->core_mask_cur - 1));

	if (cores == 0)
		return 0;

	core_pos = (uint32_t)ffs((int)cores) - 1;

	return (1U << core_pos);
}

void hvgr_pm_gpu_hang_dfx_proc(struct work_struct *work)
{
	uint32_t available_mask;
	uint32_t core_mask;
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_pm_pwr *pm_pwr = NULL;

	if ((work == NULL) || (gdev == NULL))
		return;

	pm_pwr = container_of(work, struct hvgr_pm_pwr, sh_hang_work);

	if (atomic_inc_return(&(pm_pwr->gpu_hang_cnt)) < HVGR_GPU_HANG_THRESHOLD)
		return;

	available_mask = pm_pwr->present_cores[HVGR_PM_CORE_GPC] &
		(~(uint32_t)(atomic_read(&pm_pwr->core_mask_info)));
	if (available_mask == 0) {
		hvgr_debug(gdev, HVGR_DM, "Not have available cores[0x%x]",
			(atomic_read(&pm_pwr->core_mask_info)));
		return;
	}

	if (atomic_read(&(pm_pwr->hang_core_proc_flag)) != 0)
		hvgr_updata_core_mask_info(gdev, pm_pwr->core_mask_cur);

	core_mask = hvgr_pm_select_one_core_mask(pm_pwr, available_mask);
	if (core_mask == 0 ) {
		hvgr_debug(gdev, HVGR_DM, "Not have available cores, can not set");
		atomic_set(&pm_pwr->hang_core_proc_flag, 0);
		(void)hvgr_pm_gpu_hang_cores_control(gdev,
			pm_pwr->present_cores[HVGR_PM_CORE_GPC]);
		return;
	}

	if (hvgr_pm_gpu_hang_cores_control(gdev, core_mask) != 0) {
		hvgr_err(gdev, HVGR_DM, "Set core mask failed");
		return;
	}

	atomic_set(&(pm_pwr->hang_core_proc_flag), 1);
	atomic_set(&(pm_pwr->gpu_hang_cnt), 0);
}
#endif

int hvgr_pm_init(struct hvgr_device * const gdev)
{
#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	atomic_set(&pm_pwr->gpu_hang_cnt, 0);
	atomic_set(&pm_pwr->hang_core_proc_flag, 0);
	atomic_set(&pm_pwr->core_mask_info, 0);
	atomic_set(&pm_pwr->core_mask_rd_flag, 0);

	pm_pwr->sh_hang_wq = alloc_workqueue("sh_hang_wq", WQ_HIGHPRI | WQ_UNBOUND, 1);
	if (pm_pwr->sh_hang_wq == NULL) {
		hvgr_err(gdev, HVGR_DM, "alloc sh_hang_wq fail");
		return -1;
	}
	INIT_WORK(&pm_pwr->sh_hang_work, hvgr_pm_gpu_hang_dfx_proc);
#endif

	hvgr_pm_driver_hw_init(gdev);
	hvgr_pm_policy_init(gdev);
	return 0;
}

void hvgr_pm_term(struct hvgr_device *gdev)
{
	/* Set power policy to job demand, so gpu can be power off. */
	atomic_set(&gdev->pm_dev.pm_pwr.cur_policy, HVGR_PM_JOB_DEMAND);
	/* Power off gpu when pm term. */
	hvgr_pm_power_off_sync(gdev);
#ifdef CONFIG_HVGR_GPU_RT_PM
	hvgr_pm_driver_runtime_term(gdev);
#endif
	hvgr_pm_policy_term(gdev);
#if defined(CONFIG_HVGR_DFX_SH) && defined(CONFIG_HVGR_VIRTUAL_GUEST)
	if (gdev->pm_dev.pm_pwr.sh_hang_wq != NULL) {
		destroy_workqueue(gdev->pm_dev.pm_pwr.sh_hang_wq);
		gdev->pm_dev.pm_pwr.sh_hang_wq = NULL;
	}
#endif
}

void hvgr_pm_request_cycle_counter(struct hvgr_device *gdev)
{
	hvgr_pm_driver_request_cycle_counter(gdev);
}

void hvgr_pm_release_cycle_counter(struct hvgr_device *gdev)
{
	hvgr_pm_driver_release_cycle_counter(gdev);
}

#ifdef CONFIG_HVGR_GPU_RT_PM
int hvgr_pm_runtime_suspend(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_runtime_suspend(gdev);
}

int hvgr_pm_runtime_resume(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_runtime_resume(gdev);
}

int hvgr_pm_runtime_idle(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_runtime_idle(gdev);
}

#endif
