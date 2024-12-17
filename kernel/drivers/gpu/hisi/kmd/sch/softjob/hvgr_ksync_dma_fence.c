/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_ksync_dma_fence.h"

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/slab.h>


#include "hvgr_version.h"

static DEFINE_SPINLOCK(hvgr_ksync_dma_fence_lock);
static struct kmem_cache *hvgr_fence_slab;


static const char *hvgr_ksync_get_driver_name(struct dma_fence *fence)
{
	return HVGR_DIRVER_NAME;
}

static const char *hvgr_ksync_get_timeline_name(struct dma_fence *fence)
{
	return HVGR_TIMELINE_NAME;
}

static bool hvgr_ksync_fence_signaling(struct dma_fence *fence)
{
	return true;
}

static void hvgr_ksync_fence_release(struct dma_fence *fence)
{
	kmem_cache_free(hvgr_fence_slab, fence);
}

const struct dma_fence_ops hvgr_dma_fence_ops = {
	.enable_signaling = hvgr_ksync_fence_signaling,
	.wait = dma_fence_default_wait,
	.get_driver_name = hvgr_ksync_get_driver_name,
	.get_timeline_name = hvgr_ksync_get_timeline_name,
	.release = hvgr_ksync_fence_release,
};

/**
 * Create dma_fence slab.
 * Return 0 if create slab successful, else return fail.
 */
int hvgr_fence_slab_init(void)
{
	hvgr_fence_slab = kmem_cache_create("hvgr_fence", sizeof(struct dma_fence),
		0, SLAB_HWCACHE_ALIGN, NULL);
	if (!hvgr_fence_slab)
		return -ENOMEM;

	return 0;
}

/**
 * destroy dma_fence slab.
 */
void hvgr_fence_slab_term(void)
{
	kmem_cache_destroy(hvgr_fence_slab);
}

/**
 * Create a new dma_fence object.
 * Return dma_fence pointer if successful, else return NULL.
 */
struct dma_fence *hvgr_ksync_create_dma_fence(struct hvgr_ctx * const ctx)
{
	int seqno;
	struct dma_fence *fence = NULL;

	fence = kmem_cache_zalloc(hvgr_fence_slab, GFP_KERNEL);
	if (fence == NULL)
		return NULL;

	seqno = atomic_inc_return(&ctx->ctx_cq.dma_fence_seqno);
	dma_fence_init(fence, &hvgr_dma_fence_ops,
		&hvgr_ksync_dma_fence_lock,
		ctx->ctx_cq.dma_fence_ctx, (unsigned long long)seqno);

	return fence;
}

/**
 * Destroy a dma_fence object.
 */
void hvgr_ksync_destroy_dma_fence(struct hvgr_ctx * const ctx, struct dma_fence * const fence)
{
	dma_fence_put(fence);
}
