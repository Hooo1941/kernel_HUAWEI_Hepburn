// SPDX-License-Identifier: GPL-2.0
/*
 * Routines that mimic syscalls, but don't use the user address space or file
 * descriptors.  Only for init/ and related early init code.
 */
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/fs_struct.h>
#include <linux/file.h>
#include <linux/init_syscalls.h>
#include <linux/security.h>
#include <linux/fsnotify.h>
#include <linux/dirent.h>
#include <linux/mount.h>

#include <internal.h>
#include <platform_include/basicplatform/linux/fs/vendor_fs_interface.h>

long vendor_access(const char *filename, int mode)
{
	struct path path;
	int error;

	error = kern_path(filename, LOOKUP_FOLLOW, &path);
	if (error)
		return error;
	error = inode_permission(d_inode(path.dentry), mode);
	path_put(&path);
	return error;
}
EXPORT_SYMBOL(vendor_access);

long vendor_mkdir(const char *pathname, umode_t mode)
{
	struct dentry *dentry;
	struct path path;
	int error;

	dentry = kern_path_create(AT_FDCWD, pathname, &path, LOOKUP_DIRECTORY);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);
	if (!IS_POSIXACL(path.dentry->d_inode))
		mode &= ~current_umask();
	error = security_path_mkdir(&path, dentry, mode);
	if (!error)
		error = vfs_mkdir(path.dentry->d_inode, dentry, mode);
	done_path_create(&path, dentry);
	return error;
}
EXPORT_SYMBOL(vendor_mkdir);

long vendor_rmdir(const char *pathname)
{
	return do_rmdir(AT_FDCWD, getname_kernel(pathname));
}
EXPORT_SYMBOL(vendor_rmdir);

long vendor_link(const char *oldname, const char *newname)
{
	struct dentry *new_dentry = NULL;
	struct path old_path, new_path;
	int error;

	error = kern_path(oldname, 0, &old_path);
	if (error)
		return error;

	new_dentry = kern_path_create(AT_FDCWD, newname, &new_path, 0);
	error = PTR_ERR(new_dentry);
	if (IS_ERR(new_dentry))
		goto out;

	error = -EXDEV;
	if (old_path.mnt != new_path.mnt)
		goto out_dput;
	error = may_linkat(&old_path);
	if (unlikely(error))
		goto out_dput;
	error = security_path_link(old_path.dentry, &new_path, new_dentry);
	if (error)
		goto out_dput;
	error = vfs_link(old_path.dentry, new_path.dentry->d_inode, new_dentry,
			 NULL);
out_dput:
	done_path_create(&new_path, new_dentry);
out:
	path_put(&old_path);
	return error;
}
EXPORT_SYMBOL(vendor_link);

long vendor_symlink(const char *oldname, const char *newname)
{
	struct dentry *dentry;
	struct path path;
	int error;

	dentry = kern_path_create(AT_FDCWD, newname, &path, 0);
	if (IS_ERR(dentry))
		return PTR_ERR(dentry);
	error = security_path_symlink(&path, dentry, oldname);
	if (!error)
		error = vfs_symlink(path.dentry->d_inode, dentry, oldname);
	done_path_create(&path, dentry);
	return error;
}
EXPORT_SYMBOL(vendor_symlink);

long vendor_unlink(const char *pathname)
{
	return do_unlinkat(AT_FDCWD, getname_kernel(pathname));
}
EXPORT_SYMBOL(vendor_unlink);

static int verify_dirent_name(const char *name, int len)
{
	if (len <= 0 || len >= PATH_MAX)
		return -EIO;
	if (memchr(name, '/', len))
	   return -EIO;
	return 0;
}

struct getdents_callback64 {
	struct dir_context ctx;
	struct linux_dirent64 * current_dir;
	int prev_reclen;
	int count;
	int error;
};

static int filldir64(struct dir_context *ctx, const char *name, int namlen,
		loff_t offset, u64 ino, unsigned int d_type)
{
	struct linux_dirent64 *dirent = NULL, *prev = NULL;
	struct getdents_callback64 *buf = container_of(ctx, struct getdents_callback64, ctx);
	int reclen = ALIGN(offsetof(struct linux_dirent64, d_name) + namlen + 1, sizeof(u64));
	int prev_reclen;

	buf->error = verify_dirent_name(name, namlen);
	if (unlikely(buf->error))
	   return buf->error;
	buf->error = -EINVAL;   /* only used if we fail.. */
	if (reclen > buf->count)
	  return -EINVAL;

	prev_reclen = buf->prev_reclen;
	dirent = buf->current_dir;
	prev = (void *)dirent - prev_reclen;

	/* This might be 'dirent->d_off', but if so it will get overwritten */
	prev->d_off = offset;
	dirent->d_ino = ino;
	dirent->d_reclen = reclen;
	dirent->d_type = d_type;
	memcpy((void *)dirent->d_name, (void *)name, namlen);
	dirent->d_name[namlen] = '\0';

	buf->prev_reclen = reclen;
	buf->current_dir = (void  *)dirent + reclen;
	buf->count -= reclen;
	return 0;
}

long vendor_getdents64(struct file *file, struct linux_dirent64 *dirent, unsigned int count)
{
	struct getdents_callback64 buf = {
		.ctx.actor = filldir64,
		.count = count,
		.current_dir = (struct linux_dirent64 *) dirent
	};
	int error;

	error = iterate_dir(file, &buf.ctx);
	if (error >= 0)
	   error = buf.error;
	if (buf.prev_reclen) {
		struct linux_dirent64  *lastdirent = NULL;
		typeof(lastdirent->d_off) d_off = buf.ctx.pos;

		lastdirent = (void *) buf.current_dir - buf.prev_reclen;
		lastdirent->d_off = d_off;
		error = count - buf.count;
	}
	return error;
}
EXPORT_SYMBOL(vendor_getdents64);


static int do_renameat_kernel(int olddfd, const char *oldname, int newdfd,
			const char *newname, unsigned int flags)
{
	struct dentry *old_dentry = NULL, *new_dentry = NULL;
	struct dentry *trap = NULL;
	struct path old_path, new_path;
	struct qstr old_last, new_last;
	int old_type, new_type;
	struct inode *delegated_inode = NULL;
	struct filename *from = NULL;
	struct filename *to = NULL;
	unsigned int lookup_flags = 0, target_flags = LOOKUP_RENAME_TARGET;
	bool should_retry = false;
	int error;

	if (flags & ~(RENAME_NOREPLACE | RENAME_EXCHANGE | RENAME_WHITEOUT))
		return -EINVAL;

	if ((flags & (RENAME_NOREPLACE | RENAME_WHITEOUT)) &&
	    (flags & RENAME_EXCHANGE))
		return -EINVAL;

	if (flags & RENAME_EXCHANGE)
		target_flags = 0;

retry:
	from = filename_parentat(olddfd, getname_kernel(oldname), lookup_flags,
				&old_path, &old_last, &old_type);
	if (IS_ERR(from)) {
		error = PTR_ERR(from);
		goto exit;
	}

	to = filename_parentat(newdfd, getname_kernel(newname), lookup_flags,
				&new_path, &new_last, &new_type);
	if (IS_ERR(to)) {
		error = PTR_ERR(to);
		goto exit1;
	}

	error = -EXDEV;
	if (old_path.mnt != new_path.mnt)
		goto exit2;

	error = -EBUSY;
	if (old_type != LAST_NORM)
		goto exit2;

	if (flags & RENAME_NOREPLACE)
		error = -EEXIST;
	if (new_type != LAST_NORM)
		goto exit2;

	error = mnt_want_write(old_path.mnt);
	if (error)
		goto exit2;

retry_deleg:
	trap = lock_rename(new_path.dentry, old_path.dentry);

	old_dentry = __lookup_hash(&old_last, old_path.dentry, lookup_flags);
	error = PTR_ERR(old_dentry);
	if (IS_ERR(old_dentry))
		goto exit3;
	/* source must exist */
	error = -ENOENT;
	if (d_is_negative(old_dentry))
		goto exit4;
	new_dentry = __lookup_hash(&new_last, new_path.dentry, lookup_flags | target_flags);
	error = PTR_ERR(new_dentry);
	if (IS_ERR(new_dentry))
		goto exit4;
	error = -EEXIST;
	if ((flags & RENAME_NOREPLACE) && d_is_positive(new_dentry))
		goto exit5;
	if (flags & RENAME_EXCHANGE) {
		error = -ENOENT;
		if (d_is_negative(new_dentry))
			goto exit5;

		if (!d_is_dir(new_dentry)) {
			error = -ENOTDIR;
			if (new_last.name[new_last.len])
				goto exit5;
		}
	}
	/* unless the source is a directory trailing slashes give -ENOTDIR */
	if (!d_is_dir(old_dentry)) {
		error = -ENOTDIR;
		if (old_last.name[old_last.len])
			goto exit5;
		if (!(flags & RENAME_EXCHANGE) && new_last.name[new_last.len])
			goto exit5;
	}
	/* source should not be ancestor of target */
	error = -EINVAL;
	if (old_dentry == trap)
		goto exit5;
	/* target should not be an ancestor of source */
	if (!(flags & RENAME_EXCHANGE))
		error = -ENOTEMPTY;
	if (new_dentry == trap)
		goto exit5;

	error = security_path_rename(&old_path, old_dentry,
				     &new_path, new_dentry, flags);
	if (error)
		goto exit5;
	error = vfs_rename(old_path.dentry->d_inode, old_dentry,
			   new_path.dentry->d_inode, new_dentry,
			   &delegated_inode, flags);
exit5:
	dput(new_dentry);
exit4:
	dput(old_dentry);
exit3:
	unlock_rename(new_path.dentry, old_path.dentry);
	if (delegated_inode) {
		error = break_deleg_wait(&delegated_inode);
		if (!error)
			goto retry_deleg;
	}
	mnt_drop_write(old_path.mnt);
exit2:
	if (retry_estale(error, lookup_flags))
		should_retry = true;
	path_put(&new_path);
	putname(to);
exit1:
	path_put(&old_path);
	putname(from);
	if (should_retry) {
		should_retry = false;
		lookup_flags |= LOOKUP_REVAL;
		goto retry;
	}
exit:
	return error;
}

long vendor_rename(const char *oldname, const char *newname)
{
	return do_renameat_kernel(AT_FDCWD, oldname, AT_FDCWD, newname, 0);
}
EXPORT_SYMBOL(vendor_rename);

long vendor_stat(const char *filename, struct kstat *stat)
{
	int lookup_flags = (LOOKUP_FOLLOW & AT_SYMLINK_NOFOLLOW);
	struct path path;
	int error;

	error = kern_path(filename, lookup_flags, &path);
	if (error)
		return error;
	error = vfs_getattr(&path, stat, STATX_BASIC_STATS,
			    LOOKUP_FOLLOW | AT_NO_AUTOMOUNT);
	path_put(&path);
	return error;
}
EXPORT_SYMBOL(vendor_stat);
