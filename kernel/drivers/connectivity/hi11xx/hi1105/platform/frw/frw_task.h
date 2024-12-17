/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: frw_task.c header file
 * Author: @CompanyNameTag
 */

#ifndef FRW_TASK_H
#define FRW_TASK_H

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_subsys_context.h"
#include "external/lpcpu_feature.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TASK_H

/* 线程的运行状态 */
typedef enum {
    FRW_TASK_STATE_IRQ_UNBIND = 0, /* 线程处于暂停状态 */
    FRW_TASK_STATE_IRQ_BIND,       /* 线程处于运行状态 */

    FRW_TASK_BUTT
} frw_task_state_enum;

/* STRUCT 定义 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
struct _frw_task_stru_ {
    oal_task_stru *pst_event_kthread; /*  task变量 */
    oal_wait_queue_head_stru frw_wq;  /* waitqueue */
    uint8_t uc_task_state;          /* 线程状态，0暂停，1运行 */
    uint8_t auc_resv[3]; /* 3 */
    uint32_t ul_total_loop_cnt;
    uint32_t ul_total_event_cnt; /* 统计线程空转次数 */
    uint32_t ul_max_empty_count;
    void (*p_event_handler_func)(void *_pst_bind_cpu); /* kthread主程序 */
};

#elif (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_TASKLET)
struct _frw_task_stru_ {
    oal_tasklet_stru st_ipi_tasklet;
    oal_tasklet_stru st_event_tasklet;               /* tasklet变量 */
    void (*p_event_handler_func)(oal_uint); /* tasklet处理程序 */
};
#endif

/* 全局变量声明 */
extern frw_task_stru g_st_event_task[WLAN_FRW_MAX_NUM_CORES];
OAL_STATIC OAL_INLINE oal_task_stru *get_frw_task(void)
{
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
    uint32_t core_id = oal_get_core_id();
    return g_st_event_task[core_id].pst_event_kthread;
#else
    return NULL;
#endif
}
static inline void hisi_thread_set_affinity(struct task_struct *pst_thread)
{
#if !defined(_PRE_PRODUCT_HI1620S_KUNPENG) && !defined(_PRE_WINDOWS_SUPPORT)
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
    struct cpumask cpu_mask;
    if (pst_thread == NULL) {
        return;
    }
    /* bind to CPU 1~7,let kernel to shedule */
    cpumask_setall(&cpu_mask);
    oal_cpumask_clear_cpu(0, &cpu_mask);
    set_cpus_allowed_ptr(pst_thread, &cpu_mask);
#endif
#endif
}
/* 函数声明 */
uint32_t frw_task_init_ctx(frw_subsys_context *ctx);
void frw_task_exit_ctx(frw_subsys_context *ctx);
void frw_task_event_handler_register_ctx(void (*p_func)(oal_uint), frw_subsys_context *ctx);
void frw_task_set_state_ctx(uint32_t ul_core_id, uint8_t uc_task_state, frw_subsys_context *ctx);
uint8_t frw_task_get_state_ctx(uint32_t ul_core_id, frw_subsys_context *ctx);
void frw_task_sched_ctx(uint32_t ul_core_id, frw_subsys_context *ctx);
#endif /* end of frw_task.h */
