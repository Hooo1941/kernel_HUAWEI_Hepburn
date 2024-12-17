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

#include "dkmd_log.h"
#include "dkmd_object.h"

#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "dpu_vfb_bufmgr.h"

#define VFB_GET_BUF_SLEEP_MS 1
#define VFB_GET_BUF_TIMEOUT_NS 300000000

void dpu_vfb_buffer_clear(struct vfb_buffer* buffer)
{
	unsigned long flags;
	dpu_check_and_no_retval(unlikely(!buffer), err, "buffer is null pointer");
	spin_lock_irqsave(&buffer->lock, flags);
	buffer->used_flag = false;
	buffer->present_fence = NULL;
	atomic_inc(&buffer->bufmgr->avail_buffer_num);
	dpu_pr_info("buffer[%u]: clear success, bufmgr avail_buffer_num = %d\n",
				buffer->index, atomic_read(&buffer->bufmgr->avail_buffer_num));
	spin_unlock_irqrestore(&buffer->lock, flags);
}

void dpu_vfb_buffer_connect_fence(struct vfb_buffer* buffer, struct vfb_fence *present_fence)
{
	unsigned long flags;
	dpu_check_and_no_retval(unlikely(!buffer), err, "buffer is null pointer");
	dpu_check_and_no_retval(unlikely(!present_fence), err, "present_fence is null pointer");
	spin_lock_irqsave(&buffer->lock, flags);
	buffer->present_fence = present_fence;
	present_fence->buffer = buffer;
	spin_unlock_irqrestore(&buffer->lock, flags);
}

static struct vfb_buffer* bufmgr_get_avaliable_buffer(struct vfb_bufmgr *bufmgr)
{
	uint32_t i;
	unsigned long flags;
	struct vfb_buffer* buffer = bufmgr->buffer;

	for (i = 0; i < bufmgr->buffer_num; i++) {
		spin_lock_irqsave(&buffer->lock, flags);
		if (buffer[i].used_flag == false) {
			buffer[i].used_flag = true;
			atomic_dec(&bufmgr->avail_buffer_num);
			dpu_pr_info("buffer[%u]: init success, bufmgr avail_buffer_num %d\n", i, atomic_read(&bufmgr->avail_buffer_num));
			spin_unlock_irqrestore(&buffer->lock, flags);
			return &buffer[i];
		}
		spin_unlock_irqrestore(&buffer->lock, flags);
	}
	return NULL;
}

struct vfb_buffer* dpu_vfb_get_avaliable_buffer(struct vfb_bufmgr *bufmgr)
{
	ktime_t timestart;
	ktime_t timewait;
	ktime_t duration = 0;

	dpu_check_and_return(unlikely(!bufmgr), NULL, err, "bufmgr is null pointer");

	/* Back pressure when there's not enough buffer(sleep 1ms per query) */
	timestart = ktime_get();
	while ((atomic_read(&bufmgr->avail_buffer_num) == 0) && duration < VFB_GET_BUF_TIMEOUT_NS) {
		msleep(VFB_GET_BUF_SLEEP_MS);
		duration = ktime_get() - timestart;
	}

	timewait = ktime_get() - timestart;
	if (timewait > VFB_GET_BUF_SLEEP_MS * 1000000)
		dpu_pr_info("Start time %lld, Back pressure time %lldms\n", timestart, timewait / 1000000);

	return bufmgr_get_avaliable_buffer(bufmgr);
}

struct vfb_buffer* dpu_vfb_get_buffer_by_data(struct virtio_fb_present_data *data)
{
	dpu_check_and_return(unlikely(!data), NULL, err, "data is null pointer");
	return container_of(data, struct vfb_buffer, virtio_data);
}

int32_t dpu_vfb_bufmgr_init(struct vfb_bufmgr *bufmgr)
{
	uint32_t i;
	struct vfb_buffer *buffer = NULL;

	dpu_check_and_return(unlikely(!bufmgr), -EINVAL, err, "bufmgr is null pointer");
	dpu_check_and_return(unlikely(bufmgr->buffer_num == 0), -EINVAL, err, "buffer_num invalid");

	buffer = kzalloc(sizeof(struct vfb_buffer) * bufmgr->buffer_num, GFP_KERNEL);
	if (!buffer) {
		dpu_pr_err("kzalloc buffer failed");
		return -ENOMEM;
	}

	for (i = 0; i < bufmgr->buffer_num; i++) {
		buffer[i].index = i;
		buffer[i].used_flag = false;
		buffer[i].present_fence = NULL;
		buffer[i].bufmgr = bufmgr;
		spin_lock_init(&buffer[i].lock);
	}
	bufmgr->buffer = buffer;
	atomic_set(&bufmgr->avail_buffer_num, (int32_t)bufmgr->buffer_num);

	return 0;
}

void dpu_vfb_bufmgr_release(struct vfb_bufmgr *bufmgr)
{
	dpu_check_and_no_retval(unlikely(!bufmgr), err, "bufmgr is null pointer");
	dpu_check_and_no_retval(unlikely(!bufmgr->buffer), err, "buffer is null pointer");

	kfree(bufmgr->buffer);
	bufmgr->buffer = NULL;
	bufmgr->buffer_num = 0;
}
