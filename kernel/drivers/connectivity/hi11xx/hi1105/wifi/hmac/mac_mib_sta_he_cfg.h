/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib he相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_STA_HE_CFG_H
#define MAC_MIB_STA_HE_CFG_H

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
static inline void mac_mib_set_HEOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HEOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11HEOptionImplemented;
}

static inline void mac_mib_set_OMIOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OMIOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_OMIOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OMIOptionImplemented;
}

static inline void mac_mib_set_he_TWTOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_TWTOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TWTOptionActivated;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_OperatingModeIndication(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication;
}

static inline void mac_mib_set_he_OperatingModeIndication(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11OperatingModeIndication = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_BSRSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport;
}

static inline void mac_mib_set_he_BSRSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BSRSupport = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_BQRSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BQRSupport;
}

static inline void mac_mib_set_he_BQRSupport(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11BQRSupport = val;
}

static inline void mac_mib_set_he_OFDMARandomAccess(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_OFDMARandomAccess(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11TOFDMARandomAccess;
}

static inline void mac_mib_set_he_TriggerMacPaddingDuration(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration = value;
}

static inline uint8_t mac_mib_get_he_TriggerMacPaddingDuration(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11TriggerMacPaddingDuration;
}

static inline void mac_mib_set_he_MultiBSSIDImplemented(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented = value;
}

static inline uint8_t mac_mib_get_he_MultiBSSIDImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.en_dot11MultiBSSIDImplemented;
}

static inline void mac_mib_set_he_MaxAMPDULengthExponent(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULengthExponent = value;
}

static inline uint8_t mac_mib_get_he_MaxAMPDULengthExponent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.uc_dot11MaxAMPDULengthExponent;
}

static inline void mac_mib_set_he_rx_mcs_map(mac_vap_stru *mac_vap, uint32_t he_mcs_map)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap = he_mcs_map;
}

static inline uint32_t mac_mib_get_he_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap;
}

static inline void *mac_mib_get_ptr_he_rx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HERxMCSMap);
}

static inline void mac_mib_set_he_tx_mcs_map(mac_vap_stru *mac_vap, uint32_t he_mcs_map)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap = he_mcs_map;
}

static inline uint32_t mac_mib_get_he_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap;
}

static inline void *mac_mib_get_ptr_he_tx_mcs_map(mac_vap_stru *mac_vap)
{
    return &(mac_vap->pst_mib_info->st_wlan_mib_he_sta_config.dot11HETxMCSMap);
}

#ifdef _PRE_WLAN_FEATURE_11AX
static inline void mac_mib_set_he_BeamformeeSTSBelow80Mhz(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSBelow80Mhz = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_BeamformeeSTSBelow80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSBelow80Mhz;
}

static inline void mac_mib_set_he_BeamformeeSTSOver80Mhz(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSOver80Mhz = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_BeamformeeSTSOver80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HEBeamformeeSTSOver80Mhz;
}

static inline void mac_mib_set_HENumberSoundingDimensionsBelow80Mhz(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsBelow80Mhz = val;
}
static inline oal_bool_enum_uint8 mac_mib_get_HENumberSoundingDimensionsBelow80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsBelow80Mhz;
}

static inline void mac_mib_set_HENumberSoundingDimensionsOver80Mhz(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsOver80Mhz = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HENumberSoundingDimensionsOver80Mhz(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.uc_dot11HENumberSoundingDimensionsOver80Mhz;
}

static inline void mac_mib_set_HENg16SUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16SUFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HENg16SUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16SUFeedback;
}

static inline void mac_mib_set_HENg16MUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16MUFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HENg16MUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Ng16MUFeedback;
}

static inline void mac_mib_set_HECodebook42SUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook42SUFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HECodebook42SUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook42SUFeedback;
}

static inline void mac_mib_set_HECodebook75MUFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook75MUFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HECodebook75MUFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11Codebook75MUFeedback;
}

static inline void mac_mib_set_he_TriggeredSUBeamformingFeedback(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredSUBeamformingFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_TriggeredSUBeamformingFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredSUBeamformingFeedback;
}

static inline void mac_mib_set_he_TriggeredCQIFeedback(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredCQIFeedback = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_TriggeredCQIFeedback(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11TriggeredCQIFeedback;
}

static inline void mac_mib_set_HESUPPDUwith1xHELTFand0point8GIlmplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_HESUPPDUwith1xHELTFand0point8GIlmplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented;
}

static inline void mac_mib_set_he_MultiBSSIDActived(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->st_wlan_rom_mib_he_sta_config.en_dot11MultiBSSIDActived = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_MultiBSSIDActived(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_sta_config.en_dot11MultiBSSIDActived;
}
#endif

static inline void mac_mib_set_PPEThresholdsRequired(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired = value;
}

static inline oal_bool_enum_uint8 mac_mib_get_PPEThresholdsRequired(mac_vap_stru *mac_vap)
{
    return mac_vap->st_wlan_rom_mib_he_phy_config.en_dot11PPEThresholdsRequired;
}
#endif
#endif
