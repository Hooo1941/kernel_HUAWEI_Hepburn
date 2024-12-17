/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Encapsulation timer processing
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "frw_timer.h"
#include "frw_main.h"
#include "frw_task.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TIMER_C

OAL_STATIC OAL_INLINE void frw_timer_immediate_destroy_timer_ctx_nolock(uint32_t file_id,
                                                                        uint32_t line_num,
                                                                        frw_timeout_stru *pst_timeout,
                                                                        frw_subsys_context *ctx);

void frw_timer_init_ctx(uint32_t delay, oal_timer_func p_func, uintptr_t arg, frw_subsys_context *ctx)
{
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_init_ctx:: invalid point}");
        return;
    }

    if (oal_unlikely(p_func == NULL)) {
        oal_warn_on(1);
        return;
    }

    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_dlist_init_head(&ctx->timer_list[core_id]);
        oal_spin_lock_init(&ctx->timer_list_spinlock[core_id]);

        oal_timer_init(&ctx->timer[core_id], delay, p_func, arg);
        ctx->frw_timer_start_stamp[core_id] = 0;
    }
}

void frw_timer_exit_ctx(frw_subsys_context *ctx)
{
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_exit_ctx:: invalid point}");
        return;
    }
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_timer_delete_sync(&ctx->timer[core_id]);
        ctx->frw_timer_start_stamp[core_id] = 0;
    }
}

void frw_timer_restart_ctx(frw_subsys_context *ctx)
{
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_restart_ctx:: invalid point}");
        return;
    }
    /* 重启定时器 */
    if (*(ctx->need_restart) == OAL_FALSE) {
        return;
    }

    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_timer_start(&ctx->timer[core_id], FRW_TIMER_DEFAULT_TIME);
    }
    *(ctx->need_restart) = OAL_FALSE;
}

void frw_timer_stop_ctx(frw_subsys_context *ctx)
{
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_stop_ctx:: invalid point}");
        return;
    }
    /* stop frw sys timer,record the stop time for restart timer to recalculate timestamp */
    ctx->stop_timestamp = (uint32_t)oal_time_get_stamp_ms();

    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_timer_delete(&ctx->timer[core_id]);
        ctx->frw_timer_start_stamp[core_id] = 0;
    }

    *(ctx->need_restart) = OAL_TRUE;
}

OAL_STATIC void frw_timer_dump_ctx(uint32_t core_id, frw_subsys_context *ctx)
{
    const uint32_t dump_mem_size = 64;
    oal_dlist_head_stru *pst_timeout_entry = NULL;
    frw_timeout_stru *pst_timeout_element = NULL;

    pst_timeout_entry = ctx->timer_list[core_id].pst_next;
    while (pst_timeout_entry != &ctx->timer_list[core_id]) {
        if (pst_timeout_entry == NULL) {
            oam_error_log0(0, OAM_SF_FRW, "{frw_timer_dump:: time broken break}");
            break;
        }

        if (pst_timeout_entry->pst_next == NULL) {
            /* If next is null,
             the pst_timeout_entry stru maybe released or memset */
            oam_error_log0(0, OAM_SF_FRW, "{frw_timer_dump:: pst_next is null,dump mem}");
            /* 2倍的dump_mem_size */
            oal_print_hex_dump(((uint8_t *)pst_timeout_entry) - dump_mem_size, dump_mem_size * 2,
                               HEX_DUMP_GROUP_SIZE, "timer broken: "); /* dump前后64字节长度内存 */
        }

        pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);
        pst_timeout_entry = pst_timeout_entry->pst_next;
        oam_error_log3(0, OAM_SF_FRW, "{frw_timer_dump:: time_stamp[0x%x] timeout[%d]  enabled[%d]}",
                       pst_timeout_element->time_stamp,
                       pst_timeout_element->timeout,
                       pst_timeout_element->en_is_enabled);
        oam_error_log3(0, OAM_SF_FRW, "{frw_timer_dump:: module_id[%d] file_id[%d] line_num[%d]}",
                       pst_timeout_element->en_module_id,
                       pst_timeout_element->file_id,
                       pst_timeout_element->line_num);
#ifdef CONFIG_PRINTK
        if (pst_timeout_element->p_func) {
            printk(KERN_ERR "frw_timer_dump func : %pF\n", pst_timeout_element->p_func);
        }
#endif
    }
}

/*
 * 函 数 名  : frw_timer_timeout_delete_ctx
 * 功能描述  : 一个定时器超时处理函数中创建新的定时器，如果定时器超时，则将相应的定时器进行删除，取消en_is_deleting标记
 */
void frw_timer_timeout_delete_ctx(frw_timeout_stru *pst_timeout_element, uint32_t present_time, uint32_t core_id,
    frw_subsys_context *ctx)
{
    /* 删除超时定时器，如果是周期定时器，则将其再添加进去:delete first,then add periodic_timer */
    uint32_t runtime_func_start = 0;
    uint32_t runtime_func_end = 0;
    uint32_t endtime_func = 0;

    pst_timeout_element->en_is_registerd = OAL_FALSE;
    oal_dlist_delete_entry(&pst_timeout_element->st_entry);

    if ((pst_timeout_element->en_is_periodic == OAL_TRUE) ||
        (pst_timeout_element->en_is_enabled == OAL_FALSE)) {
        pst_timeout_element->time_stamp = present_time + pst_timeout_element->timeout;
        pst_timeout_element->en_is_registerd = OAL_TRUE;
        frw_timer_add_timer_ctx(pst_timeout_element, ctx);
    }
    runtime_func_start = (uint32_t)oal_time_get_stamp_ms();
    if (pst_timeout_element->en_is_enabled) {
        oal_spin_unlock_bh(&ctx->timer_list_spinlock[core_id]);
        pst_timeout_element->p_func(pst_timeout_element->p_timeout_arg);
        oal_spin_lock_bh(&ctx->timer_list_spinlock[core_id]);
    }

    endtime_func = (uint32_t)oal_time_get_stamp_ms();
    runtime_func_end = (uint32_t)oal_time_get_runtime(runtime_func_start, endtime_func);
    if (runtime_func_end >= (uint32_t)oal_jiffies_to_msecs(2)) { /* 2 */
        oam_warning_log4(0, OAM_SF_FRW, "{frw_timer_timeout_proc:: fileid=%u, linenum=%u, moduleid=%u, runtime=%u}",
            pst_timeout_element->file_id, pst_timeout_element->line_num,
            pst_timeout_element->en_module_id, runtime_func_end);
    }
}

/*
 * 函 数 名  : frw_timer_timeout_proc_ctx
 * 功能描述  : 遍历timer链表执行到期超时函数
 */
uint32_t frw_timer_timeout_proc_ctx(frw_event_mem_stru *pst_timeout_event, frw_subsys_context *ctx)
{
    frw_timeout_stru *pst_timeout_element = NULL;
    oal_dlist_head_stru *pst_timeout_entry = NULL;
    uint32_t present_time, end_time, runtime, core_id, frw_timer_start;

    if (oal_unlikely(pst_timeout_event == NULL)) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_timeout_proc:: pst_timeout_event is null ptr!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    present_time = (uint32_t)oal_time_get_stamp_ms();
    core_id = oal_get_core_id();

    /* 执行超时定时器 */
    oal_spin_lock_bh(&ctx->timer_list_spinlock[core_id]);
    pst_timeout_entry = ctx->timer_list[core_id].pst_next;

    while (pst_timeout_entry != &ctx->timer_list[core_id]) {
        if (pst_timeout_entry == NULL) {
            oam_error_log0(0, OAM_SF_FRW, "{frw_timer_timeout_proc:: the timer list is broken! }");
            frw_timer_dump_ctx(core_id, ctx);
            break;
        }

        pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);
        pst_timeout_element->curr_time_stamp = present_time;

        if (frw_time_after(present_time, pst_timeout_element->time_stamp)) {
            frw_timer_timeout_delete_ctx(pst_timeout_element, present_time, core_id, ctx);
        } else {
            break;
        }
        pst_timeout_entry = ctx->timer_list[core_id].pst_next;
    }

    /* 获得链表的最小超时时间，重启定时器 */
    if (oal_dlist_is_empty(&ctx->timer_list[core_id]) == OAL_FALSE) {
        pst_timeout_entry = ctx->timer_list[core_id].pst_next;
        pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);
        present_time = (uint32_t)oal_time_get_stamp_ms();
        if (frw_time_after(pst_timeout_element->time_stamp, present_time) == OAL_TRUE) {
            frw_timer_start = (uint32_t)oal_time_get_runtime(present_time, pst_timeout_element->time_stamp);

            ctx->frw_timer_start_stamp[core_id] = pst_timeout_element->time_stamp;
        } else {
            frw_timer_start = FRW_TIMER_DEFAULT_TIME;

            ctx->frw_timer_start_stamp[core_id] = (present_time + FRW_TIMER_DEFAULT_TIME);
        }

        oal_timer_start(&ctx->timer[core_id], frw_timer_start);
    } else {
        ctx->frw_timer_start_stamp[core_id] = 0;
    }

    oal_spin_unlock_bh(&ctx->timer_list_spinlock[core_id]);

    end_time = (uint32_t)oal_time_get_stamp_ms();
    runtime = (uint32_t)oal_time_get_runtime(present_time, end_time);
    /* 同device侧检测日志时限一致 */
    if (runtime > (uint32_t)oal_jiffies_to_msecs(2)) { /* 2 */
        oam_warning_log1(0, OAM_SF_FRW, "{timeout process exucte time too long time[%d]}", runtime);
    }

    return OAL_SUCC;
}

uint32_t frw_timer_timeout_proc_gt(frw_event_mem_stru *pst_timeout_event)
{
    return frw_timer_timeout_proc_ctx(pst_timeout_event, GT_FRW_CONTEXT);
}

uint32_t frw_timer_timeout_proc_wifi(frw_event_mem_stru *pst_timeout_event)
{
    return frw_timer_timeout_proc_ctx(pst_timeout_event, WIFI_FRW_CONTEXT);
}

/*
 * 函 数 名  : frw_timer_add_in_order
 * 功能描述  : 向链表中按从小到大的顺序插入节点
 * 输入参数  : pst_new: 要插入的新节点
 *             pst_head: 链表头指针
 */
OAL_STATIC void frw_timer_add_in_order_ctx(oal_dlist_head_stru *pst_new, oal_dlist_head_stru *pst_head,
                                           frw_subsys_context *ctx)
{
    oal_dlist_head_stru *pst_timeout_entry = NULL;
    frw_timeout_stru *pst_timeout_element = NULL;
    frw_timeout_stru *pst_timeout_element_new = NULL;
    uint32_t core_id;

    pst_timeout_element_new = oal_dlist_get_entry(pst_new, frw_timeout_stru, st_entry);

    core_id = oal_get_core_id();

    /* 搜索链表，查找第一个比pst_timeout_element_new->time_stamp大的位置 */
    if (pst_head != NULL) {
        pst_timeout_entry = pst_head->pst_next;

        while (pst_timeout_entry != pst_head) {
            if (pst_timeout_entry == NULL) {
                oam_error_log0(0, OAM_SF_FRW, "{Driver frw_timer_add_in_order:: the timer list is broken! }");
                oam_error_log3(0, OAM_SF_FRW,
                               "{new frw_timer_add_in_order:: time_stamp[0x%x] timeout[%d]  enabled[%d]}",
                               pst_timeout_element_new->time_stamp,
                               pst_timeout_element_new->timeout,
                               pst_timeout_element_new->en_is_enabled);
                oam_error_log3(0, OAM_SF_FRW, "{new frw_timer_add_in_order:: module_id[%d] file_id[%d] line_num[%d]}",
                               pst_timeout_element_new->en_module_id,
                               pst_timeout_element_new->file_id,
                               pst_timeout_element_new->line_num);
                frw_timer_dump_ctx(core_id, ctx);
                break;
            }

            pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);
            if (frw_time_after(pst_timeout_element->time_stamp, pst_timeout_element_new->time_stamp)) {
                break;
            }

            pst_timeout_entry = pst_timeout_entry->pst_next;
        }

        if ((pst_timeout_entry != NULL) && (pst_timeout_entry->pst_prev != NULL)) {
            oal_dlist_add(pst_new, pst_timeout_entry->pst_prev, pst_timeout_entry);
        } else {
            oam_error_log0(0, OAM_SF_FRW, "{Driver frw_timer_add_in_order::timer list is broken !}");
#ifdef _PRE_WLAN_REPORT_WIFI_ABNORMAL
            oal_report_wifi_abnormal(OAL_ABNORMAL_FRW_TIMER_BROKEN, OAL_ACTION_REBOOT, 0, 0);
#endif
        }
    }
}

void frw_timer_add_timer_ctx(frw_timeout_stru *pst_timeout, frw_subsys_context *ctx)
{
    int32_t l_val;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_add_timer_ctx:: invalid point}");
        return;
    }
    if (pst_timeout == NULL) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_add_timer:: NULL == pst_timeout}");
        return;
    }

    if (oal_dlist_is_empty(&ctx->timer_list[pst_timeout->core_id]) == OAL_TRUE) {
        ctx->frw_timer_start_stamp[pst_timeout->core_id] = 0;
    }

    /* 将Frw的无序链表改为有序 */
    frw_timer_add_in_order_ctx(&pst_timeout->st_entry, &ctx->timer_list[pst_timeout->core_id], ctx);

    l_val = frw_time_after(ctx->frw_timer_start_stamp[pst_timeout->core_id], pst_timeout->time_stamp);
    if ((ctx->frw_timer_start_stamp[pst_timeout->core_id] == 0) || (l_val > 0)) {
        oal_timer_start(&ctx->timer[pst_timeout->core_id], pst_timeout->timeout);
        ctx->frw_timer_start_stamp[pst_timeout->core_id] = pst_timeout->time_stamp;
    }

    return;
}

/*
 * 函 数 名  : frw_timer_create_timer
 * 功能描述  : 启动定时器
 * 输入参数  : en_is_periodic: 该定时器是否需要周期循环
 *             en_module_id: 模块 ID维测用 取值范围 oam_module_id_enum
 *             p_timeout_arg :定时器超时处理函数需要的入参
 *             us_timeout: 超时时间
 */
void frw_timer_create_timer_ctx(uint32_t file_id,
                                uint32_t line_num,
                                frw_timeout_stru *pst_timeout,
                                frw_timeout_func p_timeout_func,
                                uint32_t timeout,
                                void *p_timeout_arg,
                                oal_bool_enum_uint8 en_is_periodic,
                                uint16_t en_module_id,
                                uint32_t core_id,
                                frw_subsys_context *ctx)
{
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_create_timer_ctx:: invalid point}");
        return;
    }

    if (pst_timeout == NULL) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_create_timer:: NULL == pst_timeout}");
        return;
    }

    if (timeout == 0 && en_is_periodic == OAL_TRUE) {
        oam_error_log4(0, OAM_SF_FRW,
            "{frw_timer_create_timer::timeout value invalid! fileid: %d, line: %d, module: %d, core: %d}",
            file_id, line_num, en_module_id, core_id);
        return;
    }

    pst_timeout->core_id = 0;

    oal_spin_lock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);

    pst_timeout->p_func = p_timeout_func;
    pst_timeout->p_timeout_arg = p_timeout_arg;
    pst_timeout->timeout = timeout;
    pst_timeout->time_stamp = (uint32_t)oal_time_get_stamp_ms() + timeout;
    pst_timeout->en_is_periodic = en_is_periodic;
    pst_timeout->en_module_id = en_module_id;
    pst_timeout->file_id = file_id;
    pst_timeout->line_num = line_num;
    pst_timeout->en_is_enabled = OAL_TRUE; /* 默认使能 */

    if (pst_timeout->en_is_registerd != OAL_TRUE) {
        pst_timeout->en_is_registerd = OAL_TRUE; /* 默认注册 */
        frw_timer_add_timer_ctx(pst_timeout, ctx);
    } else {
        oal_dlist_delete_entry(&pst_timeout->st_entry);
        frw_timer_add_timer_ctx(pst_timeout, ctx);
    }

    oal_spin_unlock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);

    return;
}

/*
 * 函 数 名  : __frw_timer_immediate_destroy_timer
 * 功能描述  : 立即删除定时器，无锁
 */
OAL_STATIC OAL_INLINE void frw_timer_immediate_destroy_timer_ctx_nolock(uint32_t file_id,
                                                                        uint32_t line_num,
                                                                        frw_timeout_stru *pst_timeout,
                                                                        frw_subsys_context *ctx)
{
    if (pst_timeout->st_entry.pst_prev == NULL || pst_timeout->st_entry.pst_next == NULL) {
        return;
    }

    if (pst_timeout->en_is_registerd == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_FRW,
                         "{frw_timer_immediate_destroy_timer::This timer is not enabled it should not be deleted}");

        return;
    }

    pst_timeout->en_is_enabled = OAL_FALSE;
    pst_timeout->en_is_registerd = OAL_FALSE;

    oal_dlist_delete_entry(&pst_timeout->st_entry);

    if (oal_dlist_is_empty(&ctx->timer_list[pst_timeout->core_id]) == OAL_TRUE) {
        ctx->frw_timer_start_stamp[pst_timeout->core_id] = 0;
    }
}

/*
 * 函 数 名  : frw_timer_immediate_destroy_timer
 * 功能描述  : 立即删除定时器,加锁处理
 */
void frw_timer_immediate_destroy_timer_ctx(uint32_t file_id,
                                           uint32_t line_num,
                                           frw_timeout_stru *pst_timeout,
                                           frw_subsys_context *ctx)
{
    oal_spin_lock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);
    frw_timer_immediate_destroy_timer_ctx_nolock(file_id, line_num, pst_timeout, ctx);
    oal_spin_unlock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);
}

void frw_timer_restart_timer_ctx(frw_timeout_stru *pst_timeout, uint32_t timeout,
                                 oal_bool_enum_uint8 en_is_periodic, frw_subsys_context *ctx)
{
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_restart_timer_ctx:: invalid point}");
        return;
    }
    if (pst_timeout == NULL) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_restart_timer:: NULL == pst_timeout}");
        return;
    }
    /* 删除当前定时器 */
    if (pst_timeout->st_entry.pst_prev == NULL || pst_timeout->st_entry.pst_next == NULL) {
        oam_error_log4(0, OAM_SF_FRW,
                       "{frw_timer_restart_timer::This timer has been deleted!file_id=%d,line=%d,core=%d,mod=%d}",
                       pst_timeout->file_id, pst_timeout->line_num,
                       pst_timeout->core_id, pst_timeout->en_module_id);
        return;
    }

    if (pst_timeout->en_is_registerd == OAL_FALSE) {
        oam_error_log4(0, OAM_SF_FRW,
                       "{frw_timer_restart_timer::This timer is not registerd!file_id=%d,line=%d,core=%d,mod=%d}",
                       pst_timeout->file_id, pst_timeout->line_num,
                       pst_timeout->core_id, pst_timeout->en_module_id);
        return;
    }

    oal_spin_lock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);
    oal_dlist_delete_entry(&pst_timeout->st_entry);

    pst_timeout->time_stamp = (uint32_t)oal_time_get_stamp_ms() + timeout;
    pst_timeout->timeout = timeout;
    pst_timeout->en_is_periodic = en_is_periodic;
    pst_timeout->en_is_enabled = OAL_TRUE;

    frw_timer_add_timer_ctx(pst_timeout, ctx);
    oal_spin_unlock_bh(&ctx->timer_list_spinlock[pst_timeout->core_id]);
}

void frw_timer_stop_timer_ctx(frw_timeout_stru *pst_timeout, frw_subsys_context *ctx)
{
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_stop_timer_ctx:: invalid point}");
        return;
    }
    if (pst_timeout == NULL) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_stop_timer:: NULL == pst_timeout}");
        return;
    }

    pst_timeout->en_is_enabled = OAL_FALSE;
}

void frw_timer_timeout_proc_event_ctx(oal_timeout_func_para_t arg, frw_subsys_context *ctx)
{
    frw_event_mem_stru *pst_event_mem = NULL;
    frw_event_stru *pst_event = NULL;
    uint32_t core_id = 0;
    oal_mem_pool_context_stru *mem_ctx = frw_context_get_mem_ctx(ctx);

#if defined(_PRE_FRW_TIMER_BIND_CPU) && defined(CONFIG_NR_CPUS)
    do {
        uint32_t cpu_id = smp_processor_id();
        if (cpu_id < CONFIG_NR_CPUS) {
            ctx->frw_timer_cpu_count[cpu_id]++;
        }
    } while (0);
#endif

    if (*(ctx->timer_pause) == OAL_TRUE) {
        return;
    }

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        if (frw_task_get_state(core_id)) {
#endif
            pst_event_mem = frw_event_alloc_append_line(sizeof(frw_event_stru), mem_ctx);
            /* 返回值检查 */
            if (oal_unlikely(pst_event_mem == NULL)) {
                /* 重启定时器 */
#ifdef _PRE_FRW_TIMER_BIND_CPU
                oal_timer_start_on(&ctx->timer[core_id], FRW_TIMER_DEFAULT_TIME, 0);
#else
                oal_timer_start(&ctx->timer[core_id], FRW_TIMER_DEFAULT_TIME);
#endif
                oam_error_log0(0, OAM_SF_FRW, "{frw_timer_timeout_proc_event:: frw_event_alloc_m failed!}");
                return;
            }

            pst_event = frw_get_event_stru(pst_event_mem);

            /* 填充事件头 */
            frw_field_setup((&pst_event->st_event_hdr), en_type, (FRW_EVENT_TYPE_TIMEOUT));
            frw_field_setup((&pst_event->st_event_hdr), uc_sub_type, (FRW_TIMEOUT_TIMER_EVENT));
            frw_field_setup((&pst_event->st_event_hdr), us_length, (WLAN_MEM_EVENT_SIZE1));
            frw_field_setup((&pst_event->st_event_hdr), en_pipeline, (FRW_EVENT_PIPELINE_STAGE_0));
            frw_field_setup((&pst_event->st_event_hdr), uc_chip_id, (0));
            frw_field_setup((&pst_event->st_event_hdr), uc_device_id, (0));
            frw_field_setup((&pst_event->st_event_hdr), uc_vap_id, (0));

            /* 抛事件 */
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
            frw_event_post_event_ctx(pst_event_mem, core_id, ctx);
#else
            frw_event_dispatch_event_ctx(pst_event_mem, ctx);
#endif
            frw_event_free_append_line(pst_event_mem, mem_ctx);
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
        }
    }
#endif
}

void frw_timer_timeout_proc_event_gt(oal_timeout_func_para_t arg)
{
    frw_timer_timeout_proc_event_ctx(arg, GT_FRW_CONTEXT);
}

void frw_timer_timeout_proc_event_wifi(oal_timeout_func_para_t arg)
{
    frw_timer_timeout_proc_event_ctx(arg, WIFI_FRW_CONTEXT);
}

/*
 * 函 数 名  : frw_timer_delete_all_timer
 * 功能描述  : 将定时器链表中的所有定时器删除，用于异常时外部模块清理定时器
             不能在定时器回调函数中调用
 */
void frw_timer_delete_all_timer_ctx(frw_subsys_context *ctx)
{
    oal_dlist_head_stru *pst_timeout_entry = NULL;
    frw_timeout_stru *pst_timeout_element = NULL;
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_delete_all_timer_ctx:: invalid point}");
        return;
    }
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_spin_lock_bh(&ctx->timer_list_spinlock[core_id]);
        /* 删除所有待删除定时器 */
        pst_timeout_entry = ctx->timer_list[core_id].pst_next;

        while (pst_timeout_entry != &ctx->timer_list[core_id]) {
            pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);

            pst_timeout_entry = pst_timeout_entry->pst_next;

            /* 删除定时器 */
            oal_dlist_delete_entry(&pst_timeout_element->st_entry);
        }

        ctx->frw_timer_start_stamp[core_id] = 0;
        oal_spin_unlock_bh(&ctx->timer_list_spinlock[core_id]);
    }
}

/*
 * 函 数 名  : frw_timer_clean_timer
 * 功能描述  : 删除指定模块残留的所有定时器
 *             本函数不能解决残留定时器的所有问题，一旦发现有残留，需要进行处理。
 * 输入参数  ：en_module_id取值范围oam_module_id_enum
 */
void frw_timer_clean_timer_ctx(uint16_t en_module_id, frw_subsys_context *ctx)
{
    oal_dlist_head_stru *pst_timeout_entry = NULL;
    frw_timeout_stru *pst_timeout_element = NULL;
    uint32_t core_id;
    if (oal_unlikely(oal_any_null_ptr1(ctx))) {
        oam_error_log0(0, OAM_SF_FRW, "{frw_timer_clean_timer_ctx:: invalid point}");
        return;
    }
    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_spin_lock_bh(&ctx->timer_list_spinlock[core_id]);
        pst_timeout_entry = ctx->timer_list[core_id].pst_next;

        while (pst_timeout_entry != &ctx->timer_list[core_id]) {
            if (pst_timeout_entry == NULL) {
                oal_io_print("!!!====TIMER LIST BROKEN====!!!\n");
                break;
            }

            pst_timeout_element = oal_dlist_get_entry(pst_timeout_entry, frw_timeout_stru, st_entry);
            pst_timeout_entry = pst_timeout_entry->pst_next;

            if (en_module_id == pst_timeout_element->en_module_id) {
                oal_dlist_delete_entry(&pst_timeout_element->st_entry);
            }
        }

        if (oal_dlist_is_empty(&ctx->timer_list[core_id])) {
            ctx->frw_timer_start_stamp[core_id] = 0;
        }
        oal_spin_unlock_bh(&ctx->timer_list_spinlock[core_id]);
    }
}

oal_module_symbol(frw_timer_restart_timer_ctx);
oal_module_symbol(frw_timer_create_timer_ctx);
oal_module_symbol(frw_timer_stop_timer_ctx);
oal_module_symbol(frw_timer_add_timer_ctx);
oal_module_symbol(frw_timer_immediate_destroy_timer_ctx);
oal_module_symbol(frw_timer_delete_all_timer_ctx);
oal_module_symbol(frw_timer_clean_timer_ctx);

oal_module_symbol(frw_timer_restart_ctx);
oal_module_symbol(frw_timer_stop_ctx);
