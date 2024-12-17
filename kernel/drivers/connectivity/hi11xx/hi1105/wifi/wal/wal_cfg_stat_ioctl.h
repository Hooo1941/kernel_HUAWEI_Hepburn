/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2022年11月19日
 */

#ifndef WAL_STAT_CFG_IOCTL_H
#define WAL_STAT_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_set_mem_info(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_show_stat_info(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_clear_stat_info(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_user_stat_info(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_event_queue_info(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_DFT_STAT
uint32_t wal_set_clear_vap_stat_info(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_get_flowctl_stat(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_list_sta(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_blacklist_show(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_all_ota(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_show_cali_data_info(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
uint32_t wal_set_show_rr_time_info(mac_vap_stru *mac_vap, uint32_t *params);
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
uint32_t wal_set_dev_customize_info(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_hipriv_set_ether_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
void wal_get_ucast_sub_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch, int8_t *param, uint32_t len);
uint32_t wal_get_ucast_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch, int8_t *param, uint32_t len);
uint32_t wal_hipriv_set_80211_ucast_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_all_80211_ucast(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_DFT_STAT
uint32_t wal_hipriv_usr_queue_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_report_all_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_get_station_info(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_get_tid_queue_info(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_psm_flt_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_memory_info(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_get_version(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_vap_log_level(oal_net_device_stru *net_dev, int8_t *pc_param);

#endif

#endif
