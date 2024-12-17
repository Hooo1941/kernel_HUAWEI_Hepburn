/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义所有MIB元素结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2012年10月18日
 */

#ifndef WLAN_MIB_H
#define WLAN_MIB_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"
#include "wlan_mib_wnm.h"
#include "wlan_mib_rmrqst.h"
#include "wlan_mib_phy_entry.h"
#include "wlan_mib_wireless.h"
#include "wlan_mib_ht_vht_he.h"
#include "wlan_mib_privacy.h"

#define WAL_HIPRIV_RSSI_DEFAULT_THRESHOLD (-95) /* 默认RSSI门限 -95dbm */

/* RowStatus ::= TEXTUAL-CONVENTION                                         */
/* The status column has six  values:`active', `notInService',`notReady',   */
/* `createAndGo', `createAndWait', `destroy'  as described in   rfc2579     */
typedef enum {
    WLAN_MIB_ROW_STATUS_ACTIVE = 1,
    WLAN_MIB_ROW_STATUS_NOT_INSERVICE,
    WLAN_MIB_ROW_STATUS_NOT_READY,
    WLAN_MIB_ROW_STATUS_CREATE_AND_GO,
    WLAN_MIB_ROW_STATUS_CREATE_AND_WAIT,
    WLAN_MIB_ROW_STATUS_DEATROY,

    WLAN_MIB_ROW_STATUS_BUTT
} wlan_mib_row_status_enum;

typedef enum {
    WLAN_MIB_PWR_MGMT_MODE_ACTIVE = 1,
    WLAN_MIB_PWR_MGMT_MODE_PWRSAVE = 2,

    WLAN_MIB_PWR_MGMT_MODE_BUTT
} wlan_mib_pwr_mgmt_mode_enum;

typedef enum {
    WLAN_MIB_DESIRED_BSSTYPE_INFRA = 1,
    WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT = 2,
    WLAN_MIB_DESIRED_BSSTYPE_ANY = 3,

    WLAN_MIB_DESIRED_BSSTYPE_BUTT
} wlan_mib_desired_bsstype_enum;

/* dot11LCIDSEAltitudeType OBJECT-TYPE              */
/* SYNTAX INTEGER { meters(1), floors(2), hagm(3) } */
typedef enum {
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_METERS = 1,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_FLOORS = 2,
    WLAN_MIB_LCI_DSEALTITUDE_TYPE_HAGM = 3,

    WLAN_MIB_LCI_DSEALTITUDE_TYPE_BUTT
} wlan_mib_lci_dsealtitude_type_enum;

/* dot11MIMOPowerSave OBJECT-TYPE                    */
/* SYNTAX INTEGER { static(1), dynamic(2), mimo(3) } */
typedef enum {
    WLAN_MIB_MIMO_POWER_SAVE_STATIC = 1,
    WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC = 2,
    WLAN_MIB_MIMO_POWER_SAVE_MIMO = 3,

    WLAN_MIB_MIMO_POWER_SAVE_BUTT
} wlan_mib_mimo_power_save_enum;

/* dot11MaxAMSDULength OBJECT-TYPE            */
/* SYNTAX INTEGER { short(3839), long(7935) } */
typedef enum {
    WLAN_MIB_MAX_AMSDU_LENGTH_SHORT = 3839,
    WLAN_MIB_MAX_AMSDU_LENGTH_LONG = 7935,

    WLAN_MIB_MAX_AMSDU_LENGTH_BUTT
} wlan_mib_max_amsdu_lenth_enum;

/* dot11MCSFeedbackOptionImplemented OBJECT-TYPE         */
/* SYNTAX INTEGER { none(0), unsolicited (2), both (3) } */
typedef enum {
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_NONE = 0,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_UNSOLICITED = 2,
    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BOTH = 3,

    WLAN_MIB_MCS_FEEDBACK_OPT_IMPLT_BUTT
} wlan_mib_mcs_feedback_opt_implt_enum;

/* dot11LocationServicesLIPReportIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                                        */
/* hours(0),                                               */
/* minutes(1),                                             */
/* seconds(2),                                             */
/* milliseconds(3)                                         */
typedef enum {
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_HOURS = 0,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MINUTES = 1,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_SECONDS = 2,
    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_MILLISECDS = 3,

    WLAN_MIB_LCT_SERVS_LIPRPT_INTERVAL_UNIT_BUTT
} wlan_mib_lct_servs_liprpt_interval_unit_enum;

/* dot11APLCIDatum OBJECT-TYPE */
/* SYNTAX INTEGER {            */
/* wgs84 (1),                  */
/* nad83navd88 (2),            */
/* nad93mllwvd (3)             */
typedef enum {
    WLAN_MIB_AP_LCI_DATUM_WGS84 = 1,
    WLAN_MIB_AP_LCI_DATUM_NAD83_NAVD88 = 2,
    WLAN_MIB_AP_LCI_DATUM_NAD93_MLLWVD = 3,

    WLAN_MIB_AP_LCI_DATUM_BUTT
} wlan_mib_ap_lci_datum_enum;

/* dot11APLCIAzimuthType OBJECT-TYPE */
/* SYNTAX INTEGER {                  */
/* frontSurfaceOfSTA(0),             */
/* radioBeam(1) }                    */
typedef enum {
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_FRONT_SURFACE_STA = 0,
    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_RADIO_BEAM = 1,

    WLAN_MIB_AP_LCI_AZIMUTH_TYPE_BUTT
} wlan_mib_ap_lci_azimuth_type_enum;

/* 当前MIB表项还未稳定，暂不需要4字节对齐，待后续MIB项稳定后再来调整这些结构体 */
/* 是否需要该结构体表示变长的字符串，待确定 */
typedef struct {
    uint16_t us_octet_nums;    /* 字节长度，此值为0时表示没有字符串信息 */
    uint8_t *puc_octec_string; /* 字符串起始地址 */
} wlan_mib_octet_string_stru;

/* dot11LocationServicesLocationIndicationIndicationParameters OBJECT-TYPE */
/* SYNTAX OCTET STRING (SIZE (1..255))                                     */
/* MAX-ACCESS read-create                                                  */
/* STATUS current                                                          */
/* DESCRIPTION                                                             */
/* "This attribute indicates the location Indication Parameters used for   */
/* transmitting Location Track Notification Frames."                       */
/* ::= { dot11LocationServicesEntry 15}                                    */
typedef struct {
    uint8_t uc_para_nums;
    uint8_t *uc_para;
} wlan_mib_local_serv_location_ind_ind_para_stru;

/******************************************************************************/
/* dot11MultiDomainCapability TABLE - members of Dot11MultiDomainCapabilityEntry            */
/******************************************************************************/
/* GThis (conceptual) table of attributes for cross-domain mobility           */
typedef struct {
    uint32_t dot11MultiDomainCapabilityIndex; /* dot11MultiDomainCapabilityIndex Unsigned32, */
    uint32_t dot11FirstChannelNumber;         /* dot11FirstChannelNumber Unsigned32,         */
    uint32_t dot11NumberofChannels;           /* dot11NumberofChannels Unsigned32,           */
    int32_t l_dot11MaximumTransmitPowerLevel;    /* dot11MaximumTransmitPowerLevel Integer32    */
} wlan_mib_Dot11MultiDomainCapabilityEntry_stru;

/******************************************************************************/
/* dot11SpectrumManagement TABLE - members of dot11SpectrumManagementEntry    */
/******************************************************************************/
/* An entry (conceptual row) in the Spectrum Management Table.                */
/* IfIndex - Each 802.11 interface is represented by an ifEntry. Interface    */
/* tables in this MIB are indexed by ifIndex.                                 */
typedef struct {
    uint32_t dot11SpectrumManagementIndex;     /* dot11SpectrumManagementIndex Unsigned32,      */
    int32_t l_dot11MitigationRequirement;         /* dot11MitigationRequirement Integer32,         */
    uint32_t dot11ChannelSwitchTime;           /* dot11ChannelSwitchTime Unsigned32,            */
    int32_t l_dot11PowerCapabilityMaxImplemented; /* dot11PowerCapabilityMaxImplemented Integer32, */
    int32_t l_dot11PowerCapabilityMinImplemented; /* dot11PowerCapabilityMinImplemented Integer32  */
} wlan_mib_dot11SpectrumManagementEntry_stru;

/****************************************************************************/
/* dot11RSNAStats TABLE - members of Dot11RSNAStatsEntry                    */
/****************************************************************************/
/* This table maintains per-STA statistics in an RSN. The entry with        */
/* dot11RSNAStatsSTAAddress equal to FF-FF-FF-FF-FF-FF contains statistics  */
/* for group addressed traffic                                              */
/***********************************************************************/
/* dot11OperatingClasses TABLE - members of Dot11OperatingClassesEntry */
/***********************************************************************/
/* (Conceptual) table of attributes for operating classes       */
typedef struct {
    uint32_t dot11OperatingClassesIndex; /* dot11OperatingClassesIndex Unsigned32,   */
    uint32_t dot11OperatingClass;        /* dot11OperatingClass Unsigned32,          */
    uint32_t dot11CoverageClass;         /* dot11CoverageClass Unsigned32            */
} wlan_mib_Dot11OperatingClassesEntry_stru;

/************************************************************************/
/* dot11LCIDSE TABLE  - members of Dot11LCIDSEEntry                     */
/************************************************************************/
/* Group contains conceptual table of attributes for Dependent Station  */
/* Enablement.                                                          */
typedef struct {
    uint32_t dot11LCIDSEIndex;
    uint32_t dot11LCIDSEIfIndex;
    uint32_t dot11LCIDSECurrentOperatingClass;
    uint32_t dot11LCIDSELatitudeResolution;
    int32_t l_dot11LCIDSELatitudeInteger;
    int32_t l_dot11LCIDSELatitudeFraction;
    uint32_t dot11LCIDSELongitudeResolution;
    int32_t l_dot11LCIDSELongitudeInteger;
    int32_t l_dot11LCIDSELongitudeFraction;
    uint8_t en_dot11LCIDSEAltitudeType;          /* wlan_mib_lci_dsealtitude_type_enum */
    uint32_t dot11LCIDSEAltitudeResolution;
    int32_t l_dot11LCIDSEAltitudeInteger;
    int32_t l_dot11LCIDSEAltitudeFraction;
    uint32_t dot11LCIDSEDatum;
    oal_bool_enum_uint8 en_dot11RegLocAgreement;
    oal_bool_enum_uint8 en_dot11RegLocDSE;
    oal_bool_enum_uint8 en_dot11DependentSTA;
    uint32_t dot11DependentEnablementIdentifier;
    uint32_t dot11DSEEnablementTimeLimit;
    uint32_t dot11DSEEnablementFailHoldTime;
    uint32_t dot11DSERenewalTime;
    uint32_t dot11DSETransmitDivisor;
} wlan_mib_Dot11LCIDSEEntry_stru;

/* dot11LocationServicesLocationIndicationChannelList OBJECT-TYPE         */
/* SYNTAX OCTET STRING (SIZE (2..254))                                    */
/* MAX-ACCESS read-create                                                 */
/* STATUS current                                                         */
/* DESCRIPTION                                                            */
/* "This attribute contains one or more Operating Class and Channel octet */
/* pairs."                                                                */
/* ::= { dot11LocationServicesEntry 12}                                   */
typedef struct {
    uint8_t uc_channel_nums;
    uint8_t *uc_channel_list;
} wlan_mib_local_serv_location_ind_ch_list_stru;

/********************************************************************************/
/* dot11LocationServices TABLE  - members of Dot11LocationServicesEntry         */
/********************************************************************************/
/* Identifies a hint for the next value of dot11LocationServicesIndex to be     */
/* used in a row creation attempt for dot11LocationServicesTable. If no new     */
/* rows can be created for some reason, such as memory, processing require-     */
/* ments, etc, the SME shall set this attribute to 0. It shall update this      */
/* attribute to a proper value other than 0 as soon as it is capable of         */
/* receiving new measurement requests. The nextIndex is not necessarily         */
/* sequential nor monotonically increasing.                                     */
typedef struct {
    uint32_t dot11LocationServicesIndex;
    uint8_t auc_dot11LocationServicesMACAddress[NUM_6_BYTES];
    uint8_t auc_dot11LocationServicesLIPIndicationMulticastAddress[NUM_6_BYTES];
    uint8_t en_dot11LocationServicesLIPReportIntervalUnits; /* wlan_mib_lct_servs_liprpt_interval_unit_enum */
    uint32_t dot11LocationServicesLIPNormalReportInterval;
    uint32_t dot11LocationServicesLIPNormalFramesperChannel;
    uint32_t dot11LocationServicesLIPInMotionReportInterval;
    uint32_t dot11LocationServicesLIPInMotionFramesperChannel;
    uint32_t dot11LocationServicesLIPBurstInterframeInterval;
    uint32_t dot11LocationServicesLIPTrackingDuration;
    uint32_t dot11LocationServicesLIPEssDetectionInterval;
    wlan_mib_local_serv_location_ind_ch_list_stru st_dot11LocationServicesLocationIndicationChannelList;
    uint32_t dot11LocationServicesLocationIndicationBroadcastDataRate;
    uint8_t uc_dot11LocationServicesLocationIndicationOptionsUsed;
    wlan_mib_local_serv_location_ind_ind_para_stru st_dot11LocationServicesLocationIndicationIndicationParameters;
    uint32_t dot11LocationServicesLocationStatus;
} wlan_mib_Dot11LocationServicesEntry_stru;

/****************************************************************************/
/* dot11MeshSTAConfig TABLE  - members of Dot11MeshSTAConfigEntry           */
/****************************************************************************/
/* Mesh Station Configuration attributes. In tabular form to allow for mul- */
/* tiple instances on an agent.                                             */
typedef struct {
    uint8_t auc_dot11MeshID[NUM_32_BYTES];                  /* dot11MeshID OCTET STRING,   (SIZE(0..32))       */
    uint32_t dot11MeshNumberOfPeerings;                     /* dot11MeshNumberOfPeerings Unsigned32,           */
    oal_bool_enum_uint8 en_dot11MeshAcceptingAdditionalPeerings; /* dot11MeshAcceptingAdditionalPeerings TruthValue, */
    oal_bool_enum_uint8 en_dot11MeshConnectedToMeshGate;         /* dot11MeshConnectedToMeshGate TruthValue,        */
    oal_bool_enum_uint8 en_dot11MeshSecurityActivated;           /* dot11MeshSecurityActivated TruthValue,          */
    uint8_t uc_dot11MeshActiveAuthenticationProtocol;          /* dot11MeshActiveAuthenticationProtocol INTEGER,  */
    uint32_t dot11MeshMaxRetries;                           /* dot11MeshMaxRetries Unsigned32,                 */
    uint32_t dot11MeshRetryTimeout;                         /* dot11MeshRetryTimeout Unsigned32,               */
    uint32_t dot11MeshConfirmTimeout;                       /* dot11MeshConfirmTimeout Unsigned32,             */
    uint32_t dot11MeshHoldingTimeout;                       /* dot11MeshHoldingTimeout Unsigned32,             */
    uint32_t dot11MeshConfigGroupUpdateCount;               /* dot11MeshConfigGroupUpdateCount Unsigned32,     */
    uint8_t uc_dot11MeshActivePathSelectionProtocol;           /* dot11MeshActivePathSelectionProtocol INTEGER,   */
    uint8_t uc_dot11MeshActivePathSelectionMetric;             /* dot11MeshActivePathSelectionMetric INTEGER,     */
    oal_bool_enum_uint8 en_dot11MeshForwarding;                  /* dot11MeshForwarding TruthValue,                 */
    uint32_t dot11MeshTTL;                                  /* dot11MeshTTL Unsigned32,                        */
    oal_bool_enum_uint8 en_dot11MeshGateAnnouncements;           /* dot11MeshGateAnnouncements TruthValue,          */
    uint32_t dot11MeshGateAnnouncementInterval;             /* dot11MeshGateAnnouncementInterval Unsigned32,   */
    uint8_t uc_dot11MeshActiveCongestionControlMode;           /* dot11MeshActiveCongestionControlMode INTEGER,   */
    uint8_t uc_dot11MeshActiveSynchronizationMethod;           /* dot11MeshActiveSynchronizationMethod INTEGER,   */
    uint32_t dot11MeshNbrOffsetMaxNeighbor;                 /* dot11MeshNbrOffsetMaxNeighbor Unsigned32,       */
    oal_bool_enum_uint8 en_dot11MBCAActivated;                   /* dot11MBCAActivated TruthValue,                  */
    uint32_t dot11MeshBeaconTimingReportInterval;           /* dot11MeshBeaconTimingReportInterval Unsigned32, */
    uint32_t dot11MeshBeaconTimingReportMaxNum;             /* dot11MeshBeaconTimingReportMaxNum Unsigned32,   */
    uint32_t dot11MeshDelayedBeaconTxInterval;              /* dot11MeshDelayedBeaconTxInterval Unsigned32,    */
    uint32_t dot11MeshDelayedBeaconTxMaxDelay;              /* dot11MeshDelayedBeaconTxMaxDelay Unsigned32,    */
    uint32_t dot11MeshDelayedBeaconTxMinDelay;              /* dot11MeshDelayedBeaconTxMinDelay Unsigned32,    */
    uint32_t dot11MeshAverageBeaconFrameDuration;           /* dot11MeshAverageBeaconFrameDuration Unsigned32, */
    uint32_t dot11MeshSTAMissingAckRetryLimit;              /* dot11MeshSTAMissingAckRetryLimit Unsigned32,    */
    uint32_t dot11MeshAwakeWindowDuration;                  /* dot11MeshAwakeWindowDuration Unsigned32,        */
    oal_bool_enum_uint8 en_dot11MCCAImplemented;                 /* dot11MCCAImplemented TruthValue,                */
    oal_bool_enum_uint8 en_dot11MCCAActivated;                   /* dot11MCCAActivated TruthValue,                  */
    uint32_t dot11MAFlimit;                                 /* dot11MAFlimit Unsigned32,                       */
    uint32_t dot11MCCAScanDuration;                         /* dot11MCCAScanDuration Unsigned32,               */
    uint32_t dot11MCCAAdvertPeriodMax;                      /* dot11MCCAAdvertPeriodMax Unsigned32,            */
    uint32_t dot11MCCAMinTrackStates;                       /* dot11MCCAMinTrackStates Unsigned32,             */
    uint32_t dot11MCCAMaxTrackStates;                       /* dot11MCCAMaxTrackStates Unsigned32,             */
    uint32_t dot11MCCAOPtimeout;                            /* dot11MCCAOPtimeout Unsigned32,                  */
    uint32_t dot11MCCACWmin;                                /* dot11MCCACWmin Unsigned32,                      */
    uint32_t dot11MCCACWmax;                                /* dot11MCCACWmax Unsigned32,                      */
    uint32_t dot11MCCAAIFSN;                                /* dot11MCCAAIFSN Unsigned32                       */
} wlan_mib_Dot11MeshSTAConfigEntry_stru;

/*****************************************************************************/
/* dot11MeshHWMPConfig TABLE  - members of Dot11MeshHWMPConfigEntry          */
/*****************************************************************************/
/* MMesh Station HWMP Configuration attributes. In tabular form to allow for */
/* tmultiple instances on an agent.                                          */
typedef struct {
    uint32_t dot11MeshHWMPmaxPREQretries;           /* dot11MeshHWMPmaxPREQretries Unsigned32,           */
    uint32_t dot11MeshHWMPnetDiameter;              /* dot11MeshHWMPnetDiameter Unsigned32,              */
    uint32_t dot11MeshHWMPnetDiameterTraversalTime; /* dot11MeshHWMPnetDiameterTraversalTime Unsigned32,, */
    uint32_t dot11MeshHWMPpreqMinInterval;          /* dot11MeshHWMPpreqMinInterval Unsigned32,          */
    uint32_t dot11MeshHWMPperrMinInterval;          /* dot11MeshHWMPperrMinInterval Unsigned32,          */
    uint32_t dot11MeshHWMPactivePathToRootTimeout;  /* dot11MeshHWMPactivePathToRootTimeout Unsigned32,  */
    uint32_t dot11MeshHWMPactivePathTimeout;        /* dot11MeshHWMPactivePathTimeout Unsigned32,        */
    uint8_t uc_dot11MeshHWMProotMode;                  /* dot11MeshHWMProotMode INTEGER,                    */
    uint32_t dot11MeshHWMProotInterval;             /* dot11MeshHWMProotInterval Unsigned32,             */
    uint32_t dot11MeshHWMPrannInterval;             /* dot11MeshHWMPrannInterval Unsigned32,             */
    uint8_t uc_dot11MeshHWMPtargetOnly;                /* dot11MeshHWMPtargetOnly INTEGER,                  */
    uint32_t dot11MeshHWMPmaintenanceInterval;      /* dot11MeshHWMPmaintenanceInterval Unsigned32,      */
    uint32_t dot11MeshHWMPconfirmationInterval;     /* dot11MeshHWMPconfirmationInterval Unsigned32      */
} wlan_mib_Dot11MeshHWMPConfigEntry_stru;

/****************************************************************************************/
/*          Start of dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }                  */
/*              --  MAC GROUPS                                                          */
/*              --  dot11OperationTable ::= { dot11mac 1 }                              */
/*              --  dot11CountersTable ::= { dot11mac 2 }                               */
/*              --  dot11GroupAddressesTable ::= { dot11mac 3 }                         */
/*              --  dot11EDCATable ::= { dot11mac 4 }                                   */
/*              --  dot11QAPEDCATable ::= { dot11mac 5 }                                */
/*              --  dot11QosCountersTable ::= { dot11mac 6 }                            */
/*              --  dot11ResourceInfoTable    ::= { dot11mac 7 }                        */
/****************************************************************************************/
/****************************************************************************************/
/* dot11OperationTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11OperationEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group contains MAC attributes pertaining to the operation of the MAC.          */
/*      This has been implemented as a table in order to allow for multiple             */
/*      instantiations on an agent."                                                    */
/* ::= { dot11mac 1 }                                                                   */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11RIFSMode : 1;                            /* dot11RIFSMode TruthValue */
    oal_bool_enum_uint8 en_dot11DualCTSProtection : 1;                   /* dot11DualCTSProtection TruthValue */
    oal_bool_enum_uint8 en_dot11LSIGTXOPFullProtectionActivated : 1;     /* dot11LSIGTXOPFullProtectionActivated */
    oal_bool_enum_uint8 en_dot11NonGFEntitiesPresent : 1;                /* dot11NonGFEntitiesPresent TruthValue */
    oal_bool_enum_uint8 en_dot11PCOActivated : 1;                        /* dot11PCOActivated TruthValue */
    oal_bool_enum_uint8 en_dot11FortyMHzIntolerant : 1;                  /* dot11FortyMHzIntolerant TruthValue */
    oal_bool_enum_uint8 en_dot112040BSSCoexistenceManagementSupport : 1; /* dot112040BSSCoexistenceManagementSupport */
    oal_bool_enum_uint8 en_resv : 1;

    uint8_t en_dot11HTProtection; /* wlan_mib_ht_protection_enum: dot11HTProtection INTEGER */
    uint8_t resv[NUM_2_BYTES];
    uint32_t dot11RTSThreshold;                         /* dot11RTSThreshold Unsigned32 */
    uint32_t dot11FragmentationThreshold;               /* dot11FragmentationThreshold Unsigned32 */
    uint32_t dot11BSSWidthTriggerScanInterval;          /* dot11BSSWidthTriggerScanInterval Unsigned32 */
    uint32_t dot11BSSWidthChannelTransitionDelayFactor; /* dot11BSSWidthChannelTransitionDelayFactor Unsigned32 */
    uint32_t dot11OBSSScanPassiveDwell;                 /* dot11OBSSScanPassiveDwell Unsigned32 */
    uint32_t dot11OBSSScanActiveDwell;                  /* dot11OBSSScanActiveDwell Unsigned32 */
    uint32_t dot11OBSSScanPassiveTotalPerChannel;       /* dot11OBSSScanPassiveTotalPerChannel Unsigned32 */
    uint32_t dot11OBSSScanActiveTotalPerChannel;        /* dot11OBSSScanActiveTotalPerChannel Unsigned32 */
    uint32_t dot11OBSSScanActivityThreshold;            /* dot11OBSSScanActivityThreshold Unsigned32 */
} wlan_mib_Dot11OperationEntry_stru;

/****************************************************************************************/
/* dot11GroupAddressesTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11GroupAddressesEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "A conceptual table containing a set of MAC addresses identifying the mul-      */
/*      ticast-group addresses for which this STA receives frames. The default          */
/*      value of this attribute is null."                                               */
/* ::= { dot11mac 3 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11GroupAddressesIndex;                      /* dot11GroupAddressesIndex InterfaceIndex */
    uint8_t auc_dot11Address[NUM_6_BYTES];                               /* dot11Address MacAddress */
    uint8_t en_dot11GroupAddressesStatus; /* wlan_mib_row_status_enum: dot11GroupAddressesStatus RowStatus */
} wlan_mib_Dot11GroupAddressesEntry_stru;

/****************************************************************************************/
/* dot11EDCATable OBJECT-TYPE                                                           */
/* SYNTAX SEQUENCE OF Dot11EDCAEntry                                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Conceptual table for EDCA default parameter values at a non-AP STA. This       */
/*      table contains the four entries of the EDCA parameters corresponding to         */
/*      four possible ACs. Index 1 corresponds to AC_BK, index 2 to AC_BE, index 3      */
/*      to AC_VI, and index 4 to AC_VO."                                                */
/*      REFERENCE                                                                       */
/*      "IEEE 802.11-<year>, 9.2.4.2"                                                   */
/* ::= { dot11mac 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11EDCATableIndex;     /* dot11EDCATableIndex     Unsigned32 */
    uint32_t dot11EDCATableCWmin;     /* dot11EDCATableCWmin Unsigned32 */
    uint32_t dot11EDCATableCWmax;     /* dot11EDCATableCWmax Unsigned32 */
    uint32_t dot11EDCATableAIFSN;     /* dot11EDCATableAIFSN Unsigned32 */
    uint32_t dot11EDCATableTXOPLimit; /* dot11EDCATableTXOPLimit Unsigned32 */
    // uint32_t  dot11EDCATableMSDULifetime;          /* dot11EDCATableMSDULifetime Unsigned32 */
    oal_bool_enum_uint8 en_dot11EDCATableMandatory; /* dot11EDCATableMandatory TruthValue */
} wlan_mib_Dot11CFGEDCAEntry_stru;

typedef struct {
    wlan_mib_Dot11CFGEDCAEntry_stru ast_wlan_mib_edca[WLAN_WME_AC_BUTT];
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    wlan_mib_Dot11QAPEDCAEntry_stru ast_wlan_mib_qap_edac[WLAN_WME_AC_BUTT];
} wlan_mib_Dot11EDCAEntry_stru;

/****************************************************************************************/
/* dot11QosCountersTable OBJECT-TYPE                                                    */
/* SYNTAX SEQUENCE OF Dot11QosCountersEntry                                             */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group containing attributes that are MAC counters implemented as a table       */
/*      to allow for multiple instantiations on an agent."                              */
/* ::= { dot11mac 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11QosCountersIndex;            /* dot11QosCountersIndex Unsigned32 */
    uint32_t dot11QosTransmittedFragmentCount; /* dot11QosTransmittedFragmentCount Counter32 */
    uint32_t dot11QosFailedCount;              /* dot11QosFailedCount Counter32 */
    uint32_t dot11QosRetryCount;               /* dot11QosRetryCount Counter32 */
    uint32_t dot11QosMultipleRetryCount;       /* dot11QosMultipleRetryCount Counter32 */
    uint32_t dot11QosFrameDuplicateCount;      /* dot11QosFrameDuplicateCount Counter32 */
    uint32_t dot11QosRTSSuccessCount;          /* dot11QosRTSSuccessCount Counter32 */
    uint32_t dot11QosRTSFailureCount;          /* dot11QosRTSFailureCount Counter32 */
    uint32_t dot11QosACKFailureCount;          /* dot11QosACKFailureCount Counter32 */
    uint32_t dot11QosReceivedFragmentCount;    /* dot11QosReceivedFragmentCount Counter32 */
    uint32_t dot11QosTransmittedFrameCount;    /* dot11QosTransmittedFrameCount Counter32 */
    uint32_t dot11QosDiscardedFrameCount;      /* dot11QosDiscardedFrameCount Counter32 */
    uint32_t dot11QosMPDUsReceivedCount;       /* dot11QosMPDUsReceivedCount Counter32 */
    uint32_t dot11QosRetriesReceivedCount;     /* dot11QosRetriesReceivedCount Counter32 */
} wlan_mib_Dot11QosCountersEntry_stru;

/****************************************************************************************/
/* dot11ResourceInfoTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11ResourceInfoEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Provides a means of indicating, in data readable from a managed object,        */
/*      information that identifies the source of the implementation."                  */
/*      REFERENCE "IEEE Std 802.1F-1993, A.7. Note that this standard has been with-    */
/*      drawn."                                                                         */
/* ::= { dot11mac 7 }                                                                   */
/* ::= { dot11resAttribute 2 }                                                          */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11manufacturerOUI[NUM_3_BYTES];              /* dot11manufacturerOUI OCTET STRING */
    uint8_t auc_dot11manufacturerName[128];           /* dot11manufacturerName DisplayString SIZE(0..128) */
    uint8_t auc_dot11manufacturerProductName[128];    /* dot11manufacturerProductName DisplayString SIZE(0..128) */
    uint8_t auc_dot11manufacturerProductVersion[128]; /* dot11manufacturerProductVersion DisplayString SIZE(0..128) */
} wlan_mib_Dot11ResourceInfoEntry_stru;

/****************************************************************************************/
/*          Start of dot11res    OBJECT IDENTIFIER ::= { ieee802dot11 3 }               */
/*              dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 }                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11ResourceTypeIDName[4]; /* dot11ResourceTypeIDName  DisplayString (SIZE(4)) */
    wlan_mib_Dot11ResourceInfoEntry_stru st_resource_info;
} wlan_mib_dot11resAttribute_stru;

/* Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6} */
/****************************************************************************************/
/*        Start of dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6}                      */
/*          -- IMT GROUPS                                                               */
/*          -- dot11BSSIdTable ::= { dot11imt 1 }                                       */
/*          -- dot11InterworkingTable ::= { dot11imt 2 }                                */
/*          -- dot11APLCI ::= { dot11imt 3 }                                            */
/*          -- dot11APCivicLocation ::= { dot11imt 4 }                                  */
/*          -- dot11RoamingConsortiumTable      ::= { dot11imt 5 }                      */
/*          -- dot11DomainNameTable ::= { dot11imt 6 }                                  */
/*          -- Generic Advertisement Service (GAS) Attributes                           */
/*          -- DEFINED AS "The Generic Advertisement Service management                 */
/*          -- object class provides the necessary support for an Advertisement         */
/*          -- service to interwork with external systems."                             */
/*          -- GAS GROUPS                                                               */
/*          -- dot11GASAdvertisementTable       ::= { dot11imt 7 }                      */
/****************************************************************************************/
/****************************************************************************************/
/* dot11BSSIdTable OBJECT-TYPE                                                          */
/* SYNTAX         SEQUENCE OF Dot11BSSIdEntry                                           */
/* MAX-ACCESS     not-accessible                                                        */
/* STATUS         current                                                               */
/* DESCRIPTION                                                                          */
/*      "This object is a table of BSSIDs contained within an Access Point (AP)."       */
/*  ::= { dot11imt 1 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11APMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11APMacAddress OBJECT-TYPE MacAddress */
} wlan_mib_Dot11BSSIdEntry_stru;

/****************************************************************************************/
/* dot11InterworkingTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11InterworkingEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the non-AP STAs associated to the AP. An entry is        */
/*      created automatically by the AP when the STA becomes associated to the AP.      */
/*      The corresponding entry is deleted when the STA disassociates. Each STA         */
/*      added to this table is uniquely identified by its MAC address. This table       */
/*      is moved to a new AP following a successful STA BSS transition event."          */
/* ::= { dot11imt 2 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11NonAPStationMacAddress[WLAN_MAC_ADDR_LEN];    /* dot11NonAPStationMacAddress MacAddress */
    uint8_t auc_dot11NonAPStationUserIdentity[NUM_255_BYTES];      /* dot11NonAPStationUserIdentity DisplayString */
    uint8_t uc_dot11NonAPStationInterworkingCapability;        /* dot11NonAPStationInterworkingCapability BITS */
    uint8_t auc_dot11NonAPStationAssociatedSSID[NUM_32_BYTES];    /* dot11NonAPStationAssociatedSSID OCTET STRING */
    uint8_t auc_dot11NonAPStationUnicastCipherSuite[NUM_4_BYTES]; /* dot11NonAPStationUnicastCipherSuite OCTET STRING */
    uint8_t auc_dot11NonAPStationBroadcastCipherSuite[NUM_4_BYTES];   /* dot11NonAPStationBroadcastCipherSuite */
    uint8_t uc_dot11NonAPStationAuthAccessCategories;          /* dot11NonAPStationAuthAccessCategories BITS */
    uint32_t dot11NonAPStationAuthMaxVoiceRate;             /* dot11NonAPStationAuthMaxVoiceRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVideoRate;             /* dot11NonAPStationAuthMaxVideoRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBestEffortRate;        /* dot11NonAPStationAuthMaxBestEffortRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBackgroundRate;        /* dot11NonAPStationAuthMaxBackgroundRate Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVoiceOctets;           /* dot11NonAPStationAuthMaxVoiceOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxVideoOctets;           /* dot11NonAPStationAuthMaxVideoOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxBestEffortOctets;      /* dot11NonAPStationAuthMaxBestEffortOctets */
    uint32_t dot11NonAPStationAuthMaxBackgroundOctets;      /* dot11NonAPStationAuthMaxBackgroundOctets */
    uint32_t dot11NonAPStationAuthMaxHCCAHEMMOctets;        /* dot11NonAPStationAuthMaxHCCAHEMMOctets Unsigned32 */
    uint32_t dot11NonAPStationAuthMaxTotalOctets;           /* dot11NonAPStationAuthMaxTotalOctets Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthHCCAHEMM;        /* dot11NonAPStationAuthHCCAHEMM TruthValue */
    uint32_t dot11NonAPStationAuthMaxHCCAHEMMRate;          /* dot11NonAPStationAuthMaxHCCAHEMMRate Unsigned32 */
    uint32_t dot11NonAPStationAuthHCCAHEMMDelay;            /* dot11NonAPStationAuthHCCAHEMMDelay Unsigned32 */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthSourceMulticast; /* dot11NonAPStationAuthSourceMulticast TruthValue */
    uint32_t dot11NonAPStationAuthMaxSourceMulticastRate;   /* dot11NonAPStationAuthMaxSourceMulticastRate */
    uint32_t dot11NonAPStationVoiceMSDUCount;               /* dot11NonAPStationVoiceMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedVoiceMSDUCount;        /* dot11NonAPStationDroppedVoiceMSDUCount Counter32 */
    uint32_t dot11NonAPStationVoiceOctetCount;              /* dot11NonAPStationVoiceOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedVoiceOctetCount;       /* dot11NonAPStationDroppedVoiceOctetCount Counter32 */
    uint32_t dot11NonAPStationVideoMSDUCount;               /* dot11NonAPStationVideoMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedVideoMSDUCount;        /* dot11NonAPStationDroppedVideoMSDUCount Counter32 */
    uint32_t dot11NonAPStationVideoOctetCount;              /* dot11NonAPStationVideoOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedVideoOctetCount;       /* dot11NonAPStationDroppedVideoOctetCount Counter32 */
    uint32_t dot11NonAPStationBestEffortMSDUCount;          /* dot11NonAPStationBestEffortMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedBestEffortMSDUCount;   /* dot11NonAPStationDroppedBestEffortMSDUCount */
    uint32_t dot11NonAPStationBestEffortOctetCount;         /* dot11NonAPStationBestEffortOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedBestEffortOctetCount;  /* dot11NonAPStationDroppedBestEffortOctetCount */
    uint32_t dot11NonAPStationBackgroundMSDUCount;          /* dot11NonAPStationBackgroundMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedBackgroundMSDUCount;   /* dot11NonAPStationDroppedBackgroundMSDUCount */
    uint32_t dot11NonAPStationBackgroundOctetCount;         /* dot11NonAPStationBackgroundOctetCount */
    uint32_t dot11NonAPStationDroppedBackgroundOctetCount;  /* dot11NonAPStationDroppedBackgroundOctetCount */
    uint32_t dot11NonAPStationHCCAHEMMMSDUCount;            /* dot11NonAPStationHCCAHEMMMSDUCount Counter32 */
    uint32_t dot11NonAPStationDroppedHCCAHEMMMSDUCount;     /* dot11NonAPStationDroppedHCCAHEMMMSDUCount */
    uint32_t dot11NonAPStationHCCAHEMMOctetCount;           /* dot11NonAPStationHCCAHEMMOctetCount Counter32 */
    uint32_t dot11NonAPStationDroppedHCCAHEMMOctetCount;    /* dot11NonAPStationDroppedHCCAHEMMOctetCount */
    uint32_t dot11NonAPStationMulticastMSDUCount;           /* dot11NonAPStationMulticastMSDUCount */
    uint32_t dot11NonAPStationDroppedMulticastMSDUCount;    /* dot11NonAPStationDroppedMulticastMSDUCount */
    uint32_t dot11NonAPStationMulticastOctetCount;          /* dot11NonAPStationMulticastOctetCount */
    uint32_t dot11NonAPStationDroppedMulticastOctetCount;   /* dot11NonAPStationDroppedMulticastOctetCount */
    uint8_t uc_dot11NonAPStationPowerManagementMode;           /* dot11NonAPStationPowerManagementMode INTEGER */
    oal_bool_enum_uint8 en_dot11NonAPStationAuthDls;             /* dot11NonAPStationAuthDls TruthValue */
    uint32_t dot11NonAPStationVLANId;                       /* dot11NonAPStationVLANId Unsigned32 */
    uint8_t auc_dot11NonAPStationVLANName[64];                 /* dot11NonAPStationVLANName DisplayString SIZE(64) */
    uint8_t uc_dot11NonAPStationAddtsResultCode;               /* dot11NonAPStationAddtsResultCode INTEGER */
} wlan_mib_Dot11InterworkingEntry_stru;

/****************************************************************************************/
/* dot11APLCITable OBJECT-TYPE                                                          */
/* SYNTAX         SEQUENCE OF Dot11APLCIEntry                                           */
/* MAX-ACCESS     not-accessible                                                        */
/* STATUS         current                                                               */
/* DESCRIPTION                                                                          */
/*      "This table represents the Geospatial location of the AP as specified in        */
/*      8.4.2.23.10."                                                                   */
/* ::= { dot11imt 3 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11APLCIIndex;                                    /* dot11APLCIIndex Unsigned32 */
    uint32_t dot11APLCILatitudeResolution;                       /* dot11APLCILatitudeResolution Unsigned32 */
    int32_t l_dot11APLCILatitudeInteger;                            /* dot11APLCILatitudeInteger Integer32 */
    int32_t l_dot11APLCILatitudeFraction;                           /* dot11APLCILatitudeFraction Integer32 */
    uint32_t dot11APLCILongitudeResolution;                      /* dot11APLCILongitudeResolution Unsigned32 */
    int32_t l_dot11APLCILongitudeInteger;                           /* dot11APLCILongitudeInteger Integer32 */
    int32_t l_dot11APLCILongitudeFraction;                          /* dot11APLCILongitudeFraction Integer32 */
    uint8_t uc_dot11APLCIAltitudeType;                              /* dot11APLCIAltitudeType INTEGER */
    uint32_t dot11APLCIAltitudeResolution;                       /* dot11APLCIAltitudeResolution Unsigned32 */
    int32_t l_dot11APLCIAltitudeInteger;                            /* dot11APLCIAltitudeInteger Integer32 */
    int32_t l_dot11APLCIAltitudeFraction;                           /* dot11APLCIAltitudeFraction Integer32 */
    uint8_t en_dot11APLCIDatum;              /* wlan_mib_ap_lci_datum_enum: dot11APLCIDatum INTEGER */
    uint8_t en_dot11APLCIAzimuthType; /* wlan_mib_ap_lci_azimuth_type_enum: dot11APLCIAzimuthType INTEGER */
    uint32_t dot11APLCIAzimuthResolution;                        /* dot11APLCIAzimuthResolution Unsigned32 */
    int32_t l_dot11APLCIAzimuth;                                    /* dot11APLCIAzimuth Integer32 */
} wlan_mib_Dot11APLCIEntry_stru;

/****************************************************************************************/
/* dot11APCivicLocationTable OBJECT-TYPE                                                */
/* SYNTAX SEQUENCE OF Dot11ApCivicLocationEntry                                         */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the location of the AP in Civic format using the         */
/*      Civic Address Type elements defined in IETF RFC-5139 [B42]."                    */
/* ::= { dot11imt 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11APCivicLocationIndex;        /* dot11APCivicLocationIndex Unsigned32 */
    uint8_t auc_dot11APCivicLocationCountry[NUM_255_BYTES]; /* dot11APCivicLocationCountry OCTET STRING */
    uint8_t auc_dot11APCivicLocationA1[NUM_255_BYTES];      /* dot11APCivicLocationA1 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA2[NUM_255_BYTES];      /* dot11APCivicLocationA2 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA3[NUM_255_BYTES];      /* dot11APCivicLocationA3 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA4[NUM_255_BYTES];      /* dot11APCivicLocationA4 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA5[NUM_255_BYTES];      /* dot11APCivicLocationA5 OCTET STRING */
    uint8_t auc_dot11APCivicLocationA6[NUM_255_BYTES];      /* dot11APCivicLocationA6 OCTET STRING */
    uint8_t auc_dot11APCivicLocationPrd[NUM_255_BYTES];     /* dot11APCivicLocationPrd OCTET STRING */
    uint8_t auc_dot11APCivicLocationPod[NUM_255_BYTES];     /* dot11APCivicLocationPod OCTET STRING */
    uint8_t auc_dot11APCivicLocationSts[NUM_255_BYTES];     /* dot11APCivicLocationSts OCTET STRING */
    uint8_t auc_dot11APCivicLocationHno[NUM_255_BYTES];     /* dot11APCivicLocationHno OCTET STRING */
    uint8_t auc_dot11APCivicLocationHns[NUM_255_BYTES];     /* dot11APCivicLocationHns OCTET STRING */
    uint8_t auc_dot11APCivicLocationLmk[NUM_255_BYTES];     /* dot11APCivicLocationLmk OCTET STRING */
    uint8_t auc_dot11APCivicLocationLoc[NUM_255_BYTES];     /* dot11APCivicLocationLoc OCTET STRING */
    uint8_t auc_dot11APCivicLocationNam[NUM_255_BYTES];     /* dot11APCivicLocationNam OCTET STRING */
    uint8_t auc_dot11APCivicLocationPc[NUM_255_BYTES];      /* dot11APCivicLocationPc OCTET STRING */
    uint8_t auc_dot11APCivicLocationBld[NUM_255_BYTES];     /* dot11APCivicLocationBld OCTET STRING */
    uint8_t auc_dot11APCivicLocationUnit[NUM_255_BYTES];    /* dot11APCivicLocationUnit OCTET STRING */
    uint8_t auc_dot11APCivicLocationFlr[NUM_255_BYTES];     /* dot11APCivicLocationFlr OCTET STRING */
    uint8_t auc_dot11APCivicLocationRoom[NUM_255_BYTES];    /* dot11APCivicLocationRoom OCTET STRING */
    uint8_t auc_dot11APCivicLocationPlc[NUM_255_BYTES];     /* dot11APCivicLocationPlc OCTET STRING */
    uint8_t auc_dot11APCivicLocationPcn[NUM_255_BYTES];     /* dot11APCivicLocationPcn OCTET STRING */
    uint8_t auc_dot11APCivicLocationPobox[NUM_255_BYTES];   /* dot11APCivicLocationPobox OCTET STRING */
    uint8_t auc_dot11APCivicLocationAddcode[NUM_255_BYTES]; /* dot11APCivicLocationAddcode OCTET STRING */
    uint8_t auc_dot11APCivicLocationSeat[NUM_255_BYTES];    /* dot11APCivicLocationSeat OCTET STRING */
    uint8_t auc_dot11APCivicLocationRd[NUM_255_BYTES];      /* dot11APCivicLocationRd OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdsec[NUM_255_BYTES];   /* dot11APCivicLocationRdsec OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdbr[NUM_255_BYTES];    /* dot11APCivicLocationRdbr OCTET STRING */
    uint8_t auc_dot11APCivicLocationRdsubbr[NUM_255_BYTES]; /* dot11APCivicLocationRdsubbr OCTET STRING */
    uint8_t auc_dot11APCivicLocationPrm[NUM_255_BYTES];     /* dot11APCivicLocationPrm OCTET STRING */
    uint8_t auc_dot11APCivicLocationPom[NUM_255_BYTES];     /* dot11APCivicLocationPom OCTET STRING */
} wlan_mib_Dot11ApCivicLocationEntry_stru;

/****************************************************************************************/
/* dot11RoamingConsortiumTable OBJECT-TYPE                                              */
/* SYNTAX SEQUENCE OF Dot11RoamingConsortiumEntry                                       */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This is a Table of OIs which are to be transmitted in an ANQP Roaming          */
/*      Consortium ANQP-element. Each table entry corresponds to a roaming consor-      */
/*      tium or single SSP. The first 3 entries in this table are transmitted in        */
/*      Beacon and Probe Response frames."                                              */
/* ::= { dot11imt 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11RoamingConsortiumOI[NUM_16_BYTES];        /* dot11RoamingConsortiumOI OCTET STRING */
    uint8_t en_dot11RoamingConsortiumRowStatus; /* wlan_mib_row_status_enum: dot11RoamingConsortiumRowStatus */
} wlan_mib_Dot11RoamingConsortiumEntry_stru;

/****************************************************************************************/
/* dot11DomainNameTable   OBJECT-TYPE                                                   */
/* SYNTAX                SEQUENCE OF Dot11DomainNameEntry                               */
/* MAX-ACCESS            not-accessible                                                 */
/* STATUS                current                                                        */
/* DESCRIPTION                                                                          */
/*      "This is a table of Domain Names which form the Domain Name list in Access      */
/*      Network Query Protocol. The Domain Name list may be transmitted to a non-       */
/*      AP STA in a GAS Response. Each table entry corresponds to a single Domain       */
/*      Name."                                                                          */
/* ::= { dot11imt 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11DomainName[NUM_255_BYTES];                         /* dot11DomainName OCTET STRING */
    uint8_t en_dot11DomainNameRowStatus; /* wlan_mib_row_status_enum: dot11DomainNameRowStatus RowStatus */
    uint8_t auc_dot11DomainNameOui[5];                   /* dot11DomainNameOui OCTET STRING (SIZE(3..5)) */
} wlan_mib_Dot11DomainNameEntry_stru;

/****************************************************************************************/
/* dot11GASAdvertisementTable OBJECT-TYPE                                               */
/* SYNTAX SEQUENCE OF Dot11GASAdvertisementEntry                                        */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This object is a table of GAS counters that allows for multiple instanti-      */
/*      ations of those counters on an STA."                                            */
/* ::= { dot11imt 7 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11GASAdvertisementId;                 /* dot11GASAdvertisementId Unsigned32 */
    oal_bool_enum_uint8 en_dot11GASPauseForServerResponse; /* dot11GASPauseForServerResponse TruthValue */
    uint32_t dot11GASResponseTimeout;                 /* dot11GASResponseTimeout Unsigned32 */
    uint32_t dot11GASComebackDelay;                   /* dot11GASComebackDelay Unsigned32 */
    uint32_t dot11GASResponseBufferingTime;           /* dot11GASResponseBufferingTime Unsigned32 */
    uint32_t dot11GASQueryResponseLengthLimit;        /* dot11GASQueryResponseLengthLimit Unsigned32 */
    uint32_t dot11GASQueries;                         /* dot11GASQueries Counter32 */
    uint32_t dot11GASQueryRate;                       /* dot11GASQueryRate Gauge32 */
    uint32_t dot11GASResponses;                       /* dot11GASResponses Counter32 */
    uint32_t dot11GASResponseRate;                    /* dot11GASResponseRate Gauge32 */
    uint32_t dot11GASTransmittedFragmentCount;        /* dot11GASTransmittedFragmentCount Counter32 */
    uint32_t dot11GASReceivedFragmentCount;           /* dot11GASReceivedFragmentCount Counter32 */
    uint32_t dot11GASNoRequestOutstanding;            /* dot11GASNoRequestOutstanding Counter32 */
    uint32_t dot11GASResponsesDiscarded;              /* dot11GASResponsesDiscarded Counter32 */
    uint32_t dot11GASFailedResponses;                 /* dot11GASFailedResponses Counter32 */
} wlan_mib_Dot11GASAdvertisementEntry_stru;

/****************************************************************************************/
/*          Start of dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7}                 */
/*              -- MAC State GROUPS                                                     */
/*              -- dot11MACStateConfigTable ::= { dot11MSGCF 1 }                        */
/*              -- dot11MACStateParameterTable ::= { dot11MSGCF 2 }                     */
/*              -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 }                       */
/****************************************************************************************/
/****************************************************************************************/
/* dot11MACStateConfigTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11MACStateConfigEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table holds configuration parameters for the 802.11 MAC                   */
/*      State Convergence Function."                                                    */
/* ::= { dot11MSGCF 1 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSDisconnectFilterInterval;   /* dot11ESSDisconnectFilterInterval Unsigned32 */
    uint32_t dot11ESSLinkDetectionHoldInterval;  /* dot11ESSLinkDetectionHoldInterval Unsigned32 */
    uint8_t auc_dot11MSCEESSLinkIdentifier[38];  /* dot11MSCEESSLinkIdentifier Dot11ESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSCENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSCENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateConfigEntry_stru;

/****************************************************************************************/
/* dot11MACStateParameterTable OBJECT-TYPE                                              */
/* SYNTAX SEQUENCE OF Dot11MACStateParameterEntry                                       */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS     current                                                                   */
/* DESCRIPTION                                                                          */
/*      "This table holds the current parameters used for each 802.11 network for       */
/*      802.11 MAC convergence functions."                                              */
/* ::= { dot11MSGCF 2 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSLinkDownTimeInterval;                      /* dot11ESSLinkDownTimeInterval Unsigned32 */
    uint32_t dot11ESSLinkRssiDataThreshold;                     /* dot11ESSLinkRssiDataThreshold Unsigned32 */
    uint32_t dot11ESSLinkRssiBeaconThreshold;                   /* dot11ESSLinkRssiBeaconThreshold Unsigned32 */
    uint32_t dot11ESSLinkDataSnrThreshold;                      /* dot11ESSLinkDataSnrThreshold Unsigned32 */
    uint32_t dot11ESSLinkBeaconSnrThreshold;                    /* dot11ESSLinkBeaconSnrThreshold Unsigned32 */
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdInteger;
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdFraction;
    uint32_t dot11ESSLinkBeaconFrameErrorRateThresholdExponent;
    uint32_t dot11ESSLinkFrameErrorRateThresholdInteger;        /* dot11ESSLinkFrameErrorRateThresholdInteger */
    uint32_t dot11ESSLinkFrameErrorRateThresholdFraction;       /* dot11ESSLinkFrameErrorRateThresholdFraction */
    uint32_t dot11ESSLinkFrameErrorRateThresholdExponent;       /* dot11ESSLinkFrameErrorRateThresholdExponent */
    uint32_t dot11PeakOperationalRate;                          /* dot11PeakOperationalRate Unsigned32 */
    uint32_t dot11MinimumOperationalRate;                       /* dot11MinimumOperationalRate Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputInteger;                 /* dot11ESSLinkDataThroughputInteger Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputFraction;                /* dot11ESSLinkDataThroughputFraction Unsigned32 */
    uint32_t dot11ESSLinkDataThroughputExponent;                /* dot11ESSLinkDataThroughputExponent Unsigned32 */
    uint8_t auc_dot11MSPEESSLinkIdentifier[38];                     /* dot11MSPEESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSPENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSPENonAPStationMacAddress MacAddress */
} wlan_mib_Dot11MACStateParameterEntry_stru;

/****************************************************************************************/
/* dot11MACStateESSLinkDetectedTable OBJECT-TYPE                                        */
/* SYNTAX SEQUENCE OF Dot11MACStateESSLinkDetectedEntry                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table holds the detected 802.11 network list used for MAC conver-         */
/*      gence functions."                                                               */
/* ::= { dot11MSGCF 3 }                                                                 */
/****************************************************************************************/
typedef struct {
    uint32_t dot11ESSLinkDetectedIndex;                 /* dot11ESSLinkDetectedIndex Unsigned32 */
    int8_t ac_dot11ESSLinkDetectedNetworkId[NUM_255_BYTES];   /* dot11ESSLinkDetectedNetworkId OCTET STRING */
    uint32_t dot11ESSLinkDetectedNetworkDetectTime;     /* dot11ESSLinkDetectedNetworkDetectTime Unsigned32 */
    uint32_t dot11ESSLinkDetectedNetworkModifiedTime;   /* dot11ESSLinkDetectedNetworkModifiedTime Unsigned32 */
    uint8_t uc_dot11ESSLinkDetectedNetworkMIHCapabilities; /* dot11ESSLinkDetectedNetworkMIHCapabilities BITS */
    uint8_t auc_dot11MSELDEESSLinkIdentifier[38]; /* dot11MSELDEESSLinkIdentifier Dot11ESSLinkIdentifier size 38 BYTE */
    uint8_t auc_dot11MSELDENonAPStationMacAddress[WLAN_MAC_ADDR_LEN]; /* dot11MSELDENonAPStationMacAddress */
} wlan_mib_Dot11MACStateESSLinkDetectedEntry_stru;

typedef struct {
    /***************************************************************************
        dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }
    ****************************************************************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    wlan_mib_Dot11StationConfigEntry_stru st_wlan_mib_sta_config;

    /* --  dot11AuthenticationAlgorithmsTable ::= { dot11smt 2 } */
    /* --  dot11WEPDefaultKeysTable ::= { dot11smt 3 } */
    /* --  dot11WEPKeyMappingsTable ::= { dot11smt 4 } */
    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    wlan_mib_Dot11PrivacyEntry_stru st_wlan_mib_privacy;

    /* --  dot11SMTnotification ::= { dot11smt 6 } */
    /* --  dot11MultiDomainCapabilityTable ::= { dot11smt 7 } */
    /* --  dot11SpectrumManagementTable ::= { dot11smt 8 } */
    /* --  dot11RSNAConfigTable ::= { dot11smt 9 } */
    /* --  dot11RSNAConfigPairwiseCiphersTable ::= { dot11smt 10 } */
    /* --  dot11RSNAConfigAuthenticationSuitesTable      ::= { dot11smt 11 } */
    /* --  dot11RSNAStatsTable ::= { dot11smt 12 } */
    /* --  dot11OperatingClassesTable ::= { dot11smt 13 } */
    /* --  dot11RadioMeasurement ::= { dot11smt 14 } */
#if defined(_PRE_WLAN_FEATURE_11R)
    /* --  dot11FastBSSTransitionConfigTable ::= { dot11smt 15 } */
    wlan_mib_Dot11FastBSSTransitionConfigEntry_stru st_wlan_mib_fast_bss_trans_cfg;
#endif
    /* --  dot11LCIDSETable        ::= { dot11smt 16 } */
    /* --  dot11HTStationConfigTable  ::= { dot11smt 17 } */
    wlan_mib_Dot11HTStationConfigEntry_stru st_wlan_mib_ht_sta_cfg;

#if (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* --  dot11WirelessMgmtOptionsTable ::= { dot11smt 18} */
    wlan_mib_Dot11WirelessMgmtOptionsEntry_stru st_wlan_mib_wireless_mgmt_op;
#endif

    /* --  dot11LocationServicesNextIndex ::= { dot11smt 19} */
    /* --  dot11LocationServicesTable ::= { dot11smt 20} */
    /* --  dot11WirelessMGTEventTable ::= { dot11smt 21} */
    /* --  dot11WirelessNetworkManagement ::= { dot11smt 22} */
    /* --  dot11MeshSTAConfigTable ::= { dot11smt 23 } */
    /* --  dot11MeshHWMPConfigTable ::= { dot11smt 24 } */
    /* --  dot11RSNAConfigPasswordValueTable ::= { dot11smt 25 } */
    /* --  dot11RSNAConfigDLCGroupTable ::= { dot11smt 26 } */
    /* --  dot11VHTStationConfig ::= { dot11smt 31 } */
    wlan_mib_Dot11VHTStationConfigEntry_stru st_wlan_mib_vht_sta_config;

#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    wlan_mib_Dot11HEStationConfigEntry_stru st_wlan_mib_he_sta_config;
    wlan_mid_Dot11HePhyCapability_stru st_wlan_mib_he_phy_config;
#endif
    /***************************************************************************
        dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 }
    ****************************************************************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    wlan_mib_Dot11OperationEntry_stru st_wlan_mib_operation;
    /* --  dot11CountersTable ::= { dot11mac 2 } */
    /* --  dot11GroupAddressesTable ::= { dot11mac 3 } */
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    wlan_mib_Dot11EDCAEntry_stru st_wlan_mib_edca;
    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    /* --  dot11QosCountersTable ::= { dot11mac 6 } */
    /* --  dot11ResourceInfoTable    ::= { dot11mac 7 } */
    /* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
    /* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
    wlan_mib_Dot11SupportedMCSEntry_stru st_supported_mcs;

    /***************************************************************************
        dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 }
    ****************************************************************************/
    /* -- dot11resAttribute OBJECT IDENTIFIER ::= { dot11res 1 } */
    /***************************************************************************
        dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 }
    ****************************************************************************/
    /* --  dot11PhyOperationTable ::= { dot11phy 1 } */
    /* --  dot11PhyAntennaTable ::= { dot11phy 2 } */
    wlan_mib_Dot11PhyAntennaEntry_stru st_wlan_mib_phy_antenna;

    /* --  dot11PhyTxPowerTable ::= { dot11phy 3 } */
    /* --  dot11PhyFHSSTable ::= { dot11phy 4 } */
    /* --  dot11PhyDSSSTable ::= { dot11phy 5 } */
    wlan_mib_Dot11PhyDSSSEntry_stru st_wlan_mib_phy_dsss;

    /* --  dot11PhyIRTable ::= { dot11phy 6 } */
    /* --  dot11RegDomainsSupportedTable ::= { dot11phy 7 } */
    /* --  dot11AntennasListTable ::= { dot11phy 8 } */
    /* --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 } */
    /* --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 } */
    /* --  dot11PhyOFDMTable ::= { dot11phy 11 } */
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    /* --  dot11HoppingPatternTable ::= { dot11phy 13 } */
    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    wlan_mib_Dot11PhyHTEntry_stru st_wlan_mib_phy_ht;

    /* --  dot11SupportedMCSTxTable ::= { dot11phy 16 } */
    /* --  dot11SupportedMCSRxTable ::= { dot11phy 17 } */
    /* --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 } */
    /* -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac) */
    wlan_mib_Dot11PhyVHTEntry_stru st_wlan_mib_phy_vht;

    /* -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
    /***************************************************************************
        dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } (该组用于归类，暂不实现)
    ****************************************************************************/
    /***************************************************************************
        dot11imt OBJECT IDENTIFIER ::= {ieee802dot11 6}
    ****************************************************************************/
    /* -- dot11BSSIdTable ::= { dot11imt 1 } */
    /* -- dot11InterworkingTable ::= { dot11imt 2 } */
    /* -- dot11APLCI ::= { dot11imt 3 } */
    /* -- dot11APCivicLocation ::= { dot11imt 4 } */
    /* -- dot11RoamingConsortiumTable      ::= { dot11imt 5 } */
    /* -- dot11DomainNameTable ::= { dot11imt 6 } */
    /* -- dot11GASAdvertisementTable       ::= { dot11imt 7 } */
    /***************************************************************************
        dot11MSGCF OBJECT IDENTIFIER ::= { ieee802dot11 7}
    ****************************************************************************/
    /* -- dot11MACStateConfigTable ::= { dot11MSGCF 1 }     */
    /* -- dot11MACStateParameterTable ::= { dot11MSGCF 2 }  */
    /* -- dot11MACStateESSLinkTable ::= { dot11MSGCF 3 }    */
} wlan_mib_ieee802dot11_stru;

#endif /* end of wlan_mib.h */
