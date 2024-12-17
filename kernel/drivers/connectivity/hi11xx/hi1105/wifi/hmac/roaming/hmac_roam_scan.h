/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_roam_scan.c 的头文件
 * 创建日期 : 2020年09月24日
 */

#ifndef HMAC_ROAM_SCAN_H
#define HMAC_ROAM_SCAN_H

#include "hmac_vap.h"
#include "hmac_device.h"
#include "hmac_roam_alg.h"
#include "hmac_config.h"

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_ROAM_SCAN_H

uint32_t hmac_roam_check_dual_threshold(hmac_roam_info_stru *roam_info);
uint32_t hmac_roam_scan_channel_init(hmac_roam_info_stru *pst_roam_info,
                                     mac_scan_req_stru *pst_scan_params);
uint32_t hmac_roam_scan_init(hmac_roam_info_stru *roam_info, void *param);
uint32_t hmac_roam_scan_channel(hmac_roam_info_stru *roam_info, void *param);

uint32_t hmac_roam_check_bkscan_result(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record);
int8_t hmac_get_rssi_from_scan_result(hmac_vap_stru *hmac_vap, uint8_t *bssid);
roam_channel_org_enum hmac_roam_choose_scan_type_by_bssid(mac_vap_stru *mac_vap,
    hmac_conn_param_stru *connect_param);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
