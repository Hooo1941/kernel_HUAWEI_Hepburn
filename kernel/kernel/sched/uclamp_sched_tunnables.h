/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * The uclamp_xxx() sched tunnables' kernel interface and usespace node.
 * Please treat the uclamp here as a new name for the old schedtune, but
 * not util clamping.
 */
#ifndef __UCLAMP_SCHED_TUNNABLES_H
#define __UCLAMP_SCHED_TUNNABLES_H

#ifdef CONFIG_HISI_EAS_SCHED
bool uclamp_latency_sensitive(struct task_struct *p);
int uclamp_task_util_boost(struct task_struct *p);
int uclamp_task_placement_boost(struct task_struct *p);
int task_placement_boosted(struct task_struct *p);
#else
static inline bool uclamp_latency_sensitive(struct task_struct *p) { return false; }
static inline int uclamp_task_util_boost(struct task_struct *p) { return 0; }
static inline int uclamp_task_placement_boost(struct task_struct *p) { return 0; }
static inline int task_placement_boosted(struct task_struct *p) { return 0; }
#endif
#ifdef CONFIG_SCHED_TOP_TASK
bool uclamp_top_task(struct task_struct *p);
#else
static inline bool uclamp_top_task(struct task_struct *p) { return false; }
#endif
#ifdef CONFIG_SCHED_VIP_CGROUP
unsigned int uclamp_vip_prio(struct task_struct *p);
#else
static inline unsigned int uclamp_vip_prio(struct task_struct *p) { return 0; }
#endif
#ifdef CONFIG_SCHED_SMT_EXPELLING
unsigned int uclamp_smt_expeller(struct task_struct *p);
#endif

#ifdef CONFIG_HISI_EAS_SCHED
u64 latency_sensitive_read(struct cgroup_subsys_state *css,
				struct cftype *cft);
int latency_sensitive_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 ls);
s64 boost_read(struct cgroup_subsys_state *css,
		struct cftype *cft);
int boost_write(struct cgroup_subsys_state *css,
		struct cftype *cft, s64 boost);
s64 placement_boost_read(struct cgroup_subsys_state *css,
				struct cftype *cft);
int placement_boost_write(struct cgroup_subsys_state *css,
				struct cftype *cft, s64 boost);
#endif
#ifdef CONFIG_SCHED_TOP_TASK
u64 top_task_read(struct cgroup_subsys_state *css,
			struct cftype *cft);
int top_task_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 top_task);
#endif
#ifdef CONFIG_SCHED_VIP_CGROUP
u64 vip_prio_read(struct cgroup_subsys_state *css,
			struct cftype *cft);
int vip_prio_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 vip_prio);
#endif
#ifdef CONFIG_SCHED_SMT_EXPELLING
u64 smt_expeller_read(struct cgroup_subsys_state *css,
			struct cftype *cft);
int smt_expeller_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 smt_expeller);
#endif

#endif
