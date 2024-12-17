/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_POOL_H
#define HVGR_POOL_H

#include <linux/types.h>
#include <linux/mutex.h>

struct hvgr_pool;

typedef long (*pool_alloc)(void *target, uint32_t size, void **array);
typedef void (*pool_free)(void *target, uint32_t size, void **array, uint32_t flag);
typedef void (*pool_callback)(struct hvgr_pool *pool, void *element, uint32_t flag);

struct hvgr_pool {
	bool inited;
	uint32_t cur_size;
	uint32_t max_size;
	uint32_t water;
	struct mutex pool_lock;
	struct list_head list_head;

	pool_alloc alloc;
	pool_free free;
	pool_callback callback;
	void *priv;
};

struct hvgr_pool_data {
	uint32_t max_size;
	uint32_t water;

	pool_alloc alloc;
	pool_free free;
	pool_callback callback;
	void *priv;
};

long hvgr_pool_init(struct hvgr_pool *pool, struct hvgr_pool_data *data);

void hvgr_pool_term(struct hvgr_pool *pool);

long hvgr_pool_alloc(struct hvgr_pool *pool, uint32_t size, void **array);

void hvgr_pool_free(struct hvgr_pool *pool, uint32_t size, void **array, uint32_t flag);

long hvgr_pool_alloc_direct(struct hvgr_pool *pool, uint32_t size, void **array);

uint32_t hvgr_pool_grow(struct hvgr_pool *pool, uint32_t size, void **array);

#endif
