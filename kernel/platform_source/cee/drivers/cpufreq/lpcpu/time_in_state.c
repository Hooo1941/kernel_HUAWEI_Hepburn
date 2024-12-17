/*
 * time_in_state.c
 *
 * counting different C-state time in each freq
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/types.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/percpu-defs.h>
#include <linux/spinlock.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/cpuidle.h>
#include <linux/pm.h>
#ifdef CONFIG_PERF_CTRL
#include <platform_include/cee/linux/cpufreq_perf_ctrl.h>
#include <linux/uaccess.h>
#endif
#ifdef CONFIG_SMT_MODE_GOV
#include <linux/sched/smt.h>
#endif

#define KHZ_PER_MHZ 1000

#ifdef CONFIG_SMT_MODE_GOV
struct time_in_smt {
	u64 *time;
	spinlock_t lock;

	/* first CPU of SMT core */
	int leader_cpu;
	/* 0=Core idle 1=ST running 2=MT running */
	unsigned int cur_mode_idx;
	unsigned int max_mode_state;
	ktime_t last_update;
};
#endif

struct time_in_state {
	int cpu;
	spinlock_t lock;
	ktime_t last_update;
	unsigned int *freq_table;
	u64 *time;
	unsigned int max_freq_state;
	unsigned int max_idle_state;
	/* 0=active 1..idle states */
	unsigned int cur_idle_idx;
	/* 0~freq_num-1 */
	unsigned int cur_freq_idx;
#ifdef CONFIG_SMT_MODE_GOV
	struct time_in_smt *smt_time;
#endif

	bool initialized;
};

static DEFINE_PER_CPU(struct time_in_state, stats_info);

#ifdef CONFIG_SMT_MODE_GOV
static void update_smt_state(struct time_in_state *icpu, bool force)
{
	ktime_t now;
	u64 delta_time;
	unsigned int pos;
	unsigned int new_mode;
	struct time_in_smt *smt_time = icpu->smt_time;

	if (!smt_time)
		return;

	spin_lock(&smt_time->lock);
	new_mode = smt_get_mode(icpu->cpu);
	if (new_mode == smt_time->cur_mode_idx && !force)
		goto unlock;
	now = ktime_get();
	delta_time = (u64)ktime_us_delta(now, smt_time->last_update);

	pos = smt_time->cur_mode_idx * icpu->max_freq_state + icpu->cur_freq_idx;
	smt_time->time[pos] += delta_time;
	smt_time->last_update = now;

	if (new_mode < smt_time->max_mode_state)
		smt_time->cur_mode_idx = new_mode;
unlock:
	spin_unlock(&smt_time->lock);
}
#endif

/* called with icpu->lock held */
static void update_cur_state(struct time_in_state *icpu)
{
	ktime_t now;
	u64 delta_time;
	unsigned int pos;

	now = ktime_get();
	delta_time = (u64)ktime_us_delta(now, icpu->last_update);
	pos = icpu->cur_idle_idx * icpu->max_freq_state + icpu->cur_freq_idx;
	icpu->time[pos] += delta_time;
	icpu->last_update = now;
}

#ifdef CONFIG_SMT_MODE_GOV
static ssize_t lpcpu_time_in_smt_show(int cpu, char *buf)
{
	unsigned int i, j, pos;
	ssize_t len = 0;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids || buf == NULL)
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized || !icpu->smt_time)
		return -ENODEV;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	update_smt_state(icpu, true);
	spin_unlock(&icpu->lock);

	for (i = 0; i < icpu->max_freq_state; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len,
				 "%u", icpu->freq_table[i]);
		for (j = 0; j < icpu->smt_time->max_mode_state; j++) {
			pos = j * icpu->max_freq_state + i;
			len += scnprintf(buf + len, PAGE_SIZE - len,
					 " %llu", icpu->smt_time->time[pos]);
		}

		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	return len;
}

u64 get_mt_time_by_cpu(int cpu)
{
	struct time_in_state *icpu = NULL;
	u64 mt_load_time = 0;
	unsigned int pos;
	unsigned int i;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids)
		return mt_load_time;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized || !icpu->smt_time)
		return mt_load_time;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	update_smt_state(icpu, true);
	spin_unlock(&icpu->lock);

	if (icpu->smt_time->max_mode_state < NR_SMT_MODE + 1) {
		pr_err("PerfD %s: icpu->smt_time->max_mode_state %u\n",
			__func__, icpu->smt_time->max_mode_state);
		return 0;
	}

	for (i = 0; i < icpu->max_freq_state; i++) {
		pos = NR_SMT_MODE * icpu->max_freq_state + i;
		mt_load_time += icpu->smt_time->time[pos] / NSEC_PER_MSEC;
	}

	return mt_load_time;
}
#endif

static ssize_t lpcpu_time_in_state_show(int cpu, char *buf)
{
	unsigned int i, j, pos;
	ssize_t len = 0;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids || buf == NULL)
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized)
		return -ENODEV;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	spin_unlock(&icpu->lock);

	for (i = 0; i < icpu->max_freq_state; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len,
				 "%u", icpu->freq_table[i]);
		for (j = 0; j < icpu->max_idle_state; j++) {
			pos = j * icpu->max_freq_state + i;
			len += scnprintf(buf + len, PAGE_SIZE - len,
					 " %llu", icpu->time[pos]);
		}

		len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	}

	return len;
}

#ifdef CONFIG_PERF_CTRL
static int hisi_time_adj_freq_get(int cpu, u64 *timeadjfreq)
{
	unsigned int i;
	struct time_in_state *icpu = per_cpu_ptr(&stats_info, cpu);

	if (!icpu->initialized)
		return -ENODEV;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	spin_unlock(&icpu->lock);

	*timeadjfreq = 0;
	for (i = 0; i < icpu->max_freq_state; i++)
		*timeadjfreq += (icpu->freq_table[i] / KHZ_PER_MHZ) *
			div_u64(icpu->time[i], NSEC_PER_USEC);

	return 0;
}

int perf_ctrl_get_cpu_busy_time(void __user *uarg)
{
	struct cpu_busy_time busy_time;
	int i;
	int ret = 0;

	if (uarg == NULL)
		return -EINVAL;

	busy_time.cpu_count = (int)nr_cpu_ids;
	if (busy_time.cpu_count > NR_CPUS)
		return -EFAULT;

	for (i = 0; i < busy_time.cpu_count; i++) {
		ret = hisi_time_adj_freq_get(i, &busy_time.time_adj_freq[i]);
		if (ret < 0)
			return ret;
	}

	if (copy_to_user(uarg, &busy_time, sizeof(struct cpu_busy_time))) {
		pr_err("get busy time copy_to_user fail.\n");
		return -EFAULT;
	}
	return 0;
}
#endif

int lpcpu_time_in_freq_get(int cpu, u64 *freqs, int freqs_len)
{
	unsigned int i, j, pos;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids || freqs == NULL)
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized)
		return -ENODEV;

	if (freqs_len != (int)icpu->max_freq_state)
		return -EINVAL;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	spin_unlock(&icpu->lock);
	for (i = 0; i < (unsigned int)freqs_len; i++) {
		freqs[i] = 0;
		for (j = 0; j < icpu->max_idle_state; j++) {
			pos = j * icpu->max_freq_state + i;
			freqs[i] += icpu->time[pos];
		}
	}

	return 0;
}

#ifdef CONFIG_SMT_MODE_GOV
/* get in MT busy, ST busy, Coreidle sequence */
int lpcpu_time_in_smt_get(int cpu, u64 *busy_time, unsigned int mode_len)
{
	unsigned int i, j, pos;
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids || busy_time == NULL)
		return -EINVAL;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized || !icpu->smt_time)
		return -ENODEV;

	if (mode_len > icpu->smt_time->max_mode_state)
		return -EINVAL;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
	update_smt_state(icpu, true);
	spin_unlock(&icpu->lock);

	for (i = 0; i < mode_len; i++) {
		busy_time[i] = 0;

		for (j = 0; j < icpu->max_freq_state; j++) {
			pos = j + icpu->max_freq_state * i;
			busy_time[i] += icpu->smt_time->time[pos];
		}
	}

	return 0;
}
#endif

void time_in_state_update_idle(int cpu, unsigned int new_idle_index)
{
	struct time_in_state *icpu = NULL;

	if (cpu < 0 || cpu >= (int)nr_cpu_ids)
		return;

	icpu = per_cpu_ptr(&stats_info, cpu);
	if (!icpu->initialized || new_idle_index >= icpu->max_idle_state)
		return;

	spin_lock(&icpu->lock);
	update_cur_state(icpu);
#ifdef CONFIG_SMT_MODE_GOV
	update_smt_state(icpu, false);
#endif
	icpu->cur_idle_idx = new_idle_index;
	spin_unlock(&icpu->lock);
}

/* update the time of old freq when freq change */
void time_in_state_update_freq(struct cpumask *cpus,
			       unsigned int new_freq_index)
{
	int cpu;
	struct time_in_state *icpu = NULL;

	if (cpus == NULL || cpumask_empty(cpus))
		return;

	for_each_cpu(cpu, cpus) {
		icpu = per_cpu_ptr(&stats_info, cpu);
		if (!icpu->initialized ||
		    new_freq_index >= icpu->max_freq_state)
			continue;

		spin_lock(&icpu->lock);
		update_cur_state(icpu);
#ifdef CONFIG_SMT_MODE_GOV
		update_smt_state(icpu, true);
#endif
		icpu->cur_freq_idx = new_freq_index;
		spin_unlock(&icpu->lock);
	}
}

static int update_cpu_idle_state(struct notifier_block *nb __maybe_unused,
				 unsigned long action, void *data __maybe_unused)
{
	int idle_idx = (int)action + 1;

	time_in_state_update_idle(smp_processor_id(), idle_idx);

	return NOTIFY_OK;
}

static struct notifier_block idle_update_nb = {
	.notifier_call = update_cpu_idle_state,
	.priority = 0,
};

static ssize_t time_in_state_show(struct device *dev,
				  struct device_attribute *attr __maybe_unused, char *buf)
{
	return lpcpu_time_in_state_show(dev->id, buf);
}

static DEVICE_ATTR(time_in_state, 0440, time_in_state_show, NULL);


#ifdef CONFIG_SMT_MODE_GOV
static ssize_t time_in_smt_show(struct device *dev,
				 struct device_attribute *attr __maybe_unused, char *buf)
{
	return lpcpu_time_in_smt_show(dev->id, buf);
}

static DEVICE_ATTR(time_in_smt, 0440, time_in_smt_show, NULL);
#endif

static struct attribute *time_in_state_attrs[] = {
	&dev_attr_time_in_state.attr,
#ifdef CONFIG_SMT_MODE_GOV
	&dev_attr_time_in_smt.attr,
#endif
	NULL,
};
static const struct attribute_group time_in_state_attr_group = {
	.name	= power_group_name,
	.attrs	= time_in_state_attrs,
};

void time_in_state_sysfs_add(void)
{
	/* only cpu add time_in_state sysfs */
	int i;
	int ret = 0;

	for_each_possible_cpu(i) {
		struct device *dev = get_cpu_device(i);

		if (dev)
			ret = sysfs_merge_group(&dev->kobj,
						&time_in_state_attr_group);
		if (ret)
			pr_err("sysfs add fail for cpu:%d\n", i);
	}
}

#ifdef CONFIG_SMT_MODE_GOV
static void time_in_smt_init(struct time_in_state *icpu)
{
	int leader_cpu;

	if (cpumask_weight(cpu_smt_mask(icpu->cpu)) < 2)
		return;

	leader_cpu = cpumask_first(cpu_smt_mask(icpu->cpu));
	if (icpu->cpu == leader_cpu) {
		icpu->smt_time =
			kzalloc(sizeof(struct time_in_smt), GFP_KERNEL);
		if (!icpu->smt_time)
			return;

		spin_lock_init(&icpu->smt_time->lock);
		icpu->smt_time->leader_cpu = icpu->cpu;
		icpu->smt_time->cur_mode_idx = 1; /* default ST mode */
		icpu->smt_time->max_mode_state = NR_SMT_MODE + 1;

		icpu->smt_time->time =
			kzalloc(icpu->smt_time->max_mode_state *
				icpu->max_freq_state * sizeof(u64), GFP_KERNEL);
		if (!icpu->smt_time->time) {
			kfree(icpu->smt_time);
			icpu->smt_time = NULL;
		}
	} else {
		icpu->smt_time = per_cpu_ptr(&stats_info, leader_cpu)->smt_time;
	}
}
#endif

static int __init time_in_state_init(void)
{
	int cpu, ret, freq_index;
	struct time_in_state *icpu = NULL;
	struct cpuidle_device *dev = NULL;
	struct cpuidle_driver *drv = NULL;
	struct cpufreq_policy policy;
	unsigned int alloc_size, i;
	struct cpufreq_frequency_table *pos = NULL;

	for_each_online_cpu(cpu) {
		icpu = per_cpu_ptr(&stats_info, cpu);
		if (icpu->initialized)
			continue;

		icpu->cpu = cpu;
		spin_lock_init(&icpu->lock);

		/* init idle */
		dev = per_cpu(cpuidle_devices, cpu);
		drv = cpuidle_get_cpu_driver(dev);
		/* all idle states plus active state */
		icpu->max_idle_state = (drv != NULL) ? drv->state_count + 1 : 2;
		icpu->cur_idle_idx = 0;

		/* init freq */
		ret = cpufreq_get_policy(&policy, cpu);
		if (ret != 0)
			continue;

		i = 0;
		cpufreq_for_each_valid_entry(pos, policy.freq_table)
			i++;
		if (i == 0)
			continue;

		icpu->max_freq_state = i;
		alloc_size = (unsigned int)(i * sizeof(int) +
			     (icpu->max_idle_state * i * sizeof(u64)));
		icpu->freq_table = kzalloc(alloc_size, GFP_KERNEL);
		if (icpu->freq_table == NULL) {
			pr_err("time in stats alloc fail for cpu:%d\n", cpu);
			goto err_out;
		}
		icpu->time = (u64 *)(icpu->freq_table + icpu->max_freq_state);

		i = 0;
		cpufreq_for_each_valid_entry(pos, policy.freq_table)
			icpu->freq_table[i++] = pos->frequency;

		freq_index = cpufreq_frequency_table_target(&policy, policy.cur,
							    CPUFREQ_RELATION_C);
		if (freq_index < 0)
			continue;
		icpu->cur_freq_idx = freq_index;

#ifdef CONFIG_SMT_MODE_GOV
		time_in_smt_init(icpu);
#endif

		icpu->last_update = ktime_get();
		icpu->initialized = true;
	}

	(void)atomic_notifier_chain_register(&cpuidle_state_notifier, &idle_update_nb);
	time_in_state_sysfs_add();
	return 0;

err_out:
	for_each_online_cpu(cpu) {
		icpu = per_cpu_ptr(&stats_info, cpu);

		if (!icpu->initialized)
			continue;

		icpu->initialized = false;
		kfree(icpu->freq_table);
		icpu->freq_table = NULL;
#ifdef CONFIG_SMT_MODE_GOV
		if (icpu->smt_time &&
		    cpu == icpu->smt_time->leader_cpu) {
			kfree(icpu->smt_time->time);
			kfree(icpu->smt_time);
			icpu->smt_time = NULL;
		}
#endif
	}

	return -ENOMEM;
}
late_initcall(time_in_state_init);
