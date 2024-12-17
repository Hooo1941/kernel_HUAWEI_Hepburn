/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * phase.h - arm64-specific Kernel Phase
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LINUX_PHASE_H
#define _LINUX_PHASE_H

#include <linux/sched/smt.h>
#include <linux/sched.h>

#define PHASE_EVENT_OVERFLOW	(~0ULL)
#define PHASE_RESET_MT_INFO	0x01
#define PHASE_PREV_MODE		0x02
#define PHASE_TASK_SWITCH	0x04
#define PHASE_TASK_TICK		0x08
#define PHASE_TASK_FORCEIDLE	0x10
#define PHASE_CONTEXT_ROTATE	0x20
#define PHASE_ARM_IRQ		0x40

#ifdef CONFIG_PHASE
#define PHASE_PEVENT_NUM		10
#define PHASE_FEVENT_NUM		10
#define PHASE_EVENT_GROUP_TERMINATOR	-1
#define PHASE_EVENT_FINAL_TERMINATOR	-2

#define for_each_phase_pevents(index, events) \
	for (index = 0; events != NULL && index < PHASE_PEVENT_NUM && \
	     events[index] != PHASE_EVENT_FINAL_TERMINATOR; index++)

#define for_each_phase_fevents(index, events) \
	for (index = 0; events != NULL && index < PHASE_FEVENT_NUM && \
	     events[index] != PHASE_EVENT_FINAL_TERMINATOR; index++)

enum phase_window_policy {
	WINDOW_POLICY_LATEST = 0,
	WINDOW_POLICY_RECENT,
	WINDOW_POLICY_AVERAGE,
	NR_WINDOW_POLICY,
};

struct phase_event_pcount {
	u64 data[PHASE_PEVENT_NUM];
};

struct phase_event_fcount {
	u64 data[PHASE_FEVENT_NUM];
};

struct phase_event_count {
	struct phase_event_pcount pcount;
	struct phase_event_fcount fcount;
};

static inline enum smt_mode next_smt_mode(enum smt_mode smt)
{
	return (smt + NR_SMT_MODE) % NR_SMT_MODE;
}

#define PHASE_WINDOW_SIZE	4000000 /* ns */
#define NR_PHASE_WINDOW		5

#ifdef CONFIG_PHASE_COUNTER_WINDOW
struct phase_counter_queue {
	struct phase_event_pcount buffer[NR_PHASE_WINDOW];
	struct phase_event_pcount counter;
	unsigned int rear;
	unsigned int size;
	u64 last_delta;
};

typedef struct phase_counter_window {
	struct phase_counter_queue queue[NR_SMT_MODE];
} phase_counter_window;

static inline unsigned int phase_window_front(phase_counter_window *window,
					      enum smt_mode smt)
{
	return (window->queue[smt].rear + 1 + NR_PHASE_WINDOW -
		window->queue[smt].size) % NR_PHASE_WINDOW;
}

static inline unsigned int phase_window_size(phase_counter_window *window,
					     enum smt_mode smt)
{
	return window->queue[smt].size;
}

static inline struct phase_event_pcount *phase_window_counter(phase_counter_window *window,
							      enum smt_mode smt, int index)
{
	struct phase_counter_queue *queue = &window->queue[smt];

	return &queue->buffer[index];
}

static inline u64 phase_active_window_size(phase_counter_window *window,
					   enum smt_mode smt)
{
	return window->queue[smt].last_delta;
}

#else /* !CONFIG_PHASE_COUNTER_WINDOW */

typedef struct phase_event_pcount phase_counter_window;

static inline struct phase_event_pcount *phase_window_counter(phase_counter_window *window,
							      enum smt_mode smt __maybe_unused,
							      int index __maybe_unused)
{
	return window;
}

static inline u64 phase_active_window_size(phase_counter_window *window __maybe_unused,
					   enum smt_mode smt __maybe_unused)
{
	return 0;
}
#endif /* CONFIG_PHASE_COUNTER_WINDOW */

struct phase_info {
	phase_counter_window window;
	u64 last_update_time;
	u64 left_decay_time_ns;
	u64 total_inst;
	u64 curr_delta;
	enum smt_mode last_mode;
	bool valid[NR_SMT_MODE];
	bool new_task;
#ifdef CONFIG_PHASE_TASK_PERF
	bool tracked;
	struct perf_event *pevents[PHASE_PEVENT_NUM];
	struct perf_event *fevents[PHASE_FEVENT_NUM];
	struct phase_event_count prev;
	struct phase_event_count curr;
#endif
};

void phase_account_task(struct task_struct *prev_task, struct task_struct *next_task, int cpu, unsigned long flags);
int phase_fork(struct task_struct *p);
void phase_free(struct task_struct *p);
u64 phase_data_of_pevent(struct phase_event_pcount *counter, int event_id);
void phase_count_merge(struct phase_event_pcount *count,
		       struct phase_event_pcount *merge);
void phase_clear_count(struct phase_event_pcount *count);

void update_history_pcount(int cpu, struct phase_event_pcount *pcount, int nr_windows);
int set_phase_state(bool enabled, const struct cpumask *new_mask);
#ifdef CONFIG_PHASE_TASK_PERF
extern unsigned int sysctl_phase_task_event_delta;
extern unsigned int sysctl_phase_task_event;
void phase_task_event(struct task_struct *p, unsigned long flags);
int phase_set_task_track(struct task_struct *p, unsigned int track);
#endif

#else /* !CONFIG_PHASE */

static inline void phase_account_task(struct task_struct *prev __maybe_unused,
				      struct task_struct *next __maybe_unused,
				      int cpu __maybe_unused,
				      unsigned long flags __maybe_unused)
{
}
static inline int phase_fork(struct task_struct *p __maybe_unused) { return 0; }
static inline void phase_free(struct task_struct *p __maybe_unused) { }
#endif /* CONFIG_PHASE */

#endif /* _LINUX_PHASE_H */
