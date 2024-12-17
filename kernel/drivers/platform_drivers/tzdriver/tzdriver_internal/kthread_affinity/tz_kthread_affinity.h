/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: exported funcs for kthread affinity
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef TZ_KTHREAD_AFFINITY_H
#define TZ_KTHREAD_AFFINITY_H

#include <linux/sched.h>
#include <linux/workqueue.h>

#ifdef CONFIG_KTHREAD_AFFINITY
void init_kthread_cpumask(void);
void tz_kthread_bind_mask(struct task_struct *kthread);
void tz_workqueue_bind_mask(struct workqueue_struct *wq, uint32_t flag);
#endif
#endif
