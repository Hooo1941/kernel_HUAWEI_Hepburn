// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_commu.c
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
#include "transfer_commu.h"

#include <linux/slab.h>
#include <linux/stddef.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_api.h"
#include "transfer_inner_commu.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define MAX_DATA_LEN 3072
#define MAX_PATH_LEN 1024

/* for serialize data that do not contains buffer member */
static unsigned char *serialize_struct(void *data, u32 len, u32 *struct_len)
{
	if (!data || len == 0)
		return NULL;

	/* just return the input param */
	*struct_len = len;
	return data;
}

/* for deserialize data that do not contains buffer member(copy buffer from commu) */
static void *deserialize_buffer(unsigned char *data, u32 len, u32 *struct_len)
{
	int ret;
	void *buffer = NULL;

	if (!data || !struct_len || len == 0)
		return NULL;

	*struct_len = 0;
	buffer = kmalloc(len, GFP_KERNEL);
	if (!buffer)
		return NULL;

	ret = memcpy_s(buffer, len, data, len);
	if (ret != EOK) {
		kfree(buffer);
		return NULL;
	}
	*struct_len = len;
	return buffer;
}

/* free the data malloc from heap */
static void release_heap_data(void *data, u32 len)
{
	kfree(data);
}

static unsigned char *serialize_handshake(void *data, u32 data_len, u32 *out_len)
{
	return serialize_path_struct(data, data_len,
				     offsetof(struct transfer_handshake, path_len),
				     offsetof(struct transfer_handshake, path),
				     out_len);
}

static void *deserialize_handshake(unsigned char *data, u32 data_len, u32 *struct_len)
{
	*struct_len = sizeof(struct transfer_handshake);

	return convert_to_path_type(data, data_len, *struct_len,
				    offsetof(struct transfer_handshake, path_len),
				    offsetof(struct transfer_handshake, path));
}

static void release_handshake(void *data, u32 struct_len)
{
	struct transfer_handshake *handshake = NULL;

	if (!data || struct_len != sizeof(*handshake))
		return;

	handshake = (struct transfer_handshake *)data;

	kfree(handshake->path);
	handshake->path = NULL;
	kfree(data);
	data = NULL;
}

static unsigned char *serialize_handshake_ack(void *data, u32 data_len, u32 *out_len)
{
	return serialize_path_struct(data, data_len,
				     offsetof(struct transfer_handshake_ack, path_len),
				     offsetof(struct transfer_handshake_ack, path),
				     out_len);
}

static void *deserialize_handshake_ack(unsigned char *data, u32 data_len, u32 *struct_len)
{
	*struct_len = sizeof(struct transfer_handshake_ack);

	return convert_to_path_type(data, data_len, *struct_len,
				    offsetof(struct transfer_handshake_ack, path_len),
				    offsetof(struct transfer_handshake_ack, path));
}

static void release_handshake_ack(void *data, u32 struct_len)
{
	struct transfer_handshake_ack *handshake = NULL;

	if (!data || struct_len != sizeof(*handshake))
		return;

	handshake = (struct transfer_handshake_ack *)data;

	kfree(handshake->path);
	handshake->path = NULL;
	kfree(data);
	data = NULL;
}

static void *deserialize_transfer_data(unsigned char *data, u32 data_len, u32 *struct_len)
{
	int ret;
	char *file_data = NULL;
	struct transfer_data *commu_data = NULL;

	if (!data || data_len < sizeof(*commu_data) - sizeof(char *))
		return NULL;

	commu_data = kmalloc(sizeof(*commu_data), GFP_KERNEL);
	if (!commu_data)
		return NULL;
	/* copy struct except data buffer */
	ret = memcpy_s(commu_data, sizeof(*commu_data) - sizeof(char *), data,
		       sizeof(*commu_data) - sizeof(char *));
	if (ret != EOK)
		goto err;

	/* check data and len */
	if (data_len < sizeof(*commu_data) - sizeof(char *) + commu_data->data_len ||
	    commu_data->data_len > MAX_DATA_LEN) {
		hwlog_err("%s data len invalid, len:%u", __func__, data_len);
		goto err;
	}

	file_data = kmalloc(commu_data->data_len, GFP_KERNEL);
	if (!file_data)
		goto err;
	/* copy data buffer */
	ret = memcpy_s(file_data, commu_data->data_len,
		       data + offsetof(struct transfer_data, data), commu_data->data_len);
	if (ret != EOK)
		goto err;
	commu_data->data = file_data;

	*struct_len = sizeof(*commu_data);
	return commu_data;
err:
	kfree(file_data);
	kfree(commu_data);
	return NULL;
}

static unsigned char *serialize_transfer_data(void *data, u32 len, u32 *data_len)
{
	int ret;
	unsigned char *serialize = NULL;
	struct transfer_data *commu_data = NULL;

	if (!data || !data_len)
		return NULL;

	commu_data = (struct transfer_data *)data;
	*data_len = offsetof(struct transfer_data, data) + commu_data->data_len;
	if (*data_len > MAX_DATA_LEN) {
		hwlog_err("%s data len invalid, len:%u", __func__, *data_len);
		return NULL;
	}

	serialize = kmalloc(*data_len, GFP_KERNEL);
	if (!serialize) {
		*data_len = 0;
		return NULL;
	}
	/* copy except data buffer */
	ret = memcpy_s(serialize, *data_len, commu_data, offsetof(struct transfer_data, data));
	if (ret != EOK) {
		kfree(serialize);
		*data_len = 0;
		return NULL;
	}
	/* copy data buffer */
	ret = memcpy_s(serialize + offsetof(struct transfer_data, data),
		       commu_data->data_len, commu_data->data, commu_data->data_len);
	if (ret != EOK) {
		kfree(serialize);
		*data_len = 0;
		return NULL;
	}
	return serialize;
}

static void release_transfer_data(void *data, u32 struct_len)
{
	struct transfer_data *commu_data = NULL;

	if (!data || struct_len != sizeof(*commu_data))
		return;

	commu_data = (struct transfer_data *)data;

	kfree(commu_data->data);
	commu_data->data = NULL;
	kfree(commu_data);
	commu_data = NULL;
}

struct commu_data_serializer {
	int transfer_flag;
	enum transfer_msg_type msg_type;

	unsigned char *(*serialize)(void *data, u32 data_len, u32 *out_len);
	void (*release_send_data)(void *data, u32 len);

	void *(*deserialize)(unsigned char *data, u32 data_len, u32 *struct_len);
	void (*release_data)(void *data, u32 struct_len);
};

static remote_msg_callback transfer_scheduler_callback;

/* this map elements order is important */
struct commu_data_serializer serializer_map[] = {
	{
		FILE_TRANS_HANDSHAKE | FILE_TRANS_TYPE_TOPIC | FILE_TRANS_ACK,
		MSG_TOPIC_HANDSHAKE_ACK,
		serialize_handshake_ack,
		release_heap_data,
		deserialize_handshake_ack,
		release_handshake_ack,
	},
	{
		FILE_TRANS_HANDSHAKE | FILE_TRANS_TYPE_TOPIC | FILE_TRANS_BUFFER,
		MSG_BUFFER_HANDSHAKE,
		serialize_handshake,
		release_heap_data,
		deserialize_handshake,
		release_handshake,
	},
	{
		FILE_TRANS_HANDSHAKE | FILE_TRANS_TYPE_TOPIC,
		MSG_TOPIC_HANDSHAKE,
		serialize_handshake,
		release_heap_data,
		deserialize_handshake,
		release_handshake,
	},
	{
		FILE_TRANS_DATA_PCK | FILE_TRANS_ACK,
		MSG_TRANSFER_DATA_ACK,
		serialize_struct,
		NULL,
		deserialize_buffer,
		release_heap_data,
	},
	{
		FILE_TRANS_DATA_PCK,
		MSG_TRANSFER_DATA,
		serialize_transfer_data,
		release_heap_data,
		deserialize_transfer_data,
		release_transfer_data,
	},
	{
		FILE_TRANS_RESP,
		MSG_ERROR_RESP,
		serialize_struct,
		NULL,
		deserialize_buffer,
		release_heap_data,
	},
	{
		FILE_TRANS_HANDSHAKE | FILE_TRANS_ACK | FILE_TRANS_ERROR,
		MSG_HANDSHAKE_ERR,
		serialize_struct,
		NULL,
		deserialize_buffer,
		release_heap_data,
	},
};

static int commu_data_callback(unsigned char service_id, unsigned char command_id,
			       unsigned char *data, int data_len)
{
	int i;
	int ret = 0;
	void *des_data = NULL;
	u32 struct_len = 0;
	struct commu_base *base = NULL;

	if (service_id != FILE_TARANSFER_COMMU_SVR || command_id != FILE_TARANSFER_COMMU_CMD ||
	    !data || data_len < sizeof(struct commu_base))
		return -EINVAL;

	base = (struct commu_base *)data;
	hwlog_info("%s get transfer cmd, flag:%08x", __func__, base->transfer_flag);
	/* find serializer in map and notify scheduler */
	for (i = 0; i < ARRAY_SIZE(serializer_map); ++i) {
		if ((base->transfer_flag & serializer_map[i].transfer_flag) !=
			serializer_map[i].transfer_flag)
			continue;

		des_data = serializer_map[i].deserialize(data, data_len, &struct_len);
		/* callback to scheduler */
		if (transfer_scheduler_callback)
			ret = transfer_scheduler_callback(base->rfd, serializer_map[i].msg_type,
							  des_data, struct_len);
		/* if data successfully deal by scheduler, the data will queue and used later.
		 * for failure need to release
		 */
		if (ret < 0)
			serializer_map[i].release_data(des_data, struct_len);
		return 0;
	}

	hwlog_info("%s cannot get serializer", __func__);
	return 0;
}

static int send_transfer_data(unsigned char *data, int data_len)
{
	int ret;
	struct command cmd = {
		FILE_TARANSFER_COMMU_SVR,
		FILE_TARANSFER_COMMU_CMD,
		data,
		data_len,
	};

	if (!data || data_len < 0)
		return -EINVAL;

	ret = send_command(FILE_TRANS_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		hwlog_err("%s send command err:%d", __func__, ret);

	return ret;
}

static int extra_flag_map[MAX_TRANSFER_TYPE] = {
	0, FILE_TRANS_FETCH, 0, FILE_TRANS_FETCH, FILE_TRANS_BUFFER, FILE_TRANS_BUFFER
};

int send_transfer_requeset(enum transfer_type type, void *data, u32 data_len)
{
	int i;
	int ret;
	int s_len = 0;
	unsigned char *s_data = NULL;
	struct commu_base *base = NULL;

	if (type >= MAX_TRANSFER_TYPE || !data || data_len < sizeof(struct commu_base))
		return -EINVAL;

	base = (struct commu_base *)data;
	for (i = 0; i < ARRAY_SIZE(serializer_map); ++i) {
		if ((base->transfer_flag & serializer_map[i].transfer_flag) !=
			serializer_map[i].transfer_flag)
			continue;
		base->transfer_flag |= extra_flag_map[type];
		/* match serializer and convert to bytes */
		s_data = serializer_map[i].serialize(data, data_len, &s_len);
		ret = send_transfer_data(s_data, s_len);
		/* after sent, release data */
		if (serializer_map[i].release_send_data)
			serializer_map[i].release_send_data(s_data, s_len);

		return ret;
	}
	hwlog_err("%s cannot find serializer for flag:%x", __func__, base->transfer_flag);
	return -EINVAL;
}

static void register_commu_callback(void)
{
	int ret;
	struct sid_cid cmd_id = {
		FILE_TARANSFER_COMMU_SVR,
		FILE_TARANSFER_COMMU_CMD,
	};
	struct subscribe_cmds cmds = {
		&cmd_id,
		/* only one command */
		1,
	};

	ret = register_data_callback(FILE_TRANS_CHANNEL, &cmds, commu_data_callback);
	hwlog_info("%s register callback ret:%d", __func__, ret);
}

void init_transfer_commu(remote_msg_callback callback)
{
	transfer_scheduler_callback = callback;
	register_commu_callback();
}
