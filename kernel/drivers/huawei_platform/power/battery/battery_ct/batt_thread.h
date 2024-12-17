/*
 * batt_thread.h
 *
 * battery thread head file
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

#ifndef BATT_THREAD_H_
#define BATT_THREAD_H_

#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <uapi/linux/sched/types.h>

#define PRIORITY_MAX 99

struct batt_thread_work;
typedef void (*batt_twork_func)(struct batt_thread_work *twork);

struct batt_sched_param {
	bool need_expeller;
	bool need_sched_set;
	struct sched_param param;
	unsigned int sched_policy;
};
struct batt_thread_work {
	batt_twork_func fn;
	spinlock_t data_lock;
	bool is_running;
	struct completion work_complete;
	struct batt_sched_param sch_param;
};

int init_thread_work(struct batt_thread_work *twork, batt_twork_func func);
int init_kwork_expeller(struct batt_thread_work *twork, bool need_expeller);
int init_kwork_sche_param(struct batt_thread_work *twork, bool need_sche_param,
	unsigned int policy, int priority);
int batt_kthread_run(char *name, struct batt_thread_work *twork);
void batt_kthread_wait_complete(struct batt_thread_work *twork);
#endif /* BATT_THREAD_H_ */