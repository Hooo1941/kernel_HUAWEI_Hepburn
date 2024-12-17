/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2020年5月20日
 */

#ifndef HISI_CUSTOMIZE_WIFI_GF61_H
#define HISI_CUSTOMIZE_WIFI_GF61_H

/* 头文件包含 */
#include "hisi_customize_wifi.h"
#include "wlan_customize.h"
#include "hisi_customize_wifi_cap.h"
#include "hisi_customize_wifi_pow.h"
#include "hisi_customize_wifi_dyn_pow.h"
#include "hisi_conn_nve_interface.h"
#include "hisi_conn_nve_interface_gf61.h"


#ifdef HISI_CONN_NVE_SUPPORT
typedef struct {
    int8_t *ini_data_addr;                  /* addr in ini */
    wlan_cali_pow_para_stru_gf61 *nv_data_addr;  /* addr in nv */
    uint8_t ini_data_num;                   /* data num */
} wlan_cust_dpn_cfg_cmd_gf61;
#endif

void wal_send_cali_data_gf61(oal_net_device_stru *cfg_net_dev);
uint32_t hwifi_custom_host_read_cfg_init_gf61(void);
#endif
