/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_api.h"

#include <linux/kernel.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>
#include <linux/module.h>
#include <linux/statfs.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/lockdep.h>
#include <linux/rcupdate.h>
#include <linux/sched/mm.h>
#include <securec.h>
#ifdef CONFIG_HW_LINUX_KERNEL_MM
#include <linux/oom.h>
#endif

#include "hvgr_kmd_defs.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_regmap.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mmu_api.h"
#include "hvgr_mem_drv.h"
#include "hvgr_mem_4gva.h"
#include "hvgr_mem_sameva.h"
#include "hvgr_mem_jit.h"
#include "hvgr_mem_sparse.h"
#include "hvgr_mem_replay.h"
#include "hvgr_mem_gpu.h"
#include "hvgr_mem_import.h"
#include "hvgr_mem_2m.h"
#include "hvgr_mem_alias.h"
#include "hvgr_memory_page.h"
#include "hvgr_mem_debugfs.h"
#include "hvgr_mem_pool.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_asid_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_protect_mode.h"
#include "hvgr_mem_interval_tree.h"
#include "hvgr_platform_api.h"


static struct kmem_cache *hvgr_mem_area_slab;
static int hvgr_mem_area_slab_init(void)
{
	hvgr_mem_area_slab = kmem_cache_create("hvgr_mem_area",
		(unsigned int)sizeof(struct hvgr_mem_area), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!hvgr_mem_area_slab)
		return -ENOMEM;

	return 0;
}

static void hvgr_mem_area_slab_term(void)
{
	kmem_cache_destroy(hvgr_mem_area_slab);
}

struct hvgr_mem_area *hvgr_mem_new_area(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_mem_area *area = kmem_cache_zalloc(hvgr_mem_area_slab, GFP_KERNEL);

	if (area == NULL)
		return NULL;

	atomic_set(&area->ref_cnt, 0);
	atomic_set(&area->cpu_map_cnt, 0);
	atomic_set(&area->import.data.umm.free_tlb_cnt, 0);
	atomic_set(&area->import.data.umm.restore_tlb_cnt, 0);
	mutex_init(&area->mutex);
	mutex_init(&area->map_lock);
	area->map_cnt = 0;
	atomic_set(&area->growable_pages, 0);
#ifdef CONFIG_HVGR_SAMEVA_CACHE
	RB_CLEAR_NODE(&(area->cache_node));
	INIT_LIST_HEAD(&(area->child));
	INIT_LIST_HEAD(&(area->lru_node));
	area->cache_root = &(container_of(pmem, struct hvgr_ctx, mem_ctx)->root);
#endif
	return area;
}

void hvgr_mem_delete_area(struct hvgr_mem_area * const area)
{
	if (area == NULL)
		return;
	if (area->page_not_free)
		return;

	kmem_cache_free(hvgr_mem_area_slab, area);
}

struct hvgr_mem_area *hvgr_mem_new_area_with_va(struct hvgr_ctx * const ctx,
	uint64_t prot, uint64_t attribute, uint64_t max_pages)
{
	struct hvgr_mem_area *area = NULL;
	struct hvgr_mem_zone *zone = NULL;

	zone = hvgr_mem_zone_get_zone(&ctx->mem_ctx, prot);
	if (zone == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u new area zone is null, 0x%llx",
			ctx->id, prot);
		return NULL;
	}

	area = hvgr_mem_new_area(&ctx->mem_ctx);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u new area alloc area fail", ctx->id);
		return NULL;
	}

	area->zone = zone;
	area->property = prot;
	area->attribute = attribute;
	area->pages = 0;
	area->max_pages = max_pages;

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_mem_delete_area(area);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u new area alloc page array fail, 0x%llx",
			ctx->id, max_pages);
		return NULL;
	}

	if (prot == HVGR_MEM_PROPERTY_IMPORT)
		area->uva = hvgr_mem_import_alloc_va(area);
	else
		area->uva = hvgr_mem_alloc_va(area, false);
	if (area->uva == 0) {
		(void)hvgr_mem_free_pages(area);
		hvgr_mem_delete_area(area);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u new area alloc va fail, 0x%llx\n",
			ctx->id, max_pages);
		return NULL;
	}

	area->gva = area->uva;
	area->node.key = area->gva;
	area->node.size = area->max_pages << PAGE_SHIFT;
	(void)hvgr_kv_add(&ctx->mem_ctx.area_rec, &area->node);
	hvgr_mem_uva_range_insert(ctx, area);
	return area;
}

static struct hvgr_mem_area *hvgr_mem_alloc_import(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alloc_para * const para, struct hvgr_mem_area_va *area_va, bool user_buf)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_zone *zone = NULL;
	struct hvgr_mem_area *area = NULL;

	pmem = &ctx->mem_ctx;
	mutex_lock(&pmem->area_mutex);
	zone = hvgr_mem_zone_get_zone(pmem, para->property);
	if (zone == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u alloc import INVALID zone. prot=%lu:%lx, page:%lu/%lu",
			ctx->id, para->property, para->attribute, para->pages, para->max_pages);
		mutex_unlock(&pmem->area_mutex);
		return NULL;
	}

	area = hvgr_mem_zone_find_area_by_gva(pmem, para->va);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc import mem invalid va 0x%lx",
			ctx->id, para->va);
		mutex_unlock(&pmem->area_mutex);
		return NULL;
	}

	if (!hvgr_mem_zone_alloc_memory(zone, area)) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u alloc import mem fail. prot=%lu:%lx, page:%lu/%lu va 0x%lx",
			ctx->id, area->property, area->attribute,
			area->pages, area->max_pages, area->gva);
		mutex_unlock(&pmem->area_mutex);
		return NULL;
	}

	if (area_va != NULL) {
		area_va->uva = area->uva;
		area_va->gva = area->gva;
		area_va->kva = area->kva;
	}

	if (user_buf)
		area->user_buf = true;

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u alloc import mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		ctx->id, area->property, area->attribute, area->pages,
		area->max_pages, area->gva);

	mutex_unlock(&pmem->area_mutex);
	return area;
}

struct hvgr_mem_area *hvgr_mem_allocate(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alloc_para * const para, struct hvgr_mem_area_va *area_va, bool user_buf)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_zone *zone = NULL;
	struct hvgr_mem_area *area = NULL;

	if ((ctx == NULL) || (para == NULL))
		return NULL;

	if (para->max_pages < para->pages || para->max_pages == 0u) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u prot=%lu:%lx err pages num %lu:%lu",
			ctx->id, para->property, para->attribute, para->pages, para->max_pages);
		return NULL;
	}

	if ((para->attribute & HVGR_MEM_ATTR_COHERENT_SYS_REQ) != 0 &&
		!hvgr_mem_is_cpu_coherent(ctx->gdev)) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u prot=%lu:%lx need coherent system in no coherent dev.",
			ctx->id, para->property, para->attribute);
		return NULL;
	}

	if ((para->attribute & HVGR_MEM_ATTR_GPU_EXEC) != 0 &&
		(para->attribute & HVGR_MEM_ATTR_GPU_WR) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u prot=%lu:%lx gpu exe is conflict with gpu write.",
			ctx->id, para->property, para->attribute);
		return NULL;
	}

	if ((para->attribute & HVGR_MEM_ATTR_COHERENT_SYSTEM) != 0 &&
		!hvgr_mem_is_cpu_coherent(ctx->gdev))
		para->attribute &= (~HVGR_MEM_ATTR_COHERENT_SYSTEM);

	if (para->property == HVGR_MEM_PROPERTY_IMPORT)
		return hvgr_mem_alloc_import(ctx, para, area_va, user_buf);

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT) && para->property == HVGR_MEM_PROPERTY_4G_VA)
		para->property = HVGR_MEM_PROPERTY_SAME_VA;

	pmem = &ctx->mem_ctx;
	zone = hvgr_mem_zone_get_zone(pmem, para->property);
	if (zone == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u alloc memory INVALID zone. prot=%lu:%lx, page:%lu/%lu",
			ctx->id, para->property, para->attribute,
			para->pages, para->max_pages);
		return NULL;
	}

#ifdef CONFIG_HVGR_SAMEVA_CACHE
	area = hvgr_mem_get_cache(ctx, para, area_va, user_buf);
	if (area != NULL)
		return area;
#endif
	area = hvgr_mem_new_area(pmem);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc area FAIL.", ctx->id);
		return NULL;
	}

	area->zone = zone;
	area->property = para->property;
	area->attribute = para->attribute;
	area->pages = para->pages;
	area->max_pages = para->max_pages;
	area->extent = para->extent;
	area->gva = para->va;
	area->phy_pages = para->phy_pages;

	mutex_lock(&pmem->area_mutex);
	if (!hvgr_mem_zone_alloc_memory(zone, area)) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u alloc memory FAIL. prot=%lu:%lx, page:%lu/%lu",
			ctx->id, area->property, area->attribute,
			area->pages, area->max_pages);

		hvgr_mem_delete_area(area);
		mutex_unlock(&pmem->area_mutex);
		return NULL;
	}

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u alloc mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		ctx->id, area->property, area->attribute, area->pages,
		area->max_pages, area->gva);

	area->node.key = area->gva;
	area->node.size = area->max_pages << PAGE_SHIFT;
	(void)hvgr_kv_add(&pmem->area_rec, &area->node);
	hvgr_mem_uva_range_insert(ctx, area);

	if (area_va != NULL) {
		area_va->uva = area->uva;
		area_va->gva = area->gva;
		area_va->kva = area->kva;
	}

	if (user_buf)
		area->user_buf = true;
	mutex_unlock(&pmem->area_mutex);

	if ((para->attribute & HVGR_MEM_ATTR_KMD_ACCESS) != 0)
		memset_s(u64_to_ptr(area->kva), area->pages << PAGE_SHIFT,
			0, area->pages << PAGE_SHIFT);

	return area;
}

long hvgr_mem_free_area_nolock(struct hvgr_ctx * const ctx,
	struct hvgr_mem_area * const area)
{
	lockdep_assert_held(&ctx->mem_ctx.area_mutex);
	hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u free mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		ctx->id, area->property, area->attribute, area->pages, area->max_pages, area->gva);

	hvgr_kv_del(&ctx->mem_ctx.area_rec, area->node.key);
	hvgr_mem_uva_range_remove(ctx, area);

#ifdef CONFIG_HVGR_SAMEVA_CACHE
	if (hvgr_mem_set_cache(ctx, area) == 0)
		return 0;
#endif

	hvgr_mem_zone_free_memory(area);

	if (atomic_read(&area->ref_cnt) == 0)
		hvgr_mem_delete_area(area);
	else
		area->freed = true;

	return 0;
}

long hvgr_mem_free(struct hvgr_ctx * const ctx,
	uint64_t gva)
{
	struct hvgr_mem_area *area = NULL;
	long ret;
	uint32_t area_prop;

	if ((ctx == NULL) || (gva == 0))
		return -EINVAL;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, gva);
	if (area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}
	area_prop = (uint32_t)area->zone->proterty;
	ret = hvgr_mem_free_area_nolock(ctx, area);
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	if (area_prop == HVGR_MEM_PROPERTY_IMPORT)
		hvgr_del_bg_dmabuf_ctx_from_global_list(ctx);
	return ret;
}

long hvgr_jit_mem_free(struct hvgr_ctx * const ctx,
	uint64_t gva)
{
	struct hvgr_mem_area *area = NULL;
	long ret;

	if ((ctx == NULL) || (gva == 0))
		return -EINVAL;

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
	ret = hvgr_mem_free_area_nolock(ctx, area);
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	return ret;
}

void hvgr_mem_sync_page(struct hvgr_ctx * const ctx, struct page *page, uint64_t offset,
	uint64_t size, uint64_t direct)
{
	dma_addr_t dma_addr = hvgr_mem_get_page_dma_addr(page);

	dma_addr += offset;

	if (direct == HVGR_MEM_SYNC_TO_CPU)
		dma_sync_single_for_cpu(ctx->gdev->dev, dma_addr, size, DMA_BIDIRECTIONAL);
	else if (direct == HVGR_MEM_SYNC_TO_DEV)
		dma_sync_single_for_device(ctx->gdev->dev, dma_addr, size, DMA_BIDIRECTIONAL);
}

long hvgr_mem_sync(struct hvgr_ctx * const ctx, uint64_t uva, uint64_t size, uint64_t direct)
{
	struct hvgr_mem_area *sync_area = NULL;
	struct page **page_arr = NULL;
	uint64_t offset;
	uint64_t offset_page;
	uint64_t offset_page_byte;
	uint64_t sync_pages;
	uint64_t sync_size;
	uint64_t i;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	sync_area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, uva);
	if (sync_area == NULL) {
		sync_area = hvgr_mem_zone_find_area_by_uva_range(&ctx->mem_ctx, uva);
		if (sync_area == NULL) {
			mutex_unlock(&ctx->mem_ctx.area_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sync no area with key 0x%lx.",
				ctx->id, uva);
			return -EINVAL;
		}
	}

	if (atomic_read(&sync_area->import.data.umm.free_tlb_cnt) != 0) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "dma buffer page was freed, memory sync was not allowed");
		return -EINVAL;
	}

	if ((sync_area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) == 0) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return 0;
	}

	if (size > (sync_area->max_pages << PAGE_SHIFT)) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u sync area with key 0x%lx size 0x%lx out of range.",
			ctx->id, uva, size);
		return -EINVAL;
	}

	offset = uva - sync_area->uva;
	offset_page = offset >> PAGE_SHIFT;
	offset_page_byte = offset & (PAGE_SIZE - 1);
	sync_pages = (size + offset_page_byte + (PAGE_SIZE - 1)) >> PAGE_SHIFT;

	if (offset_page >= sync_area->pages || (offset_page + sync_pages) > sync_area->pages) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u Sync area with key 0x%lx sync pages out of 0x%lx.",
			ctx->id, uva, sync_area->pages);
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}

	page_arr = sync_area->page_array;

	sync_size = (uint64_t)min(((uint64_t)PAGE_SIZE - offset_page_byte), size);
	hvgr_mem_sync_page(ctx, page_arr[offset_page], offset_page_byte, sync_size, direct);

	for (i = 1; sync_pages > 2 && i < sync_pages - 1; i++)
		hvgr_mem_sync_page(ctx, page_arr[offset_page + i], 0, PAGE_SIZE, direct);

	if (sync_pages > 1) {
		sync_size = ((uva + size - 1) & (PAGE_SIZE - 1)) + 1;
		hvgr_mem_sync_page(ctx, page_arr[offset_page + sync_pages - 1], 0, sync_size,
			direct);
	}

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u sync mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx uva=0x%lx size=0x%lx direct=%lu",
		ctx->id, sync_area->property, sync_area->attribute, sync_area->pages,
		sync_area->max_pages, sync_area->uva, uva, size, direct);
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	return 0;
}

long hvgr_mem_config_proc(struct hvgr_ctx * const ctx, struct hvgr_mem_cfg_para *para)
{
	struct hvgr_mem_ctx *pmem = NULL;

	if (unlikely(ctx == NULL || para == NULL))
		return -EINVAL;

	pmem = &ctx->mem_ctx;

	if (!hvgr_mem_replay_config(pmem, para))
		goto replay_fail;

	if (!hvgr_mem_driver_config(pmem))
		goto driver_fail;

	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (!hvgr_mem_4gva_config(pmem))
			goto va4g_fail;
	}

	if (!hvgr_mem_sameva_config(pmem))
		goto sameva_fail;

	if (!hvgr_mem_jit_config(pmem, para))
		goto jit_fail;

	if (!hvgr_mem_sparse_config(pmem))
		goto sparse_fail;

	if (!hvgr_mem_gpu_config(pmem))
		goto gpu_fail;

	if (!hvgr_mem_import_config(pmem))
		goto import_fail;

	if (!hvgr_mem_alias_config(pmem))
		goto alias_fail;

	if (!hvgr_mem_2m_config(pmem))
		goto alias_fail;

	return 0;
alias_fail:
	 hvgr_mem_import_deconfig(pmem);
import_fail:
gpu_fail:
	hvgr_mem_sparse_deconfig(pmem);
sparse_fail:
	hvgr_mem_jit_deconfig(pmem);
jit_fail:
sameva_fail:
	hvgr_mem_4gva_deconfig(pmem);
va4g_fail:
driver_fail:
	hvgr_mem_replay_deconfig(pmem);
replay_fail:
	return -EPERM;
}

long hvgr_mem_deconfig_proc(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx *pmem = NULL;

	if (ctx == NULL)
		return -EINVAL;

	pmem = &ctx->mem_ctx;
	mutex_lock(&pmem->area_mutex);
	hvgr_mem_replay_deconfig(pmem);
	if (!hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		hvgr_mem_4gva_deconfig(pmem);
	hvgr_mem_jit_deconfig(pmem);
	hvgr_mem_2m_deconfig(pmem);
	hvgr_mem_import_deconfig(pmem);
	mutex_unlock(&pmem->area_mutex);
	hvgr_mem_protect_jcd_term(ctx);

	return 0;
}

static long hvgr_mem_init_process_res_cb(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_process_res_cb *res_cb = NULL;
	struct hvgr_kv_node *node = NULL;

	mutex_lock(&ctx->gdev->mem_dev.mem_process_res_lock);
#ifdef CONFIG_LIBLINUX
	node = hvgr_kv_get(&ctx->gdev->mem_dev.mem_process_res_rec, (uint64_t)ctx->tgid);
#else
	node = hvgr_kv_get(&ctx->gdev->mem_dev.mem_process_res_rec, (uint64_t)ctx->mem_ctx.process_mm);
#endif
	if (node != NULL) {
		res_cb = container_of(node, struct hvgr_mem_process_res_cb, kv_node);
		atomic_inc(&res_cb->ref_cnt);
		goto out;
	}

	res_cb = kvzalloc(sizeof(*res_cb), GFP_KERNEL);
	if (res_cb == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc reserve cb fail", ctx->id);
		mutex_unlock(&ctx->gdev->mem_dev.mem_process_res_lock);
		return -ENOMEM;
	}
#ifdef CONFIG_LIBLINUX
	res_cb->kv_node.key = (uint64_t)ctx->tgid;
#else
	res_cb->kv_node.key = (uint64_t)ctx->mem_ctx.process_mm;
#endif
	(void)hvgr_kv_add(&ctx->gdev->mem_dev.mem_process_res_rec, &res_cb->kv_node);
	atomic_set(&res_cb->ref_cnt, 1);
out:
	ctx->mem_ctx.reserve_cb = res_cb;
	mutex_unlock(&ctx->gdev->mem_dev.mem_process_res_lock);
	return 0;
}

static void hvgr_mem_term_process_res_cb(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_process_res_cb *res_cb = NULL;

	mutex_lock(&ctx->gdev->mem_dev.mem_process_res_lock);
	res_cb = ctx->mem_ctx.reserve_cb;
	if (res_cb != NULL && atomic_dec_and_test(&res_cb->ref_cnt)) {
		hvgr_kv_del(&ctx->gdev->mem_dev.mem_process_res_rec,
			res_cb->kv_node.key);
		kvfree(res_cb);
		ctx->mem_ctx.reserve_cb = NULL;
	}
	mutex_unlock(&ctx->gdev->mem_dev.mem_process_res_lock);
}

static unsigned long hvgr_mem_jit_shrink_count(struct shrinker *shrink,
	struct shrink_control *shrink_c)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_area *area = NULL;
	struct list_head *entry = NULL;
	unsigned long pages = 0;

	pmem = container_of(shrink, struct hvgr_mem_ctx, jit_shrink);

	mutex_lock(&pmem->jit_shrink_mutex);
	list_for_each_prev(entry, &pmem->jit_pool_head) {
		area = list_entry(entry, struct hvgr_mem_area, jit_node);
		pages += area->pages;
	}
	mutex_unlock(&pmem->jit_shrink_mutex);

	return pages;
}

static long hvgr_mem_jit_shrink(struct hvgr_ctx * const ctx, struct hvgr_mem_area *area,
	uint64_t pages)
{
	long ret;
	uint32_t policy_id = 0;
	uint64_t diff_page = area->pages - pages;
	uint64_t va_offset = (pages << PAGE_SHIFT);

	ret = hvgr_mmu_take_down(ctx, (area->gva + va_offset), (uint32_t)diff_page);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize 0x%lx pages %lu take down fail %d",
			ctx->id, area->gva, pages, ret);
		return -ENOMEM;
	}

	hvgr_mem_flush_pt(ctx, (area->gva + va_offset), diff_page, WITH_FLUSH_L2);

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	policy_id = hvgr_get_policy_id(area->attribute);
#endif

	hvgr_mem_free_page(ctx->gdev->dev, (area->page_array + pages), (uint32_t)diff_page,
		area->order, policy_id);
	atomic_sub((int)diff_page, &ctx->mem_ctx.used_pages);
	atomic_sub((int)diff_page, &ctx->gdev->mem_dev.used_pages);

	area->pages = pages;
	area->phy_pages = pages;

	return 0;
}

static unsigned long hvgr_mem_jit_shrink_free(struct shrinker *shrink,
	struct shrink_control *shrink_c)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_area *area = NULL;
	struct list_head *entry = NULL;
	struct hvgr_ctx *ctx = NULL;
	unsigned long free_pages = 0;

	pmem = container_of(shrink, struct hvgr_mem_ctx, jit_shrink);
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	mutex_lock(&pmem->jit_shrink_mutex);
	list_for_each_prev(entry, &pmem->jit_pool_head) {
		area = list_entry(entry, struct hvgr_mem_area, jit_node);
		free_pages += area->pages;
		if (hvgr_mem_jit_shrink(ctx, area, 0u) != 0) {
			free_pages = -1;
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit shrinker free fail", ctx->id);
			goto unlock_out;
		}

		if (free_pages > shrink_c->nr_to_scan)
			break;
	}
unlock_out:
	mutex_unlock(&pmem->jit_shrink_mutex);

	return free_pages;
}

static void hvgr_mem_jit_pool_init(struct hvgr_ctx * const ctx)
{
	int ret;
	struct hvgr_mem_ctx *pmem = &ctx->mem_ctx;

	mutex_init(&pmem->jit_shrink_mutex);

	mutex_lock(&pmem->jit_shrink_mutex);
	INIT_LIST_HEAD(&pmem->jit_pool_head);
	INIT_LIST_HEAD(&pmem->jit_using_head);
	mutex_unlock(&pmem->jit_shrink_mutex);

	pmem->jit_shrink.count_objects = hvgr_mem_jit_shrink_count;
	pmem->jit_shrink.scan_objects = hvgr_mem_jit_shrink_free;
	pmem->jit_shrink.seeks = DEFAULT_SEEKS;
	pmem->jit_shrink.batch = 0;

	ret = register_shrinker(&pmem->jit_shrink);
	if (ret != 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u reg jit shrinker fail %d", ctx->id, ret);
}

long hvgr_mem_init_proc(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx *pmem = NULL;

	if (ctx == NULL)
		return -EINVAL;

	pmem = &ctx->mem_ctx;
	if (!hvgr_kv_init(&pmem->area_rec) ||
		!hvgr_kv_init(&pmem->map_rec) ||
		!hvgr_kv_init(&pmem->free_rec))
		return -EPERM;

	if (hvgr_mem_pool_ctx_init(ctx, &pmem->mem_pool, HVGR_MEM_POOL_CTX_SIZE, 0) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem pool init fail", ctx->id);
		return -ENOMEM;
	}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	if (hvgr_sc_mem_init_ctx_pool(ctx, HVGR_MEM_POOL_CTX_SIZE) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc mem pool init fail", ctx->id);
		goto init_sc_pool_fail;
	}
#endif

	mutex_init(&pmem->mmu_mutex);
	mutex_init(&pmem->area_mutex);
	mutex_init(&pmem->mmap_mutex);
	mutex_init(&pmem->profile_mutex);
	atomic_set(&pmem->used_pages, 0);
	atomic_set(&pmem->used_growable_pages, 0);
	atomic_set(&pmem->used_pt_pages, 0);
	pmem->itree = RB_ROOT_CACHED;
	if (hvgr_mmu_init_proc(ctx) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mmu init fail", ctx->id);
		goto init_mmu_fail;
	}

	hvgr_mem_jit_pool_init(ctx);

	(void)hvgr_mem_zone_init_proc(pmem);
	pmem->pend_area_bitmap = ~((unsigned long)1u);
	if (ctx->kfile != NULL) {
		rcu_read_lock();
		mmgrab(current->mm);
		rcu_assign_pointer(pmem->process_mm, current->mm);
		rcu_read_unlock();
	}

	INIT_LIST_HEAD(&pmem->dma_buf_rec_head);
	if (hvgr_mem_init_process_res_cb(ctx) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem init res fail", ctx->id);
		goto init_res_fail;
	}

	return 0;
init_res_fail:
	if (ctx->kfile != NULL) {
		rcu_read_lock();
		mmdrop(current->mm);
		rcu_read_unlock();
	}
	unregister_shrinker(&pmem->jit_shrink);
	hvgr_mmu_term_proc(ctx);
init_mmu_fail:
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_mem_deinit_ctx_pool(ctx);
init_sc_pool_fail:
#endif
	hvgr_mem_pool_term(&pmem->mem_pool);
	return -ENOMEM;
}

static void hvgr_mem_free_leak_area(struct hvgr_ctx * const ctx, struct hvgr_kv_map *area_rec)
{
	struct rb_root *rbtree = &area_rec->root;
	struct rb_node *rbnode = rb_first(rbtree);
	struct hvgr_kv_node *kvnode = NULL;
	struct hvgr_mem_area *area = NULL;

	while (rbnode != NULL) {
		kvnode = rb_entry(rbnode, struct hvgr_kv_node, node);
		area = container_of(kvnode, struct hvgr_mem_area, node);
		(void)hvgr_mem_free_area_nolock(ctx, area);

		rbnode = rb_first(rbtree);
	}
}

void hvgr_mem_free_area_by_property(struct hvgr_ctx * const ctx,
	struct hvgr_kv_map *area_rec, uint64_t prot)
{
	struct rb_root *rbtree = &area_rec->root;
	struct rb_node *next = rb_first(rbtree);
	struct hvgr_kv_node *kvnode = NULL;
	struct hvgr_mem_area *area = NULL;

	while (next != NULL) {
		kvnode = rb_entry(next, struct hvgr_kv_node, node);
		area = container_of(kvnode, struct hvgr_mem_area, node);
		next = rb_next(next);
		if (area->property == prot)
			(void)hvgr_mem_free_area_nolock(ctx, area);
	}
}

static void hvgr_mem_unbind_all_area_by_property(struct hvgr_ctx * const ctx,
	struct hvgr_kv_map *area_rec, uint64_t prot)
{
	struct rb_root *rbtree = &area_rec->root;
	struct rb_node *next = rb_first(rbtree);
	struct hvgr_kv_node *kvnode = NULL;
	struct hvgr_mem_area *area = NULL;

	while (next != NULL) {
		kvnode = rb_entry(next, struct hvgr_kv_node, node);
		area = container_of(kvnode, struct hvgr_mem_area, node);
		next = rb_next(next);
		if (area->property == prot)
			(void)hvgr_mem_sparse_unbind_nolock(ctx, area->gva, area->pages);
	}
}

static void hvgr_mem_free_map_area(struct hvgr_ctx * const ctx, struct hvgr_kv_map *map_rec)
{
	struct rb_root *rbtree = &map_rec->root;
	struct rb_node *rbnode = rb_first(rbtree);
	struct hvgr_kv_node *kvnode = NULL;
	struct hvgr_mem_area *area = NULL;

	while (rbnode != NULL) {
		kvnode = rb_entry(rbnode, struct hvgr_kv_node, node);
		area = container_of(kvnode, struct hvgr_mem_area, node);
		(void)hvgr_mem_unmap_area_nolock(ctx, area, true);

		rbnode = rb_first(rbtree);
	}
}

static void hvgr_mem_free_area_pages(struct hvgr_ctx * const ctx, struct hvgr_kv_map *free_rec)
{
	struct rb_root *rbtree = &free_rec->root;
	struct rb_node *rbnode = rb_first(rbtree);
	struct hvgr_kv_node *kvnode = NULL;
	struct hvgr_mem_area *area = NULL;

	while (rbnode != NULL) {
		kvnode = rb_entry(rbnode, struct hvgr_kv_node, node);
		area = container_of(kvnode, struct hvgr_mem_area, node);
		hvgr_kv_del(free_rec, area->node.key);
		area->map_cnt = 0;
		if (area->property == HVGR_MEM_PROPERTY_IMPORT)
			hvgr_mem_import_free_pages(area);
		else
			(void)hvgr_mem_free_pages(area);
		hvgr_mem_delete_area(area);

		rbnode = rb_first(rbtree);
	}
}

void hvgr_mem_free_area_when_gpu_fault(struct hvgr_ctx * const ctx, uint64_t prot)
{
	struct hvgr_mem_ctx * const pmem = &ctx->mem_ctx;

	mutex_lock(&pmem->area_mutex);
	if (!hvgr_kv_is_empty(&pmem->area_rec)) {
		hvgr_debug(ctx->gdev, HVGR_MEM, "ctx_%u free prot %u when gpu fault.", ctx->id, prot);
		hvgr_mem_free_area_by_property(ctx, &pmem->area_rec, prot);
	}
	mutex_unlock(&pmem->area_mutex);
}

void hvgr_mem_unbind_all_when_gpu_fault(struct hvgr_ctx * const ctx, uint64_t prot)
{
	struct hvgr_mem_ctx * const pmem = &ctx->mem_ctx;

	mutex_lock(&pmem->area_mutex);
	if (!hvgr_kv_is_empty(&pmem->area_rec)) {
		hvgr_debug(ctx->gdev, HVGR_MEM, "ctx_%u unbind prot %u when gpu fault.", ctx->id, prot);
		hvgr_mem_unbind_all_area_by_property(ctx, &pmem->area_rec, prot);
	}
	mutex_unlock(&pmem->area_mutex);
}

static void hvgr_mem_jit_check_leak(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx * const pmem = &ctx->mem_ctx;

	if (list_empty(&pmem->jit_using_head) == 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit using head not free", ctx->id);

	if (list_empty(&pmem->jit_pool_head) == 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u jit pool head not free", ctx->id);
}

void hvgr_mem_term_proc(struct hvgr_ctx * const ctx)
{
	struct hvgr_mem_ctx * const pmem = &ctx->mem_ctx;

	unregister_shrinker(&pmem->jit_shrink);

	mutex_lock(&pmem->area_mutex);
	if (!hvgr_kv_is_empty(&pmem->area_rec)) {
		hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u alloc memory not free", ctx->id);
		hvgr_mem_free_leak_area(ctx, &pmem->area_rec);
	}

	if (!hvgr_kv_is_empty(&pmem->map_rec)) {
		hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u map memory not free", ctx->id);
		hvgr_mem_free_map_area(ctx, &pmem->map_rec);
	}

	if (!hvgr_kv_is_empty(&pmem->free_rec)) {
		hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u pages not free", ctx->id);
		hvgr_mem_free_area_pages(ctx, &pmem->free_rec);
	}

	/* sparse may free other zone mem, term it before others */
	hvgr_mem_sparse_term(ctx);
	mutex_unlock(&pmem->area_mutex);
	hvgr_mem_jit_check_leak(ctx);
	hvgr_del_bg_dmabuf_ctx_from_global_list(ctx);
	hvgr_mem_zone_term_proc(pmem);
	hvgr_mmu_term_proc(ctx);
	hvgr_mem_term_process_res_cb(ctx);
	hvgr_mem_remove_profile(ctx);
	hvgr_mem_pool_term(&pmem->mem_pool);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_mem_deinit_ctx_pool(ctx);
#endif
	rcu_read_lock();
	if(rcu_dereference(pmem->process_mm) != NULL) {
		mmdrop(pmem->process_mm);
		rcu_assign_pointer(pmem->process_mm, NULL);
	}
	rcu_read_unlock();
}

bool hvgr_mem_is_cpu_coherent(struct hvgr_device * const gdev)
{
	return ((gdev->mem_dev.system_coherency == COHERENCY_ACE_LITE) ||
		(gdev->mem_dev.system_coherency == COHERENCY_ACE));
}

static void hvgr_mem_coherency_init(struct hvgr_device * const gdev)
{
	uint32_t value;
	int ret;

	gdev->mem_dev.system_coherency = COHERENCY_NONE;

	ret = of_property_read_u32(gdev->dev->of_node, "system-coherency",
		&value);
	if (ret != 0) {
		hvgr_err(gdev, HVGR_MEM, "No system-coherency cfg");
		return;
	}

	if (value != COHERENCY_NONE && value != COHERENCY_ACE_LITE && value != COHERENCY_ACE) {
		hvgr_err(gdev, HVGR_MEM, "Unsupport system-coherency cfg 0x%x", value);
		return;
	}

	gdev->mem_dev.system_coherency = value;
	hvgr_info(gdev, HVGR_MEM, "system-coherency cfg 0x%x", value);
}

static int hvgr_mem_dma_mask_init(struct hvgr_device * const gdev)
{
	int ret;
	uint32_t pa_width = (gdev->dm_dev.dev_reg.mmu_features >> 8) & 0xFF;

	ret = dma_set_mask(gdev->dev, DMA_BIT_MASK(pa_width));
	if (ret != 0) {
		hvgr_err(gdev, HVGR_MEM, "dma set mask fail %d", ret);
		return ret;
	}

	ret = dma_set_coherent_mask(gdev->dev, DMA_BIT_MASK(pa_width));
	if (ret != 0) {
		hvgr_err(gdev, HVGR_MEM, "dma set coherent mask fail %d", ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_DFX_DEBUG_FS
HVGR_STATIC void HVGR_WEAK hvgr_mem_kut_init(struct hvgr_device * const gdev)
{
}
#endif

#ifdef CONFIG_HW_LINUX_KERNEL_MM
static inline unsigned long hvgr_page_to_kib(unsigned int pages)
{
	return (pages << (PAGE_SHIFT - 10));
}

/*
 * hvgr_oom_notifier_handler - gpu driver out-of-memory handler
 * @nb - notifier block - used to retrieve gdev pointer
 * @action - action (unused)
 * @data - data pointer (unused)
 * This function simply lists memory usage by the gpu driver, per GPU device,
 * for diagnostic purposes.
 */
static int hvgr_oom_notifier_handler(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct hvgr_device *gdev = NULL;
	unsigned long gdev_alloc_total;

	if (WARN_ON(nb == NULL))
		return NOTIFY_BAD;

	gdev = hvgr_get_device();

	gdev_alloc_total =
		hvgr_page_to_kib((unsigned int)atomic_read(&(gdev->mem_dev.used_pages)));

	hvgr_err(gdev, HVGR_DM, "OOM notifier: dev %s  %lu kB\n", gdev->devname,
		gdev_alloc_total);

	return NOTIFY_OK;
}
#endif

int hvgr_mem_init(struct hvgr_device * const gdev)
{
	struct hvgr_mem_dev *mem_dev = &gdev->mem_dev;

	hvgr_mem_coherency_init(gdev);

#ifdef CONFIG_HVGR_ASYNC_MEM_RECYCLE
	if (hvgr_mem_recycle_init(gdev) != 0)
		return -1;
#endif

	if (hvgr_mem_area_slab_init() != 0)
		goto area_slab_init_fail;

	if(hvgr_mem_dma_mask_init(gdev) != 0)
		goto dma_mask_init_fail;

	if (!hvgr_kv_init(&mem_dev->mem_process_res_rec))
		goto dma_mask_init_fail;

	mutex_init(&mem_dev->mem_process_res_lock);
	spin_lock_init(&mem_dev->mmu_hw_lock);
	atomic_set(&mem_dev->used_pages, 0);

	if (hvgr_mem_sparse_alloc_zero_page(gdev) != 0)
		goto dma_mask_init_fail;

	if (hvgr_mem_pool_dev_init(gdev, &mem_dev->mem_pool, HVGR_MEM_POOL_DEV_SIZE, 0) != 0)
		goto pool_dev_init_fail;

	hvgr_mtrack_debugfs_init(gdev);

#ifdef CONFIG_DFX_DEBUG_FS
	mem_dev->mem_dir = hvgr_debugfs_create_dir("mem", gdev->root_debugfs_dir);
	if (IS_ERR_OR_NULL(mem_dev->mem_dir)) {
		hvgr_err(gdev, HVGR_MEM, "mem debugfs init fail");
	} else {
		hvgr_mem_debugfs_init(gdev);
		hvgr_mem_kut_init(gdev);
	}
#endif

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_init(gdev);
	hvgr_sc_init_remap_regs(gdev);
	hvgr_sc_mem_init(gdev);
#endif

	if(!hvgr_mem_protect_jcd_pool_init(gdev))
		hvgr_err(gdev, HVGR_MEM, "protect jcd pool init fail");

	hvgr_ion_mem_reclaim_init();
	mutex_init(&mem_dev->dmabuf_rec_lock);
	INIT_LIST_HEAD(&mem_dev->dmabuf_ctx_list_head);

	if (hvgr_mmu_init(gdev) != 0)
		goto mmu_init_fail;

	if ((int)hvgr_mmu_irq_init(gdev))
		goto mmut_irq_init_fail;

#ifdef CONFIG_HW_LINUX_KERNEL_MM
	gdev->oom_notifier_block.notifier_call = hvgr_oom_notifier_handler;
	if (register_oom_notifier(&gdev->oom_notifier_block)) {
		hvgr_err(gdev, HVGR_DM, "Unable to register OOM notifier for hvgr - but will continue\n");
		gdev->oom_notifier_block.notifier_call = NULL;
	}
#endif

	return 0;

mmut_irq_init_fail:
	hvgr_mmu_term(gdev);
mmu_init_fail:
	hvgr_mem_protect_jcd_pool_term(gdev);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_mem_deinit(gdev);
	hvgr_sc_deinit(gdev);
#endif
#ifdef CONFIG_DFX_DEBUG_FS
	if (!IS_ERR_OR_NULL(mem_dev->mem_dir)) {
		debugfs_remove_recursive(mem_dev->mem_dir);
		mem_dev->mem_dir = NULL;
	}
#endif
	hvgr_mtrack_debugfs_term(gdev);
	hvgr_mem_pool_term(&mem_dev->mem_pool);
pool_dev_init_fail:
	hvgr_mem_sparse_free_zero_page(gdev);
dma_mask_init_fail:
	hvgr_mem_area_slab_term();
area_slab_init_fail:
#ifdef CONFIG_HVGR_ASYNC_MEM_RECYCLE
	hvgr_mem_recycle_term(gdev);
#endif
	return -1;
}

void hvgr_mem_term(struct hvgr_device * const gdev)
{
	struct hvgr_mem_dev *mem_dev = &gdev->mem_dev;

	hvgr_mtrack_debugfs_term(gdev);

#ifdef CONFIG_DFX_DEBUG_FS
	if (!IS_ERR_OR_NULL(mem_dev->mem_dir)) {
		debugfs_remove_recursive(mem_dev->mem_dir);
		mem_dev->mem_dir = NULL;
	}
#endif

	hvgr_mem_pool_term(&mem_dev->mem_pool);

	hvgr_mem_sparse_free_zero_page(gdev);

	if (!hvgr_kv_is_empty(&mem_dev->mem_process_res_rec))
		hvgr_err(gdev, HVGR_MEM, "Some process 4gva do not free");

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_mem_deinit(gdev);
#endif
	hvgr_mem_area_slab_term();

	hvgr_mmu_term(gdev);

	if (!hvgr_mem_protect_jcd_pool_term(gdev))
		hvgr_info(gdev, HVGR_MEM, "protect jcd pool do not need term");

#ifdef CONFIG_HVGR_ASYNC_MEM_RECYCLE
	hvgr_mem_recycle_term(gdev);
#endif

#ifdef CONFIG_HW_LINUX_KERNEL_MM
	if (gdev->oom_notifier_block.notifier_call)
		unregister_oom_notifier(&gdev->oom_notifier_block);
#endif
}

uint32_t hvgr_mem_get_msb(struct hvgr_ctx * const ctx)
{
	if (ctx == NULL)
		return 0;

	return hvgr_mem_4gva_get_msb(&ctx->mem_ctx);
}

long hvgr_mem_rw(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t *data, bool write)
{
	struct hvgr_mem_area *area = NULL;
	uint64_t offset;
	uint64_t offset_page;
	uint64_t offset_page_byte;
	struct page *page = NULL;
	dma_addr_t dma_addr;
	char *addr_tar = NULL;
	void *addr = NULL;

	if (data == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem_rw input data is null.", ctx->id);
		return -EINVAL;
	}

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, gva);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem_rw no area for gva=0x%lx.",
			ctx->id, gva);
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}

	if ((write && (area->attribute & HVGR_MEM_ATTR_CPU_WR) == 0) ||
		(!write && (area->attribute & HVGR_MEM_ATTR_CPU_RD) == 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem_rw gva=0x%lx %lu:0x%lx attr err.",
			ctx->id, gva, area->property, area->attribute);
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return -EINVAL;
	}

	offset = gva - area->gva;
	offset_page = offset >> PAGE_SHIFT;
	offset_page_byte = offset & (PAGE_SIZE - 1);

	page = area->page_array[offset_page];
	addr = kmap_atomic(page);
	addr_tar = ((char *)addr) + offset_page_byte;

	if (write) {
		*((uint64_t *)addr_tar) = *data;
		dma_addr = hvgr_mem_get_page_dma_addr(page);
		dma_sync_single_for_device(ctx->gdev->dev, dma_addr, PAGE_SIZE, DMA_TO_DEVICE);
	}
	else {
		*data = *((uint64_t *)addr_tar);
	}

	kunmap_atomic(addr);
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return 0;
}

struct hvgr_mem_area *hvgr_mem_import(struct hvgr_ctx * const ctx,
	struct hvgr_mem_import_para * const para, uint64_t *uva, uint64_t *gva)
{
	struct hvgr_mem_area *area = NULL;

	if (para->pages == 0 || (para->attribute & HVGR_MEM_MASK_GPU_ACCESS) == 0 ||
		(para->attribute & HVGR_MEM_ATTR_GPU_EXEC) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_GROWABLE) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_KMD_ACCESS) != 0 ||
		((para->attribute & HVGR_MEM_ATTR_SECURE) != 0 &&
		(para->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0) ||
		(para->attribute & HVGR_MEM_MASK_COHERENT) != 0 ||
		(para->attribute & HVGR_MEM_ATTR_MEM_SPARSE) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import para error, 0x%lx:0x%lx",
			ctx->id, para->pages, para->attribute);
		return NULL;
	}

	mutex_lock(&ctx->mem_ctx.area_mutex);
	if (para->gva == 0) {
		area = hvgr_mem_new_area_with_va(ctx, HVGR_MEM_PROPERTY_IMPORT,
			para->attribute, para->pages);
		if (area != NULL)
			area->user_buf = true;
	} else {
		area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, para->gva);
	}
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u import area fail.fd=%d gva=0x%lx pages:%lu attr:0x%lx",
			ctx->id, para->fd, para->gva, para->pages, para->attribute);
		goto err_out;
	}

	if (!hvgr_mem_zone_import_memory(area->zone, area, para->fd)) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u import FAIL. prot=%lu:%lx, page:%lu/%lu fd=%d gva=0x%lx",
			ctx->id, area->property, area->attribute,
			area->pages, area->max_pages, para->fd, para->gva);
		area = NULL;
		goto err_out;
	}

	*uva = area->uva;
	*gva = area->gva;
	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u import mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx fd=%d",
		ctx->id, area->property, area->attribute, area->pages, area->max_pages, area->gva,
		para->fd);

err_out:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	hvgr_add_bg_dmabuf_ctx_to_global_list(ctx);
	return area;
}

static long hvgr_mem_resize_shrink(struct hvgr_ctx * const ctx, struct hvgr_mem_area *area,
	uint64_t pages)
{
	long ret;
	uint64_t diff_page = area->pages - pages;
	uint64_t va_offset = (pages << PAGE_SHIFT);

	mutex_lock(&area->mutex);
	area->pages = pages;
	area->phy_pages = pages;
	mutex_unlock(&area->mutex);

	ret = hvgr_mmu_take_down(ctx, (area->gva + va_offset), (uint32_t)diff_page);
	if (ret != 0) {
		area->pages += diff_page;
		area->phy_pages += diff_page;
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize 0x%lx pages %lu take down fail %d",
			ctx->id, area->gva, pages, ret);
		return -ENOMEM;
	}

	hvgr_mem_flush_pt(ctx, (area->gva + va_offset), diff_page, WITH_FLUSH_L2);

	hvgr_mem_pool_ctx_free(ctx, diff_page, (void **)(area->page_array + pages),
		(((area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) != 0 ? HVGR_PAGE_SYNC : 0)),
		area->attribute);

	return 0;
}

long hvgr_mem_resize_expand(struct hvgr_ctx * const ctx, struct hvgr_mem_area *area,
	uint64_t pages)
{
	long ret;
	struct hvgr_mmu_setup_paras setup;
	uint64_t diff_page = pages - area->pages;
	struct page **page_arr = area->page_array + area->pages;

	ret = hvgr_mem_pool_ctx_alloc(ctx, diff_page, (void **)page_arr, area->attribute);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize 0x%lx pages %lu alloc fail %d",
			ctx->id, area->gva, pages, ret);
		return -ENOMEM;
	}

	setup.flags = area->flags;
	setup.gva = area->gva + (area->pages << PAGE_SHIFT);
	setup.pages = diff_page;
	setup.page_array = page_arr;
	setup.mmu_flag = area->mmu_flag;

	ret = hvgr_mmu_set_up(ctx, &setup);
	if (ret != 0) {
		hvgr_mem_pool_ctx_free(ctx, diff_page, (void **)page_arr, 0,
			area->attribute);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize 0x%lx pages %lu map gva fail %d",
			ctx->id, area->gva, pages, ret);
		return -ENOMEM;
	}

	area->pages = pages;
	area->phy_pages = pages;

	hvgr_mem_flush_pt(ctx, area->gva, area->pages, WITHOUT_FLUSH_L2);
	return 0;
}

long hvgr_mem_resize(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t pages)
{
	struct hvgr_mem_area *area = NULL;
	long ret = -EINVAL;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, gva);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize no area for 0x%lx", ctx->id, gva);
		goto fail;
	}

	if (area->property != HVGR_MEM_PROPERTY_GPU_MEM) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize unsupport mem for 0x%lx %lu",
			ctx->id, gva, area->property);
		goto fail;
	}

	/* area is referenced before or mapped on cpu side, can't by resized */
	if (atomic_read(&area->ref_cnt) > 0 ||
		atomic_read(&area->cpu_map_cnt) > 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u can't be resized for 0x%lx", ctx->id, gva);
		goto fail;
	}

	if (pages > area->max_pages) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u resize 0x%lx pages err %lu/%lu",
			ctx->id, gva, pages, area->max_pages);
		goto fail;
	}

	if (pages == area->pages) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		return 0;
	}
	if (pages < area->pages)
		ret = hvgr_mem_resize_shrink(ctx, area, pages);
	else
		ret = hvgr_mem_resize_expand(ctx, area, pages);

	hvgr_info(ctx->gdev, HVGR_MEM,
		"ctx_%u resize mem prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx area to page:%lu %d",
		ctx->id, area->property, area->attribute, area->pages, area->max_pages,
		area->gva, pages, ret);
fail:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return ret;
}

long hvgr_mem_change_mem_attr(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t attr)
{
	struct hvgr_mem_area *area = NULL;
	long ret = -EINVAL;

	if (gva == 0 || attr == 0)
		return -EINVAL;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	area = hvgr_mem_zone_find_area_by_gva(&ctx->mem_ctx, gva);
	if (area == NULL) {
		mutex_unlock(&ctx->mem_ctx.area_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u change attr no area for 0x%lx", ctx->id, gva);
		return -EINVAL;
	}

	/* Until now, only sparse mem and gpu mem can change attribute */
	switch (area->property) {
	case HVGR_MEM_PROPERTY_SPARSE:
		ret = hvgr_sparse_mem_change_attr(ctx, area, attr);
		if (ret != 0)
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u change sparse mem attr fail, attr=%lx:0x%lx, gva=0x%lx, %d",
				ctx->id, attr, area->attribute, gva, ret);
		break;
	case HVGR_MEM_PROPERTY_GPU_MEM:
		ret = hvgr_gpu_mem_change_attr(ctx, area, attr);
		if (ret != 0)
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u change gpu mem attr fail, attr=%lx:0x%lx, gva=0x%lx, %d",
				ctx->id, attr, area->attribute, gva, ret);
		break;
	default:
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u area prop 0x%lx don't support attr change",
			ctx->id, area->property);
		break;
	}

	mutex_unlock(&ctx->mem_ctx.area_mutex);
	return ret;
}

static void hvgr_mem_vm_open(struct vm_area_struct *vma)
{
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_mem_area *area = NULL;

	if (gdev == NULL || vma == NULL)
		return;
	if (vma->vm_private_data == NULL)
		return;

	area = (struct hvgr_mem_area *) vma->vm_private_data;
#ifndef CONFIG_LIBLINUX
	mutex_lock(&area->map_lock);
	area->map_cnt++;
	mutex_unlock(&area->map_lock);
#endif
	hvgr_info(gdev, HVGR_MEM, "vma open 0x%lx, map cnt = %u", vma->vm_start, area->map_cnt);
}

static void hvgr_mem_vm_close(struct vm_area_struct *vma)
{
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_mem_area *area = NULL;
	struct hvgr_ctx *ctx = NULL;
	uint64_t va_start = 0;

	if (gdev == NULL || vma == NULL)
		return;

	if (vma->vm_private_data != NULL) {
		area = (struct hvgr_mem_area *) vma->vm_private_data;
#ifndef CONFIG_LIBLINUX
		mutex_lock(&area->map_lock);
		area->map_cnt--;
		mutex_unlock(&area->map_lock);
#endif
	}
	if (vma->vm_file != NULL && vma->vm_file->private_data != NULL) {
		ctx = (struct hvgr_ctx *) vma->vm_file->private_data;
		va_start = hvgr_mem_get_msb(ctx);
		if ((vma->vm_end - vma->vm_start) == HVGR_4GVA_SIZE_4G &&
			vma->vm_start == (va_start << 32)) {
			hvgr_info(ctx->gdev, HVGR_MEM, "4g va is unmapped ctx_id = %u.", ctx->id);
			hvgr_ctx_flag_set(ctx, HVGR_CTX_FLAG_DYING);
			hvgr_ctx_flag_clear(ctx, HVGR_CTX_FLAG_CFGED);
		}
	}
	hvgr_info(gdev, HVGR_MEM, "vma close  start:0x%lx, end:0x%lx", vma->vm_start, vma->vm_end);
}

#if !defined(CONFIG_LIBLINUX) || defined(CONFIG_LIBLINUX_CDC)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
int hvgr_mem_vm_fault(struct vm_fault *vmf)
{
	int ret;
#else
vm_fault_t hvgr_mem_vm_fault(struct vm_fault *vmf)
{
	vm_fault_t ret;
#endif
	struct hvgr_mem_area *area = NULL;
	struct vm_area_struct *vma = vmf->vma;
	uint32_t page_offset;
	uint64_t insert_addr;
	phys_addr_t phys;
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_mem_ctx *pmem = NULL;

	if (gdev == NULL)
		return VM_FAULT_SIGBUS;

	area = (struct hvgr_mem_area *)vma->vm_private_data;
	if (area == NULL) {
		hvgr_err(gdev, HVGR_MEM, "vm fault addr 0x%lx no area", vmf->address);
		return VM_FAULT_SIGBUS;
	}

	pmem = area->zone->pmem;
	if (pmem->process_mm != current->mm) {
		hvgr_err(gdev, HVGR_MEM, "mm is invalid, area property %lu 0x%lx error",
			area->property, area->attribute);
		return VM_FAULT_SIGBUS;
	}

	hvgr_info(gdev, HVGR_MEM, "vm fault addr 0x%lx, prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
		vmf->address, area->property, area->attribute, area->pages,
		area->max_pages, area->gva);

	if ((area->property != HVGR_MEM_PROPERTY_SAME_VA &&
		area->property != HVGR_MEM_PROPERTY_IMPORT) ||
		(area->attribute & HVGR_MEM_MASK_UMD_ACCESS) == 0) {
		hvgr_err(gdev, HVGR_MEM, "vm fault addr 0x%lx area property %lu 0x%lx error",
			vmf->address, area->property, area->attribute);
		return VM_FAULT_SIGBUS;
	}

	page_offset = (uint32_t)((vmf->address - area->uva) >> PAGE_SHIFT);
	if (page_offset >= area->pages) {
		hvgr_err(gdev, HVGR_MEM, "vm fault addr 0x%lx out of area 0x%lx size 0x%lx",
			vmf->address, area->uva, area->pages);
		return VM_FAULT_SIGBUS;
	}

	mutex_lock(&area->mutex);
	if (atomic_read(&area->import.data.umm.free_tlb_cnt) != 0) {
		hvgr_err(gdev, HVGR_MEM,
			"dma buffer page was freed, do not allow memory growable");
		mutex_unlock(&area->mutex);
		return VM_FAULT_SIGBUS;
	}

	insert_addr = vmf->address & PAGE_MASK;
	while (page_offset < area->pages && insert_addr < vma->vm_end) {
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		if (hvgr_sc_vm_insert_pfn_prot(&area->page_array[page_offset], 1,
			&vma->vm_page_prot) != 0) {
			hvgr_err(gdev, HVGR_MEM,
				"vm fault addr 0x%lx sc insert addr 0x%lx fail",
				vmf->address, insert_addr);
			mutex_unlock(&area->mutex);
			return VM_FAULT_SIGBUS;
		}
#endif
		phys = page_to_phys(area->page_array[page_offset]);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		ret = vm_insert_pfn(vma, insert_addr, PFN_DOWN(phys));
		if (ret < 0 && ret != -EBUSY) {
#else
		ret = vmf_insert_pfn(vma, insert_addr, PFN_DOWN(phys));
		if (ret != VM_FAULT_NOPAGE) {
#endif
			hvgr_err(gdev, HVGR_MEM, "vm fault addr 0x%lx insert addr 0x%lx fail %d",
				vmf->address, insert_addr, (int)ret);
			mutex_unlock(&area->mutex);
			return ret;
		}

		page_offset++;
		insert_addr += PAGE_SIZE;
		gdev->mem_dev.statics.grow_pages++;
	}

	mutex_unlock(&area->mutex);
	return VM_FAULT_NOPAGE;
}
#endif

int hvgr_mem_vm_split(struct vm_area_struct *area, unsigned long addr)
{
	/* not allow split hvgr vma */
	struct hvgr_device *gdev = hvgr_get_device();

	hvgr_err(gdev, HVGR_MEM, "hvgr_mem_vm_split tgid %d pid %d",
		current->tgid, current->pid);

	hvgr_dump_stack();
	return -EINVAL;
}

int hvgr_mem_vm_remap(struct vm_area_struct * area)
{
	/* not allow split hvgr vma */
	struct hvgr_device *gdev = hvgr_get_device();

	hvgr_err(gdev, HVGR_MEM, "hvgr_mem_vm_remap tgid %d pid %d, start:0x%lx, end:0x%lx",
		current->tgid, current->pid, area->vm_start, area->vm_end);
	hvgr_dump_stack();
	return -EINVAL;
}

/* kmd_vm_ops */
const struct vm_operations_struct hvgr_vm_opts = {
	.open  = hvgr_mem_vm_open,
	.close = hvgr_mem_vm_close,
	.fault = hvgr_mem_vm_fault,
	.split = hvgr_mem_vm_split,
	.mremap = hvgr_mem_vm_remap,
};

const struct vm_operations_struct *hvgr_mem_get_vm_opts(void)
{
	return &hvgr_vm_opts;
}

int hvgr_mem_mmap(struct hvgr_ctx * const ctx, struct vm_area_struct *vma)
{
	unsigned int area_index;
	long ret;
	struct hvgr_setup_cpu_mmu_paras paras;
	struct hvgr_mem_area *area = NULL;
	size_t nr_pages = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;

	mutex_lock(&ctx->mem_ctx.mmap_mutex);
	vma->vm_flags |= VM_PFNMAP | VM_DONTCOPY | VM_DONTDUMP | VM_DONTEXPAND | VM_IO;
	vma->vm_flags &= ~((vma->vm_flags & (VM_READ | VM_WRITE)) << 4);

	if (ctx->mem_ctx.process_mm != current->mm) {
		mutex_unlock(&ctx->mem_ctx.mmap_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "mm is invalid, ctx_%d", ctx->id);
		return -1;
	}

	if (nr_pages == 0 || !(vma->vm_flags & VM_SHARED)) {
		mutex_unlock(&ctx->mem_ctx.mmap_mutex);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map error pages %lu vm_flags 0x%lx",
			ctx->id, nr_pages, vma->vm_flags);
		return -1;
	}

	if (vma->vm_pgoff >= PFN_DOWN(HVGR_MEM_BITMAP_BASE_OFFSET)) {
		area_index = (unsigned int)vma->vm_pgoff - PFN_DOWN(HVGR_MEM_BITMAP_BASE_OFFSET);
		if (area_index >= BITS_PER_LONG || ctx->mem_ctx.pend_areas[area_index] == NULL) {
			mutex_unlock(&ctx->mem_ctx.mmap_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map unsupport area index %u",
				ctx->id, area_index);
			return -1;
		}
		hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u map area index %u", ctx->id, area_index);
		area = ctx->mem_ctx.pend_areas[area_index];

		ctx->mem_ctx.pend_areas[area_index] = NULL;
		ctx->mem_ctx.pend_area_bitmap |= (1UL << area_index);

		if (nr_pages != area->max_pages) {
			mutex_unlock(&ctx->mem_ctx.mmap_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u map area pages %lu/%lu out of vma page %lu",
				ctx->id, area->pages, area->max_pages,nr_pages);
			return -1;
		}

		if ((area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) == 0)
			vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

		paras.addr = vma->vm_start;
		paras.page_array = area->page_array;
		paras.page_nums = area->pages;
#ifdef CONFIG_LIBLINUX
		area->vm_page_prot = vma->vm_page_prot;
		paras.vm_page_prot = area->vm_page_prot;
#endif
		if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_DYING)) {
			mutex_unlock(&ctx->mem_ctx.mmap_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u is dying, no need to map cpu page table.", ctx->id);
			return -1;
		}
		ret = hvgr_mem_setup_cpu_mmu(vma, &paras);
		if (ret != 0) {
#ifdef CONFIG_LIBLINUX
			area->vm_page_prot = __pgprot(0);
#endif
			mutex_unlock(&ctx->mem_ctx.mmap_mutex);
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map cpu map fail %d",
				ctx->id, ret);
			return -1;
		}
#ifndef CONFIG_LIBLINUX
		mutex_lock(&area->map_lock);
		area->map_cnt++;
		mutex_unlock(&area->map_lock);
#endif
		area->uva = vma->vm_start;
		vma->vm_private_data = area;
	}

	vma->vm_ops = &hvgr_vm_opts;
	mutex_unlock(&ctx->mem_ctx.mmap_mutex);
	return 0;
}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
long hvgr_mem_sc_get_policy_info(struct hvgr_ctx * const ctx, uint64_t policy_info,
	uint32_t info_num, void **infos)
{
	struct hvgr_mem_sc_policy_info *info = NULL;
	struct hvgr_mem_sc_policy_info *tmp = NULL;
	uint32_t i;

	if (unlikely(ctx == NULL || policy_info == 0 || info_num == 0 ||
		info_num > HVGR_MEM_SC_UPDATE_POLICY_MAX || infos == NULL))
		return -EINVAL;

	if (!hvgr_sc_gid_remap_is_enable()) {
		*infos = NULL;
		return 0;
	}

	info = kcalloc(info_num, sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc sc policy info fail %u",
			ctx->id, info_num);
		goto fail;
	}

	if (copy_from_user(info, u64_to_user_ptr(policy_info), (sizeof(*info) * info_num)) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u copy sc policy info fail", ctx->id);
		goto free_resource;
	}

	for (i = 0; i < info_num; i++) {
		tmp = info + i;
		if (tmp->sc_id < SC_ID_MIN || tmp->sc_id > SC_ID_MAX) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc policy_%u id %u out of range",
				ctx->id, i, tmp->sc_id);
			goto free_resource;
		}

		if ((tmp->gva & (~PAGE_MASK)) != 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc policy_%u addr 0x%lx not page align",
				ctx->id, i, tmp->gva);
			goto free_resource;
		}

		tmp->size = (tmp->size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	}

	*infos = info;

	hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u get sc policy info num %u", ctx->id, info_num);
	return 0;
free_resource:
	kfree(info);
fail:
	*infos = NULL;
	return -EINVAL;
}

void hvgr_mem_sc_update_policy(struct hvgr_ctx * const ctx, void *policy_info,
	uint32_t info_num)
{
	uint32_t i;
	uint32_t area_policy_id;
	uint32_t tmp_policy_id;
	uint64_t mmu_flag;
	struct hvgr_mmu_setup_paras mmu_para;
	struct hvgr_mem_area *sc_area = NULL;
	struct hvgr_mem_sc_policy_info *info = (struct hvgr_mem_sc_policy_info *)policy_info;
	struct hvgr_mem_sc_policy_info *tmp = info;

	if (ctx == NULL || policy_info == NULL || info_num == 0)
		return;

	if (!hvgr_sc_gid_remap_is_enable())
		return;

	mutex_lock(&ctx->mem_ctx.area_mutex);
	for (i = 0; i < info_num; i++) {
		tmp = info + i;
		sc_area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, tmp->gva);
		if (sc_area == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc policy_%u addr 0x%lx no area",
				ctx->id, i, tmp->gva);
			continue;
		}

		if (hvgr_mem_get_sc_id(sc_area->attribute) == 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc policy_%u attr 0x%lx no sc",
				ctx->id, i, sc_area->attribute);
			continue;
		}

		area_policy_id = hvgr_get_policy_id(sc_area->attribute);
		tmp_policy_id = hvgr_sc_id_to_policy_id(tmp->sc_id);
		if (area_policy_id != tmp_policy_id) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u sc policy_%u policy id %u is diff with area %u",
				ctx->id, i, tmp_policy_id, area_policy_id);
			continue;
		}

		if (((sc_area->gva + (sc_area->pages << PAGE_SHIFT)) <
			(tmp->gva + (tmp->size << PAGE_SHIFT))) ||
			(sc_area->gva > tmp->gva) ||
			((sc_area->gva + (sc_area->pages << PAGE_SHIFT)) < tmp->gva) ||
			(sc_area->pages < tmp->size) ||
			(tmp->gva > (U64_MAX - (tmp->size << PAGE_SHIFT)))) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u sc policy_%u addr 0x%lx size 0x%lx out of area 0x%lx:0x%lx",
				ctx->id, i, tmp->gva, tmp->size, sc_area->gva, sc_area->pages);
			continue;
		}

		mmu_flag = sc_area->mmu_flag;
		hvgr_mmu_clear_sc_flag(ctx->gdev, &mmu_flag);
		hvgr_mmu_set_sc_flag(ctx->gdev, &mmu_flag, tmp->sc_id);

		mmu_para.gva = tmp->gva;
		mmu_para.flags = sc_area->flags;
		mmu_para.mmu_flag = mmu_flag;
		mmu_para.pages = tmp->size;
		mmu_para.page_array = sc_area->page_array +
			((tmp->gva - sc_area->gva) >> PAGE_SHIFT);
		if (hvgr_mmu_set_up(ctx, &mmu_para) != 0)
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u sc policy_%u set up mmu fail",
				ctx->id, i);
	}
	mutex_unlock(&ctx->mem_ctx.area_mutex);

	hvgr_info(ctx->gdev, HVGR_MEM, "ctx_%u update sc policy info num %u", ctx->id, info_num);
	kfree(policy_info);
}
#else
long hvgr_mem_sc_get_policy_info(struct hvgr_ctx * const ctx, uint64_t policy_info,
	uint32_t info_num, void **infos)
{
	if (infos != NULL)
		*infos = NULL;
	return 0;
}
void hvgr_mem_sc_update_policy(struct hvgr_ctx * const ctx, void *policy_info,
	uint32_t info_num)
{
}
#endif

long hvgr_mem_vmap_user_addr(struct hvgr_ctx * const ctx, uint64_t gpu_addr,
	void **target_addr, uint64_t *offset)
{
	struct hvgr_mem_area *area = NULL;
	pgprot_t prot = PAGE_KERNEL;

	if (target_addr == NULL || offset == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "input para is invalid!");
		return -EINVAL;
	}

	area = hvgr_mem_zone_find_area_by_gva_range(&ctx->mem_ctx, gpu_addr);
	if (area == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "map user addr no area for gva=0x%lx.", gpu_addr);
		return -EINVAL;
	}

	if (((area->attribute & HVGR_MEM_ATTR_CPU_WR) == 0) ||
		((area->attribute & HVGR_MEM_ATTR_CPU_RD) == 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "map user addr gva=0x%lx %lu:0x%lx attr err.",
			gpu_addr, area->property, area->attribute);
		return -EINVAL;
	}

	if ((gpu_addr < area->gva) || (gpu_addr > (area->gva + (area->pages << PAGE_SHIFT)))) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"gpu_addr 0x%lx area err gva = 0x%lx page_offset = 0x%lx pages = 0x%lx.",
			gpu_addr, area->gva, area->page_offset, area->pages);
		return -EINVAL;
	}

	*offset = gpu_addr - area->gva + (area->page_offset << PAGE_SHIFT);
	prot = pgprot_writecombine(prot);
	*target_addr = vmap(area->page_array, (unsigned int)area->pages, VM_MAP, prot);

	return 0;
}

void hvgr_mem_vunmap_user_addr(const void *gpu_addr)
{
	if (gpu_addr == NULL)
		return;

	vunmap(gpu_addr);
}

