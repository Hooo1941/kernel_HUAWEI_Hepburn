/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_STATE_MACHINE_H
#define HVGR_PM_STATE_MACHINE_H

#include <linux/types.h>

#include "hvgr_pm_defs.h"

/*
 * hvgr_pm_update_pwr_state - Power Management State Machine Transition.
 *
 * @param gdev - The gdev object pointer.
 * @dest_state - Target Status.
 *
 * @return: 0 if The target status is switched successfully.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_update_pwr_state(struct hvgr_device *gdev,
	enum hvgr_pm_gpu_status dest_state);

#endif
