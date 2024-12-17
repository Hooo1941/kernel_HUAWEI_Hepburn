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
#include <uapi/linux/if_ether.h>

#include "bsp_pcie.h"
#include "mdrv_remote_eth.h"
#include "wan_eth_table.h"
#include "wan_eth_common.h"
#include "wan_eth_event_buffer.h"
#include "bsp_dump.h"
#include "mdrv_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */

#define WETH_GET_DESC_SIZE(num) ((num) * sizeof(struct pcie_dma_data_element) + sizeof(struct weth_tab_head_info))
#define STORED_TAB 128
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

    struct delayed_work refill_work;
    struct tasklet_struct refill_push;
    int idx;
    unsigned int cur_num;
    int init_table_done;
};

struct weth_table_store {
    int idx;
    unsigned int cur_num;
    int cpu;
};

struct weth_tab_ctx {
    struct list_head tx_list;
    struct list_head rx_list;
    struct device *dev_for_map;
    enum weth_pcie_mode mode;

    struct weth_table_store tab_store[STORED_TAB];
    int tab_stored_index;
    struct weth_tab_node tx_node[WETH_EP_RX_TAB_MAX_NUM];
    spinlock_t tx_lock;

    struct weth_tab_node rx_node[WETH_RC_RX_TAB_MAX_NUM];
    spinlock_t rx_lock;

    weth_tab_rx_cb_t rx_cb;
    weth_tab_table_sync_finish_cb_t table_sync_finish_cb;
    struct workqueue_struct *refill_queue;
    unsigned int free_tx_tab_num;
    unsigned int free_rx_tab_num;
    atomic_t finished_tx_tab_num;
    atomic_t finished_rx_tab_num;
    unsigned int skb_tail_null;
    unsigned int refill_skb_null;
    int skb_init_fail;
};

struct weth_tab_ctx g_weth_tab_ctx = {{0}};
struct weth_tab_ctx *weth_get_tab_ctx(void)
{
    return &g_weth_tab_ctx;
}

STATIC unsigned int weth_tab_set_userfield(struct weth_tab_node *node, struct sk_buff *skb, int index)
{
    struct remote_eth_cb_map_s *cb_map = NULL;

    cb_map = (struct remote_eth_cb_map_s *)&skb->cb;
    cb_map->userfield0 = node->local_desc->header.usr_field[index].usr_field0;
    cb_map->userfield1 = node->local_desc->header.usr_field[index].usr_field1;
    cb_map->userfield2 = node->local_desc->header.usr_field[index].usr_field2;
    if (cb_map->userfield1 == WETH_EP_WAKEUP_FLAG) {
        weth_print_pkt_info((unsigned char *)skb->data);
    }
    return (cb_map->userfield0 & 0xFFFF);
}

STATIC void weth_tab_refill_push(struct work_struct *work)
{
    struct weth_tab_node *rx_node = container_of(work, struct weth_tab_node, refill_work.work);
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    unsigned int num;
    int i;
    struct sk_buff *skb = NULL;
    void *data_phy = NULL;
    int ret;

    num = rx_node->cur_num;
    for (i = 0; i < num; i++) {
        if (rx_node->local_skb[i] == NULL) {
            skb = __weth_alloc_skb(WETH_SKB_MTU);
            if (skb == NULL) {
                tab_ctx->refill_skb_null++;
                weth_pr_err("__weth_alloc_skb fail, line:%d\n", __LINE__); /* check ret */
                queue_delayed_work(tab_ctx->refill_queue, &rx_node->refill_work, 10); /* restart work in 10 jiffies */
                return;
            }
            rx_node->local_skb[i] = skb;
            /* invalid skb data cache */
            data_phy = (void *)dma_map_single(tab_ctx->dev_for_map, skb->data, WETH_SKB_MTU, DMA_FROM_DEVICE);
            /* fill the desc */
            bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i], (u64)data_phy);
        }
    }

    /* restore  last 2 skb from usrfield and link */
    if (!rx_node->local_skb[i] || !rx_node->local_skb[i + 1]) {
        weth_pr_err("skb null during refill\n");
        return;
    }
    bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i],
                             (u64)virt_to_phys(rx_node->local_skb[i]->data));
    bsp_pcie_set_dar_element((void *)&rx_node->local_desc->data_desc[i + 1],
                             (u64)virt_to_phys(rx_node->local_skb[i + 1]->data));
    /* flush dst addr to local desc, no unmap here */
    dma_map_single(g_weth_tab_ctx.dev_for_map, rx_node->local_data_desc, WETH_GET_DESC_SIZE(num + 2), DMA_TO_DEVICE);
    /* config refill dma req */
    ret = weth_send_event(num, rx_node->idx, WETH_REFILL_COMPLETE, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail\n"); /* check ret */
    }

    return;
}

void weth_tab_tx_complete(unsigned int idx, unsigned int num)
{
    int i;
    struct weth_tab_node *tx_node = NULL;

    if (idx >= WETH_EP_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return;
    }

    tx_node = &g_weth_tab_ctx.tx_node[idx];

    for (i = 0; i < num; i++) {
        __weth_free_skb(tx_node->local_skb[i]);
        tx_node->local_skb[i] = NULL;
    }
    return;
}

STATIC int weth_tab_init_node(struct weth_tab_node *node, int is_tx, unsigned int idx)
{
    int size;
    void *p = NULL;

    INIT_LIST_HEAD(&node->list);
    INIT_DELAYED_WORK(&node->refill_work, weth_tab_refill_push);
    node->idx = idx;

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

int weth_tab_init_skb(struct weth_tab_node *node)
{
    int i;
    struct sk_buff *skb = NULL;
    dma_addr_t data_phy;
    unsigned int total = node->local_desc->header.head_info.total;

    for (i = 0; i < total; i++) {
        if (node->local_skb[i] == NULL) {
            /* alloc skb */
            skb = __weth_alloc_skb(WETH_SKB_MTU);
            if (skb == NULL) {
                weth_pr_err("__weth_alloc_skb fail, line:%d\n", __LINE__);
                return -1;
            }

            *(unsigned int*)skb->data = 0x5a5a5a5a;
            node->local_skb[i] = skb;
            /* invalid skb data cache */
            data_phy = dma_map_single(g_weth_tab_ctx.dev_for_map, skb->data, WETH_SKB_MTU, DMA_FROM_DEVICE);

            /* fill the desc */
            bsp_pcie_set_dar_element((void*)&node->local_desc->data_desc[i], (u64)data_phy);
        }
    }

    /* flush cache for dst addr config */
    dma_map_single(g_weth_tab_ctx.dev_for_map, node->local_desc, sizeof(struct weth_tab_element), DMA_TO_DEVICE);

    /* invalide cache for recv dst addr */
    dma_map_single(g_weth_tab_ctx.dev_for_map, node->local_desc, sizeof(struct weth_tab_element), DMA_FROM_DEVICE);
    return 0;
}

STATIC void weth_tab_init_tx_desc(struct weth_tab_node *tx_node)
{
    /* invalid cache for recv downlink dst_addr to desc */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_desc, sizeof(struct weth_tab_element), DMA_FROM_DEVICE);
}

STATIC void weth_tab_update_table_sync_status(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    if (atomic_read(&tab_ctx->finished_rx_tab_num) == WETH_RC_RX_TAB_MAX_NUM &&
        atomic_read(&tab_ctx->finished_tx_tab_num) == WETH_EP_RX_TAB_MAX_NUM) {
        tab_ctx->table_sync_finish_cb();
        atomic_set(&tab_ctx->finished_tx_tab_num, 0);
        atomic_set(&tab_ctx->finished_rx_tab_num, 0);
    }
}

void weth_tab_sync_rx_table_complete(unsigned int idx)
{
    struct weth_tab_node *node = NULL;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    if (idx >= WETH_EP_RX_TAB_MAX_NUM) {
        return;
    }

    node = &tab_ctx->tx_node[idx];
    atomic_add(1, &tab_ctx->finished_tx_tab_num);
    weth_pr_warn("tx idx: %d\n", idx);

    dma_unmap_single(g_weth_tab_ctx.dev_for_map, virt_to_phys(node->local_desc), sizeof(struct weth_tab_element),
                     DMA_FROM_DEVICE);

    weth_tab_update_table_sync_status();
}

void weth_tab_sync_tx_table_complete(unsigned int idx)
{
    struct weth_tab_node *node = NULL;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    if (idx >= WETH_RC_RX_TAB_MAX_NUM) {
        return;
    }

    node = &tab_ctx->rx_node[idx];
    atomic_add(1, &tab_ctx->finished_rx_tab_num);
    weth_pr_warn("rx idx: %d\n", idx);

    dma_unmap_single(tab_ctx->dev_for_map, virt_to_phys(node->local_desc), sizeof(struct weth_tab_element),
                     DMA_FROM_DEVICE);

    weth_tab_update_table_sync_status();
}

void weth_tab_deinit_skb(struct weth_tab_node *node)
{
    int i;
    struct sk_buff *skb = NULL;

    for (i = 0; i < node->local_desc->header.head_info.total; i++) {
        skb = node->local_skb[i];
        if (skb != NULL) {
            __weth_free_skb(skb);
            node->local_skb[i] = NULL;
        }
    }
    return;
}

int weth_tab_get_status(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    return (tab_ctx->free_tx_tab_num == WETH_EP_RX_TAB_MAX_NUM) ? 0 : 1;
}

/*
 * set local table addr(phy) to peer
 * rc table in bar:
 *  tx_table
 *  rx_table
 */
void weth_tab_set_addr_to_bar(void* addr)
{
    int i, wait_count;
    int ret = -1;
    struct weth_tab_addr_table_rc* cur = (struct weth_tab_addr_table_rc*)addr;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        weth_tab_init_tx_desc(&tab_ctx->tx_node[i]);
        cur->tx_table[i].table_desc_phy = (unsigned long long)tab_ctx->tx_node[i].local_table_desc_phy;
        cur->tx_table[i].data_desc_phy = (unsigned long long)tab_ctx->tx_node[i].local_data_desc_phy;
        cur->tx_table[i].usr_field_phy = (unsigned long long)tab_ctx->tx_node[i].local_usr_field_phy;
    }

    tab_ctx->skb_init_fail = 0;
    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        wait_count = 0;
        do {
            ret = weth_tab_init_skb(&tab_ctx->rx_node[i]);
            if (ret != 0) {
                wait_count++;
                weth_pr_err("skb init fail\n");
                msleep(10); /* sleep 10 miliseconds */
            }

            if (wait_count == INIT_SKB_TIMEOUT) {
                tab_ctx->skb_init_fail = 1;
            }
        } while (ret != 0 && wait_count < INIT_SKB_TIMEOUT);

        cur->rx_table[i].table_desc_phy = (unsigned long long)tab_ctx->rx_node[i].local_table_desc_phy;
        cur->rx_table[i].data_desc_phy = (unsigned long long)tab_ctx->rx_node[i].local_data_desc_phy;
        cur->rx_table[i].usr_field_phy = (unsigned long long)tab_ctx->rx_node[i].local_usr_field_phy;
    }

    if (tab_ctx->skb_init_fail == 0) {
        cur->is_init = WETH_TAB_SETING_FLAG;
        weth_pr_err("is_init set\n");
    } else {
        weth_pr_err("is_init not set\n");
    }

    return;
}

void weth_tab_wait_setting_done(void *addr)
{
    struct weth_tab_addr_table_rc *tab = (struct weth_tab_addr_table_rc *)addr;
    int wait_count = 0;

    while (tab->is_init != WETH_TAB_SETING_FLAG && wait_count < WAIT_PEER_SKB_INIT_TIMEOUT) {
        msleep(100); /* sleep 100 miliseconds */
        wait_count++;
    }

    if (wait_count < WAIT_PEER_SKB_INIT_TIMEOUT) {
        weth_pr_err("weth_tab_wait_setting_done OK\n");
    } else {
        weth_pr_err("weth_tab_wait_setting_done flag not set\n");
    }

    return;
}

void weth_tab_get_rx_peer_usr_field(void *handle, char **peer_usr_field)
{
    struct weth_tab_node *rx_node = (struct weth_tab_node *)handle;

    *peer_usr_field = rx_node->peer_usr_field_phy;
    return;
}

void weth_tab_get_addr_from_bar(void *addr)
{
    int i;
    struct weth_tab_addr_table_ep *cur = (struct weth_tab_addr_table_ep *)addr;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    /* A: rx_node <==> B: tx_node */
    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        tab_ctx->rx_node[i].peer_table_desc_phy = (void *)cur->tx_table[i].table_desc_phy;
        tab_ctx->rx_node[i].peer_data_desc_phy = (void *)cur->tx_table[i].data_desc_phy;
        tab_ctx->rx_node[i].peer_usr_field_phy = (void *)cur->tx_table[i].usr_field_phy;
    }

    /* A: tx_node <==> B: rx_node */
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        tab_ctx->tx_node[i].peer_table_desc_phy = (void *)cur->rx_table[i].table_desc_phy;
        tab_ctx->tx_node[i].peer_data_desc_phy = (void *)cur->rx_table[i].data_desc_phy;
        tab_ctx->tx_node[i].peer_usr_field_phy = (void *)cur->rx_table[i].usr_field_phy;
    }

    return;
}

/* handle is index */
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

void weth_tab_save_tx_skb(void *handle, unsigned int cnt, struct sk_buff *skb)
{
    struct weth_tab_node *tx_node = (struct weth_tab_node *)handle;

    tx_node->local_skb[cnt] = skb;
    return;
}

int weth_tab_put_tx_desc(unsigned int idx, unsigned int num)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    struct weth_tab_node *tx_node = NULL;
    unsigned long flags;

    if (idx >= WETH_EP_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return -1;
    }

    tx_node = &g_weth_tab_ctx.tx_node[idx];

    spin_lock_irqsave(&tab_ctx->tx_lock, flags);
    list_add_tail(&tx_node->list, &tab_ctx->tx_list);
    tab_ctx->free_tx_tab_num++;
    spin_unlock_irqrestore(&tab_ctx->tx_lock, flags);

    /* invalid cache for refill, size is follow ref to weth_tab_start_tx_data_dma */
    dma_unmap_single(g_weth_tab_ctx.dev_for_map, (dma_addr_t)tx_node->local_data_desc_phy, WETH_GET_DESC_SIZE(num + 2),
                     DMA_FROM_DEVICE);

    return 0;
}

int weth_tab_start_tx_data_dma(void *handle, unsigned int num)
{
    unsigned int size;
    int ret;
    struct weth_tab_node *tx_node = (struct weth_tab_node *)handle;

    tx_node->local_desc->header.head_info.cur_num = num;
    tx_node->cur_num = num;

    /* flush cache for usrfield */
    size = sizeof(struct weth_tab_usr_field) * num;
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_usr_field, size, DMA_TO_DEVICE);

    /* flush cache for tx dma */
    size = WETH_GET_DESC_SIZE(num + 2);
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_data_desc, size, DMA_TO_DEVICE);

    /* invalid cache for refill, size is follow */
    dma_map_single(g_weth_tab_ctx.dev_for_map, tx_node->local_data_desc, size, DMA_FROM_DEVICE);

    ret = weth_send_event(num, tx_node->idx, WETH_CONFIG_DMA, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__);
    }

    return ret;
}

STATIC void weth_tab_rx_data_done(struct weth_tab_node *rx_node, unsigned int num)
{
    int i;
    unsigned int len;
    struct sk_buff *skb_tail = NULL;
    struct sk_buff *skb = NULL;
    unsigned int dev_id;
    unsigned int dev_id_old = 0;
    unsigned int rx_pkg = 0;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    rx_node->cur_num = num;
    /* invalid usr field */
    dma_unmap_single(tab_ctx->dev_for_map, (dma_addr_t)rx_node->local_usr_field_phy,
                     sizeof(struct weth_tab_usr_field) * num, DMA_FROM_DEVICE);
    /* recv skb to weth eth */
    for (i = 0; i < num; i++) {
        skb = rx_node->local_skb[i];
        if (skb == NULL) {
            weth_pr_err("skb null in rx_data_done\n");
            return;
        }
        len = rx_node->local_desc->header.usr_field[i].skb_len;
        skb_put(skb, len);
        dev_id = weth_tab_set_userfield(rx_node, skb, i);

        dma_unmap_single(tab_ctx->dev_for_map, virt_to_phys(skb->data), skb->len, DMA_FROM_DEVICE);
        dev_id = weth_tab_set_userfield(rx_node, skb, i);
        /* if dev id is change, upload the skb */
        if (skb_tail != NULL && dev_id != dev_id_old) {
            tab_ctx->rx_cb(dev_id_old, skb_tail, rx_pkg);
            skb_tail = NULL;
            rx_pkg = 0;
        }
        dev_id_old = dev_id;
        skb_onelink_tail(&skb_tail, rx_node->local_skb[i]);
        rx_node->local_skb[i] = NULL;
        rx_pkg++;
    }

    if (likely(skb_tail != NULL)) {
        /* upload last skb list to dev */
        tab_ctx->rx_cb(dev_id_old, skb_tail, rx_pkg);
    } else {
        tab_ctx->skb_tail_null++;
    }

    /* schedule workqueue to refill skb */
    queue_delayed_work(tab_ctx->refill_queue, &rx_node->refill_work, 0);
}

void weth_tab_get_rx_desc_by_idx(unsigned int idx, unsigned int num)
{
    if (idx >= WETH_RC_RX_TAB_MAX_NUM || num >= WETH_TAB_ELEMENT_CNT) {
        return;
    }

    weth_tab_rx_data_done(&g_weth_tab_ctx.rx_node[idx], num);

    return;
}

void weth_tab_regist_rx_cb(weth_tab_rx_cb_t rx_cb)
{
    g_weth_tab_ctx.rx_cb = rx_cb;
    return;
}

void weth_tab_regist_table_sync_cb(weth_tab_table_sync_finish_cb_t table_sync_finish_cb)
{
    g_weth_tab_ctx.table_sync_finish_cb = table_sync_finish_cb;
    return;
}

int weth_tab_get_free_tab_num(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    return tab_ctx->free_tx_tab_num;
}

void weth_tab_clear_local_skb(void *handle, unsigned int cnt)
{
    int i;
    struct weth_tab_node *tx_node = (struct weth_tab_node *)handle;

    for (i = 0; i < cnt; i++) {
        tx_node->local_skb[i] = NULL;
    }
    return;
}

void weth_tab_reinit(void)
{
    int i, j;
    unsigned int total_skb_num;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;
    struct weth_tab_node *tx_node = NULL;

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        tx_node = &tab_ctx->tx_node[i];
        total_skb_num = tx_node->local_desc->header.head_info.total;
        for (j = 0; j < total_skb_num; j++) {
            if (tx_node->local_skb[j]) {
                __weth_free_skb(tx_node->local_skb[j]);
                tx_node->local_skb[j] = NULL;
            }
        }
    }

    INIT_LIST_HEAD(&tab_ctx->tx_list);
    INIT_LIST_HEAD(&tab_ctx->rx_list);

    tab_ctx->free_tx_tab_num = 0;
    tab_ctx->free_rx_tab_num = 0;

    /* alloc tx/rx element */
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        list_add_tail(&tab_ctx->tx_node[i].list, &tab_ctx->tx_list);
        tab_ctx->free_tx_tab_num++;
    }

    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        list_add_tail(&tab_ctx->rx_node[i].list, &tab_ctx->rx_list);
        tab_ctx->free_rx_tab_num++;
    }

    return;
}

void weth_tab_exit(void)
{
    int i, j;
    unsigned int total_skb_num;
    struct weth_tab_node *tx_node = NULL;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        tx_node = &tab_ctx->tx_node[i];
        total_skb_num = tx_node->local_desc->header.head_info.total;
        for (j = 0; j < total_skb_num; j++) {
            if (tx_node->local_skb[j]) {
                __weth_free_skb(tx_node->local_skb[j]);
                tx_node->local_skb[j] = NULL;
            }
        }
    }

    return;
}

void weth_tab_free(struct weth_tab_ctx *tab_ctx)
{
    void *node = NULL;
    int i;

    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        node = tab_ctx->tx_node[i].local_org_desc;
        if (node != NULL) {
            kfree(node);
        }
    }

    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
        node = tab_ctx->rx_node[i].local_org_desc;
        if (node != NULL) {
            kfree(node);
        }
    }

    if (tab_ctx->refill_queue != NULL) {
        destroy_workqueue(tab_ctx->refill_queue);
    }
}

int weth_tab_init(struct device *dev_for_map)
{
    int i, j;
    int ret;
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    tab_ctx->tab_stored_index = 0;
    tab_ctx->dev_for_map = dev_for_map;
    INIT_LIST_HEAD(&tab_ctx->tx_list);
    INIT_LIST_HEAD(&tab_ctx->rx_list);

    /* alloc tx/rx element */
    for (i = 0; i < WETH_EP_RX_TAB_MAX_NUM; i++) {
        ret = weth_tab_init_node(&tab_ctx->tx_node[i], 1, i);
        if (ret) {
            goto fail;
        }
        list_add_tail(&tab_ctx->tx_node[i].list, &tab_ctx->tx_list);
        tab_ctx->free_tx_tab_num++;
    }

    for (i = 0; i < WETH_RC_RX_TAB_MAX_NUM; i++) {
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

void weth_rc_dump_table_status(void)
{
    struct weth_tab_ctx *tab_ctx = &g_weth_tab_ctx;

    weth_pr_err("weth rc table status:\n");

    weth_pr_err("free_tx_tab_num:                   %d\n", tab_ctx->free_tx_tab_num);
    weth_pr_err("free_rx_tab_num:                   %d\n", tab_ctx->free_rx_tab_num);
    weth_pr_err("finished_tx_tab_num:                   %d\n", tab_ctx->finished_tx_tab_num.counter);
    weth_pr_err("finished_rx_tab_num:                   %d\n", tab_ctx->finished_rx_tab_num.counter);
    weth_pr_err("skb_tail_null:                   %d\n", tab_ctx->skb_tail_null);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
