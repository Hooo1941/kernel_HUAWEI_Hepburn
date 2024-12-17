/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_dbac.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2019年12月26日
 */

#ifndef HMAC_DBAC_H
#define HMAC_DBAC_H

/* 1 其他头文件包含 */
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DBAC_H
/* 2 宏定义 */
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    frw_timeout_stru st_dbac_timer;
} hmac_dbac_handle_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
uint32_t hmac_dbac_status_notify(frw_event_mem_stru *pst_event_mem);
uint32_t hmac_dbac_ap_flow_other_vap_channel_handle(mac_device_stru *mac_device);
uint32_t hmac_dbac_enable_follow_channel(mac_vap_stru *mac_vap, uint32_t *params);
oal_bool_enum_uint8 hmac_dbac_get_channel_flow_info(mac_vap_stru *mac_vap0, mac_vap_stru *mac_vap1,
    mac_vap_stru **csa_vap, mac_channel_stru *csa_channel);
#endif /* end of hmac_dbac.h */
