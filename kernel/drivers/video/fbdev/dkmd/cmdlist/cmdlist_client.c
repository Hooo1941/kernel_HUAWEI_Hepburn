/**
 * @file disp_cmdlist.c
 * @brief The ioctl the interface file for cmdlist node.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/syscalls.h>
#include "chash.h"
#include "cmdlist_client.h"
#include "cmdlist_node.h"
#include "dkmd_acquire_fence.h"
#include "dkmd_log.h"

static atomic_t g_cmdlist_next_id;

void cmdlist_client_init_cmdlist_id(void)
{
	atomic_set(&g_cmdlist_next_id, 0);
}

static bool cmdlist_client_compare(const struct cnode *a, const struct cnode *b)
{
	struct cmdlist_client *na = cnode_entry(a, struct cmdlist_client, hash_node);
	struct cmdlist_client *nb = cnode_entry(b, struct cmdlist_client, hash_node);

	return na->key == nb->key;
}

static void cmdlist_client_release_node(struct cnode *node)
{
	struct cmdlist_client *client = NULL;

	if (unlikely(!node))
		return;

	client = cnode_entry(node, struct cmdlist_client, hash_node);
	if (unlikely(!client))
		return;

	dpu_pr_debug("--> release client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("release node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		client->node, client->node->scene_id, client->node->cmd_mode, client->node->node_size,
		client->node->cnode.prev, client->node->cnode.next,
		client->node->list_cmd_header, client->node->fence);

	cmdlist_release_node(client->node);
	chash_remove(&client->hash_node);
	kfree(client);
	client = NULL;
}

/**
 * @brief init cmdlist device and cmdlist table
 *
 * @param cmd_table cmdlist client table
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_client_table_setup(struct cmdlist_table *cmd_table)
{
	int ret = 0;

	if (unlikely(!cmd_table)) {
		dpu_pr_err("input error, cmd_table is null pointer!");
		return -1;
	}

	if (chash_is_valid(&cmd_table->ht)) {
		dpu_pr_info("bkts is already initialized!");
		return ret;
	}

	ret = chash_init(&cmd_table->ht, cmd_table->scene_num, cmdlist_client_compare);
	if (unlikely(ret != 0))
		dpu_pr_err("init hash table failed!");

	return ret;
}

/**
 * @brief release cmdlist table and list
 *
 * @param cmd_table cmdlist client table
 */
void cmdlist_client_table_release(struct cmdlist_table *cmd_table)
{
	if (unlikely(!cmd_table)) {
		dpu_pr_err("input error, cmd_table is null pointer!\n");
		return;
	}

	chash_deinit(&cmd_table->ht, cmdlist_client_release_node);
}

/**
 * @brief clear cmdlist table and list
 *
 * @param cmd_table cmdlist client table
 */
void cmdlist_client_table_clear(struct cmdlist_table *cmd_table)
{
	if (unlikely(!cmd_table)) {
		dpu_pr_err("input error, cmd_table is null pointer!\n");
		return;
	}
	dpu_pr_info("+\n");

	chash_clear(&cmd_table->ht, cmdlist_client_release_node);
}

// ----------------------------------------------------------------------------
static uint32_t get_cmdlist_mode(uint32_t type)
{
	if ((type & REGISTER_CONFIG_TYPE) != 0)
		return CONFIG_MODE;

	if ((type & (DATA_TRANSPORT_TYPE | DM_TRANSPORT_TYPE)) != 0)
		return TRANSPORT_MODE;

	return NOP_MODE;
}

/**
 * @brief create cmdlist node by type, node_size and dst_addr
 *
 * @param cmd_table cmdlist client table
 * @param user_client use this client type, node_size and dst_addr
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_create_client(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client = NULL;
	struct cmdlist_node *node = NULL;

	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("input error, cmd_table or user_client is null pointer!");
		return -1;
	}

	if (user_client->scene_id > cmd_table->scene_num) {
		dpu_pr_warn("invalid scene_id:%u!", user_client->scene_id);
		return -1;
	}

	if (!chash_is_valid(&cmd_table->ht)) {
		dpu_pr_warn("invalid bkts scene_id:%u!", user_client->scene_id);
		return -1;
	}

	client = (struct cmdlist_client *)kzalloc(sizeof(struct cmdlist_client), GFP_KERNEL);
	if (unlikely(!client)) {
		dpu_pr_err("kzalloc return err!");
		return -ENOMEM;
	}
	client->key = (uint32_t)atomic_inc_return(&g_cmdlist_next_id);
	client->scene_id = user_client->scene_id;
	client->node_type = user_client->type;

	node = cmdlist_create_node(user_client->scene_id, get_cmdlist_mode(user_client->type),
		user_client->node_size, user_client->dst_addr);
	if (unlikely(!node)) {
		dpu_pr_err("cmdlist create node failed!");
		kfree(client);
		client = NULL;
		return -1;
	}
	node->client = client;
	client->node = node;
	chash_add(&cmd_table->ht, client->scene_id, &client->hash_node);

	user_client->id = client->key;
	user_client->node_size = (uint32_t)node->node_size;
	user_client->phyaddr = node->cmd_header_addr;
	user_client->viraddr = (uint64_t)(uintptr_t)node->list_cmd_header;

	dpu_pr_debug("--> create client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("create node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		node, node->scene_id, node->cmd_mode, node->node_size, node->cnode.prev, node->cnode.next,
		node->list_cmd_header, node->fence);

	return 0;
}

struct cmdlist_client *cmdlist_get_current_client(struct cmdlist_table *cmd_table,
	uint32_t scene_id, uint32_t cmdlist_id)
{
	struct cnode *node = NULL;
	struct cmdlist_client compare_client = {
		.key = cmdlist_id,
		.scene_id = scene_id,
	};

	if (unlikely(!cmd_table)) {
		dpu_pr_err("input error, cmd_table is null pointer!\n");
		return NULL;
	}

	if (scene_id > cmd_table->scene_num) {
		dpu_pr_warn("invalid scene_id:%u!", scene_id);
		return NULL;
	}

	if (!chash_is_valid(&cmd_table->ht)) {
		dpu_pr_warn("invalid bkts scene_id:%u!", scene_id);
		return NULL;
	}

	node = chash_find(&cmd_table->ht, scene_id, &compare_client.hash_node);
	if (unlikely(!node)) {
		dpu_pr_warn("can't find client_id[%u] and scene_id[%u]!", cmdlist_id, scene_id);
		return NULL;
	}

	return cnode_entry(node, struct cmdlist_client, hash_node);
}

/**
 * @brief release cmdlist client or list
 *
 * @param cmd_table cmdlist client table
 * @param user_client use this client scene_id and cmdlist_id
 */
int32_t cmdlist_release_client_or_list(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *header = NULL;
	struct cmdlist_client *client = NULL;
	struct cmdlist_node *node = NULL, *_node_ = NULL;

	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("cmd_table or user_client is nullptr");
		return -1;
	}

	header = cmdlist_get_current_client(cmd_table, user_client->scene_id, user_client->id);
	if (unlikely(!header) || unlikely(!header->node)) {
		dpu_pr_warn("invalid scene_id:%u, cmdlist_id:%u!", user_client->scene_id, user_client->id);
		return -1;
	}

	if (user_client->type != SCENE_NOP_TYPE) {
		dpu_pr_warn("--> release client<%u, %pK, %u>\n", header->key, header->node, header->node_type);
		cmdlist_client_release_node(&header->hash_node);
		return 0;
	}

	clist_for_each_entry_safe(node, _node_, &header->node->cnode, typeof(*node), cnode) {
		if (node && node->client) {
			client = (struct cmdlist_client *)node->client;
			dpu_pr_debug("--> release client<%u, %pK, %u>", client->key, client->node, client->node_type);
			cmdlist_release_node(node);
			chash_remove(&client->hash_node);
			kfree(client);
			client = NULL;
		}
	}

	dpu_pr_debug("--> release scene client<%u, %pK, %u>", header->key, header->node, header->node_type);
	cmdlist_client_release_node(&header->hash_node);

	return 0;
}

/**
 * @brief append cmdlist node which recorded by user_client with append_next_id to this scene cmdlist node
 *
 * @param cmd_table cmdlist client table
 * @param user_client use this client id and append_next_id
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_append_next_client(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client = NULL;
	struct cnode *flush_list_header = NULL;

	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("input error, cmd_table or user_client is null pointer!");
		return -1;
	}

	client = cmdlist_get_current_client(cmd_table, user_client->scene_id, user_client->id);
	if (unlikely(!client || !client->node)) {
		dpu_pr_err("invalid current cmdlist client!");
		return -1;
	}

	dpu_pr_debug("--> scene client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("head node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		client->node, client->node->scene_id, client->node->cmd_mode, client->node->node_size,
		client->node->cnode.prev, client->node->cnode.next,
		client->node->list_cmd_header, client->node->fence);
	if (unlikely(client->node_type != SCENE_NOP_TYPE)) {
		dpu_pr_err("not scene header type=%d!", client->node_type);
		return -1;
	}
	flush_list_header = &client->node->cnode;

	client = cmdlist_get_current_client(cmd_table, user_client->scene_id, user_client->append_next_id);
	if (unlikely(!client)) {
		dpu_pr_err("Invalid next cmdlist client!");
		return -1;
	}
	if (unlikely(client->node_type == SCENE_NOP_TYPE)) {
	    dpu_pr_err("forbid use same scene header type to append!");
	    return -1;
	}
	clist_add_tail(flush_list_header, &(client->node->cnode));
	dpu_pr_debug("--> append client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("append node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		client->node, client->node->scene_id, client->node->cmd_mode, client->node->node_size,
		client->node->cnode.prev, client->node->cnode.next,
		client->node->list_cmd_header, client->node->fence);

	return 0;
}

/**
 * @brief signal this current scene cmdlist node
 *
 * @param cmd_table cmdlist client table
 * @param user_client use this client id
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_signal_client(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client = NULL;
	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("input error, cmd_table or user_client is null pointer!");
		return -1;
	}

	client = cmdlist_get_current_client(cmd_table, user_client->scene_id, user_client->id);
	if (unlikely(!client || !client->node)) {
		dpu_pr_err("invalid current cmdlist client");
		return -1;
	}
	dpu_pr_debug("--> signal client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("signal node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		client->node, client->node->scene_id, client->node->cmd_mode, client->node->node_size,
		client->node->cnode.prev, client->node->cnode.next,
		client->node->list_cmd_header, client->node->fence);
	/* 1. signal the async fence */
	if (client->node->fence != NULL) {
		dkmd_acquire_fence_signal_release(client->node->fence);
		client->node->fence = NULL;
	}

	/* 2. refresh config cmdlist item count */
	if ((client->node_type & REGISTER_CONFIG_TYPE) != 0) {
		if (user_client->valid_payload_size > CMDLIST_ITEMS_COUNT_MAX) {
			dpu_pr_err("invalid valid payload size");
			return -1;
		}
		/* Command node contains the number of cmd_items (namely the total_items
		 * cmd_header) cannot be equal to 1, if there are any cmd_item only one,
		 * you can copy the data, and make total_items equal to 2
		 */
		if (user_client->valid_payload_size < 2) {
			dpu_pr_err("config cmdlist error item count: %d", user_client->valid_payload_size);
			user_client->valid_payload_size = 2;
		}
		client->node->list_cmd_header->total_items.bits.items_count = user_client->valid_payload_size;
	}

	return 0;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
/**
 * @brief Dump this current scene cmdlist node
 *
 * @param cmd_table cmdlist client table
 * @param user_client use this client id
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_dump_client(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client = NULL;

	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("input error, cmd_table or user_client is null pointer!");
		return -1;
	}

	client = cmdlist_get_current_client(cmd_table, user_client->scene_id, user_client->id);
	if (unlikely(!client)) {
		dpu_pr_err("invalid current cmdlist client");
		return -1;
	}

	cmdlist_dump_node(client->node);

	return 0;
}

static void cmdlist_hash_node_dump(struct cnode *node)
{
	struct cmdlist_client *client = NULL;

	if (unlikely(!node))
		return;

	client = cnode_entry(node, struct cmdlist_client, hash_node);
	if (unlikely(!client))
		return;

	dpu_pr_debug("--> dump client <%u, %pK, %u>", client->key, client->node, client->node_type);
	dpu_pr_debug("dump node[%pK].scene[%d].mode[%d].size[%#x].prev=%pK.next=%pK.list_cmd_header=%pK.fence=%pK",
		client->node, client->node->scene_id, client->node->cmd_mode, client->node->node_size,
		client->node->cnode.prev, client->node->cnode.next,
		client->node->list_cmd_header, client->node->fence);
	cmdlist_dump_node(client->node);
}

/**
 * @brief Dump all the same scene cmdlist nodes
 *
 * @param cmd_table cmdlist client table
 * @param user_client use the scene_id
 * @return int32_t 0: success -1: fail
 */
int32_t cmdlist_dump_scene(struct cmdlist_table *cmd_table, struct cmdlist_node_client *user_client)
{
	if (unlikely(!cmd_table || !user_client)) {
		dpu_pr_err("input error, cmd_table or user_client is null pointer!");
		return -1;
	}

	chash_bkt_dump(&cmd_table->ht, user_client->scene_id, cmdlist_hash_node_dump);

	return 0;
}
#endif
