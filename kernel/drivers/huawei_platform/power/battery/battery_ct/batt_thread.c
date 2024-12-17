/*
 * batt_thread.c
 *
 * battery thread utils
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "batt_thread.h"
#include <linux/errno.h>
#include <linux/kthread.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_SCHED_SMT_EXPELLING
#include <linux/sched/smt.h>
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG batt_thread
HWLOG_REGIST();

int init_kwork_expeller(struct batt_thread_work *twork, bool need_expeller)
{
	if (!twork) {
		hwlog_err("%s: work is null\n", __func__);
		return -EINVAL;
	}
	twork->sch_param.need_expeller = need_expeller;
	return 0;
}

int init_kwork_sche_param(struct batt_thread_work *twork, bool need_sche_param,
	unsigned int policy, int priority)
{
	if (!twork) {
		hwlog_err("%s: work is null\n", __func__);
		return -EINVAL;
	}
	if (need_sche_param) {
		twork->sch_param.need_sched_set = need_sche_param;
		twork->sch_param.sched_policy = policy;
		twork->sch_param.param.sched_priority = priority;
	}
	return 0;
}

int init_thread_work(struct batt_thread_work *twork, batt_twork_func func)
{
	if (!twork || !func) {
		hwlog_err("%s: work is null\n", __func__);
		return -1;
	}
	init_completion(&twork->work_complete);
	twork->fn = func;
	spin_lock_init(&twork->data_lock);
	return 0;
}

static int batt_kthread_run_wrapper(void *data)
{
	unsigned long flags;
	struct batt_thread_work *twork = data;

	twork->fn(twork);
	complete(&twork->work_complete);
	spin_lock_irqsave(&twork->data_lock, flags);
	twork->is_running = false;
	spin_unlock_irqrestore(&twork->data_lock, flags);
	return 0;
}

static void set_task_exclusive(struct task_struct *kthread)
{
#ifdef CONFIG_SCHED_SMT_EXPELLING
	set_task_expeller(kthread, SMT_EXPELLER_FORCE_LONG);
#endif
}

static void set_task_priority(struct batt_thread_work *twork,
	struct task_struct *kthread)
{
	sched_setscheduler(kthread, twork->sch_param.sched_policy,
		&twork->sch_param.param);
}

static void set_task_schedule_param(struct batt_thread_work *twork,
	struct task_struct *kthread)
{
	if (twork->sch_param.need_expeller)
		set_task_exclusive(kthread);
	if (twork->sch_param.need_sched_set)
		set_task_priority(twork, kthread);
}

int batt_kthread_run(char *name, struct batt_thread_work *twork)
{
	unsigned long flags;
	struct task_struct *kthread = NULL;

	if (!twork || !name) {
		hwlog_err("%s:twork or name is null\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&twork->data_lock, flags);
	if (twork->is_running) {
		hwlog_err("%s: is running\n", __func__);
		spin_unlock_irqrestore(&twork->data_lock, flags);
		return -EBUSY;
	}
	twork->is_running = true;
	spin_unlock_irqrestore(&twork->data_lock, flags);
	kthread = kthread_create(batt_kthread_run_wrapper, twork, "bkt:%s", name);
	if (!kthread) {
		hwlog_err("%s: kthread_create failed\n", __func__);
		goto disable_run;
	}
	set_task_schedule_param(twork, kthread);
	wake_up_process(kthread);
	return 0;

disable_run:
	spin_lock_irqsave(&twork->data_lock, flags);
	twork->is_running = false;
	spin_unlock_irqrestore(&twork->data_lock, flags);
	return -ESRCH;
}


void batt_kthread_wait_complete(struct batt_thread_work *twork)
{
	wait_for_completion(&twork->work_complete);
}