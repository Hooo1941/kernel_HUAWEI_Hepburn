/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <linux/skbuff.h>
#include <linux/rcupdate.h>
#include <linux/etherdevice.h>
#include <linux/dma-map-ops.h>
#include <linux/usb/iot/usb_nv.h>
#include <linux/usb/iot/bsp_unet.h>
#include <securec.h>
#include <osl_types.h>
#include <mdrv_pfa.h>
#include <bsp_print.h>
#include <bsp_dra.h>
#include <bsp_ppp_ndis.h>
#include <bsp_wan_eth_stick.h>

#include "ppp_ndis_net.h"
#include "ppp_ndis_address.h"
#include "ppp_ndis_adp.h"

#define THIS_MODU mod_ppp_ndis

#define IP_TYPE_VAL 0
#define ETH_TYPE_VAL 1
/* pkt_type 取值为2给上行使用，下行不使用 */
#define PPP_TYPE_VAL 3

#define PPP_NDIS_MASK 64
unsigned long long g_ppp_ndis_mask = ~0ULL;
struct device g_ppp_ndis_dev;

enum ppp_ndis_link_state {
    LINK_STATE_DOWN,
    LINK_STATE_UP,
};

struct ppp_ndis_ctx g_ppp_ndis_ctx = { 0 };

struct ppp_ndis_ctx *ppp_ndis_get_ctx(void)
{
    return &g_ppp_ndis_ctx;
}

unsigned int bsp_ppp_ndis_get_ready_state(unsigned int dev_type, unsigned int dev_id)
{
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();

    if (dev_type == PPP_NDIS_DEV_UNET) {
        return ctx->net_ctx.usb_acore_state[dev_id];
    } else if (dev_type == PPP_NDIS_DEV_PNET) {
        return ctx->net_ctx.pcie_acore_state[dev_id];
    } else {
        ppp_ndis_err("device type is error\n");
        return 0;
    }
}

static void ppp_ndis_set_ready_state(struct ppp_ndis_ready_info *ready_info)
{
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();

    if (ready_info->field_info.chan_type == PPP_NDIS_DEV_UNET) {
        ctx->net_ctx.usb_acore_state[ready_info->field_info.chan_id] = ready_info->state;
    } else if (ready_info->field_info.chan_type == PPP_NDIS_DEV_PNET) {
        ctx->net_ctx.pcie_acore_state[ready_info->field_info.chan_id] = ready_info->state;
    } else {
        ppp_ndis_err("device type is error\n");
    }
}

static void ppp_ndis_add_ethhdr(struct ppp_ndis_map_item *map_item, struct sk_buff *skb, struct rx_cb_map_s *rx_cb)
{
    struct net_mac_header mac_header = {0};
    struct ppp_ndis_field_info field_info = {0};
    struct ethhdr *eth_hdr = (struct ethhdr *)(uintptr_t)skb_push(skb, ETH_HLEN); /* skb_pull in wan.c */

    field_info.chan_type = map_item->chan_type;
    field_info.chan_id = map_item->chan_id;
    bsp_ppp_ndis_get_mac_header(&field_info, &mac_header);
    dma_unmap_single(&g_ppp_ndis_dev, virt_to_phys(skb->data), ETH_HLEN, DMA_FROM_DEVICE);
    ether_addr_copy(eth_hdr->h_dest, mac_header.host_mac);
    ether_addr_copy(eth_hdr->h_source, mac_header.gw_mac);
    eth_hdr->h_proto = rx_cb->pfa_tft_result.bits.ip_type == 0 ? htons(ETH_P_IP) : htons(ETH_P_IPV6);
    dma_map_single(&g_ppp_ndis_dev, skb->data, ETH_HLEN, DMA_TO_DEVICE);
}

void ppp_ndis_single_down_deliver(struct sk_buff *skb)
{
    struct rx_cb_map_s *rx_cb = NULL;
    struct ndis_field0_desc *desc = NULL;
    struct ppp_ndis_map_item *map_item = NULL;
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();
    int (*deliver_cb)(unsigned int, struct sk_buff *) = NULL;

    ctx->net_stat.down_total++;
    rx_cb = (struct rx_cb_map_s *)&((skb)->cb[0]);
    desc = (struct ndis_field0_desc *)&rx_cb->userfield0;
    if (desc->pif_id >= MAX_IFACE_ID) {
        ctx->net_stat.ifid_error++;
        bsp_dra_kfree_skb(skb);
        return;
    }

    map_item = &ctx->net_ctx.map_table[desc->pif_id];
    rcu_read_lock();
    deliver_cb = rcu_dereference(map_item->deliver_cb);
    if (deliver_cb == NULL) {
        rcu_read_unlock();
        ctx->net_stat.no_deliver_cb++;
        bsp_dra_kfree_skb(skb);
        return;
    }
    rcu_read_unlock();

    if ((desc->pkt_type == IP_TYPE_VAL) && !bsp_usb_is_mbim_mode()) {
        /* IP mode, first packet, need add pfa entry */
        ppp_ndis_add_ethhdr(map_item, skb, rx_cb);
    }

    if (deliver_cb(map_item->chan_id, skb) < 0) {
        ctx->net_stat.down_fail++;
    }
}
EXPORT_SYMBOL(ppp_ndis_single_down_deliver);

void ppp_ndis_single_down_rx_complete(void)
{
    bsp_pcie_stick_rx_complete();
}

static void ppp_ndis_cfg_deliver_cb(struct ppp_ndis_map_item *map_table, unsigned char chan_type)
{
    switch (chan_type) {
        case PPP_NDIS_DEV_UNET: {
            rcu_assign_pointer(map_table->deliver_cb, bsp_unet_stick_single_down_deliver);
            break;
        }
        case PPP_NDIS_DEV_PNET: {
            rcu_assign_pointer(map_table->deliver_cb, bsp_pcie_stick_single_down_deliver);
            break;
        }
        case PPP_NDIS_DEV_MODEM: {
            rcu_assign_pointer(map_table->deliver_cb, bsp_modem_stick_single_down_deliver);
            break;
        }
        default: {
            ppp_ndis_err("dev type is unsupport\n");
            return;
        }
    }
    synchronize_rcu();
}

static void ppp_ndis_rel_deliver_cb(struct ppp_ndis_map_item *map_table)
{
    rcu_assign_pointer(map_table->deliver_cb, NULL);
    synchronize_rcu();
}

static void ppp_ndis_net_link_change(struct ppp_ndis_map_item *map_item,
                                     struct net_iface_info *iface_info,
                                     struct ppp_ndis_field_info *field_info)
{
    map_item->chan_type = field_info->chan_type;
    map_item->chan_id = field_info->chan_id;
    if (iface_info->link_state == LINK_STATE_UP) {
        ppp_ndis_cfg_deliver_cb(map_item, field_info->chan_type);
    } else {
        ppp_ndis_rel_deliver_cb(map_item);
    }
    return;
}

void bsp_set_ndis_iface_info(struct ppp_ndis_field_info *field_info, struct net_iface_info *iface_info)
{
    unsigned char iface_id;
    struct wan_callback_s wan_cb = {0};
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();

    iface_id = iface_info->iface_id;
    if (iface_id >= MAX_IFACE_ID) {
        ppp_ndis_err("iface id: %u out of range\n", iface_id);
        return;
    }
    wan_cb.unmap_en = 0;
    wan_cb.rx = ppp_ndis_single_down_deliver;
    wan_cb.rx_complete = ppp_ndis_single_down_rx_complete;
    mdrv_wan_callback_register(iface_id, &wan_cb);
    ppp_ndis_net_link_change(&ctx->net_ctx.map_table[iface_id], iface_info, field_info);
}

static void ppp_ndis_modem_link_change(struct ppp_ndis_map_item *map_item,
                                     struct ppp_ndis_field_info *field_info)
{
    map_item->chan_type = field_info->chan_type;
    map_item->chan_id = field_info->chan_id;
    ppp_ndis_cfg_deliver_cb(map_item, field_info->chan_type);
    return;
}

void bsp_set_ppp_iface_info(struct ppp_ndis_field_info *field_info, struct ppp_iface_info *iface_info)
{
    unsigned char iface_id;
    struct wan_callback_s wan_cb = {0};
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();

    iface_id = iface_info->iface_id;
    if (iface_id >= MAX_IFACE_ID) {
        ppp_ndis_err("iface id: %u out of range\n", iface_id);
        return;
    }
    wan_cb.unmap_en = 0;
    wan_cb.rx = ppp_ndis_single_down_deliver;
    wan_cb.rx_complete = ppp_ndis_single_down_rx_complete;
    mdrv_wan_callback_register(iface_id, &wan_cb);
    ppp_ndis_modem_link_change(&ctx->net_ctx.map_table[iface_id], field_info);
}

void bsp_ppp_ndis_get_netdev_cfg(unsigned char chan_type, struct net_dev_cfg *netdev_cfg)
{
    if (chan_type == PPP_NDIS_DEV_UNET) {
        netdev_cfg->rawip_en = bsp_usb_is_mbim_mode() ? 1 : 0;
    } else {
        netdev_cfg->rawip_en = 0;
    }
    netdev_cfg->ipv4_auto_en = netdev_cfg->rawip_en ? 0 : 1;
    netdev_cfg->ipv6_auto_en = netdev_cfg->rawip_en ? 0 : 1;
}

int ppp_ndis_net_init(void)
{
    struct device *dev = NULL;

    bsp_unet_registe_notify_ready_cb(ppp_ndis_set_ready_state);
    dev = &g_ppp_ndis_dev;
    dev->dma_mask = &g_ppp_ndis_mask;
    arch_setup_dma_ops(dev, 0, 0, NULL, 0);
    dma_set_mask_and_coherent(dev, DMA_BIT_MASK(PPP_NDIS_MASK));
    return 0;
}

int ppp_ndis_net_uninit(void)
{
    bsp_unet_registe_notify_ready_cb(NULL);
    return 0;
}

static bool ppp_ndis_usb_stick_support(void)
{
    bsp_usb_nv_init();
    return bsp_usb_support_stick_mode();
}

static bool ppp_ndis_pcie_stick_support(void)
{
    return 0;
}

bool ppp_ndis_stick_support(void)
{
    return ppp_ndis_usb_stick_support() || ppp_ndis_pcie_stick_support();
}

void usb_ndis_show(void)
{
    struct ppp_ndis_ctx *ctx = ppp_ndis_get_ctx();
    struct net_stat_ctx *net_stat = &ctx->net_stat;

    ppp_ndis_err("open               : %u\n", net_stat->open);
    ppp_ndis_err("close              : %u\n", net_stat->close);
    ppp_ndis_err("down total pkt     : %u\n", net_stat->down_total);
    ppp_ndis_err("down no cb         : %u\n", net_stat->no_deliver_cb);
    ppp_ndis_err("iface id err       : %u\n", ctx->net_stat.ifid_error);
    ppp_ndis_err("down fail pkt      : %u\n", net_stat->down_fail);
    ppp_ndis_err("down data total    : %u\n", net_stat->down_data_total);
    ppp_ndis_err("down data fail     : %u\n", net_stat->down_data_fail);
    ppp_ndis_err("down data succ     : %u\n", net_stat->down_data_succ);
    ppp_ndis_err("down ctrl total    : %u\n", net_stat->down_ctrl_total);
    ppp_ndis_err("down ctrl fail     : %u\n", net_stat->down_ctrl_fail);
    ppp_ndis_err("down ctrl succ     : %u\n", net_stat->down_ctrl_succ);
    ppp_ndis_err("up ctrl total      : %u\n", net_stat->up_ctrl_total);
}
