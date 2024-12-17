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
#include <linux/platform_device.h>
#include <linux/wait.h>

#include "hvgr_sch_api.h"

#include "hvgr_log_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_platform_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_pm_dvfs.h"
#include "hvgr_pm_power.h"

int hvgr_pm_suspend(struct hvgr_device *gdev)
{
	int ret;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	atomic_set(&pm_pwr->sr_state, HVGR_PM_SUSPENDING);
	/*
	 * Suspend job scheduler and associated components, so that
	 * it releases all the PM active count references
	 */
	ret = hvgr_sch_suspend(gdev);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
			gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));
		hvgr_sch_stop(gdev);
		hvgr_err(gdev, HVGR_PM, "cq suspend timeout.");
	}

	hvgr_pm_power_off_sync(gdev);
	wait_event(pm_pwr->power_off_done_wait, pm_pwr->cur_status == HVGR_PM_POWER_OFF);
	atomic_set(&pm_pwr->sr_state, HVGR_PM_SUSPENDED);
#ifndef CONFIG_GPUFREQ_INDEPDENT
	hvgr_pm_cancel_timer(gdev);
#endif
	return 0;
}

void hvgr_pm_resume(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	atomic_set(&pm_pwr->sr_state, HVGR_PM_RESUMEING);
	/*
	 * The AS needs to be refreshed after being resumed.
	 * AS will be refreshed during the GPU power-on process.
	 */
	(void)hvgr_pm_request_gpu(gdev, false);
	hvgr_sch_resume(gdev);
	hvgr_pm_release_gpu(gdev);

#ifndef CONFIG_GPUFREQ_INDEPDENT
	hvgr_pm_start_timer(gdev);
#endif
	atomic_set(&pm_pwr->sr_state, HVGR_PM_ACTIVE);
}

bool hvgr_pm_is_suspending(const struct hvgr_device *gdev)
{
	int sr_state;
	const struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	sr_state = atomic_read(&pm_pwr->sr_state);
	if (sr_state == HVGR_PM_SUSPENDED || sr_state == HVGR_PM_SUSPENDING)
		return true;

	return false;
}
