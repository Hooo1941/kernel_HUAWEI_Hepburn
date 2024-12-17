/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素WIRELESS结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_WIRELESS_H
#define WLAN_MIB_WIRELESS_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

/* dot11WirelessMGTEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WIRELESS_MGT_EVENT_TYPE_BUTT
} wlan_mib_wireless_mgt_event_type_enum;

/* dot11WirelessMGTEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* successful(0),                          */
/* requestFailed(1),                       */
/* requestRefused(2),                      */
/* requestIncapable(3),                    */
/* detectedFrequentTransition(4)           */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WIRELESS_MGT_EVENT_STATUS_BUTT
} wlan_mib_wireless_mgt_event_status_enum;

/* dot11WirelessMGTEventTransitionReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* certificateToken(8),                                */
/* apFailedIeee8021XEapAuthentication(9),              */
/* apFailed4wayHandshake(10),                          */
/* excessiveDataMICFailures(11),                       */
/* exceededFrameTransmissionRetryLimit(12),            */
/* ecessiveBroadcastDisassociations(13),               */
/* excessiveBroadcastDeauthentications(14),            */
/* previousTransitionFailed(15)                        */
typedef enum {
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_CERTIF_TOKEN = 8,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 9,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 10,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_DATA_MIC_FAIL = 11,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCEED_FRAME_TRANS_RETRY_LIMIT = 12,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISASSO = 13,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_EXCES_BROAD_DISAUTH = 14,
    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,

    WLAN_MIB_WIRELESS_MGT_EVENT_TRANSIT_REASON_BUTT
} wlan_mib_wireless_mgt_event_transit_reason_enum;

/**************************************************************************************/
/* dot11WirelessMgmtOptions TABLE  - members of Dot11WirelessMgmtOptionsEntry         */
/**************************************************************************************/
/* Wireless Management attributes. In tabular form to allow for multiple      */
/* instances on an agent. This table only applies to the interface if         */
/* dot11WirelessManagementImplemented is set to true in the                   */
/* dot11StationConfigTable. Otherwise this table should be ignored.           */
/* For all Wireless Management features, an Activated MIB variable is used    */
/* to activate/enable or deactivate/disable the corresponding feature.        */
/* An Implemented MIB variable is used for an optional feature to indicate    */
/* whether the feature is implemented. A mandatory feature does not have a    */
/* corresponding Implemented MIB variable. It is possible for there to be     */
/* multiple IEEE 802.11 interfaces on one agent, each with its unique MAC     */
/* address. The relationship between an IEEE 802.11 interface and an          */
/* interface in the context of the Internet standard MIB is one-to-one.       */
/* As such, the value of an ifIndex object instance can be directly used      */
/* to identify corresponding instances of the objects defined herein.         */
/* ifIndex - Each IEEE 802.11 interface is represented by an ifEntry.         */
/* Interface tables in this MIB module are indexed by ifIndex.                */
typedef struct {
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionFMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionEventsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMultiBSSIDActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTFSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMSleepModeActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTIMBroadcastActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionProxyARPActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionBSSTransitionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionQoSTrafficCapabilityActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionACStationCountActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionCoLocIntfReportingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionMotionDetectionActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTODActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionTimingMsmtActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionChannelUsageActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionTriggerSTAStatisticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionSSIDListActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionMulticastDiagnosticsActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionLocationTrackingActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionDMSActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUAPSDCoexistenceActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionWNMNotificationActivated;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetImplemented;
    oal_bool_enum_uint8 en_dot11MgmtOptionUTCTSFOffsetActivated;
    oal_bool_enum_uint8 en_dot11FtmInitiatorModeActivated;
    oal_bool_enum_uint8 en_dot11FtmResponderModeActivated;
    oal_bool_enum_uint8 en_dot11LciCivicInNeighborReport;
    oal_bool_enum_uint8 en_dot11RMFtmRangeReportImplemented;
    oal_bool_enum_uint8 en_dot11RMFtmRangeReportActivated;
    oal_bool_enum_uint8 en_dot11RMLCIConfigured;
    oal_bool_enum_uint8 en_dot11RMCivicConfigured;
} wlan_mib_Dot11WirelessMgmtOptionsEntry_stru;

/********************************************************************************/
/* dot11WirelessMGTEvent TABLE  - members of Dot11WirelessMGTEventEntry         */
/********************************************************************************/
/* Group contains the current list of WIRELESS Management reports that have     */
/* been received by the MLME. The report tables shall be maintained as FIFO     */
/* to preserve freshness, thus the rows in this table can be deleted for mem-   */
/* ory constraints or other implementation constraints determined by the ven-   */
/* dor. New rows shall have different RprtIndex values than those deleted within */
/* the range limitation of the index. One easy way is to monotonically          */
/* increase the EventIndex for new reports being written in the table*          */
typedef struct {
    uint32_t dot11WirelessMGTEventIndex;
    uint8_t auc_dot11WirelessMGTEventMACAddress[NUM_6_BYTES];
    uint8_t en_dot11WirelessMGTEventType;    /* wlan_mib_wireless_mgt_event_type_enum */
    uint8_t en_dot11WirelessMGTEventStatus;  /* wlan_mib_wireless_mgt_event_status_enum */
    uint8_t auc_dot11WirelessMGTEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WirelessMGTEventUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WirelessMGTEventTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WirelessMGTEventTransitionSourceBSSID[NUM_6_BYTES];
    uint8_t auc_dot11WirelessMGTEventTransitionTargetBSSID[NUM_6_BYTES];
    uint32_t dot11WirelessMGTEventTransitionTime;
    uint8_t en_dot11WirelessMGTEventTransitionReason; /* wlan_mib_wireless_mgt_event_transit_reason_enum */
    uint32_t dot11WirelessMGTEventTransitionResult;
    uint32_t dot11WirelessMGTEventTransitionSourceRCPI;
    uint32_t dot11WirelessMGTEventTransitionSourceRSNI;
    uint32_t dot11WirelessMGTEventTransitionTargetRCPI;
    uint32_t dot11WirelessMGTEventTransitionTargetRSNI;
    uint8_t auc_dot11WirelessMGTEventRSNATargetBSSID[NUM_6_BYTES];
    uint8_t auc_dot11WirelessMGTEventRSNAAuthenticationType[NUM_4_BYTES];
    uint8_t auc_dot11WirelessMGTEventRSNAEAPMethod[NUM_8_BYTES];
    uint32_t dot11WirelessMGTEventRSNAResult;
    uint8_t auc_dot11WirelessMGTEventRSNARSNElement[257]; /* 数组大小257字节 */
    uint8_t uc_dot11WirelessMGTEventPeerSTAAddress;
    uint32_t dot11WirelessMGTEventPeerOperatingClass;
    uint32_t dot11WirelessMGTEventPeerChannelNumber;
    int32_t l_dot11WirelessMGTEventPeerSTATxPower;
    uint32_t dot11WirelessMGTEventPeerConnectionTime;
    uint32_t dot11WirelessMGTEventPeerPeerStatus;
    uint8_t auc_dot11WirelessMGTEventWNMLog[2284]; /* 数组大小2284字节 */
} wlan_mib_Dot11WirelessMGTEventEntry_stru;

#endif /* end of wlan_mib_wireless.h */
