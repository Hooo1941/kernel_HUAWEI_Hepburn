/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#ifndef DKMD_ESD_PANEL_INFO_H
#define DKMD_ESD_PANEL_INFO_H

#include <linux/types.h>

struct esd_panel_info {
	uint32_t esd_recovery_max_count;
	uint32_t esd_check_max_count;
	uint8_t esd_timing_ctrl;
	uint32_t esd_check_time_period;
	uint32_t esd_first_check_delay;
};

#endif