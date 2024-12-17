/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_H
#define HVGR_MMU_H

#include <linux/workqueue.h>

#include "hvgr_version.h"

struct hvgr_device;
struct hvgr_ctx;

#define HVGR_AS_NR_INVALID     (-1)
#define MMU_SC_POLICY_MASK     ((uint64_t)0x780003C000000000)
#define MMU_ADDR_SPACE_MAX 8

#if hvgr_version_ge(HVGR_V350)
#define MMU_IRQ_MASK_DISABLE_ALL        0x0U
#define MMU_IRQ_MASK_ENABLE_ALL         0x7U
#else
#define MMU_IRQ_MASK_DISABLE_ALL        0x0U
#define MMU_IRQ_MASK_ENABLE_ALL         0xFFFFFFFFU
#endif

struct hvgr_mmu_setup_paras {
	uint64_t gva;
	struct page **page_array;
	uint64_t pages;
	/* MMU page table attr bits */
	uint64_t mmu_flag;
	/* soft define flags */
	uint64_t flags;
};

struct hvgr_mmu_as {
	uint32_t no;
	uint32_t fault_type;
	uint32_t fault_stat;
	uint64_t fault_addr;
	bool protected_mode;

	struct hvgr_ctx *ctx;
	struct workqueue_struct *fault_wq;
	struct work_struct pf_work;
	struct work_struct bf_work;
};

#if hvgr_version_ge(HVGR_V350)
struct hvgr_mmu_fault_worker {
	bool protected_mode;
	struct workqueue_struct *fault_wq;
	struct work_struct cmd_sync_work;
	struct work_struct eventq_work;
};
#endif

/*
 * hvgr_mmu_set_up() - set up MMU page tables for va with flag.
 * @ctx: Pointer to context.
 * @paras: The set up paras.
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_set_up(struct hvgr_ctx *ctx, struct hvgr_mmu_setup_paras *paras);

/*
 * hvgr_mmu_take_down() - take down MMU page tables for va in size pages.
 * @ctx: Pointer to context.
 * @va: The va in Bytes.
 * @pages: Page number
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_take_down(struct hvgr_ctx * const ctx, uint64_t va, uint32_t pages);

/*
 * hvgr_mmu_init_proc() - Init proc mmu source.
 * @ctx: Pointer to context.
 */
long hvgr_mmu_init_proc(struct hvgr_ctx * const ctx);

/*
 * hvgr_mmu_term_proc() - Deinit proc mmu source.
 * @ctx: Pointer to context.
 */
void hvgr_mmu_term_proc(struct hvgr_ctx * const ctx);

/*
 * hvgr_mmu_flush_tlb() - Flush page tables.
 * @ctx: Pointer to context.
 */
void hvgr_mmu_flush_tlb(struct hvgr_ctx * const ctx);

/*
 * hvgr_mmu_hal_unlock() - Unlock the address space.
 * @ctx: Pointer to context.
 * @asid: address space id
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_hal_unlock(struct hvgr_ctx *ctx, uint32_t asid);

/*
 * hvgr_mmu_hal_flush_pt() - Flush mmu for gva in size pages.
 * @ctx: Pointer to context.
 * @gva: The gpu va.
 * @pages: Page number
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_hal_flush_pt(struct hvgr_ctx *ctx, uint64_t gva, uint32_t pages);

/*
 * hvgr_mmu_hal_enable() - Enable the address space configure for ctx.
 * @ctx: Pointer to context.
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_hal_enable(struct hvgr_ctx *ctx);

/*
 * hvgr_mmu_hal_disable() - Disable the address space configure for ctx.
 * @ctx: Pointer to context.
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_hal_disable(struct hvgr_ctx *ctx);

/*
 * hvgr_mmu_irq_init() - Register mmu irq.
 * @gdev: Pointer to hvgr_dev.
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_irq_init(struct hvgr_device * const gdev);

/*
 * hvgr_mmu_term() - Destory mmu resource.
 * @gdev: Pointer to hvgr_dev.
 */
void hvgr_mmu_term(struct hvgr_device * const gdev);

/*
 * hvgr_mmu_dump() - Dump mmu page table for ctx.
 * @ctx: Pointer to context.
 * @file_path: The file full path, the caller make sure it is legal.
 * @is_fcp: Whether this call is for a pagetable dump for fcp or not.
 * Return: 0 for success and other for err.
 */
long hvgr_mmu_dump(struct hvgr_ctx * const ctx, char *file_path, bool is_fcp);

/*
 * hvgr_alloc_asid() - Alloc a free as.
 * @ctx: Pointer to context.
 * Return: As no.
 */
uint32_t hvgr_alloc_asid(struct hvgr_ctx * const ctx);

/*
 * hvgr_free_asid() - Free an as.
 * @ctx: Pointer to context.
 * @asid: The as to be freed.
 */
void hvgr_free_asid(struct hvgr_ctx * const ctx, uint32_t asid);

/*
 * hvgr_mmu_get_flags() - Get mmu flag by attr.
 * @gdev: Pointer to hvgr_device.
 * @attr: Attribute.
 * Return: Mmu flag.
 */
uint64_t hvgr_mmu_get_flags(struct hvgr_device *gdev, uint64_t attr);

/*
 * hvgr_mmu_flush_l2_caches() -Flush all L2 cache.
 * @ctx: Pointer to hvgr_device.
 */
void hvgr_mmu_flush_l2_caches(struct hvgr_device *gdev);

/*
 * hvgr_mmu_cbit_config() -Config mmu mode: GID or PBHA.
 * @gdev: Pointer to hvgr_device.
 */
void hvgr_mmu_cbit_config(struct hvgr_device *gdev);

#ifdef HVGR_FEATURE_SYSTEM_CACHE
/*
 * hvgr_mmu_clear_sc_flag() -Clear the sc idx in flag.
 * @gdev: Pointer to hvgr_device.
 * @flag: Pointer to flag.
 */
void hvgr_mmu_clear_sc_flag(struct hvgr_device *gdev, uint64_t *flag);

/*
 * hvgr_mmu_set_sc_flag() -Set the sc idx to flag.
 * @gdev: Pointer to hvgr_device.
 * @flag: Pointer to flag.
 * @sc_id: sc index
 */
void hvgr_mmu_set_sc_flag(struct hvgr_device *gdev, uint64_t *flag, uint32_t sc_id);

#endif

int hvgr_mmu_init(struct hvgr_device * const gdev);

#endif
