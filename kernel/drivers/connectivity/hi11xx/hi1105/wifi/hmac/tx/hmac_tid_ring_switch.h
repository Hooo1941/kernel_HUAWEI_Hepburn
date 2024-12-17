/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tid ring切换更新模块
 * 作    者 : wifi
 * 创建日期 : 2021年5月20日
 */

#ifndef HMAC_TID_RING_SWITCH_H
#define HMAC_TID_RING_SWITCH_H

#include "hmac_tid_list.h"
#include "hmac_user.h"

typedef struct {
    hmac_tid_list_stru tid_list;
    uint32_t tid_list_count;
    uint32_t tid_fix_switch;
} hmac_tid_ring_switch_stru;

extern hmac_tid_ring_switch_stru g_tid_switch_list;

static inline hmac_tid_ring_switch_stru *hmac_get_tid_ring_switch_list(void)
{
    return &g_tid_switch_list;
}

static inline uint32_t hmac_tid_ring_switch_list_enqueue(hmac_tid_info_stru *tid_info)
{
    if (tid_info->is_in_ring_list == OAL_TRUE || tid_info->tid_no == WLAN_TIDNO_BCAST) {
        return OAL_SUCC;
    }
    tid_info->is_in_ring_list = OAL_TRUE;
    return hmac_tid_list_enqueue(&g_tid_switch_list.tid_list, tid_info);
}

static inline void hmac_tid_ring_switch_list_delete(hmac_tid_info_stru *tid_info)
{
    hmac_tid_list_search_delete(&g_tid_switch_list.tid_list, tid_info);
}

static inline void hmac_tid_switch_list_clear(void)
{
    hmac_tid_list_clear(&g_tid_switch_list.tid_list);
}

void hmac_tid_ring_switch_init(void);
void hmac_tid_ring_switch_deinit(void);
void hmac_tid_ring_switch_process(uint32_t tx_throughput);
void hmac_tid_ring_switch_disable(hmac_user_stru *hmac_user);
void hmac_tid_ring_switch(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info, uint32_t mode);
uint8_t hmac_tid_ring_mode_judge(void);

#endif
