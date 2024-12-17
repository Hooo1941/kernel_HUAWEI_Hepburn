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

#ifndef __WAN_ETH_MAA_H__
#define __WAN_ETH_MAA_H__

#include "bsp_dra.h"
#include "wan_eth_common.h"
#include "mdrv_pfa.h"

#include <linux/skbuff.h>

#define BUF_SIZE_1_6KB DRA_OPIPE_LEVEL_1_SIZE

static inline struct sk_buff *__weth_alloc_maa_skb(int len)
{
    struct sk_buff *skb = NULL;

    skb = bsp_dra_skb_alloc(BUF_SIZE_1_6KB);
    if (skb == NULL) {
        weth_pr_err("skb is null\n");
        return skb;
    }

    return skb;
}

static inline void __weth_free_maa_skb(struct sk_buff *skb)
{
    bsp_dra_kfree_skb(skb);
}

static inline void __weth_free_maa_skb_list_by_null(struct sk_buff *skb)
{
    struct sk_buff *next_skb = NULL;

    while (skb != NULL) {
        next_skb = skb->next;
        __weth_free_maa_skb(skb);
        skb = next_skb;
    }
}

static inline void __weth_free_maa_skb_list(struct sk_buff *skb)
{
    struct sk_buff *cur_skb = skb->next;
    skb->next = NULL;

    __weth_free_maa_skb_list_by_null(cur_skb);
}

static inline void __weth_ep_free_skb(struct sk_buff *skb)
{
    struct rx_cb_map_s *cb_map = NULL;

    cb_map = (struct rx_cb_map_s *)&skb->cb;
    if (g_by_pass_mode == 1) {
        bsp_dra_kfree_skb(skb);
        g_packet_dump.maa_free++;
    } else {
        dev_kfree_skb_any(skb);
        g_packet_dump.kernel_free++;
    }
}

static inline void __weth_ep_free_skb_list_by_null(struct sk_buff *skb)
{
    struct sk_buff *next_skb = NULL;

    while (skb != NULL) {
        next_skb = skb->next;
        __weth_ep_free_skb(skb);
        skb = next_skb;
    }
}

static inline void __weth_ep_free_skb_list(struct sk_buff *skb)
{
    struct sk_buff *first_skb = NULL;

    if (skb == NULL) {
        return;
    }
    first_skb = skb->next;
    skb->next = NULL;

    __weth_ep_free_skb_list_by_null(first_skb);
}
#endif
