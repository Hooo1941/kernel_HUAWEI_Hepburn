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
#include "wan_eth_event_buffer.h"
#include "product_config.h"
#include "wan_eth_common.h"
#include "wan_eth_table.h"
#include "wan_eth_rc.h"

void weth_bps_dump(unsigned int devid)
{
    struct weth_ctx *priv = weth_get_ctx(devid);
    static unsigned long long last_rx_bytes = 0;
    static unsigned long long last_tx_bytes = 0;

    if (priv == NULL) {
        weth_pr_err("invalid weth id:%d\n", devid);
    }

    weth_pr_err("stat rx Mbps: %lld\n", (priv->stats.stat_rx_bytes - last_rx_bytes) *
        BYTE_TO_BITS / WETH_SPEED_BIT_TO_MB);
    weth_pr_err("stat tx Mbps: %lld\n", (priv->stats.stat_tx_bytes - last_tx_bytes) *
        BYTE_TO_BITS / WETH_SPEED_BIT_TO_MB);

    last_rx_bytes = priv->stats.stat_rx_bytes;
    last_tx_bytes = priv->stats.stat_tx_bytes;
}

void weth_rc_vote_dump(void)
{
    struct weth_rc_ctx *pctx = weth_get_rc_ctx();

    weth_pr_err("is_wakeup:                   %d\n", pctx->is_wakeup);
    weth_pr_err("stat_wait_vote_call:         %d\n", pctx->stat_wait_vote_call);
    weth_pr_err("stat_wait_vote_fail:         %d\n", pctx->stat_wait_vote_fail);
    weth_pr_err("stat_nowait_vote_call:       %d\n", pctx->stat_nowait_vote_call);
    weth_pr_err("stat_nowait_vote_fail:       %d\n", pctx->stat_nowait_vote_fail);
    weth_pr_err("stat_unvote_call:            %d\n", pctx->stat_unvote_call);
    weth_pr_err("stat_unvote_fail:            %d\n", pctx->stat_unvote_fail);

    weth_pr_err("last_total_tx_pkg:           %d\n", pctx->last_total_tx_pkg);
    weth_pr_err("cur_total_tx_pkg:            %d\n", pctx->cur_total_tx_pkg);
    weth_pr_err("last_total_rx_pkg:           %d\n", pctx->last_total_rx_pkg);
    weth_pr_err("cur_total_rx_pkg:            %d\n", pctx->cur_total_rx_pkg);

    weth_pr_err("no_pkg_count:                %lld\n", pctx->no_pkg_count);
    weth_pr_err("vote_lock_opt:                %d\n", pctx->vote_lock_opt);
    weth_pr_err("deep_sleep_vote:                %d\n", pctx->deep_sleep_vote);
}

void weth_rc_dump_common_info(void)
{
    struct common_info *comm_info = &g_common_info;

    weth_pr_err("\n weth_rc_dump_common_info:\n");

    weth_pr_err("link_status:             %d\n", comm_info->link_status);
    weth_pr_err("bar_access:             %d\n", comm_info->bar_access.counter);
    weth_pr_err("link_down_rx:             %d\n", comm_info->link_down_rx);
    weth_pr_err("link_down_tx:             %d\n", comm_info->link_down_tx);
    weth_pr_err("wait_count:             %d\n", comm_info->wait_count);
    weth_pr_err("init_finish:             %d\n", comm_info->init_finish);
    weth_pr_err("exit_finish:             %d\n", comm_info->exit_finish);
    weth_pr_err("wait_for_linkup:             %d\n", comm_info->wait_for_linkup);
    weth_pr_err("wait_init_finish:             %d\n", comm_info->wait_init_finish);
    weth_pr_err("table_sync_finish:             %d\n", comm_info->table_sync_finish);
    weth_pr_err("wait_table_finish:             %d\n", comm_info->wait_table_finish);
}

void weth_rc_dump_common_ctx(void)
{
    struct weth_common_ctx *comm_ctx = &g_common_ctx;

    weth_pr_err("\n weth_rc_dump_common_ctx:\n");

    weth_pr_err("dl_speed:             %lld\n", comm_ctx->dl_speed);
    weth_pr_err("ul_speed:             %lld\n", comm_ctx->ul_speed);
    weth_pr_err("speed_threshold:             %d\n", comm_ctx->speed_threshold);
    weth_pr_err("speed_up_threshold:             %d\n", comm_ctx->speed_up_threshold);
    weth_pr_err("speed_down_threshold:             %d\n", comm_ctx->speed_down_threshold);
    weth_pr_err("cur_speed_level:             %d\n", comm_ctx->cur_speed_level);
    weth_pr_err("low_speed_count:             %d\n", comm_ctx->low_speed_count);
}

void weth_rc_dump_init_status(void)
{
    struct weth_rc_ctx *pctx = weth_get_rc_ctx();

    weth_pr_err("\n init_status:\n");

    weth_pr_err("local stat:             %d\n", pctx->local_stat->bit32);
    weth_pr_err("remote stat:            %d\n", pctx->remote_stat->bit32);

    weth_pr_err("bar_init_success:                      %d\n", pctx->init_stats.bar_init_success);
    weth_pr_err("bar_config_fail:                        %d\n", pctx->init_stats.bar_config_fail);
    weth_pr_err("dma_ll_init_fail:                       %d\n", pctx->init_stats.dma_ll_init_fail);
    weth_pr_err("dma_ll_addr_alloc_fail:             %d\n", pctx->init_stats.dma_ll_addr_alloc_fail);
    weth_pr_err("dma_ll_addr_init_success:         %d\n", pctx->init_stats.dma_ll_addr_init_success);
    weth_pr_err("msi_intr_init_fail:                     %d\n", pctx->init_stats.msi_intr_init_fail);
    weth_pr_err("pcie_desc_init_fail:                   %d\n", pctx->init_stats.pcie_desc_init_fail);
    weth_pr_err("pcie_dma_callback_register_fail: %d\n", pctx->init_stats.pcie_dma_callback_register_fail);
    weth_pr_err("tab_init_fail: %d\n", pctx->init_stats.tab_init_fail);
    weth_pr_err("event_init_fail: %d\n", pctx->init_stats.event_init_fail);
}

void weth_rc_dump_tx_status(void)
{
    struct weth_rc_ctx *pctx = weth_get_rc_ctx();

    weth_pr_err("\n tx_status:\n");

    weth_pr_err("start_send:                     %lld\n", pctx->tx_stats.start_send);
    weth_pr_err("doing:                            %lld\n", pctx->tx_stats.doing);
    weth_pr_err("unhandle:                       %lld\n", pctx->tx_stats.unhandle);
    weth_pr_err("pending:                         %lld\n", pctx->tx_stats.pending);
    weth_pr_err("dma_send:                      %lld\n", pctx->tx_stats.dma_send);
    weth_pr_err("dma_send_packets:          %lld\n", pctx->tx_stats.dma_send_packets);
    weth_pr_err("dma_transfer_fail:            %lld\n", pctx->tx_stats.dma_transfer_fail);
    weth_pr_err("dma_list_complete:          %lld\n", pctx->tx_stats.dma_list_complete);
    weth_pr_err("dma_list_uncomplete:      %lld\n", pctx->tx_stats.dma_list_uncomplete);
    weth_pr_err("skb_pending:      %lld\n", pctx->tx_stats.skb_pending);
    weth_pr_err("remote_eth_fail:              %lld\n", pctx->tx_stats.remote_eth_fail);
    weth_pr_err("remote_eth_drop:            %lld\n", pctx->tx_stats.remote_eth_drop);
    weth_pr_err("skb_free:                        %lld\n", pctx->tx_stats.skb_free);
    weth_pr_err("skb_pending_free:           %lld\n", pctx->tx_stats.skb_pending_free);
    weth_pr_err("send_data_complete:        %lld\n", pctx->tx_stats.send_data_complete);
    weth_pr_err("dma_write_callback:         %lld\n", pctx->tx_stats.dma_write_callback);
    weth_pr_err("msi_send:                       %lld\n", pctx->tx_stats.msi_send);
    weth_pr_err("no_capacity:                    %lld\n", pctx->tx_stats.no_capacity);
    weth_pr_err("send_pending:                    %lld\n", pctx->tx_stats.send_pending);
    weth_pr_err("reschedule:                    %lld\n", pctx->tx_stats.reschedule);
    weth_pr_err("data_send_irq:                    %lld\n", pctx->tx_stats.data_send_irq);
    weth_pr_err("tx_queue_full:                    %lld\n", pctx->tx_stats.tx_queue_full);
    weth_pr_err("dma_send_pending:                    %lld\n", pctx->tx_stats.dma_send_pending);
    weth_pr_err("skb_free_due_to_tdfull:                    %lld\n", pctx->tx_stats.skb_free_due_to_tdfull);
    weth_pr_err("send_pending_write_cb:                    %lld\n", pctx->tx_stats.send_pending_write_cb);
    weth_pr_err("send_pending_resend:                    %lld\n", pctx->tx_stats.send_pending_resend);
    weth_pr_err("dma_resend:                    %lld\n", pctx->tx_stats.dma_resend);
}

void weth_rc_dump_rx_status(void)
{
    struct weth_rc_ctx *pctx = weth_get_rc_ctx();

    weth_pr_err("\n rx_status:\n");

    weth_pr_err("dma_rx_start:                    %lld\n", pctx->rx_stats.dma_rx_start);
    weth_pr_err("packets_received:                    %lld\n", pctx->rx_stats.packets_received);
    weth_pr_err("bytes_received:                    %lld\n", pctx->rx_stats.bytes_received);
    weth_pr_err("rx_done_cb:                    %lld\n", pctx->rx_stats.rx_done_cb);
    weth_pr_err("peer_dma_tx_complete:                    %lld\n", pctx->rx_stats.peer_dma_tx_complete);
    weth_pr_err("dma_read_callback:                    %lld\n", pctx->rx_stats.dma_read_callback);
    weth_pr_err("msi_send:                    %lld\n", pctx->rx_stats.msi_send);
    weth_pr_err("no_packets_received:                    %lld\n", pctx->rx_stats.no_packets_received);
    weth_pr_err("data_read_irq:                    %lld\n", pctx->rx_stats.data_read_irq);
    weth_pr_err("rx_try:                    %lld\n", pctx->rx_stats.rx_try);
    weth_pr_err("rx_skb_refill_fail:                    %lld\n", pctx->rx_stats.rx_skb_refill_fail);
    weth_pr_err("ep_td_available:                    %lld\n", pctx->rx_stats.ep_td_available);
    weth_pr_err("rx_buget_reach:                    %lld\n", pctx->rx_stats.rx_buget_reach);
    weth_pr_err("ep_td_available_rx_buget:                    %lld\n", pctx->rx_stats.ep_td_available_rx_buget);
    weth_pr_err("rx_queue_full:                    %lld\n", pctx->rx_stats.rx_queue_full);
    weth_pr_err("rx_wpos:                    %d\n", pctx->rx_wpos);
    weth_pr_err("rx_rpos:                    %d\n", pctx->rx_rpos);
    weth_pr_err("rx_num:                    %d\n", pctx->rx_num);
    weth_pr_err("invalid_devid:                    %d\n", pctx->invalid_devid);
    weth_pr_err("not_opened:                    %d\n", pctx->not_opened);
    weth_pr_err("init_change:             %d\n", pctx->init_change);
    weth_pr_err("first_change:             %d\n", pctx->first_change);
    weth_pr_err("second_change:             %d\n", pctx->second_change);
    weth_pr_err("third_change:             %d\n", pctx->third_change);
}

void weth_rc_dump_pm_status(void)
{
    struct weth_rc_ctx *pctx = weth_get_rc_ctx();

    weth_pr_err("\n pm_status:\n");

    weth_pr_err("suspend_fail:                    %d\n", pctx->pm_stats.suspend_fail);
    weth_pr_err("suspend_sucess:                    %d\n", pctx->pm_stats.suspend_sucess);
    weth_pr_err("resume_sucess:                    %d\n", pctx->pm_stats.resume_sucess);
    weth_pr_err("first_resume:                %d\n", pctx->first_resume);
    weth_pr_err("first_resume_called:                %d\n", pctx->first_resume_called);
    weth_pr_err("real_resume:                %d\n", pctx->real_resume);
}

void weth_event_tx_stats(void)
{
    struct event_buff_ctx *ectx = weth_get_event_buff_ctx();

    weth_pr_err("\n event_info:\n");

    weth_pr_err("send_event:             %d\n", ectx->tx_stats.send_event);
    weth_pr_err("event_buffer_full:             %d\n", ectx->tx_stats.event_buffer_full);
    weth_pr_err("send_irq:             %d\n", ectx->tx_stats.send_irq);
    weth_pr_err("event_buffer_nearly_full:             %d\n", ectx->tx_stats.event_buffer_nearly_full);
    weth_pr_err("invalid_tx_wp:             %d\n", ectx->tx_stats.invalid_tx_wp);
    weth_pr_err("link_up:             %d\n", ectx->tx_stats.event_count[WETH_LINK_UP]);
    weth_pr_err("link_down:             %d\n", ectx->tx_stats.event_count[WETH_LINK_DOWN]);
    weth_pr_err("config_dma:             %d\n", ectx->tx_stats.event_count[WETH_CONFIG_DMA]);
    weth_pr_err("tx_dma_complete:             %d\n", ectx->tx_stats.event_count[WETH_TX_DMA_COMPLETE]);
    weth_pr_err("rx_dma_complete:             %d\n", ectx->tx_stats.event_count[WETH_RX_DMA_COMPLETE]);
    weth_pr_err("refill_complete:             %d\n", ectx->tx_stats.event_count[WETH_REFILL_COMPLETE]);
    weth_pr_err("sync_table:             %d\n", ectx->tx_stats.event_count[WETH_SYNC_TABLE]);
    weth_pr_err("tx_table_complete:             %d\n", ectx->tx_stats.event_count[WETH_SYNC_TX_TABLE_COMPLETE]);
    weth_pr_err("rx_table_complete:             %d\n", ectx->tx_stats.event_count[WETH_SYNC_RX_TABLE_COMPLETE]);
}

void weth_event_rx_stats(void)
{
    struct event_buff_ctx *ectx = weth_get_event_buff_ctx();

    weth_pr_err("\n event_info:\n");

    weth_pr_err("handle_event_start:             %d\n", ectx->rx_stats.handle_event_start);
    weth_pr_err("invalid_event:             %d\n", ectx->rx_stats.invalid_event);
    weth_pr_err("invalid_index:             %d\n", ectx->rx_stats.invalid_index);
    weth_pr_err("event_handled:             %d\n", ectx->rx_stats.event_handled);
    weth_pr_err("link_up:             %d\n", ectx->rx_stats.event_count[WETH_LINK_UP]);
    weth_pr_err("link_down:             %d\n", ectx->rx_stats.event_count[WETH_LINK_DOWN]);
    weth_pr_err("config_dma:             %d\n", ectx->rx_stats.event_count[WETH_CONFIG_DMA]);
    weth_pr_err("tx_dma_complete:             %d\n", ectx->rx_stats.event_count[WETH_TX_DMA_COMPLETE]);
    weth_pr_err("rx_dma_complete:             %d\n", ectx->rx_stats.event_count[WETH_RX_DMA_COMPLETE]);
    weth_pr_err("refill_complete:             %d\n", ectx->rx_stats.event_count[WETH_REFILL_COMPLETE]);
    weth_pr_err("sync_table:             %d\n", ectx->rx_stats.event_count[WETH_SYNC_TABLE]);
    weth_pr_err("tx_table_complete:             %d\n", ectx->rx_stats.event_count[WETH_SYNC_TX_TABLE_COMPLETE]);
    weth_pr_err("rx_table_complete:             %d\n", ectx->rx_stats.event_count[WETH_SYNC_RX_TABLE_COMPLETE]);
}
