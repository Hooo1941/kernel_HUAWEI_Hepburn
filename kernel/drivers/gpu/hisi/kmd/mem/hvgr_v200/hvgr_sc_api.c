/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_sc_api.h"

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#include <linux/hisi/mm_lb.h>
#else
#include <linux/mm_lb/mm_lb.h>
#endif

#include "hvgr_defs.h"
#include "hvgr_regmap.h"
#include "hvgr_ioctl_mem.h"
#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"

#define SC_GID_NUM 3u
#define SC_IDX_NUM 16u

static const uint32_t sc_policy_idx_to_id_remap[SC_IDX_NUM] =
	{0, 2, 2, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 0, 0, 0};

int hvgr_sc_init_policy_list(struct hvgr_device * const gdev, struct list_head *head)
{
	int ret;
	uint32_t sc_gids[SC_GID_NUM] = {0};
	uint32_t i;
	struct hvgr_sc_policy_info *info = NULL;

	ret = of_property_read_u32_array(gdev->dev->of_node, "sc-gids", sc_gids, SC_GID_NUM);
	if (ret) {
		hvgr_err(gdev, HVGR_MEM, "There is no sc-gids cfg");
		return -EINVAL;
	}

	for (i = 0; i < SC_GID_NUM; i++) {
		info = kzalloc(sizeof(*info), GFP_KERNEL);
		if (info == NULL)
			goto failed;
		INIT_LIST_HEAD(&info->node);

		info->policy_id = sc_gids[i];
		info->sc_gid = sc_gids[i];
		hvgr_err(gdev, HVGR_MEM,
			"sc policy_id %u sc_gid %u cache_policy 0x%lx cache_quota 0x%lx",
			info->policy_id, info->sc_gid, info->cache_policy, info->cache_quota);

		list_add(&info->node, head);
	}

	return 0;
failed:
	hvgr_err(gdev, HVGR_MEM, "init policy list fail");
	return -EINVAL;
}

uint32_t hvgr_sc_id_to_policy_id(uint32_t idx)
{
	if (idx >= SC_IDX_NUM)
		return 0;

	return sc_policy_idx_to_id_remap[idx];
}

uint32_t hvgr_get_policy_id(uint64_t attr)
{
	return hvgr_sc_id_to_policy_id(hvgr_mem_get_sc_id(attr));
}

void hvgr_sc_update_map_table(uint64_t attr)
{
	return;
}

long hvgr_sc_set_gid_policy(uint32_t gid, uint32_t attr)
{
	return 0;
}

void hvgr_sc_enable_gids(struct hvgr_device * const gdev)
{
	return;
}

void hvgr_v_sc_set_page_osh(struct page *page, uint32_t osh)
{
	return;
}

