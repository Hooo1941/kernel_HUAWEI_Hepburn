/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_DRIVER_ADAPT_H
#define HVGR_CQ_DRIVER_ADAPT_H

#include <linux/types.h>
#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "hvgr_cq.h"

static inline uint32_t hvgr_get_bind_status(struct hvgr_device *gdev, uint32_t chn)
{
	return hvgr_read_reg(gdev, cq_control_reg(gdev, cq_bind_status_0(chn)));
}

static inline void hvgr_set_send_mode(struct hvgr_device *gdev)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	gcqctx->hw_cfg.bc_send_mode = CQ_BC_SEND_INTERLEAVE;
}

#define CQ_SEM_MEM_PROPERTY         HVGR_MEM_PROPERTY_SAME_VA
#define CQ_IRQ_MASK_VALUE           0xfffffc0f
#define CQ_BIND_TIME_OUT_VALUE      65535
#define CQ_TIMESLICE_VALUE          3000000

#define CQ_ENTRY_TIMEOUT_SWITCH     false
#define CQ_EMPTY_TIMEOUT_IN_FPGA    4000     /* 20MHZ */
#define CQ_EMPTY_TIMEOUT_IN_CHIP    55000    /* 300MHZ~750MHZ */

enum hvgr_cq_subchn_index {
	RENDER_LOW_IDX = 0,
	RENDER_HIGH_IDX,
	BIN_LOW_IDX,
	BIN_HIGH_IDX,
	COMPUTE_LOW_IDX,
	COMPUTE_HIGH_IDX,
	BVHG_IDX,
	RT_IDX,
	SUBCHN_MAX_IDX,
};

#endif

