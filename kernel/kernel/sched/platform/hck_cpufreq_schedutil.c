/*
 *
 * hck cpufreq schedutil module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "sched.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
#define CREATE_TRACE_POINTS
#include <trace/events/cpufreq_schedutil.h>
#undef CREATE_TRACE_POINTS
#endif

#include <platform_include/cee/linux/hck/kernel/sched/hck_cpufreq_schedutil.h>

#ifdef CONFIG_HECA
#include <linux/platform_drivers/heca.h>
#endif

#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT

enum {
	FREQ_STAT_CPU_LOAD		= (1 << 0),
	FREQ_STAT_TOP_TASK		= (1 << 1),
	FREQ_STAT_PRED_LOAD_SUM		= (1 << 2),
	FREQ_STAT_MAX_PRED_LS		= (1 << 3),
	FREQ_STAT_PRED_CPU_LOAD_MIN	= (1 << 4),
	FREQ_STAT_HWVIP_BOOST_TASK	= (1 << 5),
};

#define FREQ_STAT_USE_CPU_PRED_WINDOW \
	(FREQ_STAT_PRED_LOAD_SUM | FREQ_STAT_MAX_PRED_LS | FREQ_STAT_PRED_CPU_LOAD_MIN)
#define FREQ_STAT_USE_PRED_LOAD (FREQ_STAT_USE_CPU_PRED_WINDOW | FREQ_STAT_HWVIP_BOOST_TASK)

__read_mostly unsigned int sched_io_is_busy = 0;

/* Target load. Lower values result in higher CPU speeds. */
#define DEFAULT_TARGET_LOAD 80
static unsigned int default_target_loads[] = {DEFAULT_TARGET_LOAD};
#define DEFAULT_RATE_LIMIT_US (79 * USEC_PER_MSEC)
static unsigned int default_above_hispeed_delay[] = {
		DEFAULT_RATE_LIMIT_US };
static unsigned int default_min_sample_time[] = {
		DEFAULT_RATE_LIMIT_US };

#define show_one(file_name, object, format)		\
static ssize_t file_name##_show				\
(struct gov_attr_set *attr_set, char *buf)		\
{							\
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);	\
	return scnprintf(buf, PAGE_SIZE, format, tunables->object);\
}

void sched_set_io_is_busy(int val)
{
	sched_io_is_busy = val;
}

show_one(overload_duration, overload_duration, "%u\n");

static ssize_t overload_duration_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->overload_duration = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook)
		sg_policy->overload_duration_ns = val * NSEC_PER_MSEC;

	return count;
}

show_one(go_hispeed_load, go_hispeed_load, "%u\n");

static ssize_t go_hispeed_load_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->go_hispeed_load = val;

	return count;
}

show_one(hispeed_freq, hispeed_freq, "%u\n");

static ssize_t hispeed_freq_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->hispeed_freq = val;

	return count;
}

static void sugov_boost(struct gov_attr_set *attr_set)
{
	struct sugov_policy *sg_policy;
	u64 now;

#ifdef CONFIG_SCHED_LTS
	now = use_pelt() ? ktime_get_ns() : sched_ktime_clock();
#else
	now = use_pelt() ? ktime_get_ns() : walt_ktime_clock();
#endif

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		sugov_deferred_update(sg_policy, now, sg_policy->tunables->hispeed_freq);
	}
}

static ssize_t boostpulse_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;
	u64 now;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	now = ktime_to_us(ktime_get());
	if (tunables->boostpulse_endtime + tunables->boostpulse_min_interval > now)
		return count;

	tunables->boostpulse_endtime = now + tunables->boostpulse_duration;
	trace_perf(cpufreq_schedutil_boost, "pulse");

	if (!tunables->boosted)
		sugov_boost(attr_set);

	return count;
}

show_one(boostpulse_duration, boostpulse_duration, "%u\n");

static ssize_t boostpulse_duration_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->boostpulse_duration = val;

	return count;
}

show_one(io_is_busy, io_is_busy, "%u\n");

static ssize_t io_is_busy_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->io_is_busy = val;
	sched_set_io_is_busy(val);

	return count;
}

show_one(fast_ramp_down, fast_ramp_down, "%u\n");

static ssize_t fast_ramp_down_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->fast_ramp_down = val;

	return count;
}

show_one(fast_ramp_up, fast_ramp_up, "%u\n");

static ssize_t fast_ramp_up_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->fast_ramp_up = val;

	return count;
}

show_one(freq_reporting_policy, freq_reporting_policy, "%u\n");

static ssize_t freq_reporting_policy_store(struct gov_attr_set *attr_set,
		const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int val;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->freq_reporting_policy = val;

	return count;
}

#ifdef CONFIG_MIGRATION_NOTIFY
show_one(freq_inc_notify, freq_inc_notify, "%u\n");

static ssize_t freq_inc_notify_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->freq_inc_notify = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->freq_inc_notify = val;
	}

	return count;
}

show_one(freq_dec_notify, freq_dec_notify, "%u\n");

static ssize_t freq_dec_notify_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->freq_dec_notify = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->freq_dec_notify = val;
	}

	return count;
}
#endif /* CONFIG_MIGRATION_NOTIFY */

#ifdef CONFIG_SCHED_TOP_TASK
show_one(top_task_hist_size, top_task_hist_size, "%u\n");

static ssize_t top_task_hist_size_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	/* Allowed range: [1, RAVG_HIST_SIZE_MAX] */
	if (val < 1 || val > RAVG_HIST_SIZE_MAX)
		return -EINVAL;

	tunables->top_task_hist_size = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->top_task_hist_size = val;
	}

	return count;
}

show_one(top_task_stats_policy, top_task_stats_policy, "%u\n");

static ssize_t top_task_stats_policy_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->top_task_stats_policy = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->top_task_stats_policy = val;
	}

	return count;
}

show_one(top_task_stats_empty_window, top_task_stats_empty_window, "%u\n");

static ssize_t top_task_stats_empty_window_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->top_task_stats_empty_window = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->top_task_stats_empty_window = val;
	}

	return count;
}
#endif /* CONFIG_SCHED_TOP_TASK */

#ifdef CONFIG_ED_TASK
show_one(ed_task_running_duration, ed_task_running_duration, "%u\n");

static ssize_t ed_task_running_duration_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->ed_task_running_duration = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->ed_task_running_duration = val;
	}

	return count;
}

show_one(ed_task_waiting_duration, ed_task_waiting_duration, "%u\n");

static ssize_t ed_task_waiting_duration_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->ed_task_waiting_duration = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->ed_task_waiting_duration = val;
	}

	return count;
}

show_one(ed_new_task_running_duration, ed_new_task_running_duration, "%u\n");

static ssize_t ed_new_task_running_duration_store(struct gov_attr_set *attr_set,
				      const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->ed_new_task_running_duration = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		for_each_cpu(cpu, sg_policy->policy->cpus)
			cpu_rq(cpu)->ed_new_task_running_duration = val;
	}

	return count;
}
#endif /* CONFIG_ED_TASK */

static unsigned int *get_tokenized_data(const char *buf, int *num_tokens)
{
	const char *cp;
	int i;
	int ntokens = 1;
	unsigned int *tokenized_data;
	int err = -EINVAL;

	cp = buf;
	while ((cp = strpbrk(cp + 1, " :")))
		ntokens++;

	if (!(ntokens & 0x1))
		goto err;

	tokenized_data = kmalloc(ntokens * sizeof(unsigned int), GFP_KERNEL);
	if (!tokenized_data) {
		err = -ENOMEM;
		goto err;
	}

	cp = buf;
	i = 0;
	while (i < ntokens) {
		if (sscanf(cp, "%u", &tokenized_data[i++]) != 1) /* [false alarm]:fortify */
			goto err_kfree;

		cp = strpbrk(cp, " :");
		if (!cp)
			break;
		cp++;
	}

	if (i != ntokens)
		goto err_kfree;

	*num_tokens = ntokens;
	return tokenized_data;

err_kfree:
	kfree(tokenized_data);
err:
	return ERR_PTR(err);
}

static ssize_t target_loads_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", tunables->target_loads[i],
			       i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	return ret;
}

static ssize_t target_loads_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int *new_target_loads;
	unsigned long flags;
	int ntokens;
	int i;

	new_target_loads = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_target_loads))
		return PTR_ERR(new_target_loads);

	for (i = 0; i < ntokens; i++) {
		if (new_target_loads[i] == 0) {
			kfree(new_target_loads);
			return -EINVAL;
		}
	}

	spin_lock_irqsave(&tunables->target_loads_lock, flags);
	if (tunables->target_loads != default_target_loads)
		kfree(tunables->target_loads);
	tunables->target_loads = new_target_loads;
	tunables->ntarget_loads = ntokens;
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);

	return count;
}

static ssize_t above_hispeed_delay_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	for (i = 0; i < tunables->nabove_hispeed_delay; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", tunables->above_hispeed_delay[i],
			       i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	return ret;
}

static ssize_t above_hispeed_delay_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int *new_above_hispeed_delay;
	unsigned long flags;
	int ntokens;

	new_above_hispeed_delay = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_above_hispeed_delay))
		return PTR_ERR(new_above_hispeed_delay);

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);
	if (tunables->above_hispeed_delay != default_above_hispeed_delay)
		kfree(tunables->above_hispeed_delay);
	tunables->above_hispeed_delay = new_above_hispeed_delay;
	tunables->nabove_hispeed_delay = ntokens;
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);

	return count;
}

static ssize_t min_sample_time_show(struct gov_attr_set *attr_set, char *buf)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned long flags;
	ssize_t ret = 0;
	int i;

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);

	for (i = 0; i < tunables->nmin_sample_time; i++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret, "%u%s", tunables->min_sample_time[i],
			       i & 0x1 ? ":" : " ");

	scnprintf(buf + ret - 1, PAGE_SIZE - ret + 1, "\n");
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);

	return ret;
}

static ssize_t min_sample_time_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	unsigned int *new_min_sample_time;
	unsigned long flags;
	int ntokens;

	new_min_sample_time = get_tokenized_data(buf, &ntokens);
	if (IS_ERR(new_min_sample_time))
		return PTR_ERR(new_min_sample_time);

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);
	if (tunables->min_sample_time != default_min_sample_time)
		kfree(tunables->min_sample_time);
	tunables->min_sample_time = new_min_sample_time;
	tunables->nmin_sample_time = ntokens;
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);

	return count;
}

show_one(timer_slack, timer_slack, "%u\n");

static ssize_t timer_slack_store(struct gov_attr_set *attr_set,
				  const char *buf, size_t count)
{
	int ret;
	unsigned long val;
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	tunables->timer_slack = val;
	return count;
}

show_one(iowait_boost_step, iowait_boost_step, "%u\n");

static ssize_t iowait_boost_step_store(struct gov_attr_set *attr_set,
				       const char *buf, size_t count)
{
	int ret, cpu;
	unsigned int val;
	struct sugov_policy *sg_policy = NULL;
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);

	ret = kstrtouint(buf, 10, &val);
	if (ret < 0)
		return ret;

	tunables->iowait_boost_step = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		if (sg_policy->tunables == tunables) {
			for_each_cpu(cpu, sg_policy->policy->cpus) {
				struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);

				sg_cpu->iowait_boost_step = freq_to_util(cpu, tunables->iowait_boost_step);
			}
			break;
		}
	}

	return count;
}

#ifdef CONFIG_FREQ_IO_LIMIT
show_one(iowait_upper_limit, iowait_upper_limit, "%u\n");

static ssize_t iowait_upper_limit_store(struct gov_attr_set *attr_set, const char *buf, size_t count)
{
	struct sugov_tunables *tunables = to_sugov_tunables(attr_set);
	struct sugov_policy *sg_policy = NULL;
	unsigned int val;
	int cpu;

	if (kstrtouint(buf, 10, &val))
		return -EINVAL;

	tunables->iowait_upper_limit = val;

	list_for_each_entry(sg_policy, &attr_set->policy_list, tunables_hook) {
		if (sg_policy->tunables == tunables) {
			for_each_cpu(cpu, sg_policy->policy->cpus) {
				struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);

				sg_cpu->iowait_boost_max = freq_to_util(cpu, tunables->iowait_upper_limit);
			}
			break;
		}
	}

	return count;
}
#endif

struct governor_attr overload_duration = __ATTR_RW(overload_duration);
struct governor_attr go_hispeed_load = __ATTR_RW(go_hispeed_load);
struct governor_attr hispeed_freq = __ATTR_RW(hispeed_freq);
struct governor_attr target_loads = __ATTR_RW(target_loads);
struct governor_attr above_hispeed_delay = __ATTR_RW(above_hispeed_delay);
struct governor_attr min_sample_time = __ATTR_RW(min_sample_time);
struct governor_attr boostpulse = __ATTR_WO(boostpulse);
struct governor_attr boostpulse_duration = __ATTR_RW(boostpulse_duration);
struct governor_attr io_is_busy = __ATTR_RW(io_is_busy);
struct governor_attr timer_slack = __ATTR_RW(timer_slack);
struct governor_attr fast_ramp_down = __ATTR_RW(fast_ramp_down);
struct governor_attr fast_ramp_up = __ATTR_RW(fast_ramp_up);
struct governor_attr freq_reporting_policy = __ATTR_RW(freq_reporting_policy);
struct governor_attr iowait_boost_step = __ATTR_RW(iowait_boost_step);
#ifdef CONFIG_SCHED_TOP_TASK
struct governor_attr top_task_hist_size = __ATTR_RW(top_task_hist_size);
struct governor_attr top_task_stats_policy = __ATTR_RW(top_task_stats_policy);
struct governor_attr top_task_stats_empty_window = __ATTR_RW(top_task_stats_empty_window);
#endif
#ifdef CONFIG_ED_TASK
struct governor_attr ed_task_running_duration = __ATTR_RW(ed_task_running_duration);
struct governor_attr ed_task_waiting_duration = __ATTR_RW(ed_task_waiting_duration);
struct governor_attr ed_new_task_running_duration = __ATTR_RW(ed_new_task_running_duration);
#endif
#ifdef CONFIG_MIGRATION_NOTIFY
struct governor_attr freq_inc_notify = __ATTR_RW(freq_inc_notify);
struct governor_attr freq_dec_notify = __ATTR_RW(freq_dec_notify);
#endif
#ifdef CONFIG_FREQ_IO_LIMIT
struct governor_attr iowait_upper_limit = __ATTR_RW(iowait_upper_limit);
#endif

struct governor_user_attr schedutil_user_attrs[] = {
	{.name = "target_loads", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "hispeed_freq", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "go_hispeed_load", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "above_hispeed_delay", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "min_sample_time", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "timer_slack", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "boostpulse", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0220},
	{.name = "boostpulse_duration", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "fast_ramp_down", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "fast_ramp_up", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "freq_reporting_policy", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "iowait_boost_step", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "overload_duration", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
#ifdef CONFIG_SCHED_TOP_TASK
	{.name = "top_task_hist_size", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "top_task_stats_policy", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "top_task_stats_empty_window", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
#endif
#ifdef CONFIG_ED_TASK
	{.name = "ed_task_running_duration", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "ed_task_waiting_duration", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "ed_new_task_running_duration", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
#endif
#ifdef CONFIG_MIGRATION_NOTIFY
	{.name = "freq_inc_notify", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
	{.name = "freq_dec_notify", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
#endif
#ifdef CONFIG_FREQ_IO_LIMIT
	{.name = "iowait_upper_limit", .uid = SYSTEM_UID, .gid = SYSTEM_GID, .mode = 0660},
#endif
	/* add below */
	INVALID_ATTR,
};

static unsigned int freq_to_targetload(struct sugov_tunables *tunables,
				       unsigned int freq)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->target_loads_lock, flags);

	for (i = 0; i < tunables->ntarget_loads - 1 &&
	     freq >= tunables->target_loads[i + 1]; i += 2)
		;

	ret = tunables->target_loads[i];
	spin_unlock_irqrestore(&tunables->target_loads_lock, flags);
	return ret;
}

/*
 * If increasing frequencies never map to a lower target load then
 * choose_freq() will find the minimum frequency that does not exceed its
 * target load given the current load.
 */
static unsigned int choose_freq(struct sugov_policy *sg_policy,
				unsigned int loadadjfreq)
{
	struct cpufreq_policy *policy = sg_policy->policy;
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	unsigned int prevfreq, freqmin = 0, freqmax = UINT_MAX, tl;
	unsigned int freq = policy->cur;
	int index;

	do {
		prevfreq = freq;
		tl = freq_to_targetload(sg_policy->tunables, freq);

		/*
		 * Find the lowest frequency where the computed load is less
		 * than or equal to the target load.
		 */

		index = cpufreq_frequency_table_target(policy, loadadjfreq / tl,
			    CPUFREQ_RELATION_L);

		freq = freq_table[index].frequency;

		if (freq > prevfreq) {
			/* The previous frequency is too low */
			freqmin = prevfreq;

			if (freq < freqmax)
				continue;

			/* Find highest frequency that is less than freqmax */
			index = cpufreq_frequency_table_target(policy,
				    freqmax - 1, CPUFREQ_RELATION_H);

			freq = freq_table[index].frequency;

			if (freq == freqmin) {
				/*
				 * The first frequency below freqmax has already
				 * been found to be too low. freqmax is the
				 * lowest speed we found that is fast enough.
				 */
				freq = freqmax;
				break;
			}
		} else if (freq < prevfreq) {
			/* The previous frequency is high enough. */
			freqmax = prevfreq;

			if (freq > freqmin)
				continue;

			/* Find lowest frequency that is higher than freqmin */
			index = cpufreq_frequency_table_target(policy,
				    freqmin + 1, CPUFREQ_RELATION_L);

			freq = freq_table[index].frequency;

			/*
			 * If freqmax is the first frequency above
			 * freqmin then we have already found that
			 * this speed is fast enough.
			 */
			if (freq == freqmax)
				break;
		}

		/* If same frequency chosen as previous then done. */
	} while (freq != prevfreq);

	return freq;
}

#ifdef CONFIG_SCHED_HISI_UTIL_CLAMP
static inline unsigned int cpu_min_util(int cpu)
{
	return cpu_rq(cpu)->uclamp[UCLAMP_MIN].value;
}

static unsigned int sg_min_util(const struct cpumask *cpus)
{
	unsigned int min_util = 0;
	int i;
	for_each_cpu(i, cpus)
		min_util = max(cpu_min_util(i), min_util);
	return min_util;
}
#else
static inline unsigned int cpu_min_util(int cpu) { return 0; }
#endif

/* Re-evaluate load to see if a frequency change is required or not */
unsigned int eval_target_freq(struct sugov_policy *sg_policy,
				     unsigned long util, unsigned long max)
{
	int cpu_load = 0;
	unsigned int new_freq;

	struct sugov_tunables *tunables = sg_policy->tunables;
	struct cpufreq_policy *policy = sg_policy->policy;
	u64 now = ktime_to_us(ktime_get());

#ifdef CONFIG_HECA
	bool boosted = false;
#endif

	tunables->boosted = now < tunables->boostpulse_endtime;

	if (tunables->boosted && policy->cur < tunables->hispeed_freq) {
		new_freq = tunables->hispeed_freq;
#ifdef CONFIG_HECA
		boosted = true;
#endif
	} else {
		cpu_load = util * 100 / capacity_curr_of(policy->cpu);
		new_freq = choose_freq(sg_policy, cpu_load * policy->cur);

		if ((cpu_load >= tunables->go_hispeed_load || tunables->boosted) &&
		    new_freq < tunables->hispeed_freq)
			new_freq = tunables->hispeed_freq;
	}

	new_freq = max(util_to_freq(policy->cpu, sg_policy->iowait_boost), new_freq);
#ifdef CONFIG_SCHED_RTG
	new_freq = max(sg_policy->rtg_freq, new_freq);
#endif
#ifdef CONFIG_SCHED_HISI_UTIL_CLAMP
	/* Do not apply target_loads to min_util. */
	new_freq = max(util_to_freq(policy->cpu, sg_min_util(policy->cpus)), new_freq);
#endif

#ifdef CONFIG_HECA
	new_freq = heca_get_next_freq(policy->cpu, tunables->boosted, new_freq);
#endif

	trace_perf(cpufreq_schedutil_eval_target, sg_policy->max_cpu,
					    util, max, cpu_load,
#ifdef CONFIG_SCHED_RTG
					    sg_policy->rtg_util,
					    sg_policy->rtg_freq,
#endif
#ifdef CONFIG_SCHED_COLOC_GROUP
					    sg_policy->coloc_grp_util,
#endif
					    policy->cur, new_freq);

	return new_freq;
}

static inline unsigned int get_freq_reporting_policy(int cpu)
{
	struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);
	struct sugov_policy *sg_policy;
	unsigned int policy;

	if (!sg_cpu->enabled) {
		policy = DEFAULT_FREQ_REPORTING_POLICY;
		goto out;
	}

	sg_policy = sg_cpu->sg_policy;
	if (!sg_policy || !sg_policy->governor_enabled) {
		policy = DEFAULT_FREQ_REPORTING_POLICY;
		goto out;
	}

	policy = sg_policy->tunables->freq_reporting_policy;

#ifdef CONFIG_SCHED_PRED_LOAD
	if (!predl_enable &&
	    ((policy & FREQ_STAT_USE_PRED_LOAD) == policy)) {
		policy = DEFAULT_FREQ_REPORTING_POLICY;
		goto out;
	}
#endif
out:
	return policy;
}

#ifdef CONFIG_SCHED_PRED_LOAD
bool use_task_pred_load(int cpu, struct task_struct *p)
{
	unsigned int reporting_policy;

	if (!predl_enable)
		return false;

	reporting_policy = get_freq_reporting_policy(cpu);
	if (reporting_policy & FREQ_STAT_USE_CPU_PRED_WINDOW)
		return true;

	if ((reporting_policy & FREQ_STAT_HWVIP_BOOST_TASK) &&
	    predl_hwvip_boost(p))
		return true;

	return false;
}
#endif

static unsigned int freq_to_above_hispeed_delay(struct sugov_tunables *tunables,
				       unsigned int freq)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->above_hispeed_delay_lock, flags);

	for (i = 0; i < tunables->nabove_hispeed_delay - 1 &&
	     freq >= tunables->above_hispeed_delay[i + 1]; i += 2)
		;

	ret = tunables->above_hispeed_delay[i];
	spin_unlock_irqrestore(&tunables->above_hispeed_delay_lock, flags);
	return ret;
}

static unsigned int freq_to_min_sample_time(struct sugov_tunables *tunables,
				       unsigned int freq)
{
	unsigned long flags;
	unsigned int ret;
	int i;

	spin_lock_irqsave(&tunables->min_sample_time_lock, flags);

	for (i = 0; i < tunables->nmin_sample_time - 1 &&
	     freq >= tunables->min_sample_time[i + 1]; i += 2)
		;

	ret = tunables->min_sample_time[i];
	spin_unlock_irqrestore(&tunables->min_sample_time_lock, flags);
	return ret;
}

static void sugov_slack_timer_resched(struct sugov_policy *sg_policy)
{
	u64 expires;

	raw_spin_lock(&sg_policy->timer_lock);
	if (!sg_policy->governor_enabled)
		goto unlock;

	del_timer(&sg_policy->pol_slack_timer);
	if (sg_policy->tunables->timer_slack >= 0 &&
	    sg_policy->next_freq > sg_policy->policy->min) {
		expires = jiffies + usecs_to_jiffies(sg_policy->tunables->timer_slack);
		sg_policy->pol_slack_timer.expires = expires;
		add_timer_on(&sg_policy->pol_slack_timer, sg_policy->trigger_cpu);
	}
unlock:
	raw_spin_unlock(&sg_policy->timer_lock);
}

bool sugov_iowait_boost_pending(int cpu)
{
	struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);

	return sg_cpu->enabled && sg_cpu->iowait_boost_pending;
}

#ifndef CONFIG_SCHED_LTS
#ifdef CONFIG_SCHED_WALT
static inline unsigned long walt_cpu_util_freq(int cpu)
{
	u64 walt_cpu_util;

#ifdef CONFIG_SCHED_SMT
	if (cpu_rq(cpu)->is_smt)
		walt_cpu_util = cpu_rq(cpu)->prev_runnable_sum +
				cpu_rq(cpu_rq(cpu)->smt_sibling)->prev_runnable_sum;
	else
#endif
		walt_cpu_util = cpu_rq(cpu)->prev_runnable_sum;
	walt_cpu_util <<= SCHED_CAPACITY_SHIFT;
	do_div(walt_cpu_util, walt_ravg_window);

	return min_t(unsigned long, walt_cpu_util, capacity_orig_of(cpu));
}
#else
#define walt_cpu_util_freq(cpu) 0
#endif
#endif

unsigned long freq_policy_util(int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	unsigned int reporting_policy = get_freq_reporting_policy(cpu);
	unsigned long util = 0;

	if (reporting_policy & FREQ_STAT_CPU_LOAD)
#ifdef CONFIG_SCHED_LTS
		util = max(util, cpu_util_freq_walt(cpu));
#else
		util = max(util, walt_cpu_util_freq(cpu));
#endif

	if (reporting_policy & FREQ_STAT_TOP_TASK)
		util = max(util, top_task_util(rq));

	if (reporting_policy & FREQ_STAT_PRED_LOAD_SUM)
		util = max(util, predict_util_sum(rq));

	if (reporting_policy & FREQ_STAT_MAX_PRED_LS)
		util = max(util, max_pred_ls(rq));

	if (reporting_policy & FREQ_STAT_PRED_CPU_LOAD_MIN)
		util = max(util, cpu_util_pred_min(rq));

	if (reporting_policy & FREQ_STAT_HWVIP_BOOST_TASK)
		util = max(util, predl_max_hwvip_util(rq));

	return util;
}

static inline bool sugov_cpu_is_overload(struct sugov_cpu *sg_cpu)
{
	u64 idle_time, delta;

	if (cpu_rq(sg_cpu->cpu)->skip_overload_detect)
		return false;

	idle_time = get_cpu_idle_time(sg_cpu->cpu, NULL, 0);
	if (sg_cpu->last_idle_time != idle_time)
		sg_cpu->idle_update_ts = sg_cpu->sg_policy->update_time;

	sg_cpu->last_idle_time = idle_time;
	delta = sg_cpu->sg_policy->update_time - sg_cpu->idle_update_ts;

	return (delta > sg_cpu->sg_policy->overload_duration_ns);
}

static bool sugov_should_use_max_freq(struct sugov_cpu *sg_cpu)
{
	if (sugov_cpu_is_overload(sg_cpu))
		return true;

#ifdef CONFIG_ED_TASK
	if (cpu_rq(sg_cpu->cpu)->ed_task != NULL)
		return true;
#endif

	return false;
}

#ifdef CONFIG_CORE_CTRL
static void sugov_report_load(int cpu, unsigned long util)
{
	struct cpufreq_govinfo govinfo;

	/*
	 * Send govinfo notification.
	 * Govinfo notification could potentially wake up another thread
	 * managed by its clients. Thread wakeups might trigger a load
	 * change callback that executes this function again. Therefore
	 * no spinlock could be held when sending the notification.
	 */
	govinfo.cpu = cpu;
	govinfo.load = util * 100 / capacity_curr_of(cpu);
	govinfo.sampling_rate_us = 0;
	atomic_notifier_call_chain(&cpufreq_govinfo_notifier_list,
				   CPUFREQ_LOAD_CHANGE, &govinfo);
}
#endif

static unsigned int sugov_next_freq_shared_policy(struct sugov_policy *sg_policy)
{
	struct cpufreq_policy *policy = sg_policy->policy;
	unsigned long util = 0, max = 1;
	int j;

	sg_policy->max_cpu = cpumask_first(policy->cpus);
	sg_policy->iowait_boost = 0;

	for_each_cpu(j, policy->cpus) {
		struct sugov_cpu *j_sg_cpu = &per_cpu(sugov_cpu, j);
		unsigned long j_util, j_max;

		j_util = sugov_get_util(j_sg_cpu);
		j_sg_cpu->util = j_util;
		j_max = j_sg_cpu->max;

		if (j_sg_cpu->use_max_freq)
			j_util = j_max;

#ifdef CONFIG_CORE_CTRL
		sugov_report_load(j, j_util);
#endif
		if (j_util * max > j_max * util) {
			util = j_util;
			max = j_max;
			sg_policy->max_cpu = j;
		}

		/*
		 * Below load signal don't use target_loads,
		 * don't update util, only update the field of sg_policy
		 */
		j_util = sugov_iowait_apply(j_sg_cpu, sg_policy->update_time, j_util, j_max);

		trace_perf(cpufreq_schedutil_get_util, j, j_sg_cpu->util,
						 j_sg_cpu->max,
						 top_task_util(cpu_rq(j)),
						 predl_max_hwvip_util(cpu_rq(j)),
						 max_pred_ls(cpu_rq(j)),
						 cpu_util_pred_min(cpu_rq(j)),
						 j_sg_cpu->iowait_boost,
						 cpu_min_util(j),
						 j_sg_cpu->flags,
						 j_sg_cpu->use_max_freq);

		j_sg_cpu->flags = 0;
	}

#ifdef CONFIG_SCHED_RTG
	sched_get_max_group_util(policy->cpus, &sg_policy->rtg_util, &sg_policy->rtg_freq);
	util = max(sg_policy->rtg_util, util);
#endif
#ifdef CONFIG_SCHED_COLOC_GROUP
	cpufreq_get_coloc_grp_util(policy->cpus, &sg_policy->coloc_grp_util);
	util = max_t(unsigned long, sg_policy->coloc_grp_util, util);
#endif

	return get_next_freq(sg_policy, util, max);
}

static bool sugov_time_limit(struct sugov_policy *sg_policy, unsigned int next_freq,
			     int skip_min_sample_time, int skip_hispeed_logic)
{
	u64 delta_ns;
	unsigned int min_sample_time;

	if (!skip_hispeed_logic &&
	    next_freq > sg_policy->next_freq &&
	    sg_policy->next_freq >= sg_policy->tunables->hispeed_freq) {
		delta_ns = sg_policy->update_time - sg_policy->hispeed_validate_time;
		if (delta_ns < NSEC_PER_USEC *
		    freq_to_above_hispeed_delay(sg_policy->tunables, sg_policy->next_freq)) {
			trace_perf(cpufreq_schedutil_notyet, sg_policy->max_cpu,
						       "above_hispeed_delay", delta_ns,
						       sg_policy->next_freq, next_freq);
			return true;
		}
	}

	sg_policy->hispeed_validate_time = sg_policy->update_time;
	/*
	 * Do not scale below floor_freq unless we have been at or above the
	 * floor frequency for the minimum sample time since last validated.
	 */
	if (next_freq < sg_policy->next_freq) {
		min_sample_time = freq_to_min_sample_time(sg_policy->tunables, sg_policy->next_freq);

		if (!skip_min_sample_time) {
			delta_ns = sg_policy->update_time - sg_policy->floor_validate_time;
			if (delta_ns < NSEC_PER_USEC * min_sample_time) {
				trace_perf(cpufreq_schedutil_notyet, sg_policy->max_cpu,
						       "min_sample_time", delta_ns,
						       sg_policy->next_freq, next_freq);
				return true;
			}
		}
	}

	if (!sg_policy->tunables->boosted ||
	    next_freq > sg_policy->tunables->hispeed_freq)
		sg_policy->floor_validate_time = sg_policy->update_time;

	return false;
}

unsigned int check_freq_reporting_policy(int cpu, unsigned int flags)
{
	unsigned int reporting_policy = get_freq_reporting_policy(cpu);
	unsigned int ignore_events = 0;

	if (reporting_policy & FREQ_STAT_USE_CPU_PRED_WINDOW)
		ignore_events |= WALT_WINDOW_ROLLOVER;
	else
		ignore_events |= PRED_LOAD_WINDOW_ROLLOVER;

	if (!(reporting_policy & FREQ_STAT_TOP_TASK))
		ignore_events |= ADD_TOP_TASK;

	if (!(reporting_policy & FREQ_STAT_PRED_LOAD_SUM))
		ignore_events |= PRED_LOAD_ENQUEUE;

	if (!(reporting_policy & FREQ_STAT_USE_PRED_LOAD))
		ignore_events |= PRED_LOAD_CHANGE;

	return flags & (~ignore_events);
}

void sugov_mark_util_change(int cpu, unsigned int flags)
{
	struct sugov_cpu *sg_cpu = &per_cpu(sugov_cpu, cpu);
	struct sugov_policy *sg_policy;
	bool skip_min_sample_time = false;
	bool skip_hispeed_logic = false;

	if (!sg_cpu->enabled)
		return;

	sg_policy = sg_cpu->sg_policy;
	if (!sg_policy || !sg_policy->governor_enabled)
		return;

	flags = check_freq_reporting_policy(cpu, flags);
	if (!flags)
		return;

	sg_cpu->flags |= flags;

	if (flags & INTER_CLUSTER_MIGRATION_SRC)
		if (sg_policy->tunables->fast_ramp_down)
			skip_min_sample_time = true;

	if (flags & INTER_CLUSTER_MIGRATION_DST)
		if (sg_policy->tunables->fast_ramp_up)
			skip_hispeed_logic = true;

#ifdef CONFIG_ED_TASK
	if (flags & CLEAR_ED_TASK)
		if (sg_policy->tunables->fast_ramp_down)
			skip_min_sample_time = true;

	if (flags & ADD_ED_TASK)
		skip_hispeed_logic = true;
#endif

#ifdef CONFIG_SCHED_TOP_TASK_SKIP_HISPEED_LOGIC
	if (flags & ADD_TOP_TASK)
		skip_hispeed_logic = true;
#endif

	if (flags & FORCE_UPDATE) {
		skip_min_sample_time = true;
		skip_hispeed_logic = true;
	}

#ifdef CONFIG_SCHED_RTG
	if (flags & FRAME_UPDATE) {
		/*
		 * Boosted scene like appstart do not want to fast
		 * ramp down. FRAME_UPDATE should not break it.
		 */
		if (sg_policy->tunables->fast_ramp_down)
			skip_min_sample_time = true;
		skip_hispeed_logic = true;
	}
#endif

#ifdef CONFIG_SCHED_HISI_UTIL_CLAMP
	if (flags & (SET_MIN_UTIL | ENQUEUE_MIN_UTIL))
		skip_hispeed_logic = true;
#endif

#ifdef CONFIG_HECA
	if (heca_cpu_enable(sg_policy->policy->cpu)) {
		skip_min_sample_time = true;
		skip_hispeed_logic = true;
	}
#endif

#ifdef CONFIG_SCHED_COLOC_GROUP
	if (flags & (COLOC_GRP_WINDOW_ROLLOVER | COLOC_GRP_NEED_UPDATE))
		skip_hispeed_logic = true;
#endif

	if (skip_min_sample_time)
		atomic_set(&sg_policy->skip_min_sample_time, 1);
	if (skip_hispeed_logic)
		atomic_set(&sg_policy->skip_hispeed_logic, 1);

	sg_policy->util_changed = true;
}

void sugov_check_freq_update(int cpu)
{
	struct sugov_cpu *sg_cpu;
	struct sugov_policy *sg_policy;

	if (cpu >= nr_cpu_ids)
		return;

	sg_cpu = &per_cpu(sugov_cpu, cpu);
	if (!sg_cpu->enabled)
		return;

	sg_policy = sg_cpu->sg_policy;
	if (!sg_policy || !sg_policy->governor_enabled)
		return;

	if (sg_policy->util_changed)
		cpufreq_update_util(cpu_rq(cpu), SCHED_CPUFREQ_WALT);
}

void walt_sugov_work(struct kthread_work *work)
{
	struct sugov_policy *sg_policy = container_of(work, struct sugov_policy, work);
	struct cpufreq_policy *policy = sg_policy->policy;
	unsigned int next_freq, cpu;
	struct rq *rq;
	int skip_min_sample_time, skip_hispeed_logic;

	mutex_lock(&sg_policy->work_lock);

	for_each_cpu(cpu, policy->cpus) {
		struct sugov_cpu *j_sg_cpu = &per_cpu(sugov_cpu, cpu);

		if (!j_sg_cpu->enabled)
			goto out;

		rq = cpu_rq(cpu);
		raw_spin_rq_lock_irq(rq);

		j_sg_cpu->use_max_freq = sugov_should_use_max_freq(j_sg_cpu);
		if (j_sg_cpu->use_max_freq)
			atomic_set(&sg_policy->skip_hispeed_logic, 1);
		else {
#ifdef CONFIG_SCHED_LTS
			update_task_ravg(rq->curr, rq, TASK_UPDATE, sched_ktime_clock(), 0);
#else
			walt_update_task_ravg(rq->curr, rq, TASK_UPDATE, walt_ktime_clock(), 0);
#endif
		}

		raw_spin_rq_unlock_irq(rq);
	}

	/* Clear util changes marked above in walt update. */
	sg_policy->util_changed = false;

	/* Allow a new sugov_work to be queued now. */
	sg_policy->work_in_progress = false;

	/*
	 * Before we getting util, read and clear the skip_xxx requirements
	 * for this update.
	 *
	 * Otherwise,
	 * sugov_work():               on cpuX:
	 *   get cpuX's util(high)
	 *                               lower down cpuX's util(e.g. migration)
	 *                               set skip_min_sample_time
	 *   read and clear skip_xxx
	 *   set freq(high)
	 *                               another sugov_work()
	 *                               limited by min_sample_time
	 *
	 * In the following order we will make it consistent:
	 *   atomic_xchg(&skip_xxx, 0)   update any cpu util
	 *   get all cpu's util          atomic_set(&skip_xxx, 1)
	 *   ...                         ...
	 *   adjust frequency            adjust frequency
	 */
	skip_min_sample_time = atomic_xchg(&sg_policy->skip_min_sample_time, 0);
	skip_hispeed_logic = atomic_xchg(&sg_policy->skip_hispeed_logic, 0);

	next_freq = sugov_next_freq_shared_policy(sg_policy);

	if (sugov_time_limit(sg_policy, next_freq, skip_min_sample_time, skip_hispeed_logic))
		goto out;

	if (next_freq == sg_policy->next_freq) {
		trace_perf(cpufreq_schedutil_already, sg_policy->max_cpu,
						sg_policy->next_freq, next_freq);
#ifdef CONFIG_HECA
		if (!heca_cpu_enable(policy->cpu))
#endif
		goto out;
	}

	sg_policy->next_freq = next_freq;
	sg_policy->last_freq_update_time = sg_policy->update_time;

	__cpufreq_driver_target(sg_policy->policy, sg_policy->next_freq,
				CPUFREQ_RELATION_L);

out:
	mutex_unlock(&sg_policy->work_lock);

	sugov_slack_timer_resched(sg_policy);

	blocking_notifier_call_chain(&cpugov_status_notifier_list,
			CPUGOV_ACTIVE, &sg_policy->trigger_cpu);
}

void sugov_nop_timer(struct timer_list *unused)
{
}

void sugov_tunables_init(struct cpufreq_policy *policy,
				struct sugov_tunables *tunables)
{
	tunables->overload_duration = DEFAULT_OVERLOAD_DURATION_MS;
	tunables->above_hispeed_delay = default_above_hispeed_delay;
	tunables->nabove_hispeed_delay =
		ARRAY_SIZE(default_above_hispeed_delay);
	tunables->min_sample_time = default_min_sample_time;
	tunables->nmin_sample_time =
		ARRAY_SIZE(default_min_sample_time);
	tunables->go_hispeed_load = DEFAULT_GO_HISPEED_LOAD;
	tunables->target_loads = default_target_loads;
	tunables->ntarget_loads = ARRAY_SIZE(default_target_loads);
	tunables->boostpulse_duration = DEFAULT_BOOSTPULSE_DURATION;
	tunables->boostpulse_min_interval = DEFAULT_MIN_BOOSTPULSE_INTERVAL;
	tunables->timer_slack = DEFAULT_TIMER_SLACK;
	tunables->freq_reporting_policy = DEFAULT_FREQ_REPORTING_POLICY;
	tunables->iowait_boost_step = IOWAIT_BOOST_INC_STEP;

#ifdef CONFIG_ED_TASK
	tunables->ed_task_running_duration = EARLY_DETECTION_TASK_RUNNING_DURATION;
	tunables->ed_task_waiting_duration = EARLY_DETECTION_TASK_WAITING_DURATION;
	tunables->ed_new_task_running_duration = EARLY_DETECTION_NEW_TASK_RUNNING_DURATION;
#endif

#ifdef CONFIG_MIGRATION_NOTIFY
	tunables->freq_inc_notify = DEFAULT_FREQ_INC_NOTIFY;
	tunables->freq_dec_notify = DEFAULT_FREQ_DEC_NOTIFY;
#endif

#ifdef CONFIG_FREQ_IO_LIMIT
	tunables->iowait_upper_limit = policy->cpuinfo.max_freq;
#endif

	spin_lock_init(&tunables->target_loads_lock);
	spin_lock_init(&tunables->above_hispeed_delay_lock);
	spin_lock_init(&tunables->min_sample_time_lock);
}
#endif

static void map_util_freq_hook(struct sugov_policy *sg_policy, unsigned long util, unsigned long max, unsigned long *next_freq)
{
#ifdef CONFIG_CPU_FREQ_GOV_SCHEDUTIL_OPT
	*next_freq = eval_target_freq(sg_policy, util, max);
#endif
}

static int __init hck_cpufreq_schedutil_register(void)
{
	REGISTER_HCK_VH(map_util_freq_hook, map_util_freq_hook);
	return 0;
}

early_initcall(hck_cpufreq_schedutil_register);

#endif
