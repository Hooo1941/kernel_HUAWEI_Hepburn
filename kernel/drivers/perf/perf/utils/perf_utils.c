
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/cpufreq_times.h>
#include <linux/cgroup.h>
#include <../../../kernel/sched/sched.h>
#include "perf_utils.h"

struct task_struct *perf_get_task_by_pid(int upid)
{
	struct pid *pid;

	pid = find_vpid(upid);

	return pid_task(pid, PIDTYPE_PID);
}

int perf_get_cpu_num(void)
{
	int core_num = 0;
	int i = 0;

	for_each_possible_cpu(i)
		core_num++;

	return core_num;
}

int perf_get_cpuset(int upid, struct task_struct *tsk, char *buf)
{
	struct cgroup_subsys_state *css;
	int retval = -EINVAL;
	struct pid *pid;

	if (tsk == NULL || buf == NULL) {
		pr_err("PerfD %s: get cpuset err, para err\n", __func__);
		return retval;
	}

	pid = find_vpid(upid);
	css = task_get_css(tsk, cpuset_cgrp_id);
	retval = cgroup_path_ns(css->cgroup, buf, PATH_MAX,
		current->nsproxy->cgroup_ns);
	css_put(css);
	if (retval >= PATH_MAX)
		retval = -ENAMETOOLONG;
	if (retval < 0)
		return retval;
	return 0;
}

/* unit of the return value: ns */
static unsigned long long perf_get_per_cpu_time(struct cpu_freqs *freqs,
	struct task_struct *p, struct task_struct *child)
{
	unsigned int i;
	unsigned long long cputime = 0;

	for (i = 0; i < freqs->max_state; i++) {
		rcu_read_lock();
		do {
			if (freqs->offset + i < child->max_state &&
				child->time_in_state)
				cputime += child->time_in_state[freqs->offset + i];
		} while_each_thread(p, child);
		rcu_read_unlock();
	}

	return cputime;
}

void perf_get_cpu_time(struct task_struct *p, unsigned long long *cpu_time)
{
	spinlock_t *task_time_in_state_lock;
	unsigned int cpu;
	unsigned long flags;
	struct cpu_freqs *freqs;
	struct cpu_freqs *last_freqs = NULL;
	struct task_struct *child = p;
	struct cpu_freqs **all_freqs;

	all_freqs = cpufreq_get_all_freqs();
	task_time_in_state_lock = cpufreq_get_time_in_state_lock();

	spin_lock_irqsave(task_time_in_state_lock, flags);
	for_each_possible_cpu(cpu) {
		freqs = all_freqs[cpu];
		if (!freqs || freqs == last_freqs)
			continue;
		last_freqs = freqs;
		cpu_time[cpu] = perf_get_per_cpu_time(freqs, p, child);
	}
	spin_unlock_irqrestore(task_time_in_state_lock, flags);
}

/* refer to stat.c->get_iowait_time() */
static u64 perf_sysinfo_get_iowait_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 iowait = -1ULL;
	u64 iowait_usecs = -1ULL;

	if (cpu_online(cpu))
		iowait_usecs = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcs->cpustat[CPUTIME_IOWAIT];
	else
		iowait = iowait_usecs * NSEC_PER_USEC;

	return iowait;
}

#ifdef arch_idle_time

static u64 get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle;

	idle = kcs->cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

#else

static u64 get_idle_time(struct kernel_cpustat *kcs, int cpu)
{
	u64 idle = -1ULL;
	u64 idle_usecs = -1ULL;

	if (cpu_online(cpu))
		idle_usecs = get_cpu_idle_time_us(cpu, NULL);

	if (idle_usecs == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcs->cpustat[CPUTIME_IDLE];
	else
		idle = idle_usecs * NSEC_PER_USEC;

	return idle;
}

#endif

void perf_sysinfo_get_per_core_cpu_time(
	struct perf_cpu_time_info *per_cpu_time, int cpu)
{
	struct kernel_cpustat kcpustat;
	u64 *cpustat = kcpustat.cpustat;

	kcpustat_cpu_fetch(&kcpustat, cpu);

	per_cpu_time->user    = cpustat[CPUTIME_USER];
	per_cpu_time->nice    = cpustat[CPUTIME_NICE];
	per_cpu_time->system  = cpustat[CPUTIME_SYSTEM];
	per_cpu_time->idle    = get_idle_time(&kcpustat, cpu);
	per_cpu_time->iowait  = perf_sysinfo_get_iowait_time(&kcpustat, cpu);
	per_cpu_time->irq     = cpustat[CPUTIME_IRQ];
	per_cpu_time->softirq = cpustat[CPUTIME_SOFTIRQ];
}