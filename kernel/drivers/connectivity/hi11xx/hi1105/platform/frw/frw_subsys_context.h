/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Coexistence of GT and WIFI platforms
 * Author: @CompanyNameTag
 */

#ifndef FRW_SUBSYS_CONTEXT_H
#define FRW_SUBSYS_CONTEXT_H

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"

/*
 * 事件表的最大个数
 * 一种类型的事件对应两个表项，所以事件表的大小为事件类型的2倍
 */
#define FRW_EVENT_TABLE_MAX_ITEMS (FRW_EVENT_TYPE_BUTT * 2)

#define frw_event_alloc_append_line(_us_len, ctx) \
    frw_event_alloc_ctx(THIS_FILE_ID, __LINE__, _us_len, ctx)

#define frw_event_free_append_line(_pst_event_mem, ctx) \
    frw_event_free_ctx(THIS_FILE_ID, __LINE__, _pst_event_mem, ctx)

/* 枚举定义 */
typedef enum {
    FRW_TIMEOUT_TIMER_EVENT, /* 以FRW_TIMER_DEFAULT_TIME为周期的定时器事件子类型 */

    FRW_TIMEOUT_SUB_TYPE_BUTT
} hmac_timeout_event_sub_type_enum;

/*
 * 结构名  : frw_event_cfg_stru
 * 结构说明: 事件队列配置信息结构体
 */
typedef struct _frw_event_cfg_stru_ {
    uint8_t uc_weight;                   /* 队列权重 */
    uint8_t uc_max_events;               /* 队列所能容纳的最大事件个数 */
    uint8_t en_policy; /* 队列所属调度策略(高优先级、普通优先级) */
    uint8_t auc_resv;
} frw_event_cfg_stru;

/* 结构体前置声明 */
typedef struct _frw_event_mgmt_stru_ frw_event_mgmt_stru;
typedef struct _frw_event_ipc_register_stru_ frw_event_ipc_register_stru;
typedef struct _frw_task_stru_ frw_task_stru;

/* 用于区分不同子系统的上下文结构体 */
struct _frw_subsys_context_ {
    uint32_t event_queue_cfg_table_size;                            /* 事件配置数组, 配置数组的长度, 与具体业务相关 */
    frw_event_cfg_stru *event_queue_cfg_table;                      /* 初始化时需要设置 */
    oal_task_lock_stru *event_task_lock;                            /* 对外导出变量的指针, 初始化时需要设置 */

    /* frw事件队列, 调度队列管理相关 */
    frw_event_mgmt_stru *event_manager;     /* 指针对应的数组长度固定: WLAN_FRW_MAX_NUM_CORES, 初始化时需要设置 */

    /* oal event回调函数相关 */
    frw_event_table_item_stru event_table[FRW_EVENT_TABLE_MAX_ITEMS];
    /* oal 线程相关 */
    frw_task_stru *event_task;      /* 指针对应的数组长度固定: WLAN_FRW_MAX_NUM_CORES, 初始化时需要设置 */

    /* oal 定时器相关 */
    uint8_t *timer_pause;                                               /* 对外导出变量的指针, 初始化时需要设置 */
    oal_dlist_head_stru timer_list[WLAN_FRW_MAX_NUM_CORES];             /* 定时器列表 */
    oal_spin_lock_stru timer_list_spinlock[WLAN_FRW_MAX_NUM_CORES];     /* 定时器操作的自旋锁 */
    oal_timer_list_stru timer[WLAN_FRW_MAX_NUM_CORES];                  /* 依赖的定时器 */
    uint32_t frw_timer_start_stamp[WLAN_FRW_MAX_NUM_CORES];             /* 记录: 定时器即将触发的时间点 */
    uint32_t stop_timestamp;                                            /* 记录: oal定时器功能, 停止的时间点 */
    uint32_t *need_restart;                                             /* 标记: oal定时器功能, 是否需要重新启动 */
#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
    uint32_t frw_timer_cpu_count[CONFIG_NR_CPUS];                       /* 定时器维测信息 */
#endif
    frw_event_sub_table_item_stru timeout_event_sub_table[FRW_TIMEOUT_SUB_TYPE_BUTT]; /* 定时器回调注册函数 */
    /* frw 状态 */
    uint16_t frw_init_state;
    uint16_t max_vap_cnt;
};

extern frw_subsys_context g_frw_context_gt;
extern frw_subsys_context g_frw_context_wifi;

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 frw_context_is_gt(frw_subsys_context *ctx)
{
    if (ctx == GT_FRW_CONTEXT) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 frw_context_is_wifi(frw_subsys_context *ctx)
{
    if (ctx == WIFI_FRW_CONTEXT) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE oal_mem_pool_context_stru *frw_context_get_mem_ctx(frw_subsys_context *ctx)
{
    oal_mem_pool_context_stru *mem_ctx = NULL;
    if (frw_context_is_gt(ctx)) {
        mem_ctx = GT_OAL_MEM_CONTEXT;
    } else if (frw_context_is_wifi(ctx)) {
        mem_ctx = WIFI_OAL_MEM_CONTEXT;
    }
    return mem_ctx;
}

#endif /* end of frw_subsys_context.h */
