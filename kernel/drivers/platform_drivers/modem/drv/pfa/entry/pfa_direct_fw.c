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

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/jhash.h>
#include <linux/kernel.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/ip.h>

#include <securec.h>
#include <mdrv_pfa.h>
#include <bsp_pfa.h>
#include <bsp_dra.h>
#include <bsp_syscache.h>
#include "pfa_ip_entry.h"
#include "pfa_dbg.h"
#include "pfa_direct_fw.h"
#include "pfa_port.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PFA_DFW_DL_TO_LAN 0x444c414e
#define PFA_DFW_UL_TO_WAN 0x5557414e

#ifdef PFA_RECORD_WAN
struct wan_dev_info_s *pfa_direct_fw_find_wan_dev(struct net_device *dev)
{
    struct wan_dev_entry *entry = NULL;
    struct pfa *pfa = &g_pfa;

    list_for_each_entry(entry, &pfa->wanport.wan_dev_list, list)
    {
        if (entry->info.dev == dev) {
            return &entry->info;
        }
    }
    return NULL;
}

int pfa_direct_fw_record_wandev(struct wan_dev_info_s *wan_dev_info)
{
    struct pfa *pfa = &g_pfa;
    struct wan_dev_entry *ptr = NULL;
    int ret;

    if (!pfa->direct_fw.dfw_flags.enable) {
        return 0;
    }

    if (wan_dev_info == NULL) {
        return -EINVAL;
    }

    list_for_each_entry(ptr, &pfa->direct_fw.wan_dev_list, list)
    {
        if (ptr->info.dev == wan_dev_info->dev) {
            ret = memcpy_s(&ptr->info, sizeof(ptr->info), wan_dev_info, sizeof(*wan_dev_info));
            if (ret) {
                PFA_ERR("memset_s failed\n");
            }
            return 0;
        }
    }

    ptr = kmalloc(sizeof(struct wan_dev_entry), GFP_KERNEL);
    if (ptr == NULL) {
        return -ENOMEM;
    }

    ret = memcpy_s(&ptr->info, sizeof(ptr->info), wan_dev_info, sizeof(*wan_dev_info));
    if (ret) {
        PFA_ERR("memset_s failed\n");
    }
    list_add(&ptr->list, &pfa->direct_fw.wan_dev_list);

    return 0;
}

int pfa_wan_dev_dump(void)
{
    struct pfa *pfa = &g_pfa;
    struct wan_dev_entry *ptr = NULL;

    list_for_each_entry(ptr, &pfa->direct_fw.wan_dev_list, list)
    {
        if (ptr != NULL) {
            if ((ptr->info.dev != NULL) && (ptr->info.dev->name != NULL)) {
                bsp_err("dev_name   %s\n", ptr->info.dev->name);
            } else {
                bsp_err("dev_name   NULL! \n");
            }
            bsp_err("ipv4 info  \n");
            bsp_err("pdu_session_id   %u\n", ptr->info.v4.info.pdu_session_id);
            bsp_err("fc_head   %u\n", ptr->info.v4.info.fc_head);
            bsp_err("modem_id   %u\n", ptr->info.v4.info.modem_id);
            bsp_err("higi_pri_flag   %u\n", ptr->info.v4.info.higi_pri_flag);
            bsp_err("parse_en   %u\n", ptr->info.v4.info.parse_en);
            bsp_err("userfield0   %x\n", ptr->info.v4.userfield0);
            bsp_err("userfield1   %x\n", ptr->info.v4.userfield1);
            bsp_err("userfield2   %x\n", ptr->info.v4.userfield2);

            bsp_err("ipv6 info  \n");
            bsp_err("pdu_session_id   %u\n", ptr->info.v6.info.pdu_session_id);
            bsp_err("fc_head   %u\n", ptr->info.v6.info.fc_head);
            bsp_err("modem_id   %u\n", ptr->info.v6.info.modem_id);
            bsp_err("higi_pri_flag   %u\n", ptr->info.v6.info.higi_pri_flag);
            bsp_err("parse_en   %u\n", ptr->info.v6.info.parse_en);
            bsp_err("userfield0   %x\n", ptr->info.v6.userfield0);
            bsp_err("userfield1   %x\n", ptr->info.v6.userfield1);
            bsp_err("userfield2   %x\n", ptr->info.v6.userfield2);
        } else {
            bsp_err("wan dev NULL!\n");
        }
    }
    return 0;
}

void pfa_direct_fw_write_wan_info(struct pfa *pfa, struct wan_dev_info_s *wan_dev_info, struct pfa_ip_fw_entry *entry)
{
    struct wan_info_s *wan_info = NULL;

    if (entry->iptype) {
        wan_info = &wan_dev_info->v6;
    } else {
        wan_info = &wan_dev_info->v4;
    }

    entry->pdu_ssid = wan_info->info.pdu_session_id;
    entry->fc_head = wan_info->info.fc_head;
    entry->modem_id = wan_info->info.modem_id;
    entry->htab_usrfield_lower16 = (unsigned short)(wan_info->userfield0);
    entry->htab_usrfield_higher16 = (unsigned short)(wan_info->userfield0 >> 16); /* 16 used to bit calc */
    entry->priv->wan_dev = wan_dev_info->dev;

    if (pfa->msg_level & PFA_MSG_TUPLE) {
        PFA_ERR("pdu_ssid 0x%x fc_head 0x%x modem_id 0x%x userfield0 0x%x \n", wan_info->info.pdu_session_id,
                wan_info->info.fc_head, wan_info->info.modem_id, wan_info->userfield0);
    }
}

int pfa_direct_fw_record_add_waninfo(struct pfa_ip_fw_entry *entry)
{
    struct wan_dev_info_s *wan_info;

    wan_info = pfa_direct_fw_find_wan_dev(NULL);
    if (wan_info != NULL) {
        entry->priv->dst_net = PFA_DFW_UL_TO_WAN;
        pfa_direct_fw_write_wan_info(pfa, wan_info, entry);
    }

    pfa->direct_fw.no_wan_info++;
    return -EINVAL;
}

#else
int pfa_direct_fw_record_add_waninfo(struct pfa_ip_fw_entry *entry)
{
    return 0;
}
int pfa_direct_fw_record_wandev(struct wan_dev_info_s *wan_dev_info)
{
    return 0;
}
#endif

int pfa_make_ipv4_tuple(struct sk_buff *skb, struct nf_conntrack_tuple *tuple, unsigned int l3_proto)
{
    struct pfa *pfa = &g_pfa;
    struct iphdr *iph = NULL;
    struct udphdr *udph = NULL;
    struct tcphdr *tcph = NULL;

    iph = ip_hdr(skb);
    if (iph->ihl != 5) { /* 5 ip pkt header length */
        pfa->direct_fw.iph_len_err++;
        return -EINVAL;
    }

    if (ip_is_fragment(iph)) {
        pfa->direct_fw.iph_frag++;
        return -EINVAL;
    }
    skb->transport_header = skb->network_header + iph->ihl * 4; /* 4 Byte */

    tuple->dst.protonum = l3_proto;
    tuple->dst.dir = IP_CT_DIR_ORIGINAL;
    tuple->dst.u3.ip = iph->daddr;
    tuple->src.u3.ip = iph->saddr;
    tuple->src.l3num = AF_INET;  // AF_INET=2 as ipv4, AF_INET6=10 as ipv6

    if (l3_proto == IPPROTO_TCP) {
        tcph = tcp_hdr(skb);
        tuple->src.u.tcp.port = tcph->source;
        tuple->dst.u.tcp.port = tcph->dest;
    } else if (l3_proto == IPPROTO_UDP) {
        udph = udp_hdr(skb);
        tuple->src.u.udp.port = udph->source;
        tuple->dst.u.udp.port = udph->dest;
    } else {
        pfa->direct_fw.ipproto_err++;
        return -EINVAL;
    }
    return 0;
}

int pfa_make_ipv6_tuple(struct sk_buff *skb, struct nf_conntrack_tuple *tuple, unsigned int l3_proto)
{
    struct pfa *pfa = &g_pfa;
    struct ipv6hdr *hdr = NULL;
    struct udphdr *udph = NULL;
    struct tcphdr *tcph = NULL;
    int ret;

    hdr = ipv6_hdr(skb);

    tuple->dst.protonum = l3_proto;
    tuple->dst.dir = IP_CT_DIR_ORIGINAL;

    ret = memcpy_s(tuple->src.u3.ip6, sizeof(tuple->src.u3.ip6), &hdr->saddr, sizeof(hdr->saddr));
    if (ret) {
        PFA_ERR_ONCE("memcpy ipv6 src addr fail \n");
        return -EINVAL;
    }
    ret = memcpy_s(tuple->dst.u3.ip6, sizeof(tuple->dst.u3.ip6), &hdr->daddr, sizeof(hdr->daddr));
    if (ret) {
        PFA_ERR_ONCE("memcpy ipv6 dst addr fail \n");
        return -EINVAL;
    }

    tuple->src.l3num = AF_INET6;  // AF_INET=2 as ipv4, AF_INET6=10 as ipv6

    if (l3_proto == IPPROTO_TCP) {
        skb->transport_header = skb->network_header + sizeof(*hdr);
        tcph = tcp_hdr(skb);
        tuple->src.u.tcp.port = tcph->source;
        tuple->dst.u.tcp.port = tcph->dest;
    } else if (l3_proto == IPPROTO_UDP) {
        skb->transport_header = skb->network_header + sizeof(*hdr);
        udph = udp_hdr(skb);
        tuple->src.u.udp.port = udph->source;
        tuple->dst.u.udp.port = udph->dest;
    } else {
        pfa->direct_fw.ipproto_err++;
        return -EINVAL;
    }

    return 0;
}

static int pfa_direct_fw_add_tuple(struct sk_buff *skb, struct pfa_ip_fw_entry *entry,
    union packet_info_u *packet_info)
{
    struct nf_conntrack_tuple *tuple = &(entry->tuple);
    int ret;

    if ((packet_info->bits.ip_proto == AF_INET) &&
        ((packet_info->bits.l4_proto == IPPROTO_TCP) || (packet_info->bits.l4_proto == IPPROTO_UDP))) {
        ret = pfa_make_ipv4_tuple(skb, tuple, packet_info->bits.l4_proto);
        if (ret) {
            return -EINVAL;
        }
        entry->iptype = 0;
    } else if ((packet_info->bits.ip_proto == AF_INET6) &&
               ((packet_info->bits.l4_proto == IPPROTO_TCP) || (packet_info->bits.l4_proto == IPPROTO_UDP))) {
        ret = pfa_make_ipv6_tuple(skb, tuple, packet_info->bits.l4_proto);
        if (ret) {
            return -EINVAL;
        }
        entry->iptype = 1;
    } else {
        return -EINVAL;
    }
    entry->action = PFA_NF_NAT_MANIP_NONE;
    entry->nat_src_ip = 0;
    entry->nat_src_port = 0;
    entry->nat_dst_ip = 0;
    entry->nat_dst_port = 0;

    return ret;
}

unsigned int pfa_entry_get_port(struct pfa *pfa, struct pfa_ip_fw_entry *entry)
{
    unsigned int i;
    unsigned int most_idle_port = 0;
    unsigned int cur_port;
    unsigned int busy_level = 0;
    unsigned int cur_busy_level;
    struct pfa_direct_fw_ctx *dir_fw_ctx = NULL;

    if (pfa->direct_fw.port_cnt == 0) {
        pfa_bug(pfa);
        return pfa->cpuport.portno;
    }

    for (i = 0; i < pfa->direct_fw.port_cnt; i++) {
        cur_port = pfa->direct_fw.port_nums[i];
        dir_fw_ctx = &pfa->ports[cur_port].ctrl.direct_fw_ctx;
        if (pfa->ports[cur_port].ctrl.port_flags.enable) {
            cur_busy_level = dir_fw_ctx->port_weight / dir_fw_ctx->stream_cnt;
            if (busy_level < cur_busy_level) {
                busy_level = cur_busy_level;
                most_idle_port = cur_port;
            }
        }
    }

    pfa->ports[most_idle_port].ctrl.direct_fw_ctx.stream_cnt++;
    return most_idle_port;
}

void pfa_get_pfa_tft_mac_addr(struct pfa *pfa)
{
    unsigned char *mac = NULL;
    unsigned int mac_lo;
    unsigned int mac_hi;
    int ret;

    mac = pfa->pfa_tftport.pfa_tft_eth_head.h_dest;
    mac_lo = pfa_readl_relaxed(pfa->regs, PFA_GLB_DMAC_ADDR_L);
    mac_hi = pfa_readl_relaxed(pfa->regs, PFA_GLB_DMAC_ADDR_H);

    ret = memcpy_s(mac, sizeof(mac_lo), &mac_lo, sizeof(mac_lo));
    if (ret != EOK) {
        PFA_ERR("<pfa_get_pfa_tft_mac> memcpy_s fail!\n");
    }
    ret = memcpy_s(mac + sizeof(mac_lo), PFA_MAC_ADDR_LEFT_LEN, &mac_hi, PFA_MAC_ADDR_LEFT_LEN);
    if (ret != EOK) {
        PFA_ERR("<pfa_get_pfa_tft_mac> memcpy_s fail!\n");
    }

    mac = pfa->pfa_tftport.pfa_tft_eth_head.h_source;
    mac_lo = pfa_readl_relaxed(pfa->regs, PFA_GLB_SMAC_ADDR_L);
    mac_hi = pfa_readl_relaxed(pfa->regs, PFA_GLB_SMAC_ADDR_H);

    ret = memcpy_s(mac, sizeof(mac_lo), &mac_lo, sizeof(mac_lo));
    if (ret != EOK) {
        PFA_ERR("<pfa_get_pfa_tft_mac> memcpy_s fail!\n");
    }
    ret = memcpy_s(mac + sizeof(mac_lo), PFA_MAC_ADDR_LEFT_LEN, &mac_hi, PFA_MAC_ADDR_LEFT_LEN);
    if (ret != EOK) {
        PFA_ERR("<pfa_get_pfa_tft_mac> memcpy_s fail!\n");
    }

    return;
}

void pfa_set_mac_addr(struct pfa_direct_fw_entry_ctx *ctx, struct pfa_ip_fw_entry *entry)
{
    int ret;

    ret = memcpy_s(entry->shost, sizeof(entry->shost), &ctx->src_hd[0], ETH_ALEN);
    if (ret != EOK) {
        PFA_ERR("pfa_set_mac_addr, memcpy_s fail \n");
    }
    ret = memcpy_s(entry->dhost, sizeof(entry->dhost), &ctx->dest_hd[0], ETH_ALEN);
    if (ret != EOK) {
        PFA_ERR("pfa_set_mac_addr, memcpy_s fail \n");
    }
    return;
}

static void pfa_set_direct_fw_entry(struct pfa *pfa, struct pfa_build_fw_entry_info *info,
    struct pfa_ip_fw_entry *entry, unsigned int hash, unsigned int hkey, unsigned int net_id)
{
    // write mac here
    pfa_set_mac_addr(info->ctx, entry);

    entry->vid = 0;
    entry->net_id = info->ctx->net_id;
    entry->priv->dir = 0;
    entry->timestamp = pfa_readl(pfa->regs, PFA_TIME);
    entry->priv->hash = hash;
    entry->priv->hkey = hkey;
    if (info->ctx->add_wan_info) {
        pfa_direct_fw_record_add_waninfo(entry);
    } else {
        entry->priv->dst_net = PFA_DFW_DL_TO_LAN;
        entry->htab_usrfield_higher16 = (unsigned short)reciprocal_scale(hash, 0xffff);
        entry->htab_usrfield_lower16 = (unsigned short)(info->ctx->devid);
    }
    if (info->macfw_to_ip) {
        entry->net_id = net_id;
    }
    entry->action = 0;  // noaction

    if (info->state) {
        entry->priv->in = info->state->in;
        entry->priv->out = info->state->out;
    }

    return;
}

int pfa_add_direct_fw_entry(struct pfa *pfa, unsigned long long ip_phy_addr,
    union packet_info_u *packet_info, struct pfa_build_fw_entry_info *info)
{
    struct pfa_ip_fw_entry *entry = NULL;
    unsigned int hkey, hash, net_id;
    unsigned long long ip_phy_addr_local = ip_phy_addr;
    unsigned long flags;
    int ret = 0;

    if (info->ctx->maped) {
        bsp_sc_dma_unmap_single(pfa->dev, ip_phy_addr_local, L1_CACHE_BYTES, DMA_FROM_DEVICE);
    }

    spin_lock_irqsave(&pfa->ipfw.lock, flags);
    entry = ip_fw_node_get(pfa);
    if (entry == NULL) {
        pfa->direct_fw.ip_fw_node_get_fail++;
        goto release;
    }

    ret = pfa_direct_fw_add_tuple(info->skb, entry, packet_info);
    if (ret) {
        pfa->direct_fw.add_tuple_fail++;
        goto release;
    }

    /* get hash */
    hash = pfa_ipfw_get_hash(&entry->tuple, pfa->ipfw.hzone);
    hkey = reciprocal_scale(hash, pfa->ipfw.hlist_size);
    if (ip_fw_entry_exist(pfa, &(entry->tuple), hkey)) {
        pfa->direct_fw.entry_exist++;
        goto release;
    }
    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);
    net_id = PFA_NET_ID_BY_VIR_PORT(info->portno);
    entry->portno = PFA_GET_PHY_PORT(info->portno);
    pfa_set_direct_fw_entry(pfa, info, entry, hash, hkey, net_id);

    pfa_add_ipfw_entry_cb(pfa, info->skb, entry, 0, hkey);

    ip_fw_list_add(pfa, entry, hkey);

    spin_lock_irqsave(&pfa->ipfw.lock, flags);
    list_add(&entry->priv->wan_list, &pfa->ipfw.wan_entry_list); /* add entry after first node */
    pfa->direct_fw.entry_cnt++;
    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);
    if (info->ctx->maped) {
        ip_phy_addr_local = bsp_sc_dma_map_single(pfa->dev, info->skb->data, L1_CACHE_BYTES, DMA_FROM_DEVICE);
    }

    return 0;

release:
    if (entry != NULL) {
        ip_fw_node_put(pfa, entry, false);
    }
    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);
    if (info->ctx->maped) {
        ip_phy_addr_local = bsp_sc_dma_map_single(pfa->dev, info->skb->data, L1_CACHE_BYTES, DMA_FROM_DEVICE);
    }
    return -EINVAL;
}

int pfa_get_packet_info(struct sk_buff *skb, union packet_info_u *pkt_info)
{
    const struct ethhdr *eth;

    eth = (struct ethhdr *)skb->data;

    if (eth->h_proto == htons(ETH_P_IP)) {
        pkt_info->bits.ip_proto = AF_INET;
        pkt_info->bits.l4_proto = ip_hdr(skb)->protocol;
    } else if (eth->h_proto == htons(ETH_P_IPV6)) {
        pkt_info->bits.ip_proto = AF_INET6;
        pkt_info->bits.l4_proto = ipv6_hdr(skb)->nexthdr;
    } else {
        return -EINVAL;
    }

    return 0;
}

void pfa_build_fw_entry(struct pfa_build_fw_entry_info *info)
{
    int ret;
    unsigned long long ip_phy_addr = 0;
    struct rx_cb_map_s *cb = NULL;
    struct pfa *pfa = &g_pfa;
    union packet_info_u *packet_info = NULL;
    union packet_info_u pkt_info;
    struct sk_buff *skb = info->skb;
    struct pfa_direct_fw_entry_ctx *ctx = info->ctx;

    if (pfa->direct_fw.entry_cnt >= PFA_DIERCT_FW_ENTRY_MAX) {
        pfa->direct_fw.entry_add_max++;
        return;
    }

    if (skb == NULL) {
        pfa->direct_fw.entry_add_skb_null++;
        return;
    }

    if (ctx->skb_cb_info) {
        skb->network_header = skb_headroom(skb) + ctx->l2_hdr_len;
        cb = (struct rx_cb_map_s *)skb->cb;
        if (cb->packet_info.u32 == 0) {
            pfa->direct_fw.not_tcp_udp++;
            return;
        }
        packet_info = &cb->packet_info;
        ip_phy_addr = bsp_dra_to_phy(cb->dra_l2 + cb->packet_info.bits.l2_hdr_offeset);
        if (ip_phy_addr == 0) {
            pfa->direct_fw.entry_add_l3dra_null++;
            return;
        }
    } else {
        skb->network_header = skb_headroom(skb) + ctx->l2_hdr_len;
        ret = pfa_get_packet_info(skb, &pkt_info);
        if (unlikely(ret)) {
            pfa->direct_fw.no_packet_info++;
            return;
        }
        packet_info = &pkt_info;
        ip_phy_addr = virt_to_phys(skb->data + ctx->l2_hdr_len);
    }

    ret = pfa_add_direct_fw_entry(pfa, ip_phy_addr, packet_info, info);
    if (ret) {
        pfa->direct_fw.entry_add_pkt_err_total++;
    }
}

void bsp_pfa_add_direct_fw_entry(struct sk_buff *skb, struct pfa_direct_fw_entry_ctx *ctx,
    unsigned int portno)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_build_fw_entry_info info = {0};

    pfa->direct_fw.entry_add++;

    if (!pfa->direct_fw.dfw_flags.enable) {
        pfa->direct_fw.entry_add_dfw_disabled++;
        return;
    }
    info.skb = skb;
    info.ctx = ctx;
    info.portno = portno;
    info.macfw_to_ip = 0;
    pfa_build_fw_entry(&info);

    return;
}

void pfa_del_direct_fw_entry(struct pfa *pfa, struct pfa_ip_fw_entry *entry)
{
    unsigned int hkey;

    hkey = entry->priv->hkey;
    pfa_ipfw_list_del_entry(pfa, entry, hkey);
    pfa->ports[entry->portno].ctrl.direct_fw_ctx.stream_cnt--;
    ip_fw_node_put(pfa, entry, false);
    pfa->stat.ipfw_del++;
    return;
}

void bsp_pfa_del_direct_fw_dev(unsigned int devid)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_ip_fw_entry *entry = NULL;
    struct pfa_ip_fw_entry_priv *entry_priv = NULL;
    struct pfa_ip_fw_entry_priv *n = NULL;
    unsigned long flags = 0;
    int i = 0;

    if (!pfa->direct_fw.dfw_flags.enable) {
        return;
    }

    spin_lock_irqsave(&pfa->ipfw.lock, flags);

    list_for_each_entry_safe(entry_priv, n, &pfa->ipfw.wan_entry_list, wan_list)
    {
        entry = entry_priv->entry;
        if (((entry_priv->dst_net == PFA_DFW_DL_TO_LAN) && (entry->htab_usrfield_lower16 == devid)) ||
            (entry_priv->dst_net == PFA_DFW_UL_TO_WAN)) {
            pfa_del_direct_fw_entry(pfa, entry);
        } else {
            i++;
        }
    }
    pfa->direct_fw.entry_cnt = i;
    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);
}

void pfa_clean_ipfw_entry_cache(struct pfa *pfa)
{
    unsigned int tab_clean;
    // clean tab_ctrl;
    pfa_writel(pfa->regs, PFA_TAB_CLR, 0);

    tab_clean = BIT(4);  // clean all hash cache, bit 4
    pfa_writel(pfa->regs, PFA_TAB_CLR, tab_clean);
}

void pfa_updata_direct_fw_entry(struct pfa *pfa)
{
    struct pfa_ip_fw_entry *entry = NULL;
    struct pfa_ip_fw_entry_priv *entry_priv = NULL;
    struct pfa_ip_fw_entry_priv *n = NULL;
    unsigned int pfa_time;
    unsigned int entry_unuse_time = 0;
    unsigned long flags = 0;
    int i = 0;

    pfa_time = pfa_readl(pfa->regs, PFA_TIME); /* 0-15bit valid */

    spin_lock_irqsave(&pfa->ipfw.lock, flags);

    list_for_each_entry_safe(entry_priv, n, &pfa->ipfw.wan_entry_list, wan_list)
    {
        entry = entry_priv->entry;
        if (pfa_time >= entry->timestamp) {
            entry_unuse_time = pfa_time - entry->timestamp;
        } else {
            entry_unuse_time = PFA_TIME_MAX - entry->timestamp + pfa_time;
        }

        // process time out entry;
        if (entry_unuse_time > PFA_S_TO_TIMER_CNT(pfa->direct_fw.direct_fw_time_interval)) {
            pfa_del_direct_fw_entry(pfa, entry);
        } else {
            i++;
        }
    }
    pfa->direct_fw.entry_cnt = i;
    spin_unlock_irqrestore(&pfa->ipfw.lock, flags);
}

void pfa_direct_fw_ctrl_timer(struct timer_list *t)
{
    struct pfa *pfa = &g_pfa;

    clear_bit(PFA_EVENT_FW_TIMER_STOPED_BIT, (void *)&pfa->event);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)) {
        mod_timer(&pfa->direct_fw.direct_fw_timer, jiffies + PFA_FW_TIMER_EXPIRES);
        set_bit(PFA_EVENT_FW_TIMER_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return;
    }

    pfa_updata_direct_fw_entry(pfa);

    mod_timer(&pfa->direct_fw.direct_fw_timer, jiffies + PFA_FW_TIMER_EXPIRES);

    set_bit(PFA_EVENT_FW_TIMER_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);
}

void pfa_direct_fw_init(struct pfa *pfa)
{
    int ret;

    ret = memset_s(&pfa->direct_fw, sizeof(pfa->direct_fw), 0, sizeof(struct pfa_direct_fw_ctrl));
    if (ret) {
        PFA_ERR("pfa_direct_fw_init set empty failed\n");
        pfa_bug(pfa);
        return;
    }

    INIT_LIST_HEAD(&pfa->direct_fw.wan_dev_list);
    pfa->direct_fw.direct_fw_time_interval = PFA_DIRECT_FW_AGEING_TIME;

    timer_setup(&pfa->direct_fw.direct_fw_timer, pfa_direct_fw_ctrl_timer, 0);
    pfa->direct_fw.direct_fw_timer.expires = jiffies + msecs_to_jiffies(pfa->direct_fw.direct_fw_time_interval);

    return;
}

void pfa_direct_fw_status(void)
{
    int i;
    struct pfa *pfa = &g_pfa;
    unsigned int cur_port;
    PFA_ERR("direct_fw_en: %u\n", pfa->direct_fw.dfw_flags.enable);
    PFA_ERR("port_cnt: %u\n", pfa->direct_fw.port_cnt);
    for (i = 0; i < pfa->direct_fw.port_cnt; i++) {
        cur_port = pfa->direct_fw.port_nums[i];
        if (pfa->ports[cur_port].ctrl.port_flags.alloced) {
            PFA_ERR("port_nums: %u\n", cur_port);
            PFA_ERR("port_no: %u\n", pfa->ports[cur_port].ctrl.direct_fw_ctx.port_no);
            PFA_ERR("port_weight: %u\n", pfa->ports[cur_port].ctrl.direct_fw_ctx.port_weight);
            PFA_ERR("stream_cnt: %u\n", pfa->ports[cur_port].ctrl.direct_fw_ctx.stream_cnt);
        }
    }

    PFA_ERR("entry_add: %u\n", pfa->direct_fw.entry_add);
    PFA_ERR("entry_add_max: %u\n", pfa->direct_fw.entry_add_max);
    PFA_ERR("entry_add_dfw_disabled: %u\n", pfa->direct_fw.entry_add_dfw_disabled);
    PFA_ERR("entry_add_skb_null: %u\n", pfa->direct_fw.entry_add_skb_null);
    PFA_ERR("entry_add_l3dra_null: %u\n", pfa->direct_fw.entry_add_l3dra_null);
    PFA_ERR("not_tcp_udp: %u\n", pfa->direct_fw.not_tcp_udp);
    PFA_ERR("entry_add_pkt_err_total: %u\n", pfa->direct_fw.entry_add_pkt_err_total);
    PFA_ERR("entry_add_success: %u\n", pfa->direct_fw.entry_add_success);

    PFA_ERR("ipfw_node_get_fail: %u\n", pfa->direct_fw.ipfw_node_get_fail);
    PFA_ERR("ip_fw_node_get_fail: %u\n", pfa->direct_fw.ip_fw_node_get_fail);
    PFA_ERR("iph_len_err: %u\n", pfa->direct_fw.iph_len_err);
    PFA_ERR("iph_frag: %u\n", pfa->direct_fw.iph_frag);
    PFA_ERR("ipproto_err: %u\n", pfa->direct_fw.ipproto_err);
    PFA_ERR("add_tuple_fail: %u\n", pfa->direct_fw.add_tuple_fail);
    PFA_ERR("entry_exist: %u\n", pfa->direct_fw.entry_exist);
    PFA_ERR("no_wan_info: %u\n", pfa->direct_fw.no_wan_info);
    PFA_ERR("no_wan_info: %u\n", pfa->direct_fw.no_packet_info);
}

EXPORT_SYMBOL(pfa_direct_fw_status);
EXPORT_SYMBOL(pfa_direct_fw_record_add_waninfo);
EXPORT_SYMBOL(pfa_direct_fw_record_wandev);
EXPORT_SYMBOL(bsp_pfa_add_direct_fw_entry);
EXPORT_SYMBOL(bsp_pfa_del_direct_fw_dev);

MODULE_LICENSE("GPL");

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
