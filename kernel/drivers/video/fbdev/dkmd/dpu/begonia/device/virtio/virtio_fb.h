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

#ifndef VIRTIO_FB_H
#define VIRTIO_FB_H
#include <linux/scatterlist.h>
#include "virtio_iommu.h"

#define VIRTIO_FB_PRESENT_LAYER_NUM		1
#define VFB_BUF_KEY_BUF_OFFSET		16

struct virtio_fb_layer_info {
	uint32_t format;
	uint32_t stride;
	uint64_t buf_key;
	uint64_t iova;
};

struct virtio_fb_present_data {
	uint32_t frame_no;
	uint32_t magic_num;
	uint32_t layer_nums;
	struct virtio_fb_layer_info layer_infos[VIRTIO_FB_PRESENT_LAYER_NUM];
};

int virtio_fb_present(int index, struct virtio_fb_present_data *data);
int virtio_fb_send_sgl_info(struct sg_table *table, struct iommu_head_info *h_info);

#endif
