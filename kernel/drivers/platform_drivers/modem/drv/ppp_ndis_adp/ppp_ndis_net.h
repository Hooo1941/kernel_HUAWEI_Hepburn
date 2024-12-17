/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
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
#ifndef PPP_NDIS_NET_H
#define PPP_NDIS_NET_H

#include <linux/skbuff.h>
#include <bsp_ppp_ndis.h>

#define UNET_MAX_NUM 6
#define PNET_MAX_NUM 8

struct ndis_field0_desc {
    union {
        struct {
            u32 pif_id   :6;
            u32 pkt_type :2;
            u32 reserved :24;
        };
        u32 field_value;
    };
};

struct net_stat_ctx {
    unsigned int open;
    unsigned int close;

    unsigned int down_total;
    unsigned int down_fail;
    unsigned int no_deliver_cb;
    unsigned int ifid_error;
    unsigned int down_data_succ;
    unsigned int down_data_total;
    unsigned int down_data_fail;
    unsigned int down_ctrl_total;
    unsigned int down_ctrl_succ;
    unsigned int down_ctrl_fail;
    unsigned int up_ctrl_total;
};

#define MAX_IFACE_ID 64

typedef int (*stick_down_deliver_type)(unsigned int dev_id, struct sk_buff *skb);

struct ppp_ndis_map_item {
    int (*deliver_cb)(unsigned int dev_id, struct sk_buff *skb) __rcu;
    unsigned char chan_type;
    unsigned char chan_id;
};

struct ppp_ndis_net_ctx {
    struct ppp_ndis_map_item map_table[MAX_IFACE_ID];
    unsigned int usb_acore_state[UNET_MAX_NUM];
    unsigned int pcie_acore_state[PNET_MAX_NUM];
    unsigned int usb_ccore_state[UNET_MAX_NUM];
    unsigned int pcie_ccore_state[PNET_MAX_NUM];
};

struct ppp_ndis_ctx {
    struct ppp_ndis_net_ctx net_ctx;
    struct net_stat_ctx net_stat;
};

int ppp_ndis_net_init(void);
int ppp_ndis_net_uninit(void);
bool ppp_ndis_stick_support(void);

#endif // PPP_NDIS_NET_H