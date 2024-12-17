/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_msync.h"

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/mm.h>
#include <linux/completion.h>
#include <securec.h>

#include "apicmd/hvgr_cq_defs.h"
#include "apicmd/hvgr_kmd_defs.h"
#include "hvgr_mem_api.h"
#include "hvgr_assert.h"
#include "hvgr_msync_defs.h"
#include "hvgr_softjob.h"
#include "hvgr_log_api.h"
#include "hvgr_cq_driver_adapt.h"

#define CQ_SEMAPHORE_ADDR_ALIGNED_SIZE  (32U)  /* unit 4 byte. */
#define CQ_SEMAPHORE_NUMS   (8192U)  /* How may semaphore pre-alloc */
#define CQ_SEMAPHORE_PAGE_SIZE_L1  (CQ_SEMAPHORE_ADDR_ALIGNED_SIZE * CQ_SEMAPHORE_NUMS)
#define CQ_SEMAPHORE_PAGE_SIZE_L2  (CQ_SEMAPHORE_ADDR_ALIGNED_SIZE * TOKEN_NUMS)

uint64_t hvgr_semaphore_get_addr(struct hvgr_ctx * const ctx, uint64_t sema_addr)
{
	struct hvgr_cq_semp_info *info = NULL;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	u64 ret_addr;

	if (unlikely(ctx == NULL))
		return 0;

	ctx_cq = &ctx->ctx_cq;
	if (unlikely(sema_addr == 0 ||
		(sema_addr & (CQ_SEMAPHORE_ADDR_ALIGNED_SIZE - 1)) != 0))  {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s, sema_addr=0x%lx not valid\n",
			__func__, sema_addr);
		return 0;
	}

	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	info = &ctx->ctx_cq.semp_info;
	if (unlikely(!ctx_cq->semp_res_flag || info->semp_kva == 0 || info->semp_uva == 0 ||
		info->size == 0))
		goto exit;

	if (unlikely((sema_addr < info->semp_uva) ||
		(sema_addr >
		(info->semp_uva + info->size - CQ_SEMAPHORE_ADDR_ALIGNED_SIZE))))
		goto exit;

	ret_addr = info->semp_kva + (sema_addr - info->semp_uva);
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
	return ret_addr;
exit:
	hvgr_err(ctx->gdev, HVGR_CQ, "%s, sema_addr=0x%lx, semp_res_flag=%d\n",
		__func__, sema_addr, (int)ctx_cq->semp_res_flag);
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
	return 0;
}

void hvgr_semaphore_print_value(struct hvgr_ctx * const ctx, uint64_t sema_kaddr,
	uint64_t sema_uaddr, uint64_t sema_value)
{
	uint64_t cur_value;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct hvgr_cq_semp_info *info = NULL;

	if (unlikely((ctx == NULL) || (sema_kaddr == 0)))
		return;

	ctx_cq = &ctx->ctx_cq;

	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	info = &ctx->ctx_cq.semp_info;
	if (unlikely(!ctx_cq->semp_res_flag || info->semp_kva == 0 || info->semp_uva == 0 ||
		info->size == 0)) {
		spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
		return;
	}

	cur_value = *((volatile uint64_t *)u64_to_ptr(sema_kaddr));
	if (cur_value != 0)
		hvgr_err(ctx->gdev, HVGR_CQ,
			"error %s, cur_value is %llu, set value %llu, sem_addr 0x%llx\n",
			__func__, cur_value, sema_value, sema_uaddr);
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
	return;
}

bool hvgr_semaphore_is_set(struct hvgr_ctx * const ctx, uint64_t sema_addr, uint64_t sema_value)
{
	volatile uint64_t *addr = NULL;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	bool ret = false;
	struct hvgr_cq_semp_info *info = NULL;

	if (unlikely((ctx == NULL) || (sema_addr == 0)))
		return false;

	ctx_cq = &ctx->ctx_cq;
	addr = (volatile uint64_t *)u64_to_ptr(sema_addr);

	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	info = &ctx->ctx_cq.semp_info;
	if (unlikely(!ctx_cq->semp_res_flag || info->semp_kva == 0 || info->semp_uva == 0 ||
		info->size == 0)) {
		spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
		return false;
	}

	ret = (*addr == sema_value);
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
	return ret;
}

void hvgr_semaphore_set(struct hvgr_ctx * const ctx, uint64_t sema_addr, uint64_t sema_value)
{
	volatile uint64_t *addr = NULL;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;
	struct hvgr_cq_semp_info *info = NULL;

	if (unlikely((ctx == NULL) || (sema_addr == 0)))
		return;

	ctx_cq = &ctx->ctx_cq;
	addr = (volatile uint64_t *)u64_to_ptr(sema_addr);

	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	info = &ctx->ctx_cq.semp_info;
	if (unlikely(!ctx_cq->semp_res_flag || info->semp_kva == 0 || info->semp_uva == 0 ||
		info->size == 0)) {
		spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
		return;
	}

	*addr = sema_value;
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
}

static long hvgr_cq_alloc_semp_memory(struct hvgr_ctx * const ctx, uint32_t size)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alloc_para para;

	size = (uint32_t)hvgr_mem_page_align(size);
	para.property = CQ_SEM_MEM_PROPERTY;
	para.attribute = HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR |
		HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR |
		HVGR_MEM_ATTR_KMD_ACCESS;
	para.pages = (uint32_t)hvgr_page_nums(size);
	para.max_pages = para.pages;
	area = hvgr_mem_allocate(ctx, &para, NULL, false);
	if (!area) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s, alloc semp fail %d.\n", __func__, __LINE__);
		return -EFAULT;
	}

	ctx->ctx_cq.semp_info.semp_buffer = area;
	ctx->ctx_cq.semp_info.semp_kva = area->kva;
	ctx->ctx_cq.semp_info.semp_uva = area->uva;
	ctx->ctx_cq.semp_info.size = size;

	return 0;
}

long hvgr_ioctl_alloc_semp(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_alloc_semp * const para)
{
	long ret;
	uint32_t size;
	unsigned long flags;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;

	if ((ctx == NULL) || (para == NULL))
		return -1;

	if (unlikely(para->in.size != CQ_SEMAPHORE_PAGE_SIZE_L1 &&
		para->in.size != CQ_SEMAPHORE_PAGE_SIZE_L2)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s, alloc size is invalid.\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&ctx->ctx_lock);
	ctx_cq = &ctx->ctx_cq;
	size = para->in.size;

	if(ctx_cq->semp_res_flag == true) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s, semp has already been alloced.\n", __func__);
		mutex_unlock(&ctx->ctx_lock);
		return 0;
	}

	ret = hvgr_cq_alloc_semp_memory(ctx, size);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s, alloc cq semp mem fail(%d).\n",
			__func__, ret);
		mutex_unlock(&ctx->ctx_lock);
		return -EFAULT;
	}

	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	ctx_cq->semp_res_flag = true;
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);

	para->out.semaphore_addr = ctx->ctx_cq.semp_info.semp_uva;

	mutex_unlock(&ctx->ctx_lock);
	return 0;
}

long hvgr_ioctl_free_semp(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_free_semp * const para)
{
	return 0;
}

void hvgr_msync_term(struct hvgr_ctx * const ctx)
{
	unsigned long flags;
	long ret;
	struct hvgr_cq_ctx_data *ctx_cq = NULL;

	if (unlikely(ctx == NULL))
		return;

	ctx_cq = &ctx->ctx_cq;
	spin_lock_irqsave(&ctx_cq->semp_lock, flags);
	ctx_cq->semp_res_flag = false;
	spin_unlock_irqrestore(&ctx_cq->semp_lock, flags);
	if (ctx->ctx_cq.semp_info.semp_buffer != NULL) {
		ret = hvgr_mem_free(ctx, ctx->ctx_cq.semp_info.semp_buffer->gva);
		if (ret != 0) {
			hvgr_err(ctx->gdev, HVGR_CQ, "%s hvgr_mem_free failed, ret=%d\n",
				__func__, ret);
			return;
		}
		ctx->ctx_cq.semp_info.semp_buffer = NULL;
	}
}

void hvgr_msync_event_wakeup(struct hvgr_ctx * const ctx)
{
	wake_up_interruptible(&ctx->softq_wait);
#ifdef event_write
	event_write(ctx->kfile);
#endif
}

bool hvgr_msync_event_checkdata(struct hvgr_ctx * const ctx)
{
	struct hvgr_cq_ctx_data *ctx_cq = &ctx->ctx_cq;
	unsigned long flags;
	bool result = false;

	spin_lock_irqsave(&ctx_cq->event_lock, flags);
	result = ctx_cq->token_finish_wr != ctx_cq->token_finish_rd;
	spin_unlock_irqrestore(&ctx_cq->event_lock, flags);

	return result;
}

void hvgr_msync_event_enqueue(struct hvgr_ctx * const ctx, struct msync_event *event)
{
	uint32_t wr;
	struct hvgr_cq_ctx_data *ctx_cq = &ctx->ctx_cq;
	unsigned long flags;

	hvgr_info(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%u, enqueue",
		__func__, ctx->id, event->data);
	hvgr_assert((event->status != 0) && (event->data != 0));

	if (((event->data < CQ_TOKEN_NUMS) && (event->data != 0)) ||
		(event->data == CQ_ERR_FLOW_TOKEN_VALUE)) {
		spin_lock_irqsave(&ctx_cq->event_lock, flags);
		wr = ctx_cq->token_finish_wr;
		if (unlikely(((wr + 1) & (CQ_TOKEN_NUMS - 1)) == ctx_cq->token_finish_rd)) {
			hvgr_err(ctx->gdev, HVGR_CQ, "ctx_%u token_notify_nums full", ctx->id);
			spin_unlock_irqrestore(&ctx_cq->event_lock, flags);
			return;
		}
		(void)memcpy_s(&ctx_cq->token_finish[wr], sizeof(*event), event, sizeof(*event));
		ctx_cq->token_finish_wr = (wr + 1) & (CQ_TOKEN_NUMS - 1);
		spin_unlock_irqrestore(&ctx_cq->event_lock, flags);
	} else {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s ctx_%u token=%u, status=%u",
			__func__, ctx->id, event->data, (unsigned int)event->status);
	}
}

uint32_t hvgr_token_dequeue(struct hvgr_ctx * const ctx, char __user *buf, uint32_t num)
{
	unsigned long flags;
	uint32_t len = 0;
	uint32_t real_num;
	uint32_t read_num;
	uint32_t rd, wr;
	struct hvgr_cq_ctx_data *ctx_cq = &ctx->ctx_cq;
	void *src = NULL;
	uint32_t cpy_len;

	mutex_lock(&ctx_cq->event_data_lock);
	spin_lock_irqsave(&ctx_cq->event_lock, flags);
	rd = ctx_cq->token_finish_rd;
	wr = ctx_cq->token_finish_wr;
	spin_unlock_irqrestore(&ctx_cq->event_lock, flags);

	if (unlikely((rd >= CQ_TOKEN_NUMS) || (wr >= CQ_TOKEN_NUMS)))
		goto exit;

	src = &ctx_cq->token_finish[rd];
	if (rd == wr)
		goto exit;

	if (rd < wr) {
		real_num = wr - rd;
		read_num = min(real_num, num);
		if (read_num != 0) {
			cpy_len = read_num * (uint32_t)sizeof(struct msync_event);
			if (copy_to_user(buf, src, cpy_len) != 0)
				goto exit;
		} else {
			goto exit;
		}

		rd = (rd + read_num) & (CQ_TOKEN_NUMS - 1);
		len = read_num & (CQ_TOKEN_NUMS - 1);
	} else {
		real_num = wr + CQ_TOKEN_NUMS - rd;
		read_num = min(real_num, num);
		if (read_num > CQ_TOKEN_NUMS - rd) {
			cpy_len = (CQ_TOKEN_NUMS - rd) * (uint32_t)sizeof(struct msync_event);
			if (copy_to_user(buf, src, cpy_len) != 0)
				goto exit;
			len = CQ_TOKEN_NUMS - rd;

			buf += len * (uint32_t)sizeof(struct msync_event);
			src = &ctx_cq->token_finish[0];
			read_num = read_num - len;
			cpy_len = read_num * (uint32_t)sizeof(struct msync_event);
			if (copy_to_user(buf, src, cpy_len) != 0)
				goto exit;
			rd = read_num & (CQ_TOKEN_NUMS - 1);
			len += read_num;
		} else {
			cpy_len = read_num * (uint32_t)sizeof(struct msync_event);
			if (copy_to_user(buf, src, cpy_len) != 0)
				goto exit;
			rd = (rd + read_num) & (CQ_TOKEN_NUMS - 1);
			len = read_num & (CQ_TOKEN_NUMS - 1);
		}
	}

	spin_lock_irqsave(&ctx_cq->event_lock, flags);
	ctx_cq->token_finish_rd = rd;

	if (ctx_cq->token_finish_rd != ctx_cq->token_finish_wr)
		hvgr_msync_event_wakeup(ctx);
#ifdef CONFIG_LIBLINUX_HVGR_POLL_OPT
	else if (!(atomic_read(&ctx->softq_closed) != 0))
		liblinux_pal_sync_wakeup(ctx->kfile->liblinux_private, 0);
#endif
	spin_unlock_irqrestore(&ctx_cq->event_lock, flags);
exit:
	mutex_unlock(&ctx_cq->event_data_lock);
	if (unlikely(len > num))
		hvgr_err(ctx->gdev, HVGR_CQ, "%s len=%u, num=%u", __func__, len, num);

	len *= (uint32_t)sizeof(struct msync_event);
	return len;
}

uint32_t hvgr_msync_get_event_read_num(struct hvgr_ctx * const ctx, uint32_t count)
{
	uint32_t event_size = (uint32_t)sizeof(struct msync_event);
	uint32_t num = 0;

	if (unlikely(count < event_size || (count % event_size) != 0)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s count=%u:%u is invalid",
			__func__, count, event_size);
		return num;
	}

	num = count / event_size;
	if (unlikely(num > MSYNC_TRANS_MAX)) {
		hvgr_err(ctx->gdev, HVGR_CQ, "%s read event num[%u] reach limit.", __func__, num);
		num = MSYNC_TRANS_MAX;
	}

	return num;
}
