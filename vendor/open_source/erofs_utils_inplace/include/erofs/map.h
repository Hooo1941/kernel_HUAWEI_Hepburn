// SPDX-License-Identifier: GPL-2.0+
/*
 * map.h
 *
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 */


#ifndef __EROFS_MAP_H
#define __EROFS_MAP_H
#include "erofs/internal.h"

#define MAP_BIN_VERSION_BLK_ADDR    0x00

int erofs_map_file_init(const char *file);
void erofs_map_file_close(void);
int erofs_output_map_blocks(const char *file, u32 base, u32 blocks);
int erofs_map_save_compr_algo_ver(unsigned int version, unsigned int blk_addr);
int erofs_output_mapbin_block(u8 fmode, u8 iscompr, u32 addr, u64 size);
int erofs_output_mapbin_blocks(u8 fmode, u8 iscompr, u32 base_blk, u64 size);

#endif

