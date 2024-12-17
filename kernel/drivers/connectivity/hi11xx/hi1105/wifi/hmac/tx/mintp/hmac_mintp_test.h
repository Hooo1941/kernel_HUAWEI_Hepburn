/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MinTP测试
 * 作    者 : wifi
 * 创建日期 : 2023年1月7日
 */

#ifndef HMAC_MINTP_TEST_H
#define HMAC_MINTP_TEST_H

#include "hmac_mintp.h"
#include "hmac_vap.h"

enum {
    HMAC_MINTP_TEST_CFG,
    HMAC_MINTP_TEST_START,
    HMAC_MINTP_TEST_STOP,
};

typedef struct {
    uint8_t cmd;
    uint16_t loop_cnt;
    uint16_t netbuf_cnt;
    uint8_t user_mac[WLAN_MAC_ADDR_LEN];
} hmac_mintp_test_param_stru;

uint32_t hmac_config_mintp_test(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
oal_netbuf_stru *hmac_mintp_test_build_netbuf_list(void);

#endif
