/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 芯片差异接口文件
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
#include "hmac_host_tx_data.h"
#include "hmac_tx_amsdu.h"
#include "hmac_hcc_adapt.h"
#include "hmac_stat.h"
#include "hmac_scan.h"
#include "hmac_pm.h"

#include "hisi_customize_wifi.h"
#include "hisi_customize_wifi_mp16.h"

#include "hmac_config.h"
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
#include "hmac_tcp_ack_buf.h"
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
#include "hmac_csi.h"
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
#include "hmac_ftm.h"
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
#include "hmac_wmmac.h"
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "plat_pm_wlan.h"
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
#include "hmac_dfs.h"
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
#include "hmac_mcast_ampdu.h"
#endif
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
#include "fe_extern_if.h"
#endif
#include "mac_mib.h"
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
#include "hmac_lp_miracast.h"
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WLAN_CHIP_1106_C

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
void hwifi_cfg_host_global_switch_init_mp16(void)
{
#ifndef _PRE_LINUX_TEST
    /*************************** 低功耗定制化 *****************************/
    /* 由于device 低功耗开关不是true false,而host是,先取定制化的值赋给device开关,再根据此值给host低功耗开关赋值 */
    g_wlan_device_pm_switch = g_cust_cap.wlan_device_pm_switch;
    g_wlan_ps_mode = g_cust_host.wlan_ps_mode;
    g_wlan_fast_ps_dyn_ctl = ((MAX_FAST_PS == g_wlan_ps_mode) ? 1 : 0);
    g_wlan_min_fast_ps_idle = g_cust_cap.fast_ps.wlan_min_fast_ps_idle;
    g_wlan_max_fast_ps_idle = g_cust_cap.fast_ps.wlan_max_fast_ps_idle;
    g_wlan_auto_ps_screen_on = g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_on;
    g_wlan_auto_ps_screen_off = g_cust_cap.fast_ps.wlan_auto_ps_thresh_screen_off;

    g_feature_switch[HMAC_MIRACAST_SINK_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_SINK_SWITCH] == 1);
    g_feature_switch[HMAC_MIRACAST_REDUCE_LOG_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_MIRACAST_REDUCE_LOG_SWITCH] == 1);
    g_feature_switch[HMAC_CORE_BIND_SWITCH] =
        (g_cust_host.en_hmac_feature_switch[CUST_CORE_BIND_SWITCH] == 1);
#endif
}

void hwifi_cfg_host_global_init_sounding_mp16(void)
{
    g_pst_mac_device_capability[0].en_rx_stbc_is_supp = g_cust_cap.en_rx_stbc_is_supp;
    g_pst_mac_device_capability[0].en_tx_stbc_is_supp = g_cust_cap.en_tx_stbc_is_supp;
    g_pst_mac_device_capability[0].en_su_bfmer_is_supp = g_cust_cap.en_su_bfmer_is_supp;
    g_pst_mac_device_capability[0].en_su_bfmee_is_supp = g_cust_cap.en_su_bfmee_is_supp;
    g_pst_mac_device_capability[0].en_mu_bfmer_is_supp = g_cust_cap.en_mu_bfmer_is_supp;
    g_pst_mac_device_capability[0].en_mu_bfmee_is_supp = g_cust_cap.en_mu_bfmee_is_supp;
}

void hwifi_cfg_host_global_init_param_extend_mp16(void)
{
    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_LOCK_CPU_FREQ)) {
        g_freq_lock_control.uc_lock_max_cpu_freq = g_cust_host.lock_max_cpu_freq;
    }

#ifdef _PRE_WLAN_FEATURE_HIEX
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_HIEX_CAP)) {
        if (memcpy_s(&g_st_default_hiex_cap, sizeof(g_st_default_hiex_cap), &g_cust_cap.hiex_cap,
            sizeof(g_cust_cap.hiex_cap)) != EOK) {
            oal_io_print("hwifi_cfg_host_global_init_param_extend:hiex cap memcpy_s fail!");
        }
    }

    oam_warning_log1(0, 0, "host_global_init::hiex cap[0x%X]!", *(uint32_t *)&g_st_default_hiex_cap);
#endif
    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_OPTIMIZED_FEATURE_SWITCH)) {
        g_optimized_feature_switch_bitmap = g_cust_cap.optimized_feature_mask;
    }

    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_TRX_SWITCH)) {
        hmac_set_trx_switch(g_cust_cap.trx_switch);
    }
    oam_warning_log1(0, 0, "host_global_init::trx 0x%x!", g_cust_cap.trx_switch);
#ifdef _PRE_WLAN_CHBA_MGMT
    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_CHBA_INIT_SOCIAL_CHAN)) {
        g_chba_init_social_channel = g_cust_cap.chba_init_social_channel;
    }

    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_CHBA_SOCIAL_FOLLOW_WORK)) {
        g_chba_social_chan_cap = g_cust_cap.chba_social_follow_work_disable;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_FTM)) {
        g_mac_ftm_cap = g_cust_cap.ftm_cap;
    }
#endif
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_ALG_SWITCH)) {
        g_mac_vendor_vht_switch =  g_cust_cap.alg_switch_ini & BIT1 ? OAL_TRUE : OAL_FALSE;
    }

    if (hwifi_get_cust_read_status(CUS_TAG_HOST, WLAN_CFG_HOST_EXTPAND_FEATURE_SWITCH_BITMAP)) {
        g_expand_feature_switch_bitmap = g_cust_cap.expand_feature_switch_bitmap;
    }
}

void hwifi_cfg_host_global_init_mcm_mp16(void)
{
    uint8_t chan_radio_cap, chan_num_2g, chan_num_5g, host_rx, host_ring_tx;

    chan_radio_cap = g_cust_cap.chn_radio_cap;
    /* 获取通道数目,刷新nss信息到mac_device */
    chan_num_2g = chan_radio_cap & 0x0F;
    chan_num_5g = chan_radio_cap & 0xF0;

    g_pst_mac_device_capability[0].en_nss_num =
        oal_max(oal_bit_get_num_one_byte(chan_num_2g), oal_bit_get_num_one_byte(chan_num_5g)) - 1;

    g_mcm_mask_custom = g_cust_cap.mcm_custom_func_mask;

    /* g_dyn_pcie_switch 定制化条件
       1.功能定制化开启dyn_pcie_switch；
       2.radio_cap定制化大于2天线板级方案；
       3.trx ring支持host */
    host_rx = g_cust_cap.trx_switch & BIT0;
    host_ring_tx = (g_cust_cap.trx_switch & BIT1) >> NUM_1_BITS;
    g_dyn_pcie_switch = ((uint8_t)((g_pst_mac_device_capability[0].en_nss_num) >= WLAN_DOUBLE_NSS) &
        host_rx & host_ring_tx) == 0 ? 0 : g_cust_cap.dyn_pcie_switch;
}

/* 将定制化数据配置到host业务使用的全局变量 */
void hwifi_cfg_host_global_init_param_mp16(void)
{
    uint8_t device_idx;

    hwifi_cfg_host_global_switch_init_mp16();
    /*************************** 私有定制化 *******************************/
    for (device_idx = 0; device_idx < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; device_idx++) {
        /* 定制化 RADIO_0 bit4 给dbdc软件开关用 */
        g_wlan_priv_dbdc_radio_cap = (uint8_t)((g_cust_cap.radio_cap[device_idx] & 0x10) >> 4);
        g_wlan_service_device_per_chip[device_idx] = g_cust_cap.radio_cap[device_idx] & 0x0F;
    }
    /* 同步host侧业务device */
    if (memcpy_s(g_auc_mac_device_radio_cap, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP,
        g_wlan_service_device_per_chip, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP) != EOK) {
        oal_io_print("hwifi_cfg_host_global_init_param_mp16:device_radio cap memcpy_s fail!");
    }

    g_pst_mac_device_capability[0].en_channel_width = g_cust_cap.en_channel_width;
    g_pst_mac_device_capability[0].en_ldpc_is_supp = g_cust_cap.en_ldpc_is_supp;

    hwifi_cfg_host_global_init_sounding_mp16();

    if (memcpy_s(&g_pst_mac_device_capability[0].hisi_priv_cap, sizeof(mac_hisi_cap_vendor_ie_stru),
        &g_cust_cap.hisi_priv_cap, sizeof(uint32_t)) != EOK) {
        oal_io_print("hwifi_cfg_host_global_init_param_mp16:hisi_priv cap memcpy_s fail!");
    }

    g_pst_mac_device_capability[0].en_1024qam_is_supp = !!(g_cust_cap.hisi_priv_cap & BIT4);

    hwifi_cfg_host_global_init_mcm_mp16();

    hwifi_cfg_host_global_init_param_extend_mp16();
}

oal_bool_enum_uint8 wlan_first_powon_mark_mp16(void)
{
    oal_bool_enum_uint8 cali_first_pwr_on = OAL_TRUE;

    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_CALI_DATA_MASK)) {
        cali_first_pwr_on = !!(CALI_FIST_POWER_ON_MASK & g_cust_cap.cali_data_mask);
        oal_io_print("wlan_first_powon_mark_mp16:cali_first_pwr_on [%d]\r\n",
                     cali_first_pwr_on);
    }
    return cali_first_pwr_on;
}

oal_bool_enum_uint8 wlan_chip_is_aput_support_160m_mp16(void)
{
    if (hwifi_get_cust_read_status(CUS_TAG_CAP, WLAN_CFG_CAP_APUT_160M_ENABLE)) {
        return !!g_cust_cap.aput_160m_switch;
    }
    return OAL_FALSE;
}

void wlan_chip_get_flow_ctrl_used_mem_mp16(struct wlan_flow_ctrl_params *flow_ctrl)
{
    flow_ctrl->start = g_cust_cap.used_mem_for_start;
    flow_ctrl->stop  = g_cust_cap.used_mem_for_stop;
}
#ifdef _PRE_WLAN_FE_NEW_ARCHITECTURE
typedef uint32_t (*fe_custom_update_region_pow_cb)(regdomain_enum en_regdomain);
uint32_t fe_custom_update_region_pow(void)
{
    fe_custom_update_region_pow_cb fe_custom_update_region_pow = NULL;
    regdomain_enum en_regdomain;
    fe_custom_update_region_pow = (fe_custom_update_region_pow_cb)fe_get_out_if_cb(FE_CUSTOM_UPDATE_POW_REGION);
    if (fe_custom_update_region_pow == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "fe_custom_set_mem_addr: fe_get_out_if_cb failed!");
        BUG_ON(1);
        return OAL_FAIL;
    }
    en_regdomain = hwifi_get_regdomain_from_country_code((uint8_t *)g_wifi_country_code);
    return fe_custom_update_region_pow(en_regdomain);
}
uint32_t hwifi_force_update_rf_params_mp16(void)
{
    return fe_custom_update_region_pow();
}
#else
uint32_t hwifi_force_update_rf_params_mp16(void)
{
    return hwifi_config_init_mp16(CUS_TAG_POW);
}
#endif
uint8_t wlan_chip_get_selfstudy_country_flag_mp16(void)
{
    return g_cust_cap.country_self_study;
}

uint32_t wlan_chip_get_11ax_switch_mask_mp16(void)
{
    return g_cust_cap.wifi_11ax_switch_mask;
}

oal_bool_enum_uint8 wlan_chip_get_11ac2g_enable_mp16(void)
{
    return g_cust_host.wlan_11ac2g_switch;
}
uint32_t wlan_chip_get_probe_resp_mode_mp16(void)
{
    return g_cust_host.wlan_probe_resp_mode;
}
uint32_t wlan_chip_get_trx_switch_mp16(void)
{
    return g_cust_cap.trx_switch;
}
uint8_t wlan_chip_get_d2h_access_ddr_mp16(void)
{
    return g_cust_cap.device_access_ddr;
}
#endif // _PRE_PLAT_FEATURE_CUSTOMIZE
uint8_t wlan_chip_get_chn_radio_cap_mp16(void)
{
    return g_cust_cap.chn_radio_cap;
}

uint8_t wlan_chip_get_special_dbdc_cap_mp16(void)
{
    return g_cust_cap.dbdc_cap;
}

uint32_t wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp16(void)
{
    return g_cust_host.reduce_pwr_2g_40mhz_channel_bitmap;
}

void wlan_chip_get_rf_band_from_center_freq_mp16(uint16_t center_freq, uint8_t *rf_band_sel)
{
    uint8_t subband_idx;
    wlan_cali_div_band_stru rf_band_range[WLAN_2G_RF_BAND_NUM + WLAN_5G_RF_BAND_NUM] = {
        { 2412, 2432 }, { 2437, 2457 },
        { 2462, 2484 }, { 4920, 4980 },
        { 5160, 5240 }, { 5245, 5320 },
        { 5485, 5575 }, { 5580, 5655 },
        { 5660, 5740 }, { 5745, 5840 },
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        { 5845, 5950 }, { 5955, 6110 },
        { 6115, 6270 }, { 6275, 6430 },
        { 6435, 6590 }, { 6595, 6750 },
        { 6755, 6910 }, { 6915, 7115 },
#endif
    };
    if (center_freq == 0) {
        *rf_band_sel = 0x1;
        return;
    }
    for (subband_idx = 0; subband_idx < WLAN_2G_RF_BAND_NUM + WLAN_5G_RF_BAND_NUM; subband_idx++) {
        if (oal_value_in_valid_range(center_freq, rf_band_range[subband_idx].center_freq_start,
            rf_band_range[subband_idx].center_freq_end)) {
            break;
        }
    }

    if (subband_idx == WLAN_2G_RF_BAND_NUM + WLAN_5G_RF_BAND_NUM) {
        /* 当配置异常时，本地配置为band1 */
        *rf_band_sel = 0x1;
    } else if (subband_idx >= WLAN_2G_RF_BAND_NUM) {
        *rf_band_sel = subband_idx - WLAN_2G_RF_BAND_NUM;
    } else {
        *rf_band_sel = subband_idx;
    }
}

uint16_t wlan_chip_get_center_freq_from_chn_mp16(const mac_channel_stru *channel_info)
{
    uint16_t center_freq;
    uint8_t center_freq_chnnum = wlan_get_center_freq_idx(channel_info);

    if (channel_info->en_band == WLAN_BAND_2G) {
        if (center_freq_chnnum == 14) {            /* JP chn 14 */
            center_freq = 2484;                   /* freq is 2484 */
        } else {
            center_freq = 2407 + 5 * center_freq_chnnum;   /* 2G center freq = 2407 + 5 * (channel number) */
        }
    } else {
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        if (channel_info->ext6g_band) {
            center_freq = 5950 + 5 * center_freq_chnnum; /* center frequency = 5950 + 5 × nch (MHz) */
        } else {
#endif
            if (center_freq_chnnum <= 178) {    /* 日本支持182-196信道 频率在4915-4980Mhz 使用178信道作为分界 */
                center_freq = 5000 + 5 * center_freq_chnnum;    /* 5G center freq = 5000 + 5 * (channel number) */
            } else {
                center_freq = 4000 + 5 * center_freq_chnnum;    /* JP chn freq = 4000 + 5 * (channel number) */
            }
#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
        }
#endif
    }

    return center_freq;
}

const uint32_t g_cmd_need_filter_mp16[] = {
    WLAN_CFGID_ETH_SWITCH,
    WLAN_CFGID_80211_UCAST_SWITCH,
    WLAN_CFGID_80211_MCAST_SWITCH,
    WLAN_CFGID_PROBE_SWITCH,
    WLAN_CFGID_OTA_BEACON_SWITCH,
    WLAN_CFGID_OTA_RX_DSCR_SWITCH,
    WLAN_CFGID_VAP_STAT,
    WLAN_CFGID_REPORT_VAP_INFO,
    WLAN_CFGID_USER_INFO,
    WLAN_CFGID_ALL_STAT,
    WLAN_CFGID_BSS_TYPE,
    WLAN_CFGID_GET_HIPKT_STAT,
    WLAN_CFGID_PROT_MODE,
    WLAN_CFGID_BEACON_INTERVAL,
    WLAN_CFGID_NO_BEACON,
    WLAN_CFGID_DTIM_PERIOD,
    WLAN_CFGID_SET_POWER_TEST,
    WLAN_CFGID_GET_ANT,
    WLAN_CFGID_QUERY_RSSI,
    WLAN_CFGID_QUERY_PSST,
    WLAN_CFGID_QUERY_RATE,
    WLAN_CFGID_QUERY_ANI,
    WLAN_CFGID_SET_P2P_MIRACAST_STATUS,
    WLAN_CFGID_SET_CUS_DYN_CALI_PARAM,
    WLAN_CFGID_SET_ALL_LOG_LEVEL,
    WLAN_CFGID_SET_CUS_RF,
    WLAN_CFGID_SET_CUS_DTS_CALI,
    WLAN_CFGID_SET_CUS_NVRAM_PARAM,
    WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS,
    WLAN_CFGID_SEND_BAR,
    WLAN_CFGID_RESET_HW,
    WLAN_CFGID_GET_USER_RSSBW,
    WLAN_CFGID_ENABLE_ARP_OFFLOAD,
    WLAN_CFGID_SHOW_ARPOFFLOAD_INFO,
    WLAN_CFGID_SET_AUTO_PROTECTION,
    WLAN_CFGID_SET_BW_FIXED,
    WLAN_CFGID_SET_FEATURE_LOG,
    WLAN_CFGID_WMM_SWITCH,
    WLAN_CFGID_NARROW_BW,
};

oal_bool_enum_uint8 wlan_chip_h2d_cmd_need_filter_mp16(uint32_t cmd_id)
{
    uint32_t idx;

    for (idx = 0; idx < oal_array_size(g_cmd_need_filter_mp16); idx++) {
        if (cmd_id == g_cmd_need_filter_mp16[idx]) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE; /* 不用过滤 */
}

uint32_t wlan_chip_update_cfg80211_mgmt_tx_wait_time_mp16(uint32_t wait_time)
{
    if (!mpxx_is_asic() && wait_time <= 100) { // 100 mp16 FPGA 切换信道时间较长，对于等待时间小于100ms 发帧，修改为150ms
        wait_time = 150;    // 小于100ms 监听时间，增加为150ms，增加发送时间。避免管理帧未来得及发送，超时定时器就结束。
    }
    return wait_time;
}

oal_bool_enum_uint8 wlan_chip_check_need_setup_ba_session_mp16(void)
{
    /* mp16 device tx由于硬件约束不允许建立聚合 */
    return hmac_ring_tx_enabled();
}

oal_bool_enum_uint8 wlan_chip_check_need_process_bar_mp16(void)
{
    /* mp16 硬件处理bar，软件不需要处理 */
    return OAL_FALSE;
}

oal_bool_enum_uint8 wlan_chip_ba_need_check_lut_idx_mp16(void)
{
    /* mp16无lut限制，不用检查LUT idx */
    return OAL_FALSE;
}

void wlan_chip_mac_mib_set_auth_rsp_time_out_mp16(mac_vap_stru *mac_vap)
{
    if (mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_AP_TIMEOUT);
    } else {
        if (!mpxx_is_asic()) {
            /* mp16 FPGA阶段 入网auth时间加长为mp15一倍 */
            mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_TIMEOUT * 2); /* 入网时间增加2倍 */
        } else {
            mac_mib_set_AuthenticationResponseTimeOut(mac_vap, WLAN_AUTH_TIMEOUT);
        }
    }
}

void wlan_chip_proc_query_station_packets_mp16(hmac_vap_stru *hmac_vap,
    dmac_query_station_info_response_event *response_event)
{
    hmac_vap_stat_stru *hmac_vap_stat = hmac_stat_get_vap_stat(hmac_vap);
    /* mp16 tx/rx帧统计从host获取 */
    hmac_vap->station_info.rx_packets = hmac_vap_stat->rx_packets;
    hmac_vap->station_info.rx_bytes = hmac_vap_stat->rx_bytes;
    hmac_vap->station_info.rx_dropped_misc = response_event->rx_dropped_misc + hmac_vap_stat->rx_dropped_misc;

    hmac_vap->station_info.tx_packets = hmac_vap_stat->tx_packets;
    hmac_vap->station_info.tx_bytes = hmac_vap_stat->tx_bytes;
    hmac_vap->station_info.tx_retries = response_event->tx_retries + hmac_vap_stat->tx_retries;
    hmac_vap->station_info.tx_failed = response_event->tx_failed + hmac_vap_stat->tx_failed;
}

uint32_t wlan_chip_scan_req_alloc_and_fill_netbuf_mp16(frw_event_mem_stru *event_mem, hmac_vap_stru *hmac_vap,
    oal_netbuf_stru **netbuf_scan_req, const void *params)
{
    frw_event_stru *event = NULL;
    dmac_tx_event_stru *scan_req_event = NULL;
    mac_scan_req_h2d_stru *scan_req_h2d = (mac_scan_req_h2d_stru *)params;
    mac_scan_req_ex_stru *scan_req_ex = NULL;

    /* 申请netbuf内存  */
    (*netbuf_scan_req) = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF,
        (sizeof(mac_scan_req_ex_stru)), OAL_NETBUF_PRIORITY_MID);
    if ((*netbuf_scan_req) == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{wlan_chip_scan_req_alloc_and_fill_netbuf_mp16::netbuf_scan_req alloc failed.}");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }
    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCAN_REQ,
        sizeof(dmac_tx_event_stru), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /***************************** copy data **************************/
    memset_s(oal_netbuf_cb(*netbuf_scan_req), OAL_TX_CB_LEN, 0, OAL_TX_CB_LEN);
    scan_req_ex = (mac_scan_req_ex_stru *)(oal_netbuf_data(*netbuf_scan_req));
    /* 拷贝扫描请求参数到netbuf data区域 */
    scan_req_ex->scan_flag = scan_req_h2d->scan_flag;
    hmac_scan_param_convert_ex(scan_req_ex, &(scan_req_h2d->st_scan_params));

    /* 拷贝netbuf 到事件数据区域 */
    scan_req_event = (dmac_tx_event_stru *)event->auc_event_data;
    scan_req_event->pst_netbuf = (*netbuf_scan_req);
    scan_req_event->us_frame_len = sizeof(mac_scan_req_ex_stru);
    scan_req_event->us_remain = 0;
    return OAL_SUCC;
}

oal_bool_enum wlan_chip_is_dbdc_ini_en_mp16(void)
{
    if ((g_cust_cap.radio_cap[0] & BIT4) == 0) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

oal_bool_enum wlan_chip_is_single_dbdc_ini_en_mp16(void)
{
    if ((g_cust_cap.radio_cap[0] & BIT5) == 0) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

uint32_t wlan_chip_send_connect_security_params_mp16(mac_vap_stru *mac_vap,
    mac_conn_security_stru *connect_security_params)
{
    return hmac_config_send_event(mac_vap, WLAN_CFGID_CONNECT_REQ,
        sizeof(*connect_security_params), (uint8_t *)connect_security_params);
}

/*
 * 检查加密的PN号是否异常。
 * 加密场景下，需要后一个分片帧PN严格比前一个分片帧PN+1，否则丢弃分片帧
 * 参考资料:80211-2016 12.5.3.3.2 PN processing
 * 返回值：OAL_TRUE:PN异常；OAL_FALSE:PN正常
 */
oal_bool_enum_uint8 wlan_chip_defrag_is_pn_abnormal_mp16(hmac_user_stru *hmac_user,
    oal_netbuf_stru *netbuf, oal_netbuf_stru *last_netbuf)
{
    uint64_t                last_pn_val;
    uint64_t                rx_pn_val;
    mac_rx_ctl_stru        *rx_ctl = NULL;
    mac_rx_ctl_stru        *last_rx_ctl = NULL;

    wlan_ciper_protocol_type_enum_uint8 cipher_type = hmac_user->st_user_base_info.st_key_info.en_cipher_type;

    if (cipher_type != WLAN_80211_CIPHER_SUITE_CCMP && cipher_type != WLAN_80211_CIPHER_SUITE_GCMP &&
        cipher_type != WLAN_80211_CIPHER_SUITE_CCMP_256 && cipher_type != WLAN_80211_CIPHER_SUITE_GCMP_256) {
        return OAL_FALSE;
    }

    rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    last_rx_ctl = (mac_rx_ctl_stru *)oal_netbuf_cb(last_netbuf);

    rx_pn_val = ((uint64_t)rx_ctl->us_rx_msb_pn << BIT_OFFSET_32) | (rx_ctl->rx_lsb_pn);
    last_pn_val = ((uint64_t)last_rx_ctl->us_rx_msb_pn << BIT_OFFSET_32) | (last_rx_ctl->rx_lsb_pn);
    if (rx_pn_val != (last_pn_val + 1)) {
        oam_warning_log3(0, OAM_SF_ANY,
            "{wlan_chip_defrag_is_pn_abnormal::cipher_type[%u], rx_pn_val[%u], last_pn_val[%u].}",
            cipher_type, rx_pn_val, last_pn_val);
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

oal_bool_enum_uint8 wlan_chip_is_support_hw_wapi_mp16(void)
{
    return g_wlan_spec_cfg->feature_hw_wapi;
}

#ifdef _PRE_WLAN_CHBA_MGMT
/* mp16支持chba dbdc+dbac共存 */
oal_bool_enum_uint8 wlan_chip_is_support_chba_dbdc_dbac_mp16(void)
{
    return OAL_FALSE;
}
#endif

mac_scanned_result_extend_info_stru *wlan_chip_get_scaned_result_extend_info_mp16(oal_netbuf_stru *netbuf,
    uint16_t frame_len)
{
    return (mac_scanned_result_extend_info_stru *)(oal_netbuf_data(netbuf) -
        sizeof(mac_scanned_result_extend_info_stru));
}

uint16_t wlan_chip_get_scaned_payload_extend_len_mp16(void)
{
    return 0;
}

static inline uint8_t wlan_chip_soft_irq_sch_support_mp16(void)
{
    return OAL_TRUE;
}
/* 功能：判断当前可否使用GEN2：以GEN2可支持5000M最大物理速率为基准
   仅有HE 160M 4nss 4KQAM的最大物理速率超过GEN2限制需要GEN3 */
static uint8_t wlan_chip_get_pcie_probe_state_mp16(mac_user_stru *mac_user)
{
    wlan_nss_enum_uint8 avil_nss = mac_user->en_avail_num_spatial_stream;
    wlan_protocol_enum_uint8 cur_protocol = mac_user->en_cur_protocol_mode;
    wlan_bw_cap_enum_uint8 bw = mac_user->en_avail_bandwidth;

    if (cur_protocol < WLAN_HE_MODE || bw < WLAN_BW_CAP_160M || avil_nss < WLAN_FOUR_NSS) {
        return WLAN_PCIE_PROBE_LOW;
    }

    if (g_pst_mac_device_capability[0].hisi_priv_cap.bit_4096qam_cap == 0 ||
        mac_user->hisi_priv_cap.bit_4096qam_cap == 0) {
        return WLAN_PCIE_PROBE_LOW;
    }

    return WLAN_PCIE_PROBE_DEFAULT;
}

static void wlan_chip_custom_default_init_mp16(void)
{
    g_cust_cap.phy_cap_mask = 0x3E;
}

static oal_bool_enum_uint8 wlan_chip_is_support_dual_5g_dbdc_by_radio_cap_mp16(void)
{
    /* 0xc3: mp16 2 + 2 天线方案不支持5g 高低band dbdc */
    return (g_cust_cap.chn_radio_cap == 0xC3) ? OAL_FALSE : OAL_TRUE;
}
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST

void wlan_chip_lp_miracast_stat_mbps_mp16(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    hmac_config_lp_miracast_stat_mbps(tx_throughput_mbps, rx_throughput_mbps);
}

static uint32_t wlan_chip_set_lp_miracast_mp16(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_lp_miracast(mac_vap, len, param);
}
#endif
const struct wlan_chip_ops g_wlan_chip_ops_mp16 = {
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    .host_global_init_param = hwifi_cfg_host_global_init_param_mp16,
    .first_power_on_mark = wlan_first_powon_mark_mp16,
    .first_powon_cali_completed = hmac_first_powon_cali_completed,
    .is_aput_support_160M = wlan_chip_is_aput_support_160m_mp16,
    .get_flow_ctrl_used_mem = wlan_chip_get_flow_ctrl_used_mem_mp16,
    .force_update_custom_params = hwifi_force_update_rf_params_mp16,
    .init_nvram_main = hwifi_config_init_fcc_ce_params_mp16,
    .cpu_freq_ini_param_init = hwifi_config_cpu_freq_ini_param_mp16,
    .host_global_ini_param_init = hwifi_config_host_global_ini_param_mp16,
    .get_selfstudy_country_flag = wlan_chip_get_selfstudy_country_flag_mp16,
    .custom_cali = wal_custom_cali_mp16,
    .custom_cali_data_host_addr_init = hwifi_rf_cali_data_host_addr_init_mp16,
#ifdef HISI_CONN_NVE_SUPPORT
    .get_nv_dpd_data = wal_get_nv_dpd_data_mp16,
#endif
    .send_cali_data = wal_send_cali_data_mp16,
    .send_20m_all_chn_cali_data = wal_send_cali_data_mp16,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_mp16,
    .hcc_customize_h2d_data_cfg = hwifi_hcc_customize_h2d_data_cfg_mp16,
    .show_customize_info = hwifi_show_customize_info_mp16,
    .show_cali_data_info = hmac_dump_cali_result_mp16,
    .get_sar_ctrl_params = hwifi_get_sar_ctrl_params_mp16,
    .get_11ax_switch_mask = wlan_chip_get_11ax_switch_mask_mp16,
    .get_11ac2g_enable = wlan_chip_get_11ac2g_enable_mp16,
    .get_probe_resp_mode = wlan_chip_get_probe_resp_mode_mp16,
    .get_trx_switch = wlan_chip_get_trx_switch_mp16,
    .get_d2h_access_ddr = wlan_chip_get_d2h_access_ddr_mp16,
#endif
    .h2d_cmd_need_filter = wlan_chip_h2d_cmd_need_filter_mp16,
    .update_cfg80211_mgmt_tx_wait_time = wlan_chip_update_cfg80211_mgmt_tx_wait_time_mp16,
    // 收发和聚合相关
    .ba_rx_hdl_init = hmac_ba_rx_win_init,
    .check_need_setup_ba_session = wlan_chip_check_need_setup_ba_session_mp16,
    .tx_update_amsdu_num = hmac_update_amsdu_num_mp16,
    .check_need_process_bar = wlan_chip_check_need_process_bar_mp16,
    .ba_send_reorder_timeout = hmac_ba_send_ring_reorder_timeout,
    .ba_need_check_lut_idx = wlan_chip_ba_need_check_lut_idx_mp16,
    .trx_aspm_switch_on = NULL,
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_mp16,
#endif
    .update_arp_tid = hmac_update_arp_tid_mp16,
    .get_6g_flag = mac_get_rx_6g_flag_mp16,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_mp16,
    .save_cali_event = hmac_save_cali_event_mp16,
    .update_cur_chn_cali_data = hmac_update_cur_chn_cali_data_mp16,
    .get_chn_radio_cap = wlan_chip_get_chn_radio_cap_mp16,
    // 特殊dbdc相关
    .get_special_dbdc_cap = wlan_chip_get_special_dbdc_cap_mp16,
    .get_rf_band_from_center_freq = wlan_chip_get_rf_band_from_center_freq_mp16,
    .get_center_freq_from_chn = wlan_chip_get_center_freq_from_chn_mp16,
    .is_support_dual_5g_dbdc_by_radio_cap = wlan_chip_is_support_dual_5g_dbdc_by_radio_cap_mp16,
#ifdef _PRE_WLAN_FEATURE_11AX
    .mac_vap_init_mib_11ax = mac_vap_init_mib_11ax_mp16,
    .tx_set_frame_htc = hmac_tx_set_qos_frame_htc,
#endif
    .mac_mib_set_auth_rsp_time_out = wlan_chip_mac_mib_set_auth_rsp_time_out_mp16,
    .mac_vap_need_set_user_htc_cap = mac_vap_need_set_user_htc_cap_mp16,
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    .tx_encap_large_skb_amsdu = hmac_tx_encap_large_skb_amsdu_mp16, /* 大包AMDPU+大包AMSDU入口 mp16 不生效 */
#endif
    .check_headroom_len = check_headroom_length,
    .adjust_netbuf_data = hmac_format_netbuf_header,
    .proc_query_station_packets = wlan_chip_proc_query_station_packets_mp16,
    .scan_req_alloc_and_fill_netbuf = wlan_chip_scan_req_alloc_and_fill_netbuf_mp16,
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    .set_sniffer_config = hmac_config_set_sniffer_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    .dcsi_config = hmac_device_csi_config,
    .hcsi_config = hmac_host_csi_config,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .dftm_config = hmac_device_ftm_config,
    .hftm_config = hmac_host_ftm_config,
    .rrm_proc_rm_request = hmac_rrm_proc_rm_request_mp16,
    .dconfig_wifi_rtt_config = hmac_device_wifi_rtt_config,
    .hconfig_wifi_rtt_config = hmac_host_wifi_rtt_config,
    .ftm_vap_init = hmac_ftm_vap_init_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
    .start_zero_wait_dfs = hmac_config_start_zero_wait_dfs_handle,
#endif
    .update_rxctl_data_type = hmac_rx_netbuf_update_rxctl_data_type_mp16,
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU
    .mcast_ampdu_rx_ba_init = hmac_mcast_ampdu_rx_win_init,
    .mcast_stats_stru_init = hmac_mcast_ampdu_stats_stru_init,
#endif
#ifdef _PRE_WLAN_FEATURE_MCAST_AMPDU_HW
    .mcast_ampdu_sta_add_multi_user = NULL,
#endif
    .is_dbdc_ini_en = wlan_chip_is_dbdc_ini_en_mp16,
    .is_single_dbdc_ini_en = wlan_chip_is_single_dbdc_ini_en_mp16,
#if defined(CONFIG_ARCH_HISI) &&  defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    .frw_task_bind_cpu = hmac_rx_frw_task_bind_cpu,
#endif
#endif

    .send_connect_security_params = wlan_chip_send_connect_security_params_mp16,
    .defrag_is_pn_abnormal = wlan_chip_defrag_is_pn_abnormal_mp16,
    .is_support_hw_wapi = wlan_chip_is_support_hw_wapi_mp16,
    .get_scaned_result_extend_info = wlan_chip_get_scaned_result_extend_info_mp16,
    .get_scaned_payload_extend_len = wlan_chip_get_scaned_payload_extend_len_mp16,
    .tx_pt_mcast_set_cb = NULL,
    .soft_irq_sch_support = wlan_chip_soft_irq_sch_support_mp16,
    .get_pcie_probe_state = wlan_chip_get_pcie_probe_state_mp16,
    .custom_default_init = wlan_chip_custom_default_init_mp16,
#ifdef _PRE_WLAN_CHBA_MGMT
    .is_support_chba_dbdc_dbac = wlan_chip_is_support_chba_dbdc_dbac_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    .lp_miracast_stat_func = wlan_chip_lp_miracast_stat_mbps_mp16,
    .set_lp_miracast_func = wlan_chip_set_lp_miracast_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    .need_decrease_sts = NULL,
#endif
    .ap_p2p_device_coex_check_valid = NULL,
    .get_reduce_pwr_channel_2g_bitmap = wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp16,
};

const oal_wlan_cfg_stru g_wlan_spec_cfg_mp16 = {
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
    .feature_ht_self_cure_is_open = OAL_FALSE,
    .feature_slave_ax_is_support = OAL_TRUE,
    .longer_than_16_he_sigb_support = OAL_TRUE,
    .feature_hw_wapi = OAL_FALSE,
    .full_bandwidth_ul_mu_mimo = OAL_TRUE,
    .p2p_device_gc_use_one_vap = OAL_FALSE,
    .max_sta_num = 4,
    .max_p2p_group_num = 2,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16,  /* mp16 P2P最大接入用户数为8 */
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_user_limit = MAC_RES_MAX_USER_LIMIT,
    .invalid_user_id = MAC_RES_MAX_USER_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_MP16,
    .max_rx_ba = WLAN_MAX_RX_BA_MP16,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_MP16,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_MP16,
    .max_rf_num = 4,
    .feature_txq_ns_support = OAL_FALSE,
    .feature_txbf_mib_study = OAL_TRUE,
    .bfee_support_ants = 8,        /* mp16 最多支持 8*4 MU Sounding */
    .bfer_sounding_dimensions = 4, /* 发送最多支持4流 */
    .vht_bfee_ntx_supports = 8,    /* mp16 最多支持 8*4 MU Sounding */
    .vht_max_mpdu_lenth = WLAN_MIB_VHT_MPDU_11454, /* VHT协议mp16最大mpdu_len */
    .pcie_speed = 2, /* 默认GEN3 */
    .trigger_su_beamforming_feedback = OAL_TRUE,
    .trigger_mu_partialbw_feedback = OAL_FALSE,
    .rx_stbc_160 = OAL_TRUE,
    .num_different_channels = 3,
    .max_work_vap_num = 3,
    .rx_checksum_hw_enable = OAL_FALSE,
};
