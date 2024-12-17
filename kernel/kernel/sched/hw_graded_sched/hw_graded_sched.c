/*
 * hw_graded_sched.c
 *
 * hw graded schedule implementation
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

#include <trace/events/sched.h>
#include <linux/sched/signal.h>
#include <linux/hw_graded_sched/hw_graded_sched.h>
#include "../kernel/sched/sched.h"

DEFINE_STATIC_KEY_TRUE(g_graded_sched_key);

static inline void restore_graded_nice(struct task_struct* task)
{
	task_lock(task);
	set_graded_nice(task, task_nice(task));
	task_unlock(task);
}

void update_graded_nice(struct task_struct* task, int increase)
{
	struct task_struct *child = NULL;
	int ori_nice;
	int new_nice;

	if (!static_branch_unlikely(&g_graded_sched_key) || !task)
		return;

	if (task->pid != task->tgid) {
#ifdef CONFIG_HW_VIP_THREAD
		if (task->static_vip && task->graded_prio != task->static_prio)
			restore_graded_nice(task);
#endif
		return;
	}

	rcu_read_lock();
	child = task;
	if (increase) {
		do {
			if (task == child || child->sched_class != &fair_sched_class)
				continue;
			task_lock(child);
			if (child->graded_prio == child->static_prio) {
				ori_nice = task_nice(child);
				new_nice = max(ori_nice - HCFS_GRADED_OFFSET, MIN_NICE);
				set_graded_nice(child, new_nice);
			}
			task_unlock(child);
			trace_sched_update_graded_nice(child, increase);
		} while_each_thread (task, child);
	} else {
		do {
			if (child->graded_prio != child->static_prio)
				restore_graded_nice(child);
			trace_sched_update_graded_nice(child, increase);
		} while_each_thread (task, child);
	}
	rcu_read_unlock();
}

void init_graded_nice(struct task_struct* task, int increase)
{
	int new_nice = 0;

	if (!static_branch_unlikely(&g_graded_sched_key) || !task)
		return;

	if (increase && task->sched_class == &fair_sched_class) {
		new_nice = max(task_nice(task) - HCFS_GRADED_OFFSET, MIN_NICE);
		task->graded_prio = NICE_TO_PRIO(new_nice);
		task->prio = task->normal_prio = task->graded_prio;
		set_graded_load_weight(task, false);
	} else {
		task->graded_prio = NICE_TO_PRIO(task_nice(task));
	}
	trace_sched_init_graded_nice(task, increase);
}

#ifdef CONFIG_HW_GRADED_SCHED_WAKEUP_NEW_TASK_FAST
static void disable_wakeup_new_task_faster(struct task_struct* task)
{
	struct task_struct *child = NULL;

	if (task->pid != task->tgid)
		return;

	rcu_read_lock();
	child = task;

	do {
		if (child->static_vip != 0) {
			child->static_vip = 0;
			trace_sched_wakeup_new_task_fast(child, 0);
		}
	} while_each_thread (task, child);
	rcu_read_unlock();
}

void set_wakeup_new_task_fast(struct task_struct *p, unsigned int val)
{
	unsigned int old_val;

	old_val = p->wakeup_new_task_fast;
	p->wakeup_new_task_fast = !!val;

	if (!val && old_val)
		disable_wakeup_new_task_faster(p);
}
#endif
