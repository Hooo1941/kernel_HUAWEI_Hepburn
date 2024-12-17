/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : Host发送完成模块
 * 作    者 : wifi
 * 创建日期 : 2020年4月23日
 */

#ifndef HMAC_HOST_TX_DATA_H
#define HMAC_HOST_TX_DATA_H

#include "hmac_vap.h"
#include "hmac_tid_sche.h"

uint32_t hmac_ring_tx(hmac_vap_stru *hmac_vap, oal_netbuf_stru **netbuf, mac_tx_ctl_stru *tx_ctl);
uint32_t hmac_host_tx_tid_enqueue(hmac_tid_info_stru *hmac_tid_info, oal_netbuf_stru *netbuf);
uint32_t hmac_host_ring_tx(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
void hmac_host_ring_tx_suspend(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info);
void hmac_host_ring_tx_resume(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, hmac_tid_info_stru *tid_info);
oal_bool_enum_uint8 hmac_is_ring_tx(mac_vap_stru *mac_vap, oal_netbuf_stru *netbuf, mac_tx_ctl_stru *tx_ctl);
uint32_t hmac_host_tx_data(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);

#endif
