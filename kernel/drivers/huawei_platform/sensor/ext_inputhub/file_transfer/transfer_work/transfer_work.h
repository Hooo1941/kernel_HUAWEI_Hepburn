/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_work.h
 *
 * the head file for file transfer works
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
#ifndef TRANSFER_WORK_H
#define TRANSFER_WORK_H

#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/wait.h>
#include "file_mgr.h"
#include "transfer_record.h"
#include "transfer_commu.h"

#define PACKAGE_READ_LEN 3000
#define WAIT_REMOTE_TIMEOUT 600

struct work_event {
	struct list_head list;
	int event;
	void *data;
	u32 data_len;
};

struct transfer_worker {
	enum transfer_type transfer_type;
	/* if transfer woker is idle */
	int idle;
	/* kwork */
	struct kthread_work kwork;
	struct task_struct *kworker_task;
	struct kthread_worker kworker;
	/* for worker events */
	struct list_head list;
	wait_queue_head_t event_wait;
	/* lock for transfer worker event list */
	struct mutex list_lock;
	/* transfer details */
	struct transfer_record *record;
	/* transfer context */
	struct transfer_file *file;
	struct transfer_data *trans_data;
};

/*
 * initial transfer works
 */
void init_transfer_work(void);

/*
 * release transfer works
 */
void exit_transfer_work(void);

/*
 * queue transfer work event for the certain rfd
 * if no work process the rfd will return failure
 * @param rfd the rfd
 * @param data event data buffer
 * @param len event data buffer length
 * @return >=0 for success, otherwise for fail
 */
int queue_work_event(int rfd, char *data, u32 len);

/*
 * queue a transfer work, the transfer will start by woker
 * @param type the transfer type
 * @param record file record will start
 * @return >=0 for success, otherwise for fail
 */
bool queue_transfer_work(enum transfer_type type, struct transfer_record *record);

#endif
