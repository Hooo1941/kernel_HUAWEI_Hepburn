/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: header of thermal ipa framework
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LPCPU_DSU_PMU_CFG__
#define __LPCPU_DSU_PMU_CFG__

#include <linux/types.h>
#include <linux/bitmap.h>
#include <linux/of_device.h>
#include <linux/spinlock.h>
#include <linux/cpumask.h>
#include <linux/perf_event.h>
#include <linux/perf/arm_pmu.h>
#include <linux/rcupdate.h>
#include <linux/cpu_pm.h>

/* The following code must be consistent with the source code */
#define DSU_PMU_MAX_HW_CNTRS		32
#define DSU_PMU_MAX_COMMON_EVENTS	0x40
#define DSU_PMU_IDX_CYCLE_COUNTER	31
#define	ARMV8_IDX_COUNTER0	1

struct dsu_hw_events {
	DECLARE_BITMAP(used_mask, DSU_PMU_MAX_HW_CNTRS);
	struct perf_event	*events[DSU_PMU_MAX_HW_CNTRS];
};

struct dsu_pmu {
	struct pmu			pmu;
	struct device			*dev;
	raw_spinlock_t			pmu_lock;
	struct dsu_hw_events		hw_events;
	cpumask_t			associated_cpus;
	cpumask_t			active_cpu;
	struct hlist_node		cpuhp_node;
	s8				num_counters;
	int				irq;
#ifdef CONFIG_ARCH_PLATFORM
	struct notifier_block	cpu_pm_nb;
	bool fcm_idle;
	/* to protect fcm_idle */
	spinlock_t fcm_idle_lock;
#endif
	DECLARE_BITMAP(cpmceid_bitmap, DSU_PMU_MAX_COMMON_EVENTS);
};

/* apis */
#if defined(CONFIG_ARCH_PLATFORM) && defined(CONFIG_CPU_PM) && defined(CONFIG_ARM_DSU_PMU)
int cpu_pm_dsu_pmu_register(struct dsu_pmu *dsu_pmu);
void cpu_pm_dsu_pmu_unregister(struct dsu_pmu *dsu_pmu);
#else
static inline int cpu_pm_dsu_pmu_register(struct dsu_pmu *dsu_pmu __maybe_unused) { return 0; }
static inline void cpu_pm_dsu_pmu_unregister(struct dsu_pmu *dsu_pmu __maybe_unused) { }
#endif /* CONFIG_CPU_PM and CONFIG_CPU_PM */

#endif /* __LPCPU_DSU_PMU_CFG__ */
