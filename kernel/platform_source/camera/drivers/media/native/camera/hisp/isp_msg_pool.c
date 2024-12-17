// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: common isp msg rotation pool
 * Create: 2022-07-29
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 */
#include "isp_msg_pool.h"
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <securec.h>
#include "cam_log.h"

#define msghdr(data) ((struct isp_msg_header *)(data))
#define RPMSG_MAX_PAYLOAD 512
struct isp_msg_node {
	struct list_head link;
	uint32_t msg_size;
	u8 data[RPMSG_MAX_PAYLOAD];
};

#define MAX_ACK_IDS 256U
#define MAX_CACHED_MSGS 128  // same as requested v4l2 event queue count
struct isp_msg_pool {
	spinlock_t lock;
	struct list_head idle_msg;
	struct list_head busy_msg;
	atomic_t total_msg;
	uint32_t pending_msg;
	uint32_t ack_index;
	uint32_t ack_ids[MAX_ACK_IDS];
};

struct isp_msg_pool *create_isp_msg_pool(void)
{
	struct isp_msg_pool *pool;

	pool = kzalloc(sizeof(*pool), GFP_KERNEL);
	if (!pool)
		return NULL;

	spin_lock_init(&pool->lock);
	INIT_LIST_HEAD(&pool->idle_msg);
	INIT_LIST_HEAD(&pool->busy_msg);
	pool->pending_msg = 0;
	atomic_set(&pool->total_msg, 0);
	cam_info("%s: msg pool created", __func__);
	return pool;
}

/* clear_isp_msg_pool - release allocated msg nodes
 * @poll: created isp_msg_pool, support multi pool
 *   splice busy_msg and idle_msg to del_head,
 *   there is possibility that msg node is got but not putted yet,
 *   leave it alone for next time use.
 */
void clear_isp_msg_pool(struct isp_msg_pool *pool)
{
	unsigned long flags;
	struct isp_msg_node *msg = NULL;
	struct isp_msg_node *tmp = NULL;
	uint32_t msg_count = 0;
	LIST_HEAD(idle_head);
	LIST_HEAD(busy_head);

	if (!pool)
		return;

	spin_lock_irqsave(&pool->lock, flags);
	list_splice_init(&pool->idle_msg, &idle_head);
	list_splice_init(&pool->busy_msg, &busy_head);
	pool->pending_msg = 0;
	if (memset_s(&pool->ack_ids, sizeof(pool->ack_ids), 0x00, sizeof(pool->ack_ids)) != EOK)
		cam_err("fail to memset ack_ids");
	spin_unlock_irqrestore(&pool->lock, flags);

	list_for_each_entry_safe(msg, tmp, &idle_head, link) {
		list_del(&msg->link);
		kfree(msg);
		++msg_count;
	}
	list_for_each_entry_safe(msg, tmp, &busy_head, link) {
		cam_info("%s: unfetched isp ack id: %#x, api_name: %#x",
			__func__, msghdr(msg->data)->message_id, msghdr(msg->data)->api_name);
		list_del(&msg->link);
		kfree(msg);
		++msg_count;
	}
	cam_info("%s: msg nodes: allocated:%u, freed:%u",
		__func__, atomic_read(&pool->total_msg), msg_count);
	atomic_sub(msg_count, &pool->total_msg);
}

void destroy_isp_msg_pool(struct isp_msg_pool *pool)
{
	if (!pool)
		return;

	clear_isp_msg_pool(pool);
	kfree(pool);
	cam_info("%s: msg pool destroyed", __func__);
}

static struct isp_msg_node *get_idle_msg(struct isp_msg_pool *pool)
{
	unsigned long flags;
	struct isp_msg_node *node = NULL;

	spin_lock_irqsave(&pool->lock, flags);
	if (!list_empty(&pool->idle_msg)) {
		node = list_first_entry(&pool->idle_msg, struct isp_msg_node, link);
		list_del_init(&node->link);
	}
	spin_unlock_irqrestore(&pool->lock, flags);

	if (node)
		return node;

	if (atomic_read(&pool->total_msg) >= MAX_CACHED_MSGS) {
		cam_err("allocated %u msgs, discard coming msg",
			atomic_read(&pool->total_msg));
		return NULL;
	}

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (node)
		atomic_inc(&pool->total_msg);

	cam_debug("allocated %u msgs", atomic_read(&pool->total_msg));
	return node;
}

static inline void put_idle_msg(struct isp_msg_pool *pool, struct isp_msg_node *node)
{
	unsigned long flags;

	spin_lock_irqsave(&pool->lock, flags);
	list_add_tail(&node->link, &pool->idle_msg);
	spin_unlock_irqrestore(&pool->lock, flags);
}

static inline struct isp_msg_node *get_busy_msg(struct isp_msg_pool *pool)
{
	unsigned long flags;
	struct isp_msg_node *node = NULL;

	spin_lock_irqsave(&pool->lock, flags);
	if (!list_empty(&pool->busy_msg)) {
		node = list_first_entry(&pool->busy_msg, struct isp_msg_node, link);
		list_del_init(&node->link);
		--pool->pending_msg;
	}
	spin_unlock_irqrestore(&pool->lock, flags);
	return node;
}

static inline void put_busy_msg(struct isp_msg_pool *pool, struct isp_msg_node *node)
{
	unsigned long flags;

	spin_lock_irqsave(&pool->lock, flags);
	list_add_tail(&node->link, &pool->busy_msg);
	++pool->pending_msg;
	pool->ack_ids[pool->ack_index] = msghdr(node->data)->message_id;
	pool->ack_index = (pool->ack_index + 1) % MAX_ACK_IDS;
	spin_unlock_irqrestore(&pool->lock, flags);
	cam_debug("%s: api_name:%#x msg_id:%#x queued, %u pending msgs",
		__func__, msghdr(node->data)->api_name,
		msghdr(node->data)->message_id, pool->pending_msg);
}

/* queue_isp_msg - save remote-end sent hisp_msg_t, wait for user's dequeue
 * @pool: created isp_msg_pool, support multi pool
 * @data: kernel space pointer, points to rpmsg ring buffer
 * @len: the length of data @data points to
 *
 * return 0 if save succeeded, else negative errno
 */
int queue_isp_msg(struct isp_msg_pool *pool, void *data, uint32_t len)
{
	struct isp_msg_node *msg = NULL;

	if (!pool || !data) {
		cam_err("providing invalid msg pool or msg data");
		return -EINVAL;
	}

	msg = get_idle_msg(pool);
	if (!msg)
		return -ENOMEM;

	if (memcpy_s(msg->data, sizeof(msg->data), data, len) != EOK) {
		cam_err("%s: copy failed, dest:%zu, len:%u", __func__,
			sizeof(msg->data), len);
		put_idle_msg(pool, msg);
		return -EFAULT;
	}

	msg->msg_size = len;
	put_busy_msg(pool, msg);
	return 0;
}

/* dequeue_isp_msg - user space use ioctl to fetch hisp_msg_t
 * @pool: created isp_msg_pool, support multi pool
 * @data: user space pointer, here copy msg directly to user space
 * @len: the space that @data pointed to
 * @hdr: copy msg header to @hdr for outter usage inside kernel
 *
 * return bytes copied if copy succeeded, else negative errno
 */
int dequeue_isp_msg(struct isp_msg_pool *pool, void __user *data,
	uint32_t len, struct isp_msg_header *hdr)
{
	int rc;
	struct isp_msg_node *msg = NULL;

	if (!pool || !data) {
		cam_err("%s: providing invalid msg pool or msg data", __func__);
		return -EINVAL;
	}

	msg = get_busy_msg(pool);
	if (!msg) {
		cam_warn("%s: no available msg", __func__);
		return -EAGAIN;
	}

	if (len != msg->msg_size)
		cam_debug("%s: isp send msg size:%u, hal request size:%u",
			__func__, msg->msg_size, len);

	len = min_t(u32, len, msg->msg_size);
	if (copy_to_user(data, msg->data, len)) {
		cam_err("%s: copy to user failed, msg size:%u",
			__func__, msg->msg_size);
		rc = -EFAULT;
	} else {
		rc = len; // if copy successful, return bytes copied
	}

	if (hdr && msg->msg_size >= sizeof(*hdr))
		*hdr = *(struct isp_msg_header *)msg->data;
	put_idle_msg(pool, msg);
	return rc;
}

void dump_isp_ack_ids(struct isp_msg_pool *pool)
{
	unsigned long flags;
	uint32_t index = 0;
	uint32_t ack_ids[MAX_ACK_IDS];

	if (!pool)
		return;

	spin_lock_irqsave(&pool->lock, flags);
	for (index = 0; index < ARRAY_SIZE(ack_ids); ++index)
		ack_ids[index] = pool->ack_ids[(pool->ack_index + index) % MAX_ACK_IDS];
	spin_unlock_irqrestore(&pool->lock, flags);
	// dump acked msg ids, from oldest to newest
	print_hex_dump(KERN_INFO, "ackids: ", DUMP_PREFIX_OFFSET, 32, 4, (void *)&ack_ids[0], sizeof(ack_ids), false);
}
