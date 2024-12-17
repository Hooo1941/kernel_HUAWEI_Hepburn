/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * mips.h
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
#ifndef _MIPS_H
#define _MIPS_H
#include <platform_include/cee/linux/phase.h>
#include <platform_include/cee/linux/mips_mem.h>
#ifdef CONFIG_MIPS_LOAD_TRACKING
#define INST_PER_KINST	1000
struct rq_mips_demand {
	u64 prev_inst;
	u64 curr_inst;
	int curr_avg_ipc;
	u64 mips_window_start;
	u64 mips_last_update;
};
#define MIN_MIPS_WINDOW_SIZE 4000000 /* 4ms */
#define MAX_MIPS_WINDOW_SIZE 100000000 /* 100ms */
#define arch_set_mips_scale topology_set_mips_scale
#define arch_get_mips_scale topology_get_mips_scale

void topology_set_mips_scale(unsigned int cpu, unsigned long policy_max_freq);
extern unsigned int mips_enable;
extern unsigned int mips_task_enable;
extern unsigned int mips_task_report_policy;
extern unsigned int mips_cpu_report_policy;
extern unsigned int mips_window_size;
void phase_update_task_tmipsd(struct task_struct *p, int cpu, u64 prev_inst,
			      u64 prev_cycle, int flags);
void phase_update_rq_rmipsd(struct rq *rq, struct phase_event_pcount *delta, int flags);
void update_mips_window(struct task_struct *p, struct rq *rq, u64 now, int flags);
void mips_migrate_update(struct task_struct *p, int cpu);
void mips_tick_update(struct task_struct *p, int cpu);
void mips_inc_inst_demand(struct rq *rq, struct task_struct *p);
void mips_dev_inst_demand(struct rq *rq, struct task_struct *p);
void init_rq_mips(struct rq_mips_demand *rmipsd);
int mips_start(void);
int mips_stop(void);
u64 task_inst_demand(struct task_struct *p);
u64 cpu_inst_demand(int cpu);
void update_mips_scale(int cpu);
int mips_fork(struct task_struct *p);
void mips_free(struct task_struct *p);
int update_mips_window_start(u64 *window_start, u64 window_size, u64 now);
void get_mips_mem_info(int cpu, struct pcounts *mem_info, int nr_window);
#else
static inline void update_task_mips(struct task_struct *p, int cpu, u64 prev_inst,
				    u64 prev_cycle, int flags) {}
static inline void update_cpu_mips(struct rq *rq, struct phase_event_pcount *delta, int flags) {}
static inline void update_mips_window(struct task_struct *p, struct rq *rq, u64 now, int flags) {}
static inline void mips_freq_tick_update(struct task_struct *p, int cpu) {}
static inline void mips_inc_inst_demand(struct rq *rq, struct task_struct *p) {}
static inline void mips_dev_inst_demand(struct rq *rq, struct task_struct *p) {}
#endif

static inline u64 div_u64_protected(u64 dividend, u64 divisor)
{
	if (divisor == 0)
		return 0;
	return dividend / divisor;
}

enum {
	MIPS_RECENT		= (1 << 0),
	MIPS_MAX		= (1 << 1),
	MIPS_AVG		= (1 << 2),
};
#ifdef CONFIG_CPU_FREQ_GOV_MIPS
extern void mips_check_freq_update(int cpu, unsigned int flags);
extern void load_pred_mips_freq(int cpu, struct task_struct *p, int flags);
#ifdef CONFIG_SCHED_RTG
extern void mipsgov_mark_freq_change(int cpu);
#endif
#else
static inline void mips_check_freq_update(int cpu, unsigned int flags) {}
static inline void load_pred_mips_freq(int cpu, struct task_struct *p, int flags) {}
#ifdef CONFIG_SCHED_RTG
static inline void mipsgov_mark_freq_change(int cpu) {}
#endif
#endif
#endif
