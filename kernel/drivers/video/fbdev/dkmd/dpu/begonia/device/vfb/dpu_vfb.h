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

#ifndef DPU_VFB_H
#define DPU_VFB_H

#include <linux/atomic.h>
#include <linux/dma-fence.h>
#include <linux/spinlock.h>

#include "../virtio/virtio_fb.h"

#define get_bufkey_by_dmabuf(dmabuf) ((uint64_t)(dmabuf))

struct vfb_fence {
	struct dma_fence base;
	spinlock_t lock;
	int32_t present_fence_fd;
	struct vfb_buffer *buffer;
};

/*
 * This structure holds the mapping between virtio_data and fence per present.
 * After the present finish, backend(DRM) need to send virtio_data to frontend(VFB)
 * by calling vfb_present_done(), VFB use it to find fence and signal fence.
 */
struct vfb_buffer {
	bool used_flag;
	uint32_t index;
	/*
	 * spinlock used for locking, vfb_present_done() will clear buffer in interrupt
	 */
	spinlock_t lock;
	struct virtio_fb_present_data virtio_data;
	struct vfb_fence *present_fence;
	struct vfb_bufmgr* bufmgr;
};

struct vfb_bufmgr {
	struct vfb_buffer *buffer;
	/*
	 * Number of buffer, it will be initialized by vfbdev_init_by_dts().
	 */
	uint32_t buffer_num;
	/*
	 * Available buffer num for bufmgr, it used to control back pressure.
	 * When staring an vfb present, we need to check whether the buffer
	 * is used up to prevent producers from generating buffer too fast and
	 * consumers have no time to consume them.
	 */
	atomic_t avail_buffer_num;
};

#endif
