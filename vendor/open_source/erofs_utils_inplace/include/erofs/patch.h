// SPDX-License-Identifier: GPL-2.0+
/*
 * erofs_utils/lib/inode.c
 *
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 * with heavy changes by Gao Xiang <gaoxiang25@huawei.com>
 */
#ifndef CONFIG_EROFS_MKFS_PATCH_H
#define CONFIG_EROFS_MKFS_PATCH_H
#include "erofs/internal.h"

int erofs_mkfs_build_tree_patch(const char *path,
				struct erofs_inode *root_inode);

#endif

