/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_kv.h"

bool hvgr_kv_init(struct hvgr_kv_map * const tab)
{
	if (tab == NULL)
		return false;

	tab->root = RB_ROOT;
	rwlock_init(&(tab->rw_lock));

	return true;
}

bool hvgr_kv_add(struct hvgr_kv_map * const tab,
	struct hvgr_kv_node * const kv_node)
{
	struct rb_node **link = NULL;
	struct rb_node *rb_node = NULL;
	struct hvgr_kv_node *last_kv_node = NULL;
	unsigned long flags = 0;

	if (tab == NULL || kv_node == NULL)
		return false;

	write_lock_irqsave(&(tab->rw_lock), flags);

	link = &(tab->root.rb_node);
	while (*link) {
		rb_node = *link;
		last_kv_node = rb_entry(rb_node, struct hvgr_kv_node, node);

		if (last_kv_node->key > kv_node->key)
			link = &(rb_node->rb_left);
		else
			link = &(rb_node->rb_right);
	}

	rb_link_node(&(kv_node->node), rb_node, link);
	rb_insert_color(&(kv_node->node), &(tab->root));

	write_unlock_irqrestore(&(tab->rw_lock), flags);

	return true;
}

static struct hvgr_kv_node *hvgr_kv_get_nolock(struct hvgr_kv_map * const tab,
	uint64_t key)
{
	struct rb_node **link = NULL;
	struct rb_node *rb_node = NULL;
	struct hvgr_kv_node *kv_node = NULL;

	link = &(tab->root.rb_node);
	while (*link) {
		rb_node = *link;
		kv_node = rb_entry(rb_node, struct hvgr_kv_node, node);
		if (kv_node->key == key)
			return kv_node;

		if (kv_node->key > key)
			link = &(rb_node->rb_left);
		else
			link = &(rb_node->rb_right);
	}

	return NULL;
}

struct hvgr_kv_node *hvgr_kv_get(struct hvgr_kv_map * const tab,
	uint64_t key)
{
	struct hvgr_kv_node *kv_node = NULL;
	unsigned long flags = 0;

	if (tab == NULL)
		return NULL;

	read_lock_irqsave(&(tab->rw_lock), flags);

	kv_node = hvgr_kv_get_nolock(tab, key);

	read_unlock_irqrestore(&(tab->rw_lock), flags);

	return kv_node;
}

void hvgr_kv_del(struct hvgr_kv_map * const tab,
	uint64_t key)
{
	struct hvgr_kv_node *kv_node = NULL;
	unsigned long flags = 0;

	if (tab == NULL)
		return;

	write_lock_irqsave(&(tab->rw_lock), flags);

	kv_node = hvgr_kv_get_nolock(tab, key);
	if (kv_node == NULL) {
		write_unlock_irqrestore(&(tab->rw_lock), flags);
		return;
	}

	rb_erase(&(kv_node->node), &(tab->root));
	RB_CLEAR_NODE(&(kv_node->node));

	write_unlock_irqrestore(&(tab->rw_lock), flags);
}

struct hvgr_kv_node *hvgr_kv_find_range(struct hvgr_kv_map * const tab,
	uint64_t range)
{
	struct rb_node **link = NULL;
	struct rb_node *rb_node = NULL;
	struct rb_node *next_node = NULL;
	struct hvgr_kv_node *kv_node = NULL;
	struct hvgr_kv_node *next_kv_node = NULL;
	unsigned long flags = 0;

	if (tab == NULL)
		return NULL;

	read_lock_irqsave(&(tab->rw_lock), flags);

	link = &(tab->root.rb_node);
	while (*link) {
		rb_node = *link;
		kv_node = rb_entry(rb_node, struct hvgr_kv_node, node);

		/* ==: shold not happen */
		if (range == kv_node->key) {
			read_unlock_irqrestore(&(tab->rw_lock), flags);
			return kv_node;
		}

		/* < :walk to left */
		if (range < kv_node->key) {
			link = &(rb_node->rb_left);
			continue;
		}

		/*
		 * > : if next_node == NULL
		 * if range > (kv_node->key + kv_node->size), return NULL
		 * else return kv_node
		 */
		next_node = rb_next(rb_node);
		if (next_node == NULL)
			goto find_node;

		/*
		 * if range < next_kv_node->key, return kv_node
		 * else walk right
		 */
		next_kv_node = rb_entry(next_node, struct hvgr_kv_node, node);
		if (range < next_kv_node->key)
			goto find_node;

		link = &(rb_node->rb_right);
	}

	read_unlock_irqrestore(&(tab->rw_lock), flags);
	return NULL;
find_node:
	if (range >= (kv_node->key + kv_node->size))
		kv_node = NULL;

	read_unlock_irqrestore(&(tab->rw_lock), flags);
	return kv_node;
}

struct hvgr_kv_node *hvgr_kv_find_bigger_node(struct hvgr_kv_map * const tab,
	uint64_t key)
{
	struct rb_node *link = NULL;
	struct rb_node *temp = NULL;
	struct hvgr_kv_node *kv_node = NULL;
	unsigned long flags = 0;

	if (tab == NULL)
		return NULL;

	read_lock_irqsave(&(tab->rw_lock), flags);
	link = tab->root.rb_node;
	while (link) {
		kv_node = rb_entry(link, struct hvgr_kv_node, node);
		if (kv_node->key > key) {
			/*
			 * Current node key is bigger than search key
			 * move left to get smaller node
			 */
			temp = link;
			link = link->rb_left;
		} else {
			/*
			 * Current node key is smaller than search key
			 * If temp_node is null means we havn't get a bigger node yet,
			 * continue search right node.
			 * If temp_node is not null means temp_node is the node we want.
			 * temp_node > search_node > cur_node
			 */
			if (temp != NULL)
				break;

			link = link->rb_right;
			if (link == NULL) {
				read_unlock_irqrestore(&(tab->rw_lock), flags);
				return NULL;
			}
		}
	}
	kv_node = (temp == NULL) ? NULL : rb_entry(temp, struct hvgr_kv_node, node);
	read_unlock_irqrestore(&(tab->rw_lock), flags);

	return kv_node;
}

bool hvgr_kv_is_empty(struct hvgr_kv_map * const tab)
{
	if (tab == NULL)
		return true;

	return RB_EMPTY_ROOT(&tab->root);
}
