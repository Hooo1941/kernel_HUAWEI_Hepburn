/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_KSYNC_H
#define HVGR_KSYNC_H

#include <linux/dma-fence.h>

#include "hvgr_defs.h"
#include "hvgr_ioctl_sch.h"

#define TRIGGER_FENCE_FLAG 0
#define WAIT_FENCE_FLAG 1

/**
 * Create a new sync fd, associate to ctx and return it.
 * Return fd and fence if successful, else return error number.
 */
int hvgr_ksync_export_fd(struct hvgr_ctx * const ctx, struct dma_fence **fence);

/**
 * Import a sync fd, associate to ctx.
 */
void hvgr_ksync_import_fd(struct hvgr_ctx * const ctx, int fd);

/**
 * Convert a sync fd to dma_fence object.
 * Return a dma_fence pointer if successful, else return NULL.
 */
struct dma_fence *hvgr_ksync_get_fence(struct hvgr_ctx * const ctx, int fd);

/**
 * Destory a dma_fence.
 */
void hvgr_ksync_remove_fence(struct hvgr_ctx * const ctx, struct dma_fence * const fence,
	uint32_t token, uint32_t flag);

/**
 * Wait dma_fence be setted.
 * This function will return when dma_fence be setted, cancle or timeout.
 */
uint32_t hvgr_ksync_wait_fence(struct hvgr_ctx * const ctx,
	struct dma_fence * const fence);

long hvgr_ioctl_wait_fence(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_wait_fence_fd * const para);

long hvgr_ioctl_trigger_fence(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_trigger_fence_fd * const para);

void hvgr_ksync_init(struct hvgr_ctx * const ctx);

void hvgr_ksync_trigger_fence(struct hvgr_ctx * const ctx,
	struct dma_fence * const fence, int status, uint32_t token);

long hvgr_ioctl_cq_set_fd(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_msync_fd * const para);

long hvgr_ioctl_cq_get_fd(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_msync_fd * const para);

int hvgr_ksync_fence_validate(struct hvgr_ctx * const ctx,
	struct hvgr_ioctl_fence_validate *validate);

long hvgr_ioctl_sc_update_policy(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_sc_update_policy * const para);

long hvgr_ioctl_cq_set_top_notify(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_top_notify * const para);
#endif
