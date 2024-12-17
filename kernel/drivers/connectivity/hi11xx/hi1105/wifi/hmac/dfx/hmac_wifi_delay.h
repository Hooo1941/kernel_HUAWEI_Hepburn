/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp.c 的头文件
 * 作者       : wifi
 * 创建日期   : 2020年12月24日
 */

#ifndef HMAC_WIFI_DELAY_H
#define HMAC_WIFI_DELAY_H

#include "wlan_types.h"
#include "mac_vap.h"
#include "hmac_vap.h"
#include "hmac_user.h"
#include "mac_common.h"
#include "host_hal_device.h"

#ifdef _PRE_DELAY_DEBUG
typedef enum {
    HMAC_DELAY_TX = BIT0,
    HMAC_DELAY_RX = BIT1,
    HMAC_DELAY_TX_DSCR = BIT2,
    HMAC_DELAY_RX_DSCR = BIT3,
    HMAC_SCHED_TX = BIT4,
    HMAC_E2E_DELAY_TX = BIT5,
    HMAC_E2E_DELAY_RX = BIT6,
    HMAC_DELAY_BUT
} hmac_delay_direct_enum;

typedef struct {
    uint32_t host_tsf;
    uint32_t cur_tsf[HAL_MAX_VAP_NUM];
} hmac_wifi_delay_tsf_stru;

typedef struct {
    uint32_t magic_head;
    uint32_t timestamp;
} priv_e2e_trans_stru;

extern uint32_t g_wifi_delay_debug;
extern uint32_t g_wifi_delay_log;

void hmac_delay_rx_skb_rpt_worker(oal_work_stru *work);
uint32_t hmac_delay_rpt_event_process(frw_event_mem_stru *event_mem);
void hmac_register_hcc_callback(uint32_t delay_mask);
void hmac_wifi_delay_tx_hcc_device_tx(oal_netbuf_stru *netbuf, frw_event_hdr_stru *pst_event_hdr);
void hmac_wifi_delay_rx_hcc_device_rx(oal_netbuf_stru *netbuf, frw_event_hdr_stru *pst_event_hdr);
void hmac_wifi_delay_tx_notify(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf);
void hmac_wifi_delay_rx_notify(oal_net_device_stru *net_dev, oal_netbuf_stru *netbuf);
void hmac_host_record_tsf_lo(void);
void hmac_host_get_tsf_lo(hal_host_device_stru *hal_device, uint8_t hal_vap_id, uint32_t *tsf);
#endif
#endif
