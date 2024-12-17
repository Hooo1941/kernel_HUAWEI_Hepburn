// SPDX-License-Identifier: GPL-2.0
/*
 * handshake.c
 *
 * source file for file transfer handshake steps
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
#include "handshake.h"

#include <linux/slab.h>
#include <linux/types.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG file_trans
HWLOG_REGIST();

int send_handshake(struct transfer_worker *worker)
{
	int ret;
	int rfd = worker->record->rfd;
	struct transfer_handshake handshake;

	if (!worker->file)
		return -EFAULT;
	/* parameter check in transfer work */
	handshake.rfd = rfd;
	handshake.data_len = worker->file->len;
	handshake.topic_id = worker->record->trans_info->topic_id;
	handshake.path_len = worker->record->trans_info->file.path_len;
	handshake.src_offset = worker->record->trans_info->file.src_offset;
	handshake.dst_offset = worker->record->trans_info->file.dst_offset;
	/* don't need copy */
	handshake.path = worker->record->trans_info->file.local_path;

	handshake.transfer_flag = FILE_TRANS_HANDSHAKE | FILE_TRANS_TYPE_TOPIC;
	ret = send_transfer_requeset(worker->transfer_type, &handshake, sizeof(handshake));
	hwlog_info("%s rfd:%08x handshake ret:%d", __func__, rfd, ret);

	return ret;
}

int send_handshake_ack(struct transfer_worker *worker)
{
	int rfd = worker->record->rfd;
	struct transfer_handshake_ack ack;

	hwlog_info("%s rfd:%08x get in", __func__, rfd);
	if (!worker->file) {
		hwlog_err("%s rfd:%08x file is null", __func__, rfd);
		return -EFAULT;
	}
	/* parameter check in transfer work */
	ack.rfd = rfd;
	ack.topic_id = worker->record->trans_info->topic_id;
	/* don't need copy */
	ack.path = worker->record->trans_info->file.local_path;
	ack.path_len = worker->record->trans_info->file.path_len;
	ack.transfer_flag = FILE_TRANS_HANDSHAKE | FILE_TRANS_ACK | FILE_TRANS_TYPE_TOPIC;
	if (worker->transfer_type == AP_FETCH || worker->transfer_type == REMOTE_FETCH) {
		/* fetch handshake ack */
		ack.dst_offset = worker->file->offset;
		ack.data_len = worker->file->len;
	}
	return send_transfer_requeset(worker->transfer_type, &ack, sizeof(ack));
}

int wait_handshake_ack(struct transfer_worker *worker)
{
	int ret;
	/* parameter check in transfer work */
	int rfd = worker->record->rfd;
	struct work_event *work_event = NULL;
	struct transfer_handshake_ack *ack_event = NULL;
	u32 aim_flag = FILE_TRANS_HANDSHAKE | FILE_TRANS_ACK | FILE_TRANS_TYPE_TOPIC;

	/* 1. try to get transfer event in worker list */
	ret = wait_event_interruptible_timeout(worker->event_wait, !list_empty(&worker->list),
					       msecs_to_jiffies(WAIT_REMOTE_TIMEOUT));
	if (ret <= 0) {
		hwlog_err("%s rfd:%08x  timeout", __func__, rfd);
		return -ETIME;
	}
	mutex_lock(&worker->list_lock);
	work_event = list_first_entry(&worker->list, struct work_event, list);
	if (work_event)
		list_del(&work_event->list);
	mutex_unlock(&worker->list_lock);
	/* 2. check event is valid */
	if (!work_event || !work_event->data || work_event->data_len < sizeof(*ack_event)) {
		hwlog_err("%s rfd:%08x ack is invalid", __func__, rfd);
		ret = -EINVAL;
		goto out;
	}
	/* 3. check event is handshake ack, error flag need check */
	ack_event = (struct transfer_handshake_ack *)(work_event->data);
	if ((ack_event->transfer_flag & aim_flag) != aim_flag) {
		hwlog_err("%s rfd:%08x is not handshake ack", __func__, rfd);
		ret = -EINVAL;
		goto out;
	}
	/* 4. is a fetch handshake ack, if the length is 0, get from ack */
	if (worker->transfer_type == AP_FETCH || worker->transfer_type == REMOTE_FETCH) {
		if (worker->file && worker->file->len == 0)
			worker->file->len = ack_event->data_len;
		if (worker->file)
			hwlog_info("%s rfd:%08x file len:%u", __func__, rfd, worker->file->len);
	}
	ret = 0;
out:
	if (ack_event)
		kfree(ack_event->path);
	if (work_event) {
		kfree(work_event->data);
		kfree(work_event);
	}

	return ret;
}
