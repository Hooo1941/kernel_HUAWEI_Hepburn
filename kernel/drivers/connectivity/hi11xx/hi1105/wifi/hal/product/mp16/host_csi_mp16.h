/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : CSI功能
* 作者       : wifi
* 创建日期   : 2020年11月17日
*/
#ifndef HOST_CSI_MP16_H
#define HOST_CSI_MP16_H

#include "hmac_user.h"
#include "hmac_vap.h"
#include "host_mac_mp16.h"
#include "host_dscr_mp16.h"
#include "wlan_chip.h"

#define MP16_MAC_CFG_CSI_SAMPLE_PERIOD_LEN      4
#define MP16_MAC_CFG_CSI_SAMPLE_PERIOD_OFFSET   24
#define MP16_MAC_CFG_CSI_SAMPLE_PERIOD_MASK     0xF000000
#define MP16_MAC_CFG_CSI_BAND_WIDTH_MASK_LEN    4
#define MP16_MAC_CFG_CSI_BAND_WIDTH_MASK_OFFSET 20
#define MP16_MAC_CFG_CSI_BAND_WIDTH_MASK_MASK   0xF00000
#define MP16_MAC_CFG_CSI_FRAME_TYPE_MASK_LEN    4
#define MP16_MAC_CFG_CSI_FRAME_TYPE_MASK_OFFSET 16
#define MP16_MAC_CFG_CSI_FRAME_TYPE_MASK_MASK   0xF0000
#define MP16_MAC_CFG_CSI_H_MATRIX_RPT_EN_LEN    1
#define MP16_MAC_CFG_CSI_H_MATRIX_RPT_EN_OFFSET 1
#define MP16_MAC_CFG_CSI_H_MATRIX_RPT_EN_MASK   0x2
#define MP16_MAC_CFG_CSI_EN_LEN                 1
#define MP16_MAC_CFG_CSI_EN_OFFSET              0
#define MP16_MAC_CFG_CSI_EN_MASK                0x1

#define MP16_MAC_CFG_RX_PPDU_DESC_MODE_LEN                2
#define MP16_MAC_CFG_RX_PPDU_DESC_MODE_OFFSET             16
#define MP16_MAC_CFG_RX_PPDU_DESC_MODE_MASK               0x30000

#define MP16_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_LEN     1
#define MP16_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_OFFSET  10
#define MP16_RX_CHN_EST_REG_BANK_CFG_CHN_CONDITION_NUM_RPT_EN_MASK    0x400

#define MP16_MAC_RPT_DMAC_INTR_LOCATION_COMPLETE_LEN        1
#define MP16_MAC_RPT_DMAC_INTR_LOCATION_COMPLETE_OFFSET     29
#define MP16_MAC_RPT_DMAC_INTR_LOCATION_COMPLETE_MASK       0x20000000

#define MP16_MAC_RPT_HOST_INTR_LOCATION_COMPLETE_LEN             1
#define MP16_MAC_RPT_HOST_INTR_LOCATION_COMPLETE_OFFSET          6
#define MP16_MAC_RPT_HOST_INTR_LOCATION_COMPLETE_MASK            0x40

#define MP16_MAC_RPT_LOCATION_INFO_MASK_LEN                 2
#define MP16_MAC_RPT_LOCATION_INFO_MASK_OFFSET              0
#define MP16_MAC_RPT_LOCATION_INFO_MASK_MASK                0x3

#ifdef _PRE_WLAN_FEATURE_CSI

uint32_t mp16_get_host_ftm_csi_locationinfo(hal_host_device_stru *hal_device);
uint32_t mp16_get_csi_info(hmac_csi_info_stru *hmac_csi_info, uint8_t *addr);
uint32_t mp16_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
uint32_t mp16_host_ftm_csi_init(hal_host_device_stru *hal_device);
#endif
#endif
