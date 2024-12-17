/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef __MDRV_BMI_IH_NET_H__
#define __MDRV_BMI_IH_NET_H__
#include "hooks/mdrv_bmi_ih.h"
#include "uapi/linux/netfilter.h"
#include <linux/skbuff.h>
#include <net/if_inet6.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <linux/phy.h>
#include <linux/wireless.h>
#include "uapi/linux/netfilter/nf_nat.h"

DECLARE_IOT_HOOK(iot_hook_net_sipalg_add_ip,
    IH_PROTO(union nf_inet_addr *newaddr),
    IH_ARGS(newaddr));

DECLARE_IOT_HOOK(iot_hook_net_sipalg_remove_ip,
    IH_PROTO(union nf_inet_addr *newaddr),
    IH_ARGS(newaddr));

DECLARE_IOT_HOOK(iot_hook_net_socket_update,
    IH_PROTO(struct sk_buff *skb),
    IH_ARGS(skb));

DECLARE_IOT_HOOK(iot_hook_net_socket_recv,
    IH_PROTO(const struct sk_buff *skb, struct in_pktinfo *info),
    IH_ARGS(skb, info));

DECLARE_IOT_HOOK(iot_hook_net_clat_neigh_delete,
    IH_PROTO(const struct neighbour *v4neighkey),
    IH_ARGS(v4neighkey));

DECLARE_IOT_HOOK(iot_hook_net_clat_neigh_update,
    IH_PROTO(unsigned int v4sip, const u8 *lladdr, struct net_device *dev),
    IH_ARGS(v4sip, lladdr, dev));

#ifdef CONFIG_NF_CONNTRACK_EVENTS

DECLARE_IOT_HOOK(iot_hook_net_ct_event,
    IH_PROTO(unsigned long events, struct nf_ct_event *item),
    IH_ARGS(events, item));

#endif

DECLARE_IOT_HOOK(iot_hook_net_pppoe_send_pfa,
    IH_PROTO(struct net_device *ppp_dev, struct sk_buff *skb, bool *ret),
    IH_ARGS(ppp_dev, skb, ret));

DECLARE_IOT_HOOK(iot_hook_net_pppoe_uninit,
    IH_PROTO(struct net_device *dev),
    IH_ARGS(dev));

DECLARE_IOT_HOOK(iot_hook_net_pppoe_init,
    IH_PROTO(struct net_device *ndev, struct net *net),
    IH_ARGS(ndev, net));

DECLARE_IOT_HOOK(iot_hook_net_copy_skb,
    IH_PROTO(struct sk_buff *new, const struct sk_buff *old),
    IH_ARGS(new, old));

DECLARE_IOT_HOOK(iot_hook_net_check_ndp,
    IH_PROTO(struct inet6_dev *idev, const struct in6_addr *group, const struct ifmcaddr6 *mc, bool *rv),
    IH_ARGS(idev, group, mc, rv));

DECLARE_IOT_HOOK(iot_hook_net_check_ns,
    IH_PROTO(bool *ret),
    IH_ARGS(ret));

DECLARE_IOT_HOOK(iot_hook_net_nd_set,
    IH_PROTO(int *ret),
    IH_ARGS(ret));

DECLARE_IOT_HOOK(iot_hook_net_br_flood_hook,
    IH_PROTO(struct sk_buff *skb),
    IH_ARGS(skb));

DECLARE_IOT_HOOK(iot_hook_net_ipcapture_check,
    IH_PROTO(struct sk_buff *skb, bool *ret),
    IH_ARGS(skb, ret));

DECLARE_IOT_HOOK(iot_hook_net_phy_get_switchid,
    IH_PROTO(struct mii_bus *bus, int addr, unsigned int *phy_id, int *ret),
    IH_ARGS(bus, addr, phy_id, ret));

DECLARE_IOT_HOOK(iot_hook_net_phy_cancel_machine,
    IH_PROTO(struct phy_device *phydev),
    IH_ARGS(phydev));

DECLARE_IOT_HOOK(iot_hook_net_socket_send,
    IH_PROTO(size_t len, bool *ret),
    IH_ARGS(len, ret));

DECLARE_IOT_HOOK(iot_hook_net_atp_ioctl,
    IH_PROTO(u32 cmd, struct iwreq *iwr, int *ret),
    IH_ARGS(cmd, iwr, ret));

DECLARE_IOT_HOOK(iot_hook_net_symnat,
    IH_PROTO(const struct nf_nat_range2 *range, bool *ret),
    IH_ARGS(range, ret));

DECLARE_IOT_HOOK(iot_hook_net_xfrm_compat,
    IH_PROTO(bool *ret),
    IH_ARGS(ret));

#endif