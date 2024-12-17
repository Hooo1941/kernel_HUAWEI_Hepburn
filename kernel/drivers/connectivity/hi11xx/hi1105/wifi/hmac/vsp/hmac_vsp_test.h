/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : hmac_vsp_test.c 的头文件
 * 作者       : wifi
 * 创建日期   : 2021年09月02日
 */
#ifndef HMAC_VSP_TEST_H
#define HMAC_VSP_TEST_H

#include "hmac_vsp_if.h"

#ifdef _PRE_WLAN_FEATURE_VSP
uint32_t hmac_vsp_test_start(hmac_vsp_debug_cmd *cmd);
uint32_t hmac_vsp_test_stop(void);
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
void hmac_vsp_init_sysfs(void);
void hmac_vsp_deinit_sysfs(void);
#endif
#endif
#endif
