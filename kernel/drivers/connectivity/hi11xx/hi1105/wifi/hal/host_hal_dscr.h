﻿/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host 描述符公共部分
 * 作    者 :
 * 创建日期 : 2021年2月22日
 */

#ifndef HOST_HAL_DSCR_H
#define HOST_HAL_DSCR_H

#include "host_hal_device.h"

#define HAL_RX_DSCR_LEN              48

typedef struct {
    uint32_t bit_user_id     : 12,
             bit_tid         : 4,
             bit_tx_ba_ssn   : 12,
             bit_resv        : 3,
             bit_ba_info_vld : 1;
    uint32_t bit_tx_msdu_info_ring_rptr : 16,
             bit_tx_ba_winsize : 8,
             reserved : 8;
    uint32_t ba_bitmap[HAL_BA_BITMAP_SIZE];
} hal_tx_ba_info_dscr_stru;

typedef enum {
    HAL_TX_RING_SIZE_GRADE_DEFAULT = 0,
    HAL_TX_RING_SIZE_GRADE_DOWNGRADE = 1,
    HAL_TX_RING_SIZE_GRADE_DOWNGRADE_TWICE = 2,

    HAL_TX_RING_SIZE_GRADE_BUTT,
} hal_tx_ring_size_grade;

#define HAL_HOST_USER_TID_TX_RING_SIZE_ERROR 0xff
#define BA_INFO_DATA_SIZE sizeof(hal_tx_ba_info_dscr_stru)
#define HAL_DEFAULT_BA_INFO_COUNT 64

oal_netbuf_stru *hal_alloc_list_delete_netbuf(hal_host_device_stru *hal_dev,
    hal_host_rx_alloc_list_stru *alloc_list, dma_addr_t host_iova, uint32_t *pre_num);

#endif /* end of host_hal_dscr.h */