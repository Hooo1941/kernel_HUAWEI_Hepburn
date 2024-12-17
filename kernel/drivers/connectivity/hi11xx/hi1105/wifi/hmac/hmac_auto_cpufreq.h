/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 调频功能头文件
 * 作    者 :
 * 创建日期 : 2015年12月10日
 */

#ifndef HMAC_AUTO_CPUFREQ_H
#define HMAC_AUTO_CPUFREQ_H

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#include <linux/pm_qos.h>
#include <linux/cpufreq.h>
#endif
#include "oal_hcc_bus.h"
#include "external/lpcpu_feature.h"
typedef struct {
    uint32_t max_cpu_freq;
    uint32_t valid;
}hisi_max_cpu_freq;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern struct cpumask g_st_fastcpus;
#endif
#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
extern struct cpufreq_req *g_ast_cpufreq;
extern hisi_max_cpu_freq *g_aul_cpumaxfreq;
#endif

void hmac_set_cpu_freq(uint8_t uc_req_freq_state);
uint32_t hisi_cpufreq_get_maxfreq(unsigned int cpu);
uint32_t hmac_hisi_cpufreq_init(void);
uint32_t hmac_hisi_cpufreq_exit(void);
#endif
