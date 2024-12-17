/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SOFTQ_H
#define HVGR_SOFTQ_H

#include <linux/types.h>
#include <linux/spinlock.h>

#include "apicmd/hvgr_softq_defs.h"
#include "hvgr_kv.h"

struct hvgr_soft_q;
struct hvgr_ctx;

typedef bool (*soft_q_rev)(void *priv, void *entry);

typedef void (*soft_q_notify)(void *priv);

#define HVGR_SOFTQ_DIR_K_TO_U    (1UL << 1)
#define HVGR_SOFTQ_DIR_U_TO_K    (1UL << 2)

#define MAX_ENTRY_NUM 4096
#define MAX_ENTRY_SIZE 1024

#define HVGR_SOFTQ_RUNNING       1
#define HVGR_SOFTQ_VALID         0

struct hvgr_mem_area;

struct hvgr_soft_q {
	struct hvgr_ctx *ctx;
	uint32_t flags;

	/* SoftQ message receive callback */
	soft_q_rev recv;
	void *priv;
	/* SoftQ message receive notify */
	soft_q_notify notify;
	void *notify_priv;

	struct hvgr_kv_node node;
	spinlock_t q_lock;
	struct hvgr_softq_ctrl *ctrl;
	struct hvgr_softq_user_ctrl *user_ctrl;

	struct hvgr_mem_area *ctrl_area;
	struct hvgr_mem_area *user_ctrl_area;
	struct hvgr_mem_area *queue_area;
	bool bind_flag;
};

int hvgr_softq_create(struct hvgr_ctx * const ctx, uint32_t entry_size, uint32_t entry_nums,
	uint64_t flags, uint64_t *uva);

int hvgr_softq_term(struct hvgr_soft_q * const qctx);

void *hvgr_softq_get_entry(struct hvgr_soft_q * const qctx);
int hvgr_softq_enqueue(struct hvgr_soft_q * const qctx,
	void const * const entry, uint32_t size);
int hvgr_softq_dequeue(struct hvgr_soft_q * const qctx);

bool hvgr_softq_check_data(struct hvgr_soft_q * const qctx);

static inline uint32_t hvgr_softq_get_id(struct hvgr_soft_q * const qctx)
{
	return qctx->ctrl->softq_id;
}

uint64_t hvgr_softq_get_flag(struct hvgr_soft_q * const qctx);

uint64_t hvgr_softq_get_ctrl(struct hvgr_soft_q * const qctx);

bool hvgr_softq_register_notify(struct hvgr_soft_q * const qctx,
	soft_q_notify notify, void *priv);

void hvgr_softq_do_flush(struct hvgr_soft_q * const qctx);
#endif
