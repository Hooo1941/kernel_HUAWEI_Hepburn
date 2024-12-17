/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_as.h"

#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_mmu_driver.h"
#include "hvgr_asid_api.h"

static uint32_t hvgr_cq_sched_find_as_for_ctx(const struct hvgr_ctx *ctx)
{
	struct hvgr_device *const gdev = ctx->gdev;
	int free_as;

	hvgr_info(gdev, HVGR_CQ, "%s ctx_%u as_free=0x%x, refcount=%d, asid=%d\n",
		__func__, ctx->id, gdev->as_free, atomic_read(&ctx->refcount), ctx->asid);
	if ((ctx->asid != (uint32_t)HVGR_AS_NR_INVALID) &&
		(gdev->as_free & ((u32)1 << ctx->asid)))
		return ctx->asid;

#if hvgr_version_ge(HVGR_V350)
	/* AS = 0 is reserved for fcp firmware load, hwcnt dump, issue, mmu queue */
	free_as = ffs(gdev->as_free & (u16)0x00be) - 1;
#else
	free_as = ffs((int)(gdev->as_free & FREE_AS_MASK)) - 1;
#endif
	if (free_as >= 0 && free_as < HVGR_MAX_NR_AS)
		return (uint32_t)free_as;

	return (uint32_t)HVGR_AS_NR_INVALID;
}

uint32_t hvgr_cq_sched_retain_ctx(struct hvgr_ctx *ctx)
{
	struct hvgr_device *const gdev = ctx->gdev;
	uint32_t free_as;
	struct hvgr_ctx *prev_ctx = NULL;

	hvgr_info(gdev, HVGR_CQ, "%s ctx_%u refcount (%d), ctx->asid=%d\n",
		__func__, ctx->id, atomic_read(&ctx->refcount), ctx->asid);
	if (atomic_inc_return(&ctx->refcount) >= 2) {
		WARN_ON(ctx->asid == HVGR_AS_NR_INVALID);
		return ctx->asid;
	}

	free_as = hvgr_cq_sched_find_as_for_ctx(ctx);
	if (free_as != (uint32_t)HVGR_AS_NR_INVALID) {
		gdev->as_free &= ~(1U << free_as);
		if ((free_as != ctx->asid) && hvgr_ctx_is_asid_valid(free_as)) {
			prev_ctx = gdev->as_to_ctx[free_as];
			if (prev_ctx != NULL) {
				hvgr_info(gdev, HVGR_CQ, "%s prev_ctx_%u refcnt(%d), free_as=%d\n",
					__func__, prev_ctx->id, atomic_read(&prev_ctx->refcount),
					free_as);
				WARN_ON(atomic_read(&prev_ctx->refcount) != 0);
				(void)hvgr_mmu_hal_disable(prev_ctx);
				prev_ctx->asid = HVGR_AS_NR_INVALID;
			}

			ctx->asid = free_as;
			gdev->as_to_ctx[free_as] = ctx;
		}
	} else {
		atomic_dec(&ctx->refcount);
		WARN_ON(ctx->asid != HVGR_AS_NR_INVALID);
	}
	hvgr_info(gdev, HVGR_CQ, "%s out ctx_%u refcount (%d),ctx->asid=%d\n",
		__func__, ctx->id, atomic_read(&ctx->refcount), ctx->asid);
	return ctx->asid;
}

uint32_t hvgr_alloc_asid(struct hvgr_ctx * const ctx)
{
	return hvgr_cq_sched_retain_ctx(ctx);
}

void hvgr_free_asid(struct hvgr_ctx * const ctx, uint32_t asid)
{
	hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u refcount old(%d)->new(%d)\n",
		__func__, ctx->id, atomic_read(&ctx->refcount), atomic_read(&ctx->refcount) - 1);
	if (unlikely(!hvgr_ctx_is_asid_valid(asid))) {
		hvgr_err(ctx->gdev, HVGR_SCH, "Asid %u is invalid", asid);
		return;
	}
	ctx->gdev->as_free |= (1U << ctx->asid);
}

