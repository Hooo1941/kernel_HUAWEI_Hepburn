/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mem_sc.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#include <linux/hisi/mm_lb.h>
#else
#include <linux/mm_lb/mm_lb.h>
#endif
#include <linux/highmem.h>
#ifdef CONFIG_MM_AMA
#include <platform_include/basicplatform/linux/mem_ama.h>
#endif

#include "hvgr_mem_sc_hal.h"
#include "hvgr_mem_pool.h"
#include "hvgr_log_api.h"
#include <securec.h>

struct hvgr_mem_pool *hvgr_sc_mem_find_pool(struct list_head *head, uint32_t policy_id)
{
	struct list_head *node = NULL;
	struct hvgr_sc_mem_pool *sc_mem_pool = NULL;

	list_for_each(node, head) {
		sc_mem_pool = container_of(node, struct hvgr_sc_mem_pool, node);
		if (sc_mem_pool->policy_id == policy_id)
			return &sc_mem_pool->pool;
	}

	return NULL;
}

struct hvgr_mem_pool *hvgr_sc_mem_find_pt_pool(struct hvgr_ctx *ctx)
{
	struct hvgr_mem_pool *pool = NULL;

	if (ctx == NULL)
		return NULL;

	if (ctx->mem_ctx.sc_data.pgd_mem_gid != 0)
		pool = hvgr_sc_mem_find_pool(&ctx->mem_ctx.sc_data.mem_pool_list,
			hvgr_sc_id_to_policy_id(ctx->mem_ctx.sc_data.pgd_mem_gid));

	return pool;
}

static void hvgr_sc_mem_deinit_pool(struct list_head *pool_list)
{
	struct hvgr_sc_mem_pool *sc_pool = NULL;
	struct list_head *node = NULL;
	struct list_head *next_node = NULL;

	if (pool_list == NULL)
		return;

	list_for_each_safe(node, next_node, pool_list) {
		sc_pool = container_of(node, struct hvgr_sc_mem_pool, node);
		hvgr_mem_pool_term(&sc_pool->pool);
		list_del_init(&sc_pool->node);
		kfree(sc_pool);
	}
}

static long hvgr_sc_mem_init_pool(void *target, bool is_dev, struct list_head *pool_list,
	uint32_t pool_size)
{
	long ret;
	struct hvgr_device *gdev = NULL;
	struct hvgr_ctx *ctx = NULL;
	struct list_head *info_node = NULL;
	const struct hvgr_sc_policy_info *policy_info = NULL;
	struct hvgr_sc_mem_pool *sc_pool = NULL;

	if (is_dev) {
		gdev = (struct hvgr_device *)target;
	} else {
		ctx = (struct hvgr_ctx *)target;
		gdev = ctx->gdev;
	}

	INIT_LIST_HEAD(pool_list);

	list_for_each(info_node, &gdev->mem_dev.sc_data.policy_info_list) {
		policy_info = container_of(info_node, struct hvgr_sc_policy_info, node);
		sc_pool = kzalloc(sizeof(*sc_pool), GFP_KERNEL);
		if (sc_pool == NULL) {
			hvgr_sc_mem_deinit_pool(pool_list);
			return -ENOMEM;
		}

		if (is_dev)
			ret = hvgr_mem_pool_dev_init(gdev, &sc_pool->pool, pool_size, 0);
		else
			ret = hvgr_mem_pool_ctx_init(ctx, &sc_pool->pool, pool_size, 0);
		if (ret != 0) {
			kfree(sc_pool);
			hvgr_err(gdev, HVGR_MEM, "policy id %u init mem pool fail",
				policy_info->policy_id);
		} else {
			INIT_LIST_HEAD(&sc_pool->node);
			sc_pool->policy_id = policy_info->policy_id;
			sc_pool->pool.policy_id = policy_info->policy_id;
			list_add(&sc_pool->node, pool_list);
		}
	}

	return 0;
}

long hvgr_sc_mem_init_dev_pool(struct hvgr_device * const gdev, uint32_t pool_size)
{
	struct list_head *mem_pool_list = &gdev->mem_dev.sc_data.mem_pool_list;

	return hvgr_sc_mem_init_pool(gdev, true, mem_pool_list, pool_size);
}

long hvgr_sc_mem_init_ctx_pool(struct hvgr_ctx *ctx, uint32_t pool_size)
{
	struct list_head *mem_pool_list = &ctx->mem_ctx.sc_data.mem_pool_list;

	return hvgr_sc_mem_init_pool(ctx, false, mem_pool_list, pool_size);
}

void hvgr_sc_mem_deinit_ctx_pool(struct hvgr_ctx *ctx)
{
	struct list_head *mem_pool_list = &ctx->mem_ctx.sc_data.mem_pool_list;

	hvgr_sc_mem_deinit_pool(mem_pool_list);
}

void hvgr_sc_mem_init(struct hvgr_device * const gdev)
{
	long ret;

	ret = hvgr_sc_mem_init_dev_pool(gdev, HVGR_MEM_POOL_DEV_SIZE);
	if (ret != 0)
		hvgr_err(gdev, HVGR_MEM, "Init dev sc mem pool fail %d", ret);
}

void hvgr_sc_mem_deinit(struct hvgr_device * const gdev)
{
	hvgr_sc_mem_deinit_pool(&gdev->mem_dev.sc_data.mem_pool_list);
}

struct page *hvgr_sc_mem_alloc_page(uint32_t policy_id, uint32_t order, gfp_t gfp)
{
	struct page *pages = NULL;

	if (policy_id != 0)
		pages = lb_alloc_pages(policy_id, gfp, order);
	else
#ifdef CONFIG_MM_AMA
		pages = ama_alloc_meida_pages(gfp, order);
#else
		pages = alloc_pages(gfp, order);
#endif

	return pages;
}

void hvgr_sc_mem_free_page(struct page *page, uint32_t policy_id, uint32_t order)
{
	if (policy_id != 0)
		lb_free_pages(policy_id, page, order);
	else
		__free_pages(page, order);
}

void hvgr_sc_mem_clear_page(struct page *page)
{
#ifdef CONFIG_MM_LB
	if (PageLB(page)) {
		(void)memset_s(lb_page_to_virt(page), PAGE_SIZE, 0x0, PAGE_SIZE);
		return;
	}
#endif
	clear_highpage(page);
}

/* enable or bypass LB */
void hvgr_enable_sc_lb(struct hvgr_device *gdev, unsigned int enable)
{
	struct list_head *info_node = NULL;
	struct hvgr_sc_policy_info *policy_info = NULL;
	unsigned int pid;

	if (gdev->lb_enable == enable)
		return;

#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
	gdev->lb_enable = enable;
	list_for_each(info_node, &gdev->mem_dev.sc_data.policy_info_list) {
		policy_info = container_of(info_node, struct hvgr_sc_policy_info, node);
		pid = policy_info->policy_id;
		if (enable == 1) {
			hvgr_info(gdev, HVGR_PM, "%s: pid = %u, enable lb\n", __func__, pid);
			lb_gid_enable(pid);
		} else {
			hvgr_info(gdev, HVGR_PM, "%s: pid = %u, bypass lb\n", __func__, pid);
			lb_gid_bypass(pid);
		}
	}
#endif
}

