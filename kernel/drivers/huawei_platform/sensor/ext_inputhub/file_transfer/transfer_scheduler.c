// SPDX-License-Identifier: GPL-2.0
/*
 * file_transfer.c
 *
 * source file for file transfer
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
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <uapi/linux/sched/types.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_api.h"
#include "transfer_commu.h"
#include "transfer_topic.h"
#include "transfer_work.h"
#include "transfer_buffer.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define TRANSFER_SID MULTI_MCU_COMMU_SVR
#define TRANSFER_CID INNER_CMD_COMMU_CID

struct transfer_scheduler {
	struct kthread_work kwork;
	struct kthread_worker kworker;
	struct task_struct *kworker_task;
	/* scheduler event list header */
	struct list_head list;
	/* lock for scheduler event list */
	struct mutex list_lock;
	wait_queue_head_t wait_queue;
};

struct schedule_event {
	struct list_head list;
	struct transfer_record *record;
	enum transfer_type transfer_type;
};

static struct transfer_scheduler transfer_scheduler = {0};

static void transfer_scheduler_func(struct kthread_work *work)
{
	bool ret = false;
	struct schedule_event *data_node = NULL;
	struct schedule_event *tmp_node = NULL;
	int wait_ret;

	hwlog_info("%s get in", __func__);
	init_transfer_work();

	while (!kthread_should_stop()) {
		/* wait for transfer_scheduler event */
		wait_ret = wait_event_interruptible(transfer_scheduler.wait_queue,
						    !list_empty(&transfer_scheduler.list));

		mutex_lock(&transfer_scheduler.list_lock);
		list_for_each_entry_safe(data_node, tmp_node, &transfer_scheduler.list, list) {
			if (!data_node)
				continue;
			/* queue transfer to transfer worker */
			ret = queue_transfer_work(data_node->transfer_type, data_node->record);
			if (ret) {
				/* queue successfully, delete and free the node record */
				list_del(&data_node->list);
				kfree(data_node);
				data_node = NULL;
			} else {
				/* queue to worker fail, wait 10ms for next time */
				usleep_range(10000, 10001);
			}
			break;
		}
		mutex_unlock(&transfer_scheduler.list_lock);
	}

	exit_transfer_work();
}

int queue_remote_transfer(int rfd, enum transfer_type type, struct file_transfer *file_req)
{
	struct transfer_record *record = NULL;
	struct schedule_event *event = NULL;

	hwlog_info("%s get callback topic handshake msg\n", __func__);
	if (!file_req)
		return -EINVAL;

	record = generate_remote_record(rfd);
	if (!record)
		return -EFAULT;
	record->err_code = TRANS_STATUS_START;
	record->trans_info = file_req;

	event = kmalloc(sizeof(*event), GFP_KERNEL);
	if (!event) {
		release_record(record);
		return -EFAULT;
	}

	event->transfer_type = type;
	event->record = record;
	/* add work event to scheduler */
	INIT_LIST_HEAD(&event->list);
	mutex_lock(&transfer_scheduler.list_lock);
	list_add_tail(&event->list, &transfer_scheduler.list);
	mutex_unlock(&transfer_scheduler.list_lock);
	wake_up_interruptible(&transfer_scheduler.wait_queue);
	return 0;
}

static struct file_transfer *construct_file_transfer(struct transfer_handshake *handshake)
{
	u32 path_len = 0;
	char *local_path = NULL;
	struct file_transfer *file_req = NULL;

	file_req = kzalloc(sizeof(*file_req), GFP_KERNEL);
	if (!file_req)
		return NULL;

	file_req->topic_id = handshake->topic_id;
	file_req->file.data_len = handshake->data_len;
	file_req->file.src_offset = handshake->src_offset;
	file_req->file.dst_offset = handshake->dst_offset;

	/* query path by topic id, check in transfer work */
	local_path = query_local_path(handshake->topic_id, &path_len);
	file_req->file.path_len = path_len;
	file_req->file.local_path = local_path;

	hwlog_info("%s topic handshake msg path:%s\n", __func__, local_path);
	return file_req;
}

static int queue_topic_handshake(int rfd, void *data, u32 data_len)
{
	int ret;
	enum transfer_type type = REMOTE_TRANSFER;
	struct file_transfer *file_req = NULL;
	struct transfer_handshake *handshake = NULL;

	hwlog_info("%s get callback topic handshake msg\n", __func__);
	if (!data || data_len < sizeof(*handshake))
		return -EINVAL;

	handshake = (struct transfer_handshake *)data;
	file_req = construct_file_transfer(handshake);
	if (!file_req)
		return -EFAULT;

	if ((handshake->transfer_flag & FILE_TRANS_FETCH) != 0)
		type = REMOTE_FETCH;

	ret = queue_remote_transfer(handshake->rfd, type, file_req);
	if (ret != 0) {
		kfree(file_req->file.local_path);
		kfree(file_req);
		return -EFAULT;
	}
	/* success deal the data, free resource */
	kfree(handshake->path);
	handshake->path = NULL;
	kfree(handshake);
	return 0;
}

int queue_file_work(s8 tid, int trans_type, struct file_transfer *file_transfer)
{
	struct schedule_event *event = NULL;
	struct transfer_record *record = NULL;

	if (!file_transfer)
		return -EINVAL;

	/* is_force & need_verify deal later */
	record = generate_available_record(tid);
	if (!record)
		return -EFAULT;

	record->trans_info = file_transfer;
	record->err_code = TRANS_STATUS_START;
	event = kmalloc(sizeof(*event), GFP_KERNEL);
	if (!event) {
		release_record(record);
		return -EFAULT;
	}

	event->record = record;
	event->transfer_type = trans_type;
	INIT_LIST_HEAD(&event->list);
	mutex_lock(&transfer_scheduler.list_lock);
	/* event will deal in scheduler */
	list_add_tail(&event->list, &transfer_scheduler.list);
	mutex_unlock(&transfer_scheduler.list_lock);

	hwlog_info("%s get fd:%08x", __func__, record->rfd);
	wake_up_interruptible(&transfer_scheduler.wait_queue);
	return record->rfd;
}

static int transfer_file_process(unsigned char *data, int data_len)
{
	struct file_transfer *file_transfer = NULL;

	if (data_len < sizeof(*file_transfer))
		return -EINVAL;

	file_transfer = convert_to_path_type(data, data_len, sizeof(*file_transfer),
					     offsetof(struct file_transfer, file) +
					     offsetof(struct file_info, path_len),
					     offsetof(struct file_transfer, file) +
					     offsetof(struct file_info, local_path));

	return queue_file_work(TRANSFER_FILE_TID, AP_TRANSFER, file_transfer);
}

static int fetch_file_process(unsigned char *data, int data_len)
{
	struct file_transfer *file_transfer = NULL;

	if (data_len < sizeof(*file_transfer))
		return -EINVAL;

	file_transfer = convert_to_path_type(data, data_len, sizeof(*file_transfer),
					     offsetof(struct file_transfer, file) +
					     offsetof(struct file_info, path_len),
					     offsetof(struct file_transfer, file) +
					     offsetof(struct file_info, local_path));

	return queue_file_work(FETCH_FILE_TID, AP_FETCH, file_transfer);
}

/* callback when get remote file transfer message */
static int transfer_message_callback(int rfd, enum transfer_msg_type type, void *data, u32 len)
{
	if (!data || len <= 0) {
		hwlog_err("%s get remote msg invalid", __func__);
		return -EINVAL;
	}

	/* 1、handshake msg will deal by main worker */
	if (type == MSG_TOPIC_HANDSHAKE)
		return queue_topic_handshake(rfd, data, len);

	if (type == MSG_BUFFER_HANDSHAKE)
		return queue_buffer_handshake(rfd, data, len);
	/* 2. handshake ack & transfer data & data ack & error resp */
	return queue_work_event(rfd, data, len);
}

static int init_transfer_scheduler(void)
{
	struct sched_param param = {
		.sched_priority = 5
	};
	bool ret = false;

	kthread_init_worker(&transfer_scheduler.kworker);
	kthread_init_work(&transfer_scheduler.kwork, transfer_scheduler_func);
	transfer_scheduler.kworker_task = kthread_run(kthread_worker_fn,
						      &transfer_scheduler.kworker,
						      "%s", "transfer_scheduler");
	if (IS_ERR(transfer_scheduler.kworker_task)) {
		hwlog_err("%s failed to create transfer main task", __func__);
		return -EFAULT;
	}

	sched_setscheduler(transfer_scheduler.kworker_task, SCHED_FIFO, &param);
	mutex_init(&transfer_scheduler.list_lock);
	INIT_LIST_HEAD(&transfer_scheduler.list);

	init_waitqueue_head(&transfer_scheduler.wait_queue);
	ret = kthread_queue_work(&transfer_scheduler.kworker, &transfer_scheduler.kwork);
	hwlog_info("%s queue file transfer mian work ret:%d", __func__, ret);
	return 0;
}

static int __init transfer_scheduler_init(void)
{
	int ret;

	hwlog_info("%s get in", __func__);
	init_transfer_scheduler();
	init_transfer_commu(transfer_message_callback);

	ret = register_transfer_cmd(TRANSFER_SID, TRANSFER_CID, TRANSFER_FILE_TOPIC_TYPE,
				    transfer_file_process);
	hwlog_info("%s register (transfer file) cmd ret:%d", __func__, ret);

	ret = register_transfer_cmd(TRANSFER_SID, TRANSFER_CID, FETCH_FILE_TOPIC_TYPE,
				    fetch_file_process);
	hwlog_info("%s register (fetch file) cmd ret:%d", __func__, ret);

	return 0;
}

static void __exit transfer_scheduler_exit(void)
{
	if (!IS_ERR(transfer_scheduler.kworker_task))
		kthread_stop(transfer_scheduler.kworker_task);
}

module_init(transfer_scheduler_init);
module_exit(transfer_scheduler_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei file transfer driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
