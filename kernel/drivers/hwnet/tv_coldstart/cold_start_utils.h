/*
 * cold_start_utils.h
 *
 * cold start kernel module tool function implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _COLD_START_UTILS_H_
#define _COLD_START_UTILS_H_

#include <linux/kernel.h>
#include <linux/spinlock.h>

/* return code */
#define ERROR (-1)
#define SUCCESS 0

#define LIST_NODE_DATA_LEN_MAX 16384

int tvcold_get_log_flag(void);
#define cold_start_log_print(level, fmt, ...) \
	do { \
		if (tvcold_get_log_flag() < level) { \
			break; \
		} \
		pr_err(fmt, ##__VA_ARGS__); \
	} while (0)

#define cold_start_log_info(fmt, ...) \
	cold_start_log_print(COLD_START_LOG_LOW, "cold_start<INFO>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define cold_start_log_warn(fmt, ...) \
	cold_start_log_print(COLD_START_LOG_MID, "cold_start<WARNNG>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define cold_start_log_err(fmt, ...) \
	cold_start_log_print(COLD_START_LOG_HI, "cold_start<ERROR>[%s:%d]: " \
		fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum cold_start_log_level {
	COLD_START_LOG_NULL = 0,
	COLD_START_LOG_LOW = 1,
	COLD_START_LOG_MID = 2,
	COLD_START_LOG_HI = 4,
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

struct list_node *tvcold_alloc_list_node(const void *data, unsigned int len);
void tvcold_free_list_node(struct list_node *node);
void tvcold_list_queue_init(struct list_queue *queue);
void tvcold_list_queue_enqueue(struct list_queue *queue, struct list_node *node);
struct list_node *tvcold_list_queue_dequeue(struct list_queue *queue);
int tvcold_list_queue_is_empty(struct list_queue *queue);
unsigned int tvcold_list_queue_len(struct list_queue *queue);
void tvcold_list_queue_clean(struct list_queue *queue);
#endif /* _COLD_START_UTILS_H_ */
