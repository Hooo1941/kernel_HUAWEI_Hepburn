// SPDX-License-Identifier: GPL-2.0+
/*
 * erofs_utils/lib/inode.c
 *
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Li Guifu <bluce.liguifu@huawei.com>
 * with heavy changes by Gao Xiang <gaoxiang25@huawei.com>
 */
#define _GNU_SOURCE
#include "erofs/patch.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include "erofs/config.h"
#include "erofs/print.h"
#include "erofs/cache.h"
#include "erofs/inode.h"
#include "erofs/compress.h"
#include "erofs/map.h"
#if PATCH_ENABLED
#include "erofs/xattr.h"
#endif

static struct erofs_inode *get_root_inode(const char *file_path,
					  struct erofs_inode *root_inode)
{
	char org_path[PATH_MAX + 1] = {0};
	char path[PATH_MAX + 1] = {0};
	int ret;
	struct stat64 st;

	if (!root_inode || !file_path)
		return NULL;

	ret = snprintf(path, PATH_MAX, "%s",
		       file_path + strlen(cfg.c_patch_path));
	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf error file[%s] ret[%d]", file_path, ret);
		return NULL;
	}

	ret = snprintf(org_path, PATH_MAX, "%s%s",
		       cfg.c_src_path, path);
	if (ret < 0 || ret >= PATH_MAX) {
		erofs_err("snprintf error file[%s] ret[%d]", file_path, ret);
		return NULL;
	}

	ret = lstat64(org_path, &st);
	if (ret)
		return NULL;

	return erofs_iget(st.st_dev, st.st_ino);
}

#if !PATCH_ENABLED
static int erofs_mkfs_patch_xattr(struct erofs_inode *inode,
				  struct erofs_inode *org_inode)
{
	if (!inode || !org_inode)
		return -1;

	if (org_inode->i_xattr_scnt == 0)
		return 0;

	inode->xattr = malloc(org_inode->xattr_isize);
	if (!inode->xattr)
		return -1;
	memset(inode->xattr, 0, org_inode->xattr_isize);
	inode->i_xattr_scnt = org_inode->i_xattr_scnt;
	inode->xattr_isize = org_inode->xattr_isize;
	memcpy(inode->xattr, org_inode->xattr, org_inode->xattr_isize);
	inode->i_mode = org_inode->i_mode;
	inode->i_uid  = org_inode->i_uid;
	inode->i_gid  = org_inode->i_gid;
	return 0;
}
#endif

static int update_dentry_patch(struct erofs_inode *inode,
			       struct erofs_inode *org_inode)
{
	struct erofs_dentry *d;

	if (!inode || !org_inode)
		return -1;
	/* the dentry will be updated when updating */
	/* the dentry information */
	erofs_nid_t nid = erofs_lookupnid(inode);
	list_for_each_entry(d, &org_inode->i_parent->i_subdirs, d_child) {
		erofs_info("patch update_dentry [%s][%d][%d]", d->name, d->nid, org_inode->nid);
		if (d->nid == org_inode->nid)
			d->nid = nid;
	}
	erofs_write_dir_file(org_inode->i_parent);
	return 0;
}

int erofs_mkfs_build_tree_patch_file(const char *path,
				     struct erofs_inode *root_inode)
{
	struct erofs_inode *org_inode = NULL;
	struct erofs_inode *inode = NULL;
#if PATCH_ENABLED
	int ret;
#endif

	if (!path || !root_inode)
		return -1;

	inode = erofs_iget_from_path(path, true);
	if (IS_ERR(inode))
		return 0;
	org_inode = get_root_inode(path, root_inode);
	if (!org_inode) {
		erofs_err("%s get root inode err", path);
		return -1;
	}

#if PATCH_ENABLED
	ret = erofs_prepare_xattr_ibody(inode, org_inode);
	if (ret < 0)
		return ret;
#endif

#if !PATCH_ENABLED
	erofs_mkfs_patch_xattr(inode, org_inode);
#endif
	erofs_write_file(inode, org_inode);

	erofs_prepare_inode_buffer(inode);
	if (!is_inode_layout_compression(inode))
		erofs_output_map_blocks(inode->i_srcpath, inode->u.i_blkaddr,
					inode->u.i_blocks);
	erofs_write_tail_end(inode);
	update_dentry_patch(inode, org_inode);

	/* release inode */
	erofs_iput(org_inode); /* get at get_root_inode */
	erofs_iput(inode);
	return 0;
}


int erofs_mkfs_build_tree_patch(const char *path,
				struct erofs_inode *root_inode)
{
	int ret = 0;
	DIR *_dir = NULL;
	struct dirent *dp = NULL;
	char file_path[PATH_MAX + 1] = {0};
	struct stat64 s;

	if (!path || !root_inode)
		return -1;

	if (lstat64(path, &s) == 0) {
		if (S_ISREG(s.st_mode)) {
			erofs_err("[%s] is a regular file", path);
			return -1;
		}
	} else {
		erofs_err("stat failed [%s]", path);
		return -1;
	}

	_dir = opendir(path);
	if (!_dir) {
		erofs_err("dp is NULL dir=%s", path);
		return -1;
	}
	while ((dp = readdir(_dir)) != NULL) {
		if (!strncmp(dp->d_name, ".", strlen(".")) ||
		    !strncmp(dp->d_name, "..", strlen("..")) ||
		    !strncmp(dp->d_name, "lost+found", strlen("lost+found")))
			continue;

		memset(file_path, 0,  sizeof(file_path));
		ret = snprintf(file_path, PATH_MAX, "%s/%s", path, dp->d_name);
		if (ret < 0 || ret >= PATH_MAX) {
			erofs_err("snprintf [%s] error [%d]", file_path, ret);
			ret = -1;
			break;
		}

		if (lstat64(file_path, &s) != 0) {
			erofs_err("[%s] stat err", file_path);
			ret = -1;
			break;
		}

		if (S_ISREG(s.st_mode) && !S_ISLNK(s.st_mode))
			ret = erofs_mkfs_build_tree_patch_file(file_path, root_inode);
		else if (S_ISDIR(s.st_mode))
			ret = erofs_mkfs_build_tree_patch(file_path, root_inode);

		if (ret)
			break;
	}

	closedir(_dir);
	return ret;
}

