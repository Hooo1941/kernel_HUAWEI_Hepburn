/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib vht配置相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_STA_VHT_CFG_H
#define MAC_MIB_STA_VHT_CFG_H

static inline uint32_t mac_mib_get_maxmpdu_length(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11MaxMPDULength;
}

static inline void mac_mib_set_maxmpdu_length(mac_vap_stru *mac_vap, uint32_t maxmpdu_length)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11MaxMPDULength = maxmpdu_length;
}

static inline uint32_t mac_mib_get_vht_max_rx_ampdu_factor(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTMaxRxAMPDUFactor;
}

static inline void mac_mib_set_vht_max_rx_ampdu_factor(mac_vap_stru *mac_vap,
    uint32_t vht_max_rx_ampdu_factor)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTMaxRxAMPDUFactor = vht_max_rx_ampdu_factor;
}

static inline oal_bool_enum_uint8 mac_mib_get_vht_ctrl_field_cap(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported;
}

static inline void mac_mib_set_vht_ctrl_field_cap(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 vht_ctrl_field_supported)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported =
        vht_ctrl_field_supported;
}

static inline oal_bool_enum_uint8 mac_mib_get_txopps(mac_vap_stru *pst_vap)
{
    return pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented;
}

static inline void mac_mib_set_txopps(mac_vap_stru *pst_vap, oal_bool_enum_uint8 vht_txop_ps)
{
    pst_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented = vht_txop_ps;
}

static inline uint16_t mac_mib_get_vht_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap;
}

static inline void *mac_mib_get_ptr_vht_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap);
}

static inline void mac_mib_set_vht_rx_mcs_map(mac_vap_stru *mac_vap, uint16_t vht_mcs_mpa)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap = vht_mcs_mpa;
}

static inline uint32_t mac_mib_get_us_rx_highest_rate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTRxHighestDataRateSupported;
}

static inline void mac_mib_set_us_rx_highest_rate(mac_vap_stru *mac_vap, uint32_t rx_highest_rate)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTRxHighestDataRateSupported = rx_highest_rate;
}

static inline uint16_t mac_mib_get_vht_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap;
}

static inline void *mac_mib_get_ptr_vht_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap);
}

static inline void mac_mib_set_vht_tx_mcs_map(mac_vap_stru *mac_vap, uint16_t vht_mcs_mpa)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap = vht_mcs_mpa;
}

static inline uint32_t mac_mib_get_us_tx_highest_rate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTTxHighestDataRateSupported;
}

static inline void mac_mib_set_us_tx_highest_rate(mac_vap_stru *mac_vap, uint32_t tx_highest_rate)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.dot11VHTTxHighestDataRateSupported = tx_highest_rate;
}

static inline void mac_mib_set_VHTOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTOptionImplemented;
}

#if defined _PRE_WLAN_FEATURE_OPMODE_NOTIFY || (_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)
static inline oal_bool_enum_uint8 mac_mib_get_OperatingModeNotificationImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented;
}

static inline void mac_mib_set_OperatingModeNotificationImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11OperatingModeNotificationImplemented = val;
}
#endif

static inline void mac_mib_set_dot11VHTExtendedNSSBWCapable(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    ((mac_vap_stru *)mac_vap)->en_dot11VHTExtendedNSSBWCapable = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11VHTExtendedNSSBWCapable(mac_vap_stru *mac_vap)
{
    return ((mac_vap_stru *)mac_vap)->en_dot11VHTExtendedNSSBWCapable;
}

static inline void mac_mib_set_VHTSUBeamformeeOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformeeOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented;
}

static inline void mac_mib_set_VHTSUBeamformerOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTSUBeamformerOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented;
}

static inline void mac_mib_set_VHTMUBeamformeeOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformeeOptionImplemented(mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented;
}

static inline void mac_mib_set_VHTMUBeamformerOptionImplemented(mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTMUBeamformerOptionImplemented(
    mac_vap_stru *vap)
{
    return vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.\
        en_dot11VHTMUBeamformerOptionImplemented;
}

static inline void mac_mib_set_VHTNumberSoundingDimensions(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTNumberSoundingDimensions(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTNumberSoundingDimensions;
}

static inline void mac_mib_set_VHTBeamformeeNTxSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_VHTBeamformeeNTxSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.st_wlan_mib_vht_txbf_config.uc_dot11VHTBeamformeeNTxSupport;
}

static inline void mac_mib_set_VHTChannelWidthOptionImplemented(
    mac_vap_stru *mac_vap, wlan_mib_vht_supp_width_enum val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented = val;
}

static inline uint8_t mac_mib_get_VHTChannelWidthOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTChannelWidthOptionImplemented;
}

static inline void mac_mib_set_VHTShortGIOptionIn80Implemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented = val;
}

static inline uint8_t mac_mib_get_VHTShortGIOptionIn80Implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented;
}

static inline void mac_mib_set_VHTShortGIOptionIn160and80p80Implemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented = val;
}

static inline uint8_t mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented;
}

static inline void mac_mib_set_VHTLDPCCodingOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented = val;
}

static inline uint8_t mac_mib_get_VHTLDPCCodingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented;
}

static inline void mac_mib_set_VHTTxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented = val;
}

static inline uint8_t mac_mib_get_VHTTxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented;
}

static inline void mac_mib_set_VHTRxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented = val;
}

static inline uint8_t mac_mib_get_VHTRxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented;
}
#endif
