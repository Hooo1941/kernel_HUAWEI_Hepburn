// SPDX-License-Identifier: GPL-2.0
/*
 * dynamic_prio.c
 *
 * Set a task to a new prio (can be a new sched class) temporarily and
 * restore what it was later.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "sched.h"
#include <securec.h>
#include <trace/events/sched.h>
#include <platform_include/cee/linux/render_rt.h>

#define INVALID_POLICY 0xbbad

static bool dyn_switched(struct task_struct *p)
{
	/*
	 * Setting vip is a kind of dynamic switching but we didn't set a
	 * bit on the dyn_switched_flag to avoid redundancy. So that we
	 * check both normal_vip_prio and dyn_switched_flag here.
	 */
	return p->dyn_prio.normal_vip_prio || p->dyn_prio.dyn_switched_flag;
}

/*
 * One must backup sched attr before setting dynamic prio and the flags.
 * Must hold p->dyn_prio.lock.
 */
int backup_sched_attr(struct task_struct *p)
{
	if (task_has_dl_policy(p) ||
	    p->sched_class == &stop_sched_class ||
	    p->sched_class == &idle_sched_class)
		return -EINVAL;

	/* Already backed up. */
	if (dyn_switched(p))
		return 0;

	/* Only back up the priority before dynamic switched. */
	p->dyn_prio.saved_policy = p->policy;
	p->dyn_prio.saved_prio = p->normal_prio;

	return 0;
}

/* Must hold p->dyn_prio.lock. */
int restore_sched_attr(struct task_struct *p)
{
	unsigned int vip_prio;
	struct sched_attr attr = {
		.sched_flags = SCHED_FLAG_KEEP_ROF(p),
	};

	vip_prio = p->dyn_prio.normal_vip_prio;
#ifdef CONFIG_SCHED_VIP_PI
	vip_prio = max(vip_prio, get_max_vip_boost(p));
#endif

	if (vip_prio > 0) {
		attr.sched_policy = SCHED_RR;
		attr.sched_priority = vip_prio;
	} else {
		unsigned int policy = p->dyn_prio.saved_policy;
		unsigned int prio = p->dyn_prio.saved_prio;

		attr.sched_policy = policy;
		if (fair_policy(policy))
			attr.sched_nice = PRIO_TO_NICE(prio);
		else if (rt_policy(policy))
			attr.sched_priority = MAX_RT_PRIO-1 - prio;
		else
			return -EINVAL;
	}

	return sched_setattr_dynamic(p, &attr);
}

int sched_setattr_dynamic(struct task_struct *p, const struct sched_attr *attr)
{
	int ret;

#ifdef CONFIG_RT_SWITCH_CFS_IF_TOO_LONG
	if ((p->dyn_prio.dyn_switched_flag & LONG_RT_DOWNGRADE) &&
	    rt_policy(attr->sched_policy))
		return -EINVAL;
#endif

	/*
	 * To let __sched_setscheduler() know we are setting dynamic prio.
	 * That makes __sched_setscheduler() able to acquire p->dyn_prio.lock
	 * and deal with dyn_prio flags for callers other than us.
	 */
	set_tsk_thread_flag(current, TIF_DYN_PRIO);
	/* No check, no PI, no priority mapping. */
	ret = sched_setattr_directly(p, attr);
	clear_tsk_thread_flag(current, TIF_DYN_PRIO);

	return ret;
}

static void copy_saved_prio(struct task_struct *src, struct task_struct *dst)
{
	if (WARN_ON_ONCE(src->dyn_prio.saved_policy == INVALID_POLICY))
		return;

	dst->policy = src->dyn_prio.saved_policy;
	dst->prio = dst->normal_prio = dst->static_prio = src->dyn_prio.saved_prio;

	if (rt_policy(dst->policy))
		dst->rt_priority = MAX_RT_PRIO-1 - dst->normal_prio;
	else
		dst->rt_priority = 0;
}

static void copy_prio(struct task_struct *src, struct task_struct *dst)
{
	dst->policy = src->policy;
	dst->prio = dst->normal_prio = src->normal_prio; /* No PI boost */
	dst->static_prio = src->static_prio;
	dst->rt_priority = src->rt_priority;
}

void dynamic_prio_fork(struct task_struct *child)
{
	unsigned long flags;

	/*
	 * To make child inherit parent's non-dynamic priority correctly:
	 * 1. Copy the sched prio again here because current may have
	 *    dynamic prio when doing dup_task_struct().
	 * 2. Make sure the `p->prio = current->normal_prio` by PI is
	 *    called before us.
	 * 3. Make sure the sched_reset_on_fork is called after us.
	 * 4. We can add current->dyn_prio.lock to arch_dup_task_struct()
	 *    and check if dyn_switched(child) then restore child's saved
	 *    prio to child itself without holding any lock here.
	 *    But less change in other files is preferred.
	 *    So we hold current->dyn_prio.lock and copy current's prio
	 *    again. Note that we can't skip copying even if current
	 *    doesn't have dyn prio now.
	 */
	spin_lock_irqsave(&current->dyn_prio.lock, flags);
	if (dyn_switched(current))
		copy_saved_prio(current, child);
	else
		copy_prio(current, child);
	spin_unlock_irqrestore(&current->dyn_prio.lock, flags);
	WARN_ON_ONCE(is_vip_prio(child->normal_prio));
}

void dynamic_prio_init(struct task_struct *p)
{
	spin_lock_init(&p->dyn_prio.lock);
	dynamic_prio_clear(p);
}

void dynamic_prio_clear(struct task_struct *p)
{
	p->dyn_prio.saved_policy = INVALID_POLICY;
	p->dyn_prio.dyn_switched_flag = 0;
#ifdef CONFIG_SCHED_VIP
	p->dyn_prio.normal_vip_prio = 0;
#endif
#ifdef CONFIG_SCHED_VIP_PI
	memset_s(p->dyn_prio.vip_boost, sizeof(p->dyn_prio.vip_boost), 0,
		sizeof(p->dyn_prio.vip_boost));
#endif
#ifdef CONFIG_RT_ENERGY_EFFICIENT_SUPPORT
	/*
	 * See comments in __setscheduler().
	 * If anyone *overrides* p's priority then p has no TIF_* flag
	 * by default. After that the caller may set the flags on its
	 * own need.
	 *
	 * Considering the case: Mali thread sets its rt prio and wants
	 * no heavy flag. But it may be in a vip-boosted cgroup. We can
	 * allow mali thread to have higher rt prio than UI thread but
	 * it can't have heavy flag otherwise it will preempt the only
	 * biggest core in boosted case and hurt UI performance. Mali
	 * actually doesn't want the biggest core.
	 */
	clear_tsk_thread_flag(p, TIF_ENERGY_EFFICIENT);
	clear_tsk_thread_flag(p, TIF_EXPECTED_HEAVY);
#endif
}

/*
 * What is raw_xxx?
 * The non-dynamic sched attributes of p.
 * That is, if dyn_switched(p) return attr based on p->dyn_prio.saved_*
 * else the normal sched priority.
 */

static unsigned int _get_raw_policy(struct task_struct *p)
{
	if (dyn_switched(p))
		return p->dyn_prio.saved_policy;
	return p->policy;
}

/* Checked rt raw_policy */
static unsigned int _get_raw_rt_priority(struct task_struct *p)
{
	unsigned int rt_priority;

	if (dyn_switched(p))
		rt_priority = MAX_RT_PRIO-1 - p->dyn_prio.saved_prio;
	else
		rt_priority = p->rt_priority;

#ifdef CONFIG_RT_PRIO_EXTEND_VIP
	return vip_spare_unmap_rt_priority(rt_priority);
#else
	return rt_priority;
#endif
}

/* Checked fair raw_policy */
static int _get_raw_static_prio(struct task_struct *p)
{
	if (dyn_switched(p))
		return p->dyn_prio.saved_prio;
	return p->static_prio;
}

static int _get_raw_nice(struct task_struct *p)
{
	return PRIO_TO_NICE(_get_raw_static_prio(p));
}

unsigned int get_raw_sched_priority(struct task_struct *p)
{
	unsigned long flags;
	unsigned int rt_priority = 0;

	spin_lock_irqsave(&p->dyn_prio.lock, flags);

	if (rt_policy(_get_raw_policy(p)))
		rt_priority = _get_raw_rt_priority(p);

	spin_unlock_irqrestore(&p->dyn_prio.lock, flags);

	return rt_priority;
}

void get_raw_attr(struct task_struct *p, struct sched_attr *attr)
{
	unsigned long flags;
	unsigned int raw_policy;

	spin_lock_irqsave(&p->dyn_prio.lock, flags);

	raw_policy = _get_raw_policy(p);

	attr->sched_policy = raw_policy;
	if (p->sched_reset_on_fork)
		attr->sched_flags |= SCHED_FLAG_RESET_ON_FORK;

	if (dl_policy(raw_policy))
		__getparam_dl(p, attr);
	else if (rt_policy(raw_policy))
		attr->sched_priority = _get_raw_rt_priority(p);
	else
		attr->sched_nice = _get_raw_nice(p);

	spin_unlock_irqrestore(&p->dyn_prio.lock, flags);
}

unsigned int get_raw_scheduler_policy(struct task_struct *p)
{
	unsigned long flags;
	unsigned int policy;

	spin_lock_irqsave(&p->dyn_prio.lock, flags);
	policy = _get_raw_policy(p);
	policy |= (p->sched_reset_on_fork ? SCHED_RESET_ON_FORK : 0);
	spin_unlock_irqrestore(&p->dyn_prio.lock, flags);
	return policy;
}

void get_raw_kernel_priority(struct task_struct *p,
			unsigned int *policy, int *prio)
{
	unsigned long flags;

	spin_lock_irqsave(&p->dyn_prio.lock, flags);

	if (dyn_switched(p)) {
		*policy = p->dyn_prio.saved_policy;
		*prio = p->dyn_prio.saved_prio;
	} else {
		*policy = p->policy;
		*prio = p->normal_prio;
	}

	spin_unlock_irqrestore(&p->dyn_prio.lock, flags);
	WARN_ON(*policy == INVALID_POLICY);
}

/*
 * The irq_work helpers.
 */
static void dyn_prio_work_handler(struct irq_work *irq_work)
{
	struct rq *rq = container_of(irq_work, struct rq, dyn_prio_work);
	struct task_struct *p = rq->dyn_prio_work_task;

	spin_lock(&p->dyn_prio.lock);
	rq->dyn_prio_work_func(p);
	spin_unlock(&p->dyn_prio.lock);

	trace_perf(dyn_prio_work, (void *)rq->dyn_prio_work_func, p);
	put_task_struct(p);
}

/*
 * Note that only the first call will be queued successfully when
 * multiple calls requested at once.
 */
static bool
queue_irq_work(void (*func)(struct task_struct *), struct task_struct *p)
{
	unsigned long flags;
	struct rq *rq = NULL;
	bool queued = false;

	local_irq_save(flags);

	rq = this_rq();
	if (irq_work_queue(&rq->dyn_prio_work)) {
		get_task_struct(p);
		rq->dyn_prio_work_task = p;
		rq->dyn_prio_work_func = func;
		queued = true;
	}

	local_irq_restore(flags);
	return queued;
}

void dyn_prio_irq_work_init(struct rq *rq)
{
	init_irq_work(&rq->dyn_prio_work, dyn_prio_work_handler);
	rq->dyn_prio_work_func = NULL;
	rq->dyn_prio_work_task = NULL;
}

static inline bool __test_and_set(unsigned int mask, unsigned int *p)
{
	unsigned int old = *p;
	*p |= mask;
	return old & mask;
}

#ifdef CONFIG_VIP_RAISE_BINDED_KTHREAD

static void vip_raise_kthread(struct task_struct *p)
{
	struct sched_attr attr = {
		.sched_flags = SCHED_FLAG_KEEP_ROF(p),
		.sched_policy = SCHED_RR,
		.sched_priority = VIP_PRIO_WIDTH + 1,
	};

	if (backup_sched_attr(p))
		return;

	if (__test_and_set(RAISED_KTHREAD, &p->dyn_prio.dyn_switched_flag))
		return;

	sched_setattr_dynamic(p, &attr);
}

#define PER_CPU_KTHREAD_DELAY_NS (10 * NSEC_PER_MSEC)

/*
 * Called from check_for_rt_migration() with rq lock held.
 * If there's a vip thread running and a per cpu kthread delayed,
 * raise the kthread with VIP_PRIO_WIDTH+1 to preempt the vip.
 */
bool check_delayed_kthread(struct rq *rq)
{
	struct task_struct *p = rq->binded_kthread;

	if (likely(!p))
		return false;

	if (!is_vip_prio(rq->rt.highest_prio.curr))
		return false;

	if (task_delay(p, rq) < PER_CPU_KTHREAD_DELAY_NS)
		return false;

	if (p->dyn_prio.dyn_switched_flag & RAISED_KTHREAD)
		return false;

	return queue_irq_work(vip_raise_kthread, p);
}

#endif /* CONFIG_VIP_RAISE_BINDED_KTHREAD */

#ifdef CONFIG_RT_SWITCH_CFS_IF_TOO_LONG

static void long_rt_downgrade(struct task_struct *p)
{
	struct sched_attr attr = {
		.sched_flags = SCHED_FLAG_KEEP_ROF(p),
		.sched_policy = SCHED_NORMAL,
		.sched_priority = 0,
		.sched_nice = 0,
	};

	if (backup_sched_attr(p))
		return;

	if (__test_and_set(LONG_RT_DOWNGRADE, &p->dyn_prio.dyn_switched_flag))
		return;

	sched_setattr_dynamic(p, &attr);
}

#define TOO_LONG_RT_RUNNING_NS (2 * NSEC_PER_SEC)
#define LB_BUG_ESCAPE_NS (50 * NSEC_PER_MSEC)

/*
 * Called from check_for_rt_migration().
 * We must have this feature to protect us against buggy code (dead-busy-loop)
 * running in background cpuset with rt priority and starving other background
 * tasks.
 * Note that it can't protect us against attacking since a short sleep can
 * easily break its condition. The rt throttling can't help either. Actually
 * we can do nothing about it, even if the attacking task is in normal cfs
 * priority.
 * And an important thing here is about the threshold to determine *too long*.
 * The UI thread in OH can run for very long reasonably. It might be a good
 * choice to have different threshold on performance cores and cpuset limited
 * cores. Another better choice would be letting each cpuset cgroup have their
 * own threshold. That is, different threshold for different tasks to gurantee
 * top-app performance and prevent background misuse. We really need to have a
 * very small threshold or to discourage rt priority in any way for background
 * cgroup since other starving bg tasks may hold kernel locks.
 * But, for many reasons, let's leave it to the future. Now we simply re-enable
 * this feature and use a large threshold just to prevent system hanging.
 */
bool check_long_rt(struct task_struct *p)
{
	s64 delta_exec = p->se.sum_exec_runtime - p->sum_exec_runtime_wakeup;

	/*
	 * An unresolved cfs lb bug, that a rt thread of a game livelock on big core
	 * waiting for a cfs but the cfs failed to load balance away.
	 * Let's escape earlier than TOO_LONG_RT_RUNNING_NS in this case.
	 */
	if (delta_exec > LB_BUG_ESCAPE_NS &&
	    task_rq(p)->nr_running > 1 &&
	    is_max_cap_orig_cpu(task_cpu(p)) &&
	    render_rt_inited())
		sched_core_push_tasks(task_rq(p));

	if (likely(delta_exec < TOO_LONG_RT_RUNNING_NS &&
		   !cgroup_vip_limit_long_running(p, delta_exec)))
		return false;

	if (p->dyn_prio.dyn_switched_flag & LONG_RT_DOWNGRADE)
		return false;

	return queue_irq_work(long_rt_downgrade, p);
}

#endif /* CONFIG_RT_SWITCH_CFS_IF_TOO_LONG */

static void reset_on_sleep(struct task_struct *p)
{
	if (!(p->dyn_prio.dyn_switched_flag & RESTORE_ON_SLEEP))
		return;

	p->dyn_prio.dyn_switched_flag &= ~RESTORE_ON_SLEEP;
	restore_sched_attr(p);
}

/* Called from p sleeping. */
void reset_dyn_prio_on_sleep(struct task_struct *p)
{
	if (unlikely(p->dyn_prio.dyn_switched_flag & RESTORE_ON_SLEEP))
		queue_irq_work(reset_on_sleep, p);
}
