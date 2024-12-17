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

#include "dkmd_log.h"
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sync_file.h>
#include <securec.h>

#include "dpu_vfb_bufmgr.h"
#include "dpu_vfb_fence.h"

#define VFB_TIMELINE_VALUE 0
#define VFB_FENCE_VALUE 0
#define VFB_TIMELINE_AND_FENCE_SIZE_MAX 64

static const char *present_fence_get_driver_name(struct dma_fence *fence)
{
	return "dpu_vfb_fence";
}

static const char *present_fence_get_timeline_name(struct dma_fence *fence)
{
	return "dpu_vfb_timeline";
}

static bool present_fence_enable_signaling(struct dma_fence *fence)
{
	return true;
}

static void present_fence_release(struct dma_fence *fence)
{
	struct vfb_fence *present_fence = container_of(fence, struct vfb_fence, base);
	dpu_pr_info("fence_fd = %d, release success", present_fence->present_fence_fd);
	kfree(present_fence);
	present_fence = NULL;
}

static bool present_fence_is_signaled(struct dma_fence *fence)
{
	struct vfb_fence *present_fence = container_of(fence, struct vfb_fence, base);
	/* present_fence and buffer will be unconnected after signal */
	return !(present_fence->buffer);
}

static void present_fence_value_str(struct dma_fence *fence, char *str, int32_t size)
{
	int length;

	dpu_check_and_no_retval(size > VFB_TIMELINE_AND_FENCE_SIZE_MAX, err, "size err!");
	length = snprintf_s(str, size, size - 1, "%u", VFB_FENCE_VALUE);
	if (length < 0) {
		dpu_pr_err("snprintf_s fence_value_str fail, length=%d\n", length);
		return;
	}
}

static void present_fence_timeline_value_str(struct dma_fence *fence, char *str, int32_t size)
{
	int length;

	dpu_check_and_no_retval(size > VFB_TIMELINE_AND_FENCE_SIZE_MAX, err, "size err!");
	length = snprintf_s(str, size, size - 1, "%u", VFB_TIMELINE_VALUE);
	if (length < 0) {
		dpu_pr_err("snprintf_s fence_value_str fail, length=%d\n", length);
		return;
	}
}

static struct dma_fence_ops g_vfb_present_fence_ops = {
	.get_driver_name = present_fence_get_driver_name,
	.get_timeline_name = present_fence_get_timeline_name,
	.enable_signaling = present_fence_enable_signaling,
	.signaled = present_fence_is_signaled,
	.wait = dma_fence_default_wait,
	.release = present_fence_release,
	.fence_value_str = present_fence_value_str,
	.timeline_value_str = present_fence_timeline_value_str,
};

static int32_t vfb_get_fence_fd(struct dma_fence *fence)
{
	int32_t fd;
	struct sync_file *sync_file = NULL;

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		dpu_pr_err("fail to get unused fd");
		return fd;
	}

	sync_file = sync_file_create(fence);
	if (!sync_file) {
		put_unused_fd((uint32_t)fd);
		dpu_pr_err("failed to create sync file");
		return -ENOMEM;
	}

	fd_install((uint32_t)fd, sync_file->file);

	return fd;
}

int32_t dpu_vfb_present_fence_create(struct vfb_buffer *buffer, uint32_t frame_no, int32_t *present_fence_fd)
{
	struct vfb_fence *present_fence = NULL;
	int32_t fence_fd;

	dpu_check_and_return(unlikely(!buffer), -EINVAL, err, "buffer is null pointer");
	dpu_check_and_return(unlikely(!present_fence_fd), -EINVAL, err, "present_fence_fd is null pointer");

	present_fence = kzalloc(sizeof(*present_fence), GFP_KERNEL);
	if (!present_fence) {
		dpu_pr_err("kzalloc present_fence fail");
		return -ENOMEM;
	}

	spin_lock_init(&present_fence->lock);
	dma_fence_init(&present_fence->base, &g_vfb_present_fence_ops,
		&present_fence->lock, dma_fence_context_alloc(1), frame_no);
	fence_fd = vfb_get_fence_fd(&present_fence->base);
	if (fence_fd < 0) {
		kfree(present_fence);
		present_fence = NULL;
		return -EBADFD;
	}
	present_fence->present_fence_fd = fence_fd;
	*present_fence_fd = fence_fd;

	dpu_vfb_buffer_connect_fence(buffer, present_fence);

	dpu_pr_info("buffer[%u]: fence_fd = %d, create success", buffer->index, fence_fd);
	return 0;
}

/*
 * If anything fails in present after fence create, release fence and fd.
 * It can only be used before return fence. After that, we must use the
 * 'dma_fence_put' to decreases refcount and release fence, and fd need
 * to be released by user.
 */
void dpu_vfb_present_fence_release(struct vfb_fence *present_fence)
{
	dpu_check_and_no_retval(unlikely(!present_fence), err, "present_fence is null pointer");
	dpu_pr_info("buffer[%u]: fence_fd = %d, buf_key = 0x%llx, release success", present_fence->buffer->index,
				present_fence->present_fence_fd, present_fence->buffer->virtio_data.layer_infos[0].buf_key);
	ksys_close((uint32_t)present_fence->present_fence_fd);
	dma_fence_put(&present_fence->base);
}

void dpu_vfb_present_fence_signal(struct vfb_fence *present_fence)
{
	struct dma_fence *fence = NULL;

	dpu_check_and_no_retval(unlikely(!present_fence), err, "present_fence is null pointer");
	fence = &present_fence->base;
	if (dma_fence_signal(fence) == 0) {
		/* disconnect present_fence and buffer after signal and before release */
		present_fence->buffer = NULL;
		dma_fence_put(fence);
	}
}
