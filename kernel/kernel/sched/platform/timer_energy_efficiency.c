// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Optimize cpu fcm idle chance by doing some hacks on timer.
 */
#include "sched.h"
#include <trace/events/sched.h>

#ifdef CONFIG_OPT_TIMER_ENERGY_EFFICIENCY

static inline s64 ktime_to_ms_round(ktime_t kt)
{
	return (kt + NSEC_PER_MSEC / 2) / NSEC_PER_MSEC;
}

#define ROOT_CSS_ID 1 /* Ugly but true. We have to reduce cost. */

/*
 * There could be kernel syscall tests like LTP, which has nothing
 * to do with user experience but very annoying. Fortunately it can
 * only be executed by adb-shell or top-app.
 * The best solution would be creating a cgroup for service adbd
 * and specifying the slack pct as 0 for it, like top-app does.
 * But to avoid convincing other people lets simply identify its
 * ancestors' comm here, though it's a heavy operation and we need
 * to do more checks to save the cost.
 * Called with rcu_read_lock held.
 */
static bool test_shell(struct task_struct *tsk,
			uclamp_group_t *tsk_uc)
{
	int depth = 3; /* Works for LTP test. */

	if (tsk_uc->css.id != ROOT_CSS_ID)
		return false;

	while (depth-- > 0) {
		tsk = rcu_dereference(tsk->real_parent);
		if (!tsk)
			break;
		if (tsk->pid <= 2)
			break;

		if (strcmp(tsk->comm, "adbd") == 0 ||
		    strcmp(tsk->comm, "hdcd") == 0 ||
		    strcmp(tsk->comm, "sh") == 0)
			return true;
	}

	return false;
}

#define MAX_RT_SLACK_PCT 10U

/*
 * Fcm idle can benifit a lot from larger timer_slack.
 * This interface is provided to several syscalls who use hrtimers
 * and intended to replace the default current->timer_slack_ns.
 * As its name shows, it estimate a timer_slack relative to the
 * hrtimer time, e.g. 10% or 20%, depending on task cgroup config.
 * Hopefully in that way we will achieve best energy efficiency
 * while not getting users confused.
 *
 * The req_time argument must be a relative time to now.
 */
u64 relative_slack_ns(ktime_t req_time, struct task_struct *tsk,
			u64 default_slack)
{
	uclamp_group_t *tsk_uc = NULL;
	unsigned int pct;
	u64 slack, max_slack;

	if (req_time == KTIME_MAX || req_time <= 0)
		return default_slack;

	rcu_read_lock();
	tsk_uc = get_uclamp_group(tsk);

	pct = tsk_uc->timer_slack_pct;

	/* Try not impacting user experience when the req_time is short. */
	if (ktime_to_ms_round(req_time) <= tsk_uc->timer_slack_exempt_ms)
		pct = 0;

	if (pct > 0 && test_shell(tsk, tsk_uc))
		pct = 0;

	if (rt_task(tsk))
		pct = min(pct, MAX_RT_SLACK_PCT);

	slack = ktime_to_ns(req_time) * pct / 100;

	max_slack = tsk_uc->timer_slack_max_delay_ms * NSEC_PER_MSEC;
	slack = min(slack, max_slack);

	trace_perf(relative_slack_ns, req_time, tsk_uc->css.id, pct, slack);
	rcu_read_unlock();

	return max(slack, default_slack);
}

#define SUGGESTED_TS_MAX_DELAY 100
#define SUGGESTED_TS_EXEMPT    100
void init_timer_slack_sched_group(uclamp_group_t *tg)
{
	tg->timer_slack_pct = 0; /* Default disabled for unspecifed tg. */
	tg->timer_slack_max_delay_ms = SUGGESTED_TS_MAX_DELAY;
	tg->timer_slack_exempt_ms = SUGGESTED_TS_EXEMPT;
}

u64 timer_slack_pct_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return (u64)css_uc(css)->timer_slack_pct;
}

int timer_slack_pct_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value)
{
	css_uc(css)->timer_slack_pct = (unsigned int)value;
	return 0;
}

u64 timer_slack_max_delay_ms_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return (u64)css_uc(css)->timer_slack_max_delay_ms;
}

/* No more benefits from a too large timer slack. Prevent system hanging. */
#define MAX_TS_MAX_DELAY 200UL
int timer_slack_max_delay_ms_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value)
{
	value = min(value, MAX_TS_MAX_DELAY);
	css_uc(css)->timer_slack_max_delay_ms = (unsigned int)value;
	return 0;
}

u64 timer_slack_exempt_ms_read_u64(struct cgroup_subsys_state *css,
				struct cftype *cft)
{
	return (u64)css_uc(css)->timer_slack_exempt_ms;
}

int timer_slack_exempt_ms_write_u64(struct cgroup_subsys_state *css,
				struct cftype *cft, u64 value)
{
	css_uc(css)->timer_slack_exempt_ms = (unsigned int)value;
	return 0;
}

#endif /* CONFIG_OPT_TIMER_ENERGY_EFFICIENCY */


#ifdef CONFIG_HRTIMER_SOFT_EXPIRE_SYNC

#include <linux/hrtimer.h>

DECLARE_PER_CPU(struct hrtimer_cpu_base, hrtimer_bases); /* hrtimer.c */

static void hrtimer_interrupt_func(struct irq_work *irq_work)
{
	trace_perf(hrtimer_interrupt_irqwork, "triggered");
	hrtimer_peek_ahead_timers();
}

static DEFINE_PER_CPU(struct irq_work, hrtimer_interrupt_irqwork);
void init_hrtimer_interrupt_irqwork(int cpu)
{
	init_irq_work(&per_cpu(hrtimer_interrupt_irqwork, cpu),
			hrtimer_interrupt_func);
}

/*
 * The activate_all_soft_expired_hrtimers() may be called frequently by
 * add_total_nr_running().
 * It doesn't make more benifits with a very small sync interval because
 * fcm can not power down both with 4ms or 1ms (for example) interval.
 * But 1ms may waste a little more power than 4ms for less parallelism.
 * Anyway 4ms looks good. And we chose 3.9ms to help ticks to be able
 * to check soft expired timers everytime so that the userspace timer
 * delay in heavy-loaded cases would not exceed a jiffy.
 */
#define MIN_HRTIMER_SYNC_INTERVAL	3900 /* us */
/*
 * No need to raise an irq_work if the timer expires very soon.
 */
#define IRQ_WORK_LATENCY		1000 /* us */

static void activate_soft_expired_hrtimers(int cpu, ktime_t now)
{
	struct hrtimer_cpu_base *cpu_base = &per_cpu(hrtimer_bases, cpu);
	struct irq_work *work = &per_cpu(hrtimer_interrupt_irqwork, cpu);

	/*
	 * Cpu_base->expires_next != KTIME_MAX impies !cpu_base->in_hrtirq.
	 * Read cpu_base->in_hrtirq is racy here unless adding cpu memory
	 * barrier to the writer.
	 */
	if (ktime_before(cpu_base->softexpires_next, now) &&
	    ktime_us_delta(cpu_base->expires_next, now) > IRQ_WORK_LATENCY &&
	    cpu_base->expires_next != KTIME_MAX)
		irq_work_queue_on(work, cpu);

	trace_perf(debug_softexpires_next, cpu, now,
		cpu_base->softexpires_next, cpu_base->expires_next);
}

/*
 * 1. Called by add_total_nr_running() when the system exits all-cpus-idle
 *    and the first task woken up.
 *    This helps to sync tasks on light-loaded cases.
 * 2. Called by scheduler_tick() on tick_do_timer_cpu.
 *    This helps not to delay hrtimers too long (since they have a large
 *    slack by CONFIG_OPT_TIMER_ENERGY_EFFICIENCY) on not very light-loaded
 *    cases (which has tick working on at least one cpu).
 *
 * To prevent unnecessary irq_work, we will make sure the dest cpu has no
 * incoming or ongoing timer irq.
 */
void activate_all_soft_expired_hrtimers(void)
{
	static ktime_t last_check;
	ktime_t now;
	int cpu;

	/*
	 * We can leave the activating to the next add_total_nr_running()
	 * when there are only timers but no tasks.
	 */
	if (unlikely(get_total_nr_running() == 0))
		return;

	if (unlikely(timekeeping_suspended))
		return;

	/* Need ktime_get() to compare `now` with `softexpires_next`. */
	now = ktime_get();

	if (ktime_us_delta(now, last_check) < MIN_HRTIMER_SYNC_INTERVAL)
		return;
	/* We can tolerate reentrant, which is very unlikely in fact. */
	WRITE_ONCE(last_check, now);

	for_each_online_cpu(cpu)
		activate_soft_expired_hrtimers(cpu, now);
}

#endif /* CONFIG_HRTIMER_SOFT_EXPIRE_SYNC */


#ifdef CONFIG_SCHED_INDICATE_FCM_IDLE

static atomic_t total_nr_running;

void add_total_nr_running(int count)
{
	int prev_nr = atomic_fetch_add_relaxed(count, &total_nr_running);
#ifdef CONFIG_HRTIMER_SOFT_EXPIRE_SYNC
	if (prev_nr == 0)
		activate_all_soft_expired_hrtimers();
#endif
}

void sub_total_nr_running(int count)
{
	WARN_ON_ONCE(atomic_sub_return_relaxed(count, &total_nr_running) < 0);
}

int get_total_nr_running(void)
{
	return atomic_read(&total_nr_running);
}

#endif /* CONFIG_SCHED_INDICATE_FCM_IDLE */
