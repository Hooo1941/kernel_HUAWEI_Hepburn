/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_POWER_H
#define HVGR_PM_POWER_H

#include "hvgr_pm_defs.h"
#include "hvgr_pm_driver.h"

#ifdef CONFIG_HVGR_VIRTUAL
#define POWER_OFF_DEALY_TIME  60000000
#else
#define POWER_OFF_DEALY_TIME  1800000
#endif
/*
 * hvgr_pm_power_on_sync - Synchronously powering on the GPU.
 *
 * @param gdev - The gdev object pointer.
 * @gpu_reset - GPU reset flag
 * @date - 2021-02-03
 *
 * @return: True if power on was successful
 */
bool hvgr_pm_power_on_sync(struct hvgr_device *gdev, bool gpu_reset);

/*
 * hvgr_pm_power_off_sync - Synchronously powering off the GPU.
 * After the interface is executed, the GPU has been powered off.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_power_off_sync(struct hvgr_device *gdev);

#ifdef HVGR_GPU_POWER_ASYNC
/*
 * hvgr_pm_power_off_async - Asynchronously powering off the GPU.
 * After the interface is executed, the GPU is not powered off. Instead,
 * the GPU is powered off after gdev->pm_dev.pm_pwer.delay_time ns.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_power_off_async(struct hvgr_device *gdev);
#endif

/*
 * hvgr_pm_power_soft_init - PM module software initialization.
 * Initializes the software status, such as the lock, state machine,
 * workqueue and timer.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_power_soft_init(struct hvgr_device *gdev);

/*
 * hvgr_pm_power_soft_term - PM module termination
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_power_soft_term(struct hvgr_device *gdev);

#endif
