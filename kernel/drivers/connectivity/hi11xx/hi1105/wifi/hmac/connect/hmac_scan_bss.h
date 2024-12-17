/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_scan.c 的头文件
 * 作    者 :
 * 创建日期 : 2014年11月26日
 */

#ifndef HMAC_SCAN_BSS_H
#define HMAC_SCAN_BSS_H

#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_vap.h"
#include "mac_device.h"
#include "hmac_device.h"
#include "mac_ie.h"

hmac_scanned_bss_info *hmac_scan_alloc_scanned_bss(uint32_t mgmt_len);
uint32_t hmac_scan_update_bss_dscr(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, uint16_t buf_len, hmac_scanned_bss_info *scanned_bss, uint8_t ext_len);
void hmac_scan_save_mbss_info(hmac_vap_stru *hmac_vap,
    mac_multi_bssid_frame_info_stru *mbss_frame_info, hmac_scanned_bss_info *new_scanned_bss);
void hmac_scan_update_opmode_and_rssi(hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss);
oal_bool_enum_uint8 hmac_scan_is_hidden_ssid(uint8_t vap_id, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss);
oal_bool_enum_uint8 hmac_scan_need_update_old_scan_result(hmac_vap_stru *hmac_vap, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss);
void hmac_scan_update_time_and_rssi(oal_netbuf_stru *netbuf,
    hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss);
uint32_t hmac_scan_del_bss_from_list_nolock(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device);
uint32_t hmac_scan_add_bss_to_list(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device);
void hmac_scan_clean_expire_scanned_bss(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record);
#endif /* end of hmac_scan_bss.h */
