/*
 * hungtask_mmap_sem.h
 *
 * A kernel thread for monitoring the init process
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _HUNGTASK_MMAP_SEM_H
#define _HUNGTASK_MMAP_SEM_H

#include <linux/rwsem.h>
#include <linux/types.h>

#ifdef CONFIG_DFX_HUNGTASK_MMAP_SEM
void htmmap_post_process_check(pid_t pid);
#else
static inline void htmmap_post_process_check(pid_t pid)
{
}
#endif

#ifdef CONFIG_DFX_HUNGTASK_MMAP_SEM_DBG
int htmmap_sem_debug_init(void);
void htmmap_sem_debug(const struct rw_semaphore *sem);
#else
static inline int htmmap_sem_debug_init(void)
{
	return 0;
}

static inline void htmmap_sem_debug(const struct rw_semaphore *sem)
{
}
#endif

#endif
