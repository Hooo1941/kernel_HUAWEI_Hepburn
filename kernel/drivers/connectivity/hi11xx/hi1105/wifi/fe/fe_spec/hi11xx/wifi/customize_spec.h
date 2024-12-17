
#ifndef FE_CUSTOMIZE_SPEC_H
#define FE_CUSTOMIZE_SPEC_H

/* ****************************************************************************
  1 其他头文件包含
**************************************************************************** */
#ifdef _PRE_CUSTOM_CHECK_UT
#include "types_stub.h"
#else
#include "oal_types.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/* ****************************************************************************
  2 宏定义
************************************************************************ */
/* 定制化相关宏 */
#define FE_CUSTOMIZATION_LEN (FE_CUSTOMIZATION_2G_LEN + \
    FE_CUSTOMIZATION_5G_LEN + FE_CUSTOMIZATION_6G_LEN + FE_CUSTOMIZATION_MAGIC_NUMBER_LEN)
#define FE_CUSTOMIZATION_MAGIC_NUMBER_LEN         4
#define FE_CUSTOMIZATION_2G_OFFSET  0
#define FE_CUSTOMIZATION_2G_LEN     1492
#define FE_CUSTOMIZATION_5G_OFFSET  (FE_CUSTOMIZATION_2G_OFFSET + FE_CUSTOMIZATION_2G_LEN)
#define FE_CUSTOMIZATION_5G_LEN     1764
#define FE_CUSTOMIZATION_6G_OFFSET  (FE_CUSTOMIZATION_5G_OFFSET + FE_CUSTOMIZATION_5G_LEN)
#define FE_CUSTOMIZATION_6G_LEN     0

/* 定制化检查项相关门限值 */
/* begin */
#define FE_CUS_MAX_BASE_TXPOWER_VAL  250                 /* TPC0最大有效值 */
#define FE_CUS_BASE_TXPOWER_VAL      180                 /* TPC0经典有效值 */
#define FE_CUS_MIN_BASE_TXPOWER_VAL  50                  /* TPC0最小有效值 */

#define FE_CUS_MAX_UPC_PROTECT_VAL   0xff                /* upc保护最大有效值 */
#define FE_CUS_UPC_PROTECT_VAL       0x50                /* upc保护经典有效值 */
#define FE_CUS_MIN_UPC_PROTECT_VAL   0x5                 /* upc保护最小有效值 */

#define FE_CUS_MAX_COUPLER_GAIN_VAL  440              /* coupler保护最大有效值 */
#define FE_CUS_COUPLER_GAIN_VAL      360              /* coupler保护经典有效值 */
#define FE_CUS_MIN_COUPLER_GAIN_VAL  300              /* coupler保护最小有效值 */

#define FE_CUS_MAX_FB_LOSS_VAL       50           /* fb_loss最大有效值 */
#define FE_CUS_MIN_FB_LOSS_VAL      (-50)         /* fb_loss最小有效值 */


#define FE_CUS_MAX_DELT_BASE_POW_VAL  50           /* delt base pow 最大有效值 */
#define FE_CUS_MIN_DELT_BASE_POW_VAL (-50)          /* delt base pow 最小有效值 */
#define FE_CUS_MAX_DELT_RATE_POW_VAL  50            /* delt rate pow 最大有效值 */
#define FE_CUS_MIN_DELT_RATE_POW_VAL (-50)          /* delt rate pow 最小有效值 */
/* end */

#define FE_5G_CALI_SUB_BAND_NUM 5
#define FE_6G_CALI_SUB_BAND_NUM 15

#define FE_CUS_FB_LVL_NUM 2

#define FE_FBLOSS_PARAM_2G_NUM 13
#define FE_FBLOSS_PARAM_5G_NUM 7
#define FE_FBLOSS_PARAM_6G_NUM 15

#define FE_OFDM_DIFF_PARAM_6G_NUM 15 // 6g band数

#define FE_DYN_CALI_5G_TXPWR_CHANNEL_NUM 7
#define FE_5G_DYN_CALI_BAND 5
#define FE_6G_DYN_CALI_BAND 15
#define FE_PWR_LIMIT_2G_CH_NUM 13
#define FE_CUS_6G_BASE_PWR_NUM 15

#define FE_POW_CUSTOM_24G_11B_RATE_NUM        2 /* 定制化11b速率数目 */
#define FE_POW_CUSTOM_11G_11A_RATE_NUM        5 /* 定制化11g/11a速率数目 */
#define FE_POW_CUSTOM_HT20_VHT20_RATE_NUM     6 /* 定制化HT20_VHT20速率数目 */
#define FE_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM 8
#define FE_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM  7
#define FE_POW_CUSTOM_5G_VHT80_RATE_NUM       6
#define FE_POW_CUSTOM_MCS9_10_11_RATE_2G_NUM  5
#define FE_POW_CUSTOM_MCS9_10_11_RATE_5G_NUM  7
#define FE_POW_CUSTOM_MCS10_11_RATE_NUM       2
#define FE_POW_CUSTOM_5G_VHT160_RATE_NUM      12 /* 定制化5G_11ac_VHT160速率数目 */
#define FE_POW_CUSTOM_HE20_RATE_NUM              2
#define FE_POW_CUSTOM_HE40_RATE_NUM              2
#define FE_POW_CUSTOM_5G_HE80_RATE_NUM           2
#define FE_POW_CUSTOM_5G_HE160_RATE_NUM          2

#define FE_CUS_DELTA_POW_RATE_2G_NUM (FE_POW_CUSTOM_24G_11B_RATE_NUM + FE_POW_CUSTOM_11G_11A_RATE_NUM +               \
    FE_POW_CUSTOM_HT20_VHT20_RATE_NUM +                                                                               \
    FE_POW_CUSTOM_24G_HT40_VHT40_RATE_NUM + FE_POW_CUSTOM_MCS9_10_11_RATE_2G_NUM + FE_POW_CUSTOM_HE20_RATE_NUM +      \
    FE_POW_CUSTOM_HE40_RATE_NUM)
#define FE_CUS_DELTA_POW_RATE_5G_NUM (FE_POW_CUSTOM_11G_11A_RATE_NUM + FE_POW_CUSTOM_HT20_VHT20_RATE_NUM +            \
    FE_POW_CUSTOM_5G_HT40_VHT40_RATE_NUM + FE_POW_CUSTOM_5G_VHT80_RATE_NUM + FE_POW_CUSTOM_MCS9_10_11_RATE_5G_NUM +   \
    FE_POW_CUSTOM_5G_VHT160_RATE_NUM + FE_POW_CUSTOM_HE20_RATE_NUM + FE_POW_CUSTOM_HE40_RATE_NUM +                    \
    FE_POW_CUSTOM_5G_HE80_RATE_NUM + FE_POW_CUSTOM_5G_HE160_RATE_NUM + FE_POW_CUSTOM_MCS10_11_RATE_NUM +              \
    FE_POW_CUSTOM_5G_VHT160_RATE_NUM)
#define FE_CUS_DELTA_POW_RATE_6G_NUM (FE_POW_CUSTOM_11G_11A_RATE_NUM + FE_POW_CUSTOM_HT20_VHT20_RATE_NUM)

#define FE_PWR_LIMIT_5G20M_CH_NUM 24
#define FE_PWR_LIMIT_5G40M_CH_NUM 11
#define FE_PWR_LIMIT_5G80M_CH_NUM 5
#define FE_PWR_LIMIT_5G160M_CH_NUM 2

#define FE_PWR_LIMIT_6G20M_CH_NUM 59
#define FE_PWR_LIMIT_6G40M_CH_NUM 29
#define FE_PWR_LIMIT_6G80M_CH_NUM 14
#define FE_PWR_LIMIT_6G160M_CH_NUM 7

#define FE_CUS_SAR_LVL_NUM            20 /* 定制化降SAR档位数 */
#define FE_CUS_NUM_OF_SAR_PARAMS      8 /* 定制化降SAR参数 5G_BAND1~7 2.4G */
#define FE_CUS_RF_PA_BIAS_REG_NUM 1
#define FE_6G_RF_BAND_NUM      15

#define FE_NUM_5G_20M_SIDE_BAND        24    /* 定制化5g边带数 */
#define FE_NUM_5G_40M_SIDE_BAND        11
#define FE_NUM_5G_80M_SIDE_BAND        5
#define FE_NUM_5G_160M_SIDE_BAND       2
/* ****************************************************************************
  3 枚举定义
**************************************************************************** */
typedef enum {
    FE_TPC_INIT_MODE      = 0,    /* tpc初始功率模式 */
    FE_TPC_WORK_MODE_BUTT
}fe_tpc_work_mode_enum;

typedef enum {
    FE_CALI_HIGH_POW                  = 0,

    FE_CALI_POW_BUTT
} cust_dync_cali_power_enum;

typedef enum {
    FE_CALI_HIGH_POW_5G                  = 0,
    FE_CALI_LOW_POW_5G                   = 1,

    FE_CALI_POW_5G_BUTT
} cust_dync_cali_power_5g_enum;

typedef enum {
    FE_CALI_HIGH_CHANNEL                  = 0,

    FE_CALI_CHANNELS_LVL_BUTT
} cust_pow_ctrl_channel_high_low_enum;

typedef enum {
    FE_DYN_PWR_CUST_SNGL_MODE_11B             = 0,
    FE_DYN_PWR_CUST_SNGL_MODE_OFDM20          = 1,
    FE_DYN_PWR_CUST_SNGL_MODE_OFDM40          = 2,
    FE_DYN_PWR_CURVE_MODE_NUMS                = 3,
    FE_DYN_PWR_CUST_2G_SNGL_MODE_BUTT = FE_DYN_PWR_CURVE_MODE_NUMS
} cust_dyn_2g_pwr_sngl_mode_enum;

typedef enum {
    FE_PWR_LIMIT_MODE_20M = 0,
    FE_PWR_LIMIT_MODE_20M_BUTT
} fe_pwr_limit_mode;

typedef enum {
    FE_POW_MIMO_BACK_MCS, // 区分速率

    FE_POW_MIMO_BACK_BUTT
} fe_cust_mimo_back_enum;
typedef enum {
    FE_POW_TXBF_BACK_MCS, // 区分速率

    FE_POW_TXBF_BACK_BUTT
} fe_cust_txbf_back_enum;

typedef enum {
    FE_BW_CAP_20M = 0,                       /* 20MHz全带宽 */
    FE_BW_CAP_40M = 1,                       /* 40MHz全带宽 */
    FE_BW_CAP_2G_BUTT = 2,                   /* 2G只有40M带宽 */
    FE_BW_CAP_80M = FE_BW_CAP_2G_BUTT,    /* 80MHz全带宽 */
    FE_BW_CAP_160M = 3,                      /* 160MHz全带宽 */
    FE_BW_CAP_BUTT
} fe_be_cap_type_enum;
/* ****************************************************************************
  4 全局变量声明
**************************************************************************** */
/* ****************************************************************************
  7 STRUCT定义
**************************************************************************** */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
