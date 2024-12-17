/*
 * cloud_network_utils.c
 *
 * cloud network kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "cloud_network_utils.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <securec.h>
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gfp.h>
#include <net/xfrm.h>
#include <linux/jhash.h>
#include <linux/rtnetlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <uapi/linux/netfilter/nf_nat.h>

static int log_flag = LOG_HI;

int log_flag_get(void)
{
	return log_flag;
}

struct list_node *list_node_alloc(const void *data, unsigned int len)
{
	struct list_node *node = NULL;
	int sec_ret;
	unsigned int size;

	if (len > LIST_NODE_DATA_LEN_MAX) {
		log_err("len %u > LIST_NODE_DATA_LEN_MAX", len);
		return NULL;
	}
	if (data == NULL) {
		log_err("data is NULL");
		return NULL;
	}

	size = len + sizeof(*node);
	node = (struct list_node *)kmalloc(size, GFP_ATOMIC);
	if (node == NULL) {
		log_err("kmalloc failed");
		return NULL;
	}
	node->len = len;
	sec_ret = memcpy_s(node->data, len, data, len);
	if (sec_ret != EOK) {
		log_err("memcpy data failed! sec_ret = %d", sec_ret);
		kfree(node);
		return NULL;
	}
	return node;
}

void list_node_free(struct list_node *node)
{
	if (node != NULL)
		kfree(node);
	return;
}

void init_list_queue(struct list_queue *queue)
{
	if (queue == NULL) {
		log_err("queue is NULL");
		return;
	}
	INIT_LIST_HEAD(&(queue->queue));
	queue->len = 0;
	spin_lock_init(&(queue->lock));
	return;
}

void enqueue_list_queue(struct list_queue *queue, struct list_node *node)
{
	unsigned long flags = 0;

	if ((queue == NULL) || (node == NULL)) {
		log_err("queue or node is NULL");
		return;
	}
	spin_lock_irqsave(&(queue->lock), flags);
	list_add_tail(&node->list, &(queue->queue));
	queue->len++;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return;
}

struct list_node *dequeue_list_queue(struct list_queue *queue)
{
	struct list_node *node = NULL;
	unsigned long flags = 0;

	if (queue == NULL) {
		log_err("queue is NULL");
		return NULL;
	}
	spin_lock_irqsave(&(queue->lock), flags);
	node = list_first_entry(&(queue->queue), struct list_node, list);
	if (node == NULL) {
		spin_unlock_irqrestore(&(queue->lock), flags);
		log_err("get list first entry is NULL");
		return NULL;
	}
	list_del(&(node->list));
	queue->len--;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return node;
}

int list_queue_empty(struct list_queue *queue)
{
	unsigned long flags = 0;
	int ret;

	if (queue == NULL) {
		log_err("queue is NULL");
		return 1; /* when queue is invalid, return 1 */
	}
	spin_lock_irqsave(&(queue->lock), flags);
	ret = list_empty(&(queue->queue));
	spin_unlock_irqrestore(&(queue->lock), flags);
	return ret;
}

unsigned int list_queue_size(struct list_queue *queue)
{
	unsigned long flags = 0;
	unsigned int queue_len;

	if (queue == NULL) {
		log_err("queue is NULL");
		return 0; /* when queue is invalid, return 0 */
	}
	spin_lock_irqsave(&(queue->lock), flags);
	queue_len = queue->len;
	spin_unlock_irqrestore(&(queue->lock), flags);
	return queue_len;
}

void clean_list_queue(struct list_queue *queue)
{
	struct list_node *node = NULL;

	if (queue == NULL) {
		log_err("queue is NULL");
		return;
	}

	while (list_queue_empty(queue) == 0) {
		node = dequeue_list_queue(queue);
		if (node == NULL) {
			log_err("dequeue a null node");
			break;
		}
		list_node_free(node);
		node = NULL;
	}
	return;
}

static void memory_pool_add(struct memory_pool *pool)
{
	struct list_node *node = NULL;

	if (unlikely(pool == NULL)) {
		log_err("input param is invalid, error ");
		return;
	}

	if (atomic_read(&(pool->mm_num)) > MEMORY_POOL_NUM_MAX)
		return;

	log_info("mm_size %u", pool->mm_size);

	node = (struct list_node *)kmalloc(pool->mm_size +
		sizeof(struct list_node), GFP_ATOMIC);
	if (node == NULL) {
		log_err("kmalloc failed");
		return;
	}
	node->len = pool->mm_size;
	if (pool->mm_init_fn != NULL)
		pool->mm_init_fn((void *)(node->data), pool->mm_size);
	enqueue_list_queue(&(pool->free_mm_list), node);
	atomic_inc(&(pool->mm_num));
	return;
}

struct memory_pool *memory_pool_create(void)
{
	struct memory_pool *pool = NULL;

	log_info("create");

	pool = (struct memory_pool *)kmalloc(sizeof(struct memory_pool),
		GFP_ATOMIC);
	if (unlikely(pool == NULL)) {
		log_err("kmalloc failed");
		return NULL;
	}
	(void)memset_s(pool, sizeof(struct memory_pool), 0x00,
		sizeof(struct memory_pool));
	clear_bit(MEMORY_POOL_STATUS, &pool->mm_status);
	return pool;
}

void memory_pool_destroy(struct memory_pool *pool)
{
	log_info("destroy");

	if (unlikely(pool == NULL))
		return;

	kfree(pool);
	return;
}

int memory_pool_init(struct memory_pool *pool, u32 mm_size, u32 create_mm_num,
	bool allow_expand, u32 max_mm_num, mm_init fn)
{
	if (unlikely(fn == NULL || pool == NULL || mm_size > MEMORY_SIZE_MAX ||
		create_mm_num > MEMORY_POOL_NUM_MAX)) {
		log_err("input param is invalid, error ");
		return ERROR;
	}
	log_info("mm_size %u create_mm_num %u max_mm_num %u", mm_size,
		create_mm_num, max_mm_num);

	init_list_queue(&(pool->free_mm_list));
	atomic_set(&(pool->mm_num), 0);
	pool->mm_init_fn = fn;
	pool->mm_size = mm_size;
	pool->allow_expand = allow_expand;
	pool->max_mm_num = max_mm_num;
	for (int i = 0; i < create_mm_num; i++)
		memory_pool_add(pool);
	set_bit(MEMORY_POOL_STATUS, &pool->mm_status);
	return SUCCESS;
}

void memory_pool_deinit(struct memory_pool *pool)
{
	if (unlikely(pool == NULL))
		return;

	if (!test_and_clear_bit(MEMORY_POOL_STATUS, &pool->mm_status))
		return;

	clean_list_queue(&(pool->free_mm_list));
	atomic_set(&(pool->mm_num), 0);
	return;
}

void *memory_pool_alloc(struct memory_pool *pool)
{
	struct list_node *node = NULL;

	if (unlikely(pool == NULL)) {
		log_err("input param is invalid, error ");
		return NULL;
	}
	if (unlikely(!test_bit(MEMORY_POOL_STATUS, &pool->mm_status)))
		return NULL;

	if (list_queue_size(&(pool->free_mm_list)) == 0 && pool->allow_expand)
		memory_pool_add(pool);

	node = dequeue_list_queue(&(pool->free_mm_list));
	if (unlikely(node == NULL)) {
		log_err("node is NULL");
		return NULL;
	}
	return node->data;
}

void memory_pool_free(void *mm, struct memory_pool *pool)
{
	struct list_node *node = NULL;

	if (unlikely(mm == NULL || pool == NULL)) {
		log_err("input param is invalid, error ");
		return;
	}

	node = container_of(mm, struct list_node, data);
	if (unlikely(mm == NULL ||
		!test_bit(MEMORY_POOL_STATUS, &pool->mm_status))) {
		list_node_free(node);
		return;
	}
	enqueue_list_queue(&pool->free_mm_list, node);
	return;
}