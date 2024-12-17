/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_softq.h"

#include <linux/errno.h>
#include <asm/barrier.h>
#include <securec.h>

#include "hvgr_defs.h"
#include "hvgr_mem_api.h"
#include "hvgr_kmd_defs.h"
#include "hvgr_log_api.h"

static int hvgr_alloc_softq_id(struct hvgr_ctx * const ctx)
{
	int unused_id;

	unused_id = ffs((int)(ctx->softq_unused & (u16)HVGR_MAX_SOFTQ_BITMAP)) - 1;
	if (unused_id >= 0 && unused_id < HVGR_MAX_NR_SOFTQ) {
		ctx->softq_unused &= ~(1U << unused_id);
		return unused_id;
	}

	return HVGR_INVALID_SOFTQ_ID;
}

static void hvgr_free_softq_id(struct hvgr_ctx * const ctx, uint32_t softq_id)
{
	ctx->softq_unused |= (1U << softq_id);
}

uint64_t hvgr_softq_get_flag(struct hvgr_soft_q * const qctx)
{
	return qctx->ctrl->flags;
}

uint64_t hvgr_softq_get_ctrl(struct hvgr_soft_q * const qctx)
{
	return qctx->ctrl_area->uva;
}

static struct hvgr_mem_area *hvgr_softq_alloc_ctrl(
	struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_alloc_para para;

	para.property = HVGR_MEM_PROPERTY_DRIVER;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(sizeof(struct hvgr_softq_ctrl));
	para.max_pages = para.pages;

	return hvgr_mem_allocate(ctx, &para, NULL, false);
}

static struct hvgr_mem_area *hvgr_softq_alloc_user_ctrl(
	struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_alloc_para para;

	para.property = HVGR_MEM_PROPERTY_DRIVER;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(sizeof(struct hvgr_softq_user_ctrl));
	para.max_pages = para.pages;

	return hvgr_mem_allocate(ctx, &para, NULL, false);
}

static struct hvgr_mem_area *hvgr_softq_alloc_queue(
	struct hvgr_ctx * const ctx, uint32_t entry_size,
	uint32_t entry_nums)
{
	struct hvgr_mem_alloc_para para;

	para.property = HVGR_MEM_PROPERTY_DRIVER;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR | HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(entry_nums * entry_size);
	para.max_pages = para.pages;

	return hvgr_mem_allocate(ctx, &para, NULL, false);
}

static bool hvgr_softq_alloc_memory(struct hvgr_ctx * const ctx,
	struct hvgr_soft_q * const qctx,
	uint32_t entry_size, uint32_t entry_nums)
{
	struct hvgr_mem_area *area = NULL;

	area = hvgr_softq_alloc_queue(ctx, entry_size, entry_nums);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s alloc softq queue mem fail.", __func__);
		return false;
	}
	qctx->queue_area = area;

	area = hvgr_softq_alloc_ctrl(ctx);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s alloc ctrl mem fail.", __func__);
		hvgr_mem_free(ctx, qctx->queue_area->uva);
		return false;
	}
	qctx->ctrl_area = area;

	area = hvgr_softq_alloc_user_ctrl(ctx);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s alloc user ctrl mem fail.", __func__);
		hvgr_mem_free(ctx, qctx->queue_area->uva);
		hvgr_mem_free(ctx, qctx->ctrl_area->uva);
		return false;
	}
	qctx->user_ctrl_area = area;

	return true;
}

static int hvgr_softq_init(struct hvgr_ctx * const ctx,
	struct hvgr_soft_q *qctx, uint32_t entry_size,
	uint32_t entry_nums, uint64_t flags)
{
	int softq_id;

	if (!hvgr_softq_alloc_memory(ctx, qctx, entry_size, entry_nums))
		return -ENOMEM;

	qctx->user_ctrl = (struct hvgr_softq_user_ctrl *)u64_to_ptr(qctx->user_ctrl_area->kva);
	qctx->user_ctrl->rd = 0;
	qctx->user_ctrl->wr = 0;

	qctx->ctrl = (struct hvgr_softq_ctrl *)u64_to_ptr(qctx->ctrl_area->kva);

	qctx->ctrl->entry_nums = entry_nums;
	qctx->ctrl->entry_size = entry_size;
	qctx->ctrl->flags = flags;
	qctx->ctrl->queue_base = qctx->queue_area->uva;
	qctx->ctrl->user_ctrl_addr = qctx->user_ctrl_area->uva;
	qctx->ctrl->rd = qctx->ctrl->wr = 0;
	qctx->ctx = ctx;
	qctx->recv = NULL;
	qctx->notify = NULL;
	qctx->bind_flag = false;
	spin_lock_init(&qctx->q_lock);

	mutex_lock(&ctx->softq_lock);
	softq_id = hvgr_alloc_softq_id(ctx);
	if (softq_id == HVGR_INVALID_SOFTQ_ID) {
		mutex_unlock(&ctx->softq_lock);
		hvgr_mem_free(ctx, qctx->queue_area->uva);
		hvgr_mem_free(ctx, qctx->ctrl_area->uva);
		hvgr_mem_free(ctx, qctx->user_ctrl_area->uva);
		return -ENOMEM;
	}
	ctx->id_to_qctx[softq_id] = qctx;
	qctx->ctrl->softq_id = (uint32_t)softq_id;
	qctx->flags = HVGR_SOFTQ_RUNNING;
	mutex_unlock(&ctx->softq_lock);
	return 0;
}

int hvgr_softq_create(struct hvgr_ctx * const ctx, uint32_t entry_size, uint32_t entry_nums,
	uint64_t flags, uint64_t *uva)
{
	int ret;
	struct hvgr_soft_q *qctx = NULL;

	if (unlikely(ctx == NULL || uva == NULL))
		return -EINVAL;

	mutex_lock(&ctx->ctx_lock);
	qctx = kvzalloc(sizeof(*qctx), GFP_KERNEL);
	if (qctx == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s alloc qctx mem fail.", __func__);
		goto error;
	}

	ret = hvgr_softq_init(ctx, qctx, entry_size, entry_nums, flags);
	if (ret != 0) {
		kvfree(qctx);
		goto error;
	}
	*uva = hvgr_softq_get_ctrl(qctx);
	mutex_unlock(&ctx->ctx_lock);
	return 0;
error:
	mutex_unlock(&ctx->ctx_lock);
	return -ENOMEM;
}

int hvgr_softq_term(struct hvgr_soft_q * const qctx)
{
	unsigned long irq_flag = 0;
	uint32_t softq_id;

	if (qctx == NULL)
		return -EINVAL;

	spin_lock_irqsave(&qctx->q_lock, irq_flag);
	softq_id = hvgr_softq_get_id(qctx);
	qctx->flags = HVGR_SOFTQ_VALID;
	spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
	if (qctx->queue_area != NULL)
		hvgr_mem_free(qctx->ctx, qctx->queue_area->uva);
	if (qctx->ctrl_area != NULL)
		hvgr_mem_free(qctx->ctx, qctx->ctrl_area->uva);
	if (qctx->user_ctrl_area != NULL)
		hvgr_mem_free(qctx->ctx, qctx->user_ctrl_area->uva);
	qctx->queue_area = NULL;
	qctx->ctrl_area = NULL;
	qctx->user_ctrl_area = NULL;
	qctx->ctrl = NULL;
	qctx->user_ctrl = NULL;
	qctx->bind_flag = false;
	mutex_lock(&qctx->ctx->softq_lock);
	qctx->ctx->id_to_qctx[softq_id] = NULL;
	hvgr_free_softq_id(qctx->ctx, softq_id);
	mutex_unlock(&qctx->ctx->softq_lock);
	kvfree(qctx);

	return 0;
}

void *hvgr_softq_get_entry(struct hvgr_soft_q * const qctx)
{
	void *entry = NULL;
	volatile struct hvgr_softq_ctrl *ctrl = qctx->ctrl;
	volatile struct hvgr_softq_user_ctrl *user_ctrl = qctx->user_ctrl;
	uint32_t entry_size = ctrl->entry_size;
	unsigned long irq_flag = 0;
	uint32_t alloc_idx;
	uint32_t curr_idx;

	spin_lock_irqsave(&qctx->q_lock, irq_flag);
	if ((ctrl->flags & (HVGR_SOFTQ_DIR_U_TO_K)) ==
		HVGR_SOFTQ_DIR_U_TO_K) {
		alloc_idx = ctrl->rd;
		curr_idx = user_ctrl->wr;
	} else if ((ctrl->flags & (HVGR_SOFTQ_DIR_K_TO_U)) ==
		HVGR_SOFTQ_DIR_K_TO_U) {
		alloc_idx = ctrl->wr;
		curr_idx = user_ctrl->rd;
	} else {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return NULL;
	}

	if (unlikely((alloc_idx >= ctrl->entry_nums) || (curr_idx >= ctrl->entry_nums))) {
		hvgr_err(qctx->ctx->gdev, HVGR_CQ, "%s. rd/wr ptr exceed entry nums\n", __func__);
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return NULL;
	}

	/* queue is empty or full */
	if (alloc_idx == curr_idx) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return NULL;
	}

	entry = u64_to_ptr(qctx->queue_area->kva + alloc_idx * entry_size);
	spin_unlock_irqrestore(&qctx->q_lock, irq_flag);

	return entry;
}

bool hvgr_softq_check_data(struct hvgr_soft_q * const qctx)
{
	unsigned long irq_flag;
	volatile struct hvgr_softq_ctrl *ctrl = NULL;
	volatile struct hvgr_softq_user_ctrl *user_ctrl = NULL;
	bool result = false;

	if (unlikely(qctx == NULL))
		return result;

	spin_lock_irqsave(&qctx->q_lock, irq_flag);
	ctrl = qctx->ctrl;
	user_ctrl = qctx->user_ctrl;
	if (unlikely(ctrl == NULL || user_ctrl == NULL)) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return result;
	}

	result = user_ctrl->rd != ctrl->wr;
	spin_unlock_irqrestore(&qctx->q_lock, irq_flag);

	return result;
}

int hvgr_softq_enqueue(struct hvgr_soft_q * const qctx,
	void const * const entry, uint32_t size)
{
	unsigned long irq_flag = 0;
	struct hvgr_softq_ctrl *ctrl = NULL;
	void *wp = NULL;
	int ret;

	if (unlikely(qctx == NULL || entry == NULL))
		return -EINVAL;

	spin_lock_irqsave(&qctx->q_lock, irq_flag);
	if (unlikely(qctx->flags == HVGR_SOFTQ_VALID)) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return -EINVAL;
	}
	/* Check soft queue direction and input entry size */
	if (unlikely(((qctx->ctrl->flags & (HVGR_SOFTQ_DIR_K_TO_U)) !=
		(HVGR_SOFTQ_DIR_K_TO_U)) ||
		(size > qctx->ctrl->entry_size))) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return -EINVAL;
	}

	ctrl = qctx->ctrl;

	wp = u64_to_ptr(qctx->queue_area->kva + ctrl->wr * ctrl->entry_size);
	ret = memcpy_s(wp, ctrl->entry_size, entry, size);
	if (ret != EOK) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return -EINVAL;
	}
	ctrl->wr = (ctrl->wr + 1) & (ctrl->entry_nums - 1);
	spin_unlock_irqrestore(&qctx->q_lock, irq_flag);

	return 0;
}

int hvgr_softq_dequeue(struct hvgr_soft_q * const qctx)
{
	unsigned long irq_flag = 0;
	volatile struct hvgr_softq_ctrl *ctrl = NULL;

	if (unlikely(qctx == NULL))
		return -EINVAL;

	spin_lock_irqsave(&qctx->q_lock, irq_flag);
	if (unlikely(qctx->flags == HVGR_SOFTQ_VALID)) {
		spin_unlock_irqrestore(&qctx->q_lock, irq_flag);
		return -EINVAL;
	}
	ctrl = qctx->ctrl;
	ctrl->rd = (ctrl->rd + 1) & (ctrl->entry_nums - 1);
	spin_unlock_irqrestore(&qctx->q_lock, irq_flag);

	return 0;
}

bool hvgr_softq_register_notify(struct hvgr_soft_q * const qctx,
	soft_q_notify notify, void *priv)
{
	if ((qctx == NULL) || (notify == NULL))
		return false;

	qctx->notify = notify;
	qctx->notify_priv = priv;

	return true;
}

void hvgr_softq_do_flush(struct hvgr_soft_q * const qctx)
{
	if (qctx->notify != NULL)
		qctx->notify(qctx->notify_priv);
}
