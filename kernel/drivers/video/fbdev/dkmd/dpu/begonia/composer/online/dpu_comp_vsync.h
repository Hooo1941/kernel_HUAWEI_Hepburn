/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef DPU_VSYNC_H
#define DPU_VSYNC_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kthread.h>

#include "chrdev/dkmd_sysfs.h"

#define VSYNC_IDLE_EXPIRE_COUNT 4
#define VSYNC_MAX_TIME_OFFSET 1500
#define VSYNC_MAX_ERROR_TIMES 7

struct dpu_vsync {
	wait_queue_head_t wait;
	ktime_t timestamp;
	ktime_t prev_timestamp;
	bool routine_enabled;
	void *isr;

	spinlock_t spin_enable;
	int32_t enabled;

	struct kthread_work idle_handle_work;
	struct kthread_work vsync_trigger_handle_work;

	uint32_t listening_isr_bit;
	struct notifier_block *notifier; // isr will notify timeline to handle event

	struct dpu_composer *dpu_comp;
	int32_t vsync_not_match_times;
};

static inline bool dpu_vsync_is_enabled(struct dpu_vsync *vsync_ctrl)
{
	unsigned long flags = 0;
	bool enable = false;

	spin_lock_irqsave(&vsync_ctrl->spin_enable, flags);
	enable = !!vsync_ctrl->enabled;
	spin_unlock_irqrestore(&vsync_ctrl->spin_enable, flags);

	return enable;
}

static inline void dpu_vsync_enable(struct dpu_vsync *vsync_ctrl, int32_t enable)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&(vsync_ctrl->spin_enable), flags);
	vsync_ctrl->enabled = enable;
	spin_unlock_irqrestore(&(vsync_ctrl->spin_enable), flags);
}

static inline void dpu_vsync_enable_routine(struct dpu_vsync *vsync_ctrl)
{
	vsync_ctrl->routine_enabled = true;
}

static inline void dpu_vsync_disable_routine(struct dpu_vsync *vsync_ctrl)
{
	vsync_ctrl->routine_enabled = false;
}

void dpu_vsync_init(struct dpu_vsync *vsync_ctrl, struct dkmd_attr *attrs);

void dpu_comp_active_vsync(struct dpu_composer *dpu_comp);
void dpu_comp_deactive_vsync(struct dpu_composer *dpu_comp);

#endif