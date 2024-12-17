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
#include <linux/list.h>
#include <linux/types.h>
#include <linux/time.h>

#include "wan_eth_event_buffer.h"
#include "product_config.h"
#include "wan_eth_common.h"
#include "wan_eth_maa.h"
#include "wan_eth_table.h"
#include "wan_eth_ep.h"

void weth_bps_dump(unsigned int devid)
{
    struct weth_ctx *wctx = weth_get_ctx(devid);
    static unsigned long long last_rx_bytes = 0;
    static unsigned long long last_tx_bytes = 0;


    if (wctx == NULL) {
        weth_pr_err("invalid weth id:%d\n", devid);
    }
    weth_pr_err("stat rx Mbps: %lld\n", (wctx->stats.stat_rx_bytes - last_rx_bytes) *
        BYTE_TO_BITS / WETH_SPEED_BIT_TO_MB);
    weth_pr_err("stat tx Mbps: %lld\n", (wctx->stats.stat_tx_bytes - last_tx_bytes) *
        BYTE_TO_BITS / WETH_SPEED_BIT_TO_MB);

    last_rx_bytes = wctx->stats.stat_rx_bytes;
    last_tx_bytes = wctx->stats.stat_tx_bytes;
}

void weth_ep_vote_dump(void)
{
    struct weth_ep_ctx *epctx = weth_get_ep_ctx();

    weth_pr_err("is_wakeup:                   %d\n", epctx->is_wakeup);
    weth_pr_err("stat_wait_vote_call:         %d\n", epctx->stat_wait_vote_call);
    weth_pr_err("stat_wait_vote_fail:         %d\n", epctx->stat_wait_vote_fail);
    weth_pr_err("stat_nowait_vote_call:       %d\n", epctx->stat_nowait_vote_call);
    weth_pr_err("stat_nowait_vote_fail:       %d\n", epctx->stat_nowait_vote_fail);
    weth_pr_err("stat_unvote_call:            %d\n", epctx->stat_unvote_call);
    weth_pr_err("stat_unvote_fail:            %d\n", epctx->stat_unvote_fail);

    weth_pr_err("last_total_tx_pkg:           %d\n", epctx->last_total_tx_pkg);
    weth_pr_err("cur_total_tx_pkg:            %d\n", epctx->cur_total_tx_pkg);
    weth_pr_err("last_total_rx_pkg:           %d\n", epctx->last_total_rx_pkg);
    weth_pr_err("cur_total_rx_pkg:            %d\n", epctx->cur_total_rx_pkg);

    weth_pr_err("no_pkg_count:                %lld\n", epctx->no_pkg_count);
    weth_pr_err("vote_lock_opt:                %d\n", epctx->vote_lock_opt);
    weth_pr_err("deep_sleep_vote:                %d\n", epctx->deep_sleep_vote);
}

void weth_ep_dump_init_status(void)
{
    struct weth_ep_ctx *epctx = weth_get_ep_ctx();

    weth_pr_err("\n init_status:\n");

    weth_pr_err("link:                     %d\n", epctx->link);
    weth_pr_err("local stat:             %d\n", epctx->local_stat->bit32);
    weth_pr_err("remote stat:            %d\n", epctx->remote_stat->bit32);

    weth_pr_err("bar_init_success:                      %d\n", epctx->init_stats.bar_init_success);
    weth_pr_err("bar_config_fail:                        %d\n", epctx->init_stats.bar_config_fail);
    weth_pr_err("dma_ll_init_fail:                       %d\n", epctx->init_stats.dma_ll_init_fail);
    weth_pr_err("dma_ll_addr_alloc_fail:             %d\n", epctx->init_stats.dma_ll_addr_alloc_fail);
    weth_pr_err("dma_ll_addr_init_success:         %d\n", epctx->init_stats.dma_ll_addr_init_success);
    weth_pr_err("msi_intr_init_fail:                     %d\n", epctx->init_stats.msi_intr_init_fail);
    weth_pr_err("pcie_desc_init_fail:                   %d\n", epctx->init_stats.pcie_desc_init_fail);
    weth_pr_err("pcie_dma_callback_register_fail: %d\n", epctx->init_stats.pcie_dma_callback_register_fail);
    weth_pr_err("tab_init_fail: %d\n", epctx->init_stats.tab_init_fail);
    weth_pr_err("event_init_fail: %d\n", epctx->init_stats.event_init_fail);
}

void weth_ep_dump_tx_status(void)
{
    struct weth_ep_ctx *epctx = weth_get_ep_ctx();

    weth_pr_err("\n tx_status:\n");

    weth_pr_err("start_send:                     %lld\n", epctx->tx_stats.start_send);
    weth_pr_err("doing:                            %lld\n", epctx->tx_stats.doing);
    weth_pr_err("unhandle:                       %lld\n", epctx->tx_stats.unhandle);
    weth_pr_err("pending:                         %lld\n", epctx->tx_stats.pending);
    weth_pr_err("dma_send:                      %lld\n", epctx->tx_stats.dma_send);
    weth_pr_err("dma_send_packets:          %lld\n", epctx->tx_stats.dma_send_packets);
    weth_pr_err("dma_transfer_fail:            %lld\n", epctx->tx_stats.dma_transfer_fail);
    weth_pr_err("dma_list_complete:          %lld\n", epctx->tx_stats.dma_list_complete);
    weth_pr_err("dma_list_uncomplete:      %lld\n", epctx->tx_stats.dma_list_uncomplete);
    weth_pr_err("skb_pending:      %lld\n", epctx->tx_stats.skb_pending);
    weth_pr_err("remote_eth_fail:              %lld\n", epctx->tx_stats.remote_eth_fail);
    weth_pr_err("remote_eth_drop:            %lld\n", epctx->tx_stats.remote_eth_drop);
    weth_pr_err("skb_free:                        %lld\n", epctx->tx_stats.skb_free);
    weth_pr_err("skb_pending_free:           %lld\n", epctx->tx_stats.skb_pending_free);
    weth_pr_err("send_data_complete:        %lld\n", epctx->tx_stats.send_data_complete);
    weth_pr_err("dma_write_callback:         %lld\n", epctx->tx_stats.dma_write_callback);
    weth_pr_err("msi_send:                       %lld\n", epctx->tx_stats.msi_send);
    weth_pr_err("no_capacity:                    %lld\n", epctx->tx_stats.no_capacity);
    weth_pr_err("send_pending:                    %lld\n", epctx->tx_stats.send_pending);
    weth_pr_err("reschedule:                    %lld\n", epctx->tx_stats.reschedule);
    weth_pr_err("data_send_irq:                    %lld\n", epctx->tx_stats.data_send_irq);
    weth_pr_err("tx_queue_full:                    %lld\n", epctx->tx_stats.tx_queue_full);
    weth_pr_err("dma_send_pending:                    %lld\n", epctx->tx_stats.dma_send_pending);
    weth_pr_err("skb_free_due_to_tdfull:                    %lld\n", epctx->tx_stats.skb_free_due_to_tdfull);
    weth_pr_err("send_pending_write_cb:                    %lld\n", epctx->tx_stats.send_pending_write_cb);
    weth_pr_err("send_pending_resend:                    %lld\n", epctx->tx_stats.send_pending_resend);
    weth_pr_err("dma_resend:                    %lld\n", epctx->tx_stats.dma_resend);
    weth_pr_err("remote_eth_send:                    %lld\n", epctx->tx_stats.remote_eth_send);
    weth_pr_err("ep_link_not_up_drop:                    %lld\n", epctx->tx_stats.ep_link_not_up_drop);
    weth_pr_err("dmasend_drop:                    %lld\n", epctx->tx_stats.dmasend_drop);
    weth_pr_err("dl_speed:                    %lld\n", epctx->dl_speed);
    weth_pr_err("ul_speed:                    %lld\n", epctx->ul_speed);
}

void weth_ep_dump_rx_status(void)
{
    struct weth_ep_ctx *epctx = weth_get_ep_ctx();

    weth_pr_err("\n rx_status:\n");

    weth_pr_err("dma_rx_start:                    %lld\n", epctx->rx_stats.dma_rx_start);
    weth_pr_err("packets_received:                    %lld\n", epctx->rx_stats.packets_received);
    weth_pr_err("bytes_received:                    %lld\n", epctx->rx_stats.bytes_received);
    weth_pr_err("rx_done_cb:                    %lld\n", epctx->rx_stats.rx_done_cb);
    weth_pr_err("peer_dma_tx_complete:                    %lld\n", epctx->rx_stats.peer_dma_tx_complete);
    weth_pr_err("dma_read_callback:                    %lld\n", epctx->rx_stats.dma_read_callback);
    weth_pr_err("msi_send:                    %lld\n", epctx->rx_stats.msi_send);
    weth_pr_err("no_packets_received:                    %lld\n", epctx->rx_stats.no_packets_received);
    weth_pr_err("data_read_irq:                    %lld\n", epctx->rx_stats.data_read_irq);
    weth_pr_err("rx_try:                    %lld\n", epctx->rx_stats.rx_try);
    weth_pr_err("rx_skb_refill_fail:                    %lld\n", epctx->rx_stats.rx_skb_refill_fail);
    weth_pr_err("ep_td_available:                    %lld\n", epctx->rx_stats.ep_td_available);
    weth_pr_err("rx_buget_reach:                    %lld\n", epctx->rx_stats.rx_buget_reach);
    weth_pr_err("ep_td_available_rx_buget:                    %lld\n", epctx->rx_stats.ep_td_available_rx_buget);
    weth_pr_err("rx_queue_full:                    %lld\n", epctx->rx_stats.rx_queue_full);
    weth_pr_err("rx_wpos:                    %d\n", epctx->rx_wpos);
    weth_pr_err("rx_rpos:                    %d\n", epctx->rx_rpos);
    weth_pr_err("rx_num:                    %d\n", epctx->rx_num);
}

void weth_packet_dump(void)
{
    struct weth_packet_dump *packet_dump = &g_packet_dump;

    weth_pr_err("\n weth_packet_dump:\n");

    weth_pr_err("sent_packets:                    %d\n", packet_dump->sent_packets);
    weth_pr_err("free_packets:                    %d\n", packet_dump->free_packets);
    weth_pr_err("maa_free:                    %d\n", packet_dump->maa_free);
    weth_pr_err("kernel_free:                    %d\n", packet_dump->kernel_free);
}

void weth_event_tx_stats(void)
{
    struct event_buff_ctx *epctx = weth_get_event_buff_ctx();

    weth_pr_err("\n event_info:\n");

    weth_pr_err("send_event:             %d\n", epctx->tx_stats.send_event);
    weth_pr_err("event_buffer_full:             %d\n", epctx->tx_stats.event_buffer_full);
    weth_pr_err("send_irq:             %d\n", epctx->tx_stats.send_irq);
    weth_pr_err("event_buffer_nearly_full:             %d\n", epctx->tx_stats.event_buffer_nearly_full);
    weth_pr_err("invalid_tx_wp:             %d\n", epctx->tx_stats.invalid_tx_wp);
    weth_pr_err("link_up:             %d\n", epctx->tx_stats.event_count[WETH_LINK_UP]);
    weth_pr_err("link_down:             %d\n", epctx->tx_stats.event_count[WETH_LINK_DOWN]);
    weth_pr_err("config_dma:             %d\n", epctx->tx_stats.event_count[WETH_CONFIG_DMA]);
    weth_pr_err("tx_dma_complete:             %d\n", epctx->tx_stats.event_count[WETH_TX_DMA_COMPLETE]);
    weth_pr_err("rx_dma_complete:             %d\n", epctx->tx_stats.event_count[WETH_RX_DMA_COMPLETE]);
    weth_pr_err("refill_complete:             %d\n", epctx->tx_stats.event_count[WETH_REFILL_COMPLETE]);
    weth_pr_err("sync_table:             %d\n", epctx->tx_stats.event_count[WETH_SYNC_TABLE]);
    weth_pr_err("tx_table_complete:             %d\n", epctx->tx_stats.event_count[WETH_SYNC_TX_TABLE_COMPLETE]);
    weth_pr_err("rx_table_complete:             %d\n", epctx->tx_stats.event_count[WETH_SYNC_RX_TABLE_COMPLETE]);
}

void weth_event_rx_stats(void)
{
    struct event_buff_ctx *epctx = weth_get_event_buff_ctx();

    weth_pr_err("\n event_info:\n");

    weth_pr_err("handle_event_start:             %d\n", epctx->rx_stats.handle_event_start);
    weth_pr_err("invalid_event:             %d\n", epctx->rx_stats.invalid_event);
    weth_pr_err("invalid_index:             %d\n", epctx->rx_stats.invalid_index);
    weth_pr_err("event_handled:             %d\n", epctx->rx_stats.event_handled);
    weth_pr_err("link_up:             %d\n", epctx->rx_stats.event_count[WETH_LINK_UP]);
    weth_pr_err("link_down:             %d\n", epctx->rx_stats.event_count[WETH_LINK_DOWN]);
    weth_pr_err("config_dma:             %d\n", epctx->rx_stats.event_count[WETH_CONFIG_DMA]);
    weth_pr_err("tx_dma_complete:             %d\n", epctx->rx_stats.event_count[WETH_TX_DMA_COMPLETE]);
    weth_pr_err("rx_dma_complete:             %d\n", epctx->rx_stats.event_count[WETH_RX_DMA_COMPLETE]);
    weth_pr_err("refill_complete:             %d\n", epctx->rx_stats.event_count[WETH_REFILL_COMPLETE]);
    weth_pr_err("sync_table:             %d\n", epctx->rx_stats.event_count[WETH_SYNC_TABLE]);
    weth_pr_err("tx_table_complete:             %d\n", epctx->rx_stats.event_count[WETH_SYNC_TX_TABLE_COMPLETE]);
    weth_pr_err("rx_table_complete:             %d\n", epctx->rx_stats.event_count[WETH_SYNC_RX_TABLE_COMPLETE]);
}

unsigned char g_weth_skb_header[] = {
    0x00, 0x11, 0x09, 0x64, 0x04, 0x01, 0x00, 0x11, 0x09, 0x64, 0x00, 0x01, 0x08, 0x00,
};

int g_weth_test_times = 1000;
int g_weth_test_count = 100;

struct hrtimer g_weth_test_tx_timer;
struct sk_buff *g_weth_test_skb = NULL;

STATIC enum hrtimer_restart weth_test_timer_func(struct hrtimer *timer)
{
    struct sk_buff *nskb = NULL;
    struct weth_ctx *wctx = weth_get_ctx(0);
    int i, ret;

    if (g_weth_test_skb == NULL) {
        g_weth_test_skb = netdev_alloc_skb_ip_align(NULL, WETH_SKB_MTU);
        if (g_weth_test_skb == NULL) {
            weth_pr_err("skb alloc failed\n");
            return HRTIMER_NORESTART;
        }

        ret = memcpy_s(g_weth_test_skb->data, WETH_SKB_MTU, g_weth_skb_header,
                       min((u32)WETH_SKB_MTU, (u32)sizeof(g_weth_skb_header)) - 1);
        if (ret) {
            weth_pr_err("memcpy_s failed\n");
        }
        skb_put(g_weth_test_skb, WETH_MAX_SKB_LEN);
    }

    for (i = 0; i < g_weth_test_count; i++) {
        nskb = skb_clone(g_weth_test_skb, GFP_ATOMIC);
        if (nskb == NULL) {
            weth_pr_err("skb_clone fail! \n");
            break;
        }

        (void)weth_xmit(nskb, wctx->ndev);
    }

    if (g_weth_test_times-- <= 0) {
        __weth_ep_free_skb(g_weth_test_skb);
        g_weth_test_skb = NULL;
        return HRTIMER_NORESTART;
    } else {
        return HRTIMER_RESTART;
    }
}

int weth_test_start(int count, int times, int duration)
{
    g_weth_test_times = times;
    g_weth_test_count = count;

    if (g_weth_test_skb == NULL) {
        hrtimer_init(&g_weth_test_tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        g_weth_test_tx_timer.function = weth_test_timer_func;
        hrtimer_start(&g_weth_test_tx_timer, ktime_set(0, duration * WETH_MACROSECOND_TO_MILLISECOND),
            HRTIMER_MODE_REL);
        weth_pr_err("g_weth_test_tx_timer started!\n");
    } else {
        weth_pr_err("test have been started.");
        return -1;
    }
    return 0;
}

void weth_test_func(int count)
{
    struct sk_buff *nskb = NULL;
    struct weth_ctx *wctx = weth_get_ctx(0);
    int i;
    int ret;

    if (g_weth_test_skb == NULL) {
        g_weth_test_skb = netdev_alloc_skb_ip_align(NULL, WETH_SKB_MTU);
        if (g_weth_test_skb == NULL) {
            weth_pr_err("skb alloc failed\n");
            return;
        }

        ret = memcpy_s(g_weth_test_skb->data, WETH_SKB_MTU, g_weth_skb_header, min((u32)WETH_SKB_MTU, (u32)sizeof(g_weth_skb_header)));
        if (ret) {
            weth_pr_err("memcpy_s failed\n");
        }
        skb_put(g_weth_test_skb, WETH_MAX_SKB_LEN);
    }

    for (i = 0; i < count; i++) {
        nskb = skb_clone(g_weth_test_skb, GFP_ATOMIC);
        if (nskb == NULL) {
            weth_pr_err("skb_clone fail! \n");
            break;
        }

        (void)weth_xmit(nskb, wctx->ndev);
    }
}

int g_weth_perf_enable = 0;
int weth_perf_test(int us, int count)
{
    g_weth_perf_enable = 1;
    while (g_weth_perf_enable) {
        weth_test_func(count);

        udelay(us);
    }
    return 0;
}

int weth_perf_test_stop(int en)
{
    g_weth_perf_enable = en;
    return 0;
}

struct sk_buff *make_skb_list(int num)
{
    struct sk_buff *first_skb = NULL;
    struct sk_buff *skb = NULL;
    int i;

    skb = netdev_alloc_skb_ip_align(NULL, WETH_SKB_MTU);
    if (skb == NULL) {
        weth_pr_err("skb is null \n");
        return NULL;
    }

    skb_put(skb, WETH_MAX_SKB_LEN);
    first_skb = skb;

    for (i = 0; i < num - 1; i++) {
        skb->next = netdev_alloc_skb_ip_align(NULL, WETH_SKB_MTU);
        if (!skb->next) {
            weth_pr_err("skb->next is null \n");
            return NULL;
        }

        skb_put(skb->next, WETH_MAX_SKB_LEN);
        skb = skb->next;
    }

    skb->next = first_skb;

    return skb;
}

void send_packets_best_effort(int packet_num, int test_count)
{
    struct sk_buff *skb = NULL;
    int i;

    for (i = 0; i < test_count; i++) {
        skb = make_skb_list(packet_num);
        if (skb == NULL) {
            weth_pr_err("make_skb_list fail\n");
            return;
        }

        weth_ep_submit_tx(skb, packet_num, WETH_DEBUG_PORT_NO);
    }
}

