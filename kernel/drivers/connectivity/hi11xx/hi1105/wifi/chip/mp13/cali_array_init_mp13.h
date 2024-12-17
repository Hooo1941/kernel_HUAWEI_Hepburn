/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : mp13校准算法实现
 * 作者       : wifi
 * 创建日期   : 2022年9月29日
 */

#ifndef CALI_ARRAY_INIT_MP13_H
#define CALI_ARRAY_INIT_MP13_H

#include "hmac_cali_mgmt.h"
#include "wlan_cali.h"

extern int16_t g_txiq_h_comp_160m[MP13_CALI_TXIQ_LS_FILTER_TAP_NUM][MP13_CALI_TXIQ_LS_FILTER_FEQ_NUM_160M];
extern int16_t g_txiq_h_comp_320m[MP13_CALI_TXIQ_LS_FILTER_TAP_NUM][MP13_CALI_TXIQ_LS_FILTER_FEQ_NUM_320M];
extern int16_t g_txiq_h_comp_640m[MP13_CALI_TXIQ_LS_FILTER_TAP_NUM][MP13_CALI_TXIQ_LS_FILTER_FEQ_NUM_640M];
extern mp15_complex_stru g_mp15_rxiq_h_comp_40m[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_40M];
extern mp15_complex_stru g_mp15_rxiq_h_comp_80m[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_80M];
extern mp15_complex_stru g_mp15_rxiq_40m_h_comp_160m[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_160M];
extern mp15_complex_stru g_mp15_rxiq_h_comp_80m_fpga[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_80M_FPGA];
extern mp15_complex_stru g_mp15_rxiq_h_comp_160m[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_160M];
extern mp15_complex_stru g_mp15_rxiq_h_comp_320m[MP15_RXIQ_LSFIR_NUM][MP15_RXIQ_LSFIR_FEQ_NUM_320M];
extern mp15_complex_stru g_mp15_txiq_h_comp_80m[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_80M];
extern mp15_complex_stru g_mp15_txiq_h_comp_160m[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_160M];
extern mp15_complex_stru g_mp15_2g_txiq_h_comp_320m[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_320M];
extern mp15_complex_stru g_mp15_txiq_h_comp_160m_fpga[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_160M_FPGA];
extern mp15_complex_stru g_mp15_txiq_h_comp_320m[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_320M];
extern mp15_complex_stru g_mp15_txiq_h_comp_640m[MP15_TXIQ_LSFIR_NUM][MP15_TXIQ_LSFIR_FEQ_NUM_640M];
extern uint32_t g_corram_data_20m[DPD_CORRAM_DATA_NUM];
extern uint32_t g_corram_data_40m[DPD_CORRAM_DATA_NUM];
extern uint32_t g_corram_data_80m[DPD_CORRAM_DATA_NUM];
#endif // cali_array_init_mp13.h
