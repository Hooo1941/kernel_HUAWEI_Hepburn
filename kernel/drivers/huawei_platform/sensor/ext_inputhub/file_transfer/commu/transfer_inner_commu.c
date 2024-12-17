// SPDX-License-Identifier: GPL-2.0
/*
 * transfer_inner_commu.c
 *
 * inner command manager source file for file transfer
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
#include "transfer_inner_commu.h"

#include <linux/bsearch.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_inner_cmd.h"

#define HWLOG_TAG file_trans
HWLOG_REGIST();

#define MAX_PATH_LEN 1024

struct transfer_cmd {
	unsigned char sid;
	unsigned char cid;
	unsigned char sub_id;
	data_processor processor;
	struct list_head list;
};

struct commu_id {
	unsigned char sid;
	unsigned char cid;
};

/* mutex for transfer inner command list */
static DEFINE_MUTEX(cmd_mutex);
static LIST_HEAD(cmd_list_head);
static int cmd_list_cnt;

/* only support the path is the last member */
void *convert_to_path_type(unsigned char *data, int data_len,
			   u32 struct_len, size_t path_len_index, size_t path_index)
{
	int ret;
	void *data_struct = NULL;
	u32 *path_len = NULL;
	char **local_path = NULL;

	if (data_len < (struct_len - sizeof(char *)) ||
	    data_len <= (path_len_index + sizeof(path_len_index))) {
		hwlog_err("%s data len:%d is invalid", __func__, data_len);
		return NULL;
	}

	data_struct = kmalloc(struct_len, GFP_KERNEL);
	if (!data_struct)
		return NULL;
	/* get member except local_path */
	ret = memcpy_s(data_struct, struct_len - sizeof(char *), data, struct_len - sizeof(char *));
	if (ret != EOK) {
		kfree(data_struct);
		return NULL;
	}

	path_len = (u32 *)&data[path_len_index];
	hwlog_info("%s get path len:%u, left:%d", __func__, *path_len, data_len - path_index - 1);
	if (*path_len > (data_len - path_index - 1)) {
		kfree(data_struct);
		return NULL;
	}
	if (*path_len > MAX_PATH_LEN) {
		kfree(data_struct);
		return NULL;
	}

	/* get local path in struct */
	local_path = &data_struct[path_index];
	*local_path = kmalloc(*path_len + 1, GFP_KERNEL);
	if (!*local_path) {
		kfree(data_struct);
		return NULL;
	}
	ret = memcpy_s(*local_path, *path_len + 1, data + path_index, *path_len + 1);
	if (ret != EOK) {
		kfree(*local_path);
		kfree(data_struct);
		return NULL;
	}

	return data_struct;
}

unsigned char *serialize_path_struct(void *struct_data, u32 struct_len,
				     u32 len_index, u32 path_index, u32 *out_len)
{
	int ret;
	char **path = NULL;
	u32 *path_len = NULL;
	unsigned char *result = NULL;

	if (!struct_data || struct_len < len_index + sizeof(u32) ||
	    struct_len < path_index + sizeof(char *))
		return NULL;

	path = &struct_data[path_index];
	path_len = &struct_data[len_index];
	if (*path_len > MAX_PATH_LEN) {
		hwlog_err("%s path length invalid: %u", __func__, *path_len);
		goto err;
	}
	*out_len = path_index + *path_len + 1;
	result = kmalloc(*out_len, GFP_KERNEL);
	if (!result)
		goto err;
	/* copy except path */
	ret = memcpy_s(result, *out_len, struct_data, path_index);
	if (ret != EOK)
		goto err;

	/* copy path */
	ret = memcpy_s(result + path_index, *path_len + 1, *path, *path_len + 1);
	if (ret != EOK)
		goto err;

	return result;
err:
	kfree(result);
	*out_len = 0;
	return NULL;
}

int send_transfer_inner_resp(struct transfer_resp *resp)
{
	int ret;

	if (!resp)
		return -EINVAL;

	ret = send_inner_command(MULTI_MCU_COMMU_SVR, INNER_CMD_COMMU_RESP_CID,
				 (unsigned char *)resp, sizeof(*resp));

	return ret;
}

static int transfer_inner_callback(unsigned char sid, unsigned char cid,
				   unsigned char *data, int data_len)
{
	unsigned char sub_id;
	struct transfer_cmd *buf_node = NULL;
	int ret;

	if (!data || data_len <= 0)
		return -EINVAL;

	hwlog_info("%s inner cmd callback get in", __func__);
	sub_id = data[0];
	mutex_lock(&cmd_mutex);
	list_for_each_entry(buf_node, &cmd_list_head, list) {
		if (!buf_node)
			continue;
		/* match the service id, command id and subtype */
		if (buf_node->sid == sid && buf_node->cid == cid && sub_id == buf_node->sub_id) {
			/* callback the data without subtype */
			ret = buf_node->processor(&data[sizeof(sub_id)], data_len - sizeof(sub_id));
			mutex_unlock(&cmd_mutex);
			return ret;
		}
	}
	mutex_unlock(&cmd_mutex);

	return -EINVAL;
}

static int cmd_search(const void *key, const void *elt)
{
	struct transfer_cmd *cmd = (struct transfer_cmd *)key;
	struct commu_id *commu_id = (struct commu_id *)elt;

	if (!cmd || !commu_id)
		return -EFAULT;

	return ((cmd->sid != commu_id->sid) || (cmd->cid != commu_id->cid));
}

/*
 * Registered command may have the same service id and command id(sub id different).
 * Here to get unique service id and command id then register to communication.
 */
static int unique_transfer_cmds(unsigned char **sid, unsigned char **cid)
{
	int i;
	int count = 0;
	struct transfer_cmd *buf_node = NULL;
	struct commu_id *commu_ids = NULL;
	void *ret = NULL;

	if (!sid || !cid)
		return 0;

	commu_ids = kcalloc(cmd_list_cnt, sizeof(struct transfer_cmd), GFP_KERNEL);
	if (!commu_ids)
		return 0;

	mutex_lock(&cmd_mutex);
	list_for_each_entry(buf_node, &cmd_list_head, list) {
		/* binary search for if commu_ids already contians the command */
		ret = bsearch(buf_node, commu_ids, count, sizeof(struct transfer_cmd), cmd_search);
		if (ret)
			continue;
		commu_ids[count].sid = buf_node->sid;
		commu_ids[count].cid = buf_node->cid;
		count++;
	}
	mutex_unlock(&cmd_mutex);

	*sid = kzalloc(count, GFP_KERNEL);
	if (!*sid)
		goto err;

	*cid = kzalloc(count, GFP_KERNEL);
	if (!*cid)
		goto err;

	hwlog_info("%s get unique count:%d", __func__, count);
	for (i = 0; i < count; ++i) {
		(*sid)[i] = commu_ids[i].sid;
		(*cid)[i] = commu_ids[i].cid;
	}
	kfree(commu_ids);
	return count;
err:
	kfree(*sid);
	kfree(*cid);
	*sid = NULL;
	*cid = NULL;
	kfree(commu_ids);

	return 0;
}

int register_transfer_cmd(unsigned char sid, unsigned char cid, unsigned char sub_id,
			  data_processor processor)
{
	struct transfer_cmd *cmd = NULL;
	unsigned char *sids = NULL;
	unsigned char *cids = NULL;
	int count;
	int ret;

	if (!processor)
		return -EINVAL;
	cmd = kzalloc(sizeof(*cmd), GFP_KERNEL);
	if (!cmd)
		return -EFAULT;

	cmd->sid = sid;
	cmd->cid = cid;
	cmd->sub_id = sub_id;
	cmd->processor = processor;
	INIT_LIST_HEAD(&cmd->list);

	mutex_lock(&cmd_mutex);
	cmd_list_cnt++;
	list_add_tail(&cmd->list, &cmd_list_head);
	mutex_unlock(&cmd_mutex);
	count = unique_transfer_cmds(&sids, &cids);
	ret = register_inner_cmd(EXT_SENSORHUB_CHANNEL, sids, cids, count, transfer_inner_callback);

	kfree(sids);
	kfree(cids);
	hwlog_info("%s register file transfer inner cmd ret:%d", __func__, ret);

	return 0;
}
