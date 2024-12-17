// SPDX-License-Identifier: GPL-2.0
/*
 * ffrt_rtg.h
 *
 * ffrt rtg header
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef FFRT_RTG_H
#define FFRT_RTG_H

#include <linux/fs.h>

#ifdef CONFIG_FFRT_RTG
int proc_ffrt_open(struct inode *inode, struct file *filp);
long proc_ffrt_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
bool is_ffrt_rtg(int id);
#ifdef CONFIG_COMPAT
long proc_ffrt_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
#else /* CONFIG_FFRT_RTG */
static inline int proc_ffrt_open(struct inode *inode, struct file *filp)
{
	return 0;
}
static inline long proc_ffrt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}
bool is_ffrt_rtg(int id)
{
	return false;
}
#ifdef CONFIG_COMPAT
static inline long proc_ffrt_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}
#endif
#endif /* CONFIG_FFRT_RTG */
#endif