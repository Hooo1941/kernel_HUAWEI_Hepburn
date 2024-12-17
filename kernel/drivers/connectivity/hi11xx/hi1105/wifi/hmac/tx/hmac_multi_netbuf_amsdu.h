/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_multi_netbuf_amsdu.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2020年9月8日
 */
#ifndef HMAC_MULTI_NETBUF_AMSDU_H
#define HMAC_MULTI_NETBUF_AMSDU_H

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#include "oal_types.h"
#include "oam_ext_if.h"
#include "wlan_types.h"
#include "hmac_vap.h"
#include "hmac_auto_adjust_freq.h"
#include "wlan_chip_i.h"
#define WLAN_AMSDU_THROUGHPUT_TH_HE_VHT_DIFF             100 /* 100Mb/s */
#define WLAN_TX_AMSDU_SWITCH_DOWN_LIMIT_CNT              5
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
void hmac_tx_small_amsdu_get_limit_throughput(uint32_t *limit_throughput_high,
    uint32_t *limit_throughput_low);
void hmac_tx_small_amsdu_get_limit_pps(uint32_t *limit_pps_high, uint32_t *limit_pps_low) ;
uint8_t hmac_update_amsdu_num_mp13(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag);
uint8_t hmac_update_amsdu_num_mp15(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag);
uint8_t hmac_update_amsdu_num_mp16(mac_vap_stru *mac_vap, hmac_performance_stat_stru *performance_stat_params,
    oal_bool_enum_uint8 mu_vap_flag);
void hmac_tx_amsdu_ampdu_switch(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif
