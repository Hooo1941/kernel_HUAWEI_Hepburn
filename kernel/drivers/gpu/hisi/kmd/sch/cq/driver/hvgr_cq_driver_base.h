/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_DRIVER_BASE_H
#define HVGR_CQ_DRIVER_BASE_H

#include <linux/interrupt.h>

#include "hvgr_defs.h"
#include "hvgr_cq_data.h"
#include "hvgr_mem_api.h"

#define ORDER_DUMP      true
#define SYNC_DUMP       false

struct hvgr_cq_prioriy_cfg {
	uint8_t pri;
	uint8_t weight;
	uint8_t real;
	uint8_t rev;
};

bool hvgr_cq_is_append_exception_flag(struct hvgr_cq_ctx *cq_ctx);

void hvgr_cq_compute_affinity_init(struct hvgr_device * const gdev,
	uint32_t channel_id, uint32_t core_mask);

void hvgr_cq_set_affinity(struct hvgr_device * const gdev,
	uint32_t channel_id, uint32_t core_mask);

void hvgr_cq_config_bc_send_mode(struct hvgr_device *gdev);

bool hvgr_is_cl_task(struct hvgr_device *gdev, uint32_t channel_id);

void hvgr_cq_bind_physical_chan(struct hvgr_cq_ctx * const cq_ctx, uint32_t channel_id);

void hvgr_cq_irq_do_stop(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id);

void hvgr_cq_hwcfg_init(struct hvgr_device * const gdev, uint32_t mode);

bool hvgr_cq_get_empty_flag(struct hvgr_device *gdev, uint32_t channel_id);

bool hvgr_cq_get_wait_flag(struct hvgr_device *gdev, uint32_t channel_id);

void hvgr_cq_schedule_do_channel_irq(struct hvgr_device * const gdev, uint32_t chn);

void hvgr_get_priority_cfg(uint32_t pri, uint32_t *weight, uint32_t *real);

int hvgr_cq_register_irq(struct hvgr_device * const gdev);

void hvgr_cq_int_operate(struct hvgr_device * const gdev, uint32_t irq_mask);

void hvgr_cq_dump_debug_register(struct hvgr_device * const gdev);

void hvgr_cq_alloc_sw_dfx_memory(struct hvgr_ctx * const ctx,
	struct cq_sw_writeable_context *sw_wr_ctx);

void hvgr_cq_free_sw_dfx_memory(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx);

#ifdef CONFIG_DFX_DEBUG_FS
void hvgr_cq_ordersync_entry_dump(struct hvgr_device * const gdev,
	struct hvgr_cq_ctx * const cq_ctx, bool is_order, bool is_full);
#endif

#endif
