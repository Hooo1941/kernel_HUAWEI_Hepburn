// SPDX-License-Identifier: GPL-2.0-only
/* cee vendor_hooks.c
 *
 * HiSilicon Common Kernel Vendor Hook Support
 *
 * Copyright 2022 Huawei
 */

#define CREATE_HCK_VENDOR_HOOK
#ifdef CONFIG_CEE_CPU_HCK
#include <platform_include/cee/linux/hck/lpcpu/cpufreq/hck_lpcpu_cpufreq.h>
#include <platform_include/cee/linux/hck/lpcpu/gic/hck_lpcpu_gic.h>
#include <platform_include/cee/linux/hck/lpcpu/gic/hck_lpcpu_gic_its.h>
#include <platform_include/cee/linux/hck/lpcpu/pmu/hck_lpcpu_pmu.h>
#include <platform_include/cee/linux/hck/lpcpu/timer/hck_lpcpu_timer.h>
#include <platform_include/cee/linux/hck/lpcpu/cpuidle/hck_lpcpu_cpuidle.h>
#include <platform_include/cee/linux/hck/sched/hck_sched.h>
#include <platform_include/cee/linux/hck/lpcpu/mpam/hck_mpam.h>
#endif /* CONFIG_CEE_CPU_HCK */

#include <platform_include/cee/linux/hck/kernel/sched/hck_cpufreq_schedutil.h>
#undef CREATE_HCK_VENDOR_HOOK
