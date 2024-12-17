/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 芯片差异接口文件
 * 作    者 :
 * 创建日期 : 2020年6月19日
 */

#include "wlan_chip.h"
#include "oal_main.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "mac_hiex.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "mac_ftm.h"
#endif
#include "hmac_ext_if.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_blockack.h"
#include "hmac_cali_mgmt.h"
#include "hmac_tx_data.h"
#include "hmac_tx_amsdu.h"
#include "hmac_hcc_adapt.h"
#include "hmac_scan.h"
#include "wal_linux_ioctl.h"
#include "hmac_config.h"
#include "mac_mib.h"
#include "hmac_pm.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "plat_pm_wlan.h"
#endif
#include "hisi_customize_wifi.h"
#include "hisi_customize_wifi_mp13.h"
#include "hisi_customize_config_dts_mp13.h"
#include "hisi_customize_config_priv_mp13.h"
#include "hisi_customize_config_cmd_mp13.h"
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
#include "hmac_lp_miracast.h"
#endif


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WLAN_CHIP_1103_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
OAL_STATIC void hwifi_cfg_host_global_switch_init(void)
{
#ifndef _PRE_LINUX_TEST
    /*************************** 低功耗定制化 *****************************/
    /* 由于device 低功耗开关不是true false,而host是,先取定制化的值赋给device开关,再根据此值给host低功耗开关赋值 */
    g_wlan_device_pm_switch = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_POWERMGMT_SWITCH);
    g_wlan_ps_mode = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_PS_MODE);
    g_wlan_fast_ps_dyn_ctl = ((MAX_FAST_PS == g_wlan_ps_mode) ? 1 : 0);
    g_wlan_min_fast_ps_idle = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MIN_FAST_PS_IDLE);
    g_wlan_max_fast_ps_idle = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MAX_FAST_PS_IDLE);
    g_wlan_auto_ps_screen_on = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENON);
    g_wlan_auto_ps_screen_off = (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_AUTO_FAST_PS_THRESH_SCREENOFF);

    g_feature_switch[HMAC_MIRACAST_SINK_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_MIRACAST_SINK_ENABLE) == 1);
    g_feature_switch[HMAC_MIRACAST_REDUCE_LOG_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_REDUCE_MIRACAST_LOG) == 1);
    g_feature_switch[HMAC_BIND_LOWEST_LOAD_CPU] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_BIND_LOWEST_LOAD_CPU) == 1);
    g_feature_switch[HMAC_CORE_BIND_SWITCH] =
        (hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CORE_BIND_CAP) == 1);
#endif
}

OAL_STATIC void hwifi_cfg_host_global_init_sounding(void)
{
    int32_t l_priv_value = 0;
    int32_t l_ret;

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_RX_STBC, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: rx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_rx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_TX_STBC, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: tx stbc[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_tx_stbc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFER, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: su bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_su_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_SU_BFEE, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: su bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_su_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFER, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: mu bfer[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_MU_BFEE, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: mu bfee[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }
}

/*
 * 函 数 名  : hwifi_cfg_host_global_init_param_extend
 * 功能描述  : 配置host全局变量值hwifi_cfg_host_global_init_param的扩展函数
 * 1.日    期  : 2020年1月14日
 *   作    者  : wifi
 *   修改内容  : 新生成函数
 */
OAL_STATIC void hwifi_cfg_host_global_init_param_extend(void)
{
    int32_t priv_value = 0;
    int32_t l_ret;

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LOCK_CPU_FREQ, &priv_value);
    if (l_ret == OAL_SUCC) {
        g_freq_lock_control.uc_lock_max_cpu_freq = (oal_bool_enum_uint8) !!priv_value;
    }
    oal_io_print("hwifi_cfg_host_global_init_param_extend:lock_max_cpu_freq[%d]\r\n",
        g_freq_lock_control.uc_lock_max_cpu_freq);

#ifdef _PRE_WLAN_FEATURE_HIEX
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_HIEX_CAP, &priv_value);
    if (l_ret == OAL_SUCC) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(mac_hiex_cap_stru), &priv_value,
            sizeof(priv_value)) != EOK) {
            oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap memcpy_s fail!");
        }
    }
    oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap[0x%X]\r\n", *(uint32_t *)&g_st_default_hiex_cap);
#endif
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_OPTIMIZED_FEATURE_SWITCH, &priv_value);
    if (l_ret == OAL_SUCC) {
        g_optimized_feature_switch_bitmap = (uint8_t)priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_P2P_GO_ASSOC_USER_MAX_NUM, &priv_value);
    if (l_ret == OAL_SUCC && (uint8_t)priv_value > 0 && (uint8_t)priv_value <= 8) { /* 8为最大用户数 */
        g_p2p_go_max_user_num = (uint8_t)priv_value;
    } else {
        oam_warning_log2(0, OAM_SF_INI,
            "{hwifi_cfg_host_global_init_param_extend::wlan_go_assoc_user_max_num read fail, ret[%d], value[%d].}",
            l_ret, (uint8_t)priv_value);
    }
#ifdef _PRE_WLAN_FEATURE_FTM
        l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_FTM_CAP, &priv_value);
        if (l_ret == OAL_SUCC) {
            g_mac_ftm_cap = (uint8_t)priv_value;
        }
#endif
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_EXTPAND_FEATURE_SWITCH_BITMAP, &priv_value);
    if (l_ret == OAL_SUCC) {
        g_expand_feature_switch_bitmap = (uint32_t)priv_value;
    }
}

void hwifi_cfg_host_global_init_dbdc_param(void)
{
    uint8_t uc_cmd_idx;
    uint8_t uc_device_idx;
    int32_t l_priv_value = 0;
    int32_t l_ret;

    uc_cmd_idx = WLAN_CFG_PRIV_DBDC_RADIO_0;
    for (uc_device_idx = 0; uc_device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_idx++) {
        l_ret = hwifi_get_init_priv_value(uc_cmd_idx, &l_priv_value);
        if (l_ret == OAL_SUCC) {
            /* 定制化 RADIO_0 bit4 给dbdc软件开关用 */
            g_wlan_priv_dbdc_radio_cap = (uint8_t)(((uint32_t)l_priv_value & 0x10) >> 4);
            l_priv_value = (int32_t)((uint32_t)l_priv_value & 0x0F);
            g_wlan_service_device_per_chip[uc_device_idx] = (uint8_t)(uint32_t)l_priv_value;
        }

        uc_cmd_idx++;
    }
    /* 同步host侧业务device */
    memcpy_s(g_auc_mac_device_radio_cap, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP,
             g_wlan_service_device_per_chip, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
}

/*
 * 函 数 名  : hwifi_cfg_host_global_init_param_mp13
 * 功能描述  : 初始化定制化ini文件host侧全局变量
 */
OAL_STATIC void hwifi_cfg_host_global_init_param_mp13(void)
{
    int32_t l_priv_value = 0;
    int32_t l_ret;

    hwifi_cfg_host_global_switch_init();
    /*************************** 私有定制化 *******************************/
    hwifi_cfg_host_global_init_dbdc_param();
    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_BW_MAX_WITH, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: bw max with[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_channel_width = (wlan_bw_cap_enum_uint8)l_priv_value;
    }

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_LDPC_CODING, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: ldpc coding[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_ldpc_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    hwifi_cfg_host_global_init_sounding();

    l_ret = hwifi_get_init_priv_value(WLAN_CFG_PRIV_1024_QAM, &l_priv_value);
    oal_io_print("hwifi_cfg_host_global_init_param: 1024 qam[%d] ret[%d]\r\n", l_priv_value, l_ret);
    if (l_ret == OAL_SUCC) {
        g_pst_mac_device_capability[0].en_1024qam_is_supp = (oal_bool_enum_uint8)l_priv_value;
    }

    hwifi_cfg_host_global_init_param_extend();

    return;
}

OAL_STATIC oal_bool_enum_uint8 wlan_first_powon_mark_mp13(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;
    int32_t l_priv_value = 0;

    if (hwifi_get_init_priv_value(WLAN_CFG_PRIV_CALI_DATA_MASK, &l_priv_value) == OAL_SUCC) {
        cali_first_pwr_on = !!(CALI_FIST_POWER_ON_MASK & (uint32_t)l_priv_value);
        oal_io_print("host_module_init:cali_first_pwr_on pri_val[0x%x]first_pow[%d]\r\n",
            l_priv_value, cali_first_pwr_on);
    }
    return cali_first_pwr_on;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_is_aput_support_160m_mp13(void)
{
    int32_t l_priv_value;
    if (hwifi_get_init_priv_value(WLAN_CFG_APUT_160M_ENABLE, &l_priv_value) == OAL_SUCC) {
        return !!l_priv_value;
    }
    return OAL_FALSE;
}

OAL_STATIC void wlan_chip_get_flow_ctrl_used_mem_mp13(struct wlan_flow_ctrl_params *flow_ctrl)
{
    flow_ctrl->start = (uint16_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_START);
    flow_ctrl->stop  = (uint16_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_USED_MEM_FOR_STOP);
}

OAL_STATIC uint32_t hwifi_force_update_rf_params_mp13(void)
{
    return hwifi_config_init(CUS_TAG_NV);
}

OAL_STATIC uint8_t wlan_chip_get_selfstudy_country_flag_mp13(void)
{
    int32_t l_priv_value = 0;
    hwifi_get_init_priv_value(WLAN_CFG_PRIV_COUNRTYCODE_SELFSTUDY_CFG, &l_priv_value);
    return (uint8_t)l_priv_value;
}

OAL_STATIC uint32_t wlan_chip_get_11ax_switch_mask_mp13(void)
{
    int32_t l_priv_value = 0;
    hwifi_get_init_priv_value(WLAN_CFG_PRIV_11AX_SWITCH, &l_priv_value);
    return l_priv_value;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_get_11ac2g_enable_mp13(void)
{
    return !!hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_11AC2G_ENABLE);
}

OAL_STATIC uint32_t wlan_chip_get_probe_resp_mode_mp13(void)
{
    return hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_PROBE_RESP_MODE);
}
OAL_STATIC uint8_t wlan_chip_get_chn_radio_cap_mp15(void)
{
    return (uint8_t)hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_CHANN_RADIO_CAP);
}
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE

OAL_STATIC oal_bool_enum_uint8 wlan_chip_h2d_cmd_need_filter_mp13(uint32_t cmd_id)
{
    /* 对mp13/mp15产品，mp16独有事件不需要下发device */
    return oal_value_in_valid_range(cmd_id, WLAN_CFGID_MP16_PRIV_START, WLAN_CFGID_MP16_PRIV_END);
}

OAL_STATIC uint32_t wlan_chip_update_cfg80211_mgmt_tx_wait_time_mp13(uint32_t wait_time)
{
    return wait_time;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_setup_ba_session_mp13(void)
{
    return OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_check_need_process_bar_mp13(void)
{
    /* mp13/mp15需要软件处理bar */
    return OAL_TRUE;
}

OAL_STATIC oal_bool_enum_uint8 wlan_chip_ba_need_check_lut_idx_mp13(void)
{
    /* mp13/mp15需要检查LUT idx */
    return OAL_TRUE;
}

OAL_STATIC void wlan_chip_mac_mib_set_auth_rsp_time_out_mp13(mac_vap_stru *mac_vap)
{
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_AP_TIMEOUT);
    } else {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_TIMEOUT);
    }
}

static void wlan_chip_proc_query_station_packets_mp13(hmac_vap_stru *hmac_vap,
    dmac_query_station_info_response_event *response_event)
{
    /* mp13 tx/rx帧统计从host获取 */
    hmac_vap->station_info.rx_packets = response_event->rx_packets;
    hmac_vap->station_info.rx_bytes = response_event->rx_bytes;
    hmac_vap->station_info.rx_dropped_misc = response_event->rx_dropped_misc;

    hmac_vap->station_info.tx_packets = response_event->tx_packets;
    hmac_vap->station_info.tx_bytes = response_event->tx_bytes;
    hmac_vap->station_info.tx_retries = response_event->tx_retries;
    hmac_vap->station_info.tx_failed = response_event->tx_failed;
}
static uint32_t wlan_chip_scan_req_alloc_and_fill_netbuf_mp13(frw_event_mem_stru *event_mem, hmac_vap_stru *hmac_vap,
                                                              oal_netbuf_stru **netbuf_scan_req, const void *params)
{
    int32_t ret;
    uint8_t *netbuf_data = NULL;
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *scan_req_event = NULL;
    mac_scan_req_stru *scan_params = NULL;

    /* 申请netbuf内存  */
    (*netbuf_scan_req) = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        (sizeof(mac_scan_req_h2d_stru)), OAL_NETBUF_PRIORITY_MID);
    if ((*netbuf_scan_req) == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_req_post_event_msg_info_set::netbuf_scan_req alloc failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************** copy data **************************/
    memset_s(oal_netbuf_cb(*netbuf_scan_req), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    netbuf_data = (uint8_t *)(oal_netbuf_data(*netbuf_scan_req));
    /* 拷贝扫描请求参数到netbuf data区域 */
    ret = memcpy_s(netbuf_data, sizeof(mac_scan_req_h2d_stru), (uint8_t *)params, sizeof(mac_scan_req_h2d_stru));
    if (ret != EOK) {
        oal_netbuf_free(*netbuf_scan_req);
        oam_error_log0(0, OAM_SF_ANY, "wlan_chip_scan_req_alloc_and_fill_netbuf::memcpy fail!");
        return OAL_FAIL;
    }
    /* 同步至dmac的扫描请求参数不能携带hmac的钩子 */
    scan_params = &(((mac_scan_req_h2d_stru *)netbuf_data)->st_scan_params);
    scan_params->p_fn_cb = NULL;

    /* 拷贝netbuf 到事件数据区域 */
    scan_req_event = (dmac_tx_event_stru *)event->auc_event_data;
    scan_req_event->pst_netbuf = (*netbuf_scan_req);
    scan_req_event->us_frame_len = sizeof(mac_scan_req_h2d_stru);
    scan_req_event->us_remain = 0;
    return OAL_SUCC;
}

static uint32_t wlan_chip_send_connect_security_params_mp13(mac_vap_stru *mac_vap,
    mac_conn_security_stru *connect_security_params)
{
    mac_conn_security_stru_mp13 connect_params_mp13 = { 0 };

    /* 拷贝所有内容到mp13/mp15对应结构体 */
    connect_params_mp13.en_privacy = connect_security_params->en_privacy;
    connect_params_mp13.en_auth_type = connect_security_params->en_auth_type;
    connect_params_mp13.uc_wep_key_len = connect_security_params->uc_wep_key_len;
    connect_params_mp13.uc_wep_key_index = connect_security_params->uc_wep_key_index;
    (void)memcpy_s(connect_params_mp13.auc_wep_key, WLAN_WEP104_KEY_LEN,
        connect_security_params->auc_wep_key, WLAN_WEP104_KEY_LEN);
    connect_params_mp13.en_mgmt_proteced = connect_security_params->en_mgmt_proteced;
    connect_params_mp13.en_pmf_cap = connect_security_params->en_pmf_cap;
    connect_params_mp13.en_wps_enable = connect_security_params->en_wps_enable;
    /* mp13/mp15加密能力结构体转换 */
    connect_params_mp13.st_crypto.wpa_versions = connect_security_params->st_crypto.wpa_versions;
    connect_params_mp13.st_crypto.group_suite = connect_security_params->st_crypto.group_suite;
    connect_params_mp13.st_crypto.group_mgmt_suite = connect_security_params->st_crypto.group_mgmt_suite;
    connect_params_mp13.st_crypto.aul_pair_suite[0] = connect_security_params->st_crypto.aul_pair_suite[0];
    connect_params_mp13.st_crypto.aul_pair_suite[1] = connect_security_params->st_crypto.aul_pair_suite[1];
    connect_params_mp13.st_crypto.aul_akm_suite[0] = connect_security_params->st_crypto.aul_akm_suite[0];
    connect_params_mp13.st_crypto.aul_akm_suite[1] = connect_security_params->st_crypto.aul_akm_suite[1];

#ifdef _PRE_WLAN_FEATURE_11R
    (void)memcpy_s(connect_params_mp13.auc_mde, NUM_8_BYTES,
        connect_security_params->auc_mde, NUM_8_BYTES);
#endif
    connect_params_mp13.c_rssi = connect_security_params->c_rssi;
    (void)memcpy_s(connect_params_mp13.rssi, HD_EVENT_RF_NUM, connect_security_params->rssi, HD_EVENT_RF_NUM);
    connect_params_mp13.is_wapi_connect = connect_security_params->is_wapi_connect;

    return hmac_config_send_event(mac_vap, WLAN_CFGID_CONNECT_REQ,
        sizeof(connect_params_mp13), (uint8_t *)(&connect_params_mp13));
}

void wlan_chip_lp_miracast_stat_mbps_mp15(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    hmac_config_lp_miracast_stat_mbps(tx_throughput_mbps, rx_throughput_mbps);
}

static uint32_t wlan_chip_set_lp_miracast_mp15(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_lp_miracast(mac_vap, len, param);
}

/*
 * 功能描述  : mp15需要检查legacy ap和p2p device是否共存, 返回true表示支持此种共存模式，false表示不支持
 */
static oal_bool_enum_uint8 wlan_chip_ap_p2p_device_coex_check_valid_mp15(mac_device_stru *mac_device,
    mac_cfg_add_vap_param_stru *add_vap_param)
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;
    oal_bool_enum_uint8 is_add_chba = OAL_FALSE;
#ifdef _PRE_WLAN_CHBA_MGMT
    if (add_vap_param->chba_mode == CHBA_MODE) {
        is_add_chba = OAL_TRUE;
    }
#endif
    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_SCAN, "wlan_chip_ap_p2p_device_coex_check_valid_mp15::vap is null! vap id is %d",
                mac_device->auc_vap_id[vap_idx]);
            continue;
        }
        /* 已存在P2P device，无法添加legacy AP */
        if (mac_vap->en_p2p_mode == WLAN_P2P_DEV_MODE) {
            if (add_vap_param->en_vap_mode == WLAN_VAP_MODE_BSS_AP &&
                add_vap_param->en_p2p_mode == WLAN_LEGACY_VAP_MODE && is_add_chba == OAL_FALSE) {
                oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                    "{wlan_chip_ap_p2p_device_coex_check_valid_mp15::p2p dev exists do not add legacy ap.}");
                return OAL_FALSE;
            }
        }
        /* 已存在legacy ap, 无法添加P2P device */
#ifdef _PRE_WLAN_CHBA_MGMT
        if ((is_legacy_ap(mac_vap) && mac_is_chba_mode(mac_vap) == OAL_FALSE) &&
            (add_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE)) {
#else
        if (is_legacy_ap(mac_vap) && (add_vap_param->en_p2p_mode == WLAN_P2P_DEV_MODE)) {
#endif
            oam_error_log0(mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wlan_chip_ap_p2p_device_coex_check_valid_mp15::legacy ap exists do not add p2p dev.}");
            return OAL_FALSE;
        }
    }
    return OAL_TRUE;
}
/*
 * 检查加密的PN号是否异常。mp13 mp15 芯片不上报PN,返回FALSE
 * 返回值：OAL_TRUE:PN异常；OAL_FALSE:PN正常
 */
static oal_bool_enum_uint8 wlan_chip_defrag_is_pn_abnormal_mp13(hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, oal_netbuf_stru *last_netbuf)
{
    return OAL_FALSE;
}

#ifdef _PRE_WLAN_CHBA_MGMT
/* mp15不支持chba dbdc+dbac共存 */
static oal_bool_enum_uint8 wlan_chip_is_support_chba_dbdc_dbac_mp15(void)
{
    return OAL_FALSE;
}
#endif
static oal_bool_enum_uint8 wlan_chip_is_support_hw_wapi_mp13(void)
{
    return g_wlan_spec_cfg->feature_hw_wapi;
}

static mac_scanned_result_extend_info_stru *wlan_chip_get_scaned_result_extend_info_mp13(oal_netbuf_stru *netbuf,
    uint16_t frame_len)
{
    return (mac_scanned_result_extend_info_stru *)(oal_netbuf_data(netbuf) + frame_len);
}

static uint16_t wlan_chip_get_scaned_payload_extend_len_mp13(void)
{
    return sizeof(mac_scanned_result_extend_info_stru);
}
// 组播图传协议标记字段相对UDP头偏移量
#define HMAC_PT_MCAST_PROTOCOL_OFFSET 12
static oal_bool_enum_uint8 wlan_chip_tx_is_pt_mcast_data(uint8_t *payload)
{
    return (payload[MAC_ADDR_0] == 0x4e && payload[MAC_ADDR_1] == 0x45 && payload[MAC_ADDR_2] == 0x58 &&
            payload[MAC_ADDR_3] == 0x54);
}
static void wlan_chip_tx_pt_mcast_set_cb_mp13(hmac_vap_stru *pst_vap,
    mac_ether_header_stru *ether_hdr, mac_tx_ctl_stru *tx_ctl)
{
    mac_ip_header_stru *ip_header = (mac_ip_header_stru *)(ether_hdr + 1);
    udp_hdr_stru *pst_udp_hdr = (udp_hdr_stru *)(ip_header + 1); /* 偏移一个IP头，取UDP头 */
    uint8_t *payload = (uint8_t *)(pst_udp_hdr + 1);
    payload += HMAC_PT_MCAST_PROTOCOL_OFFSET;

    if (wlan_chip_tx_is_pt_mcast_data(payload) || pst_vap->en_pt_mcast_switch) {
        // 标识为组播图传报文
        mac_get_cb_is_pt_mcast(tx_ctl) = OAL_TRUE;
    }
}
static void wlan_chip_aspm_switch_on_mp15(uint32_t aspm)
{
    mpxx_hcc_ip_pm_ctrl(aspm, HCC_EP_WIFI_DEV);
}

static uint32_t wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp15(void)
{
    int32_t ret = hwifi_get_init_value(CUS_TAG_INI, WLAN_CFG_INIT_REDUCE_PWR_2G_40MHZ_CHANNEL_BITMAP);
    return (ret != INI_FAILED) ? (uint32_t)ret : 0;
}

const struct wlan_chip_ops g_wlan_chip_ops_mp13 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_mp13,
    .first_power_on_mark = wlan_first_powon_mark_mp13,
    .first_powon_cali_completed = NULL,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_mp13,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_mp13,
    .force_update_custom_params = hwifi_force_update_rf_params_mp13,
    .init_nvram_main = hwifi_config_init_nvram_main_mp13,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_mp13,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_mp13,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_mp13,
    .custom_cali = wal_custom_cali_mp13,
    .custom_cali_data_host_addr_init = NULL,
    .send_cali_data = wal_send_cali_data_mp13,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_mp13,
    .show_customize_info = hwifi_get_cfg_params,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_mp13,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_mp13,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_mp13,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_mp13,
    .get_d2h_access_ddr = NULL,
    .get_chn_radio_cap = NULL,
#endif
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_mp13,
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_mp13,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_mp13,

    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_hdl_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_mp13,
    .tx_update_amsdu_num = hmac_update_amsdu_num_mp13, /* 注意：该函数mp13/mp15实现不同 */
    .check_need_process_bar = wlan_chip_check_need_process_bar_mp13,
    .ba_send_reorder_timeout = hmac_ba_send_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_mp13,
    .trx_aspm_switch_on = NULL,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_mp13,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_mp13,
#endif
    .update_arp_tid = NULL,
    .get_6g_flag = mac_get_rx_6g_flag_mp13,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_mp13,
    .save_cali_event = hmac_save_cali_event_mp13,
    .update_cur_chn_cali_data = NULL,
    // 特殊dbdc相关
    .get_special_dbdc_cap = NULL,
    .get_rf_band_from_center_freq = NULL,
    .get_center_freq_from_chn = NULL,
    .is_support_dual_5g_dbdc_by_radio_cap = NULL,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_mp13,
    .tx_set_frame_htc = hmac_tx_set_frame_htc,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_mp13,

    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_mp13,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_mp13, /* 大包AMDPU+大包AMSDU入口mp13 mp15生效 */
#endif
    .check_headroom_len = check_headroom_add_length,
    .adjust_netbuf_data = hmac_adjust_netbuf_data,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_mp13,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_mp13,
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    .set_sniffer_config = hmac_config_set_sniffer_mp13,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .dcsi_config = hmac_device_csi_config,
    .hcsi_config = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .dftm_config = hmac_device_ftm_config,
    .hftm_config = NULL,
    .rrm_proc_rm_request = NULL,
    .dconfig_wifi_rtt_config = hmac_device_wifi_rtt_config,
    .hconfig_wifi_rtt_config = NULL,
    .ftm_vap_init = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = NULL,
#endif
    .update_rxctl_data_type = hmac_rx_netbuf_update_rxctl_data_type,
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    .mcast_ampdu_rx_ba_init = hmac_mcast_ampdu_rx_ba_init,
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU_HW
    .mcast_ampdu_sta_add_multi_user = NULL,
#endif
    .is_dbdc_ini_en = NULL,
    .is_single_dbdc_ini_en = NULL,
#if defined(CONFIG_ARCH_HISI) &&  defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    .frw_task_bind_cpu = NULL,
#endif
#endif

    .send_connect_security_params = wlan_chip_send_connect_security_params_mp13,
    .defrag_is_pn_abnormal = wlan_chip_defrag_is_pn_abnormal_mp13,
    .is_support_hw_wapi = wlan_chip_is_support_hw_wapi_mp13,
    .get_scaned_result_extend_info = wlan_chip_get_scaned_result_extend_info_mp13,
    .get_scaned_payload_extend_len = wlan_chip_get_scaned_payload_extend_len_mp13,
    .tx_pt_mcast_set_cb = wlan_chip_tx_pt_mcast_set_cb_mp13,
#ifdef _PRE_WLAN_CHBA_MGMT
    .is_support_chba_dbdc_dbac = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    .lp_miracast_stat_func = NULL,
    .set_lp_miracast_func = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    .need_decrease_sts = NULL,
#endif
    .ap_p2p_device_coex_check_valid = wlan_chip_ap_p2p_device_coex_check_valid_mp15,
    .get_reduce_pwr_channel_2g_bitmap = wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp15,
};

const struct wlan_chip_ops g_wlan_chip_ops_mp15 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_mp13,
    .first_power_on_mark = wlan_first_powon_mark_mp13,
    .first_powon_cali_completed = NULL,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_mp13,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_mp13,
    .force_update_custom_params = hwifi_force_update_rf_params_mp13,
    .init_nvram_main = hwifi_config_init_nvram_main_mp13,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_mp13,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_mp13,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_mp13,
    .custom_cali = wal_custom_cali_mp13,
    .custom_cali_data_host_addr_init = NULL,
    .send_cali_data = wal_send_cali_data_mp15,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_mp13,
    .show_customize_info = hwifi_get_cfg_params,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_mp13,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_mp13,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_mp13,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_mp13,
    .get_d2h_access_ddr = NULL,
    .get_chn_radio_cap = wlan_chip_get_chn_radio_cap_mp15,
#endif
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_mp13,
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_mp13,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_mp13,

    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_hdl_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_mp13,
    .tx_update_amsdu_num = hmac_update_amsdu_num_mp15, /* 注意：该函数mp13/mp15实现不同 */
    .check_need_process_bar = wlan_chip_check_need_process_bar_mp13,
    .ba_send_reorder_timeout = hmac_ba_send_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_mp13,
    .trx_aspm_switch_on = wlan_chip_aspm_switch_on_mp15,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_mp13,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_mp13,
#endif
    .update_arp_tid = NULL,
    .get_6g_flag = mac_get_rx_6g_flag_mp13,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_mp15,
    .save_cali_event = hmac_save_cali_event_mp15,
    .update_cur_chn_cali_data = NULL,
    // 特殊dbdc相关
    .get_special_dbdc_cap = NULL,
    .get_rf_band_from_center_freq = NULL,
    .get_center_freq_from_chn = NULL,
    .is_support_dual_5g_dbdc_by_radio_cap = NULL,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_mp15,
    .tx_set_frame_htc = hmac_tx_set_frame_htc,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_mp13,

    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_mp13,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_mp13, /* 大包AMDPU+大包AMSDU入口mp13 mp15生效 */
#endif
    .check_headroom_len = check_headroom_add_length,
    .adjust_netbuf_data = hmac_adjust_netbuf_data,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_mp13,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_mp13,
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    .set_sniffer_config = hmac_config_set_sniffer_mp13,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .dcsi_config = hmac_device_csi_config,
    .hcsi_config = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .dftm_config = hmac_device_ftm_config,
    .hftm_config = NULL,
    .rrm_proc_rm_request = NULL,
    .dconfig_wifi_rtt_config = hmac_device_wifi_rtt_config,
    .hconfig_wifi_rtt_config = NULL,
    .ftm_vap_init = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = NULL,
#endif
    .update_rxctl_data_type = hmac_rx_netbuf_update_rxctl_data_type,
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    .mcast_ampdu_rx_ba_init = hmac_mcast_ampdu_rx_ba_init,
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU_HW
    .mcast_ampdu_sta_add_multi_user = NULL,
#endif
    .is_dbdc_ini_en = NULL,
    .is_single_dbdc_ini_en = NULL,
#if defined(CONFIG_ARCH_HISI) &&  defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    .frw_task_bind_cpu = NULL,
#endif
#endif

    .send_connect_security_params = wlan_chip_send_connect_security_params_mp13,
    .defrag_is_pn_abnormal = wlan_chip_defrag_is_pn_abnormal_mp13,
    .is_support_hw_wapi = wlan_chip_is_support_hw_wapi_mp13,
    .get_scaned_result_extend_info = wlan_chip_get_scaned_result_extend_info_mp13,
    .get_scaned_payload_extend_len = wlan_chip_get_scaned_payload_extend_len_mp13,
    .tx_pt_mcast_set_cb = NULL,
#ifdef _PRE_WLAN_CHBA_MGMT
    .is_support_chba_dbdc_dbac = wlan_chip_is_support_chba_dbdc_dbac_mp15,
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    .lp_miracast_stat_func = wlan_chip_lp_miracast_stat_mbps_mp15,
    .set_lp_miracast_func = wlan_chip_set_lp_miracast_mp15,
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    .need_decrease_sts = NULL,
#endif
    .ap_p2p_device_coex_check_valid = wlan_chip_ap_p2p_device_coex_check_valid_mp15,
    .get_reduce_pwr_channel_2g_bitmap = wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp15,
};

const oal_wlan_cfg_stru g_wlan_spec_cfg_mp13 = {
    .feature_m2s_modem_is_open = OAL_FALSE,
    .feature_priv_closed_is_open = OAL_TRUE,
    .feature_hiex_is_open = OAL_FALSE,
    .rx_listen_ps_is_open = OAL_FALSE,
    .feature_11ax_is_open = OAL_FALSE,
    .feature_twt_is_open = OAL_FALSE,
    .feature_11ax_er_su_dcm_is_open = OAL_FALSE,
    .feature_11ax_uora_is_supported = OAL_FALSE,
    .feature_ftm_is_open = OAL_FALSE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_FALSE,
    .feature_ht_self_cure_is_open = OAL_TRUE,
    .feature_slave_ax_is_support = OAL_FALSE,
    .longer_than_16_he_sigb_support = OAL_FALSE,
    .feature_hw_wapi = OAL_FALSE,
    .full_bandwidth_ul_mu_mimo = OAL_FALSE,
    .p2p_device_gc_use_one_vap = OAL_TRUE,
    .max_sta_num = 3,
    .max_p2p_group_num = 1,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP13,
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM_MP13,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM_MP13,
    .max_user_limit = WLAN_ASSOC_USER_MAX_NUM_MP13 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .invalid_user_id = WLAN_ASSOC_USER_MAX_NUM_MP13 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_MP13,
    .max_rx_ba = WLAN_MAX_RX_BA_MP13,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_MP13,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_MP13,
    .max_rf_num = 2,
    .feature_txq_ns_support = OAL_FALSE,
    .feature_txbf_mib_study = OAL_TRUE,
    .bfee_support_ants = 4,        /* mp13最多支持 4*2 MU Sounding */
    .bfer_sounding_dimensions = 1, /* 发送最多支持单流 */
    .vht_bfee_ntx_supports = 4,    /* mp13最多支持 4*2 MU Sounding */
    .vht_max_mpdu_lenth = WLAN_MIB_VHT_MPDU_7991, /* VHT协议mp15最大mpdu_len */
    .pcie_speed = 0, /* 不使用 */
    .trigger_su_beamforming_feedback = OAL_FALSE,
    .trigger_mu_partialbw_feedback = OAL_FALSE,
    .rx_stbc_160 = OAL_FALSE,
    .num_different_channels = 2,
    .max_work_vap_num = 2,
    .rx_checksum_hw_enable = OAL_FALSE,
};

const oal_wlan_cfg_stru g_wlan_spec_cfg_mp15 = {
    .feature_m2s_modem_is_open = OAL_TRUE,
    .feature_priv_closed_is_open = OAL_FALSE,
    .feature_hiex_is_open = OAL_TRUE,
    .rx_listen_ps_is_open = OAL_TRUE,
    .feature_11ax_is_open = OAL_TRUE,
    .feature_twt_is_open = OAL_TRUE,
    .feature_11ax_er_su_dcm_is_open = OAL_TRUE,
    .feature_11ax_uora_is_supported = OAL_FALSE,
    .feature_ftm_is_open = OAL_TRUE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_TRUE,
    .feature_ht_self_cure_is_open = OAL_TRUE,
    .feature_slave_ax_is_support = OAL_FALSE,
    .longer_than_16_he_sigb_support = OAL_FALSE,
    .feature_hw_wapi = OAL_FALSE,
    .full_bandwidth_ul_mu_mimo = OAL_FALSE,
    .p2p_device_gc_use_one_vap = OAL_TRUE,
    .max_sta_num = 3,
    .max_p2p_group_num = 1,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP15,
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM_MP15,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM_MP15,
    .max_user_limit = WLAN_ASSOC_USER_MAX_NUM_MP15 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .invalid_user_id = WLAN_ASSOC_USER_MAX_NUM_MP15 + WLAN_MULTI_USER_MAX_NUM_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_MP15,
    .max_rx_ba = WLAN_MAX_RX_BA_MP15,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_MP15,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_MP15,
    .max_rf_num = 2,
    .feature_txq_ns_support = OAL_FALSE,
    .feature_txbf_mib_study = OAL_TRUE,
    .bfee_support_ants = 8,        /* mp15最多支持 8*2 MU Sounding */
    .bfer_sounding_dimensions = 1, /* 发送最多支持双流 */
    .vht_bfee_ntx_supports = 8,    /* mp13最多支持 4*2 MU Sounding */
    .vht_max_mpdu_lenth = WLAN_MIB_VHT_MPDU_7991, /* VHT协议mp15最大mpdu_len */
    .pcie_speed = 0, /* 不使用 */
    .trigger_su_beamforming_feedback = OAL_FALSE,
    .trigger_mu_partialbw_feedback = OAL_FALSE,
    .rx_stbc_160 = OAL_FALSE,
    .num_different_channels = 2,
    .max_work_vap_num = 3,
    .rx_checksum_hw_enable = OAL_FALSE,
};
