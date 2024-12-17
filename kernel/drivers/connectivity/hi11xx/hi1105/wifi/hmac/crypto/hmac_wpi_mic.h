/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_mic.c头文件
 */
#ifndef __HMAC_WPI_MIC_H__
#define __HMAC_WPI_MIC_H__
#if (defined(_PRE_WLAN_FEATURE_WAPI) || defined(_PRE_WLAN_FEATURE_PWL))

#include "mac_frame_common.h"
#define SMS4_MIC_PART1_QOS_LEN    48 /* 按照协议，如果有qos字段，mic第一部分16字节对齐后的长度 */
#define SMS4_MIC_PART1_NO_QOS_LEN 32 /* 按照协议，如果没有qos字段，mic第一部分16字节对齐后的长度 */
#define SMS4_PADDING_LEN      16 /* mic data按照16字节对齐 */
#define WPI_MIC_LEN        16   /* wpi协议下mic的长度16字节 */
#define RK_LEN         32   /* round key size */
#define SMS4_STEP      4
#define MAX_MIC_LEN    4096 /* 计算校验和允许的最大数据长度 */
#define IV_LEN         4
uint32_t hmac_prepare_wpi_mic_data(mac_ieee80211_frame_stru *mac_hdr, uint8_t *payload, uint16_t payload_len,
    uint16_t key_idx, uint8_t *mic, uint16_t mic_len);
uint8_t *hmac_wpi_mic_data_alloc(oal_bool_enum_uint8 is_qos, uint16_t payload_len, uint16_t *mic_data_len);
uint32_t hmac_wpi_calc_mic(uint8_t *data, uint32_t data_len,
    uint8_t *iv, uint8_t *key, uint8_t *mic, uint8_t mic_len);

#endif
#endif
