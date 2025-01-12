/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * include/linux/ashmem.h
 *
 * Copyright 2008 Google Inc.
 * Author: Robert Love
 */

#ifndef _LINUX_ASHMEM_H
#define _LINUX_ASHMEM_H

#include <linux/limits.h>
#include <linux/ioctl.h>
#include <linux/compat.h>

#include "uapi/ashmem.h"

/* support of 32bit userspace on 64bit platforms */
#ifdef CONFIG_COMPAT
#define COMPAT_ASHMEM_SET_SIZE		_IOW(__ASHMEMIOC, 3, compat_size_t)
#define COMPAT_ASHMEM_SET_PROT_MASK	_IOW(__ASHMEMIOC, 5, unsigned int)
#endif

int is_ashmem_file(struct file *file);
#ifdef CONFIG_DFX_MEMCHECK
size_t get_ashmem_size_by_file(struct file *f);
char* get_ashmem_name_by_file(struct file *f);
u64 ashmem_get_total_size(void);
void ashmem_mutex_lock(void);
void ashmem_mutex_unlock(void);
#endif

#endif	/* _LINUX_ASHMEM_H */
