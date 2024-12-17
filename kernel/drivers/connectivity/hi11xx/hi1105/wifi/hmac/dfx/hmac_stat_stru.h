/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI HOST统计结构体文件
 * 作    者 :
 * 创建日期 : 2020年7月21日
 */
#ifndef HMAC_STAT_STRU_H
#define HMAC_STAT_STRU_H

#include "oal_types.h"
#include "oal_schedule.h"

/* HMAC device级别统计 */
typedef struct hmac_device_stat {
    oal_atomic rx_packets;
    oal_atomic rx_bytes;
    oal_atomic tx_packets;
    oal_atomic tx_bytes;
    oal_atomic host_ring_rx_packets;

    uint32_t total_rx_packets;
    uint64_t total_rx_bytes;

    uint32_t total_tx_packets;
    uint64_t total_tx_bytes;

    oal_atomic rx_tcp_checksum_fail;
    oal_atomic rx_tcp_checksum_pass;
    oal_atomic rx_ip_checksum_fail;
    oal_atomic rx_ip_checksum_pass;

    uint64_t rx_tcp_checksum_total_fail;
    uint64_t rx_tcp_checksum_total_pass;
    uint64_t rx_ip_checksum_total_fail;
    uint64_t rx_ip_checksum_total_pass;
} hmac_device_stat_stru;

typedef struct hmac_device_screen_stat {
    uint8_t     lp_miracast_enable;
    uint8_t     debug_on;
    uint8_t     throughput_cnt;
    uint8_t     protocol_allow_siso;
    uint8_t     host_allow_m2s_siso;
    uint8_t     device_allow_m2s_siso;
    uint8_t     allow_switch_siso;
    uint8_t     bt_forbid_switch;
    uint32_t    last_time;
} hmac_lp_miracast_stat_stru;

/* HMAC VAP级别统计 */
typedef struct hmac_vap_stat {
    uint32_t rx_packets;
    uint32_t rx_dropped_misc;
    uint32_t tx_packets;
    uint32_t tx_failed;
    uint32_t tx_retries;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} hmac_vap_stat_stru;

/* HMAC user级别统计 */
typedef struct hmac_user_stat {
    uint32_t rx_packets;
    uint32_t rx_dropped_misc;
    uint32_t tx_packets;
    uint32_t tx_failed;
    uint32_t tx_retries;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} hmac_user_stat_stru;

#endif /* end of hmac_stat_stru.h */

