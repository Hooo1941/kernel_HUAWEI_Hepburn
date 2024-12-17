/*
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

#ifndef DKMD_DUBAI_INTERFACE_H
#define DKMD_DUBAI_INTERFACE_H

#include <linux/types.h>

enum frm_rate_type {
	FRM_RATE_1HZ,
	FRM_RATE_5HZ,
	FRM_RATE_10HZ,
	FRM_RATE_15HZ,
	FRM_RATE_24HZ,
	FRM_RATE_30HZ,
	FRM_RATE_45HZ,
	FRM_RATE_60HZ,
	FRM_RATE_90HZ,
	FRM_RATE_120HZ,
	FRM_RATE_180HZ,
	FRM_RATE_GEARS_NUM
};

struct panel_refresh_statistic_info {
	uint32_t refresh_cnt[FRM_RATE_GEARS_NUM];
};

extern int32_t get_panel_refresh_count(uint32_t panel_id, struct panel_refresh_statistic_info *refresh_stat_info);

#endif
