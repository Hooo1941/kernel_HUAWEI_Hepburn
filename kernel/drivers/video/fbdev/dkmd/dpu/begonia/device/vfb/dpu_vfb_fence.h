/*
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
 *
 */

#ifndef DPU_VFB_FENCE_H
#define DPU_VFB_FENCE_H

#include "dpu_vfb.h"

int32_t dpu_vfb_present_fence_create(struct vfb_buffer *buffer, uint32_t frame_no, int32_t *present_fence_fd);
void dpu_vfb_present_fence_signal(struct vfb_fence *present_fence);
void dpu_vfb_present_fence_release(struct vfb_fence *present_fence);

#endif
