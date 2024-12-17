/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: frw_timer.c header file
 * Author: @CompanyNameTag
 */

#ifndef FRW_TIMER_H
#define FRW_TIMER_H

/* 其他头文件包含 */
#include "frw_ext_if.h"
#include "frw_subsys_context.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TIMER_H

/* 宏定义 */
#define FRW_TIME_UL_MAX       0xFFFFFFFF
#define FRW_TIMER_MAX_TIMEOUT (FRW_TIME_UL_MAX >> 1) /* 32位最大值的1/2 */

/* 函数声明 */
uint32_t frw_timer_timeout_proc_gt(frw_event_mem_stru *pst_timeout_event);
uint32_t frw_timer_timeout_proc_wifi(frw_event_mem_stru *pst_timeout_event);
void frw_timer_init_ctx(uint32_t delay, oal_timer_func p_func, uintptr_t arg, frw_subsys_context *ctx);
void frw_timer_exit_ctx(frw_subsys_context *ctx);
void frw_timer_timeout_proc_event_gt(oal_timeout_func_para_t arg);
void frw_timer_timeout_proc_event_wifi(oal_timeout_func_para_t arg);
void frw_timer_restart_ctx(frw_subsys_context *ctx);
void frw_timer_stop_ctx(frw_subsys_context *ctx);

/* return true if the time a is after time b,in case of overflow and wrap around to zero */
OAL_STATIC OAL_INLINE int32_t frw_time_after(uint32_t a, uint32_t b)
{
    return ((int32_t)((b) - (a)) <= 0);
}

#endif /* end of frw_timer.h */
