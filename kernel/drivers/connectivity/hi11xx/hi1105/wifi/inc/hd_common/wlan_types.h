/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 对应所有公共的信息(HAL同时可以使用的)放到该文件中
 * 作    者 :
 * 创建日期 : 2012年12月3日
 */

#ifndef WLAN_TYPES_H
#define WLAN_TYPES_H

/* 1 其他头文件包含 */
#include "platform_spec.h"
#include "oal_ext_if.h"
#include "wlan_cfg_id.h"
#include "wlan_frame_types.h"
#include "wlan_protocol_types.h"
#include "wlan_types_macro.h"

/* 3 枚举定义 */
/* 3.1 基本枚举类型 */
/* TID编号类别放入平台 */
/* VAP的工作模式 */
typedef enum {
    WLAN_VAP_MODE_CONFIG,   /* 配置模式 */
    WLAN_VAP_MODE_BSS_STA,  /* BSS STA模式 */
    WLAN_VAP_MODE_BSS_AP,   /* BSS APUT模式 */
    WLAN_VAP_MODE_WDS,      /* WDS模式 */
    WLAN_VAP_MODE_MONITOER, /* 侦听模式 */
    WLAN_VAP_HW_TEST,

    WLAN_VAP_MODE_BUTT
} wlan_vap_mode_enum;
typedef uint8_t wlan_vap_mode_enum_uint8;

typedef enum {
    WLAN_LEGACY_VAP_MODE = 0, /* 非P2P设备 */
    WLAN_P2P_GO_MODE = 1,     /* P2P_GO */
    WLAN_P2P_DEV_MODE = 2,    /* P2P_Device */
    WLAN_P2P_CL_MODE = 3,     /* P2P_CL */

    WLAN_P2P_BUTT
} wlan_p2p_mode_enum;
typedef uint8_t wlan_p2p_mode_enum_uint8;

typedef enum {
    WLAN_BAND_2G,
    WLAN_BAND_5G,

    WLAN_BAND_BUTT
} wlan_channel_band_enum;
typedef uint8_t wlan_channel_band_enum_uint8;

typedef enum {
    WLAN_TX_SCHEDULE_TYPE_ENQ_TID = 0,
    WLAN_TX_SCHEDULE_TYPE_TX_COMP,
    WLAN_TX_SCHEDULE_TYPE_OTHERS,

    WLAN_TX_SCHEDULE_TYPE_BUTT
} wlan_tx_schedule_type_enum;
typedef uint8_t wlan_tx_schedule_type_enum_uint8;

typedef enum {
    NARROW_BW_10M = 0x80,
    NARROW_BW_5M = 0x81,
    NARROW_BW_1M = 0x82,
    NARROW_BW_BUTT
} mac_narrow_bw_enum;
typedef uint8_t mac_narrow_bw_enum_uint8;

#if ((defined(_PRE_WLAN_EXPORT)) && (defined(_PRE_WLAN_NARROW_BAND)))
#define AUTORATE_RATE_LEVEL_0 0 /* autorate档位0 */
#define AUTORATE_RATE_LEVEL_1 1 /* autorate档位1 */
#define AUTORATE_RATE_LEVEL_2 2 /* autorate档位2 */

typedef enum {
    POWER_PARAM_UPC = 0,
    POWER_PARAM_LPF = 1,
    POWER_PARAM_DBB = 2,
    POWER_PARAM_BUTT
} mac_power_param_enum;
#endif

typedef enum {
    WLAN_ADDBA_MODE_AUTO,
    WLAN_ADDBA_MODE_MANUAL,

    WLAN_ADDBA_MODE_BUTT
} wlan_addba_mode_enum;
typedef uint8_t wlan_addba_mode_enum_uint8;

typedef struct {
    wlan_cipher_key_type_enum_uint8 en_cipher_key_type; /* 密钥ID/类型 */
    wlan_ciper_protocol_type_enum_uint8 en_cipher_protocol_type;
    uint8_t uc_cipher_key_id;
    uint8_t wapi_pn_incr_mode; /* WAPI PN累加模式。0：每次+2; 1：每次+1 */
} wlan_security_txop_params_stru;

/* 频带能力枚举 */
typedef enum {
    WLAN_BAND_CAP_2G,    /* 只支持2G */
    WLAN_BAND_CAP_5G,    /* 只支持5G */
    WLAN_BAND_CAP_2G_5G, /* 支持2G 5G */

    WLAN_BAND_CAP_BUTT
} wlan_band_cap_enum;
typedef uint8_t wlan_band_cap_enum_uint8;

/* 调制方式枚举 */
typedef enum {
    WLAN_MOD_DSSS,
    WLAN_MOD_OFDM,

    WLAN_MOD_BUTT
} wlan_mod_enum;
typedef uint8_t wlan_mod_enum_uint8;

typedef enum {
    REGDOMAIN_FCC = 0,
    REGDOMAIN_ETSI = 1,
    REGDOMAIN_JAPAN = 2,
    REGDOMAIN_COMMON = 3,
    REGDOMAIN_CN = 4,

    REGDOMAIN_COUNT
} regdomain_enum;
typedef uint8_t regdomain_enum_uint8;

/* 带宽能力枚举 */
typedef enum {
    WLAN_BW_CAP_20M,
    WLAN_BW_CAP_40M,
    WLAN_BW_CAP_80M,
    WLAN_BW_CAP_160M,
    WLAN_BW_CAP_80PLUS80, /* 工作在80+80 */

    WLAN_BW_CAP_BUTT
} wlan_bw_cap_enum;
typedef uint8_t wlan_bw_cap_enum_uint8;

/* 全带宽信息，0~3与描述符字段定义匹配 */
typedef enum {
    WLAN_BANDWIDTH_20 = 0,                      /* 20MHz全带宽 */
    WLAN_BANDWIDTH_40 = 1,                      /* 40MHz全带宽 */
    WLAN_BANDWIDTH_2G_BUTT = 2,                 /* 2G只有40M带宽 */
    WLAN_BANDWIDTH_80 = WLAN_BANDWIDTH_2G_BUTT, /* 80MHz全带宽 */
    WLAN_BANDWIDTH_160 = 3,                     /* 160MHz全带宽 */

    WLAN_BANDWIDTH_BUTT
} wlan_bandwidth_type_enum;
typedef uint8_t wlan_bandwidth_type_enum_uint8;

/* 3.3 HT枚举类型 */
typedef enum {
    WLAN_BAND_WIDTH_20M = 0,
    WLAN_BAND_WIDTH_40PLUS,       /* 从20信道+1 */
    WLAN_BAND_WIDTH_40MINUS,      /* 从20信道-1 */
    WLAN_BAND_WIDTH_80PLUSPLUS,   /* 从20信道+1, 从40信道+1 */
    WLAN_BAND_WIDTH_80PLUSMINUS,  /* 从20信道+1, 从40信道-1 */
    WLAN_BAND_WIDTH_80MINUSPLUS,  /* 从20信道-1, 从40信道+1 */
    WLAN_BAND_WIDTH_80MINUSMINUS, /* 从20信道-1, 从40信道-1 */
#ifdef _PRE_WLAN_FEATURE_160M
    WLAN_BAND_WIDTH_160PLUSPLUSPLUS,    /* 从20信道+1, 从40信道+1, 从80信道+1 */
    WLAN_BAND_WIDTH_160PLUSPLUSMINUS,   /* 从20信道+1, 从40信道+1, 从80信道-1 */
    WLAN_BAND_WIDTH_160PLUSMINUSPLUS,   /* 从20信道+1, 从40信道-1, 从80信道+1 */
    WLAN_BAND_WIDTH_160PLUSMINUSMINUS,  /* 从20信道+1, 从40信道-1, 从80信道-1 */
    WLAN_BAND_WIDTH_160MINUSPLUSPLUS,   /* 从20信道-1, 从40信道+1, 从80信道+1 */
    WLAN_BAND_WIDTH_160MINUSPLUSMINUS,  /* 从20信道-1, 从40信道+1, 从80信道-1 */
    WLAN_BAND_WIDTH_160MINUSMINUSPLUS,  /* 从20信道-1, 从40信道-1, 从80信道+1 */
    WLAN_BAND_WIDTH_160MINUSMINUSMINUS, /* 从20信道-1, 从40信道-1, 从80信道-1 */
#endif

    WLAN_BAND_WIDTH_40M, // 06已不使用, 由于和mp15共用，所以暂不删除
    WLAN_BAND_WIDTH_80M, // 06已不使用, 由于和mp15共用，所以暂不删除

    WLAN_BAND_WIDTH_BUTT
} wlan_channel_bandwidth_enum;
typedef uint8_t wlan_channel_bandwidth_enum_uint8;

typedef enum {
    WLAN_AP_BANDWIDTH_20M = 1,
    WLAN_AP_BANDWIDTH_40M = 2,
    WLAN_AP_BANDWIDTH_80M = 4,
    WLAN_AP_BANDWIDTH_160M = 8,
} wlan_vendor_ap_bandwidth_enum;
typedef uint8_t wlan_vendor_ap_bandwidth_enum_uint8;

typedef enum {
    WLAN_CALI_BAND_2G,
    WLAN_CALI_BAND_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CALI_BAND_6G,
#endif
    WLAN_CALI_BAND_BUTT
} wlan_cali_band_enum;
typedef uint8_t wlan_cali_band_enum_uint8;

typedef enum {
    WLAN_CH_SWITCH_DONE = 0,     /* 信道切换已经完成，AP在新信道运行 */
    WLAN_CH_SWITCH_STATUS_1 = 1, /* AP还在当前信道，准备进行信道切换(发送CSA帧/IE) */
    WLAN_CH_SWITCH_STATUS_2 = 2,

    WLAN_CH_SWITCH_BUTT
} wlan_ch_switch_status_enum;
typedef uint8_t wlan_ch_switch_status_enum_uint8;
typedef enum {
    WLAN_AP_CHIP_OUI_NORMAL = 0,
    WLAN_AP_CHIP_OUI_RALINK = 1, /* 芯片厂商为RALINK */
    WLAN_AP_CHIP_OUI_RALINK1 = 2,
    WLAN_AP_CHIP_OUI_ATHEROS = 3, /* 芯片厂商为ATHEROS */
    WLAN_AP_CHIP_OUI_BCM = 4,     /* 芯片厂商为BROADCOM */
    WLAN_AP_CHIP_OUI_QLM = 5,     /* 芯片厂商为QUALCOMM */

    WLAN_AP_CHIP_OUI_BUTT
} wlan_ap_chip_oui_enum;
typedef uint8_t wlan_ap_chip_oui_enum_uint8;

typedef enum {
    WLAN_CSA_MODE_TX_ENABLE = 0,
    WLAN_CSA_MODE_TX_DISABLE,

    WLAN_CSA_MODE_TX_BUTT
} wlan_csa_mode_tx_enum;
typedef uint8_t wlan_csa_mode_tx_enum_uint8;

typedef enum {
    WLAN_BW_SWITCH_DONE = 0,     /* 频宽切换已完成 */
    WLAN_BW_SWITCH_40_TO_20 = 1, /* 从40MHz带宽切换至20MHz带宽 */
    WLAN_BW_SWITCH_20_TO_40 = 2, /* 从20MHz带宽切换至40MHz带宽 */

    /* 后续添加 */
    WLAN_BW_SWITCH_BUTT
} wlan_bw_switch_status_enum;
typedef uint8_t wlan_bw_switch_status_enum_uint8;

/* 空间流定义 */
#define WLAN_SINGLE_NSS 0
#define WLAN_DOUBLE_NSS 1
#define WLAN_TRIPLE_NSS 2
#define WLAN_FOUR_NSS   3
#define WLAN_NSS_LIMIT  4
/* 因为要用作预编译，所以由枚举改成宏，为了便于理解，下面的类型转义先不变 */
typedef uint8_t wlan_nss_enum_uint8;

/* 通道数定义 */
#define HAL_SINGLE_ANT 1
#define HAL_DOUBLE_ANT 2
#define HAL_TRIPLE_ANT 3
#define HAL_FOUR_ANT 4

typedef enum {
    WLAN_NO_SOUNDING = 0,
    WLAN_NDP_SOUNDING = 1,
    WLAN_STAGGERED_SOUNDING = 2,
    WLAN_LEGACY_SOUNDING = 3,

    WLAN_SOUNDING_BUTT
} wlan_sounding_enum;
typedef uint8_t wlan_sounding_enum_uint8;

typedef enum {
    WLAN_PROT_DATARATE_CHN0_1M = 0,
    WLAN_PROT_DATARATE_CHN1_1M,
    WLAN_PROT_DATARATE_CHN0_6M,
    WLAN_PROT_DATARATE_CHN1_6M,
    WLAN_PROT_DATARATE_CHN0_12M,
    WLAN_PROT_DATARATE_CHN1_12M,
    WLAN_PROT_DATARATE_CHN0_24M,
    WLAN_PROT_DATARATE_CHN1_24M,

    WLAN_PROT_DATARATE_BUTT
} wlan_prot_datarate_enum;
typedef uint8_t wlan_prot_datarate_enum_uint8;

typedef struct {
    uint8_t uc_group_id; /* group_id   */
    uint8_t uc_txop_ps_not_allowed;
    uint16_t us_partial_aid; /* partial_aid */
} wlan_groupid_partial_aid_stru;

typedef enum {
    WLAN_TX_AMSDU_NONE = 0,
    WLAN_TX_AMSDU_BY_2 = 1,
    WLAN_TX_AMSDU_BY_3 = 2,
    WLAN_TX_AMSDU_BY_4 = 3,
    WLAN_TX_AMSDU_BY_8 = 4,

    WLAN_TX_AMSDU_BUTT
} wlan_tx_amsdu_enum;
typedef uint8_t wlan_tx_amsdu_enum_uint8;

/* 动态byass外置LNA使能模式 */
#define WLAN_EXTLNA_BYPASS_EN_OFF   0 /* 动态bypass外置LNA关 */
#define WLAN_EXTLNA_BYPASS_EN_ON    1 /* 动态bypass外置LNA开 */
#define WLAN_EXTLNA_BYPASS_EN_FORCE 2 /* 动态bypass外置LNA强制开 */

/* DFS使能模式 */
#define WLAN_DFS_EN_ALL_PULSE 2 /* 算法打开,检测到雷达切换信道且打印所有Pulse */

#define wlan_band_width_is_40m(_uc_bandwidth) \
    ((WLAN_BAND_WIDTH_40MINUS == (_uc_bandwidth)) || (WLAN_BAND_WIDTH_40PLUS == (_uc_bandwidth)))
#define wlan_band_width_is_80m(_uc_bandwidth) \
    (((_uc_bandwidth) >= WLAN_BAND_WIDTH_80PLUSPLUS) && ((_uc_bandwidth) <= WLAN_BAND_WIDTH_80MINUSMINUS))
#ifdef _PRE_WLAN_FEATURE_160M
#define wlan_band_width_is_160m(_uc_bandwidth) \
    (((_uc_bandwidth) >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) && ((_uc_bandwidth) <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
#endif
#define wlan_band_width_is_20m_or_40m(_uc_bandwidth) \
    ((WLAN_BAND_WIDTH_20M == (_uc_bandwidth)) || (WLAN_BAND_WIDTH_40PLUS == (_uc_bandwidth)) \
        || (WLAN_BAND_WIDTH_40MINUS == (_uc_bandwidth)))

#define wlan_freq_bw_to_bw_type(_freq_bw, _protocol) \
    (((_protocol) == WLAN_PHY_PROTOCOL_MODE_HE_ER_SU) ? WLAN_BANDWIDTH_20 : \
    (WLAN_FREQ_BW_20M == (_freq_bw)) ? WLAN_BANDWIDTH_20 : \
    (WLAN_FREQ_BW_40M == (_freq_bw)) ? WLAN_BANDWIDTH_40 : \
    (WLAN_FREQ_BW_80M == (_freq_bw)) ? WLAN_BANDWIDTH_80 : \
    (WLAN_FREQ_BW_160M == (_freq_bw)) ? WLAN_BANDWIDTH_160 : \
    (WLAN_FREQ_BW_80M_SEC20M_PUNCTURE == (_freq_bw) || \
        WLAN_FREQ_BW_80M_SEC40M_RANDOM20M_PUUCTURE == (_freq_bw)) ? WLAN_BANDWIDTH_80 : \
    (WLAN_FREQ_BW_160M_PRI80M_SEC20M_PUNCTURE == (_freq_bw) || \
        WLAN_FREQ_BW_160M_PRI80M_PRI40M_PRESENT == (_freq_bw)) ? WLAN_BANDWIDTH_160 : WLAN_BANDWIDTH_BUTT)

/* 3.5 WME枚举类型 */
/* WMM枚举类型放入平台 */
/* 3.6 信道枚举 */
/* 信道编码方式 */
typedef enum {
    WLAN_BCC_CODE = 0,
    WLAN_LDPC_CODE = 1,
    WLAN_CHANNEL_CODE_BUTT
} wlan_channel_code_enum;
typedef uint8_t wlan_channel_code_enum_uint8;

/* 扫描类型 */
typedef enum {
    WLAN_SCAN_TYPE_PASSIVE = 0,
    WLAN_SCAN_TYPE_ACTIVE  = 1,
    WLAN_SCAN_TYPE_HALF    = 2,

    WLAN_SCAN_TYPE_BUTT
} wlan_scan_type_enum;
typedef uint8_t wlan_scan_type_enum_uint8;

/* 扫描模式 */
typedef enum {
    WLAN_SCAN_MODE_FOREGROUND = 0,      /* 前景扫描不分AP/STA(即初始扫描，连续式) */
    WLAN_SCAN_MODE_BACKGROUND_STA = 1,  /* STA背景扫描 */
    WLAN_SCAN_MODE_BACKGROUND_AP = 2,   /* AP背景扫描(间隔式) */
    WLAN_SCAN_MODE_ROAM_SCAN = 3,
    WLAN_SCAN_MODE_BACKGROUND_OBSS = 4, /* 20/40MHz共存的obss扫描 */
    WLAN_SCAN_MODE_BACKGROUND_PNO = 5, /* PNO调度扫描 */
    WLAN_SCAN_MODE_RRM_BEACON_REQ = 6,
    WLAN_SCAN_MODE_BACKGROUND_CSA = 7,    /* 信道切换扫描 */
    WLAN_SCAN_MODE_BACKGROUND_HILINK = 8, /* hilink扫描未关联用户 */
#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_SCAN_MODE_FTM_REQ = 9,
#endif
    WLAN_SCAN_MODE_GNSS_SCAN = 10,
    WLAN_SCAN_MODE_BACKGROUND_CCA = 11,
#ifdef _PRE_WLAN_FEATURE_NAN
    WLAN_SCAN_MODE_NAN_ROC = 12,
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    WLAN_SCAN_MODE_HID2D_SCAN = 13, /* HiD2D投屏场景扫描 */
#endif
    WLAN_SCAN_MODE_MGMT_TX = 14,
    WLAN_SCAN_MODE_BUTT
} wlan_scan_mode_enum;
typedef uint8_t wlan_scan_mode_enum_uint8;

/* 内核定义flag标志位，BIT9是否启动并发扫描标志位 */
typedef enum {
    WLAN_SCAN_FLAG_LOW_PRIORITY = BIT0,
    WLAN_SCAN_FLAG_LOW_FLUSH = BIT1,
    WLAN_SCAN_FLAG_AP = BIT2,
    WLAN_SCAN_FLAG_RANDOM_ADDR = BIT3,
    WLAN_SCAN_FLAG_LOW_SPAN = BIT8,
    WLAN_SCAN_FLAG_LOW_POWER = BIT9,      /* 并发扫描 */
    WLAN_SCAN_FLAG_HIFH_ACCURACY = BIT10, /* 顺序扫描,非并发 */
    WLAN_SCAN_FLAG_BUTT
} wlan_scan_flag_enum;

/* 3.7 加密枚举 */
/* pmf的能力 */
typedef enum {
    MAC_PMF_DISABLED = 0, /* 不支持pmf能力 */
    MAC_PMF_ENABLED,      /* 支持pmf能力，且不强制 */
    MAC_PMF_REQUIRED,     /* 严格执行pmf能力 */

    MAC_PMF_BUTT
} wlan_pmf_cap_status;
typedef uint8_t wlan_pmf_cap_status_uint8;

/* 用户距离状态 */
typedef enum {
    WLAN_DISTANCE_NEAR = 0,
    WLAN_DISTANCE_NORMAL = 1,
    WLAN_DISTANCE_FAR = 2,

    WLAN_DISTANCE_BUTT
} wlan_user_distance_enum;
typedef uint8_t wlan_user_distance_enum_uint8;

/* 3.8 linkloss场景枚举 */
/* linkloss场景枚举 */
typedef enum {
    WLAN_LINKLOSS_MODE_BT = 0,
    WLAN_LINKLOSS_MODE_DBAC,
    WLAN_LINKLOSS_MODE_NORMAL,

    WLAN_LINKLOSS_MODE_BUTT
} wlan_linkloss_mode_enum;
typedef uint8_t wlan_linkloss_mode_enum_uint8;

typedef enum {
    WALN_LINKLOSS_SCAN_SWITCH_CHAN_DISABLE = 0,
    WALN_LINKLOSS_SCAN_SWITCH_CHAN_EN = 1,

    WALN_LINKLOSS_SCAN_SWITCH_CHAN_BUTT
} wlan_linkloss_scan_switch_chan_enum;
typedef uint8_t wlan_linkloss_scan_switch_chan_enum_uint8;

/* 3.9 roc场景枚举 */
typedef enum {
    IEEE80211_ROC_TYPE_NORMAL = 0,
    IEEE80211_ROC_TYPE_MGMT_TX = 1, /* 管理帧发送 */
    IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_TX = 2, /* 高优先级管理帧发送 */
    IEEE80211_ROC_TYPE_HIGH_PRIO_MGMT_RX = 3, /* 高优先级管理帧接收 */
    IEEE80211_ROC_TYPE_BUTT,
} wlan_ieee80211_roc_type;
typedef uint8_t wlan_ieee80211_roc_type_uint8;

/* ldac m2s 门限场景枚举 */
typedef enum {
    WLAN_M2S_LDAC_RSSI_TO_SISO = 0,
    WLAN_M2S_LDAC_RSSI_TO_MIMO,

    WLAN_M2S_LDAC_RSSI_BUTT
} wlan_m2s_ldac_rssi_th_enum;
typedef uint8_t wlan_m2s_ldac_rssi_th_enum_uint8;

/* btcoex mcm  门限场景枚举 */
typedef enum {
    WLAN_BTCOEX_RSSI_MCM_DOWN = 0,
    WLAN_BTCOEX_RSSI_MCM_UP,

    WLAN_BTCOEX_RSSI_MCM_BUTT
} wlan_mcm_btcoex_rssi_th_enum;
typedef uint8_t wlan_mcm_btcoex_rssi_th_enum_uint8;

typedef enum {
    DEVICE_RX = 0,
    HOST_RX = 1,

    RX_SWITCH_BUTT,
} wlan_rx_switch_enum;

/* 3.18 数采类型枚举 */
typedef enum {
    RX_ADC_SISO_CH0 = 0,
    RX_ADC_SISO_CH1 = 1,
    RX_ADC_MIMO     = 2,
    TX_DFE_DAC_SISO_CH0 = 3,
    TX_DFE_DAC_SISO_CH1 = 4,
    TX_DFE_DAC_MIMO = 5,
    FFT_IN_SISO_CH0 = 6,
    FFT_IN_SISO_CH1 = 7,
    FFT_IN_MIMO     = 8,
    MIMO_DATA       = 9,
    NDP_TRIGGER     = 10,
    FCS_TRIGGER     = 11,
    RX_SCO_MIMO     = 12,
    NACI            = 13,
    EVENT_RPT_ALL   = 14,
    RX_AGC_C0       = 15,
    WLAN_DATA_COLLECT_TYPE_BUTT,
} wlan_data_collect_enum;
typedef uint8_t wlan_data_collect_enum_uint8;

typedef enum {
    WLAN_PCIE_PROBE_DEFAULT = 0, /* 该vap的默认pcie状态，为高协议 */
    WLAN_PCIE_PROBE_LOW = 1,  /* 该vap可降协议 */
} wlan_pcie_probe_state;
typedef struct {
    uint32_t cipher;                /* 加密类型 */
    uint32_t key_len;               /* 密钥长 */
    uint32_t seq_len;               /* sequnece长 */
    uint8_t auc_key[WLAN_WPA_KEY_LEN]; /* 密钥 */
    uint8_t auc_seq[WLAN_WPA_SEQ_LEN]; /* sequence */
} wlan_priv_key_param_stru;

/* action帧发送类型枚举 */
typedef enum {
    WLAN_M2S_ACTION_TYPE_SMPS = 0,   /* action采用smps */
    WLAN_M2S_ACTION_TYPE_OPMODE = 1, /* action采用opmode */
    WLAN_M2S_ACTION_TYPE_NONE = 2,   /* 切换不发action帧 */

    WLAN_M2S_ACTION_TYPE_BUTT,
} wlan_m2s_action_type_enum;
typedef uint8_t wlan_m2s_action_type_enum_uint8;

typedef struct {
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN]; /* 切换对应的AP  */
    wlan_m2s_action_type_enum_uint8 en_action_type; /* 切换对应需要发送的action帧类型，需要和vap下ori交互处理 */
    oal_bool_enum_uint8 en_m2s_result;              /* 切换是否符合初始action帧，作为vap切换成功失败标记 */
} wlan_m2s_mgr_vap_stru;
#endif
