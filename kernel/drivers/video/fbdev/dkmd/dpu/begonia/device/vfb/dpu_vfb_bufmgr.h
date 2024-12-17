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

#ifndef DPU_VFB_BUFMGR_H
#define DPU_VFB_BUFMGR_H

#include "dpu_vfb.h"

void dpu_vfb_buffer_connect_fence(struct vfb_buffer* buffer, struct vfb_fence *present_fence);
void dpu_vfb_buffer_clear(struct vfb_buffer* buffer);
struct vfb_buffer* dpu_vfb_get_avaliable_buffer(struct vfb_bufmgr *bufmgr);
struct vfb_buffer* dpu_vfb_get_buffer_by_data(struct virtio_fb_present_data *data);
int32_t dpu_vfb_bufmgr_init(struct vfb_bufmgr *bufmgr);
void dpu_vfb_bufmgr_release(struct vfb_bufmgr *bufmgr);

#endif
