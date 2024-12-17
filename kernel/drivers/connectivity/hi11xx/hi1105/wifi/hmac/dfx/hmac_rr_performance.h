/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_rr_perofrmance.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2020年2月23日
 */
#ifndef HMAC_RR_PERFOTMANCE_H
#define HMAC_RR_PERFOTMANCE_H

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/pm_qos.h>
#endif
#include "oal_types.h"
#include "oam_ext_if.h"

/* RR性能统计device时间打点 */
typedef struct {
    oal_switch_enum_uint8 rr_switch; /* 统计开关 */
    uint8_t reserve[7];            /* 7 内存对齐 */
    uint64_t tx_w2hmac_timestamp;
    uint64_t tx_hmac2d_timestamp;
    uint64_t rx_hmac2w_timestamp;
    uint64_t rx_d2hmac_timestamp;
} hmac_perform_host_timestamp_t;

extern hmac_perform_host_timestamp_t g_host_rr_perform_timestamp;

uint64_t hmac_get_ktimestamp(void);
void hmac_rr_tx_w2h_timestamp(void);
void hmac_rr_tx_h2d_timestamp(void);
void hmac_rr_rx_d2h_timestamp(void);
void hmac_rr_rx_h2w_timestamp(void);
#endif
#endif
