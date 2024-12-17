/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_WQ_H
#define HVGR_WQ_H

#include "hvgr_cq_data.h"

#define WQ_WAIT_MAX           20
#define WQ_EVENT_WAIT_TIMEOUT 3000 /* 3s */

struct hvgr_wq_dev {
	wait_queue_head_t barrier_wait_wq;
};

struct hvgr_device;
struct hvgr_ctx;

int hvgr_wq_init(struct hvgr_device * const gdev);

int hvgr_wq_ctx_init(struct hvgr_ctx * const ctx,
	struct hvgr_cq_ctx * const cq_ctx, uint32_t softq_id);

void hvgr_wq_ctx_term(struct hvgr_cq_ctx * const cq_ctx);

void hvgr_wq_recover(struct hvgr_ctx * const ctx, struct hvgr_cq_ctx * const cq_ctx);

bool hvgr_wq_start_work(struct hvgr_cq_ctx * const cq_ctx);

bool hvgr_wq_stop_work(struct hvgr_cq_ctx * const cq_ctx);

void hvgr_wq_do_entry(struct hvgr_cq_ctx * const cq_ctx, wq_work_entry * const entry,
	uint64_t *jit_addrs, int jit_num);

bool hvgr_wq_opt_get_jit_addr(struct hvgr_cq_ctx * const cq_ctx,
	wq_work_entry * const wq_entry, uint64_t *jitaddr_array, int jit_num);

#endif
