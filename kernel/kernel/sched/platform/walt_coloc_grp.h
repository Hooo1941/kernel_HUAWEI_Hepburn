/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Colocate group, a group of tasks that acts like one task in scheduler.
 */
#ifndef __WALT_COLOC_GRP_H
#define __WALT_COLOC_GRP_H

#ifndef CONFIG_SCHED_RTG
/* For our rtg_and_colocgrp_prefer_cluster(). */
static inline struct cpumask *rtg_prefer_cluster(struct task_struct *p)
{
	return NULL;
}
#endif

#ifdef CONFIG_SCHED_COLOC_GROUP

struct coloc_group;

enum coloc_grp_id {
	NO_COLOC_GRP_ID = 0,
	UX_CRITICAL_COLOC_GRP_ID,
	MAX_COLOC_GRP_ID,
};

void walt_update_coloc_grp_load(struct task_struct *p, int cpu,
				int event, u64 now);
void migrate_task_coloc_grp_contribution(struct task_struct *p,
					 int src_cpu, int dst_cpu);

void init_task_coloc_grp(struct task_struct *p);
void init_coloc_grps(void);

struct cpumask *rtg_and_colocgrp_prefer_cluster(struct task_struct *p);
void cpufreq_get_coloc_grp_util(struct cpumask *cluster_cpus, u32 *util);

u64 coloc_grp_id_read(struct cgroup_subsys_state *css, struct cftype *cft);
int coloc_grp_id_write(struct cgroup_subsys_state *css,
		       struct cftype *cft, u64 value);

#else

static inline
void walt_update_coloc_grp_load(struct task_struct *p, int cpu,
				int event, u64 now) {}
static inline
void migrate_task_coloc_grp_contribution(struct task_struct *p,
					 int src_cpu, int dst_cpu) {}

static inline
struct cpumask * rtg_and_colocgrp_prefer_cluster(struct task_struct *p)
{
	return rtg_prefer_cluster(p);
}

#endif

#endif
