// SPDX-License-Identifier: GPL-2.0
/*
 * data_transfer.c
 *
 * source file for data transfer steps
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
#include "data_transfer.h"

#include <linux/slab.h>
#include <linux/types.h>
#include <huawei_platform/log/hw_log.h>

#define ACK_PACKAGE_COUNT 8

#define HWLOG_TAG file_trans
HWLOG_REGIST();

static int wait_data_ack(struct transfer_worker *worker, struct transfer_file *file, int read_count)
{
	int ret;
	int rfd = worker->record->rfd;
	struct work_event *work_event = NULL;
	struct transfer_data_ack *data_ack_event = NULL;
	u32 aim_flag = FILE_TRANS_DATA_PCK | FILE_TRANS_ACK;

	/*
	 * need wait ack, the flag set when send transfer data
	 * 1. every ACK_PACKAGE_COUNT packages
	 * 2. the last package
	 */
	if (read_count % ACK_PACKAGE_COUNT != 0 && file->state != END)
		return 0;

	hwlog_info("%s rfd:%08x wait data ack", __func__, rfd);
	/* 1. try to wait for data ack event */
	ret = wait_event_interruptible_timeout(worker->event_wait, !list_empty(&worker->list),
					       msecs_to_jiffies(WAIT_REMOTE_TIMEOUT));
	if (ret <= 0) {
		hwlog_err("%s rfd:%08x wait data err", __func__, rfd);
		return -ETIME;
	}
	/* 2. event list is not empty and get from event list */
	mutex_lock(&worker->list_lock);
	work_event = list_first_entry(&worker->list, struct work_event, list);
	if (work_event)
		list_del(&work_event->list);
	mutex_unlock(&worker->list_lock);
	/* 3. check event is valid */
	if (!work_event || !work_event->data ||
	    work_event->data_len < sizeof(struct transfer_data_ack)) {
		ret = -EINVAL;
		goto out;
	}
	/* 4. check is a data ack package by transfer flag(need to check data error flag) */
	data_ack_event = (struct transfer_data_ack *)(work_event->data);
	if ((data_ack_event->transfer_flag & aim_flag) != aim_flag) {
		ret = -EINVAL;
		goto out;
	}
	ret = 0;
out:
	if (work_event) {
		kfree(work_event->data);
		kfree(work_event);
	}
	return ret;
}

int transfer_file_data(struct transfer_worker *worker)
{
	int ret = 0;
	int read_count = 0;
	int rfd = worker->record->rfd;
	struct transfer_file *read_file = worker->file;
	struct transfer_data *trans_data = worker->trans_data;

	if (!read_file || !trans_data) {
		hwlog_info("%s rfd:%08x invalid file to read", __func__, rfd);
		return -EINVAL;
	}
	/* transfer to the end */
	while (read_file->state != END) {
		ret = read_file->data_process(read_file, trans_data->data, PACKAGE_READ_LEN);
		if (ret <= 0) {
			hwlog_err("%s rfd:%08x process file data error:%d", __func__, rfd, ret);
			ret = -EFAULT;
			break;
		}

		read_count++;
		trans_data->transfer_flag = FILE_TRANS_DATA_PCK;
		/*
		 * set need ack flag to avoid overflow, need wait ack later
		 * 1. every ACK_PACKAGE_COUNT packages
		 * 2. the last package
		 */
		if (read_count % ACK_PACKAGE_COUNT == 0 || read_file->state == END)
			trans_data->transfer_flag |= FILE_TRANS_NEED_ACK;
		trans_data->data_len = ret;
		trans_data->offset = read_file->dst_offset - trans_data->data_len;
		/* send transfer data to remote */
		ret = send_transfer_requeset(worker->transfer_type,
					     trans_data, sizeof(*trans_data));
		if (ret < 0) {
			hwlog_info("%s rfd:%08x send file data:%d", __func__, rfd, ret);
			break;
		}
		/* wait if need ack */
		ret = wait_data_ack(worker, read_file, read_count);
		if (ret < 0)
			break;
	}
	/* release file resource */
	kfree(trans_data->data);
	trans_data->data = NULL;
	kfree(trans_data);
	worker->trans_data = NULL;

	release_transfer_file(read_file);
	worker->file = NULL;

	return ret;
}

static int send_data_ack(enum transfer_type type,
			 struct work_event *work_event, struct transfer_data *data_event)
{
	int ret;
	struct transfer_data_ack data_ack;

	/* check transfer flag */
	if ((data_event->transfer_flag & FILE_TRANS_NEED_ACK) == 0) {
		ret = 0;
		goto end;
	}

	data_ack.rfd = data_event->rfd;
	data_ack.offset = data_event->offset;
	data_ack.data_len = data_event->data_len;
	data_ack.transfer_flag = FILE_TRANS_DATA_PCK | FILE_TRANS_ACK;

	ret = send_transfer_requeset(type, &data_ack, sizeof(data_ack));
	if (ret < 0)
		hwlog_err("%s send ack err:%d", __func__, ret);
end:
	kfree(data_event->data);
	data_event->data = NULL;
	kfree(work_event->data);
	work_event->data = NULL;
	kfree(work_event);
	return ret;
}

static bool mark_offset_mismatch(struct transfer_file *write_file, u32 offset)
{
	if (write_file->c_offset == offset)
		return false;

	hwlog_warn("%s receive:%d offset not match:%d", __func__, offset, write_file->c_offset);
	return true;
}

int receive_transfer_data(struct transfer_worker *worker)
{
	int ret;
	int rfd = worker->record->rfd;
	struct work_event *work_event = NULL;
	struct transfer_data *data_event = NULL;
	struct transfer_file *write_file = worker->file;

	if (!write_file)
		return -EFAULT;

	/* receive to the aim length */
	while (write_file->state != END) {
		/* 1. try to get event data */
		ret = wait_event_interruptible_timeout(worker->event_wait,
						       !list_empty(&worker->list),
						       msecs_to_jiffies(WAIT_REMOTE_TIMEOUT));
		if (ret <= 0) {
			hwlog_err("%s rfd:%08x wait file data err:%d", __func__, rfd, ret);
			return -ETIME;
		}
		mutex_lock(&worker->list_lock);
		work_event = list_first_entry(&worker->list, struct work_event, list);
		if (work_event)
			list_del(&work_event->list);
		mutex_unlock(&worker->list_lock);
		/* 2. check data valid */
		ret = -EINVAL;
		if (!work_event || !work_event->data ||
		    work_event->data_len < sizeof(struct transfer_data))
			break;

		/* 3. check transfer flag is data ack (need check error flag) */
		data_event = (struct transfer_data *)(work_event->data);
		if ((data_event->transfer_flag & FILE_TRANS_DATA_PCK) != FILE_TRANS_DATA_PCK)
			break;

		/* 4. receive data from remote (need check aim data offset) */
		if (mark_offset_mismatch(write_file, data_event->offset))
			break;
		if (write_file->data_process(write_file, data_event->data,
			data_event->data_len) <= 0)
			break;
		/* 5. send data ack if transfer flag mark */
		ret = send_data_ack(worker->transfer_type, work_event, data_event);
		work_event = NULL;
		data_event = NULL;
		if (ret < 0)
			break;
	}
	/* 6. release file resource */
	if (data_event)
		kfree(data_event->data);
	if (work_event) {
		kfree(work_event->data);
		work_event->data = NULL;
	}
	kfree(work_event);
	release_transfer_file(write_file);
	worker->file = NULL;

	return ret;
}
