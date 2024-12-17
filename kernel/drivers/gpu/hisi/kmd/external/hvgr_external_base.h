/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_EXTERNAL_BASE_H
#define HVGR_EXTERNAL_BASE_H

#include <linux/regulator/consumer.h>

struct hvgr_device;
enum {
	HVGR_DEVFREQ_DEFAULT = 0,
	HVGR_DEVFREQ_SUSPEND,
	HVGR_DEVFREQ_RESUME
};

struct hvgr_platform_subsys_ops {
	int (*regulator_enable)(struct regulator *regulator);
	int (*regulator_disable)(struct regulator *regulator);
	int (*gpufreq_callback_register)(struct hvgr_device *gdev);
	bool (*is_ace_guest)(void);
	unsigned long (*get_gpu_id)(void);
	bool (*pm_is_always_on)(void);
	void (*update_devfreq_status)(int status);
};

struct hvgr_platform_subsys_ops *hvgr_subsys_ops_get(void);
#endif
