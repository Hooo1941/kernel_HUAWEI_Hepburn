/* SPDX-License-Identifier: GPL-2.0 */
/*
 * transfer_inner_commu.h
 *
 * inner command manager head file for file transfer
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
#ifndef INNER_CMD_MGR_H
#define INNER_CMD_MGR_H

#include <linux/types.h>

#define MULTI_MCU_COMMU_SVR 0x90
#define INNER_CMD_COMMU_CID 0x0D
#define INNER_CMD_COMMU_RESP_CID 0x0D

/* sub command id for file transfer inner cmd INNER_CMD_COMMU_CID */
enum {
	TRANSFER_FILE_TOPIC_TYPE = 0x01,
	FETCH_FILE_TOPIC_TYPE = 0x02,
	REGISTER_TOPIC = 0x03,
	UNREGISTER_TOPIC = 0x04,

	TRANSFER_BUFFER_TYPE = 0x09,
	REGISTER_BUFFER_TOPIC = 0x0A,
};

/* sub command id for file transfer response INNER_CMD_COMMU_RESP_CID */
enum {
	RESP_TOPIC_TYPE = 0x01,
	RESP_FILE_PAIR_TYPE = 0x02,
};

#pragma pack(1)
/* transfer status */
enum transfer_status {
	TRANS_STATUS_START = 0,
	TRANS_STATUS_FINISH = 1,
	TRANS_STATUS_INTERRUPT = 2,
	TRANS_STATUS_FILE_NOT_FOUND = 3,
	TRANS_STATUS_REMOTE_TOPIC_NOT_FOUND = 4,
	TRANS_STATUS_REMOTE_FILE_NOT_FOUND = 5,
	TRANS_STATUS_DATA_TIMEOUT = 7,
	TRANS_STATUS_ACK_TIMEOUT = 8,
	TRANS_STATUS_PACKAGE_UNMATCH = 9,
	TRANS_STATUS_SESSION_INVALID = 10,
	TRANS_STATUS_COMMU_ERR = 11,
	TRANS_STATUS_UNKNOWN_ERR = 12,
};

/* response event struct */
struct transfer_event {
	enum transfer_status status;
	u32 offset;
	u32 data_len;
};

/* struct for topic type response */
struct transfer_resp {
	u8 sub_type;
	u32 rfd;
	s64 topic_id;
	u8 transfer_type;
	struct transfer_event event;
};

#pragma pack()

typedef int (*data_processor)(unsigned char *data, int len);

/*
 * send a transfer inner response event
 * @param resp the response to send
 * @return >=0 for success, otherwise for fail
 */
int send_transfer_inner_resp(struct transfer_resp *resp);

/*
 * register a processor for the certain commu message
 * @param sid the commu service id
 * @param cid the commu command id
 * @param sub_id the commu payload first byte, mark for sub id
 * @param processor the input processor to init
 * @return >=0 for success, otherwise for fail
 */
int register_transfer_cmd(unsigned char sid, unsigned char cid, unsigned char sub_id,
			  data_processor processor);

/*
 * helper function to construct a path type struct
 * @param data the input data buffer contains path
 * @param len the input data buffer length
 * @param struct_len the struct length to construct
 * @param path_len_index the index of path len in struct
 * @param path_index the index of path in struct
 * @return the pointer of the struct, NULL if failed
 */
void *convert_to_path_type(unsigned char *data, int len,
			   u32 struct_len, size_t path_len_index, size_t path_index);

/*
 * helper function to serialize a path type struct to buffer
 * @param struct_data the struct contains path
 * @param struct_len the input struct length
 * @param len_index the index of path len in struct
 * @param path_index the index of path in struct
 * @param out_len the output buffer length
 * @return the pointer of the struct, NULL if failed
 */
unsigned char *serialize_path_struct(void *struct_data, u32 struct_len,
				     u32 len_index, u32 path_index, u32 *out_len);
#endif
