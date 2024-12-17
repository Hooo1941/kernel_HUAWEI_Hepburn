/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2020年5月20日
 */

/* 头文件包含 */
#include "hisi_customize_wifi_mp16.h"
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve_interface.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#endif
#include "wal_linux_customize.h"
#include "hisi_customize_wifi.h"
#include "hisi_ini.h"
#include "plat_type.h"
#include "oam_ext_if.h"
#include "wlan_spec.h"
#include "oal_sdio_comm.h"
#include "oal_hcc_host_if.h"
#include "oal_main.h"
#include "hisi_ini.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "oam_ext_if.h"

#include "wlan_spec.h"
#include "wlan_chip_i.h"

#include "mac_hiex.h"

#include "wal_config.h"

#include "hmac_auto_adjust_freq.h"
#include "hmac_cali_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_tcp_ack_buf.h"
#include "hmac_tx_amsdu.h"
#include "hmac_rx_data.h"

#ifdef HISI_CONN_NVE_SUPPORT
/* 终端头文件 */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#define hisi_nve_direct_access(...)  0
#else
#include <linux/mtd/hisi_nve_interface.h>
#endif
#endif
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
#include "fe_extern_if.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CUSTOMIZE_WIFI_HI1106_C

/* 功率定制化参数 */
wlan_cust_pow_stru g_cust_pow = {
    .fcc_ce_param = {
        {
            .fcc_ce_5g_high_band_max_pwr = CUS_MAX_TXPOWER_MAX,
            .rf_id = WLAN_SINGLE_STREAM_0,
        },
        {
            .fcc_ce_5g_high_band_max_pwr = CUS_MAX_TXPOWER_MAX,
            .rf_id = WLAN_SINGLE_STREAM_1,
        },
        {
            .fcc_ce_5g_high_band_max_pwr = CUS_MAX_TXPOWER_MAX,
            .rf_id = WLAN_SINGLE_STREAM_2,
        },
        {
            .fcc_ce_5g_high_band_max_pwr = CUS_MAX_TXPOWER_MAX,
            .rf_id = WLAN_SINGLE_STREAM_3,
        }
    },
    .txpwr_base_2g_params = {
        {CUS_BASE_POW_VAL}, {CUS_BASE_POW_VAL}, {CUS_BASE_POW_VAL}, {CUS_BASE_POW_VAL}
    },
    .txpwr_base_5g_params = {
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL}
    },
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    .txpwr_base_6g_params = {
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
        {CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL,
         CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL, CUS_BASE_POW_VAL},
    },
#endif
};

/* 动态功率校准制化参数 */
mac_cust_dyn_pow_sru g_cust_dyn_pow = {
    .dpn_para = {
        { { { { 0 } } } },
    },
};
/* 校准相关制化参数 */
mac_cus_rf_cali_stru g_cust_rf_cali = {
    .dyn_cali_dscr_interval = {0}, /* 动态校准开关2.4g 5g 6g */
    .band_5g_enable = 1,
};
/* RF前端相关制化参数 */
mac_customize_rf_front_sru g_cust_rf_front = {
    .rf_loss_gain_db = {
        .gain_db_2g = {{0xF4, 0xF4, 0xF4, 0xF4}, {0xF4, 0xF4, 0xF4, 0xF4}, {0xF4, 0xF4, 0xF4, 0xF4}},
        .gain_db_5g = {{0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        .gain_db_6g = {{0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8},
            {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}, {0xF8, 0xF8, 0xF8, 0xF8}},
#endif
    },
    .ext_rf = {
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        {
            .lna_bypass_gain_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .lna_gain_db = {0x14, 0x14, 0x14, 0x14},
            .pa_gain_b0_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .pa_gain_b1_db = {0xF4, 0xF4, 0xF4, 0xF4},
            .ext_switch_isexist = {1, 1, 1, 1},
            .ext_pa_isexist = {1, 1, 1, 1},
            .ext_lna_isexist = {1, 1, 1, 1},
            .lna_on2off_time_ns = {0x0276, 0x0276, 0x0276, 0x0276},
            .lna_off2on_time_ns = {0x0140, 0x0140, 0x0140, 0x0140},
        },
#endif
    }
};
#ifdef HISI_CONN_NVE_SUPPORT
static conn_nve_particion_stru g_cust_nvram_info = {}; /* NVRAM数据 */

static wlan_cust_dpn_cfg_cmd g_wifi_cfg_dpn_nv[] = {
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M])},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M])},
#endif
    /* 低功率段dpn */
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_11b[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_TWO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_THREE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_20[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_40[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_80[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_TWO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_5g_160[WLAN_RF_CHANNEL_THREE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_5g[WLAN_BW_CAP_160M])},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_20[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_40[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_80[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_ZERO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_ONE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_TWO][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_TWO].dpn_6g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M][WLAN_6G_BAND_IDX0],
     &g_cust_nvram_info.pow_cal_data.delta_gain_6g_160[WLAN_RF_CHANNEL_THREE][WLAN_6G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_THREE].dpn_6g[WLAN_BW_CAP_160M])},
#endif
};
#endif
oal_bool_enum_uint8 hwifi_get_cust_read_status(int32_t cus_tag, int32_t cfg_id)
{
    wlan_cust_cfg_cmd *pgast_wifi_config = NULL;
    int32_t cust_cfg_butt = 0;

    switch (cus_tag) {
        case CUS_TAG_HOST:
            pgast_wifi_config = g_wifi_cfg_host;
            cust_cfg_butt = WLAN_CFG_HOST_BUTT;
            break;
        case CUS_TAG_POW:
            pgast_wifi_config = g_wifi_cfg_pow;
            cust_cfg_butt = NVRAM_PARA_PWR_INDEX_BUTT;
            break;
        case CUS_TAG_RF:
            pgast_wifi_config = g_wifi_cfg_rf_cali;
            cust_cfg_butt = WLAN_CFG_RF_CALI_BUTT;
            break;
        case CUS_TAG_DYN_POW:
            pgast_wifi_config = g_wifi_cfg_dyn_pow;
            cust_cfg_butt = WLAN_CFG_DYN_POW_PARAMS_BUTT;
            break;
        case CUS_TAG_CAP:
            pgast_wifi_config = g_wifi_cfg_cap;
            cust_cfg_butt = WLAN_CFG_CAP_BUTT;
            break;

        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return OAL_FALSE;
    }

    if (cfg_id >= cust_cfg_butt) {
        return OAL_FALSE;
    }
    return pgast_wifi_config[cfg_id].en_value_state;
}

/*
 * 函 数 名  : hwifi_config_init_etc
 * 功能描述  : 定制化参数赋值接口
 *             读取ini文件，更新 host_init_params_etc 全局数组
 */
int32_t hwifi_set_cust_list_val(int32_t *param_list, uint16_t list_len, uint8_t *data_addr, uint8_t data_size)
{
    uint8_t param_idx;
    if (data_size > sizeof(int)) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_cpy_cust_val: data size[%d] not support!", data_size);
        oal_io_print("hwifi_cpy_cust_val: data size[%d] not support!", data_size);
        return INI_FAILED;
    }

    for (param_idx = 0; param_idx < list_len; param_idx++) {
        if (memcpy_s(data_addr, data_size, &param_list[param_idx], data_size) != EOK) {
            return INI_FAILED;
        }
        data_addr += data_size;
    }
    return INI_SUCC;
}

/*
 * 函 数 名  : hwifi_get_cust_item_list
 * 功能描述  : 从字符串中分割定制化项列表值
 * 参数  :  cust_param 字符串参数
            param_list 定制化参数列表
            list_len 参数列表长度
            max_len  列表最大长度

 */
static uint32_t hwifi_get_cust_item_list(uint8_t *cust_param, int32_t *param_list,
    uint16_t *list_len, uint16_t max_len)
{
    int8_t *pc_token = NULL;
    int8_t *pc_ctx = NULL;
    int8_t *pc_end = ";";
    int8_t *pc_sep = ",";
    uint16_t us_param_num = 0;
    uint8_t cust_param_str[CUS_PARAMS_LEN_MAX] = {0};
    int32_t ret;

    if (memcpy_s(cust_param_str, CUS_PARAMS_LEN_MAX, cust_param, OAL_STRLEN(cust_param)) != EOK) {
        return OAL_FAIL;
    }

    pc_token = oal_strtok(cust_param_str, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_get_cust_item_list read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        if (us_param_num == max_len) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "hwifi_get_cust_item_list::nv or ini param is too many idx[%d] Max[%d]", us_param_num, max_len);
            return OAL_FAIL;
        }

        /* 将字符串转换成数字 */
        if (!strncmp(pc_token, "0x", strlen("0x")) || !strncmp(pc_token, "0X", strlen("0X"))) {
            ret = sscanf_s(pc_token, "%x", &param_list[us_param_num]);
        } else {
            ret = sscanf_s(pc_token, "%d", &param_list[us_param_num]);
        }

        if (ret <= 0) {
            return OAL_FAIL;
        }

        pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    *list_len = us_param_num;
    return OAL_SUCC;
}
/*
 * 函 数 名  : hwifi_config_get_cust_val
 * 功能描述  : 定制化项参数读取
 */
int32_t hwifi_config_get_cust_val(wlan_cust_cfg_cmd *cfg_cmd)
{
    int32_t ret;
    uint8_t cust_params[CUS_PARAMS_LEN_MAX] = {0};
    uint16_t us_per_param_num = 0;
    int32_t nv_params[CUS_PARAMS_LEN_MAX] = {0};

    if ((cfg_cmd->data_addr == NULL) || (cfg_cmd->name == NULL) || cfg_cmd->data_num == 0) {
        oam_info_log2(0, OAM_SF_CUSTOM, "hwifi_config_get_cust_val::name NULL cfg id[%d] data NULL or no data[%d]!",
            cfg_cmd->case_entry, cfg_cmd->data_num);
        return INI_FAILED;
    }

    ret = get_cust_conf_string(INI_MODU_WIFI, cfg_cmd->name, cust_params, CUS_PARAMS_LEN_MAX - 1);
    if (ret == INI_SUCC) {
        if (hwifi_get_cust_item_list(cust_params, nv_params, &us_per_param_num, cfg_cmd->data_num) == OAL_SUCC) {
            ret = hwifi_set_cust_list_val(nv_params, us_per_param_num, cfg_cmd->data_addr, cfg_cmd->data_type);
        } else {
            ret = INI_FAILED;
        }

        return ret;
    } else {
        oal_io_print("hwifi_config_get_cust_val::tag %s read %s failed ret[%d] check ini files!",
            INI_MODU_WIFI, cfg_cmd->name, ret);
    }

    return ret;
}

int8_t hwifi_check_pwr_ref_delta_val(int8_t pwr_ref_delta)
{
    int32_t ret;
    if (pwr_ref_delta > CUST_PWR_REF_DELTA_HI) {
        ret = CUST_PWR_REF_DELTA_HI;
    } else if (pwr_ref_delta < CUST_PWR_REF_DELTA_LO) {
        ret = CUST_PWR_REF_DELTA_LO;
    } else {
        ret = pwr_ref_delta;
    }
    return ret;
}

int32_t hwifi_mp16_uint_val_check(wlan_cust_range_stru *cust_check, uint8_t data_type)
{
    uint32_t val = 0;
    uint32_t read_val, len;
    uint32_t max_val = (uint32_t)cust_check->max_val;
    uint32_t min_val = (uint32_t)cust_check->min_val;
    uint32_t default_val = (uint32_t)cust_check->default_val;
    uint8_t *addr = (uint8_t *)cust_check->data_addr;

    if (data_type == CUST_CFG_UINT8) {
        len = sizeof(uint8_t);
    } else if (data_type == CUST_CFG_UINT16) {
        len = sizeof(uint16_t);
    } else {
        len = sizeof(uint32_t);
    }
    if (memcpy_s(&val, len, addr, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_uint_val_check:: memory copy fail!}");
        return OAL_FAIL;
    }
    read_val = val;

    switch (cust_check->check_type) {
        case CUST_CHECK_MAX:
            cus_check_max_val(val, max_val, default_val);
            break;
        case CUST_CHECK_MIN:
            cus_check_min_val(val, min_val, default_val);
            break;
        case CUST_CHECK_VAL:
            cus_check_val(val, max_val, min_val, default_val);
            break;
        default:
            oam_error_log3(0, OAM_SF_ANY, "{hwifi_uint_val_check:: check type[%d] max %d min %d not support!}",
                cust_check->check_type, cust_check->max_val, cust_check->min_val);
            break;
    }

    if (memcpy_s(addr, len, &val, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_uint_val_check:: memory copy fail!}");
        return OAL_FAIL;
    }
    if (read_val != val) {
        oam_error_log4(0, OAM_SF_ANY, "{hwifi_uint_val_check::read_val[%x] val [%x] max val[%x] min val [%x]!}",
            read_val, val, max_val, min_val);

        return OAL_FAIL;
    }
    return OAL_SUCC;
}

int32_t hwifi_mp16_int_val_check(wlan_cust_range_stru *cust_check, uint8_t data_type)
{
    int32_t val, read_val;
    int32_t max_val = (int32_t)cust_check->max_val;
    int32_t min_val = (int32_t)cust_check->min_val;
    int32_t default_val = (int32_t)cust_check->default_val;
    uint32_t len;
    uint8_t *addr = (uint8_t *)cust_check->data_addr;
    if (data_type == CUST_CFG_INT8) {
        len = sizeof(int8_t);
        val = (int32_t)(*((int8_t *)(cust_check->data_addr)));
    } else if (data_type == CUST_CFG_INT16) {
        len = sizeof(int16_t);
        val = (int32_t)(*((int16_t *)(cust_check->data_addr)));
    } else {
        len = sizeof(int32_t);
        val = (int32_t)(*((int32_t *)(cust_check->data_addr)));
    }
    read_val = val;

    switch (cust_check->check_type) {
        case CUST_CHECK_MAX:
            cus_check_max_val(val, max_val, default_val);
            break;
        case CUST_CHECK_MIN:
            cus_check_min_val(val, min_val, default_val);
            break;
        case CUST_CHECK_VAL:
            cus_check_val(val, max_val, min_val, default_val);
            break;
        default:
            oam_error_log3(0, OAM_SF_ANY, "{hwifi_int_val_check:: check type[%d] max %d min %d not support!}",
                cust_check->check_type, cust_check->max_val, cust_check->min_val);
            break;
    }

    if (memcpy_s(addr, len, (uint8_t *)&val, len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{hwifi_int_val_check::memory copy fail!}");
        return OAL_FAIL;
    }
    if (read_val != val) {
        oam_error_log4(0, OAM_SF_ANY, "{hwifi_int_val_check::read_val[%d] val [%d] max val[%d] min val [%d]!}",
            read_val, val, max_val, min_val);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

static void hwifi_mp16_cust_val_tbl_check(wlan_cust_range_stru *range_tbl, uint32_t len)
{
    uint32_t data_idx;
    int32_t ret = OAL_SUCC;

    for (data_idx = 0; data_idx < len; data_idx++) {
        switch (range_tbl[data_idx].data_type) {
            case CUST_CFG_INT8:
            case CUST_CFG_INT16:
            case CUST_CFG_INT32:
                ret = hwifi_mp16_int_val_check(&range_tbl[data_idx], range_tbl[data_idx].data_type);
                break;

            case CUST_CFG_UINT8:
            case CUST_CFG_UINT16:
            case CUST_CFG_UINT32:
                ret = hwifi_mp16_uint_val_check(&range_tbl[data_idx], range_tbl[data_idx].data_type);
                break;

            default:
                oam_error_log4(0, OAM_SF_ANY,
                    "{hwifi_cust_val_tbl_check::data idx[%d] check type[%d] max %d min %d not support!}",
                    data_idx, range_tbl[data_idx].check_type, range_tbl[data_idx].max_val,
                    range_tbl[data_idx].min_val);
                break;
        }
        if (ret != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{hwifi_cust_val_tbl_check::data idx[%d] set invalid value!}",
                data_idx);
        }
    }
}
/* 定制化项有效范围检查 */
STATIC wlan_cust_range_stru g_host_cfg_range[] = {
    {&g_wlan_cust.uc_roam_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.uc_roam_scan_orthogonal, CUST_CFG_UINT8, CUST_CHECK_MIN,
        0, 1, CUS_ROAM_SCAN_ORTHOGONAL_DEFAULT},
    {&g_wlan_cust.ampdu_tx_max_num, CUST_CFG_UINT32, CUST_CHECK_VAL,
        WLAN_AMPDU_TX_MAX_NUM_MP16, 1, WLAN_AMPDU_TX_MAX_NUM_MP16},
    {&g_cust_host.freq_lock.en_irq_affinity, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.freq_lock.throughput_irq_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_LOW},
    {&g_cust_host.freq_lock.throughput_irq_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_HIGH},
    {&g_cust_host.freq_lock.irq_pps_low, CUST_CFG_UINT32, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_PPS_TH_LOW},
    {&g_cust_host.freq_lock.irq_pps_high, CUST_CFG_UINT32, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_PPS_TH_HIGH},
    {&g_cust_host.tx_ampdu.ampdu_hw_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.large_amsdu.large_amsdu_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.large_amsdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_LOW},
    {&g_cust_host.large_amsdu.throughput_middle, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_MIDDLE},
    {&g_cust_host.large_amsdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_AMSDU_AMPDU_TH_HIGH},
    {&g_cust_host.tx_ampdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_HW_AMPDU_TH_HIGH},
    {&g_cust_host.tx_ampdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_HW_AMPDU_TH_LOW},
    {&g_cust_host.tcp_ack_filt.filter_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.tcp_ack_filt.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_IRQ_TH_LOW},
    {&g_cust_host.tcp_ack_filt.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_FILTER_TH_HIGH},
    {&g_cust_host.small_amsdu.en_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.small_amsdu.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_HIGH},
    {&g_cust_host.small_amsdu.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_LOW},
    {&g_cust_host.small_amsdu.pps_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_PPS_LOW},
    {&g_cust_host.small_amsdu.pps_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_SMALL_AMSDU_PPS_HIGH},
    {&g_cust_host.tcp_ack_buf.host_buf_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.tcp_ack_buf.device_buf_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.tcp_ack_buf.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH},
    {&g_cust_host.tcp_ack_buf.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW},
    {&g_cust_host.tcp_ack_buf.throughput_high_40m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_40M},
    {&g_cust_host.tcp_ack_buf.throughput_low_40m, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW_40M},
    {&g_cust_host.tcp_ack_buf.throughput_high_80m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_80M},
    {&g_cust_host.tcp_ack_buf.throughput_low_80m, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_LOW_80M},
    {&g_cust_host.tcp_ack_buf.throughput_high_160m, CUST_CFG_UINT16,
     CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_HIGH_160M},
    {&g_cust_host.tcp_ack_buf.buf_userctl_test_en, CUST_CFG_UINT16, CUST_CHECK_MIN, OAL_TRUE, OAL_FALSE, OAL_FALSE},
    {&g_cust_host.tcp_ack_buf.buf_userctl_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_USERCTL_HIGH},
    {&g_cust_host.tcp_ack_buf.buf_userctl_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_TCP_ACK_BUF_USERCTL_LOW},
    {&g_wlan_cust.uc_random_mac_addr_scan, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.uc_disable_capab_2ght40, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH], CUST_CFG_UINT8,
     CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_wlan_cust.lte_gpio_check_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.dyn_extlna.switch_en, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
    {&g_cust_host.dyn_extlna.throughput_high, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_RX_DYN_BYPASS_EXTLNA_HIGH},
    {&g_cust_host.dyn_extlna.throughput_low, CUST_CFG_UINT16, CUST_CHECK_MIN, 0, 1, WLAN_RX_DYN_BYPASS_EXTLNA_LOW},
    {&g_cust_host.wlan_11ac2g_switch, CUST_CFG_UINT8, CUST_CHECK_VAL, OAL_TRUE, OAL_FALSE, OAL_TRUE},
};
static void hwifi_host_cust_check_mp16(void)
{
    hwifi_mp16_cust_val_tbl_check(g_host_cfg_range, oal_array_size(g_host_cfg_range));
}
void hwifi_cust_rf_rssi_check_mp16(void)
{
    uint8_t chan_idx, rf_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (chan_idx = 0; chan_idx < CUS_NUM_2G_DELTA_RSSI_NUM; ++chan_idx) {
            g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[chan_idx][rf_idx] =
                hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_2g[chan_idx][rf_idx]);
        }
        for (chan_idx = 0; chan_idx < CUS_NUM_5G_DELTA_RSSI_NUM; ++chan_idx) {
                g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[chan_idx][rf_idx] =
                    hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_5g[chan_idx][rf_idx]);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
                g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[chan_idx][rf_idx] =
                    hwifi_check_pwr_ref_delta_val(g_cust_rf_front.delta_pwr_ref.delta_rssi_6g[chan_idx][rf_idx]);
#endif
        }
    }

    /* RSSI amend */
    for (chan_idx = 0; chan_idx < CUS_NUM_2G_AMEND_RSSI_NUM; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            cus_check_val(g_cust_rf_front.rssi_amend_cfg.amend_rssi_2g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
        }
    }

    for (chan_idx = 0; chan_idx < CUS_NUM_5G_AMEND_RSSI_NUM; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            cus_check_val(g_cust_rf_front.rssi_amend_cfg.amend_rssi_5g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
            cus_check_val(g_cust_rf_front.rssi_amend_cfg.amend_rssi_6g[chan_idx][rf_idx], WLAN_RF_RSSI_AMEND_TH_HIGH,
                WLAN_RF_RSSI_AMEND_TH_LOW, 0x0);
#endif
        }
    }
}

void hwifi_cust_rf_lp_loss_check_mp16(void)
{
    uint8_t chan_idx;
    for (chan_idx = 0; chan_idx < MAC_NUM_2G_BAND; ++chan_idx) {
        /* 获取各2p4g 各band 0.25db精度的线损值 */
        cus_check_val(g_cust_cap.rf_lp_loss.gain_db_2g[chan_idx], RF_LP_LOSS_GAIN_DB_MAX,
            RF_LP_LOSS_GAIN_DB_MIN, 0x0);
    }

    for (chan_idx = 0; chan_idx < MAC_NUM_5G_BAND; ++chan_idx) {
        /* 获取各5g 各band 0.25db精度的线损值 */
        cus_check_val(g_cust_cap.rf_lp_loss.gain_db_5g[chan_idx], RF_LP_LOSS_GAIN_DB_MAX,
            RF_LP_LOSS_GAIN_DB_MIN, 0x0);
    }
}

void hwifi_cust_rf_check_mp16(void)
{
    uint8_t chan_idx, rf_idx;

    cus_check_min_val(g_cust_host.freq_lock.throughput_irq_low, 1, WLAN_IRQ_TH_LOW);

    /* 配置: fem口到天线口的负增益 */
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (chan_idx = 0; chan_idx < MAC_NUM_2G_BAND; ++chan_idx) {
            /* 获取各2p4g 各band 0.25db及0.1db精度的线损值 */
            cus_check_val(g_cust_rf_front.rf_loss_gain_db.gain_db_2g[chan_idx][rf_idx], RF_LINE_TXRX_GAIN_DB_MAX,
                RF_LINE_TXRX_GAIN_DB_2G_MIN, 0xF4);
        }

        for (chan_idx = 0; chan_idx < MAC_NUM_5G_BAND; ++chan_idx) {
            /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
            cus_check_val(g_cust_rf_front.rf_loss_gain_db.gain_db_5g[chan_idx][rf_idx], RF_LINE_TXRX_GAIN_DB_MAX,
                RF_LINE_TXRX_GAIN_DB_5G_MIN, 0xF8);
        }
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        for (chan_idx = 0; chan_idx < MAC_NUM_6G_BAND; ++chan_idx) {
            /* 获取各5g 各band 0.25db及0.1db精度的线损值 */
            cus_check_max_val(g_cust_rf_front.rf_loss_gain_db.gain_db_6g[chan_idx][rf_idx], 0, 0xF8);
        }
#endif
    }

    for (chan_idx = 0; chan_idx < WLAN_CALI_BAND_BUTT; ++chan_idx) {
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            cus_set_bool(g_cust_rf_front.ext_rf[chan_idx].ext_switch_isexist[rf_idx]);
            cus_set_bool(g_cust_rf_front.ext_rf[chan_idx].ext_pa_isexist[rf_idx]);
            cus_set_bool(g_cust_rf_front.ext_rf[chan_idx].ext_lna_isexist[rf_idx]);
        }
    }

    hwifi_cust_rf_rssi_check_mp16();
    hwifi_cust_rf_lp_loss_check_mp16();
}

/*
 * 函 数 名  : hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram
 * 功能描述  : FCC/CE 5G高band认证参数刷新
 */
static void hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram(void)
{
    uint8_t rf_idx;
    /* 获取管制域信息 */
    regdomain_enum_uint8 regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);

    g_cust_pow.fcc_ce_max_pwr_for_5g_high_band = (uint8_t)((regdomain_type == REGDOMAIN_ETSI) ?
        cus_get_low_16bits(g_cust_pow.fcc_ce_max_pwr_for_5g_high_band) :
        cus_get_high_16bits((g_cust_pow.fcc_ce_max_pwr_for_5g_high_band)));
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        g_cust_pow.fcc_ce_param[rf_idx].fcc_ce_5g_high_band_max_pwr =
            (uint16_t)g_cust_pow.fcc_ce_max_pwr_for_5g_high_band;
    }
}

static void hwifi_base_pow_max_val_check(uint8_t rf_idx, wlan_cali_band_enum_uint8 freq_idx)
{
    uint8_t param_idx;
    uint8_t param_num[WLAN_CALI_BAND_BUTT] = {oal_array_size(g_cust_pow.txpwr_base_2g_params[WLAN_RF_CHANNEL_ZERO]),
        oal_array_size(g_cust_pow.txpwr_base_5g_params[WLAN_RF_CHANNEL_ZERO]),
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        oal_array_size(g_cust_pow.txpwr_base_6g_params[WLAN_RF_CHANNEL_ZERO]),
#endif
    };
    uint8_t *para_addr[WLAN_CALI_BAND_BUTT] = {g_cust_pow.txpwr_base_2g_params[rf_idx],
        g_cust_pow.txpwr_base_5g_params[rf_idx],
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        g_cust_pow.txpwr_base_6g_params[rf_idx],
#endif
    };
    uint8_t *pow = para_addr[freq_idx];

    for (param_idx = 0; param_idx < param_num[freq_idx]; param_idx++) {
        if (cus_val_invalid(pow[param_idx], CUS_MAX_BASE_TXPOWER_VAL, CUS_MIN_BASE_TXPOWER_VAL)) {
            oam_error_log4(0, OAM_SF_CUSTOM,
                "hwifi_cfg_max_txpwr_base_check freq[%d] rf[%d] band[%d] base pow out of range[%d]!",
                freq_idx, rf_idx, param_idx, CUS_MAX_BASE_TXPOWER_VAL);
            pow[param_idx] = CUS_BASE_POW_VAL;
        }
    }
}

static void hwifi_cfg_max_txpwr_base_check(void)
{
    uint8_t rf_idx, freq_idx;

    /* 参数合理性检查 */
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        for (freq_idx = WLAN_BAND_2G; freq_idx < WLAN_BAND_BUTT; freq_idx++) {
            hwifi_base_pow_max_val_check(rf_idx, freq_idx);
        }
    }
}

void hwifi_cust_pow_check_mp16(void)
{
    hwifi_cfg_fcc_ce_5g_high_band_txpwr_nvram();
    hwifi_cfg_max_txpwr_base_check();
}

static void hwifi_cust_device_perf_check(void)
{
    /* SDIO FLOWCTRL */
    if (g_cust_cap.sdio_assem_h2d >= 1 && g_cust_cap.sdio_assem_h2d <= HISDIO_HOST2DEV_SCATT_MAX) {
        g_hcc_assemble_count = g_cust_cap.sdio_assem_h2d;
    } else {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_cust_device_perf_check::sdio_assem_h2d[%d] out of range(0,%d], check value in ini file!}\r\n",
            g_cust_cap.sdio_assem_h2d, HISDIO_DEV2HOST_SCATT_MAX);
    }

    /* SDIO ASSEMBLE COUNT:D2H */
    if (g_cust_cap.sdio_assem_d2h == 0 || g_cust_cap.sdio_assem_d2h > HISDIO_DEV2HOST_SCATT_MAX) {
        g_cust_cap.sdio_assem_d2h = HISDIO_DEV2HOST_SCATT_MAX;
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_cust_device_perf_check::sdio_assem_d2h[%d] out of range(0,%d], check value in ini file!}\r\n",
            g_cust_cap.sdio_assem_d2h, HISDIO_DEV2HOST_SCATT_MAX);
    }
}

#define WLAN_SDIO_TRX_SWITCH_MASK 0xFFFFFFF4 // 与定制化bitmap对应
static void hwifi_cust_trx_switch_check(void)
{
    /* 非PCIE 只支持device tx/device ring tx和device rx */
    if (hcc_is_pcie() == OAL_FALSE) {
        g_cust_cap.trx_switch &= WLAN_SDIO_TRX_SWITCH_MASK;
    }
}

static void hwifi_cust_4k_check(void)
{
    mac_hisi_cap_vendor_ie_stru *hisi_priv_cap = NULL;
    /* 根据时钟判断是否支持4K：76.8MHz-4K；其他-不支持 */
    if (mpxx_cmu_is_tcxo_dll() != TCXO_FREQ_DET_76P8M) {
        hisi_priv_cap = (mac_hisi_cap_vendor_ie_stru *)&(g_cust_cap.hisi_priv_cap);
        hisi_priv_cap->bit_4096qam_cap = 0;
    }
}

static void hwifi_cust_dpd_cali_lvl_num_check(void)
{
    if (g_cust_cap.cali_switch.dpd_cali_lvl_num > WLAN_DPD_TPC_LEVEL_NUM) {
        g_cust_cap.cali_switch.dpd_cali_lvl_num = WLAN_DPD_TPC_LEVEL_NUM;
    }
}

static int32_t hwifi_cust_sar_slide_val_check(void)
{
    uint8_t sar_lvl;
    uint8_t band_idx;
    uint8_t rf_idx;

    for (band_idx = 0; band_idx < WLAN_CALI_BAND_BUTT; band_idx++) {
        for (sar_lvl = 0; sar_lvl < CUS_SAR_LVL_NUM; sar_lvl++) {
            if (!cus_val_valid(g_cust_cap.sar_total_time[sar_lvl][band_idx], CUST_SAR_SLIDE_TIME_MAX,
                CUST_SAR_SLIDE_TIME_MIN)) {
                oam_error_log3(0, OAM_SF_ANY, "{hwifi_cust_sar_slide_val_check::sar_total_time[%d] out of range lvl[%d]\
                    band[%d], check value in ini file!}",
                    g_cust_cap.sar_total_time[sar_lvl][band_idx], sar_lvl, band_idx);
                return OAL_FAIL;
            }
        }
        for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
            if (!cus_val_valid(g_cust_cap.sar_slide_pow_amend[rf_idx][band_idx], CUST_SAR_SLIDE_POW_AMEND_MAX,
                CUST_SAR_SLIDE_POW_AMEND_MIN)) {
                oam_error_log3(0, OAM_SF_ANY, "{hwifi_cust_sar_slide_val_check::sar_slide_pow_amend[%d] out of range \
                    rf[%d] band[%d], check value in ini file!}\r\n",
                    g_cust_cap.sar_slide_pow_amend[rf_idx][band_idx], rf_idx, band_idx);
                return OAL_FAIL;
            }
        }
    }
    return OAL_SUCC;
}

static void hwifi_cust_sar_slide_check(void)
{
    if (hwifi_cust_sar_slide_val_check() == OAL_FAIL) {
        /* 定制化参数异常，默认关闭开关 */
        g_cust_cap.sar_slide_window_en = OAL_FALSE;
    }
}

#define WLAN_TB_MIN_POW 30
static void hwifi_cust_tb_mimo_pow_check(void)
{
    uint8_t ru_idx;
    uint8_t mode_idx;
    uint8_t bw_idx;
    for (ru_idx = 0; ru_idx < WLAN_HE_RU_SIZE_996; ru_idx++) {
        for (mode_idx = 0; mode_idx < HAL_POW_RF_SEL_CHAIN_BUTT; mode_idx++) {
            for (bw_idx = 0; bw_idx < WLAN_BANDWIDTH_2G_BUTT; bw_idx++) {
                g_cust_cap.tb_ru_2g_2x2_max_pow[mode_idx].ru_pwr_2g[ru_idx] =
                    (g_cust_cap.tb_ru_2g_2x2_max_pow[mode_idx].ru_pwr_2g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_2g_2x2_max_pow[mode_idx].ru_pwr_2g[ru_idx];
                g_cust_cap.tb_ru_2g_3x3_max_pow[mode_idx].ru_pwr_2g[ru_idx] =
                    (g_cust_cap.tb_ru_2g_3x3_max_pow[mode_idx].ru_pwr_2g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_2g_3x3_max_pow[mode_idx].ru_pwr_2g[ru_idx];
                g_cust_cap.tb_ru_2g_4x4_max_pow[mode_idx].ru_pwr_2g[ru_idx] =
                    (g_cust_cap.tb_ru_2g_4x4_max_pow[mode_idx].ru_pwr_2g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_2g_4x4_max_pow[mode_idx].ru_pwr_2g[ru_idx];
            }
        }
    }
    for (ru_idx = 0; ru_idx < WLAN_HE_RU_SIZE_BUTT; ru_idx++) {
        for (mode_idx = 0; mode_idx < HAL_POW_RF_SEL_CHAIN_BUTT; mode_idx++) {
            for (bw_idx = 0; bw_idx < WLAN_BANDWIDTH_BUTT; bw_idx++) {
                g_cust_cap.tb_ru_5g_2x2_max_pow[mode_idx].ru_pwr_5g[ru_idx] =
                    (g_cust_cap.tb_ru_5g_2x2_max_pow[mode_idx].ru_pwr_5g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_5g_2x2_max_pow[mode_idx].ru_pwr_5g[ru_idx];
                g_cust_cap.tb_ru_5g_3x3_max_pow[mode_idx].ru_pwr_5g[ru_idx] =
                    (g_cust_cap.tb_ru_5g_3x3_max_pow[mode_idx].ru_pwr_5g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_5g_3x3_max_pow[mode_idx].ru_pwr_5g[ru_idx];
                g_cust_cap.tb_ru_5g_4x4_max_pow[mode_idx].ru_pwr_5g[ru_idx] =
                    (g_cust_cap.tb_ru_5g_4x4_max_pow[mode_idx].ru_pwr_5g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_5g_4x4_max_pow[mode_idx].ru_pwr_5g[ru_idx];
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
                g_cust_cap.tb_ru_6g_2x2_max_pow[mode_idx].ru_pwr_6g[ru_idx] =
                    (g_cust_cap.tb_ru_6g_2x2_max_pow[mode_idx].ru_pwr_6g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_6g_2x2_max_pow[mode_idx].ru_pwr_6g[ru_idx];
                g_cust_cap.tb_ru_6g_3x3_max_pow[mode_idx].ru_pwr_6g[ru_idx] =
                    (g_cust_cap.tb_ru_6g_3x3_max_pow[mode_idx].ru_pwr_6g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_6g_3x3_max_pow[mode_idx].ru_pwr_6g[ru_idx];
                g_cust_cap.tb_ru_6g_4x4_max_pow[mode_idx].ru_pwr_6g[ru_idx] =
                    (g_cust_cap.tb_ru_6g_4x4_max_pow[mode_idx].ru_pwr_6g[ru_idx] < WLAN_TB_MIN_POW) ?
                    0x7F : g_cust_cap.tb_ru_6g_4x4_max_pow[mode_idx].ru_pwr_6g[ru_idx];
#endif
            }
        }
    }
}

STATIC wlan_cust_range_stru g_cap_cfg_range[] = {
    {&g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_2G], CUST_CFG_INT16, CUST_CHECK_VAL, FB_GAIN_DELT_POW_MAX,
        FB_GAIN_DELT_POW_MIN, 0},
    {&g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_5G], CUST_CFG_INT16, CUST_CHECK_VAL, FB_GAIN_DELT_POW_MAX,
        FB_GAIN_DELT_POW_MIN, 0},
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    {&g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_6G], CUST_CFG_INT16, CUST_CHECK_VAL, FB_GAIN_DELT_POW_MAX,
        FB_GAIN_DELT_POW_MIN, 0},
#endif
    {&g_cust_cap.en_channel_width, CUST_CFG_UINT8, CUST_CHECK_VAL, WLAN_BW_CAP_80PLUS80, WLAN_BW_CAP_20M, 0}
};

void hwifi_cust_cap_check_mp16(void)
{
    cus_set_bool(g_cust_cap.mbo_switch);

    hwifi_cust_trx_switch_check();
    hwifi_cust_device_perf_check();
    hwifi_cust_4k_check();
    hwifi_cust_dpd_cali_lvl_num_check();
    hwifi_mp16_cust_val_tbl_check(g_cap_cfg_range, oal_array_size(g_cap_cfg_range));
    hwifi_cust_sar_slide_check();
    hwifi_cust_tb_mimo_pow_check();
}

static void hwifi_mp16_cust_dyn_pow_dpn_check(uint8_t rf_idx)
{
    uint8_t band_idx;
    uint8_t mode_idx;
    uint8_t lvl_idx;
    for (lvl_idx = 0; lvl_idx < HAL_CUS_FB_LVL_NUM; lvl_idx++) {
        for (mode_idx = 0; mode_idx < DYN_PWR_CUST_2G_SNGL_MODE_BUTT; mode_idx++) {
            for (band_idx = 0; band_idx < MAC_2G_CHANNEL_NUM; band_idx++) {
                cus_check_val(g_cust_dyn_pow.dpn_para[lvl_idx][rf_idx].dpn_2g[mode_idx][band_idx],
                              CUS_DY_CALI_2G_VAL_DPN_MAX, CUS_DY_CALI_2G_VAL_DPN_MIN, 0);
            }
        }

        for (mode_idx = 0; mode_idx < WLAN_BANDWIDTH_BUTT; mode_idx++) {
            for (band_idx = 0; band_idx < MAC_NUM_5G_BAND; band_idx++) {
                cus_check_val(g_cust_dyn_pow.dpn_para[lvl_idx][rf_idx].dpn_5g[mode_idx][band_idx],
                              CUS_DY_CALI_5G_VAL_DPN_MAX, CUS_DY_CALI_5G_VAL_DPN_MIN, 0);
            }
            for (band_idx = 0; band_idx < MAC_NUM_6G_BAND; band_idx++) {
                cus_check_val(g_cust_dyn_pow.dpn_para[lvl_idx][rf_idx].dpn_6g[mode_idx][band_idx],
                              CUS_DY_CALI_5G_VAL_DPN_MAX, CUS_DY_CALI_5G_VAL_DPN_MIN, 0);
            }
        }
    }
}

void hwifi_cust_dyn_pow_check_mp16(void)
{
    uint8_t rf_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        hwifi_mp16_cust_dyn_pow_dpn_check(rf_idx);
    }
}

static void hwifi_check_custom_param_mp16(int32_t cus_tag)
{
    switch (cus_tag) {
        case CUS_TAG_HOST:
            hwifi_host_cust_check_mp16();
            break;
        case CUS_TAG_POW:
            hwifi_cust_pow_check_mp16();
            break;
        case CUS_TAG_RF:
            hwifi_cust_rf_check_mp16();
            break;
        case CUS_TAG_DYN_POW:
            hwifi_cust_dyn_pow_check_mp16();
            break;
        case CUS_TAG_CAP:
            hwifi_cust_cap_check_mp16();
            break;
        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
    }
};

int16_t *g_fcc_ce_data_addr[] = {
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m[6],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m[12],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_20m[18],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_40m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_40m[6],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_80m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_160m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX0][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX1][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX2][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX3][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX4][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX5][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX6][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX7][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX8][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX9][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX10][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX11][0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX12][0],
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_20m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_20m[30],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_40m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_80m[0],
    &g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_160m[0],
#endif
};

uint8_t g_fcc_ce_data_num[] = {
    HAL_NUM_5G_20M_SIDE_BAND,
    HAL_NUM_5G_20M_SIDE_BAND,
    HAL_NUM_5G_20M_SIDE_BAND,
    HAL_NUM_5G_20M_SIDE_BAND,
    HAL_NUM_5G_40M_SIDE_BAND,
    HAL_5G_40M_SIDE_BAND_ALL - HAL_NUM_5G_40M_SIDE_BAND,
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_80m),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_5g_160m),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX0]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX1]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX2]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX3]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX4]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX5]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX6]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX7]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX8]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX9]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX10]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX11]),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_2g[WLAN_2G_CHN_IDX12]),
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    HAL_NUM_6G_20M_SIDE_BAND1,
    HAL_NUM_6G_20M_SIDE_BAND2,
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_40m),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_80m),
    oal_array_size(g_cust_pow.fcc_ce_param[WLAN_RF_CHANNEL_ZERO].fcc_txpwr_limit_6g_160m),
#endif
};
static int16_t hwifi_get_pow_cfg_idx(int16_t cfg_id)
{
    int16_t cfg_idx;
    for (cfg_idx = 0; cfg_idx < NVRAM_PARA_PWR_INDEX_BUTT; cfg_idx++) {
        if (g_wifi_cfg_pow[cfg_idx].case_entry == cfg_id) {
            return cfg_idx;
        }
    }
    oam_error_log1(0, 0, "hwifi_get_pow_cfg_idx: config id %d, not found!", cfg_id);
    return 0;
}
static void hwifi_clean_fcc_ce_txpwr_para(int16_t start_idx, int16_t end_idx)
{
    int16_t cfg_id;
    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr = NULL;
        g_wifi_cfg_pow[cfg_id].data_type = 0;
        g_wifi_cfg_pow[cfg_id].data_num = 0;
    }
}
static void hwifi_get_fcc_ce_cn_txpwr_cmd_id(regdomain_enum regdomain_type, int16_t rf_idx,
    int16_t *start_idx, int16_t *end_idx)
{
    int16_t fcc_start_id[] = {NVRAM_PARA_FCC_C0_START_INDEX,
        NVRAM_PARA_FCC_C1_START_INDEX, NVRAM_PARA_FCC_C2_START_INDEX, NVRAM_PARA_FCC_C3_START_INDEX};
    int16_t fcc_end_id[] = {NVRAM_PARA_FCC_C0_END_INDEX_BUTT,
        NVRAM_PARA_FCC_C1_END_INDEX_BUTT, NVRAM_PARA_FCC_C2_END_INDEX_BUTT, NVRAM_PARA_FCC_C3_END_INDEX_BUTT};
    int16_t ce_start_id[] = {NVRAM_PARA_CE_C0_START_INDEX,
        NVRAM_PARA_CE_C1_START_INDEX, NVRAM_PARA_CE_C2_START_INDEX, NVRAM_PARA_CE_C3_START_INDEX};
    int16_t ce_end_id[] = {NVRAM_PARA_CE_C0_END_INDEX_BUTT,
        NVRAM_PARA_CE_C1_END_INDEX_BUTT, NVRAM_PARA_CE_C2_END_INDEX_BUTT, NVRAM_PARA_CE_C3_END_INDEX_BUTT};
    int16_t cn_start_id[] = {NVRAM_PARA_CN_C0_START_INDEX,
        NVRAM_PARA_CN_C1_START_INDEX, NVRAM_PARA_CN_C2_START_INDEX, NVRAM_PARA_CN_C3_START_INDEX};
    int16_t cn_end_id[] = {NVRAM_PARA_CN_C0_END_INDEX_BUTT,
        NVRAM_PARA_CN_C1_END_INDEX_BUTT, NVRAM_PARA_CN_C2_END_INDEX_BUTT, NVRAM_PARA_CN_C3_END_INDEX_BUTT};
    if (regdomain_type == REGDOMAIN_ETSI) {
        *start_idx = hwifi_get_pow_cfg_idx(ce_start_id[rf_idx]);
        *end_idx = hwifi_get_pow_cfg_idx(ce_end_id[rf_idx]);
    } else if (regdomain_type == REGDOMAIN_CN) {
        *start_idx = hwifi_get_pow_cfg_idx(cn_start_id[rf_idx]);
        *end_idx = hwifi_get_pow_cfg_idx(cn_end_id[rf_idx]);
    } else {
        *start_idx = hwifi_get_pow_cfg_idx(fcc_start_id[rf_idx]);
        *end_idx = hwifi_get_pow_cfg_idx(fcc_end_id[rf_idx]);
    }
}
/*
 * 函 数 名  : hwifi_config_init_fcc_ce_txpwr_cmd
 * 功能描述  : FCC/CE认证项，配置参数刷新
 */
static void hwifi_config_init_fcc_ce_txpwr_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    regdomain_enum regdomain_type;
    int16_t start_idx;
    int16_t end_idx;

    /* 清空FCC/CE配置参数 */
    hwifi_get_fcc_ce_cn_txpwr_cmd_id(REGDOMAIN_FCC, rf_idx, &start_idx, &end_idx);
    hwifi_clean_fcc_ce_txpwr_para(start_idx, end_idx);
    hwifi_get_fcc_ce_cn_txpwr_cmd_id(REGDOMAIN_ETSI, rf_idx, &start_idx, &end_idx);
    hwifi_clean_fcc_ce_txpwr_para(start_idx, end_idx);
    hwifi_get_fcc_ce_cn_txpwr_cmd_id(REGDOMAIN_CN, rf_idx, &start_idx, &end_idx);
    hwifi_clean_fcc_ce_txpwr_para(start_idx, end_idx);
    /* 获取管制域信息 */
    regdomain_type = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    hwifi_get_fcc_ce_cn_txpwr_cmd_id(regdomain_type, rf_idx, &start_idx, &end_idx);
    if (end_idx - start_idx !=  oal_array_size(g_fcc_ce_data_addr)) {
        oam_error_log2(0, 0, "hwifi_config_init_fcc_ce_txpwr_cmd:custom num %d and mem addr num %d not match",
            end_idx - start_idx, oal_array_size(g_fcc_ce_data_addr));
        return;
    }
    /* 根据国家码刷新对应值 */
    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            (uint8_t *)g_fcc_ce_data_addr[cfg_id - start_idx] + sizeof(hal_cfg_custom_fcc_txpwr_limit_stru) * rf_idx;
        g_wifi_cfg_pow[cfg_id].data_type = sizeof(int16_t);
        g_wifi_cfg_pow[cfg_id].data_num = g_fcc_ce_data_num[cfg_id - start_idx];
    }
}
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/*
 * 函 数 名  : hwifi_config_init_6g_sar_ctrl_cmd
 * 功能描述  : 更新sar命令相关参数
 */
static void hwifi_config_init_6g_sar_ctrl_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    int16_t sar_start_id[] = {NVRAM_PARA_6G_C0_SAR_START_INDEX,
        NVRAM_PARA_6G_C1_SAR_START_INDEX, NVRAM_PARA_6G_C2_SAR_START_INDEX, NVRAM_PARA_6G_C3_SAR_START_INDEX};
    int16_t sar_end_id[] = {NVRAM_PARA_6G_C0_SAR_END_INDEX_BUTT,
        NVRAM_PARA_6G_C1_SAR_END_INDEX_BUTT, NVRAM_PARA_6G_C2_SAR_END_INDEX_BUTT, NVRAM_PARA_6G_C3_SAR_END_INDEX_BUTT};
    int16_t start_idx = hwifi_get_pow_cfg_idx(sar_start_id[rf_idx]);
    int16_t end_idx = hwifi_get_pow_cfg_idx(sar_end_id[rf_idx]);
    if (end_idx - start_idx != oal_array_size(g_cust_pow.sar_ctrl_params)) {
        oam_error_log2(0, 0, "hwifi_config_init_6g_sar_ctrl_cmd:custom num %d and mem addr num %d not match",
            end_idx - start_idx, oal_array_size(g_cust_pow.sar_ctrl_params));
        return;
    }
    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            &g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx][0];
        g_wifi_cfg_pow[cfg_id].data_type =
            sizeof(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx][0]);
        g_wifi_cfg_pow[cfg_id].data_num =
            (uint8_t)oal_array_size(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_6g_params[rf_idx]);
    }
}
#endif
/*
 * 函 数 名  : hwifi_config_init_sar_ctrl_cmd
 * 功能描述  : 更新sar命令相关参数
 */
static void hwifi_config_init_sar_ctrl_cmd(uint8_t rf_idx)
{
    int16_t cfg_id;
    int16_t sar_start_id[] = {NVRAM_PARA_C0_SAR_START_INDEX,
        NVRAM_PARA_C1_SAR_START_INDEX, NVRAM_PARA_C2_SAR_START_INDEX, NVRAM_PARA_C3_SAR_START_INDEX};
    int16_t sar_end_id[] = {NVRAM_PARA_C0_SAR_END_INDEX_BUTT,
        NVRAM_PARA_C1_SAR_END_INDEX_BUTT, NVRAM_PARA_C2_SAR_END_INDEX_BUTT, NVRAM_PARA_C3_SAR_END_INDEX_BUTT};
    int16_t start_idx = hwifi_get_pow_cfg_idx(sar_start_id[rf_idx]);
    int16_t end_idx = hwifi_get_pow_cfg_idx(sar_end_id[rf_idx]);
    if (end_idx - start_idx != oal_array_size(g_cust_pow.sar_ctrl_params)) {
        oam_error_log2(0, 0, "hwifi_config_init_sar_ctrl_cmd:custom num %d and mem addr num %d not match",
            end_idx - start_idx, oal_array_size(g_cust_pow.sar_ctrl_params));
        return;
    }
    for (cfg_id = start_idx; cfg_id < end_idx; cfg_id++) {
        g_wifi_cfg_pow[cfg_id].data_addr =
            &g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx][0];
        g_wifi_cfg_pow[cfg_id].data_type =
            sizeof(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx][0]);
        g_wifi_cfg_pow[cfg_id].data_num =
            (uint8_t)oal_array_size(g_cust_pow.sar_ctrl_params[cfg_id - start_idx].sar_ctrl_params[rf_idx]);
    }
}

/*
 * 函 数 名  : hwifi_cust_init_pow_nvram_cfg
 * 功能描述  : 更新pow命令相关参数
 */
static void hwifi_cust_init_pow_nvram_cfg(void)
{
    uint8_t rf_idx;
    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        hwifi_config_init_fcc_ce_txpwr_cmd(rf_idx);
        hwifi_config_init_sar_ctrl_cmd(rf_idx);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        hwifi_config_init_6g_sar_ctrl_cmd(rf_idx);
#endif
    }
}

uint32_t hwifi_config_set_cali_chan(uint32_t chn)
{
    uint32_t band_idx;
    uint32_t ret = 0;

    g_wifi_fac_cali_chain = chn;
    if (g_wifi_fac_cali_chain != 0) {
        // 关闭多次校准
        g_cust_cap.cali_policy &= (~WLAN_MUL_CHAIN_CALI_MASK);
        g_cust_cap.cali_policy &= (~WLAN_MUL_BW_CALI_MASK);
        // 产线校准前打开offline dpd
        for (band_idx = 0; band_idx < WLAN_CALI_BAND_BUTT; band_idx++) {
            g_cust_cap.cali_switch.cali_mask[band_idx] |= BIT(WLAN_RF_CALI_DPD);
        }
    } else {
        ret += hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_CALI_POLICY]);
        for (band_idx = 0; band_idx < WLAN_CALI_BAND_BUTT; band_idx++) {
            ret += hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_CALI_MASK_2G + band_idx]);
        }
        if (ret != INI_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_config_set_cali_chan: get default val from ini failed!");
        }
    }
    oam_warning_log4(0, OAM_SF_CUSTOM,
        "hwifi_config_set_cali_chan::cali_chain 0x%x cali_policy 0x%x cali_mask 2g/5g 0x%x 0x%x!",
        g_wifi_fac_cali_chain, g_cust_cap.cali_policy,
        g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_2G], g_cust_cap.cali_switch.cali_mask[WLAN_CALI_BAND_5G]);
    return 0;
}

uint32_t hwifi_config_get_cali_chan(uint32_t chn_idx, uint32_t band)
{
    uint32_t cali_chan = g_wifi_fac_cali_chain >> (band * HD_EVENT_RF_NUM);
    if (band == WLAN_CALI_BAND_2G || band == WLAN_CALI_BAND_5G) {
        return cali_chan & (1UL << chn_idx);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    } else if (band == WLAN_CALI_BAND_6G) {
        return cali_chan & (0x3 << (chn_idx * WLAN_CALI_BAND_6G));
#endif
    } else {
        return 0;
    }
}

/*
 * 函 数 名  : hwifi_config_init_mp16
 * 功能描述  : netdev open 调用的定制化总入口
 *             读取ini文件，更新定制化结构体成员数值
 */
int32_t hwifi_config_init_mp16(int32_t cus_tag)
{
    int32_t cfg_id;
    wlan_cust_cfg_cmd *wifi_cust_cmd = NULL;
    uint32_t wlan_cfg_butt = 0;

    switch (cus_tag) {
        case CUS_TAG_HOST:
            wifi_cust_cmd = g_wifi_cfg_host;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_host);
            break;
        case CUS_TAG_POW:
            hwifi_cust_init_pow_nvram_cfg();
            wifi_cust_cmd = g_wifi_cfg_pow;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_pow);
            break;
        case CUS_TAG_RF:
            wifi_cust_cmd = g_wifi_cfg_rf_cali;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_rf_cali);
            break;
        case CUS_TAG_DYN_POW:
            wifi_cust_cmd = g_wifi_cfg_dyn_pow;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_dyn_pow);
            break;
        case CUS_TAG_CAP:
            wifi_cust_cmd = g_wifi_cfg_cap;
            wlan_cfg_butt = oal_array_size(g_wifi_cfg_cap);
            break;

        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for (cfg_id = 0; cfg_id < (int32_t)wlan_cfg_butt; cfg_id++) {
        if (cfg_id != wifi_cust_cmd[cfg_id].case_entry) {
            oam_error_log3(0, 0, "hwifi_config_init_mp16:config id error:cus_tag %d cfg_id %d case_entry %d",
                cus_tag, cfg_id, wifi_cust_cmd[cfg_id].case_entry);
        }
        /* 获取ini的配置值 */
        wifi_cust_cmd[cfg_id].en_value_state =
            hwifi_config_get_cust_val(&wifi_cust_cmd[cfg_id]) == INI_SUCC ? OAL_TRUE : OAL_FALSE;
    }
    /* 检查定制化项取值范围 */
    hwifi_check_custom_param_mp16(cus_tag);
    return INI_SUCC;
}

#ifdef HISI_CONN_NVE_SUPPORT
static int32_t hwifi_cust_read_nvram_flag_mp16(uint8_t read_mask)
{
    int32_t ret;
    oal_bool_enum_uint8 en_get_nvram_data_flag = OAL_FALSE;

    ret = hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_CALI_DATA_MASK]);
    if (ret == INI_SUCC) {
        en_get_nvram_data_flag = read_mask & g_cust_cap.cali_data_mask;
        if (read_mask & CUST_READ_NVRAM_MASK & g_cust_cap.cali_data_mask) {
            oal_io_print("hwifi_cust_read_nvram_flag_mp16::cali_data_mask[%d] ,abandon DPD nvram data!!\r\n",
                g_cust_cap.cali_data_mask);
            memset_s(&g_cust_nvram_info.dpd_cal_data, sizeof(g_cust_nvram_info.dpd_cal_data), 0,
                sizeof(g_cust_nvram_info.dpd_cal_data));
        }
        if (read_mask & CUST_READ_NVRAM_POW_MASK & g_cust_cap.cali_data_mask) {
            oal_io_print("hwifi_cust_read_nvram_flag_mp16::cali_data_mask[%d] ,abandon POW nvram data!!\r\n",
                g_cust_cap.cali_data_mask);
            memset_s(&g_cust_nvram_info.pow_cal_data, sizeof(g_cust_nvram_info.pow_cal_data), 0,
                sizeof(g_cust_nvram_info.pow_cal_data));
        }
    }
    return en_get_nvram_data_flag == read_mask ? OAL_FALSE : OAL_TRUE;
}
static uint32_t hwifi_cust_read_nvram_dpd_state_mp16(void)
{
    /* 判断定制化中是否使用nvram中的DPD校准参数 */
    if (hwifi_cust_read_nvram_flag_mp16(CUST_READ_NVRAM_MASK) == OAL_FALSE) {
        return 0;
    }
    return conn_nve_get_data_status(CONN_STATUS_DPD);
}


static int32_t hwifi_cust_get_nvram_params_mp16(conn_nve_particion_stru *cust_nvram_info)
{
    int32_t ret;
    ret = conn_nve_fetch_wifi_nvdata(cust_nvram_info);
    if (oal_unlikely(ret != 0)) {
        oal_io_print("hwifi_cust_get_nvram_params_mp16:conn_nve_fetch_wifi_nvdata failed,ret[%d].\r\n", ret);
    }
    return ret;
}
/*
 * 函 数 名  : hwifi_cust_get_nvram_mp16
 * 功能描述  : 包括读取nvram中的dpint和DPD校准系数值
 */
int32_t hwifi_cust_get_nvram_mp16(void)
{
    conn_nve_particion_stru *cust_nvram_info = NULL; /* NVRAM数组 */

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    if (hwifi_cust_read_nvram_flag_mp16(CUST_READ_NVRAM_MASK | CUST_READ_NVRAM_POW_MASK) == OAL_FALSE) {
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    cust_nvram_info = (conn_nve_particion_stru *)os_kzalloc_gfp(sizeof(conn_nve_particion_stru));
    if (cust_nvram_info == NULL) {
        // this fail will be emergency in factory mode, cause tx test maybe fail
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::cust_nvram_info mem alloc fail!");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    memset_s(cust_nvram_info, sizeof(conn_nve_particion_stru), 0, sizeof(conn_nve_particion_stru));

    if (hwifi_cust_get_nvram_params_mp16(cust_nvram_info) != 0) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::nv didn't updated!");
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    /* 检查NVRAM是否修改 */
    if (oal_memcmp(cust_nvram_info, &g_cust_nvram_info, sizeof(g_cust_nvram_info)) == 0) {
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (memcpy_s(&g_cust_nvram_info, sizeof(g_cust_nvram_info),
                 cust_nvram_info, sizeof(g_cust_nvram_info)) != EOK) {
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    os_mem_kfree(cust_nvram_info);
    return INI_NVRAM_RECONFIG;
}
/*
 * 函 数 名  : hwifi_update_nv_dpn
 * 功能描述  : 读取nvram中的delta gain
 */
int32_t hwifi_update_nv_dpn(void)
{
    int32_t ret = 0;
    uint32_t cfg_id;
    uint32_t data_id;
    uint32_t cfg_butt = oal_array_size(g_wifi_cfg_dpn_nv);
    uint32_t cfg_half = cfg_butt >> 1;
    /* 判断定制化中是否使用nvram中的POW校准参数 */
    if (hwifi_cust_read_nvram_flag_mp16(CUST_READ_NVRAM_POW_MASK) == OAL_FALSE) {
        return OAL_FALSE;
    }

    for (cfg_id = 0; cfg_id < cfg_half; cfg_id++) {
        for (data_id = 0; data_id < g_wifi_cfg_dpn_nv[cfg_id].ini_data_num; data_id++) {
            g_wifi_cfg_dpn_nv[cfg_id].ini_data_addr[data_id] +=
                g_wifi_cfg_dpn_nv[cfg_id].nv_data_addr[data_id].delta_gain[0];
            g_wifi_cfg_dpn_nv[cfg_id + cfg_half].ini_data_addr[data_id] +=
                g_wifi_cfg_dpn_nv[cfg_id + cfg_half].nv_data_addr[data_id].delta_gain[1];
            oam_warning_log2(0, OAM_SF_CUSTOM, "hwifi_update_nv_dpn::nv data[%d][%d]!",
                g_wifi_cfg_dpn_nv[cfg_id].nv_data_addr[data_id].delta_gain[0],
                g_wifi_cfg_dpn_nv[cfg_id + cfg_half].nv_data_addr[data_id].delta_gain[1]);
        }
    }
    return ret;
}

static int32_t wal_read_2g_dpd_data_from_nv(uint32_t rf_idx, wlan_cali_data_para_stru *cali_data)
{
    uint32_t band_idx;
    for (band_idx = 0; band_idx < WLAN_2G_CALI_BAND_NUM; band_idx++) {
        if (memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_2g_20m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_2g[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK) {
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    return 0;
}

static int32_t wal_read_5g_dpd_data_from_nv(uint32_t rf_idx, wlan_cali_data_para_stru *cali_data)
{
    uint32_t band_idx;
    for (band_idx = 0; band_idx < WLAN_5G_20M_CALI_BAND_NUM; band_idx++) {
        if (memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_5g_20m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_5g_20[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK ||\
            memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_5g_80m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_5g_80[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK) {
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    for (band_idx = 0; band_idx < WLAN_5G_160M_CALI_BAND_NUM; band_idx++) {
        if (memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_5g_160m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_5g_160[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK) {
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    return 0;
}
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
static int32_t wal_read_6g_dpd_data_from_nv(uint32_t rf_idx, wlan_cali_data_para_stru *cali_data)
{
    uint32_t band_idx;
    for (band_idx = 0; band_idx < WLAN_6G_20M_CALI_BAND_NUM; band_idx++) {
        if (memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_6g_20m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_6g_20[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK ||\
            memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_6g_80m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_6g_80[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK) {
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    for (band_idx = 0; band_idx < WLAN_6G_160M_CALI_BAND_NUM; band_idx++) {
        if (memcpy_s(&cali_data->rf_cali_data[rf_idx].cali_data_6g_160m.cali_data[band_idx].dpd_cali_data,
            sizeof(wlan_cali_dpd_para_stru), &g_cust_nvram_info.dpd_cal_data.cali_data_6g_160[rf_idx][band_idx],
            sizeof(wlan_cali_dpd_para_stru)) != EOK) {
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    return 0;
}
#endif

/*
 * 函 数 名  : wal_read_dpd_data_from_nv
 * 功能描述  : 读取nvram中dpd数据刷新到host校准数据buffer准备下发
 */
void wal_get_nv_dpd_data_mp16()
{
    int32_t ret = 0;
    uint32_t rf_idx;
    uint8_t *mem_addr = get_cali_data_buf_addr();
    wlan_cali_data_para_stru *cali_data = (wlan_cali_data_para_stru *)mem_addr;
    /* 判断NV DPD数据是否ready */
    if (hwifi_cust_read_nvram_dpd_state_mp16() == 0) {
        return;
    }

    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        ret += wal_read_2g_dpd_data_from_nv(rf_idx, cali_data);
        ret += wal_read_5g_dpd_data_from_nv(rf_idx, cali_data);
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        ret += wal_read_6g_dpd_data_from_nv(rf_idx, cali_data);
#endif
    }
    if (oal_unlikely(ret != 0)) {
        oal_io_print("wal_read dpd data from_nv failed, ret[%d].\r\n", ret);
    }
    return;
}

#endif // #ifdef HISI_CONN_NVE_SUPPORT
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
typedef int32_t (*fe_config_update_pow_cb)(void);
OAL_STATIC int32_t fe_host_config_update_pow(void)
{
    fe_config_update_pow_cb fe_config_update_pow = NULL;

    fe_config_update_pow = (fe_config_update_pow_cb)fe_get_out_if_cb(FE_CUSTOM_CONFIG_UPDATE_POW);
    if (fe_config_update_pow == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "fe_config_update_pow: fe_get_out_if_cb failed!");
        return OAL_FAIL;
    }
    return fe_config_update_pow();
}
typedef uint32_t (*fe_custom_host_read_cfg_ini_cb)(regdomain_enum en_regdomain);
uint32_t custom_read_fe_ini(void)
{
    fe_custom_host_read_cfg_ini_cb fe_custom_host_read_cfg_ini = NULL;
    regdomain_enum en_regdomain;
    fe_custom_host_read_cfg_ini = (fe_custom_host_read_cfg_ini_cb)fe_get_out_if_cb(FE_CUSTOM_HOST_READ_INI_PARSE);
    if (fe_custom_host_read_cfg_ini == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "FE_CUSTOM_HOST_READ_INI_PARSE: fe_get_out_if_cb failed!");
        BUG_ON(1);
        return OAL_FAIL;
    }
    en_regdomain = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    return fe_custom_host_read_cfg_ini(en_regdomain);
}
#endif
int16_t *hwifi_get_fbgain_param(void)
{
    if (hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_FB_GAIN_DELTA_POW]) != INI_SUCC) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_get_fbgain_param: get fbgain from ini failed, use default val!");
    } else {
        oam_warning_log2(0, OAM_SF_CFG, "hwifi_get_fbgain_param: get fbgain from ini [%d][%d]",
            g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_2G], g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_5G]);
    }
    return g_cust_cap.fb_gain_delta_pow;
}

uint32_t hwifi_get_elna_protect_flag(void)
{
    if (g_cust_cap.cali_policy & WLAN_USE_ELNA_DET_RESULT_MASK) {
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

static void hwifi_read_fbgain_from_nv(void)
{
#ifdef HISI_CONN_NVE_SUPPORT
    if (hwifi_cust_read_nvram_flag_mp16(CUST_READ_NVRAM_MASK) == OAL_TRUE) {
        conn_nve_read_fbgain(g_cust_cap.fb_gain_delta_pow);
        oam_warning_log2(0, OAM_SF_CFG, "hwifi_get_fbgain_param: get fbgain from nv [%d][%d]",
            g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_2G], g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_5G]);
    } else {
        oam_warning_log2(0, OAM_SF_CFG, "hwifi_get_fbgain_param: get fbgain from ini [%d][%d]",
            g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_2G], g_cust_cap.fb_gain_delta_pow[WLAN_CALI_BAND_5G]);
    }
#endif
}
/* 读定制化配置文件&NVRAM */
uint32_t hwifi_custom_host_read_cfg_init_mp16(void)
{
    int32_t ini_read_ret;
    int32_t l_nv_read_ret = 0;
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
    int32_t ret = OAL_SUCC;
#endif
#ifdef HISI_CONN_NVE_SUPPORT
    /* 检查conn_nve初始化结果 */
    if (conn_nve_init_wifi_nvdata() != CONN_NVE_RET_OK) {
        oam_warning_log0(0, OAM_SF_CFG, "conn_nve_init_wifi_nvdata failed");
    }
    /* 读取nvram参数是否修改 */
    l_nv_read_ret += hwifi_cust_get_nvram_mp16();
#endif
    /* 检查定制化文件中WIFI section的产线配置是否修改 */
    ini_read_ret = ini_file_check_conf_update(INI_SECTION_WIFI);
    if (ini_read_ret || l_nv_read_ret) {
        oam_warning_log2(0, OAM_SF_CFG,
            "hwifi_custom_host_read_cfg_init config is updated. ini_read_ret %d, nv_read_ret %d",
            ini_read_ret, l_nv_read_ret);
        hwifi_config_init_mp16(CUS_TAG_DYN_POW);
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
        ret = fe_host_config_update_pow();
        if (oal_unlikely(ret != INI_SUCC)) {
            oam_error_log1(0, OAM_SF_CFG, "fe_host_config_update_pow faild=%d", ret);
        }
#endif
#ifdef HISI_CONN_NVE_SUPPORT
        /* nv内容更新后，刷新NV中的功率校准数据到DPN */
        hwifi_update_nv_dpn();
#endif
    }

    if (ini_read_ret == INI_FILE_TIMESPEC_UNRECONFIG) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init ini file is not updated");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    oam_warning_log0(0, OAM_SF_CFG, "hwifi_custom_host_read_cfg_init config is updated");

    ini_read_ret += hwifi_config_init_mp16(CUS_TAG_HOST);
    ini_read_ret += hwifi_config_init_mp16(CUS_TAG_CAP);
    ini_read_ret += hwifi_config_init_mp16(CUS_TAG_RF);
    ini_read_ret += hwifi_config_init_mp16(CUS_TAG_POW);
    if (oal_unlikely(ini_read_ret != OAL_SUCC)) {
        oal_io_print("hwifi_custom_host_read_cfg_init NV fail l_ret[%d].\r\n", ini_read_ret);
    }
    hwifi_read_fbgain_from_nv();
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
    if (oal_unlikely(custom_read_fe_ini() != OAL_SUCC)) {
        oal_io_print("custom_read_fe_ini NV fail l_ret[%d].\r\n", ret);
    }
#endif
    return OAL_SUCC;
}
/*
 * 函 数 名  : hwifi_custom_adapt_device_init_param_mp16
 * 功能描述  : 配置定制化参数结束标志，下发到device触发device init流程
 */
static uint32_t hwifi_custom_adapt_device_init_param_mp16(uint8_t *puc_data)
{
    uint32_t data_len = 0;
    int32_t l_ret;
    hmac_to_dmac_cfg_custom_data_stru st_syn_msg = {0};

    if (puc_data == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_init_param::puc_data is NULL.}");
        return INI_FAILED;
    }
    /*
     * 发送消息的格式如下:
     * +-------------------------------------------------------------------+
     * | CFGID0    |DATA0 Length| DATA0 Value | ......................... |
     * +-------------------------------------------------------------------+
     * | 4 Bytes   |4 Byte      | DATA  Length| ......................... |
     * +-------------------------------------------------------------------+
     */
    st_syn_msg.en_syn_id = CUSTOM_CFGID_INI_ENDING_ID;
    st_syn_msg.len = 0;

    l_ret = memcpy_s(puc_data, (WLAN_LARGE_NETBUF_SIZE - data_len), &st_syn_msg, sizeof(st_syn_msg));
    if (l_ret != EOK) {
        oam_error_log2(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_init_param::memcpy_s fail[%d]. data_len[%u]}",
                       l_ret, data_len);
    }
    data_len += sizeof(st_syn_msg);

    return data_len;
}

static void hwifi_custom_adapt_hcc_flowctrl_type_mp16(uint8_t *priv_cfg_value)
{
    if (hcc_bus_flowctrl_init(*priv_cfg_value) != OAL_SUCC) {
        /* GPIO流控中断注册失败，强制device使用SDIO流控(type = 0) */
        *priv_cfg_value = 0;
    }
    oal_io_print("hwifi_custom_adapt_hcc_flowctrl_type::sdio_flow_ctl_type[0x%x].\r\n",
                 *priv_cfg_value);
}
/*
 * 函 数 名  : hwifi_rf_cali_data_host_addr_init
 * 功能描述  : host校准数据内存初始化
 */
void hwifi_rf_cali_data_host_addr_init_mp16(void)
{
    uint64_t host_dev_addr = 0;

    /* SDIO不需要 */
    if (hcc_is_pcie() == OAL_FALSE) {
        return;
    }

    /* 申请内存，并映射物理地址 */
    if (get_cali_data_buf_addr() == NULL) {
        oal_io_print("{hwifi_rf_cali_data_h_addr_init:: mem alloc fail!} \n");
        return;
    }
    pcie_if_hostca_to_devva(HCC_EP_WIFI_DEV, get_cali_data_buf_phy_addr(), &host_dev_addr);
    g_cust_cap.cali_data_h_phy_addr = (uintptr_t)host_dev_addr;
    oal_io_print("{hwifi_rf_cali_data_host_addr_init succ!} \n");
}
static uint32_t hwifi_hcc_custom_get_data_mp16(uint16_t syn_id, oal_netbuf_stru *netbuf)
{
    uint32_t data_len = 0;
    int32_t ret = EOK;
    uint8_t *netbuf_data = (uint8_t *)oal_netbuf_data(netbuf);

    switch (syn_id) {
        case CUSTOM_CFGID_INI_ID:
            /* 填充device初始化参数 */
            data_len = hwifi_custom_adapt_device_init_param_mp16(netbuf_data);
            break;
        case CUSTOM_CFGID_CAP_ID:
            /* 开机默认打开校准数据上传下发 */
            g_cust_cap.cali_data_mask = hwifi_custom_cali_ini_param(g_cust_cap.cali_data_mask);
#ifdef HISI_CONN_NVE_SUPPORT
            if (get_mpxx_subchip_type() != BOARD_VERSION_GF61) {
                g_cust_cap.cali_switch.nv_dpd_cali_done_chn = hwifi_cust_read_nvram_dpd_state_mp16() & 0xFFFF;
            }
#endif
            if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_HCC_FLOWCTRL_TYPE)) {
                g_cust_cap.hcc_flowctrl_switch = OAL_TRUE;
                hwifi_custom_adapt_hcc_flowctrl_type_mp16(&g_cust_cap.hcc_flowctrl_type);
            }
            data_len = sizeof(g_cust_cap);
            ret = memcpy_s(netbuf_data, WLAN_LARGE_NETBUF_SIZE, (uint8_t *)(&g_cust_cap), data_len);
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_get_data::unknown us_syn_id[%d]", syn_id);
            break;
    }

    if (ret != EOK) {
        oam_error_log3(0, OAM_SF_CFG,
            "{hwifi_hcc_custom_get_data::memcpy_s fail[%d] syn_id[%u] data_len[%u]}", ret, syn_id, data_len);
    }
    return data_len;
}

/*
 * 函 数 名  : hwifi_hcc_custom_ini_data_buf_mp16
 * 功能描述  : 下发定制化配置命令
 */
static int32_t hwifi_hcc_custom_ini_data_buf_mp16(uint16_t us_syn_id)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint32_t data_len;
    int32_t l_ret;
    uint32_t max_data_len = hcc_get_handler(HCC_EP_WIFI_DEV)->tx_max_buf_len;
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_handler(HCC_EP_WIFI_DEV);
    if (hcc == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf hcc::is is null");
        return -OAL_EFAIL;
    }
    pst_netbuf = hwifi_hcc_custom_netbuf_alloc();
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    /* 组netbuf */
    data_len = (uint32_t)hwifi_hcc_custom_get_data_mp16(us_syn_id, pst_netbuf);
    if (data_len > max_data_len) {
        oam_error_log2(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::got wrong data_len[%d] max_len[%d]",
            data_len, max_data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    if (data_len == 0) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf::data is null us_syn_id[%d]", us_syn_id);
        oal_netbuf_free(pst_netbuf);
        return OAL_SUCC;
    }

    if ((pst_netbuf->data_len) || (pst_netbuf->data == NULL)) {
        oal_io_print("netbuf:0x%lx, len:%d\r\n", (uintptr_t)pst_netbuf, pst_netbuf->data_len);
        oal_netbuf_free(pst_netbuf);
        return OAL_FAIL;
    }

    oal_netbuf_put(pst_netbuf, data_len);
    hcc_hdr_param_init(&st_hcc_transfer_param, HCC_ACTION_TYPE_CUSTOMIZE, us_syn_id, 0, HCC_FC_WAIT, DATA_HI_QUEUE);

    l_ret = (int32_t)hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "hwifi_hcc_custom_ini_data_buf fail ret[%d]pst_netbuf[0x%lx]",
                       l_ret, (uintptr_t)pst_netbuf);
        oal_netbuf_free(pst_netbuf);
    }

    return l_ret;
}

uint32_t hwifi_hcc_customize_h2d_data_cfg_mp16(void)
{
    int32_t l_ret;

    /* wifi上电时重读定制化配置 */
    if ((wlan_chip_custom_host_read_cfg_init()) != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg data ret[%d]");
    }

    /* 下发功能相关定制化 */
    l_ret = hwifi_hcc_custom_ini_data_buf_mp16(CUSTOM_CFGID_CAP_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg cap data fail, ret[%d]", l_ret);
        return INI_FAILED;
    }
    /* 下发device init命令 */
    l_ret = hwifi_hcc_custom_ini_data_buf_mp16(CUSTOM_CFGID_INI_ID);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_error_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg init device fail, ret[%d]", l_ret);
        return INI_FAILED;
    }

    return INI_SUCC;
}

#define WAL_CUST_DATA_SEND_LEN ((WLAN_LARGE_NETBUF_SIZE) - HMAC_NETBUF_OFFSET)

static uint32_t hwifi_custom_data_send(oal_net_device_stru *cfg_net_dev,
    uint8_t *cust_param, int32_t param_len, uint8_t cfg_id)
{
    uint32_t ret = OAL_SUCC;

    uint16_t data_len;
    int32_t remain_len = param_len;
    uint8_t *data = cust_param;
    mac_vap_stru *mac_vap = oal_net_dev_priv(cfg_net_dev);

    while (remain_len > 0) {
        data_len = (uint16_t)oal_min(remain_len, WAL_CUST_DATA_SEND_LEN);

        /* 如果所有参数都在有效范围内，则下发配置值 */
        ret += wal_send_custom_data(mac_vap, data_len, data, cfg_id);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_error_log2(0, OAM_SF_ANY, "{hwifi_custom_data_send::cutsom data cfg id[id] fail[%d]!}", cfg_id, ret);
        }
        data += data_len;
        remain_len -= (int32_t)data_len;
    }

    return ret;
}

/*
 * 函 数 名  : hwifi_cust_rf_front_data_send
 * 功能描述  : hw 2g 5g 前端定制化
 */
static uint32_t hwifi_cust_rf_front_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_rf_front),
        sizeof(g_cust_rf_front), WLAN_CFGID_SET_CUS_RF_CFG);
}

static uint32_t hwifi_cust_rf_cali_data_send(oal_net_device_stru *cfg_net_dev)
{
    g_cust_rf_cali.band_5g_enable = mac_device_check_5g_enable_per_chip();
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_rf_cali),
        sizeof(g_cust_rf_cali), WLAN_CFGID_SET_CUS_RF_CALI);
}
static uint32_t hwifi_cust_nv_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_pow),
        sizeof(g_cust_pow), WLAN_CFGID_SET_CUS_POW);
}

static uint32_t hwifi_cust_nv_dyn_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    return hwifi_custom_data_send(cfg_net_dev, (uint8_t *)(&g_cust_dyn_pow),
        sizeof(g_cust_dyn_pow), WLAN_CFGID_SET_CUS_DYN_POW_CALI);
}
/*
 * 函 数 名  : hwifi_cust_rf_cali_event_send
 * 功能描述  : 校准触发事件下发
 */
static uint32_t hwifi_cust_rf_cali_event_send(oal_net_device_stru *cfg_net_dev)
{
    wlan_bus_type_enum_uint8 bus_type = WLAN_BUS_PCIE; /* 总线类型标记 */
    if (hcc_is_pcie() == OAL_FALSE) {
        bus_type = WLAN_BUS_SDIO;
    }
    return hwifi_custom_data_send(cfg_net_dev, &bus_type, sizeof(wlan_bus_type_enum_uint8), WLAN_CFGID_SET_RF_CALI_RUN);
}
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
typedef void (*fe_custom_get_section_cb)(uint8_t band, uint32_t *memsize, uint32_t *offset, uint32_t *state);
typedef uint8_t* (*fe_get_customize_mem_addr_cb)(void);
static uint32_t hwifi_cust_nv_fcc_ce_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    uint8_t band_idx;
    uint32_t ret;
    uint32_t memsize, state, memoffset;
    fe_get_customize_mem_addr_cb fe_get_customize_mem_addr = NULL;
    uint8_t  *nv_addr = NULL;
    fe_custom_get_section_cb fe_custom_get_section = NULL;

    fe_custom_get_section = (fe_custom_get_section_cb)fe_get_out_if_cb(FE_CUSTOM_GET_SECTION_INFO);
    fe_get_customize_mem_addr = (fe_get_customize_mem_addr_cb)fe_get_out_if_cb(FE_CUSTOM_GET_MEM_ADDR);
    if (fe_custom_get_section == NULL || fe_get_customize_mem_addr == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "fe_custom_get_section_info: fe_get_out_if_cb failed!");
        return OAL_FAIL;
    }

    for (band_idx = WLAN_CALI_BAND_2G; band_idx < WLAN_CALI_BAND_BUTT; band_idx++) {
        fe_custom_get_section(band_idx, &memsize, &memoffset, &state);
        if (state != OAL_TRUE) {
            oam_error_log4(0, 0, "hwifi_cust_nv_fcc_ce_pow_data_send::band[%d] state[%d] memsize[%d], memoffset[%d]",
                band_idx, state, memsize, memoffset);
        }
        nv_addr = fe_get_customize_mem_addr();
        ret = hwifi_custom_data_send(cfg_net_dev, (uint8_t *)nv_addr + memoffset,
            memsize, ((band_idx << BIT4) | WLAN_CFGID_SET_CUS_FCC_CE_POW));
        if (ret != OAL_SUCC) {
            oam_error_log2(0, 0, "hwifi_cust_nv_fcc_ce_pow_data_send::fail[%d] rf[%d]", ret, band_idx);
            return ret;
        }
    }
    return ret;
}
#else
static uint32_t hwifi_cust_nv_fcc_ce_pow_data_send(oal_net_device_stru *cfg_net_dev)
{
    uint8_t rf_idx;
    uint32_t ret;

    for (rf_idx = 0; rf_idx < HD_EVENT_RF_NUM; rf_idx++) {
        g_cust_pow.fcc_ce_param[rf_idx].rf_id = rf_idx;
        ret = hwifi_custom_data_send(cfg_net_dev, (uint8_t *)&(g_cust_pow.fcc_ce_param[rf_idx]),
            sizeof(hal_cfg_custom_fcc_txpwr_limit_stru), WLAN_CFGID_SET_CUS_FCC_CE_POW);
        if (ret != OAL_SUCC) {
            oam_error_log2(0, 0, "hwifi_cust_nv_fcc_ce_pow_data_send::fail[%d] rf[%d]", ret, rf_idx);
            return ret;
        }
    }
    return ret;
}
#endif

uint32_t hwifi_config_init_nvram_main_mp16(oal_net_device_stru *pst_cfg_net_dev)
{
    hwifi_cust_nv_pow_data_send(pst_cfg_net_dev);
    return OAL_SUCC;
}
uint32_t hwifi_config_init_fcc_ce_params_mp16(oal_net_device_stru *pst_cfg_net_dev)
{
    hwifi_cust_nv_fcc_ce_pow_data_send(pst_cfg_net_dev);
    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
typedef uint8_t (*fe_get_customize_mem_size_cb)(void);
static void hwifi_fe_cust_ini_send(oal_net_device_stru *cfg_net_dev)
{
    fe_get_customize_mem_addr_cb fe_get_customize_mem_addr = NULL;
    fe_get_customize_mem_size_cb fe_get_customize_mem_size = NULL;
    uint8_t  *nv_addr = NULL;
    uint32_t  nv_len = 0;
    fe_get_customize_mem_addr = (fe_get_customize_mem_addr_cb)fe_get_out_if_cb(FE_CUSTOM_GET_MEM_ADDR);
    fe_get_customize_mem_size = (fe_get_customize_mem_size_cb)fe_get_out_if_cb(FE_CUSTOM_GET_CUSTOMIZE_SIZE);
    if (oal_unlikely(fe_get_customize_mem_addr == NULL || fe_get_customize_mem_size == NULL)) {
        oam_error_log0(0, OAM_SF_CFG, "fe_custom_get_section_info: fe_get_out_if_cb failed!");
        return;
    }

    nv_addr = fe_get_customize_mem_addr();
    nv_len = fe_get_customize_mem_size();
    hwifi_custom_data_send(cfg_net_dev, (uint8_t *)nv_addr, nv_len, WLAN_CFGID_SET_FE_ALL_CUST);
}
#endif
static void hwifi_config_init_ini_main_mp16(oal_net_device_stru *cfg_net_dev)
{
    hwifi_cust_nv_pow_data_send(cfg_net_dev);

    /* 国家码 */
    hwifi_config_init_ini_country(cfg_net_dev);
    /* 定制化国家码首次读取，重新下发管制域功率参数 */
    hwifi_force_refresh_rf_params(cfg_net_dev);
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
    /* 自学习国家码初始化 */
    hwifi_config_selfstudy_init_country(cfg_net_dev);
#endif
    hwifi_cust_rf_front_data_send(cfg_net_dev);
    hwifi_cust_nv_dyn_pow_data_send(cfg_net_dev);
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
    hwifi_fe_cust_ini_send(cfg_net_dev);
#endif
}

void wal_send_cali_data_mp16(oal_net_device_stru *cfg_net_dev)
{
    h2d_cali_trans_stru h2d_scan_cali_trans = {0};
    h2d_cali_trans_stru h2d_scan_cali_trans_slave = {0};
    h2d_cali_trans_stru h2d_comm_cali_trans = {0};

    hmac_set_scan_cali_data(oal_net_dev_priv(cfg_net_dev));
    h2d_scan_cali_trans.cali_data_type = WLAN_SCAN_ALL_CHN;
    hmac_send_cali_data_mp16(oal_net_dev_priv(cfg_net_dev), &h2d_scan_cali_trans);
    /* 下发辅路扫描校准数据 */
    hmac_set_scan_dbdc_cali_data();
    h2d_scan_cali_trans_slave.cali_data_type = WLAN_SCAN_ALL_CHN_SLAVE;
    hmac_send_cali_data_mp16(oal_net_dev_priv(cfg_net_dev), &h2d_scan_cali_trans_slave);
    h2d_comm_cali_trans.cali_data_type = WLAN_CALI_CTL;
    hmac_send_cali_data_mp16(oal_net_dev_priv(cfg_net_dev), &h2d_comm_cali_trans);
}

uint32_t wal_custom_cali_mp16(void)
{
    oal_net_device_stru *net_dev;
    uint32_t ret = OAL_SUCC;

    net_dev = wal_config_get_netdev("Hisilicon0", OAL_STRLEN("Hisilicon0"));  // 通过cfg vap0来下c0 c1校准
    if (net_dev == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_custom_cali::the net_device is not exist!}");
        return OAL_ERR_CODE_PTR_NULL;
    } else {
        /* 调用oal_dev_get_by_name后，必须调用oal_dev_put使net_dev的引用计数减一 */
        oal_dev_put(net_dev);
    }

    hwifi_config_init_ini_main_mp16(net_dev);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 校准数据下发 */
    wlan_chip_send_cali_data(net_dev);
    g_custom_cali_done = OAL_TRUE;
    wal_send_cali_matrix_data(net_dev);
#endif
    /* 下发参数 */
    ret += hwifi_cust_rf_cali_data_send(net_dev);
    ret += hwifi_cust_rf_cali_event_send(net_dev);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_custom_cali:init_dts_main fail!}");
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* 防止翻转为0导致开wifi时间过长 */
    if (g_custom_cali_cnt >= OAL_UINT8_MAX) {
        g_custom_cali_cnt = 1;
    }
    g_custom_cali_cnt++;
#endif
    return ret;
}

static void hwifi_config_bypass_extlna_ini_param_mp16(void)
{
    mac_rx_dyn_bypass_extlna_stru *rx_dyn_bypass_extlna_switch = NULL;
    rx_dyn_bypass_extlna_switch = mac_vap_get_rx_dyn_bypass_extlna_switch();

    rx_dyn_bypass_extlna_switch->uc_ini_en = g_cust_host.dyn_extlna.switch_en;
    rx_dyn_bypass_extlna_switch->uc_cur_status = OAL_TRUE; /* 默认低功耗场景 */
    rx_dyn_bypass_extlna_switch->us_throughput_high = g_cust_host.dyn_extlna.throughput_high;
    rx_dyn_bypass_extlna_switch->us_throughput_low = g_cust_host.dyn_extlna.throughput_low;
}

static void hwifi_config_host_global_ini_param_extend_mp16(void)
{
#ifdef _PRE_WLAN_FEATURE_MBO
    g_uc_mbo_switch = g_cust_cap.mbo_switch;
#endif
    g_uc_dbac_dynamic_switch = g_cust_cap.dbac_dynamic_switch;

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &g_cust_cap.hiex_cap,
            sizeof(mac_hiex_cap_stru)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_config_host_global_ini_param_extend::hiex cap memcpy fail!");
        }
    }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        g_mac_ftm_cap = g_cust_cap.ftm_cap;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    g_mcast_ampdu_cfg.mcast_ampdu_enable = g_cust_cap.mcast_ampdu_enable;
#endif
    g_pt_mcast_enable = g_cust_cap.pt_mcast_enable;
}


static void hwifi_config_dmac_freq_ini_param_mp16(void)
{
    uint8_t uc_flag = OAL_TRUE;
    uint8_t uc_index;
    /******************************************** 自动调频 ********************************************/
    for (uc_index = 0; uc_index < DEV_WORK_FREQ_LVL_NUM; ++uc_index) {
        if (g_cust_cap.dev_frequency[uc_index].cpu_freq_type > FREQ_HIGHEST) {
            uc_flag = OAL_FALSE;
            break;
        }
    }

    if (uc_flag) {
        for (uc_index = 0; uc_index < DEV_WORK_FREQ_LVL_NUM; ++uc_index) {
            g_host_speed_freq_level[uc_index].speed_level = g_cust_cap.dev_frequency[uc_index].speed_level;
            g_device_speed_freq_level[uc_index].uc_device_type = g_cust_cap.dev_frequency[uc_index].cpu_freq_type;
        }
    }
}

static void hwifi_config_host_amsdu_th_ini_param_mp16(void)
{
    mac_small_amsdu_switch_stru *small_amsdu_switch = mac_vap_get_small_amsdu_switch();

    g_st_tx_large_amsdu.uc_host_large_amsdu_en = g_cust_host.large_amsdu.large_amsdu_en;
    g_st_tx_large_amsdu.us_amsdu_throughput_high = g_cust_host.large_amsdu.throughput_high;
    g_st_tx_large_amsdu.us_amsdu_throughput_middle = g_cust_host.large_amsdu.throughput_middle;
    g_st_tx_large_amsdu.us_amsdu_throughput_low = g_cust_host.large_amsdu.throughput_low;
    oal_io_print("ampdu+amsdu lareg amsdu en[%d],high[%d],low[%d],middle[%d]\r\n",
        g_st_tx_large_amsdu.uc_host_large_amsdu_en, g_st_tx_large_amsdu.us_amsdu_throughput_high,
        g_st_tx_large_amsdu.us_amsdu_throughput_low, g_st_tx_large_amsdu.us_amsdu_throughput_middle);

    small_amsdu_switch->uc_ini_small_amsdu_en = g_cust_host.small_amsdu.en_switch;
    small_amsdu_switch->us_small_amsdu_throughput_high = g_cust_host.small_amsdu.throughput_high;
    small_amsdu_switch->us_small_amsdu_throughput_low = g_cust_host.small_amsdu.throughput_low;
    small_amsdu_switch->us_small_amsdu_pps_high = g_cust_host.small_amsdu.pps_high;
    small_amsdu_switch->us_small_amsdu_pps_low = g_cust_host.small_amsdu.pps_low;
    oal_io_print("SMALL AMSDU SWITCH en[%d],high[%d],low[%d]\r\n",
        small_amsdu_switch->uc_ini_small_amsdu_en, small_amsdu_switch->us_small_amsdu_throughput_high,
        small_amsdu_switch->us_small_amsdu_throughput_low);
}

static void hwifi_config_performance_ini_param_mp16(void)
{
    mac_rx_buffer_size_stru *rx_buffer_size = mac_vap_get_rx_buffer_size();

#ifdef _PRE_WLAN_FEATURE_AMPDU_TX_HW
    g_st_ampdu_hw.uc_ampdu_hw_en = g_cust_host.tx_ampdu.ampdu_hw_en;
    g_st_ampdu_hw.us_throughput_high = g_cust_host.tx_ampdu.throughput_high;
    g_st_ampdu_hw.us_throughput_low = g_cust_host.tx_ampdu.throughput_low;
    oal_io_print("ampdu_hw enable[%d]H[%u]L[%u]\r\n", g_st_ampdu_hw.uc_ampdu_hw_en, g_st_ampdu_hw.us_throughput_high,
        g_st_ampdu_hw.us_throughput_low);
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    hwifi_config_host_amsdu_th_ini_param_mp16();
#endif
#ifdef _PRE_WLAN_TCP_OPT
    g_st_tcp_ack_filter.uc_tcp_ack_filter_en = g_cust_host.tcp_ack_filt.filter_en;
    g_st_tcp_ack_filter.us_rx_filter_throughput_high = g_cust_host.tcp_ack_filt.throughput_high;
    g_st_tcp_ack_filter.us_rx_filter_throughput_low = g_cust_host.tcp_ack_filt.throughput_low;
    oal_io_print("tcp ack filter en[%d],high[%d],low[%d]\r\n", g_st_tcp_ack_filter.uc_tcp_ack_filter_en,
        g_st_tcp_ack_filter.us_rx_filter_throughput_high, g_st_tcp_ack_filter.us_rx_filter_throughput_low);
#endif

    g_uc_host_rx_ampdu_amsdu = g_cust_host.host_rx_ampdu_amsdu;
    oal_io_print("Rx:ampdu+amsdu skb en[%d]\r\n", g_uc_host_rx_ampdu_amsdu);

    rx_buffer_size->us_rx_buffer_size = g_cust_host.rx_buffer_size;
    rx_buffer_size->en_rx_ampdu_bitmap_ini = (g_cust_host.rx_buffer_size > 0) ? OAL_TRUE : OAL_FALSE;
    oal_io_print("Rx:ampdu bitmap size[%d]\r\n", rx_buffer_size->us_rx_buffer_size);
}

static void hwifi_config_tcp_ack_buf_ini_param_mp16(void)
{
    mac_tcp_ack_buf_switch_stru *tcp_ack_buf_switch = mac_vap_get_tcp_ack_buf_switch();

    g_cust_cap.device_ini_tcp_ack_buf_en = g_cust_host.tcp_ack_buf.device_buf_en;
    hmac_device_tcp_buf_init(g_cust_host.tcp_ack_buf.device_buf_en);

    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en = g_cust_host.tcp_ack_buf.host_buf_en;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high = g_cust_host.tcp_ack_buf.throughput_high;
    tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low = g_cust_host.tcp_ack_buf.throughput_low;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_high_40m = g_cust_host.tcp_ack_buf.throughput_high_40m;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_low_40m = g_cust_host.tcp_ack_buf.throughput_low_40m;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_high_80m = g_cust_host.tcp_ack_buf.throughput_high_80m;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_low_80m = g_cust_host.tcp_ack_buf.throughput_low_80m;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_high_160m = g_cust_host.tcp_ack_buf.throughput_high_160m;
    tcp_ack_buf_switch->tcp_ack_buf_throughput_low_160m = g_cust_host.tcp_ack_buf.throughput_low_160m;
    tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en = g_cust_host.tcp_ack_buf.buf_userctl_test_en;
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high = g_cust_host.tcp_ack_buf.buf_userctl_high;
    tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low = g_cust_host.tcp_ack_buf.buf_userctl_low;

    oal_io_print("TCP ACK BUF en[%d],high/low:20M[%d]/[%d],40M[%d]/[%d],80M[%d]/[%d],160M[%d]/[%d]\
        TCP ACK BUF USERCTL[%d], userctl[%d]/[%d]",
        tcp_ack_buf_switch->uc_ini_tcp_ack_buf_en, tcp_ack_buf_switch->us_tcp_ack_buf_throughput_high,
        tcp_ack_buf_switch->us_tcp_ack_buf_throughput_low, tcp_ack_buf_switch->tcp_ack_buf_throughput_high_40m,
        tcp_ack_buf_switch->tcp_ack_buf_throughput_low_40m, tcp_ack_buf_switch->tcp_ack_buf_throughput_high_80m,
        tcp_ack_buf_switch->tcp_ack_buf_throughput_low_80m, tcp_ack_buf_switch->tcp_ack_buf_throughput_high_160m,
        tcp_ack_buf_switch->tcp_ack_buf_throughput_low_160m, tcp_ack_buf_switch->uc_ini_tcp_ack_buf_userctl_test_en,
        tcp_ack_buf_switch->us_tcp_ack_buf_userctl_high, tcp_ack_buf_switch->us_tcp_ack_buf_userctl_low);
}


static void hwifi_set_voe_custom_param_mp16(void)
{
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11k = (g_cust_cap.voe_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11v = (g_cust_cap.voe_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r = (g_cust_cap.voe_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_11r_ds =
        (g_cust_cap.voe_switch_mask & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_adaptive11r =
        (g_cust_cap.voe_switch_mask & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_voe_custom_cfg.en_nb_rpt_11k =
        (g_cust_cap.voe_switch_mask & BIT5) ? OAL_TRUE : OAL_FALSE;

    return;
}

void hwifi_config_host_global_11ax_ini_param_mp16(void)
{
    g_pst_mac_device_capability[0].en_11ax_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_ignore_non_he_cap_from_beacon =
        (g_cust_cap.wifi_11ax_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_11ax_aput_he_cap_switch =
        (g_cust_cap.wifi_11ax_switch_mask & BIT3) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_responder_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT4) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_twt_requester_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT5) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_btwt_requester_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT6) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_flex_twt_support =
        (g_cust_cap.wifi_11ax_switch_mask & BIT7) ? OAL_TRUE : OAL_FALSE;
#endif

    g_pst_mac_device_capability[0].bit_multi_bssid_switch =
        (g_cust_cap.mult_bssid_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_11AX
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_htc_include =
        (g_cust_cap.htc_switch_mask & BIT0) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_om_in_data =
        (g_cust_cap.htc_switch_mask & BIT1) ? OAL_TRUE : OAL_FALSE;
    g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch =
        (g_cust_cap.htc_switch_mask & BIT2) ? OAL_TRUE : OAL_FALSE;
#endif
oal_io_print("hwifi_config_host_global_11ax_ini_param en[%d],rom cap[%d]\r\n",
    g_pst_mac_device_capability[0].en_11ax_switch, g_st_mac_device_custom_cfg.st_11ax_custom_cfg.bit_rom_cap_switch);
}


void hwifi_config_host_global_ini_param_mp16(void)
{
    mac_rx_dyn_bypass_extlna_stru *rx_dyn_bypass_extlna_switch = mac_vap_get_rx_dyn_bypass_extlna_switch();

    /******************************************** 性能 ********************************************/
    wlan_chip_cpu_freq_ini_param_init();

    hwifi_config_tcp_ack_buf_ini_param_mp16();

    hwifi_config_dmac_freq_ini_param_mp16();

    hwifi_config_bypass_extlna_ini_param_mp16();
    oal_io_print("DYN_BYPASS_EXTLNA SWITCH en[%d],high[%d],low[%d]\r\n", rx_dyn_bypass_extlna_switch->uc_ini_en,
        rx_dyn_bypass_extlna_switch->us_throughput_high, rx_dyn_bypass_extlna_switch->us_throughput_low);

    hwifi_config_performance_ini_param_mp16();

    hwifi_config_host_global_ini_param_extend_mp16();

    hwifi_set_voe_custom_param_mp16();
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hwifi_config_host_global_11ax_ini_param_mp16();
    }
#endif
    return;
}
/*
 * 函 数 名  : hwifi_config_cpu_freq_ini_param_mp16
 * 功能描述  : 初始化host全局变量
 * 1.日    期  : 2019年11月22日
 *   修改内容  : 新生成函数
 */
void hwifi_config_cpu_freq_ini_param_mp16(void)
{
    g_freq_lock_control.uc_lock_dma_latency = (g_cust_host.freq_lock.dma_latency_val > 0) ?  OAL_TRUE : OAL_FALSE;
    g_freq_lock_control.dma_latency_value = g_cust_host.freq_lock.dma_latency_val;
    oal_io_print("DMA latency[%d]latency_val[%u]\r\n", g_freq_lock_control.uc_lock_dma_latency,
        g_freq_lock_control.dma_latency_value);
    g_freq_lock_control.us_lock_cpu_th_high = g_cust_host.freq_lock.lock_cpu_th_high;
    g_freq_lock_control.us_lock_cpu_th_low = g_cust_host.freq_lock.lock_cpu_th_low;

    g_freq_lock_control.en_irq_affinity = g_cust_host.freq_lock.en_irq_affinity;
    g_freq_lock_control.us_throughput_irq_high = g_cust_host.freq_lock.throughput_irq_high;
    g_freq_lock_control.us_throughput_irq_low = g_cust_host.freq_lock.throughput_irq_low;
    g_freq_lock_control.irq_pps_high = g_cust_host.freq_lock.irq_pps_high;
    g_freq_lock_control.irq_pps_low = g_cust_host.freq_lock.irq_pps_low;
    oal_io_print("hwifi_config_cpu_freq_ini_param irq affinity enable[%d]High_th[%u]Low_th[%u]\r\n",
        g_freq_lock_control.en_irq_affinity, g_freq_lock_control.us_throughput_irq_high,
        g_freq_lock_control.us_throughput_irq_low);
}

/*
 * 函 数 名  : hwifi_config_init_etc
 * 功能描述  : 定制化读取结果打印
 */
static void hwifi_print_cust_val(wlan_cust_cfg_cmd *cfg_cmd)
{
    uint8_t idx;
    uint8_t *c_data = NULL;
    uint32_t *l_data = NULL;
    uint16_t *s_data = NULL;

    if (cfg_cmd->data_addr == NULL || cfg_cmd->name == NULL) {
        oal_io_print("hwifi_print_cust_val::cmd [%d] cmd_name or data_addr is NULL!", cfg_cmd->case_entry);
        return;
    }
    oal_io_print("hwifi_print_cust_val:name %s id %d = ", cfg_cmd->name, cfg_cmd->case_entry);

    if (cfg_cmd->data_type == sizeof(uint8_t)) {
        c_data = (uint8_t *)cfg_cmd->data_addr;
    } else if (cfg_cmd->data_type == sizeof(uint16_t)) {
        s_data = (uint16_t *)(cfg_cmd->data_addr);
    } else if (cfg_cmd->data_type == sizeof(uint32_t)) {
        l_data = (uint32_t *)(cfg_cmd->data_addr);
    }

    for (idx = 0; idx < cfg_cmd->data_num; idx++) {
        if (cfg_cmd->data_type == sizeof(uint8_t)) {
            oal_io_print("[0x%x]", *c_data);
            c_data++;
        } else if (cfg_cmd->data_type == sizeof(uint16_t)) {
            oal_io_print("[0x%x]", *s_data);
            s_data++;
        } else if (cfg_cmd->data_type == sizeof(uint32_t)) {
            oal_io_print("[0x%x]", *l_data);
            l_data++;
        }
    }
}

static void hwifi_show_customize_info(wlan_cust_cfg_cmd *wlan_cust_cfg, uint32_t num)
{
    uint32_t i;
    for (i = 0; i < num; i++) {
        hwifi_print_cust_val(&wlan_cust_cfg[i]);
    }
}

void hwifi_show_customize_info_mp16(void)
{
    oal_io_print("\nhwifi_show_customize_info\n");
    oal_io_print("CUSTOM_INFO:host_info");
    hwifi_show_customize_info(g_wifi_cfg_host, oal_array_size(g_wifi_cfg_host));
    oal_io_print("CUSTOM_INFO:cap_info");
    hwifi_show_customize_info(g_wifi_cfg_cap, oal_array_size(g_wifi_cfg_cap));
    oal_io_print("CUSTOM_INFO:rf_cali_info");
    hwifi_show_customize_info(g_wifi_cfg_rf_cali, oal_array_size(g_wifi_cfg_rf_cali));
    oal_io_print("CUSTOM_INFO:pow_info");
    hwifi_show_customize_info(g_wifi_cfg_pow, oal_array_size(g_wifi_cfg_pow));
    oal_io_print("CUSTOM_INFO:dyn_pow_info");
    hwifi_show_customize_info(g_wifi_cfg_dyn_pow, oal_array_size(g_wifi_cfg_dyn_pow));
}

/*
 * 函 数 名  : hwifi_get_sar_ctrl_params_mp16
 * 功能描述  : 获取定制中降sar的当前档位的功率值
 * 1.日    期  : 2015年1月20日
 *   修改内容  : 新生成函数
 */
uint32_t hwifi_get_sar_ctrl_params_mp16(uint8_t lvl_num, uint8_t *data_addr,
    uint16_t *data_len, uint16_t dest_len)
{
    wlan_sar_trigger_cfg_stru *sar_trigger_param = (wlan_sar_trigger_cfg_stru *)data_addr;
    *data_len = sizeof(wlan_sar_trigger_cfg_stru);
    if ((lvl_num <= CUS_NUM_OF_SAR_LVL) && (lvl_num > 0)) {
        lvl_num--;
        if ((memcpy_s(&sar_trigger_param->sar_avg_pow, dest_len,
            &g_cust_pow.sar_ctrl_params[lvl_num], sizeof(wlan_cust_sar_cfg_stru)) != EOK) ||
            (memcpy_s(sar_trigger_param->sar_cust_slide.sar_total_time, dest_len,
            g_cust_cap.sar_total_time[lvl_num], sizeof(sar_trigger_param->sar_cust_slide.sar_total_time)) != EOK)) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_get_sar_ctrl_params::memcpy fail!");
            return OAL_FAIL;
        }
    } else {
        memset_s(data_addr, dest_len, 0xFF, dest_len);
    }
    return OAL_SUCC;
}
