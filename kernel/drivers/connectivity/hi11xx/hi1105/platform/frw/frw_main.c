/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Implementation of FRW Module Initialization and Unloading
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "frw_main.h"
#include "frw_task.h"
#include "frw_event_main.h"
#include "frw_event_deploy.h"
#include "frw_ext_if.h"
#include "frw_timer.h"

#include "oal_hcc_host_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_MAIN_C

/* 事件队列配置信息全局变量 */
frw_event_cfg_stru g_event_queue_cfg_table_gt[] = WLAN_FRW_EVENT_CFG_TABLE;;
frw_event_cfg_stru g_event_queue_cfg_table_wifi[] = WLAN_FRW_EVENT_CFG_TABLE;;

/* 事件管理实体 */
frw_event_mgmt_stru g_event_manager_gt[WLAN_FRW_MAX_NUM_CORES];
frw_event_mgmt_stru g_event_manager_wifi[WLAN_FRW_MAX_NUM_CORES];

/* 事件处理全局变量 */
frw_task_stru g_st_event_task_gt[WLAN_FRW_MAX_NUM_CORES];
frw_task_stru g_st_event_task_wifi[WLAN_FRW_MAX_NUM_CORES];

/* 定时器暂停, 全局变量 */
uint8_t g_st_timer_pause_gt = OAL_FALSE;
uint8_t g_st_timer_pause_wifi = OAL_FALSE;

/* 定时器重启, 全局变量 */
uint32_t g_need_restart_gt = OAL_FALSE;
uint32_t g_need_restart_wifi = OAL_FALSE;
#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
/* smp os use the task lock to protect the event process */
oal_task_lock_stru g_event_task_lock_gt;
oal_task_lock_stru g_event_task_lock_wifi;
#endif

/* context, 用于区分GT/WIFI子系统, 两个不同实例 */
frw_subsys_context g_frw_context_gt;
frw_subsys_context g_frw_context_wifi;

/*
  按照不同的上下文, 初始化结构体
 */
void frw_ctx_init(frw_subsys_context *ctx)
{
    (void)memset_s(ctx, sizeof(frw_subsys_context), 0, sizeof(frw_subsys_context));
    if (frw_context_is_gt(ctx)) {
        ctx->event_queue_cfg_table_size = FRW_EVENT_MAX_NUM_QUEUES_GT;
        ctx->event_queue_cfg_table = g_event_queue_cfg_table_gt;
        ctx->event_manager = g_event_manager_gt;
#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
        ctx->event_task_lock = &g_event_task_lock_gt;
#endif
        (void)memset_s(g_st_event_task_gt, sizeof(g_st_event_task_gt), 0, sizeof(g_st_event_task_gt));
        ctx->event_task = g_st_event_task_gt;
        ctx->timer_pause = &g_st_timer_pause_gt;
        ctx->need_restart = &g_need_restart_gt;
    }

    if (frw_context_is_wifi(ctx)) {
        ctx->event_queue_cfg_table_size = FRW_EVENT_MAX_NUM_QUEUES_WIFI;
        ctx->event_queue_cfg_table = g_event_queue_cfg_table_wifi;
        ctx->event_manager = g_event_manager_wifi;
#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
        ctx->event_task_lock = &g_event_task_lock_wifi;
#endif
        (void)memset_s(g_st_event_task_wifi, sizeof(g_st_event_task_wifi), 0, sizeof(g_st_event_task_wifi));
        ctx->event_task = g_st_event_task_wifi;
        ctx->timer_pause = &g_st_timer_pause_wifi;
        ctx->need_restart = &g_need_restart_wifi;
    }
    ctx->frw_init_state = FRW_INIT_STATE_BUTT;
}

/*
  3 函数实现
 */
void frw_event_fsm_register_ctx(frw_subsys_context *ctx)
{
    if (frw_context_is_gt(ctx)) {
        ctx->timeout_event_sub_table[FRW_TIMEOUT_TIMER_EVENT].p_func = frw_timer_timeout_proc_gt;
    } else {
        ctx->timeout_event_sub_table[FRW_TIMEOUT_TIMER_EVENT].p_func = frw_timer_timeout_proc_wifi;
    }
    frw_event_table_register_ctx(FRW_EVENT_TYPE_TIMEOUT, FRW_EVENT_PIPELINE_STAGE_0, ctx->timeout_event_sub_table, ctx);
}

int32_t frw_main_init_ctx(frw_subsys_context *ctx)
{
    uint32_t ul_ret;

    frw_set_init_state_ctx(FRW_INIT_STATE_START, ctx);

    /* 事件管理模块初始化 */
    ul_ret = frw_event_init_ctx(ctx);
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_main_init:: frw_event_init return err code: %d}", ul_ret);
        return -OAL_EFAIL;
    }

    frw_task_init_ctx(ctx);

#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    /* 事件核间部署模块初始化 */
    ul_ret = frw_event_deploy_init();
    if (oal_unlikely(ul_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_main_init:: frw_event_deploy_init return err code: %d}", ul_ret);
        return -OAL_EFAIL;
    }
#endif

    if (frw_context_is_gt(ctx)) {
        frw_timer_init_ctx(FRW_TIMER_DEFAULT_TIME, frw_timer_timeout_proc_event_gt, 0, ctx);
    } else {
        frw_timer_init_ctx(FRW_TIMER_DEFAULT_TIME, frw_timer_timeout_proc_event_wifi, 0, ctx);
    }

    frw_event_fsm_register_ctx(ctx);

    /* 启动成功后，输出打印 设置状态始终放最后 */
    frw_set_init_state_ctx(FRW_INIT_STATE_FRW_SUCC, ctx);

    return OAL_SUCC;
}

int32_t frw_main_init(void)
{
    frw_ctx_init(GT_FRW_CONTEXT);
    if (is_gt_support() && frw_main_init_ctx(GT_FRW_CONTEXT) != OAL_SUCC) {
        return OAL_FALSE;
    }
    frw_ctx_init(WIFI_FRW_CONTEXT);
    if (is_wifi_support() && frw_main_init_ctx(WIFI_FRW_CONTEXT) != OAL_SUCC) {
        return OAL_FALSE;
    }
    return OAL_SUCC;
}

void frw_main_exit_ctx(frw_subsys_context *ctx)
{
    /* 卸载定时器 */
    frw_timer_exit_ctx(ctx);

    frw_task_exit_ctx(ctx);

    /* 卸载事件管理模块 */
    frw_event_exit_ctx(ctx);

#if (_PRE_MULTI_CORE_MODE_PIPELINE_AMP == _PRE_MULTI_CORE_MODE)
    /* 事件核间部署模块卸载 */
    frw_event_deploy_exit();
#endif

    /* 卸载成功后在置状态位 */
    frw_set_init_state_ctx(FRW_INIT_STATE_START, ctx);

    /* 卸载成功后，输出打印 */
    return;
}

void frw_main_exit(void)
{
    frw_main_exit_ctx(GT_FRW_CONTEXT);
    frw_main_exit_ctx(WIFI_FRW_CONTEXT);
}

/*
 * 函 数 名  : frw_set_init_state
 * 功能描述  : 设置初始化状态
 * 输入参数  : 初始化状态
 */
void frw_set_init_state_ctx(uint16_t en_init_state, frw_subsys_context *ctx)
{
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_set_init_state_ctx:: invalid point}");
        return;
    }

    if (en_init_state >= FRW_INIT_STATE_BUTT) {
        oam_warning_log1(0, OAM_SF_FRW, "{frw_set_init_state_ctx:: en_init_state >= FRW_INIT_STATE_BUTT: %d}",
                         en_init_state);
        return;
    }

    ctx->frw_init_state = en_init_state;

    return;
}

/*
 * 函 数 名  : frw_get_init_state
 * 功能描述  : 获取初始化状态
 */
uint16_t frw_get_init_state_ctx(frw_subsys_context *ctx)
{
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_get_init_state_ctx:: invalid point}");
        return FRW_INIT_STATE_BUTT;
    }

    return ctx->frw_init_state;
}

oal_module_symbol(frw_main_init);
oal_module_symbol(frw_main_exit);

oal_module_symbol(frw_set_init_state_ctx);
oal_module_symbol(frw_get_init_state_ctx);

oal_module_symbol(g_st_event_task_gt);
oal_module_symbol(g_st_event_task_wifi);
oal_module_symbol(g_st_timer_pause_gt);
oal_module_symbol(g_st_timer_pause_wifi);
oal_module_symbol(g_need_restart_gt);
oal_module_symbol(g_need_restart_wifi);
#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
oal_module_symbol(g_event_task_lock_gt);
oal_module_symbol(g_event_task_lock_wifi);
#endif

oal_module_symbol(g_frw_context_gt);
oal_module_symbol(g_frw_context_wifi);
