/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_regdomain_bandwidth.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2022年11月29日
 */

#ifndef __MAC_REGDOMAIN_BANDWIDTH_H__
#define __MAC_REGDOMAIN_BANDWIDTH_H__

#include "oal_ext_if.h"
#include "wlan_oneimage_define.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "hd_event.h"

/* 此处不加extern "C" UT编译不过 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct {
    uint8_t uc_cnt;
    wlan_channel_bandwidth_enum_uint8 aen_supp_bw[WLAN_BW_CAP_BUTT];
} mac_supp_mode_table_stru;

extern const mac_supp_mode_table_stru g_bw_mode_table_2g[WLAN_2G_CHANNEL_NUM];
extern const mac_supp_mode_table_stru g_bw_mode_table_5g[WLAN_5G_CHANNEL_NUM];

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
extern const mac_supp_mode_table_stru g_bw_mode_table_6g[WLAN_6G_CHANNEL_NUM];
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of mac_regdomain_bandwidth.h */
