/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_4gva.h"

#include <linux/thread_info.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>

#include "hvgr_kv.h"
#include "hvgr_mem_space.h"
#include "hvgr_mem_zone.h"
#include "hvgr_mem_opts.h"
#include "hvgr_mem_api.h"
#include "hvgr_log_api.h"

/* Reserve 4G va under 4G * HVGR_MEM_4GVA_BASE_MAX */
#define HVGR_MEM_4GVA_BASE_MAX	100

struct hvgr_mem_4gva_cb {
	struct vm_area_struct *vma;
	struct hvgr_mem_space va_space;
	atomic_t refcounter;
};

HVGR_STATIC uint64_t HVGR_WEAK hvgr_mem_4gva_find_reserve(struct hvgr_mem_ctx * const pmem)
{
	unsigned long uva;
	uint32_t idx;
	struct hvgr_ctx *ctx = NULL;

	/* Reserve memory range for 4G VA */
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	for (idx = 1; idx < HVGR_MEM_4GVA_BASE_MAX; idx++) {
		uva = vm_mmap(ctx->kfile, idx * HVGR_4GVA_SIZE_4G,
			HVGR_4GVA_SIZE_4G, PROT_NONE, MAP_SHARED, 0);
		if (IS_ERR_VALUE(uva))
			continue;

		if (uva == idx * HVGR_4GVA_SIZE_4G)
			return uva;

		(void)hvgr_mem_vm_munmap(uva, HVGR_4GVA_SIZE_4G);
	}

	hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4gva find reserve fail", ctx->id);
	return 0;
}

static struct hvgr_mem_4gva_cb *hvgr_mem_4gva_create_reserve(
	struct hvgr_mem_ctx * const pmem)
{
	uint64_t uva;

	struct hvgr_mem_4gva_cb *mem_4gva = kzalloc(sizeof(*mem_4gva), GFP_KERNEL);
	if (unlikely(mem_4gva == NULL))
		return NULL;

	uva = hvgr_mem_4gva_find_reserve(pmem);
	if (uva == 0) {
		kfree(mem_4gva);
		return NULL;
	}

	hvgr_mem_va_check((uva + HVGR_4GVA_SIZE_4G));

	mem_4gva->va_space.start = uva;
	mem_4gva->va_space.size = HVGR_4GVA_SIZE_4G;
	(void)hvgr_mem_space_init(&mem_4gva->va_space);

	return mem_4gva;
}

static struct hvgr_mem_4gva_cb *hvgr_mem_4gva_get_reserve(
	struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_dev *dmem = NULL;
	struct hvgr_mem_4gva_cb *mem_4gva = NULL;
	struct hvgr_kv_node *node = NULL;
	struct hvgr_mem_process_res_cb *res_cb = NULL;

	if (unlikely(pmem == NULL))
		return NULL;

	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	dmem = &ctx->gdev->mem_dev;

	mutex_lock(&dmem->mem_process_res_lock);
	/* Find reserve cb with tgid. */
#ifdef CONFIG_LIBLINUX
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->tgid);
#else
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->mem_ctx.process_mm);
#endif
	if (node == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4gva find reserve cb node fail", ctx->id);
		mutex_unlock(&dmem->mem_process_res_lock);
		return NULL;
	}
	res_cb = container_of(node, struct hvgr_mem_process_res_cb, kv_node);
	mem_4gva = res_cb->mem_4gva;
	if (mem_4gva != NULL) {
		/* 4gva cb is inited before */
		atomic_inc(&mem_4gva->refcounter);
		mutex_unlock(&dmem->mem_process_res_lock);
		return mem_4gva;
	}

	mem_4gva = hvgr_mem_4gva_create_reserve(pmem);
	if (mem_4gva == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4gva create reserve fail", ctx->id);
		mutex_unlock(&dmem->mem_process_res_lock);
		return NULL;
	}
	res_cb->mem_4gva = mem_4gva;
	mutex_unlock(&dmem->mem_process_res_lock);

	return mem_4gva;
}

HVGR_STATIC void HVGR_WEAK hvgr_mem_4gva_put_reserve(struct hvgr_mem_ctx * const pmem,
	struct hvgr_mem_4gva_cb * const mem_4gva)
{
	unsigned long uva;
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_dev *dmem = &ctx->gdev->mem_dev;
	struct hvgr_kv_node *node = NULL;
	struct hvgr_mem_process_res_cb *res_cb = NULL;

	mutex_lock(&dmem->mem_process_res_lock);
	if (atomic_read(&mem_4gva->refcounter) > 0) {
		/* 4gva is inuse */
		atomic_dec(&mem_4gva->refcounter);
		goto out;
	}

	if (hvgr_mem_mm_is_valid(pmem)) {
		uva = mem_4gva->va_space.start;
		(void)hvgr_mem_vm_munmap(uva, HVGR_4GVA_SIZE_4G);
	}

	hvgr_mem_space_term(&mem_4gva->va_space);
	kfree(mem_4gva);

#ifdef CONFIG_LIBLINUX
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->tgid);
#else
	node = hvgr_kv_get(&dmem->mem_process_res_rec, (uint64_t)ctx->mem_ctx.process_mm);
#endif
	if (node == NULL)
		goto out;
	res_cb = container_of(node, struct hvgr_mem_process_res_cb, kv_node);
	res_cb->mem_4gva = NULL;
out:
	mutex_unlock(&dmem->mem_process_res_lock);
}


static bool hvgr_mem_4gva_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	struct hvgr_mem_4gva_cb *mem_4gva = NULL;
	struct hvgr_ctx *ctx = NULL;

	/* Alloc private data. */
	mem_4gva = hvgr_mem_4gva_get_reserve(zone->pmem);
	if (mem_4gva == NULL) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G VA reserve FAIL.", ctx->id);
		return false;
	}

	zone->prv_data = mem_4gva;

	return true;
}

static bool hvgr_mem_4gva_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	if ((area->attribute & HVGR_MEM_MASK_GPU_ACCESS) == 0 ||
		((area->attribute & HVGR_MEM_ATTR_GROWABLE) != 0) ||
		((area->attribute & HVGR_MEM_ATTR_MEM_SPARSE) != 0)) {
		ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4gva attribute INVALID. 0x%lx",
			ctx->id, area->attribute);
		return false;
	}

	area->max_pages = area->pages;

	return true;
}

static struct vm_area_struct *hvgr_mem_4gva_create_vma(struct hvgr_ctx *ctx,
	struct hvgr_mem_4gva_cb *mem_4gva, struct hvgr_mem_area * const area)
{
	struct vm_area_struct *new_vma = NULL;

	new_vma = kzalloc(sizeof(*new_vma), GFP_KERNEL);
	if (unlikely(new_vma == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va alloc new vma fail", ctx->id);
		return NULL;
	}

	new_vma->vm_start = area->uva;
	new_vma->vm_end = new_vma->vm_start + (area->max_pages << PAGE_SHIFT);
	new_vma->vm_flags = (VM_SHARED | VM_DONTCOPY | VM_DONTDUMP | VM_DONTEXPAND | VM_IO |
		VM_PFNMAP);

	if ((area->attribute & HVGR_MEM_ATTR_CPU_RD) != 0)
		new_vma->vm_flags |= (VM_READ | VM_MAYREAD);

	if ((area->attribute & HVGR_MEM_ATTR_CPU_WR) != 0)
		new_vma->vm_flags |= (VM_WRITE | VM_MAYWRITE);

	new_vma->vm_page_prot = vm_get_page_prot(new_vma->vm_flags);

	if ((area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) == 0)
		new_vma->vm_page_prot = pgprot_writecombine(new_vma->vm_page_prot);

#ifndef CONFIG_LIBLINUX
	(void)hvgr_mem_mmap(ctx, new_vma);
#endif
	return new_vma;
}

static void hvgr_mem_4gva_destroy_vma(struct vm_area_struct *vma)
{
	if (vma != NULL)
		kfree(vma);
}

static bool hvgr_mem_4gva_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_mem_4gva_cb *mem_4gva = (struct hvgr_mem_4gva_cb *)zone->prv_data;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);

	if (unlikely(mem_4gva == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va alloc cb is NULL", ctx->id);
		return false;
	}

	if (!hvgr_mem_alloc_pages(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va alloc pages FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		return false;
	}

	/* Alloc memory */
	size = area->max_pages << PAGE_SHIFT;
	if (hvgr_mem_space_split(&mem_4gva->va_space, 0, size, &addr) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va alloc va FAIL. %lu/%lu",
			ctx->id, area->pages, area->max_pages);
		goto split_addr_fail;
	}

	area->gva = addr;
	if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0) {
		area->uva = addr;
		area->vma = hvgr_mem_4gva_create_vma(ctx, mem_4gva, area);
		if (area->vma == NULL)
			goto create_vma_fail;
#ifdef CONFIG_LIBLINUX
		area->vm_page_prot= area->vma->vm_page_prot;
#endif
		if (!hvgr_mem_map_uva(ctx, area)) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u 4G va map uva FAIL. uva = %llx, %lu/%lu",
				ctx->id, area->uva, area->pages, area->max_pages);
			goto map_uva_fail;
		}
	}

	hvgr_mem_attr2flag(area);
	if (!hvgr_mem_map_gva(area)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va map gva FAIL. gva = %llx, %lu/%lu",
			ctx->id, area->gva, area->pages, area->max_pages);
		if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) == 0)
			goto create_vma_fail;
	}
	return true;
map_uva_fail:
	hvgr_mem_4gva_destroy_vma(area->vma);
	area->vma = NULL;
create_vma_fail:
	(void)hvgr_mem_space_joint(&mem_4gva->va_space, addr, size);
split_addr_fail:
	(void)hvgr_mem_free_pages(area);
	return false;
}

static bool hvgr_mem_4gva_free(struct hvgr_mem_zone * const zone,
		struct hvgr_mem_area * const area)
{
	uint64_t addr;
	uint64_t size;
	struct hvgr_ctx *ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	struct hvgr_mem_4gva_cb *mem_4gva = (struct hvgr_mem_4gva_cb *)zone->prv_data;
	int ret = 0;

	if (unlikely(mem_4gva == NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va free cb is NULL", ctx->id);
		/*
		 * when process was killed, mem_4gva is null. user has not chance to free this area.
		 * we have to free the area's pages here called from hvgr_mem_free_leak_area.
		 * hvgr_mem_free_leak_area ignore the 'return false'.
		 */
		hvgr_mem_unmap_gva(area);
		if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0) {
			(void)hvgr_mem_zap_ptes(ctx, area, area->uva,
				(area->max_pages << PAGE_SHIFT), true);
			hvgr_mem_4gva_destroy_vma(area->vma);
			area->vma = NULL;
		}
		(void)hvgr_mem_free_pages(area);
		return false;
	}

	addr = area->gva;
	size = area->max_pages << PAGE_SHIFT;

	hvgr_mem_unmap_gva(area);
	if ((area->attribute & HVGR_MEM_MASK_UMD_ACCESS) != 0) {
		ret = hvgr_mem_zap_ptes(ctx, area, area->uva, (area->max_pages << PAGE_SHIFT),
			true);
		hvgr_mem_4gva_destroy_vma(area->vma);
		area->vma = NULL;
	}

	(void)hvgr_mem_free_pages(area);
	if (ret == -EINTR) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va free (0x%lx:0x%lx) zap pte failed.",
			ctx->id, addr, size);
		return false;
	}

	if (hvgr_mem_space_joint(&mem_4gva->va_space, addr, size) != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 4G va free (0x%lx:0x%lx) space joint fail",
			ctx->id, addr, size);
		return false;
	}

	return true;
}

static bool hvgr_mem_4gva_term(struct hvgr_mem_zone * const zone)
{
	struct hvgr_mem_4gva_cb *mem_4gva = (struct hvgr_mem_4gva_cb *)zone->prv_data;

	if (mem_4gva == NULL)
		return true;

	zone->prv_data = NULL;
	hvgr_mem_4gva_put_reserve(zone->pmem, mem_4gva);
	return true;
}

static const struct hvgr_zone_opts g_mem_zone_4geva = {
	.init = hvgr_mem_4gva_init,
	.term = hvgr_mem_4gva_term,
	.check_para = hvgr_mem_4gva_check_para,
	.alloc = hvgr_mem_4gva_alloc,
	.free = hvgr_mem_4gva_free,
};

bool hvgr_mem_4gva_config(struct hvgr_mem_ctx * const pmem)
{
	if (unlikely(pmem == NULL))
		return false;

	return hvgr_mem_zone_create(pmem, HVGR_MEM_PROPERTY_4G_VA,
		&g_mem_zone_4geva, NULL);
}

void hvgr_mem_4gva_deconfig(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_mem_zone * const zone = &pmem->zones[HVGR_MEM_PROPERTY_4G_VA];
	struct hvgr_mem_4gva_cb *mem_4gva = NULL;
	struct hvgr_ctx *ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	if (!hvgr_kv_is_empty(&pmem->area_rec)) {
		hvgr_debug(ctx->gdev, HVGR_MEM, "ctx_%u free 4g va when defconfig.", ctx->id);
		hvgr_mem_free_area_by_property(ctx, &pmem->area_rec, HVGR_MEM_PROPERTY_4G_VA);
	}

	mem_4gva = (struct hvgr_mem_4gva_cb *)zone->prv_data;
	if (unlikely(mem_4gva == NULL))
		return;

	zone->prv_data = NULL;

	hvgr_mem_4gva_put_reserve(zone->pmem, mem_4gva);
}

uint32_t hvgr_mem_4gva_get_msb(struct hvgr_mem_ctx * const pmem)
{
	struct hvgr_mem_zone *zone = NULL;
	struct hvgr_mem_4gva_cb *mem_4gva = NULL;

	if (unlikely(pmem == NULL))
		return 0;

	zone = hvgr_mem_zone_get_zone(pmem, HVGR_MEM_PROPERTY_4G_VA);
	if (unlikely(zone == NULL))
		return 0;

	mem_4gva = zone->prv_data;
	if (unlikely(mem_4gva == NULL))
		return 0;

	return (uint32_t)(mem_4gva->va_space.start >> 32);
}
