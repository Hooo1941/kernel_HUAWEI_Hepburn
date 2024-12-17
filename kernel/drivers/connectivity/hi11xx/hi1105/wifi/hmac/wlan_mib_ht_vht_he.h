/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素HT VHT HE结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_HT_VHT_HE_H
#define WLAN_MIB_HT_VHT_HE_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

/* dot11HTProtection 枚举定义 */
typedef enum {
    WLAN_MIB_HT_NO_PROTECTION = 0,
    WLAN_MIB_HT_NONMEMBER_PROTECTION = 1,
    WLAN_MIB_HT_20MHZ_PROTECTION = 2,
    WLAN_MIB_HT_NON_HT_MIXED = 3,

    WLAN_MIB_HT_PROTECTION_BUTT
} wlan_mib_ht_protection_enum;

/* dot11ExplicitCompressedBeamformingFeedbackOptionImplemented OBJECT-TYPE */
/* SYNTAX INTEGER {                                                        */
/* inCapable (0),                                                          */
/* delayed (1),                                                            */
/* immediate (2),                                                          */
/* unsolicitedImmediate (3),                                               */
/* aggregated (4),                                                         */
/* delayedAggregated (5),                                                  */
/* immediateAggregated (6),                                                */
/* unsolicitedImmediateAggregated (7)}                                     */
typedef enum {
    WLAN_MIB_HT_ECBF_INCAPABLE = 0,
    WLAN_MIB_HT_ECBF_DELAYED = 1,
    WLAN_MIB_HT_ECBF_IMMEDIATE = 2,

    /* Tip:if you select it(3) and use for HT cap, it stands for both 1 and 2. page656 */
    WLAN_MIB_HT_ECBF_UNSOLIMMEDI = 3,
    WLAN_MIB_HT_ECBF_AGGREGATE = 4,
    WLAN_MIB_HT_ECBF_DELAYAGGR = 5,
    WLAN_MIB_HT_ECBF_IMMEDIAGGR = 6,
    WLAN_MIB_HT_ECBF_UNSOLIMMAGGR = 7,

    WLAN_MIB_HT_ECBF_TYPE_BUTT
} wlan_mib_ht_ecbf_enum;

/* VHT Capabilities Info field 最大MPDU长度枚举 */
typedef enum {
    WLAN_MIB_VHT_MPDU_3895 = 0,
    WLAN_MIB_VHT_MPDU_7991 = 1,
    WLAN_MIB_VHT_MPDU_11454 = 2,

    WLAN_MIB_VHT_MPDU_LEN_BUTT
} wlan_mib_vht_mpdu_len_enum;

/* VHT Capabilites Info field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_SUPP_WIDTH_80 = 0,       /* 不支持160或者80+80 */
    WLAN_MIB_VHT_SUPP_WIDTH_160 = 1,      /* 支持160 */
    WLAN_MIB_VHT_SUPP_WIDTH_80PLUS80 = 2, /* 支持160和80+80 */

    WLAN_MIB_VHT_SUPP_WIDTH_BUTT
} wlan_mib_vht_supp_width_enum;

/* VHT Operation Info  field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_VHT_OP_WIDTH_20_40 = 0,    /* 工作在20/40 */
    WLAN_MIB_VHT_OP_WIDTH_80 = 1,       /* 工作在80 */
    WLAN_MIB_VHT_OP_WIDTH_160 = 2,      /* 工作在160 */
    WLAN_MIB_VHT_OP_WIDTH_80PLUS80 = 3, /* 工作在80+80 */

    WLAN_MIB_VHT_OP_WIDTH_BUTT
} wlan_mib_vht_op_width_enum;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/* HE Operation -> 6GHz Operation Info  field 支持带宽枚举 */
typedef enum {
    WLAN_MIB_HE_6GHZ_OP_WIDTH_20 = 0,    /* 工作在20 */
    WLAN_MIB_HE_6GHZ_OP_WIDTH_40 = 1,    /* 工作在40 */
    WLAN_MIB_HE_6GHZ_OP_WIDTH_80 = 2,    /* 工作在80 */
    WLAN_MIB_HE_6GHZ_OP_WIDTH_80PLUS80_160 = 3,   /* 工作在80+80或160 */

    WLAN_MIB_HE_6GHZ_OP_WIDTH_BUTT
} wlan_mib_he_6ghz_op_width_enum;
#endif

typedef struct {
    uint8_t uc_mcs_nums;
    uint8_t *pst_mcs_set;
} wlan_mib_ht_op_mcs_set_stru; /* dot11HTOperationalMCSSet */

/**************************************************************************************/
/* dot11HTStationConfig TABLE  - members of Dot11HTStationConfigEntry                 */
/**************************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple            */
/* instances on an agent.                                                             */
typedef struct {
    oal_bool_enum_uint8 en_dot11HTControlFieldSupported : 1;             /* dot11HTControlFieldSupported TruthValue */
    oal_bool_enum_uint8 en_dot11RDResponderOptionImplemented : 1;        /* dot11RDResponderOptionImplemented */
    oal_bool_enum_uint8 en_dot11PCOOptionImplemented : 1;                /* dot11PCOOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11LsigTxopProtectionOptionImplemented : 1; /* dot11LsigTxopProtectionOptionImplemented */
    oal_bool_enum_uint8 en_dot11HighThroughputOptionImplemented : 1;     /* dot11HighThroughputOptionImplemented */
    uint8_t dot11MinimumMPDUStartSpacing : 3;                       /* dot11MinimumMPDUStartSpacing Unsigned32 */
    uint8_t resv[3];

    uint16_t en_dot11MaxAMSDULength; /* wlan_mib_max_amsdu_lenth_enum: dot11MaxAMSDULength INTEGER */
    uint8_t en_dot11MIMOPowerSave;   /* wlan_mib_mimo_power_save_enum: dot11MIMOPowerSave INTEGER */
    uint8_t en_dot11MCSFeedbackOptionImplemented; /* wlan_mib_mcs_feedback_opt_implt_enum */
    // wlan_mib_ht_op_mcs_set_stru st_dot11HTOperationalMCSSet; /* dot11HTOperationalMCSSet OCTET STRING */
    // oal_bool_enum_uint8 en_dot11NDelayedBlockAckOptionImplemented; /* dot11NDelayedBlockAckOptionImplemented */
    // oal_bool_enum_uint8 en_dot11STBCControlFrameOptionImplemented; /* dot11STBCControlFrameOptionImplemented */
    uint32_t dot11MaxRxAMPDUFactor; /* dot11MaxRxAMPDUFactor Unsigned32 */
    uint32_t dot11TransitionTime;   /* dot11TransitionTime Unsigned32 */
    // oal_bool_enum_uint8 en_dot11SPPAMSDUCapable; /* dot11SPPAMSDUCapable TruthValue */
    // oal_bool_enum_uint8 en_dot11SPPAMSDURequired; /* dot11SPPAMSDURequired TruthValue */
    // oal_bool_enum_uint8 en_dot11FortyMHzOptionImplemented; /* dot11FortyMHzOptionImplemented TruthValue */
} wlan_mib_Dot11HTStationConfigEntry_stru;

/****************************************************************************/
/* dot11VHTStationConfig TABLE  - members of Dot11VHTStationConfigEntry */
/****************************************************************************/
/* Station Configuration attributes. In tabular form to allow for multiple   */
/* instances on an agent. */
typedef struct {
    uint32_t dot11MaxMPDULength;                 /* dot11MaxMPDULength INTEGER, */
    uint32_t dot11VHTMaxRxAMPDUFactor;           /* dot11VHTMaxRxAMPDUFactor Unsigned32, */
    uint32_t dot11VHTRxHighestDataRateSupported; /* dot11VHTRxHighestDataRateSupported Unsigned32, */
    uint32_t dot11VHTTxHighestDataRateSupported; /* dot11VHTTxHighestDataRateSupported Unsigned32, */
    uint16_t us_dot11VHTTxMCSMap;                   /* dot11VHTTxMCSMap OCTET STRING, */
    uint16_t us_dot11VHTRxMCSMap;                   /* dot11VHTRxMCSMap OCTET STRING, */
    // uint32_t          dot11VHTOBSSScanCount;                      /* dot11VHTOBSSScanCount Unsigned32 */
    oal_bool_enum_uint8 en_dot11VHTControlFieldSupported;             /* dot11VHTControlFieldSupported TruthValue */
    oal_bool_enum_uint8 en_dot11VHTTXOPPowerSaveOptionImplemented;    /* dot11VHTTXOPPowerSaveOptionImplemented */
    oal_bool_enum_uint8 en_dot11VHTOptionImplemented;                 /* dot11VHTOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11OperatingModeNotificationImplemented; /* dot11OperatingModeNotificationImplemented */
} wlan_mib_Dot11VHTStationConfigEntry_stru;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
typedef struct {
    uint32_t dot11HERxHighestDataRateSupported; /* dot11HERxHighestDataRateSupported Unsigned32, */
    uint32_t dot11HETxHighestDataRateSupported; /* dot11HETxHighestDataRateSupported Unsigned32, */
    uint32_t dot11HETxMCSMap;                   /* us_dot11HETxMCSMap OCTET STRING, */
    uint32_t dot11HERxMCSMap;                   /* us_dot11HERxMCSMap OCTET STRING, */

    uint8_t uc_dot11TriggerMacPaddingDuration;
    uint8_t uc_dot11MaxAMPDULengthExponent;

    oal_bool_enum_uint8 en_dot11HEOptionImplemented;  /* dot11HEOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11OMIOptionImplemented; /* en_dot11OMIOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TWTOptionActivated;   /* dot11TWTOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11OperatingModeIndication;
    oal_bool_enum_uint8 en_dot11TOFDMARandomAccess;
    oal_bool_enum_uint8 en_dot11MultiBSSIDImplemented;
    oal_bool_enum_uint8 en_dot11BSRSupport;
    oal_bool_enum_uint8 en_dot11BQRSupport; /* bandwidth query report support */
} wlan_mib_Dot11HEStationConfigEntry_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11HEDualBandSupport;
    uint8_t uc_dot11HEChannelWidthSet;
    uint8_t uc_dot11HighestNSS;
    uint8_t uc_dot11HighestMCS;
    oal_bool_enum_uint8 en_dot11HELDPCCodingInPayload;
    oal_bool_enum_uint8 en_dot11SUBeamformer;
    oal_bool_enum_uint8 en_dot11SUBeamformee;
    oal_bool_enum_uint8 en_dot11MUBeamformer;
    oal_bool_enum_uint8 en_dot11SUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11MUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11HESTBCTxOver80MHz;
    oal_bool_enum_uint8 en_dot11HESTBCRxOver80MHz;
    oal_bool_enum_uint8 en_dot11SRPBasedSR;
    oal_bool_enum_uint8 en_dot11HESTBCTxBelow80MHz;
    oal_bool_enum_uint8 en_dot11HESTBCRxBelow80MHz;
} wlan_mid_Dot11HePhyCapability_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11PPEThresholdsRequired;
    uint8_t uc_dot11HEBeamformeeSTSBelow80Mhz;
    uint8_t uc_dot11HEBeamformeeSTSOver80Mhz;
    uint8_t uc_dot11HENumberSoundingDimensionsBelow80Mhz;
    uint8_t uc_dot11HENumberSoundingDimensionsOver80Mhz;
    oal_bool_enum_uint8 en_dot11Ng16SUFeedback;
    oal_bool_enum_uint8 en_dot11Ng16MUFeedback;
    oal_bool_enum_uint8 en_dot11Codebook42SUFeedback;
    oal_bool_enum_uint8 en_dot11Codebook75MUFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredSUBeamformingFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredMUBeamformingPartialBWFeedback;
    oal_bool_enum_uint8 en_dot11TriggeredCQIFeedback;
    oal_bool_enum_uint8 en_dot11HESUPPDUwith1xHELTFand0point8GIlmplemented;
    uint8_t auc_rsv[NUM_2_BYTES]; /* 2字节reserve */
} wlan_mid_Dot11HePhyCapability_rom_stru;

typedef struct {
    oal_bool_enum_uint8 en_dot11MultiBSSIDActived;
    uint8_t auc_rsv[NUM_3_BYTES];
} wlan_mib_Dot11HEStationConfigEntry_rom_stru;
#endif
#endif /* end of wlan_mib_HT_VHT_HE.h */
