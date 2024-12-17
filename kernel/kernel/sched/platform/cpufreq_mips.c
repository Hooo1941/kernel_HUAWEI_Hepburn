/*
 * cpufreq_mips.c
 *
 * for hisilicon efficinecy control algorithm cpufreq mips governor.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/cpu_pm.h>
#ifdef CONFIG_CPU_FREQ_PID_MIPS
#include "pid_freq.h"
#endif
#define CREATE_TRACE_POINTS
#include <trace/events/cpufreq_mips.h>
#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#ifdef CONFIG_HECA
#include <linux/platform_drivers/heca.h>
#endif

#define DEFAULT_IPC_TARGET_LOAD 80
#define DEFAULT_IPC_MAX_FREQ    UINT_MAX
#define DEFAULT_CPU_TIMER_SLACK 5000
#define DEFAULT_CPUFREQ_MIPS_WINDOW_SIZE	4000000
static unsigned int default_ipc_target_loads[] = {DEFAULT_IPC_TARGET_LOAD};
static unsigned int default_ipc_max_freq[] = {DEFAULT_IPC_MAX_FREQ};
static unsigned int default_cpu_timer_slack = DEFAULT_CPU_TIMER_SLACK;

struct mipsgov_tunables {
	struct gov_attr_set attr_set;
	spinlock_t ipc_target_loads_lock;
	unsigned int *ipc_target_loads;
	int nipc_target_loads;
	spinlock_t ipc_max_freq_lock;
	unsigned int *ipc_max_freq;
	int nipc_max_freq;
	int cpu_timer_slack;
	u64 cpufreq_mips_window_size_ns;
};

struct mipsgov_policy {
	struct cpufreq_policy *policy;
	struct mipsgov_tunables *tunables;
	raw_spinlock_t update_lock; /* For shared policies */
	struct list_head tunables_hook;
	unsigned int next_freq;
	struct irq_work irq_work;
	struct kthread_work work;
	struct kthread_work sysfs_work;
	struct kthread_worker worker;
	struct task_struct *thread;
	raw_spinlock_t cpu_timer_lock;
	bool work_in_progress;
	bool governor_enabled;
};

struct mipsgov_cpu {
	struct mipsgov_policy *mg_policy;
	unsigned int cpu;
	unsigned int mips_freq;
	unsigned int flags;
	struct timer_list cpu_slack_timer;
	bool enabled;
};

static DEFINE_PER_CPU(struct mipsgov_cpu, mipsgov_cpu);

static struct mipsgov_tunables *global_tunables;
static DEFINE_MUTEX(global_tunables_lock);

static inline struct mipsgov_tunables *to_mipsgov_tunables(struct gov_attr_set *attr_set)
{
	return container_of(attr_set, struct mipsgov_tunables, attr_set);
}

static void mipsgov_work(struct kthread_work *work)
{
	struct mipsgov_policy *mg_policy = container_of(work, struct mipsgov_policy, work);
	struct cpufreq_policy *policy = mg_policy->policy;

	mg_policy->work_in_progress = false;
	if (!mg_policy->governor_enabled)
		return;

#ifdef CONFIG_HECA
	unsigned int next_freq = heca_get_kernel_freq(policy, mg_policy->next_freq);
	__cpufreq_driver_target(mg_policy->policy, next_freq, CPUFREQ_RELATION_L);
#else
	__cpufreq_driver_target(mg_policy->policy, mg_policy->next_freq,
				CPUFREQ_RELATION_L);
#endif

	blocking_notifier_call_chain(&cpugov_status_notifier_list,
				     CPUGOV_ACTIVE, &policy->cpu);
}

static void mipsgov_irq_work(struct irq_work *irq_work)
{
	struct mipsgov_policy *mg_policy;

	mg_policy = container_of(irq_work, struct mipsgov_policy, irq_work);
	if (!mg_policy->governor_enabled)
		return;

	kthread_queue_work(&mg_policy->worker, &mg_policy->work);
}

static unsigned int ipc_to_targetload(struct mipsgov_tunables *tunables,
				      unsigned int ipc)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->ipc_target_loads_lock, flags);

	for (i = 0; i < tunables->nipc_target_loads - 1 &&
	     ipc >= tunables->ipc_target_loads[i + 1]; i += 2)
		;

	ret = tunables->ipc_target_loads[i];
	spin_unlock_irqrestore(&tunables->ipc_target_loads_lock, flags);
	return ret;
}

static unsigned int ipc_to_maxfreq(struct mipsgov_tunables *tunables,
				   unsigned int ipc)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->ipc_max_freq_lock, flags);

	for (i = 0; i < tunables->nipc_max_freq - 1 &&
	     ipc >= tunables->ipc_max_freq[i + 1]; i += 2)
		;

	ret = tunables->ipc_max_freq[i];
	spin_unlock_irqrestore(&tunables->ipc_max_freq_lock, flags);
	return ret;
}

static unsigned int mips_get_maxfreq(unsigned int cpu, unsigned int ipc)
{
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;

	if (!mg_policy || !mg_policy->governor_enabled)
		return DEFAULT_IPC_MAX_FREQ;
	return ipc_to_maxfreq(mg_policy->tunables, ipc);
}

static unsigned int mips_get_targetload(unsigned int cpu, unsigned int ipc)
{
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;
	unsigned int targetload = DEFAULT_IPC_TARGET_LOAD;

	if (!mg_policy || !mg_policy->governor_enabled)
		return DEFAULT_IPC_TARGET_LOAD;
	return ipc_to_targetload(mg_policy->tunables, ipc);
}

static ssize_t ipc_target_loads_show(struct gov_attr_set *attr_set, char *buf)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->ipc_target_loads_lock, flags);

	for (i = 0; i < tunables->nipc_target_loads; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", tunables->ipc_target_loads[i],
			       i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->ipc_target_loads_lock, flags);

	return ret;
}

static ssize_t ipc_target_loads_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);
	unsigned int *new_ipc_target_loads;
	unsigned long flags;
	int ntokens;
	int i;

	new_ipc_target_loads = cpufreq_get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_ipc_target_loads))
		return PTR_ERR(new_ipc_target_loads);

	for (i = 0; i < ntokens; i++) {
		if (new_ipc_target_loads[i] == 0) {
			kfree(new_ipc_target_loads);
			return -EINVAL;
		}
	}

	spin_lock_irqsave(&tunables->ipc_target_loads_lock, flags);
	if (tunables->ipc_target_loads != default_ipc_target_loads)
		kfree(tunables->ipc_target_loads);
	tunables->ipc_target_loads = new_ipc_target_loads;
	tunables->nipc_target_loads = ntokens;
	spin_unlock_irqrestore(&tunables->ipc_target_loads_lock, flags);

	return count;
}

static ssize_t ipc_max_freq_show(struct gov_attr_set *attr_set, char *buf)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->ipc_max_freq_lock, flags);

	for (i = 0; i < tunables->nipc_max_freq; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", tunables->ipc_max_freq[i],
			       i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->ipc_max_freq_lock, flags);

	return ret;
}

static ssize_t ipc_max_freq_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);
	unsigned int *new_ipc_max_freq;
	unsigned long flags;
	int ntokens;
	int i;

	new_ipc_max_freq = cpufreq_get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_ipc_max_freq))
		return PTR_ERR(new_ipc_max_freq);

	for (i = 0; i < ntokens; i++) {
		if (new_ipc_max_freq[i] == 0) {
			kfree(new_ipc_max_freq);
			return -EINVAL;
		}
	}

	spin_lock_irqsave(&tunables->ipc_max_freq_lock, flags);
	if (tunables->ipc_max_freq != default_ipc_max_freq)
		kfree(tunables->ipc_max_freq);
	tunables->ipc_max_freq = new_ipc_max_freq;
	tunables->nipc_max_freq = ntokens;
	spin_unlock_irqrestore(&tunables->ipc_max_freq_lock, flags);

	return count;
}

static ssize_t cpu_timer_slack_store(struct gov_attr_set *attr_set,
				     const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	tunables->cpu_timer_slack = val;
	return count;
}

static ssize_t cpu_timer_slack_show(struct gov_attr_set *attr_set, char *buf)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);

	return scnprintf(buf, PAGE_SIZE, "%u\n", tunables->cpu_timer_slack);
}

static ssize_t cpufreq_mips_window_size_ns_store(struct gov_attr_set *attr_set,
				     		 const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	if (val < MIN_MIPS_WINDOW_SIZE || val > MAX_MIPS_WINDOW_SIZE)
		return -EINVAL;

	tunables->cpufreq_mips_window_size_ns = val;
	return count;
}

static ssize_t cpufreq_mips_window_size_ns_show(struct gov_attr_set *attr_set, char *buf)
{
	struct mipsgov_tunables *tunables = to_mipsgov_tunables(attr_set);

	return scnprintf(buf, PAGE_SIZE, "%u\n", tunables->cpufreq_mips_window_size_ns);
}

static struct governor_attr ipc_target_loads = __ATTR_RW(ipc_target_loads);
static struct governor_attr ipc_max_freq = __ATTR_RW(ipc_max_freq);
static struct governor_attr cpu_timer_slack = __ATTR_RW(cpu_timer_slack);
static struct governor_attr cpufreq_mips_window_size_ns = __ATTR_RW(cpufreq_mips_window_size_ns);

#define SYSTEM_UID (uid_t)1000
#define SYSTEM_GID (uid_t)1000

#define INVALID_ATTR \
	{.name = NULL, .uid = (uid_t)(-1), .gid = (uid_t)(-1), .mode = 0000}
static struct attribute *mipsgov_attrs[] = {
	&ipc_target_loads.attr,
	&ipc_max_freq.attr,
	&cpu_timer_slack.attr,
	&cpufreq_mips_window_size_ns.attr,
	NULL
};
ATTRIBUTE_GROUPS(mipsgov);

static struct governor_user_attr mips_user_attrs[] = {
	{.name = "ipc_target_loads", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "ipc_max_freq", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "cpu_timer_slack", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "cpufreq_mips_window_size_ns", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},

	/* add below */
	INVALID_ATTR,
};

struct cpufreq_governor mips_gov;
static void mipsgov_sys(struct kthread_work *work)
{
	struct mipsgov_policy *mg_policy = container_of(work, struct mipsgov_policy, sysfs_work);
	struct cpufreq_policy *policy = mg_policy->policy;

	gov_sysfs_set_attr(policy->cpu, mips_gov.name, mips_user_attrs);
}

static void create_sys_set(struct mipsgov_policy *mg_policy)
{
	kthread_init_work(&mg_policy->sysfs_work, mipsgov_sys);
	kthread_queue_work(&mg_policy->worker, &mg_policy->sysfs_work);
}

static struct kobj_type mipsgov_tunables_ktype = {
	.default_groups = mipsgov_groups,
	.sysfs_ops = &governor_sysfs_ops,
};

static struct mipsgov_policy *mipsgov_policy_alloc(struct cpufreq_policy *policy)
{
	struct mipsgov_policy *mg_policy;
	int cpu;

	mg_policy = kzalloc(sizeof(*mg_policy), GFP_KERNEL);
	if (!mg_policy)
		return NULL;

	mg_policy->policy = policy;
	raw_spin_lock_init(&mg_policy->update_lock);
	raw_spin_lock_init(&mg_policy->cpu_timer_lock);

	return mg_policy;
}

static void mipsgov_policy_free(struct mipsgov_policy *mg_policy)
{
	kfree(mg_policy);
}

static int mipsgov_kthread_create(struct mipsgov_policy *mg_policy)
{
	struct task_struct *thread;
	struct sched_param param = { .sched_priority = MAX_USER_RT_PRIO / 2 };

	struct cpufreq_policy *policy = mg_policy->policy;
	int ret;

	kthread_init_work(&mg_policy->work, mipsgov_work);
	kthread_init_worker(&mg_policy->worker);
	thread = kthread_create(kthread_worker_fn, &mg_policy->worker,
				"mipsgov:%d",
				cpumask_first(policy->related_cpus));
	if (IS_ERR(thread)) {
		pr_err("failed to create mipsgov thread: %ld\n", PTR_ERR(thread));
		return PTR_ERR(thread);
	}

	ret = sched_setscheduler_nocheck(thread, SCHED_FIFO, &param);
	if (ret) {
		kthread_stop(thread);
		pr_warn("%s: failed to set SCHED_FIFO\n", __func__);
		return ret;
	}

	mg_policy->thread = thread;

	if (!policy->dvfs_possible_from_any_cpu)
		kthread_bind_mask(thread, policy->related_cpus);
	init_irq_work(&mg_policy->irq_work, mipsgov_irq_work);

	wake_up_process(thread);

	return 0;
}

static void mipsgov_kthread_stop(struct mipsgov_policy *mg_policy)
{
	kthread_flush_worker(&mg_policy->worker);
	kthread_stop(mg_policy->thread);
}

static struct mipsgov_tunables *mipsgov_tunables_alloc(struct mipsgov_policy *mg_policy)
{
	struct mipsgov_tunables *tunables;

	tunables = kzalloc(sizeof(*tunables), GFP_KERNEL);
	if (tunables) {
		gov_attr_set_init(&tunables->attr_set, &mg_policy->tunables_hook);
		if (!have_governor_per_policy())
			global_tunables = tunables;
	}
	return tunables;
}

static void mipsgov_tunables_free(struct mipsgov_tunables *tunables)
{
	if (!have_governor_per_policy())
		global_tunables = NULL;

	kfree(tunables);
}

static void mipsgov_tunables_init(struct cpufreq_policy *policy,
				  struct mipsgov_tunables *tunables)
{
	tunables->ipc_target_loads = default_ipc_target_loads;
	tunables->nipc_target_loads = ARRAY_SIZE(default_ipc_target_loads);
	tunables->ipc_max_freq = default_ipc_max_freq;
	tunables->nipc_max_freq = ARRAY_SIZE(default_ipc_max_freq);
	tunables->cpu_timer_slack = default_cpu_timer_slack;
	tunables->cpufreq_mips_window_size_ns = DEFAULT_CPUFREQ_MIPS_WINDOW_SIZE;

	spin_lock_init(&tunables->ipc_target_loads_lock);
	spin_lock_init(&tunables->ipc_max_freq_lock);
}

static void mipsgov_update_shared(struct mipsgov_cpu *mg_cpu)
{
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;

	if (!mg_policy->work_in_progress) {
		mg_policy->work_in_progress = true;
		irq_work_queue_any(&mg_policy->irq_work);
	}
}

static unsigned int mips_next_freq_shared(struct mipsgov_cpu *mg_cpu)
{
	int j;
	unsigned int next_freq = 0;
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;

	if (!mg_policy || !mg_policy->governor_enabled)
		return 0;
	struct cpufreq_policy *policy = mg_policy->policy;

	for_each_cpu(j, policy->cpus) {
		struct mipsgov_cpu *j_mg_cpu = &per_cpu(mipsgov_cpu, j);

		trace_perf(cpufreq_mips_freq, j, j_mg_cpu->mips_freq,
					mg_policy->next_freq, j_mg_cpu->flags);
		next_freq = max_t(unsigned int, next_freq, j_mg_cpu->mips_freq);
		j_mg_cpu->flags = 0;
	}
	return next_freq;
}

static bool check_mips_freq_change(struct mipsgov_cpu *mg_cpu)
{
	unsigned int next_freq;
	struct mipsgov_policy *mg_policy;
	bool mips_freq_change = true;
	unsigned long flags;

	mg_policy = mg_cpu->mg_policy;
	raw_spin_lock_irqsave(&mg_policy->update_lock, flags);
	next_freq = mips_next_freq_shared(mg_cpu);

#ifdef CONFIG_HECA
	next_freq = heca_get_next_freq(mg_policy->policy->cpu, false, next_freq);
#endif
	if (mg_policy->next_freq == next_freq) {
		mips_freq_change = false;
		goto unlock;
	}
	mg_policy->next_freq = next_freq;
unlock:
	raw_spin_unlock_irqrestore(&mg_policy->update_lock, flags);
	return mips_freq_change;
}

void mips_check_freq_update(int cpu, unsigned int flags)
{
	bool mips_changed = false;
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy;

	if (!mg_cpu->enabled)
		return;

	mg_policy = mg_cpu->mg_policy;
	if (!mg_policy || !mg_policy->governor_enabled)
		return;

	mg_cpu->flags |= flags;
	if ((flags & MIPS_UPDATE) || (flags & MIPS_MIGRATE))
		if (check_mips_freq_change(mg_cpu))
			mips_changed = true;
	if (flags & POLICY_MIN_RESTORE)
		mips_changed = true;
	if (mips_changed)
		mipsgov_update_shared(mg_cpu);
}

static void cpu_slack_timer_resched(struct mipsgov_cpu *mg_cpu)
{
	u64 expires;
	unsigned long flags;
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;

	raw_spin_lock_irqsave(&mg_policy->cpu_timer_lock, flags);
	if (!mg_policy || !mg_policy->governor_enabled)
		goto unlock;

	if (mg_policy->tunables->cpu_timer_slack >= 0 &&
	    mg_cpu->mips_freq > mg_policy->policy->min) {
		expires = jiffies + usecs_to_jiffies(mg_policy->tunables->cpu_timer_slack);
		mod_timer(&mg_cpu->cpu_slack_timer, expires);
	} else {
		del_timer(&mg_cpu->cpu_slack_timer);
	}
unlock:
	raw_spin_unlock_irqrestore(&mg_policy->cpu_timer_lock, flags);
}

/*
 * Update cpu mips freq which we find lowest freq at or above target in a table.
 * Update policy freq after updating cpu mips freq.
 * To avoid cpu mips freq at a high freq in idle state,
 * add a cpu timer when cpu change mips freq, that change mips freq to 0.
 */
static void mipsgov_change_mips_freq(unsigned int cpu, unsigned int freq)
{
	unsigned int new_freq;
	int index;
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy = mg_cpu->mg_policy;

	if (!mg_policy || !mg_policy->governor_enabled)
		return;
	struct cpufreq_policy *policy = mg_policy->policy;

	index = cpufreq_table_find_index_al(policy, freq);
	new_freq = policy->freq_table[index].frequency;
	mg_cpu->mips_freq = new_freq;
	cpu_slack_timer_resched(mg_cpu);
}

static void cpu_clear_mips_freq_timer(struct timer_list *timer)
{
	struct mipsgov_cpu *mg_cpu = container_of(timer, struct mipsgov_cpu, cpu_slack_timer);

	mg_cpu->mips_freq = 0;
	mips_check_freq_update(mg_cpu->cpu, MIPS_UPDATE);
}

/*
 * This function computes a new freq for the given CPU, to be change freq
 * new freq = inst / time * ipc
 * we should the max of taskfreq and cpufreq to guarante performance
 * we need limit the maxfreq to save power
 * if the ipc of curr cpu is little
 * which we think the patten is a memorybound pattern.
 */
void load_pred_mips_freq(int cpu, struct task_struct *p, int flags)
{
	u64 cpu_demand, task_demand;
	unsigned long rtg_util;
	unsigned int cpufreq, taskfreq, nextfreq, maxfreq, pidfreq, rtg_freq;
	struct rq *rq = cpu_rq(cpu);
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy;
	struct pcounts pcount;
	int nr_cpu_load_window;

	if (!mg_cpu->enabled)
		return;

	mg_policy = mg_cpu->mg_policy;
	if (!mg_policy || !p->tmipsd)
		return;

	u64 cpu_window_size = mg_policy->tunables->cpufreq_mips_window_size_ns;
	u64 now = sched_ktime_clock();
	unsigned int tl = mips_get_targetload(cpu, rq->rmipsd.curr_avg_ipc);

	cpu_demand = cpu_inst_demand(cpu);
	task_demand = task_inst_demand(p);
	cpufreq = div_u64_protected(cpu_demand * (NSEC_PER_SEC / mips_window_size), cpu_rq(cpu)->rmipsd.curr_avg_ipc);
	taskfreq = div_u64_protected(task_demand * (NSEC_PER_SEC / mips_window_size), p->tmipsd->curr_avg_ipc);
	nextfreq = max_t(unsigned int, cpufreq, taskfreq);
	maxfreq = mips_get_maxfreq(cpu, rq->rmipsd.curr_avg_ipc);

	int nr_window = update_mips_window_start(&cpu_rq(cpu)->mips_cpu_window_start, cpu_window_size, now);
	if (nr_window) {
		nr_cpu_load_window = cpu_window_size / mips_window_size;
		get_mips_mem_info(cpu, &pcount, nr_cpu_load_window);
		cpufreq = pcount.cycle * 100 / tl / (cpu_window_size / NSEC_PER_MSEC);
		nextfreq = max_t(unsigned int, cpufreq, nextfreq);
		trace_perf(mips_cpu_freq, cpu_rq(cpu), pcount.cycle, cpufreq, nextfreq, tl, now, flags);
	} else {
		nextfreq = max_t(unsigned int, nextfreq, mg_cpu->mips_freq);
	}
	nextfreq = min_t(unsigned int, nextfreq, maxfreq);
#ifdef CONFIG_CPU_FREQ_PID_MIPS
	if (pid_control_update) {
		pidfreq = update_pid_freq(cpu, flags, div_u64_protected(rq->rmipsd.curr_inst * INST_PER_KINST,
									rq->rmipsd.curr_avg_ipc));
		sched_get_max_group_util(mg_policy->policy->cpus, &rtg_util, &rtg_freq);
		pidfreq = max_t(unsigned int, pidfreq, util_to_freq(cpu, rtg_util));
		nextfreq = max_t(unsigned int, pidfreq, taskfreq);
		nextfreq = min_t(unsigned int, nextfreq, maxfreq);
	}
#endif

	mipsgov_change_mips_freq(cpu, nextfreq);
	trace_perf(mips_load_freq, p, rq, cpufreq, taskfreq, nextfreq, flags);
}

void mipsgov_mark_freq_change(int cpu)
{
	unsigned long rtg_util;
	unsigned int rtg_freq;
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy;

	if (!mg_cpu->enabled)
		return;

	mg_policy = mg_cpu->mg_policy;
	if (!mg_policy)
		return;

	sched_get_max_group_util(mg_policy->policy->cpus, &rtg_util, &rtg_freq);
	rtg_freq = util_to_freq(cpu, rtg_util);
	if (rtg_freq > mg_cpu->mips_freq)
		mipsgov_change_mips_freq(cpu, rtg_freq);
}

void update_mips_scale(int cpu)
{
	struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
	struct mipsgov_policy *mg_policy;
	struct cpufreq_policy *policy;

	if (!mg_cpu->enabled)
		return;

	mg_policy = mg_cpu->mg_policy;
	if (!mg_policy)
		return;

	policy = mg_policy->policy;
	arch_set_mips_scale(cpu, policy->cpuinfo.max_freq);
}

static int mipsgov_init(struct cpufreq_policy *policy)
{
	struct mipsgov_policy *mg_policy;
	struct mipsgov_tunables *tunables;
	int ret = 0;

	/* State should be equivalent to EXIT */
	if (policy->governor_data)
		return -EBUSY;

	if (policy->cached_mipsgov_policy) {
		policy->governor_data = policy->cached_mipsgov_policy;
		return 0;
	}

	mg_policy = mipsgov_policy_alloc(policy);
	if (!mg_policy)
		return -ENOMEM;

	ret = mipsgov_kthread_create(mg_policy);
	if (ret)
		goto free_mg_policy;

	mutex_lock(&global_tunables_lock);

	if (global_tunables) {
		if (WARN_ON(have_governor_per_policy())) {
			ret = -EINVAL;
			goto stop_kthread;
		}
		policy->governor_data = mg_policy;
		mg_policy->tunables = global_tunables;

		gov_attr_set_get(&global_tunables->attr_set, &mg_policy->tunables_hook);
		goto out;
	}

	tunables = mipsgov_tunables_alloc(mg_policy);
	if (!tunables) {
		ret = -ENOMEM;
		goto stop_kthread;
	}

	mipsgov_tunables_init(policy, tunables);

	policy->governor_data = mg_policy;
	mg_policy->tunables = tunables;

	ret = kobject_init_and_add(&tunables->attr_set.kobj, &mipsgov_tunables_ktype,
				   get_governor_parent_kobj(policy), "%s",
				   mips_gov.name);
	if (ret)
		goto fail;

	create_sys_set(mg_policy);

	policy->cached_mipsgov_policy = mg_policy;
out:
	mutex_unlock(&global_tunables_lock);
	return 0;

fail:
	kobject_put(&tunables->attr_set.kobj);
	policy->governor_data = NULL;
	mipsgov_tunables_free(tunables);

stop_kthread:
	mipsgov_kthread_stop(mg_policy);
	mutex_unlock(&global_tunables_lock);

free_mg_policy:
	mipsgov_policy_free(mg_policy);

	pr_err("initialization failed (error %d)\n", ret);
	return ret;
}

static void mipsgov_exit(struct cpufreq_policy *policy)
{
	policy->governor_data = NULL;
}

static int mipsgov_start(struct cpufreq_policy *policy)
{
	struct mipsgov_policy *mg_policy = policy->governor_data;
	unsigned int cpu;
	unsigned long flags;

	if (!mips_enable)
		return -ENOENT;
	mg_policy->next_freq = 0;
	mg_policy->work_in_progress = false;
	for_each_cpu(cpu, policy->cpus) {
		struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);

		memset(mg_cpu, 0, sizeof(*mg_cpu));
		mg_cpu->cpu			= cpu;
		mg_cpu->mg_policy		= mg_policy;
	}

	raw_spin_lock_irqsave(&mg_policy->cpu_timer_lock, flags);
	for_each_cpu(cpu, policy->cpus) {
		struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);
		timer_setup(&mg_cpu->cpu_slack_timer, cpu_clear_mips_freq_timer, 0);
		add_timer(&mg_cpu->cpu_slack_timer);
		mg_cpu->enabled = true;
		update_mips_scale(cpu);
	}

	mg_policy->governor_enabled = true;
	raw_spin_unlock_irqrestore(&mg_policy->cpu_timer_lock, flags);

	blocking_notifier_call_chain(&cpugov_status_notifier_list,
				     CPUGOV_START, &policy->cpu);
	return 0;
}

static void mipsgov_stop(struct cpufreq_policy *policy)
{
	struct mipsgov_policy *mg_policy = policy->governor_data;
	unsigned int cpu;
	unsigned long flags;

	blocking_notifier_call_chain(&cpugov_status_notifier_list,
				     CPUGOV_STOP, &policy->cpu);

	raw_spin_lock_irqsave(&mg_policy->cpu_timer_lock, flags);
	mg_policy->governor_enabled = false;
	for_each_cpu(cpu, policy->cpus) {
		struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);

		mg_cpu->enabled = false;
	}
	raw_spin_unlock_irqrestore(&mg_policy->cpu_timer_lock, flags);

	for_each_cpu(cpu, policy->cpus) {
		struct mipsgov_cpu *mg_cpu = &per_cpu(mipsgov_cpu, cpu);

		del_timer_sync(&mg_cpu->cpu_slack_timer);
	}

	irq_work_sync(&mg_policy->irq_work);
	kthread_cancel_work_sync(&mg_policy->work);
}

static void mipsgov_limits(struct cpufreq_policy *policy)
{
	mips_check_freq_update(policy->cpu, POLICY_MIN_RESTORE);
}

struct cpufreq_governor mips_gov = {
	.name			= "mips",
	.owner			= THIS_MODULE,
	.flags			= CPUFREQ_GOV_DYNAMIC_SWITCHING,
	.init			= mipsgov_init,
	.exit			= mipsgov_exit,
	.start			= mipsgov_start,
	.stop			= mipsgov_stop,
	.limits			= mipsgov_limits,
};

cpufreq_governor_init(mips_gov);
