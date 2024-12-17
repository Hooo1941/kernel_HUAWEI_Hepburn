// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * The uclamp_xxx() sched tunnables' kernel interface and usespace node.
 * Please treat the uclamp here as a new name for the old schedtune, but
 * not util clamping.
 */
#include "sched.h"

#ifdef CONFIG_HISI_EAS_SCHED

bool uclamp_latency_sensitive(struct task_struct *p)
{
#ifdef CONFIG_SCHED_COMMON_QOS_CTRL
	if (test_tsk_thread_flag(p, TIF_LATENCY_SENSITIVE))
		return true;
#endif

	return task_uclamp_group(p, latency_sensitive);
}

int uclamp_task_util_boost(struct task_struct *p)
{
	return task_uclamp_group(p, boost);
}

int uclamp_task_placement_boost(struct task_struct *p)
{
	return task_uclamp_group(p, placement_boost);
}

int task_placement_boosted(struct task_struct *p)
{
	bool boosted;

	boosted = uclamp_task_placement_boost(p) > 0;
	/*
	 * Keep the old logic for compatibility with perfgenius on old
	 * platforms. And we hope global_boost_enable will always be false
	 * on future platforms.
	 */
	boosted = boosted || (global_boost_enable && uclamp_boosted(p));

	if (rt_task(p))
		boosted = boosted && is_heavy_rt(p);
	else
		boosted = boosted || boot_boost;

	return boosted;
}
#endif

#ifdef CONFIG_SCHED_TOP_TASK
bool uclamp_top_task(struct task_struct *p)
{
	return task_uclamp_group(p, top_task);
}
#endif

#ifdef CONFIG_SCHED_VIP_CGROUP
unsigned int uclamp_vip_prio(struct task_struct *p)
{
	return task_uclamp_group(p, vip_prio);
}
#endif

#ifdef CONFIG_SCHED_SMT_EXPELLING
unsigned int uclamp_smt_expeller(struct task_struct *p)
{
	return task_uclamp_group(p, smt_expeller);
}
#endif

#ifdef CONFIG_HISI_EAS_SCHED
u64 latency_sensitive_read(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return (u64)css_uc(css)->latency_sensitive;
}

int latency_sensitive_write(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 ls)
{
	if (ls > 1)
		return -EINVAL;

	css_uc(css)->latency_sensitive = (unsigned int)ls;
	return 0;
}

s64 boost_read(struct cgroup_subsys_state *css,
		struct cftype *cft)
{
	return (s64)css_uc(css)->boost;
}

int boost_write(struct cgroup_subsys_state *css,
		struct cftype *cft, s64 boost)
{
	if (boost < -100 || boost > 100)
		return -EINVAL;

	css_uc(css)->boost = (int)boost;
	trace_perf(sched_uclamp_boost, css->cgroup->kn->name, boost);
	return 0;
}

s64 placement_boost_read(struct cgroup_subsys_state *css,
			struct cftype *cft)
{
	return (s64)css_uc(css)->placement_boost;
}

int placement_boost_write(struct cgroup_subsys_state *css,
			struct cftype *cft, s64 boost)
{
	css_uc(css)->placement_boost = (int)boost;
	return 0;
}
#endif

#ifdef CONFIG_SCHED_TOP_TASK
u64 top_task_read(struct cgroup_subsys_state *css,
			struct cftype *cft)
{
	return (u64)css_uc(css)->top_task;
}

int top_task_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 top_task)
{
	css_uc(css)->top_task = (unsigned int)top_task;
	return 0;
}
#endif

#ifdef CONFIG_SCHED_VIP_CGROUP
u64 vip_prio_read(struct cgroup_subsys_state *css,
			struct cftype *cft)
{
	return (u64)css_uc(css)->vip_prio;
}

int vip_prio_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 vip_prio)
{
	if (vip_prio > VIP_PRIO_WIDTH)
		return -EINVAL;

	css_uc(css)->vip_prio = (unsigned int)vip_prio;
	return 0;
}
#endif

#ifdef CONFIG_SCHED_SMT_EXPELLING
u64 smt_expeller_read(struct cgroup_subsys_state *css,
			struct cftype *cft)
{
	return (u64)css_uc(css)->smt_expeller;
}

int smt_expeller_write(struct cgroup_subsys_state *css,
			struct cftype *cft, u64 smt_expeller)
{
	if (smt_expeller > SMT_EXPELLER)
		return -EINVAL;

	css_uc(css)->smt_expeller = (unsigned int)smt_expeller;
	return 0;
}
#endif
