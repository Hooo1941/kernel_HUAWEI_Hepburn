/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * trans rtg thread
 */

#include "include/trans_rtg.h"

#include <linux/atomic.h>
#include <linux/printk.h>
#include <../kernel/sched/sched.h>

#include <linux/sched/frame.h>
#include "include/set_rtg.h"
#include "include/aux.h"

static int g_trans_depth = DEFAULT_TRANS_DEPTH;
static int g_max_thread_num = DEFAULT_MAX_THREADS;
static atomic_t g_rtg_thread_num = ATOMIC_INIT(0);

void set_trans_config(int depth, int max_threads)
{
	if ((depth != INVALID_VALUE) && (depth >= 0))
		g_trans_depth = depth;
	if ((max_threads != INVALID_VALUE) && (max_threads >= 0))
		g_max_thread_num = max_threads;
}

static bool is_depth_valid(struct task_struct *from)
{
	if (g_trans_depth != DEFAULT_TRANS_DEPTH && g_trans_depth <= 0)
		return false;

	if ((g_trans_depth > 0) && (from->rtg_depth != STATIC_RTG_DEPTH) &&
		(from->rtg_depth >= g_trans_depth))
		return false;

	return true;
}

void add_trans_thread(struct task_struct *target, struct task_struct *from)
{
	int ret;
	unsigned int grpid;
	int prio;

	if ((target == NULL) || (from == NULL))
		return;

	if (!is_depth_valid(from))
		return;

	grpid = sched_get_group_id(target);
	if (is_frame_rtg(grpid) || (grpid == DEFAULT_AUX_ID))
		return;

	grpid = sched_get_group_id(from);
	if (is_frame_rtg(grpid)) {
		if (g_max_thread_num != DEFAULT_MAX_THREADS &&
			atomic_read(&g_rtg_thread_num) >= g_max_thread_num)
			return;
		prio = get_frame_prio_by_id(grpid);
	} else if (grpid == DEFAULT_AUX_ID) {
#ifdef CONFIG_RT_PRIO_EXTEND_VIP
		/*
		 * BACKGROUND:
		 * Any users calling sched_setscheduler*() must pass a
		 * sched_param.sched_priority, typically 1, 2 etc. That
		 * user side sched_priority is different from the
		 * task->prio, typically 98,97 or 88,87. We need to
		 * transform them especially when doing priority
		 * inheritance in kernel. See to_kernel_prio() and
		 * to_userspace_prio() in binder.c.
		 * The RT_PRIO_EXTEND_VIP have to change those functions.
		 * WHAT TO DO WITH HERE:
		 * So, we should assign to_userspace_prio(from->prio) from
		 * here to sp.sched_priority in do_set_rtg_sched().
		 * And it's not possible to do that in current code since
		 * the get_frame_prio_by_id() above doesn't use a
		 * sched_priority value (e.g. 1,2) but a 98,97 one.
		 * Let's take it as a special (and bad) case and simply
		 * do what we need here instead of calling a clearer
		 * function.
		 * BTW the code here is not allowed and not able to do
		 * priority inheritance for vip prio.
		 */
		prio = from->prio + VIP_PRIO_WIDTH;
		if (prio >= MAX_RT_PRIO)
			prio = NOT_RT_PRIO;
#else
		prio = (from->prio < MAX_RT_PRIO ? from->prio : NOT_RT_PRIO);
#endif
	} else {
		return;
	}

	get_task_struct(target);
	ret = set_rtg_sched(target, true, grpid, prio, true);
	if (ret < 0) {
		put_task_struct(target);
		return;
	}
	if (from->rtg_depth == STATIC_RTG_DEPTH)
		target->rtg_depth = 1;
	else
		target->rtg_depth = from->rtg_depth + 1;

	if (is_frame_rtg(grpid))
		atomic_inc(&g_rtg_thread_num);
	pr_debug("[AWARE_RTG] %s pid=%d, depth=%d", __func__,
			target->pid, target->rtg_depth);
	put_task_struct(target);
}

void remove_trans_thread(struct task_struct *target)
{
	int ret;
	unsigned int grpid;

	if (target == NULL)
		return;

	grpid = sched_get_group_id(target);
	if (!is_frame_rtg(grpid) && (grpid != DEFAULT_AUX_ID))
		return;

	get_task_struct(target);
	if (target->rtg_depth == STATIC_RTG_DEPTH) {
		put_task_struct(target);
		return;
	}

	ret = set_rtg_sched(target, false, grpid, NOT_RT_PRIO, true);
	if (ret < 0) {
		put_task_struct(target);
		return;
	}
	target->rtg_depth = 0;
	if (is_frame_rtg(grpid) &&
		(atomic_read(&g_rtg_thread_num) > 0))
		atomic_dec(&g_rtg_thread_num);
	pr_debug("[AWARE_RTG] %s pid=%d", __func__, target->pid);
	put_task_struct(target);
}
