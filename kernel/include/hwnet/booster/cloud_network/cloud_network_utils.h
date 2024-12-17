/*
 * cloud_network_utils.h
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

#ifndef _CLOUD_NETWORK_UTILS_H_
#define _CLOUD_NETWORK_UTILS_H_

#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>

/* return code */
#define ERROR (-1)
#define SUCCESS 0

#define MEMORY_SIZE_MAX 49920
#define MEMORY_POOL_NUM_MAX 2048

#define LIST_NODE_DATA_LEN_MAX 8192

int log_flag_get(void);
#define log_print(level, fmt, ...) \
	do { \
		if (log_flag_get() < level) { \
			break; \
		} \
		pr_err(fmt, ##__VA_ARGS__); \
	} while (0)

#define log_info(fmt, ...) \
	log_print(LOG_LOW, "cloud network<INFO>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define log_warn(fmt, ...) \
	log_print(LOG_MID, "cloud network<WARNNG>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define log_err(fmt, ...) \
	log_print(LOG_HI, "cloud network<ERROR>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum log_level {
	LOG_NULL = 0,
	LOG_LOW = 1,
	LOG_MID = 2,
	LOG_HI = 4,
};

enum {
	MEMORY_POOL_STATUS,
};

struct list_node {
	struct list_head list;
	unsigned int len;
	char data[0];
};

struct list_queue {
	struct list_head queue;
	unsigned int len;
	spinlock_t lock;
};

typedef void (*mm_init)(void *data, u32 data_size);

struct memory_pool {
	struct list_queue free_mm_list;
	atomic_t mm_num;
	mm_init mm_init_fn;
	u32 mm_size;
	bool allow_expand;
	u32 max_mm_num;
	unsigned long mm_status;
};

struct list_node *list_node_alloc(const void *data, unsigned int len);
void list_node_free(struct list_node *node);
void init_list_queue(struct list_queue *queue);
void enqueue_list_queue(struct list_queue *queue, struct list_node *node);
struct list_node *dequeue_list_queue(struct list_queue *queue);
int list_queue_empty(struct list_queue *queue);
unsigned int list_queue_size(struct list_queue *queue);
void clean_list_queue(struct list_queue *queue);
int list_queue_empty(struct list_queue *queue);
struct memory_pool *memory_pool_create(void);
void memory_pool_destroy(struct memory_pool *pool);
int memory_pool_init(struct memory_pool *pool, u32 mm_size, u32 create_mm_num,
	bool allow_expand, u32 max_mm_num, mm_init fn);
void memory_pool_deinit(struct memory_pool *pool);
void *memory_pool_alloc(struct memory_pool *pool);
void memory_pool_free(void *mm, struct memory_pool *pool);
#endif /* _CLOUD_NETWORK_UTILS_H_ */
