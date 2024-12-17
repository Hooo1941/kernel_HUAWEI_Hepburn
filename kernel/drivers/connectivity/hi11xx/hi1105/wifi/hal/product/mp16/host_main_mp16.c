/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 芯片初始化相关
 * 创建日期 : 2020年6月15日
 */

#include "host_hal_ops.h"
#include "host_hal_ops_mp16.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_HAL_MAIN_MP16_C

const struct hal_common_ops_stru g_hal_common_ops_mp16 = {
    .host_chip_irq_init          = mp16_host_chip_irq_init,
    .host_rx_add_buff            = mp16_host_rx_add_buff,
    .host_get_rx_msdu_status     = mp16_host_get_rx_msdu_status,
    .host_rx_amsdu_list_build    = mp16_host_rx_amsdu_list_build,
    .rx_get_next_sub_msdu        = mp16_rx_get_next_sub_msdu,
    .host_rx_get_msdu_info_dscr  = mp16_host_rx_get_msdu_info_dscr,
    .rx_host_start_dscr_queue    = mp16_rx_host_start_dscr_queue,
    .rx_host_init_dscr_queue     = mp16_rx_host_init_dscr_queue,
    .rx_host_stop_dscr_queue     = mp16_rx_host_stop_dscr_queue,
    .rx_free_res                 = mp16_rx_free_res,
    .tx_ba_info_dscr_get         = mp16_tx_ba_info_dscr_get,
    .tx_msdu_dscr_info_get       = mp16_tx_msdu_dscr_info_get,
    .host_intr_regs_init         = mp16_host_intr_regs_init,
    .host_ba_ring_regs_init      = mp16_host_ba_ring_regs_init,
    .host_ba_ring_depth_get      = mp16_host_ba_ring_depth_get,
    .host_ring_tx_init           = mp16_host_ring_tx_init,
    .host_rx_reset_smac_handler  = mp16_host_rx_reset_smac_handler,
    .rx_alloc_list_free          = mp16_rx_alloc_list_free,
    .host_al_rx_fcs_info         = mp16_host_al_rx_fcs_info,
    .host_get_rx_pckt_info       = mp16_host_get_rx_pckt_info,
    .host_rx_proc_msdu_dscr      = mp16_host_rx_proc_msdu_dscr,
    .host_init_common_timer      = mp16_host_init_common_timer,
    .host_set_mac_common_timer   = mp16_host_set_mac_common_timer,
    .host_get_tx_tid_ring_size   = mp16_host_get_tx_tid_ring_size,
    .host_get_tx_tid_ring_depth  = mp16_host_get_tx_tid_ring_depth,
#ifdef _PRE_WLAN_FEATURE_CSI
    .host_get_csi_info           = mp16_get_csi_info,
    .host_csi_config             = mp16_csi_config,
    .host_ftm_csi_init           = mp16_host_ftm_csi_init,
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    .host_vap_get_hal_vap_id     = mp16_host_vap_get_by_hal_vap_id,
    .host_ftm_reg_init           = mp16_host_ftm_reg_init,
    .host_ftm_get_info           = mp16_host_ftm_get_info,
    .host_ftm_get_divider        = mp16_host_ftm_get_divider,
    .host_ftm_set_sample         = mp16_host_ftm_set_sample,
    .host_ftm_set_enable         = mp16_host_ftm_set_enable,
    .host_ftm_set_m2s_phy        = mp16_host_ftm_set_m2s_phy,
    .host_ftm_set_buf_base_addr  = mp16_host_ftm_set_buf_base_addr,
    .host_ftm_set_buf_size       = mp16_host_ftm_set_buf_size,
    .host_ftm_set_white_list     = mp16_host_ftm_set_white_list,
    .host_ftm_transfer_device_stru_to_host_stru = mp16_host_ftm_transfer_device_stru_to_host_stru,
    .host_ftm_get_rssi_selection = mp16_host_ftm_get_rssi_selection,
#endif
    .host_tx_clear_msdu_padding  = mp16_host_tx_clear_msdu_padding,
#ifdef _PRE_WLAN_FEATURE_SNIFFER
    .host_sniffer_rx_ppdu_free_ring_init = mp16_sniffer_rx_ppdu_free_ring_init,
    .host_sniffer_rx_ppdu_free_ring_deinit = NULL,
    .host_sniffer_rx_info_fill = mp16_sniffer_rx_info_fill,
    .host_sniffer_add_rx_ppdu_dscr = NULL,
#endif
#ifdef _PRE_WLAN_FEATURE_VSP
    .host_vsp_msdu_dscr_info_get = mp16_vsp_msdu_dscr_info_get,
    .host_rx_buff_recycle = mp16_host_rx_buff_recycle,
#endif
    .host_get_tsf_lo = mp16_host_get_tsf_lo,
    .host_get_device_tx_ring_num = mp16_host_get_device_tx_ring_num,
    .host_rx_tcp_udp_checksum = NULL,
    .host_rx_ip_checksum_is_pass  = NULL,
    .host_clear_host_mac_irq_mask = mp16_clear_host_mac_irq_mask,
    .host_recover_host_mac_irq_mask = mp16_recover_host_mac_irq_mask,
    .host_mac_clear_rx_irq       = mp16_host_mac_clear_rx_irq,
    .host_mac_mask_rx_irq        = mp16_host_mac_mask_rx_irq,
    .host_mac_phy_irq_mask = mp16_host_mac_phy_irq_mask,
    .hmac_tx_complete_process = mp16_hmac_tx_complete_process,
};
