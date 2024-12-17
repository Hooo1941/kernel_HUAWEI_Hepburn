/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2022. All rights reserved.
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
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
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

#include <uapi/linux/if_ether.h>
#include <linux/if_vlan.h>

#include <osl_types.h>
#include <mdrv_udi.h>
#include <mdrv_pfa.h>
#include <bsp_wan_eth_stick.h>

#include "wan_eth_stick.h"
#include "wan_eth_spe.h"
#include "wan_eth_common.h"

#define PNET_MAX_NUM 8
#define HEX_NUM 16

#define WETH_STICK_PKTYPE_ETH 0x40
#define WETH_STICK_PKTYPE_VLAN 0x80
#define WETH_VLAN_PARSE_CHK_LEN 58
#define WETH_ETH_PARSE_CHK_LEN 54

enum sess_info_type {
    SESS_INFO_IPV4,
    SESS_INFO_IPV6,
    SESS_INFO_ETH,
    SESS_INFO_MAX,
};

struct stick_session_info {
    struct pfa_tft_info_s pfa_tft_infos[SESS_INFO_MAX];
    unsigned char iface_id[SESS_INFO_MAX];
    enum net_iface_mode mode;
    unsigned long rx_no_v4v6_sess;
};

struct stick_stat {
    unsigned int ioctl_ctrl;
    unsigned int stick_tx;
    unsigned int stick_tx_fail;
    unsigned int uplink_tx;
    unsigned int ethernet_tx;
    unsigned int ethernet_tx_fail;
    unsigned int ethernet_rx;
};

struct stick_ctx {
    struct stick_session_info sess_info[PNET_MAX_NUM];
    struct stick_stat stat[PNET_MAX_NUM];
    weth_stick_tx_cb_t tx_cb;
    struct sk_buff *skb_queue_tail;
    int skb_queue_len;
    spinlock_t tx_lock;
    int is_tx_lock_init;
    unsigned int invalid_devid_sess;
    unsigned int invalid_devid_data;
    unsigned int invalid_devid_uplink;
    unsigned int rx_complete_submit_fail;
    struct device *dev_for_map;
};

struct stick_ctx g_weth_stick_ctx = { 0 };

int weth_stick_get_session_mode(unsigned int dev_id)
{
    struct stick_ctx *ctx = &g_weth_stick_ctx;
    struct stick_session_info *sess_info = NULL;

    if (dev_id >= PNET_MAX_NUM) {
        ctx->invalid_devid_data++;
        return -1;
    }

    sess_info = &ctx->sess_info[dev_id];

    return sess_info->mode;
}

static void stick_ioctl_backup_session_info(struct stick_session_info *sess_info,
    struct net_packet_label *packet_label, enum sess_info_type type)
{
    struct pfa_tft_info_s *pfa_tft_info = NULL;

    pfa_tft_info = &sess_info->pfa_tft_infos[type];
    pfa_tft_info->pdu_session_id = packet_label->sess_id;
    pfa_tft_info->modem_id = packet_label->modem_id;
    pfa_tft_info->fc_head = packet_label->fc_head;

}

static int stick_ioctl_set_pdusession(struct stick_ctx *ctx, u32 dev_id, void *param)
{
    struct net_iface_info *iface_info = (struct net_iface_info *)param;
    struct net_packet_label *packet_label = NULL;
    struct stick_session_info *sess_info = &ctx->sess_info[dev_id];

    switch (iface_info->iface_mode) {
        case PPP_NDIS_NET_MODE_IP: {
            /* ipv4 sess info cfg */
            packet_label = &iface_info->v4_label;
            sess_info->iface_id[SESS_INFO_IPV4] = iface_info->iface_id;
            stick_ioctl_backup_session_info(sess_info, packet_label, SESS_INFO_IPV4);
            /* ipv6 sess info cfg */
            packet_label = &iface_info->v6_label;
            sess_info->iface_id[SESS_INFO_IPV6] = iface_info->iface_id;
            stick_ioctl_backup_session_info(sess_info, packet_label, SESS_INFO_IPV6);
            /* ip type wan info cfg func registe */

            sess_info->mode = PPP_NDIS_NET_MODE_IP;
            break;
        }
        case PPP_NDIS_NET_MODE_ETH: {
            /* eth sess info cfg */
            packet_label = &iface_info->eth_label;
            sess_info->iface_id[SESS_INFO_ETH] = iface_info->iface_id;
            stick_ioctl_backup_session_info(sess_info, packet_label, SESS_INFO_ETH);
            /* eth type wan info cfg func registe */
            sess_info->mode = PPP_NDIS_NET_MODE_ETH;
            break;
        }
        default: {
            printk(KERN_ERR "[usb]:%s: session cfg mode error\n", __func__);
            return -EINVAL;
        }
    }

    return 0;
}

int bsp_pnet_stick_ioctl(u32 dev_id, enum ncm_ioctl_cmd_type cmd, void *param)
{
    int ret = 0;
    struct stick_ctx *ctx = &g_weth_stick_ctx;

    if (dev_id >= PNET_MAX_NUM) {
        ctx->invalid_devid_sess++;
        return -1;
    }

    if (cmd == NCM_IOCTL_SET_PDUSESSION) {
        ctx->stat[dev_id].ioctl_ctrl++;
        ret = stick_ioctl_set_pdusession(ctx, dev_id, param);
    }

    if (ctx->is_tx_lock_init == 0) {
        spin_lock_init(&ctx->tx_lock);
        ctx->is_tx_lock_init = 1;
    }

    weth_register_get_session_mode_cb(weth_stick_get_session_mode);
    return ret;
}

int bsp_pnet_stick_open(void)
{
    return 0;
}

int bsp_pnet_stick_close(void)
{
    return 0;
}

void bsp_pcie_stick_rx_complete(void)
{
    struct stick_ctx *ctx = &g_weth_stick_ctx;
    struct sk_buff *skb_queue_tail = NULL;
    int skb_queue_len;
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&ctx->tx_lock, flags);
    skb_queue_tail = ctx->skb_queue_tail;
    ctx->skb_queue_tail = NULL;

    skb_queue_len = ctx->skb_queue_len;
    ctx->skb_queue_len = 0;
    spin_unlock_irqrestore(&ctx->tx_lock, flags);

    if (skb_queue_tail == NULL) {
        return;
    }

    ret = ctx->tx_cb(skb_queue_tail, skb_queue_len, WETH_DEBUG_PORT_NO);
    if (ret != 0) {
        ctx->rx_complete_submit_fail++;
    }
}

void wan_eth_stick_packet_enqueue(struct stick_ctx *ctx, struct sk_buff *skb, struct stick_stat *stat)
{
    unsigned long flags;

    spin_lock_irqsave(&ctx->tx_lock, flags);
    skb_onelink_tail(&ctx->skb_queue_tail, skb);
    ctx->skb_queue_len++;
    spin_unlock_irqrestore(&ctx->tx_lock, flags);

    stat->ethernet_rx++;
}

int bsp_pcie_stick_single_down_deliver(unsigned int dev_id, struct sk_buff *skb)
{
    struct stick_ctx *ctx = &g_weth_stick_ctx;
    struct stick_stat *stat = NULL;
    struct rx_cb_map_s *map_info = (struct rx_cb_map_s *)skb->cb;
    int ret;

    if (dev_id >= PNET_MAX_NUM) {
        ctx->invalid_devid_data++;
        return -1;
    }

    stat = &ctx->stat[dev_id];
    stat->stick_tx++;

    skb_onelink_tail(&skb, skb);
    map_info->userfield0 = ((struct ethhdr *)skb->data)->h_proto;
    map_info->userfield0 = (map_info->userfield0 << HEX_NUM) | dev_id; /* get high 16 bits */

    if (weth_stick_get_session_mode(dev_id) == PPP_NDIS_NET_MODE_ETH) {
        map_info->userfield2 = WETH_ETH_MODE;
        wan_eth_stick_packet_enqueue(ctx, skb, stat);
    } else {
        weth_add_pfa_direct_fw_entry(skb, dev_id);
        ret = ctx->tx_cb(skb, 1, WETH_DEBUG_PORT_NO);
        if (ret != 0) {
            stat->stick_tx_fail++;
            return ret;
        }
    }

    return 0;
}

void weth_stick_register_tx_cb(weth_stick_tx_cb_t cb)
{
    g_weth_stick_ctx.tx_cb = cb;
}

static u8 weth_stick_eth_can_parse(struct sk_buff *skb)
{
    switch (skb->protocol) {
        case htons(ETH_P_8021Q): {
            if (skb->len >= WETH_VLAN_PARSE_CHK_LEN) {
                return 1;
            }
            break;
        }
        default: {
            if (skb->len >= WETH_ETH_PARSE_CHK_LEN) {
                return 1;
            }
            break;
        }
    }
    return 0;
}

void weth_ethernet_tx(struct sk_buff *skb, unsigned int dev_id, struct stick_session_info *sess_info)
{
    struct wan_info_s *wan_info = NULL;
    struct vlan_ethhdr *vhdr = NULL;
    struct stick_ctx *ctx = &g_weth_stick_ctx;
    struct stick_stat *stat = &ctx->stat[dev_id];

    stat->ethernet_tx++;
    dma_unmap_single(ctx->dev_for_map, virt_to_phys(skb->data), L1_CACHE_BYTES, DMA_FROM_DEVICE);
    wan_info = (struct wan_info_s *)(&skb->cb);
    wan_info->userfield1 = 0;
    wan_info->userfield2 = 0;
    wan_info->info.pkt_ind = 1;

    vhdr = (struct vlan_ethhdr *)(skb->data);
    skb->protocol = vhdr->h_vlan_proto;
    if (skb->protocol == htons(ETH_P_8021Q)) {
        wan_info->userfield0 |= WETH_STICK_PKTYPE_VLAN;
    } else {
        wan_info->userfield0 |= WETH_STICK_PKTYPE_ETH;
    }

    dma_map_single(ctx->dev_for_map, skb->data, L1_CACHE_BYTES, DMA_FROM_DEVICE);
}

int weth_stick_wan_tx(struct sk_buff *skb)
{
    struct stick_ctx *ctx = &g_weth_stick_ctx;
    struct wan_info_s *wan_info = (struct wan_info_s *)skb->cb;
    struct stick_session_info *sess_info;
    unsigned short protocol;
    unsigned int dev_id;
    struct pfa_tft_info_s *pfa_tft_info = NULL;
    struct stick_stat *stat = NULL;

    dev_id = wan_info->userfield0 & 0xFFFF;
    if (dev_id >= PNET_MAX_NUM) {
        ctx->invalid_devid_uplink++;
        return -1;
    }
    stat = &ctx->stat[dev_id];
    stat->uplink_tx++;

    protocol = (wan_info->userfield0 >> 16 ) & 0xFFFF;
    sess_info = &ctx->sess_info[dev_id];
    if (sess_info->mode == PPP_NDIS_NET_MODE_ETH) {
        pfa_tft_info = &sess_info->pfa_tft_infos[SESS_INFO_ETH];
        wan_info->userfield0 = sess_info->iface_id[SESS_INFO_ETH];
    } else if (protocol == htons(ETH_P_IP)) {
        pfa_tft_info = &sess_info->pfa_tft_infos[SESS_INFO_IPV4];
        wan_info->userfield0 = sess_info->iface_id[SESS_INFO_IPV4];
    } else {
        pfa_tft_info = &sess_info->pfa_tft_infos[SESS_INFO_IPV6];
        wan_info->userfield0 = sess_info->iface_id[SESS_INFO_IPV6];
    }

    wan_info->info.pdu_session_id = pfa_tft_info->pdu_session_id;
    wan_info->info.modem_id = pfa_tft_info->modem_id;
    wan_info->info.fc_head = pfa_tft_info->fc_head;
    wan_info->info.pkt_ind = 0;
    wan_info->info.parse_en = !!weth_stick_eth_can_parse(skb);

    if (sess_info->mode == PPP_NDIS_NET_MODE_ETH) {
        weth_ethernet_tx(skb, dev_id, sess_info);
    }

    weth_pr_dbg("sess_mode %d, protocol %d, parse_en %d, pkt_ind %d\n",
                sess_info->mode, protocol, wan_info->info.parse_en, wan_info->info.pkt_ind);

    return mdrv_wan_tx(skb, WAN_DRA_OWN);
}

void weth_stick_set_dev_for_map(struct device *dev)
{
    g_weth_stick_ctx.dev_for_map = dev;
}
