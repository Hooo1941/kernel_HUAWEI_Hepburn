/*
 * vcodec_vdec_memory.h
 *
 * This is for vdec memory
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef VCODEC_VDEC_ION_H
#define VCODEC_VDEC_ION_H

#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/fb.h>
#include <linux/version.h>
#include <linux/iommu/mm_iommu.h>
#include <linux/bg_dmabuf_reclaim/bg_dmabuf_ops.h>
#include "vcodec_types.h"

int32_t vdec_mem_probe(void *dev);
int32_t vdec_mem_iommu_map(void *dev, int32_t share_fd, UADDR *iova, unsigned long *size);
int32_t vdec_mem_iommu_unmap(void *dev, int32_t share_fd, UADDR iova);
void *vdec_dma_buf_get(int32_t share_fd);
void vdec_dma_buf_put(void *dmabuf);
int32_t vdec_reclaim_dma_buf(int32_t share_fd);
int32_t vdec_restore_dma_buf(int32_t share_fd);
int32_t vdec_lock_buf_bg_reclaim(void *dmabuf);
void vdec_unlock_buf_bg_reclaim(void *dmabuf);
#endif
