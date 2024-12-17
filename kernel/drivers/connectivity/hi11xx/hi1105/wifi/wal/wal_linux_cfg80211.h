/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_cfg80211.c 的头文件
 * 作    者 :
 * 创建日期 : 2012年11月8日
 */

#ifndef WAL_LINUX_CFG80211_H
#define WAL_LINUX_CFG80211_H

#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_scan.h"
#include "hmac_mgmt_join.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_H

#define WAL_MAX_SCAN_TIME_PER_CHANNEL 400

#define WAL_MAX_SCAN_TIME_PER_SCAN_REQ (5 * 1000) /* wpa_s下发扫描请求，超时时间为5s，单位为ms */

/* channel index and frequence */
#define WAL_MIN_CHANNEL_6G 1
#define WAL_MAX_CHANNEL_6G 233

#define WAL_MIN_CHANNEL_4_9G 184
#define WAL_MAX_CHANNEL_4_9G 196

#define WAL_MIN_CHANNEL_5G 36
#define WAL_MAX_CHANNEL_5G 165

#define WAL_MIN_FREQ_2G (2412 + 5 * (WAL_MIN_CHANNEL_2G - 1))
#define WAL_MAX_FREQ_2G (2484)
#define WAL_MIN_FREQ_5G (5000 + 5 * (WAL_MIN_CHANNEL_5G))
#define WAL_MAX_FREQ_5G (5000 + 5 * (WAL_MAX_CHANNEL_5G))
#define WAL_MIN_FREQ_6G (5950 + 5 * WAL_MIN_CHANNEL_6G)
#define WAL_MAX_FREQ_6G (5950 + 5 * WAL_MAX_CHANNEL_6G)

/* wiphy 结构体初始化变量 */
#define WAL_MAX_SCAN_IE_LEN 1000

/* management */
#define IEEE80211_STYPE_ASSOC_REQ    0x0000
#define IEEE80211_STYPE_ASSOC_RESP   0x0010
#define IEEE80211_STYPE_REASSOC_REQ  0x0020
#define IEEE80211_STYPE_REASSOC_RESP 0x0030
#define IEEE80211_STYPE_PROBE_REQ    0x0040
#define IEEE80211_STYPE_PROBE_RESP   0x0050
#define IEEE80211_STYPE_BEACON       0x0080
#define IEEE80211_STYPE_ATIM         0x0090
#define IEEE80211_STYPE_DISASSOC     0x00A0
#define IEEE80211_STYPE_AUTH         0x00B0
#define IEEE80211_STYPE_DEAUTH       0x00C0
#define IEEE80211_STYPE_ACTION       0x00D0

#define WAL_COOKIE_ARRAY_SIZE    8    /* 采用8bit 的map 作为保存cookie 的索引状态 */
#define WAL_COOKIE_FULL_MASK     0xFF /* cookie全部用尽掩码 */
#define WAL_MGMT_TX_TIMEOUT_MSEC 500  /*  WAL 发送管理帧超时时间, 考虑到发送过程可能依赖于节能缓存机制，
                                         需要将等待时间延长至几倍于TBTT，
                                         实际上，wpa_supplicant层发送未关联时的管理帧时，超时时间确实是1000ms，
                                         比如Device Discoverability Request帧 */
#define WAL_MGMT_TX_RETRY_CNT 8      /* WAL 发送管理帧最大重传次数 */

#define IEEE80211_FCTL_FTYPE 0x000c
#define IEEE80211_FCTL_STYPE 0x00f0
#define IEEE80211_FTYPE_MGMT 0x0000

#define WAL_GET_STATION_THRESHOLD        1000 /* 固定时间内允许一次抛事件读DMAC RSSI */
#define WAL_VOWIFI_GET_STATION_THRESHOLD 200  /* 亮屏且vowifi正在使用时 */
#define WAL_CAST_SCREEN_GET_STATION_THRESHOLD 100  /* 投屏场景上报周期 */

#define MAX_ADDR_INDEX_0 0
#define MAX_ADDR_INDEX_1 1
#define MAX_ADDR_INDEX_2 2
#define MAX_ADDR_INDEX_3 3
#define MAX_ADDR_INDEX_4 4
#define MAX_ADDR_INDEX_5 5

typedef struct cookie_arry {
    uint64_t ull_cookie;
    uint32_t record_time;
} cookie_arry_stru;

#define WAL_MIN_RTS_THRESHOLD 256
#define WAL_MAX_RTS_THRESHOLD 0xFFFF

#define WAL_MAX_FRAG_THRESHOLD 7536
#define WAL_MIN_FRAG_THRESHOLD 256

#define WAL_MAX_WAIT_TIME 3000

/* 此处02加载ko时出现，找不到符号的错误 */
/*
 * 函 数 名  : oal_ieee80211_is_probe_resp
 * 修改历史
 * 1.日    期   : 2015年1月7日
 *   修改内容   : 新生成函数
 * 2.日    期   : 2015年1月7日
 *   修改内容   : 判断是否是probe response
 *   check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_PROBE_RESP
 *   @fc: frame control bytes in little-endian byteorder
 */
OAL_STATIC OAL_INLINE uint32_t oal_ieee80211_is_probe_resp(uint16_t fc)
{
    return (fc & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
}

/* 功能描述: 判断是否是action帧 */
OAL_STATIC OAL_INLINE uint32_t wal_cfg80211_tx_buff_is_action(uint16_t fc)
{
    return (fc & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
}

static inline oal_bool_enum_uint8 wlan_vap_mode_legacy_vap(wlan_p2p_mode_enum_uint8 p2p_mode,
    wlan_vap_mode_enum_uint8 vap_mode)
{
    return ((p2p_mode == WLAN_LEGACY_VAP_MODE) && (vap_mode == WLAN_VAP_MODE_BSS_STA));
}
void wal_cfg80211_exit(void);

uint32_t wal_cfg80211_init(void);

uint32_t wal_cfg80211_init_evt_handle(frw_event_mem_stru *pst_event_mem);
uint32_t wal_cfg80211_mgmt_tx_status(frw_event_mem_stru *pst_event_mem);

uint32_t wal_cfg80211_vowifi_report(frw_event_mem_stru *pst_event_mem);

uint32_t wal_cfg80211_cac_report(frw_event_mem_stru *pst_event_mem);

uint32_t wal_roam_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_11R
uint32_t wal_ft_event_proc_sta(frw_event_mem_stru *pst_event_mem);
#endif  // _PRE_WLAN_FEATURE_11R

void wal_cfg80211_unregister_netdev(oal_net_device_stru *pst_net_dev);

uint32_t wal_del_p2p_group(mac_device_stru *pst_mac_device);
void wal_cfg80211_reset_bands(uint8_t uc_dev_id);
void wal_cfg80211_save_bands(uint8_t uc_dev_id);
#ifdef _PRE_WLAN_FEATURE_M2S
uint32_t wal_cfg80211_m2s_status_report(frw_event_mem_stru *pst_event_mem);
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
uint32_t wal_cfg80211_tas_rssi_access_report(frw_event_mem_stru *pst_event_mem);
#endif
uint8_t wal_cfg80211_get_station_filter(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac);
int32_t wal_del_vap_try_wlan_pm_close(void);
#ifdef _PRE_WLAN_CHBA_MGMT
int32_t wal_cfg80211_cfg_chba(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev, mac_vap_stru *mac_vap);
int32_t wal_cfg80211_change_intf_chba0_proc(oal_net_device_stru *net_dev, enum nl80211_iftype if_type);
#endif
void wal_cfg80211_start_ap_set_chba_mode(oal_net_device_stru *netdev, mac_vap_stru *mac_vap);
int32_t wal_cfg80211_connect(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device,
    oal_cfg80211_conn_stru *pst_sme);
int32_t wal_cfg80211_disconnect(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, uint16_t us_reason_code);
int32_t wal_cfg80211_fill_beacon_param(oal_net_device_stru *pst_netdev,
    oal_beacon_data_stru *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param);
int32_t wal_cfg80211_change_beacon(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_beacon_data_stru *pst_beacon_info);
int32_t wal_cfg80211_change_bss(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    oal_bss_parameters *pst_bss_params);
int32_t wal_cfg80211_sched_scan_start(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev, oal_cfg80211_sched_scan_request_stru *pst_request);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev, uint64_t reqid);

#else
int32_t wal_cfg80211_sched_scan_stop(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_netdev);
#endif
int32_t wal_cfg80211_add_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
    const uint8_t *puc_mac, oal_station_parameters_stru *pst_sta_parms);
int32_t wal_cfg80211_del_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    struct station_del_parameters *params
#else
    uint8_t *puc_mac
#endif
    );

int32_t wal_cfg80211_change_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev, const uint8_t *puc_mac,
    oal_station_parameters_stru *pst_sta_parms);
int32_t wal_cfg80211_get_station(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_dev, const uint8_t *puc_mac, oal_station_info_stru *pst_sta_info);
int32_t wal_cfg80211_dump_station(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_dev,
    int32_t int_index, uint8_t *puc_mac, oal_station_info_stru *pst_sta_info);
uint32_t wal_parse_ht_vht_ie(oal_net_device_stru *pst_netdev,
    mac_vap_stru *pst_mac_vap, oal_beacon_parameters *pst_beacon_info, mac_beacon_param_stru *pst_beacon_param);
oal_bool_enum_uint8 wal_cfg80211_dfr_and_s3s4_param_check(void);
int32_t wal_cfg80211_scan(oal_wiphy_stru *pst_wiphy, oal_cfg80211_scan_request_stru *pst_request);
int32_t wal_cfg80211_set_wiphy_params(oal_wiphy_stru *pst_wiphy, uint32_t changed);
int32_t wal_cfg80211_set_channel_info(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev);
void wal_another_vap_stop_p2p_listen(mac_device_stru *pst_mac_device, uint8_t uc_self_vap_id);
int32_t wal_cfg80211_start_ap(oal_wiphy_stru *wiphy, oal_net_device_stru *netdev, oal_ap_settings_stru *ap_settings);
int32_t wal_cfg80211_stop_ap(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, oal_vif_params_stru *params);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    unsigned char name_assign_type, enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params);
#else
oal_wireless_dev_stru *wal_cfg80211_add_virtual_intf(oal_wiphy_stru *wiphy, const char *name,
    enum nl80211_iftype type, uint32_t *pul_flags, oal_vif_params_stru *params);
#endif
int32_t wal_cfg80211_del_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_wireless_dev_stru *pst_wdev);
int32_t wal_drv_remain_on_channel(oal_wireless_dev_stru *wdev, struct ieee80211_channel *chan, uint32_t duration,
    uint64_t *cookie, wlan_ieee80211_roc_type_uint8 roc_type);
int32_t wal_cfg80211_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, struct ieee80211_channel *pst_chan, uint32_t duration, uint64_t *pull_cookie);
int32_t wal_cfg80211_cancel_remain_on_channel(oal_wiphy_stru *pst_wiphy,
    oal_wireless_dev_stru *pst_wdev, uint64_t ull_cookie);
void wal_set_crypto_info(hmac_conn_param_stru *pst_conn_param, oal_cfg80211_conn_stru *pst_sme);
int32_t wal_cfg80211_add_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev, uint8_t uc_key_index,
    bool en_pairwise, const uint8_t *puc_mac_addr, oal_key_params_stru *pst_params);
int32_t wal_cfg80211_get_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr,
    void *cookie, void (*callback)(void *cookie, oal_key_params_stru *));
int32_t wal_cfg80211_remove_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_pairwise, const uint8_t *puc_mac_addr);
int32_t wal_cfg80211_set_default_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index, bool en_unicast, bool en_multicast);
int32_t wal_cfg80211_set_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa);
int32_t wal_cfg80211_del_pmksa(oal_wiphy_stru *pst_wiphy,
    oal_net_device_stru *pst_net_device, oal_cfg80211_pmksa_stru *pmksa);
int32_t wal_cfg80211_flush_pmksa(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_device);
uint8_t wal_find_oldest_cookie(cookie_arry_stru *pst_cookie_array);
int32_t wal_cfg80211_set_default_mgmt_key(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_netdev,
    uint8_t uc_key_index);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_dev,
    enum nl80211_iftype en_type, oal_vif_params_stru *pst_params);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy, oal_net_device_stru *pst_net_dev,
    enum nl80211_iftype en_type, uint32_t *pul_flags, oal_vif_params_stru *pst_params);
#else
int32_t wal_cfg80211_change_virtual_intf(oal_wiphy_stru *pst_wiphy,
    int32_t l_ifindex, enum nl80211_iftype en_type, uint32_t *pul_flags, oal_vif_params_stru *pst_params);
#endif
int32_t wal_p2p_stop_roc(mac_vap_stru *pst_mac_vap, oal_net_device_stru *pst_netdev);
void wal_cfg80211_set_scan_random_mac(hmac_device_stru *hmac_device,
    uint32_t flags, uint8_t *mac_addr, uint8_t *mac_addr_mask);
void wal_unregister_netdev_and_free_wdev(oal_net_device_stru *netdev);
void wal_set_locally_generated(uint8_t vap_id, uint8_t value);
uint8_t wal_get_locally_generated(uint8_t vap_id);
#endif /* end of wal_linux_cfg80211.h */
