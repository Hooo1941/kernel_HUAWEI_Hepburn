/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mp16c host 描述符
 * 作    者 :
 * 创建日期 : 2021年2月22日
 */

#ifndef HOST_SNIFFER_MP16_H
#define HOST_SNIFFER_MP16_H

#ifdef _PRE_WLAN_FEATURE_SNIFFER

#include "oal_ext_if.h"
#include "hal_common.h"

void mp16_sniffer_rx_info_fill(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf,
    hal_sniffer_extend_info *sniffer_rx_info, mac_rx_ctl_stru *rx_ctl);
uint32_t hmac_get_rate_kbps(hal_statistic_stru *rate_info, uint32_t *rate_kbps);
uint32_t mp16_sniffer_rx_ppdu_free_ring_init(hal_host_device_stru *hal_device);
#endif
#endif
