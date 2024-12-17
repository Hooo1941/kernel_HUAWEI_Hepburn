/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_REPLAY_H
#define HVGR_MEM_REPLAY_H

#include "hvgr_mem_api.h"
#include "hvgr_defs.h"

bool hvgr_mem_replay_config(struct hvgr_mem_ctx * const pmem,
	struct hvgr_mem_cfg_para *para);

void hvgr_mem_replay_deconfig(struct hvgr_mem_ctx * const pmem);
#endif
