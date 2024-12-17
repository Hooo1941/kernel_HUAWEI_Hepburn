/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : gf16产测pow\rssi\dpd校准NV接口实现
 * 作者       : wifi
 * 创建日期   : 2020年12月8日
 */

#include "hisi_conn_nve_interface_gf61.h"
#include "hisi_conn_nve_interface.h"
#include "oal_util.h"
#include "securec.h"
#ifdef HISI_CONN_NVE_SUPPORT
#include "external/nve_info_interface.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_CONN_NVE_INTERFACE_GF61_C


uint16_t g_xo_pwr_diff[3] = {0x4040, 0, 0}; // 1位寄存器值，2位crc校验值;

#ifdef HISI_CONN_NVE_SUPPORT
nv_pow_stru_gf61 g_pow_cail;

typedef struct {
    int32_t nv_num;
    char *nv_name;
} gf61_nv_name_map_struct;

gf61_nv_name_map_struct g_st_nv_name_map_gf61[GF61_TOTAL_ALLOC_NV_NUM] = {
    /* nv_number               nv_name */
    { GF61_2G_5G_OFDM_C0_NUM,  GF61_NV_WIC0_2G5G_OFDM_NAME },
    { GF61_UNUSED_BAK,         GF61_NV_UNUSED_NAME },
    { GF61_CKK_CO_C1_NUM,      GF61_NV_WIC0C1_CCK_NAME },
    { GF61_2G_5G_OFDM_C1_NUM,  GF61_NV_WIC1_2G5G_OFDM_NAME },
    { GF61_5G_LOW_C0_NUM,      GF61_NV_WIC0_5G_LOW_NAME },
    { GF61_5G_LOW_C1_NUM,      GF61_NV_WIC1_5G_LOW_NAME },
    { GF61_2G40_OFDM_C0_NUM,   GF61_NV_WIC0_2G40M_NAME },
    { GF61_5G160_C1_NUM,       GF61_NV_WIC1_5G160M_NAME },
    { GF61_MUFREQ_2G20_C0_NUM, GF61_NV_WIC0_2GMUOFDM_NAME },
    { GF61_MUFREQ_2G20_C1_NUM, GF61_NV_WIC1_2GMUOFDM_NAME },
    { GF61_MUFREQ_2G40_C0_NUM, GF61_NV_WIC0_2GMU40M_NAME },
    { GF61_MUFREQ_2G40_C1_NUM, GF61_NV_WIC1_2GMU40M_NAME },
    { GF61_MUFREQ_CCK_C0_NUM,  GF61_NV_WIC0_MUCCK_NAME },
    { GF61_MUFREQ_CCK_C1_NUM,  GF61_NV_WIC1_MUCCK_NAME },
};

static void conn_nve_copy_pow_data_from_user_2g_gf61(const nv_pow_user_info_stru *info)
{
    switch (info->band_type) {
        case POW_CAL_BAND_2G_11B:
            g_pow_cail.delta_gain_2g_11b[info->stream][info->band_idx].delta_gain[info->gain_lvl] = info->buf;
            break;
        case POW_CAL_BAND_2G_OFDM_20:
            g_pow_cail.delta_gain_2g_ofdm_20[info->stream][info->band_idx].delta_gain[info->gain_lvl] = \
                info->buf;
            break;
        case POW_CAL_BAND_2G_OFDM_40:
            g_pow_cail.delta_gain_2g_ofdm_40[info->stream][info->band_idx].delta_gain[info->gain_lvl] = \
                info->buf;
            break;
        default:
            break;
    }
}

static void conn_nve_copy_pow_data_from_user_5g_gf61(const nv_pow_user_info_stru *info)
{
    switch (info->band_type) {
        case POW_CAL_BAND_5G_20:
            g_pow_cail.delta_gain_5g_20[info->stream][info->band_idx].delta_gain[info->gain_lvl] = info->buf;
            break;
        case POW_CAL_BAND_5G_40:
            g_pow_cail.delta_gain_5g_40[info->stream][info->band_idx].delta_gain[info->gain_lvl] = info->buf;
            break;
        case POW_CAL_BAND_5G_80:
            g_pow_cail.delta_gain_5g_80[info->stream][info->band_idx].delta_gain[info->gain_lvl] = info->buf;
            break;
        case POW_CAL_BAND_5G_160:
            g_pow_cail.delta_gain_5g_160[info->stream][info->band_idx].delta_gain[info->gain_lvl] = info->buf;
            break;
        default:
            break;
    }
}

int32_t conn_nve_write_powcal_data_pro_gf61(const nv_pow_user_info_stru *info)
{
    // 首先拷贝数据至ram
    if (info->band_type >= POW_CAL_BAND_2G_11B && info->band_type < POW_CAL_BAND_5G_20) {
        conn_nve_copy_pow_data_from_user_2g_gf61(info);
    } else if (info->band_type >= POW_CAL_BAND_5G_20 && info->band_type <= POW_CAL_BAND_5G_160) {
        conn_nve_copy_pow_data_from_user_5g_gf61(info);
    }
    return OAL_SUCC;
}
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
int32_t write_to_nv(uint8_t *data, uint32_t size, uint32_t nv_number, const char *nv_name)
{
    return 0;
}
int32_t read_from_nv(uint8_t *pc_out, uint32_t size, uint32_t nv_number, const char *nv_name)
{
    return 0;
}
#else
int32_t write_to_nv(uint8_t *data, uint32_t size, uint32_t nv_number, const char *nv_name)
{
    struct external_nve_info_user info;
    int32_t ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), nv_name) != EOK) {
        pr_err("write_cali_data_to_nv:write failed nv_name size[%lu]", sizeof(info.nv_name));
        return CONN_NVE_ERROR_WRITE;
    }
    info.nv_name[sizeof(info.nv_name) - 1] = '\0';
    info.nv_number = nv_number;
    info.valid_size = size;
    info.nv_operation = NV_WRITE;
    if (memcpy_s(info.nv_data, sizeof(info.nv_data), data, size) != EOK) {
        pr_err("write_conf_to_nvram:write nvm[memcpy_s] failed");
        return CONN_NVE_ERROR_WRITE;
    }

    ret = external_nve_direct_access_interface(&info);
    if (ret < -1) {
        pr_err("write nvm failed");
        return CONN_NVE_ERROR_WRITE;
    }
    return CONN_NVE_RET_OK;
}

int32_t read_from_nv(uint8_t *pc_out, uint32_t size, uint32_t nv_number, const char *nv_name)
{
    struct external_nve_info_user info;
    int32_t ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    memset_s(pc_out, size, 0, size);
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), nv_name) != EOK) {
        pr_err("read nvm failed nv_name size[%lu] less than input[%s]", sizeof(info.nv_name), nv_name);
        return CONN_NVE_ERROR_READ;
    }
    info.nv_name[sizeof(info.nv_name) - 1] = '\0';
    info.nv_number = nv_number;
    info.valid_size = size;
    info.nv_operation = NV_READ;

    ret = external_nve_direct_access_interface(&info);
    if (size < sizeof(info.nv_data) || size <= OAL_STRLEN(info.nv_data)) {
        pr_err("read nvm item[%s] fail, lenth[%d] longer than input[%d]",
               nv_name, (uint32_t)OAL_STRLEN(info.nv_data), size);
        return CONN_NVE_ERROR_READ;
    }
    if (ret == OAL_SUCC) {
        if (memcpy_s(pc_out, size, info.nv_data, sizeof(info.nv_data)) != EOK) {
            pr_err("read nvm{%s}lenth[%d] longer than input[%d]",
                   info.nv_data, (uint32_t)OAL_STRLEN(info.nv_data), size);
            return CONN_NVE_ERROR_READ;
        }
    } else {
        pr_err("read nvm [%d] %s failed", nv_number, nv_name);
        return CONN_NVE_ERROR_READ;
    }
    return CONN_NVE_RET_OK;
}
#endif
int32_t write_cali_data_to_nv(uint8_t *data, int32_t size)
{
    struct external_nve_info_user info;
    int32_t ret, i = 0;
    char nv_name[NV_NAME_MAXLENGTH];
    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (size == sizeof(g_xo_pwr_diff)) {
        // xo校准数据
        if (strncpy_s(nv_name, sizeof(nv_name),
                      GF61_NV_WIC1_2G40M_NAME, strlen(GF61_NV_WIC1_2G40M_NAME)) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY,
                             "{wal_atcmdsrv_ioctl_set_xo_pwr_diff_to_nv::strncpy_s fail!}");
        return -OAL_EINVAL;
        }
        return write_to_nv(data, size, GF61_2G40_OFDM_C1_NUM, nv_name);
    } else {
        // 功率校准数据
        while (size > 0) {
            if (strncpy_s(nv_name, sizeof(nv_name), g_st_nv_name_map_gf61[i].nv_name,
                          strlen(g_st_nv_name_map_gf61[i].nv_name)) != EOK) {
                oam_warning_log0(0, OAM_SF_ANY,
                                 "{wal_atcmdsrv_ioctl_set_xo_pwr_diff_to_nv::strncpy_s fail!}");
                return -OAL_EINVAL;
            }
            if (size > NVE_NV_DATA_SIZE) {
                ret = write_to_nv(data, NVE_NV_DATA_SIZE, g_st_nv_name_map_gf61[i].nv_num, nv_name);
            } else {
                ret = write_to_nv(data, size, g_st_nv_name_map_gf61[i].nv_num, nv_name);
            }
            if (ret < -1) {
                pr_err("write nvm failed");
                return CONN_NVE_ERROR_WRITE;
            }
            i++;
            size -= NVE_NV_DATA_SIZE;
            data += NVE_NV_DATA_SIZE;
        }
    }
    return CONN_NVE_RET_OK;
}
int32_t read_cali_data_from_nvram(uint8_t *pc_out, int32_t size)
{
    struct external_nve_info_user info;
    int32_t ret, i = 0;
    char nv_name[NV_NAME_MAXLENGTH];
    char read_buf[NVE_NV_DATA_SIZE];
    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (size == sizeof(int16_t)) {
        // xo校准数据
        if (strncpy_s(nv_name, sizeof(nv_name),
                      GF61_NV_WIC1_2G40M_NAME, strlen(GF61_NV_WIC1_2G40M_NAME)) != EOK) {
            oam_warning_log0(0, OAM_SF_ANY,
                             "{wal_atcmdsrv_ioctl_set_xo_pwr_diff_to_nv::strncpy_s fail!}");
        return -OAL_EINVAL;
        }
        ret = read_from_nv(read_buf, NVE_NV_DATA_SIZE, GF61_2G40_OFDM_C1_NUM, nv_name);
        if (ret != CONN_NVE_RET_OK) {
                pr_err("write nvm failed");
                return CONN_NVE_ERROR_READ;
            }
        if (memcpy_s(pc_out, size, read_buf, size) != EOK) {
            return CONN_NVE_ERROR_READ;
        }
        return CONN_NVE_RET_OK;
    } else {
        // 功率校准数据
        while (size > 0) {
            if (strncpy_s(nv_name, sizeof(nv_name), g_st_nv_name_map_gf61[i].nv_name,
                          strlen(g_st_nv_name_map_gf61[i].nv_name)) != EOK) {
                oam_warning_log0(0, OAM_SF_ANY,
                                 "{wal_atcmdsrv_ioctl_set_xo_pwr_diff_to_nv::strncpy_s fail!}");
                return -OAL_EINVAL;
            }
            if (size > NVE_NV_DATA_SIZE) {
                ret = read_from_nv(pc_out, NVE_NV_DATA_SIZE, g_st_nv_name_map_gf61[i].nv_num, nv_name);
            } else {
                ret = read_from_nv(read_buf, NVE_NV_DATA_SIZE, g_st_nv_name_map_gf61[i].nv_num, nv_name);
                ret += memcpy_s(pc_out, size, read_buf, size);
            }
            if (ret != CONN_NVE_RET_OK) {
                pr_err("write nvm failed");
                return CONN_NVE_ERROR_READ;
            }
            i++;
            size -= NVE_NV_DATA_SIZE;
            pc_out += NVE_NV_DATA_SIZE;
        }
    }
    return CONN_NVE_RET_OK;
}
#endif
