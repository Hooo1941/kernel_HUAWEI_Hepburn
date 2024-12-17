/*
 * heca.h
 *
 * hisilicon efficinecy control algorithm header file.
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
#ifndef _PLAT_HECA_H_
#define _PLAT_HECA_H_

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/devfreq.h>

enum heca_devfreq_id {
	HECA_DEVFREQ_L3C,
	HECA_DEVFREQ_DDR,
	HECA_DEVFREQ_GPU,
	HECA_DEVFREQ_MAX,
};

#ifdef CONFIG_HECA

bool heca_cpu_enable(int cpu);
bool heca_cluster_enable(int cluster_id);
bool heca_gpu_enable(void);

u32 heca_get_next_freq(int cpu, bool boosted, u32 freq);
unsigned int heca_get_kernel_freq(struct cpufreq_policy *policy, unsigned int freq);
unsigned int heca_get_kernel_freq_index(struct cpufreq_policy *policy, unsigned int index);

void heca_register_dev_freq_notifier(int devfreq_id, struct devfreq *devfreq);
void heca_unregister_dev_freq_notifier(int devfreq_id, struct devfreq *devfreq);

int heca_perf_ctrl(void __user *uarg);
void heca_context_switch(int cpu, struct task_struct *prev, struct task_struct *next);

#else

static inline bool heca_cpu_enable(int cpu)
{
	return false;
}

static inline bool heca_cluster_enable(int cluster_id)
{
	return false;
}

static inline bool heca_gpu_enable()
{
	return false;
}

static inline u32 heca_get_next_freq(int cpu, bool boosted, u32 freq)
{
	return freq;
}

static inline unsigned int heca_get_kernel_freq(int cpu, unsigned int freq)
{
	return freq;
}

static inline unsigned int heca_get_kernel_freq_index(int cpu, unsigned int index)
{
	return index;
}

static inline void heca_register_dev_freq_notifier(int devfreq_id, struct devfreq *devfreq)
{
}

static inline void heca_unregister_dev_freq_notifier(int devfreq_id, struct devfreq *devfreq)
{
}

static inline int heca_perf_ctrl(void __user *uarg)
{
	return 0;
}

static inline void heca_context_switch(int cpu, struct task_struct *prev, struct task_struct *next)
{
}

#endif

#endif
