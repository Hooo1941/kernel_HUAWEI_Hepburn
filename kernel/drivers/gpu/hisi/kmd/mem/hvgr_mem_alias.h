/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_ALIAS_H
#define HVGR_MEM_ALIAS_H

#include "hvgr_defs.h"
#include "hvgr_mem_api.h"

bool hvgr_mem_alias_config(struct hvgr_mem_ctx * const pmem);

struct hvgr_mem_area *hvgr_mem_alias(struct hvgr_ctx * const ctx,
	struct hvgr_mem_alias_para * const para, uint64_t *pages, uint64_t *gva);
#endif
