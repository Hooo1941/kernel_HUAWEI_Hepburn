/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * erofs-utils/include/erofs/inode.h
 *
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 * with heavy changes by Gao Xiang <gaoxiang25@huawei.com>
 */
#ifndef __EROFS_INODE_H
#define __EROFS_INODE_H

#include "erofs/internal.h"

void erofs_inode_manager_init(void);
unsigned int erofs_iput(struct erofs_inode *inode);
erofs_nid_t erofs_lookupnid(struct erofs_inode *inode);
struct erofs_inode *erofs_mkfs_build_tree_from_path(struct erofs_inode *parent,
						    const char *path);

#ifdef EROFS_UTILS121
int erofs_write_file(struct erofs_inode *inode, struct erofs_inode *org_inode);
struct erofs_inode *erofs_iget_from_path(const char *path, bool is_src);
int erofs_prepare_inode_buffer(struct erofs_inode *inode);
int erofs_write_tail_end(struct erofs_inode *inode);
__attribute__((unused)) static int __allocate_inode_bh_data(struct erofs_inode *inode,
			     unsigned long nblocks);
int erofs_write_dir_file(struct erofs_inode *dir);
struct erofs_inode *erofs_iget(dev_t dev, ino_t ino);
#endif

#endif
