/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_JIT_H
#define HVGR_MEM_JIT_H

#include "hvgr_defs.h"

bool hvgr_mem_jit_config(struct hvgr_mem_ctx * const pmem, void * const para);

void hvgr_mem_jit_deconfig(struct hvgr_mem_ctx * const pmem);

#endif