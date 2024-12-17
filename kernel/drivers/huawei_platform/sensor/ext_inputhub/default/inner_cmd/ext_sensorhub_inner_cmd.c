// SPDX-License-Identifier: GPL-2.0
/*
 * ext_sensorhub_inner_cmd.c
 *
 * source file for communication with inner command
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
#include "ext_sensorhub_inner_cmd.h"
#include <linux/slab.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#include "ext_sensorhub_route.h"

#define HWLOG_TAG inner_cmd
HWLOG_REGIST();

#define INNEXR_CMD_MAX_COUNT 255
#define INNEXR_CMD_MAX_DATALEN 1000

struct inner_cmds {
	unsigned char *service_ids;
	unsigned char *command_ids;
	unsigned short cmd_cnt;
};

struct inner_callback_node {
	struct inner_cmds cmds;
	inner_cmd_callback callback;
};

/* mutex for callback record */
DEFINE_MUTEX(table_mutex);
struct inner_callback_node callback_table[EXT_CHANNEL_MAX] = {0};

int register_inner_cmd(enum ext_channel_id channel_id,
		       unsigned char *service_ids, unsigned char *command_ids,
		       unsigned short cmd_cnt, inner_cmd_callback callback)
{
	struct inner_callback_node *node = NULL;

	if (channel_id >= EXT_CHANNEL_MAX || cmd_cnt > INNEXR_CMD_MAX_COUNT)
		return -EINVAL;

	node = &callback_table[channel_id];
	mutex_lock(&table_mutex);
	node->callback = callback;
	/* free last cmds */
	if (node->cmds.cmd_cnt > 0) {
		kfree(node->cmds.service_ids);
		kfree(node->cmds.command_ids);
	}
	node->cmds.service_ids = NULL;
	node->cmds.command_ids = NULL;
	/* set new cmds */
	node->cmds.cmd_cnt = cmd_cnt;
	node->cmds.service_ids = kmalloc(cmd_cnt, GFP_KERNEL);
	if (!node->cmds.service_ids)
		goto err;
	if (memcpy_s(node->cmds.service_ids, cmd_cnt, service_ids, cmd_cnt) != EOK)
		goto err;

	node->cmds.command_ids = kmalloc(cmd_cnt, GFP_KERNEL);
	if (!node->cmds.command_ids)
		goto err;
	if (memcpy_s(node->cmds.command_ids, cmd_cnt, command_ids, cmd_cnt) != EOK)
		goto err;

	mutex_unlock(&table_mutex);
	return 0;
err:
	node->callback = NULL;
	node->cmds.cmd_cnt = 0;
	kfree(node->cmds.service_ids);
	node->cmds.service_ids = NULL;
	kfree(node->cmds.command_ids);
	node->cmds.command_ids = NULL;
	mutex_unlock(&table_mutex);
	return -EFAULT;
}

int unregister_inner_cmd(enum ext_channel_id channel_id)
{
	struct inner_callback_node *node = NULL;

	if (channel_id >= EXT_CHANNEL_MAX)
		return -EINVAL;

	node = &callback_table[channel_id];
	mutex_lock(&table_mutex);
	if (!node->callback) {
		mutex_unlock(&table_mutex);
		return 0;
	}

	node->callback = NULL;
	node->cmds.cmd_cnt = 0;
	kfree(node->cmds.service_ids);
	node->cmds.service_ids = NULL;
	kfree(node->cmds.command_ids);
	node->cmds.command_ids = NULL;
	mutex_unlock(&table_mutex);
	return 0;
}

int send_inner_command(unsigned char service_id, unsigned char command_id,
		       unsigned char *data, int data_len)
{
	unsigned char *buffer = NULL;
	struct ext_sensorhub_buf_list *resp_list = NULL;

	if (!data || data_len < 0 || data_len > INNEXR_CMD_MAX_DATALEN)
		return -EINVAL;

	buffer = kmalloc(data_len, GFP_KERNEL);
	if (!buffer)
		return -EFAULT;
	if (memcpy_s(buffer, data_len, data, data_len) != EOK) {
		kfree(buffer);
		return -EFAULT;
	}

	/* resp_list free in ext_sensorhub_route_append after used */
	resp_list = kmalloc(sizeof(*resp_list), GFP_KERNEL);
	if (!resp_list) {
		kfree(buffer);
		return -EFAULT;
	}

	resp_list->service_id = service_id;
	resp_list->command_id = command_id;
	resp_list->read_cnt = data_len;
	resp_list->buffer = buffer;
	/* buffer also kfree after used */
	ext_sensorhub_route_append(resp_list);

	return data_len;
}

bool process_inner_cmd_hook(unsigned char service_id, unsigned char command_id,
			    unsigned char *data, int data_len, int *ret)
{
	int i;
	int cmd_i;
	struct inner_callback_node *node = NULL;

	if (!ret)
		return false;

	mutex_lock(&table_mutex);
	for (i = 0; i < EXT_CHANNEL_MAX; i++) {
		node = &callback_table[i];
		if (!node->callback || node->cmds.cmd_cnt == 0)
			continue;
		for (cmd_i = 0; cmd_i < node->cmds.cmd_cnt; ++cmd_i) {
			if (node->cmds.service_ids[cmd_i] != service_id ||
			    node->cmds.command_ids[cmd_i] != command_id)
				continue;
			/* callback data */
			*ret = node->callback(service_id, command_id, data, data_len);
			hwlog_info("%s inner cmd hook process, ret:%d", __func__, *ret);
			mutex_unlock(&table_mutex);
			return true;
		}
	}
	mutex_unlock(&table_mutex);
	return false;
}
