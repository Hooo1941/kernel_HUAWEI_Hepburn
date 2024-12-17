/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: exception_common.c header file
 * Author: @CompanyNameTag
 */

#ifndef EXCEPTION_COMMON_H
#define EXCEPTION_COMMON_H

#include "plat_type.h"
#include "oal_ext_if.h"

typedef struct {
    struct sk_buff_head quenue;
    int32_t is_open;
    int32_t is_working;
    wait_queue_head_t dump_type_wait;
    struct sk_buff_head dump_type_queue;
} memdump_info_t;

typedef struct {
    uint32_t reserve; /* 预留数据用于获取结构体首地址 */
} excp_comm_t;

typedef struct {
    uint32_t exception_type;
    uint32_t subsys_type;
    uint32_t core;
} excp_record_t;

extern memdump_info_t g_bcpu_memdump_cfg;
extern memdump_info_t g_gcpu_memdump_cfg;
extern memdump_info_t g_wcpu_memdump_cfg;
extern memdump_info_t g_gt_memdump_cfg;

int32_t excp_memdump_queue(uint8_t *buf_ptr, uint16_t count, memdump_info_t *memdump_t);
int32_t notice_hal_memdump(memdump_info_t *memdump_t, uint32_t which_dump);
int32_t is_subsystem_rst_enable(void);
#endif
