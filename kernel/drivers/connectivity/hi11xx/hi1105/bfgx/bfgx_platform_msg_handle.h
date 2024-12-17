/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_platform_msg_handle.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_PLATFORM_MSG_HANDLE_H
#define BFGX_PLATFORM_MSG_HANDLE_H

#include "hw_bfg_ps.h"

typedef enum {
    THREAD_FLAG_INIT = 0x00,
    THREAD_FLAG_EXIT = 0x01,
    THREAD_FLAG_BUTT = 0x02,
} bfgx_thread_flag_enum;

/* 函数声明 */
int32_t ps_exe_sys_func(struct ps_core_s *ps_core_d, uint8_t *buf_ptr);

#endif /* __BFGX_PLATFORM_MSG_HANDLE_H__ */
