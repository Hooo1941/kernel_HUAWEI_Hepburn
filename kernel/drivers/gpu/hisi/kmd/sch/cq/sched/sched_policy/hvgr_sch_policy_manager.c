/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_sch_policy_manager.h"

#include <linux/kernel.h>

#include "hvgr_cq_driver_base.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_defs.h"
#include "hvgr_cq_data.h"
#include "hvgr_log_api.h"

struct hvgr_cq_ctx *hvgr_cq_get_available_queue(struct hvgr_device * const gdev)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	struct hvgr_cq_ctx *next_queue = NULL;

	if (unlikely(gcqctx->policy_opts[gcqctx->cur_policy].get_queue == NULL)) {
		hvgr_err(gdev, HVGR_CQ, "Sch policy is not inited, get_queue is null");
		return NULL;
	}

	next_queue = gcqctx->policy_opts[gcqctx->cur_policy].get_queue(gdev);
	while (next_queue != NULL) {
		if (unlikely(hvgr_cq_is_queue_error(next_queue))) {
			(void)hvgr_cq_schedule_unqueue(gcqctx, next_queue);
			hvgr_err(gdev, HVGR_CQ, "%s: qid=%u has error, drop it.\n",
				__func__, next_queue->queue_id);
			next_queue = gcqctx->policy_opts[gcqctx->cur_policy].get_queue(gdev);
			continue;
		}
		break;
	}
	return next_queue;
}

uint32_t hvgr_cq_get_channel(struct hvgr_cq_dev * const gcqctx,
	struct hvgr_cq_ctx * const cq_ctx)
{
	if (unlikely(gcqctx->policy_opts[gcqctx->cur_policy].get_channel == NULL))
		return CQ_CHANNEL_ID_INVALID;

	return gcqctx->policy_opts[gcqctx->cur_policy].get_channel(gcqctx, cq_ctx);
}

void hvgr_sch_policy_manager_init(struct hvgr_cq_dev * const gcqctx)
{
	uint32_t idx;
	uint32_t weight, real;
	uint32_t real_weight_sum = 0;
	uint32_t no_real_weight_sum = 0;

	for (idx = 0; idx < CQ_PRIORITY_MAX; idx++) {
		weight = 0;
		real = 0;
		hvgr_get_priority_cfg(idx, &weight, &real);
		gcqctx->pending_queue[idx].weight = weight;
		gcqctx->pending_queue[idx].real = real;
		gcqctx->pending_queue[idx].dequeue_counter = 0;
		if (real == 1)
			real_weight_sum += weight;
		else
			no_real_weight_sum += weight;
	}
	gcqctx->real_weight_sum = real_weight_sum;
	gcqctx->no_real_weight_sum = no_real_weight_sum;
	gcqctx->real_counter_sum = 0;
	gcqctx->no_real_counter_sum = 0;
	hvgr_sch_policy_init(gcqctx);
}