// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_work.c
 *
 * source file for transfer works
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
#include "transfer_work.h"

#include <linux/slab.h>
#include <huawei_platform/log/hw_log.h>
#include "transfer_processor.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define SUB_WORKER_COUNT 2

static struct transfer_worker sub_worker[SUB_WORKER_COUNT];

static void clear_last_events(struct transfer_worker *worker)
{
	struct work_event *work_event = NULL;
	struct work_event *work_temp = NULL;

	mutex_lock(&worker->list_lock);
	list_for_each_entry_safe(work_event, work_temp, &worker->list, list) {
		/* match with topic mask */
		list_del(&work_event->list);
		if (work_event)
			kfree(work_event->data);
		kfree(work_event);
		work_event = NULL;
	}
	mutex_unlock(&worker->list_lock);
}

/* need wake lock avoid SR */
void transfer_worker(struct kthread_work *work)
{
	int rfd;
	enum transfer_type type;
	struct transfer_processor **processors = get_transfer_processors();
	struct transfer_worker *worker = container_of(work, struct transfer_worker, kwork);

	hwlog_info("%s get in", __func__);
	if (!worker || !worker->record || !worker->record->trans_info) {
		hwlog_err("%s work param is invalid", __func__);
		return;
	}

	rfd = worker->record->rfd;
	type = worker->transfer_type;
	hwlog_info("%s rfd:%08x begin, type:%d", __func__, rfd, type);

	/* 1. clear last work event */
	clear_last_events(worker);

	/* 2. get processor and process */
	if (type < MAX_TRANSFER_TYPE && processors && processors[type] && processors[type]->process)
		processors[type]->process(processors[type], worker);
	else
		hwlog_warn("%s rfd:%08x get invalid type:%d", __func__, rfd, worker->transfer_type);

	clear_last_events(worker);
	/* 3. reset state after transfer */
	mutex_lock(&worker->list_lock);
	if (worker->trans_data) {
		kfree(worker->trans_data->data);
		kfree(worker->trans_data);
		worker->trans_data = NULL;
	}
	if (worker->file) {
		release_transfer_file(worker->file);
		worker->file = NULL;
	}
	release_record(worker->record);
	worker->record = NULL;
	worker->idle = 1;
	mutex_unlock(&worker->list_lock);
}

static bool check_path_transferring(struct transfer_record *record)
{
	int i;

	/* check if the path is processing by other worker */
	for (i = 0; i < SUB_WORKER_COUNT; ++i) {
		mutex_lock(&sub_worker[i].list_lock);
		if (!sub_worker[i].record || !sub_worker[i].record->trans_info) {
			mutex_unlock(&sub_worker[i].list_lock);
			continue;
		}

		/* the same path is in process */
		if (record->trans_info->file.local_path &&
		    (strcmp(record->trans_info->file.local_path,
			   sub_worker[i].record->trans_info->file.local_path) == 0)) {
			mutex_unlock(&sub_worker[i].list_lock);
			return true;
		}

		mutex_unlock(&sub_worker[i].list_lock);
	}
	return false;
}

bool queue_transfer_work(enum transfer_type transfer_type, struct transfer_record *record)
{
	int i = SUB_WORKER_COUNT;
	bool ret = false;

	if (!record || !record->trans_info)
		return false;

	/* check if the path is running */
	if (check_path_transferring(record))
		return false;

	/* get idle worker */
	if (transfer_type == TRANSFER_BUFFER || transfer_type == REMOTE_TRANSFER_BUFFER) {
		/* index 1 for buffer transfer */
		if (sub_worker[1].idle == 1)
			i = 1;
	} else if (transfer_type < MAX_TRANSFER_TYPE) {
		/* index 0 for file transfer */
		if (sub_worker[0].idle == 1)
			i = 0;
	} else {
		/* do nothing */
	}

	if (i >= SUB_WORKER_COUNT)
		return false;

	/* sub work will done the transfer record */
	sub_worker[i].idle = 0;
	sub_worker[i].record = record;
	sub_worker[i].transfer_type = transfer_type;
	ret = kthread_queue_work(&sub_worker[i].kworker, &sub_worker[i].kwork);
	if (!ret) {
		/* queue failed, rollback record */
		hwlog_warn("%s queue sub worker failed", __func__);
		sub_worker[i].record = NULL;
		sub_worker[i].idle = 1;
	}
	return ret;
}

int queue_work_event(int rfd, char *data, u32 data_len)
{
	int i;
	struct work_event *work_event = NULL;

	for (i = 0; i < SUB_WORKER_COUNT; ++i) {
		/* get the sub worker who needs to deal the message. (id: rfd) */
		if (!sub_worker[i].record || sub_worker[i].record->rfd != rfd)
			continue;
		/* initial event */
		work_event = kmalloc(sizeof(*work_event), GFP_KERNEL);
		if (!work_event)
			return -EFAULT;
		work_event->data = data;
		work_event->data_len = data_len;
		INIT_LIST_HEAD(&work_event->list);

		mutex_lock(&sub_worker[i].list_lock);
		/* the event will deal by the worker */
		list_add_tail(&work_event->list, &sub_worker[i].list);
		mutex_unlock(&sub_worker[i].list_lock);
		wake_up_interruptible(&sub_worker[i].event_wait);
		return 0;
	}

	hwlog_warn("%s cannot queue event to transfer work", __func__);
	return -EINVAL;
}

void init_transfer_work(void)
{
	int i;

	for (i = 0; i < SUB_WORKER_COUNT; ++i) {
		sub_worker[i].idle = 0;
		kthread_init_worker(&sub_worker[i].kworker);
		kthread_init_work(&sub_worker[i].kwork, transfer_worker);
		sub_worker[i].kworker_task = kthread_run(kthread_worker_fn, &sub_worker[i].kworker,
							 "%s_%d", "transfer_sub_worker", i);
		if (IS_ERR(sub_worker[i].kworker_task)) {
			hwlog_err("%s failed to create transfer sub task:%d", __func__, i);
			continue;
		}
		/* transfer_type and transfer_record will set when processing */
		sub_worker[i].record = NULL;
		mutex_init(&sub_worker[i].list_lock);
		INIT_LIST_HEAD(&sub_worker[i].list);
		init_waitqueue_head(&sub_worker[i].event_wait);
		sub_worker[i].idle = 1;
	}
}

void exit_transfer_work(void)
{
	int i;

	/* stop sub worker thread */
	for (i = 0; i < SUB_WORKER_COUNT; ++i) {
		if (!IS_ERR(sub_worker[i].kworker_task))
			kthread_stop(sub_worker[i].kworker_task);
	}
}
