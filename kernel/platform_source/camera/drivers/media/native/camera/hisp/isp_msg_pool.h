/* SPDX-License-Identifier: GPL-2.0 */
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
#ifndef ISP_MSG_POOL_H
#define ISP_MSG_POOL_H
#include <linux/types.h>

struct isp_msg_pool;

/* isp_msg_header - isp msg header
 * take first 3 u32 of hisp_msg_t cared in kernel
 */
struct isp_msg_header {
	unsigned int message_size;
	unsigned int api_name;
	unsigned int message_id;
};

struct isp_msg_pool *create_isp_msg_pool(void);
void clear_isp_msg_pool(struct isp_msg_pool *pool);
void destroy_isp_msg_pool(struct isp_msg_pool *pool);

int queue_isp_msg(struct isp_msg_pool *pool, void *data, uint32_t len);
int dequeue_isp_msg(struct isp_msg_pool *pool, void __user *data,
	uint32_t len, struct isp_msg_header *hdr);

void dump_isp_ack_ids(struct isp_msg_pool *pool);
#endif /* end of include guard: ISP_MSG_POOL_H */
