/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : MinTP
 * 作    者 : wifi
 * 创建日期 : 2022年6月9日
 */

#ifndef HMAC_MINTP_H
#define HMAC_MINTP_H

#include "wlan_types.h"

#define ETH_P_D2D 0xA85A

oal_net_dev_tx_enum hmac_bridge_vap_xmit_mintp(oal_netbuf_stru *netbuf, oal_net_device_stru *net_device);

static inline oal_netbuf_stru *hmac_mintp_tx_get_next_netbuf(oal_netbuf_stru *netbuf)
{
    return *((oal_netbuf_stru **)netbuf->head);
}

static inline void hmac_mintp_tx_set_next_netbuf(oal_netbuf_stru *netbuf, oal_netbuf_stru *next)
{
    *((oal_netbuf_stru **)netbuf->head) = next;
}

#endif
