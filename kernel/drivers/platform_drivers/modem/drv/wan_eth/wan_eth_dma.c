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
#include "wan_eth_dma.h"
#include "bsp_pcie.h"
#include "wan_eth_common.h"

struct weth_dma_ctx g_dctx;
enum pcie_dma_chn_id g_weth_dma_dev[WETH_DMA_CHN_MAX] = WETH_DMA_ETH_DEV_DEF();

STATIC struct weth_dma_req_trans_info *weth_dma_get_best_chn(struct weth_dma_req *req, int dir)
{
    int cur_chn;
    struct weth_dma_chn_rw *chn_rw = &g_dctx.dma_chn_rw[dir];
    struct weth_dma_req_trans_info *dma_info = NULL;

    cur_chn = req->port_no & 0x1;
    dma_info = &chn_rw->dma_chn[cur_chn];

    return dma_info;
}

STATIC int weth_dma_transfer(struct weth_dma_ctx *dctx, struct weth_dma_req *req,
                             struct weth_dma_req_trans_info *dma_info)
{
    int ret;
    unsigned long flags;

    dma_info->tx_stats.dma_transfer++;
    dma_info->dma_trans_info.dma_lli_addr = req->desc;
    ret = bsp_pcie_ep_dma_transfer(dctx->pcie_id, &dma_info->dma_trans_info);
    if (unlikely(ret)) {
        dma_info->tx_stats.dma_transfer_fail++;
        spin_lock_irqsave(&dma_info->req_list_lock, flags);
        while (!list_empty(&dma_info->req_list)) {
            struct weth_dma_req *req = list_last_entry(&dma_info->req_list, struct weth_dma_req, list);
            list_del(&req->list);
            dma_info->tx_stats.dma_transfer_fail_req++;
        }
        spin_unlock_irqrestore(&dma_info->req_list_lock, flags);
        req->status = -1;
        req->cb(req);
        return -1;
    }

    return 0;
}

STATIC int weth_dma_complete_callback(u32 dir, u32 status, void *para)
{
    struct weth_dma_ctx *dctx = &g_dctx;
    struct weth_dma_chn_rw *chn_rw = &dctx->dma_chn_rw[dir];
    struct weth_dma_req_trans_info *dma_info = NULL;
    int chn = lower_32_bits((long long)(uintptr_t)para);
    struct weth_dma_req *req_complete = NULL;
    struct weth_dma_req *req_next = NULL;
    unsigned long flags;

    dma_info = &chn_rw->dma_chn[chn];
    dma_info->tx_stats.complete_callback++;

    spin_lock_irqsave(&dma_info->req_list_lock, flags);
    if (!list_empty(&dma_info->req_list)) {
        req_complete = list_last_entry(&dma_info->req_list, struct weth_dma_req, list);
        list_del(&req_complete->list);
    }

    if (!list_empty(&dma_info->req_list)) {
        req_next = list_last_entry(&dma_info->req_list, struct weth_dma_req, list);
    }
    spin_unlock_irqrestore(&dma_info->req_list_lock, flags);

    if (req_complete) {
        dma_info->tx_stats.req_complete++;
        req_complete->status = status;
        req_complete->cb(req_complete);
    }

    if (req_next) {
        dma_info->tx_stats.req_next++;
        (void)weth_dma_transfer(dctx, req_next, dma_info);
    } else {
        /* all req complete, return dma chn to free list */
    }

    return 0;
}

int weth_dma_submit_req(struct weth_dma_req *req, unsigned int direct)
{
    struct weth_dma_ctx *dctx = &g_dctx;
    struct weth_dma_req_trans_info *dma_info = NULL;
    unsigned long flags;
    unsigned int is_empty = 0;

    dma_info = weth_dma_get_best_chn(req, direct);
    dma_info->tx_stats.submit_req++;

    spin_lock_irqsave(&dma_info->req_list_lock, flags);
    if (list_empty(&dma_info->req_list)) {
        is_empty = 1;
    }
    list_add(&req->list, &dma_info->req_list);
    spin_unlock_irqrestore(&dma_info->req_list_lock, flags);

    if (is_empty) {
        return weth_dma_transfer(dctx, req, dma_info);
    }

    return 0;
}

int weth_dma_init(unsigned int pcie_id)
{
    int ret;
    int chn;
    int dir;
    struct weth_dma_ctx *dctx = &g_dctx;

    dctx->pcie_id = pcie_id;
    for (dir = 0; dir < WETH_DMA_RW_MAX; dir++) {
        spin_lock_init(&dctx->dma_chn_rw[dir].chn_lock);
        dctx->dma_chn_rw[dir].next_chn = 0;
        INIT_LIST_HEAD(&dctx->dma_chn_rw[dir].chn_free_head);

        for (chn = 0; chn < WETH_DMA_CHN_MAX; chn++) {
            spin_lock_init(&dctx->dma_chn_rw[dir].dma_chn[chn].req_list_lock);
            INIT_LIST_HEAD(&dctx->dma_chn_rw[dir].dma_chn[chn].req_list);
            INIT_LIST_HEAD(&dctx->dma_chn_rw[dir].dma_chn[chn].chn_list);
            dctx->dma_chn_rw[dir].dma_chn[chn].dma_trans_info.transfer_type = PCIE_DMA_LINK_MODE;
            dctx->dma_chn_rw[dir].dma_chn[chn].dma_trans_info.channel = g_weth_dma_dev[chn];
            dctx->dma_chn_rw[dir].dma_chn[chn].dma_trans_info.direction = dir;
            dctx->dma_chn_rw[dir].dma_chn[chn].chn = chn;

            list_add_tail(&dctx->dma_chn_rw[dir].dma_chn[chn].chn_list, &dctx->dma_chn_rw[dir].chn_free_head);

            ret = bsp_pcie_ep_dma_isr_register(dctx->pcie_id, g_weth_dma_dev[chn], dir, weth_dma_complete_callback,
                                               (void *)(long long)chn);
            if (ret) {
                weth_pr_err("ep dma callback register failed\n");
                return ret;
            }
        }
    }

    return 0;
}

int weth_dump_dma_stat(void)
{
    int chn;
    int dir;
    struct weth_dma_tx_status *tx_stats = NULL;
    struct weth_dma_ctx *dctx = &g_dctx;

    weth_pr_err("pcie_id:                 %d\n", dctx->pcie_id);
    for (dir = 0; dir < WETH_DMA_RW_MAX; dir++) {
        for (chn = 0; chn < WETH_DMA_CHN_MAX; chn++) {
            tx_stats = &dctx->dma_chn_rw[dir].dma_chn[chn].tx_stats;
            weth_pr_err("submit_req:              %d\n", tx_stats->submit_req);
            weth_pr_err("dma_transfer:            %d\n", tx_stats->dma_transfer);
            weth_pr_err("dma_transfer_fail:       %d\n", tx_stats->dma_transfer_fail);
            weth_pr_err("dma_transfer_fail_req:   %d\n", tx_stats->dma_transfer_fail_req);
            weth_pr_err("complete_callback:       %d\n", tx_stats->complete_callback);
            weth_pr_err("req_complete:            %d\n", tx_stats->req_complete);
            weth_pr_err("req_next:                %d\n", tx_stats->req_next);
        }
    }
    return 0;
}
