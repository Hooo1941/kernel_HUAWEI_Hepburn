/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mac_mib edca相关头文件
 * 创建日期 : 2020年7月28日
 */
#ifndef MAC_MIB_STA_EDCA_H
#define MAC_MIB_STA_EDCA_H

static inline void mac_mib_set_EDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableIndex = value;
}

static inline uint32_t mac_mib_get_EDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableIndex;
}

static inline void mac_mib_set_EDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmin = value;
}

static inline uint32_t mac_mib_get_EDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmin;
}

static inline void mac_mib_set_EDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmax = value;
}

static inline uint32_t mac_mib_get_EDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableCWmax;
}

static inline void mac_mib_set_EDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableAIFSN = value;
}

static inline uint32_t mac_mib_get_EDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableAIFSN;
}

static inline void mac_mib_set_EDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableTXOPLimit = value;
}

static inline uint32_t mac_mib_get_EDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].dot11EDCATableTXOPLimit;
}

static inline void mac_mib_set_EDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].en_dot11EDCATableMandatory = value;
}

static inline uint8_t mac_mib_get_EDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_edca[idx].en_dot11EDCATableMandatory;
}

static inline void mac_mib_set_QAPEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex;
}

static inline void mac_mib_set_QAPEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin;
}

static inline void mac_mib_set_QAPEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax;
}

static inline void mac_mib_set_QAPEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN;
}

static inline void mac_mib_set_QAPEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit;
}

static inline void mac_mib_set_QAPEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory = value;
}

static inline uint8_t mac_mib_get_QAPEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory;
}

static inline void mac_mib_set_QAPEDCATableMSDULifetime(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableMSDULifetime = value;
}

static inline uint32_t mac_mib_get_QAPEDCATableMSDULifetime(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->pst_mib_info->st_wlan_mib_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableMSDULifetime;
}

#ifdef _PRE_WLAN_FEATURE_11AX
static inline void mac_mib_set_QAPMUEDCATableIndex(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableIndex = value;
}

static inline void mac_mib_set_QAPMUEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin = value;
}

static inline uint32_t mac_mib_get_QAPMUEDCATableCWmin(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmin;
}

static inline void mac_mib_set_QAPMUEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax = value;
}

static inline uint32_t mac_mib_get_QAPMUEDCATableCWmax(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableCWmax;
}

static inline void mac_mib_set_QAPMUEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN = value;
}

static inline uint32_t mac_mib_get_QAPMUEDCATableAIFSN(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableAIFSN;
}

static inline void mac_mib_set_QAPMUEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx, uint32_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit = value;
}

static inline uint32_t mac_mib_get_QAPMUEDCATableTXOPLimit(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].dot11QAPEDCATableTXOPLimit;
}

static inline void mac_mib_set_QAPMUEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx, uint8_t value)
{
    mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory = value;
}

static inline uint8_t mac_mib_get_QAPMUEDCATableMandatory(mac_vap_stru *mac_vap, uint8_t idx)
{
    return mac_vap->st_wlan_mib_mu_edca.ast_wlan_mib_qap_edac[idx].en_dot11QAPEDCATableMandatory;
}
#endif // _PRE_WLAN_FEATURE_11AX
#endif
