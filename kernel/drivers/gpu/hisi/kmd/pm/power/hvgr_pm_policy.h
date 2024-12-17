/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_POLICY_H
#define HVGR_PM_POLICY_H

#include <linux/atomic.h>
#include <linux/platform_device.h>
#include "hvgr_defs.h"
#include "hvgr_pm_defs.h"

static inline void hvgr_pm_user_ref_add(struct hvgr_device *gdev)
{
	atomic_inc(&gdev->pm_dev.pm_pwr.user_ref);
}

static inline void hvgr_pm_user_ref_dec(struct hvgr_device *gdev)
{
	atomic_dec(&gdev->pm_dev.pm_pwr.user_ref);
}

/*
 * hvgr_pm_is_need_power_on - Determine whether the GPU needs to be powered
 * on.
 *
 * @param gdev - The gdev object pointer.
 * @gpu_reset - GPU reset flag
 * @return: True if the gpu needed to be power on.
 * @date - 2021-02-03
 */
bool hvgr_pm_is_need_power_on(struct hvgr_device *gdev, bool gpu_reset);

/*
 * hvgr_pm_is_need_power_off - Determine whether the GPU needs to be powered
 * off.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: True if the gpu needed to be power off.
 * @date - 2021-02-03
 */
bool hvgr_pm_is_need_power_off(struct hvgr_device *gdev);

/*
 * hvgr_pm_policy_init - Power Management Policy Initialization.
 *
 * The initialization power policy of the FPGA is always_on, and
 * the initialization power policy of the SOC is job_demand.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_policy_init(struct hvgr_device *gdev);

/*
 * hvgr_pm_policy_term - Power Management Policy term.
 * off.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_policy_term(struct hvgr_device *gdev);
#endif
