/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : hmac_host_rx.c 的头文件
 * 创建日期 : 2020年09月22日
 */

#ifndef HMAC_HOST_RX_H
#define HMAC_HOST_RX_H

#include "hmac_rx_data.h"
#include "frw_ext_if.h"

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HOST_RX_H

static inline void hmac_rx_exception_free_stat(hal_host_device_stru *hal_dev)
{
    hal_dev->rx_statics.rx_exception_free_cnt++;
}

void hmac_rx_common_proc(hmac_host_rx_context_stru *rx_context, oal_netbuf_head_stru *rpt_list);
uint32_t hmac_host_rx_ring_data_event(frw_event_mem_stru *event_mem);
void hmac_rx_rpt_netbuf(oal_netbuf_head_stru *list);
uint8_t hmac_rx_da_is_in_same_bss(oal_netbuf_stru *netbuf, mac_rx_ctl_stru *rx_ctl, hmac_vap_stru *hmac_vap);
uint32_t hmac_rx_proc_feature(hmac_host_rx_context_stru *rx_context, oal_netbuf_head_stru *rpt_list,
    oal_netbuf_head_stru *w2w_list);
uint32_t hmac_host_build_rx_context(hal_host_device_stru *hal_dev, oal_netbuf_stru *netbuf,
    hmac_host_rx_context_stru *rx_context);
uint32_t hmac_rx_proc_feature(hmac_host_rx_context_stru *rx_context, oal_netbuf_head_stru *rpt_list,
    oal_netbuf_head_stru *w2w_list);
uint32_t hmac_host_rx_mpdu_build_cb(hal_host_device_stru *hal_dev, oal_netbuf_stru *netbuf);
void hmac_rx_route_print(hal_host_device_stru *hal_device, uint8_t step, mac_rx_ctl_stru *mac_rx_ctl);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
