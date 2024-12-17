/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_commu.h
 *
 * communication file for file transfer
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
#ifndef TRANSFER_COMMU_H
#define TRANSFER_COMMU_H
#include <linux/types.h>
#include "transfer_record.h"

#define FILE_TARANSFER_COMMU_SVR 0x90
#define FILE_TARANSFER_COMMU_CMD 0x0E

/* the flag for transfer package */
enum file_transfer_flag {
	FILE_TRANS_HANDSHAKE = 0x01,
	FILE_TRANS_TYPE_TOPIC = 0x02,
	FILE_TRANS_TYPE_PAIR = 0x04,
	FILE_TRANS_FETCH = 0x08,

	FILE_TRANS_DATA_PCK = 0x10, /* data pkg */
	FILE_TRANS_NEED_ACK = 0x20, /* need ack */
	FILE_TRANS_ACK = 0x40, /* ack pkg */
	FILE_TRANS_RESP = 0x80, /* the pkg for error response */
	FILE_TRANS_INTERRUPT = 0x100, /* interrupt for  */
	FILE_TRANS_ERROR = 0x200, /* pkg is error */

	FILE_TRANS_BUFFER = 0x400, /* transfer buffer */

	FILE_TRANS_RESERVED = 0x80000000, /* reserved flag */
};

#pragma pack(1)
struct commu_base {
	int rfd;
	u32 transfer_flag;
};

struct transfer_handshake {
	int rfd;
	u32 transfer_flag;
	u32 src_offset;
	u32 dst_offset;
	u32 data_len;
	s64 topic_id;
	u32 path_len;
	char *path;
};

struct transfer_handshake_ack {
	int rfd;
	u32 transfer_flag;
	s64 topic_id;
	u32 dst_offset; /* the receiver data offset */
	u32 data_len; /* the receiver data length */
	u32 path_len;
	char *path;
};

struct transfer_handshake_err {
	int rfd;
	u32 transfer_flag;
	u32 error_code;
};

struct transfer_data {
	int rfd;
	u32 transfer_flag;
	u32 offset;
	u32 data_len;
	u8 *data;
};

struct transfer_data_ack {
	int rfd;
	u32 transfer_flag;
	/* for the pkg info */
	u32 offset;
	u32 data_len;
};

struct transfer_err_resp {
	int rfd;
	u32 transfer_flag;
	u32 error_code;
};

#pragma pack()

enum transfer_msg_type {
	MSG_TOPIC_HANDSHAKE,
	MSG_TOPIC_HANDSHAKE_ACK,
	MSG_HANDSHAKE_ERR,
	MSG_TRANSFER_DATA,
	MSG_TRANSFER_DATA_ACK,
	MSG_ERROR_RESP,
	MSG_BUFFER_HANDSHAKE,
};

typedef int (*remote_msg_callback)(int rfd, enum transfer_msg_type type, void *data, u32 len);

/*
 * init transfer comminication with remote. When get message from remote will callback
 * @param callback the callback for remote message
 */
void init_transfer_commu(remote_msg_callback callback);

/*
 * send a transfer request to rempte
 * @param data the struct buffer
 * @param len the struct buffer length
 * @return >=0 for success, otherwise for fail
 */
int send_transfer_requeset(enum transfer_type type, void *data, u32 len);

#endif
