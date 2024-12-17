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
#ifndef __WAN_ETH_DMA_H__
#define __WAN_ETH_DMA_H__

#include <linux/list.h>
#include <linux/spinlock.h>

#include "bsp_pcie.h"

#define WETH_DMA_RW_MAX (PCIE_DMA_DIRECTION_WRITE + 1)
#define WETH_DMA_RW_FLAG(chn) ((chn) & 0x1)

/* the chn num and the chn def must be same */
#define WETH_DMA_CHN_MAX 2
#define WETH_DMA_ETH_DEV_DEF()               \
    {                                        \
        PCIE_DMA_ETH_DEV, PCIE_DMA_ETH_DEV2, \
        /* PCIE_DMA_ETH_DEV3, */                                          \
    }

typedef void (*weth_dma_complete_cb_t)(void *para);

struct weth_dma_req {
    struct list_head list;
    void *desc;
    weth_dma_complete_cb_t cb;
    void *priv;
    int port_no;
    int status;
};

struct weth_dma_tx_status {
    unsigned int submit_req;
    unsigned int dma_transfer;
    unsigned int dma_transfer_fail;
    unsigned int dma_transfer_fail_req;
    unsigned int complete_callback;
    unsigned int req_complete;
    unsigned int req_next;
};

struct weth_dma_req_trans_info {
    struct list_head chn_list;
    struct list_head req_list;
    struct pcie_dma_transfer_info dma_trans_info;
    struct weth_dma_req *sending_req;

    spinlock_t req_list_lock;
    unsigned int dma_idle;
    int chn;
    struct weth_dma_tx_status tx_stats;
};

struct weth_dma_chn_rw {
    spinlock_t chn_lock;
    int next_chn;
    struct list_head chn_free_head;
    struct weth_dma_req_trans_info dma_chn[WETH_DMA_CHN_MAX];
};

struct weth_dma_ctx {
    unsigned int pcie_id;
    struct weth_dma_chn_rw dma_chn_rw[WETH_DMA_RW_MAX];
};

int weth_dma_submit_req(struct weth_dma_req *req, unsigned int direct);
int weth_dma_init(unsigned int pcie_id);

#endif
