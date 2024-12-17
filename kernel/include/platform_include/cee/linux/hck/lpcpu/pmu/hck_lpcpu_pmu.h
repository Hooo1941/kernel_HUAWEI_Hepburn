#ifndef __HCK_LPCPU_PMU__
#define __HCK_LPCPU_PMU__

#include <linux/types.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */
struct dsu_pmu;
DECLARE_HCK_VH(lpcpu_pmu_dsu_pmu_init,
	       TP_PROTO(struct dsu_pmu *dsu_pmu),
	       TP_ARGS(dsu_pmu));
DECLARE_HCK_VH(lpcpu_pmu_cpu_pm_dsu_pmu_register,
	       TP_PROTO(struct dsu_pmu *dsu_pmu, char *name, int *rc, int *hook_flag),
	       TP_ARGS(dsu_pmu, name, rc, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_cpu_pm_dsu_pmu_unregister,
	       TP_PROTO(struct dsu_pmu *dsu_pmu),
	       TP_ARGS(dsu_pmu));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
struct arm_pmu;
struct pmu_hw_events;
struct perf_event;
DECLARE_HCK_VH(lpcpu_pmu_hook_flag,
	       TP_PROTO(int *hook_flag),
	       TP_ARGS(hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_arm_pmu_per_cpu_num_events,
	       TP_PROTO(struct arm_pmu *armpmu, int *num_events, int *hook_flag),
	       TP_ARGS(armpmu, num_events, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_arm_pmu_per_cpu_register,
	       TP_PROTO(struct arm_pmu *pmu, int *hook_flag, bool has_nmi),
	       TP_ARGS(pmu, hook_flag, has_nmi));
DECLARE_HCK_VH(lpcpu_pmu_arm_pmu_enable,
	       TP_PROTO(struct arm_pmu *armpmu, struct pmu_hw_events *hw_events,
			int *enable, int *hook_flag),
	       TP_ARGS(armpmu, hw_events, enable, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_armv8pmu_get_single_idx,
	       TP_PROTO(struct pmu_hw_events *cpuc, struct arm_pmu *cpu_pmu,
			struct perf_event *event, int *idx, int *hook_flag),
	       TP_ARGS(cpuc, cpu_pmu, event, idx, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_armv8pmu_get_chain_idx,
	       TP_PROTO(struct pmu_hw_events *cpuc, struct arm_pmu *cpu_pmu,
			struct perf_event *event, int *idx, int *hook_flag),
	       TP_ARGS(cpuc, cpu_pmu, event, idx, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_armv8pmu_pre_probe,
	       TP_PROTO(cpumask_t *dstp, struct arm_pmu *cpu_pmu, int *hook_flag),
	       TP_ARGS(dstp, cpu_pmu, hook_flag));
DECLARE_HCK_VH(lpcpu_pmu_armv8pmu_probe,
	       TP_PROTO(cpumask_t *probe_supported_cpus, struct arm_pmu *cpu_pmu,
			smp_call_func_t func, void *probe, int *ret, int *hook_flag),
	       TP_ARGS(probe_supported_cpus, cpu_pmu, func, probe, ret, hook_flag));
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
#endif /* __HCK_LPCPU_PMU__ */
