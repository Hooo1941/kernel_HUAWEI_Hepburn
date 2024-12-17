/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hal_cali.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2019年2月15日
 */

#ifndef WLAN_CALI_MP16_H
#define WLAN_CALI_MP16_H

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "wlan_customize.h"
#include "hal_common.h"
#include "wlan_customize_mp16.h"

/* 2 宏定义 */
/* 芯片配置寄存器数，宏不可修改 */
/* TRX IQ lut表RAM_SIZE: 260(depth)*32(width) */
#define HAL_PHY_TXDC_COMP_LVL_NUM                4   /* tx dc补偿寄存器档位数 */
#define HAL_PHY_TXIQ_COMP_LVL_NUM                4   /* tx iq补偿寄存器档位数目 */
#define HAL_PHY_RXIQ_COMP_LVL_NUM                8   /* rx iq补偿寄存器档位数目 */
#define HAL_CALI_IQ_QMC_QDC_COEF_NUM             10  /* 9阶系数 + 1个DC */

/* rx dc补偿值档位数目 */
#define HAL_CALI_RXDC_GAIN_LVL_NUM      8

#define WLAN_TX_DC_CALI_LVL                   4     /* tx dc补偿值档位数目 */
#define WLAN_TX_IQ_CALI_LVL                   2     /* tx iq补偿值档位数目 */

#define WLAN_RX_IQ_CALI_LVL                   4     /* rx iq补偿值档位数目 */

#define WLAN_RX_DC_LNA_LVL                    8

#define WLAN_LP_DC_LNA_LVL                    1

#define WLAN_IQ_QMC_QDC_COEF_NUM      10  /* 9阶系数 + 1个DC */

#define WLAN_DPD_EVEN_LUT_LENGTH      48
#define WLAN_DPD_ODD_LUT_LENGTH       48
#define WLAN_DPD_GLUE_LUT_LENGTH      45
#define WLAN_DPD_TPC_LEVEL_NUM        4
#define WLAN_DPD_RX_DC_FB_LVL         4

/* ABB CALI相关宏定义 */
#define WLAN_ABB_CALI_SUB_ADC_NUM_PER_IQ_SUB_CHN    4   /* 每一I路或者Q路SUB ADC的数目 */
/* 每个接收通道的SUB ADC数目,IQ两路和 */
#define WLAN_ABB_CALI_SUB_ADC_NUM_PER_CHN           (WLAN_ABB_CALI_SUB_ADC_NUM_PER_IQ_SUB_CHN * 2)
#define WLAN_ABB_CALI_CAP_WT_COMP_LEN               56
#define WLAN_ABB_CALI_DITHER_WT_COMP_LEN            12
#define WLAN_ABB_CALI_INTER_ADC_GAIN_COMP_LEN       3
#define WLAN_ABB_CALI_INTER_ADC_OFFSET_COMP_LEN     4
#define WLAN_ABB_CALI_INTER_ADC_SKEW_COMP_LEN       2
#define WLAN_ABB_CALI_DAC_HIGH_LUT_LEN              32
#define WLAN_ABB_CALI_DAC_MID_LUT_LEN               16

/* 2G/5G RF划分子频段数目 */
#define WLAN_2G_RF_BAND_NUM      3
/* 2G/5G/6G 软件定义校准子频段数目 */
#define WLAN_2G_CALI_BAND_NUM    3
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
#define WLAN_5G_RF_BAND_NUM      15
#else
#define WLAN_5G_RF_BAND_NUM      7
#endif
#define WLAN_5G_20M_CALI_BAND_NUM  7
#define WLAN_5G_160M_CALI_BAND_NUM  2
#define WLAN_5G_CALI_SUB_BAND_NUM (WLAN_5G_20M_CALI_BAND_NUM + WLAN_5G_160M_CALI_BAND_NUM)
#define WLAN_6G_20M_CALI_BAND_NUM  8
#define WLAN_6G_160M_CALI_BAND_NUM  7
#define WLAN_6G_CALI_SUB_BAND_NUM (WLAN_6G_20M_CALI_BAND_NUM + WLAN_6G_160M_CALI_BAND_NUM)

/* 各带宽是否启动校准标志, bit3:0 160M|80M|40M|20M */
#define WLAN_RF_CALI_BW_MASK   0xD
/* LOGEN校准MIMO模式下通道数目 */
#define WLAN_MIMO_LOGEN_CALI_CHAIN_NUM 2

static inline oal_bool_enum_uint8 wlan_cali_in_valid_range(uint16_t value, uint16_t start, uint16_t end)
{
    return (((value) >= (start)) && ((value) <= (end)));
}
#ifndef WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG
#define WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG 0x40100114
#endif
#define WLAN_CALI_WCTL_ADDR_REG WCPU_CTL_RB_WCPU_CTL_GP_REG3_REG

#define WLAN_CALI_DPD_CORRAM_DATA_64BITS_LEN    6144
#define WLAN_CALI_DPD_CORRAM_DATA_128BITS_LEN    3072

#define CALI_DATA_NETBUF_SEND_LEN 1400
/* 3 枚举定义 */
/* 4 全局变量声明 */
/* 5 消息头定义 */
/* 6 消息定义 */
/* 7 STRUCT定义 */
typedef struct {
    uint16_t us_cali_time;
    uint16_t bit_temperature : 3,
               uc_5g_chan_idx1 : 5,
               uc_5g_chan_idx2 : 5,
               bit_rev : 3;
} mp16_update_cali_channel_stru;

struct mp16_cali_param_tag {
    uint32_t dog_tag;
    mp16_update_cali_channel_stru st_cali_update_info;
    uint32_t check_hw_status;
    oal_bool_enum_uint8 en_save_all;
    uint8_t uc_last_cali_fail_status;
    uint8_t uc_cur_cali_policy;
    oal_bool_enum_uint8 en_need_close_fem_cali;
};
typedef struct mp16_cali_param_tag mp16_cali_param_stru;

typedef struct {
    uint32_t inner_adc_offset_code_i;
    uint32_t inner_adc_offset_code_q;
    uint32_t inner_adc_cap_wt[WLAN_ABB_CALI_CAP_WT_COMP_LEN];
    uint32_t inner_adc_dither_wt[WLAN_ABB_CALI_DITHER_WT_COMP_LEN];
    uint32_t inter_adc_gain[WLAN_ABB_CALI_INTER_ADC_GAIN_COMP_LEN];
    uint32_t inter_adc_offset_code[WLAN_ABB_CALI_INTER_ADC_OFFSET_COMP_LEN];
    uint32_t inter_adc_skew_code[WLAN_ABB_CALI_INTER_ADC_SKEW_COMP_LEN];
    uint32_t dac_high_lut_coeff[WLAN_ABB_CALI_DAC_HIGH_LUT_LEN];
    uint32_t dac_mid_lut_coeff[WLAN_ABB_CALI_DAC_MID_LUT_LEN];
} wlan_cali_abb_para_stru;

typedef struct {
    uint16_t rc_cmp_code;
    uint16_t r_cmp_code;
    uint16_t c_cmp_code;
    uint16_t rc_20m_cmp_code;
    uint16_t abbrc_cmp_code;
    uint8_t elna_abnormal;
    uint8_t  resv;
} wlan_cali_rc_r_c_para_stru;

typedef struct {
    uint8_t ssb_ppf_code;
    uint8_t trx_ppf_code;   /* mp16 rf不使用,共平台5x需要 */
    uint8_t resv[2];  /* 对齐 2 */
} wlan_cali_ppf_para_stru;

typedef struct {
    uint8_t lo_ssb_tune;
    uint8_t lo_vb_tune;
    uint8_t lo_buf_tune[WLAN_MIMO_LOGEN_CALI_CHAIN_NUM];
} wlan_cali_logen_para_stru;

typedef struct {
    uint8_t analog_lp_rxdc_cmp_i[WLAN_LP_DC_LNA_LVL];
    uint8_t analog_lp_rxdc_cmp_q[WLAN_LP_DC_LNA_LVL];
    uint16_t digital_lp_rxdc_cmp_i;
    uint16_t digital_lp_rxdc_cmp_q;
    uint8_t resv[2];
} wlan_cali_lp_rxdc_para_stru;

typedef struct {
    uint8_t tx_lo;
    uint8_t tx_dpd_lo_tx;
    uint8_t tx_dpd_lo_dpd;
    uint8_t rx_lo;
} wlan_cali_lodiv_para_stru;

typedef struct {
    uint16_t fine_code;
    uint8_t resv[2];  /* 对齐 2 */
} wlan_cali_lna_blk_para_stru;

typedef struct {
    uint8_t  uc_vco_ldo_index;
    uint8_t  uc_trim_buf2lo_code;
    uint8_t  auc_resev[2];  /* 对齐 2 */
}wlan_cali_involt_swing_para_stru;

typedef struct {
    uint8_t code_i;
    uint8_t code_q;
    uint8_t resv[2];  /* 对齐 2 */
} wlan_cali_iip2_para_stru;

typedef struct {
    // RX DC
    uint8_t analog_rxdc_cmp_intlna_i[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_intlna_q[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_extlna_i[WLAN_RX_DC_LNA_LVL];
    uint8_t analog_rxdc_cmp_extlna_q[WLAN_RX_DC_LNA_LVL];
    uint16_t digital_rxdc_cmp_i;
    uint16_t digital_rxdc_cmp_q;
} wlan_cali_rxdc_para_stru;

typedef struct {
    uint8_t analog_dpd_rxdc_cmp_i[WLAN_DPD_RX_DC_FB_LVL];
    uint8_t analog_dpd_rxdc_cmp_q[WLAN_DPD_RX_DC_FB_LVL];
    uint16_t digital_dpd_rxdc_cmp_i;
    uint16_t digital_dpd_rxdc_cmp_q;
    int16_t cali_temperature;
    uint16_t resv;
} wlan_cali_dpd_rxdc_para_stru;

typedef struct {
    uint16_t  ppa_cmp;
    uint16_t  ppa_cmp_online;
    uint16_t  mx_cap_cmp;
    uint16_t  ppa_cap_cmp;
    uint16_t  pa_cap_cmp;
    uint16_t  resv;
} wlan_cali_txpwr_para_stru;

typedef struct {
    uint16_t  txdc_cmp_i[WLAN_TX_DC_CALI_LVL];
    uint16_t  txdc_cmp_q[WLAN_TX_DC_CALI_LVL];
} wlan_cali_txdc_para_stru;

typedef struct {
    uint32_t coef[WLAN_IQ_QMC_QDC_COEF_NUM];
} wlan_cali_iq_coef_stru;

typedef struct {
    uint8_t vga_code;
    uint8_t att_code;
    uint8_t reserved[2];  /* 2, 对齐 */
} wlan_cali_iq_gain_cfg_stru;

typedef struct {
    wlan_cali_iq_coef_stru qmc_siso;
    /* 保存offline txiq校准调整的增益配置, 用于online iq校准 */
    wlan_cali_iq_gain_cfg_stru gain_cfg;
} wlan_cali_txiq_para_per_lvl_stru;

typedef struct {
    wlan_cali_txiq_para_per_lvl_stru qmc_data[WLAN_TX_IQ_CALI_LVL];
    wlan_cali_iq_coef_stru qmc_fb;
} wlan_cali_txiq_para_per_freq_stru;

typedef struct {
    wlan_cali_iq_coef_stru qdc_siso[WLAN_RX_IQ_CALI_LVL];
    wlan_cali_iq_coef_stru qdc_mimo[WLAN_RX_IQ_CALI_LVL];
    wlan_cali_iq_coef_stru qdc_tx;
} wlan_cali_rxiq_para_per_freq_stru;

typedef struct {
    uint32_t dpd_even_lut[WLAN_DPD_EVEN_LUT_LENGTH];
    uint32_t dpd_odd_lut[WLAN_DPD_ODD_LUT_LENGTH];
    uint32_t dpd_glue_lut[WLAN_DPD_GLUE_LUT_LENGTH];
    uint8_t vga_gain;
    uint8_t resv[3];  /* 对齐 3 */
} wlan_cali_dpd_lut_stru;

typedef struct {
    wlan_cali_dpd_lut_stru dpd_cali_para[WLAN_DPD_TPC_LEVEL_NUM];
} wlan_cali_dpd_para_stru;

typedef struct {
    uint16_t center_freq_start;
    uint16_t center_freq_end;
} wlan_cali_div_band_stru;

/* 单个带宽单个信道基准校准数据结构体 */
typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_rxdc_para_stru  rxdc_mimo_cali_data;
    wlan_cali_dpd_rxdc_para_stru  dpd_rxdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_mimo_cali_data;
    wlan_cali_txpwr_para_stru txpwr_cali_data;
    wlan_cali_txiq_para_per_freq_stru txiq_cali_data;
    wlan_cali_rxiq_para_per_freq_stru rxiq_cali_data;
    wlan_cali_iip2_para_stru iip2_cali_data;
#ifndef _PRE_WLAN_ATE
    wlan_cali_dpd_para_stru  dpd_cali_data;
#endif
} wlan_cali_basic_para_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_20m_save_stru;
typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_40m_save_stru;

typedef struct {
    uint8_t pa_current_data[WLAN_5G_20M_CALI_BAND_NUM];
    uint8_t resv;
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_20m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_40m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_80m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_5G_160M_CALI_BAND_NUM];
} wlan_cali_5g_160m_save_stru;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_20m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_40m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_80m_save_stru;

typedef struct {
    wlan_cali_basic_para_stru cali_data[WLAN_6G_160M_CALI_BAND_NUM];
} wlan_cali_6g_160m_save_stru;
#endif

/* dbdc单个带宽单信道基准校准数据结构体 */
typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_rxdc_para_stru  rxdc_mimo_cali_data;
    wlan_cali_dpd_rxdc_para_stru  dpd_rxdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_cali_data;
    wlan_cali_txdc_para_stru  txdc_mimo_cali_data;
    wlan_cali_txpwr_para_stru txpwr_cali_data;
    wlan_cali_txiq_para_per_freq_stru txiq_cali_data;
    wlan_cali_rxiq_para_per_freq_stru rxiq_cali_data;
} wlan_cali_dbdc_basic_para_stru;

typedef struct {
    wlan_cali_dbdc_basic_para_stru  cali_data_20[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_dbdc_para_stru;

typedef struct {
    wlan_cali_dbdc_basic_para_stru cali_data_20[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_80[WLAN_5G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_160[WLAN_5G_160M_CALI_BAND_NUM];
} wlan_cali_5g_dbdc_para_stru;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
typedef struct {
    wlan_cali_dbdc_basic_para_stru cali_data_20[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_80[WLAN_6G_20M_CALI_BAND_NUM];
    wlan_cali_dbdc_basic_para_stru cali_data_160[WLAN_6G_160M_CALI_BAND_NUM];
} wlan_cali_6g_dbdc_para_stru;
#endif

/* 校准策略/控制参数 */
typedef struct {
    mp16_update_cali_channel_stru cali_update_info;
    uint32_t check_hw_status[HD_EVENT_RF_NUM];

    uint8_t last_cali_fail_status;
    uint8_t fem_error_band : 3,
            pll1_err_flag : 3,
            curr_bt_on_flag : 1,
            curr_gle_on_flag : 1;
    uint16_t lna_code_abnormal_cnt;
} wlan_cali_ctl_para_stru;

/* 非通道相关(或者非带宽相关)校准数据结构体 */
typedef struct {
    wlan_cali_rc_r_c_para_stru rc_r_c_cali_data[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM];
    wlan_cali_lna_blk_para_stru lna_blk_cali_data[WLAN_CALI_BAND_BUTT][HD_EVENT_RF_NUM];
    wlan_cali_abb_para_stru abb_cali_data[HD_EVENT_RF_NUM];
    wlan_cali_lodiv_para_stru lodiv_cali_data[HD_EVENT_RF_NUM];
    wlan_cali_ppf_para_stru ppf_cali_data_5g[HD_EVENT_RF_NUM][WLAN_5G_20M_CALI_BAND_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_ppf_para_stru ppf_cali_data_6g[HD_EVENT_RF_NUM][WLAN_6G_20M_CALI_BAND_NUM];
#endif
    wlan_cali_logen_para_stru logen_cali_data_2g[HD_EVENT_RF_NUM][WLAN_2G_CALI_BAND_NUM];
    wlan_cali_logen_para_stru logen_cali_data_5g[HD_EVENT_RF_NUM][WLAN_5G_20M_CALI_BAND_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_logen_para_stru logen_cali_data_6g[HD_EVENT_RF_NUM][WLAN_6G_20M_CALI_BAND_NUM];
#endif
    uint8_t  pa_current_data_5g[HD_EVENT_RF_NUM][WLAN_5G_20M_CALI_BAND_NUM];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    uint8_t  pa_current_data_6g[HD_EVENT_RF_NUM][WLAN_6G_20M_CALI_BAND_NUM];
#endif
    wlan_cali_ctl_para_stru cali_ctl_data;
    wlan_cali_lp_rxdc_para_stru lp_rxdc_cali_date_2g[WLAN_2G_CALI_BAND_NUM];
    wlan_cali_lp_rxdc_para_stru lp_rxdc_cali_date_5g[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_common_para_stru;

typedef struct {
    wlan_cali_2g_dbdc_para_stru cali_data_2g_dbdc;
    wlan_cali_5g_dbdc_para_stru cali_data_5g_dbdc;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_dbdc_para_stru cali_data_6g_dbdc;
#endif
} wlan_cali_dbdc_para_stru;

/* 20M带宽iq校准数据:只保存一档数据 */
typedef struct {
    wlan_cali_txiq_para_per_lvl_stru qmc_data;
    wlan_cali_iq_coef_stru qmc_fb;
} wlan_cali_txiq_20m_para_per_freq_stru;

/* 20M带宽dpd校准数据:只保存一档数据 */
typedef struct {
    wlan_cali_dpd_lut_stru dpd_cali_para;
} wlan_cali_dpd_20m_para_stru;

typedef struct {
    wlan_cali_rxdc_para_stru  rxdc_cali_data;
    wlan_cali_txdc_para_stru txdc_cali_data;
    wlan_cali_txpwr_para_stru txpwr_cali_data;
} wlan_cali_20m_basic_para_stru;

/* 扫描(20m)2G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_2G_CALI_BAND_NUM];
} wlan_cali_2g_20m_para_stru;

/* 扫描(20m)5G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_5G_20M_CALI_BAND_NUM];
} wlan_cali_5g_20m_para_stru;

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/* 扫描(20m)6G校准数据结构体 */
typedef struct {
    wlan_cali_20m_basic_para_stru chn_cali_data[WLAN_6G_20M_CALI_BAND_NUM];
} wlan_cali_6g_20m_para_stru;
#endif

/* 扫描(20m)单个通道校准数据结构体 */
typedef struct {
    wlan_cali_2g_20m_para_stru cali_data_2g;
    wlan_cali_5g_20m_para_stru cali_data_5g;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_20m_para_stru cali_data_6g;
#endif
} wlan_cali_rf_20m_para_stru;

/* 扫描(20m)comm数据结构体(不区分通道) */
typedef struct {
    wlan_cali_common_para_stru common_cali_data;
    uint8_t open_wifi_cali;
    uint8_t cali_cnt;
    uint16_t fac_cali_chain;
} wlan_cali_data_20m_common_para_stru;

/* 单通道校准数据结构体 */
typedef struct {
    wlan_cali_2g_20m_save_stru cali_data_2g_20m;
    wlan_cali_5g_20m_save_stru cali_data_5g_20m;
    wlan_cali_5g_80m_save_stru cali_data_5g_80m;
    wlan_cali_5g_160m_save_stru cali_data_5g_160m;
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_6g_20m_save_stru cali_data_6g_20m;
    wlan_cali_6g_80m_save_stru cali_data_6g_80m;
    wlan_cali_6g_160m_save_stru cali_data_6g_160m;
#endif
} wlan_cali_rf_para_stru;

/* host校准数据结构体 */
typedef struct {
    wlan_cali_rf_para_stru rf_cali_data[HD_EVENT_RF_NUM];
    wlan_cali_dbdc_para_stru dbdc_cali_data[HD_EVENT_SLAVE_RF_NUM];
    wlan_cali_common_para_stru common_cali_data;
} wlan_cali_data_para_stru;

/* 单信道校准数据结构体 */
typedef struct {
    wlan_cali_basic_para_stru cali_data[HD_EVENT_RF_NUM];
    wlan_cali_dbdc_basic_para_stru dbdc_cali_data[HD_EVENT_SLAVE_RF_NUM];
} wlan_cali_chn_stru;

/* 8 UNION定义 */
/* 9 OTHERS定义 */
/* 10 函数声明 */
/*
 * 函 数 名   : wlan_rf_cali_div_cali_2g_band_idx
 * 功能描述   : 根据中心频点索引获取校准软件补偿idx值
 * 1.日    期   : 2020年09月14日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
static inline uint8_t wlan_rf_cali_div_cali_2g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    uint16_t sub_band_len = WLAN_2G_CALI_BAND_NUM;
    wlan_cali_div_band_stru sub_band_range_2g[] = { { 2412, 2432 }, { 2437, 2457 }, { 2462, 2484 } };

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range_2g[subband_idx].center_freq_start,
            sub_band_range_2g[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}

/*
 * 函 数 名   : wlan_rf_cali_div_cali_5g_band_idx
 * 功能描述   : 根据5G中心频点索引获取校准软件补偿idx值
 * 1.日    期   : 2020年09月14日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
static inline uint8_t wlan_rf_cali_div_cali_5g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    wlan_cali_div_band_stru *sub_band_range = NULL;
    uint16_t sub_band_len;
    wlan_cali_div_band_stru sub_band_range_5g[] = {
        { 4920, 4980 }, { 5160, 5240 }, { 5245, 5320 }, { 5485, 5575 }, { 5580, 5655 }, { 5660, 5740 }, { 5745, 5840 },
    };
    wlan_cali_div_band_stru sub_band_160m_range[] = { { 4920, 5320 }, { 5485, 5840 } };

    if ((rf_cali_type >= WLAN_RF_CALI_RX_DC && rf_cali_type <= WLAN_RF_CALI_DPD) && (bw == WLAN_BANDWIDTH_160)) {
        sub_band_range = sub_band_160m_range;
        sub_band_len = WLAN_5G_160M_CALI_BAND_NUM;
    } else {
        sub_band_range = sub_band_range_5g;
        sub_band_len = WLAN_5G_20M_CALI_BAND_NUM;
    }

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range[subband_idx].center_freq_start,
            sub_band_range[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}

/*
 * 函 数 名   : wlan_rf_cali_div_cali_6g_band_idx
 * 功能描述   : 根据6G中心频点索引获取校准软件补偿idx值
 * 1.日    期   : 2020年09月14日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
static inline uint8_t wlan_rf_cali_div_cali_6g_band_idx(wlan_rf_cali_type_enum_uint8 rf_cali_type,
    wlan_bandwidth_type_enum_uint8 bw, uint16_t center_freq)
{
    uint8_t subband_idx;
    wlan_cali_div_band_stru *sub_band_range = NULL;
    uint16_t sub_band_len;
    wlan_cali_div_band_stru sub_band_range_6g[] = {
        { 5845, 5950 }, { 5955, 6110 }, { 6115, 6270 }, { 6275, 6430 },
        { 6435, 6590 }, { 6595, 6750 }, { 6755, 6910 }, { 6915, 7115 },
    };
    wlan_cali_div_band_stru sub_band_160m_range[] = {
        { 5845, 6110 }, { 6115, 6270 }, { 6275, 6430 }, { 6435, 6590 }, { 6595, 6750 }, { 6755, 6910 }, { 6915, 7115 },
    };

    if ((rf_cali_type >= WLAN_RF_CALI_RX_DC && rf_cali_type <= WLAN_RF_CALI_DPD) && (bw == WLAN_BANDWIDTH_160)) {
        sub_band_range = sub_band_160m_range;
        sub_band_len = WLAN_6G_160M_CALI_BAND_NUM;
    } else {
        sub_band_range = sub_band_range_6g;
        sub_band_len = WLAN_6G_20M_CALI_BAND_NUM;
    }

    for (subband_idx = 0; subband_idx < sub_band_len; subband_idx++) {
        if (wlan_cali_in_valid_range(center_freq, sub_band_range[subband_idx].center_freq_start,
            sub_band_range[subband_idx].center_freq_end)) {
            break;
        }
    }

    /* 当配置异常时，本地配置为band1 */
    subband_idx = (subband_idx >= sub_band_len) ? 0x1 : subband_idx;
    return subband_idx;
}
/*
 * 函 数 名   : wlan_get_cali_band
 * 功能描述   : 获取校准band
 * 1.日    期   : 2020年1月20日
 *   作    者   : wifi
 *   修改内容   : 新生成函数
 */
static inline wlan_cali_band_enum_uint8 wlan_get_cali_band(mac_channel_stru *channel_info)
{
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    if ((channel_info->en_band == WLAN_BAND_5G) && (channel_info->ext6g_band == OAL_TRUE)) {
        return WLAN_CALI_BAND_6G;
    }
#endif
    return channel_info->en_band;
}

#endif /* end of wlan_cali_mp16.h */
