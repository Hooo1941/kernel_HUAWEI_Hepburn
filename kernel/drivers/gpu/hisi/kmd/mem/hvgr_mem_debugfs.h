/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_DEBUGFS_H
#define HVGR_MEM_DEBUGFS_H

#include "hvgr_defs.h"

long hvgr_mem_add_profile(struct hvgr_ctx * const ctx, uint64_t user_buf, uint64_t size);

void hvgr_mem_remove_profile(struct hvgr_ctx * const ctx);

void hvgr_mem_debugfs_init(struct hvgr_device * const gdev);

void hvgr_mtrack_debugfs_init(struct hvgr_device * const gdev);

void hvgr_mtrack_debugfs_term(struct hvgr_device * const gdev);

#endif
