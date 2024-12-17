/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <securec.h>

#include "bsp_pcie.h"
#include "wan_eth_table.h"
#include "wan_eth_common.h"
#include "wan_eth_dma.h"
#include "mdrv_pfa.h"
#include "wan_eth_event_buffer.h"
#include "wan_eth_maa.h"
#include "bsp_wan_eth.h"
#include "bsp_net_om.h"
#include "bsp_dump.h"
#include "mdrv_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */

#define WETH_GET_DESC_SIZE(num) ((num) * sizeof(struct pcie_dma_data_element) + sizeof(struct weth_tab_head_info))
#define INIT_SKB_TIMEOUT 100
#define WAIT_PEER_SKB_INIT_TIMEOUT 100

struct weth_tab_node {
    struct list_head list;
    struct sk_buff *local_skb[WETH_TAB_ELEMENT_CNT];
    struct weth_tab_element *local_desc;
    void *local_org_desc;

    void *peer_table_desc_phy;
    void *peer_data_desc_phy;
    void *peer_usr_field_phy;
    void *local_table_desc;
    void *local_data_desc;
    void *local_usr_field;

    void *local_table_desc_phy;
    void *local_data_desc_phy;
    void *local_usr_field_phy;

    struct weth_dma_req table_req;
    struct weth_dma_req init_table_req;
    struct weth_dma_req refill_req;
    struct weth_dma_req data_req;

    struct delayed_work refill_work;
    int idx;
    unsigned int cur_num;
    int init_table_done;
    int cur_port_no;
};

struct weth_tab_tx_stat {
    unsigned int stat_tx_data_dma;
    unsigned int stat_tx_data_dma_done;
    unsigned int stat_tx_refill;
    unsigned int stat_tx_refill_done;
};

struct weth_tab_rx_stat {
    unsigned int stat_rx_table_dma;
    unsigned int stat_rx_table_dma_done;
};

struct weth_tab_ctx {
    struct list_head tx_list;
    struct list_head rx_list;
    struct device *dev_for_map;
    enum weth_pcie_mode mode;

    struct weth_tab_node tx_node[WETH_RC_RX_TAB_MAX_NUM];
    spinlock_t tx_lock;

    struct weth_tab_node rx_node[WETH_EP_RX_TAB_MAX_NUM];
    spinlock_t rx_lock;

    weth_tab_rx_cb_t rx_cb;
    weth_tab_rx_cb_t tx_cb;
    struct workqueue_struct *refill_queue;

    struct weth_tab_tx_stat tx_stat;
    struct weth_tab_rx_stat rx_stat;

    unsigned int free_tx_tab_num;
    unsigned int free_rx_tab_num;
    unsigned int skb_tail_null;
    unsigned int refill_skb_null;
    int skb_init_fail;
};

struct weth_tab_ctx g_weth_tab_ctx;
struct weth_packet_dump g_packet_dump;
struct weth_rc_debug_info_s g_weth_rc_dbg_info;

int bsp_weth_rc_hds_transreport(struct weth_rc_debug_info_s *debug_info)
{
    int ret;

    ret = memcpy_s(debug_info, sizeof(struct weth_rc_debug_info_s), &g_weth_rc_dbg_info, sizeof(g_weth_rc_dbg_info));
    if (ret != EOK) {
        weth_pr_err("bsp_weth_rc_hds_transreport fail\n");
    }

    return ret;
}

STATIC void weth_tab_refill_push(struct work_struct *work)
{
    int i;
    int ret;
    unsigned int num;
    struct sk_buff *skb = NULL;
    unsigned long long data_phy;
    struct weth_tab_node *rx_node = container_of(work, struct weth_tab_node, refill_work.work);

    num = rx_node->cur_num;
    for (i = 0; i < num; i++) {
        if (rx_node->local_skb[i] == NULL) {
            if (g_by_pass_mode) {
                skb = bsp_dra_alloc_skb(BUF_SIZE_1_6KB, &data_phy);
            } else {
                skb = __weth_alloc_skb(WETH_SKB_MTU);
            }
            if (skb == NULL) {
                weth_pr_err("__weth_alloc_skb fail, line:%d\n", __LINE__);
                g_weth_tab_ctx.refill_skb_null++;
                /* restart work in 10 jiffies */
                queue_delayed_work(g_weth_tab_ctx.refill_queue, &rx_node->refill_work, 10); 
                return;
            }
            rx_node->local_skb[i] = skb;

            if (g_by_pass_mode) {
                data_phy = bsp_dra_to_phy(data_phy);
            } else {
                data_phy = dma_map_single(g_weth_tab_ctx.dev_for_map, skb->data, WETH_SKB_MTU, DMA_FROM_DEVICE);
            }

            /* fill the desc */
            bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i], (u64)data_phy);
        }
    }

    /* restore  last 2 skb from usrfield and link */
    bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i],
                             (u64)virt_to_phys(rx_node->local_skb[i]->data));
    bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i + 1],
                             (u64)virt_to_phys(rx_node->local_skb[i + 1]->data));

    /* flush dst addr to local desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, rx_node->local_data_desc, WETH_GET_DESC_SIZE(num + 2), DMA_TO_DEVICE);

    /* config refill dma req */
    ret = weth_dma_submit_req(&rx_node->refill_req, WETH_DMA_WRITE);
    if (ret) {
        weth_pr_err("weth_dma_submit_req fail:%d, line:%d\n", ret, __LINE__); /* check ret */
        return;
    }
}

STATIC void weth_tab_rx_refill_complete(void *para)
{
    int ret;
    unsigned int num;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *rx_node = (struct weth_tab_node *)req->priv;

    num = rx_node->cur_num;

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)rx_node->local_data_desc_phy, WETH_GET_DESC_SIZE(num + 2),
                     DMA_TO_DEVICE);
    /* invalid cache for new desc from rc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, rx_node->local_data_desc, WETH_GET_DESC_SIZE(num + 2), DMA_FROM_DEVICE);

    ret = weth_send_event(num, rx_node->idx, WETH_REFILL_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__); /* check ret */
    }
}

STATIC void weth_tab_tx_refill_complete(void *para)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *tx_node = (struct weth_tab_node *)req->priv;
    unsigned long flags;

    g_weth_tab_ctx.tx_stat.stat_tx_refill_done++;
    spin_lock_irqsave(&tab_ctx->tx_lock, flags);
    list_add_tail(&tx_node->list, &tab_ctx->tx_list);
    tab_ctx->free_tx_tab_num++;
    spin_unlock_irqrestore(&tab_ctx->tx_lock, flags);

    /* invalid cache for refill */
    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)tx_node->local_data_desc_phy,
                     WETH_GET_DESC_SIZE(tx_node->cur_num + 2), DMA_FROM_DEVICE);

    return;
}

STATIC unsigned int weth_tab_set_userfield(struct weth_tab_node *node, struct sk_buff *skb, int index)
{
    struct wan_info_s *cb_map = NULL;

    cb_map = (struct wan_info_s *)&skb->cb;
    cb_map->userfield0 = node->local_desc->header.usr_field[index].usr_field0;
    cb_map->userfield1 = node->local_desc->header.usr_field[index].usr_field1;
    cb_map->userfield2 = node->local_desc->header.usr_field[index].usr_field2;

    weth_pr_dbg("cb_map->userfield2: %x\n", cb_map->userfield2);

    return (cb_map->userfield0 & 0xFFFF);
}

static void weth_rx_debug_data(struct sk_buff *skb, struct wan_info_s *cb_map)
{
    struct weth_hids_trans_open_msg *open_info = NULL;
    struct net_om_msg_info *msg_info = NULL;
    int ret;

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, virt_to_phys(skb->data), skb->len, DMA_FROM_DEVICE);
    if (cb_map->userfield1 == WETH_RC_DEBUG_FLAG) {
        ret = memcpy_s(&g_weth_rc_dbg_info, sizeof(g_weth_rc_dbg_info), skb->data, sizeof(struct weth_rc_debug_info_s));
        if (ret != 0) {
            weth_pr_err("memcpy_s ret:%d\n", ret);
        }
    } else if (cb_map->userfield1 == WETH_RC_HIDS_PKT_OPEN) {
        open_info = (struct weth_hids_trans_open_msg *)skb->data;
        weth_pr_err("en_type:%d,  enable:%d\n", open_info->en_type, open_info->enable);
        bsp_net_report_enable_by_type(open_info->en_type, open_info->enable);
    } else if (cb_map->userfield1 == WETH_EP_VOTE_COUNT) {
        g_vote_count = *(int *)skb->data;
    } else if (cb_map->userfield1 == WETH_RC_DEBUG_PKT) {
        msg_info = (struct net_om_msg_info *)skb->data;
        bsp_net_report_rc_pkt(msg_info->prim_id, skb->data, skb->len);
    }

    g_packet_dump.maa_free++;
    if (g_by_pass_mode) {
        dma_map_single(g_weth_tab_ctx.dev_for_map, skb->data, skb->len, DMA_FROM_DEVICE);
        __weth_free_maa_skb(skb);
    } else {
        __weth_free_skb(skb);
    }
    return;
}

STATIC void weth_tab_rx_data_complete(void *para)
{
    int ret;
    int i;
    unsigned int len;
    unsigned int num;
    unsigned int rx_pkg = 0;
    struct sk_buff *skb_tail = NULL;
    struct sk_buff *skb = NULL;
    unsigned int dev_id;
    unsigned int dev_id_old = 0;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *rx_node = (struct weth_tab_node *)req->priv;
    struct wan_info_s *cb_map = NULL;

    num = rx_node->cur_num;

    /* invalid usr field */
    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)rx_node->local_usr_field_phy,
                     sizeof(struct weth_tab_usr_field) * num, DMA_FROM_DEVICE);

    /* recv skb to weth hal */
    for (i = 0; i < num; i++) {
        skb = rx_node->local_skb[i];
        rx_node->local_skb[i] = NULL;
        len = rx_node->local_desc->header.usr_field[i].skb_len;
        skb_put(skb, len);
        dev_id = weth_tab_set_userfield(rx_node, skb, i);
        cb_map = (struct wan_info_s *)&skb->cb;

        if (cb_map->userfield1 != 0) {
            weth_rx_debug_data(skb, cb_map);
            continue;
        }
        /* if dev id is change, upload the skb */
        if (skb_tail != NULL && dev_id != dev_id_old) {
            g_weth_tab_ctx.rx_cb(dev_id_old, skb_tail, rx_pkg);
            skb_tail = NULL;
            rx_pkg = 0;
        }
        dev_id_old = dev_id;
        skb_onelink_tail(&skb_tail, skb);
        rx_pkg++;
    }
    /* upload last skb list to dev */
    if (likely(skb_tail != NULL)) {
        g_weth_tab_ctx.rx_cb(dev_id_old, skb_tail, rx_pkg);
    } else {
        g_weth_tab_ctx.skb_tail_null++;
    }

    /* schedule workqueue to refill skb */
    queue_delayed_work(g_weth_tab_ctx.refill_queue, &rx_node->refill_work, 0);

    /* send event to RC */
    ret = weth_send_event(num, rx_node->idx, WETH_RX_DMA_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__); /* check ret */
    }

    return;
}

STATIC void weth_tab_tx_data_complete(void *para)
{
    int ret;
    int num;
    int port_no;
    int i;
    unsigned long long cur_phy;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *tx_node = (struct weth_tab_node *)req->priv;

    g_weth_tab_ctx.tx_stat.stat_tx_data_dma_done++;
    /* dma_unmap_single peer to start tx dma */
    num = tx_node->cur_num;
    port_no = tx_node->cur_port_no;

    for (i = 0; i < num; i++) {
        cur_phy = tx_node->local_desc->data_desc[i].sar_high;
        cur_phy = (cur_phy << 32) | tx_node->local_desc->data_desc[i].sar_low; /* get higher 32 bits */
        if (g_by_pass_mode == 0) {
            dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)cur_phy, tx_node->local_skb[i]->len,
                             DMA_TO_DEVICE);
        }

        g_packet_dump.free_packets++;
        __weth_ep_free_skb(tx_node->local_skb[i]);

        tx_node->local_skb[i] = NULL;
    }

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)tx_node->local_usr_field_phy,
                     sizeof(struct weth_tab_usr_field) * num, DMA_TO_DEVICE);

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)tx_node->local_data_desc_phy, WETH_GET_DESC_SIZE(num + 2),
                     DMA_TO_DEVICE);

    /* invalid cache for refill */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_data_desc, WETH_GET_DESC_SIZE(num + 2), DMA_FROM_DEVICE);

    /* send event to RC */
    ret = weth_send_event_by_port(port_no, num, tx_node->idx, WETH_TX_DMA_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__); /* check ret */
    }
}

STATIC void weth_tab_table_complete(void *para)
{
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *rx_node = (struct weth_tab_node *)req->priv;
    int ret;

    rx_node->data_req.port_no = 0;
    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)rx_node->local_data_desc_phy,
                     WETH_GET_DESC_SIZE(rx_node->cur_num + 2), DMA_FROM_DEVICE);
    /* start data dma */
    ret = weth_dma_submit_req(&rx_node->data_req, WETH_DMA_READ);
    if (ret) {
        weth_pr_err("submit req fail:%d, line:%d\n", ret, __LINE__); /* check ret */

        /* send irq to rc */
    }
}

STATIC void weth_tab_rx_sync_table_complete(void *para)
{
    int ret;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *node = (struct weth_tab_node *)req->priv;

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)virt_to_phys(node->local_desc),
                     sizeof(struct weth_tab_element), DMA_TO_DEVICE);

    ret = weth_send_event(0, node->idx, WETH_SYNC_RX_TABLE_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__);
    }
}

STATIC void weth_tab_tx_sync_table_complete(void *para)
{
    int ret;
    struct weth_dma_req *req = (struct weth_dma_req *)para;
    struct weth_tab_node *node = (struct weth_tab_node *)req->priv;

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)virt_to_phys(node->local_desc),
                     sizeof(struct weth_tab_element), DMA_FROM_DEVICE);

    ret = weth_send_event(0, node->idx, WETH_SYNC_TX_TABLE_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__);
    }
}

int weth_tab_node_local_init(struct weth_tab_node *node)
{
    int size;
    void *p = NULL;

    /* alloc node mem */
    size = ALIGN(sizeof(struct weth_tab_element), WETH_ALIGN_NUM);
    p = kzalloc(size, GFP_KERNEL);
    if (p == NULL) {
        return -1;
    }
    node->local_org_desc = p;
    node->local_desc = p;

    /* init node member */
    node->local_desc->header.head_info.total = WETH_TAB_ELEMENT_CNT;
    node->local_table_desc = &node->local_desc->table_desc[0];
    node->local_table_desc_phy = (void *)virt_to_phys(node->local_table_desc);

    node->local_data_desc = &node->local_desc->header.head_info;
    node->local_data_desc_phy = (void *)virt_to_phys(node->local_data_desc);

    node->local_usr_field = (&node->local_desc->header.usr_field[0]);
    node->local_usr_field_phy = (void *)virt_to_phys(node->local_usr_field);

    return 0;
}

STATIC int weth_tab_init_node(struct weth_tab_node *node, int is_tx, unsigned int idx)
{
    int ret;

    INIT_DELAYED_WORK(&node->refill_work, weth_tab_refill_push);
    node->idx = idx;

    ret = weth_tab_node_local_init(node);
    if (ret != 0) {
        return -1;
    }

    /* init data req */
    if (is_tx) {
        node->data_req.cb = weth_tab_tx_data_complete;
    } else {
        node->data_req.cb = weth_tab_rx_data_complete;
    }

    node->data_req.priv = (void *)node;
    node->data_req.desc = (void *)((char *)node->local_data_desc_phy + sizeof(struct weth_tab_head_info));
    INIT_LIST_HEAD(&node->data_req.list);
    INIT_LIST_HEAD(&node->list);

    /* init table req */
    node->table_req.cb = weth_tab_table_complete;
    node->table_req.priv = (void *)node;
    node->table_req.desc = (void *)virt_to_phys(&node->local_desc->dma_table_desc[0]);
    INIT_LIST_HEAD(&node->table_req.list);

    if (is_tx) {
        node->init_table_req.cb = weth_tab_tx_sync_table_complete;
    } else {
        node->init_table_req.cb = weth_tab_rx_sync_table_complete;
    }
    node->init_table_req.priv = (void *)node;
    node->init_table_req.desc = node->local_table_desc_phy;
    INIT_LIST_HEAD(&node->init_table_req.list);

    if (is_tx) {
        node->refill_req.cb = weth_tab_tx_refill_complete;
    } else {
        node->refill_req.cb = weth_tab_rx_refill_complete;
    }
    node->refill_req.priv = (void *)node;
    node->refill_req.desc = node->local_table_desc_phy;
    INIT_LIST_HEAD(&node->refill_req.list);

    return 0;
}

STATIC void weth_tab_init_table_node(struct weth_tab_node *node, int is_write)
{
    /* init table desc */
    if (is_write) {
        bsp_pcie_set_last_data_element((void *)node->local_table_desc,
                                       node->local_desc->header.head_info.total * sizeof(struct pcie_dma_data_element) +
                                           sizeof(struct weth_tab_head_info),
                                       (u64)node->local_data_desc_phy, (u64)node->peer_data_desc_phy);
    } else {
        bsp_pcie_set_last_data_element((void *)node->local_table_desc,
                                       node->local_desc->header.head_info.total * sizeof(struct pcie_dma_data_element) +
                                           sizeof(struct weth_tab_head_info),
                                       (u64)node->peer_data_desc_phy, (u64)node->local_data_desc_phy);
    }

    bsp_pcie_set_link_element((void *)&node->local_desc->table_desc[1], 1, 0);

    /* flash cache for table desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, node->local_table_desc, sizeof(node->local_desc->table_desc),
                   DMA_TO_DEVICE);
}

STATIC void weth_tab_init_dma_table_node(struct weth_tab_node *node, int is_write)
{
    /* init table desc */
    if (is_write) {
        bsp_pcie_set_last_data_element((void *)&node->local_desc->dma_table_desc[0],
                                       node->local_desc->header.head_info.total * sizeof(struct pcie_dma_data_element) +
                                           sizeof(struct weth_tab_head_info),
                                       (u64)node->peer_data_desc_phy, (u64)node->local_data_desc_phy);
    } else {
        bsp_pcie_set_last_data_element((void *)&node->local_desc->dma_table_desc[0],
                                       node->local_desc->header.head_info.total * sizeof(struct pcie_dma_data_element) +
                                           sizeof(struct weth_tab_head_info),
                                       (u64)node->local_data_desc_phy, (u64)node->peer_data_desc_phy);
    }

    bsp_pcie_set_link_element((void *)&node->local_desc->dma_table_desc[1], 1, 0);

    /* flash cache for table desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, (void *)&node->local_desc->dma_table_desc[0],
                   sizeof(node->local_desc->dma_table_desc), DMA_TO_DEVICE);
}

void weth_tab_get_rx_peer_usr_field(void *handle, char **peer_usr_field)
{
    struct weth_tab_node *rx_node = (struct weth_tab_node *)handle;

    *peer_usr_field = rx_node->peer_usr_field_phy;
    return;
}

STATIC int weth_tab_init_skb(struct weth_tab_node *node)
{
    int i;
    struct sk_buff *skb = NULL;
    unsigned long long data_phy;
    unsigned int total = node->local_desc->header.head_info.total;

    for (i = 0; i < total; i++) {
        if (node->local_skb[i] == NULL) {
            /* alloc skb */
            if (g_by_pass_mode) {
                skb = bsp_dra_alloc_skb(BUF_SIZE_1_6KB, &data_phy);
            }  else {
                skb = __weth_alloc_skb(WETH_SKB_MTU);
            }

            if (skb == NULL) {
                weth_pr_err("__weth_alloc_skb fail, line:%d, total %d cur %d\n", __LINE__, total, i);
                return -1;
            }
            node->local_skb[i] = skb;
            /* invalid skb data cache */
            if (g_by_pass_mode) {
                data_phy = bsp_dra_to_phy(data_phy);
            } else {
                data_phy = dma_map_single(g_weth_tab_ctx.dev_for_map, skb->data, WETH_SKB_MTU, DMA_FROM_DEVICE);
            }

            /* fill the desc */
            bsp_pcie_set_dar_element((void*)&node->local_desc->data_desc[i], (u64)data_phy);
        }
    }

    /* flush cache for dst addr config */
    dma_map_single(g_weth_tab_ctx.dev_for_map, (void *)node->local_desc, sizeof(struct weth_tab_element),
                   DMA_TO_DEVICE);
    return 0;
}

STATIC void weth_tab_init_tx_desc(struct weth_tab_node *tx_node)
{
    /* invalid cache for recv downlink dst_addr to desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_desc, sizeof(struct weth_tab_element), DMA_FROM_DEVICE);
}

/*
 * set local table addr(phy) to peer
 *  tx_table
 *  rx_table
 */
void weth_tab_set_addr_to_bar(void *addr)
{
    int i, wait_count;
    int ret = -1;
    struct weth_tab_addr_table_ep* cur = (struct weth_tab_addr_table_ep*)addr;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        weth_tab_init_tx_desc(&tab_ctx->tx_node[i]);
        cur->tx_table[i].table_desc_phy = (unsigned long long)tab_ctx->tx_node[i].local_table_desc_phy;
        cur->tx_table[i].data_desc_phy = (unsigned long long)tab_ctx->tx_node[i].local_data_desc_phy;
        cur->tx_table[i].usr_field_phy = (unsigned long long)tab_ctx->tx_node[i].local_usr_field_phy;
    }

    tab_ctx->skb_init_fail = 0;
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        wait_count = 0;
        do {
            ret = weth_tab_init_skb(&tab_ctx->rx_node[i]);
            if (ret != 0) {
                wait_count++;
                weth_pr_err("skb init fail\n");
                msleep(10); /* sleep 10 miliseconds */
            }

            if (wait_count >= INIT_SKB_TIMEOUT) {
                tab_ctx->skb_init_fail = 1;
            }
        } while (ret != 0 && wait_count < INIT_SKB_TIMEOUT);

        cur->rx_table[i].table_desc_phy = (unsigned long long)tab_ctx->rx_node[i].local_table_desc_phy;
        cur->rx_table[i].data_desc_phy = (unsigned long long)tab_ctx->rx_node[i].local_data_desc_phy;
        cur->rx_table[i].usr_field_phy = (unsigned long long)tab_ctx->rx_node[i].local_usr_field_phy;
    }

    cur->is_init = WETH_TAB_SETING_FLAG;
    weth_pr_err("is_init set\n");

    return;
}

void weth_tab_wait_setting_done(void *addr)
{
    struct weth_tab_addr_table_ep* tab = (struct weth_tab_addr_table_ep*)addr;
    int wait_count = 0;

    while (tab->is_init != WETH_TAB_SETING_FLAG && wait_count < WAIT_PEER_SKB_INIT_TIMEOUT) {
        wait_count++;
        msleep(100); /* sleep 100 miliseconds */
    }

    if (g_weth_tab_ctx.skb_init_fail != 0 || wait_count == WAIT_PEER_SKB_INIT_TIMEOUT) {
        weth_pr_err("weth_tab_wait_setting_done timeout\n");
        system_error(DRV_ERRNO_WETH_SKBNULL, 0, 0, NULL, 0);
    }
    weth_pr_err("weth_tab_wait_setting_done ok\n");
    return;
}

int weth_tab_fill_all_desc(void)
{
    int i;
    int ret;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    /* sync table B -> A for downlink, B recv dst skb desc from A */
    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        ret = weth_dma_submit_req(&tab_ctx->tx_node[i].init_table_req, WETH_DMA_READ);
        if (ret < 0) {
            weth_pr_err("submit req fail:%d, line:%d\n", ret, __LINE__);
            return ret;
        }
    }

    /* sync table A -> B for uplink, B send dst skb desc to A */
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        ret = weth_dma_submit_req(&tab_ctx->rx_node[i].init_table_req, WETH_DMA_WRITE);
        if (ret < 0) {
            weth_pr_err("submit req fail:%d, line:%d\n", ret, __LINE__);
            return ret;
        }
    }

    return 0;
}

void weth_tab_get_addr_from_bar(void *addr)
{
    int i;
    int ret;
    struct weth_tab_addr_table_rc *cur = (struct weth_tab_addr_table_rc *)addr;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    /* A: rx_node <==> B: tx_node; rx_node >> dma_write */
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        tab_ctx->rx_node[i].peer_table_desc_phy = (void *)cur->tx_table[i].table_desc_phy;
        tab_ctx->rx_node[i].peer_data_desc_phy = (void *)cur->tx_table[i].data_desc_phy;
        tab_ctx->rx_node[i].peer_usr_field_phy = (void *)cur->tx_table[i].usr_field_phy;
        weth_tab_init_table_node(&tab_ctx->rx_node[i], 1);
        weth_tab_init_dma_table_node(&tab_ctx->rx_node[i], 1);
    }

    /* A: tx_node <==> B: rx_node; tx_node >> dma_read */
    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        tab_ctx->tx_node[i].peer_table_desc_phy = (void *)cur->rx_table[i].table_desc_phy;
        tab_ctx->tx_node[i].peer_data_desc_phy = (void *)cur->rx_table[i].data_desc_phy;
        tab_ctx->tx_node[i].peer_usr_field_phy = (void *)cur->rx_table[i].usr_field_phy;
        weth_tab_init_table_node(&tab_ctx->tx_node[i], 0);
        weth_tab_init_dma_table_node(&tab_ctx->tx_node[i], 0);
    }

    ret = weth_tab_fill_all_desc();
    if (ret) {
        weth_pr_err("weth_tab_fill_all_desc fail:%d, line:%d\n", ret, __LINE__);
    }

    return;
}

int weth_tab_get_tx_desc(void **handle, char **desc, char **usrfield, unsigned int *total)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    struct weth_tab_element *cur_desc = NULL;
    struct weth_tab_node *cur_node = NULL;
    unsigned long flags;

    spin_lock_irqsave(&tab_ctx->tx_lock, flags);
    if (list_empty(&tab_ctx->tx_list)) {
        spin_unlock_irqrestore(&tab_ctx->tx_lock, flags);
        return -1;
    }

    cur_node = list_entry(tab_ctx->tx_list.next, struct weth_tab_node, list);
    list_del(&cur_node->list);
    tab_ctx->free_tx_tab_num--;
    spin_unlock_irqrestore(&tab_ctx->tx_lock, flags);

    *handle = (void *)cur_node;
    cur_desc = cur_node->local_desc;
    *desc = (char *)&cur_desc->data_desc[0];
    *total = cur_desc->header.head_info.total;
    *usrfield = (char *)&cur_desc->header.usr_field[0];

    return 0;
}

int weth_tab_tx_refill_event(unsigned int idx, unsigned int num)
{
    int ret;
    struct weth_tab_node *tx_node = NULL;

    if (idx >= WETH_RC_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return -1;
    }

    g_weth_tab_ctx.tx_stat.stat_tx_refill++;
    tx_node = &g_weth_tab_ctx.tx_node[idx];
    tx_node->cur_num = num;
    ret = weth_dma_submit_req(&tx_node->refill_req, WETH_DMA_READ);
    if (ret) {
        weth_pr_err("submit req fail:%d, line:%d\n", ret, __LINE__); /* check ret */

        /* send irq to rc */
    }
    return ret;
}

int weth_tab_ep_start_tx_dma(void *handle, unsigned int num, int port_no)
{
    int ret;
    struct weth_tab_node *tx_node = (struct weth_tab_node *)handle;

    tx_node->local_desc->header.head_info.cur_num = num;
    tx_node->cur_num = num;
    tx_node->cur_port_no = port_no;
    tx_node->data_req.port_no = port_no;

    /* flush cache for usrfield */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_usr_field, sizeof(struct weth_tab_usr_field) * num,
                   DMA_TO_DEVICE);

    /* flush cache for desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_data_desc, WETH_GET_DESC_SIZE(num + 2), DMA_TO_DEVICE);

    g_weth_tab_ctx.tx_stat.stat_tx_data_dma++;
    ret = weth_dma_submit_req(&tx_node->data_req, WETH_DMA_WRITE);
    return ret;
}

int weth_tab_start_rx_data_dma(unsigned int idx, unsigned int num)
{
    int ret;
    struct weth_tab_node *rx_node = NULL;

    if (idx >= WETH_EP_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return -1;
    }

    rx_node = &g_weth_tab_ctx.rx_node[idx];
    rx_node->cur_num = num;

    ret = weth_dma_submit_req(&rx_node->data_req, WETH_DMA_READ);

    return ret;
}

int weth_tab_start_rx_table_dma(unsigned int idx, unsigned int num)
{
    struct weth_tab_node *rx_node = NULL;
    int ret;

    if (idx >= WETH_EP_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return -1;
    }

    rx_node = &g_weth_tab_ctx.rx_node[idx];
    rx_node->cur_num = num;
    rx_node->table_req.port_no = 0;

    /* table desc has flush cache in init */
    ret = weth_dma_submit_req(&rx_node->table_req, WETH_DMA_READ);
    return ret;
}

int weth_tab_get_status(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    return (tab_ctx->free_tx_tab_num == WETH_RC_RX_TAB_MAX_NUM) ? 0 : 1;
}

void weth_tab_save_tx_skb(void *handle, unsigned int cnt, struct sk_buff *skb)
{
    struct weth_tab_node *tx_node = (struct weth_tab_node *)handle;

    tx_node->local_skb[cnt] = skb;
    return;
}

void weth_tab_regist_rx_cb(weth_tab_rx_cb_t rx_cb)
{
    g_weth_tab_ctx.rx_cb = rx_cb;
    return;
}

int weth_tab_get_free_tab_num(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    return tab_ctx->free_tx_tab_num;
}

void weth_tab_free(struct weth_tab_ctx *tab_ctx)
{
    void *node = NULL;
    int i;

    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        node = tab_ctx->tx_node[i].local_org_desc;
        if (node != NULL) {
            kfree(node);
            tab_ctx->tx_node[i].local_org_desc = NULL;
        }
    }

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        node = tab_ctx->rx_node[i].local_org_desc;
        if (node != NULL) {
            kfree(node);
            tab_ctx->rx_node[i].local_org_desc = NULL;
        }
    }

    if (tab_ctx->refill_queue != NULL) {
        destroy_workqueue(tab_ctx->refill_queue);
        tab_ctx->refill_queue = NULL;
    }
}

int weth_tab_init(struct device *dev_for_map)
{
    int i, j;
    int ret;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    tab_ctx->dev_for_map = dev_for_map;
    INIT_LIST_HEAD(&tab_ctx->tx_list);
    INIT_LIST_HEAD(&tab_ctx->rx_list);

    /* alloc tx/rx element */
    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        ret = weth_tab_init_node(&tab_ctx->tx_node[i], 1, i);
        if (ret) {
            goto fail;
        }
        list_add_tail(&tab_ctx->tx_node[i].list, &tab_ctx->tx_list);
        tab_ctx->free_tx_tab_num++;
    }

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        ret = weth_tab_init_node(&tab_ctx->rx_node[i], 0, i);
        if (ret) {
            goto fail;
        }
        for (j = 0; j < WETH_TAB_ELEMENT_CNT; j++) {
            tab_ctx->rx_node[i].local_skb[j] = NULL;
        }
        list_add_tail(&tab_ctx->rx_node[i].list, &tab_ctx->rx_list);
        tab_ctx->free_rx_tab_num++;
    }

    tab_ctx->refill_queue = create_singlethread_workqueue("weth_refill");
    if (tab_ctx->refill_queue == NULL) {
        goto fail;
    }

    spin_lock_init(&tab_ctx->tx_lock);
    spin_lock_init(&tab_ctx->rx_lock);

    return 0;

fail:
    weth_tab_free(tab_ctx);
    return -1;
}

void weth_ep_dump_table_status(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    weth_pr_err("weth ep table status:\n");

    weth_pr_err("free_tx_tab_num:                   %d\n", tab_ctx->free_tx_tab_num);
    weth_pr_err("free_rx_tab_num:                   %d\n", tab_ctx->free_rx_tab_num);
    weth_pr_err("skb_tail_null:                   %d\n", tab_ctx->skb_tail_null);
    weth_pr_err("refill_skb_null:                   %d\n", tab_ctx->refill_skb_null);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
