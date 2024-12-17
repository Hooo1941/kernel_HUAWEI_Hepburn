/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef COMPOSER_SECURE_H
#define COMPOSER_SECURE_H

#include <linux/types.h>
#include <dkmd_dpu.h>

void dpu_online_drm_layer_config(struct disp_frame *prev_prev_frame,
		struct disp_frame *prev_frame, struct disp_frame *curr_frame);
void dpu_offline_drm_layer_config(struct disp_frame *curr_frame);
void dpu_offline_drm_layer_clear(struct disp_frame *curr_frame);

#endif