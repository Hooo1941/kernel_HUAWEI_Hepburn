/*
 * GNU General Public License for more details.
 */

#ifndef __VENDOR_FS_INTERFACE_H__
#define __VENDOR_FS_INTERFACE_H__

extern struct filename *filename_parentat(int dfd, struct filename *name,
				unsigned int flags, struct path *parent,
				struct qstr *last, int *type);
extern struct dentry *__lookup_hash(const struct qstr *name,
		struct dentry *base, unsigned int flags);
extern long do_mkdirat(int dfd, const char __user *pathname, umode_t mode);
extern long do_rmdir(int dfd, struct filename *name);
extern long do_unlinkat(int dfd, struct filename *name);
extern int may_linkat(struct path *link);

long vendor_access(const char *filename, int mode);
long vendor_mkdir(const char *pathname, umode_t mode);
long vendor_rmdir(const char *pathname);
long vendor_link(const char *oldname, const char *newname);
long vendor_unlink(const char *pathname);
long vendor_symlink(const char *oldname, const char *newname);
long vendor_getdents64(struct file *file, struct linux_dirent64 *dirent, unsigned int count);
long vendor_rename(const char *oldname, const char *newname);
long vendor_stat(const char *filename, struct kstat *stat);
#endif
