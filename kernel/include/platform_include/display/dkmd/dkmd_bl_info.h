/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef DKMD_BL_INFO_H
#define DKMD_BL_INFO_H

#include <linux/types.h>

enum bl_set_type {
	BL_SET_BY_NONE = BIT(0),
	BL_SET_BY_PWM = BIT(1),
	BL_SET_BY_BLPWM = BIT(2),
	BL_SET_BY_MIPI = BIT(3),
	BL_SET_BY_SH_BLPWM = BIT(4)
};

/* backlight info get from different lcd */
struct bl_info {
	uint32_t bl_type;
	uint32_t bl_min;
	uint32_t bl_max;
	uint32_t bl_default;
	uint32_t delay_set_bl_support; /* support delay set backlight threshold */
	uint32_t delay_set_bl_thr;     /* delay set backlight threshold */
};
#endif