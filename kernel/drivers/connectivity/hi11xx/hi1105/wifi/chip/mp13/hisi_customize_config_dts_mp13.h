/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化函数声明
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */

#ifndef HISI_CUSTOMIZE_CONFIG_DTS_MP13_H
#define HISI_CUSTOMIZE_CONFIG_DTS_MP13_H

#include "hisi_customize_wifi_mp13.h"

/* 定制化 DTS CONFIG ID */
typedef enum {
    /* 校准 */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1,
    /* 校准 2g TXPWR_REF起始配置ID */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_START = WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN1,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN2,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN3,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN4,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN5,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN6,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN7,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN8,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN9,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN10,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN11,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN12,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_2G_VAL_CHAN13,

    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1,  // 13
    /* 校准 5g TXPWR_REF起始配置ID */
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_START = WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND1,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND2,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND3,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND4,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND5,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND6,
    WLAN_CFG_DTS_CALI_TXPWR_PA_DC_REF_5G_VAL_BAND7,
    WLAN_CFG_DTS_CALI_TONE_AMP_GRADE,

    /* DPD校准 */
    WLAN_CFG_DTS_DPD_CALI_CH_CORE0,
    WLAN_CFG_DTS_DPD_CALI_START = WLAN_CFG_DTS_DPD_CALI_CH_CORE0, /* DPD校准定制化配置起始ID */
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE0,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE0,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE0,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE0,
    WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE0,
    WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE0,
    WLAN_CFG_DTS_DPD_CALI_CH_CORE1,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX0_CORE1,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX1_CORE1,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX2_CORE1,
    WLAN_CFG_DTS_DPD_USE_CALI_CH_IDX3_CORE1,
    WLAN_CFG_DTS_DPD_CALI_20M_DEL_POW_CORE1,
    WLAN_CFG_DTS_DPD_CALI_40M_DEL_POW_CORE1,
    /* 动态校准 */
    WLAN_CFG_DTS_DYN_CALI_DSCR_ITERVL,
    WLAN_CFG_DTS_DYN_CALI_OPT_SWITCH,
    WLAN_CFG_DTS_DYN_CALI_GM0_DB10_AMEND,

    /* DPN 40M 20M 11b */
    WLAN_CFG_DTS_2G_CORE0_DPN_CH1,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH2,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH3,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH4,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH5,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH6,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH7,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH8,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH9,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH10,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH11,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH12,
    WLAN_CFG_DTS_2G_CORE0_DPN_CH13,
    WLAN_CFG_DTS_5G_CORE0_DPN_B0,
    WLAN_CFG_DTS_5G_CORE0_DPN_B1,
    WLAN_CFG_DTS_5G_CORE0_DPN_B2,
    WLAN_CFG_DTS_5G_CORE0_DPN_B3,
    WLAN_CFG_DTS_5G_CORE0_DPN_B4,
    WLAN_CFG_DTS_5G_CORE0_DPN_B5,
    WLAN_CFG_DTS_5G_CORE0_DPN_B6,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH1,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH2,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH3,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH4,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH5,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH6,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH7,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH8,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH9,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH10,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH11,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH12,
    WLAN_CFG_DTS_2G_CORE1_DPN_CH13,
    WLAN_CFG_DTS_5G_CORE1_DPN_B0,
    WLAN_CFG_DTS_5G_CORE1_DPN_B1,
    WLAN_CFG_DTS_5G_CORE1_DPN_B2,
    WLAN_CFG_DTS_5G_CORE1_DPN_B3,
    WLAN_CFG_DTS_5G_CORE1_DPN_B4,
    WLAN_CFG_DTS_5G_CORE1_DPN_B5,
    WLAN_CFG_DTS_5G_CORE1_DPN_B6,

    WLAN_CFG_DTS_BUTT,
} wlan_cfg_dts;

int32_t *hwifi_get_g_dts_params(void);
wlan_cfg_cmd *hwifi_get_g_wifi_config_dts(void);
void original_value_for_dts_params(void);
uint32_t hwifi_config_init_dts_main(oal_net_device_stru *cfg_net_dev);

#endif  // hisi_customize_config_dts_mp13.h