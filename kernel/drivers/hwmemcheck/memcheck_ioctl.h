/*
 * memcheck_ioctl.h
 *
 * implement the ioctl for user space to get memory usage information,
 * and also provider control command
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#ifndef _MEMCHECK_IOCTL_H
#define _MEMCHECK_IOCTL_H

#include <linux/printk.h>
#include <linux/types.h>

#ifdef CONFIG_DFX_LIBLINUX
#include <linux/signal.h>
#endif


#define	memcheck_err(format, ...)	\
	pr_err("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)
#define	memcheck_info(format, ...)	\
	pr_info("MemCheck[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)

u64 smaps_get_pss(struct vm_area_struct *vma, u64 *swap_pss);
#ifdef CONFIG_DFX_LIBLINUX
extern int liblinux_send_memcheck_signal(int pid, int signum, struct kernel_siginfo *info, __u64 timestamp);
#endif

#endif /* _MEMCHECK_IOCTL_H */
