/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化头文件声明
 * 作者       :
 * 创建日期   : 2015年10月08日
 */

#ifndef HISI_CUSTOMIZE_WIFI_POW_H
#define HISI_CUSTOMIZE_WIFI_POW_H

/* 定制化 NVRAM PARAMS INDEX */
typedef enum {
    /* FCC C0边带最大目标发射功率 */
    NVRAM_PARA_FCC_C0_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C0_0 = NVRAM_PARA_FCC_C0_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C0_1,
    NVRAM_PARA_FCC_5G_20M_C0_2,
    NVRAM_PARA_FCC_5G_20M_C0_3,
    NVRAM_PARA_FCC_5G_40M_C0_0,
    NVRAM_PARA_FCC_5G_40M_C0_1,
    NVRAM_PARA_FCC_5G_80M_C0,
    NVRAM_PARA_FCC_5G_160M_C0,
    NVRAM_PARA_FCC_2P4_C0_CH1,
    NVRAM_PARA_FCC_2P4_C0_CH2,
    NVRAM_PARA_FCC_2P4_C0_CH3,
    NVRAM_PARA_FCC_2P4_C0_CH4,
    NVRAM_PARA_FCC_2P4_C0_CH5,
    NVRAM_PARA_FCC_2P4_C0_CH6,
    NVRAM_PARA_FCC_2P4_C0_CH7,
    NVRAM_PARA_FCC_2P4_C0_CH8,
    NVRAM_PARA_FCC_2P4_C0_CH9,
    NVRAM_PARA_FCC_2P4_C0_CH10,
    NVRAM_PARA_FCC_2P4_C0_CH11,
    NVRAM_PARA_FCC_2P4_C0_CH12,
    NVRAM_PARA_FCC_2P4_C0_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_FCC_6G_20M_C0,
    NVRAM_PARA_FCC_6G_20M_C0_1,
    NVRAM_PARA_FCC_6G_40M_C0,
    NVRAM_PARA_FCC_6G_80M_C0,
    NVRAM_PARA_FCC_6G_160M_C0,
#endif
    NVRAM_PARA_FCC_C0_END_INDEX_BUTT,

    /* FCC C1边带最大目标发射功率 */
    NVRAM_PARA_FCC_C1_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C1_0 = NVRAM_PARA_FCC_C1_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C1_1,
    NVRAM_PARA_FCC_5G_20M_C1_2,
    NVRAM_PARA_FCC_5G_20M_C1_3,
    NVRAM_PARA_FCC_5G_40M_C1_0,
    NVRAM_PARA_FCC_5G_40M_C1_1,
    NVRAM_PARA_FCC_5G_80M_C1,
    NVRAM_PARA_FCC_5G_160M_C1,
    NVRAM_PARA_FCC_2P4_C1_CH1,
    NVRAM_PARA_FCC_2P4_C1_CH2,
    NVRAM_PARA_FCC_2P4_C1_CH3,
    NVRAM_PARA_FCC_2P4_C1_CH4,
    NVRAM_PARA_FCC_2P4_C1_CH5,
    NVRAM_PARA_FCC_2P4_C1_CH6,
    NVRAM_PARA_FCC_2P4_C1_CH7,
    NVRAM_PARA_FCC_2P4_C1_CH8,
    NVRAM_PARA_FCC_2P4_C1_CH9,
    NVRAM_PARA_FCC_2P4_C1_CH10,
    NVRAM_PARA_FCC_2P4_C1_CH11,
    NVRAM_PARA_FCC_2P4_C1_CH12,
    NVRAM_PARA_FCC_2P4_C1_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_FCC_6G_20M_C1,
    NVRAM_PARA_FCC_6G_20M_C1_1,
    NVRAM_PARA_FCC_6G_40M_C1,
    NVRAM_PARA_FCC_6G_80M_C1,
    NVRAM_PARA_FCC_6G_160M_C1,
#endif
    NVRAM_PARA_FCC_C1_END_INDEX_BUTT,

    /* FCC C2边带最大目标发射功率 */
    NVRAM_PARA_FCC_C2_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C2_0 = NVRAM_PARA_FCC_C2_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C2_1,
    NVRAM_PARA_FCC_5G_20M_C2_2,
    NVRAM_PARA_FCC_5G_20M_C2_3,
    NVRAM_PARA_FCC_5G_40M_C2_0,
    NVRAM_PARA_FCC_5G_40M_C2_1,
    NVRAM_PARA_FCC_5G_80M_C2,
    NVRAM_PARA_FCC_5G_160M_C2,
    NVRAM_PARA_FCC_2P4_C2_CH1,
    NVRAM_PARA_FCC_2P4_C2_CH2,
    NVRAM_PARA_FCC_2P4_C2_CH3,
    NVRAM_PARA_FCC_2P4_C2_CH4,
    NVRAM_PARA_FCC_2P4_C2_CH5,
    NVRAM_PARA_FCC_2P4_C2_CH6,
    NVRAM_PARA_FCC_2P4_C2_CH7,
    NVRAM_PARA_FCC_2P4_C2_CH8,
    NVRAM_PARA_FCC_2P4_C2_CH9,
    NVRAM_PARA_FCC_2P4_C2_CH10,
    NVRAM_PARA_FCC_2P4_C2_CH11,
    NVRAM_PARA_FCC_2P4_C2_CH12,
    NVRAM_PARA_FCC_2P4_C2_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_FCC_6G_20M_C2,
    NVRAM_PARA_FCC_6G_20M_C2_1,
    NVRAM_PARA_FCC_6G_40M_C2,
    NVRAM_PARA_FCC_6G_80M_C2,
    NVRAM_PARA_FCC_6G_160M_C2,
#endif
    NVRAM_PARA_FCC_C2_END_INDEX_BUTT,

    /* FCC C3边带最大目标发射功率 */
    NVRAM_PARA_FCC_C3_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C3_0 = NVRAM_PARA_FCC_C3_START_INDEX,
    NVRAM_PARA_FCC_5G_20M_C3_1,
    NVRAM_PARA_FCC_5G_20M_C3_2,
    NVRAM_PARA_FCC_5G_20M_C3_3,
    NVRAM_PARA_FCC_5G_40M_C3_0,
    NVRAM_PARA_FCC_5G_40M_C3_1,
    NVRAM_PARA_FCC_5G_80M_C3,
    NVRAM_PARA_FCC_5G_160M_C3,
    NVRAM_PARA_FCC_2P4_C3_CH1,
    NVRAM_PARA_FCC_2P4_C3_CH2,
    NVRAM_PARA_FCC_2P4_C3_CH3,
    NVRAM_PARA_FCC_2P4_C3_CH4,
    NVRAM_PARA_FCC_2P4_C3_CH5,
    NVRAM_PARA_FCC_2P4_C3_CH6,
    NVRAM_PARA_FCC_2P4_C3_CH7,
    NVRAM_PARA_FCC_2P4_C3_CH8,
    NVRAM_PARA_FCC_2P4_C3_CH9,
    NVRAM_PARA_FCC_2P4_C3_CH10,
    NVRAM_PARA_FCC_2P4_C3_CH11,
    NVRAM_PARA_FCC_2P4_C3_CH12,
    NVRAM_PARA_FCC_2P4_C3_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_FCC_6G_20M_C3,
    NVRAM_PARA_FCC_6G_20M_C3_1,
    NVRAM_PARA_FCC_6G_40M_C3,
    NVRAM_PARA_FCC_6G_80M_C3,
    NVRAM_PARA_FCC_6G_160M_C3,
#endif
    NVRAM_PARA_FCC_C3_END_INDEX_BUTT,

    /* CE C0边带最大目标发射功率 */
    NVRAM_PARA_CE_C0_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C0_0 = NVRAM_PARA_CE_C0_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C0_1,
    NVRAM_PARA_CE_5G_20M_C0_2,
    NVRAM_PARA_CE_5G_20M_C0_3,
    NVRAM_PARA_CE_5G_40M_C0_0,
    NVRAM_PARA_CE_5G_40M_C0_1,
    NVRAM_PARA_CE_5G_80M_C0,
    NVRAM_PARA_CE_5G_160M_C0,
    NVRAM_PARA_CE_2P4_C0_CH1,
    NVRAM_PARA_CE_2P4_C0_CH2,
    NVRAM_PARA_CE_2P4_C0_CH3,
    NVRAM_PARA_CE_2P4_C0_CH4,
    NVRAM_PARA_CE_2P4_C0_CH5,
    NVRAM_PARA_CE_2P4_C0_CH6,
    NVRAM_PARA_CE_2P4_C0_CH7,
    NVRAM_PARA_CE_2P4_C0_CH8,
    NVRAM_PARA_CE_2P4_C0_CH9,
    NVRAM_PARA_CE_2P4_C0_CH10,
    NVRAM_PARA_CE_2P4_C0_CH11,
    NVRAM_PARA_CE_2P4_C0_CH12,
    NVRAM_PARA_CE_2P4_C0_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CE_6G_20M_C0,
    NVRAM_PARA_CE_6G_20M_C0_1,
    NVRAM_PARA_CE_6G_40M_C0,
    NVRAM_PARA_CE_6G_80M_C0,
    NVRAM_PARA_CE_6G_160M_C0,
#endif
    NVRAM_PARA_CE_C0_END_INDEX_BUTT,

    /* CE C1边带最大目标发射功率 */
    NVRAM_PARA_CE_C1_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C1_0 = NVRAM_PARA_CE_C1_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C1_1,
    NVRAM_PARA_CE_5G_20M_C1_2,
    NVRAM_PARA_CE_5G_20M_C1_3,
    NVRAM_PARA_CE_5G_40M_C1_0,
    NVRAM_PARA_CE_5G_40M_C1_1,
    NVRAM_PARA_CE_5G_80M_C1,
    NVRAM_PARA_CE_5G_160M_C1,
    NVRAM_PARA_CE_2P4_C1_CH1,
    NVRAM_PARA_CE_2P4_C1_CH2,
    NVRAM_PARA_CE_2P4_C1_CH3,
    NVRAM_PARA_CE_2P4_C1_CH4,
    NVRAM_PARA_CE_2P4_C1_CH5,
    NVRAM_PARA_CE_2P4_C1_CH6,
    NVRAM_PARA_CE_2P4_C1_CH7,
    NVRAM_PARA_CE_2P4_C1_CH8,
    NVRAM_PARA_CE_2P4_C1_CH9,
    NVRAM_PARA_CE_2P4_C1_CH10,
    NVRAM_PARA_CE_2P4_C1_CH11,
    NVRAM_PARA_CE_2P4_C1_CH12,
    NVRAM_PARA_CE_2P4_C1_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CE_6G_20M_C1,
    NVRAM_PARA_CE_6G_20M_C1_1,
    NVRAM_PARA_CE_6G_40M_C1,
    NVRAM_PARA_CE_6G_80M_C1,
    NVRAM_PARA_CE_6G_160M_C1,
#endif
    NVRAM_PARA_CE_C1_END_INDEX_BUTT,

    /* CE C2边带最大目标发射功率 */
    NVRAM_PARA_CE_C2_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C2_0 = NVRAM_PARA_CE_C2_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C2_1,
    NVRAM_PARA_CE_5G_20M_C2_2,
    NVRAM_PARA_CE_5G_20M_C2_3,
    NVRAM_PARA_CE_5G_40M_C2_0,
    NVRAM_PARA_CE_5G_40M_C2_1,
    NVRAM_PARA_CE_5G_80M_C2,
    NVRAM_PARA_CE_5G_160M_C2,
    NVRAM_PARA_CE_2P4_C2_CH1,
    NVRAM_PARA_CE_2P4_C2_CH2,
    NVRAM_PARA_CE_2P4_C2_CH3,
    NVRAM_PARA_CE_2P4_C2_CH4,
    NVRAM_PARA_CE_2P4_C2_CH5,
    NVRAM_PARA_CE_2P4_C2_CH6,
    NVRAM_PARA_CE_2P4_C2_CH7,
    NVRAM_PARA_CE_2P4_C2_CH8,
    NVRAM_PARA_CE_2P4_C2_CH9,
    NVRAM_PARA_CE_2P4_C2_CH10,
    NVRAM_PARA_CE_2P4_C2_CH11,
    NVRAM_PARA_CE_2P4_C2_CH12,
    NVRAM_PARA_CE_2P4_C2_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CE_6G_20M_C2,
    NVRAM_PARA_CE_6G_20M_C2_1,
    NVRAM_PARA_CE_6G_40M_C2,
    NVRAM_PARA_CE_6G_80M_C2,
    NVRAM_PARA_CE_6G_160M_C2,
#endif
    NVRAM_PARA_CE_C2_END_INDEX_BUTT,

    /* CE C3边带最大目标发射功率 */
    NVRAM_PARA_CE_C3_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C3_0 = NVRAM_PARA_CE_C3_START_INDEX,
    NVRAM_PARA_CE_5G_20M_C3_1,
    NVRAM_PARA_CE_5G_20M_C3_2,
    NVRAM_PARA_CE_5G_20M_C3_3,
    NVRAM_PARA_CE_5G_40M_C3_0,
    NVRAM_PARA_CE_5G_40M_C3_1,
    NVRAM_PARA_CE_5G_80M_C3,
    NVRAM_PARA_CE_5G_160M_C3,
    NVRAM_PARA_CE_2P4_C3_CH1,
    NVRAM_PARA_CE_2P4_C3_CH2,
    NVRAM_PARA_CE_2P4_C3_CH3,
    NVRAM_PARA_CE_2P4_C3_CH4,
    NVRAM_PARA_CE_2P4_C3_CH5,
    NVRAM_PARA_CE_2P4_C3_CH6,
    NVRAM_PARA_CE_2P4_C3_CH7,
    NVRAM_PARA_CE_2P4_C3_CH8,
    NVRAM_PARA_CE_2P4_C3_CH9,
    NVRAM_PARA_CE_2P4_C3_CH10,
    NVRAM_PARA_CE_2P4_C3_CH11,
    NVRAM_PARA_CE_2P4_C3_CH12,
    NVRAM_PARA_CE_2P4_C3_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CE_6G_20M_C3,
    NVRAM_PARA_CE_6G_20M_C3_1,
    NVRAM_PARA_CE_6G_40M_C3,
    NVRAM_PARA_CE_6G_80M_C3,
    NVRAM_PARA_CE_6G_160M_C3,
#endif
    NVRAM_PARA_CE_C3_END_INDEX_BUTT,
    /* CN C0 */
    NVRAM_PARA_CN_C0_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C0_0 = NVRAM_PARA_CN_C0_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C0_1,
    NVRAM_PARA_CN_5G_20M_C0_2,
    NVRAM_PARA_CN_5G_20M_C0_3,
    NVRAM_PARA_CN_5G_40M_C0_0,
    NVRAM_PARA_CN_5G_40M_C0_1,
    NVRAM_PARA_CN_5G_80M_C0,
    NVRAM_PARA_CN_5G_160M_C0,
    NVRAM_PARA_CN_2P4_C0_CH1,
    NVRAM_PARA_CN_2P4_C0_CH2,
    NVRAM_PARA_CN_2P4_C0_CH3,
    NVRAM_PARA_CN_2P4_C0_CH4,
    NVRAM_PARA_CN_2P4_C0_CH5,
    NVRAM_PARA_CN_2P4_C0_CH6,
    NVRAM_PARA_CN_2P4_C0_CH7,
    NVRAM_PARA_CN_2P4_C0_CH8,
    NVRAM_PARA_CN_2P4_C0_CH9,
    NVRAM_PARA_CN_2P4_C0_CH10,
    NVRAM_PARA_CN_2P4_C0_CH11,
    NVRAM_PARA_CN_2P4_C0_CH12,
    NVRAM_PARA_CN_2P4_C0_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CN_6G_20M_C0,
    NVRAM_PARA_CN_6G_20M_C0_1,
    NVRAM_PARA_CN_6G_40M_C0,
    NVRAM_PARA_CN_6G_80M_C0,
    NVRAM_PARA_CN_6G_160M_C0,
#endif
    NVRAM_PARA_CN_C0_END_INDEX_BUTT,
    /* CN C1 */
    NVRAM_PARA_CN_C1_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C1_0 = NVRAM_PARA_CN_C1_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C1_1,
    NVRAM_PARA_CN_5G_20M_C1_2,
    NVRAM_PARA_CN_5G_20M_C1_3,
    NVRAM_PARA_CN_5G_40M_C1_0,
    NVRAM_PARA_CN_5G_40M_C1_1,
    NVRAM_PARA_CN_5G_80M_C1,
    NVRAM_PARA_CN_5G_160M_C1,
    NVRAM_PARA_CN_2P4_C1_CH1,
    NVRAM_PARA_CN_2P4_C1_CH2,
    NVRAM_PARA_CN_2P4_C1_CH3,
    NVRAM_PARA_CN_2P4_C1_CH4,
    NVRAM_PARA_CN_2P4_C1_CH5,
    NVRAM_PARA_CN_2P4_C1_CH6,
    NVRAM_PARA_CN_2P4_C1_CH7,
    NVRAM_PARA_CN_2P4_C1_CH8,
    NVRAM_PARA_CN_2P4_C1_CH9,
    NVRAM_PARA_CN_2P4_C1_CH10,
    NVRAM_PARA_CN_2P4_C1_CH11,
    NVRAM_PARA_CN_2P4_C1_CH12,
    NVRAM_PARA_CN_2P4_C1_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CN_6G_20M_C1,
    NVRAM_PARA_CN_6G_20M_C1_1,
    NVRAM_PARA_CN_6G_40M_C1,
    NVRAM_PARA_CN_6G_80M_C1,
    NVRAM_PARA_CN_6G_160M_C1,
#endif
    NVRAM_PARA_CN_C1_END_INDEX_BUTT,
    /* CN C2 */
    NVRAM_PARA_CN_C2_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C2_0 = NVRAM_PARA_CN_C2_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C2_1,
    NVRAM_PARA_CN_5G_20M_C2_2,
    NVRAM_PARA_CN_5G_20M_C2_3,
    NVRAM_PARA_CN_5G_40M_C2_0,
    NVRAM_PARA_CN_5G_40M_C2_1,
    NVRAM_PARA_CN_5G_80M_C2,
    NVRAM_PARA_CN_5G_160M_C2,
    NVRAM_PARA_CN_2P4_C2_CH1,
    NVRAM_PARA_CN_2P4_C2_CH2,
    NVRAM_PARA_CN_2P4_C2_CH3,
    NVRAM_PARA_CN_2P4_C2_CH4,
    NVRAM_PARA_CN_2P4_C2_CH5,
    NVRAM_PARA_CN_2P4_C2_CH6,
    NVRAM_PARA_CN_2P4_C2_CH7,
    NVRAM_PARA_CN_2P4_C2_CH8,
    NVRAM_PARA_CN_2P4_C2_CH9,
    NVRAM_PARA_CN_2P4_C2_CH10,
    NVRAM_PARA_CN_2P4_C2_CH11,
    NVRAM_PARA_CN_2P4_C2_CH12,
    NVRAM_PARA_CN_2P4_C2_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CN_6G_20M_C2,
    NVRAM_PARA_CN_6G_20M_C2_1,
    NVRAM_PARA_CN_6G_40M_C2,
    NVRAM_PARA_CN_6G_80M_C2,
    NVRAM_PARA_CN_6G_160M_C2,
#endif
    NVRAM_PARA_CN_C2_END_INDEX_BUTT,
    /* CN C3 */
    NVRAM_PARA_CN_C3_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C3_0 = NVRAM_PARA_CN_C3_START_INDEX,
    NVRAM_PARA_CN_5G_20M_C3_1,
    NVRAM_PARA_CN_5G_20M_C3_2,
    NVRAM_PARA_CN_5G_20M_C3_3,
    NVRAM_PARA_CN_5G_40M_C3_0,
    NVRAM_PARA_CN_5G_40M_C3_1,
    NVRAM_PARA_CN_5G_80M_C3,
    NVRAM_PARA_CN_5G_160M_C3,
    NVRAM_PARA_CN_2P4_C3_CH1,
    NVRAM_PARA_CN_2P4_C3_CH2,
    NVRAM_PARA_CN_2P4_C3_CH3,
    NVRAM_PARA_CN_2P4_C3_CH4,
    NVRAM_PARA_CN_2P4_C3_CH5,
    NVRAM_PARA_CN_2P4_C3_CH6,
    NVRAM_PARA_CN_2P4_C3_CH7,
    NVRAM_PARA_CN_2P4_C3_CH8,
    NVRAM_PARA_CN_2P4_C3_CH9,
    NVRAM_PARA_CN_2P4_C3_CH10,
    NVRAM_PARA_CN_2P4_C3_CH11,
    NVRAM_PARA_CN_2P4_C3_CH12,
    NVRAM_PARA_CN_2P4_C3_CH13,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_CN_6G_20M_C3,
    NVRAM_PARA_CN_6G_20M_C3_1,
    NVRAM_PARA_CN_6G_40M_C3,
    NVRAM_PARA_CN_6G_80M_C3,
    NVRAM_PARA_CN_6G_160M_C3,
#endif
    NVRAM_PARA_CN_C3_END_INDEX_BUTT,
    /* C0 SAR CONTROL */
    NVRAM_PARA_C0_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_C0 = NVRAM_PARA_C0_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_C0,
    NVRAM_PARA_SAR_LVL3_C0,
    NVRAM_PARA_SAR_LVL4_C0,
    NVRAM_PARA_SAR_LVL5_C0,
    NVRAM_PARA_SAR_LVL6_C0,
    NVRAM_PARA_SAR_LVL7_C0,
    NVRAM_PARA_SAR_LVL8_C0,
    NVRAM_PARA_SAR_LVL9_C0,
    NVRAM_PARA_SAR_LVL10_C0,
    NVRAM_PARA_SAR_LVL11_C0,
    NVRAM_PARA_SAR_LVL12_C0,
    NVRAM_PARA_SAR_LVL13_C0,
    NVRAM_PARA_SAR_LVL14_C0,
    NVRAM_PARA_SAR_LVL15_C0,
    NVRAM_PARA_SAR_LVL16_C0,
    NVRAM_PARA_SAR_LVL17_C0,
    NVRAM_PARA_SAR_LVL18_C0,
    NVRAM_PARA_SAR_LVL19_C0,
    NVRAM_PARA_SAR_LVL20_C0,
    NVRAM_PARA_C0_SAR_END_INDEX_BUTT,

    /* C1 SAR CONTROL */
    NVRAM_PARA_C1_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_C1 = NVRAM_PARA_C1_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_C1,
    NVRAM_PARA_SAR_LVL3_C1,
    NVRAM_PARA_SAR_LVL4_C1,
    NVRAM_PARA_SAR_LVL5_C1,
    NVRAM_PARA_SAR_LVL6_C1,
    NVRAM_PARA_SAR_LVL7_C1,
    NVRAM_PARA_SAR_LVL8_C1,
    NVRAM_PARA_SAR_LVL9_C1,
    NVRAM_PARA_SAR_LVL10_C1,
    NVRAM_PARA_SAR_LVL11_C1,
    NVRAM_PARA_SAR_LVL12_C1,
    NVRAM_PARA_SAR_LVL13_C1,
    NVRAM_PARA_SAR_LVL14_C1,
    NVRAM_PARA_SAR_LVL15_C1,
    NVRAM_PARA_SAR_LVL16_C1,
    NVRAM_PARA_SAR_LVL17_C1,
    NVRAM_PARA_SAR_LVL18_C1,
    NVRAM_PARA_SAR_LVL19_C1,
    NVRAM_PARA_SAR_LVL20_C1,
    NVRAM_PARA_C1_SAR_END_INDEX_BUTT,

    /* C2 SAR CONTROL */
    NVRAM_PARA_C2_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_C2 = NVRAM_PARA_C2_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_C2,
    NVRAM_PARA_SAR_LVL3_C2,
    NVRAM_PARA_SAR_LVL4_C2,
    NVRAM_PARA_SAR_LVL5_C2,
    NVRAM_PARA_SAR_LVL6_C2,
    NVRAM_PARA_SAR_LVL7_C2,
    NVRAM_PARA_SAR_LVL8_C2,
    NVRAM_PARA_SAR_LVL9_C2,
    NVRAM_PARA_SAR_LVL10_C2,
    NVRAM_PARA_SAR_LVL11_C2,
    NVRAM_PARA_SAR_LVL12_C2,
    NVRAM_PARA_SAR_LVL13_C2,
    NVRAM_PARA_SAR_LVL14_C2,
    NVRAM_PARA_SAR_LVL15_C2,
    NVRAM_PARA_SAR_LVL16_C2,
    NVRAM_PARA_SAR_LVL17_C2,
    NVRAM_PARA_SAR_LVL18_C2,
    NVRAM_PARA_SAR_LVL19_C2,
    NVRAM_PARA_SAR_LVL20_C2,
    NVRAM_PARA_C2_SAR_END_INDEX_BUTT,

    /* C3 SAR CONTROL */
    NVRAM_PARA_C3_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_C3 = NVRAM_PARA_C3_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_C3,
    NVRAM_PARA_SAR_LVL3_C3,
    NVRAM_PARA_SAR_LVL4_C3,
    NVRAM_PARA_SAR_LVL5_C3,
    NVRAM_PARA_SAR_LVL6_C3,
    NVRAM_PARA_SAR_LVL7_C3,
    NVRAM_PARA_SAR_LVL8_C3,
    NVRAM_PARA_SAR_LVL9_C3,
    NVRAM_PARA_SAR_LVL10_C3,
    NVRAM_PARA_SAR_LVL11_C3,
    NVRAM_PARA_SAR_LVL12_C3,
    NVRAM_PARA_SAR_LVL13_C3,
    NVRAM_PARA_SAR_LVL14_C3,
    NVRAM_PARA_SAR_LVL15_C3,
    NVRAM_PARA_SAR_LVL16_C3,
    NVRAM_PARA_SAR_LVL17_C3,
    NVRAM_PARA_SAR_LVL18_C3,
    NVRAM_PARA_SAR_LVL19_C3,
    NVRAM_PARA_SAR_LVL20_C3,
    NVRAM_PARA_C3_SAR_END_INDEX_BUTT,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    /* 6G C0 SAR CONTROL */
    NVRAM_PARA_6G_C0_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_6G_C0 = NVRAM_PARA_6G_C0_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_6G_C0,
    NVRAM_PARA_SAR_LVL3_6G_C0,
    NVRAM_PARA_SAR_LVL4_6G_C0,
    NVRAM_PARA_SAR_LVL5_6G_C0,
    NVRAM_PARA_SAR_LVL6_6G_C0,
    NVRAM_PARA_SAR_LVL7_6G_C0,
    NVRAM_PARA_SAR_LVL8_6G_C0,
    NVRAM_PARA_SAR_LVL9_6G_C0,
    NVRAM_PARA_SAR_LVL10_6G_C0,
    NVRAM_PARA_SAR_LVL11_6G_C0,
    NVRAM_PARA_SAR_LVL12_6G_C0,
    NVRAM_PARA_SAR_LVL13_6G_C0,
    NVRAM_PARA_SAR_LVL14_6G_C0,
    NVRAM_PARA_SAR_LVL15_6G_C0,
    NVRAM_PARA_SAR_LVL16_6G_C0,
    NVRAM_PARA_SAR_LVL17_6G_C0,
    NVRAM_PARA_SAR_LVL18_6G_C0,
    NVRAM_PARA_SAR_LVL19_6G_C0,
    NVRAM_PARA_SAR_LVL20_6G_C0,
    NVRAM_PARA_6G_C0_SAR_END_INDEX_BUTT,

    /* 6G C1 SAR CONTROL */
    NVRAM_PARA_6G_C1_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_6G_C1 = NVRAM_PARA_6G_C1_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_6G_C1,
    NVRAM_PARA_SAR_LVL3_6G_C1,
    NVRAM_PARA_SAR_LVL4_6G_C1,
    NVRAM_PARA_SAR_LVL5_6G_C1,
    NVRAM_PARA_SAR_LVL6_6G_C1,
    NVRAM_PARA_SAR_LVL7_6G_C1,
    NVRAM_PARA_SAR_LVL8_6G_C1,
    NVRAM_PARA_SAR_LVL9_6G_C1,
    NVRAM_PARA_SAR_LVL10_6G_C1,
    NVRAM_PARA_SAR_LVL11_6G_C1,
    NVRAM_PARA_SAR_LVL12_6G_C1,
    NVRAM_PARA_SAR_LVL13_6G_C1,
    NVRAM_PARA_SAR_LVL14_6G_C1,
    NVRAM_PARA_SAR_LVL15_6G_C1,
    NVRAM_PARA_SAR_LVL16_6G_C1,
    NVRAM_PARA_SAR_LVL17_6G_C1,
    NVRAM_PARA_SAR_LVL18_6G_C1,
    NVRAM_PARA_SAR_LVL19_6G_C1,
    NVRAM_PARA_SAR_LVL20_6G_C1,
    NVRAM_PARA_6G_C1_SAR_END_INDEX_BUTT,

    /* 6G C2 SAR CONTROL */
    NVRAM_PARA_6G_C2_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_6G_C2 = NVRAM_PARA_6G_C2_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_6G_C2,
    NVRAM_PARA_SAR_LVL3_6G_C2,
    NVRAM_PARA_SAR_LVL4_6G_C2,
    NVRAM_PARA_SAR_LVL5_6G_C2,
    NVRAM_PARA_SAR_LVL6_6G_C2,
    NVRAM_PARA_SAR_LVL7_6G_C2,
    NVRAM_PARA_SAR_LVL8_6G_C2,
    NVRAM_PARA_SAR_LVL9_6G_C2,
    NVRAM_PARA_SAR_LVL10_6G_C2,
    NVRAM_PARA_SAR_LVL11_6G_C2,
    NVRAM_PARA_SAR_LVL12_6G_C2,
    NVRAM_PARA_SAR_LVL13_6G_C2,
    NVRAM_PARA_SAR_LVL14_6G_C2,
    NVRAM_PARA_SAR_LVL15_6G_C2,
    NVRAM_PARA_SAR_LVL16_6G_C2,
    NVRAM_PARA_SAR_LVL17_6G_C2,
    NVRAM_PARA_SAR_LVL18_6G_C2,
    NVRAM_PARA_SAR_LVL19_6G_C2,
    NVRAM_PARA_SAR_LVL20_6G_C2,
    NVRAM_PARA_6G_C2_SAR_END_INDEX_BUTT,

    /* 6G C3 SAR CONTROL */
    NVRAM_PARA_6G_C3_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL1_6G_C3 = NVRAM_PARA_6G_C3_SAR_START_INDEX,
    NVRAM_PARA_SAR_LVL2_6G_C3,
    NVRAM_PARA_SAR_LVL3_6G_C3,
    NVRAM_PARA_SAR_LVL4_6G_C3,
    NVRAM_PARA_SAR_LVL5_6G_C3,
    NVRAM_PARA_SAR_LVL6_6G_C3,
    NVRAM_PARA_SAR_LVL7_6G_C3,
    NVRAM_PARA_SAR_LVL8_6G_C3,
    NVRAM_PARA_SAR_LVL9_6G_C3,
    NVRAM_PARA_SAR_LVL10_6G_C3,
    NVRAM_PARA_SAR_LVL11_6G_C3,
    NVRAM_PARA_SAR_LVL12_6G_C3,
    NVRAM_PARA_SAR_LVL13_6G_C3,
    NVRAM_PARA_SAR_LVL14_6G_C3,
    NVRAM_PARA_SAR_LVL15_6G_C3,
    NVRAM_PARA_SAR_LVL16_6G_C3,
    NVRAM_PARA_SAR_LVL17_6G_C3,
    NVRAM_PARA_SAR_LVL18_6G_C3,
    NVRAM_PARA_SAR_LVL19_6G_C3,
    NVRAM_PARA_SAR_LVL20_6G_C3,
    NVRAM_PARA_6G_C3_SAR_END_INDEX_BUTT,
#endif
    NVRAM_PARA_INDEX_0,
    NVRAM_PARA_INDEX_1,
    NVRAM_PARA_INDEX_2,
    NVRAM_PARA_INDEX_3,
    NVRAM_PARA_INDEX_4,
    NVRAM_PARA_INDEX_5,
    NVRAM_PARA_INDEX_6,
    NVRAM_PARA_INDEX_7,
    NVRAM_PARA_INDEX_8,
    NVRAM_PARA_INDEX_9,
    NVRAM_PARA_INDEX_10,
    NVRAM_PARA_INDEX_11,
    NVRAM_PARA_INDEX_12,
    NVRAM_PARA_INDEX_13,
    NVRAM_PARA_INDEX_14,
    NVRAM_PARA_INDEX_15,
    NVRAM_PARA_INDEX_16,
    NVRAM_PARA_INDEX_17,
    NVRAM_PARA_INDEX_18,
    NVRAM_PARA_INDEX_19,
    NVRAM_PARA_INDEX_20,
    /* 5G 功率与IQ校准UPC上限值 */
    NVRAM_PARA_INDEX_IQ_MAX_UPC,
    NVRAM_PARA_BACKOFF_POW,
    /* 2G 低功率修正值 */
    NVRAM_PARA_DSSS_2G_LOW_POW_AMEND,
    NVRAM_PARA_OFDM_2G_LOW_POW_AMEND,
    NVRAM_PARA_TXPWR_INDEX_BUTT,
    NVRAM_PARA_BASE_TXPWR_2G_C0, /* 2.4g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_2G_C1, /* 2.4g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_2G_C2, /* 2.4g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_2G_C3, /* 2.4g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_5G_C0, /* 5g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_5G_C1, /* 5g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_5G_C2, /* 5g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_5G_C3, /* 5g基准发射功率 */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_BASE_TXPWR_6G_C0, /* 6g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_6G_C1, /* 6g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_6G_C2, /* 6g基准发射功率 */
    NVRAM_PARA_BASE_TXPWR_6G_C3, /* 6g基准发射功率 */
#endif
    NVRAM_PARA_BASE_INDEX_BUTT,
    /* MIMO基准发射功率差值 */
    NVRAM_PARA_2G_DELT_BASE_POWER_23_2, /* 2G 2天线 */
    NVRAM_PARA_2G_DELT_BASE_POWER_23_3, /* 2G 3天线 */
    NVRAM_PARA_2G_DELT_BASE_POWER_23_4, /* 2G 4天线 */
    NVRAM_PARA_5G_DELT_BASE_POWER_23_2, /* 5G 2天线 */
    NVRAM_PARA_5G_DELT_BASE_POWER_23_3, /* 5G 3天线 */
    NVRAM_PARA_5G_DELT_BASE_POWER_23_4, /* 5G 4天线 */
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_6G_DELT_BASE_POWER_23_2, /* 6G 2天线 */
    NVRAM_PARA_6G_DELT_BASE_POWER_23_3, /* 6G 3天线 */
    NVRAM_PARA_6G_DELT_BASE_POWER_23_4, /* 6G 4天线 */
#endif
    NVRAM_PARA_INDEX_24_RSV,

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    NVRAM_PARA_TAS_ANT_SWITCH_EN,
    /* TAS PWR CONTROL */
    NVRAM_PARA_TAS_PWR_CTRL_2G,
    NVRAM_PARA_TAS_PWR_CTRL_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_TAS_PWR_CTRL_6G,
#endif
#endif
    NVRAM_PARA_5G_FCC_CE_HIGH_BAND_MAX_PWR,

    NVRAM_PARA_TPC_RU_POWER_2G_20M,
    NVRAM_PARA_TPC_RU_POWER_2G_40M,
    NVRAM_PARA_TPC_RU_POWER_5G_20M,
    NVRAM_PARA_TPC_RU_POWER_5G_40M,
    NVRAM_PARA_TPC_RU_POWER_5G_80M,
    NVRAM_PARA_TPC_RU_POWER_5G_160M,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_TPC_RU_POWER_6G_20M,
    NVRAM_PARA_TPC_RU_POWER_6G_40M,
    NVRAM_PARA_TPC_RU_POWER_6G_80M,
    NVRAM_PARA_TPC_RU_POWER_6G_160M,
#endif
    NVRAM_PARA_TPC_RU_MAX_POWER_2G_C0,
    NVRAM_PARA_TPC_RU_MAX_POWER_2G_C1,
    NVRAM_PARA_TPC_RU_MAX_POWER_2G_C2,
    NVRAM_PARA_TPC_RU_MAX_POWER_2G_C3,
    NVRAM_PARA_TPC_RU_MAX_POWER_5G_C0,
    NVRAM_PARA_TPC_RU_MAX_POWER_5G_C1,
    NVRAM_PARA_TPC_RU_MAX_POWER_5G_C2,
    NVRAM_PARA_TPC_RU_MAX_POWER_5G_C3,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_TPC_RU_MAX_POWER_6G_C0,
    NVRAM_PARA_TPC_RU_MAX_POWER_6G_C1,
    NVRAM_PARA_TPC_RU_MAX_POWER_6G_C2,
    NVRAM_PARA_TPC_RU_MAX_POWER_6G_C3,
#endif
    NVRAM_PARA_TPC_RU_MAX_POWER_2G,
    NVRAM_PARA_TPC_RU_MAX_POWER_5G,
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    NVRAM_PARA_TPC_RU_MAX_POWER_6G,
#endif
    NVRAM_PARA_PWR_INDEX_BUTT,
} wlan_nvram_cfg_enum;

#endif  // hisi_customize_wifi_nvram.h