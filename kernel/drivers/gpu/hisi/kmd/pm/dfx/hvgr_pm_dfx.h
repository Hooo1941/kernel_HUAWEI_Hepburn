/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_DFX_H
#define HVGR_PM_DFX_H
#include <linux/types.h>

#include "hvgr_pm_defs.h"
#include "hvgr_log_api.h"

void hvgr_pm_dfx_gpu_on_statistics(struct hvgr_device *gdev, uint64_t time);

void hvgr_pm_dfx_gpu_off_statistics(struct hvgr_device *gdev, uint64_t time);

void hvgr_pm_dfx_buck_on_statistics(struct hvgr_device *gdev, uint64_t time);

void hvgr_pm_dfx_buck_off_statistics(struct hvgr_device *gdev, uint64_t time);

void hvgr_pm_dfx_core_on_statistics(struct hvgr_device *gdev, uint64_t time);

void hvgr_pm_dfx_core_off_statistics(struct hvgr_device *gdev, uint64_t time);

uint64_t hvgr_pm_dfx_get_curtime(struct hvgr_device *gdev);

void hvgr_pm_dump_reg(struct hvgr_device *gdev, enum HVGR_LOG_LEVEL level);

void hvgr_pm_dfx_init(struct hvgr_device *gdev);
#endif
