/*
 *
 * hck pmu module
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

#include <linux/init.h>
#include <linux/percpu-defs.h>
#include <linux/smp.h>
#include <asm/sysreg.h>
#include <linux/perf_event.h>
#include "lpcpu_dsu_pmu_cfg.h"
#include <platform_include/cee/linux/hck/lpcpu/pmu/hck_lpcpu_pmu.h>

static void lpcpu_pmu_dsu_pmu_init(struct dsu_pmu *dsu_pmu)
{
#if defined(CONFIG_ARCH_PLATFORM) && defined(CONFIG_ARM_DSU_PMU)
	spin_lock_init(&dsu_pmu->fcm_idle_lock);
	dsu_pmu->fcm_idle = false;
#endif
}

static void lpcpu_pmu_cpu_pm_dsu_pmu_register(struct dsu_pmu *dsu_pmu __maybe_unused,
					      char *name __maybe_unused,
					      int *rc __maybe_unused, int *hook_flag)
{
	*hook_flag = 0;
#if defined(CONFIG_ARCH_PLATFORM) && defined(CONFIG_CPU_PM) && defined(CONFIG_ARM_DSU_PMU)
	*hook_flag = 1;
	*rc = cpu_pm_dsu_pmu_register(dsu_pmu);
	if (*rc)
		return;
	*rc = perf_pmu_register(&dsu_pmu->pmu, name, PERF_TYPE_DSU);
	if (*rc)
		cpu_pm_dsu_pmu_unregister(dsu_pmu);
#endif
}

static void lpcpu_pmu_cpu_pm_dsu_pmu_unregister(struct dsu_pmu *dsu_pmu __maybe_unused)
{
#if defined(CONFIG_ARCH_PLATFORM) && defined(CONFIG_CPU_PM) && defined(CONFIG_ARM_DSU_PMU)
	cpu_pm_dsu_pmu_unregister(dsu_pmu);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

static void lpcpu_pmu_hook_flag(int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
#endif
}

#if defined(CONFIG_LP_ARM_PMU_PER_CPU_EVENT_HOOK)
static void lpcpu_pmu_arm_pmu_per_cpu_num_events(struct arm_pmu *armpmu,
						 int *num_events, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
	if (armpmu == NULL)
		return;
	*num_events = *this_cpu_ptr(armpmu->num_events);
#endif
}

static void lpcpu_pmu_arm_pmu_enable(struct arm_pmu *armpmu,
				     struct pmu_hw_events *hw_events,
				     int *enable, int *hook_flag)
{
#ifdef CONFIG_ARCH_PLATFORM
	int num_events = *this_cpu_ptr(armpmu->num_events);
	*hook_flag = 1;
	*enable = bitmap_weight(hw_events->used_mask, num_events);
#else
	*hook_flag = 0;
#endif
}

static void lpcpu_pmu_arm_pmu_per_cpu_register(struct arm_pmu *pmu, int *hook_flag,
					       bool has_nmi)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	int cpu;
	int num_events;

	*hook_flag = 1;
	for_each_cpu(cpu, &pmu->supported_cpus) {
		num_events = *per_cpu_ptr(pmu->num_events, cpu);
		pr_err("enabled with %s PMU driver, %d counters available%s for CPU%d\n",
			pmu->name, num_events, has_nmi ? ", using NMIs" : "", cpu);
	}
#endif
}

static void lpcpu_pmu_armv8pmu_get_single_idx(struct pmu_hw_events *cpuc,
					      struct arm_pmu *cpu_pmu,
					      struct perf_event *event,
					      int *idx, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	int num_events;
	int cpu;

	*hook_flag = 1;
	cpu = event->cpu < 0 ? smp_processor_id() : event->cpu;
	num_events = *per_cpu_ptr(cpu_pmu->num_events, cpu);

	for (*idx = ARMV8_IDX_COUNTER0; *idx < num_events; (*idx)++) {
		if (!test_and_set_bit(*idx, cpuc->used_mask))
			return;
	}
	*idx = -EAGAIN;
#endif
}

static void lpcpu_pmu_armv8pmu_get_chain_idx(struct pmu_hw_events *cpuc,
					     struct arm_pmu *cpu_pmu,
					     struct perf_event *event,
					     int *idx, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	int num_events;
	int cpu;

	*hook_flag = 1;
	cpu = event->cpu < 0 ? smp_processor_id() : event->cpu;
	num_events = *per_cpu_ptr(cpu_pmu->num_events, cpu);
	/*
	 * Chaining requires two consecutive event counters, where
	 * the lower idx must be even.
	 */
	for (*idx = ARMV8_IDX_COUNTER0 + 1; *idx < num_events; (*idx) += 2) {
		if (!test_and_set_bit(*idx, cpuc->used_mask)) {
			/* Check if the preceding even counter is available */
			if (!test_and_set_bit((*idx) - 1, cpuc->used_mask))
				return;
			/* Release the Odd counter */
			clear_bit(*idx, cpuc->used_mask);
		}
	}
	*idx = -EAGAIN;
#endif
}

static inline u32 armv8pmu_pmcr_read(void)
{
	return read_sysreg(pmcr_el0);
}

static void lpcpu_pmu_armv8pmu_pre_probe(cpumask_t *dstp, struct arm_pmu *cpu_pmu,
					 int *hook_flag)
{
#ifdef CONFIG_ARCH_PLATFORM
	int cpu = smp_processor_id();
	int *num_events = this_cpu_ptr(cpu_pmu->num_events);

	*hook_flag = 1;
	/* Read the nb of CNTx counters supported from PMNC */
	*num_events = (armv8pmu_pmcr_read() >> ARMV8_PMU_PMCR_N_SHIFT)
		      & ARMV8_PMU_PMCR_N_MASK;

	/* Add the CPU cycles counter */
	*num_events += 1;

	cpumask_set_cpu(cpu, dstp);
#else
	*hook_flag = 0;
#endif
}

static void lpcpu_pmu_armv8pmu_probe(cpumask_t *probe_supported_cpus, struct arm_pmu *cpu_pmu,
				     smp_call_func_t func, void *probe, int *ret,
				     int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_ARCH_PLATFORM
	*hook_flag = 1;
	cpu_pmu->num_events = alloc_percpu(int);
	if (cpu_pmu->num_events == NULL)
		*ret = -ENOMEM;

	cpumask_clear(probe_supported_cpus);
	on_each_cpu_mask(&cpu_pmu->supported_cpus, func, probe, 1);
	*ret = 0;
#endif
}
#endif /* CONFIG_LP_ARM_PMU_PER_CPU_EVENT_HOOK */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */

static int __init hck_lpcpu_pmu_register(void)
{
	REGISTER_HCK_VH(lpcpu_pmu_dsu_pmu_init, lpcpu_pmu_dsu_pmu_init);
	REGISTER_HCK_VH(lpcpu_pmu_cpu_pm_dsu_pmu_register, lpcpu_pmu_cpu_pm_dsu_pmu_register);
	REGISTER_HCK_VH(lpcpu_pmu_cpu_pm_dsu_pmu_unregister, lpcpu_pmu_cpu_pm_dsu_pmu_unregister);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	REGISTER_HCK_VH(lpcpu_pmu_hook_flag, lpcpu_pmu_hook_flag);
#if defined(CONFIG_LP_ARM_PMU_PER_CPU_EVENT_HOOK)
	REGISTER_HCK_VH(lpcpu_pmu_arm_pmu_per_cpu_num_events, lpcpu_pmu_arm_pmu_per_cpu_num_events);
	REGISTER_HCK_VH(lpcpu_pmu_arm_pmu_per_cpu_register, lpcpu_pmu_arm_pmu_per_cpu_register);
	REGISTER_HCK_VH(lpcpu_pmu_arm_pmu_enable, lpcpu_pmu_arm_pmu_enable);
	REGISTER_HCK_VH(lpcpu_pmu_armv8pmu_get_single_idx, lpcpu_pmu_armv8pmu_get_single_idx);
	REGISTER_HCK_VH(lpcpu_pmu_armv8pmu_get_chain_idx, lpcpu_pmu_armv8pmu_get_chain_idx);
	REGISTER_HCK_VH(lpcpu_pmu_armv8pmu_pre_probe, lpcpu_pmu_armv8pmu_pre_probe);
	REGISTER_HCK_VH(lpcpu_pmu_armv8pmu_probe, lpcpu_pmu_armv8pmu_probe);
#endif /* CONFIG_LP_ARM_PMU_PER_CPU_EVENT_HOOK */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
	return 0;
}

early_initcall(hck_lpcpu_pmu_register);
