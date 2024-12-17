/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 多通道退化白名单探测函数定义
 * 作    者 : wifi
 * 创建日期 : 2020年11月23日
 */

#ifndef HMAC_DEGRADATION_H
#define HMAC_DEGRADATION_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DEGRADATION_H

/* 2 宏定义 */

/* 3 函数申明 */
void hmac_degradation_tx_chain_switch(mac_vap_stru *mac_vap, uint8_t degradation_enable);
#endif /* end of HMAC_MCM_DEGRADATION_H */

