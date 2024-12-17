/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应的帧的结构及操作接口定义的源文件(HAL模块不可以调用)
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef MAC_FRAME_H
#define MAC_FRAME_H

/* 1 其他头文件包含 */
#include "mac_frame_common.h"
#include "wlan_oneimage_define.h"
#include "mac_device.h"

/* 此文件中定义的结构体与协议相关，需要1字节对齐 */
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_H

/* MAC IE剩余长度 */
#define MAC_IE_REMAIN_LEN_2   2
#define MAC_IE_REMAIN_LEN_4   4
#define INDEX_OFFSET_2   2
#define INDEX_OFFSET_4   4
#define MAC_WLAN_OUI_TYPE_HAUWEI_COWORK  0x20 /* 端管联调Adaptive 11r */

/* A-MSDU情况下，submsdu的偏移宏 */
#define MAC_SUBMSDU_HEADER_LEN    14 /* |da = 6|sa = 6|len = 2| submsdu的头的长度 */
#define MAC_SUBMSDU_LENGTH_OFFSET 12 /* submsdu的长度字段的偏移值 */
#define MAC_SUBMSDU_DA_OFFSET     0  /* submsdu的目的地址的偏移值 */
#define MAC_SUBMSDU_SA_OFFSET     6  /* submsdu的源地址的偏移值 */
#define MAC_80211_QOS_4ADDR_FRAME_LEN     32
#define MAC_WMM_INFO_LEN  7  /* WMM info ie */
#define MAC_80211_4ADDR_FRAME_LEN         30
#define MAC_80211_REASON_CODE_LEN         2
#define MAC_SSID_OFFSET            12
#define MAC_LISTEN_INT_LEN         2
#define MAC_MIN_XRATE_LEN          1
#define MAC_MIN_RATES_LEN          1
#define MAC_MIN_XCAPS_LEN          1
#define MAC_LIS_INTERVAL_IE_LEN               2 /* listen interval信息元素长度 */
#define MAC_AID_LEN                           2
#define MAC_PWR_CAP_LEN                       2
#define MAC_AUTH_TRANS_SEQ_NUM_LEN            2 /* transaction seq num信息元素长度 */
#define MAC_STATUS_CODE_LEN                   2
#define MAC_TIMEOUT_INTERVAL_INFO_LEN         5
#define MAC_VHT_CAP_RX_MCS_MAP_FIELD_LEN      2 /* vht cap ie rx_mcs_map field length */
#define MAC_VHT_CAP_RX_HIGHEST_DATA_FIELD_LEN 2 /* vht cap ie rx_highest_data field length */
#define MAC_VHT_CAP_TX_MCS_MAP_FIELD_LEN      2 /* vht cap ie tx_mcs_map field length */
#define MAC_11N_TXBF_CAP_OFFSET               23
#define MAC_RRM_ENABLE_CAP_IE_LEN             5
#define MAC_TXBF_CAP_MIN_GROUPING             3 /* Refer to 80211-2016 Table 9-166 */
#define MAC_P2P_ATTRIBUTE_HDR_LEN   3 /* P2P_ATTRIBUTE信息元素头部 1字节ATTRIBUTE + 2字节长度 */
#define MAC_P2P_LISTEN_CHN_ATTR_LEN 5 /* LISTEN CHANNEL ATTRIBUTE长度 */
#define MAC_P2P_MIN_IE_LEN          4 /* P2P IE的最小长度 */
#define MAC_EXT_OWE_DH_PARAM_LEN    3
#define MAC_ADDBA_REQ_BA_PARAM_OFFSET 3   /* addba req帧中BA参数在action字段的偏移 */
#define MAC_ADDBA_REQ_BA_SEQCTRL_OFFSET 7 /* addba req帧中SEQ控制字段在action字段的偏移 */

#define MAC_ADDBA_REQ_FRAME_BODY_LEN 9   /* addba req帧 帧体长度 */
#define MAC_ADDBA_RSP_FRAME_BODY_LEN 9   /* addba rsp帧 帧体长度 */
#define MAC_DELBA_FRAME_BODY_LEN 6   /* addba del帧 帧体长度 */

#define MAC_ADDTS_RSP_FRAME_BODY_LEN 67   /* addts rsp帧 帧体长度 */
#define MAC_DELTS_FRAME_BODY_LEN 67       /* delts 帧 帧体长度 */

#define MAC_WMM_QOS_INFO_POS            8 /* wmm 字段中qos info位置，偏移8 */

#ifdef _PRE_WLAN_FEATURE_WMMAC
#define MAC_OUISUBTYPE_WMMAC_TSPEC 2 /* WMMAC TSPEC OUI subtype */
#endif
#define MAC_QOS_CTL_LEN           2  /* QOS CONTROL字段的长度 */

#ifdef _PRE_WLAN_FEATURE_11AX
#define MAC_MULTIPLE_BSSID_IE_MIN_LEN    11
#define MAC_HE_NDP_FEEDBACK_REPORT_LEN   2
#define MAC_HE_NDP_FEEDBACK_REPORT_OFFSET 3
#define MAC_HE_CAP_MIN_LEN               22
#define PUNCTURED_PREAMBLE_RX_EN         0xf  /* punctured preamble使能, 非强制 */
#endif

#define MAC_HE_BSS_COLOR_CHANGE_ANNOUNCEMENT_LEN 3
#define MAC_TAG_PARAM_OFFSET (MAC_80211_FRAME_LEN + MAC_TIME_STAMP_LEN + \
                              MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN)

/* chartiot信令包通过tcp端口号无法正确识别,chariot软件重启则端口号改变.识别逻辑无效 */
#define MAC_CHARIOT_NETIF_PORT 10115
#define MAC_WFD_RTSP_PORT      7236

#define MAC_WLAN_OUI_TYPE_HAUWEI_MAX_TX_POWER 0x41 /* sta携带 发射功率 */
#define MAC_WLAN_OUI_TYPE_HAUWEI_TB_FRM_GAIN 0x42 /* ap回应 窄带收益 */

#ifdef _PRE_WLAN_FEATURE_MBO
#define MBO_IE_HEADER              6        /* type + length + oui + oui type */
#define MAC_MBO_OUI_LENGTH         3
#define MAC_MBO_ASSOC_DIS_ATTR_LEN 3
#endif

/* Type4Bytes len4Bytes mac16Bytes mac26Bytes timestamp23Bytes RSSIInfo8Bytes SNR2Bytes */
#define MAC_REPORT_RSSIINFO_LEN     8
#define MAC_REPORT_RSSIINFO_SNR_LEN 10

/* DHCP message types */
#define MAC_DHCP_DISCOVER 1
#define MAC_DHCP_OFFER    2
#define MAC_DHCP_REQUEST  3
#define MAC_DHCP_DECLINE  4
#define MAC_DHCP_ACK      5
#define MAC_DHCP_NAK      6
#define MAC_DHCP_RELEASE  7
#define MAC_DHCP_INFORM   8
#define MAC_DHCP_UNKNOWN  0xFF

static inline oal_bool_enum_uint8 mac_is_golden_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x00) && ((bssid)[MAC_ADDR_1] == 0x13) &&
                                 ((bssid)[MAC_ADDR_2] == 0xE9));
}
static inline oal_bool_enum_uint8 mac_is_belkin_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x14) && ((bssid)[MAC_ADDR_1] == 0x91) &&
                                 ((bssid)[MAC_ADDR_2] == 0x82));
}
static inline oal_bool_enum_uint8 mac_is_netgear_wndr_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8) (((bssid)[MAC_ADDR_0] == 0x08) && ((bssid)[MAC_ADDR_1] == 0xbd) &&
                                  ((bssid)[MAC_ADDR_2] == 0x43));
}
static inline oal_bool_enum_uint8 mac_is_trendnet_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xd8) && ((bssid)[MAC_ADDR_1] == 0xeb) &&
                                 ((bssid)[MAC_ADDR_2] == 0x97));
}
static inline oal_bool_enum_uint8 mac_is_dlink_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xcc) && ((bssid)[MAC_ADDR_1] == 0xb2) &&
                                 ((bssid)[MAC_ADDR_2] == 0x55));
}
static inline oal_bool_enum_uint8 mac_is_linksys_ea8500_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xc0) && ((bssid)[MAC_ADDR_1] == 0x56) &&
                                 ((bssid)[MAC_ADDR_2] == 0x27));
}
static inline oal_bool_enum_uint8 mac_is_360_ap0(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xb0) && ((bssid)[MAC_ADDR_1] == 0xd5) &&
                                 ((bssid)[MAC_ADDR_2] == 0x9d));
}
static inline oal_bool_enum_uint8 mac_is_360_ap1(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xc8) && ((bssid)[MAC_ADDR_1] == 0xd5) &&
                                 ((bssid)[MAC_ADDR_2] == 0xfe));
}
static inline oal_bool_enum_uint8 mac_is_360_ap2(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x70) && ((bssid)[MAC_ADDR_1] == 0xb0) &&
                                 ((bssid)[MAC_ADDR_2] == 0x35));
}
/* TP-LINK 7300识别:AP OUI + 芯片OUI */
static inline oal_bool_enum_uint8 mac_is_tplink_7300(mac_bss_dscr_stru *bss_dscr)
{
    return (oal_bool_enum_uint8)(((bss_dscr)->auc_bssid[MAC_ADDR_0] == 0xd0) &&
        ((bss_dscr)->auc_bssid[MAC_ADDR_1] == 0x76) && ((bss_dscr)->auc_bssid[MAC_ADDR_2] == 0xe7) &&
        ((bss_dscr)->chip_oui == WLAN_AP_CHIP_OUI_RALINK));
}
/* NETGEAR RAX20识别:AP OUI + 芯片OUI */
static inline oal_bool_enum_uint8 mac_is_netgear_rax20(mac_bss_dscr_stru *bss_dscr)
{
    return (oal_bool_enum_uint8)(((bss_dscr)->auc_bssid[MAC_ADDR_0] == 0x28) &&
        ((bss_dscr)->auc_bssid[MAC_ADDR_1] == 0x80) && ((bss_dscr)->auc_bssid[MAC_ADDR_2] == 0x88) &&
        ((bss_dscr)->chip_oui == WLAN_AP_CHIP_OUI_BCM));
}
static inline oal_bool_enum_uint8 mac_is_feixun_k3(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x2c) && ((bssid)[MAC_ADDR_1] == 0xb2) &&
                                 ((bssid)[MAC_ADDR_2] == 0x1a));
}
static inline oal_bool_enum_uint8 mac_is_linksys(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x60) && ((bssid)[MAC_ADDR_1] == 0x38) &&
                                 ((bssid)[MAC_ADDR_2] == 0xe0));
}
static inline oal_bool_enum_uint8 mac_is_haier_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x08) && ((bssid)[MAC_ADDR_1] == 0x10) &&
                                 ((bssid)[MAC_ADDR_2] == 0x79));
}
static inline oal_bool_enum_uint8 mac_is_jcg_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x04) && ((bssid)[MAC_ADDR_1] == 0x5f) &&
                                 ((bssid)[MAC_ADDR_2] == 0xa7));
}
static inline oal_bool_enum_uint8 mac_is_fast_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x44) && ((bssid)[MAC_ADDR_1] == 0x97) &&
                                 ((bssid)[MAC_ADDR_2] == 0x5a));
}
static inline oal_bool_enum_uint8 mac_is_tplink_845_ap(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x88) && ((bssid)[MAC_ADDR_1] == 0x25) &&
                                 ((bssid)[MAC_ADDR_2] == 0x93));
}
static inline oal_bool_enum_uint8 mac_is_wdr2041n(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x88) && ((bssid)[MAC_ADDR_1] == 0x25) &&
                                 ((bssid)[MAC_ADDR_2] == 0x93));
}
static inline oal_bool_enum_uint8 mac_is_netgear_r2000(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xb0) && ((bssid)[MAC_ADDR_1] == 0x7f) &&
                                 ((bssid)[MAC_ADDR_2] == 0xb9));
}
static inline oal_bool_enum_uint8 mac_is_honor_ws851(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xc4) && ((bssid)[MAC_ADDR_1] == 0xf0) &&
                                 ((bssid)[MAC_ADDR_2] == 0x81));
}

static inline oal_bool_enum_uint8 mac_is_sco_retry_blacklist(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)((mac_is_wdr2041n(bssid)) || (mac_is_netgear_r2000(bssid)) ||
        (mac_is_honor_ws851(bssid)));
}
static inline oal_bool_enum_uint8 mac_is_fast_fw450r(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0xd4) && ((bssid)[MAC_ADDR_1] == 0x83) &&
                                 ((bssid)[MAC_ADDR_2] == 0x04));
}
static inline oal_bool_enum_uint8 mac_is_ax3000(uint8_t *bssid)
{
    return (oal_bool_enum_uint8)((((bssid)[MAC_ADDR_0] == 0x50) && ((bssid)[MAC_ADDR_1] == 0xd4) &&
                                  ((bssid)[MAC_ADDR_2] == 0xf7)) ||
                                 (((bssid)[MAC_ADDR_0] == 0x48) && ((bssid)[MAC_ADDR_1] == 0x0e) &&
                                  ((bssid)[MAC_ADDR_2] == 0xec)));
}
static inline oal_bool_enum_uint8 mac_is_huaweite_ap(mac_bss_dscr_stru *bss_dscr)
{
    /* huawei识别:AP OUI + 芯片OUI */
    return (oal_bool_enum_uint8)(((bss_dscr)->auc_bssid[MAC_ADDR_0] == 0xe0) &&
        ((bss_dscr)->auc_bssid[MAC_ADDR_1] == 0xcc) && ((bss_dscr)->auc_bssid[MAC_ADDR_2] == 0x7a) &&
        ((bss_dscr)->chip_oui == WLAN_AP_CHIP_OUI_QLM));
}
static inline oal_bool_enum_uint8 mac_is_smc_ap(uint8_t *bssid)
{
    return  (oal_bool_enum_uint8)(((bssid)[MAC_ADDR_0] == 0x00) && ((bssid)[MAC_ADDR_1] == 0x22) &&
                                  ((bssid)[MAC_ADDR_2] == 0x2d));
}

static inline oal_bool_enum_uint8 mac_is_fiberhome(uint8_t *bssid)
{
    return ((bssid[MAC_ADDR_0] == 0x74) && (bssid[MAC_ADDR_1] == 0xec) && (bssid[MAC_ADDR_2] == 0x42)) ||
        ((bssid[MAC_ADDR_0] == 0x14) && (bssid[MAC_ADDR_1] == 0xe9) && (bssid[MAC_ADDR_2] == 0xb2));
}

#define MAC_WLAN_CHIP_OUI_ATHEROSC       0x00037f
#define MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC  0x1
#define MAC_WLAN_CHIP_OUI_RALINK         0x000c43
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK    0x3
#define MAC_WLAN_CHIP_OUI_TYPE_RALINK_2  0x4
#define MAC_WLAN_CHIP_OUI_BROADCOM       0x001018
#define MAC_WLAN_CHIP_OUI_TYPE_BROADCOM  0x2
#define MAC_WLAN_CHIP_OUI_SHENZHEN       0x000aeb
#define MAC_WLAN_CHIP_OUI_TYPE_SHENZHEN  0x1
#define MAC_WLAN_CHIP_OUI_APPLE1         0x0017f2
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_1 0x1
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_2 0x7
#define MAC_WLAN_CHIP_OUI_APPLE2         0x000393
#define MAC_WLAN_CHIP_OUI_TYPE_APPLE_2_1 0x1
#define MAC_WLAN_CHIP_OUI_MARVELL        0x005043
#define MAC_WLAN_CHIP_OUI_TYPE_MARVELL   0x3
#define MAC_WLAN_CHIP_OUI_REALTEK        0x00e04c
#define MAC_WLAN_CHIP_OUI_TYPE_REALTEK   0x2
#define MAC_WLAN_CHIP_OUI_METALINK       0x000986
#define MAC_WLAN_CHIP_OUI_TYPE_METALINK  0x1
#define MAC_WLAN_CHIP_OUI_QUALCOMM       0x8cfdf0
#define MAC_WLAN_CHIP_OUI_TYPE_QUALCOMM  0x1
#define MAC_WLAN_CHIP_OUI_MTK            0x000ce7
#define MAC_WLAN_CHIP_OUI_TYPE_MTK       0x0

#ifdef _PRE_WLAN_FEATURE_HIEX
#define MAC_HIEX_CAP_CHANGE  (BIT9)
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
#define MAC_AMSDU_SKB_LEN_UP_LIMIT   1544
#endif

#define MAC_WLAN_OUI_TYPE_WFA_WFD       0x0a
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE  0x08
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2 0x17
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE3 0x07
#define MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE4 0x10

#ifdef _PRE_WLAN_FEATURE_PMF
static inline wlan_pmf_cap_status_uint8 mac_is_rsn_enable_pmf(uint16_t rsn_cap_info)
{
    return ((rsn_cap_info & BIT7) ? MAC_PMF_ENABLED : MAC_PMF_DISABLED);
}
static inline wlan_pmf_cap_status_uint8 mac_rsn_cap_to_pmf(uint16_t rsn_cap_info)
{
    return (((rsn_cap_info & BIT6) && (rsn_cap_info & BIT7)) ? \
        MAC_PMF_REQUIRED : (rsn_cap_info & BIT7) ? MAC_PMF_ENABLED : MAC_PMF_DISABLED);
}

/* PMF能力为使能，非强制 */
static inline oal_bool_enum_uint8 mac_is_rsn_pmf_only_enable(uint16_t rsn_cap_info)
{
    return (oal_bool_enum_uint8)((rsn_cap_info & BIT7) && !(rsn_cap_info & BIT6));
}
#endif

#define DEFAULT_AKM_VALUE     0xff
#define TYPE_TLV_CAPABILITY   0
#define TYPE_TLV_DC_ROAM_INFO 7
#define COWORK_IE_LEN         19
#define TYPE_TLV_LEN          2
#define TYPE_TLV_DC_ROAM_LEN  9

typedef enum {
    MAC_HISHELL_ACTION_PASSWORD_CHANGE_REQUEST = 0x65,
    MAC_HISHELL_ACTION_PASSWORD_CHANGE_RESPONSE = 0x66,
} mac_hishell_action_type_enum;

/* Timeout_Interval ie中的类型枚举 */
typedef enum {
    MAC_TIE_REASSOCIATION_DEADLINE_INTERVAL = 1, /* 毫秒级 */
    MAC_TIE_KEY_LIFETIME_INTERVAL = 2,           /* 秒级 */
    MAC_TIE_ASSOCIATION_COMEBACK_TIME = 3,       /* 毫秒级 */

    MAC_TIE_BUTT
} mac_timeout_interval_type_enum;
typedef uint8_t mac_timeout_interval_type_enum_uint8;

/* HMAC模块接收流程处理MSDU状态 */
typedef enum {
    MAC_PROC_ERROR = 0,
    MAC_PROC_LAST_MSDU,
    MAC_PROC_MORE_MSDU,
    MAC_PROC_LAST_MSDU_ODD,

    MAC_PROC_BUTT
} mac_msdu_proc_status_enum;
typedef uint8_t mac_msdu_proc_status_enum_uint8;

typedef enum {
    MAC_FT_ACTION_REQUEST = 1,
    MAC_FT_ACTION_RESPONSE = 2,
    MAC_FT_ACTION_CONFIRM = 3,
    MAC_FT_ACTION_ACK = 4,
    MAC_FT_ACTION_PREAUTH_REQUEST  = 100,
    MAC_FT_ACTION_PREAUTH_RESPONSE = 101,
    MAC_FT_ACTION_BUTT = 200
} mac_ft_action_type_enum;
typedef uint8_t mac_ft_action_type_enum_uint8;

/* Capability Information field bit assignments  */
typedef enum {
    MAC_CAP_ESS = 0x01,             /* ESS capability               */
    MAC_CAP_IBSS = 0x02,            /* IBSS mode                    */
    MAC_CAP_POLLABLE = 0x04,        /* CF Pollable                  */
    MAC_CAP_POLL_REQ = 0x08,        /* Request to be polled         */
    MAC_CAP_PRIVACY = 0x10,         /* WEP encryption supported     */
    MAC_CAP_SHORT_PREAMBLE = 0x20,  /* Short Preamble is supported  */
    MAC_CAP_SHORT_SLOT = 0x400,     /* Short Slot is supported      */
    MAC_CAP_PBCC = 0x40,            /* PBCC                         */
    MAC_CAP_CHANNEL_AGILITY = 0x80, /* Channel Agility              */
    MAC_CAP_SPECTRUM_MGMT = 0x100,  /* Spectrum Management          */
    MAC_CAP_DSSS_OFDM = 0x2000 /* DSSS-OFDM                    */
} mac_capability_enum;

typedef enum {
    MAC_P2P_ATTRIBUTE_CAP = 2,
    MAC_P2P_ATTRIBUTE_GROUP_OI = 4,
    MAC_P2P_ATTRIBUTE_CFG_TIMEOUT = 5,
    MAC_P2P_ATTRIBUTE_LISTEN_CHAN = 6,
    MAC_P2P_ATTRIBUTE_NOA = 12,
} mac_p2p_attribute_enum;

typedef struct {
    uint8_t bandwidth;
    int8_t freq_seg0_offset;
    int8_t freq_seg1_offset;
    uint8_t resv;
} mac_chan_band_offset_stru;

/* 10 函数声明 */
uint8_t *mac_find_p2p_attribute(uint8_t uc_eid, uint8_t *puc_ies, int32_t l_len);
uint8_t *mac_find_ie(uint8_t uc_eid, uint8_t *puc_ies, int32_t l_len);
uint8_t *mac_find_ie_sec(uint8_t uc_eid, uint8_t *puc_ies, int32_t l_len, uint32_t *pul_len);
uint8_t *mac_find_vendor_ie(uint32_t oui, uint8_t uc_oui_type, uint8_t *puc_ies, int32_t l_len);
uint8_t *mac_get_wmm_ie_ram(uint8_t *puc_beacon_body, uint16_t us_frame_len);
#ifdef _PRE_WLAN_FEATURE_SMPS
uint8_t mac_calc_smps_field(uint8_t en_smps);
#endif
uint8_t *mac_get_ssid(uint8_t *puc_beacon_body, int32_t l_frame_body_len, uint8_t *puc_ssid_len);
uint16_t mac_get_beacon_period(uint8_t *puc_beacon_body);
uint8_t mac_get_dtim_period(uint8_t *puc_frame_body, uint16_t us_frame_body_len);
uint8_t mac_get_dtim_cnt(uint8_t *puc_frame_body, uint16_t us_frame_body_len);
uint8_t *mac_get_wmm_ie(uint8_t *puc_beacon_body, uint16_t us_frame_len);
void mac_set_power_cap_ie(uint8_t *pst_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_supported_channel_ie(uint8_t *pst_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_wmm_ie_sta(uint8_t *pst_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_WMMAC
uint16_t mac_set_wmmac_ie_sta(uint8_t *pst_vap, uint8_t *puc_buffer, mac_wmm_tspec_stru *pst_addts_args);
#endif  // _PRE_WLAN_FEATURE_WMMAC
void mac_set_listen_interval_ie(uint8_t *pst_vap, uint8_t *puc_buffer, uint8_t *puc_ie_len);
void mac_set_status_code_ie(uint8_t *puc_buffer, mac_status_code_enum_uint16 en_status_code);
void mac_set_aid_ie(uint8_t *puc_buffer, uint16_t uc_aid);
uint8_t mac_get_bss_type(uint16_t us_cap_info);
uint32_t mac_check_mac_privacy(uint16_t us_cap_info, uint8_t *pst_mac_vap);
oal_bool_enum_uint8 mac_is_wmm_ie(uint8_t *puc_ie);
uint32_t mac_get_hisi_cap_vendor_ie(uint8_t *payload, uint32_t msg_len,
    mac_hisi_cap_vendor_ie_stru *hisi_cap_ie);

uint8_t *mac_find_ie_ext_ie(uint8_t uc_eid, uint8_t uc_ext_ie, uint8_t *puc_ies, int32_t l_len);
uint32_t mac_set_csa_ie(uint8_t uc_mode, uint8_t uc_channel, uint8_t uc_csa_cnt, uint8_t *puc_buffer,
    uint8_t *puc_ie_len);
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
void mac_set_opmode_field(uint8_t *pst_vap, uint8_t *puc_buffer,
    wlan_nss_enum_uint8 en_nss);
void mac_set_opmode_notify_ie(uint8_t *pst_vap, uint8_t *puc_buffer,
    uint8_t *puc_ie_len);
#endif
oal_bool_enum_uint8 mac_frame_is_null_data(oal_netbuf_stru *pst_net_buf);
uint16_t mac_get_rsn_capability(const uint8_t *puc_rsn_ie);
/* P2P OUI 定义 */
extern const uint8_t g_auc_p2p_oui[MAC_OUI_LEN];
/* 窄带 OUI 定义 */
extern const uint8_t g_auc_huawei_oui[MAC_OUI_LEN];
/* 携带vendor私有VHT IE开关 */
extern uint8_t g_mac_vendor_vht_switch;
uint8_t *mac_get_rx_payload(oal_netbuf_stru *netbuf, uint16_t *payload_len);
#endif /* end of mac_frame.h */
