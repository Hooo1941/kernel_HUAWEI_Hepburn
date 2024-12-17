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
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include <net/ip.h>

#include <mdrv_diag.h>
#include <drv_comm.h>
#include "bsp_slice.h"
#include "bsp_pfa.h"
#include "bsp_net_om.h"
#include "bsp_diag.h"
#include "bsp_dra.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define TRANS_IP_HDR_MAX_LEN 60
#define TRANS_IPV6VERSION 6
#define TRANS_BUFF_MAX_LEN 180
#define TRANS_BUFF_DEFAULT_NUM 100
#define TRANS_BUFF_MAX_NUM 1000
#define TRANS_BUFF_MAX_LEVEL ((TRANS_BUFF_MAX_LEN) - 1)
#define TRANS_ICMP_MAGIC 0xABCD
#define TRANS_UNKNOWN_MAGIC 0xBAD
#define TRANS_PKT_MSGID 0x9f383340

#define TRANS_DELAY_TIME 100
#define TRANS_IP_VERSION_MASK 4
#define TRANS_IP_ID_MASK 8

#define NET_OM_DMA_MASK 64
static unsigned long long g_net_om_dma_mask = ~0ULL;
struct platform_device g_net_om_device = {
    .name = "net_om",
    .id = -1,
    .num_resources = 0,
};

struct ip_l4_hdr {
    unsigned int source : 16;
    unsigned int dest : 16;
    unsigned int seq;
    unsigned int ack_seq;
};

struct trans_report_buff_node {
    struct list_head list;
    unsigned int pos;
    struct net_om_report_pkt_info buff[TRANS_BUFF_MAX_LEN];
};

struct trans_report_buff_entry {
    int is_enable;
    int type;
    struct trans_report_buff_node *cur_buff_node;
    spinlock_t buff_lock;
    struct net_om_diag_info trans_para;
    struct delayed_work trans_work;
    struct list_head free_pool;
    struct list_head busy_pool;
};

struct trans_report_buff_ctx {
    struct trans_report_buff_entry buff_entry[NET_OM_TYPE_MAX];
    struct workqueue_struct *trans_queue;
};

struct net_om_report_ctx g_net_om_report_ctx = {0};
struct trans_report_buff_ctx g_trans_report_hids_buff;

struct net_om_report_pkt_info *trans_report_get_buff(enum net_om_report_type trans_type)
{
    struct net_om_report_pkt_info *buff = NULL;
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
    int ret;
    unsigned long flags;
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
        buff_entry->trans_para.ulLength = sizeof(struct net_om_report_pkt_info) * buff_node->pos;
        buff_entry->trans_para.pData = buff_node->buff;

        /* set msgid by type */
        buff_entry->trans_para.ulMsgId = buff_entry->trans_para.ulMsgId + buff_entry->type;
        g_net_om_report_ctx.stat_last_num = buff_node->pos;
        g_net_om_report_ctx.stat_last_length = buff_entry->trans_para.ulLength;
        g_net_om_report_ctx.stat_addr_trans_byte += buff_entry->trans_para.ulLength;
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

        ret = bsp_diag_trans_report(buff_entry->trans_para.ulMsgId, DIAG_DRV_HDS_PID, buff_entry->trans_para.pData, buff_entry->trans_para.ulLength);
        g_net_om_report_ctx.stat_last_err = ret;
        if (ret != BSP_OK) {
            g_net_om_report_ctx.stat_addr_trans_fail++;
            buff_entry->trans_para.ulMsgId = NET_OM_PKT_MSGID_BASE;
            queue_delayed_work(g_trans_report_hids_buff.trans_queue, &buff_entry->trans_work, 1);
            return;
        }

        /* transreport success, move node to free list */
        spin_lock_irqsave(&buff_entry->buff_lock, flags);
        list_del(&buff_node->list);
        buff_node->pos = 0;
        list_add_tail(&buff_node->list, &buff_entry->free_pool);

        /* restore the msg id */
        buff_entry->trans_para.ulMsgId = NET_OM_PKT_MSGID_BASE;
        spin_unlock_irqrestore(&buff_entry->buff_lock, flags);

        g_net_om_report_ctx.stat_addr_trans_succ++;
    }
    return;
}

void trans_report_queue_pkts(enum net_om_report_type trans_type)
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

/* if pfa is disable, use jiffies time */
static inline unsigned int trans_report_get_slice_time(void)
{
    if (g_net_om_report_ctx.last_jiff != jiffies) {
        bsp_slice_getcurtime(&g_net_om_report_ctx.last_slice);
    }
    g_net_om_report_ctx.last_jiff = jiffies;
    return (unsigned int)g_net_om_report_ctx.last_slice;
}

void bsp_net_report_set_time(void)
{
    bsp_slice_getcurtime(&g_net_om_report_ctx.last_slice);
}

void bsp_net_report_set_buff_cnt(unsigned int buf_cnt)
{
    if (buf_cnt > TRANS_BUFF_MAX_NUM) {
        buf_cnt = TRANS_BUFF_MAX_NUM;
    }
    g_net_om_report_ctx.buf_cnt = buf_cnt;
}

void bsp_net_report_enable_by_type(enum net_om_report_type trans_type, int enable)
{
    int i;
    unsigned long flags;
    struct trans_report_buff_node *buff_node = NULL;
    struct trans_report_buff_entry *buff_entry = NULL;

    if (trans_type >= NET_OM_TYPE_MAX) {
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

    for (i = 0; i < g_net_om_report_ctx.buf_cnt; i++) {
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

void netom_get_ipv4_pkt_info(char *data, struct net_om_report_pkt_info *pkt_info)
{
    struct iphdr *ipv4_hdr = NULL;
    struct ip_l4_hdr *l4_hdr = NULL;

    ipv4_hdr = (struct iphdr *)data;
    l4_hdr = (struct ip_l4_hdr *)(data + sizeof(struct iphdr));

    pkt_info->l3_id = ntohs(ipv4_hdr->id);
    pkt_info->l3_version = (unsigned short)ipv4_hdr->version;
    pkt_info->l4_protocol = (unsigned short)ipv4_hdr->protocol;
    if (ipv4_hdr->protocol == IPPROTO_TCP) {
        pkt_info->l4_seq = ntohl(l4_hdr->seq);
        pkt_info->l4_ack = ntohl(l4_hdr->ack_seq);
    } else if (ipv4_hdr->protocol == IPPROTO_UDP) {
        pkt_info->l4_seq = ntohs(l4_hdr->source);
        pkt_info->l4_ack = ntohs(l4_hdr->dest);
    } else if (ipv4_hdr->protocol == IPPROTO_ICMP) {
        pkt_info->l4_seq = TRANS_ICMP_MAGIC;
        pkt_info->l4_ack = TRANS_ICMP_MAGIC;
    } else {
        pkt_info->l4_seq = TRANS_UNKNOWN_MAGIC;
        pkt_info->l4_ack = TRANS_UNKNOWN_MAGIC;
        g_net_om_report_ctx.stat_addr_protocol_err++;
    }
}

void netom_get_ipv6_pkt_info(char *data, struct net_om_report_pkt_info *pkt_info)
{
    struct ipv6hdr *ipv6_hdr = NULL;
    struct ip_l4_hdr *l4_hdr = NULL;

    ipv6_hdr = (struct ipv6hdr *)data;
    l4_hdr = (struct ip_l4_hdr *)(data + sizeof(struct ipv6hdr));

    pkt_info->l3_id = ntohs(ipv6_hdr->flow_lbl[0] | (((unsigned int)ipv6_hdr->flow_lbl[1]) << TRANS_IP_ID_MASK));
    pkt_info->l3_version = (unsigned short)ipv6_hdr->version;
    pkt_info->l4_protocol = (unsigned short)ipv6_hdr->nexthdr;
    if (ipv6_hdr->nexthdr == IPPROTO_TCP) {
        pkt_info->l4_seq = ntohl(l4_hdr->seq);
        pkt_info->l4_ack = ntohl(l4_hdr->ack_seq);
    } else if (ipv6_hdr->nexthdr == IPPROTO_UDP) {
        pkt_info->l4_seq = ntohs(l4_hdr->source);
        pkt_info->l4_ack = ntohs(l4_hdr->dest);
    } else if (ipv6_hdr->nexthdr == IPPROTO_ICMP) {
        pkt_info->l4_seq = TRANS_ICMP_MAGIC;
        pkt_info->l4_ack = TRANS_ICMP_MAGIC;
    } else {
        pkt_info->l4_seq = TRANS_UNKNOWN_MAGIC;
        pkt_info->l4_ack = TRANS_UNKNOWN_MAGIC;
        g_net_om_report_ctx.stat_addr_protocol_err++;
    }
}

#define NETOM_PKT_TYPE_SIZE 2
void netom_get_other_pkt_info(char *data, struct net_om_report_pkt_info *pkt_info)
{
    unsigned short *type_data = (unsigned short *)(data - NETOM_PKT_TYPE_SIZE); // 获取非v4v6数据包类型

    pkt_info->l3_version = ntohs(*type_data++);
    pkt_info->l4_protocol = ntohs(*type_data++);

    pkt_info->l3_id = ntohs(*type_data++);
    pkt_info->l4_seq = ntohs(*type_data++);
    pkt_info->l4_ack = ntohs(*type_data);
}

/*
 * is_skb:1 : addr is skb->data
 * is_skb:0 : addr is dra
 * addr is IP Header
 * is_cache: is invalid cache for dma buffer
 */
void bsp_net_report_mark_pkt_info(unsigned long long addr, enum net_om_report_type trans_type, bool is_skb,
    bool is_cache)
{
    struct device *dev = &g_net_om_device.dev;
    struct net_om_report_pkt_info *pkt_info = NULL;
    struct trans_report_buff_entry *buff_entry = NULL;
    char *data = NULL;
    unsigned int ip_version;

    if (addr == 0 || trans_type >= NET_OM_TYPE_MAX) {
        return;
    }

    buff_entry = &g_trans_report_hids_buff.buff_entry[trans_type];
    /* disable always return */
    if (buff_entry->is_enable == 0) {
        return;
    }

    pkt_info = trans_report_get_buff(trans_type);
    if (pkt_info == NULL) {
        return;
    }

    if (is_cache) {
        unsigned long long phy = bsp_dra_to_phy(addr);
        dma_unmap_single(dev, phy - ETH_HLEN, TRANS_IP_HDR_MAX_LEN, DMA_FROM_DEVICE);
        data = (char *)phys_to_virt(phy);
    } else { /* for usb rx */
        data = (char *)((uintptr_t)addr);
    }

    ip_version = ((unsigned int)*data) >> TRANS_IP_VERSION_MASK;
    if (ip_version == IPVERSION) {
        netom_get_ipv4_pkt_info(data, pkt_info);
    } else if (ip_version == TRANS_IPV6VERSION) {
        netom_get_ipv6_pkt_info(data, pkt_info);
    } else {
        g_net_om_report_ctx.stat_addr_pkt_other++;
        netom_get_other_pkt_info(data, pkt_info);
    }

    bsp_slice_getcurtime((u64 *)&pkt_info->time_stamp);
    g_net_om_report_ctx.stat_addr_pkt_succ++;

    if (is_cache) {
        dma_map_single(dev, (void *)data, TRANS_IP_HDR_MAX_LEN, DMA_FROM_DEVICE);
    }

    trans_report_queue_pkts(trans_type);
    return;
}

void trans_report_rc_pkt(unsigned int msg_id, void *data, unsigned int len)
{
    int ret;

    ret = bsp_diag_trans_report(msg_id, DIAG_DRV_HDS_PID, data, len);
    if (ret != BSP_OK) {
        g_net_om_report_ctx.stat_rc_pkt_fail++;
    } else {
        g_net_om_report_ctx.stat_rc_pkt_succ++;
    }

    return;
}

void bsp_net_report_rc_pkt(unsigned int msg_id, void *data, unsigned int len)
{
    int ret;
    unsigned int i;
    unsigned int pkt_num;
    struct net_om_report_default_pkt *pkt_info = NULL;

    if (len % (sizeof(struct net_om_report_default_pkt)) == 0) {
        g_net_om_report_ctx.stat_rc_default_pkt++;

        pkt_num = len / (sizeof(struct net_om_report_default_pkt));
        pkt_info = (struct net_om_report_default_pkt *)data;
        for (i = 0; i < pkt_num; i++) {
            ret = bsp_diag_trans_report(msg_id, DIAG_DRV_HDS_PID, (unsigned char *)pkt_info, sizeof(struct net_om_report_default_pkt));
            if (ret != BSP_OK) {
                g_net_om_report_ctx.stat_rc_pkt_fail++;
            } else {
                g_net_om_report_ctx.stat_rc_pkt_succ++;
            }
            pkt_info++;
        }
    } else {
        g_net_om_report_ctx.stat_rc_all_pkt++;
        trans_report_rc_pkt(msg_id, data, len);
    }

    return;
}

static void net_om_dma_config(void)
{
    if (platform_device_register(&g_net_om_device)) {
        printk("[trans_report] register net om device fail\n");
    }
    g_net_om_device.dev.dma_mask = &g_net_om_dma_mask;
    of_dma_configure(&g_net_om_device.dev, NULL, true);
    dma_set_mask_and_coherent(&g_net_om_device.dev, DMA_BIT_MASK(NET_OM_DMA_MASK));

    return;
}

int trans_report_init(void)
{
    int i;
    struct trans_report_buff_entry *buff_entry = NULL;

    g_trans_report_hids_buff.trans_queue = create_singlethread_workqueue("trans_report");
    if (g_trans_report_hids_buff.trans_queue == NULL) {
        printk("[trans_report] create work fail\n");
        return -1;
    }

    for (i = 0; i < (int)NET_OM_TYPE_MAX; i++) {
        buff_entry = &g_trans_report_hids_buff.buff_entry[i];

        spin_lock_init(&buff_entry->buff_lock);
        INIT_DELAYED_WORK(&buff_entry->trans_work, trans_report_send_pkts_work);
        INIT_LIST_HEAD(&buff_entry->free_pool);
        INIT_LIST_HEAD(&buff_entry->busy_pool);
        buff_entry->trans_para.ulModule = 0xf0900;
        buff_entry->trans_para.ulPid = 0x8003;
        buff_entry->trans_para.ulMsgId = NET_OM_PKT_MSGID_BASE;
        buff_entry->is_enable = 0;
        buff_entry->type = i;
    }
    g_net_om_report_ctx.last_jiff = jiffies;
    g_net_om_report_ctx.buf_cnt = TRANS_BUFF_DEFAULT_NUM;

    net_om_dma_config();

    printk("[trans_report] init ok\n");
    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(trans_report_init);
#endif

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

void trans_report_enable_dump(void)
{
    unsigned int i;

    for (i = 0; i < NET_OM_TYPE_MAX; i++) {
        printk("[trans_report] buff_entry[%d]   :%u \n", i, g_trans_report_hids_buff.buff_entry[i].is_enable);
    }

    return;
}

EXPORT_SYMBOL(g_net_om_report_ctx);
EXPORT_SYMBOL(bsp_net_report_set_time);
EXPORT_SYMBOL(bsp_net_report_enable_by_type);
EXPORT_SYMBOL(bsp_net_report_mark_pkt_info);
EXPORT_SYMBOL(bsp_net_report_rc_pkt);
EXPORT_SYMBOL(trans_report_dump);
EXPORT_SYMBOL(trans_report_enable_dump);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
