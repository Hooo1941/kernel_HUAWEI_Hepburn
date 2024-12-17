/* drivers/cpufreq/cpufreq_times.c
 *
 * Copyright (C) 2018 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/cpufreq.h>
#include <linux/cpufreq_times.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/threads.h>
#include <trace/hooks/cpufreq.h>
#include <../kernel/sched/sched.h>

/* DMIPS is 0.65 scale in MT duration. */
#define mt_diff_scale(x) (((x) * 35) / 100)

static DEFINE_SPINLOCK(task_time_in_state_lock); /* task->time_in_state */

static struct cpu_freqs *all_freqs[NR_CPUS];

static unsigned int next_offset;

struct cpu_freqs **cpufreq_get_all_freqs(void)
{
	return all_freqs;
}

spinlock_t *cpufreq_get_time_in_state_lock(void)
{
	return &task_time_in_state_lock;
}

#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
void set_cpufreq_task_times_total(struct task_struct *p, void *temp_total, unsigned int max_state)
{
	p->time_in_state_total = temp_total;
	p->max_state_total = max_state;
}

void *cpufreq_task_times_total_alloc(struct task_struct *p, unsigned int max_state)
{
#ifdef CONFIG_SMT_MODE_GOV
	return kcalloc(max_state * NR_SMT_MODE, sizeof(p->time_in_state_total[0]), GFP_ATOMIC);
#else
	return kcalloc(max_state, sizeof(p->time_in_state_total[0]), GFP_ATOMIC);
#endif
}

void cpufreq_task_times_total_exit(struct task_struct *p)
{
	void *temp_total;

	if (p->time_in_state_total) {
		temp_total = p->time_in_state_total;
		p->time_in_state_total = NULL;
		kfree(temp_total);
	}
}

static int cpufreq_task_times_realloc_locked_total(struct task_struct *p)
{
	void *temp;
	unsigned int max_state = READ_ONCE(next_offset);

#ifdef CONFIG_SMT_MODE_GOV
	temp = krealloc(p->time_in_state_total, max_state * NR_SMT_MODE * sizeof(u64), GFP_ATOMIC);
#else
	temp = krealloc(p->time_in_state_total, max_state * sizeof(u64), GFP_ATOMIC);
#endif
	if (!temp)
		return -ENOMEM;

	p->time_in_state_total = temp;

#ifdef CONFIG_SMT_MODE_GOV
	memset_s(p->time_in_state_total + p->max_state_total * NR_SMT_MODE,
			(max_state - p->max_state_total) * NR_SMT_MODE * sizeof(u64),
			0, (max_state - p->max_state_total) * NR_SMT_MODE * sizeof(u64));
#endif

	memset_s(p->time_in_state_total + p->max_state_total,
			(max_state - p->max_state_total) * sizeof(u64),
			0, (max_state - p->max_state_total) * sizeof(u64));

	p->max_state_total = max_state;

	return 0;
}

void cpufreq_acct_time_in_state_total(struct task_struct *p, unsigned int state, u64 cputime)
{
	struct task_struct *main_thread;

	main_thread = p->group_leader;
	if ((state < main_thread->max_state_total || !cpufreq_task_times_realloc_locked_total(main_thread)) &&
	    main_thread->time_in_state_total) {
		main_thread->time_in_state_total[state] += cputime;
#ifdef CONFIG_SMT_MODE_GOV
		if (smt_get_mode(task_cpu(p)) > 1)
			main_thread->time_in_state_total[main_thread->max_state_total + state] += cputime;
#endif
	}
}

int proc_time_in_state_total_show(struct seq_file *m, struct pid_namespace *ns,
	struct pid *pid, struct task_struct *p)
{
	unsigned int cpu, i;
	u64 cputime;
#ifdef CONFIG_SMT_MODE_GOV
	u64 mt_cputime;
#endif
	unsigned long flags;
	struct cpu_freqs *freqs;
	struct cpu_freqs *last_freqs = NULL;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	for_each_possible_cpu(cpu) {
		freqs = all_freqs[cpu];
		if (!freqs || freqs == last_freqs)
			continue;
		last_freqs = freqs;

		seq_printf(m, "cpu%u\n", cpu);
		for (i = 0; i < freqs->max_state; i++) {
#ifdef CONFIG_SMT_MODE_GOV
			cputime = 0;
			mt_cputime = 0;
			if (freqs->offset + i < p->max_state_total &&
			    p->time_in_state_total) {
				cputime = p->time_in_state_total[freqs->offset + i];
				mt_cputime = p->time_in_state_total[p->max_state_total + freqs->offset + i];
			}
			seq_printf(m, "%u %lu %lu\n", freqs->freq_table[i],
				   (unsigned long)nsec_to_clock_t(cputime),
				   (unsigned long)nsec_to_clock_t(mt_cputime));
#else
			cputime = 0;
			if (freqs->offset + i < p->max_state &&
			    p->time_in_state_total)
				cputime = p->time_in_state_total[freqs->offset + i];
			seq_printf(m, "%u %lu\n", freqs->freq_table[i],
				   (unsigned long)nsec_to_clock_t(cputime));
#endif
		}
	}
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	return 0;
}
#endif

void cpufreq_task_times_init(struct task_struct *p)
{
	unsigned long flags;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	p->time_in_state = NULL;
#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
	set_cpufreq_task_times_total(p, NULL, 0);
#endif
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	p->max_state = 0;
}

void cpufreq_task_times_alloc(struct task_struct *p)
{
	void *temp;
	unsigned long flags;
	unsigned int max_state = READ_ONCE(next_offset);

	/* We use one array to avoid multiple allocs per task */
#ifdef CONFIG_SMT_MODE_GOV
	temp = kcalloc(max_state * NR_SMT_MODE, sizeof(p->time_in_state[0]), GFP_ATOMIC);
#else
	temp = kcalloc(max_state, sizeof(p->time_in_state[0]), GFP_ATOMIC);
#endif
	if (!temp)
		return;

#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
	void *temp_total;

	temp_total = cpufreq_task_times_total_alloc(p, max_state);
#endif

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	p->time_in_state = temp;
#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
	if (temp_total)
		set_cpufreq_task_times_total(p, temp_total, max_state);
#endif
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	p->max_state = max_state;
}

/* Caller must hold task_time_in_state_lock */
static int cpufreq_task_times_realloc_locked(struct task_struct *p)
{
	void *temp;
	unsigned int max_state = READ_ONCE(next_offset);

#ifdef CONFIG_SMT_MODE_GOV
	temp = krealloc(p->time_in_state, max_state * NR_SMT_MODE * sizeof(u64), GFP_ATOMIC);
#else
	temp = krealloc(p->time_in_state, max_state * sizeof(u64), GFP_ATOMIC);
#endif
	if (!temp)
		return -ENOMEM;
	p->time_in_state = temp;
#ifdef CONFIG_SMT_MODE_GOV
	memset(p->time_in_state + p->max_state * NR_SMT_MODE, 0,
	       (max_state - p->max_state) * NR_SMT_MODE * sizeof(u64));
#endif
	memset(p->time_in_state + p->max_state, 0,
	       (max_state - p->max_state) * sizeof(u64));

	p->max_state = max_state;

	return 0;
}

void cpufreq_task_times_exit(struct task_struct *p)
{
	unsigned long flags;
	void *temp;

	if (!p->time_in_state)
		return;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	temp = p->time_in_state;
	p->time_in_state = NULL;
#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
	cpufreq_task_times_total_exit(p);
#endif
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	kfree(temp);
}

int proc_time_in_state_show(struct seq_file *m, struct pid_namespace *ns,
	struct pid *pid, struct task_struct *p)
{
	unsigned int cpu, i;
	u64 cputime;
#ifdef CONFIG_SMT_MODE_GOV
	u64 mt_cputime;
#endif
	unsigned long flags;
	struct cpu_freqs *freqs;
	struct cpu_freqs *last_freqs = NULL;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	for_each_possible_cpu(cpu) {
		freqs = all_freqs[cpu];
		if (!freqs || freqs == last_freqs)
			continue;
		last_freqs = freqs;

		seq_printf(m, "cpu%u\n", cpu);
		for (i = 0; i < freqs->max_state; i++) {
#ifdef CONFIG_SMT_MODE_GOV
			cputime = 0;
			mt_cputime = 0;
			if (freqs->offset + i < p->max_state &&
			    p->time_in_state) {
				cputime = p->time_in_state[freqs->offset + i];
				mt_cputime = p->time_in_state[p->max_state + freqs->offset + i];
			}
			seq_printf(m, "%u %lu %lu\n", freqs->freq_table[i],
				   (unsigned long)nsec_to_clock_t(cputime),
				   (unsigned long)nsec_to_clock_t(mt_cputime));
#else
			cputime = 0;
			if (freqs->offset + i < p->max_state &&
			    p->time_in_state)
				cputime = p->time_in_state[freqs->offset + i];
			seq_printf(m, "%u %lu\n", freqs->freq_table[i],
				   (unsigned long)nsec_to_clock_t(cputime));
#endif
		}
	}
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	return 0;
}

#ifdef CONFIG_HUAWEI_PERFD
static u64 get_proc_each_cpu_load(struct cpu_freqs *freqs, struct task_struct *p,
	char dmips_value)
{
	unsigned int i;
	u64 cputime;
#ifdef CONFIG_SMT_MODE_GOV
	u64 mt_cputime;
#endif
	u64 cputime_total;
	cputime_total = 0;
	struct task_struct *child = p;
	for (i = 0; i < freqs->max_state; i++) {
#ifdef CONFIG_SMT_MODE_GOV
		cputime = 0;
		mt_cputime = 0;
		rcu_read_lock();
		do {
			if (freqs->offset + i < child->max_state &&
				child->time_in_state) {
				cputime += child->time_in_state[freqs->offset + i];
				mt_cputime += child->time_in_state[child->max_state + freqs->offset + i];
			}
		} while_each_thread(p, child);
		rcu_read_unlock();
		cputime_total += (freqs->freq_table[i] * ((cputime - mt_diff_scale(mt_cputime)) / NSEC_PER_MSEC) * dmips_value);
#else
		cputime = 0;
		rcu_read_lock();
		do {
			if (freqs->offset + i < child->max_state &&
				child->time_in_state)
				cputime += child->time_in_state[freqs->offset + i];
		} while_each_thread(p, child);
		rcu_read_unlock();
		cputime_total += (freqs->freq_table[i] * (cputime / NSEC_PER_MSEC) *
			dmips_value);
#endif
	}
	return cputime_total;
}

u64 get_proc_cpu_load(struct task_struct *p, int dmips_value_buffer[], unsigned int dmips_num)
{
	unsigned int cpu;
	u64 cputime_total = 0;
	unsigned long flags;
	struct cpu_freqs *freqs;
	struct cpu_freqs *last_freqs = NULL;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	for_each_possible_cpu(cpu) {
		freqs = all_freqs[cpu];
		if (!freqs || freqs == last_freqs)
			continue;
		last_freqs = freqs;
		if (cpu >= dmips_num)
			break;
		cputime_total += get_proc_each_cpu_load(freqs, p, dmips_value_buffer[cpu]);
	}
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	return cputime_total;
}

u64 get_cpuload_total(struct task_struct *p, int dmips_value_buffer[],
	unsigned int dmips_num, bool report_thread)
{
	unsigned int cpu, i;
	u64 cputime;
#ifdef CONFIG_SMT_MODE_GOV
	u64 mt_cputime;
#endif
	u64 cpuload_total = 0;
	struct cpu_freqs *freqs;
	struct cpu_freqs *last_freqs = NULL;
	unsigned int max_state;
	u64 *time_in_state;

#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
	if (report_thread) {
		max_state = p->max_state;
		time_in_state = p->time_in_state;
	} else {
		max_state = p->max_state_total;
		time_in_state = p->time_in_state_total;
	}
#else
	max_state = p->max_state;
	time_in_state = p->time_in_state;
#endif

	for_each_possible_cpu(cpu) {
		freqs = all_freqs[cpu];
		if (!freqs || freqs == last_freqs)
			continue;
		last_freqs = freqs;
		if (cpu >= dmips_num)
			break;

		for (i = 0; i < freqs->max_state; i++) {
			cputime = 0;
#ifdef CONFIG_SMT_MODE_GOV
			mt_cputime = 0;
			if (freqs->offset + i < max_state && time_in_state) {
				cputime = time_in_state[freqs->offset + i];
				mt_cputime = time_in_state[max_state + freqs->offset + i];
			}
			cpuload_total += (freqs->freq_table[i] *
				((cputime - mt_diff_scale(mt_cputime)) / NSEC_PER_MSEC) * dmips_value_buffer[cpu]);
#else
			if (freqs->offset + i < max_state && time_in_state)
				cputime = time_in_state[freqs->offset + i];
			cpuload_total += (freqs->freq_table[i] *
				(cputime / NSEC_PER_MSEC) * dmips_value_buffer[cpu]);
#endif
		}
	}

	return cpuload_total;
}
#endif

void cpufreq_acct_update_power(struct task_struct *p, u64 cputime)
{
	unsigned long flags;
	unsigned int state;
	struct cpu_freqs *freqs = all_freqs[task_cpu(p)];

	if (!freqs || is_idle_task(p) || p->flags & PF_EXITING)
		return;

	state = freqs->offset + READ_ONCE(freqs->last_index);

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	if ((state < p->max_state || !cpufreq_task_times_realloc_locked(p)) &&
	    p->time_in_state) {
		p->time_in_state[state] += cputime;
#ifdef CONFIG_SMT_MODE_GOV
		if (smt_get_mode(task_cpu(p)) > 1)
			p->time_in_state[p->max_state + state] += cputime;
#endif

#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
		cpufreq_acct_time_in_state_total(p, state, cputime);
#endif

#ifdef CONFIG_CPU_FREQ_POWER_STAT
		/* Account power usage */
		if (p->cpu_power != ULLONG_MAX)
			p->cpu_power += (unsigned long long)((unsigned long)freqs->current_table[freqs->last_index] * cputime / NSEC_PER_MSEC);
#endif
	}
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);

	trace_android_vh_cpufreq_acct_update_power(cputime, p, state);
}

static int cpufreq_times_get_index(struct cpu_freqs *freqs, unsigned int freq)
{
	int index;
        for (index = 0; index < freqs->max_state; ++index) {
		if (freqs->freq_table[index] == freq)
			return index;
        }
	return -1;
}

void cpufreq_times_create_policy(struct cpufreq_policy *policy)
{
	int cpu, index = 0;
	unsigned int count = 0;
	struct cpufreq_frequency_table *pos, *table;
	struct cpu_freqs *freqs;
	void *tmp;

	if (all_freqs[policy->cpu])
		return;

	table = policy->freq_table;
	if (!table)
		return;

	cpufreq_for_each_valid_entry(pos, table)
		count++;

	tmp =  kzalloc(sizeof(*freqs) + sizeof(freqs->freq_table[0]) * count,
		       GFP_KERNEL);
	if (!tmp)
		return;

	freqs = tmp;
	freqs->max_state = count;

#ifdef CONFIG_CPU_FREQ_POWER_STAT
	freqs->current_table = kzalloc(sizeof(int) * count, GFP_KERNEL);
	if (!freqs->current_table) {
		kfree(freqs);
		return;
	}
#endif

	cpufreq_for_each_valid_entry(pos, table) {
		freqs->freq_table[index] = pos->frequency;
#ifdef CONFIG_CPU_FREQ_POWER_STAT
		freqs->current_table[index] = pos->electric_current;
#endif
		index++;
	}
	index = cpufreq_times_get_index(freqs, policy->cur);
	if (index >= 0)
		WRITE_ONCE(freqs->last_index, index);

	freqs->offset = next_offset;
	WRITE_ONCE(next_offset, freqs->offset + count);
	for_each_cpu(cpu, policy->related_cpus)
		all_freqs[cpu] = freqs;
}

void cpufreq_times_record_transition(struct cpufreq_policy *policy,
	unsigned int new_freq)
{
	int index;
	struct cpu_freqs *freqs = all_freqs[policy->cpu];
	if (!freqs)
		return;

	index = cpufreq_times_get_index(freqs, new_freq);
	if (index >= 0)
		WRITE_ONCE(freqs->last_index, index);
}
