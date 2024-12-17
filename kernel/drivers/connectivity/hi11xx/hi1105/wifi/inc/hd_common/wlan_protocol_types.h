/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应所有公共的信息(HAL同时可以使用的)放到该文件中
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef WLAN_PROTOCOL_TYPES_H
#define WLAN_PROTOCOL_TYPES_H
#include "oal_ext_if.h"
/* 2.4 安全相关宏定义 */
/* 内核如果已经定义则使用内核的宏定义，但要注意内核宏定义值是否符合预期!! */
/* cipher suite selectors */
#ifndef WLAN_CIPHER_SUITE_USE_GROUP
#define WLAN_CIPHER_SUITE_USE_GROUP 0x000FAC00
#endif

#ifndef WLAN_CIPHER_SUITE_WEP40
#define WLAN_CIPHER_SUITE_WEP40 0x000FAC01
#endif

#ifndef WLAN_CIPHER_SUITE_TKIP
#define WLAN_CIPHER_SUITE_TKIP 0x000FAC02
#endif

/* reserved:                0x000FAC03 */
#ifndef WLAN_CIPHER_SUITE_CCMP
#define WLAN_CIPHER_SUITE_CCMP 0x000FAC04
#endif

#ifndef WLAN_CIPHER_SUITE_WEP104
#define WLAN_CIPHER_SUITE_WEP104 0x000FAC05
#endif

#ifndef WLAN_CIPHER_SUITE_AES_CMAC
#define WLAN_CIPHER_SUITE_AES_CMAC 0x000FAC06
#endif

#ifndef WLAN_CIPHER_SUITE_GCMP
#define WLAN_CIPHER_SUITE_GCMP 0x000FAC08
#endif

#ifndef WLAN_CIPHER_SUITE_GCMP_256
#define WLAN_CIPHER_SUITE_GCMP_256 0x000FAC09
#endif

#ifndef WLAN_CIPHER_SUITE_CCMP_256
#define WLAN_CIPHER_SUITE_CCMP_256 0x000FAC0A
#endif

#ifndef WLAN_CIPHER_SUITE_BIP_GMAC_128
#define WLAN_CIPHER_SUITE_BIP_GMAC_128 0x000FAC0B
#endif

#ifndef WLAN_CIPHER_SUITE_BIP_GMAC_256
#define WLAN_CIPHER_SUITE_BIP_GMAC_256 0x000FAC0C
#endif

#ifndef WLAN_CIPHER_SUITE_BIP_CMAC_256
#define WLAN_CIPHER_SUITE_BIP_CMAC_256 0x000FAC0D
#endif

#ifdef _PRE_WLAN_FEATURE_PWL
#ifndef WLAN_CIPHER_SUITE_PWL_CMAC_128
#define WLAN_CIPHER_SUITE_PWL_CMAC_128 0x00e0fc01
#endif

#ifndef WLAN_CIPHER_SUITE_PWL_GCM_128
#define WLAN_CIPHER_SUITE_PWL_GCM_128 0x00e0fc02
#endif
#endif

#undef WLAN_CIPHER_SUITE_SMS4
#define WLAN_CIPHER_SUITE_SMS4 0x00147201

/* AKM suite selectors */
#define WITP_WLAN_AKM_SUITE_8021X     0x000FAC01
#define WITP_WLAN_AKM_SUITE_PSK       0x000FAC02
#define WITP_WLAN_AKM_SUITE_WAPI_PSK  0x000FAC04
#define WITP_WLAN_AKM_SUITE_WAPI_CERT 0x000FAC12

#define WLAN_PMKID_LEN        16
#define WLAN_PMKID_CACHE_SIZE 32

#define WLAN_TEMPORAL_KEY_LENGTH 16
#define WLAN_MIC_KEY_LENGTH      8

#define WLAN_WEP40_KEY_LEN  5
#define WLAN_WEP104_KEY_LEN 13
#define WLAN_TKIP_KEY_LEN   32 /* TKIP KEY length 256 BIT */

#define WLAN_BIP_KEY_LEN    16 /* BIP KEY length 128 BIT */

#define WLAN_NUM_DOT11WEPDEFAULTKEYVALUE 4
#define WLAN_MAX_WEP_STR_SIZE            27 /* 5 for WEP 40; 13 for WEP 104 */
#define WLAN_WEP_SIZE_OFFSET             0
#define WLAN_WEP_40_KEY_SIZE             40
#define WLAN_WEP_104_KEY_SIZE            104
#define WLAN_COUNTRY_STR_LEN 3

/* 11i参数 */
/* WPA 密钥长度 */
#define WLAN_WPA_KEY_LEN    32
#define WLAN_CIPHER_KEY_LEN 16
#define WLAN_TKIP_MIC_LEN   16
/* WPA 序号长度 */
#define WLAN_WPA_SEQ_LEN 16

#define WLAN_WITP_CAPABILITY_PRIVACY BIT4

#define WLAN_NUM_TK             4
#define WLAN_NUM_IGTK           2
#define WLAN_MAX_IGTK_KEY_INDEX 5
#define WLAN_MAX_WEP_KEY_COUNT  4

#ifdef _PRE_WLAN_FEATURE_11AX
/* 11AX MCS相关的内容 */
#define MAC_MAX_SUP_MCS7_11AX_EACH_NSS    0 /* 11AX各空间流支持的最大MCS序号，支持0-7 */

#define MAC_MAX_SUP_MCS11_11AX_EACH_NSS   2 /* 11AX各空间流支持的最大MCS序号，支持0-11 */
#define MAC_MAX_SUP_INVALID_11AX_EACH_NSS 3 /* 不支持 */

#define MAC_MAX_RATE_SINGLE_NSS_80M_11AX 212 /* 1个空间流40MHz的最大Long GI速率 */


#define MAC_TRIGGER_FRAME_PADDING_DURATION0US  0
#define MAC_TRIGGER_FRAME_PADDING_DURATION16US 2
#define MAC_HE_MAX_AMPDU_LEN_EXP_0             0x2 /* HE最大的AMPDU长度系数2^(20+factor)-1字节 */
#endif

#define CIPHER_SUITE_SELECTOR(a, b, c, d) \
    ((((uint32_t)(d)) << 24) | (((uint32_t)(c)) << 16) | (((uint32_t)(b)) << 8) | (uint32_t)(a))

#define MAC_WPA_CHIPER_OUI(_suit_type) CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, _suit_type)
#define MAC_RSN_CHIPER_OUI(_suit_type) CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, _suit_type)

#define MAC_WPA_CHIPER_CCMP    CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 4)
#define MAC_WPA_CHIPER_TKIP    CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK        CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 2)
#define MAC_WPA_AKM_PSK_SHA256 CIPHER_SUITE_SELECTOR(0x00, 0x50, 0xf2, 6)

#define MAC_RSN_CHIPER_CCMP    CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 4)
#define MAC_RSN_CHIPER_TKIP    CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_CIPHER_SUITE_AES_128_CMAC CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 6)

#define MAC_RSN_AKM_8021X              CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 1)
#define MAC_RSN_AKM_PSK                CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 2)
#define MAC_RSN_AKM_FT_OVER_8021X      CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 3)
#define MAC_RSN_AKM_FT_PSK             CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 4)
#define MAC_RSN_AKM_PSK_SHA256         CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 6)
#define MAC_RSN_AKM_SAE                CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 8)
#define MAC_RSN_AKM_FT_OVER_SAE        CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 9)
#define MAC_RSN_AKM_FT_OVER_8021X_SHA384 CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 13)
#define MAC_RSN_AKM_OWE                CIPHER_SUITE_SELECTOR(0x00, 0x0f, 0xac, 18)
#define MAC_RSN_AKM_TBPEKE             CIPHER_SUITE_SELECTOR(0xac, 0x85, 0x3d, 98)

#define WLAN_HT_MCS32                       32
#define WLAN_MIN_MPDU_LEN_FOR_MCS32         12
#define WLAN_MIN_MPDU_LEN_FOR_MCS32_SHORTGI 13
/*
 * 1）在协议规定时长5.484ms下，HE最大物理层速率为2.4Gbps(VHT为2.1667Gbps)，
 *    可以支持的最大字节为1645200 Bytes(VHT:1484793Bytes)；
 * 2）其中2^21 = 2097152，2^20 = 1048576，所以最大次幂为20；
 * 3）所以，HE Max A-MPDU Length Exponent Extension字段填写0；
 * 4）VHT Max A-MPDU Length Exponent Extension字段需要修改为7
 */
#define MAC_VHT_AMPDU_MAX_LEN_EXP 7 /* 2^(13+factor)-1字节 */

/* 默认的数据类型业务的TID */
#define WLAN_TID_FOR_DATA 0

/* 不应该BUTT后续存在枚举 */
typedef enum {
    WLAN_WME_AC_BE = 0, /* best effort */
    WLAN_WME_AC_BK = 1, /* background */
    WLAN_WME_AC_VI = 2, /* video */
    WLAN_WME_AC_VO = 3, /* voice */

    WLAN_WME_AC_BUTT = 4,
    WLAN_WME_BASIC_AC_NUM = WLAN_WME_AC_BUTT, /* 仅包含VI,VO,BE,BK 4个队列,涉及调度算法模块,EDCA AC队列参数配置 */
    WLAN_WME_AC_MGMT = WLAN_WME_AC_BUTT, /* 管理AC，协议没有,对应硬件高优先级队列 */

    WLAN_WME_AC_PSM = 5 /* 节能AC, 协议没有，对应硬件组播队列 */
} wlan_wme_ac_type_enum;
typedef uint8_t wlan_wme_ac_type_enum_uint8;

/* TID个数为8,0~7 */
#define WLAN_TID_MAX_NUM WLAN_TIDNO_BUTT

/* TID编号类别 */
typedef enum {
    WLAN_TIDNO_BEST_EFFORT = 0,            /* BE业务 */
    WLAN_TIDNO_BACKGROUND = 1,             /* BK业务 */
    WLAN_TIDNO_UAPSD = 2,                  /* U-APSD */
    WLAN_TIDNO_NETWORK_SLICING = 2,        /* 网络切片队列：游戏帧业务 */
    WLAN_TIDNO_MCAST_AMPDU = 3,  /* 组播聚合帧 */
    WLAN_TIDNO_ANT_TRAINING_LOW_PRIO = 3,  /* 智能天线低优先级训练帧 */
    WLAN_TIDNO_ANT_TRAINING_HIGH_PRIO = 4, /* 智能天线高优先级训练帧 */
    WLAN_TIDNO_VSP = 4,                    /* VSP低时延业务帧 */
    WLAN_TIDNO_VIDEO = 5,                  /* VI业务 */
    WLAN_TIDNO_MINTP = 5,                  /* MinTP低时延业务帧 */
    WLAN_TIDNO_VOICE = 6,                  /* VO业务 */
    WLAN_TIDNO_BCAST = 7,                  /* 广播用户的广播或者组播报文 */

    WLAN_TIDNO_BUTT
} wlan_tidno_enum;
typedef uint8_t wlan_tidno_enum_uint8;

/* Protection mode for MAC */
typedef enum {
    WLAN_PROT_NO,  /* Do not use any protection       */
    WLAN_PROT_ERP, /* Protect all ERP frame exchanges */
    WLAN_PROT_HT,  /* Protect all HT frame exchanges  */
    WLAN_PROT_GF,  /* Protect all GF frame exchanges  */

    WLAN_PROT_BUTT
} wlan_prot_mode_enum;
typedef uint8_t wlan_prot_mode_enum_uint8;

typedef enum {
    WLAN_RTS_RATE_SELECT_MODE_REG,  /* 0: RTS速率 = PROT_DATARATE的配置                           */
    WLAN_RTS_RATE_SELECT_MODE_DESC, /* 1: RTS速率 = 硬件根据TX描述符计算出的值                    */
    WLAN_RTS_RATE_SELECT_MODE_MIN,  /* 2: RTS速率 = min(PROT_DATARATE,硬件根据TX描述符计算出的值) */
    WLAN_RTS_RATE_SELECT_MODE_MAX,  /* 3: RTS速率 = max(PROT_DATARATE,硬件根据TX描述符计算出的值) */

    WLAN_RTS_RATE_SELECT_MODE_BUTT
} wlan_rts_rate_select_mode_enum;

typedef enum {
    /*
     *  注意: wlan_cipher_key_type_enum和hal_key_type_enum 值一致,
     *        如果硬件有改变，则应该在HAL 层封装
     */
    /* 此处保持和协议规定额秘钥类型一致 */
    WLAN_KEY_TYPE_TX_GTK = 0,  /* TX GTK */
    WLAN_KEY_TYPE_PTK = 1,     /* PTK */
    WLAN_KEY_TYPE_RX_GTK = 2,  /* RX GTK */
    WLAN_KEY_TYPE_RX_GTK2 = 3, /* RX GTK2 51不用 */
    WLAN_KEY_TYPE_BUTT
} wlan_cipher_key_type_enum;
typedef uint8_t wlan_cipher_key_type_enum_uint8;

typedef enum {
    /*
     *  注意: wlan_key_origin_enum_uint8和hal_key_origin_enum_uint8 值一致,
     *        如果硬件有改变，则应该在HAL 层封装
     */
    WLAN_AUTH_KEY = 0, /* Indicates that for this key, this STA is the authenticator */
    WLAN_SUPP_KEY = 1, /* Indicates that for this key, this STA is the supplicant */

    WLAN_KEY_ORIGIN_BUTT,
} wlan_key_origin_enum;
typedef uint8_t wlan_key_origin_enum_uint8;

typedef enum {
    /* 按照80211-2012/ 11ac-2013 协议 Table 8-99 Cipher suite selectors 定义 */
    WLAN_80211_CIPHER_SUITE_GROUP_CIPHER = 0,
    WLAN_80211_CIPHER_SUITE_WEP_40 = 1,
    WLAN_80211_CIPHER_SUITE_TKIP = 2,
    WLAN_80211_CIPHER_SUITE_RSV = 3,
    WLAN_80211_CIPHER_SUITE_NO_ENCRYP = WLAN_80211_CIPHER_SUITE_RSV, /* 采用协议定义的保留值做不加密类型 */
    WLAN_80211_CIPHER_SUITE_CCMP = 4,
    WLAN_80211_CIPHER_SUITE_WEP_104 = 5,
    WLAN_80211_CIPHER_SUITE_BIP = 6,
    WLAN_80211_CIPHER_SUITE_GROUP_DENYD = 7,
    WLAN_80211_CIPHER_SUITE_GCMP = 8, /* GCMP-128 default for a DMG STA */
    WLAN_80211_CIPHER_SUITE_GCMP_256 = 9,
    WLAN_80211_CIPHER_SUITE_CCMP_256 = 10,
    WLAN_80211_CIPHER_SUITE_BIP_GMAC_128 = 11, /* BIP GMAC 128 */
    WLAN_80211_CIPHER_SUITE_BIP_GMAC_256 = 12, /* BIP GMAC 256 */
    WLAN_80211_CIPHER_SUITE_BIP_CMAC_256 = 13, /* BIP CMAC 256 */

    WLAN_80211_CIPHER_SUITE_WAPI = 14, /* 随意定，不影响11i即可 */
} wlan_ciper_protocol_type_enum;
typedef uint8_t wlan_ciper_protocol_type_enum_uint8;

/* 按照80211-2012 协议 Table 8-101 AKM suite selectors 定义 */
#define WLAN_AUTH_SUITE_PSK        2
#define WLAN_AUTH_SUITE_FT_1X      3
#define WLAN_AUTH_SUITE_FT_PSK     4
#define WLAN_AUTH_SUITE_PSK_SHA256 6
#define WLAN_AUTH_SUITE_FT_SHA256  9

#define MAC_OWE_GROUP_SUPPORTED_NUM (3)
#define MAC_OWE_GROUP_19            (19)
#define MAC_OWE_GROUP_20            (20)
#define MAC_OWE_GROUP_21            (21)
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif
/* 注意：需要对齐内核 nl80211_wpa_versions */
typedef enum {
    WITP_WPA_VERSION_1 = 1 << 0,
    WITP_WPA_VERSION_2 = 1 << 1,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#ifdef _PRE_WLAN_FEATURE_WAPI
    WITP_WAPI_VERSION = 1 << 2,
#endif
#else
    WITP_WPA_VERSION_3 = 1 << 2,
#ifdef _PRE_WLAN_FEATURE_WAPI
    WITP_WAPI_VERSION = 1 << 3,
#endif
#endif
} witp_wpa_versions_enum;
typedef uint8_t witp_wpa_versions_enum_uint8;

enum wapi_pn_incr_mode {
    WAPI_PN_INCR_MODE_PLUS_2 = 0,
    WAPI_PN_INCR_MODE_PLUS_1 = 1,
};

/* 协议能力枚举 */
typedef enum {
    WLAN_PROTOCOL_CAP_LEGACY,
    WLAN_PROTOCOL_CAP_HT,
    WLAN_PROTOCOL_CAP_VHT,
#ifdef _PRE_WLAN_FEATURE_11AX
    WLAN_PROTOCOL_CAP_HE,
#endif

    WLAN_PROTOCOL_CAP_BUTT,
} wlan_protocol_cap_enum;
typedef uint8_t wlan_protocol_cap_enum_uint8;

/* 3.4 HE枚举类型 */
/* 3.4 VHT枚举类型 */
typedef enum {
    WLAN_VHT_MCS0,
    WLAN_VHT_MCS1,
    WLAN_VHT_MCS2,
    WLAN_VHT_MCS3,
    WLAN_VHT_MCS4,
    WLAN_VHT_MCS5,
    WLAN_VHT_MCS6,
    WLAN_VHT_MCS7,
    WLAN_VHT_MCS8,
    WLAN_VHT_MCS9,
#ifdef _PRE_WLAN_FEATURE_1024QAM
    WLAN_VHT_MCS10,
    WLAN_VHT_MCS11,
#endif

    WLAN_VHT_MCS_BUTT,
} wlan_vht_mcs_enum;
typedef uint8_t wlan_vht_mcs_enum_uint8;

/*
 * 复用1101定义的顺序
 * 和周赟讨论后，修正速率的先后顺序
 */
typedef enum {
    WLAN_LEGACY_11B_RESERVED1 = 0,
    WLAN_SHORT_11B_2M_BPS = 1,
    WLAN_SHORT_11B_5_HALF_M_BPS = 2,

    WLAN_SHORT_11B_11_M_BPS = 3,

    WLAN_LONG_11B_1_M_BPS = 4,
    WLAN_LONG_11B_2_M_BPS = 5,
    WLAN_LONG_11B_5_HALF_M_BPS = 6,
    WLAN_LONG_11B_11_M_BPS = 7,

    WLAN_LEGACY_OFDM_48M_BPS = 8,
    WLAN_LEGACY_OFDM_24M_BPS = 9,
    WLAN_LEGACY_OFDM_12M_BPS = 10,
    WLAN_LEGACY_OFDM_6M_BPS = 11,
    WLAN_LEGACY_OFDM_54M_BPS = 12,
    WLAN_LEGACY_OFDM_36M_BPS = 13,
    WLAN_LEGACY_OFDM_18M_BPS = 14,
    WLAN_LEGACY_OFDM_9M_BPS = 15,

    WLAN_LEGACY_RATE_VALUE_BUTT
} wlan_legacy_rate_value_enum;
typedef uint8_t wlan_legacy_rate_value_enum_uint8;

/* WIFI协议类型定义 */
/* Note: 此定义若修改, 请知会 00330043, 需要相应更新g_auc_default_mac_to_phy_protocol_mapping数组值 */
typedef enum {
    WLAN_LEGACY_11A_MODE = 0,    /* 11a, 5G, OFDM */
    WLAN_LEGACY_11B_MODE = 1,    /* 11b, 2.4G */
    WLAN_LEGACY_11G_MODE = 2,    /* 旧的11g only已废弃, 2.4G, OFDM */
    WLAN_MIXED_ONE_11G_MODE = 3, /* 11bg, 2.4G */
    WLAN_MIXED_TWO_11G_MODE = 4, /* 11g only, 2.4G */
    WLAN_HT_MODE = 5,            /* 11n(11bgn或者11an，根据频段判断) */
    WLAN_VHT_MODE = 6,           /* 11ac */
    WLAN_HT_ONLY_MODE = 7,       /* 11n only mode,只有带HT的设备才可以接入 */
    WLAN_VHT_ONLY_MODE = 8,      /* 11ac only mode 只有带VHT的设备才可以接入 */
    WLAN_HT_11G_MODE = 9,        /* 11ng,不包括11b */
    WLAN_HE_MODE = 10,           /* 11ax */
    WLAN_HT_ONLY_MODE_2G = 11,
    WLAN_VHT_ONLY_MODE_2G = 12,
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
    WLAN_PROTOCOL_BUTT = 11,
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
    WLAN_PROTOCOL_BUTT = 12,
#else
    WLAN_PROTOCOL_BUTT,
#endif
} wlan_protocol_enum;
typedef uint8_t wlan_protocol_enum_uint8;

/* 重要:代表VAP的preamble协议能力的使用该枚举，0表示long preamble; 1表示short preamble */
typedef enum {
    WLAN_LEGACY_11B_MIB_LONG_PREAMBLE = 0,
    WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE = 1,
} wlan_11b_mib_preamble_enum;
typedef uint8_t wlan_11b_mib_preamble_enum_uint8;

/* 重要:仅限描述符接口使用(表示发送该帧使用的pramble类型)，0表示short preamble; 1表示long preamble */
typedef enum {
    WLAN_LEGACY_11B_DSCR_SHORT_PREAMBLE = 0,
    WLAN_LEGACY_11B_DSCR_LONG_PREAMBLE = 1,

    WLAN_LEGACY_11B_PREAMBLE_BUTT
} wlan_11b_dscr_preamble_enum;
typedef uint8_t wlan_11b_dscr_preamble_enum_uint8;

/* 3.12 HE枚举 */
typedef enum {
    WLAN_HE_MCS0,
    WLAN_HE_MCS1,
    WLAN_HE_MCS2,
    WLAN_HE_MCS3,
    WLAN_HE_MCS4,
    WLAN_HE_MCS5,
    WLAN_HE_MCS6,
    WLAN_HE_MCS7,
    WLAN_HE_MCS8,
    WLAN_HE_MCS9,
    WLAN_HE_MCS10,
    WLAN_HE_MCS11,
#ifdef _PRE_WLAN_FEATURE_4096QAM
    WLAN_HE_MCS12,
    WLAN_HE_MCS13,
#endif
    WLAN_HE_MCS_BUTT,
} wlan_he_mcs_enum;
typedef uint8_t wlan_he_mcs_enum_uint8;

/* ax协议支持的RU大小 */
typedef enum {
    WLAN_RUSIZE_26_TONES = 0,    /* 26-tones大小的RU */
    WLAN_RUSIZE_52_TONES = 1,    /* 52-tones大小的RU */
    WLAN_RUSIZE_106_TONES = 2,   /* 106-tones大小的RU */
    WLAN_RUSIZE_242_TONES = 3,   /* 242-tones大小的RU */
    WLAN_RUSIZE_484_TONES = 4,   /* 484-tones大小的RU */
    WLAN_RUSIZE_996_TONES = 5,   /* 996-tones大小的RU */
    WLAN_RUSIZE_2X996_TONES = 6, /* 2x996-tones大小的RU */

    WLAN_RUSIZE_BUTT
} wlan_rusize_type_enum;
typedef uint8_t wlan_rusize_type_enum_uint8;
#endif /* end of wlan_protocol_types.h */
