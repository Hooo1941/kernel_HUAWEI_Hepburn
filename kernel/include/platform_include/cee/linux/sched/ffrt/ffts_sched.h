// SPDX-License-Identifier: GPL-2.0
/*
 * ffts_sched.h
 *
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
#ifndef FFTS_SCHED_H
#define FFTS_SCHED_H
#include <linux/types.h>

struct task_struct;
typedef void (*func_thread_switch)(u8 lpid, u16 ffts_uid, u16 ffts_tid);
struct ffts_sched_callback {
	func_thread_switch  thread_switch;
};
/* callback is support ffrt driver ko, make test easy for temporary */
#ifdef CONFIG_FFTS_SCHED
int ffts_sched_proc_init(u16 ffts_uid, u16 ffts_tid, u8 qos_num);
void ffts_sched_proc_exit(pid_t pid);
void ffts_sched_worker_wakeup(int tid, int qos);
int ffts_sched_register_callback(struct ffts_sched_callback *cb);

void ffts_sched_context_switch(int cpu, struct task_struct *prev, struct task_struct *next);
void ffts_sched_release(struct task_struct *p);
void ffts_sched_init(struct task_struct *p);
#else
static inline int ffts_sched_proc_init(u16 ffts_uid __maybe_unused, u16 ffts_tid __maybe_unused,
	u8 qos_num __maybe_unused){return 0;}
static inline void ffts_sched_proc_exit(struct task_struct *p __maybe_unused){}
static inline void ffrt_task_switch(int cpu __maybe_unused,
	struct task_struct *prev __maybe_unused,
	struct task_struct *next __maybe_unused){}
static inline void ffts_sched_release(struct task_struct *p __maybe_unused){}
static inline void ffts_sched_worker_wakeup(int tid __maybe_unused, int qos __maybe_unused){}
static inline void ffts_sched_init(struct task_struct *p){}


static inline int ffts_sched_register_callback(struct ffts_sched_callback *cb __maybe_unused){return 0;}
#endif
#endif