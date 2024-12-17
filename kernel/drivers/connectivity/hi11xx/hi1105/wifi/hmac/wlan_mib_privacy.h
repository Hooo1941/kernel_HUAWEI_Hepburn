/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素privacy结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_PRIVACY_H
#define WLAN_MIB_PRIVACY_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

/* 根据802.11-2016 9.4.1.1 Authentication Algorithm Number field 重新定义认证算法枚举 */
typedef enum {
    WLAN_MIB_AUTH_ALG_OPEN_SYS = 0,
    WLAN_MIB_AUTH_ALG_SHARE_KEY = 1,
    WLAN_MIB_AUTH_ALG_FT = 2,
    WLAN_MIB_AUTH_ALG_SAE = 3,
    WLAN_MIB_AUTH_ALG_TBPEKE = 0x60,

    WLAN_MIB_AUTH_ALG_BUTT
} wlan_mib_auth_alg_enum;

typedef enum {
    WLAN_MIB_RSNACFG_GRPREKEY_DISABLED = 1,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEBASED = 2,
    WLAN_MIB_RSNACFG_GRPREKEY_PACKETBASED = 3,
    WLAN_MIB_RSNACFG_GRPREKEY_TIMEPACKETBASED = 4,

    WLAN_MIB_RSNACFG_GRPREKEY_BUTT
} wlan_mib_rsna_cfg_grp_rekey_enum;

/******************************************************************************/
/* dot11WEPDefaultKeys  TABLE - members of Dot11WEPDefaultKeysEntry           */
/******************************************************************************/
/* Conceptual table for WEP default keys. This table contains the four WEP    */
/* default secret key values corresponding to the four possible KeyID values. */
/* The WEP default secret keys are logically WRITE-ONLY. Attempts to read the */
/* entries in this table return unsuccessful status and values of null or 0.  */
/* The default value of each WEP default key is null.                         */
typedef struct {
    uint8_t auc_dot11WEPDefaultKeyValue[WLAN_MAX_WEP_STR_SIZE]; /* dot11WEPDefaultKeyValue WEPKeytype  */
    uint8_t uc_rsv;
} wlan_mib_Dot11WEPDefaultKeysEntry_stru;

/* Start of dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 } */
typedef struct {
    uint8_t auc_dot11StationID[WLAN_MAC_ADDR_LEN];      /* dot11StationID MacAddress, */
    uint8_t auc_p2p0_dot11StationID[WLAN_MAC_ADDR_LEN]; /* P2P0 dot11StationID MacAddress, */

    uint32_t dot11AuthenticationResponseTimeOut;    /* dot11AuthenticationResponseTimeOut Unsigned32, */
    uint32_t dot11BeaconPeriod;                     /* dot11BeaconPeriod Unsigned32, */
    uint32_t dot11DTIMPeriod;                       /* dot11DTIMPeriod Unsigned32, */
    uint32_t dot11AssociationResponseTimeOut;       /* dot11AssociationResponseTimeOut Unsigned32, */
    uint32_t dot11AssociationSAQueryMaximumTimeout; /* dot11AssociationSAQueryMaximumTimeout Unsigned32, */
    uint32_t dot11AssociationSAQueryRetryTimeout;   /* dot11AssociationSAQueryRetryTimeout Unsigned32, */

    uint8_t auc_dot11DesiredSSID[WLAN_SSID_MAX_LEN]; /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    /* +1预留\0 */
    uint16_t us_dot11MaxAssocUserNums;

    oal_bool_enum_uint8 en_dot11TxAggregateActived;   /* VAP协议能力是否支持发送聚合 */
    oal_bool_enum_uint8 en_dot11CfgAmpduTxAtive;      /* 配置命令是否允许发送聚合 */
    oal_bool_enum_uint8 en_dot11AmsduPlusAmpduActive; /* ADDBA REQ中标志是否支持amsdu */
    oal_bool_enum_uint8 en_dot11CfgAmsduTxAtive;      /* 配置命令是否支持amsdu */

    oal_bool_enum_uint8 en_dot11AmsduAggregateAtive; /* VAP协议能力是否支持amsdu发送聚合 */
    uint8_t uc_dot11RxBASessionNumber;             /* 该vap下，rx BA会话的数目 */
    uint8_t uc_dot11TxBASessionNumber;             /* 该vap下，tx BA会话的数目 */
    uint8_t uc_dot11VAPClassifyTidNo;              /* 仅在基于vap的流分类使能后有效 */

    wlan_auth_alg_mode_enum_uint8 en_dot11AuthenticationMode; /* 认证算法 同内核nl80211_auth_type */
    wlan_addba_mode_enum_uint8 en_dot11AddBaMode;
    uint8_t en_dot11DesiredBSSType;    /* wlan_mib_desired_bsstype_enum: dot11DesiredBSSType INTEGER, */
    uint8_t dot11PowerManagementMode; /* wlan_mib_pwr_mgmt_mode_enum: dot11PowerManagementMode INTEGER, */

    uint8_t uc_dot11StaAuthCount; /* 记录STA发起关联的次数 */
    uint8_t uc_dot11StaAssocCount;
    uint8_t uc_dot11SupportRateSetNums;
    uint8_t en_dot11TxTrafficClassifyFlag; /* 业务识别功能开关 */

    /* Byte1 */
    oal_bool_enum_uint8 en_dot11MultiDomainCapabilityActivated : 1;     /* dot11MultiDomainCapabilityActivated */
    oal_bool_enum_uint8 en_dot11SpectrumManagementImplemented : 1;      /* dot11SpectrumManagementImplemented */
    oal_bool_enum_uint8 en_dot11SpectrumManagementRequired : 1;         /* dot11SpectrumManagementRequired */
    oal_bool_enum_uint8 en_dot11QosOptionImplemented : 1;               /* dot11QosOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11ImmediateBlockAckOptionImplemented : 1; /* dot11ImmediateBlockAckOptionImplemented */
    oal_bool_enum_uint8 en_dot11DelayedBlockAckOptionImplemented : 1;   /* dot11DelayedBlockAckOptionImplemented */
    oal_bool_enum_uint8 en_dot11APSDOptionImplemented : 1;              /* dot11APSDOptionImplemented TruthValue, */
    oal_bool_enum_uint8 en_dot11QBSSLoadImplemented : 1;                /* dot11QBSSLoadImplemented TruthValue, */
    /* Byte2 */
    oal_bool_enum_uint8 en_dot11RadioMeasurementActivated : 1;           /* dot11RadioMeasurementActivated */
    oal_bool_enum_uint8 en_dot11ExtendedChannelSwitchActivated : 1;      /* dot11ExtendedChannelSwitchActivated */
    oal_bool_enum_uint8 en_dot11RMBeaconTableMeasurementActivated : 1;   /* dot11RMBeaconTableMeasurementActivated */
    oal_bool_enum_uint8 en_dot11WirelessManagementImplemented : 1;       /* dot11WirelessManagementImplemented */
    oal_bool_enum_uint8 en_resv1 : 4;
    /* Byte3 */
    oal_bool_enum_uint8 en_dot112040SwitchProhibited : 1;
    oal_bool_enum_uint8 en_dot11WPSActive : 1;
    oal_bool_enum_uint8 en_resv : 6;
    /* Byte4 */
    uint8_t en_dot1180211iMode; /* 指示当前的方式时WPA还是WPA2, bit0 = 1,WPA; bit1 = 1, RSN */

    wlan_bw_cap_enum_uint8 en_dot11VapMaxBandWidth; /* 当前vap可用的最大带宽能力 */
} wlan_mib_Dot11StationConfigEntry_stru;

/******************************************************************************/
/* dot11RSNAConfig TABLE (RSNA and TSN) - members of dot11RSNAConfigEntry     */
/******************************************************************************/
/* An entry in the dot11RSNAConfigTable                                       */
typedef struct {
    uint8_t uc_dot11RSNAConfigPTKSAReplayCounters; /* dot11RSNAConfigNumberOfPTKSAReplayCountersImplemented */
    uint8_t uc_dot11RSNAConfigGTKSAReplayCounters; /* dot11RSNAConfigNumberOfGTKSAReplayCountersImplemented */
    uint32_t wpa_group_suite;
    uint32_t rsn_group_suite;
    uint32_t aul_wpa_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_wpa_akm_suites[WLAN_AUTHENTICATION_SUITES];
    uint32_t aul_rsn_pair_suites[WLAN_PAIRWISE_CIPHER_SUITES];
    uint32_t aul_rsn_akm_suites[WLAN_AUTHENTICATION_SUITES];
    uint32_t rsn_group_mgmt_suite;
} wlan_mib_dot11RSNAConfigEntry_stru;

/******************************************************************************/
/* dot11WEPKeyMappings  TABLE - members of Dot11WEPKeyMappingsEntry           */
/******************************************************************************/
/* Conceptual table for WEP Key Mappings. The MIB supports the ability to     */
/* share a separate WEP key for each RA/TA pair. The Key Mappings Table con-  */
/* tains zero or one entry for each MAC address and contains two fields for   */
/* each entry: WEPOn and the corresponding WEP key. The WEP key mappings are  */
/* logically WRITE-ONLY. Attempts to read the entries in this table return    */
/* unsuccessful status and values of null or 0. The default value for all     */
/* WEPOn fields is false                                                      */
/******************************************************************************/
/* dot11Privacy TABLE - members of Dot11PrivacyEntry                          */
/******************************************************************************/
/* Group containing attributes concerned with IEEE 802.11 Privacy. Created    */
/* as a table to allow multiple instantiations on an agent.                   */
typedef struct {
    oal_bool_enum_uint8 en_dot11PrivacyInvoked : 1;                 /* dot11PrivacyInvoked TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAActivated : 1;                  /* dot11RSNAActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAPreauthenticationActivated : 1; /* dot11RSNAPreauthenticationActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RSNAMFPC : 1;                       /* dot11RSNAManagementFramerProtectionCapbility */
    oal_bool_enum_uint8 en_dot11RSNAMFPR : 1;                       /* dot11RSNAManagementFramerProtectionRequired */
    oal_bool_enum_uint8 en_resv : 3;

    uint8_t uc_dot11WEPDefaultKeyID; /* dot11WEPDefaultKeyID Unsigned8,                 */
    uint8_t auc_reserve[NUM_2_BYTES];

    wlan_mib_Dot11WEPDefaultKeysEntry_stru ast_wlan_mib_wep_dflt_key[WLAN_NUM_DOT11WEPDEFAULTKEYVALUE];
    wlan_mib_dot11RSNAConfigEntry_stru st_wlan_mib_rsna_cfg;
} wlan_mib_Dot11PrivacyEntry_stru;

/**************************************************************************************/
/* dot11RSNAConfigPasswordValue TABLE  - members of Dot11RSNAConfigPasswordValueEntry */
/**************************************************************************************/
/* When SAE authentication is the selected AKM suite,     */
/* this table is used to locate the binary representation */
/* of a shared, secret, and potentially low-entropy word, */
/* phrase, code, or key that will be used as the          */
/* authentication credential between a TA/RA pair.        */
/* This table is logically write-only. Reading this table */
/* returns unsuccessful status or null or zero."          */
typedef struct {
    uint32_t dot11RSNAConfigPasswordValueIndex; /* dot11RSNAConfigPasswordValueIndex Unsigned32,   */
    uint8_t uc_dot11RSNAConfigPasswordCredential;  /* dot11RSNAConfigPasswordCredential OCTET STRING, */
    uint8_t auc_dot11RSNAConfigPasswordPeerMac[NUM_6_BYTES]; /* dot11RSNAConfigPasswordPeerMac MacAddress */
} wlan_mib_Dot11RSNAConfigPasswordValueEntry_stru;

/****************************************************************************/
/* dot11RSNAConfigDLCGroup TABLE  - members of Dot11RSNAConfigDLCGroupEntry */
/****************************************************************************/
/* This table gives a prioritized list of domain parameter set   */
/* Identifiers for discrete logarithm cryptography (DLC) groups. */
typedef struct {
    uint32_t dot11RSNAConfigDLCGroupIndex;      /* dot11RSNAConfigDLCGroupIndex Unsigned32,      */
    uint32_t dot11RSNAConfigDLCGroupIdentifier; /* dot11RSNAConfigDLCGroupIdentifier Unsigned32  */
} wlan_mib_Dot11RSNAConfigDLCGroupEntry_stru;

/**************************************************************************************/
/* dot11FastBSSTransitionConfig TABLE  - members of Dot11FastBSSTransitionConfigEntry */
/**************************************************************************************/
/* The table containing fast BSS transition configuration objects                     */
typedef struct {
    oal_bool_enum_uint8 en_dot11FastBSSTransitionActivated : 1; /* dot11FastBSSTransitionActivated TruthValue, */
    oal_bool_enum_uint8 en_dot11FTOverDSActivated : 1;          /* dot11FTOverDSActivated TruthValue,          */
    oal_bool_enum_uint8 en_dot11FTResourceRequestSupported : 1; /* dot11FTResourceRequestSupported TruthValue, */
    oal_bool_enum_uint8 en_resv : 5;

    uint8_t resv;
    uint16_t us_dot11FTMobilityDomainID; /* dot11FTMobilityDomainID OCTET STRING,SIZE(2) */
} wlan_mib_Dot11FastBSSTransitionConfigEntry_stru;
#endif /* end of wlan_mib_privacy.h */
