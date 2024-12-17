/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_mgr.h
 *
 * file manager head file for file transfer
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
#ifndef FILE_MGR_H
#define FILE_MGR_H
#include <linux/types.h>

enum transfer_ops {
	TRANSFER_READ = 0,
	TRANSFER_WRITE = 1,
	TRANSFER_BUFFER_READ = 2,
	TRANSFER_BUFFER_WRITE = 3,
};

enum file_state {
	INITIAL,
	PRE_PROCESSED,
	PROCESSING,
	END,
};

struct transfer_file {
	struct file *filp;
	u32 c_offset;
	u32 dst_offset;
	u32 offset;
	u32 len;
	enum transfer_ops opt;
	enum file_state state;
	char *filepath;
	int (*pre_process)(struct transfer_file *file);
	int (*data_process)(struct transfer_file *file, unsigned char *data, u32 len);
};

/*
 * init transfer file by file information
 * @param filepath the file path to read or write
 * @param offset the file offset to operation
 * @param len the file len to operation(0 means to the file end)
 * @param opt the operation for read or write
 * @return the initialized transfer file
 */
struct transfer_file *initial_transfer_file(const char *filepath, u32 offset,
					    u32 len, enum transfer_ops opt);

/*
 * release resources in transfer file
 * @param file the transfer file to release
 */
void release_transfer_file(struct transfer_file *file);

#endif
