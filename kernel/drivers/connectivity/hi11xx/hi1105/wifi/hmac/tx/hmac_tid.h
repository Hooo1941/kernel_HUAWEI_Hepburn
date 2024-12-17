/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Hmac tid模块
 * 作    者 : wifi
 * 创建日期 : 2020年7月23日
 */

#ifndef HMAC_TID_H
#define HMAC_TID_H

#include "mac_common.h"
#include "oal_ext_if.h"
#include "frw_ext_if.h"

typedef union {
    uint32_t word2;
    struct {
        uint16_t write_ptr;
        uint16_t tx_msdu_ring_depth : 4;
        uint16_t max_aggr_amsdu_num : 4;
        uint16_t reserved : 8;
    } word2_bit;
} msdu_info_word2;

typedef union {
    uint32_t word3;
    struct {
        uint16_t read_ptr;
        uint16_t reserved;
    } word3_bit;
} msdu_info_word3;

#define TX_RING_INFO_LEN      16
#define TX_RING_INFO_WORD_NUM (TX_RING_INFO_LEN / sizeof(uint32_t))

typedef struct {
    /* 保存tx ring table的4个word地址,经devca_to_hostva转换为64位地址, 结构与hal_tx_msdu_info_ring_stru一致 */
    uintptr_t word_addr[TX_RING_INFO_WORD_NUM];
    msdu_info_word2 msdu_info_word2;  /* wptr所属word2 */
    msdu_info_word3 msdu_info_word3;  /* rptr所属word3 */
} hmac_tx_ring_device_info_stru;

typedef struct {
    uint8_t chr_report_flag;
    uint32_t last_push_ring_tsf;
} hmac_tx_ring_stuck_check_stru;

typedef struct {
    msdu_info_ring_stru base_ring_info;
    uint8_t *host_ring_buf;
    uintptr_t host_ring_dma_addr;
    uint32_t host_ring_buf_size;
    uint16_t release_index;
    oal_atomic last_period_tx_msdu;
    oal_atomic msdu_cnt;             /* 当前ring中的msdu数 */
    oal_atomic inited;
    oal_atomic ring_sync_th;
    uint32_t ring_sync_cnt;
    oal_atomic ring_tx_opt;
    hmac_tx_ring_device_info_stru *tx_ring_device_info;
    uint8_t host_tx_flag;
    uint16_t push_ring_full_cnt;
    uint32_t tx_msdu_cnt;            /* 当前ring已发送的msdu个数 */
    oal_netbuf_stru **netbuf_list;
    oal_mutex_stru tx_lock;
    oal_mutex_stru tx_comp_lock;
    hmac_tx_ring_stuck_check_stru tx_ring_stuck_check;
} hmac_msdu_info_ring_stru;

typedef struct {
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct list_head tid_sche_entry;
    struct list_head tid_update_entry;
#else
    oal_dlist_head_stru tid_sche_entry;
    oal_dlist_head_stru tid_update_entry;
#endif
    oal_dlist_head_stru tid_tx_entry;
    oal_dlist_head_stru tid_tx_comp_entry;
    oal_dlist_head_stru tid_ring_switch_entry;
    uint8_t tid_no;
    uint8_t is_in_ring_list; /* 该tid是否已存在tx ring切换链表 */
    uint16_t user_index;
    uint8_t ring_switch_fail_cnt;
    uint8_t resv[3];
    uint32_t push_fail_cnt;
    oal_atomic inited;
    oal_atomic ring_tx_mode;         /* 发送模式host ring tx/device ring tx/device tx */
    oal_atomic tid_sche_th;
    oal_spin_lock_stru lock;
    oal_netbuf_head_stru tid_queue;
    hmac_msdu_info_ring_stru tx_ring;
} hmac_tid_info_stru;

typedef uint8_t (* hmac_tid_list_func)(hmac_tid_info_stru *, void *);

typedef struct {
    hmac_tid_info_stru* (*tid_info_getter)(void *);
    void* (*entry_getter)(hmac_tid_info_stru *);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint8_t (*for_each_rcu)(hmac_tid_list_func, void *);
#endif
} hmac_tid_list_ops;

static inline uint8_t hmac_tid_info_inited(hmac_tid_info_stru *tid_info)
{
    return oal_atomic_read(&tid_info->inited);
}

static inline uint8_t hmac_tx_ring_switching(hmac_tid_info_stru *tid_info)
{
    return oal_atomic_read(&tid_info->ring_tx_mode) != HOST_RING_TX_MODE &&
           oal_atomic_read(&tid_info->ring_tx_mode) != DEVICE_RING_TX_MODE;
}

static inline bool hmac_tid_ring_tx_opt(hmac_tid_info_stru *tid_info)
{
    return oal_atomic_read(&tid_info->tx_ring.ring_tx_opt);
}

static inline void hmac_tid_netbuf_enqueue(oal_netbuf_head_stru *tid_queue, oal_netbuf_stru *netbuf)
{
    oal_spin_lock_head_bh(tid_queue);
    mac_get_cb_retried_num((mac_tx_ctl_stru *)oal_netbuf_cb(netbuf))++;
    oal_netbuf_list_tail_nolock(tid_queue, netbuf);
    oal_spin_unlock_head_bh(tid_queue);
}

static inline oal_netbuf_stru *hmac_tid_netbuf_dequeue(oal_netbuf_head_stru *tid_queue)
{
    if (oal_netbuf_list_empty(tid_queue)) {
        return NULL;
    }

    return oal_netbuf_delist(tid_queue);
}

#endif
