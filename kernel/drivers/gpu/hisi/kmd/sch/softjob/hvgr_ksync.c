/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_ksync.h"

#include <linux/moduleparam.h>
#include <linux/anon_inodes.h>
#include <linux/sync_file.h>
#include <linux/semaphore.h>
#include <linux/file.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/syscalls.h>

#ifdef HVGR_DATAN_JOB_TIMELINE
#include "hvgr_datan_api.h"
#endif
#include "hvgr_log_api.h"
#include "hvgr_ksync_dma_fence.h"
#include "hvgr_msync.h"
#include "hvgr_softjob.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_mem_api.h"
#include "hvgr_pm_api.h"

int hvgr_ksync_fence_validate(struct hvgr_ctx * const ctx,
	struct hvgr_ioctl_fence_validate *validate)
{
#if defined(CONFIG_SYNC) || defined(CONFIG_SYNC_FILE)
	struct dma_fence *fence = sync_file_get_fence(validate->fd);

	if (fence == NULL) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s ctx_%u get fence fail %d",
			__func__, ctx->id, validate->fd);
		return -EINVAL;
	}

	dma_fence_put(fence);

	return 0; /* valid */
#else
	return -ENOENT;
#endif
}

/**
 * Create a new sync fd, associate to ctx and return it.
 * Return fd if successful, else return error number.
 */
int hvgr_ksync_export_fd(struct hvgr_ctx * const ctx, struct dma_fence **new_fence)
{
	int fd = -1;
	struct dma_fence *fence = NULL;
	struct sync_file *sync_file = NULL;

	fence = hvgr_ksync_create_dma_fence(ctx);
	if (fence == NULL) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u create dma fence failed!", __func__, ctx->id);
		return -EINVAL;
	}

	sync_file = sync_file_create(fence);
	if (sync_file == NULL) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u create sync file failed!", __func__, ctx->id);
		hvgr_ksync_destroy_dma_fence(ctx, fence);
		return -EINVAL;
	}

	fd = get_unused_fd_flags(O_CLOEXEC);
	if (fd < 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u get unused fd flag failed!", __func__, ctx->id);
		fput(sync_file->file);
		hvgr_ksync_destroy_dma_fence(ctx, fence);
		return -EINVAL;
	}

	fd_install((unsigned int)fd, sync_file->file);
	*new_fence = fence;

	return fd;
}

/**
 * Convert a sync fd to dma_fence object.
 * Return a dma_fence pointer if successful, else return NULL.
 */
struct dma_fence *hvgr_ksync_get_fence(struct hvgr_ctx * const ctx, int fd)
{
	return sync_file_get_fence(fd);
}

/**
 * Destory a dma_fence.
 * flag: 0: trigger, 1 wait.
 */
void hvgr_ksync_remove_fence(struct hvgr_ctx * const ctx, struct dma_fence * const fence,
	uint32_t token, uint32_t flag)
{
	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u, flag=%d, fence token=%u, remove\n",
		__func__, ctx->id, flag, token);
	dma_fence_put(fence);

	if (flag == WAIT_FENCE_FLAG) {
		atomic_dec(&ctx->ctx_cq.wait_uncomplte_cnt);
		atomic_inc(&ctx->ctx_cq.wait_complte_cnt);
	}
}

static uint32_t hvgr_ksync_create_fence(struct hvgr_ctx * const ctx,
	int *fd, struct dma_fence **fence)
{
	int new_fd;
	struct dma_fence *new_fence = NULL;

	new_fd = hvgr_ksync_export_fd(ctx, &new_fence);
	if (new_fd < 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"Create fd fail when create fence ctx_%u", ctx->id);
		return -1;
	}

	*fd = new_fd;
	*fence = new_fence;

	return 0;
}

/**
 * Trigger a dma_fence.
 */
void hvgr_ksync_trigger_fence(struct hvgr_ctx * const ctx,
	struct dma_fence * const fence, int status, uint32_t token)
{
	if (status)
		dma_fence_set_error(fence, status);

	(void)dma_fence_signal(fence);
	atomic_dec(&ctx->ctx_cq.trigger_uncomplte_cnt);
	atomic_inc(&ctx->ctx_cq.trigger_complte_cnt);
	hvgr_info(ctx->gdev, HVGR_FENCE, "%s [fence_info] ctx_%u trigger fence token=%u, signaled.\n",
		__func__, ctx->id, token);
}

static void hvgr_ksync_wait_fence_callback(struct dma_fence *fence,
	struct dma_fence_cb *cb)
{
	struct token_callback_data *callback = container_of(cb,
		struct token_callback_data, fence_cb);

	hvgr_info(callback->ctx->gdev, HVGR_FENCE,
		"%s [fence_info] wait fence drv:%s timeline:%s seqno:%d %d\n",
		__func__, fence->ops->get_driver_name(fence),
		fence->ops->get_timeline_name(fence),
		fence->seqno, (int)callback->ctx->tgid);

	hvgr_info(callback->ctx->gdev, HVGR_FENCE, "%s [fence_info] ctx_%u, token=%d has signaled.\n",
		__func__, callback->ctx->id, callback->cq_event.token);
	hvgr_semaphore_set(callback->ctx,
		callback->cq_event.sem_addr,
		callback->cq_event.sem_value);
#ifdef HVGR_DATAN_JOB_TIMELINE
	if (callback != NULL && callback->ctx != NULL)
		hvgr_timeline_record_fence(callback->ctx, callback->cq_event.token);
#endif
	hvgr_token_finish(&callback->ctx->ctx_cq.cq_ctx[0],
		callback->cq_event.token);
}

long hvgr_ioctl_wait_fence(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_wait_fence_fd * const para)
{
	union hvgr_ioctl_para_msync_fd fd_para;

	atomic_inc(&ctx->ctx_cq.wait_total_cnt);
	atomic_inc(&ctx->ctx_cq.wait_uncomplte_cnt);
	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u, in token=%d,fd=%d\n",
		__func__, ctx->id, para->in.token, para->in.fd);
	fd_para.in.token = para->in.token;
	fd_para.in.sem_addr = para->in.semaphore_addr;
	fd_para.in.sem_value = para->in.semaphore_value;
	fd_para.in.fd = para->in.fd;
	return hvgr_ioctl_cq_set_fd(ctx, &fd_para);
}

long hvgr_ioctl_trigger_fence(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_trigger_fence_fd * const para)
{
	int fd;
	struct dma_fence *fence = NULL;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct hvgr_msync msync;
	uint32_t token;
	struct hvgr_msync dep_msync;
	uint64_t semp_addr_tmp = 0;

	token = para->in.token;
	if (!hvgr_token_is_valid(ctx, token)) {
		para->out.fd = -1;
		return -EINVAL;
	}

	if ((para->in.wait_token != CQ_TOKEN_INVALID) &&
		!hvgr_token_is_valid(ctx, para->in.wait_token)) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s ctx_%u wait_token=%u not valid\n",
			__func__, ctx->id, para->in.wait_token);
		para->out.fd = -1;
		return -EINVAL;
	}

	if (para->in.wait_token != CQ_TOKEN_INVALID) {
		semp_addr_tmp = hvgr_semaphore_get_addr(ctx, para->in.wait_semp_addr);
		if (semp_addr_tmp == 0) {
			hvgr_err(ctx->gdev, HVGR_FENCE,
				"%s ctx_%u semp_addr_tmp=0x%x not valid. \n",
				__func__, ctx->id, para->in.wait_semp_addr);
			para->out.fd = -1;
			return -EINVAL;
		}
	}
	if (para->in.wait_token == para->in.token) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u self dependency is not allowed. token = %u.\n",
			__func__, ctx->id, para->in.token);
		para->out.fd = -1;
		return -EINVAL;
	}

	mutex_lock(&ctx->ctx_lock);
	if (hvgr_ksync_create_fence(ctx, &fd, &fence) != 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s create fence fail token=%u.\n", __func__, token);
		para->out.fd = -1;
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	hvgr_info(ctx->gdev, HVGR_FENCE,
		"[fence_info] trig fence drv:%s timeline:%s seqno:%d %d\n",
		fence->ops->get_driver_name(fence),
		fence->ops->get_timeline_name(fence),
		fence->seqno, (int)ctx->tgid);

	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u token=%d fd=%d\n",
		__func__, ctx->id, para->in.token, fd);

	ctx_cq = &ctx->ctx_cq;

	atomic_inc(&ctx->ctx_cq.trigger_total_cnt);
	atomic_inc(&ctx->ctx_cq.trigger_uncomplte_cnt);

	if ((para->in.wait_token == CQ_TOKEN_INVALID) || (hvgr_cq_has_queue_error(ctx)))
		goto exit;

	msync.token = token;
	msync.sem_addr = 0;
	msync.sem_value = 0;
	if (!hvgr_token_register_trigger_fence(ctx, &msync, fence, para->in.ai_freq_enabled))
		goto exit;

	dep_msync.token = para->in.wait_token;
	dep_msync.sem_addr = semp_addr_tmp;
	dep_msync.sem_value = para->in.wait_semp_value;
	if (hvgr_token_set_dependency(ctx, token, &dep_msync) != 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"%s ctx_%u Set trigger fence dependency FAIL %x -> %x\n",
			__func__, ctx->id, token, dep_msync.token);

		hvgr_token_finish(&ctx_cq->cq_ctx[CQ_PRIORITY_LOW], token);
	}

	para->out.fd = fd;
	mutex_unlock(&ctx->ctx_lock);
	return 0;
exit:
	hvgr_ksync_trigger_fence(ctx, fence, 0, para->in.token);
	hvgr_ksync_remove_fence(ctx, fence, para->in.token, TRIGGER_FENCE_FLAG);

	hvgr_token_finish(&ctx_cq->cq_ctx[CQ_PRIORITY_LOW], token);
	para->out.fd = fd;
	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

void hvgr_ksync_init(struct hvgr_ctx * const ctx)
{
	struct hvgr_cq_ctx_data *ctx_cq = &ctx->ctx_cq;

	ctx_cq->dma_fence_ctx = dma_fence_context_alloc(1);
	atomic_set(&ctx_cq->dma_fence_seqno, 0);
}

long hvgr_ioctl_cq_get_fd(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_msync_fd * const para)
{
	int fd;
	struct dma_fence *fence = NULL;
	struct hvgr_msync msync;
	uint32_t token;

	if (unlikely((ctx == NULL) || (para == NULL)))
		return -EINVAL;

	token = para->in.token;
	if (!hvgr_token_is_valid(ctx, token))
		goto exit_err;

	msync.sem_addr = hvgr_semaphore_get_addr(ctx, para->in.sem_addr);
	if (msync.sem_addr == 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "%s ctx_%u semp_addr_tmp=0x%x not valid. \n",
			__func__, ctx->id, para->in.sem_addr);
		goto exit_err;
	}

	mutex_lock(&ctx->ctx_lock);
	if (hvgr_ksync_create_fence(ctx, &fd, &fence) != 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE, "Token %x create fence fail.\n", para->in.token);
		mutex_unlock(&ctx->ctx_lock);
		goto exit_err;
	}

	msync.sem_value = para->in.sem_value;
	msync.token = para->in.token;
	msync.fd = para->in.fd;
	if ((hvgr_cq_has_queue_error(ctx)) ||
		!hvgr_token_register_trigger_fence(ctx, &msync, fence, para->in.ai_freq_enabled)) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"[cq]: %s: ctx_%u: resigster fence callback FAIL. fd = %d",
			__func__, ctx->id, fd);
		hvgr_ksync_trigger_fence(ctx, fence, 0, para->in.token);
		hvgr_ksync_remove_fence(ctx, fence, para->in.token, TRIGGER_FENCE_FLAG);
		hvgr_token_finish(&ctx->ctx_cq.cq_ctx[0], token);
#ifndef CONFIG_LIBLINUX
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 17, 0)
		ksys_close((u32)fd);
#else
		sys_close((u32)fd);
#endif
#else
		__close_fd(NULL, (u32)fd);
#endif
		para->out.fd = -1;
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	para->out.fd = fd;
	mutex_unlock(&ctx->ctx_lock);
	return 0;
exit_err:
	para->out.fd = -1;
	return -EINVAL;
}

long hvgr_ioctl_cq_set_fd(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_msync_fd * const para)
{
	int fd;
	struct dma_fence *fence = NULL;
	struct hvgr_msync msync;

	if (unlikely((ctx == NULL) || (para == NULL) ||
		!hvgr_token_is_valid(ctx, para->in.token)))
		return -EINVAL;

	fd = para->in.fd;
	if (fd < 0) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"[cq]: %s: ctx_%u: fd is INVALID",
			__func__, ctx->id);
		return -EINVAL;
	}

	mutex_lock(&ctx->ctx_lock);
	fence = hvgr_ksync_get_fence(ctx, fd);
	if (fence == NULL) {
		hvgr_err(ctx->gdev, HVGR_FENCE,
			"[cq]: %s: ctx_%u: fence is NULL. fd = %d",
			__func__, ctx->id, fd);
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	hvgr_info(ctx->gdev, HVGR_FENCE, "[fence_info] wait fence drv:%s timeline:%s seqno:%d %d\n",
		fence->ops->get_driver_name(fence),
		fence->ops->get_timeline_name(fence),
		fence->seqno, (int)ctx->tgid);

	hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u, in token=%u,fd=%d\n",
		__func__, ctx->id, para->in.token, para->in.fd);

	msync.token = para->in.token;
	msync.sem_addr = hvgr_semaphore_get_addr(ctx, para->in.sem_addr);
	if (msync.sem_addr == 0) {
		hvgr_ksync_remove_fence(ctx, fence, para->in.token, WAIT_FENCE_FLAG);
		mutex_unlock(&ctx->ctx_lock);
		return -EINVAL;
	}

	msync.sem_value = para->in.sem_value;
	msync.fd = para->in.fd;
	if ((hvgr_cq_has_queue_error(ctx)) || (!hvgr_token_register_wait_fence(ctx,
		&msync, fence, hvgr_ksync_wait_fence_callback))) {
		hvgr_info(ctx->gdev, HVGR_FENCE, "%s ctx_%u, fence token=%u, reg wrong, set semp\n",
			__func__, ctx->id, para->in.token);
		hvgr_semaphore_set(ctx, msync.sem_addr, msync.sem_value);
#ifdef HVGR_DATAN_JOB_TIMELINE
		hvgr_timeline_record_fence(ctx, msync.token);
#endif
		hvgr_ksync_remove_fence(ctx, fence, para->in.token, WAIT_FENCE_FLAG);

		hvgr_token_finish(&ctx->ctx_cq.cq_ctx[0], msync.token);
	}

	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

long hvgr_ioctl_sc_update_policy(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_sc_update_policy * const para)
{
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct hvgr_msync msync;
	uint32_t token;
	struct hvgr_msync dep_msync;
	long ret;

	token = para->in.token;
	if (!hvgr_token_is_valid(ctx, token))
		return -EINVAL;

	ctx_cq = &ctx->ctx_cq;

	hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%d",
		__func__, ctx->id, para->in.token);
	msync.token = token;
	msync.sem_addr = hvgr_semaphore_get_addr(ctx, para->in.semaphore_addr);
	msync.sem_value = para->in.semaphore_value;
	ret = hvgr_token_register_sc_policy_callback(ctx, &msync, para->in.policy_info,
		para->in.info_num);
	if (ret != 0)
		return -EINVAL;

	if ((para->in.wait_token == CQ_TOKEN_INVALID) ||
		(hvgr_cq_is_queue_error(&ctx_cq->cq_ctx[CQ_PRIORITY_LOW]) ||
		hvgr_cq_is_queue_error(&ctx_cq->cq_ctx[CQ_PRIORITY_HIGH]))) {
		hvgr_semaphore_set(ctx, msync.sem_addr, msync.sem_value);
		hvgr_token_finish(&ctx_cq->cq_ctx[CQ_PRIORITY_LOW], token);
	} else {
		dep_msync.token = para->in.wait_token;
		dep_msync.sem_addr = hvgr_semaphore_get_addr(ctx, para->in.wait_semp_addr);
		dep_msync.sem_value = para->in.wait_semp_value;
		if (hvgr_token_set_dependency(ctx, token, &dep_msync) != 0) {
			hvgr_err(ctx->gdev, HVGR_CQ,
				"%s ctx_%u Set trigger fence dependency FAIL %x -> %x\n",
				__func__, ctx->id, token, dep_msync.token);

			hvgr_semaphore_set(ctx, msync.sem_addr, msync.sem_value);
			hvgr_token_finish(&ctx_cq->cq_ctx[CQ_PRIORITY_LOW], token);
		}
	}
	return 0;
}

long hvgr_ioctl_cq_set_top_notify(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_top_notify * const para)
{
	uint32_t token;
	struct token_callback_data *callback = NULL;

	token = para->in.notify_token;
	if (!hvgr_token_is_valid(ctx, token)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"%s token %d not right ", __func__, token);
		return -1;
	}

	hvgr_callback_data_alloc_and_init(ctx, token);
	callback = ctx->ctx_cq.token_callback[token];
	if (unlikely(callback == NULL)) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"%s notify_token token=%d init failed", __func__, token);
		return -1;
	}

	return 0;
}
