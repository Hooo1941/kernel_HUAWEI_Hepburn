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

#ifndef _LINUX_CPUFREQ_TIMES_H
#define _LINUX_CPUFREQ_TIMES_H
#define NS_TO_MS 1000000

#include <linux/cpufreq.h>
#include <linux/pid.h>

/**
 * struct cpu_freqs - per-cpu frequency information
 * @offset: start of these freqs' stats in task time_in_state array
 * @max_state: number of entries in freq_table
 * @last_index: index in freq_table of last frequency switched to
 * @freq_table: list of available frequencies
 */
struct cpu_freqs {
	unsigned int offset;
	unsigned int max_state;
	unsigned int last_index;
#ifdef CONFIG_CPU_FREQ_POWER_STAT
	unsigned int *current_table;
#endif
	unsigned int freq_table[0];
};

struct cpu_freqs **cpufreq_get_all_freqs(void);
spinlock_t *cpufreq_get_time_in_state_lock(void);

#ifdef CONFIG_CPU_FREQ_TIMES
void cpufreq_task_times_init(struct task_struct *p);
void cpufreq_task_times_alloc(struct task_struct *p);
void cpufreq_task_times_exit(struct task_struct *p);
int proc_time_in_state_show(struct seq_file *m, struct pid_namespace *ns,
			    struct pid *pid, struct task_struct *p);
#ifdef CONFIG_CPU_FREQ_TIMES_TOTAL
int proc_time_in_state_total_show(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *p);
#endif
void cpufreq_acct_update_power(struct task_struct *p, u64 cputime);
void cpufreq_times_create_policy(struct cpufreq_policy *policy);
void cpufreq_times_record_transition(struct cpufreq_policy *policy,
                                     unsigned int new_freq);
#ifdef CONFIG_HUAWEI_PERFD
u64 get_proc_cpu_load(struct task_struct *p, int dmips_value_buffer[],
	unsigned int dmips_num);
extern u64 get_cpuload_total(struct task_struct *p, int dmips_value_buffer[],
	unsigned int dmips_num, bool report_thread);
#endif
#else
static inline void cpufreq_task_times_init(struct task_struct *p) {}
static inline void cpufreq_task_times_alloc(struct task_struct *p) {}
static inline void cpufreq_task_times_exit(struct task_struct *p) {}
static inline void cpufreq_acct_update_power(struct task_struct *p,
					     u64 cputime) {}
static inline void cpufreq_times_create_policy(struct cpufreq_policy *policy) {}
static inline void cpufreq_times_record_transition(
	struct cpufreq_policy *policy, unsigned int new_freq) {}
#endif /* CONFIG_CPU_FREQ_TIMES */
#endif /* _LINUX_CPUFREQ_TIMES_H */
