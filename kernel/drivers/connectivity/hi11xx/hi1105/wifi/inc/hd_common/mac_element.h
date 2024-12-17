/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : IE相关的结构体定义
 * 作    者 :
 * 创建日期 : 2022年8月18日
 */

#ifndef MAC_ELEMENT_H
#define MAC_ELEMENT_H
#include "oal_ext_if.h"

#define MAC_OUI_LEN 3

#pragma pack(1)

/* presence response Frame - Frame Body */
/* ---------------------------------------------------------------------- */
/* |Category |OUI   |OUI type|OUI subtype  | Dialog Token   |Element    | */
/* ---------------------------------------------------------------------- */
/* |1        |3     |1       |1            |1               |variable   | */
/* ---------------------------------------------------------------------- */
/* ************************************************************************ */
typedef struct {
    uint8_t category;
    uint8_t oui[3];
    uint8_t oui_type;
    uint8_t oui_sub_type;
    uint8_t token;
} DECLARE_PACKED mac_hiex_action_hdr_stru;

typedef struct {
    uint32_t bit_hiex_enable : 1;
    uint32_t bit_hiex_version : 3;
    uint32_t bit_himit_enable : 1;
    uint32_t bit_ht_himit_enable : 1;
    uint32_t bit_vht_himit_enable : 1;
    uint32_t bit_he_himit_enable : 1;

    uint32_t bit_he_htc_himit_id : 4;
    uint32_t bit_chip_type : 4;

    uint32_t bit_himit_version : 2;
    uint32_t bit_ersru_enable : 1;
    uint32_t bit_resv : 12;
    uint32_t bit_ext_flag : 1;
} DECLARE_PACKED mac_hiex_cap_stru;

/* 厂家自定义IE 数据结构，摘自linux 内核 */
typedef struct mac_ieee80211_vendor_ie {
    uint8_t uc_element_id;
    uint8_t uc_len;
    uint8_t auc_oui[MAC_OUI_LEN];
    uint8_t uc_oui_type;
} DECLARE_PACKED mac_ieee80211_vendor_ie_stru;

typedef struct {
    mac_ieee80211_vendor_ie_stru vender;
    mac_hiex_cap_stru cap;
} DECLARE_PACKED mac_hiex_vendor_ie_stru;

#pragma pack()

#endif /* MAC_ELEMENT_H */
