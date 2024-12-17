/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * Description : information about cpuidle
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

#ifndef __LPCPU_IDLE_DRV_H__
#define __LPCPU_IDLE_DRV_H__

#include <linux/cpumask.h>
#include <linux/spinlock.h>

#ifdef CONFIG_ARCH_PLATFORM

#ifdef RESOLUTION
#undef RESOLUTION
#endif

#define RESOLUTION 64

#ifdef DECAY
#undef DECAY
#endif

#define DECAY 128

#endif /* CONFIG_ARCH_PLATFORM */

#ifdef CONFIG_CPUIDLE_SKIP_ALL_CORE_DOWN
extern spinlock_t g_idle_spin_lock;
extern struct cpumask g_idle_cpus_mask;
extern struct cpumask g_core_idle_cpus_mask;
#define __g_idle_cpus_mask ((const struct cpumask *)&g_idle_cpus_mask)
#define __g_core_idle_cpus_mask ((const struct cpumask *)&g_core_idle_cpus_mask)
#define num_idle_cpus()		cpumask_weight(__g_idle_cpus_mask)
#define num_core_idle_cpus()	cpumask_weight(__g_core_idle_cpus_mask)
#endif

#ifdef CONFIG_CPUIDLE_LP_MODE
extern int get_lp_mode(void);
extern void cpuidle_switch_to_lp_mode(int enabled);
#endif

#ifdef CONFIG_CPUIDLE_MENU_GOV_HIST

#define HIST_SIZE 8

struct menu_hist_cstate_data {
	int	idle_state_idx;
	int	org_state_idx;
	u64	residency_ns;
	s64	exit_time_ns;
};

struct menu_hist_state_info {
	unsigned int	same_state_failed_count;
	u64	same_state_failed_ns;

	u64	min_residency_ns;
	unsigned int	total_count;
	u64	total_residency_ns;
	u64	total_ns;

	u64	last_run_ns;
};
#endif /* CONFIG_CPUIDLE_MENU_GOV_HIST */

#ifdef CONFIG_CPU_IDLE_GOV_ALIGN
extern void idle_irq_info_update(unsigned int hwirq);
extern void set_deepest_idle_flag(void);
#else
static inline void idle_irq_info_update(unsigned int hwirq) {}
static inline void set_deepest_idle_flag(void) {}
#endif
#endif /* __LPCPU_IDLE_DRV_H__ */
