/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : gf61产测pow\rssi\dpd校准NV接口声明
 * 作者       : wifi
 * 创建日期   : 2020年12月8日
 */

#ifndef HISI_CONN_NVE_INTERFACE_GF61_H
#define HISI_CONN_NVE_INTERFACE_GF61_H


/* 其他头文件包含 */
#include "hisi_conn_nve.h"
#include "wlan_cali.h"
#include "wlan_types.h"
#include "oam_wdk.h"
#include "wlan_oneimage_define.h"
#include "hisi_conn_nve_interface.h"
#ifdef HISI_CONN_NVE_SUPPORT
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CONN_NVE_INTERFACE_GF61_H

#define WICAL_STREAM_NUM_GF61       2

/* nv项相关 */
#define NV_NAME_MAXLENGTH 15     /* NV name maximum length */
#define NVE_NV_DATA_SIZE 104 /* NV data maximum length */
#define GF61_TOTAL_ALLOC_NV_NUM           14 /* wifi功率校准的nv数 */
#define GF61_NV_WIC0_2G5G_OFDM_NAME   "txdif1"
#define GF61_NV_UNUSED_NAME           "txdif2"
#define GF61_NV_WIC0C1_CCK_NAME       "txdif3"
#define GF61_NV_WIC1_2G5G_OFDM_NAME   "Standby"
#define GF61_NV_WIC0_5G_LOW_NAME      "Standby"
#define GF61_NV_WIC1_5G_LOW_NAME      "Standby"
#define GF61_NV_WIC0_2G40M_NAME       "xodiff"
#define GF61_NV_WIC1_5G160M_NAME      "Standby"
#define GF61_NV_WIC0_2GMUOFDM_NAME    "Standby"
#define GF61_NV_WIC1_2GMUOFDM_NAME    "Standby"
#define GF61_NV_WIC0_2GMU40M_NAME     "Standby"
#define GF61_NV_WIC1_2GMU40M_NAME     "Standby"
#define GF61_NV_WIC0_MUCCK_NAME       "Standby"
#define GF61_NV_WIC1_MUCCK_NAME       "Standby"
#define GF61_NV_WIC1_2G40M_NAME       "Standby"

#define NV_WRITE 0 /* NV write operation */
#define NV_READ 1  /* NV read  operation */

enum gf61_nv_number {
    GF61_2G_5G_OFDM_C0_NUM = 340,
    GF61_UNUSED_BAK = 363,
    GF61_CKK_CO_C1_NUM = 367,
    GF61_2G_5G_OFDM_C1_NUM,
    GF61_5G_LOW_C0_NUM,
    GF61_5G_LOW_C1_NUM,
    GF61_2G40_OFDM_C0_NUM = 384,
    GF61_2G40_OFDM_C1_NUM,
    GF61_5G160_C0_NUM,
    GF61_5G160_C1_NUM,
    GF61_MUFREQ_2G20_C0_NUM = 396,
    GF61_MUFREQ_2G20_C1_NUM,
    GF61_MUFREQ_2G40_C0_NUM,
    GF61_MUFREQ_2G40_C1_NUM,
    GF61_MUFREQ_CCK_C0_NUM,
    GF61_MUFREQ_CCK_C1_NUM,
};

typedef struct {
    int8_t delta_gain[WICAL_POW_CAL_LVL_NUM];
} wlan_cali_pow_para_stru_gf61;

typedef struct {
    wlan_cali_pow_para_stru_gf61 delta_gain_2g_11b[WICAL_STREAM_NUM_GF61][WICAL_2G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_2g_ofdm_20[WICAL_STREAM_NUM_GF61][WICAL_2G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_2g_ofdm_40[WICAL_STREAM_NUM_GF61][WICAL_2G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_5g_20[WICAL_STREAM_NUM_GF61][WICAL_5G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_5g_40[WICAL_STREAM_NUM_GF61][WICAL_5G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_5g_80[WICAL_STREAM_NUM_GF61][WICAL_5G_RF_BAND_NUM];
    wlan_cali_pow_para_stru_gf61 delta_gain_5g_160[WICAL_STREAM_NUM_GF61][WICAL_5G_RF_BAND_NUM];
    uint32_t crc32;
} nv_pow_stru_gf61;

enum nv_op {
    TEL_HUAWEI_NV_WRITE = 0,
    TEL_HUAWEI_NV_READ,
    TEL_HUAWEI_NV_DELETE,
};

/* 变量声明 */
#endif
extern uint16_t g_xo_pwr_diff[3]; // 1位寄存器值，2位crc校验值;
#ifdef HISI_CONN_NVE_SUPPORT
extern nv_pow_stru_gf61 g_pow_cail;
/* 函数声明 */
int32_t write_cali_data_to_nv(uint8_t *data, int32_t size);
int32_t read_cali_data_from_nvram(uint8_t *pc_out, int32_t size);
int32_t conn_nve_write_powcal_data_pro_gf61(const nv_pow_user_info_stru *info);
#endif
#endif
