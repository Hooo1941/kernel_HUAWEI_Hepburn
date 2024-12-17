/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素RadioMeasurement Request结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_RMRQST_H
#define WLAN_MIB_RMRQST_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

typedef enum {
    WLAN_MIB_RMRQST_TYPE_CH_LOAD = 3,
    WLAN_MIB_RMRQST_TYPE_NOISE_HISTOGRAM = 4,
    WLAN_MIB_RMRQST_TYPE_BEACON = 5,
    WLAN_MIB_RMRQST_TYPE_FRAME = 6,
    WLAN_MIB_RMRQST_TYPE_STA_STATISTICS = 7,
    WLAN_MIB_RMRQST_TYPE_LCI = 8,
    WLAN_MIB_RMRQST_TYPE_TRANS_STREAM = 9,
    WLAN_MIB_RMRQST_TYPE_PAUSE = 255,

    WLAN_MIB_RMRQST_TYPE_BUTT
} wlan_mib_rmrqst_type_enum;

/* dot11RMRqstBeaconRqstMode INTEGER{  passive(0), active(1),beaconTable(2) */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_MODE_PASSIVE = 0,
    WLAN_MIB_RMRQST_BEACONRQST_MODE_ACTIVE = 1,
    WLAN_MIB_RMRQST_BEACON_MODE_BEACON_TABLE = 2,

    WLAN_MIB_RMRQST_BEACONRQST_MODE_BUTT
} wlan_mib_rmrqst_beaconrqst_mode_enum;

/* dot11RMRqstBeaconRqstDetail INTEGER {noBody(0),fixedFieldsAndRequestedElements(1),allBody(2) } */
typedef enum {
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_NOBODY = 0,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_FIXED_FLDANDELMT = 1,
    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_ALLBODY = 2,

    WLAN_MIB_RMRQST_BEACONRQST_DETAIL_BUTT
} wlan_mib_rmrqst_beaconrqst_detail_enum;

/* dot11RMRqstFrameRqstType INTEGER { frameCountRep(1) } */
typedef enum {
    WLAN_MIB_RMRQST_FRAMERQST_TYPE_FRAME_COUNTREP = 1,

    WLAN_MIB_RMRQST_FRAMERQST_TYPE_BUTT
} wlan_mib_rmrqst_famerqst_type_enum;

typedef enum {
    WLAN_MIB_RMRQST_BEACONRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_ABS_THRESHOLD = 1,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_ABS_THRESHOLD = 2,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_ABS_THRESHOLD = 3,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_ABS_THRESHOLD = 4,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_ABOVE_OFFSET_THRESHOLD = 5,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_BELOW_OFFSET_THRESHOLD = 6,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_ABOVE_OFFSET_THRESHOLD = 7,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_BELOW_OFFSET_THRESHOLD = 8,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCPI_IN_BOUND = 9,
    WLAN_MIB_RMRQST_BEACONRPT_CDT_RCNI_IN_BOUND = 10,

    WLAN_MIB_RMRQST_BEACONRPT_CDT_BUTT
} wlan_mib_rmrqst_beaconrpt_cdt_enum;

typedef enum {
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_TABLE = 0,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_MAC_STATS = 1,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP0 = 2,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP1 = 3,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP2 = 4,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP3 = 5,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP4 = 6,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP5 = 7,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP6 = 8,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_QOS_COUNTER_TABLE_UP7 = 9,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BSS_AVERG_ACCESS_DELAY = 10,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR31 = 11,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR32 = 12,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR33 = 13,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR34 = 14,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_COUNTER_GRP3_FOR35 = 15,
    WLAN_MIB_RMRQST_STASTATRQST_GRPID_RSNA_STATS_TABLE = 16,

    WLAN_MIB_RMRQST_STASTATRQST_GRPID_BUTT
} wlan_mib_rmrqst_stastatrqst_grpid_enum;

/* dot11RMRqstLCIRqstSubject OBJECT-TYPE  */
/* SYNTAX INTEGER { local(0), remote(1) } */
typedef enum {
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_LOCAL = 0,
    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_REMOTE = 1,

    WLAN_MIB_RMRQST_LCIRQST_SUBJECT_BUTT
} wlan_mib_rmrqst_lcirpt_subject_enum;

/* dot11RMRqstLCIAzimuthType OBJECT-TYPE                  */
/* SYNTAX INTEGER { frontSurfaceofSta(0), radioBeam(1) }  */
typedef enum {
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_RMRQST_LCIAZIMUTH_TYPE_BUTT
} wlan_mib_rmrqst_lciazimuth_type_enum;

/* dot11RMRqstChannelLoadReportingCondition OBJECT-TYPE   */
/* SYNTAX INTEGER {                                       */
/* afterEveryMeasurement(0),                             */
/* chanLoadAboveReference(1),                            */
/* chanLoadBelowReference(2) }                           */
typedef enum {
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_CH_LOAD_BELOW_REF = 2,

    WLAN_MIB_RMRQST_CH_LOADRPT_CDT_BUTT
} wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum;

/* dot11RMRqstNoiseHistogramReportingCondition OBJECT-TYPE  */
/* SYNTAX INTEGER {                                         */
/* afterEveryMeasurement(0),                                */
/* aNPIAboveReference(1),                                   */
/* aNPIBelowReference(2) }                                  */
typedef enum {
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_AFTER_EVERY_MEAS = 0,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_ABOVE_REF = 1,
    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_ANPI_BELOW_REF = 2,

    WLAN_MIB_RMRQST_NOISE_HISTGRPT_CDT_BUTT
} wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum;

/***********************************************************************/
/* dot11RMRequest TABLE  - members of dot11RadioMeasurement         */
/***********************************************************************/
typedef struct {
    /* dot11RMRequestNextIndex ::= { dot11RMRequest 1 } */
    uint32_t dot11RMRequestNextIndex; /* dot11RMRequestNextIndex  Unsigned32 */

    /*  dot11RMRequestTable OBJECT-TYPE ::= { dot11RMRequest 2 } */
    uint32_t dot11RMRqstIndex;
    uint8_t en_dot11RMRqstRowStatus;   /* wlan_mib_row_status_enum */
    uint8_t auc_dot11RMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11RMRqstRepetitions;
    uint32_t dot11RMRqstIfIndex;
    uint16_t en_dot11RMRqstType;       /* wlan_mib_rmrqst_type_enum */
    uint8_t auc_dot11RMRqstTargetAdd[NUM_6_BYTES];
    uint32_t dot11RMRqstTimeStamp;
    uint32_t dot11RMRqstChanNumber;
    uint32_t dot11RMRqstOperatingClass;
    uint32_t dot11RMRqstRndInterval;
    uint32_t dot11RMRqstDuration;
    oal_bool_enum_uint8 en_dot11RMRqstParallel;
    oal_bool_enum_uint8 en_dot11RMRqstEnable;
    oal_bool_enum_uint8 en_dot11RMRqstRequest;
    oal_bool_enum_uint8 en_dot11RMRqstReport;
    oal_bool_enum_uint8 en_dot11RMRqstDurationMandatory;
    uint8_t en_dot11RMRqstBeaconRqstMode;              /* wlan_mib_rmrqst_beaconrqst_mode_enum */
    uint8_t en_dot11RMRqstBeaconRqstDetail;            /* wlan_mib_rmrqst_beaconrqst_detail_enum */
    uint8_t en_dot11RMRqstFrameRqstType;               /* wlan_mib_rmrqst_famerqst_type_enum */
    uint8_t auc_dot11RMRqstBssid[NUM_6_BYTES];
    uint8_t uc_dot11RMRqstSSID[NUM_32_BYTES];
    uint8_t en_dot11RMRqstBeaconReportingCondition;    /* wlan_mib_rmrqst_beaconrpt_cdt_enum */
    int32_t l_dot11RMRqstBeaconThresholdOffset;
    uint8_t en_dot11RMRqstSTAStatRqstGroupID;          /* wlan_mib_rmrqst_stastatrqst_grpid_enum */
    uint8_t en_dot11RMRqstLCIRqstSubject;              /* wlan_mib_rmrqst_lcirpt_subject_enum */
    uint32_t dot11RMRqstLCILatitudeResolution;
    uint32_t dot11RMRqstLCILongitudeResolution;
    uint32_t dot11RMRqstLCIAltitudeResolution;
    uint8_t en_dot11RMRqstLCIAzimuthType;              /* wlan_mib_rmrqst_lciazimuth_type_enum */
    uint32_t dot11RMRqstLCIAzimuthResolution;
    uint32_t dot11RMRqstPauseTime;
    uint8_t auc_dot11RMRqstTransmitStreamPeerQSTAAddress[NUM_6_BYTES];
    uint32_t dot11RMRqstTransmitStreamTrafficIdentifier;
    uint32_t dot11RMRqstTransmitStreamBin0Range;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSAverageCondition;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSConsecutiveCondition;
    oal_bool_enum_uint8 en_dot11RMRqstTrigdQoSDelayCondition;
    uint32_t dot11RMRqstTrigdQoSAverageThreshold;
    uint32_t dot11RMRqstTrigdQoSConsecutiveThreshold;
    uint32_t dot11RMRqstTrigdQoSDelayThresholdRange;
    uint32_t dot11RMRqstTrigdQoSDelayThreshold;
    uint32_t dot11RMRqstTrigdQoSMeasurementCount;
    uint32_t dot11RMRqstTrigdQoSTimeout;
    uint8_t en_dot11RMRqstChannelLoadReportingCondition;    /* wlan_mib_rmrqst_ch_loadrpt_cdt_type_enum */
    uint32_t dot11RMRqstChannelLoadReference;
    uint8_t en_dot11RMRqstNoiseHistogramReportingCondition; /* wlan_mib_rmrqst_noise_histgrpt_cdt_type_enum */
    uint32_t dot11RMRqstAnpiReference;
    uint8_t auc_dot11RMRqstAPChannelReport[NUM_255_BYTES];
    uint8_t auc_dot11RMRqstSTAStatPeerSTAAddress[NUM_6_BYTES];
    uint8_t auc_dot11RMRqstFrameTransmitterAddress[NUM_6_BYTES];
    uint8_t auc_dot11RMRqstVendorSpecific[NUM_255_BYTES];
    uint32_t dot11RMRqstSTAStatTrigMeasCount;
    uint32_t dot11RMRqstSTAStatTrigTimeout;
    uint8_t auc_dot11RMRqstSTAStatTrigCondition[NUM_2_BYTES];
    uint32_t dot11RMRqstSTAStatTrigSTAFailedCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAFCSErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAMultRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAFrameDupeCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTARTSFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTAAckFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigSTARetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSTrigCondition;
    uint32_t dot11RMRqstSTAStatTrigQoSFailedCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSMultRetryCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSFrameDupeCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSRTSFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSAckFailCntThresh;
    uint32_t dot11RMRqstSTAStatTrigQoSDiscardCntThresh;
    uint8_t auc_dot11RMRqstSTAStatTrigRsnaTrigCondition[NUM_2_BYTES];
    uint32_t dot11RMRqstSTAStatTrigRsnaCMACICVErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCMACReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaRobustCCMPReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaTKIPICVErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaTKIPReplayCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCCMPDecryptErrCntThresh;
    uint32_t dot11RMRqstSTAStatTrigRsnaCCMPReplayCntThresh;
} wlan_mib_dot11RadioMeasurement_stru;

#endif /* end of wlan_mib_rmrqst.h */
