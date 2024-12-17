/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_import.h"

#include <linux/kernel.h>
#include <linux/pfn.h>
#include <linux/dma-buf.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched/mm.h>
#include <linux/rcupdate.h>
#ifdef CONFIG_BG_DMABUF_RECLAIM
#include <linux/bg_dmabuf_reclaim/bg_dmabuf_ops.h>
#endif
#include "hvgr_mem_zone.h"
#include "hvgr_mem_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mem_space.h"

struct hvgr_mem_import_cb {
	struct hvgr_mem_space va_space;
	uint64_t va_start;
};

HVGR_STATIC struct hvgr_mem_import_cb * HVGR_WEAK hvgr_mem_import_get_reserve(
	struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_import_cb *mem_import = NULL;

	mem_import = kzalloc(sizeof(*mem_import), GFP_KERNEL);
	if (unlikely(mem_import == NULL))
		return NULL;

	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	mem_import->va_space.start = HVGR_MEM_IMPORT_START;
	mem_import->va_space.size = HVGR_MEM_IMPORT_SIZE;

	if (hvgr_mem_space_init(&mem_import->va_space) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import init space fail", ctx->id);
		kfree(mem_import);
		return NULL;
	}

	return mem_import;
}

static bool hvgr_mem_import_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	struct hvgr_mem_import_cb *mem_import = NULL;
	struct hvgr_ctx *ctx = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		mem_import = hvgr_mem_import_get_reserve(zone->pmem);
		if (mem_import == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u IMPORT VA init FAIL.", ctx->id);
			return false;
		}
		zone->prv_data = mem_import;
	}

	return true;
}

static bool hvgr_mem_import_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_import_cb *mem_import = NULL;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	mem_import = (struct hvgr_mem_import_cb *)zone->prv_data;

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (mem_import == NULL)
			return true;
		hvgr_mem_space_term(&mem_import->va_space);
		zone->prv_data = NULL;
		kfree(mem_import);
	}

	return true;
}

static uint64_t hvgr_mem_import_reserve_va_alloc(struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_import_cb *mem_import = NULL;
	int ret;

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	size = area->max_pages << PAGE_SHIFT;

	mem_import = (struct hvgr_mem_import_cb *)area->zone->prv_data;
	if (unlikely(mem_import == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u mem_import prv is null", ctx->id);
		return 0;
	}

	ret = hvgr_mem_space_split(&mem_import->va_space, (uint64_t)0, size, &addr);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import alloc va FAIL. %lu/%lu %d",
			ctx->id, area->pages, area->max_pages, ret);
		return 0;
	}

	return addr;
}

uint64_t hvgr_mem_import_alloc_va(struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_ctx *pmem = NULL;

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		return hvgr_mem_import_reserve_va_alloc(area);

	if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0)
		return hvgr_mem_alloc_va(area, true);
	else
		return hvgr_mem_alloc_va(area, false);
}

void hvgr_add_bg_dmabuf_ctx_to_global_list(struct hvgr_ctx * const ctx)
{
	struct hvgr_dmabuf_ctx_rec *ctx_rec = NULL;

	mutex_lock(&ctx->gdev->mem_dev.dmabuf_rec_lock);
	mutex_lock(&ctx->mem_ctx.area_mutex);
	if (ctx->mem_ctx.bd_dmabuf_flag)
		goto out;

	if (ctx->mem_ctx.bg_dmabuf_cnt == 1) {
		ctx_rec = kzalloc(sizeof(*ctx_rec), GFP_KERNEL);
		if (ctx_rec == NULL)
			goto out;

		ctx->mem_ctx.bd_dmabuf_flag = true;
		ctx_rec->ctx = ctx;
		ctx->mem_ctx.dmabuf_ctx = ctx_rec;
		list_add_tail(&ctx_rec->node, &ctx->gdev->mem_dev.dmabuf_ctx_list_head);
	}
out:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	mutex_unlock(&ctx->gdev->mem_dev.dmabuf_rec_lock);
}

void hvgr_del_bg_dmabuf_ctx_from_global_list(struct hvgr_ctx * const ctx)
{
	mutex_lock(&ctx->gdev->mem_dev.dmabuf_rec_lock);
	mutex_lock(&ctx->mem_ctx.area_mutex);
	if (!ctx->mem_ctx.bd_dmabuf_flag)
		goto out;

	if (ctx->mem_ctx.bg_dmabuf_cnt == 0) {
		list_del(&ctx->mem_ctx.dmabuf_ctx->node);
		kvfree(ctx->mem_ctx.dmabuf_ctx);
		ctx->mem_ctx.dmabuf_ctx = NULL;
		ctx->mem_ctx.bd_dmabuf_flag = false;
	}
out:
	mutex_unlock(&ctx->mem_ctx.area_mutex);
	mutex_unlock(&ctx->gdev->mem_dev.dmabuf_rec_lock);
}

static bool hvgr_mem_import_map(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area)
{
	hvgr_mem_attr2flag(area);
	if (!hvgr_mem_map_gva(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import alloc map gva fail, uva/gva=0x%lx",
			ctx->id, area->gva);
		return false;
	}

	return true;
}

static bool hvgr_mem_import_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (atomic_read(&area->import.data.umm.free_tlb_cnt) != 0)
		return false;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	area->import.data.umm.mapped = false;
	if (!hvgr_mem_import_map(ctx, area))
		return false;

	area->import.data.umm.mapped = true;
	return true;
}

static void hvgr_mem_import_umm_free_pages(struct hvgr_mem_area * const area)
{
	uint32_t i;
	struct hvgr_mem_import_record *record = NULL;
	struct hvgr_mem_ctx *pmem = NULL;

	for (i = 0; i < area->import.data.umm.cnt; i++) {
		record = &area->import.data.umm.record[i];
		if (record->dmabuf_attach != NULL && record->sg_table != NULL) {
			dma_buf_unmap_attachment(record->dmabuf_attach, record->sg_table,
				DMA_BIDIRECTIONAL);
			dma_buf_detach(record->dmabuf, record->dmabuf_attach);
			record->dmabuf_attach = NULL;
			record->sg_table = NULL;
		}

		dma_buf_put(record->dmabuf);
		record->dmabuf = NULL;
		if (record->dma_buf_rec != NULL) {
			pmem = area->zone->pmem;
			list_del(&record->dma_buf_rec->dma_buf_list);
			kvfree(record->dma_buf_rec);
			if (pmem->bg_dmabuf_cnt)
				pmem->bg_dmabuf_cnt--;
			if (atomic_read(&area->import.data.umm.free_tlb_cnt) > 0)
				atomic_dec(&area->import.data.umm.free_tlb_cnt);
			if (atomic_read(&area->import.data.umm.restore_tlb_cnt) > 0)
				atomic_dec(&area->import.data.umm.restore_tlb_cnt);
			record->dma_buf_rec = NULL;
		}
	}

	kvfree(area->page_array);
}

static void hvgr_mem_import_reserve_va_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	int ret;
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mem_import_cb *mem_import = NULL;
	struct hvgr_ctx *ctx = NULL;

	pmem = zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	mem_import = (struct hvgr_mem_import_cb *)zone->prv_data;
	if (unlikely(mem_import == NULL)) {
		 hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import prv is null", ctx->id);
		 return;
	}

	addr = area->gva;
	size = area->max_pages << PAGE_SHIFT;
	ret = hvgr_mem_space_joint(&mem_import->va_space, addr, size);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import free va FAIL. 0x%lx %lu/%lu %d",
			ctx->id, addr, area->pages, area->max_pages, ret);
		return;
	}
}

static bool hvgr_mem_import_umm_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx = NULL;

	pmem = zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	if (area->import.data.umm.mapped && (atomic_read(&area->import.data.umm.free_tlb_cnt) == 0))
		hvgr_mem_unmap_gva(area);

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT))
		hvgr_mem_import_reserve_va_free(zone, area);
	else
		(void)hvgr_mem_free_va(area);

	mutex_lock(&area->map_lock);
	if (area->map_cnt == 0) {
		if (atomic_read(&area->ref_cnt) == 0)
			hvgr_mem_import_umm_free_pages(area);
	} else {
		area->page_not_free = true;
		(void)hvgr_kv_add(&area->zone->pmem->free_rec, &area->node);
	}
	mutex_unlock(&area->map_lock);

	return true;
}

static bool hvgr_mem_import_free(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	return hvgr_mem_import_umm_free(zone, area);
}

static uint32_t hvgr_mem_import_insert_pages(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	struct sg_table *sg_table, uint64_t need_pages, uint32_t start_page_nr)
{
	struct scatterlist *sg = NULL;
	uint32_t i;
	uint64_t page_cnt = 0;
	uint32_t pages;

	pages = start_page_nr;
	for_each_sg(sg_table->sgl, sg, sg_table->nents, i) {
		uint32_t j;
		uint32_t page_num = PFN_UP(sg_dma_len(sg));
		struct page *page = NULL;
		struct page **page_arr = NULL;

		WARN_ONCE(sg_dma_len(sg) & (u32)(PAGE_SIZE - 1),
			"dma len %u is not a multiple of PAGE_SIZE\n", sg_dma_len(sg));

		WARN_ONCE(sg_dma_address(sg) & (u32)(PAGE_SIZE - 1),
			"dma addr %llx is not aligned to PAGE_SIZE\n",
			(unsigned long long)sg_dma_address(sg));

		page = sg_page(sg);
		page_arr = &area->page_array[pages];
		for (j = 0; (j < page_num) && (page_cnt < need_pages); j++, page_cnt++) {
			page_arr[j] = &page[j];
			pages++;
		}
	}
	return (pages - start_page_nr);
}

#ifdef CONFIG_BG_DMABUF_RECLAIM
static void hvgr_record_dma_buf(struct hvgr_ctx *ctx, struct dma_buf *dmabuf,
	uint32_t start_page_nr, struct hvgr_mem_area *area)
{
	struct hvgr_dma_buf_record *dma_buf_rec = NULL;

	dma_buf_rec = kzalloc(sizeof(*dma_buf_rec), GFP_KERNEL);
	if (dma_buf_rec == NULL)
		return;

	dma_buf_rec->dmabuf = dmabuf;
	dma_buf_rec->area = area;
	dma_buf_rec->ctx = ctx;
	dma_buf_rec->start_page_nr = start_page_nr;
	dma_buf_rec->page_nr = (uint32_t)(area->pages - start_page_nr);
	dma_buf_rec->record_idx = area->import.data.umm.cnt;
	area->import.data.umm.record[area->import.data.umm.cnt].dma_buf_rec = dma_buf_rec;
	list_add_tail(&dma_buf_rec->dma_buf_list, &ctx->mem_ctx.dma_buf_rec_head);
	ctx->mem_ctx.bg_dmabuf_cnt++;
}
#endif

static bool hvgr_mem_import_get_pages(struct hvgr_ctx *ctx, struct hvgr_mem_area *area, int fd)
{
	struct dma_buf *dmabuf = NULL;
	struct dma_buf_attachment *dmabuf_attach = NULL;
	struct sg_table *sg_table = NULL;
	struct hvgr_mem_import_record *import_record = NULL;
	uint64_t pages;
	uint32_t insert_pages;
#ifdef CONFIG_BG_DMABUF_RECLAIM
	uint32_t start_page_nr;
#endif

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import 0x%lx get buf for fd %d fail",
			ctx->id, area->gva, fd);
		goto get_buf_fail;
	}

	dmabuf_attach = dma_buf_attach(dmabuf, ctx->gdev->dev);
	if (IS_ERR_OR_NULL(dmabuf_attach)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import 0x%lx attach buf for fd %d fail",
			ctx->id, area->gva, fd);
		goto attach_fail;
	}

	pages = (PAGE_ALIGN(dmabuf->size) >> PAGE_SHIFT);
	if (pages == 0 || (pages + area->pages) > area->max_pages) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import 0x%lx fd %d pages 0x%lx 0x%lx out of range (0x%lx:0x%lx)",
			ctx->id, area->gva, fd, pages, (u64)dmabuf->size, area->pages, area->max_pages);
		goto size_out_of_range;
	}

	sg_table = dma_buf_map_attachment(dmabuf_attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(sg_table)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import 0x%lx fd %d map fail",
			ctx->id, area->gva, fd);
		goto map_fail;
	}
#ifdef CONFIG_BG_DMABUF_RECLAIM
	start_page_nr = (uint32_t)area->pages;
#endif
	insert_pages = hvgr_mem_import_insert_pages(ctx, area, sg_table, pages,
		(uint32_t)area->pages);
	area->pages += insert_pages;

	import_record = &(area->import.data.umm.record[area->import.data.umm.cnt]);
	import_record->sg_table = sg_table;
	import_record->dmabuf_attach = dmabuf_attach;
	import_record->dmabuf = dmabuf;
#ifdef CONFIG_BG_DMABUF_RECLAIM
	if (is_bg_dmabuf(dmabuf))
		hvgr_record_dma_buf(ctx, dmabuf, start_page_nr, area);
#endif
	area->import.data.umm.cnt++;

	area->import.type = HVGR_MEM_IMPORT_UMM;
	return true;
map_fail:
size_out_of_range:
	dma_buf_detach(dmabuf, dmabuf_attach);
attach_fail:
	dma_buf_put(dmabuf);
get_buf_fail:
	return false;
}

static bool hvgr_mem_import_import(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area, int fd)
{
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (area->import.data.umm.cnt >= HVGR_MEM_IMPORT_MAX) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u import 0x%lx cnts out of range",
			ctx->id, area->gva);
		return false;
	}

	return hvgr_mem_import_get_pages(ctx, area, fd);
}

bool hvgr_mem_import_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	if (area->import.type != HVGR_MEM_IMPORT_UMM || area->pages != area->max_pages) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"ctx_%u import attribute INVALID. type:%u 0x%lx:0x%lx",
			ctx->id, area->import.type, area->pages, area->max_pages);
		return false;
	}

	return true;
}

static const struct hvgr_zone_opts g_mem_zone_import = {
	.init = hvgr_mem_import_init,
	.term = hvgr_mem_import_term,
	.check_para = hvgr_mem_import_check_para,
	.alloc = hvgr_mem_import_alloc,
	.free = hvgr_mem_import_free,
	.import = hvgr_mem_import_import,
};

bool hvgr_mem_import_config(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_IMPORT,
		&g_mem_zone_import, NULL);
}

void hvgr_mem_import_free_pages(struct hvgr_mem_area *area)
{
	if (area->import.type == HVGR_MEM_IMPORT_UMM)
		hvgr_mem_import_umm_free_pages(area);
}

void hvgr_mem_import_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_zone * const zone = &pmem->zones[HVGR_MEM_PROPERTY_IMPORT];
	struct hvgr_mem_import_cb *mem_import = NULL;

	mem_import = (struct hvgr_mem_import_cb *)zone->prv_data;
	if (unlikely(mem_import == NULL))
		return;

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		zone->prv_data = NULL;
		hvgr_mem_space_term(&mem_import->va_space);
		kfree(mem_import);
	}
}

static bool hvgr_mem_is_dmabuf_rec_invalid(struct dma_buf *dmabuf,
	struct hvgr_dma_buf_record *dmabuf_rec)
{
	struct hvgr_mem_import_record *record = NULL;
	struct hvgr_mem_area *area = NULL;

	area = dmabuf_rec->area;
	if (unlikely(dmabuf_rec->record_idx >= HVGR_MEM_IMPORT_MAX)) {
		hvgr_err(dmabuf_rec->ctx->gdev, HVGR_MEM, "record idx is invalid, record idx = %d",
			dmabuf_rec->record_idx);
		return false;
	}

	record = &area->import.data.umm.record[dmabuf_rec->record_idx];
	if (record->dmabuf != dmabuf) {
		hvgr_err(dmabuf_rec->ctx->gdev, HVGR_MEM, "record dmabuf is invalid, record idx = %d",
			dmabuf_rec->record_idx);
		return false;
	}

	return true;
}

static bool hvgr_mem_release_tlb_by_dmabuf_rec(struct hvgr_ctx *ctx,
	struct hvgr_dma_buf_record *dmabuf_rec, struct dma_buf *dmabuf)
{
	struct hvgr_mem_import_record *record = NULL;
	struct hvgr_mem_area *area = NULL;

	/* Check dmabuf is invalid */
	if (!hvgr_mem_is_dmabuf_rec_invalid(dmabuf, dmabuf_rec))
		return false;

	area = dmabuf_rec->area;
	if (atomic_read(&area->ref_cnt) != 0)
		return false;

	record = &area->import.data.umm.record[dmabuf_rec->record_idx];
	hvgr_info(ctx->gdev, HVGR_MEM, "%s ctx_%d", __func__, ctx->id);
	/* Delete cpu and gpu tlb */
	if (atomic_inc_return(&area->import.data.umm.free_tlb_cnt) == 1) {
		hvgr_mem_unmap_gva(area);
		if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0) {
			ctx->mem_ctx.ion_flag = true;
			if(unlikely(hvgr_mem_zap_ptes(ctx, area, area->uva,
				(area->max_pages << PAGE_SHIFT), false) != 0)) {
				ctx->mem_ctx.ion_flag = false;
				hvgr_mem_map_gva(area);
				return false;
			}

			ctx->mem_ctx.ion_flag = false;
		}
	}

	/* Detachment sg table */
	dma_buf_unmap_attachment(record->dmabuf_attach, record->sg_table,
		DMA_BIDIRECTIONAL);
	dma_buf_detach(record->dmabuf, record->dmabuf_attach);
	record->sg_table = NULL;
	record->dmabuf_attach = NULL;
	return true;
}

static void hvgr_mem_rollback_dmabuf_tlb(struct hvgr_ctx *ctx,
	struct hvgr_dma_buf_record *dmabuf_rec, struct dma_buf *dmabuf)
{
	struct hvgr_mem_import_record *record = NULL;
	struct hvgr_mem_area *area = NULL;

	/* Check dmabuf is invalid */
	if (!hvgr_mem_is_dmabuf_rec_invalid(dmabuf, dmabuf_rec))
		return;

	area = dmabuf_rec->area;

	record = &area->import.data.umm.record[dmabuf_rec->record_idx];
	if (record->dmabuf_attach != NULL || record->sg_table != NULL)
		return;

	record->dmabuf_attach = dma_buf_attach(dmabuf, ctx->gdev->dev);
	record->sg_table = dma_buf_map_attachment(record->dmabuf_attach, DMA_BIDIRECTIONAL);

	hvgr_info(ctx->gdev, HVGR_MEM, "%s ctx_%d", __func__, ctx->id);
	/* Delete cpu and gpu tlb */
	if (area->import.data.umm.mapped && (atomic_read(&area->import.data.umm.free_tlb_cnt) == 1))
		hvgr_mem_map_gva(area);

	atomic_dec(&area->import.data.umm.free_tlb_cnt);
	return;
}

static int hvgr_mem_dmabuf_restore_pages(struct dma_buf *dmabuf,
	struct hvgr_dma_buf_record *dmabuf_rec)
{
	struct dma_buf_attachment *dmabuf_attach = NULL;
	struct sg_table *sg_table = NULL;
	uint64_t pages;
	struct hvgr_mem_import_record *record = NULL;
	struct hvgr_mem_area *area = NULL;

	area = dmabuf_rec->area;
	record = &area->import.data.umm.record[dmabuf_rec->record_idx];
	/*
	 * if sg_table or dmabuf_attach is not null, it means the dmabuffer not released,
	 * abort restore.
	 */
	if (record->sg_table != NULL || record->dmabuf_attach != NULL)
		return -1;

	pages = (PAGE_ALIGN(dmabuf->size) >> PAGE_SHIFT);
	if (pages < dmabuf_rec->page_nr) {
		hvgr_err(dmabuf_rec->ctx->gdev, HVGR_MEM,
			"ctx_%u restore tlb dma buf size[%d] is smaller than needed[%d]",
			dmabuf_rec->ctx->id, pages, dmabuf_rec->page_nr);
		return -1;
	}

	dmabuf_attach = dma_buf_attach(dmabuf, dmabuf_rec->ctx->gdev->dev);
	if (IS_ERR_OR_NULL(dmabuf_attach)) {
		hvgr_err(dmabuf_rec->ctx->gdev, HVGR_MEM, "ctx_%u restore tlb 0x%lx attach buf fail",
			dmabuf_rec->ctx->id, dmabuf_rec->area->gva);
		return -1;
	}

	sg_table = dma_buf_map_attachment(dmabuf_attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(sg_table)) {
		hvgr_err(dmabuf_rec->ctx->gdev, HVGR_MEM, "ctx_%u restore tlb 0x%lx map fail",
			dmabuf_rec->ctx->id, dmabuf_rec->area->gva);
		dma_buf_detach(dmabuf, dmabuf_attach);
		return -1;
	}
	(void)hvgr_mem_import_insert_pages(dmabuf_rec->ctx, dmabuf_rec->area, sg_table,
		dmabuf_rec->page_nr, dmabuf_rec->start_page_nr);

	record->sg_table = sg_table;
	record->dmabuf_attach = dmabuf_attach;
	atomic_inc(&area->import.data.umm.restore_tlb_cnt);

	return 0;
}

static void hvgr_mem_restore_tlb_by_dmabuf_rec(struct hvgr_ctx *ctx,
	struct hvgr_dma_buf_record *dmabuf_rec, struct dma_buf *dmabuf)
{
	struct hvgr_mem_area *area = NULL;
	/* Check dmabuf is invalid */
	if (!hvgr_mem_is_dmabuf_rec_invalid(dmabuf, dmabuf_rec))
		return;

	area = dmabuf_rec->area;

	hvgr_info(ctx->gdev, HVGR_MEM, "%s ctx_%d", __func__, ctx->id);
	/* Get new pages from dma buf */
	if (hvgr_mem_dmabuf_restore_pages(dmabuf, dmabuf_rec) != 0)
		return;

	/* Restore cpu and gpu tlb */
	if (atomic_read(&area->import.data.umm.restore_tlb_cnt) ==
		atomic_read(&area->import.data.umm.free_tlb_cnt)) {
		if (area->import.data.umm.mapped) {
			ctx->mem_ctx.ion_flag = true;
			(void)hvgr_mem_import_map(ctx, area);
			ctx->mem_ctx.ion_flag = false;
		}

		atomic_set(&area->import.data.umm.free_tlb_cnt, 0);
		atomic_set(&area->import.data.umm.restore_tlb_cnt, 0);
	}
}

static bool hvgr_mem_dmabuf_area_operation(struct hvgr_device *gdev, struct dma_buf *dmabuf,
	uint32_t oper)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_dev *mem_dev = NULL;
	struct hvgr_dmabuf_ctx_rec *iter = NULL;
	struct hvgr_dmabuf_ctx_rec *tmp = NULL;

	struct hvgr_dma_buf_record *iter_dmabuf = NULL;
	struct hvgr_dma_buf_record *tmp_dmabuf  = NULL;

	mem_dev = &gdev->mem_dev;
	list_for_each_entry_safe(iter, tmp, &mem_dev->dmabuf_ctx_list_head, node) {
		ctx = iter->ctx;
		if (iter->ctx == NULL)
			continue;
		mutex_lock(&ctx->mem_ctx.area_mutex);
		if (list_empty(&ctx->mem_ctx.dma_buf_rec_head)) {
			mutex_unlock(&ctx->mem_ctx.area_mutex);
			continue;
		}
		list_for_each_entry_safe(iter_dmabuf, tmp_dmabuf, &ctx->mem_ctx.dma_buf_rec_head,
			dma_buf_list) {
			if (iter_dmabuf->dmabuf != dmabuf)
				continue;

			switch(oper) {
			case HVGR_DMA_RELEASE_TLB:
				if (!hvgr_mem_release_tlb_by_dmabuf_rec(ctx, iter_dmabuf, dmabuf)) {
					mutex_unlock(&ctx->mem_ctx.area_mutex);
					return false;
				}
				break;
			case HVGR_DMA_ROLLBACK_TLB:
				hvgr_mem_rollback_dmabuf_tlb(ctx, iter_dmabuf, dmabuf);
				break;
			case HVGR_DMA_RESTORE_TLB:
				hvgr_mem_restore_tlb_by_dmabuf_rec(ctx, iter_dmabuf, dmabuf);
				break;
			default:
				break;
			}
		}
		mutex_unlock(&ctx->mem_ctx.area_mutex);
	}
	return true;
}

int hvgr_mem_release_dmabuf_mmu_tlb(struct dma_buf *dmabuf)
{
	struct hvgr_device *gdev = NULL;

	gdev = hvgr_get_device();
	if (gdev == NULL)
		return -EFAULT;

	mutex_lock(&gdev->mem_dev.dmabuf_rec_lock);
	if (list_empty(&gdev->mem_dev.dmabuf_ctx_list_head))
		goto out;

	if (!hvgr_mem_dmabuf_area_operation(gdev, dmabuf, HVGR_DMA_RELEASE_TLB))
		goto fail;

out:
	mutex_unlock(&gdev->mem_dev.dmabuf_rec_lock);
	return 0;
fail:
	/*
	 * delete tlb fail, recover the tlb which has been deleted and return -EFAULT. so the
	 * physical will not be freed.
	 */
	hvgr_mem_dmabuf_area_operation(gdev, dmabuf, HVGR_DMA_ROLLBACK_TLB);
	mutex_unlock(&gdev->mem_dev.dmabuf_rec_lock);
	return -EFAULT;
}

int hvgr_mem_restore_dmabuf_mmu_tlb(struct dma_buf *dmabuf)
{
	struct hvgr_device *gdev = NULL;

	gdev = hvgr_get_device();
	if (gdev == NULL)
		return -EFAULT;

	mutex_lock(&gdev->mem_dev.dmabuf_rec_lock);
	if (list_empty(&gdev->mem_dev.dmabuf_ctx_list_head))
		goto out;
	hvgr_mem_dmabuf_area_operation(gdev, dmabuf, HVGR_DMA_RESTORE_TLB);
out:
	mutex_unlock(&gdev->mem_dev.dmabuf_rec_lock);
	return 0;
}

void hvgr_ion_mem_reclaim_init(void)
{
#ifdef CONFIG_BG_DMABUF_RECLAIM
	(void)bg_buffer_register_callback(BG_BUF_CB_GPU_UNMAP, hvgr_mem_release_dmabuf_mmu_tlb);
	(void)bg_buffer_register_callback(BG_BUF_CB_GPU_MAP, hvgr_mem_restore_dmabuf_mmu_tlb);
#endif
}
