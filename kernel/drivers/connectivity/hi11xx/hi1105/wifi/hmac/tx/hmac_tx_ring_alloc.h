/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tx ring内存分配模块
 * 作    者 : wifi
 * 创建日期 : 2021年12月23日
 */

#ifndef HMAC_TX_RING_ALLOC_H
#define HMAC_TX_RING_ALLOC_H

#include "hmac_user.h"

uint32_t hmac_ring_tx_init(hmac_user_stru *hmac_user, uint8_t tid);
void hmac_ring_tx_deinit(hmac_user_stru *hmac_user, uint8_t tid);
void hmac_tid_info_deinit(hmac_tid_info_stru *tid_info);
uint32_t hmac_alloc_tx_ring_by_tid(hmac_msdu_info_ring_stru *tx_ring, uint8_t tid_no);
uint32_t hmac_alloc_tx_ring_after_inited(hmac_tid_info_stru *tid_info, hmac_msdu_info_ring_stru *tx_ring);
void hmac_user_tx_ring_deinit(hmac_user_stru *hmac_user);
uint32_t hmac_set_tx_ring_device_base_addr(frw_event_mem_stru *frw_mem);
void hmac_tx_host_ring_release_nolock(hmac_msdu_info_ring_stru *tx_ring);
void hmac_tx_host_ring_release(hmac_msdu_info_ring_stru *tx_ring);
void hmac_wait_tx_ring_empty(hmac_tid_info_stru *tid_info);
void hmac_tx_ring_release_all_netbuf(hmac_msdu_info_ring_stru *tx_ring);

static inline uint8_t hmac_tx_ring_alloced(hmac_msdu_info_ring_stru *tx_ring)
{
    return tx_ring->host_ring_buf != NULL && tx_ring->netbuf_list != NULL;
}

#endif
