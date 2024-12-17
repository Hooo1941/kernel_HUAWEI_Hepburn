/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_ASID_API_H
#define HVGR_ASID_API_H

#include "hvgr_defs.h"

/*
 * hvgr_ctx_sched_in() - set ctx in schedule.
 * @ctx: Pointer to context.
 *
 *  Return: true for success and false for fail.
 */
bool hvgr_ctx_sched_in(struct hvgr_ctx * const ctx);

/*
 * hvgr_ctx_sched_out() - set ctx out schedule..
 * @ctx: Pointer to context.
 *
 */
void hvgr_ctx_sched_out(struct hvgr_ctx * const ctx);

/*
 * hvgr_cq_retain_active_ctx() - ctx is active.
 * @gdev: Pointer to hvgr device.
 * @ctx: Pointer to context.
 * Return: True for active.
 */
bool hvgr_cq_retain_active_ctx(struct hvgr_device * const gdev,
	struct hvgr_ctx * const ctx);

/*
 * hvgr_cq_sched_release_ctx_without_lock() - release ctx.
 * @gdev: Pointer to hvgr device.
 * @ctx: Pointer to context.
 */
void hvgr_cq_sched_release_ctx_without_lock(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx);

/*
 * hvgr_cq_lookup_ctx() - find ctx by asid.
 * @gdev: Pointer to hvgr device.
 * @asid: The as no.
 * Return: Ctx or null.
 */
struct hvgr_ctx *hvgr_cq_lookup_ctx(struct hvgr_device *gdev, int asid);

/*
 * hvgr_cq_sched_release_ctx() - release ctx.
 * @gdev: Pointer to hvgr device.
 * @ctx: Pointer to context.
 */
void hvgr_cq_sched_release_ctx(struct hvgr_device *gdev, struct hvgr_ctx *ctx);

/**
 * hvgr_ctx_is_asid_valid - check as id valid.
 *
 * @asid: as id.
 *
 */
bool hvgr_ctx_is_asid_valid(uint32_t asid);

#endif
