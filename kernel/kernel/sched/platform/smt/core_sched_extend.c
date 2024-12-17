// SPDX-License-Identifier: GPL-2.0-only

#include <linux/prctl.h>
#include <../kernel/sched/sched.h>

#define CREATE_TRACE_POINTS
#include <trace/events/core_sched.h>

struct sched_core_cookie root_core_cookie = {
	.refcnt = 1,
};

struct sched_core_cookie other_core_cookie = {
	.refcnt = 1,
};

#define UID_ROOT_SYSTEM	10000
unsigned int sched_core_set_cookie_by_uid = 1;

static inline unsigned long get_cookie(uid_t uid)
{
	if (sched_core_set_cookie_by_uid)
		return (uid <= UID_ROOT_SYSTEM) ?
			(unsigned long)&root_core_cookie :
			(unsigned long)&other_core_cookie;

	return (unsigned long)&root_core_cookie;
}

#ifdef CONFIG_SCHED_CORE_PREFER
static struct sched_core_prefer *sched_core_alloc_prefer(void)
{
	struct sched_core_prefer *cp = kzalloc(sizeof(*cp), GFP_KERNEL);

	if (cp)
		refcount_set(&cp->refcnt, 1);

	return cp;
}

static struct sched_core_prefer *sched_core_get_prefer(struct sched_core_prefer *cp)
{
	if (cp)
		refcount_inc(&cp->refcnt);

	return cp;
}

static void sched_core_put_prefer(struct sched_core_prefer *cp)
{
	if (cp) {
		if (refcount_dec_and_test(&cp->refcnt))
			kfree(cp);
	}
}

void sched_core_prefer_enqueue(struct rq *rq, struct task_struct *p)
{
	if (p->core_prefer) {
		struct sched_core_prefer *cp = (struct sched_core_prefer *)p->core_prefer;

		atomic_inc(&cp->total_active);
		if (!atomic_inc_and_test(&cp->nr_active[cpu_of(rq)]))
			cpumask_set_cpu(cpu_of(rq), &cp->active_mask);
	}
}

void sched_core_prefer_dequeue(struct rq *rq, struct task_struct *p)
{
	if (p->core_prefer) {
		struct sched_core_prefer *cp = (struct sched_core_prefer *)p->core_prefer;

		atomic_dec(&cp->total_active);
		if (atomic_dec_and_test(&cp->nr_active[cpu_of(rq)]))
			cpumask_clear_cpu(cpu_of(rq), &cp->active_mask);
	}
}

int find_core_prefer_cpu(struct task_struct *p)
{
	cpumask_t search_cpus, core_prefer_cpus, fit_cpus;
	unsigned long target_cap = ULONG_MAX;
	long max_spare_cap = LONG_MIN;
	int i;
	int target_cpu = -1;
	bool boosted;
	struct sched_core_prefer *cp = (struct sched_core_prefer *)p->core_prefer;

	if (!cp)
		return -1;

	if (cpumask_empty(&cp->active_mask))
		return -1;

	/* It's hard to make decision with cp->active_mask for a queued task. */
	if (p->state != TASK_WAKING)
		return -1;

	cpumask_clear(&core_prefer_cpus);
	for_each_cpu(i, &cp->active_mask)
		cpumask_or(&core_prefer_cpus, &core_prefer_cpus, cpu_smt_mask(i));
	cpumask_andnot(&core_prefer_cpus, &core_prefer_cpus, &cp->active_mask);

	get_task_fit_cpus(p, &fit_cpus);
	boosted = task_placement_boosted(p);

	cpumask_and(&search_cpus, &core_prefer_cpus, &fit_cpus);
	target_cpu = find_best_candidate(p, &search_cpus, boosted);
	if (target_cpu > 0)
		goto out;

	cpumask_andnot(&search_cpus, &core_prefer_cpus, &fit_cpus);
	target_cpu = find_best_candidate(p, &search_cpus, true);

out:
	trace_perf(find_core_prefer_cpu, p, &cp->active_mask, target_cpu);
	return target_cpu;
}

/*
 * sched_core_update_prefer - replace the core_prefer on a task
 * @p: the task to update
 * @cp: the new core_prefer
 *
 * Effectively exchange the task core_prefer; caller is responsible for lifetimes on
 * both ends.
 *
 * Returns: the old core_prefer
 */
static struct sched_core_prefer *sched_core_update_prefer(struct task_struct *p,
							  struct sched_core_prefer *cp)
{
	struct sched_core_prefer *old_cp = NULL;
	struct rq_flags rf;
	struct rq *rq;
	int queued;

	rq = task_rq_lock(p, &rf);
	queued = task_on_rq_queued(p);
	if (queued)
		sched_core_prefer_dequeue(rq, p);

	old_cp = p->core_prefer;
	p->core_prefer = cp;

	if (queued)
		sched_core_prefer_enqueue(rq, p);

	task_rq_unlock(rq, p, &rf);

	return old_cp;
}

static void __sched_core_set_prefer(struct task_struct *p,
				    struct sched_core_prefer *new_cp)

{
	struct sched_core_prefer *cp = sched_core_get_prefer(new_cp);
	cp = sched_core_update_prefer(p, cp);
	sched_core_put_prefer(cp);
}

static struct sched_core_prefer *sched_core_clone_prefer(struct task_struct *p)
{
	bool new_cp = false;
	unsigned long flags;
	struct sched_core_prefer *cp = NULL;

	raw_spin_lock_irqsave(&p->pi_lock, flags);
	if (!p->core_prefer)
		new_cp = true;
	else
		cp = p->core_prefer;

	raw_spin_unlock_irqrestore(&p->pi_lock, flags);

	if (new_cp) {
		cp = sched_core_alloc_prefer();
		sched_core_update_prefer(p, cp);
	}

	cp = sched_core_get_prefer(cp);

	return cp;
}

int sched_core_share_prefer(struct task_struct *to, struct task_struct *from)
{
	struct sched_core_prefer *cp = sched_core_clone_prefer(from);

	if (!cp)
		return -ENOMEM;

	cp = sched_core_update_prefer(to, cp);
	sched_core_put_prefer(cp);

	return 0;
}

int sched_core_share_prefer_to(struct task_struct *to, enum pid_type type)
{
	int ret = 0;
	struct sched_core_prefer *cp = sched_core_clone_prefer(current);
	struct task_struct *p = NULL;
	struct pid *grp = NULL;

	if (!cp)
		return -ENOMEM;

	if (type == PIDTYPE_PID) {
		__sched_core_set_prefer(to, cp);
		goto out;
	}

	read_lock(&tasklist_lock);
	grp = task_pid_type(to, type);

	do_each_pid_thread(grp, type, p) {
		if (!ptrace_may_access(p, PTRACE_MODE_READ_REALCREDS)) {
			ret = -EPERM;
			goto out_tasklist;
		}
	} while_each_pid_thread(grp, type, p);

	do_each_pid_thread(grp, type, p) {
		__sched_core_set_prefer(p, cp);
	} while_each_pid_thread(grp, type, p);
out_tasklist:
	read_unlock(&tasklist_lock);

out:
	sched_core_put_prefer(cp);

	return ret;
}

int sched_core_share_prefer_from(struct task_struct *p)
{
	return sched_core_share_prefer(current, p);
}

int sched_core_prefer_destroy(struct task_struct *task, enum pid_type type)
{
	int ret = 0;
	struct task_struct *p = NULL;
	struct pid *grp = NULL;

	if (type == PIDTYPE_PID) {
		__sched_core_set_prefer(task, NULL);
		goto out;
	}

	read_lock(&tasklist_lock);
	grp = task_pid_type(task, type);

	do_each_pid_thread(grp, type, p) {
		if (!ptrace_may_access(p, PTRACE_MODE_READ_REALCREDS)) {
			ret = -EPERM;
			goto out_tasklist;
		}
	} while_each_pid_thread(grp, type, p);

	do_each_pid_thread(grp, type, p) {
		__sched_core_set_prefer(p, NULL);
	} while_each_pid_thread(grp, type, p);
out_tasklist:
	read_unlock(&tasklist_lock);

out:
	return ret;
}

int sched_core_prefer_share_pid(unsigned int cmd, pid_t pid, enum pid_type type,
			   unsigned long uaddr)
{
	unsigned long id = 0;
	struct sched_core_prefer *cp = NULL;
	struct task_struct *task, *p;
	struct pid *grp;
	int err = 0;

	if (!static_branch_likely(&sched_smt_present))
		return -ENODEV;

	if (type > PIDTYPE_PGID || cmd >= PR_SCHED_CORE_PREFER_MAX || pid < 0)
		return -EINVAL;

	rcu_read_lock();
	if (pid == 0) {
		task = current;
	} else {
		task = find_task_by_vpid(pid);
		if (!task) {
			rcu_read_unlock();
			return -ESRCH;
		}
	}
	get_task_struct(task);
	rcu_read_unlock();

	/*
	 * Check if this process has the right to modify the specified
	 * process. Use the regular "ptrace_may_access()" checks.
	 */
	if (!ptrace_may_access(task, PTRACE_MODE_READ_REALCREDS)) {
		err = -EPERM;
		goto out;
	}

	switch (cmd) {
	case PR_SCHED_CORE_PREFER_SHARE_TO:
		err = sched_core_share_prefer_to(task, type);
		break;

	case PR_SCHED_CORE_PREFER_SHARE_FROM:
		if (type != PIDTYPE_PID) {
			err = -EINVAL;
			goto out;
		}
		err = sched_core_share_prefer_from(task);
		break;

	case PR_SCHED_CORE_PREFER_DESTROY:
		err = sched_core_prefer_destroy(task, type);
		break;

	default:
		err = -EINVAL;
		goto out;
	}

out:
	put_task_struct(task);
	return err;
}
#endif

static unsigned long sched_core_alloc_cookie(void)
{
	struct sched_core_cookie *ck = kmalloc(sizeof(*ck), GFP_KERNEL);
	if (!ck)
		return 0;

	refcount_set(&ck->refcnt, 1);
#ifndef CONFIG_SMT_ENABLE_CORE_SCHED
	sched_core_get();
#endif

	return (unsigned long)ck;
}

static void sched_core_put_cookie(unsigned long cookie)
{
	struct sched_core_cookie *ptr = (void *)cookie;

	if (ptr && refcount_dec_and_test(&ptr->refcnt)) {
		kfree(ptr);
#ifndef CONFIG_SMT_ENABLE_CORE_SCHED
		sched_core_put();
#endif
	}
}

static unsigned long sched_core_get_cookie(unsigned long cookie)
{
	struct sched_core_cookie *ptr = (void *)cookie;

	if (ptr)
		refcount_inc(&ptr->refcnt);

	return cookie;
}

/*
 * sched_core_update_cookie - replace the cookie on a task
 * @p: the task to update
 * @cookie: the new cookie
 *
 * Effectively exchange the task cookie; caller is responsible for lifetimes on
 * both ends.
 *
 * Returns: the old cookie
 */
static unsigned long sched_core_update_cookie(struct task_struct *p,
					      unsigned long cookie)
{
	unsigned long old_cookie;
	struct rq_flags rf;
	struct rq *rq;
	bool enqueued;

	rq = task_rq_lock(p, &rf);

#ifndef CONFIG_SMT_ENABLE_CORE_SCHED
	/*
	 * Since creating a cookie implies sched_core_get(), and we cannot set
	 * a cookie until after we've created it, similarly, we cannot destroy
	 * a cookie until after we've removed it, we must have core scheduling
	 * enabled here.
	 */
	SCHED_WARN_ON((p->core_cookie || cookie) && !sched_core_enabled(rq));
#endif

	enqueued = sched_core_enqueued(p);
	if (enqueued)
		sched_core_dequeue(rq, p, DEQUEUE_SAVE);

	old_cookie = p->core_cookie;
	p->core_cookie = cookie;

	if (enqueued)
		sched_core_enqueue(rq, p);

	/*
	 * If task is currently running, it may not be compatible anymore after
	 * the cookie change, so enter the scheduler on its CPU to schedule it
	 * away.
	 *
	 * Note that it is possible that as a result of this cookie change, the
	 * core has now entered/left forced idle state. Defer accounting to the
	 * next scheduling edge, rather than always forcing a reschedule here.
	 */
	if (task_running(rq, p))
		resched_curr(rq);

	task_rq_unlock(rq, p, &rf);
	trace_perf(task_update_cookie, p, old_cookie, cookie);

	return old_cookie;
}

static unsigned long sched_core_clone_cookie(struct task_struct *p)
{
	unsigned long cookie, flags;

	raw_spin_lock_irqsave(&p->pi_lock, flags);
	cookie = sched_core_get_cookie(p->core_cookie);
	raw_spin_unlock_irqrestore(&p->pi_lock, flags);

	return cookie;
}

void sched_core_fork(struct task_struct *p)
{
	RB_CLEAR_NODE(&p->core_node);

	p->core_cookie = get_cookie(task_uid(p).val);
	sched_core_get_cookie(p->core_cookie);

	if (p->core_cookie)
		trace_perf(task_update_cookie, p, -1, p->core_cookie);

#ifdef CONFIG_SCHED_CORE_PREFER
	p->core_prefer = NULL;
#endif
}

void sched_core_free(struct task_struct *p)
{
	sched_core_put_cookie(p->core_cookie);
#ifdef CONFIG_SCHED_CORE_PREFER
	sched_core_put_prefer(p->core_prefer);
#endif
}

static void __sched_core_set(struct task_struct *p, unsigned long cookie)
{
	cookie = sched_core_get_cookie(cookie);
	cookie = sched_core_update_cookie(p, cookie);
	sched_core_put_cookie(cookie);
}

void sched_core_setuid(struct task_struct *p, uid_t new_uid)
{
	unsigned long new_core_cookie = get_cookie(new_uid);

	if (p->core_cookie != new_core_cookie)
		__sched_core_set(p, new_core_cookie);
}

/* Called from prctl interface: PR_SCHED_CORE */
int sched_core_share_pid(unsigned int cmd, pid_t pid, enum pid_type type,
			 unsigned long uaddr)
{
	unsigned long cookie = 0, id = 0;
	struct task_struct *task, *p;
	struct pid *grp;
	int err = 0;

	if (!static_branch_likely(&sched_smt_present))
		return -ENODEV;

	if (type > PIDTYPE_PGID || cmd >= PR_SCHED_CORE_MAX || pid < 0 ||
	    (cmd != PR_SCHED_CORE_GET && uaddr))
		return -EINVAL;

	rcu_read_lock();
	if (pid == 0) {
		task = current;
	} else {
		task = find_task_by_vpid(pid);
		if (!task) {
			rcu_read_unlock();
			return -ESRCH;
		}
	}
	get_task_struct(task);
	rcu_read_unlock();

	/*
	 * Check if this process has the right to modify the specified
	 * process. Use the regular "ptrace_may_access()" checks.
	 */
	if (!ptrace_may_access(task, PTRACE_MODE_READ_REALCREDS)) {
		err = -EPERM;
		goto out;
	}

	switch (cmd) {
	case PR_SCHED_CORE_GET:
		if (type != PIDTYPE_PID || uaddr & 7) {
			err = -EINVAL;
			goto out;
		}
		cookie = sched_core_clone_cookie(task);
		if (cookie) {
			/* XXX improve ? */
			ptr_to_hashval((void *)cookie, &id);
		}
		err = put_user(id, (u64 __user *)uaddr);
		goto out;

	case PR_SCHED_CORE_CREATE:
		cookie = sched_core_alloc_cookie();
		if (!cookie) {
			err = -ENOMEM;
			goto out;
		}
		break;

	case PR_SCHED_CORE_SHARE_TO:
		cookie = sched_core_clone_cookie(current);
		break;

	case PR_SCHED_CORE_SHARE_FROM:
		if (type != PIDTYPE_PID) {
			err = -EINVAL;
			goto out;
		}
		cookie = sched_core_clone_cookie(task);
		__sched_core_set(current, cookie);
		goto out;

	default:
		err = -EINVAL;
		goto out;
	}

	if (type == PIDTYPE_PID) {
		__sched_core_set(task, cookie);
		goto out;
	}

	read_lock(&tasklist_lock);
	grp = task_pid_type(task, type);

	do_each_pid_thread(grp, type, p) {
		if (!ptrace_may_access(p, PTRACE_MODE_READ_REALCREDS)) {
			err = -EPERM;
			goto out_tasklist;
		}
	} while_each_pid_thread(grp, type, p);

	do_each_pid_thread(grp, type, p) {
		__sched_core_set(p, cookie);
	} while_each_pid_thread(grp, type, p);
out_tasklist:
	read_unlock(&tasklist_lock);

out:
	sched_core_put_cookie(cookie);
	put_task_struct(task);
	return err;
}

#ifdef CONFIG_SCHEDSTATS

/* REQUIRES: rq->core's clock recently updated. */
void __sched_core_account_forceidle(struct rq *rq)
{
	const struct cpumask *smt_mask = cpu_smt_mask(cpu_of(rq));
	u64 delta, now = rq_clock(rq->core);
	struct rq *rq_i;
	struct task_struct *p;
	int i;

	lockdep_assert_rq_held(rq);

	if (!rq->core->core_forceidle_count ||
	    !rq->core->core_forceidle_occupation ||
	    rq->core->core_forceidle_start == 0)
		return;

	delta = now - rq->core->core_forceidle_start;
	if (unlikely((s64)delta <= 0))
		return;

	rq->core->core_forceidle_start = now;

	if (rq->core->core_forceidle_count > 1 ||
	    rq->core->core_forceidle_occupation > 1) {
		/*
		 * For larger SMT configurations, we need to scale the charged
		 * forced idle amount since there can be more than one forced
		 * idle sibling and more than one running cookied task.
		 */
		delta *= rq->core->core_forceidle_count;
		delta = div_u64(delta, rq->core->core_forceidle_occupation);
	}

#ifdef CONFIG_SCHED_SMT_EXPELLING
	if (rq->core->core_smt_expeller)
		rq->core_forceidle_time[1] += delta;
	else
#endif
		rq->core_forceidle_time[0] += delta;

	if (!schedstat_enabled())
		return;

	for_each_cpu(i, smt_mask) {
		rq_i = cpu_rq(i);
		p = rq_i->core_pick ?: rq_i->curr;

		if (p == rq_i->idle)
			continue;

		__schedstat_add(p->se.statistics.core_forceidle_sum, delta);
	}
}

void __sched_core_tick(struct rq *rq)
{
	if (!rq->core->core_forceidle_count)
		return;

	if (rq != rq->core)
		update_rq_clock(rq->core);

	__sched_core_account_forceidle(rq);
}

#endif /* CONFIG_SCHEDSTATS */
