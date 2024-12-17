/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 调频功能
 * 作    者 :
 * 创建日期 : 2015年12月10日
 */

#include "oal_types.h"
#include "oam_ext_if.h"
#include "frw_task.h"
#include "mac_vap.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_auto_cpufreq.h"

#include "external/oal_pm_qos_interface.h"
#ifdef CONFIG_NETWORK_LATENCY_PLATFORM_QOS
#include <linux/pm_qos.h>
#endif
#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
#include <linux/cpufreq.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_AUTO_CPUFREQ_C

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
struct cpumask g_st_fastcpus;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && defined(CONFIG_PLATFORM_QOS)
struct platform_qos_request g_st_pmqos_requset;
#else
struct pm_qos_request g_st_pmqos_requset;
#endif


#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
struct cpufreq_req *g_ast_cpufreq = NULL;
hisi_max_cpu_freq *g_aul_cpumaxfreq = NULL;

#define HMAC_MAX_CPU_FREQ 2516000 // kHZ
OAL_STATIC uint32_t hmac_get_max_cpu_freq(uint8_t cpu_id)
{
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
    if (g_freq_lock_control.limit_cpu_freq) {
        return oal_min(g_aul_cpumaxfreq[cpu_id].max_cpu_freq, HMAC_MAX_CPU_FREQ);
    }
#endif
    return g_aul_cpumaxfreq[cpu_id].max_cpu_freq;
}

static void hmac_hisi_free_cpufreq_res(void)
{
    if (g_aul_cpumaxfreq != NULL) {
        oal_free(g_aul_cpumaxfreq);
        g_aul_cpumaxfreq = NULL;
    }
    if (g_ast_cpufreq != NULL) {
        oal_free(g_ast_cpufreq);
        g_ast_cpufreq = NULL;
    }
}

OAL_STATIC void hmac_lock_max_cpu_freq(void)
{
    uint8_t uc_cpuid_loop;

    if (oal_any_null_ptr2(g_aul_cpumaxfreq, g_ast_cpufreq)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_lock_max_cpu_freq:ptr is NULL!}");
        hmac_hisi_free_cpufreq_res();
        return;
    }
    /* 所有核都锁定最高频率 */
    for (uc_cpuid_loop = 0; uc_cpuid_loop < OAL_BUS_MAXCPU_NUM; uc_cpuid_loop++) {
        /* 未获取到正确的cpu频率则不设置 */
        if (g_aul_cpumaxfreq[uc_cpuid_loop].valid != OAL_TRUE) {
            continue;
        }

        external_cpufreq_update_req(&g_ast_cpufreq[uc_cpuid_loop], hmac_get_max_cpu_freq(uc_cpuid_loop));
    }
}

OAL_STATIC void hmac_unlock_max_cpu_freq(void)
{
    uint8_t uc_cpuid_loop;

    if (oal_any_null_ptr2(g_aul_cpumaxfreq, g_ast_cpufreq)) {
        oam_error_log0(0, OAM_SF_ANY, "{hmac_unlock_max_cpu_freq:ptr is NULL!}");
        hmac_hisi_free_cpufreq_res();
        return;
    }
    for (uc_cpuid_loop = 0; uc_cpuid_loop < OAL_BUS_MAXCPU_NUM; uc_cpuid_loop++) {
        /* 未获取到正确的cpu频率则不设置 */
        if (g_aul_cpumaxfreq[uc_cpuid_loop].valid != OAL_TRUE) {
            continue;
        }

        external_cpufreq_update_req(&g_ast_cpufreq[uc_cpuid_loop], 0);
    }
}

void hmac_lock_cpu_freq_high_throughput_proc(void)
{
    if (g_freq_lock_control.uc_lock_max_cpu_freq == OAL_TRUE) {
        /* 当前还存在锁频后频率会掉下来，并且后面也无法锁到最高频率，需要继续定位。后续需要调整锁频时间。
         * 以及确认是否每次需要重新req。 */
        core_ctl_set_boost(20 * WLAN_FREQ_TIMER_PERIOD * WLAN_THROUGHPUT_STA_PERIOD); // 20是锁频时间为单位时间的20倍
        hmac_lock_max_cpu_freq();
    }
}
#endif
#endif

/*
 * 函 数 名  : hmac_set_cpu_freq
 * 功能描述  : 绑定主核的PCIE中断/WiFi收发线程/主核最高频率
 * 1.日    期  : 2019年11月24日
 *   修改内容  : 新生成函数
 */
void hmac_set_cpu_freq(uint8_t uc_req_freq_state)
{
#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
    if (uc_req_freq_state == g_freq_lock_control.uc_cur_cpu_freq_state) {
        return;
    }

    g_freq_lock_control.uc_cur_cpu_freq_state = uc_req_freq_state;
#else
    if (uc_req_freq_state == g_freq_lock_control.uc_cur_cpu_freq_state) {
        if (uc_req_freq_state == WLAN_CPU_FREQ_SUPER) {
#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
            /* sdio锁频后频率会掉下来，需要每次都执行锁频 */
            hmac_lock_cpu_freq_high_throughput_proc();
#endif
        }
        return;
    }

    g_freq_lock_control.uc_cur_cpu_freq_state = uc_req_freq_state;
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_set_cpu_freq: cur state:%d! 1: super 0: idle}", uc_req_freq_state);

#ifdef _PRE_FEATURE_PLAT_LOCK_CPUFREQ
    if (g_freq_lock_control.uc_lock_max_cpu_freq == OAL_TRUE) {
        if (uc_req_freq_state == WLAN_CPU_FREQ_SUPER) {
            /* 所有核都锁定最高频率 */
            hmac_lock_max_cpu_freq();
        } else {
            hmac_unlock_max_cpu_freq();
        }
    }
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_freq_lock_control.uc_lock_dma_latency == OAL_TRUE) {
        if (uc_req_freq_state == WLAN_CPU_FREQ_SUPER) {
            /* 修改DMA latency,避免cpu进入过深的idle state */
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_set_cpu_freq: ddr_freq_max:%u!}",
                g_freq_lock_control.dma_latency_value);
            oal_pm_qos_update_request(&g_st_pmqos_requset, g_freq_lock_control.dma_latency_value);
        } else {
            oam_warning_log1(0, OAM_SF_ANY, "{hmac_set_cpu_freq: ddr_freq_init:%u!}", DDR_FREQ_INIT_VALUE);
            oal_pm_qos_update_request(&g_st_pmqos_requset, DDR_FREQ_INIT_VALUE);
        }
    }
#endif
#endif
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ)
static void hmac_hisi_cpumaxfreq_init(void)
{
    uint8_t cpuid;

    memset_s(g_aul_cpumaxfreq, OAL_BUS_MAXCPU_NUM * sizeof(hisi_max_cpu_freq), 0,
        OAL_BUS_MAXCPU_NUM * sizeof(hisi_max_cpu_freq));
    memset_s(g_ast_cpufreq, OAL_BUS_MAXCPU_NUM * sizeof(struct cpufreq_req), 0,
        OAL_BUS_MAXCPU_NUM * sizeof(struct cpufreq_req));

    for (cpuid = 0; cpuid < OAL_BUS_MAXCPU_NUM; cpuid++) {
        /* 有效的CPU id则返回1,否则返回0 */
        if (cpu_possible(cpuid) == 0) {
            continue;
        }

        if (external_cpufreq_init_req(&g_ast_cpufreq[cpuid], cpuid) < 0) {
            g_aul_cpumaxfreq[cpuid].valid = OAL_FALSE;
            oam_error_log1(0, OAM_SF_ANY, "{cpufreq::cpufreq_init_req fail, cpu_id[%d]}", cpuid);
            continue;
        }
        g_aul_cpumaxfreq[cpuid].max_cpu_freq = hisi_cpufreq_get_maxfreq(cpuid);
        g_aul_cpumaxfreq[cpuid].valid = OAL_TRUE;
        oam_warning_log2(0, OAM_SF_ANY, "{cpufreq::cpu_id[%d]max_freq%d}", cpuid, g_aul_cpumaxfreq[cpuid].max_cpu_freq);
    }
}
#endif
#endif

#define MAX_CPU_FREQ_LIMIT 2900000
/*
 * 函 数 名  : hisi_cpufreq_get_maxfreq
 * 功能描述  : 获取CPU的最大频率
 * 1.日    期  : 2019年1月7日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hisi_cpufreq_get_maxfreq(unsigned int cpu)
{
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ)
    struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);
    uint32_t ret_freq;
    int32_t idx;

    if (!policy) {
        oam_error_log0(0, OAM_SF_ANY, "hisi_cpufreq_get_maxfreq: get cpufreq policy fail!");
        return 0;
    }

    ret_freq = policy->cpuinfo.max_freq;
    if (ret_freq > MAX_CPU_FREQ_LIMIT && policy->freq_table != NULL) {
        idx = cpufreq_frequency_table_target(policy, MAX_CPU_FREQ_LIMIT, CPUFREQ_RELATION_H);
        if (idx >= 0) {
            ret_freq = (policy->freq_table + idx)->frequency;
            oam_warning_log1(0, OAM_SF_ANY, "{hisi_cpufreq_get_maxfreq::limit freq:%u}", ret_freq);
        }
    }
    cpufreq_cpu_put(policy);
    return ret_freq;
#else
    return 0;
#endif
}

uint32_t hmac_hisi_cpufreq_init(void)
{
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    external_get_fast_cpus(&g_st_fastcpus);
#endif
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ)
    if (g_freq_lock_control.uc_lock_max_cpu_freq == OAL_FALSE) {
        return OAL_SUCC;
    }
    g_aul_cpumaxfreq = oal_memalloc(OAL_BUS_MAXCPU_NUM * sizeof(hisi_max_cpu_freq));
    g_ast_cpufreq = oal_memalloc(OAL_BUS_MAXCPU_NUM * sizeof(struct cpufreq_req));
    if (oal_any_null_ptr2(g_aul_cpumaxfreq, g_ast_cpufreq)) {
        oam_error_log0(0, OAM_SF_ANY, "{cpufreq_init:fail to alloc}");
        hmac_hisi_free_cpufreq_res();
        return OAL_FAIL;
    }

    hmac_hisi_cpumaxfreq_init();
#ifdef CONFIG_HI110X_SOFT_AP_LIMIT_CPU_FREQ
    g_freq_lock_control.limit_cpu_freq = OAL_FALSE;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#if defined(CONFIG_PLATFORM_QOS) && defined(CONFIG_DEVFREQ_GOV_PLATFORM_QOS)
    oam_warning_log1(0, OAM_SF_ANY, "{hmac_hisi_cpufreq_init::ddr freq:%u}", DDR_FREQ_INIT_VALUE);
    oal_pm_qos_add_request(&g_st_pmqos_requset, PLATFORM_QOS_MEMORY_LATENCY, DDR_FREQ_INIT_VALUE);
#else
    oal_pm_qos_add_request(&g_st_pmqos_requset, 0, DDR_FREQ_INIT_VALUE);
#endif
#else
    oal_pm_qos_add_request(&g_st_pmqos_requset, PM_QOS_CPU_DMA_LATENCY, DDR_FREQ_INIT_VALUE);
#endif

#endif
#endif
    return OAL_SUCC;
}
uint32_t hmac_hisi_cpufreq_exit(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ)
    uint8_t uc_cpuid;

    if (g_freq_lock_control.uc_lock_max_cpu_freq == OAL_FALSE) {
        return OAL_SUCC;
    }

    if (oal_any_null_ptr2(g_aul_cpumaxfreq, g_ast_cpufreq)) {
        hmac_hisi_free_cpufreq_res();
        return OAL_FAIL;
    }

    for (uc_cpuid = 0; uc_cpuid < OAL_BUS_MAXCPU_NUM; uc_cpuid++) {
        /* 未获取到正确的cpu频率则不设置 */
        if (g_aul_cpumaxfreq[uc_cpuid].valid != OAL_TRUE) {
            continue;
        }

        external_cpufreq_exit_req(&g_ast_cpufreq[uc_cpuid]);
        g_aul_cpumaxfreq[uc_cpuid].valid = OAL_FALSE;
    }
    hmac_hisi_free_cpufreq_res();
#endif
    oal_pm_qos_remove_request(&g_st_pmqos_requset);
#endif
    return OAL_SUCC;
}
