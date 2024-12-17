/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2020年5月20日
 */

/* 头文件包含 */
#include "hisi_customize_wifi_gf61.h"
#include "hisi_customize_wifi_mp16.h"
#include "hisi_customize_wifi_mp13.h"
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve_interface.h"
#include "hisi_conn_nve_interface_gf61.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#endif

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

/* 终端头文件 */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#define hisi_nve_direct_access(...)  0
#else
#ifdef HISI_CONN_NVE_SUPPORT
#include <linux/mtd/hisi_nve_interface.h>
#include "external/nve_info_interface.h"
#endif
#endif
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
#include "fe_extern_if.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CUSTOMIZE_WIFI_GF61_C

#define XOCALICRC1 1
#define XOCALICRC2 2

#ifdef HISI_CONN_NVE_SUPPORT
static nv_pow_stru_gf61 g_cust_nvram_info = {}; /* NVRAM数据 */

static wlan_cust_dpn_cfg_cmd_gf61 g_wifi_cfg_dpn_nv[] = {
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_11b[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_11b[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_20[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_20[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_40[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_40[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_80[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_80[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_160[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_160[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[0][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M])},
    /* 低功率段dpn */
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_11b[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_11b[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_11B])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_20[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM20])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ZERO][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40][WLAN_2G_CHN_IDX0],
     &g_cust_nvram_info.delta_gain_2g_ofdm_40[WLAN_RF_CHANNEL_ONE][WLAN_2G_CHN_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_2g[DYN_PWR_CUST_SNGL_MODE_OFDM40])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_20[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_20[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_20M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_40[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_40[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_40M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_80[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_80[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_80M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_160[WLAN_RF_CHANNEL_ZERO][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ZERO].dpn_5g[WLAN_BW_CAP_160M])},
    {&g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M][WLAN_5G_BAND_IDX0],
     &g_cust_nvram_info.delta_gain_5g_160[WLAN_RF_CHANNEL_ONE][WLAN_5G_BAND_IDX0],
     oal_array_size(g_cust_dyn_pow.dpn_para[1][WLAN_RF_CHANNEL_ONE].dpn_5g[WLAN_BW_CAP_160M])},
};

static int32_t hwifi_cust_read_nvram_flag_gf61(uint8_t read_mask)
{
    int32_t ret;
    oal_bool_enum_uint8 en_get_nvram_data_flag = OAL_FALSE;

    ret = hwifi_config_get_cust_val(&g_wifi_cfg_cap[WLAN_CFG_CAP_CALI_DATA_MASK]);
    if (ret == INI_SUCC) {
        en_get_nvram_data_flag = read_mask & g_cust_cap.cali_data_mask;
        if (read_mask & CUST_READ_NVRAM_POW_MASK & g_cust_cap.cali_data_mask) {
            oal_io_print("hwifi_cust_read_nvram_flag_gf61::cali_data_mask[%d] ,abandon POW nvram data!!\r\n",
                g_cust_cap.cali_data_mask);
            memset_s(&g_cust_nvram_info, sizeof(g_cust_nvram_info), 0,
                sizeof(g_cust_nvram_info));
        }
    }
    return en_get_nvram_data_flag == read_mask ? OAL_FALSE : OAL_TRUE;
}

/*
 * 函 数 名  : hwifi_cust_get_nvram_gf16
 * 功能描述  : 包括读取nvram中的dpint和校准系数值
 */
int32_t hwifi_cust_get_nvram_gf16(void)
{
    nv_pow_stru_gf61 *cust_nvram_info = NULL; /* NVRAM数组 */

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    if (hwifi_cust_read_nvram_flag_gf61(CUST_READ_NVRAM_MASK | CUST_READ_NVRAM_POW_MASK) == OAL_FALSE) {
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    cust_nvram_info = (nv_pow_stru_gf61 *)os_kzalloc_gfp(sizeof(nv_pow_stru_gf61));
    if (cust_nvram_info == NULL) {
        // this fail will be emergency in factory mode, cause tx test maybe fail
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::cust_nvram_info mem alloc fail!");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
    memset_s(cust_nvram_info, sizeof(nv_pow_stru_gf61), 0, sizeof(nv_pow_stru_gf61));

    /* 61读取wifi功率校准和xo校准 */
    if (read_cali_data_from_nvram((uint8_t *)cust_nvram_info, sizeof(nv_pow_stru_gf61)) != 0) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::nv didn't updated!");
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
    if (read_cali_data_from_nvram((uint8_t *)g_xo_pwr_diff, sizeof(uint16_t)) != 0) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::nv didn't updated!");
        os_mem_kfree(cust_nvram_info);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if ((g_xo_pwr_diff[XOCALICRC1] == 0) && (g_xo_pwr_diff[XOCALICRC2] == 0)) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "hwifi_cust_get_nvram::xo didn't cali!");
        g_xo_pwr_diff[0] = 0x4040;
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
 * 函 数 名  : hwifi_update_nv_dpn_gf61
 * 功能描述  : 读取nvram中的delta gain
 */
int32_t hwifi_update_nv_dpn_gf61(void)
{
    int32_t ret = 0;
    uint32_t cfg_id;
    uint32_t data_id;
    uint32_t cfg_butt = oal_array_size(g_wifi_cfg_dpn_nv);
    uint32_t cfg_half = cfg_butt >> 1;
    /* 判断定制化中是否使用nvram中的POW校准参数 */
    if (hwifi_cust_read_nvram_flag_gf61(CUST_READ_NVRAM_POW_MASK) == OAL_FALSE) {
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
#endif
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
OAL_STATIC uint32_t custom_read_fe_ini(void)
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
/*
 * 函 数 名  : custom_host_read_cfg_init
 * 功能描述  : 首次读取定制化配置文件总入口
 */
uint32_t hwifi_custom_host_read_cfg_init_gf61(void)
{
    int32_t ini_read_ret;
    int32_t l_nv_read_ret = 0;
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
    int32_t ret = OAL_SUCC;
#endif
#ifdef HISI_CONN_NVE_SUPPORT
    /* 读取nvram参数是否修改 */
    l_nv_read_ret += hwifi_cust_get_nvram_gf16();
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
        hwifi_update_nv_dpn_gf61();
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
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
    ret = custom_read_fe_ini();
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_io_print("custom_read_fe_ini NV fail l_ret[%d].\r\n", ret);
    }
#endif
    return OAL_SUCC;
}

void wal_send_cali_data_gf61(oal_net_device_stru *cfg_net_dev)
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
    hmac_config_send_event(cfg_net_dev->ml_priv, WLAN_CFGID_SEND_XOCALI_DATA,
                           sizeof(uint16_t), (uint8_t *)g_xo_pwr_diff);
}
