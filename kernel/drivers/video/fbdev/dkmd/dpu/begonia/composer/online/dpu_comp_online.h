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

#ifndef COMPOSER_ONLINE_H
#define COMPOSER_ONLINE_H

#include <linux/kthread.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>

#include "scene/dpu_comp_scene.h"
#include "timeline/dkmd_timeline.h"
#include "online/dpu_comp_vsync.h"
#include "online/dpu_comp_maintain.h"
#include "dfr/dpu_comp_dfr.h"
#include "ppc/dpu_comp_ppc.h"
#include "isr/dkmd_isr.h"

#define COMP_FRAME_MAX 3

struct comp_online_present {
	/* online compose info */
	struct dpu_comp_frame frames[COMP_FRAME_MAX];
	uint32_t incoming_idx;
	uint32_t displaying_idx;
	uint32_t displayed_idx;

	uint32_t vactive_start_flag;
	uint32_t frame_start_flag;
	wait_queue_head_t vactive_start_wq;
	ktime_t vactive_start_timestamp;
	uint32_t vactive_end_flag;
	int32_t vactive_ldi_status;

	uint32_t frame_rate;
	struct dpu_comp_dfr_ctrl dfr_ctrl;
	struct dpu_comp_ppc_ctrl ppc_ctrl;

	struct dpu_vsync vsync_ctrl;
	struct dkmd_timeline timeline;

	struct dpu_comp_maintain comp_maintain;

	struct kthread_work m1_qic_handle_work;
	/* abnormal could be underflow or frame_end timeout */
	struct kthread_work abnormal_handle_work;

	struct dpu_composer *dpu_comp;
	uint32_t buffers;
};

static inline void composer_online_update_frame_rate(struct comp_online_present *present,
													 uint32_t active_frame_rate)
{
	present->frame_rate = active_frame_rate;
}
void composer_online_setup(struct dpu_composer *dpu_comp, struct comp_online_present *present);
void composer_online_release(struct dpu_composer *dpu_comp, struct comp_online_present *present);
void composer_online_recovery(struct dpu_composer *dpu_comp);

#endif
