/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DATAN_CT_CONFIG_H
#define HVGR_DATAN_CT_CONFIG_H

#include <linux/types.h>

struct hvgr_ct_setup {
	u64 dump_buffer;
	u32 fcm_bm;
	u32 mmu_l2_bm;
	u32 btc_bm;
	u32 bvh_bm;
	u32 gpc_bm;
	u32 set_id;
};

#endif // HVGR_DATAN_CT_CONFIG_H
