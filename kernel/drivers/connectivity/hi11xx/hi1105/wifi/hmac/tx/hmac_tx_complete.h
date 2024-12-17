/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host发送完成模块
 * 作    者 : wifi
 * 创建日期 : 2020年4月23日
 */

#ifndef HMAC_TX_COMPLETE_H
#define HMAC_TX_COMPLETE_H

#include "hmac_tx_data.h"
#include "hmac_tid_list.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TX_COMPLETE_H
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    struct semaphore tx_comp_sema;
    oal_atomic hal_dev_tx_comp_triggered[WLAN_DEVICE_MAX_NUM_PER_CHIP];
    oal_task_stru *tx_comp_thread;
    hmac_tid_list_stru tid_tx_list;
} hmac_tx_comp_stru;

typedef struct {
    hmac_user_stru *hmac_user;
    uint8_t tid_no;
} hmac_tx_comp_info_stru;

extern hmac_tx_comp_stru g_tx_comp_mgmt;

static inline hmac_tx_comp_stru *hmac_get_tx_comp_mgmt(void)
{
    return &g_tx_comp_mgmt;
}

static inline void hmac_set_tx_comp_triggered(uint8_t hal_dev_id)
{
    oal_atomic_set(&g_tx_comp_mgmt.hal_dev_tx_comp_triggered[hal_dev_id], OAL_TRUE);
}

static inline void hmac_clear_tx_comp_triggered(uint8_t hal_dev_id)
{
    oal_atomic_set(&g_tx_comp_mgmt.hal_dev_tx_comp_triggered[hal_dev_id], OAL_FALSE);
}

static inline uint8_t hmac_get_tx_comp_triggered(uint8_t hal_dev_id)
{
    return oal_atomic_read(&g_tx_comp_mgmt.hal_dev_tx_comp_triggered[hal_dev_id]) == OAL_TRUE;
}

static inline void hmac_tx_comp_scheduled(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    up(&g_tx_comp_mgmt.tx_comp_sema);
#endif
}

uint32_t hmac_tx_complete_update_tid_ring_rptr(hmac_tid_info_stru *tid_info);
void hmac_tx_complete_tid_ring_process(hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info);
void hmac_tx_comp_init(void);
int32_t hmac_tx_comp_thread(void *p_data);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
