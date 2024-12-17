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
#include <linux/module.h>
#include <linux/list.h>
#include <linux/ip.h>
#include <linux/in6.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <uapi/linux/netfilter.h>
#include <uapi/linux/netfilter_ipv4.h>
#include <securec.h>

#include <net/ip.h>

#include "bsp_wan_eth.h"
#include "bsp_net_om.h"
#include "bsp_diag.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define TRANS_HOOK_ENABLE 0xE1E1
#define TRANS_MAX_LEN 1560U
#define TRANS_IPV4_MASK 0xFFFF0000U
#define TRANS_IPV6_MASK ((unsigned long long)(0))

#define TRANS_BUFF_MAX_LEN 18
#define TRANS_BUFF_MAX_NUM 1000
#define TRANS_BUFF_MAX_LEVEL ((TRANS_BUFF_MAX_LEN) - 1)
#define TRANS_DELAY_TIME 100
#define TRANS_IP_VERSION_MASK 4

#define TRANS_MIN(x, y) (((x) < (y)) ? (x) : (y))

enum TRANS_PRIM_ID {
    TRANS_IP_PRE = 0xD317,
    TRANS_IP_POST = 0xD318,
    TRANS_IPV4_FORWARD = 0xD326,
    TRANS_IPV4_LOCAL_IN = 0xD327,
    TRANS_IPV4_LOCAL_OUT = 0xD328,
    TRANS_IPV6_FORWARD = 0xD329,
    TRANS_IPV6_LOCAL_IN = 0xD32A,
    TRANS_IPV6_LOCAL_OUT = 0xD32B,
    TRANS_BUTTOM
};

struct trans_hook_entry {
    unsigned short enable;
    unsigned short init;
    unsigned int registed;
    struct nf_hook_ops hook_ops;
};

struct trans_hook_ctx {
    unsigned int trans_cnt[TRANS_HOOK_ENUM_BUTT];

    unsigned int trans_succ;
    unsigned int trans_fail;
};

struct trans_report_buff_node {
    struct list_head list;
    unsigned int pos;
    struct net_om_report_default_pkt buff[TRANS_BUFF_MAX_LEN];
};

struct trans_report_buff_entry {
    int is_enable;
    int type;
    struct trans_report_buff_node *cur_buff_node;
    spinlock_t buff_lock;
    struct delayed_work trans_work;
    struct list_head free_pool;
    struct list_head busy_pool;
};

struct trans_report_buff_ctx {
    struct trans_report_buff_entry buff_entry[TRANS_HOOK_ENUM_BUTT];
    struct workqueue_struct *trans_queue;
};

struct net_om_report_ctx g_net_om_report_ctx = {0};
struct trans_report_buff_ctx g_trans_report_hids_buff;

struct trans_hook_entry g_trans_hook_ctx[TRANS_HOOK_ENUM_BUTT];
struct trans_hook_ctx g_trans_hook_cnt;

#define pr_trans_fmt(args) "[trans_report]" args

#define trans_print(args, ...) do { \
    printk(KERN_EMERG pr_trans_fmt(args), ##__VA_ARGS__); \
} while (0)

struct net_om_report_default_pkt *trans_report_get_buff(enum net_om_hook_id trans_type)
{
    struct net_om_report_default_pkt *buff = NULL;
    struct trans_report_buff_node *buff_node = NULL;
    struct trans_report_buff_entry *buff_entry = NULL;
    unsigned long flags;

    buff_entry = &g_trans_report_hids_buff.buff_entry[trans_type];

    if (buff_entry->is_enable == 0) {
        g_net_om_report_ctx.stat_not_enable++;
        return NULL;
    }

    spin_lock_irqsave(&buff_entry->buff_lock, flags);

    /* get new buffer */
    if (buff_entry->cur_buff_node == NULL) {
        if (list_empty(&buff_entry->free_pool)) {
            g_net_om_report_ctx.stat_buff_full++;
            spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
            return NULL;
        }
        buff_node = list_entry(buff_entry->free_pool.next, struct trans_report_buff_node, list);
        list_del(&buff_node->list);
        buff_entry->cur_buff_node = buff_node;
    }

    buff_node = buff_entry->cur_buff_node;
    buff = &buff_node->buff[buff_node->pos];
    buff_node->pos++;
    g_net_om_report_ctx.stat_buff_ok++;
    spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

    return buff;
}

static void trans_report_send_pkts_work(struct work_struct *work)
{
    struct trans_report_buff_entry *buff_entry = container_of(work, struct trans_report_buff_entry, trans_work.work);
    unsigned long flags;
    unsigned int data_len;
    struct trans_report_buff_node *buff_node = NULL;

    g_net_om_report_ctx.stat_send_process++;

    spin_lock_irqsave(&buff_entry->buff_lock, flags);
    buff_node = buff_entry->cur_buff_node;
    if (buff_node != NULL) {
        buff_node = buff_entry->cur_buff_node;
        buff_entry->cur_buff_node = NULL;
        list_add_tail(&buff_node->list, &buff_entry->busy_pool);
    }
    spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

    while (1) {
        /* prepare transreport param */
        spin_lock_irqsave(&buff_entry->buff_lock, flags);
        if (list_empty(&buff_entry->busy_pool)) {
            spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
            return;
        }
        buff_node = list_entry(buff_entry->busy_pool.next, struct trans_report_buff_node, list);
        data_len = sizeof(struct net_om_report_default_pkt) * buff_node->pos;

        /* set msgid by type */
        g_net_om_report_ctx.stat_last_num = buff_node->pos;
        g_net_om_report_ctx.stat_last_length = data_len;
        g_net_om_report_ctx.stat_addr_trans_byte += data_len;
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

        bsp_weth_rc_pkt_transreport(buff_node->buff, data_len);

        /* transreport success, move node to free list */
        spin_lock_irqsave(&buff_entry->buff_lock, flags);
        list_del(&buff_node->list);
        buff_node->pos = 0;
        list_add_tail(&buff_node->list, &buff_entry->free_pool);
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

        g_net_om_report_ctx.stat_addr_trans_succ++;
    }
}

void trans_report_queue_pkts(enum net_om_hook_id trans_type)
{
    struct trans_report_buff_entry *buff_entry = NULL;
    struct trans_report_buff_node *buff_node = NULL;
    unsigned long flags;

    buff_entry = &g_trans_report_hids_buff.buff_entry[trans_type];

    spin_lock_irqsave(&buff_entry->buff_lock, flags);
    buff_node = buff_entry->cur_buff_node;
    if (buff_node != NULL && buff_node->pos >= TRANS_BUFF_MAX_LEVEL) {
        buff_node = buff_entry->cur_buff_node;
        buff_entry->cur_buff_node = NULL;
        list_add_tail(&buff_node->list, &buff_entry->busy_pool);
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
        queue_delayed_work(g_trans_report_hids_buff.trans_queue, &buff_entry->trans_work, 0);
        return;
    }
    spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
    queue_delayed_work(g_trans_report_hids_buff.trans_queue, &buff_entry->trans_work, TRANS_DELAY_TIME);

    return;
}

void trans_report_enable_by_type(enum net_om_hook_id trans_type, int enable)
{
    int i;
    unsigned long flags;
    struct trans_report_buff_node *buff_node = NULL;
    struct trans_report_buff_entry *buff_entry = NULL;

    if (trans_type >= TRANS_HOOK_ENUM_BUTT) {
        return;
    }

    buff_entry = &g_trans_report_hids_buff.buff_entry[trans_type];
    spin_lock_irqsave(&buff_entry->buff_lock, flags);

    /* setting is same, do nothing */
    if (enable == buff_entry->is_enable) {
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
        return;
    }

    buff_entry->is_enable = enable;
    if (enable == 0) {
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
        goto disable;
    }

    for (i = 0; i < TRANS_BUFF_MAX_NUM; i++) {
        buff_node = kzalloc(sizeof(struct trans_report_buff_node), GFP_KERNEL);
        if (buff_node == NULL) {
            spin_unlock_irqrestore(&buff_entry->buff_lock, flags);
            goto disable;
        }
        list_add_tail(&buff_node->list, &buff_entry->free_pool);
    }
    spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

    return;

disable:
    spin_lock_irqsave(&buff_entry->buff_lock, flags);
    while (!list_empty(&buff_entry->free_pool)) {
        buff_node = list_entry(buff_entry->free_pool.next, struct trans_report_buff_node, list);
        list_del(&buff_node->list);
        kfree(buff_node);
    }
    spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

    return;
}

void bsp_trans_enable_hook(enum net_om_hook_id hook_id, unsigned int enable)
{
    struct trans_hook_entry *hook_entry = &g_trans_hook_ctx[0];

    if (hook_id >= TRANS_HOOK_ENUM_BUTT) {
        trans_print("hook_id fail\n");
        return;
    }
    trans_report_enable_by_type(hook_id, enable);
    hook_entry[hook_id].enable = enable;
    return;
}

static void trans_mask_ipv4(struct iphdr *ip_hdr)
{
    if (ip_hdr == NULL) {
        return;
    }

    ip_hdr->saddr = ip_hdr->saddr & TRANS_IPV4_MASK;
    ip_hdr->daddr = ip_hdr->daddr & TRANS_IPV4_MASK;
}

static void trans_mask_ipv6(struct ipv6hdr *ip6_hdr)
{
    unsigned long long *saddr = NULL;
    unsigned long long *daddr = NULL;

    if (ip6_hdr == NULL) {
        return;
    }
    saddr = (unsigned long long *)&ip6_hdr->saddr.in6_u;
    daddr = (unsigned long long *)&ip6_hdr->daddr.in6_u;

    *saddr = *saddr & TRANS_IPV6_MASK;
    *daddr = *daddr & TRANS_IPV6_MASK;
}

static unsigned int g_hook_all_pkt;
void bsp_net_report_enable_all_pkt(unsigned int enable)
{
    g_hook_all_pkt = enable;
    trans_print("enable:%u\n", enable);
    return;
}

void trans_get_default_pkt_info(const struct sk_buff *skb, const char *net_if_name, enum TRANS_PRIM_ID prim_id,
                                enum net_om_hook_id hook_id)
{
    struct net_om_report_default_pkt *msg_info = NULL;
    unsigned int ip_version;
    int ret;

    msg_info = trans_report_get_buff(hook_id);
    if (msg_info == NULL) {
        return;
    }

    msg_info->msg_hdr.prim_id = prim_id;
    msg_info->msg_hdr.tool_id = 0;
    ret = memcpy_s(msg_info->msg_hdr.net_if_name, TRANS_IF_NAME_SIZE, net_if_name, ETH_HLEN);
    if (ret != 0) {
        trans_print("memcpy_s fail\n");
        goto fail;
    }
    msg_info->msg_hdr.len = TRANS_PKT_HDR_LEN;

    /* copy eth_pkt */
    ret = memcpy_s(msg_info->msg_hdr.data, TRANS_PKT_HDR_LEN, (void *)((uintptr_t)skb->data - ETH_HLEN),
                   TRANS_PKT_HDR_LEN - 1);
    if (ret != 0) {
        trans_print("memcpy_s fail\n");
        goto fail;
    }
    ip_version = ((unsigned int)*skb->data) >> TRANS_IP_VERSION_MASK;
#ifdef TRANS_REPORT_USER
    {
        struct iphdr *ip_hdr = NULL;
        struct ipv6hdr *ipv6_hdr = NULL;
        /* user version mask ip addr */
        if (ip_version == IPVERSION) {
            ip_hdr = (struct iphdr *)(msg_info->data + ETH_HLEN);
            trans_mask_ipv4(ip_hdr);
        } else if (ip_version == TRANS_IPV6VERSION) {
            ipv6_hdr = (struct ipv6hdr *)(msg_info->data + ETH_HLEN);
            trans_mask_ipv6(ipv6_hdr);
        }
    }
#endif
    trans_report_queue_pkts(hook_id);

fail:

    if (ret != 0) {
        g_trans_hook_cnt.trans_fail++;
    } else {
        g_trans_hook_cnt.trans_succ++;
    }
    return;
}

#ifdef TRANS_REPORT_USER
void net_om_mask_ip_addr(unsigned int ip_version, struct net_om_msg_info *msg_info)
{
    struct iphdr *ip_hdr = NULL;
    struct ipv6hdr *ipv6_hdr = NULL;

    /* user version mask ip addr */
    if (ip_version == IPVERSION) {
        ip_hdr = (struct iphdr *)(msg_info->data + ETH_HLEN);
        trans_mask_ipv4(ip_hdr);
    } else if (ip_version == TRANS_IPV6VERSION) {
        ipv6_hdr = (struct ipv6hdr *)(msg_info->data + ETH_HLEN);
        trans_mask_ipv6(ipv6_hdr);
    }
}
#else
void net_om_get_pkt_info(unsigned int ip_version, struct net_om_msg_info *msg_info)
{
    return;
}
#endif

void trans_get_all_pkt_info(const struct sk_buff *skb, const char *net_if_name, enum TRANS_PRIM_ID prim_id)
{
    struct net_om_msg_info *msg_info = NULL;
    unsigned int data_len;
    unsigned int need_mem_len;
    unsigned int buff_size;
    unsigned int msg_hdr_size;
    unsigned int ip_version;
    int ret;

    msg_hdr_size = sizeof(struct net_om_msg_info);
    data_len = skb->len + ETH_HLEN;
    need_mem_len = data_len + msg_hdr_size;
    buff_size = TRANS_MIN(need_mem_len, TRANS_MAX_LEN);
    msg_info = kmalloc(buff_size, GFP_ATOMIC);
    if (msg_info == NULL) {
        trans_print("no mem\n");
        goto fail;
    }

    msg_info->prim_id = prim_id;
    msg_info->tool_id = 0;
    ret = memcpy_s(msg_info->net_if_name, TRANS_IF_NAME_SIZE, net_if_name, ETH_HLEN);
    if (ret != 0) {
        trans_print("memcpy_s fail\n");
        goto fail;
    }
    msg_info->len = TRANS_MIN(data_len, buff_size - msg_hdr_size);

    /* copy eth_pkt */
    ret = memcpy_s(msg_info->data, msg_info->len, (void *)((uintptr_t)skb->data - ETH_HLEN), data_len);
    if (ret != 0) {
        trans_print("memcpy_s fail\n");
        goto fail;
    }
    ip_version = ((unsigned int)*skb->data) >> TRANS_IP_VERSION_MASK;
#ifdef TRANS_REPORT_USER
    net_om_mask_ip_addr(ip_version, msg_info);
#endif
    bsp_weth_rc_pkt_transreport((void *)msg_info, buff_size);

fail:
    if (msg_info != NULL) {
        kfree(msg_info);
    }

    if (ret != 0) {
        g_trans_hook_cnt.trans_fail++;
    } else {
        g_trans_hook_cnt.trans_succ++;
    }
    return;
}

void trans_report_get_pkt_info(const struct sk_buff *skb, const char *net_if_name, enum TRANS_PRIM_ID prim_id,
                               enum net_om_hook_id hook_id)
{
    if (skb->len == 0 || net_if_name == NULL) {
        trans_print("pkt err\n");
        return;
    }

    g_trans_hook_cnt.trans_cnt[prim_id - TRANS_IP_PRE]++;
    if (g_hook_all_pkt == 0) {
        trans_get_default_pkt_info(skb, net_if_name, prim_id, hook_id);
    } else {
        trans_get_all_pkt_info(skb, net_if_name, prim_id);
    }
}

unsigned int trans_report_hook_ip4_pre(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP4_PRE_ROUTING].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IP_PRE, TRANS_HOOK_IP4_PRE_ROUTING);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip4_post(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP4_POST_ROUTING].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->out->name, TRANS_IP_POST, TRANS_HOOK_IP4_POST_ROUTING);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip4_local_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP4_LOCAL_IN].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IPV4_LOCAL_IN, TRANS_HOOK_IP4_LOCAL_IN);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip4_local_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP4_LOCAL_OUT].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->out->name, TRANS_IPV4_LOCAL_OUT, TRANS_HOOK_IP4_LOCAL_OUT);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip4_forward(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP4_FORWARD].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IPV4_FORWARD, TRANS_HOOK_IP4_FORWARD);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip6_pre(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP6_PRE_ROUTING].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IP_PRE, TRANS_HOOK_IP6_PRE_ROUTING);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip6_post(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP6_POST_ROUTING].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->out->name, TRANS_IP_POST, TRANS_HOOK_IP6_POST_ROUTING);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip6_local_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP6_LOCAL_IN].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IPV6_LOCAL_IN, TRANS_HOOK_IP6_LOCAL_IN);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip6_local_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP6_LOCAL_OUT].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->out->name, TRANS_IPV6_LOCAL_OUT, TRANS_HOOK_IP6_LOCAL_OUT);
    return NF_ACCEPT;
}

unsigned int trans_report_hook_ip6_forward(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (g_trans_hook_ctx[TRANS_HOOK_IP6_FORWARD].enable == 0) {
        return NF_ACCEPT;
    }
    trans_report_get_pkt_info(skb, state->in->name, TRANS_IPV6_FORWARD, TRANS_HOOK_IP6_FORWARD);
    return NF_ACCEPT;
}

void trans_report_hook_each_entry_init(enum net_om_hook_id id, nf_hookfn *hook_cb, unsigned int protol,
                                       enum nf_inet_hooks hook_num, int priority)
{
    struct trans_hook_entry *hook_ctx = &(g_trans_hook_ctx[0]);

    hook_ctx[id].enable = 0;
    hook_ctx[id].init = TRANS_HOOK_ENABLE;
    hook_ctx[id].hook_ops.hook = hook_cb;
    hook_ctx[id].hook_ops.priv = NULL;
    hook_ctx[id].hook_ops.pf = protol;
    hook_ctx[id].hook_ops.hooknum = hook_num;
    hook_ctx[id].hook_ops.priority = priority;

    return;
}

static void trans_report_hook_init(void)
{
    struct trans_hook_entry *hook_ctx = &(g_trans_hook_ctx[0]);
    unsigned int ctx_len;
    int ret;

    ctx_len = sizeof(struct trans_hook_entry) * TRANS_HOOK_ENUM_BUTT;
    ret = memset_s(hook_ctx, ctx_len, 0, ctx_len);
    if (ret != 0) {
        trans_print("memset_s fail\n");
        return;
    }
    /* ipv4 hooks add here */
    trans_report_hook_each_entry_init(TRANS_HOOK_IP4_PRE_ROUTING, trans_report_hook_ip4_pre, NFPROTO_IPV4,
                                      NF_INET_PRE_ROUTING, NF_IP_PRI_MANGLE);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP4_POST_ROUTING, trans_report_hook_ip4_post, NFPROTO_IPV4,
                                      NF_INET_POST_ROUTING, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP4_LOCAL_IN, trans_report_hook_ip4_local_in, NFPROTO_IPV4,
                                      NF_INET_LOCAL_IN, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP4_LOCAL_OUT, trans_report_hook_ip4_local_out, NFPROTO_IPV4,
                                      NF_INET_LOCAL_OUT, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP4_FORWARD, trans_report_hook_ip4_forward, NFPROTO_IPV4,
                                      NF_INET_FORWARD, NF_IP_PRI_SELINUX_LAST);

    /* ipv6 hooks add here */
    trans_report_hook_each_entry_init(TRANS_HOOK_IP6_PRE_ROUTING, trans_report_hook_ip6_pre, NFPROTO_IPV6,
                                      NF_INET_PRE_ROUTING, NF_IP_PRI_MANGLE);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP6_POST_ROUTING, trans_report_hook_ip6_post, NFPROTO_IPV6,
                                      NF_INET_POST_ROUTING, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP6_LOCAL_IN, trans_report_hook_ip6_local_in, NFPROTO_IPV6,
                                      NF_INET_LOCAL_IN, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP6_LOCAL_OUT, trans_report_hook_ip6_local_out, NFPROTO_IPV6,
                                      NF_INET_LOCAL_OUT, NF_IP_PRI_SELINUX_LAST);

    trans_report_hook_each_entry_init(TRANS_HOOK_IP6_FORWARD, trans_report_hook_ip6_forward, NFPROTO_IPV6,
                                      NF_INET_FORWARD, NF_IP_PRI_SELINUX_LAST);
    /* add hook init here */
}

static void trans_report_unregister_hook(struct nf_hook_ops *reg)
{
    struct net *net = NULL;

    rtnl_lock();
    for_each_net(net)
    {
        nf_unregister_net_hook(net, reg);
    }
    rtnl_unlock();

    return;
}

void trans_report_unregister_hooks(void)
{
    struct trans_hook_entry *hook_ctx = &(g_trans_hook_ctx[0]);
    struct nf_hook_ops *hook_ops = NULL;
    unsigned int i;

    for (i = 0; i < TRANS_HOOK_ENUM_BUTT; i++) {
        if (hook_ctx[i].init != TRANS_HOOK_ENABLE) {
            continue;
        }
        hook_ops = &(hook_ctx[i].hook_ops);
        trans_report_unregister_hook(hook_ops);
        hook_ctx[i].registed = 0;
    }
}

static int trans_report_register_hook(struct nf_hook_ops *reg)
{
    struct net *net = NULL;
    struct net *last = NULL;
    unsigned int roll_back = 0;
    int ret;

    rtnl_lock();

    for_each_net(net)
    {
        ret = nf_register_net_hook(net, reg);
        if ((ret != 0) && (ret != -ENOENT)) {
            trans_print("register fail\n");
            roll_back = 1;
            break;
        }
    }

    if (roll_back == 1) {
        last = net;
        for_each_net(net)
        {
            if (net == last) {
                break;
            }
            nf_unregister_net_hook(net, reg);
        }
    } else {
        ret = 0;
    }
    rtnl_unlock();

    return ret;
}

static int trans_report_register_hooks(void)
{
    struct trans_hook_entry *hook_ctx = &(g_trans_hook_ctx[0]);
    struct nf_hook_ops *hook_ops = NULL;
    unsigned int i;
    int ret;

    for (i = 0; i < TRANS_HOOK_ENUM_BUTT; i++) {
        if (hook_ctx[i].init != TRANS_HOOK_ENABLE) {
            continue;
        }
        hook_ops = &(hook_ctx[i].hook_ops);
        ret = trans_report_register_hook(hook_ops);
        if (ret != 0) {
            trans_print("hook:%u register fail\n", i);
            trans_report_unregister_hooks();
            return -1;
        }
        hook_ctx[i].registed = 1;
    }

    return 0;
}

int trans_report_rc_init(void)
{
    int ret;
    unsigned int i;
    struct trans_report_buff_entry *buff_entry = NULL;

    trans_print("hook init start\n");

    g_trans_report_hids_buff.trans_queue = create_singlethread_workqueue("trans_report");
    if (g_trans_report_hids_buff.trans_queue == NULL) {
        trans_print("create work fail\n");
        return -1;
    }

    for (i = 0; i < (unsigned int)TRANS_HOOK_ENUM_BUTT; i++) {
        buff_entry = &g_trans_report_hids_buff.buff_entry[i];

        spin_lock_init(&buff_entry->buff_lock);
        INIT_DELAYED_WORK(&buff_entry->trans_work, trans_report_send_pkts_work);
        INIT_LIST_HEAD(&buff_entry->free_pool);
        INIT_LIST_HEAD(&buff_entry->busy_pool);
        buff_entry->is_enable = 0;
        buff_entry->type = i;
    }

    trans_report_hook_init();
    ret = trans_report_register_hooks();
    if (ret != 0) {
        trans_print("hook init fail\n");
        return -1;
    }

    ret = memset_s(&g_trans_hook_cnt, sizeof(g_trans_hook_cnt), 0, sizeof(struct trans_hook_ctx));
    if (ret != 0) {
        trans_print("memset_s fail\n");
        return ret;
    }

    trans_print("hook init succ\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(trans_report_rc_init);
#endif

void bsp_net_enable_hooks(void)
{
    bsp_trans_enable_hook(TRANS_HOOK_IP4_PRE_ROUTING, 1);
    bsp_trans_enable_hook(TRANS_HOOK_IP4_POST_ROUTING, 1);
    bsp_trans_enable_hook(TRANS_HOOK_IP6_PRE_ROUTING, 1);
    bsp_trans_enable_hook(TRANS_HOOK_IP6_POST_ROUTING, 1);
}

void trans_report_dump(void)
{
    printk("[trans_report] stat_addr_err          :%u \n", g_net_om_report_ctx.stat_addr_err);
    printk("[trans_report] stat_addr_set_err      :%u \n", g_net_om_report_ctx.stat_addr_set_err);
    printk("[trans_report] stat_addr_pkt_other    :%u \n", g_net_om_report_ctx.stat_addr_pkt_other);
    printk("[trans_report] stat_addr_pkt_succ     :%u \n", g_net_om_report_ctx.stat_addr_pkt_succ);
    printk("[trans_report] stat_addr_protocol_err :%u \n", g_net_om_report_ctx.stat_addr_protocol_err);
    printk("[trans_report] stat_addr_trans_fail   :%u \n", g_net_om_report_ctx.stat_addr_trans_fail);
    printk("[trans_report] stat_addr_trans_succ   :%u \n", g_net_om_report_ctx.stat_addr_trans_succ);
    printk("[trans_report] stat_buff_ok           :%u \n", g_net_om_report_ctx.stat_buff_ok);
    printk("[trans_report] stat_buff_full         :%u \n", g_net_om_report_ctx.stat_buff_full);
    printk("[trans_report] stat_alloc_fail        :%u \n", g_net_om_report_ctx.stat_alloc_fail);
    printk("[trans_report] stat_buff_err          :%u \n", g_net_om_report_ctx.stat_buff_err);
    printk("[trans_report] stat_addr_trans_byte   :%u \n", g_net_om_report_ctx.stat_addr_trans_byte);
    printk("[trans_report] stat_send_process      :%u \n", g_net_om_report_ctx.stat_send_process);
    printk("[trans_report] stat_last_err          :%x \n", g_net_om_report_ctx.stat_last_err);
    printk("[trans_report] stat_last_length       :%u \n", g_net_om_report_ctx.stat_last_length);
    printk("[trans_report] stat_last_num          :%u \n", g_net_om_report_ctx.stat_last_num);
    printk("[trans_report] stat_not_enable        :%u \n", g_net_om_report_ctx.stat_not_enable);

    printk("[trans_report] stat_rc_default_pkt    :%u \n", g_net_om_report_ctx.stat_rc_default_pkt);
    printk("[trans_report] stat_rc_all_pkt        :%u \n", g_net_om_report_ctx.stat_rc_all_pkt);
    printk("[trans_report] stat_rc_pkt_succ       :%u \n", g_net_om_report_ctx.stat_rc_pkt_succ);
    printk("[trans_report] stat_rc_pkt_fail       :%u \n", g_net_om_report_ctx.stat_rc_pkt_fail);

    return;
}

void trans_report_entry_dump(void)
{
    struct trans_hook_entry *hook_ctx = &(g_trans_hook_ctx[0]);
    unsigned int i;

    for (i = 0; i < TRANS_HOOK_ENUM_BUTT; i++) {
        if (hook_ctx[i].enable == TRANS_HOOK_ENABLE) {
            trans_print("g_trans_hook_ctx[%d] enable\n", i);
        } else {
            trans_print("g_trans_hook_ctx[%d] disable\n", i);
        }
    }

    for (i = 0; i < TRANS_HOOK_ENUM_BUTT; i++) {
        if (hook_ctx[i].registed == 1) {
            trans_print("g_trans_hook_ctx[%d] registed\n", i);
        } else {
            trans_print("g_trans_hook_ctx[%d] unregisted\n", i);
        }
    }

    return;
}

void trans_report_hook_cnt(void)
{
    unsigned int i;

    for (i = 0; i < TRANS_HOOK_ENUM_BUTT; i++) {
        trans_print("trans_cnt[%d] %u\n", i, g_trans_hook_cnt.trans_cnt[i]);
    }

    trans_print("trans_succ %u\n", g_trans_hook_cnt.trans_succ);
    trans_print("trans_fail %u\n", g_trans_hook_cnt.trans_fail);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
