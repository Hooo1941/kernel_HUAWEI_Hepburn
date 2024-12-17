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

#ifndef DKMD_WLT_H
#define DKMD_WLT_H

#include <linux/types.h>

struct wlt_cmdlist_info {
	uint32_t scene_id;

	uint32_t slice0_cmdlist_id;
	uint32_t slice1_cmdlist_id;
	uint32_t slice2_cmdlist_id;
	uint32_t slice3_cmdlist_id;
};

void dkmd_wlt_set_cmdlist(struct wlt_cmdlist_info *cmdlist_info);

#endif