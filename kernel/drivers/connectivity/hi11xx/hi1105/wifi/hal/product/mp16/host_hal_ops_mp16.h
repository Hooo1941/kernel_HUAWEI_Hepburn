/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : host hal common ops
 * 作    者 : wifi
 * 创建日期 : 2012年12月18日
 */

#ifndef HOST_HAL_OPS_MP16_H
#define HOST_HAL_OPS_MP16_H

#include "host_hal_device.h"
#include "hal_common.h"
#include "mac_common.h"

#include "host_dscr_mp16.h"
#include "host_ftm_mp16.h"
#include "host_csi_mp16.h"
#include "host_tx_mp16.h"
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#include "host_sniffer_mp16.h"
#endif
extern const struct hal_common_ops_stru g_hal_common_ops_mp16;


int32_t mp16_rx_host_init_dscr_queue(uint8_t hal_dev_id);

uint32_t mp16_host_rx_get_msdu_info_dscr(hal_host_device_stru *hal_dev,
    oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_info, uint8_t first_msdu);

void mp16_host_rx_add_buff(hal_host_device_stru *hal_device, uint8_t en_queue_num);
void mp16_host_rx_amsdu_list_build(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf);
uint8_t mp16_host_get_rx_msdu_status(oal_netbuf_stru *netbuf);
uint32_t mp16_host_rx_proc_msdu_dscr(hal_host_device_stru *hal_dev, oal_netbuf_stru *netbuf);
void mp16_host_chip_irq_init(void);
void mp16_rx_free_res(hal_host_device_stru *hal_device);

void mp16_rx_alloc_list_free(hal_host_device_stru *hal_dev, hal_host_rx_alloc_list_stru *alloc_list);
void mp16_host_al_rx_fcs_info(hmac_vap_stru *hmac_vap);
void mp16_host_get_rx_pckt_info(hmac_vap_stru *hmac_vap,
    dmac_atcmdsrv_atcmd_response_event *rx_pkcg_event_info);
int32_t mp16_host_init_common_timer(hal_mac_common_timer *mac_timer);
void mp16_host_set_mac_common_timer(hal_mac_common_timer *mac_common_timer);
uint32_t  mp16_host_get_tsf_lo(hal_host_device_stru *hal_device, uint8_t hal_vap_id, uint32_t *tsf);
void mp16_clear_host_mac_irq_mask(void);
void mp16_recover_host_mac_irq_mask(void);
void mp16_host_mac_clear_rx_irq(hal_host_device_stru *hal_device);
void mp16_host_mac_mask_rx_irq(hal_host_device_stru *hal_device);
void mp16_host_mac_phy_irq_mask(void);
#endif

