/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_MEM_SPACE_H
#define HVGR_MEM_SPACE_H

#include "linux/types.h"
#include <linux/rbtree.h>
#include <linux/mutex.h>

struct hvgr_mem_space_node {
	struct rb_node addr_node;
	struct rb_node size_node;

	uint64_t start;
	uint64_t size;
};

struct hvgr_mem_space {
	struct mutex space_mutex;
	struct rb_root addr_root;
	struct rb_root size_root;

	uint64_t start;
	uint64_t size;
};

int hvgr_mem_space_init(struct hvgr_mem_space * const space);

void hvgr_mem_space_term(struct hvgr_mem_space * const space);

bool hvgr_mem_space_is_empty(struct hvgr_mem_space * const space);

int hvgr_mem_space_joint(struct hvgr_mem_space * const space, uint64_t addr, uint64_t size);

int hvgr_mem_space_split(struct hvgr_mem_space * const space, uint64_t spec_addr,
	uint64_t size, uint64_t *addr);
#endif
