/*
 * hwcfs_rwsem.h
 *
 * rwsem schedule declaration
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HWCFS_RWSEM_H_
#define _HWCFS_RWSEM_H_

#include <linux/sched.h>
#include <linux/rwsem.h>

void rwsem_list_add(struct task_struct *tsk, struct list_head *entry,
	struct list_head *head);
void rwsem_dynamic_vip_enqueue(struct task_struct *task,
	struct task_struct *waiter_task,
	struct task_struct *owner,
	struct rw_semaphore *sem);
void rwsem_dynamic_vip_dequeue(struct rw_semaphore *sem,
	struct task_struct *tsk);
void clear_rwsem_boost_tag(struct rw_semaphore *sem);

#ifdef CONFIG_RWSEM_PRIO_OPT
void rwsem_read_dynamic_vip_enqueue(
	struct task_struct *tsk, struct task_struct *waiter_task,
	struct task_struct *owner, struct rw_semaphore *sem, int vip_bit);
void rwsem_read_dynamic_vip_dequeue(struct rw_semaphore *sem,
	struct task_struct *tsk, int vip_bit);
#endif
#endif
