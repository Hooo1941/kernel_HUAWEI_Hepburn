/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DM_CTX_H
#define HVGR_DM_CTX_H

#include "hvgr_defs.h"

struct hvgr_ctx *hvgr_open_context(struct hvgr_device * const gdev,
	struct file *kfile);

void hvgr_close_context(struct hvgr_ctx * const ctx);

#if hvgr_version_ge(HVGR_V350)
/*
 * common ctx use the defined va for gpu va
 * each va range size is reserved for 1GB
 */
#define COMMON_CTX_FCP_BUF_START                    0x40000000
#define COMMON_CTX_FAST_PWR_BUF_START               0x80000000

int hvgr_ctx_common_ctx_init(struct hvgr_device * const gdev);

void hvgr_ctx_common_ctx_term(struct hvgr_device * const gdev);

int hvgr_ctx_common_ctx_enable(struct hvgr_device * const gdev);
#endif

#endif
