/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_flowctl.c 的头文件
 * 作    者 :
 * 创建日期 : 2012年12月10日
 */

#ifndef WAL_LINUX_IOCTL_DEBUG_H
#define WAL_LINUX_IOCTL_DEBUG_H
#ifdef _PRE_WLAN_CFGID_DEBUG
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif

#include "securec.h"
#include "oal_util.h"
#include "oal_hcc_host_if.h"
#include "board.h"
#include "plat_pm_wlan.h"
#include "plat_cali.h"

#include "oam_ext_if.h"
#include "oam_event_wifi.h"
#include "frw_ext_if.h"

#include "mac_vap.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "hmac_chan_mgmt.h"
#include "hmac_p2p.h"
#include "hmac_roam_main.h"
#include "hmac_twt.h"
#ifdef _PRE_WLAN_FEATURE_VSP
#include "hmac_vsp_if.h"
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
#include "hmac_hid2d.h"
#endif

#include "wal_main.h"
#include "wal_config.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_ioctl_macro.h"
#include "wal_linux_cfg80211.h"
#include "wal_linux_netdev_ops.h"
#include "wal_cfg_ioctl.h"
#include "wal_cfg_feature_ioctl.h"
#include "wal_cfg_trx_ioctl.h"
#include "wal_cfg_dscr_ioctl.h"
#include "wal_cfg_vap_ioctl.h"
#include "wal_cfg_stat_ioctl.h"
#include "wal_cfg_pm_ioctl.h"
#include "wal_dfx.h"
#ifdef _PRE_WLAN_CHBA_MGMT
#include "wal_hipriv_chba.h"
#endif
#ifdef _PRE_WLAN_EXPORT
#include "wal_linux_atcmdsrv.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

uint32_t wal_hipriv_getcountry(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_user_info(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_reg_info(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef PLATFORM_DEBUG_ENABLE
uint32_t wal_hipriv_reg_write(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
#ifdef _PRE_WLAN_EXPORT
uint32_t wal_hipriv_mac_addr_write(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_mac_addr_read(oal_net_device_stru *net_dev, int8_t *param);
#endif
uint32_t wal_hipriv_add_vap(oal_net_device_stru *cfg_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_always_tx(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_ru_index(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_always_rx(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_SNIFFER
uint32_t wal_hipriv_set_sniffer(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_enable_monitor_mode(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_freq(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_mode(oal_net_device_stru *net_dev, int8_t *param);
uint32_t wal_hipriv_set_band(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_bw(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_rx_fcs_info(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_add_user(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_del_user(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_sdio_flowctrl(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_DELAY_STATISTIC
uint32_t wal_hipriv_pkt_time_switch(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif

#ifdef _PRE_WLAN_FEATURE_11K
uint32_t wal_hipriv_send_neighbor_req(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_memory_monitor(oal_net_device_stru *net_dev, int8_t *pc_param);

#ifdef _PRE_WLAN_DELAY_STATISTIC
uint32_t wal_hipriv_pkt_time_switch(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif

uint32_t wal_hipriv_vap_log_level(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
uint32_t wal_hipriv_tcp_ack_buf_cfg(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
#ifdef _PRE_WLAN_FEATURE_APF
uint32_t wal_hipriv_apf_filter_list(oal_net_device_stru *net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_cali_debug(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_dpd_cfg(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_start_chan_meas(oal_net_device_stru *net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_pt_mcast(oal_net_device_stru *net_dev, int8_t *pc_param);
#ifdef _PRE_WLAN_FEATURE_FTM
uint32_t _wal_hipriv_ftm(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#endif
uint32_t wal_hipriv_set_dbdc_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_phy_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_enable_2040bss(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_set_extlna_bypass_rssi(oal_net_device_stru *net_device, int8_t *param);
uint32_t wal_hipriv_set_str_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
uint32_t wal_hipriv_sar_slide_cfg(oal_net_device_stru *pst_net_dev, int8_t *pc_param);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

#endif /* end of wal_linux_ioctl.h */
