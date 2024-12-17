/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DATAN_CT_INNER_H
#define HVGR_DATAN_CT_INNER_H

#include "hvgr_defs.h" // For struct hvgr_device and struct hvgr_ctx
#include "hvgr_datan_ct_config.h" // For hvgr_ct_setup

int hvgr_datan_ct_enable_inner(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx, struct hvgr_ct_setup *setup);

int hvgr_datan_ct_disable_inner(struct hvgr_ctx *ctx);

int hvgr_datan_ct_dump_inner(struct hvgr_ctx *ctx);

int hvgr_datan_ct_wait_for_dump_inner(struct hvgr_ctx *ctx);

int hvgr_datan_ct_clear_status_inner(struct hvgr_device *gdev);

int hvgr_datan_ct_init_status_inner(struct hvgr_device *gdev);

#endif // HVGR_DATAN_CT_INNER_H
