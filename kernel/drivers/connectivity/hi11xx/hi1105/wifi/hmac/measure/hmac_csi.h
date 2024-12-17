/*
* Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
* 功能说明   : CSI功能
* 作者       : wifi
* 创建日期   : 2020年11月17日
*/
#ifndef HMAC_CSI_H
#define HMAC_CSI_H
#include "oal_kernel_file.h"
#include "hmac_user.h"
#include "hmac_vap.h"
#include "host_hal_device.h"

#define FILE_LEN 128
#define BASE_YEAR 1900

#define CSI_HEDA_LEN 64

#ifdef _PRE_WLAN_FEATURE_CSI

uint32_t hmac_host_csi_config(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);
void hmac_proc_location_print_timestamp(oal_file *file);
uint32_t hmac_rx_location_data_event(frw_event_mem_stru *event_mem);
#endif
#endif

