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

#ifndef DPU_VFB_API_H
#define DPU_VFB_API_H

#include "../virtio/virtio_fb.h"
#include "dkmd_dpu.h"
#include "dkmd_vfb.h"

/*
 * virtio_fb_fill_product_config() save the produce config to VFB info.
 */
void vfb_fill_product_config(int index, struct vfb_product_config *config);

/*
 * vfb_present_done() signal fence and clear buffer.
 */
void vfb_present_done(int index, struct virtio_fb_present_data *data);

#endif
