/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_H
#define HVGR_CQ_H

#include <linux/interrupt.h>

#include "hvgr_defs.h"
#include "apicmd/hvgr_ioctl_sch.h"
#include "hvgr_cq_data.h"


/* GPU CQ model. */
#define CQ_MODE_ENABLE              1U
#define CQ_MODE_DISABLE             0U

/* CQ schedule mode */
#define CQ_SCH_MODE_SERIAL          0U
#define CQ_SCH_MODE_PARALLEL        1U

/* CQ bin/comp chain send out mode */
#define CQ_BC_SEND_INTERLEAVE       0U
#define CQ_BC_SEND_PARALLEL         1U

/* CQ rt bin/comp chain send out mode */
#define CQ_RT_RBC_SEND_INTERLEAVE       0U
#define CQ_RT_RBC_SEND_PARALLEL         1U

long hvgr_ioctl_alloc_cq(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_alloc_cq * const para);

long hvgr_ioctl_free_cq(struct hvgr_ctx * const ctx, union hvgr_ioctl_para_free_cq * const para);

long hvgr_ioctl_submit_cq(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_submit_cq * const para);

int hvgr_cq_dev_init(struct hvgr_device * const gdev);

void hvgr_cq_dev_term(struct hvgr_device * const gdev);

irqreturn_t hvgr_cq_irq_handler(int irq, void *data);

void hvgr_cq_config_bc_send_mode(struct hvgr_device *gdev);

void hvgr_cq_affinity_config(struct hvgr_device * const gdev, uint32_t channel_id);

/**
 * hvgr_ioctl_cq_clear_err() - clear cq err.
 * @ctx: Pointer to kmd context.
 * @para: Pointer to the param which inlcude queue id.
 *
 *  Return: 0 on success: cq error has clear successfully.
 *  Negative error code is returned on error.
 */
long hvgr_ioctl_cq_clear_err(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_cq_clear_err * const para);

long hvgr_ioctl_cq_set_hold_en_info(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_hold_en_info * const para);

long hvgr_ioctl_init_entry_addr(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_init_entry_addr * const para);

long hvgr_ioctl_mod_queue_priv(struct hvgr_ctx * const ctx,
	union hvgr_ioctl_para_mod_queue_priv * const para);

void hvgr_cq_enable(struct hvgr_device * const gdev, uint32_t mode, uint32_t sch_flag);

void hvgr_cq_intr_switch(struct hvgr_device * const gdev, uint32_t flag);

void hvgr_sch_switch_off_request(struct hvgr_device *gdev);

void hvgr_sch_switch_on_request(struct hvgr_device *gdev);

#endif
