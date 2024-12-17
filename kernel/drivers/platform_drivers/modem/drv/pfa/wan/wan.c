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
#include <linux/if_vlan.h>
#include <product_config.h>
#include <mdrv_pfa.h>
#include <securec.h>
#include <bsp_dra.h>
#include <bsp_pfa.h>
#include "wan.h"
#include "pfa_tft.h"
#include "bsp_net_om.h"
#include <bsp_slice.h>
#include <bsp_syscache.h>
#include "bsp_dt.h"
#include "pfa.h"
#include "pfa_interrupt.h"
#include "pfa_hal_reg.h"
#include "pfa_desc.h"


#define PFA_WAN_TD_NUM (1024)
#ifdef CONFIG_PFA_PHONE_SOC
#define PFA_WAN_RD_NUM (4096)
#else
#define PFA_WAN_RD_NUM (320)
#endif
#define PFA_WAN_RD_SKB_SIZE (1800)
#define TX_QUEUE_HIGH_WATERLEVEL (108)
#define TX_QUEUE_LOW_WATERLEVEL (64)
#define TX_MAP_FAIL_MAX (10000)
#define ERROR_VALUE (-1)
#define NAPI_WEIGHT (64)
#define UDP_PKT_LEN 1500
#define NUM_PER_SLICE (32768)
#define BIT_NUM_PER_BYTE (8)
#define WAN_SPEED_BTOMB (1024 * 1024)

struct wan_ctx_s g_wan_ctx = {0};

int wan_info_dump(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;

    bsp_err("pfa_port   %u\n", wan->pfa_port);
    bsp_err("water_level   %u\n", wan->water_level);

    bsp_err("rx_finish_rd_cnt   %u\n", wan->rx_finish_rd_cnt);
    bsp_err("rx_drop_cnt   %u\n", wan->rx_drop_cnt);
    bsp_err("rx_free   %u\n", wan->rx_free);
    bsp_err("rx_stick_cnt   %u\n", wan->rx_stick_cnt);
    bsp_err("rx_to_wan_cnt   %u\n", wan->rx_to_wan_cnt);
    bsp_err("rx_to_net_cnt   %u\n", wan->rx_to_net_cnt);
    bsp_err("rx_complete_intr_cnt   %u\n", wan->rx_complete_intr_cnt);
    bsp_err("tx_config_pfa_fail   %u\n", wan->tx_config_pfa_fail);

    bsp_err("tx_xmit_cnt   %u\n", wan->tx_xmit_cnt);
    bsp_err("tx_skb_null   %u\n", wan->tx_skb_null);
    bsp_err("tx_map_fail   %u\n", wan->tx_map_fail);
    bsp_err("tx_config_pfa_succ   %u\n", wan->tx_config_pfa_succ);
    bsp_err("tx_config_pfa_fail   %u\n", wan->tx_config_pfa_fail);

    bsp_err("rx_wan_nr_drop_stub   %u\n", wan->rx_wan_nr_drop_stub);
    bsp_err("tx_finish_td_cnt   %u\n", wan->tx_finish_td_cnt);
    bsp_err("tx_pkt_len_out_bound   %u\n", wan->tx_pkt_len_out_bound);
    bsp_err("tx_skb_mac_len_err   %u\n", wan->tx_skb_mac_len_err);

    return 0;
}

void wan_hds_transreport(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_debug_info_s *debug_info = &wan->hds_info;

    debug_info->dbg_pfa_port = wan->pfa_port;
    debug_info->dbg_water_level = wan->water_level;
    debug_info->dbg_rx_finish_rd_cnt = wan->rx_finish_rd_cnt;
    debug_info->dbg_rx_drop_cnt = wan->rx_drop_cnt;
    debug_info->dbg_rx_to_wan_cnt = wan->rx_to_wan_cnt;
    debug_info->dbg_rx_to_net_cnt = wan->rx_to_net_cnt;
    debug_info->dbg_rx_complete_intr_cnt = wan->rx_complete_intr_cnt;
    debug_info->dbg_in_rd = wan->in_rd[0];
    debug_info->dbg_tx_config_pfa_fail = wan->tx_config_pfa_fail;
    debug_info->dbg_tx_xmit_cnt = wan->tx_xmit_cnt;
    debug_info->dbg_tx_skb_null = wan->tx_skb_null;
    debug_info->dbg_tx_map_fail = wan->tx_map_fail;
    debug_info->dbg_tx_config_pfa_succ = wan->tx_config_pfa_succ;
    debug_info->dbg_tx_config_pfa_fail = wan->tx_config_pfa_fail;
    debug_info->dbg_rx_wan_nr_drop_stub = wan->rx_wan_nr_drop_stub;
    debug_info->dbg_tx_finish_td_cnt = wan->tx_finish_td_cnt;
    debug_info->dbg_tx_pkt_len_out_bound = wan->tx_pkt_len_out_bound;

    return;
}

void wan_trans_report_register(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct net_om_info *info = &wan->report_info;
    int ret;

    info->ul_msg_id = NET_OM_MSGID_WAN;
    info->data = (void *)&wan->hds_info;
    info->size = sizeof(wan->hds_info);
    info->func_cb = wan_hds_transreport;
    INIT_LIST_HEAD(&info->list);

    ret = bsp_net_report_register(info);
    if (ret) {
        bsp_err("trans report register fail\n");
    }
}

int wan_dev_dump(void)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_dev_entry *ptr = NULL;
    struct wan_info_s dev_info;

    list_for_each_entry(ptr, &wan->dev_list, list) {
        if (ptr != NULL) {
            bsp_err("dev_name   %s\n", ptr->info.dev->name);
            if (ptr->info.get_fw_info) {
                ptr->info.get_fw_info(ptr->info.dev, PFA_FW_TYPE_IPV4, &dev_info);
                bsp_err("ipv4 info  \n");
                bsp_err("pdu_session_id   %u\n", dev_info.info.pdu_session_id);
                bsp_err("fc_head   %u\n", dev_info.info.fc_head);
                bsp_err("modem_id   %u\n", dev_info.info.modem_id);
                bsp_err("higi_pri_flag   %u\n", dev_info.info.higi_pri_flag);
                bsp_err("parse_en   %u\n", dev_info.info.parse_en);
                bsp_err("userfield0   %x\n", dev_info.userfield0);
                bsp_err("userfield1   %x\n", dev_info.userfield1);
                bsp_err("userfield2   %x\n", dev_info.userfield2);

                ptr->info.get_fw_info(ptr->info.dev, PFA_FW_TYPE_IPV6, &dev_info);
                bsp_err("ipv6 info  \n");
                bsp_err("pdu_session_id   %u\n", dev_info.info.pdu_session_id);
                bsp_err("fc_head   %u\n", dev_info.info.fc_head);
                bsp_err("modem_id   %u\n", dev_info.info.modem_id);
                bsp_err("higi_pri_flag   %u\n", dev_info.info.higi_pri_flag);
                bsp_err("parse_en   %u\n", dev_info.info.parse_en);
                bsp_err("userfield0   %x\n", dev_info.userfield0);
                bsp_err("userfield1   %x\n", dev_info.userfield1);
                bsp_err("userfield2   %x\n", dev_info.userfield2);
            }
        } else {
            bsp_err("wan dev NULL!\n");
        }
    }
    return 0;
}

int wan_rd_drop_en(int drop_stub_en)
{
    g_wan_ctx.rx_wan_nr_drop_stub = drop_stub_en;
    return 0;
}

void wan_speed(void)
{
    struct wan_ctx_s *wan_ctx = &g_wan_ctx;
    static unsigned int last_cpu_rd_cnt = 0;
    static unsigned int last_timestamp = 0;
    unsigned int cur_time;
    unsigned int cpu_rd_inc_cnt;
    unsigned int time_slice;

    cur_time = bsp_get_slice_value();

    if (wan_ctx->rx_to_wan_cnt >= last_cpu_rd_cnt) {
        cpu_rd_inc_cnt = wan_ctx->rx_to_wan_cnt - last_cpu_rd_cnt;
    } else {
        cpu_rd_inc_cnt = 0xFFFFFFFF + wan_ctx->rx_to_wan_cnt - last_cpu_rd_cnt;
    }

    if (cur_time > last_timestamp) {
        time_slice = cur_time - last_timestamp;
    } else {
        time_slice = 0xFFFFFFFF + cur_time - last_timestamp;
    }
    bsp_err("wan port speed during last sample:%lld Mb/s\n",
            ((long long)cpu_rd_inc_cnt * UDP_PKT_LEN * BIT_NUM_PER_BYTE * NUM_PER_SLICE) / time_slice / WAN_SPEED_BTOMB);

    last_cpu_rd_cnt = wan_ctx->rx_to_wan_cnt;

    last_timestamp = cur_time;
    return;
}


int wan_help(void)
{
    bsp_err("wan_info_dump  \n");
    bsp_err("wan_dev_dump  \n");
    bsp_err("wan_rd_drop_en  \n");
    return 0;
}

int mdrv_wan_callback_register(unsigned int handle, struct wan_callback_s* cb)
{
    if (!cb || !cb->rx) {
        return -1;
    }

    if (memcpy_s(&g_wan_ctx.cb[handle], sizeof(struct wan_callback_s), cb, sizeof(*cb))) {
        bsp_err("memcpy_s failed\n");
    }

    return 0;
}

int mdrv_wan_get_handle_cb_register(wan_get_handle_cb cb)
{
    if (!cb) {
        return -1;
    }
    g_wan_ctx.get_handle_cb = cb;

    return 0;
}

int mdrv_wan_dev_info_register(struct wan_dev_info_s *info)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct wan_dev_entry *ptr = NULL;

    if (info == NULL || info->dev == NULL) {
        return -1;
    }


    list_for_each_entry(ptr, &wan->dev_list, list) {
        if (ptr->info.dev == info->dev) {
            if (memcpy_s(&ptr->info, sizeof(ptr->info), info, sizeof(*info))) {
                bsp_err("memcpy_s failed\n");
            }
            return bsp_pfa_update_ipfw_entry(info);
        }
    }

    ptr = kzalloc(sizeof(struct wan_dev_entry), GFP_KERNEL);
    if (ptr == NULL) {
        return -1;
    }

    if (memcpy_s(&ptr->info, sizeof(ptr->info), info, sizeof(*info))) {
        bsp_err("memcpy_s failed\n");
    }
    list_add(&ptr->list, &wan->dev_list);

    return bsp_pfa_update_ipfw_entry(info);
}

int mdrv_wan_dev_info_deregister(struct wan_dev_info_s *info)
{
    return 0;
}

void mdrv_wan_del_direct_fw_dev(unsigned int devid)
{
    bsp_pfa_del_direct_fw_dev(devid);
}

struct wan_dev_info_s *find_wan_dev(struct net_device *dev)
{
    struct wan_dev_entry *entry = NULL;
    struct wan_ctx_s *wan = &g_wan_ctx;
    if (dev == NULL) {
        return NULL;
    }

    list_for_each_entry(entry, &wan->dev_list, list) {
        if (entry->info.dev == dev) {
            return &entry->info;
        }
    }
    return NULL;
}

int wan_set_napi_weight(int weight)
{
    struct wan_ctx_s *wan = &g_wan_ctx;

    wan->priv->napi.weight = weight;

    return weight;
}

static int wan_poll(struct napi_struct *napi, int budget)
{
    struct wan_private *priv = container_of(napi, struct wan_private, napi);
    int work_done;
    struct pfa *pfa = &g_pfa;

    work_done = pfa_process_desc_poll(priv->evt_rd_done, budget);
    if (work_done < budget) {
        napi_complete_done(napi, work_done);
        priv->evt_rd_done = 0;
        pfa_writel_relaxed(pfa->regs, PFA_RD_DONE_INTA_MASK, pfa->rd_done_mask);
    }

    return work_done >= budget ? budget : work_done;
}

#if (defined(CONFIG_PFA))
int wan_pfa_xmit_config_td(struct sk_buff *skb, unsigned long long dra_addr, unsigned int dra_flag)
{
    struct rx_cb_map_s *rx_cb = NULL;
    int ret;
    struct wan_ctx_s *wan = &g_wan_ctx;

    if (dra_flag & WAN_PFA_L2ADDR_FROM_CB) {
        rx_cb = (struct rx_cb_map_s *)skb->cb;
        ret = bsp_pfa_config_td(wan->pfa_port, rx_cb->dra_l2, skb->len, skb, PFA_TD_KICK_PKT);
    } else {
        ret = bsp_pfa_config_td(wan->pfa_port, dra_addr, skb->len, skb, PFA_TD_KICK_PKT);
    }

    return ret;
}

static int wan_pfa_xmit_skb_check(struct wan_ctx_s *wan, struct sk_buff *skb, unsigned int dra_flag)
{
    if (skb->len > DRA_OPIPE_LEVEL_1_SIZE) {
        wan->tx_pkt_len_out_bound++;
    }
    if (unlikely(skb->mac_len > VLAN_ETH_HLEN + VLAN_HLEN)) {
        wan->tx_skb_mac_len_err++;
        if (dra_flag & WAN_DRA_OWN) {
            bsp_dra_kfree_skb(skb);
        } else {
            dev_kfree_skb_any(skb);
        }
        return -1;
    }
    return 0;
}

static netdev_tx_t wan_pfa_xmit(struct sk_buff *skb, unsigned int dra_flag)
{
    struct wan_ctx_s *wan = &g_wan_ctx;
    struct sk_buff* skb_unmap = NULL;
    unsigned long long dra_addr, org_phy_addr;

    if (unlikely(wan_pfa_xmit_skb_check(wan, skb, dra_flag))) {
        return NETDEV_TX_OK;
    }

    if (dra_flag & WAN_DRA_OWN) {
        dra_addr = bsp_dra_get_dra(skb); // this dra addr is mac header;
        if (!dra_addr) {
            bsp_err("input skb %lx is not from dra!\n", (unsigned long)(uintptr_t)skb);
            wan->tx_map_fail++;
            if (wan->tx_map_fail > TX_MAP_FAIL_MAX) {
                BUG_ON(1);
            }
            return NETDEV_TX_OK;
        }
        org_phy_addr = virt_to_phys(skb->data);
        dra_addr = ((dra_addr >> 32) << 32) | org_phy_addr;

        bsp_net_report_mark_pkt_info(dra_addr + skb->mac_len, NET_OM_TYPE_PFA_UL, 0, 1);

        if (wan_pfa_xmit_config_td(skb, dra_addr, dra_flag)) {
            wan->tx_config_pfa_fail++;
            bsp_dra_free(dra_addr);
            return NETDEV_TX_OK;
        }
    } else {
        bsp_net_report_mark_pkt_info((uintptr_t)skb->data + skb->mac_len, NET_OM_TYPE_PFA_UL, 1, 0);
        dra_addr = bsp_dra_skb_map(skb, skb->mac_len);  // this dra addr is ip header;
        if (!dra_addr) {
            dev_kfree_skb_any(skb);
            wan->tx_map_fail++;
            return NETDEV_TX_OK;
        }

        if (bsp_pfa_config_td(wan->pfa_port, dra_addr - skb->mac_len, skb->len, skb, PFA_TD_KICK_PKT)) {
            wan->tx_config_pfa_fail++;
            skb_unmap = bsp_dra_skb_unmap(dra_addr);
            if (skb_unmap != NULL) {
                dev_kfree_skb_any(skb_unmap);
            }
            return NETDEV_TX_OK;
        }
    }

    wan->tx_config_pfa_succ++;
    return NETDEV_TX_OK;
}
#endif

static netdev_tx_t wan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct wan_info_s *cb = NULL;
    unsigned int dra_flag = 0;
    int ret;

    if (skb == NULL) {
        bsp_err("skb null \n");
        return NETDEV_TX_OK;
    }

    cb = (struct wan_info_s *)skb->cb;

    cb->info.fc_head = 0;
    cb->info.pdu_session_id = 1;
    cb->info.modem_id = 1;
    cb->info.higi_pri_flag = 1;
    cb->info.parse_en = 0;
    cb->userfield0 = 0;
    cb->userfield1 = 1;
    cb->userfield2 = 2; /* 2, test info */
    skb->mac_len = ETH_HLEN;

    ret = mdrv_wan_tx(skb, dra_flag);
    if (ret) {
        bsp_err("mdrv_wan_tx failed. \n");
        return NETDEV_TX_BUSY;
    }

    return NETDEV_TX_OK;
}

void wan_rx_complete(void *priv)
{
    unsigned int i;

    for (i = 0; i < WAN_CB_HANDLE_MAX; i++) {
        if (g_wan_ctx.cb[i].rx_complete && g_wan_ctx.in_rd[i]) {
            g_wan_ctx.cb[i].rx_complete();
            g_wan_ctx.rx_complete_intr_cnt++;
            g_wan_ctx.in_rd[i] = 0;
        }
    }
}

static const uint8_t g_wan_dst_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x04, 0x01
};
static const uint8_t g_wan_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x01, 0x01
};


static void set_mac_header(struct sk_buff *skb)
{
    int ret;
    struct ethhdr *eth = NULL;

    eth = (struct ethhdr *)skb->data;
    ret = memcpy_s(&eth->h_dest, ETH_ALEN, g_wan_dst_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }
    ret = memcpy_s(&eth->h_source, ETH_ALEN, g_wan_src_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }

    eth->h_proto = (__be16)ETH_P_PPP_MP;

    return;
}

void wan_set_cpu0_stub_en(unsigned int en)
{
    g_wan_ctx.cpu0_stub_en = en;
}

void pfa_restore_skb_data(struct sk_buff *skb, char* data)
{
    skb->data = data;
    skb_reset_tail_pointer(skb);
}

static struct sk_buff *wan_rx_unmap(struct wan_ctx_s *wan_ctx, struct sk_buff *skb)
{
    struct sk_buff *skb_new = NULL;
    struct rx_cb_map_s rx_cb_bak = {0};
    unsigned int len = skb->len;
    char *data = skb->data;
    unsigned int update_only_flag;

    rx_cb_bak = *((struct rx_cb_map_s *)skb->cb);
    update_only_flag = pfa_get_skb_update_only_value(skb);
    skb_new = bsp_dra_skb_unmap(rx_cb_bak.dra_org);
    if (unlikely(skb_new == NULL)) {
        bsp_err("dra unmap fail:0x%llx\n", rx_cb_bak.dra_org);
        return NULL;
    }
    pfa_set_skb_update_only_value(skb_new, update_only_flag);
    *((struct rx_cb_map_s *)skb_new->cb) = rx_cb_bak;
    if (g_pfa.pfa_version == PFA_VERSION_V200) {
        pfa_restore_skb_data(skb_new, data);
    }

    skb_put(skb_new, len);
    bsp_sc_dma_unmap_single(wan_ctx->dev, virt_to_phys(skb_new->data) - ETH_HLEN,
                     skb_new->len + ETH_HLEN, DMA_FROM_DEVICE);
    return skb_new;
}

static void wan_stub_rx(struct wan_ctx_s *wan_ctx, struct sk_buff *skb)
{
    struct sk_buff *skb_new = skb;
    struct rx_cb_map_s *rx_cb = (struct rx_cb_map_s*)(&skb->cb);

    if (rx_cb->packet_info.bits.unmapped != 1) {
        skb_new = wan_rx_unmap(wan_ctx, skb);
        if (unlikely(skb_new == NULL)) {
            return;
        }
    }

    skb_push(skb_new, ETH_HLEN);
    set_mac_header(skb_new);
    skb_new->protocol = eth_type_trans(skb_new, wan_ctx->ndev);
    netif_rx(skb_new);
    wan_ctx->rx_to_net_cnt++;
}

static inline void wan_free_skb(struct sk_buff *skb)
{
    struct rx_cb_map_s *rx_cb = (struct rx_cb_map_s*)(&skb->cb);

    if (rx_cb->packet_info.bits.unmapped == 1) {
        dev_kfree_skb_any(skb);
    } else {
        bsp_dra_kfree_skb(skb);
    }
}

int wan_skb_process(struct wan_ctx_s *wan_ctx, struct sk_buff *skb, unsigned int net_id)
{
    unsigned int handle = g_wan_ctx.get_handle_cb(skb);
    struct sk_buff *skb_new = skb;
    struct rx_cb_map_s *rx_cb = (struct rx_cb_map_s*)(&skb->cb);

    if (handle >= WAN_CB_HANDLE_MAX || g_wan_ctx.cb[handle].rx == NULL) {
        return -1;
    }
    if (g_wan_ctx.cb[handle].unmap_en && rx_cb->packet_info.bits.unmapped != 1) {
        skb_new = wan_rx_unmap(wan_ctx, skb);
        if (skb_new == NULL) {
            return -1;
        }
        rx_cb = (struct rx_cb_map_s*)(&skb_new->cb);
        rx_cb->packet_info.bits.unmapped = 1;
    }
    g_wan_ctx.cb[handle].rx(skb_new);
    wan_ctx->in_rd[handle] = 1;

    return 0;
}

void wan_finish_rd(struct sk_buff *skb, unsigned int len, void *param, unsigned int net_id)
{
    struct wan_ctx_s *wan_ctx = &g_wan_ctx;

    wan_ctx->rx_finish_rd_cnt++;
    if (unlikely(wan_ctx->rx_wan_nr_drop_stub)) {
        wan_ctx->rx_drop_cnt++;
        wan_free_skb(skb);
        return;
    }
    if (unlikely(g_wan_ctx.cpu0_stub_en != 0)) {
        wan_stub_rx(wan_ctx, skb);
        return;
    }
    if (wan_skb_process(wan_ctx, skb, net_id)) {
        wan_ctx->rx_free++;
        wan_free_skb(skb);
        return;
    }

    return;
}

static const struct net_device_ops g_wan_ops = {
    .ndo_start_xmit = wan_start_xmit,
};

int mdrv_wan_tx(struct sk_buff *skb, unsigned int dra_flag)
{
    struct wan_ctx_s* wan = &g_wan_ctx;
    if (unlikely(skb == NULL)) {
        wan->tx_skb_null++;
        return NETDEV_TX_OK;
    }
    wan->tx_xmit_cnt++;
#if (defined(CONFIG_PFA))
    return wan_pfa_xmit(skb, dra_flag);
#else
    consume_skb(skb);
    return 0;
#endif
}

void wan_port_config_init(void)
{
    struct pfa *pfa = &g_pfa;
    struct wan_ctx_s *wan = &g_wan_ctx;
    int ret;

    ret = bsp_dt_property_read_u32_array(pfa->np, "wan_td_depth", &wan->td_depth, 1);
    if (ret || wan->td_depth > PFA_RD_NUM_MAX) {
        wan->td_depth = PFA_WAN_TD_NUM;
    }

    ret = bsp_dt_property_read_u32_array(pfa->np, "wan_rd_depth", &wan->rd_depth, 1);
    if (ret || wan->rd_depth > PFA_RD_NUM_MAX) {
        wan->rd_depth = PFA_WAN_RD_NUM;
    }
}

static int wan_alloc_pfa_port(struct wan_ctx_s *wan)
{
#if (defined(CONFIG_PFA))
    struct pfa_port_comm_attr attr = {0};
    int ret;

    attr.rd_depth = wan->rd_depth;
    attr.td_depth = wan->td_depth;
    attr.net = wan->ndev;
    attr.priv = (void *)wan->ndev;
    attr.enc_type = PFA_ENC_CPU;

    attr.ops.pfa_finish_rd = wan_finish_rd;
    attr.ops.pfa_complete_rd = wan_rx_complete;
    attr.ops.get_wan_info = find_wan_dev;

    wan->pfa_port = bsp_pfa_alloc_port(&attr);
    if (wan->pfa_port < 0) {
        bsp_err("invalid wan->pfa_port: %u\n", wan->pfa_port);
        return ERROR_VALUE;
    }

    ret = bsp_pfa_enable_port(wan->pfa_port);
    if (ret != 0) {
        bsp_err("[probe]bsp_pfa_enable_port fail\n");
        return ERROR_VALUE;
    }
#endif
    return 0;
}

int wan_init(struct device *dev)
{
    int ret = 0;
    struct wan_ctx_s *wan = &g_wan_ctx;

    wan->ndev = alloc_etherdev(sizeof(struct wan_private));
    if (wan->ndev == NULL) {
        ret = ERROR_VALUE;
        bsp_err("alloc_etherdev faild!\n");
        goto WAN_PRB_ERR0;
    }

    dev_alloc_name(wan->ndev, "CPU%d");
    dev_hold(wan->ndev);
    wan->ndev->netdev_ops = &g_wan_ops;
    dev_put(wan->ndev);
    wan->priv = (struct wan_private *)netdev_priv(wan->ndev);
    netif_napi_add(wan->ndev, &wan->priv->napi, wan_poll, NAPI_WEIGHT);

    if (register_netdev(wan->ndev)) {
        ret = ERROR_VALUE;
        bsp_err("[probe]register_netdev faild!\n");
        goto WAN_PRB_ERR1;
    }

    ret = memcpy_s(wan->ndev->dev_addr, ETH_ALEN, g_wan_dst_mac_addr, ETH_ALEN);
    if (ret) {
        bsp_err("memcpy_s failed\n");
    }

    ret = wan_alloc_pfa_port(wan);
    if (ret != 0) {
        goto WAN_PRB_ERR1;
    }
    INIT_LIST_HEAD(&wan->dev_list);

    ret = lan_init();
    if (ret != 0) {
        bsp_err("[probe]lan init fail\n");
    }

    wan->dev = dev;
    wan->cpu0_stub_en = 0;
    wan_trans_report_register();
    napi_enable(&wan->priv->napi);
    bsp_err("[probe]wan_probe done\n");
    return 0;

WAN_PRB_ERR1:
    free_netdev(wan->ndev);
WAN_PRB_ERR0:
    return ret;
}
EXPORT_SYMBOL(wan_info_dump);
EXPORT_SYMBOL(wan_dev_dump);
EXPORT_SYMBOL(wan_speed);
EXPORT_SYMBOL(wan_help);
EXPORT_SYMBOL(wan_set_cpu0_stub_en);
EXPORT_SYMBOL(wan_rd_drop_en);
EXPORT_SYMBOL(mdrv_wan_callback_register);
EXPORT_SYMBOL(wan_finish_rd);
EXPORT_SYMBOL(mdrv_wan_dev_info_register);
EXPORT_SYMBOL(g_wan_ctx);
EXPORT_SYMBOL(mdrv_wan_tx);
