/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib sta配置相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_STA_CONFIG_H
#define MAC_MIB_STA_CONFIG_H

static inline uint8_t *mac_mib_get_StationID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID;
}

static inline void mac_mib_set_StationID(mac_vap_stru *mac_vap, uint8_t *sta_id)
{
    oal_set_mac_addr(mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID, sta_id);
}
static inline oal_bool_enum_uint8 mac_mib_get_SpectrumManagementImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented;
}

static inline void mac_mib_set_SpectrumManagementImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementImplemented = val;
}

static inline uint32_t mac_mib_get_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryMaximumTimeout;
}

static inline uint32_t mac_mib_get_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryRetryTimeout;
}

static inline void mac_mib_set_dot11AssociationSAQueryMaximumTimeout(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryMaximumTimeout = value;
}

static inline void mac_mib_set_dot11AssociationSAQueryRetryTimeout(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationSAQueryRetryTimeout = value;
}

static inline void mac_mib_set_dot11ExtendedChannelSwitchActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11ExtendedChannelSwitchActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated;
}

static inline void mac_mib_set_dot11RadioMeasurementActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11RadioMeasurementActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated;
}

static inline void mac_mib_set_dot11QBSSLoadImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11QBSSLoadImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented;
}

static inline void mac_mib_set_dot11APSDOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11APSDOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented;
}

static inline void mac_mib_set_dot11DelayedBlockAckOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11DelayedBlockAckOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented;
}

static inline void mac_mib_set_dot11ImmediateBlockAckOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11ImmediateBlockAckOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented;
}

static inline void mac_mib_set_dot11QosOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11QosOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented;
}

static inline void mac_mib_set_dot11MultiDomainCapabilityActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11MultiDomainCapabilityActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated;
}

static inline void mac_mib_set_dot11AssociationResponseTimeOut(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationResponseTimeOut = val;
}

static inline uint32_t mac_mib_get_dot11AssociationResponseTimeOut(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AssociationResponseTimeOut;
}

static inline void mac_mib_set_MaxAssocUserNums(mac_vap_stru *mac_vap, uint16_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums = val;
}

static inline uint16_t mac_mib_get_MaxAssocUserNums(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.us_dot11MaxAssocUserNums;
}

static inline void mac_mib_set_SupportRateSetNums(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums = val;
}

static inline uint8_t mac_mib_get_SupportRateSetNums(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11SupportRateSetNums;
}

static inline void mac_mib_set_CfgAmsduTxAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive = val;
}

static inline uint8_t mac_mib_get_CfgAmsduTxAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmsduTxAtive;
}

static inline void mac_mib_set_AmsduAggregateAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive = val;
}

static inline uint8_t mac_mib_get_AmsduAggregateAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduAggregateAtive;
}

static inline void mac_mib_set_AmsduPlusAmpduActive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive = val;
}

static inline uint8_t mac_mib_get_AmsduPlusAmpduActive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AmsduPlusAmpduActive;
}

static inline void mac_mib_set_WPSActive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive = val;
}

static inline uint8_t mac_mib_get_WPSActive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WPSActive;
}

static inline void mac_mib_set_2040SwitchProhibited(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited = val;
}

static inline uint8_t mac_mib_get_2040SwitchProhibited(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot112040SwitchProhibited;
}

static inline void mac_mib_set_TxAggregateActived(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived = val;
}

static inline uint8_t mac_mib_get_TxAggregateActived(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxAggregateActived;
}

static inline void mac_mib_set_CfgAmpduTxAtive(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive = val;
}

static inline uint8_t mac_mib_get_CfgAmpduTxAtive(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11CfgAmpduTxAtive;
}

static inline void mac_mib_set_RxBASessionNumber(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber = val;
}

static inline uint8_t mac_mib_get_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber;
}

static inline void mac_mib_incr_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber++;
}

static inline void mac_mib_decr_RxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11RxBASessionNumber--;
}

static inline void mac_mib_set_TxBASessionNumber(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber = val;
}

static inline uint8_t mac_mib_get_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber;
}

static inline void mac_mib_incr_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber++;
}

static inline void mac_mib_decr_TxBASessionNumber(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11TxBASessionNumber--;
}

static inline void mac_mib_set_VAPClassifyTidNo(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo = val;
}

static inline uint8_t mac_mib_get_VAPClassifyTidNo(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11VAPClassifyTidNo;
}

static inline void mac_mib_set_AuthenticationMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode = val;
}

static inline uint8_t mac_mib_get_AuthenticationMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AuthenticationMode;
}

static inline void mac_mib_set_AddBaMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode = val;
}

static inline uint8_t mac_mib_get_AddBaMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11AddBaMode;
}

static inline void mac_mib_set_80211iMode(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode = val;
}

static inline uint8_t mac_mib_get_80211iMode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot1180211iMode;
}

static inline void mac_mib_set_TxTrafficClassifyFlag(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag = val;
}

static inline uint8_t mac_mib_get_TxTrafficClassifyFlag(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11TxTrafficClassifyFlag;
}

static inline void mac_mib_set_StaAuthCount(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount = val;
}

static inline uint8_t mac_mib_get_StaAuthCount(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount;
}

static inline void mac_mib_set_StaAssocCount(mac_vap_stru *mac_vap, uint8_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount = val;
}

static inline uint8_t mac_mib_get_StaAssocCount(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount;
}

static inline void mac_mib_incr_StaAuthCount(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAuthCount++;
}

static inline void mac_mib_incr_StaAssocCount(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.uc_dot11StaAssocCount++;
}

static inline void mac_mib_set_dot11RMBeaconTableMeasurementActivated(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11RMBeaconTableMeasurementActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated;
}

static inline void mac_mib_set_2040BSSCoexistenceManagementSupport(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_operation.en_dot112040BSSCoexistenceManagementSupport = val;
}

static inline uint32_t mac_mib_get_dot11dtimperiod(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11DTIMPeriod;
}

static inline void mac_mib_set_dot11VapMaxBandWidth(mac_vap_stru *mac_vap,
    wlan_bw_cap_enum_uint8 bandwidth)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth = bandwidth;
}

static inline wlan_bw_cap_enum_uint8 mac_mib_get_dot11VapMaxBandWidth(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VapMaxBandWidth;
}

static inline void mac_mib_set_BeaconPeriod(mac_vap_stru *mac_vap, uint32_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11BeaconPeriod = value;
    }
}

static inline uint32_t mac_mib_get_BeaconPeriod(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11BeaconPeriod;
}

static inline void mac_mib_set_DesiredBSSType(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType = value;
}

static inline uint8_t mac_mib_get_DesiredBSSType(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType;
}

static inline uint8_t *mac_mib_get_DesiredSSID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;
}

static inline void mac_mib_set_AuthenticationResponseTimeOut(mac_vap_stru *mac_vap, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AuthenticationResponseTimeOut = value;
}
static inline uint32_t mac_mib_get_AuthenticationResponseTimeOut(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11AuthenticationResponseTimeOut;
}

static inline uint8_t *mac_mib_get_p2p0_dot11StationID(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
}

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
static inline oal_bool_enum_uint8 mac_mib_get_WirelessManagementImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented;
}

static inline void mac_mib_set_WirelessManagementImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11WirelessManagementImplemented = val;
}
#endif  // _PRE_WLAN_FEATURE_11V_ENABLE

static inline void mac_mib_set_dot11dtimperiod(mac_vap_stru *mac_vap, uint32_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11DTIMPeriod = value;
    }
}

static inline uint32_t mac_mib_get_powermanagementmode(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11PowerManagementMode;
}

static inline void mac_mib_set_powermanagementmode(mac_vap_stru *mac_vap, uint8_t value)
{
    if (value != 0) {
        mac_vap->pst_mib_info->st_wlan_mib_sta_config.dot11PowerManagementMode = value;
    }
}

static inline void mac_mib_set_SpectrumManagementRequired(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_dot11SpectrumManagementRequired(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired;
}
#endif
