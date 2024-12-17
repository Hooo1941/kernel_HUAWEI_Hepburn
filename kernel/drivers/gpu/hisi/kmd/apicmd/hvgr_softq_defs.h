/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SOFTQ_DEFS_H
#define HVGR_SOFTQ_DEFS_H

#include "linux/types.h"

#define HVGR_MAX_NR_SOFTQ       (9)

struct hvgr_softq_user_ctrl {
	uint32_t wr; /* Write index */
	uint32_t rd; /* Read index */
};

/* hvgr_softq_ctrl is read-only in user space */
struct hvgr_softq_ctrl {
	uint64_t flags;
	uint32_t softq_id;

	uint32_t entry_size;
	uint32_t entry_nums;

	/* queue_base is address point to soft Q start address */
	uint64_t queue_base;

	/* user_ctrl_addr point to hvgr_softq_user_ctrl */
	uint64_t user_ctrl_addr;

	uint32_t wr; /* Write index */
	uint32_t rd; /* Read index */
};

#endif
