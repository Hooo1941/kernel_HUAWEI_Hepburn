/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MinTP日志
 * 作    者 : wifi
 * 创建日期 : 2023年1月30日
 */

#ifndef HMAC_MINTP_LOG_H
#define HMAC_MINTP_LOG_H

#include "mac_vap.h"

/* MinTP维测等级, lvl0 -> lvl2维测等级逐步提升, 日志打印范围变广 */
enum {
    MINTP_LOG_LVL_DISABLE,
    MINTP_LOG_LVL_0,
    MINTP_LOG_LVL_1,
    MINTP_LOG_LVL_2,
};

/* MinTP维测打印数据类型 */
enum {
    MINTP_LOG_DATATYPE_SKB,
    MINTP_LOG_DATATYPE_SKB_NEXT_LIST,
    MINTP_LOG_DATATYPE_SKB_HEAD_LIST,
    MINTP_LOG_DATATYPE_CMD,
    MINTP_LOG_DATATYPE_BUTT,
};

/* MinTP维测类型 */
enum {
    MINTP_LOG_TYPE_PARSE_CMD,
    MINTP_LOG_TYPE_BUILD_LIST,
    MINTP_LOG_TYPE_TX,
    MINTP_LOG_TYPE_PUSH_RING,
    MINTP_LOG_TYPE_RX,
    MINTP_LOG_TYPE_BUTT,
};

void hmac_mintp_log(uint8_t *data, uint8_t lvl, uint8_t datatype, uint8_t type);
uint32_t hmac_mintp_set_log_lvl(mac_vap_stru *mac_vap, uint32_t *params);

#endif
