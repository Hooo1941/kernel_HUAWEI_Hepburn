/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 :
 * 创建日期 : 2022年11月19日
 */

#ifndef WAL_TRX_CFG_IOCTL_H
#define WAL_TRX_CFG_IOCTL_H
#include "mac_vap.h"
#include "oal_ext_if.h"

#ifdef WIFI_DEBUG_ENABLE

uint32_t wal_set_stbc_cap(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_ldpc_cap(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_txbf_cap(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_amsdu_tx_on(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_flowctl_param(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
uint32_t wal_set_pk_mode_debug(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_set_frag_threshold(mac_vap_stru *mac_vap, uint32_t *params);

#ifdef _PRE_WLAN_FEATURE_TXOPPS
uint32_t wal_set_txop_ps_machw(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_set_all_ether_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_dhcp_arp_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_80211_mcast_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_ota_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_global_log_switch(mac_vap_stru *mac_vap, uint32_t *params);
#ifdef _PRE_WLAN_TCP_OPT
uint32_t wal_get_tcp_ack_stream_info(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_tcp_tx_ack_opt_enable(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_tcp_rx_ack_opt_enable(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_tcp_tx_ack_limit(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t wal_set_tcp_rx_ack_limit(mac_vap_stru *mac_vap, uint32_t *params);
#endif
uint32_t wal_hipriv_ota_rx_dscr_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif

#endif
