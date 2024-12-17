/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#ifndef VIRTIO_IOMMU_H
#define VIRTIO_IOMMU_H
#include <linux/dma-buf.h>

struct iommu_head_info {
	uint64_t vfb_buf_key;
	int ops_code;
	int sg_nents;
	uint64_t map_size;
	int err_code;
	unsigned long out_size;
};

struct dma_buf *dpu_vfb_get_dmabuf(int sharefd);
int dpu_vfb_map_iova(struct dma_buf *dmabuf, int32_t fd);
void decrease_buf_refcnt(uint64_t buf_key);
void set_buf_decrease_flag(uint64_t buf_key);

#endif