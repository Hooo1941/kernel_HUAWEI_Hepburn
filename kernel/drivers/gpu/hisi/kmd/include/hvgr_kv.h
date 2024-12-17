/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_KV_H
#define HVGR_KV_H

#include "linux/types.h"
#include <linux/rbtree.h>
#include <linux/spinlock.h>

struct hvgr_kv_map {
	struct rb_root root;
	rwlock_t rw_lock;
};

struct hvgr_kv_node {
	struct rb_node node;
	uint64_t key;
	uint64_t size;
};

bool hvgr_kv_init(struct hvgr_kv_map * const tab);

bool hvgr_kv_add(struct hvgr_kv_map * const tab,
	struct hvgr_kv_node * const node);

void hvgr_kv_del(struct hvgr_kv_map * const tab,
	uint64_t key);

struct hvgr_kv_node *hvgr_kv_get(struct hvgr_kv_map * const tab,
	uint64_t key);

struct hvgr_kv_node *hvgr_kv_find_range(struct hvgr_kv_map * const tab,
	uint64_t range);

struct hvgr_kv_node *hvgr_kv_find_bigger_node(struct hvgr_kv_map * const tab,
	uint64_t key);

bool hvgr_kv_is_empty(struct hvgr_kv_map * const tab);

#endif
