/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_wq.h"

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <securec.h>
#include <linux/mutex.h>

#include "hvgr_defs.h"
#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_mem_api.h"
#include "hvgr_mem_zone.h"
#include "mem/hvgr_mem_sparse.h"
#include "hvgr_msync.h"
#include "hvgr_log_api.h"
#include "hvgr_softjob.h"
#include "hvgr_log_api.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_datan_api.h"

#define JIT_INIT_PAGES 64u
#define JIT_EXTENT_PAGES 32u

static void hvgr_wq_workqueue_handle(struct work_struct *data);

typedef enum {
	WQ_RET_PASS = 0,
	WQ_RET_TIMEOUT = 1,
	WQ_RET_FAIL = 2,
	WQ_RET_ERROR = 3,
	WQ_RET_CALLBACK = 4,
	WQ_RET_SUSPEND = 5,
	WQ_RET_CANCEL = 6,
} hvgr_wq_opt_ret;

static hvgr_wq_opt_ret hvgr_wq_barrier_max_wait(struct hvgr_cq_ctx * const cq_ctx);

void __attribute__((weak)) hvgr_wq_init_jit_para(struct hvgr_mem_alloc_para* jit_para,
	uint64_t size)
{
	jit_para->property = HVGR_MEM_PROPERTY_JIT;
	jit_para->attribute = HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_GROWABLE;
	jit_para->pages = JIT_INIT_PAGES;
	jit_para->extent = JIT_EXTENT_PAGES;
	jit_para->max_pages = (uint32_t)hvgr_page_nums(size);
}

static struct hvgr_mem_area *hvgr_wq_find_jit_area(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alloc_para *para)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_area *iter = NULL;
	struct hvgr_mem_area *temp = NULL;

	list_for_each_entry_safe(iter, temp, &ctx->mem_ctx.jit_pool_head, jit_node) {
		if (iter->max_pages == para->max_pages) {
			area = iter;
			break;
		}
	}

	return area;
}

static int hvgr_wq_jit_area_add_pages(struct hvgr_ctx * const ctx, struct hvgr_mem_alloc_para *para,
	struct hvgr_mem_area *area)
{
	mutex_lock(&ctx->mem_ctx.area_mutex);

	if (area->pages >= para->pages)
		goto unlock_out;

	if (hvgr_mem_resize_expand(ctx, area, para->pages) != 0) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -1;
	}

unlock_out:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return 0;
}

struct hvgr_mem_area * __attribute__((weak)) hvgr_wq_get_jit_area(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alloc_para* jit_para)
{
	struct hvgr_mem_area *area = NULL;
	int ret;

	mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);

	area = hvgr_wq_find_jit_area(ctx, jit_para);
	if (area != NULL) {
		list_move(&area->jit_node, &ctx->mem_ctx.jit_using_head);
		mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);

		ret = hvgr_wq_jit_area_add_pages(ctx, jit_para, area);
		if (ret != 0) {
			mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);
			list_move(&area->jit_node, &ctx->mem_ctx.jit_pool_head);
			mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);
			return NULL;
		}
	} else {
		mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);
		area = hvgr_mem_allocate(ctx, jit_para, NULL, false);
		if (area == NULL)
			return NULL;

		mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);
		list_move(&area->jit_node, &ctx->mem_ctx.jit_using_head);
		mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);
	}

	return area;
}

long __attribute__((weak)) hvgr_wq_free_jit_memory(struct hvgr_ctx * const ctx, uint64_t gva)
{
	struct hvgr_mem_area *area = NULL;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, gva);
	if (area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}
	if (area->property != HVGR_MEM_PROPERTY_JIT) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return 0;
	}
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	mutex_lock(&ctx->mem_ctx.jit_shrink_mutex);
	list_move(&area->jit_node, &ctx->mem_ctx.jit_pool_head);
	mutex_unlock(&ctx->mem_ctx.jit_shrink_mutex);

	return 0;
}

static uint64_t hvgr_wq_alloc_jit_memory(struct hvgr_ctx * const ctx, uint64_t size)
{
	dma_addr_t dma_addr;
	void *jitaddr = NULL;
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_alloc_para para = {0};

	hvgr_wq_init_jit_para(&para, size);

	area = hvgr_wq_get_jit_area(ctx, &para);
	if (area == NULL)
		return 0;

	jitaddr = kmap_atomic(area->page_array[0]);
	if (unlikely(jitaddr == NULL)) {
		(void)hvgr_wq_free_jit_memory(ctx, area->gva);
		return 0;
	}

	*((volatile uint64_t *)jitaddr) = area->gva + 128;
	kunmap_atomic(jitaddr);

	dma_addr = hvgr_mem_get_page_dma_addr(area->page_array[0]);
	dma_sync_single_for_device(ctx->gdev->dev, dma_addr, PAGE_SIZE, DMA_TO_DEVICE);

	return area->gva;
}

typedef hvgr_wq_opt_ret(*hvgr_wq_opts)(struct hvgr_cq_ctx * const, wq_work_opt const * const);

typedef hvgr_wq_opt_ret(*hvgr_wq_semp_wait)(struct hvgr_cq_ctx * const,
	uint32_t, wq_work_entry * const);

typedef hvgr_wq_opt_ret(*hvgr_wq_free_jit)(struct hvgr_cq_ctx * const,
	wq_work_opt const * const, uint64_t *, int);

/* do nothing. */
void __attribute__((weak)) datan_jobtrace_set_value(uint64_t *addr, uint64_t val)
{
	return;
}

static hvgr_wq_opt_ret hvgr_wq_opt_alloc_jit_memory_helper(struct hvgr_cq_ctx * const
	cq_ctx, uint64_t mem_addr_pointer, uint64_t mem_request_size, uint64_t *jitaddr_ret)
{
	uint64_t jit_addr_for_usr;
	uint64_t jitaddr;
	long ret;

	jitaddr = hvgr_wq_alloc_jit_memory(cq_ctx->ctx, mem_request_size);
	if (jitaddr == 0)
		return WQ_RET_FAIL;

	jit_addr_for_usr = jitaddr + 128;
	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s jit mem=0x%llx, mem_addr_pointer=0x%llx",
		__func__, jit_addr_for_usr, mem_addr_pointer);
	ret = hvgr_mem_rw(cq_ctx->ctx, mem_addr_pointer, &jit_addr_for_usr, true);
	if (ret != 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s write jit mem fail, ret=%d",
			__func__, ret);
		(void)hvgr_wq_free_jit_memory(cq_ctx->ctx, jitaddr);
		return WQ_RET_FAIL;
	}

	*jitaddr_ret = jitaddr;
	datan_jobtrace_set_value((uint64_t *)u64_to_ptr(mem_addr_pointer), jitaddr + 128);

	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_JD_DUMP_PT, cq_ctx->ctx);

	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_JD_UPDATE_JIT_MEMORY,
		cq_ctx->ctx, jitaddr, jitaddr, mem_request_size, cq_ctx->queue_id);

	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		jitaddr,  jitaddr + 128, "jit", cq_ctx->queue_id);

	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		mem_addr_pointer, jitaddr + 128, "jit", cq_ctx->queue_id);

	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_alloc_jit_memory(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_jit_memory const * const para = &opt->paras.jit_memory;
	hvgr_wq_opt_ret ret;
	uint32_t idx, alloc_idx = 0;
	uint64_t jit_addr[WQ_JIT_NUMS];
	uint32_t num = para->num;

	if (num > WQ_JIT_NUMS) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s error, invalid para num %u",
			__func__, num);
		return WQ_RET_FAIL;
	}

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s alloc jit mem entrys %u", __func__, num);
	for (idx = 0; idx < num; idx++) {
		ret = hvgr_wq_opt_alloc_jit_memory_helper(cq_ctx, para->mem_addr_pointer[idx],
			(uint64_t)para->mem_request_size[idx], &jit_addr[idx]);
		if (ret != WQ_RET_PASS) {
			alloc_idx = idx;
			goto alloc_jit_fail;
		}
	}

	return WQ_RET_PASS;

alloc_jit_fail:
	for (idx = 0; idx < alloc_idx; idx++)
		(void)hvgr_wq_free_jit_memory(cq_ctx->ctx, jit_addr[idx]);
	hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s jit alloc mem fail, num %u",
		__func__, num);

	return WQ_RET_FAIL;
}

bool hvgr_wq_opt_get_jit_addr(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_entry * const wq_entry, uint64_t *jit_addrs, int jit_num)
{
	wq_opt_para_jit_memory const *para = NULL;
	uint32_t run_counter = wq_entry->run_counter;
	uint32_t jit_idx = 0;
	uint32_t entry_idx, j;
	uint32_t num;
	long ret;

	for (entry_idx = run_counter; entry_idx < ARRAY_SIZE(wq_entry->opts); entry_idx++) {
		wq_work_opt const *opt = &wq_entry->opts[entry_idx];

		if (opt->cmd != WQ_OPT_FREE_JIT_MEMORY)
			continue;

		para = &opt->paras.jit_memory;
		num = para->num;
		hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s num %u\n", __func__, num);

		if (unlikely(num > WQ_JIT_NUMS)) {
			hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s error, invalid para num %u",
				__func__, num);
			continue;
		}

		for (j = 0; j < num; j++) {
			if (jit_idx >= (uint32_t)jit_num) {
				hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ,
					"hvgr %s jit index overflow jit_idx %u", __func__, jit_idx);
				return false;
			}

			ret = hvgr_mem_rw(cq_ctx->ctx, para->mem_addr_pointer[j],
				&jit_addrs[jit_idx], false);
			if (ret != 0) {
				hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ,
					"hvgr %s get jit addr fail, ret %d", __func__, ret);
				return false;
			}
			jit_idx++;
		}
	}

	return true;
}

static hvgr_wq_opt_ret hvgr_wq_opt_free_jit_memory_direct(struct hvgr_cq_ctx * const
	cq_ctx, wq_work_opt const * const opt, uint64_t *jit_addrs, int jit_num)
{
	wq_opt_para_jit_memory const * const para = &opt->paras.jit_memory;
	uint64_t jitaddr;
	uint32_t idx;
	uint32_t num = para->num;
	long ret;

	if (num > (uint32_t)jit_num) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ,
			"hvgr %s error, invalid para num %u, jit_num %d",
			__func__, num, jit_num);
		return WQ_RET_FAIL;
	}

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s entrys %u", __func__, num);
	for (idx = 0; idx < num; idx++) {
		jitaddr = jit_addrs[idx];
		hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s jit mem 0x%llx", __func__, jitaddr);

		jitaddr -= 128;
		ret = hvgr_wq_free_jit_memory(cq_ctx->ctx, jitaddr);
		if (ret != 0) {
			hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s hvgr_mem_free failed, ret=%d\n",
				__func__, ret);
			return WQ_RET_FAIL;
		}
	}

	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_free_jit_memory_helper(struct hvgr_cq_ctx * const
	cq_ctx, uint64_t mem_addr_pointer)
{
	long ret;
	long res;
	uint64_t jitaddr = 0;

	ret = hvgr_mem_rw(cq_ctx->ctx, mem_addr_pointer, &jitaddr, false);
	if (ret != 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s read jit mem fail, ret %d",
			__func__, ret);
		return WQ_RET_FAIL;
	}

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s jit mem 0x%llx", __func__, jitaddr);
	jitaddr -= 128;
	res = hvgr_wq_free_jit_memory(cq_ctx->ctx, jitaddr);
	if (res != 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s hvgr_mem_free failed, ret=%d\n",
			__func__, ret);
		return WQ_RET_FAIL;
	}
	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_free_jit_memory(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt, uint64_t *jit_addrs, int jit_num)
{
	wq_opt_para_jit_memory const * const para = &opt->paras.jit_memory;
	uint32_t idx;
	uint32_t num = para->num;

	if (num > WQ_JIT_NUMS) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "hvgr %s error, invalid para num %u",
			__func__, num);
		return WQ_RET_FAIL;
	}

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s free jit mem entrys %u", __func__, num);

	/* from workqueue token_callback_wq, has jit_addrs */
	if (jit_num > 0)
		return hvgr_wq_opt_free_jit_memory_direct(cq_ctx, opt, jit_addrs, jit_num);

	/* from workqueue hvgr_wq_kwq */
	for (idx = 0; idx < num; idx++)
		(void)hvgr_wq_opt_free_jit_memory_helper(cq_ctx, para->mem_addr_pointer[idx]);

	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_bind_sparse_memory(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_sparse_mem const * const para = &opt->paras.sparse_memory;
	uint64_t sparse_addr = para->sparse_addr;
	uint64_t mem_addr = para->mem_addr;
	uint64_t pages = para->pages;

	if (sparse_addr == 0 || mem_addr == 0 || pages == 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ,
			"sparse bind input error, 0x%llx, 0x%llx, 0x%llx\n",
			sparse_addr, mem_addr, pages);
		return WQ_RET_FAIL;
	}

	if (hvgr_mem_sparse_bind(cq_ctx->ctx, sparse_addr, mem_addr, pages) != 0)
		return WQ_RET_FAIL;

	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_unbind_sparse_memory(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_sparse_mem const * const para = &opt->paras.sparse_memory;
	uint64_t sparse_addr = para->sparse_addr;
	uint64_t pages = para->pages;

	if (sparse_addr == 0 || pages == 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "sparse unbind input error, 0x%llx, 0x%llx\n",
			sparse_addr, pages);
		return WQ_RET_FAIL;
	}

	if (hvgr_mem_sparse_unbind(cq_ctx->ctx, sparse_addr, pages) != 0)
		return WQ_RET_FAIL;

	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_set_semaphore(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_semaphore const * const para = &opt->paras.semaphore;
	uint64_t sem_addr;

	sem_addr = hvgr_semaphore_get_addr(cq_ctx->ctx, para->sem_addr);
	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s Semaddr %llx, set to 0x%llx",
		__func__, para->sem_addr, para->sem_value);

	hvgr_semaphore_print_value(cq_ctx->ctx, sem_addr, para->sem_addr, para->sem_value);

	hvgr_semaphore_set(cq_ctx->ctx, sem_addr, para->sem_value);
	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		para->sem_addr, para->sem_value, "semphore", cq_ctx->queue_id);
	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_set_semaphore_and_notify(struct hvgr_cq_ctx * const
	cq_ctx, wq_work_opt const * const opt)
{
	wq_opt_para_semaphore const * const para = &opt->paras.semaphore;
	uint32_t token = para->token;

	if (unlikely(cq_ctx == NULL))
		return WQ_RET_FAIL;

	if (!hvgr_token_is_valid(cq_ctx->ctx, token))
		return WQ_RET_FAIL;

	(void)hvgr_wq_opt_set_semaphore(cq_ctx, opt);

	/* Wakeup */
	hvgr_token_finish(cq_ctx, token);
	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_reset_semaphore(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_semaphore const * const para = &opt->paras.semaphore;
	uint64_t sem_addr;

	sem_addr = hvgr_semaphore_get_addr(cq_ctx->ctx, para->sem_addr);
	hvgr_semaphore_set(cq_ctx->ctx, sem_addr, 0);

	hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s Semaddr %llx set to 0x%llx",
		__func__, para->sem_addr, 0);
	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		para->sem_addr, 0, "semphore", cq_ctx->queue_id);
	return WQ_RET_PASS;
}

static bool hvgr_wq_has_wait_semaphore(struct hvgr_cq_ctx * const cq_ctx)
{
	if (unlikely(hvgr_cq_is_queue_error(cq_ctx)))
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s ctx_%u queue err",
			__func__, cq_ctx->ctx->id);
	if (atomic_read(&cq_ctx->wq_hold_count) == 0 ||
		cq_ctx->status != CQ_STATUS_RUNNING ||
		hvgr_cq_is_queue_error(cq_ctx))
		return true;

	return false;
}

static hvgr_wq_opt_ret hvgr_wq_opt_barrier(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	struct hvgr_device *gdev = cq_ctx->ctx->gdev;
	long timeout = (long)msecs_to_jiffies(WQ_EVENT_WAIT_TIMEOUT);

	hvgr_info(gdev, HVGR_CQ, "ctx_%u WQ do barrier wait = %d",
		cq_ctx->ctx->id, (int)atomic_read(&cq_ctx->wq_hold_count));
	if (cq_ctx->status != CQ_STATUS_RUNNING &&
		atomic_read(&cq_ctx->wq_hold_count) != 0)
		return WQ_RET_SUSPEND;

	if (wait_event_timeout(gdev->wq_dev.barrier_wait_wq,
		hvgr_wq_has_wait_semaphore(cq_ctx), timeout) == 0) {
		hvgr_info(gdev, HVGR_CQ, "ctx_%u WQ barrier wait timeout, count=%d",
			cq_ctx->ctx->id, (int)atomic_read(&cq_ctx->wq_hold_count));
		return WQ_RET_SUSPEND;
	}

	if (atomic_read(&cq_ctx->wq_hold_count) == 0) {
		hvgr_info(gdev, HVGR_CQ, "ctx_%u WQ barrier done", cq_ctx->ctx->id);
		return WQ_RET_PASS;
	}
	hvgr_info(gdev, HVGR_CQ, "ctx_%u WQ barrier suspend wait = %d",
		cq_ctx->ctx->id, (int)atomic_read(&cq_ctx->wq_hold_count));

	return WQ_RET_SUSPEND;
}

static hvgr_wq_opt_ret hvgr_wq_opt_set_atomic_semaphore(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	wq_opt_para_atomic_semaphore const * const para = &opt->paras.atomic_semp;
	uint64_t sem_addr;

	sem_addr = hvgr_semaphore_get_addr(ctx, para->sem_addr);
	hvgr_info(ctx->gdev, HVGR_CQ, "%s Semaddr %llx, set to 0x%llx",
		__func__, para->sem_addr, para->sem_value);
	hvgr_semaphore_set(ctx, sem_addr, para->sem_value);
	hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		para->sem_addr, para->sem_value, "semphore", cq_ctx->queue_id);
	wake_up(&ctx->gdev->wq_dev.barrier_wait_wq);
	return WQ_RET_PASS;
}

static hvgr_wq_opt_ret hvgr_wq_opt_set_as_notify(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	wq_opt_para_atomic_semaphore const * const para = &opt->paras.atomic_semp;
	uint32_t token;

	token = para->token;
	if (!hvgr_token_is_valid(cq_ctx->ctx, token))
		return WQ_RET_PASS;

	(void)hvgr_wq_opt_set_atomic_semaphore(cq_ctx, opt);
	/* Wakeup */
	hvgr_token_finish(cq_ctx, token);
	return WQ_RET_PASS;
}

static bool hvgr_wq_is_set_as(struct hvgr_cq_ctx * const cq_ctx,
	uint64_t as_addr, uint64_t as_value)
{
	if (hvgr_semaphore_is_set(cq_ctx->ctx, as_addr, as_value) ||
		cq_ctx->status != CQ_STATUS_RUNNING ||
		hvgr_cq_is_queue_error(cq_ctx))
		return true;

	return false;
}

static hvgr_wq_opt_ret hvgr_wq_opt_wait_atomic_semaphore(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_opt const * const opt)
{
	struct hvgr_ctx *ctx = cq_ctx->ctx;
	wq_opt_para_atomic_semaphore const * const para = &opt->paras.atomic_semp;
	uint64_t sem_addr;
	uint32_t token = para->token;
	long timeout = (long)msecs_to_jiffies(WQ_EVENT_WAIT_TIMEOUT);

	if (!hvgr_token_is_valid(ctx, token))
		return WQ_RET_FAIL;

	sem_addr = hvgr_semaphore_get_addr(ctx, para->sem_addr);
	hvgr_info(ctx->gdev, HVGR_CQ, "%s Semaddr %llx wait 0x%llx",
		__func__, para->sem_addr, para->wait_value);

	mutex_lock(&ctx->ctx_cq.as_lock[token]);
	if (hvgr_semaphore_is_set(cq_ctx->ctx, sem_addr, para->wait_value)) {
		hvgr_semaphore_set(ctx, sem_addr, para->sem_value);
		hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		para->sem_addr, para->sem_value, "semphore", cq_ctx->queue_id);

		mutex_unlock(&ctx->ctx_cq.as_lock[token]);
		hvgr_info(ctx->gdev, HVGR_CQ, "Atomic semphore wait done");
		return WQ_RET_PASS;
	}
	if (wait_event_timeout(ctx->gdev->wq_dev.barrier_wait_wq,
		hvgr_wq_is_set_as(cq_ctx, sem_addr, para->wait_value),
		timeout) == 0) {
		hvgr_err(ctx->gdev, HVGR_CQ, "Atomic semphore wait timeout");
		goto out;
	}

	if (hvgr_semaphore_is_set(cq_ctx->ctx, sem_addr, para->wait_value)) {
		hvgr_semaphore_set(ctx, sem_addr, para->sem_value);
		hvgr_datan_jobtrace(cq_ctx->ctx, JOBTRACE_TYPE_OBJ, KDATAN_SET_PTR, cq_ctx->ctx,
		para->sem_addr, para->sem_value, "semphore", cq_ctx->queue_id);
		mutex_unlock(&ctx->ctx_cq.as_lock[token]);
		hvgr_info(ctx->gdev, HVGR_CQ, "Atomic semphore wait done");
		return WQ_RET_PASS;
	}
out:
	mutex_unlock(&ctx->ctx_cq.as_lock[token]);
	return WQ_RET_SUSPEND;
}

static hvgr_wq_opt_ret hvgr_wq_opt_wait_semaphore_new(struct hvgr_cq_ctx * const cq_ctx,
	uint32_t opt_count, wq_work_entry * const wq_entry)
{
	hvgr_wq_opt_ret ret;
	wq_work_opt const *opt = &wq_entry->opts[opt_count];
	wq_opt_para_semaphore const * const para = &opt->paras.semaphore;
	struct hvgr_msync msync;

	ret = hvgr_wq_barrier_max_wait(cq_ctx);
	if (ret != WQ_RET_PASS)
		return ret;

	msync.sem_addr = hvgr_semaphore_get_addr(cq_ctx->ctx, para->sem_addr);
	msync.sem_value = para->sem_value;
	msync.token = para->token;

	if (unlikely(msync.sem_addr == 0)) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s get semaphore addr fail", __func__);
		return WQ_RET_ERROR;
	}

	wq_entry->run_counter = ++opt_count;
	if (!hvgr_token_register_wait_semaphore(cq_ctx, &msync, wq_entry)) {
		hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "%s register wait semaphore fail",
			__func__);
		return WQ_RET_PASS;
	}

	return WQ_RET_CALLBACK;
}

typedef struct {
	hvgr_wq_opts opts[WQ_OPT_END];
	hvgr_wq_semp_wait wait;
	hvgr_wq_free_jit free_jit;
} hvgr_wq_opt_tables;

static const hvgr_wq_opt_tables g_hvgr_wq_opts = {
	.opts[WQ_OPT_ALLOC_JIT_MEMORY] = hvgr_wq_opt_alloc_jit_memory,
	.opts[WQ_OPT_FREE_JIT_MEMORY] = NULL,
	.opts[WQ_OPT_BIND_SPARSE_MEMORY] = hvgr_wq_opt_bind_sparse_memory,
	.opts[WQ_OPT_UNBIND_SPARSE_MEMORY] = hvgr_wq_opt_unbind_sparse_memory,
	.opts[WQ_OPT_WAIT_SEMAPHORE] = NULL,
	.opts[WQ_OPT_SET_SEMAPHORE] = hvgr_wq_opt_set_semaphore,
	.opts[WQ_OPT_SET_SEMAPHORE_AND_NOTIFY] = hvgr_wq_opt_set_semaphore_and_notify,
	.opts[WQ_OPT_RESET_SEMAPHORE] = hvgr_wq_opt_reset_semaphore,
	.opts[WQ_OPT_BARRIER] = hvgr_wq_opt_barrier,
	.opts[WQ_OPT_SET_ATOMIC_SEMAPHORE] = hvgr_wq_opt_set_atomic_semaphore,
	.opts[WQ_OPT_SET_ATOMIC_SEMAPHORE_AND_NOTIFY] = hvgr_wq_opt_set_as_notify,
	.opts[WQ_OPT_WAIT_ATOMIC_SEMAPHORE] = hvgr_wq_opt_wait_atomic_semaphore,
	.wait = hvgr_wq_opt_wait_semaphore_new,
	.free_jit = hvgr_wq_opt_free_jit_memory,
};

static hvgr_wq_opt_ret hvgr_wq_do_opts(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_entry * const wq_entry, uint64_t *jit_addrs, int jit_num)
{
	uint32_t idx;
	hvgr_wq_opt_ret ret = WQ_RET_PASS;

	for (idx = wq_entry->run_counter; idx < ARRAY_SIZE(wq_entry->opts); idx++) {
		wq_work_opt const *opt = &wq_entry->opts[idx];
		wq_opt_cmd cmd = opt->cmd;
		if (cmd >= WQ_OPT_END)
			continue;

		if (cmd == WQ_OPT_WAIT_SEMAPHORE) {
			ret = g_hvgr_wq_opts.wait(cq_ctx, idx, wq_entry);
		} else if (cmd == WQ_OPT_FREE_JIT_MEMORY) {
			ret = g_hvgr_wq_opts.free_jit(cq_ctx, opt, jit_addrs, jit_num);
		} else {
			if (g_hvgr_wq_opts.opts[cmd] == NULL)
				continue;
			ret = g_hvgr_wq_opts.opts[cmd](cq_ctx, opt);
		}

		if (ret != WQ_RET_PASS)
			break;
	}
	/* Here is a invalid WQ order. drop entry */
	if (ret == WQ_RET_FAIL || ret == WQ_RET_ERROR)
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "WQ running FAIL");
	return ret;
}

void hvgr_wq_do_entry(struct hvgr_cq_ctx * const cq_ctx, wq_work_entry * const entry,
	uint64_t *jit_addrs, int jit_num)
{
	(void)hvgr_wq_do_opts(cq_ctx, entry, jit_addrs, jit_num);
}

static bool hvgr_wq_has_wait_max(struct hvgr_cq_ctx * const cq_ctx)
{
	struct hvgr_device *gdev = cq_ctx->ctx->gdev;

	if (unlikely(hvgr_cq_is_queue_error(cq_ctx)))
		hvgr_err(gdev, HVGR_CQ, "%s ctx_%u queue err",
			__func__, cq_ctx->ctx->id);
	if (atomic_read(&cq_ctx->wq_hold_count) < (int)cq_ctx->ctx->wq_wait_max_cnt ||
		cq_ctx->status != CQ_STATUS_RUNNING ||
		hvgr_cq_is_queue_error(cq_ctx))
		return true;

	return false;
}

static hvgr_wq_opt_ret hvgr_wq_barrier_max_wait(struct hvgr_cq_ctx * const cq_ctx)
{
	long timeout = (long)msecs_to_jiffies(WQ_EVENT_WAIT_TIMEOUT);
	if (wait_event_timeout(cq_ctx->ctx->gdev->wq_dev.barrier_wait_wq,
		hvgr_wq_has_wait_max(cq_ctx), timeout) == 0) {
		hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "ctx_%u barrier max wait timeout, count=%d",
			cq_ctx->ctx->id, atomic_read(&cq_ctx->wq_hold_count));
		return WQ_RET_SUSPEND;
	}

	if (atomic_read(&cq_ctx->wq_hold_count) < (int)cq_ctx->ctx->wq_wait_max_cnt)
		return WQ_RET_PASS;

	return WQ_RET_SUSPEND;
}

static void hvgr_wq_worker(struct hvgr_cq_ctx * const cq_ctx)
{
	hvgr_wq_opt_ret ret;
	wq_work_entry *entry = NULL;
	unsigned long flags;

	while (!hvgr_ctx_flag(cq_ctx->ctx, HVGR_CTX_FLAG_DYING)) {
		if (unlikely(hvgr_cq_is_queue_error(cq_ctx))) {
			hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "%s ctx_%u, qid=%u, fault, wq quit.",
				__func__, cq_ctx->ctx->id, cq_ctx->queue_id);
			spin_lock_irqsave(&cq_ctx->wq_kwq_lock, flags);
			cq_ctx->wq_kwq_is_running = false;
			spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
			return;
		}

		entry = (wq_work_entry *)hvgr_softq_get_entry(cq_ctx->sq_ctx);
		if (entry == NULL) {
			spin_lock_irqsave(&cq_ctx->wq_kwq_lock, flags);
			if (cq_ctx->wq_notify_count != 0) {
				cq_ctx->wq_notify_count = 0;
				spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
				continue;
			}
			cq_ctx->wq_kwq_is_running = false;
			spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
			hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "WQ empty");
			break;
		}

		ret = hvgr_wq_do_opts(cq_ctx, entry, NULL, 0);
		if (ret == WQ_RET_SUSPEND) {
			spin_lock_irqsave(&cq_ctx->wq_kwq_lock, flags);
			if (cq_ctx->status == CQ_STATUS_RUNNING ||
				atomic_read(&cq_ctx->wq_hold_count) == 0) {
				spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
				continue;
			}
			cq_ctx->wq_kwq_is_running = false;
			spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
			hvgr_info(cq_ctx->ctx->gdev, HVGR_CQ, "WQ barrier suspend");
			break;
		}

		if (ret != WQ_RET_PASS && ret != WQ_RET_CALLBACK) {
			usleep_range(1000, 1500);
			continue;
		}

		if (hvgr_softq_dequeue(cq_ctx->sq_ctx) != 0)
			hvgr_err(cq_ctx->ctx->gdev, HVGR_CQ, "SQ dequeue FAIL");
	}
	cq_ctx->wq_kwq_is_running = false;
}

static void hvgr_wq_workqueue_handle(struct work_struct *data)
{
	struct hvgr_cq_ctx *cq_ctx = container_of(data, struct hvgr_cq_ctx, wq_kwq_handle);

	hvgr_wq_worker(cq_ctx);
}

static void hvgr_wq_notify_handle(void *priv)
{
	struct hvgr_cq_ctx * const cq_ctx = (struct hvgr_cq_ctx *)priv;

	(void)hvgr_wq_start_work(cq_ctx);
}

bool hvgr_wq_start_work(struct hvgr_cq_ctx * const cq_ctx)
{
	unsigned long flags;

	if (cq_ctx->sq_ctx == NULL)
		return true;

	spin_lock_irqsave(&cq_ctx->wq_kwq_lock, flags);
	if (cq_ctx->wq_kwq_is_running) {
		++cq_ctx->wq_notify_count;
		spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
		return true;
	}
	cq_ctx->wq_kwq_is_running = true;
	spin_unlock_irqrestore(&cq_ctx->wq_kwq_lock, flags);
	(void)queue_work(cq_ctx->wq_kwq, &cq_ctx->wq_kwq_handle);

	return true;
}

bool hvgr_wq_stop_work(struct hvgr_cq_ctx * const cq_ctx)
{
	struct hvgr_ctx *ctx = cq_ctx->ctx;

	if (ctx == NULL)
		return true;

	if (!cq_ctx->wq_kwq_is_running)
		return true;

	wake_up(&ctx->gdev->wq_dev.barrier_wait_wq);
	return true;
}

void hvgr_wq_recover(struct hvgr_ctx * const ctx, struct hvgr_cq_ctx * const cq_ctx)
{
	struct hvgr_soft_q *sq_ctx = cq_ctx->sq_ctx;

	if (sq_ctx == NULL) {
		hvgr_debug(ctx->gdev, HVGR_CQ, "%s ctx_%u sq ctx null", __func__, ctx->id);
		return;
	}
	cq_ctx->wq_notify_count = 0;
	atomic_set(&cq_ctx->wq_hold_count, 0);
	sq_ctx->ctrl->rd = 0;
}

int hvgr_wq_ctx_init(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx, uint32_t softq_id)
{
	if (softq_id > HVGR_MAX_NR_SOFTQ)
		return -1;

	if (softq_id == HVGR_MAX_NR_SOFTQ) {
		cq_ctx->sq_ctx = NULL;
		return 0;
	}

	/* 1. init softq */
	mutex_lock(&ctx->softq_lock);
	if (cq_ctx->sq_ctx != NULL) {
		mutex_unlock(&ctx->softq_lock);
		return -1;
	}
	if (ctx->id_to_qctx[softq_id] == NULL ||
		ctx->id_to_qctx[softq_id]->bind_flag) {
		mutex_unlock(&ctx->softq_lock);
		return -1;
	}
	ctx->id_to_qctx[softq_id]->bind_flag = true;
	cq_ctx->sq_ctx = ctx->id_to_qctx[softq_id];
	if (cq_ctx->sq_ctx == NULL || cq_ctx->sq_ctx->ctrl == NULL ||
		cq_ctx->sq_ctx->ctrl->flags != HVGR_SOFTQ_DIR_U_TO_K ||
		cq_ctx->sq_ctx->ctrl->entry_size != sizeof(wq_work_entry)||
		cq_ctx->sq_ctx->ctrl->entry_nums != WQ_QUEUE_LENGTH) {
		hvgr_err(ctx->gdev, HVGR_CQ, "ctx_%u wq sq_ctx is null or softq not right for %u",
			ctx->id, softq_id);
		if (cq_ctx->sq_ctx != NULL && cq_ctx->sq_ctx->ctrl != NULL)
			hvgr_err(ctx->gdev, HVGR_CQ, "ctx_%u num=%u, size=%u, flag=%lu", ctx->id,
				cq_ctx->sq_ctx->ctrl->entry_nums, cq_ctx->sq_ctx->ctrl->entry_size,
				cq_ctx->sq_ctx->ctrl->flags);
		cq_ctx->sq_ctx = NULL;
		mutex_unlock(&ctx->softq_lock);
		return -1;
	}
	mutex_unlock(&ctx->softq_lock);
	cq_ctx->wq_notify_count = 0;
	(void)hvgr_softq_register_notify(cq_ctx->sq_ctx, hvgr_wq_notify_handle, cq_ctx);
	/* 2. init variables which used by wait process */
	atomic_set(&cq_ctx->wq_hold_count, 0);
	/* 3. init kernel workqueue */
	spin_lock_init(&cq_ctx->wq_kwq_lock);
	cq_ctx->wq_kwq = alloc_workqueue("hvgr_wq_kwq",
		WQ_HIGHPRI | WQ_UNBOUND | __WQ_ORDERED | WQ_MEM_RECLAIM, 1);
	if (cq_ctx->wq_kwq == NULL) {
		hvgr_err(ctx->gdev, HVGR_CQ,
			"alloc workqueue hvgr_wq_kwq fail.\n");
		return -1;
	}
	INIT_WORK(&cq_ctx->wq_kwq_handle, hvgr_wq_workqueue_handle);
	return 0;
}

void hvgr_wq_ctx_term(struct hvgr_cq_ctx * const cq_ctx)
{
	if (cq_ctx->sq_ctx == NULL)
		return;
	/* 1. stop hvgr workqueue */
	(void)hvgr_wq_stop_work(cq_ctx);
	/* 2. destroy kernel workqueue */
	if (cq_ctx->wq_kwq != NULL) {
		flush_workqueue(cq_ctx->wq_kwq);
		destroy_workqueue(cq_ctx->wq_kwq);
		cq_ctx->wq_kwq = NULL;
		while (cq_ctx->wq_kwq_is_running)
			usleep_range(1000, 1500);
	}
	/* 3. term softq */
	(void)hvgr_softq_term(cq_ctx->sq_ctx);
}

int hvgr_wq_init(struct hvgr_device * const gdev)
{
	init_waitqueue_head(&(gdev->wq_dev.barrier_wait_wq));
	return 0;
}
