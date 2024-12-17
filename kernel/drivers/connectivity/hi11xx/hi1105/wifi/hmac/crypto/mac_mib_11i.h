/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib加密相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_11I_H
#define MAC_MIB_11I_H

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_pre_auth_actived(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_pre_auth_actived(mac_vap_stru *mac_vap, oal_bool_enum_uint8 pre_auth)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAPreauthenticationActivated = pre_auth;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsnacfg_ptksareplaycounters(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnacfg_ptksareplaycounters(mac_vap_stru *mac_vap, uint8_t ptksa_replay_counters)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigPTKSAReplayCounters =
        ptksa_replay_counters;
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsnacfg_gtksareplaycounters(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnacfg_gtksareplaycounters(mac_vap_stru *mac_vap, uint8_t gtksa_replay_counters)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.uc_dot11RSNAConfigGTKSAReplayCounters =
        gtksa_replay_counters;
}

OAL_STATIC OAL_INLINE void mac_mib_init_rsnacfg_suites(mac_vap_stru *mac_vap)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite = MAC_WPA_CHIPER_TKIP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[0] = MAC_WPA_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[1] = MAC_WPA_CHIPER_TKIP;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[0] = MAC_WPA_AKM_PSK;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[1] = MAC_WPA_AKM_PSK_SHA256;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite = MAC_RSN_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[0] = MAC_RSN_CHIPER_CCMP;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[1] = MAC_RSN_CHIPER_TKIP;

    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[0] = MAC_RSN_AKM_PSK;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[1] = MAC_RSN_AKM_PSK_SHA256;
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite =
        MAC_RSN_CIPHER_SUITE_AES_128_CMAC;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_wpa_group_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_group_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.wpa_group_suite = suite;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_rsn_group_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_group_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_suite = suite;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_get_rsn_group_mgmt_suite(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_group_mgmt_suite(mac_vap_stru *mac_vap, uint32_t suite)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.rsn_group_mgmt_suite = suite;
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    uint32_t i;
    for (i = 0; i < WLAN_PAIRWISE_CIPHER_SUITES; i++) {
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites[i] = suites[i];
    }
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_pair_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    uint32_t i;
    for (i = 0; i < WLAN_PAIRWISE_CIPHER_SUITES; i++) {
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites[i] = suites[i];
    }
}

OAL_STATIC OAL_INLINE void mac_mib_set_wpa_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    uint32_t i;
    for (i = 0; i < WLAN_AUTHENTICATION_SUITES; i++) {
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites[i] = suites[i];
    }
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsn_akm_suites(mac_vap_stru *mac_vap, uint32_t *suites)
{
    uint32_t i;
    for (i = 0; i < WLAN_AUTHENTICATION_SUITES; i++) {
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites[i] = suites[i];
    }
}
OAL_STATIC OAL_INLINE uint32_t mac_mib_rsna_cfg_params_match_suites(
    uint32_t *suites, uint32_t *rsna_cfg_params, uint8_t cipher_len)
{
    uint8_t idx_local;
    uint8_t idx_peer;

    for (idx_local = 0; idx_local < cipher_len; idx_local++) {
        for (idx_peer = 0; idx_peer < cipher_len; idx_peer++) {
            if (rsna_cfg_params[idx_local] == suites[idx_peer]) {
                return suites[idx_peer];
            }
        }
    }
    return 0;
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_pair_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_pair_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_wpa_akm_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint32_t mac_mib_rsn_akm_match_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_rsna_cfg_params_match_suites(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsna_cfg_params(
    uint32_t *suites, uint32_t *rsna_cfg_params, uint8_t cipher_len)
{
    uint8_t idx;
    uint8_t nums = 0;

    for (idx = 0; idx < cipher_len; idx++) {
        if (rsna_cfg_params[idx] != 0) {
            suites[nums++] = rsna_cfg_params[idx];
        }
    }
    return nums;
}


OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_pair_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_pair_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_pair_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_wpa_akm_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_wpa_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE uint8_t mac_mib_get_rsn_akm_suites_s(mac_vap_stru *mac_vap,
    uint32_t *suites, uint8_t cipher_len)
{
    return mac_mib_get_rsna_cfg_params(suites,
        mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.aul_rsn_akm_suites,
        cipher_len / sizeof(uint32_t));
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_privacyinvoked(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked;
}

OAL_STATIC OAL_INLINE void mac_mib_set_privacyinvoked(mac_vap_stru *mac_vap, oal_bool_enum_uint8 privacyinvoked)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11PrivacyInvoked = privacyinvoked;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_rsnaactivated(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated;
}

OAL_STATIC OAL_INLINE void mac_mib_set_rsnaactivated(mac_vap_stru *mac_vap, oal_bool_enum_uint8 rsnaactivated)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAActivated = rsnaactivated;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_wep_enabled(mac_vap_stru *mac_vap)
{
    if (mac_vap == NULL) {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(mac_vap) == OAL_FALSE) ||
        (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE)) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_wep_allowed(mac_vap_stru *mac_vap)
{
    if (mac_mib_get_rsnaactivated(mac_vap) == OAL_TRUE) {
        return OAL_FALSE;
    } else {
        return mac_is_wep_enabled(mac_vap);
    }
}

OAL_STATIC OAL_INLINE void mac_set_wep_default_keyid(mac_vap_stru *mac_vap, uint8_t default_key_id)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID = default_key_id;
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_default_keyid(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID);
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_default_keysize(mac_vap_stru *mac_vap)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[mac_get_wep_default_keyid(mac_vap)].\
        auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}

OAL_STATIC OAL_INLINE uint8_t mac_get_wep_keysize(mac_vap_stru *mac_vap, uint8_t uc_idx)
{
    return (mac_vap->pst_mib_info->st_wlan_mib_privacy.ast_wlan_mib_wep_dflt_key[uc_idx].\
        auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET]);
}

OAL_STATIC OAL_INLINE wlan_ciper_protocol_type_enum_uint8 mac_get_wep_type(mac_vap_stru *mac_vap, uint8_t key_id)
{
    wlan_ciper_protocol_type_enum_uint8 cipher_type;

    switch (mac_get_wep_keysize(mac_vap, key_id)) {
        case 40: /* 40 keysize */
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case 104: /* 104 keysize */
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
        default:
            cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
    }
    return cipher_type;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_is_tkip_only(mac_vap_stru *mac_vap)
{
    uint8_t pair_suites_num;
    uint32_t pcip[WLAN_PAIRWISE_CIPHER_SUITES] = { 0 };

    if (mac_vap == NULL) {
        return OAL_FALSE;
    }

    if ((mac_vap->st_cap_flag.bit_wpa2 == OAL_FALSE) && (mac_vap->st_cap_flag.bit_wpa == OAL_FALSE)) {
        /* pbc第一次关联 wpa未设置  */
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(mac_vap) == OAL_FALSE) ||
        (mac_mib_get_rsnaactivated(mac_vap) == OAL_FALSE)) { // 不加密或者是WEP加密时，返回false
        return OAL_FALSE;
    }

    if (mac_vap->st_cap_flag.bit_wpa == OAL_TRUE) {
        pair_suites_num = mac_mib_get_wpa_pair_suites_s(mac_vap, &pcip[0], sizeof(pcip));
        if ((pair_suites_num != 1) || (pcip[0] != MAC_WPA_CHIPER_TKIP)) { // wpa加密时，若非tkip only，返回false
            return OAL_FALSE;
        }
    }

    if (mac_vap->st_cap_flag.bit_wpa2 == OAL_TRUE) {
        pair_suites_num = mac_mib_get_rsn_pair_suites_s(mac_vap, &pcip[0], sizeof(pcip));
        if ((pair_suites_num != 1) || (pcip[0] != MAC_RSN_CHIPER_TKIP)) { // wpa2加密时，若非tkip only，返回false
            return OAL_FALSE;
        }
    }

    return OAL_TRUE;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RSNAMFPC(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC;
}

OAL_STATIC OAL_INLINE oal_bool_enum_uint8 mac_mib_get_dot11RSNAMFPR(mac_vap_stru *mac_vap)
{
    return mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RSNAMFPC(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPC = value;
}

OAL_STATIC OAL_INLINE void mac_mib_set_dot11RSNAMFPR(mac_vap_stru *mac_vap, oal_bool_enum_uint8 value)
{
    mac_vap->pst_mib_info->st_wlan_mib_privacy.en_dot11RSNAMFPR = value;
}
#endif
