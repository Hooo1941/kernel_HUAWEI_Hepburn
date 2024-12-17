/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:oal_pm_qos_interface.h header file
 * Author: @CompanyNameTag
 */

#ifndef OAL_PM_QOS_H
#define OAL_PM_QOS_H

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_mm.h"
#include "arch/oal_util.h"
#include "securec.h"
#include "platform_oneimage_define.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)) && defined(CONFIG_PLATFORM_QOS)
#include <linux/platform_drivers/platform_qos.h>
#endif
/* 宏定义 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
typedef enum {
    PM_QOS_RESERVED = 0,
    PM_QOS_CPU_DMA_LATENCY,
    PM_QOS_NETWORK_LATENCY,
    PM_QOS_NETWORK_THROUGHPUT,
    PM_QOS_MEMORY_BANDWIDTH,
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
    PM_QOS_MEMORY_LATENCY,
    PM_QOS_MEMORY_THROUGHPUT,
    PM_QOS_MEMORY_THROUGHPUT_UP_THRESHOLD,
#ifdef CONFIG_DEVFREQ_L1BUS_LATENCY
    PM_QOS_L1BUS_LATENCY,
#endif
#endif

#ifdef CONFIG_NPUFREQ_PM_QOS
    PM_QOS_NPU_FREQ_DNLIMIT,
    PM_QOS_NPU_FREQ_UPLIMIT,
#endif
#ifdef CONFIG_GPUTOP_FREQ
    PM_QOS_GPUTOP_FREQ_DNLIMIT,
    PM_QOS_GPUTOP_FREQ_UPLIMIT,
#endif
    /* insert new class ID */
    PM_QOS_NUM_CLASSES,
} oal_qos_type;
/* 调用HISI接口, VALUE入参直接为期望的DDR频率, 入参为0则自动调频 */
#define DDR_FREQ_INIT_VALUE  0x0
#ifdef CONFIG_PLATFORM_QOS
#define oal_pm_qos_add_request(req, type, val) platform_qos_add_request(req, type, val)
#define oal_pm_qos_remove_request(req) platform_qos_remove_request(req)
#define oal_pm_qos_update_request(req, val) platform_qos_update_request(req, val)
#else
#define oal_pm_qos_add_request(req, type, val) cpu_latency_qos_add_request(req, val)
#define oal_pm_qos_remove_request(req) cpu_latency_qos_remove_request(req)
#define oal_pm_qos_update_request(req, val) cpu_latency_qos_update_request(req, val)
#endif
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define oal_pm_qos_add_request(req, type, val)
#define oal_pm_qos_remove_request(req)
#define oal_pm_qos_update_request(req, val)
#define DDR_FREQ_INIT_VALUE PM_QOS_DEFAULT_VALUE
#else
#define oal_pm_qos_add_request(req, type, val) pm_qos_add_request(req, type, val)
#define oal_pm_qos_remove_request(req) pm_qos_remove_request(req)
#define oal_pm_qos_update_request(req, val) pm_qos_update_request(req, val)
#define DDR_FREQ_INIT_VALUE PM_QOS_DEFAULT_VALUE
#endif
#endif
#endif /* end of oal_pm_qos.h */
