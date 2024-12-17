/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : WIFI 芯片差异接口文件
 * 作    者 :
 * 创建日期 : 2020年6月19日
 */

#include "wlan_chip.h"
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
#include "hmac_scan.h"
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
#include "mac_mib.h"
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
#include "hmac_lp_miracast.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif

oal_bool_enum_uint8 wlan_chip_is_support_hw_wapi_mp16c(void)
{
    mac_device_stru *mac_device = mac_res_get_mac_dev();
    if (mac_device->debug_hw_wapi == OAL_FALSE) {
        /* 调试命令配置关闭WAPI软件加解密，关闭WAPI硬件加解密 */
        return OAL_FALSE;
    }
    return g_wlan_spec_cfg->feature_hw_wapi;
}

static inline uint8_t wlan_chip_soft_irq_sch_support_mp16c(void)
{
    return OAL_TRUE;
}

static uint8_t wlan_chip_get_pcie_probe_state_mp16c(mac_user_stru *mac_user)
{
    wlan_nss_enum_uint8 avil_nss = mac_user->en_avail_num_spatial_stream;
    wlan_protocol_enum_uint8 cur_protocol = mac_user->en_cur_protocol_mode;
    wlan_bw_cap_enum_uint8 bw = mac_user->en_avail_bandwidth;

    /* 低于HE协议所有速率，GEN1可支持 */
    if (cur_protocol < WLAN_HE_MODE || bw < WLAN_BW_CAP_160M) {
        return WLAN_PCIE_PROBE_LOW;
    } else if (avil_nss < WLAN_DOUBLE_NSS) {
        /* HE协议仅有160M 2nss需要GEN2，其余GEN1 */
        return WLAN_PCIE_PROBE_LOW;
    }

    return WLAN_PCIE_PROBE_DEFAULT;
}

static void wlan_chip_custom_default_init_mp16c(void)
{
    g_cust_cap.phy_cap_mask = 0xBF;
}

static oal_bool_enum_uint8 wlan_chip_is_support_dual_5g_dbdc_by_radio_cap_mp16c(void)
{
    return OAL_TRUE;
}
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
void wlan_chip_lp_miracast_stat_mbps_mp16c(uint32_t tx_throughput_mbps, uint32_t rx_throughput_mbps)
{
    hmac_config_lp_miracast_stat_mbps(tx_throughput_mbps, rx_throughput_mbps);
}

static uint32_t wlan_chip_set_lp_miracast_mp16c(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hmac_config_set_lp_miracast(mac_vap, len, param);
}
#endif
const struct wlan_chip_ops g_wlan_chip_ops_mp16c = {
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
    .send_cali_data = wal_send_cali_data_mp16,
    .send_20m_all_chn_cali_data = wal_send_cali_data_mp16,
    .custom_host_read_cfg_init = hwifi_custom_host_read_cfg_init_mp16,
    .trx_aspm_switch_on = NULL,
#ifdef HISI_CONN_NVE_SUPPORT
     .get_nv_dpd_data = wal_get_nv_dpd_data_mp16,
#endif
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
#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    .tcp_ack_buff_config = hmac_tcp_ack_buff_config_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_WMMAC
    .wmmac_need_degrade_for_ts = hmac_need_degrade_for_ts_mp16,
#endif
    .update_arp_tid = hmac_update_arp_tid_mp16,
    .get_6g_flag = mac_get_rx_6g_flag_mp16,
    // 校准相关
    .send_cali_matrix_data = hmac_send_cali_matrix_data_mp16c,
    .save_cali_event = hmac_save_cali_event_mp16,
    .update_cur_chn_cali_data = hmac_update_cur_chn_cali_data_mp16,
    .get_chn_radio_cap = wlan_chip_get_chn_radio_cap_mp16,
    // 特殊dbdc相关
    .get_special_dbdc_cap = wlan_chip_get_special_dbdc_cap_mp16,
    .get_rf_band_from_center_freq = wlan_chip_get_rf_band_from_center_freq_mp16,
    .get_center_freq_from_chn = wlan_chip_get_center_freq_from_chn_mp16,
    .is_support_dual_5g_dbdc_by_radio_cap = wlan_chip_is_support_dual_5g_dbdc_by_radio_cap_mp16c,

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
    .mcast_ampdu_sta_add_multi_user = hmac_mcast_ampdu_sta_add_multi_user,
#endif
    .is_dbdc_ini_en = wlan_chip_is_dbdc_ini_en_mp16,
    .is_single_dbdc_ini_en = NULL,
#if defined(CONFIG_ARCH_HISI) &&  defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    .frw_task_bind_cpu = hmac_rx_frw_task_bind_cpu,
#endif
#endif
    .send_connect_security_params = wlan_chip_send_connect_security_params_mp16,
    .defrag_is_pn_abnormal = wlan_chip_defrag_is_pn_abnormal_mp16,
    .is_support_hw_wapi = wlan_chip_is_support_hw_wapi_mp16c,
    .get_scaned_result_extend_info = wlan_chip_get_scaned_result_extend_info_mp16,
    .get_scaned_payload_extend_len = wlan_chip_get_scaned_payload_extend_len_mp16,

    .tx_pt_mcast_set_cb = NULL,
    .soft_irq_sch_support = wlan_chip_soft_irq_sch_support_mp16c,
    .get_pcie_probe_state = wlan_chip_get_pcie_probe_state_mp16c,
    .custom_default_init = wlan_chip_custom_default_init_mp16c,
#ifdef _PRE_WLAN_CHBA_MGMT
    .is_support_chba_dbdc_dbac = wlan_chip_is_support_chba_dbdc_dbac_mp16,
#endif
#ifdef _PRE_WLAN_FEATURE_LP_MIRACAST
    .lp_miracast_stat_func = wlan_chip_lp_miracast_stat_mbps_mp16c,
    .set_lp_miracast_func = wlan_chip_set_lp_miracast_mp16c,
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    .need_decrease_sts = hmac_decrease_below_80mhz_max_sts_ability,
#endif
    .ap_p2p_device_coex_check_valid = NULL,
    .get_reduce_pwr_channel_2g_bitmap = wlan_chip_get_reduce_pwr_channel_2g_bitmap_mp16,
};

const oal_wlan_cfg_stru g_wlan_spec_cfg_mp16c = {
    .feature_m2s_modem_is_open = OAL_TRUE,
    .feature_priv_closed_is_open = OAL_FALSE,
    .feature_hiex_is_open = OAL_TRUE,
    .rx_listen_ps_is_open = OAL_TRUE,
    .feature_11ax_is_open = OAL_TRUE,
    .feature_twt_is_open = OAL_TRUE,
    .feature_11ax_er_su_dcm_is_open = OAL_TRUE,
    .feature_11ax_uora_is_supported = OAL_TRUE,
    .feature_ftm_is_open = OAL_TRUE,
    .feature_psm_dfx_ext_is_open = OAL_TRUE,
    .feature_wow_opt_is_open  = OAL_FALSE,
    .feature_dual_wlan_is_supported = OAL_TRUE,
    .feature_ht_self_cure_is_open = OAL_FALSE,
    .feature_slave_ax_is_support = OAL_TRUE,
    .longer_than_16_he_sigb_support = OAL_TRUE,
    .feature_hw_wapi = OAL_TRUE,
    .full_bandwidth_ul_mu_mimo = OAL_TRUE,
    .p2p_device_gc_use_one_vap = OAL_FALSE,
    .max_sta_num = 4,
    .max_p2p_group_num = 2,
    .p2p_go_max_user_num = WLAN_P2P_GO_ASSOC_USER_MAX_NUM_MP16C,  /* MP16C P2P最大接入用户数为8 */
    .max_asoc_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_active_user = WLAN_ASSOC_USER_MAX_NUM,
    .max_user_limit = MAC_RES_MAX_USER_LIMIT,
    .invalid_user_id = MAC_RES_MAX_USER_LIMIT,
    .max_tx_ba = WLAN_MAX_TX_BA_MP16C,
    .max_rx_ba = WLAN_MAX_RX_BA_MP16C,
    .other_bss_id = WLAN_HAL_OHTER_BSS_ID_MP16C,
    .max_tx_ampdu_num = WLAN_AMPDU_TX_MAX_NUM_MP16C,
    .max_rf_num = 2,
    .feature_txq_ns_support = OAL_FALSE,
    .feature_txbf_mib_study = OAL_TRUE,
    .bfee_support_ants = 8,        /* biseng最多支持 8*2 MU Sounding */
    .bfer_sounding_dimensions = 2, /* 发送最多支持双流 */
    .vht_bfee_ntx_supports = 8,    /* biseng最多支持 8*2 MU Sounding */
    .vht_max_mpdu_lenth = WLAN_MIB_VHT_MPDU_11454, /* VHT协议mp16c最大mpdu_len */
    .pcie_speed = 1, /* 默认GEN2 */
    .trigger_su_beamforming_feedback = OAL_TRUE,
    .trigger_mu_partialbw_feedback = OAL_FALSE,
    .rx_stbc_160 = OAL_TRUE,
    .num_different_channels = 3,
    .max_work_vap_num = 3,
    .rx_checksum_hw_enable = OAL_TRUE,
};
