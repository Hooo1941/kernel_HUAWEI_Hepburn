/*
 * cloud_network_detect.h
 *
 * cloud network detect compass function kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _CLOUD_NETWORK_DETECT_H_
#define _CLOUD_NETWORK_DETECT_H_

#include <linux/types.h>
#include <linux/spinlock.h>

#define DETECT_PATH_NUM_MAX 4
#define PATH_LIST_NUM 100

enum {
	DETECT_MODULE_ID_ERROR = -1,
	ALGORITHM_MODULE_ID = 0,
	OPTION_MODULE_ID = 1,
	COMPASS_CHECK_ID = 2,
	DETECT_MODULE_ID_MAX
};

enum {
	DETECT_WORK_SEND_PACKET = 0,
	DETECT_WORK_RCV_PACKET = 1,
	DETECT_WORK_START = 2,
	DETECT_WORK_STOP = 3
};

typedef void detect_callback(int path, u32 rtt, int average_rtt, int loss_rate);

struct detect_tlv {
	u32 type;
	u32 msg_len; /* sizeof(detect_tlv) + length of data */
	char data[0];
};

struct detect_work_start_info {
	u32 type;
	u32 msg_len;
	int module_id;
	int time_interval;
	int paths;
	detect_callback *detect_fn;
};

struct detect_work_stop_info {
	u32 type;
	u32 msg_len;
	int module_id;
};

struct detect_work_send_packet {
	u32 type;
	u32 msg_len;
	int paths;
	u32 timestamp_send;
	int module_id;
};

struct detect_work_rcv_packet {
	u32 type;
	u32 msg_len;
	int path;
	u32 timestamp_send;
	u32 timestamp_recv;
	int module_id;
};

struct detect_work_path_timeout {
	int path;
	int loss_rate;
};

struct detect_path_info {
	int path;
	int send_iterator;
	int *timestamp_list;
	int *rtt_list;
	atomic_t path_seq;
	spinlock_t lock;
};

struct detect_ops {
	int module_id;
	int time_interval;
	int paths;
	detect_callback *detect_fn;
	spinlock_t lock;
};

#pragma pack(1)
struct hon_option_detect {
	u8 type;
	u8 length;
	u32 timestamp;
};
#pragma pack()

void change_detect_path(int module_id, int paths);
void add_path_timestamp_send(int paths, u32 timestamp_send);
int detect_packet_recv(int path, struct hon_option_detect *detect);
int cloud_network_detect_init(void);
void detect_module_start(int module_id, int time_interval, int paths, detect_callback *detect_fn);
void detect_module_stop(int module_id);
void cloud_network_detect_exit(void);
void cloud_network_detect_reset(void);
#endif /* _CLOUD_NETWORK_DETECT_H_ */
