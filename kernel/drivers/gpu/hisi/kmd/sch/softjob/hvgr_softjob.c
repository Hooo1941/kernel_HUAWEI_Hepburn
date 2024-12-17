/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_softjob.h"

#include <linux/workqueue.h>
#include <linux/file.h>
#include <linux/sync_file.h>
#include <linux/slab.h>
#include <linux/sched.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/kthread.h>
#endif
#include "securec.h"

#include "hvgr_asid_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_msync.h"
#include "hvgr_ksync.h"
#include "hvgr_wq.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_mem_api.h"
#ifdef HVGR_DATAN_JOB_TIMELINE
#include "hvgr_datan_api.h"
#endif

static struct kmem_cache *hvgr_token_data_slab;
static struct kmem_cache *hvgr_wq_entry_slab;

/* some flag cann't set at the same time. */
bool hvgr_token_is_flag_valid(struct token_callback_data *callback, uint64_t flag, uint32_t token)
{
	struct hvgr_ctx *ctx = callback->ctx;

	if ((flag == HVGR_TOKEN_CB_WAIT_FENCE) || (flag == HVGR_TOKEN_CB_TRIGGER_FENCE) ||
		(flag == HVGR_TOKEN_HOLD_EN_IN) || (flag == HVGR_TOKEN_HOLD_EN_OUT) ||
		(flag == HVGR_TOKEN_SC_UPDATE_POLICY)) {
		if ((callback->flags & (~HVGR_TOKEN_NORMAL_ENTRY)) != 0) {
			hvgr_err(ctx->gdev, HVGR_CQ,
				"%s ctx_%u flag 0x%llx token=%u, call_flag 0x%llx not right.",
				__func__, ctx->id, flag, token, callback->flags);
			return false;
		}
	}

	if ((flag == HVGR_TOKEN_FREQ_HINT) ||
		(flag == HVGR_TOKEN_CL_PROFILING_EXEC) ||
		(flag == HVGR_TOKEN_CL_PROFILING_COMPLETE)) {
		if ((callback->flags & (HVGR_TOKEN_CB_WQ_WAIT | HVGR_TOKEN_CB_WAIT_FENCE |
			HVGR_TOKEN_CB_TRIGGER_FENCE | HVGR_TOKEN_HOLD_EN_IN |
			HVGR_TOKEN_HOLD_EN_OUT | HVGR_TOKEN_SC_UPDATE_POLICY)) != 0) {
			hvgr_err(ctx->gdev, HVGR_CQ,
				"%s ctx_%u flag 0x%llx token=%u, call_flag 0x%llx not right.",
				__func__, ctx->id, flag, token, callback->flags);
			return false;
		}
	}
	return true;
}

enum hrtimer_restart hvgr_fence_timeout_callback(struct hrtimer *timer)
{
	struct token_callback_data *callback = NULL;
	unsigned long flags;
	struct hvgr_ctx *ctx = NULL;

	callback = container_of(timer, struct token_callback_data, fence_timeout_timer);
	spin_lock_irqsave(&callback->lock, flags);
	ctx = callback->ctx;
	if (ctx == NULL || ctx->gdev == NULL) {
		pr_err("[hvgr][HVGR_ERROR] fence timeout");
		goto out;
	}

	if (!ctx->gdev->cq_dev.submit_allow) {
		hvgr_info(ctx->gdev, HVGR_FENCE, "CQ submit is stop when fence timeout");
		goto out;
	}

	/* flag bit2 is wait fence, bit3 is trigger fence */
	hvgr_debug(ctx->gdev, HVGR_FENCE, "ctx_%u fence timeout tpid=%d pid=%d flag=0x%x token=%u",
		ctx->id, ctx->tgid, ctx->pid, callback->flags, callback->cq_event.token);

	if (callback->fence_data != NULL && callback->fence_data->ops != NULL)
		hvgr_debug(ctx->gdev, HVGR_FENCE, "fence timeout drv:%s, timeline:%s, seqno:%d\n",
			callback->fence_data->ops->get_driver_name(callback->fence_data),
			callback->fence_data->ops->get_timeline_name(callback->fence_data),
			callback->fence_data->seqno);

out:
	spin_unlock_irqrestore(&callback->lock, flags);
	return HRTIMER_NORESTART;
}

void hvgr_free_token_callback_cb(struct hvgr_cq_ctx_data *ctx_cq,
	struct token_callback_data *callback)
{
	uint32_t token = callback->token;
	struct token_wq_entry *wq_entry = NULL;

	mutex_lock(&ctx_cq->as_lock[token]);
	kmem_cache_free(hvgr_token_data_slab, callback);
	wq_entry = hvgr_token_wq_entry_remove(ctx_cq);
	if (wq_entry != NULL)
		kmem_cache_free(hvgr_wq_entry_slab, wq_entry);
	mutex_unlock(&ctx_cq->as_lock[token]);
}

void hvgr_callback_data_alloc_and_init(struct hvgr_ctx *ctx, uint32_t token)
{
	struct token_callback_data *cb_data = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct token_wq_entry *wq_entry = NULL;

	if (unlikely(ctx == NULL))
		return;

	ctx_cq = &ctx->ctx_cq;

	mutex_lock(&ctx_cq->as_lock[token]);
	cb_data = ctx_cq->token_callback[token];
	if (cb_data != NULL)
		goto out;

	cb_data = kmem_cache_zalloc(hvgr_token_data_slab, GFP_KERNEL);
	if (cb_data == NULL)
		goto out;

	wq_entry = kmem_cache_zalloc(hvgr_wq_entry_slab, GFP_KERNEL);
	if (wq_entry == NULL) {
		kmem_cache_free(hvgr_token_data_slab, cb_data);
		goto out;
	}

	cb_data->ctx = ctx;
	cb_data->dep_own_nums = 0;
	cb_data->token = token;
	spin_lock_init(&cb_data->lock);
	hvgr_token_wq_entry_add(ctx_cq, wq_entry);
	hrtimer_init(&cb_data->fence_timeout_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cb_data->fence_timeout_timer.function = hvgr_fence_timeout_callback;
	ctx_cq->token_callback[token] = cb_data;
out:
	mutex_unlock(&ctx_cq->as_lock[token]);
}

static inline void hvgr_fence_timeout_timer_start(struct hvgr_ctx * const ctx,
	struct token_callback_data *callback)
{
	uint64_t timeout_time = hvgr_is_fpga(ctx->gdev) ?
		HVGR_FENCE_TIMEOUT_TIME_FPGA : HVGR_FENCE_TIMEOUT_TIME;
	struct hvgr_cq_dev *gcqctx = &ctx->gdev->cq_dev;
	if (!gcqctx->fence_timeout_enable)
		return;
	hrtimer_start(&callback->fence_timeout_timer, ns_to_ktime(timeout_time), HRTIMER_MODE_REL);
}

static inline void hvgr_fence_timeout_timer_cancel(struct hvgr_ctx * const ctx,
	struct token_callback_data *callback)
{
	struct hvgr_cq_dev *gcqctx = &ctx->gdev->cq_dev;
	if (!gcqctx->fence_timeout_enable)
		return;
	(void)hrtimer_cancel(&callback->fence_timeout_timer);
}

static inline void hvgr_token_set_cq_event(struct hvgr_msync * const dest,
	struct hvgr_msync const * const msync)
{
	dest->token = msync->token;
	dest->sem_addr = msync->sem_addr;
	dest->sem_value = msync->sem_value;
}

struct token_wq_entry *hvgr_token_wq_entry_remove(struct hvgr_cq_ctx_data *ctx_cq)
{
	struct token_wq_entry *tw_entry = NULL;
	unsigned long flags;

	spin_lock_irqsave(&ctx_cq->callback_lock, flags);
	if (list_empty(&ctx_cq->wq_entry_list))
		goto out;

	tw_entry = list_first_entry(&ctx_cq->wq_entry_list, struct token_wq_entry, link);
	list_del(&tw_entry->link);
out:
	spin_unlock_irqrestore(&ctx_cq->callback_lock, flags);
	return tw_entry;
}

void hvgr_token_wq_entry_add(struct hvgr_cq_ctx_data *ctx_cq, struct token_wq_entry *tw_entry)
{
	unsigned long flags;

	spin_lock_irqsave(&ctx_cq->callback_lock, flags);
	list_add_tail(&tw_entry->link, &ctx_cq->wq_entry_list);
	spin_unlock_irqrestore(&ctx_cq->callback_lock, flags);
}

bool hvgr_token_register_wait_semaphore(struct hvgr_cq_ctx *cq_ctx,
	struct hvgr_msync * const msync, wq_work_entry *entry)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	struct token_wq_entry *tw_entry = NULL;
	uint64_t jit_addrs[WQ_JIT_NUMS];
	unsigned long flags;
	errno_t err;

	if (unlikely(!hvgr_token_is_valid(ctx, msync->token))) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token %u is invalid",
			__func__, msync->token);
		return false;
	}

	(void)memset_s(jit_addrs, sizeof(jit_addrs), 0, sizeof(jit_addrs));
	if (!hvgr_wq_opt_get_jit_addr(cq_ctx, entry, jit_addrs, WQ_JIT_NUMS)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, get jit addrs fail",
			__func__, msync->token);
		return false;
	}

	hvgr_callback_data_alloc_and_init(ctx, msync->token);
	callback = ctx->ctx_cq.token_callback[msync->token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, msync->token);
		return false;
	}

	spin_lock_irqsave(&callback->lock, flags);
	if (hvgr_semaphore_is_set(ctx, msync->sem_addr, msync->sem_value)) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	if (callback->flags != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"ctx_%u Token %d can not used", ctx->id, msync->token);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	tw_entry = hvgr_token_wq_entry_remove(&ctx->ctx_cq);
	if (unlikely(tw_entry == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "tw_entry is NULL.", __func__);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	err = memcpy_s(&tw_entry->wq_work, sizeof(*entry),
		entry, sizeof(*entry));
	if (err != EOK) {
		hvgr_token_wq_entry_add(&ctx->ctx_cq, tw_entry);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	err = memcpy_s(tw_entry->jit_addrs, sizeof(jit_addrs),
		jit_addrs, sizeof(jit_addrs));
	if (err != EOK) {
		hvgr_token_wq_entry_add(&ctx->ctx_cq, tw_entry);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	callback->tw_entry = tw_entry;
	callback->flags = HVGR_TOKEN_CB_WQ_WAIT;
#ifdef CONFIG_DFX_DEBUG_FS
	callback->set_flag = 1;
#endif
	callback->cq_ctx = cq_ctx;
	hvgr_token_set_cq_event(&callback->cq_event, msync);
	atomic_inc(&cq_ctx->wq_hold_count);

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "ctx_%u WQ register wait token = %u, count = %d",
		ctx->id, msync->token, (int)atomic_read(&cq_ctx->wq_hold_count));
	spin_unlock_irqrestore(&callback->lock, flags);

	return true;
}

void hvgr_token_do_wq_wait(struct hvgr_ctx *ctx,
	struct token_callback_data *callback)
{
	struct hvgr_cq_ctx *cq_ctx = NULL;
	uint32_t token;
	struct token_wq_entry *tw_entry = NULL;
	unsigned long flags;

	spin_lock_irqsave(&callback->lock, flags);
	/* callback is not wq wait */
	if ((callback->flags & HVGR_TOKEN_CB_WQ_WAIT) != HVGR_TOKEN_CB_WQ_WAIT ||
		callback->cq_ctx == NULL) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	cq_ctx = callback->cq_ctx;
	/* cq fault flow. */
	if (hvgr_cq_is_queue_error(cq_ctx)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u queue error", __func__, ctx->id);
		callback->cq_ctx = NULL;
		callback->flags = 0;
		tw_entry = callback->tw_entry;
		callback->tw_entry = NULL;
		spin_unlock_irqrestore(&callback->lock, flags);
		hvgr_token_wq_entry_add(&ctx->ctx_cq, tw_entry);
		return;
	}

	/* callback is wq wait but not trigger, need wait again */
	if (!hvgr_semaphore_is_set(ctx, callback->cq_event.sem_addr,
		callback->cq_event.sem_value)) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}
	callback->cq_ctx = NULL;
	callback->flags = 0;
	token = callback->cq_event.token;
	tw_entry = callback->tw_entry;

	callback->tw_entry = NULL;
	spin_unlock_irqrestore(&callback->lock, flags);

	hvgr_wq_do_entry(cq_ctx, &tw_entry->wq_work, tw_entry->jit_addrs, WQ_JIT_NUMS);
	if (atomic_dec_and_test(&cq_ctx->wq_hold_count))
		(void)hvgr_wq_start_work(cq_ctx);

	hvgr_token_wq_entry_add(&ctx->ctx_cq, tw_entry);
	hvgr_info(ctx->gdev, HVGR_CQ, "ctx_%u WQ do wait token = %u, count = %d", ctx->id,
		token, (int)atomic_read(&cq_ctx->wq_hold_count));

	wake_up(&ctx->gdev->wq_dev.barrier_wait_wq);
}

bool hvgr_token_register_hold_en_info(struct hvgr_ctx * const ctx, uint32_t token,
	enum protect_flag flag, void *data)
{
	struct token_callback_data *callback = NULL;
	unsigned long flags;
	bool ret = false;
	uint64_t callback_flag = (flag == CQ_PROTECT_MODE_IN) ?
		HVGR_TOKEN_HOLD_EN_IN : HVGR_TOKEN_HOLD_EN_OUT;

	hvgr_callback_data_alloc_and_init(ctx, token);
	callback = ctx->ctx_cq.token_callback[token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, token);
		return false;
	}

	spin_lock_irqsave(&callback->lock, flags);
	if (((callback->flags & callback_flag) != 0) ||
		!hvgr_token_is_flag_valid(callback, callback_flag, token)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u, fence token %u already set wait fence\n",
			__func__, ctx->id, token);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	if (callback->res_data == NULL) {
		callback->res_data = data;
		callback->flags |= callback_flag;
		ret = true;
	}
	spin_unlock_irqrestore(&callback->lock, flags);
	return ret;
}

bool hvgr_token_register_trigger_fence(struct hvgr_ctx * const ctx,
	struct hvgr_msync *msync, struct dma_fence *fence, bool ai_freq_enabled)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long flags;

	ctx_cq = &ctx->ctx_cq;
	hvgr_callback_data_alloc_and_init(ctx, msync->token);

	callback = ctx_cq->token_callback[msync->token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s token = %u, alloc callback data failed.",
			__func__, msync->token);
		return false;
	}

	spin_lock_irqsave(&callback->lock, flags);

	if (((callback->flags & HVGR_TOKEN_CB_TRIGGER_FENCE) != 0) ||
		!hvgr_token_is_flag_valid(callback, HVGR_TOKEN_CB_TRIGGER_FENCE, msync->token)) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u, fence token %u already register trigger fence",
			__func__, ctx->id,  msync->token);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	if (callback->fence_data == NULL) {
		callback->fence_data = fence;
		callback->cq_event.token = msync->token;
		callback->flags |= HVGR_TOKEN_CB_TRIGGER_FENCE;
		callback->ai_freq_enabled = ai_freq_enabled;
#ifdef CONFIG_DFX_DEBUG_FS
		callback->set_flag = 1;
#endif

		hvgr_info(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u, fence token %u callback->flags=0x%x", __func__,
			ctx->id, msync->token, callback->flags);
		/* This token already setted. */
		if (hvgr_semaphore_is_set(ctx, msync->sem_addr, msync->sem_value)) {
			spin_unlock_irqrestore(&callback->lock, flags);
			hvgr_token_finish(&ctx->ctx_cq.cq_ctx[0], msync->token);
		} else {
			hvgr_fence_timeout_timer_start(ctx, callback);
			spin_unlock_irqrestore(&callback->lock, flags);
		}

		return true;
	} else {
		spin_unlock_irqrestore(&callback->lock, flags);
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u, fence token %u TRIGGER_FENCE NOT_NULL", __func__,
			ctx->id, msync->token);
	}

	return false;
}

static void hvgr_token_do_trigger_fence(struct hvgr_ctx *ctx,
	struct token_callback_data *callback)
{
	unsigned long flags;
	struct dma_fence *fence = NULL;

	spin_lock_irqsave(&callback->lock, flags);

	if ((callback->flags & HVGR_TOKEN_CB_TRIGGER_FENCE) == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u, token %u callback->flags=0x%x", __func__,
		ctx->id, callback->cq_event.token, callback->flags);

	if (callback->fence_data == NULL) {
		hvgr_err(callback->ctx->gdev, HVGR_FENCE,
			"%s ctx_%u token callback trigger fence is NULL. token = %u\n",
			__func__, callback->ctx->id, callback->cq_event.token);

		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	fence = callback->fence_data;
	callback->fence_data = NULL;
#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
		if (callback->ai_freq_enabled &&
			(ctx->gdev->ai_freq_game_pid == callback->ctx->tgid))
			ctx->gdev->ai_freq_signaled_seqno++;
#endif

	spin_unlock_irqrestore(&callback->lock, flags);
	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u token=%u done.\n",
		__func__, ctx->id, callback->cq_event.token);

	hvgr_fence_timeout_timer_cancel(ctx, callback);

	hvgr_ksync_trigger_fence(ctx, fence, 0, callback->cq_event.token);
	hvgr_ksync_remove_fence(ctx, fence, callback->cq_event.token, TRIGGER_FENCE_FLAG);
}

bool hvgr_token_register_wait_fence(struct hvgr_ctx * const ctx,
	struct hvgr_msync *msync, struct dma_fence *fence,
	dma_fence_func_t callback_func)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long flags;
	uint32_t token;
	int ret;

	token = msync->token;
	if (!hvgr_token_is_valid(ctx, token))
		return false;

	ctx_cq = &ctx->ctx_cq;
	hvgr_callback_data_alloc_and_init(ctx, token);

	callback = ctx_cq->token_callback[token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s token = %u, alloc callback data failed.",
			__func__, token);
		return false;
	}
	spin_lock_irqsave(&callback->lock, flags);

	if (((callback->flags & HVGR_TOKEN_CB_WAIT_FENCE) != 0) ||
		!hvgr_token_is_flag_valid(callback, HVGR_TOKEN_CB_WAIT_FENCE, token)) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s ctx_%u, fence token %u already set wait fence\n",
			__func__, ctx->id, token);
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	if (callback->fence_data == NULL) {
		ret = dma_fence_add_callback(fence, &callback->fence_cb, callback_func);
		if (ret == 0) {
			hvgr_token_set_cq_event(&callback->cq_event, msync);
			callback->fence_data = fence;
			callback->flags |= HVGR_TOKEN_CB_WAIT_FENCE;
#ifdef CONFIG_DFX_DEBUG_FS
			callback->set_flag = 1;
#endif
			hvgr_fence_timeout_timer_start(ctx, callback);
			spin_unlock_irqrestore(&callback->lock, flags);
			return true;
		} else if (ret == -ENOENT) {
			ret = dma_fence_get_status(fence);
			if (ret == 1) {
				/* Fence signaled, get the completion result */
				hvgr_token_set_cq_event(&callback->cq_event, msync);
				callback->fence_data = fence;
				callback->flags |= HVGR_TOKEN_CB_WAIT_FENCE;
#ifdef CONFIG_DFX_DEBUG_FS
				callback->set_flag = 1;
#endif
				spin_unlock_irqrestore(&callback->lock, flags);

				hvgr_semaphore_set(ctx,
					callback->cq_event.sem_addr,
					callback->cq_event.sem_value);
#ifdef HVGR_DATAN_JOB_TIMELINE
				hvgr_timeline_record_fence(ctx, token);
#endif
				hvgr_token_finish(&ctx->ctx_cq.cq_ctx[0], token);

				return true;
			} else {
				hvgr_err(callback->ctx->gdev, HVGR_FENCE,
					"Wait fence register FAIL and status ERROR. token = %u, error = %d\n",
					token, ret);
			}
		} else {
			hvgr_err(callback->ctx->gdev, HVGR_FENCE,
				"Wait fence register FAIL. token = %u, error = %d\n",
				token, ret);
		}
	} else {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u, token %u callback wait fence is NOT_NULL\n",
			__func__, ctx->id, token);
	}

	spin_unlock_irqrestore(&callback->lock, flags);
	return false;
}

static void hvgr_token_do_wait_fence(struct hvgr_ctx *ctx,
	struct token_callback_data *callback)
{
	unsigned long flags;
	struct dma_fence *fence = NULL;

	spin_lock_irqsave(&callback->lock, flags);

	if ((callback->flags & HVGR_TOKEN_CB_WAIT_FENCE) == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	if (callback->fence_data == NULL) {
		spin_unlock_irqrestore(&callback->lock, flags);
		hvgr_err(callback->ctx->gdev, HVGR_FENCE,
			"ctx_%u, token callback wait fence is NULL. token = %x\n", ctx->id,
			callback->cq_event.token);
		return;
	}

	fence = callback->fence_data;
	(void)dma_fence_remove_callback(callback->fence_data, &callback->fence_cb);
	callback->fence_data = NULL;
	spin_unlock_irqrestore(&callback->lock, flags);

	hvgr_fence_timeout_timer_cancel(ctx, callback);
	hvgr_ksync_remove_fence(ctx, fence, callback->cq_event.token, WAIT_FENCE_FLAG);
}

void hvgr_token_unregister_wait_fence(struct hvgr_ctx * const ctx,
	uint32_t token)
{
	unsigned long flags;
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;

	if (!hvgr_token_is_valid(ctx, token))
		return;

	ctx_cq = &ctx->ctx_cq;
	callback = ctx_cq->token_callback[token];
	if (unlikely(callback == NULL))
		return;

	spin_lock_irqsave(&callback->lock, flags);

	if (callback->flags & HVGR_TOKEN_CB_WAIT_FENCE) {
		if (callback->fence_data != NULL) {
			(void)dma_fence_remove_callback(callback->fence_data,
				&callback->fence_cb);
			callback->fence_data = NULL;
			spin_unlock_irqrestore(&callback->lock, flags);

			hvgr_fence_timeout_timer_cancel(ctx, callback);
			hvgr_ksync_remove_fence(ctx, callback->fence_data, token, WAIT_FENCE_FLAG);
			return;
		} else {
			hvgr_err(callback->ctx->gdev, HVGR_FENCE,
				"ctx_%u, unregister wait fence, but fence is NULL. token = %u\n",
				ctx->id, token);
		}
	}

	spin_unlock_irqrestore(&callback->lock, flags);
}

void hvgr_token_do_freq_hint(struct hvgr_ctx *ctx, struct token_callback_data *callback)
{
	unsigned long flags;
	uint32_t hint;
	uint32_t dvfs_status;

	spin_lock_irqsave(&callback->lock, flags);
	if ((callback->flags & HVGR_TOKEN_FREQ_HINT) == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	hint = callback->freq_hint;
	switch (hint) {
	case HVGR_TOKEN_FREQ_HIGH_START:
		dvfs_status = HVGR_THRO_HIGH;
		break;
	case HVGR_TOKEN_FREQ_MED_START:
		dvfs_status = HVGR_THRO_MEDIUM;
		break;
	case HVGR_TOKEN_FREQ_LOW_START:
		dvfs_status = HVGR_THRO_LOW;
		break;
	case HVGR_TOKEN_FREQ_END:
		dvfs_status = HVGR_THRO_EMPTY;
		break;
	default:
		dvfs_status = HVGR_THRO_NORMAL;
		hvgr_err(ctx->gdev, HVGR_CQ, "Hint in token callback flag is invalid, hint = %d",
			hint);
		break;
	}
	callback->flags &= ~HVGR_TOKEN_FREQ_HINT;
	callback->freq_hint = 0;
	hvgr_cq_sched_update_dvfs_status(callback->cq_ctx, dvfs_status,
		HVGR_THRO_ACTION_JOB_BOUNDARY);
	spin_unlock_irqrestore(&callback->lock, flags);
	hvgr_update_freq_hint(ctx->gdev);
}

long hvgr_token_add_one_depend(struct hvgr_ctx * const ctx,
	struct token_callback_data *depd_callback, uint32_t dep_token)
{
	if (depd_callback->dep_own_nums < HVGR_TOKEN_DEP_NUMS) {
		depd_callback->dep_own_token[depd_callback->dep_own_nums] = dep_token;
		depd_callback->dep_own_nums++;
		hvgr_info(ctx->gdev, HVGR_CQ,
				"%s ctx_%u, token=%d, dep_own_nums=%u, dep_own_token=%u\n",
				__func__, ctx->id, depd_callback->cq_event.token,
				depd_callback->dep_own_nums, dep_token);
		return 0;
	}

	hvgr_err(ctx->gdev, HVGR_CQ,
		"%s ctx_%u, token=%d, dep own nums %d too big \n",
		__func__, ctx->id, dep_token, depd_callback->dep_own_nums);
	return -EINVAL;
}

long hvgr_token_set_dependency(struct hvgr_ctx * const ctx,
	uint32_t token, struct hvgr_msync *dependency)
{
	long ret = 0;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct token_callback_data *dep_callback = NULL;

	if (dependency->token >= CQ_TOKEN_NUMS)
		return -1;

	ctx_cq = &ctx->ctx_cq;
	hvgr_callback_data_alloc_and_init(ctx, dependency->token);
	dep_callback = ctx_cq->token_callback[dependency->token];
	if (unlikely(dep_callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, dependency->token);
		return -1;
	}

	spin_lock_irqsave(&dep_callback->lock, flags);

	if (hvgr_semaphore_is_set(ctx, dependency->sem_addr, dependency->sem_value)) {
		hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%u semp alreay set.",
			__func__, ctx->id, token);
		/* The dependency already set. */
		hvgr_token_finish(&ctx->ctx_cq.cq_ctx[0], token);
	} else {
		ret = hvgr_token_add_one_depend(ctx, dep_callback, token);
	}

	spin_unlock_irqrestore(&dep_callback->lock, flags);
	return ret;
}

static void hvgr_token_do_sc_update_policy(struct hvgr_ctx *ctx,
	struct token_callback_data *callback)
{
	unsigned long flags;
	void *policy_info = NULL;
	uint32_t info_num;
	uint64_t sem_addr;
	uint64_t sem_value;

	spin_lock_irqsave(&callback->lock, flags);
	if ((callback->flags & HVGR_TOKEN_SC_UPDATE_POLICY) == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return;
	}

	if (callback->policy_info == NULL) {
		spin_unlock_irqrestore(&callback->lock, flags);
		hvgr_err(callback->ctx->gdev, HVGR_CQ,
			"ctx_%u, token callback sc policy_info is NULL. token = %x", ctx->id,
			callback->cq_event.token);
		return;
	}

	policy_info = callback->policy_info;
	info_num = callback->policy_info_num;
	sem_addr = callback->cq_event.sem_addr;
	sem_value = callback->cq_event.sem_value;
	callback->policy_info = NULL;
	callback->policy_info_num = 0;
	spin_unlock_irqrestore(&callback->lock, flags);

	hvgr_mem_sc_update_policy(ctx, policy_info, info_num);
	hvgr_semaphore_set(ctx, sem_addr, sem_value);
}

void hvgr_token_do_cl_profiling(struct hvgr_ctx *ctx, struct token_callback_data *callback)
{
	unsigned long flags;
	void *timestamp_addr = NULL;
	uint64_t offset = 0;
	union hvgr_ioctl_para_timestamp *timestamp = NULL;
	uint64_t tmp_timestamp_addr = 0;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	spin_lock_irqsave(&callback->lock, flags);

	if ((callback->flags & HVGR_TOKEN_CL_PROFILING_EXEC) == 0 &&
		(callback->flags & HVGR_TOKEN_CL_PROFILING_COMPLETE) == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return;
	}
	tmp_timestamp_addr = callback->profiling_data.timestamp_addr;
	spin_unlock_irqrestore(&callback->lock, flags);

	if (hvgr_mem_vmap_user_addr(ctx, tmp_timestamp_addr, &timestamp_addr, &offset) != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"map gpuaddr = 0x%lx failed", tmp_timestamp_addr);
		timestamp_addr = NULL;
	}

	spin_lock_irqsave(&callback->lock, flags);
	if (timestamp_addr != NULL) {
		timestamp = (union hvgr_ioctl_para_timestamp *)(timestamp_addr + offset);
		timestamp->out.os_timestamp = callback->profiling_data.os_timestamp;
		timestamp->out.soc_timestamp = callback->profiling_data.soc_timestamp;
	} else {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"map gpuaddr = 0x%lx is nullptr", tmp_timestamp_addr);
	}

	callback->profiling_data.timestamp_addr = 0;
	callback->profiling_data.soc_timestamp = 0;
	callback->profiling_data.os_timestamp = 0;

	if ((callback->flags & HVGR_TOKEN_CL_PROFILING_EXEC) != 0) {
		callback->flags &= ~((uint64_t)HVGR_TOKEN_CL_PROFILING_EXEC);
		hvgr_semaphore_set(ctx,
			callback->profiling_data.wait_semp_addr,
			callback->profiling_data.wait_semp_value);
		callback->profiling_data.wait_semp_addr = 0;
		callback->profiling_data.wait_semp_value = 0;
	} else {
		callback->flags &= ~((uint64_t)HVGR_TOKEN_CL_PROFILING_COMPLETE);
	}

	spin_unlock_irqrestore(&callback->lock, flags);
	hvgr_mem_vunmap_user_addr(timestamp_addr);
	mutex_unlock(&ctx->mem_ctx.area_mutex);
}

static void hvgr_token_do_callback_list(struct hvgr_ctx *ctx, struct token_callback_data *callback)
{
	if ((callback->flags & HVGR_TOKEN_CB_WQ_WAIT) == HVGR_TOKEN_CB_WQ_WAIT) {
		hvgr_token_do_wq_wait(ctx, callback);
	} else {
		hvgr_token_do_trigger_fence(ctx, callback);

		hvgr_token_do_wait_fence(ctx, callback);

		hvgr_token_do_freq_hint(ctx, callback);

		hvgr_token_do_sc_update_policy(ctx, callback);

		hvgr_token_do_cl_profiling(ctx, callback);
	}
}

static bool hvgr_handle_token_work_direct(struct hvgr_ctx *ctx, uint32_t token)
{
	struct token_callback_data *callback;
	struct msync_event event;
	unsigned long flags;

	if (unlikely(token == CQ_ERR_FLOW_TOKEN_VALUE || !hvgr_token_is_valid(ctx, token)))
		return false;

	callback = ctx->ctx_cq.token_callback[token];
	if (unlikely(callback == NULL))
		return false;

	spin_lock_irqsave(&callback->lock, flags);
	if ((callback->flags & HVGR_TOKEN_CB_WORK_RUNNING) != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"ctx_%u, receive same token %u when work is running.",
			ctx->id, callback->cq_event.token);
		spin_unlock_irqrestore(&callback->lock, flags);
		return true;
	}

	if (callback->dep_own_nums > 0)
		hvgr_err(ctx->gdev, HVGR_CQ,
			"ctx_%u, token %d, callback->flags=0x%x, dep_own_nums=%u which is invalid",
			ctx->id, token, callback->flags, callback->dep_own_nums);

	if (callback->flags == 0) {
		spin_unlock_irqrestore(&callback->lock, flags);
		return false;
	}

	callback->flags |= HVGR_TOKEN_CB_WORK_RUNNING;
	spin_unlock_irqrestore(&callback->lock, flags);

	hvgr_token_do_callback_list(ctx, callback);

	spin_lock_irqsave(&callback->lock, flags);
	callback->dep_own_nums = 0;
	callback->flags = 0;
	callback->cq_ctx = NULL;
#ifdef CONFIG_DFX_DEBUG_FS
	callback->set_flag = 0;
#endif
	spin_unlock_irqrestore(&callback->lock, flags);

	event.status = BASE_MSYNC_SIGNAL;
	event.queue_id = 0;
	event.data = token;
	/* Token enqueue, but not notify. */
	hvgr_msync_event_enqueue(ctx, &event);
	return true;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static void hvgr_token_work(struct work_struct *data)
#else
static void hvgr_token_work(struct kthread_work *data)
#endif
{
	uint32_t token;
	uint32_t idx;
	struct hvgr_ctx *ctx = NULL;
	struct token_callback_data *callback = NULL;
	struct msync_event event;
	unsigned long flags;
	uint32_t token_tmp;
	uint32_t dep_own_token[HVGR_TOKEN_DEP_NUMS] = {0};
	uint32_t dep_own_nums = 0;

	callback = container_of(data, struct token_callback_data, work);
	ctx = callback->ctx;
	token = callback->token;

	hvgr_info(ctx->gdev, HVGR_CQ, "%s token %d,ctx_%u in", __func__, token, ctx->id);
	hvgr_token_do_callback_list(ctx, callback);

	event.status = BASE_MSYNC_SIGNAL;
	event.queue_id = 0;

	spin_lock_irqsave(&callback->lock, flags);
	if (unlikely(callback->dep_own_nums > HVGR_TOKEN_DEP_NUMS)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token %d depended token num %d too much",
			__func__, token, callback->dep_own_nums);
		goto exit;
	}
	/* Deal each dependency token */
	if (callback->dep_own_nums != 0) {
		hvgr_info(ctx->gdev, HVGR_CQ, "%s token %d depended token num %d(%d)", __func__,
			token, callback->dep_own_nums, callback->dep_own_token[0]);
		dep_own_nums = callback->dep_own_nums;
		(void)memcpy_s(dep_own_token, sizeof(dep_own_token),
			callback->dep_own_token, dep_own_nums * sizeof(callback->dep_own_token[0]));
	}
exit:
	hvgr_info(ctx->gdev, HVGR_CQ, "%s token %d,ctx_%u set dep_own_nums is %u, fence_data %s",
		__func__, token, ctx->id, callback->dep_own_nums,
		callback->fence_data == NULL ? "null" : "not null");
	callback->dep_own_nums = 0;
	callback->flags = 0;
	callback->cq_ctx = NULL;

#ifdef CONFIG_DFX_DEBUG_FS
	callback->set_flag = 0;
#endif
	spin_unlock_irqrestore(&callback->lock, flags);

	event.data = token;
	/* Token enqueue, but not notify. */
	hvgr_msync_event_enqueue(ctx, &event);

	for (idx = 0; idx < dep_own_nums; idx++) {
		token_tmp = dep_own_token[idx];
		if (unlikely(token_tmp == token) ||
			unlikely(!hvgr_token_is_valid(ctx, token_tmp))) {
			hvgr_err(ctx->gdev, HVGR_CQ, "Token duty-cycle or not valid. %d,ctx_%u",
				token, ctx->id);
			continue;
		}

		if (!hvgr_handle_token_work_direct(ctx, token_tmp)) {
			event.data = token_tmp;
			hvgr_msync_event_enqueue(ctx, &event);
		}
	}

	/* Wake up UMD */
	hvgr_msync_event_wakeup(ctx);
#ifdef CONFIG_LIBLINUX_HVGR_POLL_OPT
	if (hvgr_msync_event_checkdata(ctx) || (atomic_read(&ctx->softq_closed) != 0))
		liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, POLLIN | POLLRDNORM);
	else
		liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, 0);
#endif
}

static inline void hvgr_token_create_work(struct hvgr_ctx *ctx,
	struct token_callback_data *callback)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	INIT_WORK(&callback->work, hvgr_token_work);
	(void)queue_work(ctx->ctx_cq.token_callback_wq, &callback->work);
#else
	kthread_init_work(&callback->work, hvgr_token_work);
	(void)kthread_queue_work(ctx->ctx_cq.token_callback_wq, &callback->work);
#endif
}

bool hvgr_token_is_valid(struct hvgr_ctx * const ctx, uint32_t token)
{
	if (unlikely((token >= CQ_TOKEN_NUMS) || (token == 0))) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s. token=%d is invalid. ctx_%u",
			__func__, token, ctx->id);
		return false;
	}
	return true;
}

static bool hvgr_callback_deal(struct hvgr_ctx *ctx, struct token_callback_data *callback)
{
	if ((callback->flags & HVGR_TOKEN_CB_WORK_RUNNING) != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"ctx_%u, receive same token %u when work is running.",
			ctx->id, callback->cq_event.token);
		return true;
	}

	hvgr_info(ctx->gdev, HVGR_CQ,
			"ctx_%u, callback->dep_own_nums=%u, callback->flags=0x%x.",
			ctx->id, callback->dep_own_nums, callback->flags);
	if ((callback->dep_own_nums == 0) &&
		(callback->flags == 0))
		return false;

	callback->flags |= HVGR_TOKEN_CB_WORK_RUNNING;
	hvgr_token_create_work(ctx, callback);
	return true;
}

bool hvgr_token_callback_deal_without_lock(struct hvgr_ctx *ctx, uint32_t token)
{
	struct token_callback_data *callback = NULL;

	if (token == CQ_ERR_FLOW_TOKEN_VALUE || token >= CQ_TOKEN_NUMS)
		return false;

	callback = ctx->ctx_cq.token_callback[token];
	if (callback == NULL)
		return false;
	return hvgr_callback_deal(ctx, callback);
}

bool hvgr_token_callback_deal(struct hvgr_ctx *ctx, uint32_t token)
{
	unsigned long flags;
	struct token_callback_data *callback = NULL;
	bool ret = false;

	if (unlikely(token == CQ_ERR_FLOW_TOKEN_VALUE || token >= CQ_TOKEN_NUMS || token == 0))
		return false;

	callback = ctx->ctx_cq.token_callback[token];
	if (callback == NULL)
		return false;
	spin_lock_irqsave(&callback->lock, flags);

	ret = hvgr_callback_deal(ctx, callback);
	spin_unlock_irqrestore(&callback->lock, flags);

	return ret;
}

void hvgr_token_clean_all(struct hvgr_ctx *ctx, bool need_free_token)
{
	unsigned int idx;
	struct token_callback_data *callback = NULL;

	for (idx = 1; idx < CQ_TOKEN_NUMS; idx++) {
		callback = ctx->ctx_cq.token_callback[idx];
		if (callback == NULL)
			continue;

		hvgr_token_unregister_wait_fence(ctx, idx);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	flush_workqueue(ctx->ctx_cq.token_callback_wq);
#else
	kthread_flush_worker(ctx->ctx_cq.token_callback_wq);
#endif

	for (idx = 1; idx < CQ_TOKEN_NUMS; idx++) {
		callback = ctx->ctx_cq.token_callback[idx];
		if (callback == NULL)
			continue;

		hvgr_token_do_callback_list(ctx, callback);

		callback->flags = 0;
#ifdef CONFIG_DFX_DEBUG_FS
		callback->set_flag = 0;
#endif
		callback->dep_own_nums = 0;

		if (need_free_token) {
			hvgr_free_token_callback_cb(&ctx->ctx_cq, callback);
			ctx->ctx_cq.token_callback[idx] = NULL;
		}
	}
}

#ifdef CONFIG_DFX_DEBUG_FS
long hvgr_ioctl_cq_record_token(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_cq_record_token * const para)
{
	struct token_callback_data *callback = NULL;
	unsigned long flags;

	if (!hvgr_token_is_valid(ctx, para->in.token))
		return -1;
	hvgr_callback_data_alloc_and_init(ctx, para->in.token);

	callback = ctx->ctx_cq.token_callback[para->in.token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, para->in.token);
		return -1;
	}
	spin_lock_irqsave(&callback->lock, flags);
	callback->flags |= HVGR_TOKEN_NORMAL_ENTRY;
	callback->set_flag = 1;
	hvgr_info(ctx->gdev, HVGR_CQ, "%s. callback->flags=0x%x. set_flag=%d, ctx_%u",
		__func__, callback->flags, callback->set_flag, ctx->id);
	spin_unlock_irqrestore(&callback->lock, flags);
	return 0;
}

static void hvgr_callback_clear_set_flag(struct hvgr_ctx *ctx, uint32_t token)
{
	struct token_callback_data *callback = NULL;
	unsigned long flags;

	callback = ctx->ctx_cq.token_callback[token];
	if (callback == NULL)
		return;
	spin_lock_irqsave(&callback->lock, flags);
	callback->flags = 0;
	callback->set_flag = 0;
	hvgr_info(ctx->gdev, HVGR_CQ, "%s. callback->flags=0x%x. set_flag=%d, ctx_%u",
		__func__, callback->flags, callback->set_flag, ctx->id);
	spin_unlock_irqrestore(&callback->lock, flags);
}
#endif
/**
 * Token finish deal enter.
 */
void hvgr_token_finish(struct hvgr_cq_ctx * const cq_ctx, uint32_t token)
{
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	struct hvgr_device *gdev = hvgr_get_device();
	struct msync_event event;
	bool ret = false;

	if (ctx == NULL) {
		hvgr_err(gdev, HVGR_CQ, "%s ctx is null", __func__);
		return;
	}

	hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%u", __func__, ctx->id, token);
	ret = hvgr_token_callback_deal(ctx, token);
	/* wakeup directly */
	if (!ret) {
		event.data = token;
		event.status = (token == CQ_ERR_FLOW_TOKEN_VALUE) ?
			BASE_MSYNC_TASK_CANCELLED : BASE_MSYNC_SIGNAL;
		event.queue_id =
			(cq_ctx == &cq_ctx->ctx->ctx_cq.cq_ctx[0]) ? 0 : 1;

		if (token == CQ_ERR_FLOW_TOKEN_VALUE)
			hvgr_debug(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%u, status=%d",
				__func__, ctx->id, token, (unsigned int)event.status);
#ifdef CONFIG_DFX_DEBUG_FS
		else
			hvgr_callback_clear_set_flag(ctx, token);
#endif
		hvgr_msync_event_enqueue(ctx, &event);
		hvgr_msync_event_wakeup(ctx);
#ifdef CONFIG_LIBLINUX_HVGR_POLL_OPT
		if (hvgr_msync_event_checkdata(ctx) || (atomic_read(&ctx->softq_closed) != 0))
			liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, POLLIN | POLLRDNORM);
		else
			liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, 0);
#endif
	}
}

/**
 * Create  slab.
 * Return 0 if create slab successful, else return fail.
 */
int hvgr_token_cb_data_slab_init(void)
{
	hvgr_token_data_slab = kmem_cache_create("hvgr_token_cb_data",
		sizeof(struct token_callback_data), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!hvgr_token_data_slab)
		return -ENOMEM;

	hvgr_wq_entry_slab = kmem_cache_create("hvgr_wq_entry_data",
		sizeof(struct token_wq_entry), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!hvgr_wq_entry_slab) {
		kmem_cache_destroy(hvgr_token_data_slab);
		return -ENOMEM;
	}
	return 0;
}

/**
 * destroy dma_fence slab.
 */
void hvgr_token_cb_data_slab_term(void)
{
	kmem_cache_destroy(hvgr_token_data_slab);
	kmem_cache_destroy(hvgr_wq_entry_slab);
}

long hvgr_token_init(struct hvgr_ctx *ctx)
{
	struct hvgr_cq_ctx_data *ctx_cq = &ctx->ctx_cq;
	unsigned long flags;
	unsigned int i;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	ctx_cq->token_callback_wq = alloc_workqueue("token_callback_wq",
		WQ_MEM_RECLAIM | WQ_HIGHPRI | WQ_UNBOUND, 1);
	if (ctx_cq->token_callback_wq == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"alloc workqueue token_callback_wq fail.\n");
		return -1;
	}
#else
	ctx_cq->token_callback_wq = kthread_create_worker(0, "token_callback_wq");
	if (IS_ERR(ctx_cq->token_callback_wq)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "alloc workqueue token_callback_wq fail.\n");
		return -1;
	}
	sched_set_fifo(ctx_cq->token_callback_wq->task);
#endif

	spin_lock_irqsave(&ctx_cq->callback_lock, flags);
	for (i = 0; i < CQ_TOKEN_NUMS; i++)
		mutex_init(&ctx_cq->as_lock[i]);

	spin_unlock_irqrestore(&ctx_cq->callback_lock, flags);
	INIT_LIST_HEAD(&ctx_cq->wq_entry_list);
	return 0;
}

void hvgr_token_term(struct hvgr_ctx *ctx)
{
	if (ctx->ctx_cq.token_callback_wq == NULL)
		return;

	hvgr_token_clean_all(ctx, true);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	destroy_workqueue(ctx->ctx_cq.token_callback_wq);
#else
	kthread_destroy_worker(ctx->ctx_cq.token_callback_wq);
#endif
	ctx->ctx_cq.token_callback_wq = NULL;
}

long hvgr_token_register_sc_policy_callback(struct hvgr_ctx * const ctx,
	struct hvgr_msync *msync, uint64_t policy_info, uint32_t info_num)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long flags;
	long ret;

	if (!hvgr_token_is_valid(ctx, msync->token) || info_num > HVGR_MEM_SC_UPDATE_POLICY_MAX) {
		hvgr_err(ctx->gdev, HVGR_CQ, "sc policy INVALID token %u or info num %u",
			msync->token, info_num);
		return -EINVAL;
	}

	ctx_cq = &ctx->ctx_cq;
	hvgr_callback_data_alloc_and_init(ctx, msync->token);

	callback = ctx_cq->token_callback[msync->token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, msync->token);
		return -EINVAL;
	}

	spin_lock_irqsave(&callback->lock, flags);

	if ((callback->flags & HVGR_TOKEN_SC_UPDATE_POLICY) != 0 ||
		callback->policy_info != NULL ||
		!hvgr_token_is_flag_valid(callback, HVGR_TOKEN_SC_UPDATE_POLICY, msync->token)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"Token %u callback flags 0x%lx or policy_info not null",
			msync->token, callback->flags);
		spin_unlock_irqrestore(&callback->lock, flags);
		return -EINVAL;
	}

	ret = hvgr_mem_sc_get_policy_info(ctx, policy_info, info_num, &callback->policy_info);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ, "Token %u get policy_info fail %d", msync->token, ret);
	} else {
		callback->policy_info_num = info_num;
		callback->cq_event.token = msync->token;
		callback->cq_event.sem_addr = msync->sem_addr;
		callback->cq_event.sem_value= msync->sem_value;
		callback->flags |= HVGR_TOKEN_SC_UPDATE_POLICY;
		hvgr_info(ctx->gdev, HVGR_CQ,
			"%s ctx_%u, token %u callback->flags=0x%x", __func__,
			ctx->id, msync->token, callback->flags);
	}

	spin_unlock_irqrestore(&callback->lock, flags);
	return ret;
}

static void hvgr_cq_register_freq_hint(struct hvgr_ctx * const ctx, uint32_t token,
	uint32_t freq_hint)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long lock_flag;

	ctx_cq = &ctx->ctx_cq;
	hvgr_callback_data_alloc_and_init(ctx, token);
	callback = ctx_cq->token_callback[token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, token);
		return;
	}

	spin_lock_irqsave(&callback->lock, lock_flag);
	if (((callback->flags & HVGR_TOKEN_FREQ_HINT) != 0) ||
		!hvgr_token_is_flag_valid(callback, HVGR_TOKEN_FREQ_HINT, token)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, flag=0x%llx.",
			__func__, token, callback->flags);
		spin_unlock_irqrestore(&callback->lock, lock_flag);
		return;
	}

	callback->flags |= HVGR_TOKEN_FREQ_HINT;
	callback->freq_hint = freq_hint;
	spin_unlock_irqrestore(&callback->lock, lock_flag);
}

long hvgr_ioctl_cq_set_freq_hint(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_set_freq_hint * const para)
{
	uint32_t token_start = para->in.token_start;
	uint32_t token_end = para->in.token_end;
	uint32_t freq_hint = para->in.freq_hint;
	long ret = 0;

	if (unlikely(!hvgr_token_is_valid(ctx, token_start))) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token_start[%u] is invalid", __func__,
			token_start);
		return -EINVAL;
	}

	if (unlikely(!hvgr_token_is_valid(ctx, token_end))) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token_end[%u] is invalid", __func__,
			token_end);
		return -EINVAL;
	}

	if (unlikely(hvgr_cq_has_queue_error(ctx))) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"queue error, token_start = %u, token_end = %u freq_hint = %u.",
			token_start, token_end, freq_hint);
		return -EINVAL;
	}

	switch (freq_hint) {
	case HVGR_THRO_HIGH:
		hvgr_cq_register_freq_hint(ctx, token_start, HVGR_TOKEN_FREQ_HIGH_START);
		hvgr_cq_register_freq_hint(ctx, token_end, HVGR_TOKEN_FREQ_END);
		break;
	case HVGR_THRO_MEDIUM:
		hvgr_cq_register_freq_hint(ctx, token_start, HVGR_TOKEN_FREQ_MED_START);
		hvgr_cq_register_freq_hint(ctx, token_end, HVGR_TOKEN_FREQ_END);
		break;
	case HVGR_THRO_LOW:
		hvgr_cq_register_freq_hint(ctx, token_start, HVGR_TOKEN_FREQ_LOW_START);
		hvgr_cq_register_freq_hint(ctx, token_end, HVGR_TOKEN_FREQ_END);
		break;
	default:
		hvgr_err(ctx->gdev, HVGR_CQ, "Freq hint %d is invalid.", freq_hint);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static long hvgr_cq_register_cl_profiling(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_set_cl_profiling * const para, uint64_t flag)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long lock_flag = 0;
	uint64_t timestamp_addr;
	uint32_t token;
	uint64_t wait_semp_value;
	uint64_t wait_semp_addr;

	ctx_cq = &ctx->ctx_cq;

	if (flag == HVGR_TOKEN_CL_PROFILING_EXEC) {
		token = para->in.exec_token;
		timestamp_addr = para->in.exec_timestamp_addr;
		wait_semp_addr = hvgr_semaphore_get_addr(ctx, para->in.wait_semp_addr);
		if (wait_semp_addr == 0) {
			hvgr_err(ctx->gdev, HVGR_CQ,
				"%s ctx_%u semp_addr_tmp=0x%x not valid. \n",
				__func__, ctx->id, para->in.wait_semp_addr);
			return -EINVAL;
		}
		wait_semp_value = para->in.wait_semp_value;
	} else {
		token = para->in.complete_token;
		timestamp_addr = para->in.complete_timestamp_addr;
		wait_semp_addr = 0;
		wait_semp_value = 0;
	}

	hvgr_callback_data_alloc_and_init(ctx, token);
	callback = ctx_cq->token_callback[token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token = %u, alloc callback data failed.",
			__func__, token);
		return 0;
	}

	spin_lock_irqsave(&callback->lock, lock_flag);
	if (((callback->flags & flag) != 0) ||
		!hvgr_token_is_flag_valid(callback, flag, token)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s token=%u, flag=0x%llx not right.",
			__func__, token, callback->flags);
		spin_unlock_irqrestore(&callback->lock, lock_flag);
		return 0;
	}

	callback->flags |= flag;
	callback->profiling_data.timestamp_addr = timestamp_addr;
	callback->profiling_data.wait_semp_addr = wait_semp_addr;
	callback->profiling_data.wait_semp_value = wait_semp_value;
	spin_unlock_irqrestore(&callback->lock, lock_flag);
	return 0;
}

long hvgr_ioctl_cq_set_cl_profiling(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_set_cl_profiling * const para)
{
	long ret;

	if (!hvgr_token_is_valid(ctx, para->in.exec_token) ||
		!hvgr_token_is_valid(ctx, para->in.complete_token)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"Input token is invalid, exec_token = %u,complete_token = %u.",
			para->in.exec_token, para->in.complete_token);
		return -EINVAL;
	}

	if (para->in.exec_timestamp_addr == 0 || para->in.complete_timestamp_addr == 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"Input exec_addr = 0x%lx, complete_addr = 0x%lx is invalid!",
			para->in.exec_timestamp_addr, para->in.complete_timestamp_addr);
		return -EINVAL;
	}

	if (para->in.wait_semp_addr == 0 || para->in.wait_semp_value == 0) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"Input semp_addr = 0x%lx semp_value = 0x%lx is invalid!",
			para->in.wait_semp_addr, para->in.wait_semp_value);
		return -EINVAL;
	}

	if (unlikely(hvgr_cq_has_queue_error(ctx))) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"queue error, exec_token = %u, complete_token = %u.",
			para->in.exec_token, para->in.complete_token);
		return -EINVAL;
	}

	ret = hvgr_cq_register_cl_profiling(ctx, para, HVGR_TOKEN_CL_PROFILING_EXEC);
	if (ret != 0)
		return ret;
	ret = hvgr_cq_register_cl_profiling(ctx, para, HVGR_TOKEN_CL_PROFILING_COMPLETE);
	if (ret != 0)
		return ret;
	return 0;
}

long hvgr_ioctl_cq_clear_cl_flags(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_clear_cl_flags * const para)
{
	struct token_callback_data *callback = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	unsigned long lock_flag;

	if (!hvgr_token_is_valid(ctx, para->in.token)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"Input token is invalid, token = %u.", para->in.token);
		return -EINVAL;
	}

	ctx_cq = &ctx->ctx_cq;
	callback = ctx_cq->token_callback[para->in.token];
	if (unlikely(callback == NULL))
		return 0;

	spin_lock_irqsave(&callback->lock, lock_flag);
	if ((callback->flags & HVGR_TOKEN_CL_PROFILING_EXEC) != 0)
		callback->flags &= ~((uint64_t)HVGR_TOKEN_CL_PROFILING_EXEC);

	if ((callback->flags & HVGR_TOKEN_CL_PROFILING_COMPLETE) != 0)
		callback->flags &= ~((uint64_t)HVGR_TOKEN_CL_PROFILING_COMPLETE);

	if ((callback->flags & HVGR_TOKEN_FREQ_HINT) != 0)
		callback->flags &= ~((uint64_t)HVGR_TOKEN_FREQ_HINT);
	spin_unlock_irqrestore(&callback->lock, lock_flag);

	return 0;
}