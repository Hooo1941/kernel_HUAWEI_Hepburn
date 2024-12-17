/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 文 件 名   : hmac_gnss_rsmc.h
 * 功能描述   : hmac_gnss_rsmc.c头文件
 * 修改历史   :
 * 1.日    期   : 2021年3月25日
 *   作    者   : wifi
 *   修改内容   : 创建文件
 */
#ifndef HMAC_GNSS_RSMC_H
#define HMAC_GNSS_RSMC_H

#ifdef _PRE_WLAN_FEATURE_GNSS_RSMC
#include "mac_vap.h"
uint32_t hmac_process_gnss_rsmc_status_sync(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t hmac_process_gnss_rsmc_status_cmd(mac_vap_stru *mac_vap, uint32_t *params);
#endif

#endif
