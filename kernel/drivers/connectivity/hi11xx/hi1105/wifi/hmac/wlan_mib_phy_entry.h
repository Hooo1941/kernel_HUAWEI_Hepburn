/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : 定义MIB元素PHY ENTRY结构及其对应枚举 的头文件
 * 作者       :
 * 创建日期   : 2022年10月11日
 */

#ifndef WLAN_MIB_PHY_ENTRY_H
#define WLAN_MIB_PHY_ENTRY_H

#include "oal_types.h"
#include "wlan_types.h"
#include "hd_event.h"

/****************************************************************************************/
/*          Start of dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 }                  */
/*              --  PHY GROUPS                                                          */
/*              --  dot11PhyOperationTable ::= { dot11phy 1 }                           */
/*              --  dot11PhyAntennaTable ::= { dot11phy 2 }                             */
/*              --  dot11PhyTxPowerTable ::= { dot11phy 3 }                             */
/*              --  dot11PhyFHSSTable ::= { dot11phy 4 }                                */
/*              --  dot11PhyDSSSTable ::= { dot11phy 5 }                                */
/*              --  dot11PhyIRTable ::= { dot11phy 6 }                                  */
/*              --  dot11RegDomainsSupportedTable ::= { dot11phy 7 }                    */
/*              --  dot11AntennasListTable ::= { dot11phy 8 }                           */
/*              --  dot11SupportedDataRatesTxTable ::= { dot11phy 9 }                   */
/*              --  dot11SupportedDataRatesRxTable ::= { dot11phy 10 }                  */
/*              --  dot11PhyOFDMTable ::= { dot11phy 11 }                               */
/*              --  dot11PhyHRDSSSTable ::= { dot11phy 12 }                             */
/*              --  dot11HoppingPatternTable ::= { dot11phy 13 }                        */
/*              --  dot11PhyERPTable ::= { dot11phy 14 }                                */
/*              --  dot11PhyHTTable  ::= { dot11phy 15 }                                */
/*              --  dot11SupportedMCSTxTable ::= { dot11phy 16 }                        */
/*              --  dot11SupportedMCSRxTable ::= { dot11phy 17 }                        */
/*              --  dot11TransmitBeamformingConfigTable ::= { dot11phy 18 }             */
/*              -- dot11PhyVHTTable ::= { dot11phy 23 } (802.11 ac)                     */
/*              -- dot11VHTTransmitBeamformingConfigTable ::= { dot11phy 24 }(802.11 ac) */
/****************************************************************************************/
/****************************************************************************************/
/* dot11PhyOperationTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11PhyOperationEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "PHY level attributes concerned with operation. Implemented as a table          */
/*      indexed on ifIndex to allow for multiple instantiations on an Agent."           */
/* ::= { dot11phy 1 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint8_t uc_dot11PHYType;           /* dot11PHYType INTEGER */
    uint32_t dot11CurrentRegDomain; /* dot11CurrentRegDomain Unsigned32 */
    uint8_t uc_dot11TempType;          /* dot11TempType INTEGER */
} wlan_mib_Dot11PhyOperationEntry_stru;

/****************************************************************************************/
/* dot11PhyAntennaTable OBJECT-TYPE                                                     */
/* SYNTAX SEQUENCE OF Dot11PhyAntennaEntry                                              */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for PhyAntenna. Implemented as a table indexed on          */
/*      ifIndex to allow for multiple instances on an agent."                           */
/* ::= { dot11phy 2}                                                                    */
/****************************************************************************************/
typedef struct {
    // uint32_t          dot11CurrentTxAntenna; /* dot11CurrentTxAntenna Unsigned32 */
    // uint8_t           uc_dot11DiversitySupportImplemented;
    // uint32_t          dot11CurrentRxAntenna;
    oal_bool_enum_uint8 en_dot11AntennaSelectionOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitExplicitCSIFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitIndicesFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitCSIFeedbackASOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ReceiveAntennaSelectionOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitSoundingPPDUOptionImplemented : 1;
    oal_bool_enum_uint8 en_resv : 2;

    oal_bool_enum_uint8 resv[NUM_3_BYTES];
    // oal_bool_enum_uint8 en_dot11TransmitIndicesComputationASOptionImplemented;
    // uint32_t          dot11NumberOfActiveRxAntennas;
} wlan_mib_Dot11PhyAntennaEntry_stru;

/****************************************************************************************/
/* dot11PhyTxPowerTable OBJECT-TYPE                                                     */
/* SYNTAX SEQUENCE OF Dot11PhyTxPowerEntry                                              */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyTxPowerTable. Implemented as a table           */
/*      indexed on STA ID to allow for multiple instances on an Agent."                 */
/* ::= { dot11phy 3}                                                                    */
/****************************************************************************************/
typedef struct {
    uint32_t dot11NumberSupportedPowerLevelsImplemented; /* dot11NumberSupportedPowerLevelsImplemented */
    uint32_t dot11TxPowerLevel1;                         /* dot11TxPowerLevel1 Unsigned32 */
    uint32_t dot11TxPowerLevel2;                         /* dot11TxPowerLevel2 Unsigned32 */
    uint32_t dot11TxPowerLevel3;                         /* dot11TxPowerLevel3 Unsigned32 */
    uint32_t dot11TxPowerLevel4;                         /* dot11TxPowerLevel4 Unsigned32 */
    uint32_t dot11TxPowerLevel5;                         /* dot11TxPowerLevel5 Unsigned32 */
    uint32_t dot11TxPowerLevel6;                         /* dot11TxPowerLevel6 Unsigned32 */
    uint32_t dot11TxPowerLevel7;                         /* dot11TxPowerLevel7 Unsigned32 */
    uint32_t dot11TxPowerLevel8;                         /* dot11TxPowerLevel8 Unsigned32 */
    uint32_t dot11CurrentTxPowerLevel;                   /* dot11CurrentTxPowerLevel Unsigned32 */
    uint8_t auc_dot11TxPowerLevelExtended;                  /* dot11TxPowerLevelExtended OCTET STRING */
    uint32_t dot11CurrentTxPowerLevelExtended;           /* dot11CurrentTxPowerLevelExtended Unsigned32 */
} wlan_mib_Dot11PhyTxPowerEntry_stru;

/****************************************************************************************/
/* dot11PhyFHSSTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyFHSSEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyFHSSTable. Implemented as a table indexed      */
/*      on STA ID to allow for multiple instances on an Agent."                         */
/* ::= { dot11phy 4 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11HopTime;                            /* dot11HopTime Unsigned32 */
    uint32_t dot11CurrentChannelNumber;               /* dot11CurrentChannelNumber Unsigned32 */
    uint32_t dot11MaxDwellTime;                       /* dot11MaxDwellTime Unsigned32 */
    uint32_t dot11CurrentDwellTime;                   /* dot11CurrentDwellTime Unsigned32 */
    uint32_t dot11CurrentSet;                         /* dot11CurrentSet Unsigned32 */
    uint32_t dot11CurrentPattern;                     /* dot11CurrentPattern Unsigned32 */
    uint32_t dot11CurrentIndex;                       /* dot11CurrentIndex Unsigned32 */
    uint32_t dot11EHCCPrimeRadix;                     /* dot11EHCCPrimeRadix Unsigned32 */
    uint32_t dot11EHCCNumberofChannelsFamilyIndex;    /* dot11EHCCNumberofChannelsFamilyIndex Unsigned32 */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityImplemented; /* dot11EHCCCapabilityImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11EHCCCapabilityActivated;   /* dot11EHCCCapabilityActivated TruthValue */
    uint8_t uc_dot11HopAlgorithmAdopted;                 /* dot11HopAlgorithmAdopted INTEGER */
    oal_bool_enum_uint8 en_dot11RandomTableFlag;           /* dot11RandomTableFlag TruthValue */
    uint32_t dot11NumberofHoppingSets;                /* dot11NumberofHoppingSets Unsigned32 */
    uint32_t dot11HopModulus;                         /* dot11HopModulus Unsigned32 */
    uint32_t dot11HopOffset;                          /* dot11HopOffset Unsigned32 */
} wlan_mib_Dot11PhyFHSSEntry_stru;

/****************************************************************************************/
/* dot11PhyIRTable OBJECT-TYPE                                                          */
/* SYNTAX SEQUENCE OF Dot11PhyIREntry                                                   */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyIRTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 6 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11CCAWatchdogTimerMax; /* dot11CCAWatchdogTimerMax Unsigned32 */
    uint32_t dot11CCAWatchdogCountMax; /* dot11CCAWatchdogCountMax Unsigned32 */
    uint32_t dot11CCAWatchdogTimerMin; /* dot11CCAWatchdogTimerMin Unsigned32 */
    uint32_t dot11CCAWatchdogCountMin; /* dot11CCAWatchdogCountMin Unsigned32 */
} wlan_mib_Dot11PhyIREntry_stru;

/****************************************************************************************/
/* dot11RegDomainsSupportedTable OBJECT-TYPE                                            */
/* SYNTAX SEQUENCE OF Dot11RegDomainsSupportedEntry                                     */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS deprecated                                                                    */
/* DESCRIPTION                                                                          */
/*      "Superceded by dot11OperatingClassesTable.                                      */
/*      There are different operational requirements dependent on the regulatory        */
/*      domain. This attribute list describes the regulatory domains the PLCP and       */
/*      PMD support in this implementation. Currently defined values and their          */
/*      corresponding Regulatory Domains are:                                           */
/*      FCC (USA) = X'10', DOC (Canada) = X'20', ETSI (most of Europe) = X'30',         */
/*      Spain = X'31', France = X'32', Japan = X'40', China = X'50', Other = X'00'"     */
/* ::= { dot11phy 7}                                                                    */
/****************************************************************************************/
typedef struct {
    uint32_t dot11RegDomainsSupportedIndex;  /* dot11RegDomainsSupportedIndex Unsigned32 */
    uint8_t uc_dot11RegDomainsImplementedValue; /* dot11RegDomainsImplementedValue INTEGER */
} wlan_mib_Dot11RegDomainsSupportedEntry_stru;

/****************************************************************************************/
/* dot11AntennasListTable OBJECT-TYPE                                                   */
/* SYNTAX SEQUENCE OF Dot11AntennasListEntry                                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "This table represents the list of antennae. An antenna can be marked to        */
/*      be capable of transmitting, receiving, and/or for participation in receive      */
/*      diversity. Each entry in this table represents a single antenna with its        */
/*      properties. The maximum number of antennae that can be contained in this        */
/*      table is 255."                                                                  */
/* ::= { dot11phy 8 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11AntennaListIndex;                /* dot11AntennaListIndex Unsigned32 */
    uint32_t dot11TxAntennaImplemented;            /* dot11TxAntennaImplemented TruthValue */
    uint32_t dot11RxAntennaImplemented;            /* dot11RxAntennaImplemented TruthValue */
    uint32_t dot11DiversitySelectionRxImplemented; /* dot11DiversitySelectionRxImplemented TruthValue */
} wlan_mib_Dot11AntennasListEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesTxTable OBJECT-TYPE                                           */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesTxEntry                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The Transmit bit rates supported by the PLCP and PMD, represented by a         */
/*      count from X'02-X'7f, corresponding to data rates in increments of              */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s subject to limitations of each individ-      */
/*      ual PHY."                                                                       */
/* ::= { dot11phy 9 }                                                                   */
/****************************************************************************************/
typedef struct {
    uint32_t dot11SupportedDataRatesTxIndex;   /* dot11SupportedDataRatesTxIndex Unsigned32 */
    uint32_t dot11ImplementedDataRatesTxValue; /* dot11ImplementedDataRatesTxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedDataRatesRxTable OBJECT-TYPE                                           */
/* SYNTAX SEQUENCE OF Dot11SupportedDataRatesRxEntry                                    */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The receive bit rates supported by the PLCP and PMD, represented by a          */
/*      count from X'002-X'7f, corresponding to data rates in increments of             */
/*      500kbit/s from 1 Mb/s to 63.5 Mb/s."                                            */
/* ::= { dot11phy 10 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11SupportedDataRatesRxIndex;   /* dot11SupportedDataRatesRxIndex Unsigned32 */
    uint32_t dot11ImplementedDataRatesRxValue; /* dot11ImplementedDataRatesRxValue Unsigned32 */
} wlan_mib_Dot11SupportedDataRatesRxEntry_stru;

/****************************************************************************************/
/* dot11PhyOFDMTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyOFDMEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Group of attributes for dot11PhyOFDMTable. Implemented as a table indexed      */
/*      on ifindex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 11 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11CurrentFrequency;                       /* dot11CurrentFrequency Unsigned32 */
    int32_t l_dot11TIThreshold;                              /* dot11TIThreshold Integer32 */
    uint32_t dot11FrequencyBandsImplemented;              /* dot11FrequencyBandsImplemented Unsigned32 */
    uint32_t dot11ChannelStartingFactor;                  /* dot11ChannelStartingFactor Unsigned32 */
    oal_bool_enum_uint8 en_dot11FiveMHzOperationImplemented;   /* dot11FiveMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TenMHzOperationImplemented;    /* dot11TenMHzOperationImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TwentyMHzOperationImplemented; /* dot11TwentyMHzOperationImplemented TruthValue */
    uint8_t uc_dot11PhyOFDMChannelWidth;                     /* dot11PhyOFDMChannelWidth INTEGER */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDImplemented;          /* dot11OFDMCCAEDImplemented  TruthValue */
    oal_bool_enum_uint8 en_dot11OFDMCCAEDRequired;             /* dot11OFDMCCAEDRequired  TruthValue */
    uint32_t dot11OFDMEDThreshold;                        /* dot11OFDMEDThreshold  Unsigned32 */
    uint8_t uc_dot11STATransmitPowerClass;                   /* dot11STATransmitPowerClass INTEGER */
    uint8_t uc_dot11ACRType;                                 /* dot11ACRType INTEGER */
} wlan_mib_Dot11PhyOFDMEntry_stru;

/****************************************************************************************/
/* dot11PhyHRDSSSTable OBJECT-TYPE                                                      */
/* SYNTAX SEQUENCE OF Dot11PhyHRDSSSEntry                                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHRDSSSEntry. Implemented as a table            */
/*      indexed on ifIndex to allow for multiple instances on an Agent."                */
/* ::= { dot11phy 12 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ShortPreambleOptionImplemented; /* dot11ShortPreambleOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11PBCCOptionImplemented;          /* dot11PBCCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ChannelAgilityPresent;          /* dot11ChannelAgilityPresent TruthValue */
    // oal_bool_enum_uint8 en_dot11ChannelAgilityActivated;        /* dot11ChannelAgilityActivated TruthValue */
    // uint32_t  dot11HRCCAModeImplemented;                   /* dot11HRCCAModeImplemented Unsigned32 */
} wlan_mib_Dot11PhyHRDSSSEntry_stru;

/****************************************************************************************/
/* dot11HoppingPatternTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11HoppingPatternEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The (conceptual) table of attributes necessary for a frequency hopping         */
/*      implementation to be able to create the hopping sequences necessary to          */
/*      operate in the subband for the associated domain country string."               */
/* ::= { dot11phy 13 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint32_t dot11HoppingPatternIndex;    /* dot11HoppingPatternIndex Unsigned32 */
    uint32_t dot11RandomTableFieldNumber; /* dot11RandomTableFieldNumber Unsigned32 */
} wlan_mib_Dot11HoppingPatternEntry_stru;

/****************************************************************************************/
/* dot11PhyERPTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyERPEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyERPEntry. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 14 }                                                                  */
/****************************************************************************************/
typedef struct {
    // oal_bool_enum_uint8 en_dot11ERPPBCCOptionImplemented;       /* dot11ERPPBCCOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11ERPBCCOptionActivated;          /* dot11ERPBCCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11DSSSOFDMOptionImplemented;      /* dot11DSSSOFDMOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11DSSSOFDMOptionActivated;        /* dot11DSSSOFDMOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionImplemented; /* dot11ShortSlotTimeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11ShortSlotTimeOptionActivated;   /* dot11ShortSlotTimeOptionActivated TruthValue */
} wlan_mib_Dot11PhyERPEntry_stru;

/****************************************************************************************/
/* dot11PhyDSSSTable OBJECT-TYPE                                                        */
/* SYNTAX SEQUENCE OF Dot11PhyDSSSEntry                                                 */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyDSSSEntry. Implemented as a table indexed      */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 5 }                                                                   */
/****************************************************************************************/
typedef struct {
    wlan_mib_Dot11PhyHRDSSSEntry_stru st_phy_hrdsss;
    wlan_mib_Dot11PhyERPEntry_stru st_phy_erp;
} wlan_mib_Dot11PhyDSSSEntry_stru;

/****************************************************************************************/
/* dot11TransmitBeamformingConfigTable OBJECT-TYPE                                      */
/* SYNTAX SEQUENCE OF Dot11TransmitBeamformingConfigEntry                               */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11TransmitBeamformingConfigTable. Implemented       */
/*      as a table indexed on ifIndex to allow for multiple instances on an             */
/*      Agent."                                                                         */
/* ::= { dot11phy 18 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11ReceiveStaggerSoundingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitStaggerSoundingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ReceiveNDPOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11TransmitNDPOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ImplicitTransmitBeamformingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitCSITransmitBeamformingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented : 1;
    oal_bool_enum_uint8 en_resv : 1;

    oal_bool_enum_uint8 uc_dot11CalibrationOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented : 2;
    oal_bool_enum_uint8 uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented : 2;

    uint8_t uc_dot11NumberBeamFormingCSISupportAntenna;
    uint8_t uc_dot11NumberNonCompressedBeamformingMatrixSupportAntenna;
    uint8_t uc_dot11NumberCompressedBeamformingMatrixSupportAntenna;
} wlan_mib_Dot11TransmitBeamformingConfigEntry_stru;
/****************************************************************************************/
/* dot11PhyHTTable OBJECT-TYPE                                                          */
/* SYNTAX SEQUENCE OF Dot11PhyHTEntry                                                   */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11PhyHTTable. Implemented as a table indexed        */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 15 }                                                                  */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot112GFortyMHzOperationImplemented : 1;    /* dot11FortyMHzOperationImplemented */
    oal_bool_enum_uint8 en_dot115GFortyMHzOperationImplemented : 1;    /* dot11FortyMHzOperationImplemented */
    oal_bool_enum_uint8 en_dot11HTGreenfieldOptionImplemented : 1;     /* dot11HTGreenfieldOptionImplemented */
    oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyImplemented : 1;  /* dot11ShortGIOptionInTwentyImplemented */
    oal_bool_enum_uint8 en_dot112GShortGIOptionInFortyImplemented : 1; /* dot11ShortGIOptionInFortyImplemented */
    oal_bool_enum_uint8 en_dot115GShortGIOptionInFortyImplemented : 1; /* dot11ShortGIOptionInFortyImplemented */
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionImplemented : 1;       /* dot11LDPCCodingOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11LDPCCodingOptionActivated : 1;         /* dot11LDPCCodingOptionActivated TruthValue */

    oal_bool_enum_uint8 en_dot11TxSTBCOptionImplemented : 1;      /* dot11TxSTBCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TxSTBCOptionActivated : 1;        /* dot11TxSTBCOptionActivated TruthValue */
    oal_bool_enum_uint8 en_dot11RxSTBCOptionImplemented : 1;      /* dot11RxSTBCOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11TxMCSSetDefined : 1;              /* dot11TxMCSSetDefined TruthValue */
    oal_bool_enum_uint8 en_dot11TxRxMCSSetNotEqual : 1;           /* dot11TxRxMCSSetNotEqual TruthValue */
    oal_bool_enum_uint8 en_dot11TxUnequalModulationSupported : 1; /* dot11TxUnequalModulationSupported TruthValue */
    oal_bool_enum_uint8 en_resv : 2;

    // oal_bool_enum_uint8 en_dot11FortyMHzOperationActivated;        /* dot11FortyMHzOperationActivated TruthValue */
    // uint32_t          dot11CurrentPrimaryChannel;             /* dot11CurrentPrimaryChannel Unsigned32 */
    // uint32_t          dot11CurrentSecondaryChannel;           /* dot11CurrentSecondaryChannel Unsigned32 */
    // uint32_t          dot11NumberOfSpatialStreamsImplemented; /* dot11NumberOfSpatialStreamsImplemented */
    // uint32_t          dot11NumberOfSpatialStreamsActivated;   /* dot11NumberOfSpatialStreamsActivated */
    // oal_bool_enum_uint8 en_dot11HTGreenfieldOptionActivated;       /* dot11HTGreenfieldOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInTwentyActivated;    /* dot11ShortGIOptionInTwentyActivated */
    // oal_bool_enum_uint8 en_dot11ShortGIOptionInFortyActivated;     /* dot11ShortGIOptionInFortyActivated */
    // oal_bool_enum_uint8 en_dot11RxSTBCOptionActivated;             /* dot11RxSTBCOptionActivated TruthValue */
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionImplemented;      /* dot11BeamFormingOptionImplemented TruthValue */
    // oal_bool_enum_uint8 en_dot11BeamFormingOptionActivated;        /* dot11BeamFormingOptionActivated TruthValue */
    uint32_t dot11HighestSupportedDataRate;               /* dot11HighestSupportedDataRate Unsigned32 */
    uint32_t dot11TxMaximumNumberSpatialStreamsSupported; /* dot11TxMaximumNumberSpatialStreamsSupported */

    wlan_mib_Dot11TransmitBeamformingConfigEntry_stru st_wlan_mib_txbf_config;
} wlan_mib_Dot11PhyHTEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSTxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSTxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "he Transmit MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 16 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t auc_dot11SupportedMCSTxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSTxValue Unsigned32 */
} wlan_mib_Dot11SupportedMCSTxEntry_stru;

/****************************************************************************************/
/* dot11SupportedMCSRxTable OBJECT-TYPE                                                 */
/* SYNTAX SEQUENCE OF Dot11SupportedMCSRxEntry                                          */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "The receive MCS supported by the PLCP and PMD, represented by a count          */
/*      from 1 to 127, subject to limitations of each individual PHY."                  */
/* ::= { dot11phy 17 }                                                                  */
/****************************************************************************************/
typedef struct {
    // uint32_t  dot11SupportedMCSRxIndex;                             /* dot11SupportedMCSRxIndex Unsigned32 */
    uint8_t auc_dot11SupportedMCSRxValue[WLAN_HT_MCS_BITMASK_LEN]; /* dot11SupportedMCSRxValue Unsigned32 */
    // uint8_t   uc_num_rx_mcs;
} wlan_mib_Dot11SupportedMCSRxEntry_stru;

typedef struct {
    wlan_mib_Dot11SupportedMCSTxEntry_stru st_supported_mcstx;
    wlan_mib_Dot11SupportedMCSRxEntry_stru st_supported_mcsrx;
} wlan_mib_Dot11SupportedMCSEntry_stru;
/****************************************************************************************/
/* dot11VHTTransmitBeamformingConfigTable OBJECT-TYPE                                   */
/* SYNTAX SEQUENCE OF Dot11VHTTransmitBeamformingConfigEntry                            */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/* DESCRIPTION                                                                          */
/*      "Entry of attributes for dot11VHTTransmitBeamformingConfigTable. Imple-         */
/*      mented as a table indexed on ifIndex to allow for multiple instances on an      */
/*      Agent."                                                                         */
/*  ::= { dot11phy 24 }                                                                 */
/****************************************************************************************/
typedef struct {
    oal_bool_enum_uint8 en_dot11VHTSUBeamformeeOptionImplemented; /* dot11VHTSUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTSUBeamformerOptionImplemented; /* dot11VHTSUBeamformerOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformeeOptionImplemented; /* dot11VHTMUBeamformeeOptionImplemented TruthValue */
    oal_bool_enum_uint8 en_dot11VHTMUBeamformerOptionImplemented; /* dot11VHTMUBeamformerOptionImplemented TruthValue */
    uint8_t uc_dot11VHTNumberSoundingDimensions;                /* dot11VHTNumberSoundingDimensions  Unsigned32 */
    uint8_t uc_dot11VHTBeamformeeNTxSupport;                    /* dot11VHTBeamformeeNTxSupport Unsigned32 */
} wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru;

/****************************************************************************************/
/* dot11PhyVHTTable OBJECT-TYPE                                                         */
/* SYNTAX SEQUENCE OF Dot11PhyVHTEntry                                                  */
/* MAX-ACCESS not-accessible                                                            */
/* STATUS current                                                                       */
/*      DESCRIPTION                                                                     */
/*      "Entry of attributes for dot11PhyVHTTable. Implemented as a table indexed       */
/*      on ifIndex to allow for multiple instances on an Agent."                        */
/* ::= { dot11phy 23 }                                                                  */
/****************************************************************************************/
typedef struct {
    uint8_t en_dot11VHTChannelWidthOptionImplemented : 2; /* wlan_mib_vht_supp_width_enum */
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Implemented : 1;
    oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Implemented : 1;
    oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionImplemented : 1;
    oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionImplemented : 1;
    uint8_t en_rev : 1;

    uint8_t resv[NUM_3_BYTES];
    // uint8_t           uc_dot11CurrentChannelwidth;                        /* dot11CurrentChannelWidth INTEGER */
    // uint32_t          dot11CurrentChannelCenterFrequencyIndex0;
    // uint32_t          dot11CurrentChannelCenterFrequencyIndex1;
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn80Activated;              /* dot11VHTShortGIOptionIn80Activated */
    // oal_bool_enum_uint8 en_dot11VHTShortGIOptionIn160and80p80Activated;
    // oal_bool_enum_uint8 en_dot11VHTLDPCCodingOptionActivated;               /* dot11VHTLDPCCodingOptionActivated */
    // oal_bool_enum_uint8 en_dot11VHTTxSTBCOptionActivated;                   /* dot11VHTTxSTBCOptionActivated */
    // oal_bool_enum_uint8 en_dot11VHTRxSTBCOptionActivated;                   /* dot11VHTRxSTBCOptionActivated */
    // uint32_t          dot11VHTMUMaxUsersImplemented;                   /* dot11VHTMUMaxUsersImplemented */
    // uint32_t          dot11VHTMUMaxNSTSPerUserImplemented;             /* dot11VHTMUMaxNSTSPerUserImplemented */
    // uint32_t          dot11VHTMUMaxNSTSTotalImplemented;               /* dot11VHTMUMaxNSTSTotalImplemented */
    wlan_mib_Dot11VHTTransmitBeamformingConfigEntry_stru st_wlan_mib_vht_txbf_config;
} wlan_mib_Dot11PhyVHTEntry_stru;

#endif /* end of wlan_mib_phy_entry.h */
