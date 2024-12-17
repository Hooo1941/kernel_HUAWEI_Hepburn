/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/kernel.h>
#include <linux/interval_tree_generic.h>

#include "hvgr_defs.h"
#include "hvgr_dm_api.h"
#include "hvgr_mem_interval_tree.h"
#include "hvgr_log_api.h"

#define do_start(node) ((node)->start)
#define do_last(node)  ((node)->last)

INTERVAL_TREE_DEFINE(struct hvgr_mem_interval_tree_node, rb,
		     unsigned long, __subtree_last,
		     do_start, do_last, static inline, hvgr_mem_interval_tree)


void hvgr_mem_uva_range_insert(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;

	RB_CLEAR_NODE(&area->itn.rb);
	area->itn.start = (unsigned long)(area->node.key >> PAGE_SHIFT);
	area->itn.last = area->itn.start + (unsigned long)area->max_pages - 1;
	hvgr_mem_interval_tree_insert(&area->itn, &pmem->itree);
}

void hvgr_mem_uva_range_remove(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;

	if (RB_EMPTY_NODE(&area->itn.rb))
		return;

	hvgr_mem_interval_tree_remove(&area->itn, &pmem->itree);
}


bool hvgr_mem_uva_check_valid(struct hvgr_ctx *ctx, unsigned long va, unsigned long size)
{
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx_tmp = NULL;
	struct hvgr_ctx *tmp = NULL;
	struct hvgr_mem_interval_tree_node *node;
	struct hvgr_mem_area *area = NULL;
	unsigned long va_pgoff_start = va >> PAGE_SHIFT;
	unsigned long va_pgoff_last = va_pgoff_start + (size >> PAGE_SHIFT) - 1;

	mutex_lock(&gdev->dm_dev.ctx_list_mutex);
	list_for_each_entry_safe(ctx_tmp, tmp, &gdev->dm_dev.ctx_list, dev_link) {
		if (ctx_tmp->tgid != ctx->tgid)
			continue;
		pmem = &ctx->mem_ctx;
		node = hvgr_mem_interval_tree_iter_first(&pmem->itree, va_pgoff_start, va_pgoff_last);
		if (node) {
			area = container_of(node, struct hvgr_mem_area, itn);
			hvgr_err(gdev, HVGR_MEM, "uva check invalid tgid:%u, overlap area 0x%lx:0x%lx, 0x%lx:0x%lx",
				ctx_tmp->tgid, area->uva, area->max_pages, va, size >> PAGE_SHIFT);
			mutex_unlock(&gdev->dm_dev.ctx_list_mutex);
			return false;
		}
	}

	mutex_unlock(&gdev->dm_dev.ctx_list_mutex);
	return true;
}