/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2022年11月19日
 */

#ifndef WAL_FEATURE_CFG_IOCTL_H
#define WAL_FEATURE_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_set_bgscan_type(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_random_mac_addr_scan(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_FEATURE_WMMAC
uint32_t wal_set_addts_req(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_delts(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_config_reassoc_req(mac_vap_stru *mac_vap, uint32_t *param);
#endif
uint32_t wal_set_2040_coext_support(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
uint32_t wal_hipriv_phy_event_rpt(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_set_voe_enable(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_remove_app_ie(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_ota_beacon_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_mem_leak(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_list_channel(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_start_deauth(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_probe_switch(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_FEATURE_PMF
uint32_t wal_set_enable_pmf(mac_vap_stru *mac_vap, uint32_t *params);
#endif
#ifdef _PRE_WLAN_FEATURE_DFR
uint32_t wal_test_dfr_start(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_set_bw_fixed(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_send_2040_coext(mac_vap_stru *mac_vap, uint32_t *params);
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
uint32_t wal_set_11v_cfg_bsst(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_set_start_join(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_FEATURE_TWT
uint32_t _wal_hipriv_twt_setup_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t _wal_hipriv_twt_teardown_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_feature_log_switch(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_NARROW_BAND
uint32_t wal_hipriv_narrow_bw(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_EXPORT
uint32_t wal_hipriv_export_para_query(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_export_set_tpc_param(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_nb_info_report(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_autorate_cfg_set(oal_net_device_stru *net_dev, int8_t *param);
#endif
#endif
uint32_t wal_hipriv_addba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_delba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_WMMAC
uint32_t wal_hipriv_wmmac_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif

uint32_t wal_hipriv_btcoex_set_perf_param(oal_net_device_stru *net_dev, int8_t *param);
#ifdef _PRE_WLAN_FEATURE_NRCOEX
uint32_t wal_hipriv_nrcoex_cfg_test(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif

uint32_t wal_hipriv_start_scan(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_start_join(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_kick_user(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_send_bar(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_wmm_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_CSI
uint32_t wal_hipriv_set_csi(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
uint32_t wal_hipriv_get_user_nssbw(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_blacklist_add(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_blacklist_del(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_blacklist_mode(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_WAPI
#ifdef _PRE_WAPI_DEBUG
uint32_t wal_hipriv_show_wpi_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
#endif
uint32_t wal_hipriv_vsp_dbg(oal_net_device_stru *net_device, int8_t *param);
uint32_t wal_hipriv_nan_dbg(oal_net_device_stru *net_device, int8_t *param);
uint32_t wal_hipriv_send_frame(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_roam_start(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_test_add_key(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
uint32_t wal_hipriv_11v_tx_query(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_set_owe(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_HIEX
uint32_t _wal_hipriv_set_user_hiex_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_mintp_test(oal_net_device_stru *net_dev, int8_t *param);
#endif
#endif
