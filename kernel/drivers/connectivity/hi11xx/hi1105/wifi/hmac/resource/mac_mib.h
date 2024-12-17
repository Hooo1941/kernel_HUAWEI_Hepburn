/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib.c 的头文件
 * 创建日期 : 2020年7月28日
 */

#ifndef MAC_MIB_H
#define MAC_MIB_H

#include "mac_vap.h"
#include "mac_mib_11i.h"
#include "mac_mib_sta_ht_cfg.h"
#include "mac_mib_sta_vht_cfg.h"
#include "mac_mib_sta_he_cfg.h"
#include "mac_mib_sta_config.h"
#include "mac_mib_edca.h"

/*****************************************************************************
 * mib操作函数
 *****************************************************************************/
uint32_t mac_mib_get_beacon_period(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_dtim_period(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_bss_type(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_get_ssid(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_set_beacon_period(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_dtim_period(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_bss_type(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_ssid(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_set_station_id(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_mib_get_shpreamble(mac_vap_stru *mac_vap, uint8_t *len, uint8_t *param);
uint32_t mac_mib_set_shpreamble(mac_vap_stru *mac_vap, uint8_t len, uint8_t *param);
uint32_t mac_vap_set_bssid(mac_vap_stru *mac_vap, uint8_t *bssid, uint8_t bssid_len);

void mac_vap_init_mib_11ax_mp13(mac_vap_stru *mac_vap, uint32_t nss_num);
void mac_vap_init_mib_11ax(mac_vap_stru *mac_vap);
void mac_vap_init_mib_11ax_mp15(mac_vap_stru *mac_vap, uint32_t nss_num);
void mac_vap_init_mib_11ax_mp16(mac_vap_stru *mac_vap, uint32_t nss_num);

void mac_init_mib_extend(mac_vap_stru *mac_vap);
void mac_vap_init_mib_11n_txbf(mac_vap_stru *mac_vap);
void mac_init_mib(mac_vap_stru *mac_vap);

void mac_vap_change_mib_by_bandwidth(mac_vap_stru *mac_vap,
    wlan_channel_band_enum_uint8 band, wlan_channel_bandwidth_enum_uint8 bandwidth);
void mac_mib_set_wep(mac_vap_stru *mac_vap, uint8_t key_id, uint8_t key_value);
#ifdef _PRE_WLAN_FEATURE_11R
uint32_t mac_mib_init_ft_cfg(mac_vap_stru *mac_vap, uint8_t *mde);
uint32_t mac_mib_get_md_id(mac_vap_stru *mac_vap, uint16_t *mdid);
#endif  // _PRE_WLAN_FEATURE_11R

#if defined(_PRE_WLAN_FEATURE_11R)
static inline oal_bool_enum_uint8 mac_mib_get_ft_trainsistion(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated;
}

static inline void mac_mib_set_ft_trainsistion(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 ft_trainsistion)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = ft_trainsistion;
}

static inline uint16_t mac_mib_get_ft_mdid(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID;
}

static inline void mac_mib_set_ft_mdid(mac_vap_stru *mac_vap, uint16_t ft_mdid)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.us_dot11FTMobilityDomainID = ft_mdid;
}

static inline oal_bool_enum_uint8 mac_mib_get_ft_over_ds(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated;
}

static inline void mac_mib_set_ft_over_ds(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 ft_over_ds)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated = ft_over_ds;
}

static inline oal_bool_enum_uint8 mac_mib_get_ft_resource_req(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported;
}

static inline void mac_mib_set_ft_resource_req(mac_vap_stru *mac_vap, oal_bool_enum_uint8 ft_resource_req)
{
    mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported = ft_resource_req;
}
#endif  // _PRE_WLAN_FEATURE_11R

static inline uint32_t mac_mib_get_OBSSScanPassiveDwell(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveDwell;
}

static inline void mac_mib_set_OBSSScanPassiveDwell(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveDwell = value;
}

static inline uint32_t mac_mib_get_OBSSScanActiveDwell(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveDwell;
}

static inline void mac_mib_set_OBSSScanActiveDwell(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveDwell = value;
}

static inline uint32_t mac_mib_get_BSSWidthTriggerScanInterval(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthTriggerScanInterval;
}

static inline void mac_mib_set_BSSWidthTriggerScanInterval(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthTriggerScanInterval = value;
}

static inline uint32_t mac_mib_get_OBSSScanPassiveTotalPerChannel(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveTotalPerChannel;
}

static inline void mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanPassiveTotalPerChannel = value;
}

static inline uint32_t mac_mib_get_OBSSScanActiveTotalPerChannel(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveTotalPerChannel;
}

static inline void mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActiveTotalPerChannel = value;
}

static inline uint32_t mac_mib_get_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthChannelTransitionDelayFactor;
}

static inline void mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11BSSWidthChannelTransitionDelayFactor = value;
}

static inline uint32_t mac_mib_get_OBSSScanActivityThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActivityThreshold;
}

static inline void mac_mib_set_OBSSScanActivityThreshold(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11OBSSScanActivityThreshold = value;
}

static inline oal_bool_enum_uint8 mac_mib_get_FortyMHzIntolerant(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant;
}

static inline void mac_mib_set_FortyMHzIntolerant(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11FortyMHzIntolerant = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_2040BSSCoexistenceManagementSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport;
}

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
static inline oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated;
}

static inline void mac_mib_set_MgmtOptionBSSTransitionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_MgmtOptionBSSTransitionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented;
}

static inline void mac_mib_set_MgmtOptionBSSTransitionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11MgmtOptionBSSTransitionImplemented = val;
}
#endif  // _PRE_WLAN_FEATURE_11V_ENABLE

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
static inline void mac_mib_set_FtmInitiatorModeActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmInitiatorModeActivated = val;
}
static inline oal_bool_enum_uint8 mac_mib_get_FtmInitiatorModeActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmInitiatorModeActivated;
}
static inline void mac_mib_set_FtmResponderModeActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmResponderModeActivated = val;
}
static inline oal_bool_enum_uint8 mac_mib_get_FtmResponderModeActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11FtmResponderModeActivated;
}
static inline void mac_mib_set_FtmRangeReportActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFtmRangeReportActivated = val;
}
static inline oal_bool_enum_uint8 mac_mib_get_FtmRangeReportActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_wireless_mgmt_op.en_dot11RMFtmRangeReportActivated;
}
#endif

static inline void mac_mib_init_2040(mac_vap_stru *mac_vap)
{
    mac_mib_set_FortyMHzIntolerant(mac_vap, OAL_FALSE);
    mac_mib_set_SpectrumManagementImplemented(mac_vap, OAL_TRUE);
    mac_mib_set_2040BSSCoexistenceManagementSupport(mac_vap, OAL_TRUE);
}

static inline void mac_mib_init_obss_scan(mac_vap_stru *mac_vap)
{
    mac_mib_set_OBSSScanPassiveDwell(mac_vap, 20); /* 20 OBSSScanPassiveDwel ֵ */
    mac_mib_set_OBSSScanActiveDwell(mac_vap, 10); /* 10 OBSSScanActiveDwell ֵ */
    mac_mib_set_BSSWidthTriggerScanInterval(mac_vap, 300); /* 300 BSSWidthTriggerScanInterval ֵ */
    mac_mib_set_OBSSScanPassiveTotalPerChannel(mac_vap, 200); /* 200 OBSSScanPassiveTotalPerChannel ֵ */
    mac_mib_set_OBSSScanActiveTotalPerChannel(mac_vap, 20); /* 20 OBSSScanActiveTotalPerChannel ֵ */
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(mac_vap, 5); /* 5 BSSWidthChannelTransitionDelayFactor ֵ */
    mac_mib_set_OBSSScanActivityThreshold(mac_vap, 25); /* 25 OBSSScanActivityThreshold ֵ */
}

static inline oal_bool_enum_uint8 mac_mib_get_LsigTxopFullProtectionActivated(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated);
}

static inline void mac_mib_set_LsigTxopFullProtectionActivated(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_lsig_txop_full_protection_activated)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11LSIGTXOPFullProtectionActivated =
        en_lsig_txop_full_protection_activated;
}

static inline oal_bool_enum_uint8 mac_mib_get_NonGFEntitiesPresent(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent);
}

static inline void mac_mib_set_NonGFEntitiesPresent(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_non_gf_entities_present)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11NonGFEntitiesPresent = en_non_gf_entities_present;
}

static inline oal_bool_enum_uint8 mac_mib_get_RifsMode(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode);
}

static inline void mac_mib_set_RifsMode(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 en_rifs_mode)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11RIFSMode = en_rifs_mode;
}

static inline wlan_mib_ht_protection_enum mac_mib_get_HtProtection(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection);
}

static inline void mac_mib_set_DualCTSProtection(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_DualCTSProtection(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection);
}

static inline void mac_mib_set_PCOActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_PCOActivated(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated);
}

static inline void mac_mib_set_HtProtection(mac_vap_stru *mac_vap,
    wlan_mib_ht_protection_enum en_ht_protection)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot11HTProtection = en_ht_protection;
}

static inline void mac_mib_set_FragmentationThreshold(mac_vap_stru *mac_vap, uint32_t frag_threshold)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11FragmentationThreshold = frag_threshold;
}

static inline uint32_t mac_mib_get_FragmentationThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11FragmentationThreshold;
}

static inline void mac_mib_set_RTSThreshold(mac_vap_stru *mac_vap, uint32_t rts_threshold)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.dot11RTSThreshold = rts_threshold;
}

static inline uint32_t mac_mib_get_RTSThreshold(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_operation.dot11RTSThreshold;
}

static inline void mac_mib_set_AntennaSelectionOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_AntennaSelectionOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented;
}

static inline void mac_mib_set_TransmitExplicitCSIFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TransmitExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented;
}

static inline void mac_mib_set_TransmitIndicesFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TransmitIndicesFeedbackASOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented;
}

static inline void mac_mib_set_ExplicitCSIFeedbackASOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitCSIFeedbackASOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented;
}

static inline void mac_mib_set_ReceiveAntennaSelectionOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ReceiveAntennaSelectionOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented;
}

static inline void mac_mib_set_TransmitSoundingPPDUOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TransmitSoundingPPDUOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented;
}

static inline wlan_11b_mib_preamble_enum_uint8 mac_mib_get_ShortPreambleOptionImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented);
}

static inline void mac_mib_set_ShortPreambleOptionImplemented(
    mac_vap_stru *mac_vap, wlan_11b_mib_preamble_enum_uint8 en_preamble)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ShortPreambleOptionImplemented = en_preamble;
}

static inline void mac_mib_set_PBCCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_PBCCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11PBCCOptionImplemented;
}

static inline void mac_mib_set_ChannelAgilityPresent(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ChannelAgilityPresent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_hrdsss.en_dot11ChannelAgilityPresent;
}

static inline void mac_mib_set_DSSSOFDMOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_DSSSOFDMOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11DSSSOFDMOptionActivated;
}

static inline void mac_mib_set_ShortSlotTimeOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionImplemented;
}

static inline void mac_mib_set_ShortSlotTimeOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ShortSlotTimeOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_dsss.st_phy_erp.en_dot11ShortSlotTimeOptionActivated;
}

static inline void mac_mib_set_SupportedMCSTxValue(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[idx] = value;
}

static inline uint8_t mac_mib_get_SupportedMCSTxValue(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue[idx];
}

static inline uint8_t *mac_mib_get_SupportedMCSTx(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcstx.auc_dot11SupportedMCSTxValue;
}

static inline void mac_mib_set_SupportedMCSRxValue(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[idx] = value;
}

static inline uint8_t mac_mib_get_SupportedMCSRxValue(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue[idx];
}

static inline uint8_t *mac_mib_get_SupportedMCSRx(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_supported_mcs.st_supported_mcsrx.auc_dot11SupportedMCSRxValue;
}

#endif
