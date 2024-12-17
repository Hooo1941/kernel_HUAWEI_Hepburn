/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_SC_API_H
#define HVGR_SC_API_H

#include <linux/types.h>
#include "hvgr_defs.h"

int hvgr_sc_init_policy_list(struct hvgr_device * const gdev, struct list_head *head);

uint32_t hvgr_sc_id_to_policy_id(uint32_t idx);

uint32_t hvgr_get_policy_id(uint64_t attr);

void hvgr_sc_update_map_table(uint64_t attr);

long hvgr_sc_set_gid_policy(uint32_t gid, uint32_t attr);

void hvgr_sc_enable_gids(struct hvgr_device * const gdev);

void hvgr_v_sc_set_page_osh(struct page *page, uint32_t osh);

#endif
