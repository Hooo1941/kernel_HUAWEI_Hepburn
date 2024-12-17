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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <uapi/linux/if_ether.h>
#include <linux/pci.h>
#include <asm-generic/io.h>
#include <linux/etherdevice.h>
#include <asm/barrier.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#include <linux/syscore_ops.h>
#include <net/ip.h>

#include "bsp_pcie.h"
#include "bsp_net_om.h"
#include "wan_eth_rc.h"
#include "wan_eth_common.h"
#include "wan_eth_table.h"
#include "wan_eth_event_buffer.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */

#define WETH_NAME_LEN 16
#define WETH_TX_TIMEOUT (8 * HZ)
#define MAX_PENDING_PACKETS 2048
#define RX_BUGET 128
#define BUF_SIZE_1_6KB 1560U

#define WETH_SET_VOTE_TIMEOUT() (jiffies + msecs_to_jiffies(WETH_VOTE_TIMEOUT))
#define GRO_STAT_LEN 2
#define VOTE_LOCK_LEN 10
#define WETH_MIN_VOTE_THRU 5
#define WETH_MAX_VOTE_THRU 200
#define WETH_MIN_IMS_ID 16
#define WETH_MAX_IMS_ID 19
#define WETH_IMS_MTU 2000
#define MAX_TRY_TIMES_FOR_LINKUP 50
#define OVER_SIZE_SRC 2
#define OVER_SIZE_DTS 1
#define ENABLE_PKT_REPORT 2

#define INIT_WEIGHT 1
#define FIRST_WEIGHT 2
#define SECOND_WEIGHT 8
#define THIRD_WEIGHT 64
#define FIRST_WEIGHT_THRU 20
#define SECOND_WEIGHT_THRU 30
#define THIRD_WEITHG_THRU 60
#define NO_DATA_DURATION 500
#define EVALUATION_DURATION 200
#define WAIT_COUNT_THRU 10000
#define INIT_CPU_MAX 10
#define SK_PACING_SHIFT 4

struct weth_rc_ctx g_weth_rc_ctx;
struct common_info g_common_info;
struct weth_common_ctx g_common_ctx;

STATIC void weth_rx_done_cb(unsigned int dev_id, struct sk_buff *skb, unsigned int rx_num);
STATIC void set_mac_header(struct weth_ctx *priv, struct sk_buff *skb);
STATIC int weth_rc_init(void);
STATIC int weth_rc_submit_tx(struct sk_buff *skb, unsigned int num);
STATIC void weth_xmit_work(struct work_struct *work);
void weth_rc_pkt_transreport(void *data, unsigned int size, unsigned int flag);

unsigned int g_weth_debug_level = 0x1;
int g_by_pass_mode __read_mostly = 1;

struct weth_ctx *g_weth_ctx[WETH_MUX_NUM];

static const uint8_t g_rc_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x04, 0x01
};
static const uint8_t g_ep_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x00, 0x01
};

struct weth_rc_ctx *weth_get_rc_ctx(void)
{
    return &g_weth_rc_ctx;
}

struct weth_ctx *weth_get_ctx(unsigned int devid)
{
    if (devid >= WETH_MUX_NUM) {
        return NULL;
    }
    return g_weth_ctx[devid];
}

int set_by_pass_mode(int value)
{
    g_by_pass_mode = value;
    return g_by_pass_mode;
}

int weth_set_dynamic_napi_weight(int value)
{
    g_weth_rc_ctx.dynamic_napi_weight = value;
    return g_weth_rc_ctx.dynamic_napi_weight;
}

int weth_set_threshold(unsigned int thres, unsigned int thres_up, unsigned int thres_down)
{
    g_common_ctx.speed_threshold = thres;
    g_common_ctx.speed_up_threshold = thres_up;
    g_common_ctx.speed_down_threshold = thres_down;

    return 0;
}

unsigned int weth_set_tx_timer_duration(unsigned int dev_id, unsigned int timer_duration)
{
    if (dev_id >= WETH_MUX_NUM) {
        return 0;
    }
    g_weth_ctx[dev_id]->tx_timer_duration = timer_duration;
    return g_weth_ctx[dev_id]->tx_timer_duration;
}

int weth_set_gro(unsigned int dev_id, int en)
{
    if (dev_id >= WETH_MUX_NUM) {
        return -1;
    }
    g_weth_ctx[dev_id]->gro_enable = en;
    return g_weth_ctx[dev_id]->gro_enable;
}

int weth_get_gro(unsigned int dev_id)
{
    if (dev_id >= WETH_MUX_NUM) {
        return -1;
    }
    return g_weth_ctx[dev_id]->gro_enable;
}

int weth_set_vote_count(int value)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    pctx->vote_count = value;
    return pctx->vote_count;
}

int weth_get_vote_count(void)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;
    return pctx->vote_count;
}

unsigned int weth_set_max_list_len(unsigned int dev_id, unsigned int max_list_len)
{
    if (dev_id >= WETH_MUX_NUM) {
        return 0;
    }
    g_weth_ctx[dev_id]->max_list_len = max_list_len;
    return g_weth_ctx[dev_id]->max_list_len;
}

int weth_set_debug_level(int value)
{
    g_weth_debug_level = value;
    return g_weth_debug_level;
}

void weth_set_weight(int init_weight, int first_weight, int second_weight, int third_weight)
{
    g_weth_rc_ctx.init_weight = init_weight;
    g_weth_rc_ctx.first_weight = first_weight;
    g_weth_rc_ctx.second_weight = second_weight;
    g_weth_rc_ctx.third_weight = third_weight;
}

void weth_set_weight_thru(int first_weight_thru, int second_weight_thru, int third_weight_thru)
{
    g_weth_rc_ctx.first_weight_thru = first_weight_thru;
    g_weth_rc_ctx.second_weight_thru = second_weight_thru;
    g_weth_rc_ctx.third_weight_thru = third_weight_thru;
}

void weth_set_duration(int no_data_duration, int evaluate_duration)
{
    g_weth_rc_ctx.no_data_duration = no_data_duration;
    g_weth_rc_ctx.no_data_jiffies = msecs_to_jiffies(g_weth_rc_ctx.no_data_duration);

    g_weth_rc_ctx.evaluate_duration = evaluate_duration;
    g_weth_rc_ctx.evaluate_jiffies = msecs_to_jiffies(g_weth_rc_ctx.evaluate_duration);
}

static unsigned int atoi(char *s)
{
    char *p = s;
    char c;
    unsigned long ret = 0;

    if (s == NULL) {
        return 0;
    }

    while ((c = *p++) != '\0') {
        if ('0' <= c && c <= '9') {
            ret *= 10; /* ret times 10 */
            ret += (unsigned long)((unsigned char)c - '0');
            if (ret > U32_MAX)
                return 0;
        } else {
            break;
        }
    }
    return (unsigned int)ret;
}

STATIC unsigned int wan_eth_build_user_field2(struct sk_buff *skb)
{
    return 0;
}

STATIC int weth_set_userfield(struct sk_buff *skb, unsigned int devid)
{
    struct iphdr *ipheader = NULL;
    unsigned short protocol;
    int send_immediately = 0;
    struct remote_eth_cb_map_s *map_info = (struct remote_eth_cb_map_s *)skb->cb;

    protocol = ((struct ethhdr *)skb->data)->h_proto;
    map_info->userfield0 = protocol;
    map_info->userfield0 = (map_info->userfield0 << 16) | devid; /* get high 16 bits */
    map_info->userfield2 = wan_eth_build_user_field2(skb);
    weth_pr_dbg("map_info->userfield2: %x\n", map_info->userfield2);

    if (protocol == htons(ETH_P_IP)) {
        ipheader = ip_hdr(skb);
        weth_pr_dbg("more_frag: %d\n", ipheader->frag_off & htons(IP_MF));
        send_immediately = (ipheader->protocol == IPPROTO_ICMP) && ((ipheader->frag_off & htons(IP_MF)) == 0);
    }

    return send_immediately;
}

STATIC void set_mac_header(struct weth_ctx *priv, struct sk_buff *skb)
{
    int ret;
    struct ethhdr *eth = NULL;
    struct remote_eth_cb_map_s *map_info = (struct remote_eth_cb_map_s *)skb->cb;

    if (map_info->userfield2 == WETH_ETH_MODE) {
        return;
    }

    skb_push(skb, ETH_HLEN);
    eth = (struct ethhdr *)skb->data;
    ret = memcpy_s(&eth->h_dest, ETH_ALEN, priv->ndev->dev_addr, ETH_ALEN);
    if (ret) {
        weth_pr_err("memcpy_s failed\n");
    }
    ret = memcpy_s(&eth->h_source, ETH_ALEN, g_ep_src_mac_addr, ETH_ALEN);
    if (ret) {
        weth_pr_err("memcpy_s failed\n");
    }

    eth->h_proto = (__be16)((map_info->userfield0 >> 16) & 0xffff); /* right shift 16, use 0xffff to get low 32 bits */

    return;
}

STATIC void weth_queue_packets(unsigned long data)
{
    struct weth_ctx *priv = (struct weth_ctx *)data;
    struct sk_buff *queue_skb = NULL;
    int ret;
    unsigned long flags;
    unsigned int tx_list_len;

    priv->stats.stat_tx_queue++;

    spin_lock_irqsave(&priv->tx_lock, flags);
    if (unlikely(priv->tx_tail_skb == NULL)) {
        priv->stats.stat_tx_none++;
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        return;
    }
    /* tail skb's next is the first skb in the list */
    queue_skb = priv->tx_tail_skb;
    tx_list_len = priv->tx_list_len;
    /* can be reused in xmit */
    priv->tx_list_len = 0;
    priv->tx_tail_skb = NULL;
    spin_unlock_irqrestore(&priv->tx_lock, flags);

    ret = weth_rc_submit_tx(queue_skb, tx_list_len);
    if (unlikely(ret)) {
        priv->stats.stat_tx_submit_fail++;
    }

    return;
}

STATIC struct weth_ctx *weth_xmit_get_priv(struct net_device *ndev)
{
    if (ndev != NULL) {
        return (struct weth_ctx *)netdev_priv(ndev);
    } else {
        return (struct weth_ctx *)netdev_priv(g_weth_ctx[0]->ndev);
    }
}

STATIC int weth_xmit_pre_check_skb(struct weth_ctx *priv, struct sk_buff *skb)
{
    struct remote_eth_cb_map_s *map_info = NULL;

    if (unlikely(skb->len < ETH_HLEN)) {
        priv->stats.stat_len_err++;
        return -1;
    }

    if (unlikely(skb_linearize(skb))) {
        priv->stats.stat_liner_err++;
        return -1;
    }

    map_info = (struct remote_eth_cb_map_s *)skb->cb;
    map_info->userfield1 = 0;

    /* set id / prot info for modem */
    sk_pacing_shift_update(skb->sk, SK_PACING_SHIFT);

    return 0;
}

STATIC int weth_xmit_queue_skb(struct weth_ctx *priv, struct sk_buff *skb, int *start_timer)
{
    unsigned long flags;
    unsigned int tx_list_len;

    spin_lock_irqsave(&priv->tx_lock, flags);
    __skb_onelink_init(skb);
    if (priv->tx_tail_skb != NULL) {
        __skb_onelink_tail(priv->tx_tail_skb, skb);
        priv->stats.stat_tx_tail_enq++;
    } else {
        priv->stats.tx_timer_start++;
        *start_timer = 1;
    }
    priv->tx_list_len++;
    tx_list_len = priv->tx_list_len;
    /* move the tail_skb to new pos */
    priv->tx_tail_skb = skb;
    spin_unlock_irqrestore(&priv->tx_lock, flags);

    priv->stats.stat_tx_packets++;
    priv->stats.stat_tx_bytes += skb->len;

    return tx_list_len;
}

STATIC void weth_xmit_handle_timer(struct weth_ctx *priv, int start_timer,
    unsigned int tx_list_len, int icmp_flag)
{
    int force_schedule = 0;

    if (time_after_eq(jiffies, priv->tx_last_jiff + WETH_TX_CALC_TIME)) {
        start_timer = 0;
        force_schedule = 1;
        priv->stats.force_schedule++;
    }
    priv->tx_last_jiff = jiffies;

    if (start_timer) {
        hrtimer_start(&priv->tx_timer, ktime_set(0, priv->tx_timer_duration), HRTIMER_MODE_REL);
    }

    if (force_schedule || tx_list_len >= priv->max_list_len || icmp_flag == 1) {
        priv->stats.tx_timer_cancel++;
        hrtimer_cancel(&priv->tx_timer);
        tasklet_schedule(&priv->xmit_push);
    }
}

STATIC netdev_tx_t weth_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    struct weth_ctx *priv = NULL;
    unsigned int tx_list_len;
    int start_timer = 0;
    int icmp_flag;
    int ret;

    priv = weth_xmit_get_priv(ndev);
    priv->stats.stat_xmit_packets++;

    ret = weth_xmit_pre_check_skb(priv, skb);
    if (ret != 0) {
        goto out_free_drop;
    }

    icmp_flag = weth_set_userfield(skb, priv->devid);

    tx_list_len = weth_xmit_queue_skb(priv, skb, &start_timer);

    weth_xmit_handle_timer(priv, start_timer, tx_list_len, icmp_flag);

    return NETDEV_TX_OK;

out_free_drop:
    __weth_free_skb(skb);
    priv->stats.stat_tx_dropped++;

    return NETDEV_TX_OK;
}

STATIC enum hrtimer_restart weth_tx_timer_func(struct hrtimer *timer)
{
    struct weth_ctx *priv = container_of(timer, struct weth_ctx, tx_timer);

    priv->stats.tx_timer_running++;
    tasklet_schedule(&priv->xmit_push);
    return HRTIMER_NORESTART;
}

STATIC int weth_open(struct net_device *ndev)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(ndev);
    int i;

    for (i = 0; i < WETH_CHN_NUM; i++) {
        napi_enable(&priv->chn[i].napi);
    }
    netif_tx_start_all_queues(ndev);
    netif_carrier_on(ndev);
    priv->is_open = 1;

    return 0;
}

STATIC int weth_close(struct net_device *ndev)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(ndev);
    int i;

    for (i = 0; i < WETH_CHN_NUM; i++) {
        napi_disable(&priv->chn[i].napi);
    }
    netif_tx_stop_all_queues(ndev);
    netif_carrier_off(ndev);
    priv->is_open = 0;
    return 0;
}

STATIC int weth_change_mtu(struct net_device *dev, int new_mtu)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(dev);

    priv->stats.stat_set_mtu++;
    dev->mtu = new_mtu;
    return 0;
}

STATIC struct net_device_stats *weth_get_stats(struct net_device *dev)
{
    struct net_device_stats *stat = &dev->stats;
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(dev);

    stat->rx_packets = priv->stats.stat_rx_packets;
    stat->rx_bytes = priv->stats.stat_rx_bytes;
    stat->rx_errors = priv->stats.stat_rx_errors;

    stat->tx_packets = priv->stats.stat_tx_packets;
    stat->tx_bytes = priv->stats.stat_tx_bytes;
    stat->tx_errors = priv->stats.stat_tx_errors;
    stat->tx_dropped = priv->stats.stat_tx_dropped;

    return stat;
}

STATIC int weth_set_mac_address(struct net_device *dev, void *addr)
{
    struct sockaddr *saddr = addr;
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(dev);
    int ret;

    if (!is_valid_ether_addr(saddr->sa_data))
        return -EADDRNOTAVAIL;

    ret = memcpy_s(dev->dev_addr, ETH_ALEN, saddr->sa_data, ETH_ALEN);
    if (ret) {
        weth_pr_err("memcpy_s failed\n");
    }
    priv->stats.stat_set_mac++;
    return 0;
}

STATIC void weth_tx_timeout(struct net_device *dev, unsigned int txqueue)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(dev);

    priv->stats.stat_tx_timeout++;
    return;
}

STATIC const struct net_device_ops g_weth_ops = {
    .ndo_open = weth_open,
    .ndo_stop = weth_close,
    .ndo_start_xmit = weth_xmit,
    .ndo_change_mtu = weth_change_mtu,
    .ndo_get_stats = weth_get_stats,
    .ndo_set_mac_address = weth_set_mac_address,
    .ndo_tx_timeout = weth_tx_timeout,
};

STATIC unsigned int weth_get_dev_id(const struct device_node *np)
{
    int err;
    unsigned int dev_id = 0;

    err = of_property_read_u32_array(np, "dev_id", &dev_id, 1);
    if (err) {
        weth_pr_err("can't find [dev_id], use default phy-addr = 0\n");
    }
    return dev_id;
}

STATIC bool weth_napi_check_gro(struct sk_buff *skb)
{
    struct iphdr *iph = NULL;
    struct ipv6hdr *ipv6h = NULL;

    switch (skb->protocol) {
        case cpu_to_be16(ETH_P_IP):
            iph = (struct iphdr *)skb->data;
            if (iph->protocol == IPPROTO_TCP)
                return true;
            break;
        case cpu_to_be16(ETH_P_IPV6):
            ipv6h = (struct ipv6hdr *)skb->data;
            if (ipv6h->nexthdr == IPPROTO_TCP)
                return true;
            break;
        default:
            break;
    }

    return false;
}

STATIC int weth_netif_receive_skb(struct sk_buff *skb, struct weth_ctx *priv, struct weth_chn *pchn, int budget)
{
    struct sk_buff *next_skb = NULL;
    unsigned int num = 0;

    while (skb != NULL) {
        next_skb = skb->next;
        priv->stats.stat_rx_packets++;
        priv->stats.stat_rx_bytes += skb->len;
        g_weth_rc_ctx.rx_stats.total_rx_packets++;
        g_weth_rc_ctx.rx_stats.total_rx_bytes += skb->len;

        num++;
        if (g_by_pass_mode) {
            set_mac_header(priv, skb);
        }

        weth_pr_pkg(skb->data, MAX_PRINT_PKT_LEN);
        /* prepare skb */
        skb->protocol = eth_type_trans(skb, priv->ndev);
        g_weth_rc_ctx.cur_total_rx_pkg++;

        /* to kernel tcp/ip stack */
        skb->next = NULL;
        skb->prev = NULL;

        if (weth_napi_check_gro(skb)) {
            napi_gro_receive(&pchn->napi, skb);
        } else {
            netif_receive_skb(skb);
        }
        atomic_add(1, &g_weth_rc_ctx.weth_rc_debug.rx_netif);
        skb = next_skb;
    }

    if (num < budget) {
        atomic_add(1, &g_weth_rc_ctx.weth_rc_debug.rx_napi_complete);
        napi_complete(&pchn->napi);
    }

    return num;
}

STATIC int weth_poll(struct napi_struct *napi, int budget)
{
    struct weth_chn *pchn = container_of(napi, struct weth_chn, napi);
    struct weth_ctx *priv = pchn->pctx;
    unsigned long flags;
    int this_cpu;
    struct sk_buff *rx_skb = NULL;
    struct sk_buff *cur_skb = NULL;
    unsigned int cur_skb_num = 0;

    priv->stats.stat_poll++;

    this_cpu = get_cpu();
    put_cpu();

    spin_lock_irqsave(&pchn->rx_napi_lock, flags);
    if (unlikely(pchn->rx_napi_skb == NULL)) {
        priv->stats.stat_poll_none++;
        spin_unlock_irqrestore(&pchn->rx_napi_lock, flags);
        atomic_add(1, &g_weth_rc_ctx.weth_rc_debug.rx_napi_complete);
        napi_complete(&pchn->napi);
        return 0;
    }

    cur_skb = pchn->rx_napi_skb->next;
    while (cur_skb != pchn->rx_napi_skb) {
        cur_skb_num++;
        cur_skb = cur_skb->next;
        if (cur_skb_num >= budget - 1) {
            break;
        }
    }

    rx_skb = pchn->rx_napi_skb->next;
    pchn->rx_napi_skb->next = cur_skb->next;
    cur_skb->next = NULL;
    pchn->rx_qlen -= (cur_skb_num + 1);
    g_common_ctx.rx_packets_per_cpu[this_cpu] -= (cur_skb_num + 1);

    if (cur_skb == pchn->rx_napi_skb) {
        /* can be reused in weth_rx_packets_done */
        pchn->rx_napi_skb = NULL;
    }
    spin_unlock_irqrestore(&pchn->rx_napi_lock, flags);

    cur_skb_num = weth_netif_receive_skb(rx_skb, priv, pchn, budget);
    return cur_skb_num;
}

STATIC void weth_print_resume_packet(struct sk_buff *skb)
{
    int ret;

    ret = bsp_is_pcie_first_user(PCIE_EP_MSI_ETH_DEV);
    if (ret) {
        g_weth_rc_ctx.real_resume++;
        weth_print_pkt_info((unsigned char *)skb->data);
    }
    return;
}

void weth_set_napi_weight(unsigned int dev_id, int weight)
{
    struct weth_ctx *priv = NULL;
    int i;
    if (dev_id >= WETH_MUX_NUM) {
        return;
    }
    priv = g_weth_ctx[dev_id];

    /* init rx per cpu ctx */
    for (i = 0; i < WETH_CHN_NUM; i++) {
        priv->chn[i].napi.weight = weight;
    }
}

void weth_change_napi_weight(struct weth_ctx *priv, int rx_num)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;
    if (rx_num <= pctx->first_weight_thru) {
        pctx->init_change++;
        weth_set_napi_weight(priv->devid, pctx->init_weight);
    } else if (rx_num > pctx->first_weight_thru && rx_num <= pctx->second_weight_thru) {
        pctx->first_change++;
        weth_set_napi_weight(priv->devid, pctx->first_weight);
    } else if (rx_num > pctx->second_weight_thru && rx_num <= pctx->third_weight_thru) {
        pctx->second_change++;
        weth_set_napi_weight(priv->devid, pctx->second_weight);
    } else if (rx_num > pctx->third_weight_thru) {
        pctx->third_change++;
        weth_set_napi_weight(priv->devid, pctx->third_weight);
    }

    return;
}

STATIC int weth_get_avg_num(struct weth_chn *pchn, int rx_num)
{
    struct weth_rc_ctx *pctx  = &g_weth_rc_ctx;
    unsigned int avg_rx_num = -1;

    if (weth_jiffies_sub(jiffies, pchn->last_jiffies) > pctx->no_data_jiffies) {
        pchn->last_sum_num = rx_num;
        pchn->last_jiffies = jiffies;
        weth_pr_rx("no_data_jiffies\n");

        return 1;
    }

    pchn->last_sum_num += rx_num;
    if (weth_jiffies_sub(jiffies, pchn->last_jiffies) < pctx->evaluate_jiffies) {
        weth_pr_rx("in evaluate_jiffies\n");
        avg_rx_num = -1;
    } else {
        avg_rx_num = pchn->last_sum_num / pctx->evaluate_duration;
        pchn->last_jiffies = jiffies;
        pchn->last_sum_num = 0;
        weth_pr_rx("evaluate_jiffies, avg_rx_num: %d\n", avg_rx_num);
    }

    return avg_rx_num;
}

void weth_rx_done_no_gro(struct sk_buff *skb, struct weth_ctx *priv)
{
    struct sk_buff *rx_skb = NULL;
    struct sk_buff *next_skb = NULL;

    /* tail skb's next is the first skb in the list */
    rx_skb = skb->next;
    skb->next = NULL;

    weth_pr_pkg(skb->data, MAX_PRINT_PKT_LEN);

    while (rx_skb != NULL) {
        next_skb = rx_skb->next;
        priv->stats.stat_rx_packets++;
        priv->stats.stat_rx_bytes += rx_skb->len;
        g_weth_rc_ctx.rx_stats.total_rx_packets++;
        g_weth_rc_ctx.rx_stats.total_rx_bytes += rx_skb->len;
        atomic_add(1, &g_weth_rc_ctx.weth_rc_debug.rx_packet);

        if (g_by_pass_mode) {
            set_mac_header(priv, rx_skb);
        }
        weth_pr_pkg(rx_skb->data, MAX_PRINT_PKT_LEN);
        /* prepare skb */
        rx_skb->protocol = eth_type_trans(rx_skb, priv->ndev);

        /* to kernel tcp/ip stack */
        netif_rx(rx_skb);

        g_weth_rc_ctx.cur_total_rx_pkg++;

        rx_skb = next_skb;
    }
}

void weth_rx_done_gro(struct sk_buff *skb, struct weth_ctx *priv, unsigned int rx_num)
{
    int this_cpu;
    struct weth_chn *pchn = NULL;
    unsigned long flags;
    int avg_num;

    this_cpu = get_cpu();
    put_cpu();
    pchn = &priv->chn[this_cpu];

    if (pchn->rx_qlen > WETH_MAX_NAP_QLEN) {
        pchn->extra_qlen += rx_num;
        atomic_add(rx_num, &g_weth_rc_ctx.weth_rc_debug.rx_extra_num);
        __weth_free_skb_list(skb);
        return;
    }

    spin_lock_irqsave(&pchn->rx_napi_lock, flags);
    if (pchn->rx_napi_skb != NULL) {
        __skb_onelink_tail(pchn->rx_napi_skb, skb);
    }
    pchn->rx_napi_skb = skb;
    pchn->rx_qlen += rx_num;
    g_common_ctx.rx_packets_per_cpu[this_cpu] += rx_num;
    atomic_add(rx_num, &g_weth_rc_ctx.weth_rc_debug.rx_packet);
    spin_unlock_irqrestore(&pchn->rx_napi_lock, flags);

    if (unlikely(g_weth_rc_ctx.dynamic_napi_weight == 1)) {
        avg_num = weth_get_avg_num(pchn, rx_num);
        if (avg_num != -1) {
            weth_change_napi_weight(priv, avg_num);
        }
    }
    napi_schedule(&pchn->napi);
}

STATIC void weth_rx_done_cb(unsigned int dev_id, struct sk_buff *skb, unsigned int rx_num)
{
    struct weth_ctx *priv = NULL;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    if (pctx->first_resume == 1) {
        pctx->first_resume_called++;
        weth_print_resume_packet(skb->next);
        pctx->first_resume = 0;
    }

    if (dev_id >= WETH_MUX_NUM) {
        g_weth_rc_ctx.invalid_devid += rx_num;
        __weth_free_skb_list(skb);
        return;
    }

    priv = g_weth_ctx[dev_id];
    if (priv->is_open != 1) {
        pctx->not_opened += rx_num;
        __weth_free_skb_list(skb);
        return;
    }

    if (!priv->gro_enable) {
        weth_rx_done_no_gro(skb, priv);
    } else {
        weth_rx_done_gro(skb, priv, rx_num);
    }

    return;
}

void weth_dump_percpu_status(void)
{
    struct weth_chn *pchn = NULL;
    struct weth_ctx *priv = NULL;
    int i;

    priv = g_weth_ctx[0];
    for (i = 0; i < WETH_CHN_NUM; i++) {
        pchn = &priv->chn[i];
        weth_pr_err("rx_qlen of chn %d: %d\n", i, pchn->rx_qlen);
        weth_pr_err("extra_qlen of chn %d: %d\n", i, pchn->extra_qlen);
    }
}

static ssize_t weth_gro_state_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct device_node *np = dev->of_node;
    unsigned int dev_id;

    dev_id = weth_get_dev_id(np);
    weth_pr_err("dev_id: %d\n", dev_id);

    if (dev_id >= WETH_MUX_NUM) {
        weth_pr_err("invalid dev\n");
        return 0;
    }

    if (strncmp(buf, "1", GRO_STAT_LEN - 1) == 0) {
        g_weth_ctx[dev_id]->gro_enable = 1;
    } else if (strncmp(buf, "0", GRO_STAT_LEN - 1) == 0) {
        g_weth_ctx[dev_id]->gro_enable = 0;
    }

    return (ssize_t)count;
}

static ssize_t weth_gro_state_get(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t len = 0;
    unsigned int dev_id;
    struct device_node *np = dev->of_node;

    dev_id = weth_get_dev_id(np);
    if (dev_id >= WETH_MUX_NUM) {
        weth_pr_err("invalid dev\n");
        return len;
    }

    weth_pr_err("dev_id: %d\n", dev_id);

    if (g_weth_ctx[dev_id]->gro_enable == 1) {
        len = snprintf_s((char *)buf, GRO_STAT_LEN + 1, GRO_STAT_LEN, "%s\n", "1");
    } else if (g_weth_ctx[dev_id]->gro_enable == 0) {
        len = snprintf_s((char *)buf, GRO_STAT_LEN + 1, GRO_STAT_LEN, "%s\n", "0");
    }

    return len;
}

static DEVICE_ATTR(gro_state, 0660, weth_gro_state_get, weth_gro_state_set);

static int weth_gro_state_probe(struct platform_device *pdev)
{
    int ret;

    ret = device_create_file(&(pdev->dev), &dev_attr_gro_state);
    if (ret) {
        weth_pr_err("weth_gro_state_probe failed \n");
        return ret;
    }
    weth_pr_err("weth_gro_state_probe success\n");

    return ret;
}

STATIC int weth_netdev_attr_init(struct weth_ctx **ppriv, struct net_device *ndev, unsigned int dev_id, const char *name)
{
    int ret;
    struct weth_ctx *priv;

    ret = memcpy_s(ndev->name, IFNAMSIZ - 1, name, strlen(name));
    if (ret) {
        weth_pr_err("memcpy_s failed\n");
    }

    priv = (struct weth_ctx *)netdev_priv(ndev);
    if (priv == NULL) {
        weth_pr_err("netdev_priv fail!\n");
        return -ENOMEM;
    }

    priv->devid = dev_id;
    priv->ndev = ndev;
    priv->ndev->netdev_ops = &g_weth_ops;
    priv->ndev->watchdog_timeo = WETH_TX_TIMEOUT;
    priv->tx_last_jiff = jiffies;

    /* add bugget to sysfs */
    tasklet_init(&priv->xmit_push, weth_queue_packets, (uintptr_t)priv);
    hrtimer_init(&priv->tx_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    priv->tx_timer.function = weth_tx_timer_func;
    priv->tx_timer_duration = 1000000 * NANOSECOND; /* tx timer duration is 1000000 nanoseconds, 1ms */
    priv->max_list_len = WETH_MAX_TX_QUEUE_LEN;

    spin_lock_init(&priv->tx_lock);
    spin_lock_init(&priv->rx_lock);
    spin_lock_init(&priv->rx_smp_lock);

    ndev->features |= NETIF_F_SG;
    ndev->hw_features |= NETIF_F_SG;
    ndev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

    *ppriv = priv;

    return 0;
}

void weth_netdev_percpu_init(struct weth_ctx *priv)
{
    int i;
    /* init rx per cpu ctx */
    for (i = 0; i < WETH_CHN_NUM; i++) {
        netif_napi_add(priv->ndev, &priv->chn[i].napi, weth_poll, WETH_NAPI_WEIGHT);
        priv->chn[i].rx_napi_skb = NULL;
        spin_lock_init(&priv->chn[i].rx_napi_lock);
        priv->chn[i].pctx = priv;

        priv->chn[i].last_sum_num = 0;
        priv->chn[i].last_jiffies = jiffies;
    }
    priv->rx_num = WETH_MAX_RX_NODE;
    priv->rx_rpos = 0;
    priv->rx_wpos = 0;
    priv->gro_enable = 1;
}

STATIC int weth_netdev_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct net_device *ndev = NULL;
    struct weth_ctx *priv = NULL;
    unsigned int dev_id;
    char buf[WETH_NAME_LEN] = {0};
    const char *name = buf;
    int ret, retv;

    ndev = alloc_etherdev(sizeof(struct weth_ctx));
    if (ndev == NULL) {
        weth_pr_err("alloc_etherdev fail!\n");
        ret = -ENXIO;
        goto error;
    }

    dev_id = weth_get_dev_id(np);
    if (dev_id >= WETH_MUX_NUM) {
        weth_pr_err("Can't find [nic_name]!\n");
        return -ENXIO;
    }

    ret = of_property_read_string(np, "nic_name", &name);
    if (ret < 0) {
        weth_pr_err("Can't find [nic_name]!\n");
        return ret;
    }

    weth_netdev_attr_init(&priv, ndev, dev_id, name);

    ret = register_netdev(ndev);
    if (ret) {
        weth_pr_err("register_netdev failed\n");
        goto error;
    }

    retv = memcpy_s(priv->ndev->dev_addr, ETH_ALEN, g_rc_src_mac_addr, ETH_ALEN);
    if (retv) {
        weth_pr_err("memcpy_s failed\n");
    }
    g_weth_ctx[dev_id] = priv;

    weth_netdev_percpu_init(priv);
    weth_gro_state_probe(pdev);

    weth_pr_err("wan eth init sucsess\n");

    return 0;

error:
    free_netdev(ndev);
    return ret;
}

STATIC int weth_netdev_remove(struct platform_device *pdev)
{
    return 0;
}

#ifdef CONFIG_PM
STATIC int weth_suspend(struct device *dev)
{
    weth_pr_err("weth_suspend  suspend ok\n");
    return 0;
}

STATIC int weth_resume(struct device *dev)
{
    weth_pr_err("peth_resumet resume ok\n");
    return 0;
}

STATIC const struct dev_pm_ops weth_pm_ops = {
    .suspend = weth_suspend,
    .resume = weth_resume,
};

#define WETH_PM_OPS (&weth_pm_ops)
#else
#define WETH_PM_OPS NULL
#endif

STATIC const struct of_device_id weth_dt_ids[] = {
    { .compatible = "iot,rmnet0" },        { .compatible = "iot,rmnet1" },
    { .compatible = "iot,rmnet2" },        { .compatible = "iot,rmnet3" },
    { .compatible = "iot,rmnet4" },        { .compatible = "iot,rmnet5" },
    { .compatible = "iot,rmnet6" },        { .compatible = "iot,rmnet7" },
    { .compatible = "iot,rmnet8" },        { .compatible = "iot,rmnet9" },
    { .compatible = "iot,rmnet10" },        { .compatible = "iot,rmnet11" },
    { .compatible = "iot,rmnet_ims00" },
    { .compatible = "iot,rmnet_ims10" },   { .compatible = "iot,rmnet_emc0" },
    { .compatible = "iot,rmnet_emc1" },    { .compatible = "iot,rmnet_r_ims00" },
    { .compatible = "iot,rmnet_r_ims01" }, { .compatible = "iot,rmnet_r_ims10" },
    { .compatible = "iot,rmnet_r_ims11" }, { .compatible = "iot,rmnet_tun00" },
    { .compatible = "iot,rmnet_tun01" },   { .compatible = "iot,rmnet_tun02" },
    { .compatible = "iot,rmnet_tun03" },   { .compatible = "iot,rmnet_tun04" },
    { .compatible = "iot,rmnet_tun10" },   { .compatible = "iot,rmnet_tun11" },
    { .compatible = "iot,rmnet_tun12" },   { .compatible = "iot,rmnet_tun13" },
    { .compatible = "iot,rmnet_tun14" },   { /* sentinel */ }
};

struct platform_driver weth_driver = {
    .probe = weth_netdev_probe,
    .remove = weth_netdev_remove,
    .driver = {
        .name = "weth",
        .owner = THIS_MODULE,
        .pm = WETH_PM_OPS,
        .of_match_table = of_match_ptr(weth_dt_ids),
        },
};

STATIC void weth_rc_set_last_dma_desc(void *handle, char *usr_addr, char *desc, int cnt)
{
    dma_addr_t sar;
    dma_addr_t dar;

    /* last desc copy usrfield */
    sar = (dma_addr_t)virt_to_phys(usr_addr);

    weth_tab_get_rx_peer_usr_field(handle, (char **)&dar);
    bsp_pcie_set_last_data_element((void *)desc, cnt * sizeof(struct weth_tab_usr_field), sar, dar);

    desc += sizeof(struct pcie_dma_data_element);
    bsp_pcie_set_link_element(desc, 1, 0);

    return;
}

STATIC int weth_rc_set_dma_desc(struct weth_rc_ctx *pctx, struct sk_buff *skb, char *addr, char *usr_field)
{
    struct remote_eth_cb_map_s *cb_map = NULL;
    dma_addr_t sar;
    struct weth_tab_usr_field *usr_addr = (struct weth_tab_usr_field *)usr_field;

    sar = dma_map_single(pctx->pdev_for_map, skb->data, skb->len, DMA_TO_DEVICE);
    if (sar == 0) {
        pctx->tx_stats.phy_addr_null_drop++;
        weth_pr_err("sar is null\n");
        return -1;
    }
    /* config desc */
    bsp_pcie_set_sar_element(addr, skb->len, sar);

    /* copy usrfield */
    cb_map = (struct remote_eth_cb_map_s *)&skb->cb;
    usr_addr->usr_field0 = cb_map->userfield0;
    usr_addr->usr_field1 = cb_map->userfield1;
    usr_addr->usr_field2 = cb_map->userfield2;
    usr_addr->skb_len = skb->len;

    if (cb_map->userfield1 == 0) {
        atomic_add(1, &pctx->weth_rc_debug.tx_submit_ok);
    }

    return 0;
}

int weth_rc_set_desc(struct weth_rc_ctx *pctx, struct sk_buff *skb, int num)
{
    struct sk_buff *unsubmit_skb = NULL;
    char *desc = NULL;
    char *usr_field = NULL;
    char *usr_field_org = NULL;
    unsigned int tx_capacity;
    void *handle = NULL;
    unsigned int cnt;
    int ret;

    do {
        /* get desc to prepare cfg skb */
        ret = weth_tab_get_tx_desc(&handle, &desc, &usr_field, &tx_capacity);
        if (ret) {
            atomic_add(1, &g_weth_rc_ctx.weth_rc_debug.tx_submit_fail);
            pctx->tx_stats.tx_queue_full++;

            __weth_free_skb_list_by_null(skb);
            return -1;
        }

        usr_field_org = usr_field;
        /* config desc */
        unsubmit_skb = skb;
        for (cnt = 0; skb != NULL && cnt < tx_capacity - TABLE_REDUNDANCY_FOR_HEAD; skb = skb->next) {
            weth_tab_save_tx_skb(handle, cnt, skb);
            ret = weth_rc_set_dma_desc(pctx, skb, desc, usr_field);
            if (ret != 0) {
                continue;
            }
            desc += sizeof(struct pcie_dma_data_element);
            usr_field += sizeof(struct weth_tab_usr_field);
            cnt++;
            pctx->tx_stats.dma_send_packets++;
            pctx->tx_stats.dma_send_bytes += skb->len;
        }

        weth_rc_set_last_dma_desc(handle, usr_field_org, desc, cnt);
        pctx->tx_stats.dma_send_packets++;

        /* start tx dma */
        ret = weth_tab_start_tx_data_dma(handle, cnt);
        if (ret) {
            pctx->tx_stats.dma_transfer_fail += cnt;
            __weth_free_skb_list_by_null(unsubmit_skb);
            weth_tab_clear_local_skb(handle, cnt);
            return -1;
        }
    } while (cnt >= tx_capacity - TABLE_REDUNDANCY_FOR_HEAD);

    return 0;
}

int weth_rc_dma_send(struct weth_rc_ctx *pctx, struct sk_buff *skb, int num)
{
    struct sk_buff *cur_skb = NULL;
    struct sk_buff *last_skb = NULL;
    int ret;

    pctx->tx_stats.dma_send++;
    last_skb = skb;

    /* get first skb and set last's next to NULL */
    cur_skb = skb->next;
    last_skb->next = NULL;

    ret = weth_rc_set_desc(pctx, cur_skb, num);

    return ret;
}

/* event callback functions */
STATIC void weth_rc_config_dma(unsigned int idx, unsigned int cnt)
{
    return;
}

STATIC void weth_rc_sync_table(unsigned int idx, unsigned int cnt)
{
    weth_tab_get_addr_from_bar(&g_weth_rc_ctx.desc->rc.addr_table_rc);
}

STATIC void weth_rc_sync_rx_table_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_sync_rx_table_complete(idx);
}

STATIC void weth_rc_sync_tx_table_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_sync_tx_table_complete(idx);
}

STATIC void weth_rc_refill_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_put_tx_desc(idx, cnt);
}

STATIC void weth_rc_tx_dma_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_get_rx_desc_by_idx(idx, cnt);
}

STATIC void weth_rc_rx_dma_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_tx_complete(idx, cnt);
}

STATIC void weth_rc_linkup(unsigned int idx, unsigned int cnt)
{
    g_common_info.link_status = 1;
}

STATIC void weth_rc_linkdown(unsigned int idx, unsigned int cnt)
{
    g_common_info.link_status = 0;
}

void weth_rc_update_mac_address(struct weth_rc_ctx *pctx)
{
    int i, j;

    for (i = 0; i < WETH_STICK_NIC_NUM; i++) {
        struct weth_ctx *priv = g_weth_ctx[i];

        if (pctx->desc->rc.mac_addr[i].has_mac != 1) {
            continue;
        }

        for (j = 0; j < ETH_ALEN; j++) {
            priv->ndev->dev_addr[j] = pctx->desc->rc.mac_addr[i].mac_addr[j];
        }
    }
}

STATIC int weth_rc_get_bar_info(struct weth_rc_ctx *pctx)
{
    pctx->phys_addr = (void *)bsp_pcie_rc_get_bar_addr(PCIE_BAR_ETH_DEV);
    if (pctx->phys_addr == NULL) {
        weth_pr_err("cannot resource\n");
    }

    pctx->buffer_size = bsp_pcie_rc_get_bar_size(PCIE_BAR_ETH_DEV);
    if (pctx->buffer_size < MIN_IO_SIZE) {
        weth_pr_err("Invalid PCI region size, aborting\n");
        return -1;
    }

    pctx->virt_addr = ioremap((phys_addr_t)pctx->phys_addr, pctx->buffer_size);
    if (pctx->virt_addr == NULL) {
        weth_pr_err("cannot map\n");
        return -1;
    }

    /* pctx member init */
    pctx->rx_num = NUM_R2E_DESC;
    pctx->desc = pctx->virt_addr;
    pctx->vote_count = WETH_NO_PKG_COUNT;

    return 0;
}

STATIC void weth_table_sync_finish(void)
{
    g_common_info.table_sync_finish = 1;
}

STATIC void weth_init_pm_counts(struct weth_rc_ctx *pctx, struct common_info *comm_info)
{
    comm_info->init_finish = 0;
    comm_info->exit_finish = 0;

    pctx->vote_lock_opt = 0;
    pctx->last_total_tx_pkg = 0;
    pctx->cur_total_tx_pkg = 0;
    pctx->last_total_rx_pkg = 0;
    pctx->cur_total_rx_pkg = 0;
    pctx->no_pkg_count = 0;
    pctx->is_wakeup = 0;
}

void weth_set_first_resume_flag(void)
{
    weth_pr_dbg("weth_set_first_resume_flag enter\n");
    g_weth_rc_ctx.first_resume = 1;

    return;
}

static struct syscore_ops weth_rc_dpm_ops = {
    .resume = weth_set_first_resume_flag,
};

STATIC int weth_rc_init_event_buff(struct weth_rc_ctx *pctx)
{
    weth_event_cb_t event[WETH_EVENT_TYPE_NUM] = {0};
    int ret;

    ret = weth_event_buff_init(WETH_PCIE_EP, &pctx->desc->rc.tx_event, &pctx->desc->rc.rx_event);
    if (ret) {
        pctx->init_stats.event_init_fail++;
        weth_pr_err("weth_event_buff_init failed\n");
        return -1;
    }

    /* fill event handler */
    event[WETH_LINK_UP] = weth_rc_linkup;
    event[WETH_LINK_DOWN] = weth_rc_linkdown;
    event[WETH_CONFIG_DMA] = weth_rc_config_dma;
    event[WETH_TX_DMA_COMPLETE] = weth_rc_tx_dma_complete;
    event[WETH_RX_DMA_COMPLETE] = weth_rc_rx_dma_complete;
    event[WETH_REFILL_COMPLETE] = weth_rc_refill_complete;
    event[WETH_SYNC_TABLE] = weth_rc_sync_table;
    event[WETH_SYNC_TX_TABLE_COMPLETE] = weth_rc_sync_tx_table_complete;
    event[WETH_SYNC_RX_TABLE_COMPLETE] = weth_rc_sync_rx_table_complete;

    weth_event_buff_set_cb(event);

    return 0;
}

STATIC int weth_rc_init_table(struct weth_rc_ctx *pctx, struct common_info *comm_info)
{
    int ret;

    weth_tab_regist_rx_cb(weth_rx_done_cb);
    weth_tab_regist_table_sync_cb(weth_table_sync_finish);

    /* table init */
    ret = weth_tab_init(pctx->pdev_for_map);
    if (ret) {
        pctx->init_stats.tab_init_fail++;
        weth_pr_err("weth_tab_init failed\n");
        return -1;
    }

    weth_tab_set_addr_to_bar((void *)&pctx->desc->rc.addr_table_rc);

    weth_tab_wait_setting_done((void *)&pctx->desc->rc.addr_table_ep);

    weth_tab_get_addr_from_bar((void *)&pctx->desc->rc.addr_table_ep);

    weth_rc_update_mac_address(pctx);

    while (!comm_info->table_sync_finish && comm_info->wait_for_linkup < MAX_TRY_TIMES_FOR_LINKUP) {
        comm_info->wait_for_linkup++;
        msleep(100); /* sleep 100 miliseconds to wait ep ready */
    }
    comm_info->wait_for_linkup = 0;

    return 0;
}

STATIC void weth_rc_init_set_finish(struct weth_rc_ctx *pctx, struct common_info *comm_info,
    struct weth_common_ctx *comm_ctx)
{
    comm_info->link_status = 1;

    pctx->first_resume = 0;

    comm_ctx->speed_threshold = WETH_RC_SPEED_THRESHOLD;
    comm_ctx->speed_up_threshold = WETH_RC_SPEED_UP_THRESHOLD;
    comm_ctx->speed_down_threshold = WETH_RC_SPEED_DOWN_THRESHOLD;
    comm_ctx->cur_speed_level = WETH_RC_LOW_SPEED;
    comm_ctx->low_speed_count = 0;

    comm_info->init_finish = 1;
}

STATIC int weht_rc_core_init(struct weth_rc_ctx *pctx, struct common_info *comm_info)
{
    int ret;

    dma_set_mask_and_coherent(&pctx->pdev->dev, WETH_DMA_MASK);

    ret = weth_rc_get_bar_info(pctx);
    if (ret) {
        weth_pr_err("weth_rc_get_bar_info failed\n");
        return -1;
    }

    spin_lock_init(&pctx->tx_lock);
    spin_lock_init(&pctx->rx_lock);

    ret = weth_rc_init_event_buff(pctx);
    if (ret != 0) {
        return -1;
    }
    /* event buffer init */
    ret = weth_rc_init_table(pctx, comm_info);
    if (ret != 0) {
        return -1;
    }

    ret = weth_send_event(0, 0, WETH_LINK_UP, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__);
    }

    return 0;
}

STATIC int weth_rc_init(void)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;
    struct common_info *comm_info = &g_common_info;
    struct weth_common_ctx *comm_ctx = &g_common_ctx;
    int ret;

    /* wait probe init end */
    weth_init_pm_counts(pctx, comm_info);

    while (pctx->pdev_for_map == NULL) {
        weth_pr_err("wait platform device\n");
        msleep(100); /* sleep 100 miliseconds to wait pdev init succ */
    }

    ret = bsp_pcie_rc_vote_lock(PCIE_USER_ETH_DEV, 1);
    if (unlikely(ret != 0)) {
        weth_pr_err("vote_lock failed\n");
        pctx->stat_wait_vote_fail++;
        goto err_out2;
    }

    ret = weht_rc_core_init(pctx, comm_info);
    if (ret != 0) {
        goto err_out1;
    }

    if (bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV)) {
        weth_pr_err("vote_unlock failed\n");
        pctx->stat_unvote_fail++;
    }

    register_syscore_ops(&weth_rc_dpm_ops);

    weth_rc_init_set_finish(pctx, comm_info, comm_ctx);
    weth_pr_err("weth_rc_init ok\n");

    return 0;

err_out1:
    if (bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV)) {
        weth_pr_err("vote_unlock failed\n");
        pctx->stat_unvote_fail++;
    }
err_out2:
    platform_driver_unregister(pctx->rc_platform_driver);

    return ret;
}

STATIC int wan_eth_core_reinit(struct weth_rc_ctx *pctx, struct common_info *comm_info)
{
    int ret;

    ret = weth_rc_get_bar_info(pctx);
    if (ret) {
        weth_pr_err("weth_rc_get_bar_info failed\n");
        return -1;
    }

    /* event buffer init */
    weth_event_buff_reinit(&pctx->desc->rc.tx_event, &pctx->desc->rc.rx_event);
    if (ret) {
        pctx->init_stats.event_init_fail++;
        weth_pr_err("weth_event_buff_init failed\n");
        return -1;
    }

    weth_tab_reinit();

    weth_tab_set_addr_to_bar(&pctx->desc->rc.addr_table_rc);

    weth_tab_wait_setting_done(&pctx->desc->rc.addr_table_ep);

    weth_tab_get_addr_from_bar(&pctx->desc->rc.addr_table_ep);

    while (!comm_info->table_sync_finish && comm_info->wait_for_linkup < MAX_TRY_TIMES_FOR_LINKUP) {
        comm_info->wait_for_linkup++;
        msleep(100); /* sleep 100 miliseconds to wait linkup */
    }

    comm_info->wait_for_linkup = 0;

    ret = weth_send_event(0, 0, WETH_LINK_UP, 1);
    if (ret) {
        weth_pr_err("weth_send_event fail:%d, line:%d\n", ret, __LINE__);
        return -1;
    }

    return 0;
}

STATIC int weth_rc_reinit(struct weth_rc_ctx *pctx)
{
    int ret;
    struct common_info *comm_info = &g_common_info;
    struct weth_common_ctx *comm_ctx = &g_common_ctx;

    comm_info->init_finish = 0;
    comm_info->exit_finish = 0;
    weth_init_pm_counts(pctx, comm_info);

    ret = bsp_pcie_rc_vote_lock(PCIE_USER_ETH_DEV, 1);
    if (unlikely(ret != 0)) {
        weth_pr_err("vote_lock failed\n");
        pctx->stat_wait_vote_fail++;
        return -1;
    }

    ret = wan_eth_core_reinit(pctx, comm_info);
    if (ret != 0) {
        goto err_out1;
    }

    if (bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV)) {
        pctx->stat_unvote_fail++;
        weth_pr_err("vote_unlock fail\n");
    }

    comm_info->link_status = 1;
    comm_ctx->cur_speed_level = WETH_RC_LOW_SPEED;
    comm_ctx->low_speed_count = 0;
    add_timer(&g_weth_rc_ctx.vote_timer);

    comm_info->init_finish = 1;
    weth_pr_err("weth_rc_init ok\n");

    return 0;

err_out1:
    if (bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV)) {
        pctx->stat_unvote_fail++;
        weth_pr_err("vote_lock failed\n");
    }

    return ret;
}

static ssize_t weth_vote_count_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int vote_count;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    weth_pr_err("%s enter\n", __func__);
    vote_count = atoi((char *)buf);
    if (vote_count < WETH_MIN_VOTE_THRU || vote_count > WETH_MAX_VOTE_THRU) {
        weth_pr_err("invalid vote count, valid value range: %d~%d \n", WETH_MIN_VOTE_THRU, WETH_MAX_VOTE_THRU);
        return -1;
    }

    pctx->vote_count = vote_count;
    weth_rc_pkt_transreport(&pctx->vote_count, sizeof(pctx->vote_count), WETH_EP_VOTE_COUNT);
    return (ssize_t)count;
}

static ssize_t weth_vote_count_get(struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t len;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    len = snprintf_s((char *)buf, VOTE_LOCK_LEN + OVER_SIZE_SRC, VOTE_LOCK_LEN + OVER_SIZE_DTS,
        "%d\n", pctx->vote_count);

    weth_pr_err("buf after copy: %s\n", buf);

    return len;
}

static DEVICE_ATTR(vote_count, 0660, weth_vote_count_get, weth_vote_count_set);

static int weth_vote_count_state(struct platform_device *pdev)
{
    int ret;

    ret = device_create_file(&(pdev->dev), &dev_attr_vote_count);
    if (ret) {
        weth_pr_err("weth_vote_count_state failed \n");
        return ret;
    }
    weth_pr_err("weth_vote_count_statesuccess\n");

    return ret;
}

static ssize_t weth_set_hook(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int type;

    weth_pr_err("weth_set_hook\n");
    type = atoi((char *)buf);
    if (type < 0) {
        return -1;
    }
    weth_pr_err("type:%u\n", type);

    return (ssize_t)count;
}

static ssize_t weth_show_hook(struct device *dev, struct device_attribute *attr, char *buf)
{
    return 0;
}

static DEVICE_ATTR(hook, 0660, weth_show_hook, weth_set_hook);

static int weth_hook_state(struct platform_device *pdev)
{
    int ret;

    ret = device_create_file(&(pdev->dev), &dev_attr_hook);
    if (ret) {
        weth_pr_err("weth_hook_state failed \n");
        return ret;
    }
    weth_pr_err("weth_hook_state succ\n");

    return ret;
}

STATIC int rc_pltfm_probe(struct platform_device *pdev)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    if (pdev == NULL) {
        weth_pr_err("rc_pltfm_rc_probe: pdev is null!\n");
        return -1;
    }
    pctx->pdev = pdev;
    pctx->pdev_for_map = &pdev->dev;
    weth_vote_count_state(pdev);
    weth_hook_state(pdev);
    pctx->dynamic_napi_weight = 0;
    weth_set_weight_thru(FIRST_WEIGHT_THRU, SECOND_WEIGHT_THRU, THIRD_WEITHG_THRU);
    weth_set_weight(INIT_WEIGHT, FIRST_WEIGHT, SECOND_WEIGHT, THIRD_WEIGHT);
    weth_set_duration(NO_DATA_DURATION, EVALUATION_DURATION);

    weth_pr_err("rc_pltfm_rc_probe\n");
    return 0;
}

STATIC int rc_pltfm_remove(struct platform_device *pdev)
{
    weth_pr_err("rc_pltfm_remove\n");
    return 0;
}

STATIC void rc_pltfm_shutdown(struct platform_device *pdev)
{
    g_common_info.link_status = 0;
    weth_pr_err("rc_pltfm_shutdown ok\n");
    return;
}

#ifdef CONFIG_PM
STATIC int rc_pltfm_suspend(struct device *dev)
{
    weth_pr_err("rc_pltfm  suspend ok\n");
    return 0;
}

STATIC int rc_pltfm_resume(struct device *dev)
{
    weth_pr_err("rc_pltfm resume ok\n");
    return 0;
}

STATIC const struct dev_pm_ops rc_pltfm_pm_ops = {
    .suspend = rc_pltfm_suspend,
    .resume = rc_pltfm_resume,
};

#define PETH_RC_PM_OPS (&rc_pltfm_pm_ops)
#else
#define PETH_RC_PM_OPS NULL
#endif

STATIC const struct of_device_id rc_match[] = {
    { .compatible = "iot,weth_rc" },
    {},
};

STATIC struct platform_driver rc_pltfm_driver = {
    .probe	= rc_pltfm_probe,
    .remove = rc_pltfm_remove,
    .shutdown = rc_pltfm_shutdown,
    .driver	= {
        .name = "weth_rc",
        .of_match_table = rc_match,
        .pm = PETH_RC_PM_OPS,
    },
};

STATIC int rc_platform_driver_register(void)
{
    int ret;

    g_weth_rc_ctx.rc_platform_driver = &rc_pltfm_driver;
    ret = platform_driver_register(&rc_pltfm_driver);
    if (ret) {
        weth_pr_err("register peth_rc_device failed\n");
    }

    return ret;
}

STATIC struct task_struct *g_peth_tsk;

STATIC int weth_rc_thread(void *data)
{
    int ret;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    while (1) {
        while (down_interruptible(&g_weth_rc_ctx.weth_rc_init_sem)) ;
        if (pctx->is_first_init) {
            ret = weth_rc_init();
        } else {
            ret = weth_rc_reinit(pctx);
        }

        if (ret) {
            weth_pr_err("weth_rc_reinit failed\n");
            return -1;
        }
    }
    return 0;
}

STATIC int weth_rc_init_handle(void)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    up(&pctx->weth_rc_init_sem);

    return 0;
}

int weth_exit(void)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;
    struct common_info *comm_info = &g_common_info;

    weth_pr_err("weth_exit start\n");

    while (!comm_info->init_finish) {
        udelay(1);
        comm_info->wait_init_finish++;
    }
    comm_info->wait_init_finish = 0;

    if (comm_info->exit_finish) {
        weth_pr_err("exit twice \n");
        return 0;
    }

    pctx->is_first_init = 0;
    comm_info->exit_finish = 1;
    comm_info->link_status = 0;
    comm_info->wait_count = 0;
    comm_info->table_sync_finish = 0;
    del_timer(&pctx->vote_timer);

    while (atomic_read(&comm_info->bar_access) && comm_info->wait_count < WAIT_COUNT_THRU) {
        udelay(1);
        comm_info->wait_count++;
    }

    if (comm_info->wait_count >= WAIT_COUNT_THRU) {
        weth_pr_err("bar_access time out\n");
    }
    comm_info->wait_count = 0;

    while (weth_tab_get_status() && comm_info->wait_table_finish < WAIT_COUNT_THRU) {
        udelay(1);
        comm_info->wait_table_finish++;
    }

    if (comm_info->wait_table_finish >= WAIT_COUNT_THRU) {
        weth_pr_err("wait_table_finish time out\n");
    }
    comm_info->wait_table_finish = 0;

    weth_event_buff_exit();

    iounmap(pctx->virt_addr);

    weth_pr_err("weth_exit\n");
    return 0;
}

void weth_linkdown(void)
{
    weth_pr_err("weth_linkdown\n");
    return;
}

STATIC int weth_rc_init_start(u32 usr_id, u32 callback_stage, void *callback_args)
{
    switch (callback_stage) {
        case PCIE_RC_CB_ENUM_DONE:
            return weth_rc_init_handle();
            break;
        case PCIE_RC_CB_EXIT:
            break;
        case PCIE_RC_CB_LINKDOWN:
            weth_linkdown();
            break;
        default:
            break;
    }

    return 0;
}

STATIC int weth_rc_callback_register(struct weth_rc_ctx *pctx)
{
    int ret;
    struct pcie_callback_info pcie_nic_callback_info = {0};

    sema_init(&pctx->weth_rc_init_sem, 0);
    g_peth_tsk = kthread_run(weth_rc_thread, NULL, "weth_rc_thread");
    weth_pr_err("weth_rc_thread init sucsess\n");

    ret = rc_platform_driver_register();
    if (ret) {
        weth_pr_err("rc_platform_driver_register fail\n");
        return ret;
    }

    pcie_nic_callback_info.callback = (pcie_callback)weth_rc_init_start;
    pcie_nic_callback_info.callback_args = NULL;

    ret = bsp_pcie_rc_cb_register(PCIE_USER_ETH_DEV, &pcie_nic_callback_info);
    if (ret) {
        weth_pr_err("bsp_pcie_rc_callback_register fail\n");
        return ret;
    }

    pctx->init = 1;
    return ret;
}

void weth_print_pkt_info(unsigned char *data)
{
    struct iphdr *iph = (struct iphdr *)data;
    struct udphdr *udph = NULL;
    struct tcphdr *tcph = NULL;
    struct icmphdr *icmph = NULL;
    struct ipv6hdr *ip6h = NULL;

    pr_err("[C SR][WETH]pcie wakeup, ip version=%d\n", iph->version);

    if (iph->version == 4) { /* ip version is 4 */
        /* use bit shift and 0xff to get lower 8 bits of saddr */
        pr_err("[C SR][WETH]src ip:%d.%d.%d.x, dst ip:%d.%d.%d.x\n", iph->saddr & 0xff, (iph->saddr >> 8) & 0xff,
                /* use bit shift and 0xff to get lower 8 bits or 16 bit of daddr */
               (iph->saddr >> 16) & 0xff, iph->daddr & 0xff, (iph->daddr >> 8) & 0xff, (iph->daddr >> 16) & 0xff);
        if (iph->protocol == IPPROTO_UDP) {
            udph = (struct udphdr *)(data + sizeof(struct iphdr));
            pr_err("[C SR][WETH]UDP packet, src port:%d, dst port:%d.\n", ntohs(udph->source), ntohs(udph->dest));
        } else if (iph->protocol == IPPROTO_TCP) {
            tcph = (struct tcphdr *)(data + sizeof(struct iphdr));
            pr_err("[C SR][WETH]TCP packet, src port:%d, dst port:%d\n", ntohs(tcph->source), ntohs(tcph->dest));
        } else if (iph->protocol == IPPROTO_ICMP) {
            icmph = (struct icmphdr *)(data + sizeof(struct iphdr));
            pr_err("[C SR][WETH]ICMP packet, type(%d):%s, code:%d.\n", icmph->type,
                   ((icmph->type == ICMP_ECHOREPLY) ? "ping reply" :
                   ((icmph->type == ICMP_ECHO) ? "ping request" : "other icmp pkt")),
                   icmph->code);
        } else if (iph->protocol == IPPROTO_IGMP) {
            pr_err("[C SR][WETH]ICMP packet\n");
        } else {
            pr_err("[C SR][WETH]Other IPV4 packet\n");
        }
    } else if (iph->version == 6) { /* ip version is 6 */
        ip6h = (struct ipv6hdr *)data;
        pr_err("[C SR][WETH]version: %d, payload length: %d, nh->nexthdr: %d. \n", ip6h->version,
               ntohs(ip6h->payload_len), ip6h->nexthdr);
        /* get byte 7 of u6_addr16 */
        pr_err("[C SR][WETH]ipv6 src addr:%04x:%x:%xx:x:x:x:x:x  \n", ntohs(ip6h->saddr.in6_u.u6_addr16[7]),
               /* get byte 6 and 11 of u6_addr8 */
               ntohs(ip6h->saddr.in6_u.u6_addr16[6]), (ip6h->saddr.in6_u.u6_addr8[11]));
         /* get byte 7 of u6_addr16 */
        pr_err("[C SR][WETH]ipv6 dst addr:%04x:%x:%xx:x:x:x:x:x \n", ntohs(ip6h->saddr.in6_u.u6_addr16[7]),
               /* get byte 6 and 11 of u6_addr8 */
               ntohs(ip6h->saddr.in6_u.u6_addr16[6]), (ip6h->saddr.in6_u.u6_addr8[11]));
    } else {
        weth_pr_pkg_pm(data, MAX_PRINT_PM_PKT_LEN);
    }
}

STATIC struct sk_buff *weth_rc_get_skb_to_send(struct weth_rc_ctx *pctx, unsigned int *tx_list_len)
{
    struct sk_buff *skb = NULL;

    if (unlikely(pctx->tx_tail_skb_all == NULL)) {
        return NULL;
    }
    /* tail skb's next is the first skb in the list */
    skb = pctx->tx_tail_skb_all;
    *tx_list_len = pctx->tx_list_len_all;
    /* can be reused in xmit */
    pctx->tx_list_len_all = 0;
    pctx->tx_tail_skb_all = NULL;

    return skb;
}

STATIC void weth_rc_send_skb(struct weth_rc_ctx *pctx, struct sk_buff *queue_skb,
    unsigned int tx_list_len, int direct_free)
{
    unsigned long flags;

    if (unlikely(queue_skb == NULL)) {
        pctx->stat_work_no_skb++;
        goto lock_out;
    }

    if (unlikely(direct_free)) {
        pctx->stat_vote_fail_free++;
        __weth_free_skb_list(queue_skb);
        goto lock_out;
    }

    pctx->cur_total_tx_pkg += tx_list_len;
    weth_rc_dma_send(pctx, queue_skb, tx_list_len);

lock_out:
    spin_lock_irqsave(&pctx->tx_vote_lock, flags);
    pctx->vote_lock_opt--;
    spin_unlock_irqrestore(&pctx->tx_vote_lock, flags);
}

STATIC void weth_xmit_work(struct work_struct *work)
{
    int ret;
    int direct_free = 0;
    struct sk_buff *queue_skb = NULL;
    unsigned int tx_list_len = 0;
    unsigned long flags;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    spin_lock_irqsave(&pctx->tx_vote_lock, flags);
    pctx->vote_lock_opt++;
    if (unlikely(!pctx->is_wakeup)) {
        pctx->stat_wait_vote_call++;
        spin_unlock_irqrestore(&pctx->tx_vote_lock, flags);
        pctx->deep_sleep_vote++;
        weth_pr_err("bsp_pcie_rc_vote_lock 1\n");
        if (pctx->tx_tail_skb_all != NULL) {
            struct sk_buff *first_skb = pctx->tx_tail_skb_all->next;
            weth_print_pkt_info((unsigned char *)first_skb->data + ETH_HLEN);
        }
        ret = bsp_pcie_rc_vote_lock(PCIE_USER_ETH_DEV, 1);
        pctx->deep_sleep_vote--;
        weth_pr_err("bsp_pcie_rc_vote_lock 1 finish\n");

        spin_lock_irqsave(&pctx->tx_vote_lock, flags);
        if (unlikely(ret != 0)) {
            pctx->stat_wait_vote_fail++;
            direct_free = 1;
        } else {
            pctx->is_wakeup = 1;
        }
        pctx->cur_total_tx_pkg += tx_list_len;
    }

    queue_skb = weth_rc_get_skb_to_send(pctx, &tx_list_len);
    spin_unlock_irqrestore(&pctx->tx_vote_lock, flags);

    weth_rc_send_skb(pctx, queue_skb, tx_list_len, direct_free);

    return;
}

STATIC int weth_rc_submit_precheck(struct weth_rc_ctx *pctx, struct common_info *comm_info,
    struct sk_buff *skb, unsigned int num)
{
    struct sk_buff *cur_skb = NULL;

    if (unlikely(!comm_info->link_status)) {
        weth_pr_dbg("link is not up\n");
        pctx->tx_stats.rc_link_not_up++;
        __weth_free_skb_list(skb);
        return -1;
    }

    if (unlikely(pctx->tx_list_len_all > WETH_VOTE_PROTECT_NUM)) {
        pctx->stat_work_protect++;
        atomic_add(num, &pctx->weth_rc_debug.tx_vote_protect);
        cur_skb = skb->next;
        skb->next = NULL;
        __weth_free_skb_list_by_null(cur_skb);
        return -1;
    }

    return 0;
}

STATIC int weth_rc_submit_queue_skb(struct weth_rc_ctx *pctx, struct sk_buff *skb, int is_need_queue, int num)
{
    if (unlikely(is_need_queue != 0)) {
        pctx->stat_nowait_vote_fail++;
        if (pctx->tx_tail_skb_all != NULL) {
            __skb_onelink_tail(pctx->tx_tail_skb_all, skb);
        }

        pctx->tx_tail_skb_all = skb;
        pctx->tx_list_len_all += num;
        pctx->vote_lock_opt--;
        /* tx_list_len_all protect skb num */
        return 1;
    }
    pctx->is_wakeup = 1;
    pctx->cur_total_tx_pkg += num;

    return 0;
}

STATIC int weth_rc_submit_tx(struct sk_buff *skb, unsigned int num)
{
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;
    struct common_info *comm_info = &g_common_info;
    unsigned long flags;
    int ret, start_work;
    int vote;

    pctx->tx_stats.rc_submit_tx++;

    ret = weth_rc_submit_precheck(pctx, comm_info, skb, num);
    if (ret != 0) {
        return -1;
    }

    /* don't use lock in bulk packets */
    vote = 1;

    spin_lock_irqsave(&pctx->tx_vote_lock, flags);
    /* may modify by other cpu/task */
    if (unlikely(pctx->is_wakeup)) {
        vote = 0;
        ret = 0;
    }
    /* may vote lock in workqueue */
    if (unlikely(pctx->vote_lock_opt > 0)) {
        vote = 0;
        ret = -1;
    }

    pctx->vote_lock_opt++;
    if (vote) {
        pctx->stat_nowait_vote_call++;
        weth_pr_dbg("bsp_pcie_rc_vote_lock 0\n");
        ret = bsp_pcie_rc_vote_lock(PCIE_USER_ETH_DEV, 0);
    }

    start_work = weth_rc_submit_queue_skb(pctx, skb, ret, num);
    spin_unlock_irqrestore(&pctx->tx_vote_lock, flags);

    if (start_work == 1) {
        queue_delayed_work(g_weth_rc_ctx.xmit_queue, &pctx->xmit_work, 0);
        return 0;
    }

    ret = weth_rc_dma_send(pctx, skb, num);

    spin_lock_irqsave(&pctx->tx_vote_lock, flags);
    pctx->vote_lock_opt--;
    spin_unlock_irqrestore(&pctx->tx_vote_lock, flags);

    return ret;
}

void weth_bps_record(struct weth_rc_ctx *pctx)
{
    struct weth_common_ctx *comm_ctx = &g_common_ctx;

    static unsigned long long last_rx_bytes = 0;
    static unsigned long long last_tx_bytes = 0;
    unsigned int dl_speed;
    int speed_level;

    comm_ctx->ul_speed = (pctx->tx_stats.dma_send_bytes - last_rx_bytes) * BYTE_TO_BITS * VOTE_PER_SECOND;
    comm_ctx->dl_speed = (pctx->rx_stats.total_rx_bytes - last_tx_bytes) * BYTE_TO_BITS * VOTE_PER_SECOND;

    last_rx_bytes = pctx->tx_stats.dma_send_bytes;
    last_tx_bytes = pctx->rx_stats.total_rx_bytes;

    dl_speed = comm_ctx->dl_speed >> BITS_TO_MEGABITS;

    if (dl_speed > comm_ctx->speed_up_threshold) {
        speed_level = WETH_RC_HIGH_SPEED;
        comm_ctx->low_speed_count = 0;
    } else if (dl_speed < comm_ctx->speed_down_threshold) {
        speed_level = WETH_RC_LOW_SPEED;
    } else {
        comm_ctx->low_speed_count = 0;
        return;
    }

    if (speed_level != comm_ctx->cur_speed_level) {
        if (dl_speed > comm_ctx->speed_up_threshold) {
            comm_ctx->cur_speed_level = speed_level;
        } else if (dl_speed < comm_ctx->speed_down_threshold) {
            if (comm_ctx->low_speed_count >= WETH_LOW_SPEED_COUNT) {
                comm_ctx->cur_speed_level = speed_level;
                comm_ctx->low_speed_count = 0;
            } else {
                comm_ctx->low_speed_count++;
            }
        }
    }
}

void weth_rc_pkt_transreport(void *data, unsigned int size, unsigned int flag)
{
    struct sk_buff *skb = NULL;
    struct remote_eth_cb_map_s *map_info = NULL;
    unsigned int data_len = size;
    int ret;

    if (unlikely(!g_common_info.link_status)) {
        return;
    }
    if (unlikely(data_len < ETH_HLEN)) {
        data_len += ETH_HLEN;
    } else if (data_len > BUF_SIZE_1_6KB) {
        return;
    }
    skb = __netdev_alloc_skb(NULL, data_len, GFP_ATOMIC);
    if (skb != NULL) {
        ret = memcpy_s(skb->data, data_len, data, size);
        if (ret != 0) {
            weth_pr_err("memcpy_s ret:%d\n", ret);
        }
        map_info = (struct remote_eth_cb_map_s *)skb->cb;
        map_info->userfield1 = flag;
        __skb_onelink_init(skb);
        skb_put(skb, data_len);
        (void)weth_xmit(skb, NULL);
        g_weth_rc_ctx.tx_stats.debug_pkt_xmit_succ++;
    }
    return;
}

void bsp_weth_rc_pkt_transreport(void *data, unsigned int size)
{
    if (data == NULL || size <= 0) {
        return;
    }
    g_weth_rc_ctx.tx_stats.debug_pkt_xmit++;
    weth_rc_pkt_transreport(data, size, WETH_RC_DEBUG_PKT);
    return;
}

void weth_rc_open_pkt_trans(enum net_om_report_type en_type, unsigned int enable)
{
    struct weth_hids_trans_open_msg data;
    data.en_type = en_type;
    data.enable = enable;

    if (en_type == 0xff) {
        g_weth_rc_ctx.trans_report_enable = enable;
    } else {
        weth_rc_pkt_transreport(&data, sizeof(data), WETH_RC_HIDS_PKT_OPEN);
    }
    return;
}

void weth_rc_dump_debug(void)
{
    weth_pr_err("debug_pkt_xmit:       %u\n", g_weth_rc_ctx.tx_stats.debug_pkt_xmit);
    weth_pr_err("debug_pkt_xmit_succ:  %u\n", g_weth_rc_ctx.tx_stats.debug_pkt_xmit_succ);

    return;
}

void weth_vote_timer_handle(struct timer_list *t)
{
    struct weth_rc_ctx *ctx = container_of(t, struct weth_rc_ctx, vote_timer);
    struct common_info *comm_info = &g_common_info;
    unsigned int cur_tx_pkg = ctx->cur_total_tx_pkg;
    unsigned int cur_rx_pkg = ctx->cur_total_rx_pkg;
    unsigned long flags;
    unsigned int is_sleep = 0;

    weth_bps_record(ctx);

    spin_lock_irqsave(&ctx->tx_vote_lock, flags);
    if (!ctx->is_wakeup || ctx->vote_lock_opt > 0) {
        spin_unlock_irqrestore(&ctx->tx_vote_lock, flags);

        if (comm_info->exit_finish != 1) {
            mod_timer(&ctx->vote_timer, WETH_SET_VOTE_TIMEOUT());
        }
        return;
    }

    /* check pkg count */
    if (ctx->last_total_tx_pkg == cur_tx_pkg && ctx->last_total_rx_pkg == cur_rx_pkg && !weth_tab_get_status() &&
        !weth_event_get_status()) {
        ctx->no_pkg_count++;
    } else {
        ctx->no_pkg_count = 0;
    }

    /* upate last count */
    ctx->last_total_tx_pkg = cur_tx_pkg;
    ctx->last_total_rx_pkg = cur_rx_pkg;

    if (ctx->no_pkg_count >= ctx->vote_count) {
        ctx->vote_lock_opt++;
        ctx->stat_unvote_call++;
        ctx->no_pkg_count = 0;
        weth_pr_dbg("bsp_pcie_rc_vote_unlock\n");
        if (bsp_pcie_rc_vote_unlock(PCIE_USER_ETH_DEV)) {
            ctx->stat_unvote_fail++;
        } else {
            ctx->is_wakeup = 0;
            is_sleep = 1;
        }
        ctx->vote_lock_opt--;
    }
    spin_unlock_irqrestore(&ctx->tx_vote_lock, flags);

    if (is_sleep == 0 && ctx->trans_report_enable == 1) {
        weth_rc_pkt_transreport((void *)&ctx->weth_rc_debug, sizeof(ctx->weth_rc_debug), WETH_RC_DEBUG_FLAG);
    }

    if (comm_info->exit_finish != 1) {
        mod_timer(&ctx->vote_timer, WETH_SET_VOTE_TIMEOUT());
    }
    return;
}

void weth_init_timer(struct weth_rc_ctx *pctx)
{
    /* init timer */
    timer_setup(&pctx->vote_timer, weth_vote_timer_handle, 0);
    pctx->vote_timer.expires = WETH_SET_VOTE_TIMEOUT();
    add_timer(&pctx->vote_timer);
}

int weth_init(void)
{
    int ret;
    struct weth_rc_ctx *pctx = &g_weth_rc_ctx;

    weth_pr_err("weth_init start!\n");
    ret = platform_driver_register(&weth_driver);
    if (ret) {
        weth_pr_err("register peth_device failed\n");
    }

    pctx->is_first_init = 1;
    ret = weth_rc_callback_register(pctx);
    if (ret) {
        platform_driver_unregister(&weth_driver);
    }

    pctx->xmit_queue = create_singlethread_workqueue("weth_tx_wk");
    if (pctx->xmit_queue == NULL) {
        ret = -1;
        goto fail;
    }
    INIT_DELAYED_WORK(&pctx->xmit_work, weth_xmit_work);
    spin_lock_init(&pctx->tx_vote_lock);
    weth_init_timer(pctx);

    weth_pr_err("[weth]weth_init ok\n");
    return ret;

fail:
    weth_pr_err("[weth]weth_init fail\n");
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(weth_init);
#endif

MODULE_DESCRIPTION("Pcie Driver");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
