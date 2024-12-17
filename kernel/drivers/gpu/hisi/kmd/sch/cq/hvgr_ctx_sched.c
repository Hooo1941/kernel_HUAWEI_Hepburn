/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_ctx_sched.h"

#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_datan_api.h"

bool hvgr_ctx_is_asid_valid(uint32_t asid)
{
	if (unlikely((asid == (uint32_t)HVGR_AS_NR_INVALID) || (asid >= MMU_ADDR_SPACE_MAX)))
		return false;
	return true;
}

struct hvgr_ctx *hvgr_cq_lookup_ctx(struct hvgr_device *gdev, int asid)
{
	unsigned long flags;
	struct hvgr_ctx *found_ctx = NULL;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	if (hvgr_ctx_is_asid_valid((uint32_t)asid)) {
		found_ctx = gdev->as_to_ctx[asid];
		if (found_ctx == NULL) {
			hvgr_info(gdev, HVGR_CQ, " %s not found ctx by asid(%d)\n", __func__, asid);
			spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
			return NULL;
		}

		atomic_inc(&found_ctx->refcount);
	}
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
	return found_ctx;
}

struct hvgr_ctx *hvgr_cq_lookup_ctx_noretain(
	struct hvgr_device *gdev, int asid)
{
	struct hvgr_ctx *found_ctx = NULL;
	unsigned long flags;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	if (hvgr_ctx_is_asid_valid(asid))
		found_ctx = gdev->as_to_ctx[asid];
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);

	return found_ctx;
}

void hvgr_cq_sched_release_ctx_without_lock(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx)
{
	if (atomic_dec_return(&ctx->refcount) == 0)
		hvgr_free_asid(ctx, ctx->asid);
}

void hvgr_cq_sched_release_ctx(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx)
{
	unsigned long flags;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	hvgr_info(gdev, HVGR_CQ, "%s ctx_%u refcount old(%d)->new(%d) ,ctx->asid=%d\n",
		__func__, ctx->id, atomic_read(&ctx->refcount),
		atomic_read(&ctx->refcount) - 1, ctx->asid);
	if (atomic_dec_return(&ctx->refcount) == 0)
		hvgr_free_asid(ctx, ctx->asid);

	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
}

void kmd_ctx_sched_remove_ctx(struct hvgr_ctx *ctx)
{
	struct hvgr_device *const gdev = ctx->gdev;
	unsigned long flags;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	WARN_ON(atomic_read(&ctx->refcount) != 0);

	mutex_lock(&pm_pwr->power_lock);
	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	if (hvgr_ctx_is_asid_valid(ctx->asid)) {
		if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_GPU_READY)
			(void)hvgr_mmu_hal_disable(ctx);

		gdev->as_to_ctx[ctx->asid] = NULL;
		ctx->asid = HVGR_AS_NR_INVALID;
		hvgr_info(gdev, HVGR_CQ, "%s ctx->asid = 0x%x", __func__, ctx->asid);
	}
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
	mutex_unlock(&pm_pwr->power_lock);
}

bool hvgr_cq_retain_active_ctx(struct hvgr_device * const gdev,
	struct hvgr_ctx * const ctx)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	uint32_t chn_idx;

	for (chn_idx = 0; chn_idx < CQ_CHANNEL_NUM; chn_idx++) {
		if (gcqctx->channel_info[chn_idx].running_queue == NULL)
			continue;

		if (gcqctx->channel_info[chn_idx].running_queue->ctx == ctx) {
			atomic_inc(&ctx->refcount);
			return true;
		}
	}

	return false;
}

bool hvgr_ctx_sched_in(struct hvgr_ctx * const ctx)
{
	struct hvgr_device *gdev = ctx->gdev;

	ctx->asid = hvgr_alloc_asid(ctx);
	hvgr_info(gdev, HVGR_CQ, "%s asid=%d.\n", __func__, ctx->asid);
	if (unlikely(!hvgr_ctx_is_asid_valid(ctx->asid))) {
		hvgr_err(gdev, HVGR_CQ, "%s get asid fail.\n", __func__);
		return false;
	}
	hvgr_datan_jobtrace(ctx, JOBTRACE_TYPE_OBJ, KDATAN_TL_RET_AS_CTX, ctx, ctx->asid);
	hvgr_mmu_flush_tlb(ctx);
	return true;
}

void hvgr_ctx_sched_out(struct hvgr_ctx * const ctx)
{
	struct hvgr_device *gdev = ctx->gdev;

	if (unlikely(!hvgr_ctx_is_asid_valid(ctx->asid))) {
		hvgr_err(gdev, HVGR_CQ, "%s asid invalid.\n", __func__);
		return;
	}

	if (hvgr_pm_get_gpu_status(ctx->gdev) == HVGR_PM_GPU_READY)
		(void)hvgr_mmu_hal_disable(ctx);

	if (atomic_read(&ctx->refcount) == 0) {
		hvgr_err(gdev, HVGR_CQ, "%s ctx_%u refcount is already 0.\n", __func__, ctx->id);
		return;
	}

	if (atomic_dec_return(&ctx->refcount) == 0)
		hvgr_free_asid(ctx, ctx->asid);
}

bool hvgr_ctx_sched_in_with_lock(struct hvgr_ctx * const ctx)
{
	unsigned long flags;
	bool ret = false;
	struct hvgr_device *gdev = ctx->gdev;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	ret = hvgr_ctx_sched_in(ctx);
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
	return ret;
}

void hvgr_ctx_sched_out_with_lock(struct hvgr_ctx * const ctx)
{
	unsigned long flags;
	struct hvgr_device *gdev = ctx->gdev;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, flags);
	hvgr_ctx_sched_out(ctx);
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, flags);
}

