/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SCH_API_H
#define HVGR_SCH_API_H

#include "hvgr_defs.h"

/**
 * hvgr_sch_init - Schedule init when device create.
 *
 * @gdev: Device pointer
 *
 */
int hvgr_sch_init(struct hvgr_device * const gdev);

/**
 * hvgr_sch_term - Schedule term at device remove.
 *
 * @gdev: Device pointer
 *
 */
void hvgr_sch_term(struct hvgr_device * const gdev);

/**
 * hvgr_sch_ctx_create - Schedule init at context create
 *
 * @ctx: The hvgr context to schedule module init to
 *
 * Return: 0 on success or error code
 */
long hvgr_sch_ctx_create(struct hvgr_ctx * const ctx);

/**
 * hvgr_sch_ctx_destroy - Schedule term at context destroy
 *
 * @ctx: The hvgr context to schedule module destroy to
 *
 */
void hvgr_sch_ctx_destroy(struct hvgr_ctx * const ctx);

/**
 * hvgr_sch_cmd_dispatch - Dispatch command to backend schedule
 *
 * @ctx: The hvgr context to dispatch to
 * @cmd: command type
 * @arg: The address in user space of the schedule para
 *
 * Return: 0 on success or error code
 */
long hvgr_sch_cmd_dispatch(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg);

/**
 * hvgr_sch_suspend - Suspend schedule
 *
 * @gdev: Device pointer
 *
 * Return: 0 on success or error code
 */
int hvgr_sch_suspend(struct hvgr_device *gdev);

/**
 * hvgr_sch_resume - Resume schedule
 *
 * @gdev: Device pointer
 *
 */
void hvgr_sch_resume(struct hvgr_device *gdev);

/**
 * hvgr_sch_start - Start schedule
 *
 * @gdev: Device pointer
 *
 */
void hvgr_sch_start(struct hvgr_device * const gdev);

/**
 * hvgr_sch_stop - Stop schedule, but it will not stop the running queue.
 *
 * @gdev: Device pointer
 *
 */
void hvgr_sch_stop(struct hvgr_device * const gdev);

/**
 * @ctx: The hvgr context to dispatch to
 *
 * @gdev: Device pointer
 *
 */
void hvgr_ctx_stop(struct hvgr_ctx * const ctx);

/**
 * hvgr_sch_stop - Stop schedule, but it will not stop the running queue.
 *
 * @gdev: Device pointer
 * @gdev: Device pointer
 *
 */
void hvgr_sch_reset(struct hvgr_device * const gdev, enum sch_rst_status reset_stage);

/**
 * hvgr_sch_set_mode - Start schedule
 *
 * @gdev: Device pointer
 * @mode: schedule mode.
 * @mode: sch_flag 1 open schedule, 0 will ignore schedule.
 *
 */
void hvgr_sch_set_mode(struct hvgr_device * const gdev, uint32_t mode, uint32_t sch_flag);

/**
 * intr_enable - interrupt enable
 *
 * @gdev: Device pointer
 *
 */
void hvgr_sch_intr_switch(struct hvgr_device * const gdev, uint32_t flag);

/**
 * hvgr_msync_term
 *
 * @ctx: The hvgr context to dispatch to
 *
 */
void hvgr_msync_term(struct hvgr_ctx * const ctx);

#endif
