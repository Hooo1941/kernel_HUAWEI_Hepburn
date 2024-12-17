/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素WirelessNetworkManagement结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_WNM_H
#define WLAN_MIB_WNM_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

/* dot11WNMRqstType OBJECT-TYPE */
/* SYNTAX INTEGER {             */
/* mcastDiagnostics(0),         */
/* locationCivic(1),            */
/* locationIdentifier(2),       */
/* event(3),                    */
/* dignostic(4),                */
/* locationConfiguration(5),    */
/* bssTransitionQuery(6),       */
/* bssTransitionRqst(7),        */
/* fms(8),                      */
/* colocInterference(9)         */
typedef enum {
    WLAN_MIB_WNM_RQST_TYPE_MCAST_DIAG = 0,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CIVIC = 1,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_IDTIF = 2,
    WLAN_MIB_WNM_RQST_TYPE_EVENT = 3,
    WLAN_MIB_WNM_RQST_TYPE_DIAGOSTIC = 4,
    WLAN_MIB_WNM_RQST_TYPE_LOCATION_CFG = 5,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_QUERY = 6,
    WLAN_MIB_WNM_RQST_TYPE_BSS_TRANSIT_RQST = 7,
    WLAN_MIB_WNM_RQST_TYPE_FMS = 8,
    WLAN_MIB_WNM_RQST_TYPE_COLOC_INTERF = 9,

    WLAN_MIB_WNM_RQST_TYPE_BUTT
} wlan_mib_wnm_rqst_type_enum;

/* dot11WNMRqstLCRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LCRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lcrrqst_subj_enum;

/* dot11WNMRqstLCRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LCR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lcr_interval_unit_enum;

/* dot11WNMRqstLIRRqstSubject OBJECT-TYPE */
/* SYNTAX INTEGER {                       */
/* local(0),                              */
/* remote(1)                              */
typedef enum {
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_LOCAL = 0,
    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_REMOTE = 1,

    WLAN_MIB_WNM_RQST_LIRRQST_SUBJ_BUTT
} wlan_mib_wnm_rqst_lirrqst_subj_enum;

/* dot11WNMRqstLIRIntervalUnits OBJECT-TYPE */
/* SYNTAX INTEGER {                         */
/* seconds(0),                              */
/* minutes(1),                              */
/* hours(2)                                 */
typedef enum {
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_SECOND = 0,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_MINUTE = 1,
    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_HOUR = 2,

    WLAN_MIB_WNM_RQST_LIR_INTERVAL_UNIT_BUTT
} wlan_mib_wnm_rqst_lir_interval_unit_enum;

/* dot11WNMRqstEventType OBJECT-TYPE */
/* SYNTAX INTEGER {                      */
/* transition(0),                        */
/* rsna(1),                              */
/* peerToPeer(2),                        */
/* wnmLog(3),                            */
/* vendorSpecific(221)                   */
typedef enum {
    WLAN_MIB_WNM_RQST_EVENT_TYPE_TRANSITION = 0,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_RSNA = 1,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_PEERTOPEER = 2,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_WNMLOG = 3,
    WLAN_MIB_WNM_RQST_EVENT_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_EVENT_TYPE_BUTT
} wlan_mib_wnm_rqst_event_tpye_enum;

/* dot11WNMRqstDiagType OBJECT-TYPE */
/* SYNTAX INTEGER {                 */
/* cancelRequest(0),                */
/* manufacturerInfoStaRep(1),       */
/* configurationProfile(2),         */
/* associationDiag(3),              */
/* ieee8021xAuthDiag(4),            */
/* vendorSpecific(221)              */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CANCEL_RQST = 0,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_MANUFCT_INFO_STA_REP = 1,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_CFG_PROFILE = 2,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_ASSO_DIAG = 3,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_IEEE8021X_AUTH_DIAG = 4,
    WLAN_MIB_WNM_RQST_DIAG_TYPE_VENDOR_SPECIFIC = 221,

    WLAN_MIB_WNM_RQST_DIAG_TYPE_BUTT
} wlan_mib_wnm_rqst_diag_type_enum;

/* dot11WNMRqstDiagCredentials OBJECT-TYPE */
/* SYNTAX INTEGER {                        */
/* none(0),                                */
/* preSharedKey(1),                        */
/* usernamePassword(2),                    */
/* x509Certificate(3),                     */
/* otherCertificate(4),                    */
/* oneTimePassword(5),                     */
/* token(6)                                */
typedef enum {
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_NONT = 0,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_PRE_SHAREKEY = 1,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_USERNAME_PASSWORD = 2,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_X509_CTERTIFICATE = 3,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_OTHER_CTERTIFICATE = 4,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_ONE_TIME_PASSWORD = 5,
    WLAN_MIB_WNM_RQST_DIAG_CREDENT_TOKEN = 6,

    WLAN_MIB_WNM_RQST_DIAG_CREDENT_BUTT
} wlan_mib_wnm_rqst_diag_credent_enum;

/* dot11WNMRqstBssTransitQueryReason OBJECT-TYPE       */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_RQST_BSS_TRANSIT_QUERY_REASON_BUTT
} wlan_mib_wnm_rqst_bss_transit_query_reason_enum;

/* dot11WNMEventTransitRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                                */
/* successful(0),                                  */
/* requestFailed(1),                               */
/* requestRefused(2),                              */
/* requestIncapable(3),                            */
/* detectedFrequentTransition(4)                   */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_TRANSIT_RPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_transit_rprt_event_status_enum;

/* dot11WNMEventTransitRprtTransitReason OBJECT-TYPE   */
/* SYNTAX INTEGER {                                    */
/* unspecified(0),                                     */
/* excessiveFrameLossRatesPoorConditions(1),           */
/* excessiveDelayForCurrentTrafficStreams(2),          */
/* insufficientQosCapacityForCurrentTrafficStreams(3), */
/* firstAssociationToEss(4),                           */
/* loadBalancing(5),                                   */
/* betterApFound(6),                                   */
/* deauthenticatedDisassociatedFromPreviousAp(7),      */
/* apFailedIeee8021XEapAuthentication(8),              */
/* apFailed4wayHandshake(9),                           */
/* receivedTooManyReplayCounterFailures(10),           */
/* receivedTooManyDataMICFailures(11),                 */
/* exceededMaxNumberOfRetransmissions(12),             */
/* receivedTooManyBroadcastDisassociations(13),        */
/* receivedTooManyBroadcastDeauthentications(14),      */
/* previousTransitionFailed(15),                       */
/* lowRSSI(16)                                         */
typedef enum {
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_UNSPEC = 0,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_FRAME_LOSSRATE_POORCDT = 1,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCES_DELAY_CURRT_TRAFIC_STRMS = 2,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_INSUF_QOS_CAP_CURRT_TRAFIC_STRMS = 3,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_FIRST_ASSO_ESS = 4,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOAD_BALANCE = 5,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BETTER_AP_FOUND = 6,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_DEAUTH_DISASSO_FROM_PRE_AP = 7,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_IEEE8021X_EAP_AUTH = 8,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_AP_FAIL_4WAY_HANDSHAKE = 9,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_REPLAY_COUNT_FAIL = 10,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_DATA_MIC_FAIL = 11,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_EXCEED_MAXNUM_RETANS = 12,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DISASSO = 13,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_RECEIVE_TOOMANY_BRDCAST_DEAUTH = 14,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_PRE_TRANSIT_FAIL = 15,
    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_LOW_RSSI = 16,

    WLAN_MIB_WNM_EVENT_TRANSITRPRT_TRANSIT_REASON_BUTT
} wlan_mib_wnm_event_transitrprt_transit_reason_enum;

/* dot11WNMEventRsnaRprtEventStatus OBJECT-TYPE */
/* SYNTAX INTEGER {                             */
/* successful(0),                               */
/* requestFailed(1),                            */
/* requestRefused(2),                           */
/* requestIncapable(3),                         */
/* detectedFrequentTransition(4)                */
typedef enum {
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_SUCC = 0,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_FAIL = 1,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_REFUSE = 2,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_RQST_INCAP = 3,
    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_DETECT_FREQ_TRANSIT = 4,

    WLAN_MIB_WNM_EVENT_RSNARPRT_EVENT_STATUS_BUTT
} wlan_mib_wnm_event_rsnarprt_event_status_enum;

typedef struct {
    /* dot11WNMRequest OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 1 }   */
    /* dot11WNMRequestNextIndex Unsigned32(0..4294967295) ::= { dot11WNMRequest 1 } */
    uint32_t dot11WNMRequestNextIndex;

    /* dot11WNMRequestTable ::= { dot11WNMRequest 2 } */
    uint32_t dot11WNMRqstIndex;
    uint8_t en_dot11WNMRqstRowStatus;         /* wlan_mib_row_status_enum */
    uint8_t auc_dot11WNMRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMRqstIfIndex;
    uint8_t en_dot11WNMRqstType;              /* wlan_mib_wnm_rqst_type_enum */
    uint8_t auc_dot11WNMRqstTargetAdd[NUM_6_BYTES];
    uint32_t dot11WNMRqstTimeStamp;
    uint32_t dot11WNMRqstRndInterval;
    uint32_t dot11WNMRqstDuration;
    uint8_t auc_dot11WNMRqstMcstGroup[NUM_6_BYTES];
    uint8_t uc_dot11WNMRqstMcstTrigCon;
    uint32_t dot11WNMRqstMcstTrigInactivityTimeout;
    uint32_t dot11WNMRqstMcstTrigReactDelay;
    uint8_t en_dot11WNMRqstLCRRqstSubject;    /* wlan_mib_wnm_rqst_lcrrqst_subj_enum */
    uint8_t en_dot11WNMRqstLCRIntervalUnits;  /* wlan_mib_wnm_rqst_lcr_interval_unit_enum */
    uint32_t dot11WNMRqstLCRServiceInterval;
    uint8_t en_dot11WNMRqstLIRRqstSubject;    /* wlan_mib_wnm_rqst_lirrqst_subj_enum */
    uint8_t en_dot11WNMRqstLIRIntervalUnits;  /* wlan_mib_wnm_rqst_lir_interval_unit_enum */
    uint32_t dot11WNMRqstLIRServiceInterval;
    uint32_t dot11WNMRqstEventToken;
    uint8_t en_dot11WNMRqstEventType;         /* wlan_mib_wnm_rqst_event_type_enum */
    uint32_t dot11WNMRqstEventResponseLimit;
    uint8_t auc_dot11WNMRqstEventTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMRqstEventSourceBssid[NUM_6_BYTES];
    uint32_t dot11WNMRqstEventTransitTimeThresh;
    uint8_t uc_dot11WNMRqstEventTransitMatchValue;
    uint32_t dot11WNMRqstEventFreqTransitCountThresh;
    uint32_t dot11WNMRqstEventFreqTransitInterval;
    uint8_t auc_dot11WNMRqstEventRsnaAuthType[NUM_4_BYTES];
    uint32_t dot11WNMRqstEapType;
    uint8_t auc_dot11WNMRqstEapVendorId[NUM_3_BYTES];
    uint8_t auc_dot11WNMRqstEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMRqstEventRsnaMatchValue;
    uint8_t auc_dot11WNMRqstEventPeerMacAddress[NUM_6_BYTES];
    uint32_t dot11WNMRqstOperatingClass;
    uint32_t dot11WNMRqstChanNumber;
    uint32_t dot11WNMRqstDiagToken;
    uint8_t en_dot11WNMRqstDiagType;         /* wlan_mib_wnm_rqst_diag_type_enum */
    uint32_t dot11WNMRqstDiagTimeout;
    uint8_t auc_dot11WNMRqstDiagBssid[NUM_6_BYTES];
    uint32_t dot11WNMRqstDiagProfileId;
    uint8_t en_dot11WNMRqstDiagCredentials;  /* wlan_mib_wnm_rqst_diag_credent_enum */
    uint8_t auc_dot11WNMRqstLocConfigLocIndParams[NUM_16_BYTES];
    uint8_t auc_dot11WNMRqstLocConfigChanList[252]; /* 数组大小252字节 */
    uint32_t dot11WNMRqstLocConfigBcastRate;
    uint8_t auc_dot11WNMRqstLocConfigOptions[NUM_255_BYTES];
    uint8_t en_dot11WNMRqstBssTransitQueryReason;  /* wlan_mib_wnm_rqst_bss_transit_query_reason_enum */
    uint8_t uc_dot11WNMRqstBssTransitReqMode;
    uint32_t dot11WNMRqstBssTransitDisocTimer;

    /* This is a control variable.It is written by an external management entity when making a management
       request. Changes take effect when dot11WNMRqstRowStatus is set to Active.
       This attribute contains a variable-length field formatted in accordance with IETF RFC 3986-2005." */
    uint8_t auc_dot11WNMRqstBssTransitSessInfoURL[NUM_255_BYTES];
    uint8_t auc_dot11WNMRqstBssTransitCandidateList[2304]; /* 数组大小2304字节 */
    oal_bool_enum_uint8 en_dot11WNMRqstColocInterfAutoEnable;
    uint32_t dot11WNMRqstColocInterfRptTimeout;
    uint8_t auc_dot11WNMRqstVendorSpecific[NUM_255_BYTES];
    uint8_t auc_dot11WNMRqstDestinationURI[253]; /* 数组大小253字节 */

    /* dot11WNMReport OBJECT IDENTIFIER ::= { dot11WirelessNetworkManagement 2 } */
    /* dot11WNMVendorSpecificReportTable::= { dot11WNMReport 1 }                 */
    uint32_t dot11WNMVendorSpecificRprtIndex;
    uint8_t auc_dot11WNMVendorSpecificRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMVendorSpecificRprtIfIndex;
    uint8_t auc_dot11WNMVendorSpecificRprtContent[NUM_255_BYTES];
    /* dot11WNMMulticastDiagnosticReportTable ::= { dot11WNMReport 2 } */
    uint32_t dot11WNMMulticastDiagnosticRprtIndex;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMMulticastDiagnosticRprtIfIndex;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtMeasurementTime[NUM_8_BYTES];
    uint32_t dot11WNMMulticastDiagnosticRprtDuration;
    uint8_t auc_dot11WNMMulticastDiagnosticRprtMcstGroup[NUM_6_BYTES];
    uint8_t uc_dot11WNMMulticastDiagnosticRprtReason;
    uint32_t dot11WNMMulticastDiagnosticRprtRcvdMsduCount;
    uint32_t dot11WNMMulticastDiagnosticRprtFirstSeqNumber;
    uint32_t dot11WNMMulticastDiagnosticRprtLastSeqNumber;
    uint32_t dot11WNMMulticastDiagnosticRprtMcstRate;

    /* dot11WNMLocationCivicReportTable ::= { dot11WNMReport 3 } */
    uint32_t dot11WNMLocationCivicRprtIndex;
    uint8_t auc_dot11WNMLocationCivicRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocationCivicRprtIfIndex;

    /* This is a status variable.                                             */
    /* It is written by the SME when a management report is completed.        */
    /* This attribute indicates a variable octet field and contains a list of */
    /* civic address elements in TLV format as defined in IETF RFC 4776-2006  */
    /* dot11WNMLocationCivicRprtCivicLocation OCTET STRING */
    uint8_t auc_dot11WNMLocationCivicRprtCivicLocation[NUM_255_BYTES];

    /* dot11WNMLocationIdentifierReportTable ::= { dot11WNMReport 4 } */
    uint32_t dot11WNMLocationIdentifierRprtIndex;
    uint8_t auc_dot11WNMLocationIdentifierRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocationIdentifierRprtIfIndex;
    uint8_t auc_dot11WNMLocationIdentifierRprtExpirationTSF[NUM_8_BYTES];
    uint8_t uc_dot11WNMLocationIdentifierRprtPublicIdUri;

    /* dot11WNMEventTransitReportTable ::= { dot11WNMReport 5 } */
    uint32_t dot11WNMEventTransitRprtIndex;
    uint8_t auc_dot11WNMEventTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventTransitRprtIfIndex;
    uint8_t en_dot11WNMEventTransitRprtEventStatus; /* wlan_mib_wnm_event_transit_rprt_event_status_enum */
    uint8_t auc_dot11WNMEventTransitRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtSourceBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMEventTransitRprtTargetBssid[NUM_6_BYTES];
    uint32_t dot11WNMEventTransitRprtTransitTime;
    uint8_t en_dot11WNMEventTransitRprtTransitReason; /* wlan_mib_wnm_event_transitrprt_transit_reason_enum */
    uint32_t dot11WNMEventTransitRprtTransitResult;
    uint32_t dot11WNMEventTransitRprtSourceRCPI;
    uint32_t dot11WNMEventTransitRprtSourceRSNI;
    uint32_t dot11WNMEventTransitRprtTargetRCPI;
    uint32_t dot11WNMEventTransitRprtTargetRSNI;

    /* dot11WNMEventRsnaReportTable ::= { dot11WNMReport 6 } */
    uint32_t dot11WNMEventRsnaRprtIndex;
    uint8_t auc_dot11WNMEventRsnaRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventRsnaRprtIfIndex;
    uint8_t en_dot11WNMEventRsnaRprtEventStatus; /* wlan_mib_wnm_event_rsnarprt_event_status_enum */
    uint8_t auc_dot11WNMEventRsnaRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtAuthType[NUM_4_BYTES];
    uint8_t auc_dot11WNMEventRsnaRprtEapMethod[NUM_8_BYTES];
    uint32_t dot11WNMEventRsnaRprtResult;
    uint8_t uc_dot11WNMEventRsnaRprtRsnElement;

    /* dot11WNMEventPeerReportTable ::= { dot11WNMReport 7 } */
    uint32_t dot11WNMEventPeerRprtIndex;
    uint8_t auc_dot11WNMEventPeerRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventPeerRprtIfIndex;
    uint8_t uc_dot11WNMEventPeerRprtEventStatus;
    uint8_t auc_dot11WNMEventPeerRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventPeerRprtPeerMacAddress[NUM_6_BYTES];
    uint32_t dot11WNMEventPeerRprtOperatingClass;
    uint32_t dot11WNMEventPeerRprtChanNumber;
    int32_t l_dot11WNMEventPeerRprtStaTxPower;
    uint32_t dot11WNMEventPeerRprtConnTime;
    uint8_t uc_dot11WNMEventPeerRprtPeerStatus;

    /* dot11WNMEventWNMLogReportTable ::= { dot11WNMReport 8 } */
    uint32_t dot11WNMEventWNMLogRprtIndex;
    uint8_t auc_dot11WNMEventWNMLogRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMEventWNMLogRprtIfIndex;
    uint8_t uc_dot11WNMEventWNMLogRprtEventStatus;
    uint8_t auc_dot11WNMEventWNMLogRprtEventTSF[NUM_8_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtUTCOffset[NUM_10_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtTimeError[NUM_5_BYTES];
    uint8_t auc_dot11WNMEventWNMLogRprtContent[2284]; /* 数组大小2284字节 */
    /* dot11WNMDiagMfrInfoReportTable ::= { dot11WNMReport 9 } */
    uint32_t dot11WNMDiagMfrInfoRprtIndex;
    uint8_t auc_dot11WNMDiagMfrInfoRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagMfrInfoRprtIfIndex;
    uint8_t uc_dot11WNMDiagMfrInfoRprtEventStatus;
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrOi[NUM_5_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrIdString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrModelString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrSerialNumberString[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrFirmwareVersion[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagMfrInfoRprtMfrAntennaType[NUM_255_BYTES];
    uint8_t uc_dot11WNMDiagMfrInfoRprtCollocRadioType;
    uint8_t uc_dot11WNMDiagMfrInfoRprtDeviceType;
    uint8_t auc_dot11WNMDiagMfrInfoRprtCertificateID[251]; /* 数组大小251字节 */

    /* dot11WNMDiagConfigProfReportTable ::= { dot11WNMReport 10 } */
    uint32_t dot11WNMDiagConfigProfRprtIndex;
    uint8_t auc_dot11WNMDiagConfigProfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagConfigProfRprtIfIndex;
    uint8_t uc_dot11WNMDiagConfigProfRprtEventStatus;
    uint32_t dot11WNMDiagConfigProfRprtProfileId;
    uint8_t auc_dot11WNMDiagConfigProfRprtSupportedOperatingClasses[NUM_255_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtTxPowerMode;
    uint8_t auc_dot11WNMDiagConfigProfRprtTxPowerLevels[NUM_255_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtCipherSuite[NUM_4_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtAkmSuite[NUM_4_BYTES];
    uint32_t dot11WNMDiagConfigProfRprtEapType;
    uint8_t auc_dot11WNMDiagConfigProfRprtEapVendorID[NUM_3_BYTES];
    uint8_t auc_dot11WNMDiagConfigProfRprtEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtCredentialType;
    uint8_t auc_dot11WNMDiagConfigProfRprtSSID[NUM_32_BYTES];
    uint8_t uc_dot11WNMDiagConfigProfRprtPowerSaveMode;

    /* dot11WNMDiagAssocReportTable ::= { dot11WNMReport 11 } */
    uint32_t dot11WNMDiagAssocRprtIndex;
    uint8_t auc_dot11WNMDiagAssocRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiagAssocRprtIfIndex;
    uint8_t uc_dot11WNMDiagAssocRprtEventStatus;
    uint8_t auc_dot11WNMDiagAssocRprtBssid[NUM_6_BYTES];
    uint32_t dot11WNMDiagAssocRprtOperatingClass;
    uint32_t dot11WNMDiagAssocRprtChannelNumber;
    uint32_t dot11WNMDiagAssocRprtStatusCode;

    /* dot11WNMDiag8021xAuthReportTable ::= { dot11WNMReport 12 } */
    uint32_t dot11WNMDiag8021xAuthRprtIndex;
    uint8_t auc_dot11WNMDiag8021xAuthRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMDiag8021xAuthRprtIfIndex;
    uint8_t uc_dot11WNMDiag8021xAuthRprtEventStatus;
    uint8_t auc_dot11WNMDiag8021xAuthRprtBssid[NUM_6_BYTES];
    uint32_t dot11WNMDiag8021xAuthRprtOperatingClass;
    uint32_t dot11WNMDiag8021xAuthRprtChannelNumber;
    uint32_t dot11WNMDiag8021xAuthRprtEapType;
    uint8_t auc_dot11WNMDiag8021xAuthRprtEapVendorID[NUM_3_BYTES];
    uint8_t auc_dot11WNMDiag8021xAuthRprtEapVendorType[NUM_4_BYTES];
    uint8_t uc_dot11WNMDiag8021xAuthRprtCredentialType;
    uint32_t dot11WNMDiag8021xAuthRprtStatusCode;

    /* dot11WNMLocConfigReportTable ::= { dot11WNMReport 13 } */
    uint32_t dot11WNMLocConfigRprtIndex;
    uint8_t auc_dot11WNMLocConfigRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMLocConfigRprtIfIndex;
    uint8_t auc_dot11WNMLocConfigRprtLocIndParams[NUM_16_BYTES];
    uint8_t auc_dot11WNMLocConfigRprtLocIndChanList[254]; /* 数组大小254字节 */
    uint32_t dot11WNMLocConfigRprtLocIndBcastRate;
    uint8_t auc_dot11WNMLocConfigRprtLocIndOptions[NUM_255_BYTES];
    uint8_t uc_dot11WNMLocConfigRprtStatusConfigSubelemId;
    uint8_t uc_dot11WNMLocConfigRprtStatusResult;
    uint8_t auc_dot11WNMLocConfigRprtVendorSpecificRprtContent[NUM_255_BYTES];

    /* dot11WNMBssTransitReportTable ::= { dot11WNMReport 14 } */
    uint32_t dot11WNMBssTransitRprtIndex;
    uint8_t auc_dot11WNMBssTransitRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMBssTransitRprtIfIndex;
    uint8_t uc_dot11WNMBssTransitRprtStatusCode;
    uint32_t dot11WNMBssTransitRprtBSSTerminationDelay;
    uint8_t auc_dot11WNMBssTransitRprtTargetBssid[NUM_6_BYTES];
    uint8_t auc_dot11WNMBssTransitRprtCandidateList[2304]; /* 数组大小2304字节 */

    /* 备注: 没有找到 dot11WNMReport 15 */
    /* dot11WNMColocInterfReportTable ::= { dot11WNMReport 16 } */
    uint32_t dot11WNMColocInterfRprtIndex;
    uint8_t auc_dot11WNMColocInterfRprtRqstToken[WLAN_MIB_TOKEN_STRING_MAX_LENGTH];
    uint32_t dot11WNMColocInterfRprtIfIndex;
    uint32_t dot11WNMColocInterfRprtPeriod;
    int32_t l_dot11WNMColocInterfRprtInterfLevel;
    uint32_t dot11WNMColocInterfRprtInterfAccuracy;
    uint32_t dot11WNMColocInterfRprtInterfIndex;
    int32_t l_dot11WNMColocInterfRprtInterfInterval;
    int32_t l_dot11WNMColocInterfRprtInterfBurstLength;
    int32_t l_dot11WNMColocInterfRprtInterfStartTime;
    int32_t l_dot11WNMColocInterfRprtInterfCenterFreq;
    uint32_t dot11WNMColocInterfRprtInterfBandwidth;
} wlan_mib_dot11WirelessNetworkManagement_stru;

#endif /* end of wlan_mib_wnm.h */
