/* SPDX-License-Identifier: GPL-2.0 */
/*
 * dynamic_prio.h
 *
 * Set a task to a new prio (can be a new sched class) temporarily and
 * restore what it was later.
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

#ifndef __SCHED_DYNAMIC_PRIO_H
#define __SCHED_DYNAMIC_PRIO_H

#include <uapi/linux/sched.h>

int backup_sched_attr(struct task_struct *p);
int restore_sched_attr(struct task_struct *p);

void dynamic_prio_fork(struct task_struct *p);
void dynamic_prio_init(struct task_struct *p);
void dynamic_prio_clear(struct task_struct *p);

int sched_setattr_dynamic(struct task_struct *p,
		const struct sched_attr *attr);

unsigned int get_raw_sched_priority(struct task_struct *p);
void get_raw_attr(struct task_struct *p, struct sched_attr *attr);
unsigned int get_raw_scheduler_policy(struct task_struct *p);

void dyn_prio_irq_work_init(struct rq *rq);

#ifdef CONFIG_VIP_RAISE_BINDED_KTHREAD
bool check_delayed_kthread(struct rq *rq);
#endif

#ifdef CONFIG_RT_SWITCH_CFS_IF_TOO_LONG
bool check_long_rt(struct task_struct *p);
#endif

/*
 * Acts like a SCHED_FLAG_KEEP_RESET_ON_FORK flag.
 * We don't have to backup the ROF flag together with saved_prio. This
 * way works as well.
 * Should hold p->dyn_prio.lock.
 */
#define SCHED_FLAG_KEEP_ROF(p) \
	(p->sched_reset_on_fork ? SCHED_FLAG_RESET_ON_FORK : 0)

#define RAISED_KTHREAD		(1U << 0)
#define LONG_RT_DOWNGRADE	(1U << 1)
#define VIP_BOOSTED		(1U << 2)
#define BINDER_DEFERRED_RESTORE	(1U << 3)

#define RESTORE_ON_SLEEP (RAISED_KTHREAD | LONG_RT_DOWNGRADE |\
				BINDER_DEFERRED_RESTORE)

void reset_dyn_prio_on_sleep(struct task_struct *p);

#endif
