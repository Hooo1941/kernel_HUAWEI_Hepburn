/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DATAN_CT_HAL_H
#define HVGR_DATAN_CT_HAL_H

#include "hvgr_defs.h"

#define OFFSET_32BIT         32
#define DUMP_WAIT_MAX_TIME   1000000 /* max wait 1s. */

struct hvgr_datan_ct_inner_config {
	u32 dump_buffer_base_low;
	u32 dump_buffer_base_high;
	u32 fcm_enable;
	u32 gpc_enable;
	u32 mmu_enable;
	u32 btc_enable;
	u32 bvh_enable;
	u32 dump_mode;
	u32 set_id;
};

void hvgr_datan_ct_open_config(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	struct hvgr_datan_ct_inner_config *config);

void hvgr_datan_ct_close_config(struct hvgr_device *gdev);

void hvgr_datan_ct_dump_data(struct hvgr_device *gdev);

void hvgr_datan_ct_irq_enable(struct hvgr_device *gdev);

void hvgr_datan_ct_irq_disable(struct hvgr_device *gdev);

#endif // HVGR_DATAN_CT_HAL_H
