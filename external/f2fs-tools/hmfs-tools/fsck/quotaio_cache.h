/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * quotaio_cache.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _QUOTAIO_CACHE_H_
#define _QUOTAIO_CACHE_H_

#include "f2fs_fs.h"
#include "fsck.h"
#include "node.h"

struct data_cache {
	char *blk_buffer;
	nid_t nid;
	unsigned int ofs_in_node;
	block_t data_blkaddr;
	pgoff_t index;
	int dirty;
};

struct node_cache {
	struct f2fs_node *blk_buffer;
	nid_t nid;
	block_t node_blkaddr;
	int dirty;
};

struct tree_node {
	struct node_cache *nc;
	struct tree_node *left;
	struct tree_node *right;
};

struct data_list {
	struct data_cache *dc;
	struct data_list *next;
};

void hmfs_quota_cache_init(nid_t ino);
void hmfs_quota_cache_clean();
void hmfs_quota_cache_write_back(struct f2fs_sb_info *sbi);
int hmfs_grab_node_from_cache(struct f2fs_node *buf, nid_t nid, block_t blk_addr);
int hmfs_write_node_to_cache(struct f2fs_node *node_blk, block_t blk_addr);
void hmfs_grab_data_from_cache(void *blk_buf, pgoff_t index, block_t blk_addr);
void hmfs_write_data_to_cache(char *blk_buf, pgoff_t index,
			      struct dnode_of_data *dn, block_t blk_addr);
int flush_curseg_cached_blocks(struct f2fs_sb_info *sbi, int type);

#endif /* _QUOTAIO_CACHE_H_ */
