/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 11k 功能处理
 * 创建日期 : 2020年7月30日
 */

#ifndef HMAC_11K_H
#define HMAC_11K_H

#include "mac_frame.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "hmac_vap.h"
#include "mac_mib.h"
#include "hmac_mgmt_bss_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_WLAN_FEATURE_11K)
#define HMAC_11K_SUPPORT_AP_CHAN_RPT_NUM 8
#define HMAC_MEASUREMENT_RPT_FIX_LEN     5
#define HMAC_BEACON_RPT_FIX_LEN          26
#define HMAC_MAX_RPT_DETAIL_LEN          224 /* 255 - 26(bcn fix) - 3(Meas rpt fix) - 2(subid 1) */

#define HMAC_ACTION_RPT_FIX_LEN                 3
#define HMAC_RADIO_MEAS_ACTION_REQ_FIX_LEN      5
#define HMAC_NEIGHBOR_REPORT_ACTION_REQ_FIX_LEN 3
#define HMAC_NEIGHBOR_REPORT_ACTION_RSP_FIX_LEN 3
#define HMAC_NEIGHBOR_RPT_MIN_LEN               13
#endif

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
void hmac_scan_update_bss_list_rrm(mac_bss_dscr_stru *bss_dscr,
                                   uint8_t *frame_body,
                                   uint16_t frame_len);
#endif
#if defined(_PRE_WLAN_FEATURE_11K)
uint32_t hmac_scan_rrm_proc_save_bss(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *param);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of hmac_11k.h */
