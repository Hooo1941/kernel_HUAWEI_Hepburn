/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_record.h
 *
 * the head file for file transfer records
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
#ifndef TRANSFER_RECORD_H
#define TRANSFER_RECORD_H

#include "transfer_inner_commu.h"

#define FETCH_FILE_TID 0x12
#define TRANSFER_FILE_TID 0x21

enum transfer_type {
	AP_TRANSFER = 0,
	AP_FETCH = 1,
	REMOTE_TRANSFER = 2,
	REMOTE_FETCH = 3,

	TRANSFER_BUFFER = 4,
	REMOTE_TRANSFER_BUFFER = 5,

	MAX_TRANSFER_TYPE,
};

#pragma pack(1)
/* subcmd TRANSFER_FILE_TOPIC_TYPE & FETCH_FILE_TOPIC_TYPE used file info */
struct file_info {
	u32 src_offset;
	u32 dst_offset;
	u32 data_len;
	u32 path_len;
	char *local_path;
};

/* subcmd struct for TRANSFER_FILE_TOPIC_TYPE & FETCH_FILE_TOPIC_TYPE */
struct file_transfer {
	s64 topic_id;
	u8 is_force;
	u8 need_verify;
	struct file_info file;
};

#pragma pack()

/* transfer info, worker hold */
struct transfer_record {
	int rfd;
	enum transfer_status err_code;
	struct file_transfer *trans_info;
	/* maintains record list */
	struct list_head list;
	enum transfer_type transfer_type;
};

/*
 * generate a new transfer record by rfd for remote transfer
 * @param rfd the remote fd received from remote
 * @return the new record pointer generated
 */
struct transfer_record *generate_remote_record(int rfd);

/*
 * generate a new transfer record
 * @param tid the rfd marks for transfer or fetch.
 * @return the new record pointer generated
 */
struct transfer_record *generate_available_record(s8 tid);

/*
 * release resources in record
 * @param record the transfer record to release
 */
void release_record(struct transfer_record *record);

#endif
