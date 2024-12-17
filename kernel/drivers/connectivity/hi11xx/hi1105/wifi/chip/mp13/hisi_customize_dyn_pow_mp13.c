/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : wifi动态功率校准定制化函数实现
 * 作者       : wifi
 * 创建日期   : 2015年10月08日
 */

/* 头文件包含 */
#include "hisi_customize_wifi_mp13.h"
#include "hisi_customize_config_dts_mp13.h"
#include "hisi_customize_config_priv_mp13.h"
#include "hisi_customize_config_cmd_mp13.h"
#include "hisi_customize_nvram_config_mp13.h"
#include "wlan_spec.h"
#include "wlan_chip_i.h"
#include "hisi_customize_wifi.h"
#include "wal_config.h"
#include "wal_linux_ioctl.h"

#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#ifdef CONFIG_ARCH_PLATFORM
#define nve_direct_access_interface(...)  0
#else
#define hisi_nve_direct_access(...)  0
#endif /* end for CONFIG_ARCH_PLATFORM */
#else
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve_interface.h"
#include "external/nve_info_interface.h"
#endif
#endif /* end for _PRE_CONFIG_READ_DYNCALI_E2PROM */

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CUSTOMIZE_DYN_POW_HI1103_C

/* 产测定制化参数数组 */
wlan_customize_pwr_fit_para_stru g_pro_line_params[WLAN_RF_CHANNEL_NUMS][DY_CALI_PARAMS_NUM] = {{{0}}};
OAL_STATIC uint8_t g_cust_nvram_info[WLAN_CFG_DTS_NVRAM_END][CUS_PARAMS_LEN_MAX] = {{0}}; /* NVRAM数组 */

static int16_t g_gs_extre_point_vals[WLAN_RF_CHANNEL_NUMS][DY_CALI_NUM_5G_BAND] = {{0}};

oal_bool_enum_uint8 g_en_fact_cali_completed = OAL_FALSE;

/* kunpeng eeprom */
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
OAL_STATIC wlan_cfg_nv_map_handler g_wifi_nvram_cfg_handler[WLAN_CFG_DTS_NVRAM_END] = {
    {"WITXCCK",  "pa2gccka0",   HWIFI_CFG_NV_WITXNVCCK_NUMBER,       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0, {0}, 0x100},
    {"WINVRAM",  "pa2ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_NVRAM_RATIO_PA2GA0,        {0}, 0x0},
    {"WITXLC0", "pa2g40a0",    HWIFI_CFG_NV_WITXL2G5G0_NUMBER,      WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,  {0}, 0x400},
    {"WINVRAM",  "pa5ga0",      HWIFI_CFG_NV_WINVRAM_NUMBER,         WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,    {0}, 0x80},
    {"WITXCCK",  "pa2gccka1",   HWIFI_CFG_NV_WITXNVCCK_NUMBER,       WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1, {0}, 0x180},
    {"WITXRF1",  "pa2ga1",      HWIFI_CFG_NV_WITXNVC1_NUMBER,        WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1,    {0}, 0x200},
    {"WITXLC1",  "pa2g40a1",    HWIFI_CFG_NV_WITXL2G5G1_NUMBER,      WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1,  {0}, 0x480},
    {"WITXRF1",  "pa5ga1",      HWIFI_CFG_NV_WITXNVC1_NUMBER,        WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1,    {0}, 0x280},
    {"WIC0_5GLOW",  "pa5glowa0",  HWIFI_CFG_NV_WITXNVBWC0_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW,  {0}, 0x300},
    {"WIC1_5GLOW",  "pa5glowa1",  HWIFI_CFG_NV_WITXNVBWC1_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW,  {0}, 0x380},
    // DPN
    {"WIC0CCK",  "mf2gccka0",   HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0,  {0}, 0x0},
    {"WC0OFDM",  "mf2ga0",      HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,   {0}, 0x0},
    {"C02G40M",  "mf2g40a0",    HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,   {0}, 0x0},
    {"WIC1CCK",  "mf2gccka1",   HWIFI_CFG_NV_MUFREQ_CCK_C1_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1,  {0}, 0x0},
    {"WC1OFDM",  "mf2ga1",      HWIFI_CFG_NV_MUFREQ_2G20_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1,   {0}, 0x0},
    {"C12G40M",  "mf2g40a1",    HWIFI_CFG_NV_MUFREQ_2G40_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1,   {0}, 0x0},
    {"WIC0_5G160M", "dpn160c0", HWIFI_CFG_NV_MUFREQ_5G160_C0_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0, {0}, 0x500},
    {"WIC1_5G160M", "dpn160c1", HWIFI_CFG_NV_MUFREQ_5G160_C1_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1, {0}, 0x580},
};
#else
OAL_STATIC wlan_cfg_nv_map_handler g_wifi_nvram_cfg_handler[WLAN_CFG_DTS_NVRAM_END] = {
    { "WITXCCK",    "pa2gccka0", HWIFI_CFG_NV_WITXNVCCK_NUMBER,  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0,  {0}},
    { "WINVRAM",    "pa2ga0",    HWIFI_CFG_NV_WINVRAM_NUMBER,    WLAN_CFG_NVRAM_RATIO_PA2GA0,         {0}},
    { "WITXLC0",    "pa2g40a0",  HWIFI_CFG_NV_WITXL2G5G0_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0,   {0}},
    { "WINVRAM",    "pa5ga0",    HWIFI_CFG_NV_WINVRAM_NUMBER,    WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0,     {0}},
    { "WITXCCK",    "pa2gccka1", HWIFI_CFG_NV_WITXNVCCK_NUMBER,  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1,  {0}},
    { "WITXRF1",    "pa2ga1",    HWIFI_CFG_NV_WITXNVC1_NUMBER,   WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1,     {0}},
    { "WITXLC1",    "pa2g40a1",  HWIFI_CFG_NV_WITXL2G5G1_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1,   {0}},
    { "WITXRF1",    "pa5ga1",    HWIFI_CFG_NV_WITXNVC1_NUMBER,   WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1,     {0}},
    { "W5GLOW0",    "pa5glowa0", HWIFI_CFG_NV_WITXNVBWC0_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW, {0}},
    { "W5GLOW1",    "pa5glowa1", HWIFI_CFG_NV_WITXNVBWC1_NUMBER, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW, {0}},
    // DPN
    { "WIC0CCK",     "mf2gccka0", HWIFI_CFG_NV_MUFREQ_CCK_C0_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0, {0}},
    { "WC0OFDM",     "mf2ga0",    HWIFI_CFG_NV_MUFREQ_2G20_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0,  {0}},
    { "C02G40M",     "mf2g40a0",  HWIFI_CFG_NV_MUFREQ_2G40_C0_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0,  {0}},
    { "WIC1CCK",     "mf2gccka1", HWIFI_CFG_NV_MUFREQ_CCK_C1_NUMBER,   WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1, {0}},
    { "WC1OFDM",     "mf2ga1",    HWIFI_CFG_NV_MUFREQ_2G20_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1,  {0}},
    { "C12G40M",     "mf2g40a1",  HWIFI_CFG_NV_MUFREQ_2G40_C1_NUMBER,  WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1,  {0}},
    { "W160MC0",     "dpn160c0",  HWIFI_CFG_NV_MUFREQ_5G160_C0_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0, {0}},
    { "W160MC1",     "dpn160c1",  HWIFI_CFG_NV_MUFREQ_5G160_C1_NUMBER, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1, {0}},
};
#endif

OAL_STATIC wlan_cfg_cmd g_nvram_pro_line_config_ini[] = {
    /* 产侧nvram参数 */
    { "nvram_pa2gccka0",  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0 },
    { "nvram_pa2ga0",     WLAN_CFG_NVRAM_RATIO_PA2GA0 },
    { "nvram_pa2g40a0",   WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A0 },
    { "nvram_pa5ga0",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 },
    { "nvram_pa2gccka1",  WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1 },
    { "nvram_pa2ga1",     WLAN_CFG_DTS_NVRAM_RATIO_PA2GA1 },
    { "nvram_pa2g40a1",   WLAN_CFG_DTS_NVRAM_RATIO_PA2G40A1 },
    { "nvram_pa5ga1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1 },
    { "nvram_pa5ga0_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW },
    { "nvram_pa5ga1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW },

    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G20_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_2G40_C1 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0 },
    { NULL, WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C1 },

    { "nvram_pa5ga0_band1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 },
    { "nvram_pa5ga1_band1",     WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1 },
    { "nvram_pa2gcwa0",         WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA0 },
    { "nvram_pa2gcwa1",         WLAN_CFG_DTS_NVRAM_RATIO_PA2GCWA1 },
    { "nvram_pa5ga0_band1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW },
    { "nvram_pa5ga1_band1_low", WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW },

    { "nvram_ppa2gcwa0", WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA0 },
    { "nvram_ppa2gcwa1", WLAN_CFG_DTS_NVRAM_RATIO_PPA2GCWA1 },

    { NULL, WLAN_CFG_DTS_NVRAM_PARAMS_BUTT },
};

int32_t hwifi_read_conf_from(uint8_t *puc_buffer_cust_nvram, uint8_t uc_idx)
{
    int32_t l_ret;
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#ifdef _PRE_SUSPORT_OEMINFO
    l_ret = is_hitv_miniproduct();
    if (l_ret == OAL_SUCC) {
        l_ret = read_conf_from_oeminfo(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
            g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
    } else {
        l_ret = read_conf_from_eeprom(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
            g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
    }
#else
    l_ret = read_conf_from_eeprom(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
        g_wifi_nvram_cfg_handler[uc_idx].eeprom_offset);
#endif
#else
    l_ret = read_conf_from_nvram(puc_buffer_cust_nvram, CUS_PARAMS_LEN_MAX,
        g_wifi_nvram_cfg_handler[uc_idx].nv_map_idx, g_wifi_nvram_cfg_handler[uc_idx].puc_nv_name);
#endif
    return l_ret;
}

static int32_t hwifi_set_cust_nvram_info(uint8_t *cust_nvram_info, int8_t **pc_token, int32_t *pl_params,
                                         int8_t *pc_ctx, uint8_t idx)
{
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    uint8_t param_idx;
    uint8_t times_idx = 0;
    *(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)) + OAL_STRLEN(*pc_token)) = *pc_end;

     /* 拟合系数获取检查 */
    if (idx <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) {
        /* 二次参数合理性检查 */
        *pc_token = oal_strtok(*pc_token, pc_sep, &pc_ctx);
        param_idx = 0;
        /* 获取定制化系数 */
        while (*pc_token != NULL) {
            /* 将字符串转换成10进制数 */
            *(pl_params + param_idx) = (int32_t)simple_strtol(*pc_token, NULL, 10);
            *pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            param_idx++;
        }
        if (param_idx % DY_CALI_PARAMS_TIMES) {
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                           g_wifi_nvram_cfg_handler[idx].nv_map_idx);
            memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                     CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0, CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
            return OAL_FALSE;
        }
        times_idx = param_idx / DY_CALI_PARAMS_TIMES;
        /* 二次项系数非0检查 */
        while (times_idx) {
            times_idx--;
            if (pl_params[(times_idx)*DY_CALI_PARAMS_TIMES] == 0) {
                oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::check NV id[%d]!",
                               g_wifi_nvram_cfg_handler[idx].nv_map_idx);
                memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                         CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0, CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
                break;
            }
        }

        return OAL_TRUE;
    }
    return OAL_FALSE;
}

static void hwifi_init_custnvram_and_debug_nvram_cfg(uint8_t *cust_nvram_info, uint8_t idx)
{
    memset_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0, CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    oal_io_print("hwifi_custom_host_read_dyn_cali_nvram::NVRAM get fail NV id[%d] name[%s] para[%s]!\r\n",
                 g_wifi_nvram_cfg_handler[idx].nv_map_idx, g_wifi_nvram_cfg_handler[idx].puc_nv_name,
                 g_wifi_nvram_cfg_handler[idx].puc_param_name);
}

static uint32_t hwifi_read_conf_and_set_cust_nvram(uint8_t *buffer_cust_nvram_tmp, uint8_t idx,
                                                   uint8_t *cust_nvram_info, int32_t *params,
                                                   oal_bool_enum_uint8 *tmp_fact_cali_completed)
{
    int32_t ret;
    uint8_t *end = ";";
    int8_t *str = NULL;
    int8_t *ctx = NULL;
    int8_t *token = NULL;

    ret = hwifi_read_conf_from(buffer_cust_nvram_tmp, idx);
    if (ret != INI_SUCC) {
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    str = OAL_STRSTR(buffer_cust_nvram_tmp, g_wifi_nvram_cfg_handler[idx].puc_param_name);
    if (str == NULL) {
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    /* 获取等号后面的实际参数 */
    str += (OAL_STRLEN(g_wifi_nvram_cfg_handler[idx].puc_param_name) + 1);
    token = oal_strtok(str, end, &ctx);
    if (token == NULL) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::get null value check NV id[%d]!",
                       g_wifi_nvram_cfg_handler[idx].nv_map_idx);
        hwifi_init_custnvram_and_debug_nvram_cfg(cust_nvram_info, idx);
        return OAL_TRUE;
    }

    ret = memcpy_s(cust_nvram_info + (idx * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)),
                   (WLAN_CFG_DTS_NVRAM_END - idx) * CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
                   token, OAL_STRLEN(token));
    if (ret != EOK) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::memcpy_s fail[%d]!", ret);
        return OAL_FALSE;
    }

    if (hwifi_set_cust_nvram_info(cust_nvram_info, &token, params, ctx, idx) == OAL_TRUE) {
        *tmp_fact_cali_completed = OAL_TRUE;
    }

    return OAL_TRUE;
}

OAL_STATIC int32_t hwifi_custom_check_nvram_data_flag(void)
{
    int32_t val, l_ret;
    oal_bool_enum_uint8 nv_flag = OAL_FALSE;

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &val);
    if (l_ret == OAL_SUCC) {
        nv_flag = !!(CUST_READ_NVRAM_MASK & (uint32_t)val);
        if (nv_flag) {
            oal_io_print("hwifi_custom_host_read_dyn_cali_nvram::nv flag[%d] to abandon nvram data!!\r\n", val);
            memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
            return INI_FILE_TIMESPEC_RECONFIG;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC void hwifi_custom_init_nvram_data(uint8_t *buffer_cust_nvram, int32_t *pl_params, uint8_t *cust_nvram_info)
{
    memset_s(buffer_cust_nvram, CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    memset_s(pl_params, DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t), 0,
             DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t));
    memset_s(cust_nvram_info, WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t), 0,
             WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
}

OAL_STATIC int32_t hwifi_custom_set_nvram_data(uint8_t *cust_nvram_info)
{
    if (oal_memcmp(cust_nvram_info, g_cust_nvram_info, sizeof(g_cust_nvram_info)) == 0) {
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (memcpy_s(g_cust_nvram_info, sizeof(g_cust_nvram_info),
                 cust_nvram_info, WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t)) != EOK) {
        return INI_FAILED;
    }
    return INI_NVRAM_RECONFIG;
}

/*
 * 函 数 名  : hwifi_get_init_nvram_params
 * 功能描述  : 获取定制化NV数据结构体
 */
oal_bool_enum_uint8 hwifi_get_g_fact_cali_completed(void)
{
    return g_en_fact_cali_completed;
}

/*
 * 函 数 名  : hwifi_custom_host_read_dyn_cali_nvram
 * 功能描述  : 包括读取nvram中的dpint和校准系数值
 */
int32_t hwifi_custom_host_read_dyn_cali_nvram(void)
{
    int32_t l_ret;
    uint8_t uc_idx;
    uint8_t *buffer_cust_nvram = NULL;
    int32_t *pl_params = NULL;
    uint8_t *cust_nvram_info = NULL; /* NVRAM数组 */
    oal_bool_enum_uint8 tmp_en_fact_cali_completed = OAL_FALSE;

    /* 判断定制化中是否使用nvram中的动态校准参数 */
    l_ret = hwifi_custom_check_nvram_data_flag();
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }

    buffer_cust_nvram = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (buffer_cust_nvram == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::cust_nvram_tmp mem alloc fail!");
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    pl_params = (int32_t *)os_kzalloc_gfp(DY_CALI_PARAMS_NUM * DY_CALI_PARAMS_TIMES * sizeof(int32_t));
    if (pl_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::ps_params mem alloc fail!");
        os_mem_kfree(buffer_cust_nvram);
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    cust_nvram_info = (uint8_t *)os_kzalloc_gfp(WLAN_CFG_DTS_NVRAM_END * CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (cust_nvram_info == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_custom_host_read_dyn_cali_nvram::cust_nvram_info mem alloc fail!");
        os_mem_kfree(buffer_cust_nvram);
        os_mem_kfree(pl_params);
        memset_s(g_cust_nvram_info, sizeof(g_cust_nvram_info), 0, sizeof(g_cust_nvram_info));
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    hwifi_custom_init_nvram_data(buffer_cust_nvram, pl_params, cust_nvram_info);

    /* 拟合系数 */
    for (uc_idx = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; uc_idx < WLAN_CFG_DTS_NVRAM_END; uc_idx++) {
        if (hwifi_read_conf_and_set_cust_nvram(buffer_cust_nvram, uc_idx, cust_nvram_info,
                                               pl_params, &tmp_en_fact_cali_completed) != OAL_TRUE) {
            break;
        }
    }

    g_en_fact_cali_completed = tmp_en_fact_cali_completed;

    os_mem_kfree(buffer_cust_nvram);
    os_mem_kfree(pl_params);

    /* 检查NVRAM是否修改 */
    l_ret = hwifi_custom_set_nvram_data(cust_nvram_info);
    os_mem_kfree(cust_nvram_info);
    return l_ret;
}

uint8_t *hwifi_get_nvram_param(uint32_t nvram_param_idx)
{
    return g_cust_nvram_info[nvram_param_idx];
}

/*
 * 函 数 名  : hwifi_config_sepa_coefficient_from_param
 * 功能描述  : 从字符串中分割二次系数项
 */
uint32_t hwifi_config_sepa_coefficient_from_param(uint8_t *puc_cust_param_info, int32_t *pl_coe_params,
    uint16_t *pus_param_num, uint16_t us_max_idx)
{
    int8_t *pc_token = NULL;
    int8_t *pc_ctx = NULL;
    int8_t *pc_end = ";";
    int8_t *pc_sep = ",";
    uint16_t us_param_num = 0;
    uint8_t auc_cust_param[CUS_PARAMS_LEN_MAX];

    if (memcpy_s(auc_cust_param, CUS_PARAMS_LEN_MAX, puc_cust_param_info, OAL_STRLEN(puc_cust_param_info)) != EOK) {
        return OAL_FAIL;
    }

    pc_token = oal_strtok(auc_cust_param, pc_end, &pc_ctx);
    if (pc_token == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_sepa_coefficient_from_param read get null value check!");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
    /* 获取定制化系数 */
    while (pc_token != NULL) {
        if (us_param_num == us_max_idx) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                           "hwifi_config_sepa_coefficient_from_param::nv or ini param is too many idx[%d] Max[%d]",
                           us_param_num, us_max_idx);
            return OAL_FAIL;
        }
        /* 将字符串转换成10进制数 */
        *(pl_coe_params + us_param_num) = (int32_t)simple_strtol(pc_token, NULL, 10);
        pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
        us_param_num++;
    }

    *pus_param_num = us_param_num;
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_config_get_switch_point_5g
 * 功能描述  : 根据ini文件获取5G二次曲线功率切换点
 */
OAL_STATIC void hwifi_config_get_5g_curv_switch_point(uint8_t *puc_ini_pa_params, uint32_t cfg_id)
{
    int32_t l_ini_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    uint16_t us_ini_param_num = 0;
    uint8_t uc_secon_ratio_idx = 0;
    uint8_t uc_param_idx;
    uint8_t uc_chain_idx;
    int16_t *ps_extre_point_val = NULL;

    if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1)) {
        uc_chain_idx = WLAN_RF_CHANNEL_ZERO;
    } else if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1)) {
        uc_chain_idx = WLAN_RF_CHANNEL_ONE;
    } else {
        return;
    }

    /* 获取拟合系数项 */
    if (hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num,
                                                 sizeof(l_ini_params) / sizeof(int32_t)) != OAL_SUCC ||
        (us_ini_param_num % DY_CALI_PARAMS_TIMES)) {
        oam_error_log2(0, OAM_SF_CUSTOM,
                       "hwifi_config_get_5g_curv_switch_point::ini is unsuitable,num of ini[%d] cfg_id[%d]!",
                       us_ini_param_num, cfg_id);
        return;
    }

    ps_extre_point_val = g_gs_extre_point_vals[uc_chain_idx];
    us_ini_param_num /= DY_CALI_PARAMS_TIMES;
    if (cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1) {
        if (us_ini_param_num != CUS_NUM_5G_BW) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", cfg_id, us_ini_param_num);
            return;
        }
        ps_extre_point_val++;
    } else {
        if (us_ini_param_num != 1) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_get_5g_curv_switch_point::ul_cfg_id[%d] us_ini_param_num[%d]", cfg_id, us_ini_param_num);
            return;
        }
    }

    /* 计算5g曲线switch point */
    for (uc_param_idx = 0; uc_param_idx < us_ini_param_num; uc_param_idx++) {
        *(ps_extre_point_val + uc_param_idx) = (int16_t)hwifi_dyn_cali_get_extre_point(l_ini_params +
                                               uc_secon_ratio_idx);
        oal_io_print("hwifi_config_get_5g_curv_switch_point::extre power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                     *(ps_extre_point_val + uc_param_idx), uc_param_idx, cfg_id);
        oal_io_print("hwifi_config_get_5g_curv_switch_point::param[%d %d] uc_secon_ratio_idx[%d]!\r\n",
            (l_ini_params + uc_secon_ratio_idx)[0], (l_ini_params + uc_secon_ratio_idx)[1], uc_secon_ratio_idx);
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }

    return;
}

/*
 * 函 数 名  : hwifi_config_nvram_second_coefficient_check
 * 功能描述  : 检查修正nvram中的二次系数是否合理
 */
OAL_STATIC uint32_t hwifi_config_nvram_second_coefficient_check(uint8_t *puc_g_cust_nvram_info,
                                                                uint8_t *puc_ini_pa_params,
                                                                uint32_t cfg_id,
                                                                int16_t *ps_5g_delt_power)
{
    int32_t l_ini_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    int32_t l_nv_params[CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES] = {0};
    uint16_t us_ini_param_num = 0;
    uint16_t us_nv_param_num = 0;
    uint8_t uc_secon_ratio_idx = 0;
    uint8_t uc_param_idx;

    /* 获取拟合系数项 */
    if (hwifi_config_sepa_coefficient_from_param(puc_g_cust_nvram_info, l_nv_params, &us_nv_param_num,
                                                 sizeof(l_nv_params) / (sizeof(int16_t))) != OAL_SUCC ||
        (us_nv_param_num % DY_CALI_PARAMS_TIMES) ||
        hwifi_config_sepa_coefficient_from_param(puc_ini_pa_params, l_ini_params, &us_ini_param_num,
                                                 sizeof(l_ini_params) / (sizeof(int16_t))) != OAL_SUCC ||
        (us_ini_param_num % DY_CALI_PARAMS_TIMES) || (us_nv_param_num != us_ini_param_num)) {
        oam_error_log2(0, OAM_SF_CUSTOM,
            "hwifi_config_nvram_second_coefficient_check::nvram or ini is unsuitable,num of nv and ini[%d %d]!",
            us_nv_param_num, us_ini_param_num);
        return OAL_FAIL;
    }

    us_nv_param_num /= DY_CALI_PARAMS_TIMES;
    /* 检查nv和ini中二次系数是否匹配 */
    for (uc_param_idx = 0; uc_param_idx < us_nv_param_num; uc_param_idx++) {
        if (uc_secon_ratio_idx >= (CUS_NUM_5G_BW * DY_CALI_PARAMS_TIMES)) {
            continue;
        }
        if (l_ini_params[uc_secon_ratio_idx] != l_nv_params[uc_secon_ratio_idx]) {
            oam_warning_log4(0, OAM_SF_CUSTOM, "hwifi_config_nvram_second_coefficient_check::nvram get mismatch value \
                idx[%d %d] val are [%d] and [%d]!", uc_param_idx, uc_secon_ratio_idx, l_ini_params[uc_secon_ratio_idx],
                l_nv_params[uc_secon_ratio_idx]);

            /* 量产后二次系数以nvram中为准，刷新NV中的二次拟合曲线切换点 */
            hwifi_config_get_5g_curv_switch_point(puc_g_cust_nvram_info, cfg_id);
            uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
            continue;
        }

        if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0) || (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1)) {
            /* 计算产线上的delt power */
            *(ps_5g_delt_power + uc_param_idx) = hwifi_get_5g_pro_line_delt_pow_per_band(
                l_nv_params + (int32_t)uc_secon_ratio_idx, l_ini_params + (int32_t)uc_secon_ratio_idx);
            oal_io_print("hwifi_config_nvram_second_coefficient_check::delt power[%d] param_idx[%d] cfg_id[%d]!\r\n",
                         *(ps_5g_delt_power + uc_param_idx), uc_param_idx, cfg_id);
        }
        uc_secon_ratio_idx += DY_CALI_PARAMS_TIMES;
    }

    return OAL_SUCC;
}

static int32_t hwifi_config_get_cust_string(uint32_t cfg_id, uint8_t *nv_pa_params)
{
    uint32_t cfg_id_tmp;
    if (get_cust_conf_string(INI_MODU_WIFI, g_nvram_pro_line_config_ini[cfg_id].name,
                             nv_pa_params, CUS_PARAMS_LEN_MAX - 1) == INI_FAILED) {
        if (oal_value_eq_any4(cfg_id, WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW,
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW)) {
            cfg_id_tmp = ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_BAND1 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_BAND1 :
                              (cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) ?
                              WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1 : cfg_id);
            get_cust_conf_string(INI_MODU_WIFI, g_nvram_pro_line_config_ini[cfg_id_tmp].name,
                                 nv_pa_params, CUS_PARAMS_LEN_MAX - 1);
        } else {
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_get_cust_string read, check id[%d] exists!", cfg_id);
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static uint8_t hwifi_config_set_cali_param_2g_and_5g(int32_t *pl_params, int16_t *s_5g_delt_power)
{
    uint8_t rf_idx;
    uint8_t cali_param_idx;
    uint8_t delt_pwr_idx = 0;
    uint8_t idx = 0;
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        for (cali_param_idx = 0; cali_param_idx < DY_CALI_PARAMS_BASE_NUM; cali_param_idx++) {
            if (cali_param_idx == (DY_2G_CALI_PARAMS_NUM - 1)) {
                /* band1 & CW */
                cali_param_idx += PRO_LINE_2G_TO_5G_OFFSET;
            }
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0 = pl_params[idx++];
        }
    }

    /* 5g band2&3 4&5 6 7 low power */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        delt_pwr_idx = 0;
        for (cali_param_idx = DY_CALI_PARAMS_BASE_NUM + 1;
             cali_param_idx < DY_CALI_PARAMS_NUM - 1; cali_param_idx++) {
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1 = pl_params[idx++];
            g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0 = pl_params[idx++];

            cus_flush_nv_ratio_by_delt_pow(&g_pro_line_params[rf_idx][cali_param_idx].l_pow_par0,
                                           &g_pro_line_params[rf_idx][cali_param_idx].l_pow_par1,
                                           g_pro_line_params[rf_idx][cali_param_idx].l_pow_par2,
                                           *(s_5g_delt_power + rf_idx * CUS_NUM_5G_BW + delt_pwr_idx));
            delt_pwr_idx++;
        }
    }
    return idx;
}

static void hwifi_config_set_cali_param_left_num(int32_t *pl_params, uint8_t idx)
{
    uint8_t rf_idx;
    /* band1 & CW */
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM].l_pow_par0 = pl_params[idx++];
    }
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_2G_CALI_PARAMS_NUM - 1].l_pow_par0 = pl_params[idx++];
    }
    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        /* 5g band1 low power */
        /* band1产线不校准 */
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_BASE_NUM].l_pow_par0 = pl_params[idx++];
    }

    for (rf_idx = 0; rf_idx < WLAN_RF_CHANNEL_NUMS; rf_idx++) {
        /* 2g cw ppa */
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par2 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par1 = pl_params[idx++];
        g_pro_line_params[rf_idx][DY_CALI_PARAMS_NUM - 1].l_pow_par0 = pl_params[idx++];
    }
}

static uint32_t hwifi_nvram_param_check_second_coefficient(uint8_t *nv_pa_params, uint8_t *cust_nvram_info,
                                                           int16_t *delt_power_5g, uint32_t cfg_id)
{
    /* 先取nv中的参数值,为空则从ini文件中读取 */
    if (OAL_STRLEN(cust_nvram_info)) {
        /* NVRAM二次系数异常保护 */
        if (hwifi_config_nvram_second_coefficient_check(cust_nvram_info, nv_pa_params, cfg_id, delt_power_5g +
            (cfg_id < WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA1 ? WLAN_RF_CHANNEL_ZERO : WLAN_RF_CHANNEL_ONE)) == OAL_SUCC) {
            /* 手机如果low part为空,则取ini中的系数,并根据产测结果修正;否则直接从nvram中取得 */
            if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA0_LOW) &&
                (oal_memcmp(cust_nvram_info, nv_pa_params, OAL_STRLEN(cust_nvram_info)))) {
                memset_s(delt_power_5g + WLAN_RF_CHANNEL_ZERO, CUS_NUM_5G_BW * sizeof(int16_t),
                         0, CUS_NUM_5G_BW * sizeof(int16_t));
            }
            if ((cfg_id == WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) &&
                (oal_memcmp(cust_nvram_info, nv_pa_params, OAL_STRLEN(cust_nvram_info)))) {
                memset_s(delt_power_5g + WLAN_RF_CHANNEL_ONE, CUS_NUM_5G_BW * sizeof(int16_t),
                         0, CUS_NUM_5G_BW * sizeof(int16_t));
            }

            if (memcpy_s(nv_pa_params, CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
                         cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
                return OAL_FAIL;
            }
        } else {
            return OAL_FAIL;
        }
    } else {
        /* 提供产线第一次上电校准初始值 */
        if (memcpy_s(cust_nvram_info, CUS_PARAMS_LEN_MAX,
                     nv_pa_params, OAL_STRLEN(nv_pa_params)) != EOK) {
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

static uint32_t hwifi_coefficient_check_and_set(uint8_t **cust_nvram_info, uint8_t *nv_pa_params,
                                                int32_t *params, int16_t *delt_power_5g,
                                                uint16_t *param_num)
{
    uint32_t cfg_id;
    uint16_t per_param_num = 0;
    uint16_t param_len = WLAN_RF_CHANNEL_NUMS * DY_CALI_PARAMS_TIMES * DY_CALI_PARAMS_NUM * sizeof(int32_t);
    for (cfg_id = WLAN_CFG_DTS_NVRAM_RATIO_PA2GCCKA0; cfg_id < WLAN_CFG_DTS_NVRAM_PARAMS_BUTT; cfg_id++) {
        /* 二次拟合系数 */
        if ((cfg_id >= WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0) && (cfg_id < WLAN_CFG_DTS_NVRAM_END)) {
            /* DPN */
            continue;
        }

        if (hwifi_config_get_cust_string(cfg_id, nv_pa_params) != OAL_SUCC) {
            return OAL_FAIL;
        }

        /* 获取ini中的二次拟合曲线切换点 */
        hwifi_config_get_5g_curv_switch_point(nv_pa_params, cfg_id);

        if (cfg_id <= WLAN_CFG_DTS_NVRAM_RATIO_PA5GA1_LOW) {
            *cust_nvram_info = hwifi_get_nvram_param(cfg_id);
            if (hwifi_nvram_param_check_second_coefficient(nv_pa_params, *cust_nvram_info,
                                                           delt_power_5g, cfg_id) != OAL_SUCC) {
                return OAL_FAIL;
            }
        }

        if (hwifi_config_sepa_coefficient_from_param(nv_pa_params, params + *param_num,
                                                     &per_param_num, param_len - *param_num) != OAL_SUCC ||
                                                     (per_param_num % DY_CALI_PARAMS_TIMES)) {
            oam_error_log3(0, OAM_SF_CUSTOM,
                "hwifi_coefficient_check_and_set read get wrong value,len[%d] check id[%d] exists per_param_num[%d]!",
                OAL_STRLEN(*cust_nvram_info), cfg_id, per_param_num);
            return OAL_FAIL;
        }
        *param_num += per_param_num;
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : hwifi_config_init_dy_cali_custom
 * 功能描述  : 获取定制化文件和二次产测系数
 */
uint32_t hwifi_config_init_dy_cali_custom(void)
{
    uint32_t ret;
    uint8_t uc_idx = 0;
    uint16_t us_param_num = 0;
    int16_t s_5g_delt_power[WLAN_RF_CHANNEL_NUMS][CUS_NUM_5G_BW] = {{0}};
    uint8_t *puc_g_cust_nvram_info = NULL;
    uint8_t *puc_nv_pa_params = NULL;
    int32_t *pl_params = NULL;
    uint16_t us_param_len = WLAN_RF_CHANNEL_NUMS * DY_CALI_PARAMS_TIMES *
                              DY_CALI_PARAMS_NUM * sizeof(int32_t);

    puc_nv_pa_params = (uint8_t *)os_kzalloc_gfp(CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    if (puc_nv_pa_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom::puc_nv_pa_params mem alloc fail!");
        return OAL_FAIL;
    }

    pl_params = (int32_t *)os_kzalloc_gfp(us_param_len);
    if (pl_params == NULL) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_config_init_dy_cali_custom::ps_params mem alloc fail!");
        os_mem_kfree(puc_nv_pa_params);
        return OAL_FAIL;
    }

    memset_s(puc_nv_pa_params,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t),
             0,
             CUS_PARAMS_LEN_MAX * sizeof(uint8_t));
    memset_s(pl_params, us_param_len, 0, us_param_len);

    ret = hwifi_coefficient_check_and_set(&puc_g_cust_nvram_info, puc_nv_pa_params, pl_params,
                                          (int16_t *)s_5g_delt_power, &us_param_num);

    os_mem_kfree(puc_nv_pa_params);

    if (ret == OAL_FAIL) {
        /* 置零防止下发到device */
        memset_s(g_pro_line_params, sizeof(g_pro_line_params), 0, sizeof(g_pro_line_params));
    } else {
        if (us_param_num != us_param_len / sizeof(int32_t)) {
            oam_error_log1(0, OAM_SF_CUSTOM,
                           "hwifi_config_init_dy_cali_custom read get wrong ini value num[%d]!", us_param_num);
            memset_s(g_pro_line_params, sizeof(g_pro_line_params), 0, sizeof(g_pro_line_params));
            os_mem_kfree(pl_params);
            return OAL_FAIL;
        }

        uc_idx = hwifi_config_set_cali_param_2g_and_5g(pl_params, (int16_t *)s_5g_delt_power);

        hwifi_config_set_cali_param_left_num(pl_params, uc_idx);
    }

    os_mem_kfree(pl_params);
    return ret;
}
#if (defined(HISI_CONN_NVE_SUPPORT) && (!defined(_PRE_CONFIG_READ_DYNCALI_E2PROM))) || \
    (defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && defined(_PRE_CONFIG_READ_E2PROM_MAC))
/*
 * 函 数 名  : char2byte
 * 功能描述  : 统计值，判断有无读取到mac地址
 */
OAL_STATIC uint32_t char2byte(const char *strori, char *outbuf)
{
    uint32_t i = 0;
    uint8_t temp = 0;
    uint32_t sum = 0;
    uint8_t *ptr_out = (uint8_t *)outbuf;
    const int l_loop_times = 12; /* 单字节遍历是不是正确的mac地址:xx:xx:xx:xx:xx:xx */

    for (i = 0; i < l_loop_times; i++) {
        if (isdigit(strori[i])) {
            temp = strori[i] - '0';
        } else if (islower(strori[i])) {
            temp = (strori[i] - 'a') + 10; /* 加10为了保证'a'~'f'分别对应10~15 */
        } else if (isupper(strori[i])) {
            temp = (strori[i] - 'A') + 10; /* 加10为了保证'A'~'F'分别对应10~15 */
        }
        sum += temp;
        /* 为了组成正确的mac地址:xx:xx:xx:xx:xx:xx */
        if (i % 2 == 0) { /* 对2取余 */
            ptr_out[i / 2] |= (temp << BIT_OFFSET_4); /* 除以2 */
        } else {
            ptr_out[i / 2] |= temp; /* 除以2 */
        }
    }

    return sum;
}
#endif
#if (defined(HISI_CONN_NVE_SUPPORT) && (!defined(_PRE_CONFIG_READ_DYNCALI_E2PROM))) || \
    defined(_PRE_CONFIG_READ_DYNCALI_E2PROM)
OAL_STATIC uint8_t g_wifi_mac[WLAN_MAC_ADDR_LEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif

#if defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && defined(_PRE_CONFIG_READ_E2PROM_MAC)
/*
 * 函 数 名  : hwifi_get_mac_addr_drveprom
 * 功能描述  : 从e2prom中获取mac地址，用宏_PRE_CONFIG_READ_E2PROM_MACADDR区分PC和大屏
 */
int32_t hwifi_get_mac_addr_drveprom(uint8_t *puc_buf)
{
#define READ_CHECK_MAX_CNT    20
#define READ_CHECK_WAIT_TIME  50

    char original_mac_addr[NUM_12_BYTES] = {0};
    int32_t ret = INI_FAILED;
    int32_t i;
    unsigned int offset = 0;
    unsigned int bit_len = 12;
    uint32_t sum = 0;

    if (g_wifi_mac[MAC_ADDR_0] != 0 || g_wifi_mac[MAC_ADDR_1]  != 0 || g_wifi_mac[MAC_ADDR_2]  != 0 ||
        g_wifi_mac[MAC_ADDR_3] != 0 || g_wifi_mac[MAC_ADDR_4] != 0 || g_wifi_mac[MAC_ADDR_5] != 0) {
        if (memcpy_s(puc_buf, WLAN_MAC_ADDR_LEN, g_wifi_mac, WLAN_MAC_ADDR_LEN) != EOK) {
            return INI_FAILED;
        }

        ini_warning("hwifi_get_mac_addr_drveprom get MAC from g_wifi_mac SUCC\n");
        return INI_SUCC;
    }

    for (i = 0; i < READ_CHECK_MAX_CNT; i++) {
        if (ini_eeprom_read("MACWLAN", offset, original_mac_addr, bit_len) == INI_SUCC) {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM SUCC\n");
            ret = INI_SUCC;
            break;
        }

        msleep(READ_CHECK_WAIT_TIME);
    }

    if (ret == INI_SUCC) {
        oal_io_print("hwifi_get_mac_addr_drveprom ini_eeprom_read return success\n");
        sum = char2byte(original_mac_addr, (char *)puc_buf);
        if (sum != 0) {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM: mac=" MACFMT "\n", ini_mac2str(puc_buf));
            if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
                ini_warning("hwifi_get_mac_addr_drveprom memcpy_s g_wifi_mac fail\n");
            }
        } else {
            ini_warning("hwifi_get_mac_addr_drveprom get MAC from EEPROM is not char,use random mac\n");
            random_ether_addr(puc_buf);
            puc_buf[1] = 0x11;
            puc_buf[BYTE_OFFSET_2] = 0x03;
        }
    } else {
        oal_io_print("hwifi_get_mac_addr ini_eeprom_read return fail,use random mac\n");
        chr_exception_report(CHR_READ_EEPROM_ERROR_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_EEPROM_READ_INIT);
        random_ether_addr(puc_buf);
        puc_buf[1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x03;
    }

    return ret;
}
#endif

/*
 * 函 数 名  : hwifi_get_mac_addr
 * 功能描述  : 从e2prom中获取mac地址，用宏_PRE_CONFIG_READ_E2PROM_MACADDR区分PC和大屏
 */
#if defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && (!defined(_PRE_CONFIG_READ_E2PROM_MAC))
#define WIFI_2G_MAC_TYPE    (1)
#define WIFI_5G_MAC_TYPE    (2)
#define WIFI_P2P_MAC_TYPE   (3)
#define BT_MAC_TYPE         (4)
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
    if (puc_buf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::buf is NULL!");
        return INI_FAILED;
    }

    if (get_board_mac(puc_buf, WIFI_2G_MAC_TYPE) == 0) {
        ini_warning("hwifi_get_mac_addr get MAC from NV: mac="MACFMT"\n", ini_mac2str(puc_buf));
        if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::memcpy_s mac to g_wifi_mac failed");
        }
    } else {
        random_ether_addr(puc_buf);
        puc_buf[1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x02;
        ini_warning("hwifi_get_mac_addr get random mac: mac="MACFMT"\n", ini_mac2str(puc_buf));
    }
    return INI_SUCC;
}

#elif defined(_PRE_CONFIG_READ_DYNCALI_E2PROM) && defined(_PRE_CONFIG_READ_E2PROM_MAC)
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
    return hwifi_get_mac_addr_drveprom(puc_buf);
}

#else
int32_t hwifi_get_mac_addr(uint8_t *puc_buf)
{
#ifdef HISI_CONN_NVE_SUPPORT
    struct external_nve_info_user st_info;
    int32_t l_ret;
    uint32_t sum = 0;

    if (puc_buf == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr::buf is NULL!");
        return INI_FAILED;
    }

    memset_s(puc_buf, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);

    memset_s(&st_info, sizeof(st_info), 0, sizeof(st_info));

    st_info.nv_number = NV_WLAN_NUM;  // nve item

    if (strcpy_s(st_info.nv_name, sizeof(st_info.nv_name), "MACWLAN") != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "hwifi_get_mac_addr:: strcpy_s failed.");
        return INI_FAILED;
    }

    st_info.valid_size = NV_WLAN_VALID_SIZE;
    st_info.nv_operation = NV_READ;

    if (g_wifi_mac[0] != 0 || g_wifi_mac[1]  != 0 || g_wifi_mac[BYTE_OFFSET_2]  != 0 || g_wifi_mac[BYTE_OFFSET_3] != 0
        || g_wifi_mac[BYTE_OFFSET_4] != 0 || g_wifi_mac[BYTE_OFFSET_5] != 0) {
        if (memcpy_s(puc_buf, WLAN_MAC_ADDR_LEN, g_wifi_mac, WLAN_MAC_ADDR_LEN) != EOK) {
            return INI_FAILED;
        }

        return INI_SUCC;
    }

    l_ret = external_nve_direct_access_interface(&st_info);
    if (!l_ret) {
        sum = char2byte(st_info.nv_data, (char *)puc_buf);
        if (sum != 0) {
            ini_warning("hwifi_get_mac_addr get MAC from NV: mac=" MACFMT "\n", ini_mac2str(puc_buf));
            if (memcpy_s(g_wifi_mac, WLAN_MAC_ADDR_LEN, puc_buf, WLAN_MAC_ADDR_LEN) != EOK) {
                return INI_FAILED;
            }
        } else {
            random_ether_addr(puc_buf);
            puc_buf[BYTE_OFFSET_1] = 0x11;
            puc_buf[BYTE_OFFSET_2] = 0x02;
        }
    } else {
        random_ether_addr(puc_buf);
        puc_buf[BYTE_OFFSET_1] = 0x11;
        puc_buf[BYTE_OFFSET_2] = 0x02;
    }
#endif
    return INI_SUCC;
}
#endif
void hwifi_get_cfg_pro_line_params(void)
{
    int32_t cfg_idx_one;
    int32_t cfg_idx_two;

    for (cfg_idx_one = 0; cfg_idx_one < WLAN_RF_CHANNEL_NUMS; cfg_idx_one++) {
        for (cfg_idx_two = 0; cfg_idx_two < DY_CALI_PARAMS_NUM; cfg_idx_two++) {
            oal_io_print("%s CORE[%d]para_idx[%d]::{%d, %d, %d}\n", "g_pro_line_params: ",
                cfg_idx_one, cfg_idx_two, g_pro_line_params[cfg_idx_one][cfg_idx_two].l_pow_par2,
                g_pro_line_params[cfg_idx_one][cfg_idx_two].l_pow_par1,
                g_pro_line_params[cfg_idx_one][cfg_idx_two].l_pow_par0);
        }
    }

    /* NVRAM */
    oal_io_print("%s : { %d }\n", "en_nv_dp_init_is_null: ", g_en_nv_dp_init_is_null);
    for (cfg_idx_one = 0; cfg_idx_one < WLAN_CFG_DTS_NVRAM_END; cfg_idx_one++) {
        oal_io_print("%s para_idx[%d] name[%s]::DATA{%s}\n", "dp init & ratios nvram_param: ", cfg_idx_one,
            g_wifi_nvram_cfg_handler[cfg_idx_one].puc_param_name,
            hwifi_get_nvram_param(cfg_idx_one));
    }
}

OAL_STATIC uint32_t hwifi_cfg_init_cus_5g_160m_dpn_cali(mac_cus_dy_cali_param_stru *dyn_cali_param,
    int8_t *pc_ctx, uint8_t rf_idx)
{
    uint8_t  uc_dpn_5g_nv_id = WLAN_CFG_DTS_NVRAM_MUFREQ_5G160_C0;
    uint8_t  nv_pa_params[CUS_PARAMS_LEN_MAX] = { 0 };
    int8_t   dpn_5g_nv[OAL_5G_160M_CHANNEL_NUM];
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    int8_t  *pc_token = NULL;
    uint8_t *cust_nvram_info = NULL;
    uint8_t  num_idx;
    int32_t  val;

    cust_nvram_info = hwifi_get_nvram_param(uc_dpn_5g_nv_id);
    uc_dpn_5g_nv_id++;
    if (OAL_STRLEN(cust_nvram_info)) {
        memset_s(nv_pa_params, sizeof(nv_pa_params), 0, sizeof(nv_pa_params));
        if (memcpy_s(nv_pa_params, sizeof(nv_pa_params),
                     cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
            oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_5g_160m_dpn_cali::memcpy fail!");
        }
        pc_token = oal_strtok(nv_pa_params, pc_end, &pc_ctx);
        pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
        num_idx = 0;
        while ((pc_token != NULL)) {
            if (num_idx >= OAL_5G_160M_CHANNEL_NUM) {
                num_idx++;
                break;
            }
            val = simple_strtol(pc_token, NULL, 10) / 10; /* 10表示十进制 */
            pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            if (oal_value_not_in_valid_range(val, CUS_DY_CALI_5G_VAL_DPN_MIN, CUS_DY_CALI_5G_VAL_DPN_MAX)) {
                oam_error_log3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d]\
                    unexpect:idx[%d] num_idx[%d}}", val, MAC_NUM_5G_BAND, num_idx);
                val = 0;
            }
            dpn_5g_nv[num_idx] = (int8_t)val;
            num_idx++;
        }

        if (num_idx != OAL_5G_160M_CHANNEL_NUM) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num unexpected id[%d] rf[%d}}", MAC_NUM_5G_BAND, rf_idx);
            return OAL_FAIL;
        }
        /* 5250  5570 */
        for (num_idx = 0; num_idx < OAL_5G_160M_CHANNEL_NUM; num_idx++) {
            dyn_cali_param->ac_dy_cali_5g_dpn_params[num_idx + 1][WLAN_BW_CAP_160M] += dpn_5g_nv[num_idx];
        }
    }
    return OAL_SUCC;
}

OAL_STATIC void hwifi_cfg_init_cus_dpn_cali(mac_cus_dy_cali_param_stru *dyn_cali_param, int8_t *pc_ctx, uint8_t rf_idx)
{
    uint8_t  idx, num_idx;
    int32_t  val;
    uint8_t *cust_nvram_info = NULL;
    uint8_t  dpn_2g_nv_id = WLAN_CFG_DTS_NVRAM_MUFREQ_2GCCK_C0;
    uint8_t  nv_pa_params[CUS_PARAMS_LEN_MAX] = { 0 };
    int8_t   ac_dpn_nv[HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_CW][MAC_2G_CHANNEL_NUM];
    uint8_t *pc_end = ";";
    uint8_t *pc_sep = ",";
    int8_t  *pc_token = NULL;

    for (idx = HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_11B;
        idx <= HWIFI_CFG_DYN_PWR_CALI_2G_SNGL_MODE_OFDM40; idx++) {
        /* 获取产线计算DPN值修正 */
        cust_nvram_info = hwifi_get_nvram_param(dpn_2g_nv_id);
        dpn_2g_nv_id++;

        if (0 == OAL_STRLEN(cust_nvram_info)) {
            continue;
        }

        memset_s(nv_pa_params, sizeof(nv_pa_params), 0, sizeof(nv_pa_params));
        if (memcpy_s(nv_pa_params, sizeof(nv_pa_params), cust_nvram_info, OAL_STRLEN(cust_nvram_info)) != EOK) {
            oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_dpn_cali::memcpy fail!");
        }
        pc_token = oal_strtok(nv_pa_params, pc_end, &pc_ctx);
        pc_token = oal_strtok(pc_token, pc_sep, &pc_ctx);
        num_idx = 0;
        while ((pc_token != NULL)) {
            if (num_idx >= MAC_2G_CHANNEL_NUM) {
                num_idx++;
                break;
            }
            val = simple_strtol(pc_token, NULL, 10) / 10; /* 10表示十进制 */
            pc_token = oal_strtok(NULL, pc_sep, &pc_ctx);
            if (oal_value_not_in_valid_range(val, CUS_DY_CALI_2G_VAL_DPN_MIN, CUS_DY_CALI_2G_VAL_DPN_MAX)) {
                oam_error_log3(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn val[%d]\
                    unexpected idx[%d] num_idx[%d}!}", val, idx, num_idx);
                val = 0;
            }
            ac_dpn_nv[idx][num_idx] = (int8_t)val;
            num_idx++;
        }

        if (num_idx != MAC_2G_CHANNEL_NUM) {
            oam_error_log2(0, OAM_SF_CUSTOM,
                "{hwifi_cfg_init_cus_dyn_cali::nvram 2g dpn num is unexpect uc_id[%d] rf[%d}}", idx, rf_idx);
            continue;
        }

        for (num_idx = 0; num_idx < MAC_2G_CHANNEL_NUM; num_idx++) {
            dyn_cali_param->ac_dy_cali_2g_dpn_params[num_idx][idx] += ac_dpn_nv[idx][num_idx];
        }
    }
}

/*
 * 函 数 名  : hwifi_cfg_init_cus_dyn_cali
 * 功能描述  : 修改动态校准参数
 * 1.日    期  : 2017年12月21日
 *   修改内容  : 新生成函数
 */
OAL_STATIC uint32_t hwifi_cfg_init_cus_dyn_cali(mac_cus_dy_cali_param_stru *puc_dyn_cali_param, int num)
{
    int32_t  val, ret;
    uint8_t  uc_idx = 0;
    uint8_t  uc_rf_idx, uc_dy_cal_param_idx;
    uint8_t  uc_cfg_id = WLAN_CFG_DTS_2G_CORE0_DPN_CH1;
    int8_t  *pc_ctx = NULL;

    for (uc_rf_idx = 0; uc_rf_idx < num; uc_rf_idx++) {
        puc_dyn_cali_param->uc_rf_id = uc_rf_idx;

        /* 动态校准二次项系数入参检查 */
        for (uc_dy_cal_param_idx = 0; uc_dy_cal_param_idx < DY_CALI_PARAMS_NUM; uc_dy_cal_param_idx++) {
            if (!g_pro_line_params[uc_rf_idx][uc_dy_cal_param_idx].l_pow_par2) {
                oam_error_log1(0, OAM_SF_CUSTOM, "{hwifi_cfg_init_cus_dyn_cali::unexpect val[%d] s_pow_par2[0]!}",
                    uc_dy_cal_param_idx);
                return OAL_FAIL;
            }
        }
        ret = memcpy_s(puc_dyn_cali_param->al_dy_cali_base_ratio_params,
                       sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_params),
                       g_pro_line_params[uc_rf_idx], sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_params));

        ret += memcpy_s(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params,
                        sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params),
                        &g_pro_line_params[uc_rf_idx][CUS_DY_CALI_PARAMS_NUM],
                        sizeof(puc_dyn_cali_param->al_dy_cali_base_ratio_ppa_params));

        ret += memcpy_s(puc_dyn_cali_param->as_extre_point_val, sizeof(puc_dyn_cali_param->as_extre_point_val),
                        g_gs_extre_point_vals[uc_rf_idx], sizeof(puc_dyn_cali_param->as_extre_point_val));

        /* DPN */
        for (uc_idx = 0; uc_idx < MAC_2G_CHANNEL_NUM; uc_idx++) {
            val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            ret += memcpy_s(puc_dyn_cali_param->ac_dy_cali_2g_dpn_params[uc_idx],
                CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t), &val, CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t));
        }
        uc_cfg_id += MAC_2G_CHANNEL_NUM;
        hwifi_cfg_init_cus_dpn_cali(puc_dyn_cali_param, pc_ctx, uc_rf_idx);

        for (uc_idx = 0; uc_idx < MAC_NUM_5G_BAND; uc_idx++) {
            val = hwifi_get_init_value(CUS_TAG_DTS, uc_cfg_id + uc_idx);
            ret += memcpy_s(puc_dyn_cali_param->ac_dy_cali_5g_dpn_params[uc_idx],
                CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t), &val, CUS_DY_CALI_DPN_PARAMS_NUM * sizeof(int8_t));
        }
        uc_cfg_id += MAC_NUM_5G_BAND;

        /* 5G 160M DPN */
        if (hwifi_cfg_init_cus_5g_160m_dpn_cali(puc_dyn_cali_param, pc_ctx, uc_rf_idx) != OAL_SUCC) {
            continue;
        }
        puc_dyn_cali_param++;
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_CUSTOM, "hwifi_cfg_init_cus_dyn_cali::memcpy fail!");
        return OAL_FAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t hwifi_config_parameter_init(mac_cus_dy_cali_param_stru *dy_cus_cali)
{
    uint32_t ret;
    /* 配置动态校准参数TXPWR_PA_DC_REF */
    memset_s(dy_cus_cali, sizeof(mac_cus_dy_cali_param_stru) * WLAN_RF_CHANNEL_NUMS,
             0, sizeof(mac_cus_dy_cali_param_stru) * WLAN_RF_CHANNEL_NUMS);
    ret = hwifi_cfg_init_cus_dyn_cali(dy_cus_cali, WLAN_RF_CHANNEL_NUMS);
    return ret;
}

/*
 * 函 数 名  : hwifi_config_init_cus_dyn_cali
 * 功能描述  : 定制化参数动态校准
 * 1.日    期  : 2015年12月21日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
void hwifi_config_init_cus_dyn_cali(oal_net_device_stru *cfg_net_dev)
{
    wal_msg_write_stru write_msg;
    uint32_t offset = 0;
    mac_cus_dy_cali_param_stru *dy_cus_cali = NULL;
    uint8_t rf_id;
    wal_msg_stru *rsp_msg = NULL;
    wal_msg_write_rsp_stru *write_rsp_msg = NULL;
    int32_t ret;

    if (oal_warn_on(cfg_net_dev == NULL)) {
        return;
    }

    dy_cus_cali = (mac_cus_dy_cali_param_stru *)
            oal_memalloc(sizeof(mac_cus_dy_cali_param_stru) * WLAN_RF_CHANNEL_NUMS);
    if (dy_cus_cali == NULL) {
        return;
    }

    if (oal_unlikely(hwifi_config_parameter_init(dy_cus_cali) != OAL_SUCC)) {
        oal_free(dy_cus_cali);
        return;
    }

    for (rf_id = 0; rf_id < WLAN_RF_CHANNEL_NUMS; rf_id++) {
        rsp_msg = NULL;

        /* 如果所有参数都在有效范围内，则下发配置值 */
        ret = memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
            (int8_t *) &dy_cus_cali[rf_id], sizeof(mac_cus_dy_cali_param_stru));
        if (ret != EOK) {
            oam_error_log0(0, OAM_SF_CFG, "hwifi_config_init_cus_dyn_cali::memcpy fail!");
            break;
        }

        offset = sizeof(mac_cus_dy_cali_param_stru);
        wal_write_msg_hdr_init(&write_msg, WLAN_CFGID_SET_CUS_DYN_CALI_PARAM, offset);
        ret = wal_send_cfg_event(cfg_net_dev, WAL_MSG_TYPE_WRITE,
            WAL_MSG_WRITE_MSG_HDR_LENGTH + offset, (uint8_t *)&write_msg, OAL_TRUE, &rsp_msg);
        if (oal_unlikely(ret != OAL_SUCC)) {
            break;
        }

        if (rsp_msg != NULL) {
            write_rsp_msg = (wal_msg_write_rsp_stru *)(rsp_msg->auc_msg_data);
            if (write_rsp_msg->err_code != OAL_SUCC) {
                oam_error_log2(0, OAM_SF_SCAN, "{wal_check_and_release_msg_resp::detect err code:[%u],wid:[%u]}",
                               write_rsp_msg->err_code, write_rsp_msg->en_wid);
                oal_free(rsp_msg);
                break;
            }

            oal_free(rsp_msg);
        }
    }
    oal_free(dy_cus_cali);
    return;
}

