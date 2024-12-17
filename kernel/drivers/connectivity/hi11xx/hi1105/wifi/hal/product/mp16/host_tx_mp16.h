/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host侧Shenkuo芯片相关TX功能
 * 作    者 : wifi
 * 创建日期 : 2021年3月1日
 */

#ifndef HOST_TX_MP16_H
#define HOST_TX_MP16_H

#include "oal_ext_if.h"

uint32_t mp16_host_tx_clear_msdu_padding(oal_netbuf_stru *netbuf);
uint8_t mp16_host_get_device_tx_ring_num(void);

#endif

