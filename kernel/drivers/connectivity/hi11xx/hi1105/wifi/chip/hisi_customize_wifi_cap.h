/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化头文件声明
 * 作者       :
 * 创建日期   : 2015年10月08日
 */

#ifndef HISI_CUSTOMIZE_WIFI_CAP_H
#define HISI_CUSTOMIZE_WIFI_CAP_H
#include "wlan_customize.h"
/* 特性功能定制化 CAP CONFIG ID */
typedef enum {
    /* 校准开关 */
    WLAN_CFG_CAP_CALI_POLICY,
    WLAN_CFG_CAP_CALI_MASK_2G,
    WLAN_CFG_CAP_CALI_MASK_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    WLAN_CFG_CAP_CALI_MASK_6G,
#endif
    WLAN_CFG_CAP_CALI_ONLINE_MASK,
    WLAN_CFG_CAP_CALI_DPD_MODULE_MASK,
    WLAN_CFG_CAP_CALI_DPD_CALI_LVL_NUM,
    WLAN_CFG_CAP_CALI_DATA_MASK,

    WLAN_CFG_CAP_BW_MAX_WITH,
    WLAN_CFG_CAP_LDPC_CODING,
    WLAN_CFG_CAP_RX_STBC,
    WLAN_CFG_CAP_TX_STBC,
    WLAN_CFG_CAP_SU_BFER,
    WLAN_CFG_CAP_SU_BFEE,
    WLAN_CFG_CAP_MU_BFER,
    WLAN_CFG_CAP_MU_BFEE,

    WLAN_CFG_CAP_HISI_PRIV_IE,
    WLAN_CFG_CAP_VOE_SWITCH,
    WLAN_CFG_CAP_11AX_SWITCH,
    WLAN_CFG_CAP_HTC_SWITCH,
    WLAN_CFG_CAP_MULTI_BSSID_SWITCH,
    WLAN_CFG_CAP_DYNAMIC_DBAC_SWITCH,
    WLAN_CFG_CAP_MCAST_AMPDU_ENABLE,
    WLAN_CFG_CAP_PT_MCAST_ENABLE,
    /* DBDC */
    WLAN_CFG_CAP_DBDC_RADIO,
    WLAN_CFG_CAP_DBDC,
    WLAN_CFG_CAP_FASTSCAN_SWITCH,
    WLAN_CFG_CAP_DEVICE_ACCESS_DDR,

    /* 天线切换功能 */
    WLAN_CFG_CAP_ANT_SWITCH,
    /* 国家码自学习开关 */
    WLAN_CFG_CAP_COUNRTYCODE_SELFSTUDY_CFG,
    WLAN_CFG_CAP_DEGRADATION_CAP,

    /* dyn_pcie_switch */
    WLAN_CFG_DYN_PCIE_SWITCH_CAP,
    /* MCM */
    WLAN_CFG_CAP_RADIO_CAP,
    WLAN_CFG_CAP_RADIO_CAP_6G,
    WLAN_CFG_CAP_MCM_CUSTOM_FUNCTION_MASK,
    WLAN_CFG_CAP_MCM_FUNCTION_MASK,

    /* linkloss门限固定开关 */
    WLAN_CFG_CAP_LINKLOSS_THRESHOLD_FIXED,

    /* 160M APUT使能 */
    WLAN_CFG_CAP_APUT_160M_ENABLE,
    /* 屏蔽硬件上报的雷达信号 */
    WLAN_CFG_CAP_RADAR_ISR_FORBID,

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    WLAN_CFG_CAP_TXOPPS_SWITCH,
#endif
    WLAN_CFG_CAP_OVER_TEMPER_PROTECT_THRESHOLD,
    WLAN_CFG_CAP_OVER_TEMP_PRO_ENABLE,
    WLAN_CFG_CAP_OVER_TEMP_PRO_REDUCE_PWR_ENABLE,
    WLAN_CFG_CAP_OVER_TEMP_PRO_SAFE_TH,
    WLAN_CFG_CAP_OVER_TEMP_PRO_OVER_TH,
    WLAN_CFG_CAP_OVER_TEMP_PRO_PA_OFF_TH,

    WLAN_CFG_CAP_ALG_SWITCH,

    WLAN_CAP_DSSS2OFDM_DBB_PWR_BO_VAL,
    WLAN_CFG_CAP_EVM_PLL_REG_FIX,

    /* M_BSSID */
    WLAN_CFG_CAP_AC_SUSPEND,

    /* 动态bypass外置LNA方案 */
    WLAN_CFG_CAP_DYN_BYPASS_EXTLNA,
    WLAN_CFG_CAP_CTRL_FRAME_TX_CHAIN,
    WLAN_CFG_CAP_CTRL_11B_DOUBLE_CHAIN_BO_POW,

    /* 非高优先级hcc流控类型 0:SDIO 1:GPIO */
    WLAN_CFG_CAP_HCC_FLOWCTRL_TYPE,
#ifdef _PRE_WLAN_FEATURE_MBO
    WLAN_CFG_CAP_MBO_SWITCH,
#endif
    /* DC流控特性定制化开关 */
    WLAN_CFG_CAP_DC_FLOWCTL_SWITCH,
    /* phy相关能力定制化 */
    WLAN_CFG_CAP_PHY_CAP_SWITCH,

    WLAN_CFG_CAP_HAL_PS_RSSI_PARAM,
    WLAN_CFG_CAP_HAL_PS_PPS_PARAM,
    WLAN_CFG_CAP_HIEX_CAP,

    /* FEM pow saving 50 */
    WLAN_CFG_CAP_FEM_DELT_POW,
    WLAN_CFG_CAP_FEM_ADJ_TPC_TBL_START_IDX,
    /*  低功耗定制化  */
    WLAN_CFG_CAP_POWERMGMT_SWITCH,
    /* 低功耗代理 */
    WLAN_CFG_INIT_LOWPOWER_AGENT_SWITCH,
    WLAN_CFG_CAP_MIN_FAST_PS_IDLE,  // How many 20ms
    WLAN_CFG_CAP_MAX_FAST_PS_IDLE,
    WLAN_CFG_CAP_AUTO_FAST_PS_THRESH_SCREENON,
    WLAN_CFG_CAP_AUTO_FAST_PS_THRESH_SCREENOFF,
    WLAN_CFG_CAP_CLOSE_FILTER_SWITCH,

    /* 可维可测 */
    WLAN_CFG_CAP_LOGLEVEL,
    /*  性能  */
    WLAN_CFG_CAP_USED_MEM_FOR_START,
    WLAN_CFG_CAP_USED_MEM_FOR_STOP,
    WLAN_CFG_CAP_SDIO_D2H_ASSEMBLE_COUNT,
    WLAN_CFG_CAP_SDIO_H2D_ASSEMBLE_COUNT,
    /* LINKLOSS */
    WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_BT,  // 13,这里不能直接赋值
    WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_DBAC,
    WLAN_CFG_CAP_LINK_LOSS_THRESHOLD_NORMAL,
    /* 自动调频 */
    WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_0,  // 16
    WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_1,
    WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_2,
    WLAN_CFG_CAP_PSS_THRESHOLD_LEVEL_3,
    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_0,
    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_1,
    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_2,
    WLAN_CFG_CAP_DEVICE_TYPE_LEVEL_3,
    /* btcoex mcm rssi */
    WLAN_CFG_CAP_BTCOEX_THRESHOLD_MCM_DOWN,
    WLAN_CFG_CAP_BTCOEX_THRESHOLD_MCM_UP,
    WLAN_CFG_CAP_FTM,
    WLAN_CFG_CAP_FTM_BOARD_DELAY,
    WLAN_CFG_CAP_FAST_MODE,
    WLAN_CFG_CAP_TRX_SWITCH,
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    /* 5g nr coex */
    WLAN_CFG_CAP_NRCOEX_ENABLE,
    WLAN_CFG_CAP_NRCOEX_VERSION,
    WLAN_CFG_CAP_NRCOEX_HW,
    WLAN_CFG_CAP_NRCOEX_ADVANCE_LOW_POWER,
    WLAN_CFG_CAP_NRCOEX_RULE0_FREQ,
    WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE0_40M_20M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE0_160M_80M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE0_SMALLGAP0_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE0_GAP01_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE0_GAP12_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE0_RXSLOT_RSSI,
    WLAN_CFG_CAP_NRCOEX_RULE1_FREQ,
    WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE1_40M_20M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE1_160M_80M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE1_SMALLGAP0_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE1_GAP01_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE1_GAP12_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE1_RXSLOT_RSSI,
    WLAN_CFG_CAP_NRCOEX_RULE2_FREQ,
    WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE2_40M_20M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE2_160M_80M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE2_SMALLGAP0_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE2_GAP01_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE2_GAP12_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE2_RXSLOT_RSSI,
    WLAN_CFG_CAP_NRCOEX_RULE3_FREQ,
    WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP0,
    WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP1,
    WLAN_CFG_CAP_NRCOEX_RULE3_40M_20M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE3_160M_80M_GAP2,
    WLAN_CFG_CAP_NRCOEX_RULE3_SMALLGAP0_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE3_GAP01_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE3_GAP12_ACT,
    WLAN_CFG_CAP_NRCOEX_RULE3_RXSLOT_RSSI,
#endif
    WLAN_CFG_CAP_PHY_SAMPLE_SWITCH,
#ifdef _PRE_WLAN_FEATURE_UWBCOEX
    WLAN_CFG_CAP_UWBCOEX_ENABLE,
#endif
    WLAN_CFG_CAP_CALI_FREQ_MASK,
    WLAN_CFG_CAP_FB_GAIN_DELTA_POW,
    WLAN_CFG_CAP_CALI_DPD_EN_START_MCS,
    WLAN_CFG_CAP_DPD_LVL_NUM,
    WLAN_CFG_CAP_LP_LOSS_2G,
    WLAN_CFG_CAP_LP_LOSS_5G,

    /* SAR滑窗定制化相关 */
    NVRAM_PARA_SAR_CNT_TIME_LV1,
    NVRAM_PARA_SAR_CNT_TIME_LV2,
    NVRAM_PARA_SAR_CNT_TIME_LV3,
    NVRAM_PARA_SAR_CNT_TIME_LV4,
    NVRAM_PARA_SAR_CNT_TIME_LV5,
    NVRAM_PARA_SAR_CNT_TIME_LV6,
    NVRAM_PARA_SAR_CNT_TIME_LV7,
    NVRAM_PARA_SAR_CNT_TIME_LV8,
    NVRAM_PARA_SAR_CNT_TIME_LV9,
    NVRAM_PARA_SAR_CNT_TIME_LV10,
    NVRAM_PARA_SAR_CNT_TIME_LV11,
    NVRAM_PARA_SAR_CNT_TIME_LV12,
    NVRAM_PARA_SAR_CNT_TIME_LV13,
    NVRAM_PARA_SAR_CNT_TIME_LV14,
    NVRAM_PARA_SAR_CNT_TIME_LV15,
    NVRAM_PARA_SAR_CNT_TIME_LV16,
    NVRAM_PARA_SAR_CNT_TIME_LV17,
    NVRAM_PARA_SAR_CNT_TIME_LV18,
    NVRAM_PARA_SAR_CNT_TIME_LV19,
    NVRAM_PARA_SAR_CNT_TIME_LV20,

    NVRAM_PARA_SAR_SLIDE_POW_AMEND_C0,
    NVRAM_PARA_SAR_SLIDE_POW_AMEND_C1,
    NVRAM_PARA_SAR_SLIDE_POW_AMEND_C2,
    NVRAM_PARA_SAR_SLIDE_POW_AMEND_C3,

    WLAN_CFG_CAP_SAR_SLIDE_WINDOW_EN,
    WLAN_CFG_CAP_FEM_POLAR,
    WLAN_CFG_CAP_GLE_ANT_USE,
    WLAN_CFG_CAP_LP_CAP,
    WLAN_CFG_CAP_FEM_DELT_POW_3A,
    WLAN_CFG_CAP_FEM_ADJ_TPC_TBL_START_IDX_3A,
    WLAN_CFG_FEM_MODE_3A,

    NVRAM_RU_2X2_MAX_POWER_2G_C0,
    NVRAM_RU_2X2_MAX_POWER_2G_C1,
    NVRAM_RU_2X2_MAX_POWER_2G_C2,
    NVRAM_RU_2X2_MAX_POWER_2G_C3,
    NVRAM_RU_2X2_MAX_POWER_5G_C0,
    NVRAM_RU_2X2_MAX_POWER_5G_C1,
    NVRAM_RU_2X2_MAX_POWER_5G_C2,
    NVRAM_RU_2X2_MAX_POWER_5G_C3,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_RU_2X2_MAX_POWER_6G_C0,
    NVRAM_RU_2X2_MAX_POWER_6G_C1,
    NVRAM_RU_2X2_MAX_POWER_6G_C2,
    NVRAM_RU_2X2_MAX_POWER_6G_C3,
#endif
    NVRAM_RU_3X3_MAX_POWER_2G_C0,
    NVRAM_RU_3X3_MAX_POWER_2G_C1,
    NVRAM_RU_3X3_MAX_POWER_2G_C2,
    NVRAM_RU_3X3_MAX_POWER_2G_C3,
    NVRAM_RU_3X3_MAX_POWER_5G_C0,
    NVRAM_RU_3X3_MAX_POWER_5G_C1,
    NVRAM_RU_3X3_MAX_POWER_5G_C2,
    NVRAM_RU_3X3_MAX_POWER_5G_C3,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_RU_3X3_MAX_POWER_6G_C0,
    NVRAM_RU_3X3_MAX_POWER_6G_C1,
    NVRAM_RU_3X3_MAX_POWER_6G_C2,
    NVRAM_RU_3X3_MAX_POWER_6G_C3,
#endif
    NVRAM_RU_4X4_MAX_POWER_2G_C0,
    NVRAM_RU_4X4_MAX_POWER_2G_C1,
    NVRAM_RU_4X4_MAX_POWER_2G_C2,
    NVRAM_RU_4X4_MAX_POWER_2G_C3,
    NVRAM_RU_4X4_MAX_POWER_5G_C0,
    NVRAM_RU_4X4_MAX_POWER_5G_C1,
    NVRAM_RU_4X4_MAX_POWER_5G_C2,
    NVRAM_RU_4X4_MAX_POWER_5G_C3,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_RU_4X4_MAX_POWER_6G_C0,
    NVRAM_RU_4X4_MAX_POWER_6G_C1,
    NVRAM_RU_4X4_MAX_POWER_6G_C2,
    NVRAM_RU_4X4_MAX_POWER_6G_C3,
#endif
    WLAN_CFG_CAP_DYN_BYPASS_2G_EXTLNA_TH_H,
    WLAN_CFG_CAP_DYN_BYPASS_2G_EXTLNA_TH_L,
    WLAN_CFG_CAP_DYN_BYPASS_5G_EXTLNA_TH_H,
    WLAN_CFG_CAP_DYN_BYPASS_5G_EXTLNA_TH_L,
    WLAN_CFG_POW_CALI_AMEND_C0,
    WLAN_CFG_POW_CALI_AMEND_C1,
    WLAN_CFG_POW_CALI_AMEND_C2,
    WLAN_CFG_POW_CALI_AMEND_C3,
    WLAN_CFG_CAP_BUTT,
} wlan_cap_cfg_enum;

typedef enum {
    CUST_MIRACAST_SINK_SWITCH = 0,
    CUST_MIRACAST_REDUCE_LOG_SWITCH = 1,
    CUST_CORE_BIND_SWITCH = 2,
    CUST_FEATURE_SWITCH_BUTT
} cust_hmac_feature_switch_enum;
/*  host侧定制化参数结构体  */
typedef struct {
    uint16_t throughput_irq_high;
    uint16_t throughput_irq_low;
    uint32_t irq_pps_high;
    uint32_t irq_pps_low;
    uint16_t lock_cpu_th_high;
    uint16_t lock_cpu_th_low;
    uint32_t dma_latency_val;
    oal_bool_enum_uint8 en_irq_affinity; /* 中断开关 */
} cust_freq_lock_ctl_stru;

typedef struct {
    /* 定制化硬件聚合是否生效,默认为软件聚合 */
    uint8_t ampdu_hw_en;
    uint16_t us_remain_hw_cnt;
    uint16_t throughput_high;
    uint16_t throughput_low;
} cust_tx_ampdu_hw_stru;

typedef struct {
    /* 定制化是否打开amsdu大包聚合 */
    uint8_t large_amsdu_en;
    /* 当前聚合是否为amsdu聚合 */
    uint16_t throughput_high;
    uint16_t throughput_low;
    uint16_t throughput_middle;
} cust_tx_large_amsdu_stru;

typedef struct {
    uint8_t host_buf_en;
    uint8_t device_buf_en;
    uint16_t throughput_high;
    uint16_t throughput_low;
    uint16_t throughput_high_40m;
    uint16_t throughput_low_40m;
    uint16_t throughput_high_80m;
    uint16_t throughput_low_80m;
    uint16_t throughput_high_160m;
    uint16_t throughput_low_160m;
    uint16_t buf_userctl_high;
    uint16_t buf_userctl_low;
    uint8_t  buf_userctl_test_en;
} cust_tcp_ack_buf_stru;

typedef struct {
    /* 定制化小包amsdu开关 */
    uint8_t en_switch;
    uint16_t throughput_high;
    uint16_t throughput_low;
    uint16_t pps_high;
    uint16_t pps_low;
} cust_small_amsdu_stru;
typedef struct {
    /* 定制化是否打开tcp ack过滤 */
    uint8_t filter_en;
    uint16_t throughput_high;
    uint16_t throughput_low;
} cust_tcp_ack_filt_stru;

typedef struct {
    uint8_t  switch_en;     /* 定制化根据吞吐动态bypass extLNA开关 */
    uint16_t throughput_high;
    uint16_t throughput_low;
} cust_rx_dyn_bypass_extlna_stru;

typedef struct {
    cust_freq_lock_ctl_stru freq_lock;
    cust_tx_ampdu_hw_stru tx_ampdu;
    cust_tx_large_amsdu_stru large_amsdu;     /* 定制化amsdu大包聚合相关参数 */
    cust_small_amsdu_stru small_amsdu;   /* 定制化小包amsdu参数 */

    uint8_t host_rx_ampdu_amsdu;
    oal_bool_enum_uint8 wlan_11ac2g_switch;
    oal_bool_enum_uint8 wlan_probe_resp_mode;
    uint8_t wlan_ps_mode;

    oal_bool_enum_uint8 en_hmac_feature_switch[CUST_FEATURE_SWITCH_BUTT];
    /* addba rsp回复的聚合BAW SIZE */
    uint16_t rx_buffer_size;

    cust_tcp_ack_buf_stru tcp_ack_buf;
    cust_tcp_ack_filt_stru tcp_ack_filt;
    cust_rx_dyn_bypass_extlna_stru dyn_extlna;

    oal_bool_enum_uint8 lock_max_cpu_freq;
    unsigned char roam_switch;
    uint16_t rsv;
    uint32_t reduce_pwr_2g_40mhz_channel_bitmap;
} cust_host_cfg_stru;

#endif  // hisi_customize_wifi_cap.h
