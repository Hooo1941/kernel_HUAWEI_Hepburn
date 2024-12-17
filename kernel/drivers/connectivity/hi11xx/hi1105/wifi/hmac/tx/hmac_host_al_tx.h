/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_host_al_tx.c 的头文件
 * 作者       : wifi
 * 创建日期   : 2020年05月15日
 */

#ifndef HMAC_HOST_AL_TX_H
#define HMAC_HOST_AL_TX_H

/* 1 其他头文件包含 */
#include "mac_common.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_HMAC_TX_MSDU_DSCR_H

extern hmac_msdu_info_ring_stru g_always_tx_ring[WLAN_DEVICE_MAX_NUM_PER_CHIP];
uint32_t hmac_always_tx_proc(uint8_t *param, uint8_t hal_dev_id);
void hmac_al_tx_ring_release(hmac_msdu_info_ring_stru *tx_ring);

#endif
