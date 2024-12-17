/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2022年11月19日
 */

#ifndef WAL_PM_CFG_IOCTL_H
#define WAL_PM_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_set_uapsd_para(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_pm_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_hipriv_set_pm_debug_switch(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_set_close_fem_cali_status(mac_vap_stru *mac_vap, uint32_t *params);
#endif

#endif
