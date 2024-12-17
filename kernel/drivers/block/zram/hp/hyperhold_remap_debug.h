/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: hyperhold remap debug header file.
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

#ifndef __HYPERHOLD_REMAP_DEBUG_H__
#define __HYPERHOLD_REMAP_DEBUG_H__
#include "zram_drv.h"
#include "hyperhold_internal.h"
#include "hyperhold_area.h"

#define REMAP_DEBUG_SIZE (512 * 1024 * 1024)
#define REMAP_DEBUG_MAGIC_SHIFT 16
#define REMAP_DEBUG_CRC_START 0
#define REMAP_DEBUG_MAGIC_START 4
#define REMAP_DEBUG_MAGIC_SIZE 4092

struct remap_debug_io {
	int ext_id;
	int page_cnt;
	struct page *page[EXTENT_PG_CNT];
};

void hh_remap_stub_write(int ext_id);
void hh_remap_stub_read(int ext_id);
void hh_remap_stub_discrete(void);

extern int g_hh_remap_step;
#endif
