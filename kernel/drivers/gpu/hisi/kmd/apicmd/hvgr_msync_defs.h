/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef MSYNC_DEFS_H
#define MSYNC_DEFS_H

#include "linux/types.h"

#define MSYNC_QUEUE_LENGTH 4096
#define MSYNC_TRANS_MAX    32

enum msync_status {
	BASE_MSYNC_NOT_STARTED = 0x00,
	BASE_MSYNC_SIGNAL = 0x01,
	BASE_MSYNC_TIME_OUT = 0x02,
	BASE_MSYNC_TASK_CANCELLED = 0x05,
};

struct msync_event {
	enum msync_status status;
	uint32_t data;
	uint32_t queue_id;
};

struct hvgr_msync {
	uint64_t sem_addr;
	uint64_t sem_value;
	uint32_t token;
	int fd;
};

#endif
