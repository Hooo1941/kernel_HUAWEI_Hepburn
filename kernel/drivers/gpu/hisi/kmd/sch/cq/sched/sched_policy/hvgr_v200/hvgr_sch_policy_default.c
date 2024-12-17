/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_sch_policy_default.h"

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <securec.h>

#include "hvgr_dm_api.h"
#include "hvgr_mmu_api.h"
#include "mem/hvgr_as.h"
#include "hvgr_mem_api.h"
#include "hvgr_cq_schedule.h"
#include "hvgr_sch_policy_manager.h"

#include "hvgr_cq_queue.h"
#include "hvgr_log_api.h"

#include "hvgr_asid_api.h"
#include "hvgr_cq.h"

static const uint32_t g_dequeue_weight[CQ_PRIORITY_MAX] = {1, 3, 0};

static inline uint32_t hvgr_cq_schedule_compare_priority(struct hvgr_cq_ctx const * const cur_cqctx,
	struct hvgr_cq_ctx const * const new_cqctx)
{
	if (new_cqctx->priority > cur_cqctx->priority)
		return new_cqctx->priority - cur_cqctx->priority;
	else
		return 0;
}

static struct hvgr_cq_ctx *hvgr_sch_policy_dequeue(struct hvgr_device * const gdev)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	int idx;
	struct cq_pending *choise = NULL;
	struct cq_pending *pending_queue = NULL;
	struct hvgr_cq_ctx *cq_ctx = NULL;
	struct list_head *cq_item = NULL;

	for (idx = CQ_PRIORITY_HIGH; idx >= 0; idx--) {
		pending_queue = &gcqctx->pending_queue[idx];
		if (!list_empty(&pending_queue->pending_list))
			choise = pending_queue;
		else
			continue;
		if (pending_queue->dequeue_counter < g_dequeue_weight[idx])
			break;
	}

	if (choise == NULL) {
		gcqctx->pending_queue[CQ_PRIORITY_HIGH].dequeue_counter = 0;
		gcqctx->pending_queue[CQ_PRIORITY_LOW].dequeue_counter = 0;
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
		pending_queue = &gcqctx->pending_queue[CQ_PRIORITY_SPECIAL_LOW];
		if (!list_empty(&pending_queue->pending_list)) {
			choise = pending_queue;
		} else {
			hvgr_info(gdev, HVGR_SCH, "%s all pending queue empty.\n", __func__);
			return NULL;
		}
#else
		hvgr_info(gdev, HVGR_SCH, "%s all pending queue empty.\n", __func__);
		return NULL;
#endif
	}
	choise->dequeue_counter++;
	if (choise == &gcqctx->pending_queue[CQ_PRIORITY_LOW]) {
		choise->dequeue_counter = 0;
		gcqctx->pending_queue[CQ_PRIORITY_HIGH].dequeue_counter = 0;
	}
	cq_item = choise->pending_list.next;
	list_del(cq_item);
	cq_ctx = container_of(cq_item, struct hvgr_cq_ctx, pending_item);
	return cq_ctx;
}

static uint32_t hvgr_sch_policy_get_channel(struct hvgr_cq_dev * const gcqctx,
	struct hvgr_cq_ctx * const cq_ctx)
{
	uint32_t max = 0;
	uint32_t chn;
	uint32_t channel = CQ_CHANNEL_ID_INVALID;

	for (chn = 0; chn < CQ_CHANNEL_NUM; chn++) {
		uint32_t cmp;
		struct hvgr_cq_ctx *running_cq = NULL;

		if (unlikely(!gcqctx->channel_info[chn].available))
			continue;

		running_cq = gcqctx->channel_info[chn].running_queue;
		if (running_cq == NULL) {
			channel = chn;
			break;
		}
		/* Check the priority */
		cmp = hvgr_cq_schedule_compare_priority(running_cq, cq_ctx);
		if (cmp > max) {
			max = cmp;
			channel = running_cq->channel_id;
		}
	}
	return channel;
}

void hvgr_sch_policy_init(struct hvgr_cq_dev * const gcqctx)
{
	struct hvgr_sch_policy_opts *opts = &gcqctx->policy_opts[CQ_POLICY_WEIGHT_ROUND_ROBIN];

	opts->get_queue = hvgr_sch_policy_dequeue;
	opts->get_channel = hvgr_sch_policy_get_channel;
	gcqctx->cur_policy = CQ_POLICY_WEIGHT_ROUND_ROBIN;
}
