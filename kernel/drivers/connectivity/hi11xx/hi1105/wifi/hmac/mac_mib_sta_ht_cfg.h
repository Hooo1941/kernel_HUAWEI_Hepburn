/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib ht配置相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_STA_HT_CFG_H
#define MAC_MIB_STA_HT_CFG_H

#ifdef _PRE_WLAN_FEATURE_SMPS
static inline wlan_mib_mimo_power_save_enum mac_mib_get_smps(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;
}

static inline void mac_mib_set_smps(mac_vap_stru *mac_vap, wlan_mib_mimo_power_save_enum sm_power_save)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave =  sm_power_save;
}
#endif  // _PRE_WLAN_FEATURE_SMPS

static inline wlan_mib_max_amsdu_lenth_enum mac_mib_get_max_amsdu_length(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength;
}

static inline void mac_mib_set_max_amsdu_length(mac_vap_stru *mac_vap,
    wlan_mib_max_amsdu_lenth_enum max_amsdu_length)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength = max_amsdu_length;
}

static inline oal_bool_enum_uint8 mac_mib_get_lsig_txop_protection(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented;
}

static inline void mac_mib_set_lsig_txop_protection(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 lsig_txop_protection)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented = lsig_txop_protection;
}

static inline uint32_t mac_mib_get_max_ampdu_len_exponent(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MaxRxAMPDUFactor;
}

static inline void mac_mib_set_max_ampdu_len_exponent(mac_vap_stru *mac_vap, uint32_t max_ampdu_len_exponent)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MaxRxAMPDUFactor = max_ampdu_len_exponent;
}

static inline uint8_t mac_mib_get_min_mpdu_start_spacing(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MinimumMPDUStartSpacing;
}

static inline void mac_mib_set_min_mpdu_start_spacing(mac_vap_stru *mac_vap, uint8_t min_mpdu_start_spacing)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11MinimumMPDUStartSpacing = min_mpdu_start_spacing;
}

static inline oal_bool_enum_uint8 mac_mib_get_pco_option_implemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented;
}

static inline void mac_mib_set_pco_option_implemented(mac_vap_stru *mac_vap,
    oal_bool_enum_uint8 pco_option_implemented)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented = pco_option_implemented;
}

static inline uint32_t mac_mib_get_transition_time(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11TransitionTime;
}

static inline void mac_mib_set_transition_time(mac_vap_stru *mac_vap, uint32_t transition_time)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.dot11TransitionTime = transition_time;
}

static inline wlan_mib_mcs_feedback_opt_implt_enum mac_mib_get_mcs_fdbk(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented;
}

static inline void mac_mib_set_mcs_fdbk(mac_vap_stru *mac_vap,
    wlan_mib_mcs_feedback_opt_implt_enum mcs_fdbk)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented = mcs_fdbk;
}

static inline oal_bool_enum_uint8 mac_mib_get_htc_sup(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported;
}

static inline void mac_mib_set_htc_sup(mac_vap_stru *mac_vap, oal_bool_enum_uint8 htc_sup)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported = htc_sup;
}

static inline oal_bool_enum_uint8 mac_mib_get_rd_rsp(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented;
}

static inline void mac_mib_set_rd_rsp(mac_vap_stru *mac_vap, oal_bool_enum_uint8 rd_rsp)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented = rd_rsp;
}

static inline oal_bool_enum_uint8 mac_mib_get_HighThroughputOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented;
}

static inline void mac_mib_set_HighThroughputOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HighThroughputOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_DualBandSupport(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport;
}

static inline void mac_mib_set_he_DualBandSupport(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HEDualBandSupport = value;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_LDPCCodingInPayload(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload;
}

static inline void mac_mib_set_he_LDPCCodingInPayload(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HELDPCCodingInPayload = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_STBCTxBelow80M(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCTxBelow80MHz;
}

static inline void mac_mib_set_he_STBCTxBelow80M(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCTxBelow80MHz = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_STBCRxBelow80M(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCRxBelow80MHz;
}

static inline void mac_mib_set_he_STBCRxBelow80M(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11HESTBCRxBelow80MHz = val;
}


static inline void mac_mib_set_he_SUBeamformer(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_SUBeamformer(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformer;
}

static inline void mac_mib_set_he_SUBeamformee(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_SUBeamformee(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11SUBeamformee;
}

static inline void mac_mib_set_he_MUBeamformer(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_he_MUBeamformer(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.en_dot11MUBeamformer;
}

static inline void mac_mib_set_he_HEChannelWidthSet(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet = value;
}

static inline uint8_t mac_mib_get_he_HEChannelWidthSet(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HEChannelWidthSet;
}

static inline void mac_mib_set_he_HighestNSS(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS = value;
}

static inline uint8_t mac_mib_get_he_HighestNSS(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestNSS;
}

static inline void mac_mib_set_he_HighestMCS(mac_vap_stru *mac_vap, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS = value;
}

static inline uint8_t mac_mib_get_he_HighestMCS(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_he_phy_config.uc_dot11HighestMCS;
}

static inline oal_bool_enum_uint8 mac_mib_get_FortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->st_channel.en_band == WLAN_BAND_2G) ? \
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented : \
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented;
}

static inline oal_bool_enum_uint8 mac_mib_get_2GFortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented);
}

static inline oal_bool_enum_uint8 mac_mib_get_5GFortyMHzOperationImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented);
}

static inline void mac_mib_set_FortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = val;
    } else {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = val;
    }
}

static inline void mac_mib_set_2GFortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GFortyMHzOperationImplemented = val;
}

static inline void mac_mib_set_5GFortyMHzOperationImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GFortyMHzOperationImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap)
{
    return (mac_vap->st_channel.en_band == WLAN_BAND_2G) ?
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented :
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented;
}

static inline void mac_mib_set_ShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    if (mac_vap->st_channel.en_band == WLAN_BAND_2G) {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = val;
    } else {
        mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = val;
    }
}

static inline void mac_mib_set_2GShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot112GShortGIOptionInFortyImplemented = val;
}

static inline void mac_mib_set_5GShortGIOptionInFortyImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot115GShortGIOptionInFortyImplemented = val;
}

static inline void mac_mib_set_ReceiveStaggerSoundingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented =
        val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ReceiveStaggerSoundingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.\
        st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented;
}

static inline void mac_mib_set_TransmitStaggerSoundingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented =
        val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TransmitStaggerSoundingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11TransmitStaggerSoundingOptionImplemented;
}

static inline void mac_mib_set_ReceiveNDPOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ReceiveNDPOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented;
}

static inline void mac_mib_set_TransmitNDPOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TransmitNDPOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented;
}

static inline void mac_mib_set_ImplicitTransmitBeamformingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented =
        val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ImplicitTransmitBeamformingOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ImplicitTransmitBeamformingOptionImplemented;
}

static inline void mac_mib_set_CalibrationOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_CalibrationOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented;
}

static inline void mac_mib_set_ExplicitCSITransmitBeamformingOptionImplemented(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitCSITransmitBeamformingOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitCSITransmitBeamformingOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitCSITransmitBeamformingOptionImplemented;
}

static inline void mac_mib_set_ExplicitNonCompressedBeamformingMatrixOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingMatrixOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented;
}

static inline void mac_mib_set_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented;
}

static inline void mac_mib_set_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented;
}

static inline void mac_mib_set_ExplicitCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_ExplicitCompressedBeamformingFeedbackOptionImplemented(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented;
}

static inline void mac_mib_set_NumberBeamFormingCSISupportAntenna(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_NumberBeamFormingCSISupportAntenna(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.uc_dot11NumberBeamFormingCSISupportAntenna;
}

static inline void mac_mib_set_NumberNonCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_NumberNonCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
}

static inline void mac_mib_set_NumberCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *vap, oal_bool_enum_uint8 val)
{
    vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberCompressedBeamformingMatrixSupportAntenna = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_NumberCompressedBeamformingMatrixSupportAntenna(
    mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.st_wlan_mib_txbf_config.\
        uc_dot11NumberCompressedBeamformingMatrixSupportAntenna;
}

static inline void mac_mib_set_HTGreenfieldOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented = val;
}

static inline uint8_t mac_mib_get_HTGreenfieldOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11HTGreenfieldOptionImplemented;
}

static inline void mac_mib_set_ShortGIOptionInTwentyImplemented(
    mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented = val;
}

static inline uint8_t mac_mib_get_ShortGIOptionInTwentyImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11ShortGIOptionInTwentyImplemented;
}

static inline void mac_mib_set_LDPCCodingOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented = val;
}

static inline uint8_t mac_mib_get_LDPCCodingOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionImplemented;
}

static inline void mac_mib_set_LDPCCodingOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated = val;
}

static inline uint8_t mac_mib_get_LDPCCodingOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11LDPCCodingOptionActivated;
}

static inline void mac_mib_set_TxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionImplemented;
}

static inline void mac_mib_set_TxSTBCOptionActivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated = val;
}

static inline oal_bool_enum_uint8 mac_mib_get_TxSTBCOptionActivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxSTBCOptionActivated;
}

static inline void mac_mib_set_RxSTBCOptionImplemented(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented = val;
}

static inline uint8_t mac_mib_get_RxSTBCOptionImplemented(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11RxSTBCOptionImplemented;
}

static inline void mac_mib_set_HighestSupportedDataRate(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11HighestSupportedDataRate = val;
}

static inline uint32_t mac_mib_get_HighestSupportedDataRate(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11HighestSupportedDataRate;
}

static inline void mac_mib_set_TxMCSSetDefined(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined = val;
}

static inline uint8_t mac_mib_get_TxMCSSetDefined(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxMCSSetDefined;
}

static inline void mac_mib_set_TxRxMCSSetNotEqual(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual = val;
}

static inline uint8_t mac_mib_get_TxRxMCSSetNotEqual(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxRxMCSSetNotEqual;
}

static inline void mac_mib_set_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *mac_vap, uint32_t val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11TxMaximumNumberSpatialStreamsSupported = val;
}

static inline uint32_t mac_mib_get_TxMaximumNumberSpatialStreamsSupported(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.dot11TxMaximumNumberSpatialStreamsSupported;
}

static inline void mac_mib_set_TxUnequalModulationSupported(mac_vap_stru *mac_vap, oal_bool_enum_uint8 val)
{
    mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported = val;
}

static inline uint8_t mac_mib_get_TxUnequalModulationSupported(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_phy_ht.en_dot11TxUnequalModulationSupported;
}
#endif
