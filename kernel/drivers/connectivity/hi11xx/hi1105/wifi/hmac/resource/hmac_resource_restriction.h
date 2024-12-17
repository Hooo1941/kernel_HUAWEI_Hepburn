/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 资源限制头文件
 * 作    者 :
 * 创建日期 : 2023年2月1日
 */
#ifndef HMAC_RESOURCE_RESTRICTION_H
#define HMAC_RESOURCE_RESTRICTION_H

#include "oal_types.h"
#include "mac_device.h"
#include "mac_vap.h"
#define HMAC_MULTI_VAP_STATE_SUPPORT 1
#define HMAC_MULTI_VAP_STATE_UNSUPPORT 0
/* 当前vap共存规格最大支持3个，偏移转化时最大偏移数也为3 */
#define HMAC_VAP_MODE_OFFSET_TRANS_MAX_CNT 3
#define HMAC_MULTI_VAP_MODE_OFFSET 5

oal_bool_enum_uint8 hmac_res_restriction_is_ap_with_other_vap(void);
uint32_t hmac_vap_mode_trans(wlan_vap_mode_enum_uint8 vap_mode, wlan_p2p_mode_enum_uint8 p2p_mode,
    uint8_t is_add_chba);
oal_bool_enum_uint8 hmac_vap_up_check_vap_res_supp(mac_vap_stru *mac_vap);
#endif /* endof hmac_resource_restriction.h */
