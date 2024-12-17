/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * dedup.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _DEDUP_H_
#define _DEDUP_H_

#include "f2fs_fs.h"
#include "fsck.h"

#define F2FS_DEDUPED_FL                 0x00000001
#define F2FS_INNER_FL                   0x00000002
#define F2FS_REVOKE_FL                  0x00000008
#define F2FS_DOING_DEDUP_FL             0x00000010

struct dedup_inner_node {
	u32 nid;
	u32 links;
	u32 actual_links;
	bool is_valid;
	struct dedup_inner_node *next;
};

bool f2fs_is_deduped_inode(struct f2fs_node *node_blk);
bool f2fs_is_inner_inode(struct f2fs_node *node_blk);
bool f2fs_is_out_inode(struct f2fs_node *node_blk);
bool f2fs_is_unstable_dedup_inode(struct f2fs_node *node_blk);
nid_t f2fs_get_dedup_inner_ino(struct f2fs_node *node_blk);
bool f2fs_sanity_check_dedup_inner_nid(struct f2fs_sb_info *sbi,
						nid_t inner_ino);
void f2fs_inc_inner_actual_links(struct f2fs_sb_info *sbi, nid_t inner_ino);
void f2fs_fix_dedup_inner_list(struct f2fs_sb_info *sbi);
void f2fs_check_dedup_extent_info(struct child_info *child);
#endif /* _DEDUP_H_ */
