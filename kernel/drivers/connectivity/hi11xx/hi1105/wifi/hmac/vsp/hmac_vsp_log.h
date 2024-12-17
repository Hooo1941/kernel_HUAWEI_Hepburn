/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : vsp维测头文件
 * 作    者 : wifi
 * 创建日期 : 2022年8月24日
 */

#ifndef HMAC_VSP_LOG_H
#define HMAC_VSP_LOG_H

#include "mac_vap.h"

#ifdef _PRE_WLAN_FEATURE_VSP
/* VSP维测等级, lvl0 -> lvl2维测等级逐步提升, 日志打印范围变广 */
typedef enum {
    VSP_LOG_LVL_DISABLE,
    VSP_LOG_LVL_0,
    VSP_LOG_LVL_1,
    VSP_LOG_LVL_2,
} hmac_vsp_log_lvl_enum;

/* VSP维测打印数据类型 */
typedef enum {
    VSP_LOG_DATATYPE_SLICE,
    VSP_LOG_DATATYPE_LAYER,
    VSP_LOG_DATATYPE_PKT,
    VSP_LOG_DATATYPE_FEEDBACK,
    VSP_LOG_DATATYPE_RX_SLICE,
    VSP_LOG_DATATYPE_RX_SLICE_PRIV,
    VSP_LOG_DATATYPE_TIMER,
    VSP_LOG_DATATYPE_PARAM,
    VSP_LOG_DATATYPE_CMD,
    VSP_LOG_DATATYPE_BUTT,
} hmac_vsp_log_datatype_enum;

/* VSP维测类型 */
typedef enum {
    VSP_LOG_TYPE_PARSE_CMD,
    VSP_LOG_TYPE_CFG_PARAM,
    VSP_LOG_TYPE_START_TIMER,
    VSP_LOG_TYPE_BUILD_SLICE,
    VSP_LOG_TYPE_SEND_PKT,
    VSP_LOG_TYPE_BUILD_RX_SLICE,
    VSP_LOG_TYPE_INIT_RX_TIMER,
    VSP_LOG_TYPE_FILL_RX_SLICE,
    VSP_LOG_TYPE_RX_EXPIRED_PKT,
    VSP_LOG_TYPE_RX_NONCURRENT_PKT,
    VSP_LOG_TYPE_RX_FEEDBACK,
    VSP_LOG_TYPE_SEND_FEEDBACK,
    VSP_LOG_TYPE_RPT_SLICE,
    VSP_LOG_TYPE_BUTT,
} hmac_vsp_log_type_enum;

void hmac_vsp_log(uint8_t *data, uint8_t lvl, uint8_t datatype, uint8_t type);
#endif
#endif
