/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_KSYNC_DMA_FENCE_H
#define HVGR_KSYNC_DMA_FENCE_H

#include <linux/dma-fence.h>

#include "hvgr_defs.h"

/**
 * Create a new dma_fence object.
 * Return dma_fence pointer if successful, else return NULL.
 */
struct dma_fence *hvgr_ksync_create_dma_fence(struct hvgr_ctx * const ctx);

/**
 * Destroy a dma_fence object.
 */
void hvgr_ksync_destroy_dma_fence(struct hvgr_ctx * const ctx, struct dma_fence * const fence);

int hvgr_fence_slab_init(void);

void hvgr_fence_slab_term(void);

#endif
