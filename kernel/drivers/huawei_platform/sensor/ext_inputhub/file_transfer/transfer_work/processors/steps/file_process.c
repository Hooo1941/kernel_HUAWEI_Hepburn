// SPDX-License-Identifier: GPL-2.0
/*
 * file_process.c
 *
 * source file for file transfer file process steps
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
#include "file_process.h"

#include <linux/slab.h>
#include <linux/types.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG file_trans
HWLOG_REGIST();

int preprocess_write_file(struct transfer_worker *worker)
{
	int ret;
	enum transfer_ops ops = TRANSFER_WRITE;
	struct transfer_file *write_file = NULL;

	if (!worker->record->trans_info->file.local_path) {
		hwlog_err("%s file path is null", __func__);
		return -EINVAL;
	}
	if (worker->transfer_type == REMOTE_TRANSFER_BUFFER)
		ops = TRANSFER_BUFFER_WRITE;
	/* 1. init the write file */
	write_file = initial_transfer_file(worker->record->trans_info->file.local_path,
					   worker->record->trans_info->file.dst_offset,
					   worker->record->trans_info->file.data_len,
					   ops);
	if (!write_file)
		return -EFAULT;
	/* 2. preproceess(open etc.) the write file */
	ret = write_file->pre_process(write_file);
	if (ret < 0) {
		release_transfer_file(write_file);
		return -EFAULT;
	}
	/* 3. set to worker context */
	worker->file = write_file;

	return 0;
}

int preprocess_read_file(struct transfer_worker *worker)
{
	int ret;
	enum transfer_ops ops = TRANSFER_READ;
	struct transfer_file *read_file = NULL;
	struct transfer_data *trans_data = NULL;

	if (!worker->record->trans_info->file.local_path)
		return -EINVAL;

	if (worker->transfer_type == TRANSFER_BUFFER)
		ops = TRANSFER_BUFFER_READ;

	/* 1. init the read file */
	read_file = initial_transfer_file(worker->record->trans_info->file.local_path,
					  worker->record->trans_info->file.src_offset,
					  worker->record->trans_info->file.data_len, ops);
	if (!read_file) {
		hwlog_err("%s rfd:%08x init transfer file error", __func__, worker->record->rfd);
		return -EFAULT;
	}
	read_file->dst_offset = worker->record->trans_info->file.dst_offset;
	/* 2. preproceess(open etc.) the read file */
	ret = read_file->pre_process(read_file);
	if (ret < 0) {
		hwlog_err("%s rfd:%08x pre_process error:%d", __func__, worker->record->rfd, ret);
		goto err_release;
	}
	/* 3. prepare buffer to read */
	trans_data = kzalloc(sizeof(*trans_data), GFP_KERNEL);
	if (!trans_data)
		goto err_release;

	trans_data->rfd = worker->record->rfd;
	trans_data->data = kmalloc(PACKAGE_READ_LEN, GFP_KERNEL);
	if (!trans_data->data)
		goto err_release;

	/* 4. set to worker context */
	worker->file = read_file;
	worker->trans_data = trans_data;
	return 0;

err_release:
	if (trans_data)
		kfree(trans_data->data);
	kfree(trans_data);
	release_transfer_file(read_file);
	return -EFAULT;
}
