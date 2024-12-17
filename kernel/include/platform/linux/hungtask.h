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

#ifndef _HUNGTASK_INTERFACE_H
#define _HUNGTASK_INTERFACE_H

#include <linux/rwsem.h>

#ifdef CONFIG_DFX_HUNGTASK
void hungtask_show_state_filter(u64 state_filter);
pid_t get_system_server_pid(void);
#else
static inline void hungtask_show_state_filter(u64 state_filter)
{
}

static inline pid_t get_system_server_pid(void)
{
	return 0;
}
#endif

#ifdef CONFIG_DFX_HUNGTASK_MMAP_SEM
void htmmap_get_mmap_sem(struct mm_struct *mm,
			 void (*func)(struct rw_semaphore *));
int htmmap_get_mmap_sem_killable(struct mm_struct *mm,
				 int (*func)(struct rw_semaphore *));
void htmmap_post_process_check(pid_t pid);
#else
static inline void htmmap_get_mmap_sem(struct mm_struct *mm,
				       void (*func)(struct rw_semaphore *))
{
}

static inline int htmmap_get_mmap_sem_killable(struct mm_struct *mm,
					       int (*func)(struct rw_semaphore *))
{
	return 0;
}

static inline void htmmap_post_process_check(pid_t pid)
{
}
#endif

#endif
