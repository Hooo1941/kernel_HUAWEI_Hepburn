/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_CQ_QUEUE_H
#define HVGR_CQ_QUEUE_H

#include "hvgr_defs.h"
#include "apicmd/hvgr_ioctl_sch.h"
#include "hvgr_cq_data.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
static inline void kmd_timer_setup(struct timer_list *timer,
	void (*callback)(unsigned long addr))
{
	setup_timer(timer, (void (*)(unsigned long)) callback,
		(unsigned long) timer);
}
#else
static inline void kmd_timer_setup(struct timer_list *timer,
	void (*callback)(struct timer_list *timer))
{
	timer_setup(timer, callback, 0);
}
#endif

struct hvgr_cq_ctx *hvgr_cq_alloc_queue(struct hvgr_ctx * const ctx, uint32_t softq_id,
	uint32_t all_entry_size, uint32_t priority);

void hvgr_cq_free_queue(struct hvgr_ctx * const ctx, struct hvgr_cq_ctx *cq_ctx);

struct hvgr_cq_ctx *hvgr_cq_find_cqctx(struct hvgr_ctx * const ctx, uint32_t queue_id);

void hvgr_cq_append_flag_all_queue(struct hvgr_device *gdev, uint32_t flag, bool expend_all_err);

void hvgr_cq_unsubmit_all_queue(struct hvgr_ctx * const ctx);

void hvgr_cq_free_all_queue(struct hvgr_ctx * const ctx);

void hvgr_cq_append_all_queue_with_ctx(struct hvgr_device *gdev, struct hvgr_ctx *ctx,
	uint32_t flag);

long hvgr_cq_submit_to_queue(struct hvgr_cq_ctx * const cq_ctx,
	struct token_callback_data *callback);

#endif
