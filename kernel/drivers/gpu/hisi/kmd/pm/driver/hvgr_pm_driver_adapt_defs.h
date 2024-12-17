/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_DRIVER_ADAPT_DEFS_H
#define HVGR_PM_DRIVER_ADAPT_DEFS_H
#include <linux/types.h>

#define HVGR_OPER_CYCLE_TIMEOUT               0x1000
#define HVGR_MAX_GPC_NUM_POWER_UPDATE_ONCE    4

enum hvgr_pm_core_type {
	HVGR_PM_CORE_BL,
	HVGR_PM_CORE_GPC,
	HVGR_PM_CORE_MAX,
};
#endif