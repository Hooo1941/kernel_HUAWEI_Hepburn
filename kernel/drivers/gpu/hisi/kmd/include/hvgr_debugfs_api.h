/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DEBUGFS_API_H
#define HVGR_DEBUGFS_API_H

#include <linux/device.h>
#include <linux/types.h>
#ifdef CONFIG_DFX_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif
#include "hvgr_defs.h"

int hvgr_debugfs_init(struct hvgr_device * const gdev);
void hvgr_debugfs_term(struct hvgr_device * const gdev);

#ifdef CONFIG_DFX_DEBUG_FS

#define hvgr_debugfs_read_write_declare(__fops, __read_func, __write_func) \
static ssize_t __fops ## read(struct file *f, char __user *ubuf,           \
	size_t size, loff_t *off)                                              \
{                                                                          \
	void *data = f->private_data;                                          \
	char buf[PAGE_SIZE] = {0};                                             \
	int str_len = 0;                                                       \
	if (data == NULL)                                                      \
		return -EFAULT;                                                    \
	if (__read_func == NULL)                                               \
		return -EFAULT;                                                    \
	str_len = __read_func(data, buf);                                      \
	return simple_read_from_buffer(ubuf, size, off, buf, str_len);         \
}                                                                          \
static ssize_t  __fops ## write(struct file *f,                            \
	const char __user *ubuf, size_t size, loff_t *off)                     \
{                                                                          \
	void *data = f->private_data;                                          \
	char buf[PAGE_SIZE] = {0};                                             \
	size_t cpy_count;                                                      \
	if (data == NULL)                                                      \
		return -EFAULT;                                                    \
	if (__write_func == NULL)                                              \
		return -EFAULT;                                                  \
	cpy_count = min_t(size_t, sizeof(buf) - 1, size);                      \
	if (copy_from_user(buf, (void __user *)ubuf, cpy_count))               \
		return -EFAULT;                                                    \
	return __write_func(data, buf, cpy_count);                             \
}                                                                          \
static const struct file_operations __fops = {                             \
	.open = simple_open,                                                   \
	.read = __fops ## read,                                                \
	.write = __fops ## write,                                              \
	.llseek = seq_lseek,                                                   \
}

#define hvgr_debugfs_register(gdev, file_name, dir, fops) \
	debugfs_create_file(file_name, 0644, dir, gdev, fops)

#define hvgr_debugfs_create_dir(name, parent) debugfs_create_dir(name, parent)

#else

#define hvgr_debugfs_read_write_declare(__fops, __read_func, __write_func)
#define hvgr_debugfs_register(gdev, file_name, dir, fops)
#define hvgr_debugfs_create_dir(name, parent) NULL

#endif

#endif
