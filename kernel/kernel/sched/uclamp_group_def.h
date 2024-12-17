/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: uclamp_group provide encapsulation of schedtune and
 * CONFIG_SEPARATE_SCHED_TUNE_AND_GROUP_SCHED.
 */
#ifndef __UCLAMP_GROUP_DEF_H
#define __UCLAMP_GROUP_DEF_H

/*
 * Sched features:
 *
 * `schedtune`
 * `groupsched`
 *
 * The Android AMS manages top-app/background/.. and system services declare
 * themselves to foreground/system-background/.. cgroups for different
 * scheduler hints/biases/tunnables. That is a `schedtune` cgroup subsys. By
 * mistake, the schedtune author rename schedtune to uclamp in kernel 5.4 and
 * integrate uclamp to the cpuctl (i.e. `groupsched`) cgroup subsys.
 * So Android userspace now write their pids to /dev/cpuctl/ for the `schedtune`
 * feature. We must separate `schedtune` and `groupsched` as different cgroup
 * subsystems again to get things right.
 * Before kernel 5.4, we have /dev/schedtune/ and /dev/cpuctl/.
 * In the future we would like to have /dev/cpuctl/ and /dev/groupsched/.
 *
 * Userspace interfaces:
 *
 * #ifdef CONFIG_SEPARATE_SCHED_TUNE_AND_GROUP_SCHED
 * /dev/cpuctl/, for `schedtune`, in order to make userspace unaware.
 * /dev/groupsched/, which is newly added, for `groupsched`.
 * #else
 * /dev/cpuctl/ for both `schedtune` and `groupsched`.
 * #endif
 *
 * Kernel variables:
 *
 * #ifdef CONFIG_SEPARATE_SCHED_TUNE_AND_GROUP_SCHED
 * cpu_cgrp, for `schedtune`. (Alternatively we can name it as schedtune_cgrp
 * and mount /dev/cpuctl to schedtune. Both ways are good.)
 * groupsched_cgrp, for `groupsched`.
 * #else
 * cpu_cgrp for both `schedtune` and `groupsched`.
 * #endif
 *
 * Uclamp: an alias (or an encapsulation) of `schedtune` part of cpu_cgrp.
 * There're already kernel interfaces like uclamp_latency_sensitive() /
 * uclamp_task_util_boost(). So we don't use schedtune as a name prefix for
 * the interfaces but use uclamp as an alias of schedtune.
 *
 * Detailed sched features of `schedtune` and `groupsched`:
 * `groupsched`:
 *   FAIR_GROUP_SCHED
 *   SCHED_LATENCY_NICE
 *   CFS_BANDWIDTH
 *   RT_GROUP_SCHED
 *   UCLAMP_TASK_GROUP(uclamp.min/max)
 *   (Note that UCLAMP_TASK_GROUP belongs to `schedtune` in concept but we
 *    still keep it in `groupsched` subsys since it's useless and hard to
 *    modify its implementation. So there is a /dev/groupsched/uclamp.min,
 *    and nobody wants to write it.)
 * `schedtune`:
 * Others. including
 *   latency_sensitive
 *   boost
 *   placement_boost
 *   top_task
 *   vip_prio
 *   smt_expeller
 *   timer_slack_pct
 *
 * #ifdef CONFIG_SEPARATE_SCHED_TUNE_AND_GROUP_SCHED
 * `groupsched` variables and nodes are in `struct task_group` (related to
 * struct cfs_rq) / `groupsched_cgrp` / `/dev/groupsched/`.
 * `schedtune` variables and nodes are in `struct schedtune` / `cpu_cgrp` /
 * `/dev/cpuctl/`.
 * #else
 * All are in `struct task_group` / `cpu_cgrp` / `/dev/cpuctl/`.
 * #endif
 */

#ifdef CONFIG_SEPARATE_SCHED_TUNE_AND_GROUP_SCHED
struct schedtune;
typedef struct schedtune uclamp_group_t;
#else
struct task_group;
typedef struct task_group uclamp_group_t;
#endif

#define css_uc(_css) ({						\
	struct cgroup_subsys_state *__css = (_css);		\
	__css ? container_of(__css, uclamp_group_t, css) : NULL; })

#define uclamp_cgrp_id cpu_cgrp_id

#define get_uclamp_group(p) css_uc(task_css_check((p), uclamp_cgrp_id, true))

#define task_uclamp_group(p, member) ({			\
	uclamp_group_t *ug = NULL;			\
	typeof(ug->member) val;				\
							\
	rcu_read_lock();				\
	ug = get_uclamp_group(p);			\
	val = ug ? ug->member : (typeof(ug->member))0;	\
	rcu_read_unlock();				\
	val; })

#endif
