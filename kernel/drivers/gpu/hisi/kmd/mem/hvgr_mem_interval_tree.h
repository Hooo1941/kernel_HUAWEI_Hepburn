/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_MEM_INTERVAL_TREE_H
#define HVGR_MEM_INTERVAL_TREE_H

#include "hvgr_memory_data.h"

void hvgr_mem_uva_range_insert(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area);

void hvgr_mem_uva_range_remove(struct hvgr_ctx * const ctx, struct hvgr_mem_area * const area);

/*
 * to prevent that, the mem alloced by using ioctl and released by using munmap system call,
 * the uva would be overlapping.
 * This function is to check if the new allocted uva is overlapped in a process.
 */
bool hvgr_mem_uva_check_valid(struct hvgr_ctx *ctx, unsigned long va, unsigned long size);

#endif