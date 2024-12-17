/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold remap header file.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author:	Li Peng <lipeng113@huawei.com>
 *
 * Create: 2023-3-3
 *
 */

#ifndef __HYPERHOLD_REMAP_H__
#define __HYPERHOLD_REMAP_H__
#include "zram_drv.h"
#include "hyperhold_internal.h"
#include "hyperhold_area.h"

#define HH_ZONE_SHIFT 	21
#define HH_ZONE_SIZE 	(1 << HH_ZONE_SHIFT)
#define HH_ZONE_EXT_CNT (1 << (HH_ZONE_SHIFT - EXTENT_SHIFT))

struct hh_zone {
	struct list_head list;
	unsigned long bitmap;
	u32 zone_off;
	u32 bit_max;
	atomic_t last_alloc_bit;
};

struct remap_map {
	struct list_head list;
	int src_bit;
	int dst_bit;
};

struct remap_io {
	struct block_device *bdev;
	struct hyperhold_area *area;
	struct list_head pair_list;
	int cnt;
	struct stor_dev_data_remap_info info;
};

int alloc_hyperhold_bitmap_zone(struct hyperhold_area *area);
bool test_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id);
bool test_and_set_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id);
bool test_and_clear_bit_in_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id);
int alloc_bitmap_with_zones(struct hyperhold_area *area, bool discard);
void mayfree_zone_bitmap_by_extent(struct hyperhold_area *area, int ext_id);
int remap_bitmap_with_zones(struct hyperhold_area *area, int require_seqsize_in_zone);
#endif
