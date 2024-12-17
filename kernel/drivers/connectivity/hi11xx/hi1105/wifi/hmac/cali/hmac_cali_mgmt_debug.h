/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_cali_mgmt_debug.c 的头文件
 * 作    者 : wifi1
 * 创建日期 : 2020年11月27日
 */

#ifndef HMAC_CALI_MGMT_DEBUG_H
#define HMAC_CALI_MGMT_DEBUG_H

/* 1 其他头文件包含 */
#include "frw_ext_if.h"

#include "hmac_vap.h"
#include "plat_cali.h"
#include "wlan_spec.h"
#include "wlan_cali.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_DEBUG_H

void hmac_dump_cali_result_mp16(void);
void hmac_dump_cali_result(void);
void hmac_dump_cali_result_mp15(void);

#endif
