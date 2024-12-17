/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_pm_power.h"

#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#ifdef HVGR_GPU_POWER_ASYNC
#include <linux/hrtimer.h>
#endif
#include "hvgr_pm_defs.h"
#include "hvgr_log_api.h"
#include "hvgr_datan_api.h"

#include "hvgr_pm_dfx.h"
#include "hvgr_pm_state_machine.h"
#include "hvgr_pm_policy.h"
#include "hvgr_pm_driver.h"


bool hvgr_pm_power_on_sync(struct hvgr_device *gdev, bool gpu_reset)
{
	bool ret = true;
	uint64_t start_time;
	uint64_t dfx_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_dump_stack();
	start_time = hvgr_start_timing();
	mutex_lock(&pm_pwr->power_lock);
	hvgr_info(gdev, HVGR_PM, "Power on sync, count = %d", atomic_read(&pm_pwr->user_ref));
#ifdef HVGR_GPU_POWER_ASYNC
	if (pm_pwr->power_off_processing) {
		hvgr_info(gdev, HVGR_PM, "Power off is processing when powered on");
		pm_pwr->power_off_processing = false;
		(void)hrtimer_cancel(&pm_pwr->poweroff_timer);
	}
#endif
	if (pm_pwr->cur_status == HVGR_PM_GPU_READY)
		goto out;

	wake_up(&pm_pwr->power_off_done_wait);
	if (hvgr_pm_is_need_power_on(gdev, gpu_reset) == false) {
		ret = false;
		goto out;
	}

	dfx_time = hvgr_pm_dfx_get_curtime(gdev);
	(void)hvgr_pm_update_pwr_state(gdev, HVGR_PM_GPU_READY);
	hvgr_datan_ct_enable_by_power(gdev);
	hvgr_info(gdev, HVGR_PM, "Power on and enable hwcnt.");
	mutex_unlock(&pm_pwr->power_lock);

	hvgr_pm_dfx_gpu_on_statistics(gdev, dfx_time);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Power on");
	return ret;

out:
	mutex_unlock(&pm_pwr->power_lock);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Power on");
	return ret;
}

void hvgr_pm_power_off_sync(struct hvgr_device *gdev)
{
	uint64_t start_time;
	uint64_t dfx_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	mutex_lock(&pm_pwr->power_lock);
	hvgr_info(gdev, HVGR_PM, "Power off sync, count = %d", atomic_read(&pm_pwr->user_ref));
	start_time = hvgr_start_timing();
	/* If the power-off conditions are not met, enable the hwcnt again. */
	if (hvgr_pm_is_need_power_off(gdev) == false) {
		hvgr_info(gdev, HVGR_PM, "%s Power off abort and re-enable hwcnt", __func__);
		goto exit;
	}

	if (pm_pwr->cur_status == HVGR_PM_POWER_OFF)
		goto exit;

	dfx_time = hvgr_pm_dfx_get_curtime(gdev);
	hvgr_info(gdev, HVGR_PM, "Dump gpu run satus before power off.");
	hvgr_pm_dump_reg(gdev, HVGR_LOG_INFO);
	hvgr_datan_ct_disable_by_power(gdev, true);
	(void)hvgr_pm_update_pwr_state(gdev, HVGR_PM_POWER_OFF);
	hvgr_pm_dfx_gpu_off_statistics(gdev, dfx_time);

#ifdef HVGR_GPU_POWER_ASYNC
	pm_pwr->power_off_processing = false;
#endif
	mutex_unlock(&pm_pwr->power_lock);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Power off");
	return;

exit:
#ifdef HVGR_GPU_POWER_ASYNC
	pm_pwr->power_off_processing = false;
#endif
	mutex_unlock(&pm_pwr->power_lock);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Power off");
}

#ifdef HVGR_GPU_POWER_ASYNC
static void hvgr_pm_power_off_wq(struct work_struct *data)
{
	struct hvgr_device *gdev = NULL;

	gdev = container_of(data, struct hvgr_device, pm_dev.pm_pwr.poweroff_work);
	if (gdev == NULL)
		return;
	hvgr_pm_power_off_sync(gdev);
}

static enum hrtimer_restart hvgr_pm_power_off_callback(
	struct hrtimer *timer)
{
	struct hvgr_device *gdev = NULL;
	struct hvgr_pm_pwr *pm_pwr = NULL;

	gdev = container_of(timer, struct hvgr_device,
		pm_dev.pm_pwr.poweroff_timer);
	if (gdev == NULL)
		return HRTIMER_NORESTART;

	pm_pwr = &gdev->pm_dev.pm_pwr;
	hvgr_info(gdev, HVGR_PM,
		"Power off async timer callback, count = %d",
		atomic_read(&pm_pwr->user_ref));
	(void)queue_work(pm_pwr->poweroff_wq, &pm_pwr->poweroff_work);
	return HRTIMER_NORESTART;
}

void hvgr_pm_power_off_async(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_info(gdev, HVGR_PM, "Power off async, count = %d",
		atomic_read(&pm_pwr->user_ref));
	hvgr_dump_stack();

	if (atomic_read(&pm_pwr->sr_state) != HVGR_PM_ACTIVE) {
		hvgr_pm_power_off_sync(gdev);
		return;
	}

	mutex_lock(&pm_pwr->power_lock);
	if (pm_pwr->cur_status == HVGR_PM_POWER_OFF)
		goto exit;

	if (hvgr_pm_is_need_power_off(gdev) == false)
		goto exit;

	if (pm_pwr->power_off_processing)
		goto exit;
	else
		pm_pwr->power_off_processing = true;

	hvgr_datan_ct_disable_by_power(gdev, true);
	(void)hvgr_pm_update_pwr_state(gdev, HVGR_PM_GPC_POWERING_OFF);
	hrtimer_start(&pm_pwr->poweroff_timer, pm_pwr->delay_time, HRTIMER_MODE_REL);
exit:
	mutex_unlock(&pm_pwr->power_lock);
	return;
}

int hvgr_pm_async_power_off_init(struct hvgr_device *gdev)
{
	struct workqueue_struct *wq = NULL;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	wq = alloc_workqueue("hvgr_pm_poweroff", WQ_HIGHPRI | WQ_UNBOUND, 1);
	if (wq == NULL)
		return -ENOMEM;
	pm_pwr->power_off_processing = false;
	pm_pwr->poweroff_wq = wq;
	INIT_WORK(&pm_pwr->poweroff_work, hvgr_pm_power_off_wq);
	pm_pwr->delay_time = ns_to_ktime(POWER_OFF_DEALY_TIME);

	hrtimer_init(&pm_pwr->poweroff_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pm_pwr->poweroff_timer.function = hvgr_pm_power_off_callback;
	return 0;
}
#endif

int hvgr_pm_get_gpu_status(const struct hvgr_device *gdev)
{
	return gdev->pm_dev.pm_pwr.cur_status;
}

int hvgr_pm_power_soft_init(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	mutex_init(&pm_pwr->power_lock);
	mutex_init(&pm_pwr->core_mask_lock);
	atomic_set(&pm_pwr->user_ref, 0);
	atomic_set(&pm_pwr->sr_state, HVGR_PM_ACTIVE);
	pm_pwr->powered_off_directly = false;
	init_waitqueue_head(&pm_pwr->power_off_done_wait);
	pm_pwr->cur_status = HVGR_PM_POWER_OFF;
#ifdef HVGR_GPU_POWER_ASYNC
	if (hvgr_pm_async_power_off_init(gdev))
		return -ENOMEM;
#endif
	return hvgr_pm_driver_soft_init(gdev);
}

void hvgr_pm_power_soft_term(struct hvgr_device *gdev)
{
	/* Set power policy to job demand, so gpu can be power off. */
	atomic_set(&gdev->pm_dev.pm_pwr.cur_policy, HVGR_PM_JOB_DEMAND);
	/* Power off gpu when pm term. */
	hvgr_pm_power_off_sync(gdev);
#ifdef HVGR_GPU_POWER_ASYNC
	destroy_workqueue(gdev->pm_dev.pm_pwr.poweroff_wq);
#endif
	hvgr_pm_driver_soft_term(gdev);
}
