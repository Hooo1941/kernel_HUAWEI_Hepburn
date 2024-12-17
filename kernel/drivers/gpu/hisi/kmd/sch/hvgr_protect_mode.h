/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_PROTECT_MODE_H
#define HVGR_PROTECT_MODE_H

#include <linux/types.h>
#include "hvgr_defs.h"
#include "sch/cq/hvgr_cq_data.h"

bool hvgr_get_prot_enhance(struct hvgr_device * const gdev);

bool hvgr_mem_protect_jcd_pool_init(struct hvgr_device * const gdev);

bool hvgr_mem_protect_jcd_pool_term(struct hvgr_device * const gdev);

int hvgr_mem_protect_jcd_init(struct hvgr_ctx * const ctx);

void hvgr_mem_protect_jcd_term(struct hvgr_ctx * const ctx);

irqreturn_t hvgr_job_irq_handler(int irq, void *data);

void hvgr_protect_mode_disable_interrupts(struct hvgr_device *gdev);

void *hvgr_cq_vmap_user_addr(struct hvgr_ctx * const ctx, uint64_t gpu_addr);

void hvgr_cq_protect_mode_check_running(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t channel_id, enum protect_flag flag);

void hvgr_cq_schedule_channel_running_proc(struct hvgr_device * const gdev,
	struct hvgr_cq_ctx * const cq_ctx, uint32_t channel_id, enum protect_flag flag,
	bool need_stop);

void hvgr_cq_protect_mode_check_stopped(struct hvgr_cq_ctx * const cq_ctx,
	struct hvgr_device *gdev, uint32_t channel_id);

void hvgr_cq_protect_mode_enter(struct hvgr_cq_ctx * const cq_ctx, struct hvgr_device *gdev,
	uint32_t channel_id);
#endif
