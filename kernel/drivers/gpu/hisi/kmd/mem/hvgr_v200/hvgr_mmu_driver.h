/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MMU_DRIVER_H
#define HVGR_MMU_DRIVER_H

#include <linux/kernel.h>
#include "hvgr_regmap.h"
#include "hvgr_mmu_api.h"

struct hvgr_mmu_cfg;

#define MMU_BUS_FAULT_OFFSET 0x8u
#define MMU_FAULT_MASK 0xFFu
#define mmu_bus_fault_bit(as) ((uint32_t)1u << ((as) + MMU_BUS_FAULT_OFFSET))
#define FREE_AS_MASK          ((u16)0x003f)

char *hvgr_mmu_other_fault_process(struct hvgr_ctx *ctx, struct hvgr_mmu_as *mmu_as);

void hvgr_mmu_hal_cfg_transcfg(struct hvgr_ctx *ctx, uint64_t *trans_val);

void hvgr_mmu_hal_cfg_lock(struct hvgr_ctx *ctx, uint64_t gva, uint32_t pages);

void hvgr_mmu_hal_get_enable_cfg(struct hvgr_ctx *ctx, struct hvgr_mmu_cfg *mmu_cfg);

void hvgr_mmu_hal_get_disable_cfg(struct hvgr_mmu_cfg *mmu_cfg);

void hvgr_mmu_hal_get_mode_cfg(struct hvgr_device *gdev, uint32_t *value);

void hvgr_mmu_hal_flush_l2_caches(struct hvgr_device *gdev);

static inline uint32_t hvgr_mmu_get_fault_adrmode(uint32_t fault_stat)
{
	return 0u;
}

static inline uint32_t hvgr_mmu_get_mem_info(uint32_t fault_stat)
{
	return 0u;
}

static inline bool hvgr_mmu_is_trans_fault(uint32_t fault_stat)
{
	uint32_t fault_code = (fault_stat >> AS_EXCEPTION_CODE_OFFSET) & AS_EXCEPTION_CODE_MASK;
	return fault_code == AS_EXCEPTION_CODE_TRANSLATION_FAULT;
}

static inline void hvgr_mmu_get_fault(uint32_t stat, uint32_t *page_fault, uint32_t *bus_fault)
{
	*bus_fault = (stat >> MMU_BUS_FAULT_OFFSET) & MMU_FAULT_MASK;
	*page_fault = (stat & MMU_FAULT_MASK) & (~(*bus_fault));
}

static inline bool hvgr_mmu_flush_l2_is_needed(void)
{
	return true;
}

#endif
