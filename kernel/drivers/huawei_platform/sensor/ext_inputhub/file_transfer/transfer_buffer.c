// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_buffer.c
 *
 * source file for buffer transfer
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
#include <linux/types.h>
#include <linux/slab.h>
#include <huawei_platform/log/hw_log.h>
#include "securec.h"
#include "transfer_record.h"
#include "transfer_inner_commu.h"
#include "transfer_commu.h"
#include "transfer_scheduler.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define BUFFER_SID MULTI_MCU_COMMU_SVR
#define BUFFER_CID INNER_CMD_COMMU_CID

#pragma pack(1)

struct transfer_buffer {
	u8 type;
	s64 topic_id;
	u32 data_len;
};

#pragma pack()

static int transfer_buffer(unsigned char *data, int data_len)
{
	int ret;
	struct file_transfer *file_transfer = NULL;
	struct transfer_buffer buffer = {0};

	if (data_len != sizeof(buffer)) {
		hwlog_err("%s invalid data_len is :%d\n", __func__, data_len);
		return -EINVAL;
	}
	ret = memcpy_s(&buffer, sizeof(buffer), data, data_len);
	if (ret != EOK)
		return -EFAULT;

	file_transfer = kzalloc(sizeof(*file_transfer), GFP_KERNEL);
	if (!file_transfer)
		return -EFAULT;

	file_transfer->topic_id = buffer.topic_id;
	file_transfer->file.data_len = buffer.data_len;

	/* set path to empty string */
	file_transfer->file.local_path = kzalloc(1, GFP_KERNEL);
	if (!file_transfer->file.local_path) {
		kfree(file_transfer);
		return -EFAULT;
	}
	file_transfer->file.local_path[0] = '\0';

	return queue_file_work(TRANSFER_FILE_TID, TRANSFER_BUFFER, file_transfer);
}

static int register_buffer_topic(unsigned char *data, int data_len)
{
	/* do nothing now */
	return 0;
}

int queue_buffer_handshake(int rfd, void *data, u32 data_len)
{
	int ret;
	struct file_transfer *file_req = NULL;
	struct transfer_handshake *handshake = NULL;

	hwlog_info("%s get callback handshake msg\n", __func__);
	if (!data || data_len < sizeof(*handshake))
		return -EINVAL;

	handshake = (struct transfer_handshake *)data;
	file_req = kzalloc(sizeof(*file_req), GFP_KERNEL);
	if (!file_req)
		return -EFAULT;

	file_req->topic_id = handshake->topic_id;
	file_req->file.data_len = handshake->data_len;
	file_req->file.src_offset = handshake->src_offset;
	file_req->file.dst_offset = handshake->dst_offset;
	/* deal path */
	file_req->file.path_len = handshake->path_len;
	file_req->file.local_path = handshake->path;

	ret = queue_remote_transfer(handshake->rfd, REMOTE_TRANSFER_BUFFER, file_req);
	if (ret != 0) {
		kfree(file_req->file.local_path);
		kfree(file_req);
		return -EFAULT;
	}
	/* success deal the data, free resource. path will release in record */
	kfree(handshake);
	return 0;
}

static int __init register_buffer_command(void)
{
	int ret;

	hwlog_info("%s call in", __func__);
	ret = register_transfer_cmd(BUFFER_SID, BUFFER_CID, TRANSFER_BUFFER_TYPE,
				    transfer_buffer);
	hwlog_info("%s register (buffer transfer) cmd ret:%d", __func__, ret);

	ret = register_transfer_cmd(BUFFER_SID, BUFFER_CID, REGISTER_BUFFER_TOPIC,
				    register_buffer_topic);
	hwlog_info("%s register (buffer topic) cmd ret:%d", __func__, ret);
	return 0;
}

fs_initcall(register_buffer_command);
