/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description:  header file of ctx scheduled.

 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 */

#ifndef HVGR_CTX_SCHED_H
#define HVGR_CTX_SCHED_H

#include "hvgr_defs.h"

bool hvgr_ctx_sched_in_with_lock(struct hvgr_ctx * const ctx);

void hvgr_ctx_sched_out_with_lock(struct hvgr_ctx * const ctx);

void kmd_ctx_sched_remove_ctx(struct hvgr_ctx *ctx);

#endif
