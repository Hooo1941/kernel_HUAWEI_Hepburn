/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_mem_space.h"

#include <linux/kernel.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/mm.h>

static struct hvgr_mem_space_node *hvgr_mem_space_alloc_node(uint64_t addr, uint64_t size)
{
	struct hvgr_mem_space_node *space_node = NULL;

	space_node = kzalloc(sizeof(*space_node), GFP_KERNEL);
	if (unlikely(space_node == NULL))
		return NULL;

	space_node->start = addr;
	space_node->size = size;

	return space_node;
}

int hvgr_mem_space_init(struct hvgr_mem_space * const space)
{
	struct hvgr_mem_space_node *space_node = NULL;
	struct rb_node **link = NULL;

	if (unlikely(space == NULL))
		return -EINVAL;

	if (unlikely((space->size == (uint64_t)0) || (space->start > (U64_MAX - space->size))))
		return -EINVAL;

	space->addr_root = RB_ROOT;
	space->size_root = RB_ROOT;
	mutex_init(&(space->space_mutex));

	space_node = hvgr_mem_space_alloc_node(space->start, space->size);
	if (unlikely(space_node == NULL))
		return -ENOMEM;

	link = &(space->addr_root.rb_node);
	rb_link_node(&(space_node->addr_node), NULL, link);
	rb_insert_color(&(space_node->addr_node), &(space->addr_root));

	link = &(space->size_root.rb_node);
	rb_link_node(&(space_node->size_node), NULL, link);
	rb_insert_color(&(space_node->size_node), &(space->size_root));

	return 0;
}

static void hvgr_mem_space_insert_size_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node)
{
	struct rb_node **link = NULL;
	struct rb_node *rb_node = NULL;
	struct hvgr_mem_space_node *last_space_node = NULL;

	link = &(space->size_root.rb_node);
	while (*link) {
		rb_node = *link;
		last_space_node = rb_entry(rb_node, struct hvgr_mem_space_node, size_node);

		if (last_space_node->size > space_node->size)
			link = &(rb_node->rb_left);
		else
			link = &(rb_node->rb_right);
	}

	rb_link_node(&(space_node->size_node), rb_node, link);
	rb_insert_color(&(space_node->size_node), &(space->size_root));
}

static void hvgr_mem_space_insert_addr_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node)
{
	struct rb_node **link = NULL;
	struct rb_node *rb_node = NULL;
	struct hvgr_mem_space_node *last_space_node = NULL;

	link = &(space->addr_root.rb_node);
	while (*link) {
		rb_node = *link;
		last_space_node = rb_entry(rb_node, struct hvgr_mem_space_node, addr_node);

		if (last_space_node->start > space_node->start)
			link = &(rb_node->rb_left);
		else
			link = &(rb_node->rb_right);
	}

	rb_link_node(&(space_node->addr_node), rb_node, link);
	rb_insert_color(&(space_node->addr_node), &(space->addr_root));
}

static int hvgr_mem_space_insert_node(struct hvgr_mem_space * const space, uint64_t addr,
	uint64_t size)
{
	struct hvgr_mem_space_node *new_space_node = NULL;

	new_space_node = hvgr_mem_space_alloc_node(addr, size);
	if (unlikely(new_space_node == NULL))
		return -ENOMEM;

	hvgr_mem_space_insert_addr_node(space, new_space_node);
	hvgr_mem_space_insert_size_node(space, new_space_node);

	return 0;
}

static void hvgr_mem_space_erase_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node)
{
	rb_erase(&(space_node->addr_node), &(space->addr_root));
	RB_CLEAR_NODE(&(space_node->addr_node));

	rb_erase(&(space_node->size_node), &(space->size_root));
	RB_CLEAR_NODE(&(space_node->size_node));

	kfree(space_node);
}

static void hvgr_mem_space_reinsert_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node)
{
	rb_erase(&(space_node->addr_node), &(space->addr_root));
	RB_CLEAR_NODE(&(space_node->addr_node));
	hvgr_mem_space_insert_addr_node(space, space_node);

	rb_erase(&(space_node->size_node), &(space->size_root));
	RB_CLEAR_NODE(&(space_node->size_node));
	hvgr_mem_space_insert_size_node(space, space_node);
}

static void hvgr_mem_space_reinsert_size_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node)
{
	rb_erase(&(space_node->size_node), &(space->size_root));
	RB_CLEAR_NODE(&(space_node->size_node));
	hvgr_mem_space_insert_size_node(space, space_node);
}

static int hvgr_mem_space_joint_right(struct hvgr_mem_space * const space,
	struct rb_node *addr_node, uint64_t addr, uint64_t size)
{
	struct rb_node *next_node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;
	struct hvgr_mem_space_node *next_space_node = NULL;

	space_node = rb_entry(addr_node, struct hvgr_mem_space_node, addr_node);

	/* 1.new addr can merge with cur node in size, even next node */
	if ((space_node->start + space_node->size) == addr) {
		/* merge with cur node in size  */
		space_node->size += size;

		/* check if next node can be merge */
		next_node = rb_next(addr_node);
		if (next_node == NULL) {
			hvgr_mem_space_reinsert_size_node(space, space_node);
			return 0;
		}

		next_space_node = rb_entry(next_node, struct hvgr_mem_space_node,
			addr_node);
		if ((space_node->start + space_node->size) ==
			next_space_node->start) {
			/*
			 * merge next node in addr,
			 * erase next node in size and addr
			 */
			space_node->size += next_space_node->size;

			hvgr_mem_space_erase_node(space, next_space_node);
		}

		hvgr_mem_space_reinsert_size_node(space, space_node);
		return 0;
	}

	/* 2.new addr is the max key, insert it */
	next_node = rb_next(addr_node);
	if (next_node == NULL)
		return hvgr_mem_space_insert_node(space, addr, size);

	/* 3.new addr can merge with next node in addr */
	next_space_node = rb_entry(next_node, struct hvgr_mem_space_node,
		addr_node);
	if ((addr + size) == next_space_node->start) {
		next_space_node->start = addr;
		next_space_node->size += size;

		hvgr_mem_space_reinsert_node(space, next_space_node);
		return 0;
	}

	/* 4.new addr is between cur and next, insert it */
	if ((addr + size) < next_space_node->start)
		return hvgr_mem_space_insert_node(space, addr, size);

	return -1;
}

static int hvgr_mem_space_joint_left(struct hvgr_mem_space * const space,
	struct rb_node *addr_node, uint64_t addr, uint64_t size)
{
	struct rb_node *prev_node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;
	struct hvgr_mem_space_node *prev_space_node = NULL;

	space_node = rb_entry(addr_node, struct hvgr_mem_space_node, addr_node);

	/* 1. new addr can merge with cur node in addr */
	if ((addr + size) == space_node->start) {
		space_node->start = addr;
		space_node->size += size;

		/* check if prev node can be merge */
		prev_node = rb_prev(addr_node);
		if (prev_node == NULL) {
			hvgr_mem_space_reinsert_node(space, space_node);
			return 0;
		}

		prev_space_node = rb_entry(prev_node, struct hvgr_mem_space_node,
			addr_node);
		if ((prev_space_node->start + prev_space_node->size) ==
			space_node->start) {
			prev_space_node->size += space_node->size;

			hvgr_mem_space_erase_node(space, space_node);
			hvgr_mem_space_reinsert_size_node(space, prev_space_node);
		} else {
			hvgr_mem_space_reinsert_node(space, space_node);
		}

		return 0;
	}

	/* 2.new addr is the min key, insert it */
	prev_node = rb_prev(addr_node);
	if (prev_node == NULL)
		return hvgr_mem_space_insert_node(space, addr, size);

	/* 3.new addr can merge with prev node in size */
	prev_space_node = rb_entry(prev_node, struct hvgr_mem_space_node,
		addr_node);
	if ((prev_space_node->start + prev_space_node->size) == addr) {
		prev_space_node->size += size;

		hvgr_mem_space_reinsert_size_node(space, prev_space_node);
		return 0;
	}

	/* 4.new addr is between prev and cur, insert it */
	if ((prev_space_node->start + prev_space_node->size) < addr)
		return hvgr_mem_space_insert_node(space, addr, size);

	return -1;
}

int hvgr_mem_space_joint(struct hvgr_mem_space * const space, uint64_t addr, uint64_t size)
{
	struct rb_node **link = NULL;
	struct rb_node *addr_node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;
	int ret;

	if (unlikely(space == NULL || size == 0))
		return -EINVAL;

	mutex_lock(&(space->space_mutex));

	link = &(space->addr_root.rb_node);
	while (*link) {
		addr_node = *link;
		space_node = rb_entry(addr_node, struct hvgr_mem_space_node, addr_node);
		if (space_node->start < addr) {
			if (hvgr_mem_space_joint_right(space, addr_node, addr, size) == 0) {
				ret = 0;
				goto joint_out;
			}

			/* new addr is great than next node addr, walk to right */
			link = &(addr_node->rb_right);
		} else if (space_node->start > addr) {
			if (hvgr_mem_space_joint_left(space, addr_node, addr, size) == 0) {
				ret = 0;
				goto joint_out;
			}

			/* new addr is less than prev node addr, walk to left */
			link = &(addr_node->rb_left);
		} else {
			WARN_ON(1);
			ret = -EFAULT;
			goto joint_out;
		}
	}

	/* addr_tree is null, insert new node */
	ret = hvgr_mem_space_insert_node(space, addr, size);
joint_out:
	mutex_unlock(&(space->space_mutex));
	return ret;
}

static void hvgr_mem_space_find_equal_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node, uint64_t *addr)
{
	*addr = space_node->start;

	hvgr_mem_space_erase_node(space, space_node);
}

static void hvgr_mem_space_split_node(struct hvgr_mem_space * const space,
	struct hvgr_mem_space_node *space_node, uint64_t *addr, uint64_t size)
{
	*addr = (space_node->start + space_node->size) - size;
	space_node->size -= size;

	/* reinsert cur node in size_root, nothing to do with addr_root */
	rb_erase(&(space_node->size_node), &(space->size_root));
	RB_CLEAR_NODE(&(space_node->size_node));
	hvgr_mem_space_insert_size_node(space, space_node);
}

static int hvgr_mem_space_split_normal(struct hvgr_mem_space * const space, uint64_t size,
	uint64_t *addr)
{
	struct rb_node **link = NULL;
	struct rb_node *size_node = NULL;
	struct rb_node *prev_node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;
	struct hvgr_mem_space_node *prev_space_node = NULL;
	struct hvgr_mem_space_node *tmp_space_node = NULL;

	mutex_lock(&(space->space_mutex));
	link = &(space->size_root.rb_node);
	while (*link) {
		size_node = *link;
		space_node = rb_entry(size_node, struct hvgr_mem_space_node, size_node);
		/* 1.cur node size is equal to size */
		if (space_node->size == size) {
			tmp_space_node = space_node;
			goto find_equal_node;
		}

		/* 2.cur node size is less than size, walk to right */
		if (space_node->size < size) {
			/* walk to right */
			link = &(size_node->rb_right);
			continue;
		}

		/* 3.cur node size is great than size and prev node is null, split cur node */
		prev_node = rb_prev(size_node);
		if (prev_node == NULL)
			goto split_node;

		/* 4.pre node size is equal to size */
		prev_space_node = rb_entry(prev_node, struct hvgr_mem_space_node,
			size_node);
		if (prev_space_node->size == size) {
			tmp_space_node = prev_space_node;
			goto find_equal_node;
		}

		/* 5.pre node size is less than size, split cur node */
		if (prev_space_node->size < size)
			goto split_node;

		/* 6.pre node size is great than size, walk to left */
		link = &(size_node->rb_left);
	}

	mutex_unlock(&(space->space_mutex));
	/* no mem to split */
	return -ENOMEM;

find_equal_node:
	hvgr_mem_space_find_equal_node(space, tmp_space_node, addr);
	mutex_unlock(&(space->space_mutex));
	return 0;

split_node:
	hvgr_mem_space_split_node(space, space_node, addr, size);
	mutex_unlock(&(space->space_mutex));
	return 0;
}

static int hvgr_mem_space_split_spec(struct hvgr_mem_space * const space, uint64_t spec_addr,
	uint64_t size, uint64_t *addr)
{
	struct rb_node **link = NULL;
	struct rb_node *addr_node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;
	int ret;
	uint64_t new_addr;
	uint64_t new_size;

	mutex_lock(&(space->space_mutex));
	link = &(space->addr_root.rb_node);
	while (*link) {
		addr_node = *link;
		space_node = rb_entry(addr_node, struct hvgr_mem_space_node, addr_node);
		/* 1. find target spec_addr, check size */
		if (spec_addr == space_node->start) {
			if (space_node->size < size) {
				mutex_unlock(&(space->space_mutex));
				return -ENOMEM;
			}

			if (space_node->size == size) {
				hvgr_mem_space_find_equal_node(space, space_node, addr);
				mutex_unlock(&(space->space_mutex));
				return 0;
			}

			/*
			 * del space_node.
			 * insert (space_node->addr + size) (space_node->size - size) node.
			 */
			*addr = space_node->start;
			new_addr = space_node->start + size;
			new_size = space_node->size - size;
			hvgr_mem_space_erase_node(space, space_node);
			ret = hvgr_mem_space_insert_node(space, new_addr, new_size);
			mutex_unlock(&(space->space_mutex));
			return ret;
		}

		/* 2. spec_addr < space_node->start, walk to left */
		if (spec_addr < space_node->start) {
			link = &(addr_node->rb_left);
			continue;
		}

		/* 3. spec_addr > (space_node->start + space_node->size), walk to right */
		if (spec_addr > (space_node->start + space_node->size)) {
			link = &(addr_node->rb_right);
			continue;
		}

		/* 4. spec_addr == (space_node->start + space_node->size), no mem to split */
		if (spec_addr == (space_node->start + space_node->size)) {
			mutex_unlock(&(space->space_mutex));
			return -EINVAL;
		}

		/* 5. spec_addr > space_node->start && addr + size > space_node->end,
		 * no mem to split
		 */
		if (spec_addr > space_node->start &&
			((spec_addr + size) > (space_node->start + space_node->size))) {
			mutex_unlock(&(space->space_mutex));
			return -ENOMEM;
		}

		/* 6. spec_addr ~ (spec_addr + size) in space_node
		 * resize space_node size to (spec_addr - space_node->start).
		 * insert (spec_addr + size)
		 * (space_node->start + space_node->size - spec_addr - size) node.
		 */
		*addr = spec_addr;
		new_addr = spec_addr + size;
		new_size = space_node->start + space_node->size - spec_addr - size;

		space_node->size = spec_addr - space_node->start;
		hvgr_mem_space_reinsert_size_node(space, space_node);

		ret = 0;
		if (new_size > 0)
			ret = hvgr_mem_space_insert_node(space, new_addr, new_size);

		mutex_unlock(&(space->space_mutex));
		return ret;
	}

	mutex_unlock(&(space->space_mutex));
	return -ENOMEM;
}

int hvgr_mem_space_split(struct hvgr_mem_space * const space,
	uint64_t spec_addr, uint64_t size, uint64_t *addr)
{
	if (unlikely(space == NULL || addr == NULL || size == 0))
		return -EINVAL;

	if (spec_addr == 0)
		return hvgr_mem_space_split_normal(space, size, addr);
	else
		return hvgr_mem_space_split_spec(space, spec_addr, size, addr);
}

void hvgr_mem_space_term(struct hvgr_mem_space * const space)
{
	struct rb_node *node = NULL;
	struct hvgr_mem_space_node *space_node = NULL;

	if (unlikely(space == NULL))
		return;

	mutex_lock(&(space->space_mutex));

	node = rb_first(&space->addr_root);
	while (node != NULL) {
		space_node = rb_entry(node, struct hvgr_mem_space_node, addr_node);
		hvgr_mem_space_erase_node(space, space_node);

		node = rb_first(&space->addr_root);
	}

	mutex_unlock(&(space->space_mutex));
}

bool hvgr_mem_space_is_empty(struct hvgr_mem_space * const space)
{
	if (unlikely(space == NULL))
		return true;

	return RB_EMPTY_ROOT(&space->addr_root);
}

