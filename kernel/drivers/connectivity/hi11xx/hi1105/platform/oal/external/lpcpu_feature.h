/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:lpcpu_feature.h header file
 * Author: @CompanyNameTag
 */

#ifndef LPCPU_FEATURE_H
#define LPCPU_FEATURE_H

#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_ARCH_PLATFORM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/platform_drivers/lpcpu_cpufreq_req.h>
#include <linux/platform_drivers/hisi_core_ctl.h>
#else
#include <linux/hisi/lpcpu_cpufreq_req.h>
#include <linux/core_ctl.h>
#endif
#define external_cpufreq_init_req     lpcpu_cpufreq_init_req
#define external_cpufreq_exit_req     lpcpu_cpufreq_exit_req
#define external_cpufreq_update_req   lpcpu_cpufreq_update_req
#else
#include <linux/hisi/hisi_cpufreq_req.h>
#include <linux/hisi/hisi_core_ctl.h>
#define external_cpufreq_init_req     hisi_cpufreq_init_req
#define external_cpufreq_exit_req     hisi_cpufreq_exit_req
#define external_cpufreq_update_req   hisi_cpufreq_update_req
#endif
#endif
#endif

#ifndef _PRE_PRODUCT_HI1620S_KUNPENG
#if defined(CONFIG_ARCH_HISI)
#if defined(CONFIG_ARCH_PLATFORM)
extern void get_slow_cpus(struct cpumask *cpumask);
extern void get_fast_cpus(struct cpumask *cpumask);

#define external_get_slow_cpus  get_slow_cpus
#define external_get_fast_cpus  get_fast_cpus
#else
extern void hisi_get_slow_cpus(struct cpumask *cpumask);
extern void hisi_get_fast_cpus(struct cpumask *cpumask);
#define external_get_slow_cpus  hisi_get_slow_cpus
#define external_get_fast_cpus  hisi_get_fast_cpus

#endif /* endif for  CONFIG_ARCH_PLATFORM */

static inline void oal_cpumask_clear_cpu(uint32_t cpu, struct cpumask *cpumask)
{
#if defined(CONFIG_NR_CPUS)
#if defined(CONFIG_SCHED_SMT)
    /* 支持SMT的平台,需调用cpu_smt_mask获取物理核下的所有虚拟核,并将对应bit置0,才能完全屏蔽物理核 */
    cpumask_andnot(cpumask, cpumask, cpu_smt_mask(cpu));
#else
    cpumask_clear_cpu(cpu, cpumask);
#endif
#endif /* endif for CONFIG_NR_CPUS */
}

#endif /* endif for CONFIG_ARCH_HISI */
#endif /* endif for _PRE_PRODUCT_HI1620S_KUNPENG */

int32_t bfgx_ilde_sleep_vote_nolock(uint32_t index, uint32_t val);
void wlan_pm_idle_sleep_vote(uint8_t uc_allow);


#endif // __EXTERNAL_FEATURE_H__
