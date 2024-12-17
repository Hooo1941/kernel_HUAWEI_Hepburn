/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_topic.h
 *
 * topic manager head file for file transfer
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#ifndef TRANSFER_TOPIC_H
#define TRANSFER_TOPIC_H

#include <linux/types.h>

/*
 * query path by topic id
 * @param topic_id id to find the path
 * @param path_len the output path len, if not found reuturn 0
 * @return the path if found, otherwise return NULL
 */
char *query_local_path(u64 topic_id, u32 *path_len);

#endif
