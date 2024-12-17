/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : mp16 wlan定制化头文件
 * 作    者 : wifi
 * 创建日期 : 2022年10月11日
 */

#ifndef WLAN_CUSTOMIZE_MP16_H
#define WLAN_CUSTOMIZE_MP16_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_customize.h"
#include "hal_common.h"

/* 3 枚举定义 */
/* RF通道枚举 */
typedef enum {
    WLAN_SINGLE_STREAM_0 = 0,  /* C0 */
    WLAN_SINGLE_STREAM_1 = 1,  /* C1 */
    WLAN_SINGLE_STREAM_2 = 2,  /* C2 */
    WLAN_SINGLE_STREAM_3 = 3,  /* C3 */
    WLAN_STREAM_TYPE_BUTT = 4, /* 代表无效的通道类型，规格宏请采用WLAN_RF_STREAM_NUMS */
} wlan_stream_type_enum;
typedef uint8_t wlan_stream_type_enum_uint8;

typedef enum {
    WLAN_ABB_CALI                = 0,
    WLAN_RF_CALI_ABBRC           = 1,
    WLAN_RF_CALI_RC              = 2,
    WLAN_RF_CALI_R               = 3,
    WLAN_RF_CALI_C               = 4,
    WLAN_RF_CALI_PPF             = 5,
    WLAN_RF_CALI_LOGEN_SSB       = 6,
    WLAN_RF_CALI_LO_BF           = 7,
    WLAN_RF_CALI_LO_INVOLT_SWING = 8,
    WLAN_RF_CALI_PA_CURRENT      = 9,
    WLAN_RF_CALI_LNA_BLK_DCOC    = 10,
    WLAN_RF_CALI_RX_DC           = 11,
    WLAN_RF_CALI_MIMO_RX_DC      = 12,
    WLAN_RF_CALI_DPD_RX_DC       = 13,
    WLAN_RF_CALI_TX_PWR          = 14,
    WLAN_RF_CALI_TX_LO           = 15,
    WLAN_RF_CALI_MIMO_TX_LO      = 16,
    WLAN_RF_CALI_TX_IQ           = 17,
    WLAN_RF_CALI_MIMO_TX_IQ      = 18,
    WLAN_RF_CALI_RX_IQ           = 19,
    WLAN_RF_CALI_MIMO_RX_IQ      = 20,
    WLAN_RF_CALI_IIP2            = 21,
    WLAN_RF_CALI_DPD             = 22,
#ifdef _PRE_WLAN_FEATURE_LP
    WLAN_RF_CALI_LP_RX_DC        = 23,
#else
    WLAN_RF_CALI_DFS_RX_DC       = 23,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM_CALI
    WLAN_RF_CALI_FTM             = 24,
#endif
    WLAN_RF_CALI_TYPE_BUTT,

    WLAN_RF_CALI_CHANNEL_START    = 31,
    WLAN_RF_CALI_CHANNEL_BUTT
} wlan_rf_cali_type_enum;
typedef uint8_t wlan_rf_cali_type_enum_uint8;

/* 校准数据上传枚举变量 */
typedef enum {
    WLAN_CALI_DATA_C0_2G_20M = 0,
    WLAN_CALI_DATA_C0_2G_40M = 1,
    WLAN_CALI_DATA_C0_5G_20M = 2,
    WLAN_CALI_DATA_C0_5G_40M = 3,
    WLAN_CALI_DATA_C0_5G_80M = 4,
    WLAN_CALI_DATA_C0_5G_160M = 5,
    WLAN_CALI_DATA_C0_6G_20M = 6,
    WLAN_CALI_DATA_C0_6G_40M = 7,
    WLAN_CALI_DATA_C0_6G_80M = 8,
    WLAN_CALI_DATA_C0_6G_160M = 9,
    WLAN_CALI_DATA_C0_BUTT = 10,

    WLAN_CALI_DATA_C1_2G_20M = WLAN_CALI_DATA_C0_BUTT,
    WLAN_CALI_DATA_C1_2G_40M = 11,
    WLAN_CALI_DATA_C1_5G_20M = 12,
    WLAN_CALI_DATA_C1_5G_40M = 13,
    WLAN_CALI_DATA_C1_5G_80M = 14,
    WLAN_CALI_DATA_C1_5G_160M = 15,
    WLAN_CALI_DATA_C1_6G_20M = 16,
    WLAN_CALI_DATA_C1_6G_40M = 17,
    WLAN_CALI_DATA_C1_6G_80M = 18,
    WLAN_CALI_DATA_C1_6G_160M = 19,

    WLAN_CALI_DATA_C2_2G_20M = 20,
    WLAN_CALI_DATA_C2_2G_40M = 12,
    WLAN_CALI_DATA_C2_5G_20M = 22,
    WLAN_CALI_DATA_C2_5G_40M = 23,
    WLAN_CALI_DATA_C2_5G_80M = 24,
    WLAN_CALI_DATA_C2_5G_160M = 25,
    WLAN_CALI_DATA_C2_6G_20M = 26,
    WLAN_CALI_DATA_C2_6G_40M = 27,
    WLAN_CALI_DATA_C2_6G_80M = 28,
    WLAN_CALI_DATA_C2_6G_160M = 29,

    WLAN_CALI_DATA_C3_2G_20M = 30,
    WLAN_CALI_DATA_C3_2G_40M = 31,
    WLAN_CALI_DATA_C3_5G_20M = 32,
    WLAN_CALI_DATA_C3_5G_40M = 33,
    WLAN_CALI_DATA_C3_5G_80M = 34,
    WLAN_CALI_DATA_C3_5G_160M = 35,
    WLAN_CALI_DATA_C3_6G_20M = 36,
    WLAN_CALI_DATA_C3_6G_40M = 37,
    WLAN_CALI_DATA_C3_6G_80M = 38,
    WLAN_CALI_DATA_C3_6G_160M = 39,
    WLAN_CALI_DATA_DBDC = 40,
    WLAN_CALI_DATA_COMMON = 41,
    WLAN_CALI_SDIO_NULL_EVENT = 42,
    WLAN_D2H_CALI_DATA_TYPE_BUTT
} wlan_d2h_cali_data_type_enum;
typedef uint8_t wlan_d2h_cali_data_type_enum_uint8;

typedef enum {
    WLAN_CALI_MULT_BW_2G_20M_5G_160M = 0,
    WLAN_CALI_MULT_BW_5G_20M = 1,
    WLAN_CALI_MULT_BW_5G_80M = 2,
    WLAN_CALI_MULT_BW_6G_20M = 3,
    WLAN_CALI_MULT_BW_6G_80M = 4,
    WLAN_CALI_MULT_BW_6G_160M = 5,

    WLAN_CALI_MULT_BW_BUTT
} wlan_cali_mult_bw_type_enum;

typedef enum {
    WLAN_ONLINE_RXDC_CALI   = 0,
    WLAN_ONLINE_PWR_CALI    = 1,
    WLAN_ONLINE_TXIQ_CALI   = 2,
    WLAN_ONLINE_RXIQ_CALI   = 3,
    WLAN_ONLINE_DPD_CALI    = 4,
    WLAN_ONLINE_ITR_FRM_DC_CALI   = 5,
    WLAN_ONLINE_TXDC_CALI   = 6,

    WLAN_RF_ONLINE_CALI_TYPE_BUTT,
} wlan_rf_online_cali_type_enum;

typedef enum {
    WLAN_VMIN_ABNROMAL_TYPE_PLL_REG_WRITE = 0,
    WLAN_VMIN_ABNROMAL_TYPE_PHY_REG_WRITE = 1,

    WLAN_VMIN_ABNROMAL_TYPE_BUTT,
} wlan_vmin_abnormal_type_enum;

typedef enum {
    WLAN_CURRENT_WORK_CHN = 0,    /* 主路当前信道 */
    WLAN_SCAN_ALL_CHN = 1,   /* 20M扫描与通道有关的部分 */
    WLAN_CALI_CTL = 2,    /* 20M扫描与通道无关的部分 */
    WLAN_SCAN_ALL_CHN_SLAVE = 3,   /* 辅路20M扫描与通道有关的部分 */

    WLAN_H2D_CALI_DATA_TYPE_BUTT = WLAN_SCAN_ALL_CHN_SLAVE,   /* 此枚举废弃，rom化原因无法删除 */
    WLAN_H2D_CALI_DATA_TYPE_EXTEND_BUTT
} wlan_h2d_cali_data_type_enum;
typedef uint8_t wlan_h2d_cali_data_type_enum_uint8;
/* 多次校准是否开启标志 */
#define WLAN_MUL_CHAIN_CALI_MASK 0x4
#define WLAN_MUL_BW_CALI_MASK 0x8
/* 是否使用elna检测结果标记 */
#define WLAN_USE_ELNA_DET_RESULT_MASK 0x1
/* 分通道校准次数 */
#define WLAN_MUL_CHAIN_CALI_TIMES 3
/* 分带宽校准次数 */
#define WLAN_MUL_BW_CALI_TIMES 3
#define WLAN_MUL_BW_CALI_TIMES_6G 6
/* fem pa状态 */
#define WLAN_FEM_PA_STATUS_OFFSET 0
#define WLAN_FEM_PA_STATUS_MASK 0xF
/* LNA GAIN检测状态 */
#define WLAN_LNA_GAIN_STATUS_OFFSET 4
#define WLAN_LNA_GAIN_STATUS_MASK 0xF0
/* ELNA GAIN检测状态 */
#define WLAN_ELNA_GAIN_STATUS_OFFSET 8
#define WLAN_ELNA_GAIN_STATUS_MASK 0xF00
/* 支持切换TAS的天线索引 */
#define WLAN_RF_TAS_ANT_IDX 0
typedef enum {
    WLAN_HW_STATUS_CHECK_FEM_PA = 0,
    WLAN_HW_STATUS_CHECK_LNA = 1,
    WLAN_HW_STATUS_CHECK_ELNA = 2,

    WLAN_HW_STATUS_CHECK_TYPE_BUTT,
} wlan_cali_hw_status_check_type_enum;

typedef enum {
    WLAN_CALI_FAIL_RXDC = 0,
    WLAN_CALI_FAIL_DPD = 1,

    WLAN_CALI_FAIL_TYPE_BUTT,
} wlan_cali_fail_type_enum;

/* 定制化 power ref 2g 5g 6g配置参数 */
/* customize rf front cfg struct */
typedef struct {
    /* 2g */
    int8_t gain_db_2g[MAC_NUM_2G_BAND][HD_EVENT_RF_NUM];
    /* 5g */
    int8_t gain_db_5g[MAC_NUM_5G_BAND][HD_EVENT_RF_NUM];
    /* 6g */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t gain_db_6g[MAC_NUM_6G_BAND][HD_EVENT_RF_NUM];
#endif
} mac_cust_rf_loss_gain_db_stru;

typedef struct {
    int8_t delta_rssi_2g[CUS_NUM_2G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
    int8_t delta_rssi_5g[CUS_NUM_5G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t delta_rssi_6g[CUS_NUM_5G_DELTA_RSSI_NUM][HD_EVENT_RF_NUM];
#endif
} mac_cus_rf_delta_pwr_ref_stru;

typedef struct {
    int8_t amend_rssi_2g[CUS_NUM_2G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
    int8_t amend_rssi_5g[CUS_NUM_5G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t amend_rssi_6g[CUS_NUM_5G_AMEND_RSSI_NUM][HD_EVENT_RF_NUM];
#endif
} mac_custom_rf_amend_rssi_stru;

typedef struct {
    uint16_t lna_on2off_time_ns[HD_EVENT_RF_NUM]; /* LNA开到LNA关的时间(ns) */
    uint16_t lna_off2on_time_ns[HD_EVENT_RF_NUM]; /* LNA关到LNA开的时间(ns) */

    int8_t lna_bypass_gain_db[HD_EVENT_RF_NUM];   /* 外部LNA bypass时的增益(dB) */
    int8_t lna_gain_db[HD_EVENT_RF_NUM];          /* 外部LNA增益(dB) */
    int8_t pa_gain_b0_db[HD_EVENT_RF_NUM];        /* 外部PA b0 增益(dB) */
    uint8_t ext_switch_isexist[HD_EVENT_RF_NUM]; /* 是否使用外部switch */

    uint8_t ext_pa_isexist[HD_EVENT_RF_NUM];                  /* 是否使用外部pa */
    oal_fem_lp_state_enum_uint8 en_fem_lp_enable[HD_EVENT_RF_NUM]; /* 是否支持fem低功耗标志 */
    int8_t c_fem_spec_value[HD_EVENT_RF_NUM];                    /* fem spec功率点 */
    uint8_t ext_lna_isexist[HD_EVENT_RF_NUM];                 /* 是否使用外部lna */

    int8_t pa_gain_b1_db[HD_EVENT_RF_NUM];     /* 外部PA b1增益(dB) */
    uint8_t pa_gain_lvl_num[HD_EVENT_RF_NUM]; /* 外部PA 增益档位数 */
    uint16_t fem_mode; /* FEM控制真值表 */
    uint8_t resv[2];
} mac_custom_ext_rf_stru;

typedef struct {
    mac_cust_rf_loss_gain_db_stru rf_loss_gain_db;           /* 2.4g 5g 插损 */
    mac_custom_ext_rf_stru ext_rf[WLAN_CALI_BAND_BUTT];   /* 2.4g 5g fem */
    mac_cus_rf_delta_pwr_ref_stru delta_pwr_ref;         /* delta_rssi */
    mac_custom_rf_amend_rssi_stru rssi_amend_cfg;       /* rssi_amend */

    /* 注意，如果修改了对应的位置，需要同步修改函数: hal_config_custom_rf  */
    int8_t delta_cca_ed_high_20th_2g;
    int8_t delta_cca_ed_high_40th_2g;
    int8_t delta_cca_ed_high_20th_5g;
    int8_t delta_cca_ed_high_40th_5g;

    int8_t delta_cca_ed_high_80th_5g;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    int8_t delta_cca_ed_high_20th_6g;
    int8_t delta_cca_ed_high_40th_6g;
    int8_t delta_cca_ed_high_80th_6g;
#else
    int8_t cca_resv[3]; /* 保留字段:3字节 */
#endif
    uint8_t far_dist_pow_gain_switch;           /* 超远距离功率增益开关 */
    int8_t resv[3];
} mac_customize_rf_front_sru;

typedef struct {
    hal_pwr_fit_para_stru pa2gccka_para[HD_EVENT_RF_NUM];
    hal_pwr_fit_para_stru pa2ga_para[HD_EVENT_RF_NUM];
    hal_pwr_fit_para_stru pa2g40a_para[HD_EVENT_RF_NUM];

    hal_pwr_fit_para_stru pa5ga_para[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
    hal_pwr_fit_para_stru pa5ga_low_para[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
    /* 6G定制化band过多，每个band的4个通道数据在同一项定制化中 */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    hal_pwr_fit_para_stru pa6ga_para[CUS_6G_BASE_PWR_NUM][HD_EVENT_RF_NUM];
#endif
    cus_dy_cali_dpn_stru dpn_para[HAL_CUS_FB_LVL_NUM][HD_EVENT_RF_NUM];
    int16_t extre_point_val[CUS_DY_CALI_NUM_5G_BAND];
    int8_t rsv[2];    /* 保留字段:2字节 */
}mac_cust_dyn_pow_sru;

/* 4字节对齐 */
typedef struct {
    int8_t delt_txpwr_params[CUST_NORMAL_DELTA_POW_RATE_ALGN_BUTT];
    uint8_t upper_upc_5g_params[HD_EVENT_RF_NUM];
    uint8_t backoff_pow_5g_params[HD_EVENT_RF_NUM];
    uint8_t dsss_low_pow_amend_2g[HD_EVENT_RF_NUM];
    uint8_t ofdm_low_pow_amend_2g[HD_EVENT_RF_NUM];

    uint8_t txpwr_base_2g_params[HD_EVENT_RF_NUM][CUS_2G_BASE_PWR_NUM];
    uint8_t txpwr_base_5g_params[HD_EVENT_RF_NUM][CUS_5G_BASE_PWR_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t txpwr_base_6g_params[HD_EVENT_RF_NUM][CUS_6G_BASE_PWR_NUM];
#endif
    int8_t delt_txpwr_base_params_2[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM]; /* 2*2MIMO相对SISO的base power差值 */
    int8_t delt_txpwr_base_params_3[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM]; /* 3*3MIMO相对SISO的base power差值 */
    int8_t delt_txpwr_base_params_4[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM]; /* 4*4MIMO相对SISO的base power差值 */
    /* FCC/CE边带功率定制项 */
    hal_cfg_custom_fcc_txpwr_limit_stru fcc_ce_param[HD_EVENT_RF_NUM];
    /* SAR CTRL */
    wlan_cust_sar_cfg_stru sar_ctrl_params[CUS_SAR_LVL_NUM];
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    /* TAS CTRL */
    uint8_t tas_ctrl_params[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM];
#endif
    wlan_cust_2g_ru_pow_stru  full_bw_ru_pow_2g[WLAN_BW_CAP_80M];
    wlan_cust_5g_ru_pow_stru  full_bw_ru_pow_5g[WLAN_BW_CAP_BUTT];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cust_6g_ru_pow_stru  full_bw_ru_pow_6g[WLAN_BW_CAP_BUTT];
#endif
    wlan_cust_2g_tpc_ru_pow_stru tpc_tb_ru_2g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
    wlan_cust_5g_tpc_ru_pow_stru tpc_tb_ru_5g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cust_6g_tpc_ru_pow_stru tpc_tb_ru_6g_max_pow[HAL_POW_RF_SEL_CHAIN_BUTT];
#endif
    uint8_t tpc_tb_ru_max_power[WLAN_CALI_BAND_BUTT];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t rsv[1];    /* 保留字段:1字节 */
#else
    uint8_t rsv[2];    /* 保留字段:2字节 */
#endif
    uint32_t fcc_ce_max_pwr_for_5g_high_band;
}wlan_cust_pow_stru;

typedef struct {
    wlan_stream_type_enum_uint8 stream_type;
    wlan_cali_band_enum_uint8 wlan_band;
    wlan_bandwidth_type_enum_uint8 band_width;
    oal_bool_enum_uint8 is_master;
    wlan_bus_type_enum_uint8 bus_type;
    wlan_d2h_cali_data_type_enum_uint8 data_type;
    uint8_t cali_run_times;
    uint8_t resv;
}wlan_rf_cali_cfg_stru;
typedef struct {
    uint16_t size;
    uint8_t version;
    uint8_t compete_enable;
    /*
        NVRAM_PARAM0_OFFSET
        ~
        NVRAM_PARAM20_OFFSET
    */
    int8_t delt_txpwr_params[CUST_NORMAL_DELTA_POW_RATE_ALGN_BUTT];
} wlan_compete_mode_stru;

typedef struct {
    uint8_t band;
    uint8_t resv;
    uint16_t center_freq;
    uint32_t duration; /* remain_on_channel时长 */
} wlan_remain_on_channel_info_stru;

#endif /* end of wlan_customize_mp16.h */
