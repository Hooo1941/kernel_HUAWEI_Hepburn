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
#include <linux/platform_device.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/list.h>
#include <product_config.h>
#include <mdrv_pfa.h>
#include <securec.h>
#include <bsp_dra.h>
#include <bsp_pfa.h>
#include "wan.h"
#include "pfa_tft.h"
#include "bsp_net_om.h"
#include "pfa_desc.h"
#include <bsp_slice.h>

#define PFA_LAN_TD_NUM 1024
#define PFA_LAN_RD_NUM 4096
#define PFA_LAN_RD_SKB_SIZE 1800
#define TX_QUEUE_HIGH_WATERLEVEL 108
#define TX_QUEUE_LOW_WATERLEVEL 64
#define TX_MAP_FAIL_MAX 10000
#define ERROR_VALUE (-1)
#define NAPI_WEIGHT 64
#define UDP_PKT_LEN 1500
#define NUM_PER_SLICE 32768
#define BIT_NUM_PER_BYTE 8
#define LAN_SPEED_BTOMB (1024 * 1024)

struct lan_ctx_s g_lan_ctx = {0};

int lan_info_dump(void)
{
    struct lan_ctx_s *lan = &g_lan_ctx;

    bsp_err("pfa_port   %u\n", lan->pfa_port);

    bsp_err("rx_finish_rd_cnt   %u\n", lan->rx_finish_rd_cnt);
    bsp_err("rx_drop_cnt   %u\n", lan->rx_drop_cnt);
    bsp_err("rx_to_LAN_cnt   %u\n", lan->rx_to_lan_cnt);
    bsp_err("rx_to_net_cnt   %u\n", lan->rx_to_net_cnt);
    bsp_err("rx_complete_intr_cnt   %u\n", lan->rx_complete_intr_cnt);
    bsp_err("in_rd   %u\n", lan->in_rd);
    bsp_err("tx_config_pfa_fail   %u\n", lan->tx_config_pfa_fail);

    bsp_err("tx_xmit_cnt   %u\n", lan->tx_xmit_cnt);
    bsp_err("tx_skb_null   %u\n", lan->tx_skb_null);
    bsp_err("tx_map_fail   %u\n", lan->tx_map_fail);
    bsp_err("tx_config_pfa_succ   %u\n", lan->tx_config_pfa_succ);
    bsp_err("tx_config_pfa_fail   %u\n", lan->tx_config_pfa_fail);

    bsp_err("rx_LAN_nr_drop_stub   %u\n", lan->rx_lan_nr_drop_stub);
    bsp_err("tx_finish_td_cnt   %u\n", lan->tx_finish_td_cnt);
    bsp_err("tx_pkt_len_out_bound   %u\n", lan->tx_pkt_len_out_bound);

    return 0;
}

int lan_help(void)
{
    bsp_err("lan_info_dump  \n");
    return 0;
}

#if (defined(CONFIG_PFA))
static netdev_tx_t lan_pfa_xmit(struct sk_buff *skb, unsigned int dra_flag)
{
    struct lan_ctx_s *lan = &g_lan_ctx;
    struct sk_buff* skb_unmap = NULL;
    unsigned long long dra_addr;
    unsigned int dra_addr_type;

    if (skb->len > ETH_PKT_LEN_MAX) {
        lan->tx_pkt_len_out_bound++;
    }

    if (dra_flag & WAN_DRA_OWN) {
        dra_addr = bsp_dra_get_dra(skb); // this dra addr is mac header;
        if (!dra_addr) {
            bsp_err("input skb %lx is not from dra!\n", (unsigned long)(uintptr_t)skb);
            lan->tx_map_fail++;
            if (lan->tx_map_fail > TX_MAP_FAIL_MAX) {
                BUG_ON(1);
            }
            return NETDEV_TX_OK;
        }

        dra_addr_type = bsp_dra_get_addr_type(dra_addr);
        if (unlikely(dra_addr_type != DRA_SKB_FROM_OWN)) {
            WARN_ON_ONCE(1);
        }
        if (bsp_pfa_config_td(lan->pfa_port, dra_addr, skb->len, skb, PFA_TD_KICK_PKT)) {
            lan->tx_config_pfa_fail++;
            bsp_dra_free(dra_addr);
            return NETDEV_TX_OK;
        }
    } else {
        /* dra对齐到mac头，pfa rd出来时是ip头，pfa会push到mac头，才能和dra tcpip链表对应 */
        dra_addr = bsp_dra_skb_map(skb, 0);
        if (!dra_addr) {
            dev_kfree_skb_any(skb);
            lan->tx_map_fail++;
            return NETDEV_TX_OK;
        }

        if (bsp_pfa_config_td(lan->pfa_port, dra_addr, skb->len, skb, PFA_TD_KICK_PKT)) {
            lan->tx_config_pfa_fail++;
            skb_unmap = bsp_dra_skb_unmap(dra_addr);
            if (skb_unmap != NULL) {
                dev_kfree_skb_any(skb_unmap);
            }
            return NETDEV_TX_OK;
        }
    }

    lan->tx_config_pfa_succ++;
    return NETDEV_TX_OK;
}
#else
static netdev_tx_t lan_pfa_xmit(struct sk_buff *skb, unsigned int dra_flag)
{
    return NETDEV_TX_OK;
}
#endif


void lan_rx_complete(void *priv)
{
    if (g_lan_ctx.cb.rx_complete && g_lan_ctx.in_rd) {
        g_lan_ctx.cb.rx_complete();
        g_lan_ctx.rx_complete_intr_cnt++;
        g_lan_ctx.in_rd = 0;
    }
}

void lan_finish_rd(struct sk_buff *skb, unsigned int len, void *param, unsigned int flags)
{
    struct lan_ctx_s *lan_ctx = &g_lan_ctx;
    struct rx_cb_map_s *cb_info = NULL;
    lan_ctx->rx_finish_rd_cnt++;

    if (skb == NULL) {
        lan_ctx->skb_null++;
        return;
    }

    if (lan_ctx->ndev == NULL) {
        lan_ctx->ndev_null++;
        return;
    }
    skb->dev = lan_ctx->ndev;
    cb_info = ((struct rx_cb_map_s *)&((skb)->cb));
    pfa_pts_rpt(cb_info->userfield2, PFA_LAN_UP_RCV);
    if (lan_ctx->cb.lan_rx) {
        lan_ctx->cb.lan_rx(skb);
        lan_ctx->rx_to_lan_cnt++;
        lan_ctx->in_rd = 1;
    } else {
        lan_ctx->rx_to_net_cnt++;
        bsp_dra_kfree_skb(skb);
    }

    return;
}

int mdrv_lan_tx(struct sk_buff *skb, unsigned int dra_flag)
{
    struct lan_ctx_s* lan = &g_lan_ctx;
    struct rx_cb_map_s *cb_info = NULL;

    if (unlikely(skb == NULL)) {
        lan->tx_skb_null++;
        return NETDEV_TX_OK;
    }
    lan->tx_xmit_cnt++;
    cb_info = ((struct rx_cb_map_s *)&((skb)->cb));
    pfa_pts_rpt(cb_info->userfield2, PFA_LAN_DOWN_RCV);

    return lan_pfa_xmit(skb, dra_flag);
}

int mdrv_lan_add_netdev(struct net_device *ndev, int (*lan_rx)(struct sk_buff* skb), void (*lan_rx_complete)(void))
{
    struct lan_ctx_s* lan = &g_lan_ctx;

    if (ndev == NULL) {
        return -1;
    }
    dev_hold(ndev);
    lan->ndev = ndev;
    lan->cb.lan_rx = lan_rx;
    lan->cb.rx_complete = lan_rx_complete;

    return bsp_pfa_update_net(lan->pfa_port, ndev, 1);
}

int mdrv_lan_rm_netdev(struct net_device *ndev)
{
    struct lan_ctx_s* lan = &g_lan_ctx;

    if (ndev == NULL) {
        return -1;
    }
    dev_put(ndev);

    return bsp_pfa_update_net(0, lan->ndev_init, 0);
}

int lan_init(void)
{
    int ret = 0;
    struct lan_ctx_s *lan = &g_lan_ctx;
    struct pfa_port_comm_attr attr = {0};

    lan->ndev_init = alloc_etherdev(sizeof(struct wan_private));
    if (lan->ndev_init == NULL) {
        ret = ERROR_VALUE;
        bsp_err("alloc_etherdev faild!\n");
        goto LAN_PRB_ERR0;
    }
    attr.rd_depth = PFA_LAN_RD_NUM;
    attr.td_depth = PFA_LAN_TD_NUM;
    attr.net = lan->ndev_init;
    attr.priv = (void *)lan->ndev_init;
    attr.enc_type = PFA_ENC_NONE;
    attr.rsv_port_id = bsp_pfa_get_reserve_port_id(PFA_ACORE_LAN_CTRL_VIR_ID);

    attr.ops.pfa_finish_rd = lan_finish_rd;
    attr.ops.pfa_complete_rd = lan_rx_complete;

    lan->pfa_port = bsp_pfa_alloc_port(&attr);
    if (lan->pfa_port < 0) {
        bsp_err("invalid lan->pfa_port: %u\n", lan->pfa_port);
        goto LAN_PRB_ERR1;
    }

    ret = bsp_pfa_enable_port(lan->pfa_port);
    if (ret != 0) {
        bsp_err("[probe]bsp_pfa_enable_port fail\n");
        ret = -1;
        goto LAN_PRB_ERR1;
    }

    bsp_err("lan init ok\n");
    return 0;

LAN_PRB_ERR1:
    free_netdev(lan->ndev_init);
LAN_PRB_ERR0:
    return ret;
}

EXPORT_SYMBOL(mdrv_lan_tx);
EXPORT_SYMBOL(mdrv_lan_add_netdev);
EXPORT_SYMBOL(mdrv_lan_rm_netdev);
EXPORT_SYMBOL(lan_info_dump);
EXPORT_SYMBOL(lan_help);
