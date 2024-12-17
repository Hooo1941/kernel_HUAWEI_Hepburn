/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Platform implementation for scheduler (common, not for a specific sched
 * class). Aim to avoid duplicated code in oh kernel.
 */
#ifndef __PLAT_SCHED_H
#define __PLAT_SCHED_H

#ifdef CONFIG_HISI_EAS_SCHED
void get_task_fit_cpus(struct task_struct *p, struct cpumask *fit_cpus);
bool task_fit_cpu(struct task_struct *p, int cpu);

/* Note that there's a if after for. Use it carefully. */
#define for_each_cpu_reverse(cpu, mask)					\
	for (cpu = cpumask_last(mask);					\
	     cpu < nr_cpu_ids && cpu >= cpumask_first(mask); cpu--)	\
		if (cpumask_test_cpu(cpu, mask))

void get_avail_cpus(struct cpumask *cpus, struct task_struct *p);
void get_max_spare_cap_cpus(struct cpumask *cpus, struct task_struct *p);
void honor_prev_cpu(struct cpumask *cpus, struct task_struct *p);
int find_best_candidate(struct task_struct *p, struct cpumask *candidate_cpus, bool boosted);
bool fit_margin(u64 util, u64 cap, u32 margin);
#endif

#ifdef CONFIG_SMT_MODE_GOV
void update_smt_capacity(struct rq *rq);
#define HIGHER_CPU_CAP_PCT 110
#else
static inline void update_smt_capacity(struct rq *rq) {}
#define HIGHER_CPU_CAP_PCT 100
#endif

#ifdef CONFIG_SCHED_SMT
#define sibling_rq(rq) cpu_rq(rq->smt_sibling)
#define is_smt(cpu) cpu_rq(cpu)->is_smt
#else
#define is_smt(cpu) false
#endif

#endif
