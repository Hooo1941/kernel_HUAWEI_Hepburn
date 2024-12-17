/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_MACRO_H
#define MAC_FRAME_MACRO_H

/* 1 其他头文件包含 */
#include "wlan_types.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"
#include "securectype.h"
#include "mac_element.h"

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
/* 2 宏定义 */
#define MAC_80211_FRAME_LEN               24 /* 非四地址情况下，MAC帧头的长度 */
#define MAC_80211_CTL_HEADER_LEN          16 /* 控制帧帧头长度 */
#define MAC_80211_QOS_FRAME_LEN           26
#define MAC_80211_QOS_HTC_FRAME_LEN       30
#define MAC_80211_QOS_HTC_4ADDR_FRAME_LEN 36

/* 信息元素长度定义 */
#define MAC_IE_EXT_HDR_LEN         3  /* 信息元素头部 1字节EID + 1字节长度 + 1字节EXT_EID */
#define MAC_IE_HDR_LEN             2  /* 信息元素头部 1字节EID + 1字节长度 */
#define MAC_NEIGHBOR_REPORT_IE_LEN 13 /* NEIGHBOR_REPORT长度 */
#define MAC_TIME_STAMP_LEN         8
#define MAC_BEACON_INTERVAL_LEN    2
#define MAC_CAP_INFO_LEN           2
#define MAC_MAX_SUPRATES           8 /* WLAN_EID_RATES最大支持8个速率 */
#define MAC_DSPARMS_LEN            1 /* ds parameter set 长度 */
#define MAC_MIN_TIM_LEN            4
#define MAC_MIN_RSN_LEN            12
#define MAC_MAX_RSN_LEN            64
#define MAC_MIN_WPA_LEN            12
#define MAC_PWR_CONSTRAINT_LEN     1 /* 功率限制ie长度为1 */
#define MAC_TPCREP_IE_LEN          2
#define MAC_ERP_IE_LEN             1
#define MAC_OBSS_SCAN_IE_LEN       14
#define MAC_VHT_IE_CAP_INFO_LEN    4
#define MAC_VHT_IE_SUPP_MCS_NSS_LEN 8
#define MAC_2040_COEX_IE_INFO_LEN  1
#define MAC_DSSS_PARAMTER_SET_IE_LEN 1
#define MAC_RRM_LINK_MAX_TX_PWR_OFFSET 5
#define MAC_HT_OPERATION_SEC_CHAN_OFFSET 2
/* 商用网卡（Tplink6200）关联不上APUT */
#ifdef _PRE_WLAN_FEATURE_11AX
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9, twt特性需要长度为10 */
#define MAC_XCAPS_EX_TWT_LEN 10
#endif
#define MAC_HISI_CAP_VENDOR_IE_LEN 7   /* hisi cap vendor ie字段长度 */ /* mp15所用私有ie长度，06长度为10 */

/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_FTM_LEN 9
/* 初始值为5，由于11ac Operating Mode Notification特性标志为bit62长度修改为8,ftm特性需要长度为9 */
#define MAC_XCAPS_EX_LEN  8
#define MAC_WMM_PARAM_LEN 24 /* WMM parameters ie */

#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_WMMAC_INFO_LEN  61 /* WMMAC info ie */
#define MAC_WMMAC_TSPEC_LEN 55 /* TSPEC元素长度 */
#endif
#define MAC_QOS_INFO_LEN                      1
#define MAC_AC_PARAM_LEN                      4
#define MAC_COUNTRY_REG_FIELD_LEN             3
#define MAC_VHT_CAP_IE_LEN                    12
#define MAC_VHT_INFO_IE_LEN                   5
#define MAC_VHT_CAP_INFO_FIELD_LEN            4
#define MAC_VHT_OPERN_LEN                     5 /* vht opern ie length */
#define MAC_2040_COEX_LEN                     1 /* 20/40 BSS Coexistence element */
#define MAC_2040_INTOLCHREPORT_LEN_MIN        1 /* 20/40 BSS Intolerant Channel Report element */
#define MAC_CHANSWITCHANN_LEN                 3 /* Channel Switch Announcement element */
#define MAC_SA_QUERY_LEN                      4 /* SA Query element len */
#define MAC_HT_NOTIFY_CHANNEL_WIDTH_LEN       3
#define MAC_MIN_WPS_IE_LEN                    5

/* action帧相关定义 */
#define MAC_ACTION_CATEGORY_LEN               1
#define MAC_ACTION_CATEGORY_AND_CODE_LEN      2
#define MAC_ACTION_VENDOR_TYPE_OFFSET         5
#define MAC_ACTION_VENDOR_TYPE_LEN            1

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
#define MAC_OPMODE_NOTIFY_LEN 1 /* Operating Mode Notification element len */
#endif
#define MAC_MOBILITY_DOMAIN_LEN 5

/* auth帧相关定义 */
#define MAC_AUTH_SEQ_LEN 2 /* auth帧payload中auth seq字段的长度 */
#define MAC_AUTH_ALG_LEN 2 /* auth帧中algorithm字段的长度 */
#define MAC_AUTH_STATUS_LEN 2 /* auth帧中status字段的长度 */

/* RSN信息元素相关定义 */
#define MAC_RSN_IE_VERSION 1
#define MAC_RSN_CAP_LEN    2

/* OUI相关定义 */
#define MAC_OUI_LEN     3
#define MAC_OUITYPE_LEN 1
#define MAC_OUITYPE_WPA 1
#define MAC_OUITYPE_WMM 2
#define MAC_OUITYPE_P2P 9
#define MAC_WMM_OUI_BYTE_ONE            0x00
#define MAC_WMM_OUI_BYTE_TWO            0x50
#define MAC_WMM_OUI_BYTE_THREE          0xF2
#define MAC_OUISUBTYPE_WMM_INFO         0
#define MAC_OUISUBTYPE_WMM_PARAM        1
#define MAC_OUISUBTYPE_WMM_PARAM_OFFSET 6 /* wmm 字段中EDCA_INFO位置,表示是否携带EDCA参数 偏移6 */
#define MAC_OUI_WMM_VERSION             1
#define MAC_WMM_IE_LEN                  7 /* wmm ie长度为7 */
#define MAC_HT_CAP_LEN            26 /* HT能力信息长度为26 */
#define MAC_HT_CAPINFO_LEN        2  /* HT Capabilities Info域长度为2 */
#define MAC_HT_AMPDU_PARAMS_LEN   1  /* A-MPDU parameters域长度为1 */
#define MAC_HT_SUP_MCS_SET_LEN    16 /* Supported MCS Set域长度为16 */
#define MAC_HT_EXT_CAP_LEN        2  /* Extended cap.域长度为2 */
#define MAC_HT_TXBF_CAP_LEN       4  /* Transmit Beamforming Cap.域长度为4 */
#define MAC_HT_OPERN_LEN          22 /* HT Operation信息长度为22 */
#define MAC_HT_BASIC_MCS_SET_LEN  16 /* HT info中的basic mcs set信息的长度 */
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
#define MAC_PRIV_ACK_VENDOR_TYPE_MCAST_AMPDU 0xA0 /* 组播聚合私有ACK帧 */
#endif
#define MAC_EXT_CAP_IE_INTERWORKING_OFFSET   4
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
#define MAC_HE_MAC_CAP_LEN               6
#define MAC_HE_PHY_CAP_LEN               11
#define MAC_HE_OPE_PARAM_LEN             3
#define MAC_HE_OPE_BSS_COLOR_INFO_LEN    1
#define MAC_HE_OPE_BASIC_MCS_NSS_LEN     2
#define MAC_HE_VHT_OPERATION_INFO_LEN    3
#define MAC_HE_UORA_PARAMETER_SET_LEN    2
#define MAC_HE_6GHZ_BAND_CAP_LEN         2
#define MAC_HE_MU_EDCA_PARAMETER_SET_LEN 14
#define MAC_HE_OPERAION_MIN_LEN          7
#define MAC_HE_DEFAULT_PE_DURATION       7    /* 值5-7 reserved */
#define MAC_HE_DURATION_RTS_THRESHOLD    1023 /* 1023表示该字段不可用 */
#define PHY_DEVICE_CLASS_A         1  /* TB功率精度正负3dB */
#define HTC_ONLY_UPH_VALUE           0XFFFFC013  /* 仅含有UPH 字段 */
#define HTC_INVALID_VALUE            0XFFFFFFFF
#define HTC_WITH_OM_MAX_FRAME_NUMS   64
#define MAC_HTC_VHT_HE_PROTOCOL_CONTROL_BIT 2
#define MAC_HTC_HE_CONTROL_ID_BIT           4
#define MAC_HTC_HE_OM_LENGTH                12
#define MAC_HTC_HE_UPH_OR_CAS_LENGTH        8
#define MAC_HTC_HE_BQR_LENGTH               10
#define MAC_HTC_HE_PROTOCOL_BIT0_1  0x3 /* he control */
#ifdef _PRE_WLAN_FEATURE_HIEX
#define HTC_HIMIT_HE_VALUE           0X7B      /* HE模式himit插入HTC字段 */
#define HTC_HIMIT_VHT_VALUE          0X8000001 /* VHT模式himit插入HTC字段 */
#define HTC_HIMIT_HT_OR_LEGACY_VALUE 0X100000  /* HT/LEGACY模式himit插入HTC字段 */
#endif

/* EOCW Range: 0~7(3-bit), the following is EOCW default max/min */
#define MAC_HE_UORA_EOCW_MIN   3
#define MAC_HE_UORA_EOCW_MAX   5
/* OCW: (2^EOCW - 1) */
#define MAC_HE_UORA_OCW_MIN   1
#define MAC_HE_UORA_OCW_MAX   0x7F
#define MAC_80211_REASON_CODE_LEN  2
typedef enum {
    MAC_FRAME_HE_TRIG_TYPE_BASIC = 0,
    MAC_FRAME_HE_TRIG_TYPE_BRRP = 1,
    MAC_FRAME_HE_TRIG_TYPE_MU_BAR = 2,
    MAC_FRAME_HE_TRIG_TYPE_MU_RTS = 3,
    MAC_FRAME_HE_TRIG_TYPE_BSRP = 4,
    MAC_FRAME_HE_TRIG_TYPE_GCR_MUBAR = 5,
    MAC_FRAME_HE_TRIG_TYPE_BQRP = 6,
    MAC_FRAME_HE_TRIG_TYPE_NFRP = 7,

    MAC_FRAME_HE_TRIG_TYPE_BUTT
} mac_frame_he_trig_type_enum;
typedef uint8_t mac_frame_he_trig_type_enum_uint8;

typedef enum {
    MAC_HTC_A_CONTROL_TYPE_TRS = 0,
    MAC_HTC_A_CONTROL_TYPE_OM = 1,
    MAC_HTC_A_CONTROL_TYPE_HLA = 2,
    MAC_HTC_A_CONTROL_TYPE_BSRP = 3,
    MAC_HTC_A_CONTROL_TYPE_UPH = 4,
    MAC_HTC_A_CONTROL_TYPE_BQR = 5,
    MAC_HTC_A_CONTROL_TYPE_CAS = 6,
#ifdef _PRE_WLAN_FEATURE_HIEX
    MAC_HTC_A_CONTROL_TYPE_HIMIT = 14,
#endif
    MAC_HTC_A_CONTROL_TYPE_INVALID = 15
} mac_htc_a_contorl_type_enum;
typedef uint8_t mac_htc_a_contorl_type_enum_uint8;
#endif

#define MAC_QOS_CTRL_FIELD_OFFSET       24
#define MAC_QOS_CTRL_FIELD_OFFSET_4ADDR 30

/* EDCA参数相关的宏 */
#define MAC_WMM_QOS_PARAM_AIFSN_MASK                    0x0F
#define MAC_WMM_QOS_PARAM_ACI_BIT_OFFSET                5
#define MAC_WMM_QOS_PARAM_ACI_MASK                      0x03
#define MAC_WMM_QOS_PARAM_ECWMIN_MASK                   0x0F
#define MAC_WMM_QOS_PARAM_ECWMAX_MASK                   0xF0
#define MAC_WMM_QOS_PARAM_ECWMAX_BIT_OFFSET             4
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_MASK                0x00FF
#define MAC_WMM_QOS_PARAM_BIT_NUMS_OF_ONE_BYTE          8
#define MAC_WMM_QOS_PARAM_TXOPLIMIT_SAVE_TO_TRANS_TIMES 5

/* 关闭WMM后，所有帧进入此宏定义的队列 */
#define MAC_WMM_SWITCH_TID 6
/* IP协议相关 */
#define IP_HEADER_LEN_UNIT 4 /* IP头中首部长度的单位是4字节 */
#define ip_header_len(_ip_header_len) ((_ip_header_len) * IP_HEADER_LEN_UNIT)
/* TCP协议类型，chartiot tcp连接端口号 */
#define MAC_TCP_PROTOCAL 6
/* TCP包头的长度是tcp offset乘以4 */
#define tcp_header_len(_tcp_offset) ((_tcp_offset) << BIT_OFFSET_2)
#define MAC_UDP_PROTOCAL 17

/* ICMP协议报文 */
#define MAC_ICMP_PROTOCAL 1
/* huawei申请多个OUI;
   http://standards-oui.ieee.org/oui.txt */
#define MAC_WLAN_OUI_HUAWEI              0x00E0fC

#ifdef _PRE_WLAN_FEATURE_HIEX
#define MAC_HISI_HIEX_IE    0x81
#endif
#define MAC_HUAWEI_VENDER_IE      0xAC853D /* 打桩HW IE */
#define MAC_HISI_HISTREAM_IE      0x11     /* histream IE */ /* 废弃 */
#define MAC_HISI_LOCATION_CSI_IE  0x13
#define MAC_HISI_CAP_IE 0x20
#define MAC_HISI_1024QAM_IE 0xbd /* 1024QAM IE */
#define MAC_HISI_NB_IE      0x31 /* nb(narrow band) IE */ /* 废弃 */
#define MAC_HISI_SAP_IE 0xb0

#ifdef _PRE_WLAN_FEATURE_MBO
#define MAC_MBO_VENDOR_IE          0x506F9A /* WFA specific OUI */
#define MAC_MBO_IE_OUI_TYPE        0x16     /* Identifying the type and version of the MBO-OCE IE */
#define MAC_MBO_ATTRIBUTE_HDR_LEN  2        /* Attribute ID + Attribute Length */
#endif

#define MAC_IPV6_UDP_SRC_PORT 546
#define MAC_IPV6_UDP_DES_PORT 547
#define MAC_CSI_LOCATION_INFO_LEN   57
#define MAC_FTM_LOCATION_INFO_LEN   52
#define MAC_CSI_REPORT_HEADER_LEN   53
#define MAC_HISI_SAP_IE_LEN 8

/* ARP types, 1: ARP request, 2:ARP response, 3:RARP request, 4:RARP response */
#define MAC_ARP_REQUEST   0x0001
#define MAC_ARP_RESPONSE  0x0002

/* Neighbor Discovery */
#define MAC_ND_RSOL  133 /* Router Solicitation */
#define MAC_ND_RADVT 134 /* Router Advertisement */
#define MAC_ND_NSOL  135 /* Neighbor Solicitation */
#define MAC_ND_NADVT 136 /* Neighbor Advertisement */
#define MAC_ND_RMES  137 /* Redirect Message */

/* p2p相关 */
#define P2P_PAF_GON_REQ  0
#define WFA_OUI_BYTE1       0x50
#define WFA_OUI_BYTE2       0x6F
#define WFA_OUI_BYTE3       0x9A
#define WFA_P2P_V1_0        0x09

#define MAC_VHT_CHANGE (BIT1)
#define MAC_HT_CHANGE  (BIT2)
#define MAC_BW_CHANGE  (BIT3)
#ifdef _PRE_WLAN_FEATURE_11AX
#define MAC_HE_BSS_COLOR_CHANGE         (BIT7)
#define MAC_HE_PARTIAL_BSS_COLOR_CHANGE (BIT8)
#endif

#define MAC_NO_CHANGE        (0)
#define MAC_RADIO_MEAS_START_TIME_LEN 8

#ifdef _PRE_WLAN_FEATURE_PRIV_CLOSED_LOOP_TPC
#define MAC_HISI_ADJUST_POW_IE              0x50  /* 功率调整IE */
#endif

/* 3 枚举定义 */
typedef enum {
    MAC_AP_TYPE_NORMAL = BIT0,
    MAC_AP_TYPE_GOLDENAP = BIT1,
    MAC_AP_TYPE_DDC_WHITELIST = BIT2,
    MAC_AP_TYPE_BTCOEX_PS_BLACKLIST = BIT3,
    MAC_AP_TYPE_BTCOEX_DISABLE_CTS = BIT4,
    MAC_AP_TYPE_TPLINK = BIT5,
    MAC_AP_TYPE_M2S = BIT6,
    MAC_AP_TYPE_ROAM = BIT7,
    MAC_AP_TYPE_160M_OP_MODE = BIT8,
    MAC_AP_TYPE_AGGR_BLACKLIST = BIT9,
    MAC_AP_TYPE_MIMO_BLACKLIST = BIT10,
    MAC_AP_TYPE_BTCOEX_SCO_RETRY_MIDDLE_PRIORITY = BIT11,
    MAC_AP_TYPE_BTCOEX_SCO_MIDDLE_PRIORITY = BIT12,
    MAC_AP_TYPE_BTCOEX_20DBM_BLACKLIST = BIT13,
    MAC_AP_TYPE_MCM_DEGRADARION_WRITELIST = BIT14,
    MAC_AP_TYPE_KEEPALIVE_COMPATIBLE = BIT15,

    MAC_AP_TYPE_BUTT
} mac_ap_type_enum;
typedef uint16_t mac_ap_type_enum_uint16;

/* Action Frames: Category字段枚举 */
typedef enum {
    MAC_ACTION_CATEGORY_SPECMGMT = 0,
    MAC_ACTION_CATEGORY_QOS = 1,
    MAC_ACTION_CATEGORY_DLS = 2,
    MAC_ACTION_CATEGORY_BA = 3,
    MAC_ACTION_CATEGORY_PUBLIC = 4,
    MAC_ACTION_CATEGORY_RADIO_MEASURMENT = 5,
    MAC_ACTION_CATEGORY_FAST_BSS_TRANSITION = 6,
    MAC_ACTION_CATEGORY_HT = 7,
    MAC_ACTION_CATEGORY_SA_QUERY = 8,
    MAC_ACTION_CATEGORY_PROTECTED_DUAL_OF_ACTION = 9,
    MAC_ACTION_CATEGORY_WNM = 10,
    MAC_ACTION_CATEGORY_MESH = 13,
    MAC_ACTION_CATEGORY_MULTIHOP = 14,
#ifdef _PRE_WLAN_FEATURE_WMMAC
    MAC_ACTION_CATEGORY_WMMAC_QOS = 17,
#endif
    MAC_ACTION_CATEGORY_VHT = 21,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_ACTION_CATEGORY_S1G = 22,
#endif
    MAC_ACTION_CATEGORY_HE = 30,
    MAC_ACTION_CATEGORY_VENDOR_SPECIFIC_PROTECTED = 126,
    MAC_ACTION_CATEGORY_VENDOR = 127,
} mac_action_category_enum;
typedef uint8_t mac_category_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_11AX
/* S1G下的Action值的枚举 */
typedef enum {
    MAC_S1G_ACTION_AID_SWITCH_REQ = 0,
    MAC_S1G_ACTION_AID_SWITCH_RESP = 1,
    MAC_S1G_ACTION_SYNC_CONTROL = 2,
    MAC_S1G_ACTION_STA_INFORMATION_ANNOUNCEMENT = 3,
    MAC_S1G_ACTION_EDCA_PARAMETER_SET = 4,
    MAC_S1G_ACTION_EL_OPERATION = 5,
    MAC_S1G_ACTION_TWT_SETUP = 6,
    MAC_S1G_ACTION_TWT_TEARDOWN = 7,
    MAC_S1G_ACTION_SECTORIZED_GROUP_ID_LIST = 8,
    MAC_S1G_ACTION_SECTOR_ID_FEEDBACK = 9,
    MAC_S1G_ACTION_RESERVE = 10,
    MAC_S1G_ACTION_TWT_INFORMATION = 11,
    MAC_S1G_ACTION_BUTT
} mac_s1g_action_type_enum;
typedef uint8_t mac_s1g_action_type_enum_uint8;
#endif

/* HT Category下的Action值的枚举 */
typedef enum {
    MAC_HT_ACTION_NOTIFY_CHANNEL_WIDTH = 0,
    MAC_HT_ACTION_SMPS = 1,
    MAC_HT_ACTION_PSMP = 2,
    MAC_HT_ACTION_SET_PCO_PHASE = 3,
    MAC_HT_ACTION_CSI = 4,
    MAC_HT_ACTION_NON_COMPRESSED_BEAMFORMING = 5,
    MAC_HT_ACTION_COMPRESSED_BEAMFORMING = 6,
    MAC_HT_ACTION_ASEL_INDICES_FEEDBACK = 7,

    MAC_HT_ACTION_BUTT
} mac_ht_action_type_enum;
typedef uint8_t mac_ht_action_type_enum_uint8;

/* SA QUERY Category下的Action值的枚举 */
typedef enum {
    MAC_SA_QUERY_ACTION_REQUEST = 0,
    MAC_SA_QUERY_ACTION_RESPONSE = 1
} mac_sa_query_action_type_enum;
typedef uint8_t mac_sa_query_action_type_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
/* 组播聚合场景模式,接收端反馈私有BA帧类型 */
typedef enum {
    PRIV_BA_ACTION_SUBTYPE_NACK = 0, /* 私有NACK */
    PRIV_BA_ACTION_SUBTYPE_PACK = 1, /* 私有PACK */

    PRIV_BA_ACTION_SUBTYPE_BUTT
} priv_ba_action_subtype_enum;

/* PACK Category下的Action值的枚举 */
typedef enum {
    PRIV_PACK_ACTION_SUBTYPE_NORMAL = 0, /* 默认PACK帧格式 */

    PRIV_PACK_ACTION_SUBTYPE_BUTT
} priv_pack_aciton_subtype_enum;
#endif

/* VHT Category下的Action值的枚举 */
typedef enum {
    MAC_VHT_ACTION_COMPRESSED_BEAMFORMING = 0,
    MAC_VHT_ACTION_GROUPID_MANAGEMENT = 1,
    MAC_VHT_ACTION_OPREATING_MODE_NOTIFICATION = 2,

    MAC_VHT_ACTION_BUTT
} mac_vht_action_type_enum;
typedef uint8_t mac_vht_action_type_enum_uint8;

/* HE Category下的Action值的枚举 */
typedef enum {
    MAC_HE_ACTION_COMPRESSED_BEAMFORMING_AND_CQI = 0,
    MAC_HE_ACTION_QUICK_TIME_PERIOD = 1,
    MAC_HE_ACTION_OPS = 2,

    MAC_HE_ACTION_BUTT
} mac_he_action_type_enum;
typedef uint8_t mac_he_action_type_enum_uint8;

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* WNM Category下的Action值的枚举 */
typedef enum {
    MAC_WNM_ACTION_EVENT_REQUEST = 0,
    MAC_WNM_ACTION_EVENT_REPORT = 1,
    MAC_WNM_ACTION_DIALGNOSTIC_REQUEST = 2,
    MAC_WNM_ACTION_DIALGNOSTIC_REPORT = 3,
    MAC_WNM_ACTION_LOCATION_CONF_REQUEST = 4,
    MAC_WNM_ACTION_LOCATION_CONF_RESPONSE = 5,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_QUERY = 6,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_REQUEST = 7,
    MAC_WNM_ACTION_BSS_TRANSITION_MGMT_RESPONSE = 8,
    MAC_WNM_ACTION_FMS_REQUEST = 9,
    MAC_WNM_ACTION_FMS_RESPONSE = 10,
    MAC_WNM_ACTION_COLLOCATED_INTER_REQUEST = 11,
    MAC_WNM_ACTION_COLLOCATEC_INTER_REPORT = 12,
    MAC_WNM_ACTION_TFS_REQUEST = 13,
    MAC_WNM_ACTION_TFS_RESPONSE = 14,
    MAC_WNM_ACTION_TFS_NOTIFY = 15,
    MAC_WNM_ACTION_SLEEP_MODE_REQUEST = 16,
    MAC_WNM_ACTION_SLEEP_MODE_RESPONSE = 17,
    MAC_WNM_ACTION_TIM_BROADCAST_REQUEST = 18,
    MAC_WNM_ACTION_TIM_BROADCAST_RESPONSE = 19,
    MAC_WNM_ACTION_QOS_TRAFFIC_CAP_UPDATE = 20,
    MAC_WNM_ACTION_CHANNEL_USAGE_REQUEST = 21,
    MAC_WNM_ACTION_CHANNEL_USAGE_RESPONSE = 22,
    MAC_WNM_ACTION_DMS_REQUEST = 23,
    MAC_WNM_ACTION_DMS_RESPONSE = 24,
    MAC_WNM_ACTION_TIMING_MEAS_REQUEST = 25,
    MAC_WNM_ACTION_NOTIFICATION_REQUEST = 26,
    MAC_WNM_ACTION_NOTIFICATION_RESPONSE = 27,

    MAC_WNM_ACTION_BUTT
} mac_wnm_action_type_enum;
typedef uint8_t mac_wnm_action_type_enum_uint8;
#endif  // _PRE_WLAN_FEATURE_11V_ENABLE

/* BlockAck Category下的Action值的枚举 */
typedef enum {
    MAC_BA_ACTION_ADDBA_REQ = 0,
    MAC_BA_ACTION_ADDBA_RSP = 1,
    MAC_BA_ACTION_DELBA = 2,

    MAC_BA_ACTION_BUTT
} mac_ba_action_type_enum;
typedef uint8_t mac_ba_action_type_enum_uint8;

/* Public Category下的Action枚举值 */
typedef enum {
    MAC_PUB_COEXT_MGMT = 0,            /* 20/40 BSS Coexistence Management */
    MAC_PUB_EX_CH_SWITCH_ANNOUNCE = 4, /* Extended Channel Switch Announcement */
    MAC_PUB_VENDOR_SPECIFIC = 9,
    MAC_PUB_GAS_INIT_REQ = 10,
    MAC_PUB_GAS_INIT_RESP = 11, /* public Action: GAS Initial Response(0x0b) */
    MAC_PUB_GAS_COMBAK_REQ = 12,
    MAC_PUB_GAS_COMBAK_RESP = 13, /* public Action: GAS Comeback Response(0x0d) */
    MAC_PUB_FTM_REQ = 32,
    MAC_PUB_FTM = 33,
    MAC_PUB_FILS_DISCOVERY = 34,
} mac_public_action_type_enum;
typedef uint8_t mac_public_action_type_enum_uint8;
#ifdef _PRE_WLAN_FEATURE_WMMAC
/* WMMAC中TSPEC相关ACTION的枚举值 */
typedef enum {
    MAC_WMMAC_ACTION_ADDTS_REQ = 0,
    MAC_WMMAC_ACTION_ADDTS_RSP = 1,
    MAC_WMMAC_ACTION_DELTS = 2,

    MAC_WMMAC_ACTION_BUTT
} mac_wmmac_action_type_enum;

/* ADDTS REQ中TSPEC Direction元素的枚举值 */
typedef enum {
    MAC_WMMAC_DIRECTION_UPLINK = 0,
    MAC_WMMAC_DIRECTION_DOWNLINK = 1,
    MAC_WMMAC_DIRECTION_RESERVED = 2,
    MAC_WMMAC_DIRECTION_BIDIRECTIONAL = 3,

    MAC_WMMAC_DIRECTION_BUTT
} mac_wmmac_direction_enum;
typedef uint8_t mac_wmmac_direction_enum_uint8;
/* TS建立的状态枚举 */
typedef enum {
    MAC_TS_NONE = 0,   /* TS不需要建立 */
    MAC_TS_INIT,       /* TS需要建立，未建立 */
    MAC_TS_INPROGRESS, /* TS建立过程中 */
    MAC_TS_SUCCESS,    /* TS建立成功 */

    MAC_TS_BUTT
} mac_ts_conn_status_enum;
typedef uint8_t mac_ts_conn_status_enum_uint8;
#endif

/* Block ack的确认类型 */
typedef enum {
    MAC_BACK_BASIC = 0,
    MAC_BACK_COMPRESSED = 2,
    MAC_BACK_MULTI_TID = 3,

    MAC_BACK_BUTT
} mac_back_variant_enum;
typedef uint8_t mac_back_variant_enum_uint8;

/* ACTION帧中，各个域的偏移量 */
typedef enum {
    MAC_ACTION_OFFSET_CATEGORY = 0,
    MAC_ACTION_OFFSET_ACTION = 1,
} mac_action_offset_enum;
typedef uint8_t mac_action_offset_enum_uint8;

/* Reason Codes for Deauthentication and Disassociation Frames */
typedef enum {
    MAC_UNSPEC_REASON = 1,
    MAC_AUTH_NOT_VALID = 2,
    MAC_DEAUTH_LV_SS = 3,
    MAC_INACTIVITY = 4,
    MAC_AP_OVERLOAD = 5,
    MAC_NOT_AUTHED = 6,
    MAC_NOT_ASSOCED = 7,
    MAC_DISAS_LV_SS = 8,
    MAC_ASOC_NOT_AUTH = 9,
    MAC_INVLD_ELEMENT = 13,
    MAC_MIC_FAIL = 14,
    MAC_4WAY_HANDSHAKE_TIMEOUT = 15,
    MAC_IEEE_802_1X_AUTH_FAIL = 23,
    MAC_UNSPEC_QOS_REASON = 32,
    MAC_QAP_INSUFF_BANDWIDTH = 33,
    MAC_POOR_CHANNEL = 34,
    MAC_STA_TX_AFTER_TXOP = 35,
    MAC_QSTA_LEAVING_NETWORK = 36,
    MAC_QSTA_INVALID_MECHANISM = 37,
    MAC_QSTA_SETUP_NOT_DONE = 38,
    MAC_QSTA_TIMEOUT = 39,
    MAC_QSTA_CIPHER_NOT_SUPP = 45
} mac_reason_code_enum;
typedef uint16_t mac_reason_code_enum_uint16;

/* Status Codes for Authentication and Association Frames */
typedef enum {
    MAC_SUCCESSFUL_STATUSCODE = 0,
    MAC_UNSPEC_FAIL = 1,
    MAC_UNSUP_CAP = 10,
    MAC_REASOC_NO_ASOC = 11,
    MAC_FAIL_OTHER = 12,
    MAC_UNSUPT_ALG = 13,
    MAC_AUTH_SEQ_FAIL = 14,
    MAC_CHLNG_FAIL = 15,
    MAC_AUTH_TIMEOUT = 16,
    MAC_AP_FULL = 17,
    MAC_UNSUP_RATE = 18,
    MAC_SHORT_PREAMBLE_UNSUP = 19,
    MAC_PBCC_UNSUP = 20,
    MAC_CHANNEL_AGIL_UNSUP = 21,
    MAC_MISMATCH_SPEC_MGMT = 22,
    MAC_MISMATCH_POW_CAP = 23,
    MAC_MISMATCH_SUPP_CHNL = 24,
    MAC_SHORT_SLOT_UNSUP = 25,
    MAC_OFDM_DSSS_UNSUP = 26,
    MAC_MISMATCH_HTCAP = 27,
    MAC_R0KH_UNREACHABLE = 28,
    MAC_MISMATCH_PCO = 29,
    MAC_REJECT_TEMP = 30,
    MAC_MFP_VIOLATION = 31,
    MAC_UNSPEC_QOS_FAIL = 32,
    MAC_QAP_INSUFF_BANDWIDTH_FAIL = 33,
    MAC_POOR_CHANNEL_FAIL = 34,
    MAC_REMOTE_STA_NOT_QOS = 35,
    MAC_REQ_DECLINED = 37,
    MAC_INVALID_REQ_PARAMS = 38,
    MAC_RETRY_NEW_TSPEC = 39,
    MAC_INVALID_INFO_ELMNT = 40,
    MAC_INVALID_GRP_CIPHER = 41,
    MAC_INVALID_PW_CIPHER = 42,
    MAC_INVALID_AKMP_CIPHER = 43,
    MAC_UNSUP_RSN_INFO_VER = 44,
    MAC_INVALID_RSN_INFO_CAP = 45,
    MAC_CIPHER_REJ = 46,
    MAC_RETRY_TS_LATER = 47,
    MAC_DLS_NOT_SUPP = 48,
    MAC_DST_STA_NOT_IN_QBSS = 49,
    MAC_DST_STA_NOT_QSTA = 50,
    MAC_LARGE_LISTEN_INT = 51,
    MAC_ANTI_CLOGGING = 76,
    MAC_FINITE_CYCLIC_GROUP_NOT_SUPPORTED = 77,
    MAC_MISMATCH_VHTCAP = 104,
    MAC_MISMATCH_HECAP = 124,
    MAC_SAE_HASH_TO_ELEMENT = 126,

    /* 私有的定义 */
    MAC_JOIN_RSP_TIMEOUT = 5200,  /* 状态机没从join跳到auth超时 */
    MAC_AUTH_RSP2_TIMEOUT = 5201, /* auth seq2没收到auth rsp */
    MAC_AUTH_RSP4_TIMEOUT = 5202, /* auth seq4没收到auth rsp */
    MAC_ASOC_RSP_TIMEOUT = 5203,  /* asoc req发出去没收到asoc rsp */

    MAC_AUTH_REQ_SEND_FAIL_BEGIN = 5250,
    MAC_AUTH_REQ_SEND_FAIL_NO_ACK = 5251,  /* auth req发出去没收到ack */
    MAC_AUTH_REQ_SEND_FAIL_TIMEOUT = 5252, /* auth req发出去lifetime超时（没法送出去） */
    MAC_AUTH_REQ_SEND_FAIL_ABORT = 5261,   /* auth req发送失败(因为蓝牙abort) */

    MAC_ASOC_REQ_SEND_FAIL_BEGIN = 5300,
    MAC_ASOC_REQ_SEND_FAIL_NO_ACK = 5301,  /* asoc req发出去没收到ack */
    MAC_ASOC_REQ_SEND_FAIL_TIMEOUT = 5302, /* asoc req发出去lifetime超时（没法送出去） */
    MAC_ASOC_REQ_SEND_FAIL_ABORT = 5311,   /* asoc req发送失败(因为蓝牙abort) */

    MAC_AP_AUTH_RSP_TIMEOUT = 6100, /* softap auth rsp发送完wait asoc req超时 */
#ifdef _PRE_WLAN_CHBA_MGMT
    /* CHBA: 添加私有code */
    MAC_CHBA_INIT_CODE = 6200,
    MAC_CHBA_INVAILD_CONNECT_CMD = 6201,
    MAC_CHBA_REPEAT_CONNECT_CMD = 6202, /* 重复建链命令，且两条建链命令参数不匹配 */
    MAC_CHBA_CREATE_NEW_USER_FAIL = 6203, /* 创建新用户失败，一般是因为超过芯片规格 */
    MAC_CHBA_UNSUP_ASSOC_CHANNEL = 6204, /* 无法在下发的建链信道上完成建链 */
    MAC_CHBA_COEX_FAIL = 6205, /* 不满足vap共存条件 */
    MAC_CHBA_UNSUP_PARALLEL_CONNECT = 6206, /* 暂不支持并行建链 */
#endif
} mac_status_code_enum;
typedef uint16_t mac_status_code_enum_uint16;

/* BA会话管理确认策略 */
typedef enum {
    MAC_BA_POLICY_DELAYED = 0,
    MAC_BA_POLICY_IMMEDIATE,

    MAC_BA_POLICY_BUTT
} mac_ba_policy_enum;
typedef uint8_t mac_ba_policy_enum_uint8;

/* 发起DELBA帧的端点的枚举 */
typedef enum {
    MAC_RECIPIENT_DELBA = 0, /* 数据的接收端 */
    MAC_ORIGINATOR_DELBA,    /* 数据的发起端 */

    MAC_BUTT_DELBA
} mac_delba_initiator_enum;
typedef uint8_t mac_delba_initiator_enum_uint8;

/* 发起DELBA帧的业务类型的枚举 */
typedef enum {
    MAC_DELBA_TRIGGER_COMM = 0, /* 配置命令触发 */
    MAC_DELBA_TRIGGER_BTCOEX,   /* BT业务触发 */
    MAC_DELBA_TRIGGER_PS,       /* PS业务触发 */

    MAC_DELBA_TRIGGER_BUTT
} mac_delba_trigger_enum;
typedef uint8_t mac_delba_trigger_enum_uint8;

/*****************************************************************************
  信息元素(Infomation Element)的Element ID
  协议521页，Table 8-54—Element IDs
*****************************************************************************/
typedef enum {
    MAC_EID_SSID = 0,
    MAC_EID_RATES = 1,
    MAC_EID_FHPARMS = 2,
    MAC_EID_DSPARMS = 3,
    MAC_EID_CFPARMS = 4,
    MAC_EID_TIM = 5,
    MAC_EID_IBSSPARMS = 6,
    MAC_EID_COUNTRY = 7,
    MAC_EID_REQINFO = 10,
    MAC_EID_QBSS_LOAD = 11,
    MAC_EID_TSPEC = 13,
    MAC_EID_TCLAS = 14,
    MAC_EID_CHALLENGE = 16,
    /* 17-31 reserved */
    MAC_EID_PWRCNSTR = 32,
    MAC_EID_PWRCAP = 33,
    MAC_EID_TPCREQ = 34,
    MAC_EID_TPCREP = 35,
    MAC_EID_SUPPCHAN = 36,
    MAC_EID_CHANSWITCHANN = 37, /* Channel Switch Announcement IE */
    MAC_EID_MEASREQ = 38,
    MAC_EID_MEASREP = 39,
    MAC_EID_QUIET = 40,
    MAC_EID_IBSSDFS = 41,
    MAC_EID_ERP = 42,
    MAC_EID_TCLAS_PROCESS = 44,
    MAC_EID_HT_CAP = 45,
    MAC_EID_QOS_CAP = 46,
    MAC_EID_RESERVED_47 = 47,
    MAC_EID_RSN = 48,
    MAC_EID_RESERVED_49 = 49,
    MAC_EID_XRATES = 50,
    MAC_EID_AP_CHAN_REPORT = 51,
    MAC_EID_NEIGHBOR_REPORT = 52,
    MAC_EID_MOBILITY_DOMAIN = 54,
    MAC_EID_FT = 55,
    MAC_EID_TIMEOUT_INTERVAL = 56,
    MAC_EID_RDE = 57,
    MAC_EID_OPERATING_CLASS = 59,  /* Supported Operating Classes */
    MAC_EID_EXTCHANSWITCHANN = 60, /* Extended Channel Switch Announcement IE */
    MAC_EID_HT_OPERATION = 61,
    MAC_EID_SEC_CH_OFFSET = 62, /* Secondary Channel Offset IE */
    MAC_EID_WAPI = 68,          /* IE for WAPI */
    MAC_EID_TIME_ADVERTISEMENT = 69,
    MAC_EID_RRM = 70,                /* Radio resource measurement */
    MAC_EID_MULTI_BSSID = 71,        /* Multiple BSSID Element   */
    MAC_EID_2040_COEXT = 72,         /* 20/40 BSS Coexistence IE */
    MAC_EID_2040_INTOLCHREPORT = 73, /* 20/40 BSS Intolerant Channel Report IE */
    MAC_EID_OBSS_SCAN = 74,          /* Overlapping BSS Scan Parameters IE */
    MAC_EID_MMIE = 76,               /* 802.11w Management MIC IE */
    MAC_EID_NONTRANSMITTED_BSSID_CAP = 83,
    MAC_EID_MULTI_BSSID_INDEX = 85,  /*  m-bssid 相关 */
    MAC_EID_FMS_DESCRIPTOR = 86,     /* 802.11v FMS descriptor IE */
    MAC_EID_FMS_REQUEST = 87,        /* 802.11v FMS request IE */
    MAC_EID_FMS_RESPONSE = 88,       /* 802.11v FMS response IE */
    MAC_EID_BSSMAX_IDLE_PERIOD = 90, /* BSS MAX IDLE PERIOD */
    MAC_EID_TFS_REQUEST = 91,
    MAC_EID_TFS_RESPONSE = 92,
    MAC_EID_TIM_BCAST_REQUEST = 94,
    MAC_EID_TIM_BCAST_RESPONSE = 95,
    MAC_EID_INTERWORKING = 107,
    MAC_EID_ADVERTISEMENT_PROTOCOL = 108,
    MAC_EID_EXT_CAPS = 127, /* Extended Capabilities IE */
    MAC_EID_VHT_TYPE = 129,
    MAC_EID_11NTXBF = 130, /* 802.11n txbf */ /* 废弃 */
    MAC_EID_RESERVED_133 = 133,
    MAC_EID_TPC = 150,
    MAC_EID_CCKM = 156,
    MAC_EID_VHT_CAP = 191,
    MAC_EID_VHT_OPERN = 192,         /* VHT Operation IE */
    MAC_EID_WIDE_BW_CH_SWITCH = 194, /* Wide Bandwidth Channel Switch IE */
    MAC_EID_OPMODE_NOTIFY = 199,     /* Operating Mode Notification IE */
    MAC_EID_FTMP = 206,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_TWT = 216,
#endif
    MAC_EID_FTMSI = 255,
    MAC_EID_VENDOR = 221, /* vendor private */
    MAC_EID_WMM = 221,
    MAC_EID_WPA = 221,
    MAC_EID_WPS = 221,
    MAC_EID_P2P = 221,
    MAC_EID_WFA_TPC_RPT = 221,
    MAC_EID_ADJUST_POW_PRIVATE = 221, /* private pow adjust vendor */
    MAC_EID_FRAGMENT = 242,
    MAC_EID_RSNX = 244,
    MAC_EID_EXTENSION = 255,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_HE = 255,
#endif
} mac_eid_enum;
typedef uint8_t mac_eid_enum_uint8;

/* Element ID Extension (EID 255) values */
typedef enum {
    MAC_EID_EXT_FTMSI = 9,
    MAC_EID_EXT_OWE_DH_PARAM = 32,
#ifdef _PRE_WLAN_FEATURE_11AX
    MAC_EID_EXT_HE_CAP = 35,
    MAC_EID_EXT_HE_OPERATION = 36,
    MAC_EID_EXT_UORA_PARAMETER_SET = 37,
    MAC_EID_EXT_HE_EDCA = 38,
    MAC_EID_EXT_HE_SRP = 39,
    MAC_EID_EXT_HE_NDP_FEEDBACK_REPORT = 41,
    MAC_EID_EXT_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT = 42,
    MAC_EID_EXT_HE_6GHZ_BAND_CAP = 59,
#endif

    MAC_EID_EXT_BUT
} mac_eid_extension_enum;
typedef uint8_t mac_eid_extension_enum_uint8;

typedef enum {
    MAC_SMPS_STATIC_MODE = 0,  /*   静态SMPS   */
    MAC_SMPS_DYNAMIC_MODE = 1, /*   动态SMPS   */
    MAC_SMPS_MIMO_MODE = 3,    /* disable SMPS */

    MAC_SMPS_MODE_BUTT
} mac_mimo_power_save_enum;
typedef uint8_t mac_mimo_power_save_mode_enum_uint8;

typedef enum {
    MAC_SCN = 0, /* 不存在次信道 */
    MAC_SCA = 1, /* 次信道在主信道之上(Secondary Channel Above) */
    MAC_SCB = 3, /* 次信道在主信道之下(Secondary Channel Below) */

    MAC_SEC_CH_BUTT,
} mac_sec_ch_off_enum;
typedef uint8_t mac_sec_ch_off_enum_uint8;

typedef enum {
    P2P_PUB_ACT_OUI_OFF1 = 2,
    P2P_PUB_ACT_OUI_OFF2 = 3,
    P2P_PUB_ACT_OUI_OFF3 = 4,
    P2P_PUB_ACT_OUI_TYPE_OFF = 5,
    P2P_PUB_ACT_OUI_SUBTYPE_OFF = 6,
    P2P_PUB_ACT_DIALOG_TOKEN_OFF = 7,
    P2P_PUB_ACT_TAG_PARAM_OFF = 8
} p2p_pub_act_frm_off;

/* Radio Measurement下的Action枚举值 */
typedef enum {
    MAC_RM_ACTION_RADIO_MEASUREMENT_REQUEST = 0,
    MAC_RM_ACTION_RADIO_MEASUREMENT_REPORT = 1,
    MAC_RM_ACTION_LINK_MEASUREMENT_REQUEST = 2,
    MAC_RM_ACTION_LINK_MEASUREMENT_REPORT = 3,
    MAC_RM_ACTION_NEIGHBOR_REPORT_REQUEST = 4,
    MAC_RM_ACTION_NEIGHBOR_REPORT_RESPONSE = 5
} mac_rm_action_type_enum;
typedef uint8_t mac_rm_action_type_enum_uint8;

/* 度量类型的枚举 */
/* Basic 0
Clear Channel Assessment (CCA) 1
Receive Power Indication (RPI) Histogram 2
Channel Load 3
Noise Histogram 4
Beacon 5
Frame 6
STA Statistics 7
LCI 8
Transmit Stream/Category Measurement 9
Multicast Diagnostics 10
Location Civic 11
Location Identifier 12
Directional Channel Quality 13
Directional Measurement 14
Directional Statistics 15
Fine Timing Measurement Range 16
Reserved 17–254
Measurement Pause 255 */
typedef enum {
    RM_RADIO_MEASUREMENT_BASIC = 0,
    RM_RADIO_MEAS_CHANNEL_LOAD = 3,
    RM_RADIO_MEAS_BCN = 5,
    RM_RADIO_MEAS_FRM = 6,
    RM_RADIO_MEAS_STA_STATS = 7,
    RM_RADIO_MEASUREMENT_LCI = 8,
    RM_RADIO_MEAS_TSC = 9,
    RM_RADIO_MEASUREMENT_LOCATION_CIVIC = 11,
    RM_RADIO_MEASUREMENT_FTM_RANGE = 16,
    RM_RADIO_MEAS_PAUSE = 255
} mac_radio_meas_type_enum;
typedef uint8_t mac_radio_meas_type_enum_uint8;

typedef enum {
    RM_BCN_REQ_MEAS_MODE_PASSIVE = 0,
    RM_BCN_REQ_MEAS_MODE_ACTIVE = 1,
    RM_BCN_REQ_MEAS_MODE_TABLE = 2,
    RM_BCN_REQ_MEAS_MODE_BUTT
} rm_bcn_req_meas_mode_enum;
typedef uint8_t rm_bcn_req_meas_mode_enum_uint8;

#ifdef _PRE_WLAN_FEATURE_TWT
/* TWT命令类型 */
typedef enum {
    MAC_TWT_COMMAND_REQUEST = 0,
    MAC_TWT_COMMAND_SUGGEST = 1,
    MAC_TWT_COMMAND_DEMAND = 2,
    MAC_TWT_COMMAND_GROUPING = 3,
    MAC_TWT_COMMAND_ACCEPT = 4,
    MAC_TWT_COMMAND_ALTERNATE = 5,
    MAC_TWT_COMMAND_DICTATE = 6,
    MAC_TWT_COMMAND_REJECT = 7,
} mac_twt_command_enum;
typedef uint8_t mac_twt_command_enum_uint8;

typedef enum {
    MAC_TWT_NEGO_ZERO = 0,
    MAC_TWT_NEGO_INDIVIDUAL = 1,
    MAC_TWT_NEGO_BROADCAST = 2,
    MAC_TWT_NEGO_TBTT = 3,
} mac_twt_nego_enum;
typedef uint8_t mac_twt_nego_enum_uint8;
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
typedef enum {
    MBO_ATTR_ID_AP_CAPA_IND = 1,
    MBO_ATTR_ID_NON_PREF_CHAN_REPORT = 2,
    MBO_ATTR_ID_CELL_DATA_CAPA = 3,
    MBO_ATTR_ID_ASSOC_DISALLOW = 4,
    MBO_ATTR_ID_CELL_DATA_PREF = 5,
    MBO_ATTR_ID_TRANSITION_REASON = 6,
    MBO_ATTR_ID_TRANSITION_REJECT_REASON = 7,
    MBO_ATTR_ID_ASSOC_RETRY_DELAY = 8,
} mac_mbo_attr_id_enum;
typedef uint8_t mac_mbo_attr_id_enum_uint8;
#endif

typedef enum {
    MAC_SAP_OFF = 0,
    MAC_SAP_MASTER = 1,
    MAC_SAP_SLAVE = 2,
} mac_sap_mode_enum;
typedef uint8_t mac_sap_mode_enum_uint8;

#define MAC_WLAN_OUI_WFA                0x506f9a
#define MAC_WLAN_OUI_TYPE_WFA_P2P       9
#define MAC_WLAN_OUI_MICROSOFT          0x0050f2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WMM 2
#define MAC_WLAN_OUI_TYPE_MICROSOFT_WPS 4
#define MAC_WLAN_OUI_VENDOR_VHT_HEADER   5
#define MAC_WLAN_OUI_BROADCOM_EPIGRAM    0x00904c
#define MAC_WLAN_OUI_VENDOR_VHT_TYPE     0x04

/* eapol key 结构宏定义 */
#define WPA_REPLAY_COUNTER_LEN    8
#define WPA_NONCE_LEN             32
#define WPA_KEY_RSC_LEN           8
#define IEEE802_1X_TYPE_EAPOL_KEY 3
#define WPA_KEY_INFO_KEY_TYPE     BIT(3) /* 1 = Pairwise, 0 = Group key */
#define WPA_KEY_INFO_KEY_ACK BIT(7)
#define WPA_KEY_INFO_KEY_MIC BIT(0)

/* EAPOL数据帧子类型枚举定义 */
typedef enum {
    MAC_EAPOL_PTK_1_4 = 1,
    MAC_EAPOL_PTK_2_4,
    MAC_EAPOL_PTK_3_4,
    MAC_EAPOL_PTK_4_4,

    MAC_EAPOL_PTK_BUTT
} mac_eapol_type_enum_uint8;
#endif
