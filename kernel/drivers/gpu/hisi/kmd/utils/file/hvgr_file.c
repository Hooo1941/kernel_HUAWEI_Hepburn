/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_file.h"

#include <linux/statfs.h>
#include <linux/namei.h>
#include <securec.h>
#include <linux/security.h>
#include <linux/version.h>
#if defined(CONFIG_LIBLINUX) && !defined(CONFIG_LIBLINUX_CDC)
#include <linux/syscalls.h>
#endif

struct hvgr_info_readdir_callback {
	struct dir_context ctx;
	int result;
	int is_dir;
	char file_name[MAX_FILE_NAME_SIZE];
};

int hvgr_file_delete_file(const char *file)
{
	int error;
	struct dentry *dentry = NULL;
	struct inode *pinode = NULL;
	struct inode *delegated_inode = NULL;
	struct path path;

	if (unlikely(file == NULL))
		/* No such file or directory */
		return -EINVAL;

#if defined(CONFIG_LIBLINUX) && !defined(CONFIG_LIBLINUX_CDC)
	return ksys_unlink(file);
#endif
	/* find the parent dentry of the file */
	error = kern_path(file, LOOKUP_FOLLOW, &path);
	if (error)
		return error;

	pinode = path.dentry->d_parent->d_inode;
	if (pinode == NULL) {
		path_put(&path);
		return -EINVAL;
	}

	/* find the file dentry in parent dentry */
	down_write(&(pinode)->i_rwsem);
	dentry = lookup_one_len(path.dentry->d_name.name,
		path.dentry->d_parent, strlen(path.dentry->d_name.name));
	if (dentry == NULL) {
		error = -EINVAL;
		goto exit;
	}

	/* delete the file dentry from the parent dentry */
	error = vfs_unlink(pinode, dentry, &delegated_inode);
	dput(dentry);
exit:
	up_write(&(pinode)->i_rwsem);
	path_put(&path);
	return error;
}

unsigned long long hvgr_file_get_free_space(const char *dir)
{
#ifdef CONFIG_LIBLINUX
	return U64_MAX;
#else
	struct kstatfs disk_info;
	struct path partition_path;
	int err;
	unsigned long long free_size;

	if (unlikely(dir == NULL))
		return 0;

	err = kern_path(dir, LOOKUP_FOLLOW | LOOKUP_DIRECTORY, &partition_path);
	if (err)
		return 0;

	if (vfs_statfs(&partition_path, &disk_info) < 0)
		return 0;

	free_size = disk_info.f_bfree * (unsigned long)disk_info.f_bsize;

	return free_size;
#endif
}

static int hvgr_info_filldir(struct dir_context *ctx, const char *name,
	int namelen, loff_t offset, u64 ino, unsigned int d_type)
{
	struct hvgr_info_readdir_callback *fill_dir = container_of(ctx,
		struct hvgr_info_readdir_callback, ctx);

	if (fill_dir->result)
		return -EINVAL;

	if ((unsigned int)namelen >= MAX_FILE_NAME_SIZE) {
		fill_dir->result = -ENOMEM;
		return -ENOMEM;
	}
	fill_dir->result++;
	fill_dir->is_dir = (int)d_type;
	(void)strncpy_s(fill_dir->file_name, MAX_FILE_NAME_SIZE, name, (unsigned int)namelen);

	return 0;
}

static int hvgr_read_dir(struct file *file, int *is_dir, char *file_name)
{
	int ret;
	size_t len;

	struct hvgr_info_readdir_callback fill_dir = {
		.ctx.actor = hvgr_info_filldir,
	};
	(void)memset_s(fill_dir.file_name, MAX_FILE_NAME_SIZE, 0, MAX_FILE_NAME_SIZE);

	if (file == NULL)
		return -EBADF;
	ret = iterate_dir(file, &fill_dir.ctx);
	if (fill_dir.result)
		ret = fill_dir.result;
	*is_dir = fill_dir.is_dir;
	len = strlen(fill_dir.file_name) + 1;

	(void)strncpy_s(file_name, MAX_FILE_NAME_SIZE, fill_dir.file_name, len);
	return ret;
}

int hvgr_file_get_file_num(const char *dir, const char *file_prefix)
{
	int count = 0;
	unsigned int is_dir = 0;
	struct file *file = NULL;
	char file_name[MAX_FILE_NAME_SIZE] = {0};

	if (unlikely((dir == NULL) || (file_prefix == NULL)))
		return -EINVAL;

	file = filp_open(dir, O_RDONLY, 0);
	if (IS_ERR_OR_NULL(file))
		return -ENOENT;

	while (hvgr_read_dir(file, &is_dir, file_name) > 0) {
		if ((is_dir == DT_DIR) || (!strcmp(file_name, ".") || (!strcmp(file_name, ".."))))
			continue;

		if (!strncmp(file_name, file_prefix, strlen(file_prefix)))
			count++;
	}

	return count;
}

#if !defined(CONFIG_LIBLINUX) || defined(CONFIG_LIBLINUX_CDC)
static int hvgr_mkdir(const char *name, umode_t mode)
{
	struct dentry *dentry = NULL;
	struct path path;
	int error = 0;
	unsigned int lookup_flags = LOOKUP_DIRECTORY;
	bool retry = false;

	do {
		retry = false;
		dentry = kern_path_create(AT_FDCWD, name, &path, lookup_flags);
		if (IS_ERR(dentry))
			return PTR_ERR(dentry);

		if (!IS_POSIXACL(path.dentry->d_inode))
			mode &= ~(umode_t)current_umask();

		error = security_path_mkdir(&path, dentry, mode);
		if (!error)
			error = vfs_mkdir(path.dentry->d_inode, dentry, mode);

		done_path_create(&path, dentry);
		if (retry_estale(error, lookup_flags)) {
			lookup_flags |= LOOKUP_REVAL;
			retry = true;
		}
	} while (retry);

	return error;
}
#endif

bool hvgr_file_make_dir(const char *dir, umode_t mode)
{
	struct file *dir_file = NULL;

	if (unlikely(dir == NULL))
		return false;

	dir_file = filp_open(dir, O_RDONLY, 0);
	if (!IS_ERR(dir_file)) {
		(void)filp_close(dir_file, NULL);
		return true;
	}

#if defined(CONFIG_LIBLINUX) && !defined(CONFIG_LIBLINUX_CDC)
	if (ksys_mkdir(dir, mode) == 0)
		return true;
#else
	if (hvgr_mkdir(dir, mode) == 0)
		return true;
#endif
	return false;
}

bool hvgr_file_write_to_file(struct file *data_file, char *buffer, size_t buf_len, loff_t *pos)
{
	if (!IS_ERR(data_file) && buffer != NULL)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		return buf_len == (size_t)vfs_write(data_file, buffer, buf_len, pos);
#else
		return buf_len == (size_t)kernel_write(data_file, buffer, buf_len, pos);
#endif

	return false;
}

