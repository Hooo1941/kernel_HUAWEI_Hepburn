/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_GPU_RESET_H
#define HVGR_PM_GPU_RESET_H
#include <linux/types.h>
#include "hvgr_pm_defs.h"

/*
 * GPU soft reset timeout interval, in milliseconds.
 * After the GPU soft reset command is executed, the GPU reset is not complete
 * within 500 ms and an exception log is recorded.
 */
#define HVGR_PM_GPU_RESET_TIMEOUT                        500

#define HVGR_PM_GPU_RESET_DELAY_TIME_MS                  500
/*
 * hvgr_pm_gpu_reset_soft_init - Initialize the GPU reset waiting queue.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_gpu_reset_soft_init(struct hvgr_device *gdev);

#endif
