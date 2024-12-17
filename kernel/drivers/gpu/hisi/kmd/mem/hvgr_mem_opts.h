/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_OPTS_H
#define HVGR_MEM_OPTS_H

#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/version.h>

#include "hvgr_memory_data.h"
#include "hvgr_ioctl_mem.h"

#define HVGR_MEM_MASK_KMD_ACCESS (HVGR_MEM_ATTR_KMD_ACCESS)
#define HVGR_MEM_MASK_UMD_ACCESS \
	(HVGR_MEM_ATTR_CPU_RD | HVGR_MEM_ATTR_CPU_WR)
#define HVGR_MEM_MASK_GPU_ACCESS \
	(HVGR_MEM_ATTR_GPU_RD | HVGR_MEM_ATTR_GPU_WR | HVGR_MEM_ATTR_GPU_EXEC)
#define HVGR_MEM_MASK_COHERENT \
	(HVGR_MEM_ATTR_COHERENT_SYSTEM | HVGR_MEM_ATTR_COHERENT_LOCAL | \
	HVGR_MEM_ATTR_COHERENT_SYS_REQ)

#define HVGR_MEM_BITMAP_BASE_OFFSET ((u64)4u << 12)

struct hvgr_setup_cpu_mmu_paras {
	unsigned long addr;
	struct page **page_array;
	uint64_t page_nums;
#ifdef CONFIG_LIBLINUX
	pgprot_t vm_page_prot;
#endif
};

/**
 * Follow functions are default operations of zone implementation.
 */
bool hvgr_mem_opt_init(struct hvgr_mem_zone * const zone,
	void * const para);

bool hvgr_mem_opt_term(struct hvgr_mem_zone * const zone);

bool hvgr_mem_opt_check_para(struct hvgr_mem_zone *const zone,
	struct hvgr_mem_area * const area);

bool hvgr_mem_opt_alloc(struct hvgr_mem_zone * const zone,
	struct hvgr_mem_area * const area);

/**
 * Follow functions are general operations of zone module.
 */

bool hvgr_mem_alloc_pages(struct hvgr_mem_area * const area);

bool hvgr_mem_free_pages(struct hvgr_mem_area * const area);

uint64_t hvgr_mem_alloc_va(struct hvgr_mem_area * const area, bool need_map);

bool hvgr_mem_free_va(struct hvgr_mem_area * const area);

bool hvgr_mem_map_kva(struct hvgr_mem_area * const area);

void hvgr_mem_unmap_kva(struct hvgr_mem_area * const area);

bool hvgr_mem_map_gva(struct hvgr_mem_area * const area);

void hvgr_mem_unmap_gva(struct hvgr_mem_area * const area);

bool hvgr_mem_map_uva(struct hvgr_ctx *ctx, struct hvgr_mem_area * const area);

long hvgr_mem_insert_pages(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	struct hvgr_setup_cpu_mmu_paras *paras);

bool hvgr_mem_in_range(uint64_t range_start, uint64_t range_size,
	uint64_t addr, uint64_t size);

void hvgr_mem_attr2flag(struct hvgr_mem_area * const area);

long hvgr_mem_zap_ptes(struct hvgr_ctx *ctx, struct hvgr_mem_area *area,
	unsigned long address, unsigned long size, bool is_free);

long hvgr_mem_setup_cpu_mmu(struct vm_area_struct *vma, struct hvgr_setup_cpu_mmu_paras *paras);

bool hvgr_mem_mm_is_valid(struct hvgr_mem_ctx * const pmem);

static inline int hvgr_mem_mm_write_lock_killable(struct mm_struct *mm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	return down_write_killable(&mm->mmap_sem);
#else
	return down_write_killable(&mm->mmap_lock);
#endif
}

static inline void hvgr_mem_mm_write_lock(struct mm_struct *mm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	down_write(&mm->mmap_sem);
#else
	down_write(&mm->mmap_lock);
#endif
}

static inline void hvgr_mem_mm_write_unlock(struct mm_struct *mm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	up_write(&mm->mmap_sem);
#else
	up_write(&mm->mmap_lock);
#endif
}

static inline void hvgr_mem_mm_read_lock(struct mm_struct *mm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	down_read(&mm->mmap_sem);
#else
	down_read(&mm->mmap_lock);
#endif
}

static inline void hvgr_mem_mm_read_unlock(struct mm_struct *mm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	up_read(&mm->mmap_sem);
#else
	up_read(&mm->mmap_lock);
#endif
}

static inline void hvgr_mem_va_check(unsigned long va)
{
	WARN_ON((va >= ((unsigned long)1u << 39)));
}

#endif
