// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_topic.c
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

#include "transfer_topic.h"

#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "transfer_inner_commu.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define TOPIC_SID MULTI_MCU_COMMU_SVR
#define TOPIC_CID INNER_CMD_COMMU_CID
#pragma pack(1)
/* subcmd struct for REGISTER_TOPIC */
struct topic_register {
	s64 topic_id;
	u32 path_len;
	char *local_path;
};

/* subcmd struct for UNREGISTER_TOPIC */
struct topic_unregister {
	s64 topic_id;
	u64 topic_mask;
};

#pragma pack()

/* mutex for lock topic list */
static DEFINE_MUTEX(topic_mutex);
static LIST_HEAD(topic_list_head);

struct topic_node {
	struct list_head list;
	struct topic_register *topic;
};

static int register_transfer_topic(unsigned char *data, int data_len)
{
	struct topic_node *node = NULL;
	struct topic_node *tmp_node = NULL;
	struct topic_register *topic = NULL;
	struct topic_register *old_topic = NULL;

	if (!data || data_len < sizeof(*topic))
		return -EINVAL;

	topic = convert_to_path_type(data, data_len, sizeof(*topic),
				     offsetof(struct topic_register, path_len),
				     offsetof(struct topic_register, local_path));
	if (!topic || !topic->local_path)
		return -EINVAL;

	hwlog_info("%s topic id:%16x\n", __func__, topic->topic_id);
	mutex_lock(&topic_mutex);
	/* check if contains old record and override the record */
	list_for_each_entry_safe(node, tmp_node, &topic_list_head, list) {
		if (!node || !node->topic || node->topic->topic_id != topic->topic_id)
			continue;

		old_topic = node->topic;
		/* replace topic in list */
		node->topic = topic;
		/* release old record */
		kfree(old_topic->local_path);
		old_topic->local_path = NULL;
		kfree(old_topic);
		old_topic = NULL;
		mutex_unlock(&topic_mutex);
		return 0;
	}

	/* new topic */
	node = kmalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		kfree(topic->local_path);
		kfree(topic);
		mutex_unlock(&topic_mutex);
		return -EFAULT;
	}
	INIT_LIST_HEAD(&node->list);
	node->topic = topic;
	list_add_tail(&node->list, &topic_list_head);
	mutex_unlock(&topic_mutex);
	return 0;
}

static int unregister_transfer_topic(unsigned char *data, int data_len)
{
	struct topic_node *buf_node = NULL;
	struct topic_node *tmp_node = NULL;
	struct topic_unregister *topic = NULL;

	if (!data || data_len != sizeof(struct topic_unregister))
		return -EINVAL;

	topic = (struct topic_unregister *)data;
	hwlog_info("%s topic id:%16x\n", __func__, topic->topic_id);

	mutex_lock(&topic_mutex);
	list_for_each_entry_safe(buf_node, tmp_node, &topic_list_head, list) {
		/* match with topic mask */
		if (!buf_node || !buf_node->topic ||
		    (buf_node->topic->topic_id & topic->topic_mask) !=
		    (topic->topic_id & topic->topic_mask))
			continue;
		/* delete and free the topic */
		list_del(&buf_node->list);
		kfree(buf_node->topic->local_path);
		buf_node->topic->local_path = NULL;

		kfree(buf_node->topic);
		buf_node->topic = NULL;

		kfree(buf_node);
		buf_node = NULL;
	}
	mutex_unlock(&topic_mutex);

	return 0;
}

char *query_local_path(u64 topic_id, u32 *path_len)
{
	int ret;
	char *local_path = NULL;
	struct topic_node *buf_node = NULL;

	if (!path_len)
		return NULL;

	mutex_lock(&topic_mutex);
	list_for_each_entry(buf_node, &topic_list_head, list) {
		if (!buf_node || !buf_node->topic || buf_node->topic->topic_id != topic_id)
			continue;
		/* match topic id and get local path */
		local_path = kmalloc(buf_node->topic->path_len + 1, GFP_KERNEL);
		if (!local_path)
			goto err;

		ret = strcpy_s(local_path, buf_node->topic->path_len + 1,
			       buf_node->topic->local_path);
		if (ret != EOK)
			goto err;

		*path_len = buf_node->topic->path_len;
		mutex_unlock(&topic_mutex);
		return local_path;
	}
err:
	mutex_unlock(&topic_mutex);
	*path_len = 0;
	kfree(local_path);
	hwlog_warn("%s cannot query path by topic:%16x", __func__, topic_id);
	return NULL;
}

static int __init register_topic_command(void)
{
	int ret;

	hwlog_info("%s call in", __func__);
	ret = register_transfer_cmd(TOPIC_SID, TOPIC_CID, REGISTER_TOPIC, register_transfer_topic);
	hwlog_info("%s register (topic register) cmd ret:%d", __func__, ret);

	ret = register_transfer_cmd(TOPIC_SID, TOPIC_CID,
				    UNREGISTER_TOPIC, unregister_transfer_topic);
	hwlog_info("%s register (topic unregister) cmd ret:%d", __func__, ret);

	return 0;
}

fs_initcall(register_topic_command);
