/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化函数声明
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */

#ifndef HISI_CUSTOMIZE_CONFIG_CMD_MP13_H
#define HISI_CUSTOMIZE_CONFIG_CMD_MP13_H

#include "hisi_customize_wifi_mp13.h"

/* 定制化 INI CONFIG ID */
typedef enum {
    /* ROAM */
    WLAN_CFG_INIT_ROAM_SWITCH = 0,
    WLAN_CFG_INIT_SCAN_ORTHOGONAL,
    WLAN_CFG_INIT_TRIGGER_B,
    WLAN_CFG_INIT_TRIGGER_A,
    WLAN_CFG_INIT_DELTA_B,
    WLAN_CFG_INIT_DELTA_A,
    WLAN_CFG_INIT_DENSE_ENV_TRIGGER_B,
    WLAN_CFG_INIT_DENSE_ENV_TRIGGER_A,
    WLAN_CFG_INIT_SCENARIO_ENABLE,
    WLAN_CFG_INIT_CANDIDATE_GOOD_RSSI,
    WLAN_CFG_INIT_CANDIDATE_GOOD_NUM,
    WLAN_CFG_INIT_CANDIDATE_WEAK_NUM,
    WLAN_CFG_INIT_INTERVAL_VARIABLE,

    /* 性能 */
    WLAN_CFG_INIT_AMPDU_TX_MAX_NUM,  // 7
    WLAN_CFG_INIT_USED_MEM_FOR_START,
    WLAN_CFG_INIT_USED_MEM_FOR_STOP,
    WLAN_CFG_INIT_RX_ACK_LIMIT,
    WLAN_CFG_INIT_SDIO_D2H_ASSEMBLE_COUNT,
    WLAN_CFG_INIT_SDIO_H2D_ASSEMBLE_COUNT,
    /* LINKLOSS */
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_BT,  // 13,这里不能直接赋值
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_DBAC,
    WLAN_CFG_INIT_LINK_LOSS_THRESHOLD_NORMAL,
    /* 自动调频 */
    WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_0,  // 16
    WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_0,
    WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_0,
    WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_1,
    WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_1,
    WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_1,
    WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_2,
    WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_2,
    WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_2,
    WLAN_CFG_INIT_PSS_THRESHOLD_LEVEL_3,
    WLAN_CFG_INIT_CPU_FREQ_LIMIT_LEVEL_3,
    WLAN_CFG_INIT_DDR_FREQ_LIMIT_LEVEL_3,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_0,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_1,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_2,
    WLAN_CFG_INIT_DEVICE_TYPE_LEVEL_3,
    WLAN_CFG_PRIV_DMA_LATENCY,
    WLAN_CFG_PRIV_LOCK_CPU_TH_HIGH,
    WLAN_CFG_PRIV_LOCK_CPU_TH_LOW,
    WLAN_CFG_INIT_IRQ_AFFINITY,
    WLAN_CFG_INIT_IRQ_TH_LOW,
    WLAN_CFG_INIT_IRQ_TH_HIGH,
    WLAN_CFG_INIT_IRQ_PPS_TH_LOW,
    WLAN_CFG_INIT_IRQ_PPS_TH_HIGH,
#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    WLAN_CFG_INIT_HW_AMPDU,
    WLAN_CFG_INIT_HW_AMPDU_TH_LOW,
    WLAN_CFG_INIT_HW_AMPDU_TH_HIGH,
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    WLAN_CFG_INIT_AMPDU_AMSDU_SKB,
    WLAN_CFG_INIT_AMSDU_AMPDU_TH_LOW,
    WLAN_CFG_INIT_AMSDU_AMPDU_TH_MIDDLE,
    WLAN_CFG_INIT_AMSDU_AMPDU_TH_HIGH,
#endif
#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFG_INIT_TCP_ACK_FILTER,
    WLAN_CFG_INIT_TCP_ACK_FILTER_TH_LOW,
    WLAN_CFG_INIT_TCP_ACK_FILTER_TH_HIGH,
#endif
    WLAN_CFG_INIT_TX_SMALL_AMSDU,
    WLAN_CFG_INIT_SMALL_AMSDU_HIGH,
    WLAN_CFG_INIT_SMALL_AMSDU_LOW,
    WLAN_CFG_INIT_SMALL_AMSDU_PPS_HIGH,
    WLAN_CFG_INIT_SMALL_AMSDU_PPS_LOW,

    WLAN_CFG_INIT_TX_TCP_ACK_BUF,
    WLAN_CFG_DEVICE_TX_TCP_BUF,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_40M,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW_40M,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_80M,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW_80M,
    WLAN_CFG_INIT_TCP_ACK_BUF_HIGH_160M,
    WLAN_CFG_INIT_TCP_ACK_BUF_LOW_160M,

    WLAN_CFG_INIT_TX_TCP_ACK_BUF_USERCTL,
    WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_HIGH,
    WLAN_CFG_INIT_TCP_ACK_BUF_USERCTL_LOW,

    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA,
    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_HIGH,
    WLAN_CFG_INIT_RX_DYN_BYPASS_EXTLNA_LOW,

    /* 接收ampdu+amsdu */
    WLAN_CFG_INIT_RX_AMPDU_AMSDU_SKB,
    WLAN_CFG_INIT_RX_AMPDU_BITMAP,

    /* 低功耗 */
    WLAN_CFG_INIT_POWERMGMT_SWITCH,  // 31
    WLAN_CFG_INIT_PS_MODE,
    WLAN_CFG_INIT_MIN_FAST_PS_IDLE,  // How many 20ms
    WLAN_CFG_INIT_MAX_FAST_PS_IDLE,
    WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON,
    WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF,

    /* 低功耗代理 */
    WLAN_CFG_INIT_LOWPOWER_AGENT_SWITCH_MP13,

    /* 可维可测 */
    WLAN_CFG_INIT_LOGLEVEL,
    /* 2G RF前端 */
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START,  // 33
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND1 = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_START,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND2,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND_END = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_2G_BAND3,
    /* 5G RF前端 */
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START,  // 37
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND1 = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_START,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND2,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND3,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND4,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND5,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND6,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7,
    WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND_END = WLAN_CFG_INIT_RF_RX_INSERTION_LOSS_5G_BAND7,

    /* 用于定制化计算PWR RF值的偏差 */
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C0_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_2G_C1_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C0_MULT4,
    WLAN_CFG_INIT_RF_PWR_REF_RSSI_5G_C1_MULT4,

    /* 用于定制化计算RSSI的偏差 */
    WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C0,
    WLAN_CFG_INIT_RF_AMEND_RSSI_2G_C1,
    WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C0,
    WLAN_CFG_INIT_RF_AMEND_RSSI_5G_C1,

    /* FEM mp13考虑2g */
    WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_2G,
    WLAN_CFG_INIT_RF_LNA_GAIN_DB_2G,
    WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_2G,
    WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_2G,
    WLAN_CFG_INIT_RF_PA_GAIN_LVL_2G,
    WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_2G,
    WLAN_CFG_INIT_EXT_PA_ISEXIST_2G,
    WLAN_CFG_INIT_EXT_LNA_ISEXIST_2G,
    WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_2G,
    WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_2G,
    WLAN_CFG_INIT_RF_LNA_BYPASS_GAIN_DB_5G,
    WLAN_CFG_INIT_RF_LNA_GAIN_DB_5G,
    WLAN_CFG_INIT_RF_PA_GAIN_DB_B0_5G,
    WLAN_CFG_INIT_RF_PA_GAIN_DB_B1_5G,
    WLAN_CFG_INIT_RF_PA_GAIN_LVL_5G,
    WLAN_CFG_INIT_EXT_SWITCH_ISEXIST_5G,
    WLAN_CFG_INIT_EXT_PA_ISEXIST_5G,
    WLAN_CFG_INIT_EXT_LNA_ISEXIST_5G,
    WLAN_CFG_INIT_LNA_ON2OFF_TIME_NS_5G,
    WLAN_CFG_INIT_LNA_OFF2ON_TIME_NS_5G,
    /* SCAN */
    WLAN_CFG_INIT_RANDOM_MAC_ADDR_SCAN,
    /* 11AC2G */
    WLAN_CFG_INIT_11AC2G_ENABLE,
    /* 11ac2g开关 */                    // 56
    WLAN_CFG_INIT_DISABLE_CAPAB_2GHT40, /* 2ght40禁止开关 */
    WLAN_CFG_INIT_DUAL_ANTENNA_ENABLE,  /* 双天线开关 */

    /* probe req应答模式功能配置 */
    /* BIT7~BIT4:动态功能开关 ;BIT3~BIT0:功能模式(mac_probe_resp_mode_enum_uint8) , 目前此配置仅针对p2p dev设备有效 */
    WLAN_CFG_INIT_PROBE_RESP_MODE,
    WLAN_CFG_INIT_MIRACAST_SINK_ENABLE, /* miracast_sink特性开关 */
    WLAN_CFG_INIT_REDUCE_MIRACAST_LOG, /* miracast log 降级开关 */
    WLAN_CFG_INIT_BIND_LOWEST_LOAD_CPU, /* 是否支持绑定负载最轻的CPU */
    WLAN_CFG_INIT_CORE_BIND_CAP, /* 是否支持cpu绑定 */
    WLAN_CFG_INIT_FAST_MODE, /* BUCK stay in fastmode */

    /* sta keepalive */
    WLAN_CFG_INIT_STA_KEEPALIVE_CNT_TH, /* sta keepalive th */
    WLAN_CFG_INIT_FAR_DIST_POW_GAIN_SWITCH,
    WLAN_CFG_INIT_FAR_DIST_DSSS_SCALE_PROMOTE_SWITCH, /* 超远距11b 1m 2m dbb scale提升使能开关 */
    WLAN_CFG_INIT_CHANN_RADIO_CAP, /* 通道0、1 2g 5g频道能力 */

    WLAN_CFG_LTE_GPIO_CHECK_SWITCH, /* lte?????? */
    WLAN_ATCMDSRV_ISM_PRIORITY,
    WLAN_ATCMDSRV_LTE_RX,
    WLAN_ATCMDSRV_LTE_TX,
    WLAN_ATCMDSRV_LTE_INACT,
    WLAN_ATCMDSRV_ISM_RX_ACT,
    WLAN_ATCMDSRV_BANT_PRI,
    WLAN_ATCMDSRV_BANT_STATUS,
    WLAN_ATCMDSRV_WANT_PRI,
    WLAN_ATCMDSRV_WANT_STATUS,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_1,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_2,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_3,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_4,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_5,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_6,
    WLAN_TX5G_UPC_MIX_GAIN_CTRL_7,

    WLAN_TX2G_PA_GATE_VCTL_REG236,
    WLAN_TX2G_PA_GATE_VCTL_REG237,
    WLAN_TX2G_PA_GATE_VCTL_REG238,
    WLAN_TX2G_PA_GATE_VCTL_REG239,
    WLAN_TX2G_PA_GATE_VCTL_REG240,
    WLAN_TX2G_PA_GATE_VCTL_REG241,
    WLAN_TX2G_PA_GATE_VCTL_REG242,
    WLAN_TX2G_PA_GATE_VCTL_REG243,
    WLAN_TX2G_PA_GATE_VCTL_REG244,

    WLAN_TX2G_PA_VRECT_GATE_THIN_REG253,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG254,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG255,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG256,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG257,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG258,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG259,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG260,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG261,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG262,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG263,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG264,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG265,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG266,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG267,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG268,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG269,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG270,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG271,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG272,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG273,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG274,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG275,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG276,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG277,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG278,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG279,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND1,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG281,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG282,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG283,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG284,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND2,
    WLAN_TX2G_PA_VRECT_GATE_THIN_REG280_BAND3,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_2G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_20TH_5G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_40TH_5G,
    WLAN_CFG_INIT_DELTA_CCA_ED_HIGH_80TH_5G,
    WLAN_CFG_INIT_VOE_SWITCH,
    WLAN_CFG_INIT_11AX_SWITCH,
    WLAN_CFG_INIT_HTC_SWITCH,
    WLAN_CFG_INIT_MULTI_BSSID_SWITCH,

    /* ldac m2s rssi */
    WLAN_CFG_INIT_LDAC_THRESHOLD_M2S,
    WLAN_CFG_INIT_LDAC_THRESHOLD_S2M,

    /* btcoex mcm rssi */
    WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_DOWN,
    WLAN_CFG_INIT_BTCOEX_THRESHOLD_MCM_UP,

#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* 5g nr coex */
    WLAN_CFG_INIT_NRCOEX_ENABLE,
    WLAN_CFG_INIT_NRCOEX_VERSION,
    WLAN_CFG_INIT_NRCOEX_RULE0_FREQ,
    WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE0_40M_20M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE0_160M_80M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE0_SMALLGAP0_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE0_GAP01_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE0_GAP12_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE0_RXSLOT_RSSI,
    WLAN_CFG_INIT_NRCOEX_RULE1_FREQ,
    WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE1_40M_20M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE1_160M_80M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE1_SMALLGAP0_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE1_GAP01_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE1_GAP12_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE1_RXSLOT_RSSI,
    WLAN_CFG_INIT_NRCOEX_RULE2_FREQ,
    WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE2_40M_20M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE2_160M_80M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE2_SMALLGAP0_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE2_GAP01_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE2_GAP12_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE2_RXSLOT_RSSI,
    WLAN_CFG_INIT_NRCOEX_RULE3_FREQ,
    WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP0,
    WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP1,
    WLAN_CFG_INIT_NRCOEX_RULE3_40M_20M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE3_160M_80M_GAP2,
    WLAN_CFG_INIT_NRCOEX_RULE3_SMALLGAP0_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE3_GAP01_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE3_GAP12_ACT,
    WLAN_CFG_INIT_NRCOEX_RULE3_RXSLOT_RSSI,
#endif
    WLAN_CFG_INIT_DYNAMIC_DBAC_SWITCH,

#ifdef _PRE_WLAN_FEATURE_MBO
    WLAN_CFG_INIT_MBO_SWITCH,
#endif

    WLAN_CFG_INIT_DDR_FREQ,
    WLAN_CFG_INIT_HIEX_CAP,
    WLAN_CFG_INIT_FTM_CAP,
    /* 用于修正由于phy mode滤波器收窄导致的rssi偏差 */
    WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C0,
    WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_2G_C1,
    WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C0,
    WLAN_CFG_INIT_RF_FILTER_NARROW_RSSI_AMEND_5G_C1,

#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    /* 组播聚合参数配置 */
    WLAN_CFG_INIT_MCAST_AMPDU_ENABLE,
#endif
    WLAN_CFG_INIT_PT_MCAST_ENABLE,
    WLAN_CFG_INIT_FEM_POLAR_POLICY_2G,
    WLAN_CFG_INIT_FEM_POLAR_POLICY_5G,
    WLAN_CFG_INIT_DYN_BYPASS_2G_EXTLNA_RSSI_HIGH_TH,
    WLAN_CFG_INIT_DYN_BYPASS_2G_EXTLNA_RSSI_LOW_TH,
    WLAN_CFG_INIT_DYN_BYPASS_5G_EXTLNA_RSSI_HIGH_TH,
    WLAN_CFG_INIT_DYN_BYPASS_5G_EXTLNA_RSSI_LOW_TH,
    WLAN_CFG_INIT_REDUCE_PWR_2G_40MHZ_CHANNEL_BITMAP,
    WLAN_CFG_INIT_BUTT,
} wlan_cfg_init;

int32_t *hwifi_get_g_host_init_params(void);
wlan_cfg_cmd *hwifi_get_g_wifi_config_cmds(void);
void host_params_init_first(void);
int32_t hwifi_custom_adapt_device_ini_param(uint8_t *puc_data);

#endif // hisi_customize_config_cm_mp13.h