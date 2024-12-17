/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef __MAC_6GHZ_FRAME_COMMON_H__
#define __MAC_6GHZ_FRAME_COMMON_H__

/* 头文件包含 */
#include "wlan_types.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "securectype.h"
#include "mac_element.h"

#define MAC_HE_6GHZ_OPERATION_INFO_LEN 5
#define MAC_HE_6GHZ_BAND_CAP_LEN       2

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
struct mac_frame_he_6ghz_oper_info {
    uint8_t primary_channel;
    uint8_t bit_channel_width : 2,
            bit_duplicate_bcn : 1,
            bit_reserved : 5;
    uint8_t center_freq_seg0;
    uint8_t center_freq_seg1;
    uint8_t minimum_rate;
} DECLARE_PACKED;
typedef struct mac_frame_he_6ghz_oper_info mac_frame_he_6ghz_oper_info_stru;
#endif

#endif /* end of mac_6ghz_frame_common.h */
