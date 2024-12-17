/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SCH_POLICY_MANAGER_H
#define HVGR_SCH_POLICY_MANAGER_H

#include <linux/types.h>
#include "hvgr_sch_policy_default.h"

struct hvgr_cq_ctx;
struct hvgr_device;
struct hvgr_cq_dev;

struct hvgr_sch_policy_opts {
	struct hvgr_cq_ctx * (*get_queue) (struct hvgr_device * const gdev);
	uint32_t (*get_channel)(struct hvgr_cq_dev * const gcqctx,
		struct hvgr_cq_ctx * const cq_ctx);
};

void hvgr_sch_policy_manager_init(struct hvgr_cq_dev * const gcqctx);

struct hvgr_cq_ctx *hvgr_cq_get_available_queue(struct hvgr_device * const gdev);

uint32_t hvgr_cq_get_channel(struct hvgr_cq_dev * const gcqctx,
	struct hvgr_cq_ctx * const cq_ctx);

void hvgr_sch_policy_init(struct hvgr_cq_dev * const gcqctx);
#endif
