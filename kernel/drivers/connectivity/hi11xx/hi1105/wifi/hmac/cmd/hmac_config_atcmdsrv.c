/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 配置相关实现hmac接口实现源文件
 * 作    者 :
 * 创建日期 : 2013年1月8日
 */

/* 1 头文件包含 */
#include "securec.h"
#include "securectype.h"

#include "oam_ext_if.h"
#include "wlan_chip_i.h"
#include "frw_ext_if.h"
#include "hmac_config.h"
#include "hmac_ext_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "wal_linux_atcmdsrv.h"
#endif

#include "hisi_conn_nve_interface.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CONFIG_ATCMDSRV_C
#ifdef HISI_CONN_NVE_SUPPORT
uint32_t hwifi_config_set_cali_chan(uint32_t);
uint32_t hwifi_config_get_cali_chan(uint32_t, uint32_t);
#endif
#ifdef PLATFORM_DEBUG_ENABLE
/*
 * 函 数 名  : hmac_atcmdsrv_dbb_num_response
 * 功能描述  : hmac接收dmac抛回来的查询DBB版本号事件
 * 1.日    期  : 2015年07月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_atcmdsrv_dbb_num_response(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_mac_vap = &(pst_hmac_vap->st_vap_base_info);
    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_DBB_NUM_INFO_EVENT) {
        pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = pst_atcmdsrv_dbb_num_response_event->event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}

/*
 * 函 数 名  : hmac_atcmdsrv_get_ant_response
 * 功能描述  : hmac接收dmac抛回来的查询DBB版本号事件
 * 1.日    期  : 2015年07月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_atcmdsrv_get_ant_response(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_dbb_num_response_event = NULL;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_dbb_num_response::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_atcmdsrv_dbb_num_response_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_dbb_num_response_event->uc_event_id == OAL_ATCMDSRV_GET_ANT) {
        pst_hmac_vap->st_atcmdsrv_get_status.uc_ant_status = pst_atcmdsrv_dbb_num_response_event->event_para;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_ant_flag = OAL_TRUE;
    oal_wait_queue_wake_up_interrupt(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
/*
 * 函 数 名  : hmac_atcmdsrv_lte_gpio_check
 * 功能描述  : 查询LTE GPIO管脚检测dmac返回情况
 * 1.日    期  : 2015年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_atcmdsrv_lte_gpio_check(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    dmac_atcmdsrv_atcmd_response_event *pst_atcmdsrv_lte_gpio_check_event = NULL;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_atcmdsrv_lte_gpio_check_event = (dmac_atcmdsrv_atcmd_response_event *)(puc_param);
    if (pst_atcmdsrv_lte_gpio_check_event->uc_event_id == OAL_ATCMDSRV_LTE_GPIO_CHECK) {
        /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
        pst_hmac_vap->st_atcmdsrv_get_status.uc_lte_gpio_check_flag = OAL_TRUE;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        g_uc_dev_lte_gpio_level = pst_atcmdsrv_lte_gpio_check_event->uc_reserved;
#endif
        oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
    }

    return OAL_SUCC;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/*
 * 函 数 名  : hmac_atcmdsrv_report_efuse_reg
 * 1.日    期  : 2015年10月10日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_atcmdsrv_report_efuse_reg(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint16_t loop;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_lte_gpio_check::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    for (loop = 0; loop < 32; loop++) { /* buffer length 32 */
        g_us_efuse_buffer[loop] = *(uint16_t *)(puc_param);
        puc_param += BYTE_OFFSET_2;
    }
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_efuse_reg_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));

    return OAL_SUCC;
}
#endif
#endif
#ifdef HISI_CONN_NVE_SUPPORT
OAL_STATIC int32_t hmac_save_dpd_by_band(nv_dpd_user_info_stru *info,
    wlan_cali_data_para_stru *cali_data)
{
    uint8_t dpd_lvl_idx;
    int32_t ret;
    wlan_cali_basic_para_stru *basic_cali_data = NULL;
    wlan_cali_basic_para_stru *basic_cali_data_5g[] = {
        cali_data->rf_cali_data[info->stream].cali_data_5g_20m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_5g_20m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_5g_80m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_5g_160m.cali_data
    };
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    wlan_cali_basic_para_stru *basic_cali_data_6g[] = {
        cali_data->rf_cali_data[info->stream].cali_data_6g_20m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_6g_20m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_6g_80m.cali_data,
        cali_data->rf_cali_data[info->stream].cali_data_6g_160m.cali_data
    };
#endif
    if (info->band_type == WLAN_CALI_BAND_2G) {
        basic_cali_data = &cali_data->rf_cali_data[info->stream].cali_data_2g_20m.cali_data[info->band_idx];
    } else if (info->band_type == WLAN_CALI_BAND_5G) {
        basic_cali_data = &basic_cali_data_5g[info->bw][info->band_idx];
    } else {
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        basic_cali_data = &basic_cali_data_6g[info->bw][info->band_idx];
#endif
    }

    for (dpd_lvl_idx = 0; dpd_lvl_idx < WLAN_DPD_TPC_LEVEL_NUM; dpd_lvl_idx++) {
        info->buf = &basic_cali_data->dpd_cali_data.dpd_cali_para[dpd_lvl_idx];
        info->dpd_lvl = dpd_lvl_idx;
        ret = conn_nve_write_dpd_data(info);
        if (ret != CONN_NVE_RET_OK) {
            return ret;
        }
    }
    return CONN_NVE_RET_OK;
}

OAL_STATIC int32_t hmac_atcmdsrv_save_2g_dpd_data(wlan_cali_data_para_stru *cali_data, uint8_t rf_num)
{
    nv_dpd_user_info_stru info = { WLAN_SINGLE_STREAM_0, WLAN_CALI_BAND_2G, WLAN_BANDWIDTH_20, 0, 0, 0, NULL };
    int32_t ret;

    for (info.stream = WLAN_SINGLE_STREAM_0; info.stream < rf_num; info.stream++) {
        if (hwifi_config_get_cali_chan(info.stream, WLAN_CALI_BAND_2G) == 0) {
            continue;
        }
        for (info.band_idx = 0; info.band_idx < WLAN_2G_CALI_BAND_NUM; info.band_idx++) {
            ret = hmac_save_dpd_by_band(&info, cali_data);
            if (ret != CONN_NVE_RET_OK) {
                return ret;
            }
        }
    }
    return CONN_NVE_RET_OK;
}
OAL_STATIC int32_t hmac_atcmdsrv_save_dpd_band_data(wlan_cali_data_para_stru *cali_data, nv_dpd_user_info_stru *info,
    uint8_t *band_num_info, uint8_t len)
{
    int32_t ret = CONN_NVE_RET_OK;
    for (info->bw = WLAN_BANDWIDTH_20; info->bw <= WLAN_BANDWIDTH_160; info->bw++) {
        if (info->bw >= len) {
            break;
        }
        for (info->band_idx = 0; info->band_idx < band_num_info[info->bw]; info->band_idx++) {
            ret += hmac_save_dpd_by_band(info, cali_data);
        }
    }
    return ret;
}
OAL_STATIC int32_t hmac_atcmdsrv_save_5g_dpd_data(wlan_cali_data_para_stru *cali_data, uint8_t rf_num)
{
    nv_dpd_user_info_stru info = { WLAN_SINGLE_STREAM_0, WLAN_CALI_BAND_5G, 0, 0, 0, 0, NULL };
    uint8_t band_num_info[] = { WLAN_5G_20M_CALI_BAND_NUM, WLAN_5G_20M_CALI_BAND_NUM, WLAN_5G_20M_CALI_BAND_NUM,
                                WLAN_5G_160M_CALI_BAND_NUM };
    int32_t ret = CONN_NVE_RET_OK;

    for (info.stream = WLAN_SINGLE_STREAM_0; info.stream < rf_num; info.stream++) {
        if (hwifi_config_get_cali_chan(info.stream, WLAN_CALI_BAND_5G) == 0) {
            continue;
        }
        ret += hmac_atcmdsrv_save_dpd_band_data(cali_data, &info, band_num_info, sizeof(band_num_info));
    }
    return ret;
}

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
OAL_STATIC int32_t hmac_atcmdsrv_save_6g_dpd_data(wlan_cali_data_para_stru *cali_data, uint8_t rf_num)
{
    nv_dpd_user_info_stru info = { WLAN_SINGLE_STREAM_0, WLAN_CALI_BAND_6G, 0, 0, 0, 0, NULL };
    uint8_t band_num_info[] = { WLAN_6G_20M_CALI_BAND_NUM, WLAN_6G_20M_CALI_BAND_NUM, WLAN_6G_20M_CALI_BAND_NUM,
                                WLAN_6G_160M_CALI_BAND_NUM };
    int32_t ret = CONN_NVE_RET_OK;

    for (info.stream = WLAN_SINGLE_STREAM_0; info.stream < rf_num; info.stream++) {
        if (hwifi_config_get_cali_chan(info.stream, WLAN_CALI_BAND_6G) == 0) {
            continue;
        }
        ret += hmac_atcmdsrv_save_dpd_band_data(cali_data, &info, band_num_info, sizeof(band_num_info));
    }
    return ret;
}
#endif

OAL_STATIC uint32_t hmac_atcmdsrv_save_dpd_data(mac_vap_stru *mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    hmac_vap_stru *pst_hmac_vap;
    uint8_t *mem_addr = get_cali_data_buf_addr();
    wlan_cali_data_para_stru *cali_data = (wlan_cali_data_para_stru *)mem_addr;
    uint8_t rf_num = g_wlan_spec_cfg->max_rf_num;
    int32_t ret;

    pst_hmac_vap = mac_res_get_hmac_vap(mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(mac_vap->uc_vap_id, OAM_SF_ANY, "{hmac_atcmdsrv_save_dpd_data::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = 0;

    ret = hmac_atcmdsrv_save_2g_dpd_data(cali_data, rf_num);
    if (ret != CONN_NVE_RET_OK) {
        pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = (uint32_t)ret;
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_atcmdsrv_save_dpd_data::save_2g_dpd_data to nv fail, ret = %d", ret);
    }

    ret = hmac_atcmdsrv_save_5g_dpd_data(cali_data, rf_num);
    if (ret != CONN_NVE_RET_OK) {
        pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = (uint32_t)ret;
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_atcmdsrv_save_dpd_data::save_5g_dpd_data to nv fail, ret = %d", ret);
    }
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
    ret = hmac_atcmdsrv_save_6g_dpd_data(cali_data, rf_num);
    if (ret != CONN_NVE_RET_OK) {
        pst_hmac_vap->st_atcmdsrv_get_status.dbb_num = (uint32_t)ret;
        oam_error_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
            "hmac_atcmdsrv_save_dpd_data::save_6g_dpd_data to nv fail, ret = %d", ret);
    }
#endif
    pst_hmac_vap->st_atcmdsrv_get_status.uc_get_dbb_completed_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
    return OAL_SUCC;
}
#endif
/*
 * 函 数 名  : hmac_config_dpd_cali_factory
 * 功能描述  : 产线触发host保存DPD数据到NV
 * 1.日    期  : 2021年6月30日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
uint32_t hmac_config_dpd_cali_factory(mac_vap_stru *pst_mac_vap, uint16_t us_len, uint8_t *puc_param)
{
    uint32_t ret;
#ifdef HISI_CONN_NVE_SUPPORT
    ret = hmac_atcmdsrv_save_dpd_data(pst_mac_vap, us_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
            "{hmac_config_dpd_cali_factory::hmac_atcmdsrv_save_dpd_data failed[%d].}", ret);
    }
    /* 校准数据保存完后清除校准通道 */
    hwifi_config_set_cali_chan(0);
    conn_nve_write_fbgain(hwifi_get_fbgain_param());
#else
    oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
        "{hmac_config_dpd_cali_factory::HISI_CONN_NVE_SUPPORT not defined.}");
    ret = OAL_FAIL;
#endif
    return ret;
}

#ifdef PLATFORM_DEBUG_ENABLE
uint32_t hmac_config_get_temperature(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_TEMPERATURE, uc_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                         "{hmac_config_get_temperature::hmac_config_send_event failed[%d].}", ret);
    }
#endif
    return ret;
}

uint32_t hmac_config_report_temperature(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_temperature::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_us_tsensor_val = *(int16_t *)(puc_param);
    oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
        "{hmac_config_report_temperature::g_us_tsensor_val[%d].}", g_us_tsensor_val);
    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_tsensor_val_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

uint32_t hmac_config_get_hw_word(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_HW_WORD, uc_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                         "{hmac_config_get_hw_word::hmac_config_send_event failed[%d].}", ret);
    }
#endif
    return ret;
}

uint32_t hmac_config_report_hw_word(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_hw_word::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_us_config_word = *(uint32_t *)(puc_param);

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_config_word_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

uint32_t hmac_config_get_board_sn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t ret;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_GET_BOARD_SN, uc_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                         "{hmac_config_get_board_sn::hmac_config_send_event failed[%d].}", ret);
    }
#endif
    return ret;
}

uint32_t hmac_config_report_board_sn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint8_t i;
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_board_sn::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (i = 0; i < WAL_ATCMDSRV_BOARD_SN_LENGTH; i++) {
        g_us_bsn[i] = puc_param[i];
    }

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_report_board_sn_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

uint32_t hmac_config_set_board_sn(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_SET_BOARD_SN, uc_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                         "{hmac_config_set_board_sn::hmac_config_send_event failed[%d].}", ret);
    }
#endif
    return ret;
}

uint32_t hmac_config_flash_test(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
    uint32_t ret = OAL_SUCC;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /***************************************************************************
        抛事件到DMAC层, 同步DMAC数据
    ***************************************************************************/
    ret = hmac_config_send_event(pst_mac_vap, WLAN_CFGID_FLASH_TEST, uc_len, puc_param);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                         "{hmac_config_flash_test::hmac_config_send_event failed[%d].}", ret);
    }
#endif
    return ret;
}

uint32_t hmac_config_report_flash_test(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_board_sn::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_us_flash_check_result = *(int8_t *)(puc_param);

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_check_flash_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

uint32_t hmac_config_report_spmi_test(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hmac_vap_stru *pst_hmac_vap;
    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_board_sn::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_us_spmi_check_result = *(int8_t *)(puc_param);

    /* 唤醒wal_sdt_recv_reg_cmd等待的进程 */
    pst_hmac_vap->st_atcmdsrv_get_status.uc_check_spmi_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}

uint32_t hmac_config_ssi_test(mac_vap_stru *pst_mac_vap, uint8_t uc_len, uint8_t *puc_param)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint32_t ret = OAL_SUCC;
    hmac_vap_stru *pst_hmac_vap;
    hmac_atcmdsrv_ssi_stru st_ssi_stru;

    pst_hmac_vap = mac_res_get_hmac_vap(pst_mac_vap->uc_vap_id);
    if (pst_hmac_vap == NULL) {
        oam_error_log0(pst_mac_vap->uc_vap_id, OAM_SF_EQUIP_TEST,
                       "{hmac_config_report_board_sn::pst_hmac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (EOK != memcpy_s(&st_ssi_stru, sizeof(hmac_atcmdsrv_ssi_stru),
                        puc_param, sizeof(hmac_atcmdsrv_ssi_stru))) {
        oam_error_log0(0, OAM_SF_CFG, "wal_atcmdsrv_ioctl_get_pwr_diff::memcpy fail!");
        return OAL_FAIL;
    }

    ret = gf61_ssi_test(st_ssi_stru.ssi_addr, st_ssi_stru.ssi_value);
    if (ret != OAL_SUCC) {
        g_us_ssi_check_result = OAL_FAIL;
        pst_hmac_vap->st_atcmdsrv_get_status.uc_check_ssi_flag = OAL_TRUE;
        oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
        return ret;
    }

    g_us_ssi_check_result = OAL_SUCC;
    pst_hmac_vap->st_atcmdsrv_get_status.uc_check_ssi_flag = OAL_TRUE;
    oal_wait_queue_wake_up(&(pst_hmac_vap->query_wait_q));
#endif
    return OAL_SUCC;
}
#endif
