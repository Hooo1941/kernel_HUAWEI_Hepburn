/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "lbuf_mgr.h"
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include "res_mgr.h"
#include "dkmd_opr_id.h"
#include "dkmd_log.h"

#define DISP_LOG_TAG "RES_MGR"

struct used_node_info {
	struct list_head list_node;
	int32_t user_type;
	int32_t scene_id;
	int32_t user_tgid;
	uint32_t used_lb;
};

struct reserved_node_info {
	struct list_head list_node;
	int32_t user_type;
	int32_t scene_id;
	int32_t user_tgid;
	uint32_t reserved_lb; // current lb which has been reserved
	uint32_t reserve_lb; // total lb which need to be reserved
};

struct lbuf_node_info {
	struct semaphore sem;
	uint32_t hw_node_id;
	uint32_t used_lb;
	uint32_t max_lb;
	struct list_head used_node_head;
	struct list_head reserved_node_head;
};

struct lbuf_pool_info {
	uint32_t node_size;
	struct lbuf_node_info *node_info;
};

struct dpu_lbuf_mgr {
	uint32_t pool_size;
	struct lbuf_pool_info *lb_pool_info;
	struct semaphore sem;
	bool has_turn_to_client;
};

static int32_t init_lbuf_node(const struct lbuf_pool_config_info *lb_cfg_info,
	struct lbuf_pool_info *lb_pool_info)
{
	uint32_t node_index;
	struct lbuf_node_info *node_info = NULL;
	struct lbuf_node_config_info *node_config_info = NULL;

	if (unlikely(!lb_cfg_info)) {
		dpu_pr_err("lb_cfg_info is nullptr");
		return -1;
	}

	lb_pool_info->node_size = lb_cfg_info->node_size;
	if (lb_pool_info->node_size == 0) {
		dpu_pr_err("node_size's value cannot be zero!\n");
		return -1;
	}
	lb_pool_info->node_info = kzalloc(sizeof(struct lbuf_node_info) * lb_pool_info->node_size, GFP_KERNEL);
	if (unlikely(!lb_pool_info->node_info)) {
		dpu_pr_err("kzalloc lb node info fail");
		return -ENOMEM;
	}

	for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index) {
		node_info = &lb_pool_info->node_info[node_index];
		node_config_info = &lb_cfg_info->node_config_info[node_index];
		if (unlikely(!node_config_info)) {
			dpu_pr_err("node_config_info is nullptr");
			kfree(lb_pool_info->node_info);
			lb_pool_info->node_info = NULL;
			return -1;
		}

		sema_init(&node_info->sem, 1);
		node_info->hw_node_id = (uint32_t)node_config_info->hw_node_id;
		node_info->used_lb = 0;
		node_info->max_lb = node_config_info->max_lb;
		INIT_LIST_HEAD(&node_info->used_node_head);
		INIT_LIST_HEAD(&node_info->reserved_node_head);
		dpu_pr_info("init lbuf node %u succ: hw_node_id=%u max_lb=%u",
			node_index, node_config_info->hw_node_id, node_config_info->max_lb);
	}

	return 0;
}

static void init_lbuf_pool(struct dpu_lbuf_mgr *lbuf_mgr)
{
	uint32_t pool_index;
	struct lbuf_pool_node_config_info *lb_pool_node_config_info= dpu_get_lb_pool_node_config_info();

	if (!lb_pool_node_config_info) {
		dpu_pr_warn("lb_pool_node_config_info is nullptr");
		return;
	}

	lbuf_mgr->pool_size = lb_pool_node_config_info->pool_size;
	if (lbuf_mgr->pool_size == 0) {
		dpu_pr_err("pool_size's value cannot be zero!\n");
		return;
	}
	lbuf_mgr->lb_pool_info = kzalloc(sizeof(struct lbuf_pool_info) * lbuf_mgr->pool_size, GFP_KERNEL);
	if (!lbuf_mgr->lb_pool_info) {
		dpu_pr_err("kzalloc lb pool info fail");
		return;
	}
	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		if (init_lbuf_node(&lb_pool_node_config_info->lb_cfg_info[pool_index],
			&lbuf_mgr->lb_pool_info[pool_index]))
			continue;
		dpu_pr_info("init lbuf pool %u succ", pool_index);
	}
}

static void *lbuf_mgr_init(struct dpu_res_data *rm_data)
{
	struct dpu_lbuf_mgr *lbuf_mgr = kzalloc(sizeof(struct dpu_lbuf_mgr), GFP_KERNEL);
	if (!lbuf_mgr)
		return NULL;

	init_lbuf_pool(lbuf_mgr);
	sema_init(&lbuf_mgr->sem, 1);
	lbuf_mgr->has_turn_to_client = false;

	return lbuf_mgr;
}

static void deinit_lbuf_pool(struct dpu_lbuf_mgr *lbuf_mgr)
{
	uint32_t pool_index;
	struct lbuf_pool_info *lb_pool_info = NULL;

	if (lbuf_mgr->pool_size == 0)
		return;

	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		lb_pool_info = &lbuf_mgr->lb_pool_info[pool_index];
		if (!lb_pool_info)
			continue;

		if (lb_pool_info->node_info) {
			kfree(lb_pool_info->node_info);
			lb_pool_info->node_info = NULL;
		}
	}
}

static void lbuf_mgr_deinit(void *data)
{
	struct dpu_lbuf_mgr *lbuf_mgr = (struct dpu_lbuf_mgr *)data;
	if (!lbuf_mgr)
		return;

	deinit_lbuf_pool(lbuf_mgr);
	kfree(lbuf_mgr);
	lbuf_mgr = NULL;
}

static void lbuf_mgr_reset_node_info(struct lbuf_node_info *node_info)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	down(&node_info->sem);
	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		dpu_pr_debug("node: hw_node_id=%u used_lb=%u max_lb=%u release: user_type=%d scene_id=%d used_lb=%u",
			node_info->hw_node_id, node_info->used_lb, node_info->max_lb,
			used_node->user_type, used_node->scene_id, used_node->used_lb);
		list_del(&used_node->list_node);
		kfree(used_node);
		used_node = NULL;
	}

	node_info->used_lb = 0;

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		dpu_pr_debug("release reserved node: user_type=%d reserved=%u",
			rsv_node->user_type, rsv_node->reserved_lb);
		list_del(&rsv_node->list_node);
		kfree(rsv_node);
		rsv_node = NULL;
	}
	up(&node_info->sem);
}

static void lbuf_mgr_reset(void *data)
{
	struct dpu_lbuf_mgr *lbuf_mgr = (struct dpu_lbuf_mgr *)data;
	uint32_t node_index;
	uint32_t pool_index;
	struct lbuf_pool_info *lb_pool_info = NULL;

	dpu_pr_debug("reset lbuf_mgr");

	if (!lbuf_mgr)
		return;

	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		lb_pool_info = &lbuf_mgr->lb_pool_info[pool_index];
		if (!lb_pool_info)
			continue;

		if (!lb_pool_info->node_info)
			continue;

		for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index)
			lbuf_mgr_reset_node_info(&lb_pool_info->node_info[node_index]);
	}
}

static void lbuf_mgr_release_node_info(struct lbuf_node_info *node_info, int32_t tgid)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	down(&node_info->sem);
	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		if (used_node->user_tgid == tgid) {
			dpu_pr_debug("node: hw_node_id=%u used_lb=%u max_lb=%u release:user_type=%d scene_id=%d used_lb=%u",
				node_info->hw_node_id, node_info->used_lb, node_info->max_lb,
				used_node->user_type, used_node->scene_id, used_node->used_lb);
			node_info->used_lb -= used_node->used_lb;
			list_del(&used_node->list_node);
			kfree(used_node);
			used_node = NULL;
		}
	}

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		if (rsv_node->user_tgid == tgid) {
			dpu_pr_debug("release reserved node: user_type=%d scene_id=%d reserved=%u",
				rsv_node->user_type, rsv_node->scene_id, rsv_node->reserved_lb);
			node_info->used_lb -= rsv_node->reserved_lb;
			list_del(&rsv_node->list_node);
			kfree(rsv_node);
			rsv_node = NULL;
		}
	}
	up(&node_info->sem);
}

static void lbuf_mgr_release(void *data)
{
	struct dpu_lbuf_mgr *lbuf_mgr = (struct dpu_lbuf_mgr *)data;
	uint32_t node_index;
	uint32_t pool_index;
	struct lbuf_pool_info *lb_pool_info = NULL;
	int32_t tgid = task_tgid_vnr(current);

	dpu_pr_debug("release lbuf_mgr tgid=%d", tgid);

	if (!lbuf_mgr)
		return;

	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		lb_pool_info = &lbuf_mgr->lb_pool_info[pool_index];
		if (!lb_pool_info)
			continue;

		if (!lb_pool_info->node_info)
			continue;

		for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index)
			lbuf_mgr_release_node_info(&lb_pool_info->node_info[node_index], tgid);
	}
}

static bool lbuf_mgr_is_reserve_lb_satisfy(const struct lbuf_node_info *node_info)
{
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		if (rsv_node->reserved_lb < rsv_node->reserve_lb) {
			dpu_pr_info("current node: hw_node_id=%u used_lb=%u max_lb=%u cannot request, "
				"reserve node: user_type=%d user_tgid=%d reserved_lb=%u reserve_lb=%u",
				node_info->hw_node_id, node_info->used_lb, node_info->max_lb,
				rsv_node->user_type, rsv_node->user_tgid, rsv_node->reserved_lb, rsv_node->reserve_lb);
			return false;
		}
	}

	return true;
}

static uint32_t lbuf_mgr_get_node_need_reserved_lb(const struct lbuf_node_info *node_info)
{
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;
	uint32_t need_reserved_lb = 0;

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		need_reserved_lb += rsv_node->reserve_lb - rsv_node->reserved_lb;
	}

	return need_reserved_lb;
}

static void lbuf_mgr_dec_used_node_lb(struct lbuf_node_info *node_info, const struct req_lbuf_info *lb_info,
	const struct req_lbuf_node_info *req_node_info)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;

	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		if ((used_node->user_type == lb_info->user_type) &&
			(used_node->scene_id == lb_info->scene_id)) {
			dpu_pr_debug("node: hw_node_id=%u total used_lb=%u max_lb=%u, used node: user_type=%d scene_id=%d"
				" used_lb=%u, dec_value=%u", node_info->hw_node_id, node_info->used_lb, node_info->max_lb,
				used_node->user_type, used_node->scene_id, used_node->used_lb, req_node_info->used_lb);

			if (req_node_info->used_lb > used_node->used_lb) {
				dpu_pr_err("dec_value=%u is greater than current used_lb=%u", req_node_info->used_lb, used_node->used_lb);
				return; // user of same scene only exists once in one node
			}
			used_node->used_lb -= req_node_info->used_lb;
			return; // user of same scene only exists once in one node
		}
	}

	dpu_pr_err("not found: user_type=%d scene_id=%d hw_pool_id=%d hw_node_id=%d used_lb=%u",
		lb_info->user_type, lb_info->scene_id, req_node_info->hw_pool_id, req_node_info->hw_node_id,
		req_node_info->used_lb);
}

static void lbuf_mgr_inc_used_node_lb(struct lbuf_node_info *node_info, const struct req_lbuf_info *lb_info,
	const struct req_lbuf_node_info *req_node_info, bool need_create_node)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;

	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		if ((used_node->user_type == lb_info->user_type) &&
			(used_node->scene_id == lb_info->scene_id)) {
			dpu_pr_debug("node: hw_node_id=%u total used_lb=%u max_lb=%u, used node: user_type=%d scene_id=%d"
				" used_lb=%u, inc_value=%u", node_info->hw_node_id, node_info->used_lb, node_info->max_lb,
				used_node->user_type, used_node->scene_id, used_node->used_lb, req_node_info->used_lb);

			used_node->used_lb += req_node_info->used_lb;
			return; // user of same scene only exists once in one node
		}
	}

	if (!need_create_node)
		return;

	// create a new used_node when there is no node in list of used_node_head
	used_node = kzalloc(sizeof(struct used_node_info), GFP_KERNEL);
	if (!used_node) {
		dpu_pr_err("used_node is nullptr");
		return;
	}

	used_node->user_type = lb_info->user_type;
	used_node->scene_id = lb_info->scene_id;
	used_node->user_tgid = task_tgid_vnr(current);
	used_node->used_lb = req_node_info->used_lb;
	list_add_tail(&used_node->list_node, &node_info->used_node_head);
	return;
}

static int32_t lbuf_mgr_check_pool_node(struct dpu_lbuf_mgr *lbuf_mgr, const struct req_lbuf_info *lb_info,
	const struct req_lbuf_node_info *req_node_info, struct lbuf_pool_info **lb_pool_info,
	struct lbuf_node_info **node_info)
{
	struct lbuf_pool_info *lb_pool_info_ = NULL;
	struct lbuf_node_info *node_info_ = NULL;

	dpu_pr_debug("node: user_type=%d node_num=%u hw_pool_id=%d hw_node_id=%d used_lb=%u",
		lb_info->user_type, lb_info->node_num, req_node_info->hw_pool_id, req_node_info->hw_node_id,
		req_node_info->used_lb);

	if (unlikely(req_node_info->hw_pool_id < 0 || req_node_info->hw_pool_id >= lbuf_mgr->pool_size)) {
		dpu_pr_err("node: user_type=%d node_num=%u hw_pool_id=%d->invalid hw_node_id=%d used_lb=%u",
			lb_info->user_type, lb_info->node_num, req_node_info->hw_pool_id, req_node_info->hw_node_id,
			req_node_info->used_lb);
		return -1;
	}

	lb_pool_info_ = &lbuf_mgr->lb_pool_info[req_node_info->hw_pool_id];
	if (unlikely(!lb_pool_info_) || unlikely(!lb_pool_info_->node_info)) {
		dpu_pr_err("lb_pool_info or node_info is nullptr");
		return -1;
	}

	if (unlikely(req_node_info->hw_node_id < 0 || req_node_info->hw_node_id >= lb_pool_info_->node_size)) {
		dpu_pr_err("node: user_type=%d node_num=%u hw_pool_id=%d hw_node_id=%d->invalid used_lb=%u",
			lb_info->user_type, lb_info->node_num, req_node_info->hw_pool_id, req_node_info->hw_node_id,
			req_node_info->used_lb);
		return -1;
	}

	node_info_ = &lb_pool_info_->node_info[req_node_info->hw_node_id];
	if (unlikely(!node_info_)) {
		dpu_pr_err("node_info is nullptr");
		return -1;
	}

	*lb_pool_info = lb_pool_info_;
	*node_info = node_info_;
	return 0;
}

static void lbuf_mgr_update_requested_node_info(struct req_lbuf_info *lb_info,
	int32_t hw_pool_id, int32_t hw_node_id, uint32_t requested_lb)
{
	uint32_t pool_index;

	if (requested_lb == 0)
		return;

	for (pool_index = 0; pool_index < lb_info->node_num; ++pool_index) {
		if (lb_info->req_lb_node_info[pool_index].hw_pool_id == hw_pool_id &&
			lb_info->req_lb_node_info[pool_index].hw_node_id == hw_node_id) {
			lb_info->req_lb_node_info[pool_index].used_lb += requested_lb;
			return;
		}
	}

	if (lb_info->node_num == LBUF_NODE_NUM_MAX) {
		dpu_pr_err("node_num will be greater than %d", LBUF_NODE_NUM_MAX);
		return;
	}

	lb_info->req_lb_node_info[lb_info->node_num].hw_pool_id = hw_pool_id;
	lb_info->req_lb_node_info[lb_info->node_num].hw_node_id = hw_node_id;
	lb_info->req_lb_node_info[lb_info->node_num].used_lb = requested_lb;
	++lb_info->node_num;
}

static void lbuf_mgr_request_node(struct dpu_lbuf_mgr *lbuf_mgr, struct req_lbuf_info *lb_info,
	const struct lbuf_conn_pool_config_info *lb_conn_pool_cfg_info, uint32_t *request_lb)
{
	uint32_t node_index;
	int32_t hw_pool_id = lb_conn_pool_cfg_info->hw_pool_id;
	int32_t hw_node_id = 0;
	struct lbuf_node_info *node_info = NULL;
	uint32_t remained_lb = 0;
	uint32_t need_reserved_lb = 0;

	for (node_index = 0; node_index < lb_conn_pool_cfg_info->node_size; ++node_index) {
		hw_node_id = lb_conn_pool_cfg_info->hw_node_ids[node_index];
		node_info = &lbuf_mgr->lb_pool_info[hw_pool_id].node_info[hw_node_id];

		down(&node_info->sem);
		dpu_pr_debug("node_index=%u hw_pool_id=%d hw_node_id=%d used_lb=%u max_lb=%u request_lb=%u requested_size=%u",
			node_index, hw_pool_id, hw_node_id, node_info->used_lb, node_info->max_lb, *request_lb, lb_info->node_num);

		if (!lbuf_mgr_is_reserve_lb_satisfy(node_info)) {
			dpu_pr_warn("request node: stop request cause reserve is not satisfied");
			up(&node_info->sem);
			return;
		}

		remained_lb = node_info->max_lb - node_info->used_lb;
		need_reserved_lb = lbuf_mgr_get_node_need_reserved_lb(node_info);
		if (need_reserved_lb > remained_lb) {
			dpu_pr_warn("request node: stop request cause need_reserved_lb=%u is greater than "
				"remained=%u: max_lb=%u used_lb=%u",
				need_reserved_lb, remained_lb, node_info->max_lb, node_info->used_lb);
			up(&node_info->sem);
			return;
		}

		remained_lb -= need_reserved_lb;
		if (*request_lb <= remained_lb) {
			dpu_pr_debug("request node: succ-> request_lb=%u remained_lb=%u", *request_lb, remained_lb);
			node_info->used_lb += *request_lb;
			up(&node_info->sem);
			lbuf_mgr_update_requested_node_info(lb_info, hw_pool_id, hw_node_id, *request_lb);
			*request_lb = 0; // set to 0 for successfully stop traverse
			return;
		}

		// current node lb size cannot satisfy request size, just use remained size and continue traverse
		node_info->used_lb += remained_lb;
		*request_lb -= remained_lb;
		up(&node_info->sem);
		lbuf_mgr_update_requested_node_info(lb_info, hw_pool_id, hw_node_id, remained_lb);
	}
}

static void lbuf_mgr_get_scene_node_lb(struct lbuf_node_info *node_info, int32_t scene_id,
	uint32_t *used_scene_lb_num, uint32_t *used_other_scene_lb_num)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	down(&node_info->sem);
	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		dpu_pr_debug("used_node:scene_id=%d used_lb=%u", used_node->scene_id, used_node->used_lb);
		if (used_node->scene_id == scene_id)
			*used_scene_lb_num += used_node->used_lb;
		else
			*used_other_scene_lb_num += used_node->used_lb;
	}

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		dpu_pr_debug("rsv_node_node:scene_id=%d used_lb=%u", rsv_node->scene_id, rsv_node->reserved_lb);
		if (rsv_node->scene_id == scene_id)
			*used_scene_lb_num += rsv_node->reserved_lb;
		else
			*used_other_scene_lb_num += rsv_node->reserved_lb;
	}
	up(&node_info->sem);
}

static uint32_t lbuf_mgr_get_pool_a_lb_num(struct dpu_lbuf_mgr *lbuf_mgr, uint32_t scene_id,
	uint32_t *used_scene_pool_a_lb_num)
{
	uint32_t node_index;
	uint32_t used_other_pool_a_lb_num = 0;
	struct lbuf_pool_info *lb_pool_info = NULL;

	if (unlikely(HW_POOL_ID_A >= lbuf_mgr->pool_size)) {
		dpu_pr_err("HW_POOL_ID_A=%u is out of range", HW_POOL_ID_A);
		return 0;
	}

	lb_pool_info = &lbuf_mgr->lb_pool_info[HW_POOL_ID_A];
	if (!lb_pool_info->node_info)
		return 0;

	for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index)
		lbuf_mgr_get_scene_node_lb(&lb_pool_info->node_info[node_index], (int32_t)scene_id,
			used_scene_pool_a_lb_num, &used_other_pool_a_lb_num);

	return used_other_pool_a_lb_num;
}

static void lbuf_mgr_get_pool_b_lb_num_func(struct lbuf_node_info *node_info,
	uint32_t *scene_pool_b_lb_num)
{
	struct used_node_info *used_node = NULL;
	struct used_node_info *_u_node_ = NULL;
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	down(&node_info->sem);
	list_for_each_entry_safe(used_node, _u_node_, &node_info->used_node_head, list_node) {
		if (!used_node)
			continue;

		dpu_pr_debug("used_node:scene_id=%d used_lb=%u", used_node->scene_id, used_node->used_lb);
		scene_pool_b_lb_num[used_node->scene_id] += used_node->used_lb;
	}

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		dpu_pr_debug("rsv_node_node:scene_id=%d used_lb=%u", rsv_node->scene_id, rsv_node->reserved_lb);
		scene_pool_b_lb_num[rsv_node->scene_id] += rsv_node->reserved_lb;
	}
	up(&node_info->sem);
}

static uint32_t lbuf_mgr_get_pool_b_lb_num(struct dpu_lbuf_mgr *lbuf_mgr, uint32_t scene_id)
{
	uint32_t node_index;
	struct lbuf_pool_info *lb_pool_info = &lbuf_mgr->lb_pool_info[HW_POOL_ID_B];
	uint32_t scene_pool_b_lb_num[DPU_SCENE_MAX] = {0};
	uint32_t pool_b_lb_num = 0;

	if (!lb_pool_info->node_info)
		return 0;

	for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index)
		lbuf_mgr_get_pool_b_lb_num_func(&lb_pool_info->node_info[node_index], (uint32_t *)scene_pool_b_lb_num);

	for (node_index = 0; node_index < (uint32_t)DPU_SCENE_MAX; ++node_index) {
		if (node_index == scene_id)
			continue;
		pool_b_lb_num += dpu_get_pool_b_lb_constraint(scene_pool_b_lb_num[node_index]);
		dpu_pr_debug("scene%d pool_b_lb_num=%u", node_index, scene_pool_b_lb_num[node_index]);
	}

	return pool_b_lb_num;
}

static void lbuf_mgr_turn_to_client(struct dpu_lbuf_mgr *lbuf_mgr,
	const struct req_lbuf_info *lb_info)
{
	if (lb_info->user_type != SCENE_USER_MDC)
		return;

	down(&lbuf_mgr->sem);
	dpu_res_send_uevent(RES_EVENT_TURN_TO_CLIENT);
	lbuf_mgr->has_turn_to_client = true;
	dpu_pr_info("mdc request lb fail, need turn to client");
	up(&lbuf_mgr->sem);
}

static void lbuf_mgr_turn_to_device(struct dpu_lbuf_mgr *lbuf_mgr,
	const struct req_lbuf_info *lb_info)
{
	if (lb_info->user_type != SCENE_USER_MDC)
		return;

	down(&lbuf_mgr->sem);
	if (lbuf_mgr->has_turn_to_client) {
		dpu_res_send_uevent(RES_EVENT_TURN_TO_DEVICE);
		lbuf_mgr->has_turn_to_client = false;
		dpu_pr_info("mdc release lb succ, need turn to device");
	}
	up(&lbuf_mgr->sem);
}

static int32_t lbuf_mgr_check_scene_lb_num(struct dpu_lbuf_mgr *lbuf_mgr,
	const struct req_lbuf_info *lb_info, bool exclude_scene)
{
	uint32_t used_scene_pool_a_lb_num = 0;
	uint32_t used_other_pool_a_lb_num = 0;
	uint32_t used_other_pool_b_lb_num = 0;
	uint32_t scene_lb_num = 0;

	void_unused(exclude_scene);

	used_other_pool_a_lb_num =
		lbuf_mgr_get_pool_a_lb_num(lbuf_mgr, (uint32_t)lb_info->scene_id, &used_scene_pool_a_lb_num);

	used_other_pool_b_lb_num = lbuf_mgr_get_pool_b_lb_num(lbuf_mgr, (uint32_t)lb_info->scene_id);
	scene_lb_num = lb_info->req_scene_lb_num + used_other_pool_a_lb_num + used_other_pool_b_lb_num;

	dpu_pr_info("user_type=%d scene%d req_scene_lb_num=%u used_scene_pool_a_lb_num=%u, other scene:used total "
		"pool_a_lb_num=%u pool_b_lb_num=%u, current total scene_lb_num=%u", lb_info->user_type, lb_info->scene_id,
		lb_info->req_scene_lb_num, used_scene_pool_a_lb_num, used_other_pool_a_lb_num, used_other_pool_b_lb_num,
		scene_lb_num);
	if (scene_lb_num > dpu_get_pool_a_lb_max_size()) {
		dpu_pr_warn("scene_lb_num=%u is overflow %u", scene_lb_num, dpu_get_pool_a_lb_max_size());
		return -1;
	}

	return 0;
}

/*
 * When user invoke request operation, lbuf_mgr will do these things to handle its demand
 * 1、check request information valid
 * 2、max_lb dec used_lb of current node, which is denoted as remained_lb,
 * can satisfy current reserve demand in reserved_node_head, remained_lb will dec reserve lb size
 * 3、create a new node of used type if there is none, or use a created one otherwise
 * 4、if remained_lb can satisfy request used lb size, node will update to used_node.
 * if remained_lb cannot satisfy, continue search for next pool, if fail, clear lb info of requested, and return -1.
 */
static int32_t lbuf_mgr_request_lb(struct dpu_lbuf_mgr *lbuf_mgr, struct req_lbuf_info *lb_info)
{
	int32_t ret = 0;
	uint32_t req_conn_index;
	const struct lbuf_opr_conn_info *req_conn_info = NULL;
	struct lbuf_pool_info *lb_pool_info = NULL;
	struct lbuf_node_info *node_info = NULL;
	const struct lbuf_conn_pool_node_config_info *conn_pnc_info = NULL;
	uint32_t conn_pool_size = 0;
	uint32_t conn_pool_cfg_idx = 0;
	uint32_t request_lb = 0;
	const struct req_lbuf_node_info *req_node_info = NULL;

	lb_info->node_num = 0;

	if (lbuf_mgr_check_scene_lb_num(lbuf_mgr, lb_info, false) != 0) {
		lbuf_mgr_turn_to_client(lbuf_mgr, lb_info);
		return -1;
	}

	for (req_conn_index = 0; req_conn_index < lb_info->conn_num; ++req_conn_index) {
		req_conn_info = &lb_info->opr_conn_info[req_conn_index];
		dpu_pr_debug("req_conn_index=%u: scene_id=%d cur_opr=%#x nxt_opr=%#x used_lb=%u", req_conn_index,
			lb_info->scene_id, req_conn_info->cur_opr, req_conn_info->nxt_opr, req_conn_info->used_lb);

		conn_pnc_info = dpu_get_lb_conn_pool_node_config_info(req_conn_info->cur_opr,
			req_conn_info->nxt_opr, &conn_pool_size);
		if (unlikely(!conn_pnc_info)) {
			dpu_pr_err("request node: stop request cause get connection pool node config fail");
			ret = -1;
			break;
		}

		request_lb = req_conn_info->used_lb;
		if (conn_pool_size == 1) { // means opr connect supports node with one pool id, such as SDMA0->OV0
			lbuf_mgr_request_node(lbuf_mgr, lb_info,
				&conn_pnc_info[0].lb_conn_pool_cfg_info[conn_pnc_info[0].conn_pool_index], &request_lb);
			if (request_lb > 0) {
				dpu_pr_warn("request node: stop request cur_opr=%#x nxt_opr=%#x used_lb=%u", req_conn_info->cur_opr,
					req_conn_info->nxt_opr, req_conn_info->used_lb);
				ret = -1;
				break;
			}
			continue;
		}

		// means opr connect supports node with several pool id, such as SROT0->VSCF1
		for (conn_pool_cfg_idx = 0; conn_pool_cfg_idx < conn_pool_size; ++conn_pool_cfg_idx)
			lbuf_mgr_request_node(lbuf_mgr, lb_info,
				&conn_pnc_info[conn_pool_cfg_idx]. \
				lb_conn_pool_cfg_info[conn_pnc_info[conn_pool_cfg_idx].conn_pool_index],
				&request_lb);
		if (request_lb > 0) {
			dpu_pr_warn("request node: stop request cur_opr=%#x nxt_opr=%#x used_lb=%u", req_conn_info->cur_opr,
				req_conn_info->nxt_opr, req_conn_info->used_lb);
			ret = -1;
			break;
		}
	}

	if (unlikely(lb_info->node_num > LBUF_NODE_NUM_MAX)) {
		dpu_pr_err("lb_info->node_num=%u is out of range", lb_info->node_num);
		return -1;
	}

	if (ret) {
		for (req_conn_index = 0; req_conn_index < lb_info->node_num; ++req_conn_index) {
			req_node_info = &lb_info->req_lb_node_info[req_conn_index];
			lb_pool_info = &lbuf_mgr->lb_pool_info[req_node_info->hw_pool_id];
			node_info = &lb_pool_info->node_info[req_node_info->hw_node_id];

			down(&node_info->sem);
			node_info->used_lb -= req_node_info->used_lb;
			up(&node_info->sem);
		}

		lbuf_mgr_turn_to_client(lbuf_mgr, lb_info);

		return -1;
	}

	for (req_conn_index = 0; req_conn_index < lb_info->node_num; ++req_conn_index) {
		req_node_info = &lb_info->req_lb_node_info[req_conn_index];
		lb_pool_info = &lbuf_mgr->lb_pool_info[req_node_info->hw_pool_id];
		node_info = &lb_pool_info->node_info[req_node_info->hw_node_id];

		dpu_pr_debug("req node: hw_pool_id=%d hw_node_id=%d total used_lb=%u",
			req_node_info->hw_pool_id, req_node_info->hw_node_id, req_node_info->used_lb);
		down(&node_info->sem);
		lbuf_mgr_inc_used_node_lb(node_info, lb_info, req_node_info, true);
		up(&node_info->sem);
	}

	lbuf_mgr_turn_to_device(lbuf_mgr, lb_info);

	return 0;
}

static int32_t lbuf_mgr_release_lb(struct dpu_lbuf_mgr *lbuf_mgr, const struct req_lbuf_info *lb_info)
{
	uint32_t req_node_index;
	uint32_t err_req_node_size;
	const struct req_lbuf_node_info *req_node_info = NULL;
	struct lbuf_pool_info *lb_pool_info = NULL;
	struct lbuf_node_info *node_info = NULL;

	for (req_node_index = 0; req_node_index < lb_info->node_num; ++req_node_index) {
		req_node_info = &lb_info->req_lb_node_info[req_node_index];
		if (lbuf_mgr_check_pool_node(lbuf_mgr, lb_info, req_node_info, &lb_pool_info, &node_info)) {
			dpu_pr_err("release node: stop release cause check fail");
			break;
		}

		down(&node_info->sem);
		if (req_node_info->used_lb > node_info->used_lb) {
			dpu_pr_err("release node: skip release cause release_lb=%u is greater than used_lb=%u",
				req_node_info->used_lb, node_info->used_lb);
			up(&node_info->sem);
			continue;
		}
		node_info->used_lb -= req_node_info->used_lb;

		lbuf_mgr_dec_used_node_lb(node_info, lb_info, req_node_info);
		up(&node_info->sem);
	}

	if (req_node_index != lb_info->node_num) {
		err_req_node_size = req_node_index;
		for (req_node_index = 0; req_node_index < err_req_node_size; ++req_node_index) {
			req_node_info = &lb_info->req_lb_node_info[req_node_index];
			lb_pool_info = &lbuf_mgr->lb_pool_info[req_node_info->hw_pool_id];
			node_info = &lb_pool_info->node_info[req_node_info->hw_node_id];

			down(&node_info->sem);
			lbuf_mgr_inc_used_node_lb(node_info, lb_info, req_node_info, false);
			node_info->used_lb += req_node_info->used_lb;
			up(&node_info->sem);
		}
		return -1;
	}

	return 0;
}

static struct reserved_node_info *lbuf_mgr_get_reserved_lb_node(struct lbuf_node_info *node_info,
	struct req_lbuf_info *lb_info)
{
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		if ((rsv_node->user_type == lb_info->user_type) &&
			(rsv_node->scene_id == lb_info->scene_id))
			return rsv_node; // user of same scene only exists once in one node
	}

	rsv_node = kzalloc(sizeof(struct reserved_node_info), GFP_KERNEL);
	if (!rsv_node) {
		dpu_pr_err("rsv_node is nullptr");
		return NULL;
	}

	rsv_node->user_type = lb_info->user_type;
	rsv_node->scene_id = lb_info->scene_id;
	rsv_node->user_tgid = task_tgid_vnr(current);
	rsv_node->reserved_lb = 0;
	rsv_node->reserve_lb = 0;

	list_add_tail(&rsv_node->list_node, &node_info->reserved_node_head);

	return rsv_node;
}

static void lbuf_mgr_clear_reserve_node_info(struct lbuf_node_info *node_info,
	const struct req_lbuf_info *lb_info)
{
	struct reserved_node_info *rsv_node = NULL;
	struct reserved_node_info *_r_node_ = NULL;

	down(&node_info->sem);
	list_for_each_entry_safe(rsv_node, _r_node_, &node_info->reserved_node_head, list_node) {
		if (!rsv_node)
			continue;

		if ((rsv_node->user_type == lb_info->user_type) &&
			(rsv_node->scene_id == lb_info->scene_id)) {
			dpu_pr_debug("clear reserved node: user_type=%d scene_id=%d reserved=%u",
				rsv_node->user_type, rsv_node->scene_id, rsv_node->reserved_lb);
			node_info->used_lb -= rsv_node->reserved_lb;
			rsv_node->reserve_lb = 0;
			rsv_node->reserved_lb = 0;
		}
	}
	up(&node_info->sem);
}

static void lbuf_mgr_clear_user_none_reserve_node_info(struct dpu_lbuf_mgr *lbuf_mgr,
	struct req_lbuf_info *lb_info)
{
	uint32_t node_index;
	uint32_t pool_index;
	uint32_t requested_index;
	struct lbuf_pool_info *lb_pool_info = NULL;
	const struct req_lbuf_node_info *req_node_info = NULL;
	bool has_requested = false;

	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		lb_pool_info = &lbuf_mgr->lb_pool_info[pool_index];
		if (!lb_pool_info)
			continue;

		if (!lb_pool_info->node_info)
			continue;

		for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index) {
			has_requested = false;
			for (requested_index = 0; requested_index < lb_info->node_num; ++requested_index) {
				req_node_info = &lb_info->req_lb_node_info[requested_index];
				if (pool_index == req_node_info->hw_pool_id && node_index == req_node_info->hw_node_id)
					has_requested = true;
			}

			if (!has_requested)
				lbuf_mgr_clear_reserve_node_info(&lb_pool_info->node_info[node_index], lb_info);
		}
	}
}

static void lbuf_mgr_clear_user_reserve_node_info(struct dpu_lbuf_mgr *lbuf_mgr,
	const struct req_lbuf_info *lb_info)
{
	uint32_t node_index;
	uint32_t pool_index;
	struct lbuf_pool_info *lb_pool_info = NULL;

	for (pool_index = 0; pool_index < lbuf_mgr->pool_size; ++pool_index) {
		lb_pool_info = &lbuf_mgr->lb_pool_info[pool_index];
		if (!lb_pool_info)
			continue;

		if (!lb_pool_info->node_info)
			continue;

		for (node_index = 0; node_index < lb_pool_info->node_size; ++node_index)
			lbuf_mgr_clear_reserve_node_info(&lb_pool_info->node_info[node_index], lb_info);
	}
}

static void lbuf_mgr_reserve_node(struct dpu_lbuf_mgr *lbuf_mgr, struct req_lbuf_info *lb_info,
	const struct lbuf_conn_pool_config_info *lb_conn_pool_cfg_info, uint32_t *reserve_lb)
{
	uint32_t node_index;
	int32_t hw_pool_id = lb_conn_pool_cfg_info->hw_pool_id;
	int32_t hw_node_id = 0;
	struct lbuf_node_info *node_info = NULL;
	struct reserved_node_info *rsv_node = NULL;
	uint32_t remained_lb = 0;
	uint32_t need_reserved_lb = 0;

	for (node_index = 0; node_index < lb_conn_pool_cfg_info->node_size; ++node_index) {
		hw_node_id = lb_conn_pool_cfg_info->hw_node_ids[node_index];
		node_info = &lbuf_mgr->lb_pool_info[hw_pool_id].node_info[hw_node_id];

		down(&node_info->sem);
		rsv_node = lbuf_mgr_get_reserved_lb_node(node_info, lb_info);
		if (!rsv_node) {
			dpu_pr_err("reserve node: skip reserve%u cause get reserve node fail", node_index);
			up(&node_info->sem);
			return;
		}

		if (*reserve_lb <= rsv_node->reserved_lb) {
			dpu_pr_debug("reserve node: succ-> reserve%u reserve_lb=%u reserved_lb=%u "
				"remained=%u: max_lb=%u used_lb=%u", node_index, *reserve_lb,
				rsv_node->reserved_lb, remained_lb, node_info->max_lb, node_info->used_lb);

			node_info->used_lb -= (rsv_node->reserved_lb - *reserve_lb);
			rsv_node->reserve_lb = *reserve_lb;
			rsv_node->reserved_lb = *reserve_lb;
			up(&node_info->sem);
			lbuf_mgr_update_requested_node_info(lb_info, hw_pool_id, hw_node_id, *reserve_lb);
			*reserve_lb = 0; // set to 0 for successfully stop traverse
			return;
		}

		remained_lb = node_info->max_lb - node_info->used_lb;
		need_reserved_lb = *reserve_lb - rsv_node->reserved_lb;
		if (need_reserved_lb <= remained_lb) {
			dpu_pr_debug("reserve node: succ-> reserve%u reserve_lb=%u need_reserved_lb=%u "
				"remained=%u: max_lb=%u used_lb=%u", node_index, *reserve_lb,
				need_reserved_lb, remained_lb, node_info->max_lb, node_info->used_lb);

			node_info->used_lb += need_reserved_lb;
			rsv_node->reserve_lb = *reserve_lb;
			rsv_node->reserved_lb = *reserve_lb;
			up(&node_info->sem);
			lbuf_mgr_update_requested_node_info(lb_info, hw_pool_id, hw_node_id, need_reserved_lb);
			*reserve_lb = 0; // set to 0 for successfully stop traverse
			return;
		}

		// current node lb size cannot satisfy request size, just use remained size and continue traverse
		node_info->used_lb = node_info->max_lb;
		rsv_node->reserve_lb += remained_lb;
		rsv_node->reserved_lb += remained_lb;
		*reserve_lb = need_reserved_lb - remained_lb;
		up(&node_info->sem);
		lbuf_mgr_update_requested_node_info(lb_info, hw_pool_id, hw_node_id, remained_lb);
	}

	// use last node to set reserve_lb greater than reserved_lb, make sure lbuf_mgr_request_node fail
	// cause reserve is more important than request
	hw_node_id = lb_conn_pool_cfg_info->hw_node_ids[lb_conn_pool_cfg_info->node_size - 1];
	node_info = &lbuf_mgr->lb_pool_info[hw_pool_id].node_info[hw_node_id];
	down(&node_info->sem);
	rsv_node = lbuf_mgr_get_reserved_lb_node(node_info, lb_info);
	if (!rsv_node) {
		dpu_pr_err("reserve node: skip reserve%u cause get reserve node fail", node_index);
		up(&node_info->sem);
		return;
	}

	rsv_node->reserve_lb += *reserve_lb;
	up(&node_info->sem);
}

/*
 * When user invoke reserve operation, lbuf_mgr will do these things to handle its demand
 * 1、create a new node of reserve type if there is none, or use a created one otherwise and get its
 * already reserved lb num
 * 2、determin used_lb and max_lb of current node, which is denoted as remained_lb, plus with reserved lb num,
 * can satisfy current reserve demand, remained_lb will dec size of not reserved lb size(reserve lb num
 * dec reserved lb num), and reserved lb num is set to reserve lb num
 * 3、if remained_lb, plus with reserved lb num, cannot satisfy, reserved lb num is to inc remained_lb
 * Note: each user invoke this function will overwrite its own reserve information, return 0 as succ,
 * return 1 means user should invoke at next time, and it reserves all lbuf_mgr can provide
 * return -1 means user input parameters is invalid
 */
static int32_t lbuf_mgr_reserve_lb(struct dpu_lbuf_mgr *lbuf_mgr, struct req_lbuf_info *lb_info)
{
	int32_t ret = 0;
	uint32_t req_conn_index;
	const struct lbuf_opr_conn_info *req_conn_info = NULL;
	const struct lbuf_conn_pool_node_config_info *conn_pnc_info = NULL;
	uint32_t conn_pool_size = 0;
	uint32_t conn_pool_cfg_idx = 0;
	uint32_t reserve_lb = 0;

	lb_info->node_num = 0;

	if (lbuf_mgr_check_scene_lb_num(lbuf_mgr, lb_info, true) != 0)
		return 1;

	for (req_conn_index = 0; req_conn_index < lb_info->conn_num; ++req_conn_index) {
		req_conn_info = &lb_info->opr_conn_info[req_conn_index];
		conn_pnc_info = dpu_get_lb_conn_pool_node_config_info(req_conn_info->cur_opr,
			req_conn_info->nxt_opr, &conn_pool_size);
		if (unlikely(!conn_pnc_info)) {
			dpu_pr_err("reserve node: stop reserve cause get connection pool node config fail");
			ret = -1;
			break;
		}

		reserve_lb = req_conn_info->used_lb;
		if (conn_pool_size == 1) { // means opr connect supports node with one pool id, such as SDMA0->OV0
			lbuf_mgr_reserve_node(lbuf_mgr, lb_info,
				&conn_pnc_info[0].lb_conn_pool_cfg_info[conn_pnc_info[0].conn_pool_index], &reserve_lb);
			if (reserve_lb > 0) {
				dpu_pr_warn("reserve node: stop reserve cur_opr=%#x nxt_opr=%#x used_lb=%u", req_conn_info->cur_opr,
					req_conn_info->nxt_opr, req_conn_info->used_lb);
				ret = -1;
				break;
			}
			continue;
		}

		// means opr connect supports node with several pool id, such as SROT0->VSCF1
		for (conn_pool_cfg_idx = 0; conn_pool_cfg_idx < conn_pool_size; ++conn_pool_cfg_idx)
			lbuf_mgr_reserve_node(lbuf_mgr, lb_info,
				&conn_pnc_info[conn_pool_cfg_idx]. \
				lb_conn_pool_cfg_info[conn_pnc_info[conn_pool_cfg_idx].conn_pool_index],
				&reserve_lb);
		if (reserve_lb > 0) {
			dpu_pr_warn("reserve node: stop reserve cur_opr=%#x nxt_opr=%#x used_lb=%u", req_conn_info->cur_opr,
				req_conn_info->nxt_opr, req_conn_info->used_lb);
			ret = -1;
			break;
		}
	}
	lbuf_mgr_clear_user_none_reserve_node_info(lbuf_mgr, lb_info);

	return ret;
}

/* Currently user invoke this for free all reserved information of given user_type
 */
static int32_t lbuf_mgr_free_reserved_lb(struct dpu_lbuf_mgr *lbuf_mgr, const struct req_lbuf_info *lb_info)
{
	lbuf_mgr_clear_user_reserve_node_info(lbuf_mgr, lb_info);
	return 0;
}

static int32_t lbuf_mgr_handle_lb_opt(struct dpu_lbuf_mgr *lbuf_mgr, void __user *argp)
{
	int32_t ret = -1;
	struct req_lbuf_info lb_info = {0};

	if (unlikely(copy_from_user(&lb_info, argp, sizeof(lb_info)) != 0)) {
		dpu_pr_err("copy from user fail");
		return -1;
	}

	if (unlikely(lb_info.user_type < 0 || lb_info.user_type >= SCENE_USER_MAX)) {
		dpu_pr_err("user_type=%d is invalid", lb_info.user_type);
		return -1;
	}

	if (unlikely(lb_info.scene_id < 0 || lb_info.scene_id >= DPU_SCENE_MAX)) {
		dpu_pr_err("invalid scene_id=%u", lb_info.scene_id);
		return -1;
	}

	if (unlikely(lb_info.node_num > LBUF_NODE_NUM_MAX)) {
		dpu_pr_err("node_num=%u is invalid", lb_info.node_num);
		return -1;
	}

	if (lb_info.req_scene_lb_num > dpu_get_pool_a_lb_max_size()) {
		dpu_pr_warn("user_type=%d scene_id=%u, req_scene_lb_num=%u is overflow %u",
			lb_info.user_type, lb_info.scene_id, lb_info.req_scene_lb_num, dpu_get_pool_a_lb_max_size());
		return -1;
	}

	if (unlikely(lb_info.conn_num > LBUF_CONNECT_NUM_MAX)) {
		dpu_pr_err("conn_num=%u is invalid", lb_info.conn_num);
		return -1;
	}

	switch (lb_info.opt) {
	case LB_REQUEST:
		ret = lbuf_mgr_request_lb(lbuf_mgr, &lb_info);
		break;
	case LB_RELEASE:
		return lbuf_mgr_release_lb(lbuf_mgr, &lb_info);
	case LB_RESERVE:
		ret = lbuf_mgr_reserve_lb(lbuf_mgr, &lb_info);
		break;
	case LB_FREE_RESERVED:
		return lbuf_mgr_free_reserved_lb(lbuf_mgr, &lb_info);
	default:
		dpu_pr_err("lbuf mgr unsupport opt = 0x%x", lb_info.opt);
		return -1;
	}

	if (ret)
		return -1;

	if (unlikely(copy_to_user(argp, &lb_info, sizeof(lb_info)) != 0)) {
		dpu_pr_err("copy to user fail");
		return -1;
	}

	return 0;
}

static long lbuf_mgr_ioctl(struct dpu_res_resouce_node *res_node, uint32_t cmd, void __user *argp)
{
	struct dpu_lbuf_mgr *lbuf_mgr = NULL;

	if (!res_node || !res_node->data || !argp) {
		dpu_pr_err("res_node or node data or argp is NULL");
		return -EINVAL;
	}

	lbuf_mgr = (struct dpu_lbuf_mgr*)res_node->data;
	switch (cmd) {
	case RES_HANDLE_LBUF_OPT:
		return lbuf_mgr_handle_lb_opt(lbuf_mgr, argp);
	default:
		dpu_pr_debug("lbuf mgr unsupport cmd, need processed by other module, cmd = 0x%x", cmd);
		return 1;
	}

	return 0;
}

void dpu_res_register_lbuf_mgr(struct list_head *resource_head)
{
	struct dpu_res_resouce_node *lbuf_mgr_node = kzalloc(sizeof(struct dpu_res_resouce_node), GFP_KERNEL);
	if (!lbuf_mgr_node)
		return;

	lbuf_mgr_node->res_type = RES_LBUF;
	atomic_set(&lbuf_mgr_node->res_ref_cnt, 0);

	lbuf_mgr_node->init = lbuf_mgr_init;
	lbuf_mgr_node->deinit = lbuf_mgr_deinit;
	lbuf_mgr_node->reset = lbuf_mgr_reset;
	lbuf_mgr_node->release = lbuf_mgr_release;
	lbuf_mgr_node->ioctl = lbuf_mgr_ioctl;

	list_add_tail(&lbuf_mgr_node->list_node, resource_head);
	dpu_pr_debug("dpu_res_register_lbuf_mgr success!");
}