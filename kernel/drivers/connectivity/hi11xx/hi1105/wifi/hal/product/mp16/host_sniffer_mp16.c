/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : HOST SNIFFER FUNCTION
 * 作    者 : wifi3
 * 创建日期 : 2021年3月29日
 */

#ifdef _PRE_WLAN_FEATURE_SNIFFER

#include "oal_util.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#include "host_hal_dscr.h"
#include "host_hal_ring.h"
#include "host_hal_device.h"
#include "hal_common.h"

#include "host_dscr_mp16.h"
#include "host_mac_mp16.h"
#include "host_sniffer_mp16.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HOST_SNIFFER_MP16_C

void mp16_sniffer_rx_info_fill(hal_host_device_stru *hal_device, oal_netbuf_stru *netbuf,
    hal_sniffer_extend_info *sniffer_rx_info, mac_rx_ctl_stru *rx_ctl)
{
    hal_sniffer_extend_info *sniffer_info = NULL;
    uint8_t *cb = oal_netbuf_cb(netbuf);
    sniffer_info = (hal_sniffer_extend_info *)(cb + HAL_DEVICE_MAX_CB_LEN);
    /* mp16 sniffer 仅支持device rx */
    if (oal_atomic_read(&hal_device->rx_mode) != HAL_RAM_RX) {
        return;
    }
    if (!rx_ctl->bit_is_first_buffer) {
        return;
    }
    memcpy_s(sniffer_rx_info, sizeof(hal_sniffer_extend_info), sniffer_info, sizeof(hal_sniffer_extend_info));
    if (hmac_get_rate_kbps(&sniffer_rx_info->per_rate, &sniffer_rx_info->rate_kbps) == OAL_FALSE) {
        oam_warning_log0(0, OAM_SF_ANY, "{sniffer_rx_info_fill::get rate failed.}");
        /* 若获取速率失败, 默认速率为6Mbps（6000kbps） */
        sniffer_rx_info->rate_kbps = 6000;
    }
}
uint32_t mp16_sniffer_rx_ppdu_free_ring_init(hal_host_device_stru *hal_device)
{
    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_SNIFFER */
