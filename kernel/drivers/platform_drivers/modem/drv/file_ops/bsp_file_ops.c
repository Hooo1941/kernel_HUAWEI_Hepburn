/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/namei.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/version.h>

#include <bsp_file_ops.h>


int bsp_symlink(const char *oldname, const char *newname)
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

int mdrv_symlink(const char *old_name, const char *new_name)
{
    return bsp_symlink(old_name, new_name);
}
EXPORT_SYMBOL(mdrv_symlink);

int bsp_file_mkdir(const char *pathname, umode_t mode)
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

int mdrv_mkdir(const char *path_name, umode_t mode)
{
    return bsp_file_mkdir(path_name, mode);
}
EXPORT_SYMBOL(mdrv_mkdir);

int bsp_eaccess(const char *filename)
{
    struct path path;
    int error;

    error = kern_path(filename, LOOKUP_FOLLOW, &path);
    if (error)
            return error;
    error = inode_permission(d_inode(path.dentry), MAY_ACCESS);
    path_put(&path);
    return error;
}

int bsp_unlink(const char *pathname)
{
    return do_unlinkat(AT_FDCWD, getname_kernel(pathname));
}

int mdrv_unlink(const char *path_name)
{
    return bsp_unlink(path_name);
}
EXPORT_SYMBOL(mdrv_unlink);
