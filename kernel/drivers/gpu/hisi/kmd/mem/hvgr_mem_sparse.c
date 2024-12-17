/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_sparse.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mem_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_kv.h"
#include "hvgr_asid_api.h"
#include <linux/slab.h>
#include <linux/rbtree.h>
#include <linux/vmalloc.h>
#include "hvgr_log_api.h"

static void hvgr_mem_sparse_dec_area_ref(struct hvgr_mem_area *ref_area)
{
	if (atomic_dec_and_test(&ref_area->ref_cnt) && ref_area->freed) {
		/*
		 * If ref area's ref count reach 0 and area is freed,
		 * free the ref area.
		 */
		(void)hvgr_mem_free_pages(ref_area);
		hvgr_mem_delete_area(ref_area);
	}
}

static void hvgr_mem_sparse_del_all_spnode(struct hvgr_kv_map * const tab)
{
	struct rb_node *next = NULL;
	struct hvgr_kv_node *kv_node = NULL;
	struct hvgr_mem_sparse_node *sp_node = NULL;

	if (tab == NULL)
		return;

	next = rb_first(&tab->root);
	while (next) {
		kv_node = rb_entry(next, struct hvgr_kv_node, node);
		next = rb_next(next);
		if (kv_node != NULL) {
			sp_node = container_of(kv_node, struct hvgr_mem_sparse_node, node);
			if (sp_node->ref_area != NULL)
				hvgr_mem_sparse_dec_area_ref(sp_node->ref_area);

			rb_erase(&(sp_node->node.node), &(tab->root));
			RB_CLEAR_NODE(&(sp_node->node.node));
			kfree(sp_node);
		}
	}
}

static int hvgr_mem_sparse_add_spnode(struct hvgr_kv_map * const tab,
	struct hvgr_mem_area *sparse_area, struct hvgr_mem_area *ref_area,
	u64 key, u64 num)
{
	struct hvgr_mem_sparse_node *new_node = NULL;

	new_node = kzalloc(sizeof(*new_node), GFP_KERNEL);
	if (unlikely(new_node == NULL))
		return -1;

	new_node->sparse_area = sparse_area;
	new_node->ref_area = ref_area;
	atomic_inc(&new_node->ref_area->ref_cnt);
	new_node->node.key = key;
	new_node->node.size = num;
	(void)hvgr_kv_add(tab, &new_node->node);

	return 0;
}

static inline void hvgr_mem_sparse_del_spnode(struct hvgr_kv_map * const tab,
	struct hvgr_mem_sparse_node *node)
{
	hvgr_mem_sparse_dec_area_ref(node->ref_area);
	hvgr_kv_del(tab, node->node.key);
	kfree(node);
}

struct hvgr_mem_sparse_node *hvgr_mem_sparse_find_spnode(struct hvgr_kv_map * const tab,
	u64 start)
{
	struct hvgr_kv_node *kv_node = NULL;
	struct hvgr_mem_sparse_node *sp_node = NULL;

	kv_node = hvgr_kv_find_range(tab, start);
	if (kv_node != NULL) {
		sp_node = container_of(kv_node, struct hvgr_mem_sparse_node, node);
		if (sp_node->node.key + sp_node->node.size > start)
			return sp_node;
	}

	return NULL;
}

static int hvgr_mem_sparse_joint_spnode(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	struct hvgr_mem_area *ref_area, u64 start, u64 num)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	struct hvgr_mem_sparse_node *pri_node = NULL;
	struct hvgr_mem_sparse_node *next_node = NULL;

	sparse_cb = (struct hvgr_mem_sparse_cb *)area->zone->prv_data;
	if (sparse_cb == NULL)
		return -1;

	pri_node = hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start - 1);
	if (pri_node != NULL && pri_node->sparse_area == area &&
		pri_node->ref_area == ref_area) {
		/*
		 * The new region and the nearest pri region can
		 * merge into a new region.
		 */
		pri_node->node.size += num;
		next_node = hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start + num);
		if (next_node != NULL && next_node->sparse_area == area &&
			next_node->ref_area == ref_area) {
			pri_node->node.size += next_node->node.size;
			hvgr_mem_sparse_del_spnode(&sparse_cb->sparse_rec, next_node);
		}
		return 0;
	}

	next_node = hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start + num);
	if (next_node != NULL && next_node->sparse_area == area &&
		next_node->ref_area == ref_area) {
		/*
		 * The new region and the nearest next region
		 * can be merged into a new region.
		 */
		hvgr_kv_del(&sparse_cb->sparse_rec, next_node->node.key);
		next_node->node.key = start;
		next_node->node.size += num;
		(void)hvgr_kv_add(&sparse_cb->sparse_rec, &next_node->node);
		return 0;
	}

	/* create a new region */
	if (hvgr_mem_sparse_add_spnode(&sparse_cb->sparse_rec, area, ref_area, start, num)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse add new node fail when create ref",
			ctx->id);
		return -1;
	}

	return 0;
}

static int hvgr_mem_sparse_del_in_spnode(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	struct hvgr_mem_sparse_node *node, u64 *start, u64 *num)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	u64 key;
	u64 page_num;
	u64 gap;

	sparse_cb = (struct hvgr_mem_sparse_cb *)area->zone->prv_data;
	if (unlikely(sparse_cb == NULL))
		return -1;

	key = node->node.key;
	page_num = node->node.size;
	gap = *start - key;

	if (gap == 0 && *num >= page_num) {
		/*
		 * Start addr is equal to the node's start addr and the input
		 * range covers the range of the node.
		 *  1. X X X    2. X X X
		 *     # # #       # # # # #
		 * Node should be delete,  input range should be adjusted too.
		 */
		hvgr_mem_sparse_del_spnode(&sparse_cb->sparse_rec, node);
		*num -= page_num;
		if (*num != 0)
			*start += page_num;
	} else if (gap == 0 && *num < page_num) {
		/*
		 * Start addr is equal to the node's start addr and the input
		 * range covers the front range of the node.
		 *  1. X X X
		 *     # #
		 * Adjust the start addr and size of the node.
		 */
		hvgr_kv_del(&sparse_cb->sparse_rec, key);
		node->node.key = key + *num;
		node->node.size = page_num - *num;
		(void)hvgr_kv_add(&sparse_cb->sparse_rec, &node->node);
		*num = 0;
	} else if (gap + *num >= page_num) {
		/*
		 * Start addr is bigger than the node's start addr and the input
		 * range covers the rear range of the node.
		 *  1. X X X   2. X X X
		 *       # #        # # # #
		 * Adjust the size of the node, input range should be adjusted too.
		 */
		node->node.size = gap;
		*num -= (page_num - gap);
		if (*num != 0)
			*start += (page_num - gap);
	} else if (gap + *num < node->node.size) {
		/*
		 * The input range is in the middle of the node range.
		 *  1. X X X
		 *       #
		 * Split the current node, the front area size is gap.
		 * Rear area should be a new node.
		 */
		node->node.size = gap;
		if (hvgr_mem_sparse_add_spnode(&sparse_cb->sparse_rec, area, node->ref_area,
			key + gap + *num, page_num - gap - *num)) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse del ref add new node fail",
				ctx->id);
			return -1;
		}
		*num = 0;
	}

	return 0;
}

static int hvgr_mem_sparse_del_out_spnode(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	u64 *start, u64 *num)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	struct hvgr_mem_sparse_node *sp_node = NULL;
	struct hvgr_kv_node *kv_node = NULL;
	u64 key;

	sparse_cb = (struct hvgr_mem_sparse_cb *)area->zone->prv_data;
	if (unlikely(sparse_cb == NULL))
		return -1;
	/*
	 * Start addr is not in any storged node's range.
	 * Find the node whose start address is greater than input start.
	 * If could not find any node with start addr greater than input start.
	 * There's no need to deal with.
	 */
	kv_node = hvgr_kv_find_bigger_node(&sparse_cb->sparse_rec, *start);
	if (kv_node == NULL)
		goto out;
	sp_node = container_of(kv_node, struct hvgr_mem_sparse_node, node);
	if (sp_node->sparse_area != area)
		goto out;
	key = sp_node->node.key;
	/*
	 * The start address of the node is greater than
	 * the input range. There's no need to deal with.
	 */
	if (key > *start + *num)
		goto out;
	/*
	 * The input range contains the node's range.
	 * 1.    X X X    2.    X X X
	 *     # # #          # # # # # #
	 * Input range should be adjusted
	 */
	*num -= (key - *start);
	*start = key;

	return 0;
out:
	*num = 0;
	return 0;
}

static int hvgr_mem_sparse_del_ref(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	u64 start, u64 num)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	struct hvgr_mem_sparse_node *sp_node = NULL;

	sparse_cb = (struct hvgr_mem_sparse_cb *)area->zone->prv_data;
	if (sparse_cb == NULL)
		return -1;

	while (num > 0) {
		sp_node = hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start);
		if (sp_node != NULL) {
			/* Start addr is in the storged node's range */
			if (sp_node->sparse_area != area)
				break;
			if (hvgr_mem_sparse_del_in_spnode(ctx, area, sp_node, &start, &num) != 0) {
				hvgr_err(ctx->gdev, HVGR_MEM,
					"ctx_%u sparse del ref in spnode fail",
					ctx->id);
				return -1;
			}
		} else {
			if (hvgr_mem_sparse_del_out_spnode(ctx, area, &start, &num) != 0) {
				hvgr_err(ctx->gdev, HVGR_MEM,
					"ctx_%u sparse del ref out spnode fail",
					ctx->id);
				return -1;
			}
		}
	}

	return 0;
}

static int hvgr_mem_sparse_create_ref(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	struct hvgr_mem_area *ref_area, u64 start, u64 num)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;

	sparse_cb = (struct hvgr_mem_sparse_cb *)area->zone->prv_data;
	if (sparse_cb == NULL)
		return -1;

	if (hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start) != NULL ||
		hvgr_mem_sparse_find_spnode(&sparse_cb->sparse_rec, start + num - 1) != NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u old ref is not all cleared when create new ref",
			ctx->id);
		return -1;
	}
	if (hvgr_mem_sparse_joint_spnode(ctx, area, ref_area, start, num)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u spnode joint fail when create new ref",
			ctx->id);
		return -1;
	}

	return 0;
}

static bool hvgr_mem_sparse_map_gva(struct hvgr_ctx *ctx, uint64_t va,
	struct page **page_array, uint32_t pages, struct hvgr_mem_area *area)
{
	long ret;
	struct hvgr_mmu_setup_paras mmu_para = {0};

	if (ctx == NULL || page_array == NULL)
		return false;

	mmu_para.gva = va;
	mmu_para.page_array = page_array;
	mmu_para.pages = pages;
	mmu_para.mmu_flag = area->mmu_flag;
	mmu_para.flags = area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u spnode setup mmu fail ret=%d", ctx->id, ret);
		return false;
	}

	hvgr_mem_flush_pt(ctx, va, (uint64_t)pages, WITHOUT_FLUSH_L2);
	return true;
}

static struct hvgr_mem_area *hvgr_mem_sparse_get_area(struct hvgr_ctx *ctx, u64 addr, u64 num,
	u64 *pfn_off, bool is_sparse_area)
{
	struct hvgr_mem_area *area = NULL;
	u64 start_pfn;
	u64 addr_pfn;

	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, addr);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse mem, area not find", ctx->id);
		return NULL;
	}

	if ((is_sparse_area && area->property != HVGR_MEM_PROPERTY_SPARSE) ||
		(!is_sparse_area && area->property == HVGR_MEM_PROPERTY_SPARSE)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse area property error", ctx->id);
		return NULL;
	}
	if (!is_sparse_area && area->property != HVGR_MEM_PROPERTY_SAME_VA &&
		area->property != HVGR_MEM_PROPERTY_GPU_MEM) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparsing property is %lu",
			ctx->id, area->property);
		return NULL;
	}
	start_pfn = area->gva >> PAGE_SHIFT;
	addr_pfn = addr >> PAGE_SHIFT;
	if (unlikely(addr_pfn > U64_MAX - num) ||
		addr_pfn + num > start_pfn + area->pages) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse no enough space", ctx->id);
		return NULL;
	}

	*pfn_off = addr_pfn - start_pfn;

	return area;
}

static bool hvgr_mem_sparse_init(struct hvgr_mem_zone * const zone, void * const para)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	/* Alloc private data. */
	sparse_cb = kzalloc(sizeof(*sparse_cb), GFP_KERNEL);
	if (unlikely(sparse_cb == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse alloc cb fail", ctx->id);
		return false;
	}

	if (!hvgr_kv_init(&sparse_cb->sparse_rec)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse init rec map fail", ctx->id);
		goto fail;
	}
	zone->prv_data = sparse_cb;

	return true;
fail:
	kfree(sparse_cb);
	return false;
}

void hvgr_mem_sparse_term(struct hvgr_ctx *ctx)
{
	struct hvgr_mem_sparse_cb *sparse_cb = NULL;
	struct hvgr_mem_zone * const zone = &ctx->mem_ctx.zones[HVGR_MEM_PROPERTY_SPARSE];

	sparse_cb = (struct hvgr_mem_sparse_cb *)zone->prv_data;
	if (sparse_cb == NULL)
		return;
	zone->prv_data = NULL;

	if (!hvgr_kv_is_empty(&sparse_cb->sparse_rec)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse term but rec map not empty", ctx->id);
		hvgr_mem_sparse_del_all_spnode(&sparse_cb->sparse_rec);
	}
	kfree(sparse_cb);
}

static bool hvgr_mem_sparse_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	if ((((~(HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_GPU_CACHEABLE))) &
		area->attribute) != 0) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	area->max_pages = area->pages;

	return true;
}

static bool hvgr_mem_sparse_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	u64 i;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (area->max_pages > (U64_MAX / sizeof(struct page *))) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse alloc invalid para",
			ctx->id);
		return false;
	}

	if (area->max_pages <= MEM_PHY_ALLOC_THRESHOLD)
		area->page_array = kcalloc(area->max_pages, sizeof(struct page *), GFP_KERNEL);
	else
		area->page_array = vzalloc((area->max_pages * sizeof(struct page *)));
	if (unlikely(area->page_array == NULL))
		return false;

	area->gva = hvgr_mem_alloc_va(area, false);
	if (unlikely(area->gva == 0)) {
		kvfree(area->page_array);
		return false;
	}
	/*
	 * when sparse mem is alloced at the frist time,
	 * we map the sparse va with zero page
	 */
	for (i = 0; i < area->pages; i++)
		area->page_array[i] = ctx->gdev->mem_dev.sparse_zero_page;

	area->attribute |= HVGR_MEM_ATTR_MEM_SPARSE;
	hvgr_mem_attr2flag(area);
	if (!hvgr_mem_sparse_map_gva(ctx, area->gva, area->page_array, (uint32_t)area->pages,
		area)) {
		area->attribute &= ~HVGR_MEM_ATTR_MEM_SPARSE;
		(void)hvgr_mem_free_va(area);
		kvfree(area->page_array);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse mem alloc map fail", ctx->id);
		return false;
	}
	area->attribute &= ~HVGR_MEM_ATTR_MEM_SPARSE;

	return true;
}

static bool hvgr_mem_sparse_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	hvgr_mem_unmap_gva(area);

	if (hvgr_mem_sparse_del_ref(ctx, area, area->gva >> PAGE_SHIFT, area->pages))
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse del ref fail when free, %d_%d",
			ctx->id, ctx->tgid, ctx->pid);

	(void)hvgr_mem_free_va(area);
	if (area->page_array != NULL)
		kvfree(area->page_array);

	return true;
}

static const struct hvgr_zone_opts g_mem_zone_sparse = {
	.init = hvgr_mem_sparse_init,
	.term = hvgr_mem_opt_term,
	.check_para = hvgr_mem_sparse_check_para,
	.alloc = hvgr_mem_sparse_alloc,
	.free = hvgr_mem_sparse_free,
};

bool hvgr_mem_sparse_config(struct hvgr_mem_ctx * const pmem)
{
	if (pmem == NULL)
		return false;

	if (unlikely(!hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_SPARSE,
		&g_mem_zone_sparse, NULL)))
		return false;

	return true;
}

void hvgr_mem_sparse_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	mutex_lock(&ctx->mem_ctx.area_mutex);
	hvgr_mem_sparse_term(ctx);
	mutex_unlock(&ctx->mem_ctx.area_mutex);
}

int hvgr_mem_sparse_bind(struct hvgr_ctx *ctx, u64 sparse_addr, u64 mem_addr, u64 num)
{
	struct hvgr_mem_area *sparse_area = NULL;
	struct hvgr_mem_area *mem_area = NULL;
	u64 sparse_off;
	u64 mem_off;
	u64 cnt;
	u64 i;

	if (ctx == NULL || sparse_addr == 0 || mem_addr == 0 || num == 0)
		return -ENOENT;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	/* find the area according to gva */
	sparse_area = hvgr_mem_sparse_get_area(ctx, sparse_addr, num, &sparse_off, true);
	if (sparse_area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse bind, can't get the right sparse area",
			ctx->id);
		goto fail;
	}

	mem_area = hvgr_mem_sparse_get_area(ctx, mem_addr, num, &mem_off, false);
	if (mem_area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse bind, can't get the right mem area",
			ctx->id);
		goto fail;
	}

	if ((sparse_area->attribute & HVGR_MEM_MASK_GPU_ACCESS) !=
		(mem_area->attribute & HVGR_MEM_MASK_GPU_ACCESS)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse gpu attribute is error %lx,%lx",
			ctx->id, sparse_area->attribute, mem_area->attribute);
		goto fail;
	}

	/* use the specified page as the physical page */
	for (i = 0; i < num; i++) {
		if (mem_area->page_array[i + mem_off] == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u sparse bind, ref page not alloced, 0x%llx, 0x%llx",
				ctx->id, i, num);
			if (i == 0)
				goto fail;
			break;
		}
	}
	cnt = i;
	if (hvgr_mem_sparse_del_ref(ctx, sparse_area, sparse_addr >> PAGE_SHIFT, cnt)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse del old ref fail",
			ctx->id);
		goto fail;
	}

	for (i = 0; i < cnt; i++)
		sparse_area->page_array[i + sparse_off] = mem_area->page_array[i + mem_off];

	/* create the reference of sparse gva to another area */
	if (unlikely(hvgr_mem_sparse_create_ref(ctx, sparse_area, mem_area,
		sparse_addr >> PAGE_SHIFT, cnt)))
		goto term_array;

	hvgr_mem_attr2flag(sparse_area);
	/* set up mappings on the GPU side */
	if (!hvgr_mem_sparse_map_gva(ctx, sparse_addr, sparse_area->page_array + sparse_off,
		(uint32_t)cnt, sparse_area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse bind, setup mmu fail", ctx->id);
		if (hvgr_mem_sparse_del_ref(ctx, sparse_area, sparse_addr >> PAGE_SHIFT, cnt))
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse bind, del ref fail, %d_%d",
				ctx->id, ctx->tgid, ctx->pid);
		goto term_array;
	}

	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return 0;

term_array:
	/* use the zero page as the physical page */
	for (i = 0; i < cnt; i++)
		sparse_area->page_array[i + sparse_off] =
			ctx->gdev->mem_dev.sparse_zero_page;
fail:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return -EFAULT;
}

int hvgr_mem_sparse_unbind_nolock(struct hvgr_ctx *ctx, u64 sparse_addr, u64 num)
{
	struct hvgr_mem_area *sparse_area = NULL;
	u64 sparse_off;
	u64 i;

	if (ctx == NULL || sparse_addr == 0 || num == 0)
		return -ENOENT;

	/* find the area according to gva */
	sparse_area =  hvgr_mem_sparse_get_area(ctx, sparse_addr, num, &sparse_off, true);
	if (sparse_area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u sparse unbind, can't get the right sparse area", ctx->id);
		goto fail;
	}

	/* use the zero page as the physical page */
	for (i = 0; i < num; i++)
		sparse_area->page_array[i + sparse_off] =
			ctx->gdev->mem_dev.sparse_zero_page;

	sparse_area->attribute |= HVGR_MEM_ATTR_MEM_SPARSE;
	hvgr_mem_attr2flag(sparse_area);
	/* set up mappings on the GPU side */
	if (!hvgr_mem_sparse_map_gva(ctx, sparse_addr, sparse_area->page_array + sparse_off,
		(uint32_t)num, sparse_area)) {
		sparse_area->attribute &= ~HVGR_MEM_ATTR_MEM_SPARSE;
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse unbind, setup mmu fail", ctx->id);
		WARN_ON(1);
		goto fail;
	}
	sparse_area->attribute &= ~HVGR_MEM_ATTR_MEM_SPARSE;

	/* delete the reference of sparse gva to another area */
	if (hvgr_mem_sparse_del_ref(ctx, sparse_area, sparse_addr >> PAGE_SHIFT, num)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse unbind, del old ref fail, %d_%d",
			 ctx->id, ctx->tgid, ctx->pid);
		WARN_ON(1);
		goto fail;
	}
	return 0;
fail:
	return -EFAULT;
}

int hvgr_mem_sparse_unbind(struct hvgr_ctx *ctx, u64 sparse_addr, u64 num)
{
	int ret;

	if (ctx == NULL || sparse_addr == 0 || num == 0)
		return -ENOENT;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	ret = hvgr_mem_sparse_unbind_nolock(ctx, sparse_addr, num);
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	return ret;
}

int hvgr_mem_sparse_alloc_zero_page(struct hvgr_device * const gdev)
{
	if (gdev == NULL)
		return -ENOENT;

	gdev->mem_dev.sparse_zero_page = alloc_page(GFP_HIGHUSER | __GFP_ZERO);
	if (gdev->mem_dev.sparse_zero_page == NULL) {
		hvgr_err(gdev, HVGR_MEM, "alloc zero page for sparse fail");
		return -ENOMEM;
	}

	return 0;
}

void hvgr_mem_sparse_free_zero_page(struct hvgr_device * const gdev)
{
	if (gdev->mem_dev.sparse_zero_page != NULL)
		__free_page(gdev->mem_dev.sparse_zero_page);
}

int hvgr_sparse_mem_change_attr(struct hvgr_ctx *ctx,
	struct hvgr_mem_area *area, uint64_t attr)
{
	uint64_t temp_attr;

	if (unlikely(ctx == NULL || area == NULL || attr == 0))
		return -ENOENT;

	/* nothing to do */
	if (area->attribute == attr)
		return 0;

	if ((area->attribute & (~HVGR_MEM_ATTR_GPU_CACHEABLE)) !=
		(attr & (~HVGR_MEM_ATTR_GPU_CACHEABLE))) {
		hvgr_err(ctx->gdev, HVGR_MEM, "sparse attribute change to much");
		return -EFAULT;
	}

	temp_attr = area->attribute;
	area->attribute = attr;
	area->flags |= HVGR_MEM_AREA_FLAG_SPARSE_CACHE;
	hvgr_mem_attr2flag(area);
	if (!hvgr_mem_sparse_map_gva(ctx, area->gva, area->page_array,
		(uint32_t)area->pages, area)) {
		area->attribute = temp_attr;
		area->flags &= ~HVGR_MEM_AREA_FLAG_SPARSE_CACHE;
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sparse attribute change map fail",
			ctx->id);
		return -EFAULT;
	}
	area->flags &= ~HVGR_MEM_AREA_FLAG_SPARSE_CACHE;
	return 0;
}
