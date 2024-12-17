/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_pool.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

long hvgr_pool_init(struct hvgr_pool *pool, struct hvgr_pool_data *data)
{
	if (pool == NULL || data == NULL || data->alloc == NULL || data->free == NULL)
		return -EINVAL;

	pool->max_size = data->max_size;
	pool->water = data->water;
	pool->priv = data->priv;
	pool->alloc = data->alloc;
	pool->free = data->free;
	pool->callback = data->callback;
	pool->cur_size = 0;
	INIT_LIST_HEAD(&pool->list_head);
	mutex_init(&pool->pool_lock);
	pool->inited = true;

	return 0;
}

void hvgr_pool_term(struct hvgr_pool *pool)
{
	if (pool == NULL || !pool->inited)
		return;

	pool->cur_size = 0;
	pool->max_size = 0;
}

long hvgr_pool_alloc(struct hvgr_pool *pool, uint32_t size, void **array)
{
	uint32_t i;
	uint32_t pool_get_size;
	struct page *page = NULL;

	if (pool == NULL || array == NULL || size == 0)
		return -EINVAL;

	mutex_lock(&pool->pool_lock);
	if (pool->cur_size == 0) {
		mutex_unlock(&pool->pool_lock);
		return pool->alloc(pool->priv, size, array);
	}

	pool_get_size = min(pool->cur_size, size);

	for (i = 0; i < pool_get_size; i++) {
		page = list_first_entry(&pool->list_head, struct page, lru);
		list_del_init(&page->lru);

		array[i] = (void *)page;
		pool->cur_size--;
	}
	mutex_unlock(&pool->pool_lock);
	if (pool_get_size < size)
		return pool->alloc(pool->priv, (size - pool_get_size), (array + pool_get_size));

	return 0;
}

void hvgr_pool_free(struct hvgr_pool *pool, uint32_t size, void **array, uint32_t flag)
{
	uint32_t i;
	uint32_t pool_put_size;
	struct page *page = NULL;

	if (pool == NULL || array == NULL || size == 0)
		return;

	mutex_lock(&pool->pool_lock);
	if (pool->cur_size == pool->max_size) {
		mutex_unlock(&pool->pool_lock);
		pool->free(pool->priv, size, array, flag);
		return;
	}

	pool_put_size = pool->max_size - pool->cur_size;
	pool_put_size = min(pool_put_size, size);

	for (i = 0; i < pool_put_size; i++) {
		if (flag != 0 && pool->callback != NULL)
			pool->callback(pool, array[i], flag);

		page = (struct page *)array[i];
		list_add(&page->lru, &pool->list_head);
		pool->cur_size++;
	}
	mutex_unlock(&pool->pool_lock);
	if (pool_put_size < size)
		pool->free(pool->priv, (size - pool_put_size), (array + pool_put_size), flag);
}

long hvgr_pool_alloc_direct(struct hvgr_pool *pool, uint32_t size, void **array)
{
	uint32_t i;
	struct page *page = NULL;

	if (pool == NULL || array == NULL || size == 0)
		return -EINVAL;

	mutex_lock(&pool->pool_lock);
	if (pool->cur_size < size) {
		mutex_unlock(&pool->pool_lock);
		return -ENOMEM;
	}

	for (i = 0; i < size; i++) {
		page = list_first_entry(&pool->list_head, struct page, lru);
		list_del_init(&page->lru);

		array[i] = (void *)page;
		pool->cur_size--;
	}

	mutex_unlock(&pool->pool_lock);
	return 0;
}

uint32_t hvgr_pool_grow(struct hvgr_pool *pool, uint32_t size, void **array)
{
	uint32_t pool_grow_size;
	uint32_t i;
	struct page *page = NULL;

	if (pool == NULL || array == NULL || size == 0)
		return 0;

	mutex_lock(&pool->pool_lock);
	if (pool->cur_size == pool->max_size) {
		mutex_unlock(&pool->pool_lock);
		return 0;
	}

	pool_grow_size = pool->max_size - pool->cur_size;
	pool_grow_size = min(pool_grow_size, size);

	for (i = 0; i < pool_grow_size; i++) {
		page = (struct page *)array[i];
		list_add(&page->lru, &pool->list_head);
		pool->cur_size++;
	}
	mutex_unlock(&pool->pool_lock);

	return pool_grow_size;
}

