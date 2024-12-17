/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_mem_api.h"

#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/rwsem.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/rcupdate.h>

#include "hvgr_defs.h"
#include "hvgr_dm_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_mem_pool.h"
#include "hvgr_asid_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mem_interval_tree.h"

bool hvgr_mem_opt_init(struct hvgr_mem_zone * const zone,
	void * const para)
{
	return true;
}

bool hvgr_mem_opt_term(struct hvgr_mem_zone * const zone)
{
	return true;
}

bool hvgr_mem_opt_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely((zone == NULL) || (area == NULL)))
		return false;

	ctx = container_of(zone->pmem, struct hvgr_ctx, mem_ctx);
	if (unlikely(area->attribute == 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u attribute INVALID, 0x%lx:0x%lx",
			ctx->id, area->property, area->attribute);
		return false;
	}

	if (area->attribute & HVGR_MEM_ATTR_GROWABLE) {
		if (area->pages > area->max_pages) {
			hvgr_err(ctx->gdev, HVGR_MEM,
				"ctx_%u pages INVALID, 0x%lx:0x%lx 0x%lx:0x%lx",
				ctx->id, area->property, area->attribute,
				area->pages, area->max_pages);
			return false;
		}
	} else {
		area->max_pages = area->pages;
	}

	return true;
}

bool hvgr_mem_opt_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area)
{
	return true;
}

#define HVGR_MEM_DEFAULT_GFP GFP_HIGHUSER

bool hvgr_mem_in_range(uint64_t range_start, uint64_t range_size,
	uint64_t addr, uint64_t size)
{
	uint64_t range_end = range_start + range_size;
	uint64_t addr_end = addr + size;

	return ((addr >= range_start) && (addr < range_end) &&
		(addr_end > range_start) && (addr_end <= range_end));
}

bool hvgr_mem_alloc_pages(struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely(area == NULL))
		return false;

	ctx = container_of(area->zone->pmem, struct hvgr_ctx, mem_ctx);
	if (unlikely(area->page_array != NULL)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u alloc pages but page_array is not NULL",
			ctx->id);
		return false;
	}

	area->gfp = HVGR_MEM_DEFAULT_GFP;
	area->page_array = hvgr_mem_alloc_page_array(ctx, area);
	if (area->page_array == NULL)
		return false;

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_update_map_table(area->attribute);
#endif
	return true;
}

bool hvgr_mem_free_pages(struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;

	if (unlikely(area == NULL || area->page_array == NULL)) {
		struct hvgr_device *gdev = hvgr_get_device();

		hvgr_err(gdev, HVGR_MEM, "free pages para INVALID, area %s NULL",
			(area == NULL ? "is" : "is not"));
		return false;
	}

	/* area is mapped, free page in hvgr_mem_unmap */
	if (atomic_read(&area->ref_cnt) != 0)
		return true;

	ctx = container_of(area->zone->pmem, struct hvgr_ctx, mem_ctx);
	mutex_lock(&area->map_lock);
	if (area->map_cnt == 0) {
		hvgr_mem_free_page_array(ctx, area);
	} else {
		area->page_not_free = true;
		(void)hvgr_kv_add(&area->zone->pmem->free_rec, &area->node);
	}
	mutex_unlock(&area->map_lock);
	return true;
}

#if !defined(CONFIG_LIBLINUX) || defined(CONFIG_LIBLINUX_CDC)
long hvgr_mem_setup_cpu_mmu(struct vm_area_struct *vma, struct hvgr_setup_cpu_mmu_paras *paras)
{
	uint32_t idx;
	int ret;
	phys_addr_t phys;
	unsigned long addr = paras->addr;
	struct hvgr_device *gdev = hvgr_get_device();

	for (idx = 0; idx < paras->page_nums; idx++) {
		if (unlikely(paras->page_array[idx] == NULL)) {
			hvgr_err(gdev, HVGR_MEM, "insert pages page_%u is NULL", idx);
			goto error;
		}
#ifdef HVGR_FEATURE_SYSTEM_CACHE
		ret = hvgr_sc_vm_insert_pfn_prot(&paras->page_array[idx], 1, &vma->vm_page_prot);
		if (ret != 0) {
			hvgr_err(gdev, HVGR_MEM, "insert pages sc insert pfn fail %d", ret);
			goto error;
		}
#endif
		phys = page_to_phys(paras->page_array[idx]);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		ret = vm_insert_pfn(vma, addr, PFN_DOWN(phys));
#else
#ifdef CONFIG_LIBLINUX
		ret = remap_pfn_range(vma, addr, PFN_DOWN(phys),
			PAGE_SIZE, paras->vm_page_prot);
#else
		ret = remap_pfn_range(vma, addr, PFN_DOWN(phys),
			PAGE_SIZE, vma->vm_page_prot);
#endif
#endif
		if (ret != 0) {
			hvgr_err(gdev, HVGR_MEM, "insert pages vm insert pfn fail %d", ret);
			goto error;
		}
		addr += PAGE_SIZE;
	}

	return 0;
error:
	if (idx != 0) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		(void)zap_vma_ptes(vma, paras->addr, PAGE_SIZE * idx);
#else
		zap_vma_ptes(vma, paras->addr, PAGE_SIZE * idx);
#endif
	}
	return -1;
}
#endif

static struct vm_area_struct *hvgr_mem_find_vma(struct hvgr_ctx * const ctx, unsigned long addr,
	uint64_t size)
{
	struct vm_area_struct *vma = NULL;

	vma = find_vma(ctx->mem_ctx.process_mm, addr);
#ifndef CONFIG_LIBLINUX
	if (unlikely(vma == NULL || vma->vm_ops != hvgr_mem_get_vm_opts())) {
		hvgr_err(ctx->gdev, HVGR_MEM, "cant find vma for 0x%lx vma %s null",
			addr, (vma == NULL ? "is" : "is not"));
		return NULL;
	}

	if (!hvgr_mem_in_range(vma->vm_start, vma->vm_end - vma->vm_start,
		addr, size)) {
		hvgr_err(ctx->gdev, HVGR_MEM,
			"addr out of range 0x%lx:0x%lx 0x%lx:0x%lx",
			vma->vm_start, vma->vm_end, addr, size);
		return NULL;
	}
#endif
	return vma;
}

static bool hvgr_mem_is_addr_valid(struct hvgr_ctx *ctx, uint64_t addr)
{
#ifndef CONFIG_LIBLINUX
	struct vm_area_struct *vma = NULL;
	uint64_t va_start = 0;

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_DYING)) {
		hvgr_err(ctx->gdev, HVGR_DM, "ctx_%d is dying, do not allowed insert pages\n", ctx->id);
		return false;
	}

	va_start = hvgr_mem_get_msb(ctx);
	vma = find_vma(current->mm, addr);
	if (vma == NULL || vma->vm_ops != hvgr_mem_get_vm_opts() ||
		(vma->vm_end - vma->vm_start) != HVGR_4GVA_SIZE_4G ||
		vma->vm_start != (va_start << 32)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%d addr is invalid, addr = 0x%lx.", ctx->id, addr);
		return false;
	}
	return true;
#else
	return true;
#endif
}

long hvgr_mem_insert_pages(struct hvgr_ctx * const ctx, struct hvgr_mem_area *area,
	struct hvgr_setup_cpu_mmu_paras *paras)
{
	long ret;
	struct vm_area_struct *vma = area->vma;

	if (unlikely(((paras == NULL) || paras->addr == 0) || (paras->page_array == NULL))) {
		hvgr_err(ctx->gdev, HVGR_MEM, "insert pages para invaild.");
		return -1;
	}

	if (vma == NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "vma is invalid");
		return -1;
	}

	ret = hvgr_mem_mm_write_lock_killable(current->mm);
	if (unlikely(ret != 0)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "insert pages mm lock fail %d", ret);
		return -1;
	}
	if (ctx->mem_ctx.process_mm != current->mm) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx process mm is invalid");
		ret = -1;
		goto out;
	}

	if (unlikely(!hvgr_mem_is_addr_valid(ctx, paras->addr))) {
		ret = -1;
		goto out;
	}
	rcu_read_lock();
	vma->vm_mm = rcu_dereference(ctx->mem_ctx.process_mm);
	rcu_read_unlock();

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_DYING)) {
		ret = -1;
		goto out;
	}
	ret = hvgr_mem_setup_cpu_mmu(vma, paras);
out:
	hvgr_mem_mm_write_unlock(current->mm);
	return ret;
}

static int hvgr_mem_get_pend_area_index(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area)
{
	int index;

	if (ctx->mem_ctx.pend_area_bitmap == 0u) {
		hvgr_err(ctx->gdev, HVGR_MEM, "no bit for pend area");
		return -1;
	}

	index = (int)__ffs(ctx->mem_ctx.pend_area_bitmap);
	ctx->mem_ctx.pend_area_bitmap &= ~(1ul << index);
	if (ctx->mem_ctx.pend_areas[index] != NULL) {
		hvgr_err(ctx->gdev, HVGR_MEM, "bit_%d is not null", index);
		return -1;
	}

	ctx->mem_ctx.pend_areas[index] = area;
	return index;
}

uint64_t hvgr_mem_alloc_va(struct hvgr_mem_area * const area, bool need_map)
{
	unsigned long addr;
	unsigned long prot = PROT_NONE;
	unsigned long size;
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx = NULL;
	unsigned long pg_offset = 0;
	int area_index;

	if (unlikely((area == NULL) || (area->zone == NULL))) {
		struct hvgr_device *gdev = hvgr_get_device();

		hvgr_err(gdev, HVGR_MEM, "alloc va para INVALID");
		return 0;
	}

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);
	size = area->max_pages << PAGE_SHIFT;

	if (need_map) {
		if ((area->attribute & HVGR_MEM_ATTR_CPU_RD) != 0)
			prot |= PROT_READ;
		if ((area->attribute & HVGR_MEM_ATTR_CPU_WR) != 0)
			prot |= PROT_WRITE;

		area_index = hvgr_mem_get_pend_area_index(ctx, area);
		if (area_index < 0) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map va fail no bitmap", ctx->id);
			return 0;
		}

		pg_offset = (unsigned long)area_index +
			(unsigned long)PFN_DOWN(HVGR_MEM_BITMAP_BASE_OFFSET);
		pg_offset <<= 12;
		area->uva = 0;
	}

	addr = vm_mmap(ctx->kfile, 0, size, prot, MAP_SHARED, pg_offset);
	if (IS_ERR_VALUE(addr) || !hvgr_mem_uva_check_valid(ctx, addr, size)) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u map va fail errno=%d prot=0x%lx size=0x%lx",
			ctx->id, addr, prot, size);
		if (need_map) {
			mutex_lock(&ctx->mem_ctx.mmap_mutex);
			ctx->mem_ctx.pend_areas[area_index] = NULL;
			ctx->mem_ctx.pend_area_bitmap |= (1ul << area_index);
			mutex_unlock(&ctx->mem_ctx.mmap_mutex);
		}
		/*
		 * When vm_mmap and user-mode mmap are executed concurrently, mmap mapping may
		 * succeed first. As a result, vm_mmap mapping fails. In this case, we need to
		 * delete the page table created by mmap.
		 */
		if (area->uva != 0) {
			(void)vm_munmap(area->uva, area->max_pages << PAGE_SHIFT);
#ifdef CONFIG_LIBLINUX
			area->vm_page_prot = __pgprot(0);
#endif
		}
		return 0;
	}

	if (hvgr_ctx_flag(ctx, HVGR_CTX_FLAG_COMPAT)) {
		if (addr > (1UL << 32))
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u 32bit get va 0x%llx out of 4G",
				ctx->id, addr);
	}

	hvgr_mem_va_check((addr + size));
	return (uint64_t)addr;
}

bool hvgr_mem_free_va(struct hvgr_mem_area * const area)
{
	size_t len;
	unsigned long addr;
	struct hvgr_device *gdev = hvgr_get_device();
	struct hvgr_mem_ctx *pmem = NULL;

	if (unlikely((area == NULL))) {
		hvgr_err(gdev, HVGR_MEM, "free va para INVALID, area is NULL");
		return false;
	}

	len = area->max_pages << PAGE_SHIFT;
	addr = area->uva;
	if (addr == 0u)
		addr = area->gva;
	if (unlikely(addr == 0u)) {
		hvgr_err(gdev, HVGR_MEM,
			"free va area prot=%lu:0x%lx, page:%lu/%lu uva/gva=0x%lx",
			area->property, area->attribute, area->pages,
			area->max_pages, area->gva);
		return true;
	}

	pmem = area->zone->pmem;

	if (hvgr_mem_mm_is_valid(pmem))
		(void)hvgr_mem_vm_munmap(addr, len);

	return true;
}

bool hvgr_mem_map_kva(struct hvgr_mem_area * const area)
{
	void *addr = NULL;
	struct hvgr_ctx *ctx = NULL;
	pgprot_t prot = PAGE_KERNEL;
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	int ret;
#endif
	if ((area->attribute & HVGR_MEM_ATTR_CPU_CACHEABLE) == 0)
		prot = pgprot_writecombine(prot);

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	ret = hvgr_sc_vm_insert_pfn_prot(area->page_array, area->pages, &prot);
	WARN_ON((ret != 0));
#endif
	addr = vmap(area->page_array, (uint32_t)area->pages, VM_MAP, prot);
	if (addr == NULL) {
		ctx = container_of(area->zone->pmem, struct hvgr_ctx, mem_ctx);
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u vmap kva fail", ctx->id);
		udelay(1);
		addr = vmap(area->page_array, (uint32_t)area->pages, VM_MAP, prot);
		if (addr == NULL) {
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u vmap kva fail", ctx->id);
			return false;
		}
	}

	area->kva = ptr_to_u64(addr);
	atomic_inc(&area->cpu_map_cnt);
	return true;
}

void hvgr_mem_unmap_kva(struct hvgr_mem_area * const area)
{
	if (area->kva != 0) {
		vunmap(u64_to_ptr(area->kva));
		atomic_dec(&area->cpu_map_cnt);
	}
}

void hvgr_mem_flush_pt(struct hvgr_ctx * const ctx, uint64_t gva, uint64_t pages, uint32_t flag)
{
	long ret;
	unsigned long flags;
	bool ctx_is_in_runpool = false;

	spin_lock_irqsave(&ctx->gdev->cq_dev.schedule_lock, flags);
	ctx_is_in_runpool = hvgr_cq_retain_active_ctx(ctx->gdev, ctx);
	if (!ctx_is_in_runpool) {
		spin_unlock_irqrestore(&ctx->gdev->cq_dev.schedule_lock, flags);
		return;
	}

	ret = hvgr_mmu_hal_flush_pt(ctx, gva, (uint32_t)pages);
	if (ret != 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%d flush pt 0x%lx 0x%lx fail %d",
			ctx->id, gva, pages, ret);
	if (flag == WITH_FLUSH_L2)
		hvgr_mmu_flush_l2_caches(ctx->gdev);

	hvgr_cq_sched_release_ctx_without_lock(ctx->gdev, ctx);
	spin_unlock_irqrestore(&ctx->gdev->cq_dev.schedule_lock, flags);
}

bool hvgr_mem_map_gva(struct hvgr_mem_area * const area)
{
	long ret;
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_mmu_setup_paras mmu_para = {0};

	if (unlikely((area == NULL) || (area->zone == NULL) ||
		(area->zone->pmem == NULL) || (area->gva == 0))) {
		struct hvgr_device *gdev = hvgr_get_device();

		hvgr_err(gdev, HVGR_MEM, "map gva para INVALID");
		return false;
	}

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	mmu_para.gva = area->gva;
	mmu_para.page_array = area->page_array;
	mmu_para.pages = area->pages;
	mmu_para.mmu_flag = area->mmu_flag;
	mmu_para.flags = area->flags;

	ret = hvgr_mmu_set_up(ctx, &mmu_para);
	if (ret != 0) {
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%u setup mmu fail ret=%d", ctx->id, ret);
		return false;
	}

	hvgr_mem_flush_pt(ctx, area->gva, area->pages, WITHOUT_FLUSH_L2);
	return true;
}

void hvgr_mem_unmap_gva(struct hvgr_mem_area * const area)
{
	struct hvgr_ctx *ctx = NULL;
	struct hvgr_mem_ctx *pmem = NULL;

	if (unlikely((area == NULL) || (area->zone == NULL) ||
		(area->zone->pmem == NULL) || (area->gva == 0))) {
		struct hvgr_device *gdev = hvgr_get_device();

		hvgr_err(gdev, HVGR_MEM, "unmap gva para INVALID");
		return;
	}

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	(void)hvgr_mmu_take_down(ctx, area->gva, (uint32_t)area->pages);
	hvgr_mem_flush_pt(ctx, area->gva, area->pages, WITH_FLUSH_L2);
}

bool hvgr_mem_map_uva(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area)
{
	struct hvgr_setup_cpu_mmu_paras paras;

	if (unlikely(area == NULL))
		return false;

	paras.addr = area->uva;
	paras.page_array = area->page_array;
	paras.page_nums = area->pages;
#ifdef CONFIG_LIBLINUX
	paras.vm_page_prot = area->vm_page_prot;
#endif
	if (hvgr_mem_insert_pages(ctx, area, &paras) != 0)
		return false;
#ifndef CONFIG_LIBLINUX
	mutex_lock(&area->map_lock);
	area->map_cnt++;
	mutex_unlock(&area->map_lock);
#endif
	return true;
}

long hvgr_mem_zap_ptes(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	unsigned long address, unsigned long size, bool is_free)
{
	long ret = 0;
	struct vm_area_struct *vma = area->vma;

	hvgr_mem_mm_write_lock(ctx->mem_ctx.process_mm);
	if (vma == NULL) {
		vma = hvgr_mem_find_vma(ctx, address, size);
		if (unlikely(vma == NULL)) {
			hvgr_mem_mm_write_unlock(ctx->mem_ctx.process_mm);
			hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%d zap ptes cant find vma for 0x%lx",
				ctx->id, address);
			return -EINVAL;
		}
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	ret = zap_vma_ptes(vma, address, size);
#else
	zap_vma_ptes(vma, address, size);
#endif
#ifndef CONFIG_LIBLINUX
	if (is_free) {
		mutex_lock(&area->map_lock);
		area->map_cnt--;
		mutex_unlock(&area->map_lock);
	}
#endif
	hvgr_mem_mm_write_unlock(ctx->mem_ctx.process_mm);
	if (ret != 0)
		hvgr_err(ctx->gdev, HVGR_MEM, "ctx_%d zap ptes fail %d 0x%lx 0x%lx 0x%lx",
			ctx->id, ret, vma->vm_start, vma->vm_end, vma->vm_flags);

	return ret;
}

void hvgr_mem_attr2flag(struct hvgr_mem_area * const area)
{
	struct hvgr_mem_ctx *pmem = NULL;
	struct hvgr_ctx *ctx = NULL;

	pmem = area->zone->pmem;
	ctx = container_of(pmem, struct hvgr_ctx, mem_ctx);

	area->mmu_flag = hvgr_mmu_get_flags(ctx->gdev, area->attribute);
}

bool hvgr_mem_mm_is_valid(struct hvgr_mem_ctx * const pmem)
{
	bool ret = false;

	rcu_read_lock();
	if (rcu_dereference(pmem->process_mm) == current->mm &&
		current->mm != NULL)
		ret = true;
	rcu_read_unlock();

	return ret;
}

int hvgr_mem_vm_munmap(unsigned long addr, size_t len)
{
	int ret;
	struct hvgr_device *gdev;

	gdev = hvgr_get_device();
	ret = vm_munmap(addr, len);
	if (ret != 0)
		hvgr_err(gdev, HVGR_MEM, "vm_munmap fail addr=0x%lx len=0x%lx ret %d",
			addr, len, ret);

	return ret;
}
