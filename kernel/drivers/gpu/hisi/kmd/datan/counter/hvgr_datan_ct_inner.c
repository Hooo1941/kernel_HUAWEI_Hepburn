/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_datan_ct_inner.h"

#include <linux/delay.h>

#include "hvgr_regmap.h"
#include "hvgr_pm_api.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_mmu_api.h"

#include "hvgr_datan_ct_hal.h"

int hvgr_datan_ct_enable_inner(struct hvgr_device *gdev,
	struct hvgr_ctx *ctx, struct hvgr_ct_setup *setup)
{
	unsigned long flags;
	struct hvgr_datan_ct_inner_config config;

	if ((setup->dump_buffer == 0ULL) || ((setup->dump_buffer & (u64)0x7FF) != 0ULL))
		return -EINVAL;

	spin_lock_irqsave(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	if ((gdev->datan_dev.datan_ct_dev.state != HVGR_DATAN_CT_STATE_CLOSED) &&
		(gdev->datan_dev.datan_ct_dev.state != HVGR_DATAN_CT_STATE_IDLE)) {
		spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
		return -1;
	}

	hvgr_datan_ct_irq_enable(gdev);

	gdev->datan_dev.datan_ct_dev.ctx = ctx;
	gdev->datan_dev.datan_ct_dev.dump_addr = setup->dump_buffer;

	spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);

	config.dump_buffer_base_low = (u32)(setup->dump_buffer & 0xFFFFFFFFU);
	config.dump_buffer_base_high = (u32)(setup->dump_buffer >> OFFSET_32BIT);
	config.fcm_enable = setup->fcm_bm;
	config.gpc_enable = setup->gpc_bm;
	config.mmu_enable = setup->mmu_l2_bm;
	config.btc_enable = setup->btc_bm;
	config.bvh_enable = setup->bvh_bm;
	config.dump_mode = PRFCNT_CONFIG_MODE_MANUAL;
	config.set_id = setup->set_id;

	hvgr_mmu_flush_tlb(ctx);
	hvgr_datan_ct_open_config(gdev, ctx, &config);

	spin_lock_irqsave(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_IDLE;
	spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	return 0;
}

int hvgr_datan_ct_disable_inner(struct hvgr_ctx *ctx)
{
	unsigned long flags;
	struct hvgr_device *gdev = ctx->gdev;
	long timeout = (long)msecs_to_jiffies(2000); // timeout 2000ms
	unsigned long sch_flags;

	if (hvgr_pm_get_gpu_status(gdev) != HVGR_PM_GPU_READY) {
		gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_CLOSED;
		gdev->datan_dev.datan_ct_dev.triggered = 0;
		gdev->datan_dev.datan_ct_dev.ctx = NULL;
		gdev->datan_dev.datan_ct_dev.dump_addr = 0ULL;
		return 0;
	}

	while (1) {
		spin_lock_irqsave(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);

		if (gdev->datan_dev.datan_ct_dev.state == HVGR_DATAN_CT_STATE_CLOSED) {
			spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
			return -EINVAL;
		}

		if (gdev->datan_dev.datan_ct_dev.ctx != ctx) {
			spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
			return -EINVAL;
		}

		if (gdev->datan_dev.datan_ct_dev.state == HVGR_DATAN_CT_STATE_IDLE) {
			hvgr_info(gdev, HVGR_DATAN, "%s, state is IDLE", __func__);
			break;
		}

		if (timeout == 0) {
			hvgr_info(gdev, HVGR_DATAN, "%s, wait_event_timeout", __func__);
			break;
		}

		spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);

		timeout = wait_event_timeout(gdev->datan_dev.datan_ct_dev.wait,
			gdev->datan_dev.datan_ct_dev.triggered != 0, timeout);
	}

	gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_CLOSED;
	gdev->datan_dev.datan_ct_dev.triggered = 0;

	spin_lock_irqsave(&gdev->cq_dev.schedule_lock, sch_flags);
	hvgr_datan_ct_irq_disable(gdev);
	hvgr_datan_ct_close_config(gdev);
	gdev->datan_dev.datan_ct_dev.ctx = NULL;
	gdev->datan_dev.datan_ct_dev.dump_addr = 0ULL;
	spin_unlock_irqrestore(&gdev->cq_dev.schedule_lock, sch_flags);

	spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	return 0;
}

int hvgr_datan_ct_dump_inner(struct hvgr_ctx *ctx)
{
	unsigned long flags;
	int err = -EINVAL;
	struct hvgr_device *gdev = ctx->gdev;

	spin_lock_irqsave(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	if (gdev->datan_dev.datan_ct_dev.ctx != ctx) {
		hvgr_err(gdev, HVGR_DATAN, "[%s] old t=[%d] p=[%d] new t=[%d] p=[%d].",
			__func__, gdev->datan_dev.datan_ct_dev.ctx->tgid,
			gdev->datan_dev.datan_ct_dev.ctx->pid, ctx->tgid, ctx->pid);
		goto unlock;
	}

	if (gdev->datan_dev.datan_ct_dev.state != HVGR_DATAN_CT_STATE_IDLE) {
		hvgr_err(gdev, HVGR_DATAN, "[%s].", __func__);
		goto unlock;
	}

	gdev->datan_dev.datan_ct_dev.triggered = 0;
	gdev->datan_dev.datan_ct_dev.int_done = false;
	gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_DUMPING;
	hvgr_datan_ct_dump_data(gdev);
	err = 0;
unlock:
	spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	return err;
}

int hvgr_datan_ct_wait_for_dump_inner(struct hvgr_ctx *ctx)
{
	struct hvgr_device *gdev = ctx->gdev;
	u32 val;
	u32 times = 0;

	while (times < DUMP_WAIT_MAX_TIME) {
		val = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_STATUS));
		if ((val & PRFCNT_SAMPLE_COMPLETED) || gdev->datan_dev.datan_ct_dev.int_done)
			return hvgr_datan_ct_clear_status_inner(gdev);
		udelay(1); /* delay 1us */
		times++;
	}

	return hvgr_datan_ct_clear_status_inner(gdev);
}

int hvgr_datan_ct_clear_status_inner(struct hvgr_device *gdev)
{
	unsigned long flags;
	int err;

	spin_lock_irqsave(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	if (gdev->datan_dev.datan_ct_dev.state == HVGR_DATAN_CT_STATE_FAULT)
		err = -EINVAL;
	else
		err = 0;
	gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_IDLE;
	gdev->datan_dev.datan_ct_dev.triggered = 1;
	wake_up(&gdev->datan_dev.datan_ct_dev.wait);
	spin_unlock_irqrestore(&gdev->datan_dev.datan_ct_dev.ct_lock, flags);
	return err;
}

int hvgr_datan_ct_init_status_inner(struct hvgr_device *gdev)
{
	gdev->datan_dev.datan_ct_dev.state = HVGR_DATAN_CT_STATE_CLOSED;

	init_waitqueue_head(&gdev->datan_dev.datan_ct_dev.wait);

	gdev->datan_dev.datan_ct_dev.triggered = 0;
	gdev->datan_dev.datan_ct_dev.ct_config = false;
	gdev->datan_dev.datan_ct_dev.ct_closed = true;
	return 0;
}
