/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
#include "dpu_comp_online_utils.h"

static void dpu_online_switch_present_index(struct comp_online_present *present)
{
	present->displayed_idx = present->displaying_idx;
	present->displaying_idx = present->incoming_idx;
	present->incoming_idx = (present->incoming_idx + 1) % COMP_FRAME_MAX;
}

void dpu_online_utiles_post_process(struct comp_online_present *present)
{
	if (!present) {
		dpu_pr_err("present is null");
		return;
	}

	dpu_online_switch_present_index(present);
	present->vsync_ctrl.prev_timestamp = present->vsync_ctrl.timestamp;
}
