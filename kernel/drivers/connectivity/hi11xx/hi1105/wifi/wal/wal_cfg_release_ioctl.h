/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2020年6月17日
 */

#ifndef WAL_RELEASE_CFG_IOCTL_H
#define WAL_RELEASE_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

extern const int8_t *g_pauc_non_ht_rate_tbl[WLAN_LEGACY_RATE_VALUE_BUTT];
uint32_t wal_set_sta_ps_mode(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_uapsd_cap(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_default_key(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_roam_enable(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_p2p_scenes(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_hipriv_set_dscr_param(oal_net_device_stru *net_dev, int8_t *param, uint8_t func_type);
int32_t wal_ioctl_get_current_tsf(oal_net_device_stru *net_dev, oal_ifreq_stru *ifr,
    wal_wifi_priv_cmd_stru *priv_cmd);
#endif

