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
#include <linux/jiffies.h>
#include <linux/socket.h>
#include "securec.h"

#include <linux/io.h>
#include <uapi/linux/if_ether.h>
#include <linux/of_device.h>

#include <nva_id.h>
#include <bsp_nva.h>

#include "bsp_pcie.h"
#include "bsp_net_om.h"
#include "wan_eth_table.h"
#include "wan_eth_ep.h"

#include "mdrv_pfa.h"
#include "wan_eth_event_buffer.h"
#include "wan_eth_common.h"
#include "wan_eth_dma.h"
#include "wan_eth_maa.h"
#include "wan_eth_spe.h"
#include "wan_eth_stick.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */

#define WETH_NAME_LEN 16
#define WETH_TX_TIMEOUT (8 * HZ)
#define WETH_MAX_RX_PKT 2048
#define HIGH_SPEED 3000
#define LOW_SPEED 0
#define SPEED_THRESHOLD 500
#define SPEED_UP_THRESHOLD 550
#define SPEED_DOWN_THRESHOLD 450
#define DOUBLE_FOR_ALIGN 2
#define WETH_PCIE_ID_MAX_NUM 2

#define WETH_SET_VOTE_TIMEOUT() (jiffies + msecs_to_jiffies(WETH_VOTE_TIMEOUT))

unsigned int g_weth_debug_level = 0x1;
int g_by_pass_mode __read_mostly = 1;

int g_vote_count = WETH_NO_PKG_COUNT;

struct weth_ep_ctx g_weth_ep_ctx;

int g_is_first_init = 1;

/* for debug */
struct weth_ctx *g_weth_ctx[WETH_MUX_NUM];

static const uint8_t g_rc_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x04, 0x01
};
static const uint8_t g_ep_src_mac_addr[ETH_ALEN] = {
    0x00, 0x11, 0x09, 0x64, 0x00, 0x01
};

STATIC void weth_xmit_work(struct work_struct *work);
struct weth_ep_ctx *weth_get_ep_ctx(void)
{
    return &g_weth_ep_ctx;
}

struct weth_ctx *weth_get_ctx(unsigned int devid)
{
    if (devid >= WETH_MUX_NUM) {
        return NULL;
    }
    return g_weth_ctx[devid];
}

unsigned int weth_set_tx_timer_duration(unsigned int dev_id, unsigned int timer_duration)
{
    if (dev_id >= WETH_MUX_NUM) {
        return 0;
    }

    g_weth_ctx[dev_id]->tx_timer_duration = timer_duration;
    return g_weth_ctx[dev_id]->tx_timer_duration;
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

int set_by_pass_mode(int value)
{
    g_by_pass_mode = value;
    return g_by_pass_mode;
}

int weth_set_vote_count(int value)
{
    g_vote_count = value;
    return g_vote_count;
}

int weth_get_vote_count(void)
{
    return g_vote_count;
}

int g_send_cnt = 1;
int weth_set_send_cnt(int cnt)
{
    g_send_cnt = cnt;
    return g_send_cnt;
}

STATIC void weth_set_userfield(struct sk_buff *skb, unsigned int devid)
{
    struct rx_cb_map_s *map_info = (struct rx_cb_map_s *)skb->cb;

    map_info->userfield0 = ((struct ethhdr *)skb->data)->h_proto;
    map_info->userfield0 = (map_info->userfield0 << 16) | devid; /* get high 16 bits */
    map_info->userfield1 = 0;

    return;
}

STATIC void weth_stick_tx(struct sk_buff *skb)
{
    struct sk_buff *cur_skb = NULL;
    struct sk_buff *next_skb = NULL;

    if (skb == NULL) {
        weth_pr_err("skb is null !\n");
        g_weth_ep_ctx.rx_stats.remote_eth_tx_skb_null++;
        return;
    }

    cur_skb = skb->next;
    skb->next = NULL;
    while (cur_skb != NULL) {
        next_skb = cur_skb->next;
        g_weth_ep_ctx.rx_stats.remote_eth_tx_total++;
        g_weth_ep_ctx.rx_stats.remote_eth_tx_total_bytes += cur_skb->len;
        weth_stick_wan_tx(cur_skb);
        cur_skb = next_skb;
    }
}

STATIC void weth_rx_done_cb(unsigned int dev_id, struct sk_buff *skb, unsigned int rx_num)
{
    struct weth_ctx *priv = NULL;
    struct sk_buff *rx_skb = NULL;
    struct sk_buff *next_skb = NULL;
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    if (g_by_pass_mode) {
        weth_stick_tx(skb);
        return;
    }

    if (dev_id >= WETH_MUX_NUM) {
        return;
    }
    priv = g_weth_ctx[dev_id];

    /* tail skb's next is the first skb in the list */
    rx_skb = skb->next;
    skb->next = NULL;

    while (rx_skb != NULL) {
        next_skb = rx_skb->next;
        priv->stats.stat_rx_packets++;
        priv->stats.stat_rx_bytes += rx_skb->len;

        dma_unmap_single(pctx->pdev_for_map, virt_to_phys(rx_skb->data), rx_skb->len, DMA_FROM_DEVICE);

        weth_pr_pkg(rx_skb->data, MAX_PRINT_PKT_LEN);

        bsp_net_report_mark_pkt_info((unsigned long long)rx_skb->data + ETH_HLEN, NET_OM_TYPE_WETH_RX, 1, 0);
        /* prepare skb */
        rx_skb->protocol = eth_type_trans(rx_skb, priv->ndev);

        rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
        /* to kernel tcp/ip stack */
        netif_rx(rx_skb);

        g_weth_ep_ctx.cur_total_rx_pkg++;

        rx_skb = next_skb;
    }

    return;
}

STATIC int weth_deliver_skb(struct weth_ctx *priv, struct weth_ep_ctx *pctx, struct sk_buff *skb)
{
    struct sk_buff *next_skb = NULL;
    int deliver_num = 0;

    while (skb != NULL) {
        next_skb = skb->next;
        priv->stats.stat_rx_packets++;
        priv->stats.stat_rx_bytes += skb->len;

        dma_unmap_single(pctx->pdev_for_map, virt_to_phys(skb->data), skb->len, DMA_FROM_DEVICE);
        deliver_num++;

        weth_pr_pkg(skb->data, MAX_PRINT_PKT_LEN);
        /* prepare skb */
        skb->protocol = eth_type_trans(skb, priv->ndev);

        skb->ip_summed = CHECKSUM_UNNECESSARY;
        /* to kernel tcp/ip stack */
        napi_gro_receive(&priv->rx_napi, skb);
        skb = next_skb;
    }

    return deliver_num;
}

STATIC int weth_poll(struct napi_struct *napi, int budget)
{
    struct weth_ctx *priv = container_of(napi, struct weth_ctx, rx_napi);
    unsigned long flags;
    struct sk_buff *rx_skb = NULL;
    struct sk_buff *cur_skb = NULL;
    unsigned int cur_skb_num = 0;
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    priv->stats.stat_poll++;

    spin_lock_irqsave(&priv->rx_lock, flags);
    if (unlikely(priv->rx_tail_skb == NULL)) {
        priv->stats.stat_poll_none++;
        spin_unlock_irqrestore(&priv->rx_lock, flags);
        napi_complete(&priv->rx_napi);
        return 0;
    }
    /* tail skb's next is the first skb in the list */
    cur_skb = priv->rx_tail_skb->next;

    while (cur_skb != priv->rx_tail_skb) {
        cur_skb_num++;
        cur_skb = cur_skb->next;
        if (cur_skb_num >= budget - 1) {
            break;
        }
    }

    rx_skb = priv->rx_tail_skb->next;
    priv->rx_tail_skb->next = cur_skb->next;
    cur_skb->next = NULL;

    if (cur_skb == priv->rx_tail_skb) {
        /* can be reused in weth_rx_packets_done */
        priv->rx_tail_skb = NULL;
    }
    spin_unlock_irqrestore(&priv->rx_lock, flags);

    cur_skb_num = weth_deliver_skb(priv, pctx, rx_skb);

    if (cur_skb_num < budget) {
        napi_complete(&priv->rx_napi);
    }
    return cur_skb_num;
}

int weth_set_debug_port(int port_no)
{
    g_weth_ep_ctx.debug_port = port_no;
    return g_weth_ep_ctx.debug_port;
}

STATIC void weth_queue_packets(unsigned long data)
{
    struct weth_ctx *priv = (struct weth_ctx *)data;
    int ret;
    struct sk_buff *queue_skb = NULL;
    unsigned int tx_list_len;
    unsigned long flags;

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

    ret = weth_ep_submit_tx(queue_skb, tx_list_len, g_weth_ep_ctx.debug_port);
    if (unlikely(ret)) {
        priv->stats.stat_tx_submit_fail++;
    }

    return;
}


STATIC int weth_skb_pre_handle(struct sk_buff *skb, struct weth_ctx *priv)
{
    priv->stats.stat_xmit_packets++;

    if (unlikely(skb->len < ETH_HLEN)) {
        priv->stats.stat_len_err++;
        return -1;
    }

    if (unlikely(skb_linearize(skb))) {
        priv->stats.stat_liner_err++;
        return -1;
    }

    weth_set_userfield(skb, priv->devid);

    return 0;
}

STATIC void weth_timer_handle(struct weth_ctx *priv, int start_timer, int tx_list_len)
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

    if (force_schedule || tx_list_len >= priv->max_list_len) {
        priv->stats.tx_timer_cancel++;
        hrtimer_cancel(&priv->tx_timer);
        tasklet_schedule(&priv->xmit_push);
    }
}

STATIC netdev_tx_t weth_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(ndev);
    unsigned long flags;
    unsigned int tx_list_len = 0;
    int start_timer = 0;
    int ret;

    ret = weth_skb_pre_handle(skb, priv);
    if (ret != 0) {
        goto out_free_drop;
    }

    spin_lock_irqsave(&priv->tx_lock, flags);
    __skb_onelink_init(skb);
    if (priv->tx_tail_skb != NULL) {
        __skb_onelink_tail(priv->tx_tail_skb, skb);
        priv->stats.stat_tx_tail_enq++;
    } else {
        priv->stats.tx_timer_start++;
        start_timer = 1;
    }
    priv->tx_list_len++;
    tx_list_len = priv->tx_list_len;
    /* move the tail_skb to new pos */
    priv->tx_tail_skb = skb;
    spin_unlock_irqrestore(&priv->tx_lock, flags);

    priv->stats.stat_tx_packets++;
    priv->stats.stat_tx_bytes += skb->len;

    weth_timer_handle(priv, start_timer, tx_list_len);
    return NETDEV_TX_OK;

out_free_drop:
    __weth_ep_free_skb(skb);
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

    netif_tx_start_all_queues(ndev);
    netif_carrier_on(ndev);
    priv->is_open = 1;
    return 0;
}

STATIC int weth_close(struct net_device *ndev)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(ndev);

    netif_tx_stop_all_queues(ndev);
    netif_carrier_off(ndev);
    priv->is_open = 0;
    return 0;
}

STATIC int weth_change_mtu(struct net_device *dev, int new_mtu)
{
    struct weth_ctx *priv = (struct weth_ctx *)netdev_priv(dev);

    dev->mtu = new_mtu;
    priv->stats.stat_set_mtu++;
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

STATIC void weth_tx_timeout(struct net_device *dev, unsigned int txqueu)
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

STATIC int weth_netdev_attr_init(struct weth_ctx **ppriv, struct net_device *ndev, unsigned int dev_id, const char *name)
{
    int ret;
    struct weth_ctx *priv = NULL;

    ret = memcpy_s(ndev->name, IFNAMSIZ - 1, name, strlen(name));
    if (ret) {
        weth_pr_err("memcpy_s failed\n");
        return -1;
    }

    priv = (struct weth_ctx *)netdev_priv(ndev);
    if (priv == NULL) {
        weth_pr_err("netdev_priv fail!\n");
        return -1;
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
    priv->rx_num = WETH_MAX_RX_PKT;
    priv->rx_rpos = 0;
    priv->rx_wpos = 0;
    ndev->features |= NETIF_F_SG;
    ndev->hw_features |= NETIF_F_SG;

    *ppriv = priv;
    return 0;
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
        return -ENOMEM;
    }

    dev_id = weth_get_dev_id(np);
    if (dev_id >= WETH_MUX_NUM) {
        weth_pr_err("Can't find [nic_name]!\n");
        return -ENXIO;
    }

    ret = of_property_read_string(np, "nic_name", &name);
    if (ret < 0) {
        weth_pr_err("Can't find [nic_name]!\n");
        goto error;
    }

    ret = weth_netdev_attr_init(&priv, ndev, dev_id, name);
    if (ret != 0) {
        ret = -ENOMEM;
        goto error;
    }

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
    return 0;
}

STATIC int weth_resume(struct device *dev)
{
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
    { .compatible = "iot,wan_eth0" },
    { .compatible = "iot,wan_eth1" },
    { /* sentinel */ }
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

void weth_bps_record(struct weth_ep_ctx *pctx)
{
    static unsigned long long last_rx_bytes = 0;
    static unsigned long long last_tx_bytes = 0;

    pctx->dl_speed = (pctx->tx_stats.dma_send_bytes - last_rx_bytes) * BYTE_TO_BITS * VOTE_PER_SECOND;
    pctx->ul_speed = (pctx->rx_stats.remote_eth_tx_total_bytes - last_tx_bytes) * BYTE_TO_BITS * VOTE_PER_SECOND;

    last_rx_bytes = pctx->tx_stats.dma_send_bytes;
    last_tx_bytes = pctx->rx_stats.remote_eth_tx_total_bytes;
}

void weth_speed_change(struct work_struct *work)
{
    unsigned int speed;
    int speed_level;
    int ret = -1;
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    speed = (pctx->dl_speed + pctx->ul_speed) >> BITS_TO_MEGABITS;

    if (speed > SPEED_THRESHOLD) {
        speed_level = HIGH_SPEED_LEVEL;
    } else {
        speed_level = LOW_SPEED_LEVEL;
    }

    if (speed_level != pctx->cur_speed_level) {
        if (speed > SPEED_UP_THRESHOLD) {
            ret = bsp_pcie_data_volume_set(PCIE_USER_ETH_DEV, HIGH_SPEED);
        } else if (speed < SPEED_DOWN_THRESHOLD) {
            ret = bsp_pcie_data_volume_set(PCIE_USER_ETH_DEV, LOW_SPEED);
        }

        if (ret == 0) {
            pctx->cur_speed_level = speed_level;
            weth_pr_dbg("speed level changed, current level: %d\n", pctx->cur_speed_level);
        }
    }

    return;
}

void weth_vote_timer_handle(struct timer_list *t)
{
    struct weth_ep_ctx *ctx = container_of(t, struct weth_ep_ctx, vote_timer);
    unsigned int cur_tx_pkg = ctx->cur_total_tx_pkg;
    unsigned int cur_rx_pkg = ctx->cur_total_rx_pkg;
    unsigned long flags;

    if (ctx == NULL) {
        weth_pr_err("weth_vote_timer_handle ctx is NULL\n");
        return;
    }

    if (ctx->speed_change_queue == NULL) {
        weth_pr_err("weth_vote_timer_handle speed change queue is NULL\n");
        return;
    }

    weth_bps_record(ctx);
    queue_delayed_work(ctx->speed_change_queue, &ctx->speed_change_work, 0);

    spin_lock_irqsave(&ctx->tx_lock_all, flags);
    if (!ctx->is_wakeup || ctx->vote_lock_opt > 0) {
        spin_unlock_irqrestore(&ctx->tx_lock_all, flags);
        ctx->mod_timer_vote_lock++;
        mod_timer(&ctx->vote_timer, WETH_SET_VOTE_TIMEOUT());
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

    if (ctx->no_pkg_count >= g_vote_count) {
        ctx->stat_unvote_call++;
        ctx->vote_lock_opt++;
        weth_pr_dbg("bsp_pcie_ep_vote_unlock\n");
        if (bsp_pcie_ep_vote_unlock(PCIE_USER_ETH_DEV)) {
            ctx->stat_unvote_fail++;
        } else {
            ctx->is_wakeup = 0;
        }
        ctx->vote_lock_opt--;
    }
    spin_unlock_irqrestore(&ctx->tx_lock_all, flags);

    mod_timer(&ctx->vote_timer, WETH_SET_VOTE_TIMEOUT());
    return;
}

void weth_init_timer(void)
{
    /* init speed change workqueue */
    INIT_DELAYED_WORK(&g_weth_ep_ctx.speed_change_work, weth_speed_change);
    g_weth_ep_ctx.speed_change_queue = create_singlethread_workqueue("weth_speed_change");
    if (g_weth_ep_ctx.speed_change_queue == NULL) {
        weth_pr_err("init speed_change_queue work failed\n");
        return;
    }

    /* init timer */
    timer_setup(&g_weth_ep_ctx.vote_timer, weth_vote_timer_handle, 0);
    g_weth_ep_ctx.vote_timer.expires = WETH_SET_VOTE_TIMEOUT();
    add_timer(&g_weth_ep_ctx.vote_timer);

    return;
}

int weth_init(void)
{
    int ret;

    weth_pr_err("weth_init start!\n");
    ret = platform_driver_register(&weth_driver);
    if (ret) {
        weth_pr_err("register peth_device failed\n");
        return -1;
    }

    g_weth_ep_ctx.xmit_queue = create_singlethread_workqueue("weth_tx_wk");
    if (g_weth_ep_ctx.xmit_queue == NULL) {
        weth_pr_err("init xmit_queue work failed\n");
        return -1;
    }

    INIT_DELAYED_WORK(&g_weth_ep_ctx.xmit_work, weth_xmit_work);
    spin_lock_init(&g_weth_ep_ctx.tx_lock_all);
    weth_init_timer();
    weth_pr_err("weth_init success!\n");
    return ret;
}

module_init(weth_init);

STATIC void weth_ep_set_last_dma_desc(void *handle, char *usr_addr, char *desc, int cnt)
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

STATIC int weth_ep_set_dma_desc(struct weth_ep_ctx *pctx, struct sk_buff *skb, char *addr, char *usr_field)
{
    struct rx_cb_map_s *cb_map = NULL;
    dma_addr_t sar;
    struct weth_tab_usr_field *usr_addr = (struct weth_tab_usr_field *)usr_field;

    cb_map = (struct rx_cb_map_s *)&skb->cb;
    if (g_by_pass_mode) {
        if (cb_map->userfield2 == WETH_ETH_MODE) {
            sar = bsp_dra_to_phy(cb_map->dra_l2);
        } else {
            sar = bsp_dra_to_phy(cb_map->dra_l2 + ETH_HLEN);
        }
        bsp_net_report_mark_pkt_info((unsigned long long)(uintptr_t)skb->data, NET_OM_TYPE_WETH_TX, 0, 1);
    } else {
        bsp_net_report_mark_pkt_info((unsigned long long)(uintptr_t)skb->data + ETH_HLEN, NET_OM_TYPE_WETH_TX, 1, 0);
        sar = dma_map_single(pctx->pdev_for_map, skb->data, skb->len, DMA_TO_DEVICE);
    }

    if (sar == 0) {
        pctx->tx_stats.phy_addr_null_drop++;
        weth_pr_err("sar is null\n");
        return -1;
    }
    /* config desc */
    bsp_pcie_set_sar_element(addr, skb->len, sar);

    /* copy usrfield */
    usr_addr->usr_field0 = cb_map->userfield0;
    usr_addr->usr_field1 = cb_map->userfield1;
    usr_addr->usr_field2 = cb_map->userfield2;
    usr_addr->skb_len = skb->len;

    return 0;
}

int weth_ep_set_desc(struct weth_ep_ctx *pctx, struct sk_buff *skb, int num, int port_no)
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
            pctx->tx_stats.tx_queue_full++;
            pctx->tx_stats.dmasend_drop += num;
            __weth_ep_free_skb_list_by_null(skb);
            return -1;
        }

        usr_field_org = usr_field;
        /* config desc */
        unsubmit_skb = skb;
        for (cnt = 0; skb != NULL && cnt < tx_capacity - TABLE_REDUNDANCY_FOR_HEAD; skb = skb->next) {
            weth_tab_save_tx_skb(handle, cnt, skb);
            ret = weth_ep_set_dma_desc(pctx, skb, desc, usr_field);
            if (ret != 0) {
                num--;
                continue;
            }
            desc += sizeof(struct pcie_dma_data_element);
            usr_field += sizeof(struct weth_tab_usr_field);
            cnt++;
            num--;
            pctx->tx_stats.dma_send_packets++;
            pctx->tx_stats.dma_send_bytes += skb->len;
            g_packet_dump.sent_packets++;
        }

        weth_ep_set_last_dma_desc(handle, usr_field_org, desc, cnt);
        /* start tx dma */
        ret = weth_tab_ep_start_tx_dma(handle, cnt, port_no);
        if (ret) {
            pctx->tx_stats.dma_transfer_fail += cnt;
            __weth_ep_free_skb_list_by_null(unsubmit_skb);
            return -1;
        }
    } while (skb != NULL);

    return 0;
}

int weth_ep_dma_send_cnt(struct weth_ep_ctx *pctx, struct sk_buff *skb, int num, int cnt, int port_no)
{
    struct sk_buff *cur_skb = NULL;
    struct sk_buff *last_skb = NULL;
    struct sk_buff *first_skb = NULL;
    struct sk_buff *org_skb = NULL;
    int i;

    pctx->tx_stats.dma_send++;

    last_skb = skb;
    first_skb = skb->next;
    for (i = 0; i < cnt - 1; i++) {
        org_skb = first_skb;
        while (org_skb != last_skb) {
            cur_skb = skb_clone(org_skb, GFP_ATOMIC);
            if (cur_skb != NULL) {
                skb_onelink_tail(&skb, cur_skb);
                num++;
            }
            org_skb = org_skb->next;
        }
    }

    pctx->tx_stats.dma_send++;
    cur_skb = skb->next;
    skb->next = NULL;

    if (cur_skb != NULL) {
        weth_pr_pkg(cur_skb->data, MAX_PRINT_PKT_LEN);
    }

    return weth_ep_set_desc(pctx, cur_skb, num, port_no);
}

/* event callback functions */
STATIC void weth_ep_config_dma(unsigned int idx, unsigned int cnt)
{
    weth_tab_start_rx_table_dma(idx, cnt);
}

STATIC void weth_ep_sync_table(unsigned int idx, unsigned int cnt)
{
    weth_tab_get_addr_from_bar(&g_weth_ep_ctx.desc->ep.addr_table_ep);
}

STATIC void weth_ep_sync_rx_table_complete(unsigned int idx, unsigned int cnt)
{
    return;
}

STATIC void weth_ep_sync_tx_table_complete(unsigned int idx, unsigned int cnt)
{
    return;
}

STATIC void weth_ep_refill_complete(unsigned int idx, unsigned int cnt)
{
    weth_tab_tx_refill_event(idx, cnt);
}

STATIC void weth_ep_tx_complete(unsigned int idx, unsigned int cnt)
{
    return;
}

STATIC void weth_ep_rx_complete(unsigned int idx, unsigned int cnt)
{
    return;
}

STATIC void weth_ep_linkup(unsigned int idx, unsigned int cnt)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    pctx->link = 1;
    if (pctx->state_enable_cb != NULL) {
        pctx->state_enable_cb();
    }
}

STATIC void weth_ep_linkdown(unsigned int idx, unsigned int cnt)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    pctx->link = 0;
    if (pctx->state_disable_cb != NULL) {
        pctx->state_disable_cb();
    }
}

void weth_ep_event_cb_init(void)
{
    weth_event_cb_t event[WETH_EVENT_TYPE_NUM] = {0};

        /* fill event handler */
    event[WETH_LINK_UP] = weth_ep_linkup;
    event[WETH_LINK_DOWN] = weth_ep_linkdown;
    event[WETH_CONFIG_DMA] = weth_ep_config_dma;
    event[WETH_TX_DMA_COMPLETE] = weth_ep_tx_complete;
    event[WETH_RX_DMA_COMPLETE] = weth_ep_rx_complete;
    event[WETH_REFILL_COMPLETE] = weth_ep_refill_complete;
    event[WETH_SYNC_TABLE] = weth_ep_sync_table;
    event[WETH_SYNC_TX_TABLE_COMPLETE] = weth_ep_sync_tx_table_complete;
    event[WETH_SYNC_RX_TABLE_COMPLETE] = weth_ep_sync_rx_table_complete;
    weth_event_buff_set_cb(event);
}

void weth_ep_nv_init(struct weth_ep_ctx *pctx)
{
    if (bsp_nv_read(NVA_ID_DRV_PCIE_NIC_NET_FEATURE, (u8 *)&(pctx->net_feature), sizeof(pctx->net_feature))) {
        weth_pr_err("nv net feature init fail, not support 5g lan mac\n");
        pctx->net_feature.ethernet_feature = 0;
        pctx->stat_nv_net_feature++;
    }
}

void weth_ep_set_mac_addr_to_bar(struct weth_ep_ctx *pctx)
{
    int ret;
    int i;

    for (i = 0; i < WETH_STICK_NIC_NUM; i++) {
        struct ppp_ndis_mac_index index_info = { 0 };
        unsigned char mac_addr[ETH_ALEN];

        index_info.field_info.chan_type = PPP_NDIS_DEV_PNET;
        index_info.field_info.chan_id = i;
        index_info.field_info.data_len = sizeof(HOST_ADDR_TYPE);
        index_info.addr_type = HOST_ADDR_TYPE;

        if (pctx->get_mac_cb == NULL) {
            weth_pr_err("get_mac_cb not register, use default mac\n");
            continue;
        }
        ret = pctx->get_mac_cb(&index_info, mac_addr);
        if (ret != 0) {
            weth_pr_err("get mac addr for nic :%d get not succ\n", i);
            continue;
        }

        ret = memcpy_s(pctx->desc->ep.mac_addr[i].mac_addr, ETH_ALEN, mac_addr, ETH_ALEN);
        if (ret != 0) {
            weth_pr_err("copy mac addr for nic :%d get not succ\n", i);
            continue;
        }
        pctx->desc->ep.mac_addr[i].has_mac = 1;
    }
}

STATIC int weth_ep_hal_init(struct weth_ep_ctx *pctx)
{
    int ret;
    /* event buffer init */
    ret = weth_event_buff_init(WETH_PCIE_EP, &pctx->desc->ep.tx_event, &pctx->desc->ep.rx_event);
    if (ret) {
        pctx->init_stats.event_init_fail++;
        weth_pr_err("weth_event_buff_init failed\n");
        return -1;
    }

    weth_tab_regist_rx_cb(weth_rx_done_cb);
    weth_spe_register_tx_cb(weth_ep_submit_tx);
    weth_stick_register_tx_cb(weth_ep_submit_tx);

    /* dma init */
    ret = weth_dma_init(pctx->pcie_id);
    if (ret) {
        weth_pr_err("weth_dma_init fail:%d, line:%d\n", ret, __LINE__);
        return -1;
    }

    weth_ep_event_cb_init();
    /* table init */
    ret = weth_tab_init(pctx->pdev_for_map);
    if (ret) {
        pctx->init_stats.tab_init_fail++;
        weth_pr_err("weth_tab_init failed\n");
        return -1;
    }

    weth_ep_nv_init(pctx);
    weth_ep_set_mac_addr_to_bar(pctx);
    weth_tab_set_addr_to_bar((void *)&pctx->desc->ep.addr_table_ep);
    weth_tab_wait_setting_done((void *)&pctx->desc->ep.addr_table_rc);
    weth_tab_get_addr_from_bar((void *)&pctx->desc->ep.addr_table_rc);

    return 0;
}

/* initialization function */
STATIC int weth_ep_init(void)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    int ret;

    pctx->rx_num = NUM_R2E_DESC;

    spin_lock_init(&pctx->tx_lock);
    spin_lock_init(&pctx->rx_lock);

    ret = bsp_pcie_ep_vote_lock(PCIE_USER_ETH_DEV, 1);
    if (unlikely(ret != 0)) {
        pctx->stat_wait_vote_fail++;
        weth_pr_err("vote_lock failed\n");
        goto err_out1;
    }

    ret = weth_ep_hal_init(pctx);
    if (ret != 0) {
        goto err_out1;
    }

    if (bsp_pcie_ep_vote_unlock(PCIE_USER_ETH_DEV)) {
        weth_pr_err("vote_unlock failed\n");
        pctx->stat_unvote_fail++;
    }

    pctx->cur_speed_level = LOW_SPEED_LEVEL;
    weth_pr_err("ep init succ\n");
    return 0;

err_out1:
    if (bsp_pcie_ep_vote_unlock(PCIE_USER_ETH_DEV)) {
        weth_pr_err("vote_unlock failed\n");
        pctx->stat_unvote_fail++;
    }

    return ret;
}

STATIC unsigned int ep_weth_get_pcie_id(const struct device_node *np)
{
    int err;
    unsigned int pcie_id = 0;

    err = of_property_read_u32_array(np, "pcie_id", &pcie_id, 1);
    if (err) {
        weth_pr_err("can't find [pcie_id], use default pcie_id 0\n");
    }
    return pcie_id;
}


STATIC int ep_pltfm_probe(struct platform_device *pdev)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    unsigned int pcie_id = 0;

    if (pdev == NULL) {
        weth_pr_err("ep_pltfm_rc_probe: pdev is null!\n");
        return -1;
    }
    pctx->pdev = pdev;
    pctx->debug_port = WETH_DEBUG_PORT_NO;
    pctx->pdev_for_map = &pdev->dev;
    dma_set_mask_and_coherent(pctx->pdev_for_map, WETH_DMA_MASK);
    weth_stick_set_dev_for_map(pctx->pdev_for_map);

    pcie_id = ep_weth_get_pcie_id((struct device_node *)(pdev->dev.of_node));
    if (pcie_id >= WETH_PCIE_ID_MAX_NUM) {
        weth_pr_err("Can't find [pcie_id]!\n");
        return -ENXIO;
    }
    pctx->pcie_id = pcie_id;

    return 0;
}

STATIC int ep_pltfm_remove(struct platform_device *pdev)
{
    weth_pr_err("ep_pltfm_remove\n");
    return 0;
}

#ifdef CONFIG_PM
STATIC int ep_pltfm_suspend(struct device *dev)
{
    return 0;
}

STATIC int ep_pltfm_resume(struct device *dev)
{
    weth_pr_err("ep_pltfm resume ok\n");
    return 0;
}

STATIC const struct dev_pm_ops ep_pltfm_pm_ops = {
    .suspend = ep_pltfm_suspend,
    .resume = ep_pltfm_resume,
};

#define PETH_EP_PM_OPS (&ep_pltfm_pm_ops)
#else
#define PETH_EP_PM_OPS NULL
#endif

STATIC const struct of_device_id ep_match[] = {
    { .compatible = "iot,weth_ep" },
    {},
};

STATIC struct platform_driver ep_pltfm_driver = {
    .probe	= ep_pltfm_probe,
    .remove = ep_pltfm_remove,
    .driver = {
        .name = "weth_ep",
        .of_match_table = ep_match,
        .pm = PETH_EP_PM_OPS,
    },
};

STATIC int ep_platform_driver_register(void)
{
    int ret;

    ret = platform_driver_register(&ep_pltfm_driver);
    if (ret) {
        weth_pr_err("register peth_rc_device failed\n");
    }

    return ret;
}

int weth_ep_bar_init(void)
{
    int ret;
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    pctx->virt_addr = dma_alloc_coherent(pctx->pdev_for_map,
        DOUBLE_FOR_ALIGN * PCIE_NIC_BAR_SIZE, (dma_addr_t *)&pctx->org_phys_addr, GFP_KERNEL);
    if (pctx->virt_addr == NULL) {
        return -1;
    }

    pctx->phys_addr = ALIGN_64K((unsigned long)pctx->org_phys_addr);
    pctx->virt_addr += ((char *)pctx->phys_addr - (char *)pctx->org_phys_addr);
    pctx->buffer_size = PCIE_NIC_BAR_SIZE;

    ret = bsp_pcie_ep_bar_config(PCIE_NIC_BAR_NUM, PCIE_NIC_BAR_SIZE,
        (u64)(unsigned long)pctx->phys_addr);
    if (ret) {
        pctx->init_stats.bar_config_fail++;
        weth_pr_err("pcie ep msi bar config fail\n");
        return ret;
    }

    pctx->desc = pctx->virt_addr;
    pctx->init_stats.bar_init_success++;

    return 0;
}

static struct task_struct *g_peth_tsk;

STATIC int weth_ep_thread(void *data)
{
    while (down_interruptible(&g_weth_ep_ctx.weth_ep_init_sem));
    return weth_ep_init();
}

STATIC void weth_ep_init_handle(void)
{
    if (g_is_first_init) {
        up(&g_weth_ep_ctx.weth_ep_init_sem);
    }
}

void weth_exit(void)
{
    g_is_first_init = 0;
    g_weth_ep_ctx.link = 0;
    weth_pr_err("weth_exit\n");
    return;
}

void weth_linkdown(void)
{
    weth_pr_err("weth_linkdown\n");
    return;
}

void weth_powerdown(void)
{
    g_weth_ep_ctx.cur_speed_level = LOW_SPEED_LEVEL;
}

STATIC int weth_ep_init_start(unsigned int usr_id, unsigned int callback_stage, void *callback_args)
{
    int ret = 0;

    switch (callback_stage) {
        case PCIE_EP_CB_BAR_CONFIG:
            ret = weth_ep_bar_init();
            break;
        case PCIE_EP_CB_PCIE_INIT_DONE:
            weth_ep_init_handle();
            break;
        case PCIE_EP_CB_EXIT:
            weth_exit();
            break;
        case PCIE_EP_CB_LINKDOWN:
            weth_linkdown();
            break;
        case PCIE_EP_CB_POWER_DOWN:
            weth_powerdown();
            break;
        default:
            break;
    }
    return ret;
}

STATIC int weth_ep_callback_register(void)
{
    int ret;
    struct pcie_callback_info pcie_nic_callback_info = {0};

    ret = ep_platform_driver_register();
    if (ret) {
        return ret;
    }

    sema_init(&g_weth_ep_ctx.weth_ep_init_sem, 0);
    g_peth_tsk = kthread_run(weth_ep_thread, NULL, "weth_ep_thread");
    weth_pr_err("weth_ep_thread init sucsess\n");

    pcie_nic_callback_info.callback = (pcie_callback)weth_ep_init_start;
    pcie_nic_callback_info.callback_args = NULL;
    ret = bsp_pcie_ep_cb_register(PCIE_USER_ETH_DEV, &pcie_nic_callback_info);

    g_weth_ep_ctx.init = 1;
    return ret;
}

postcore_initcall_sync(weth_ep_callback_register);

STATIC struct sk_buff *weth_ep_get_skb_to_send(struct weth_ep_ctx *pctx, unsigned int *tx_list_len)
{
    struct sk_buff *skb = NULL;
    if (unlikely(pctx->tx_tail_skb_all == NULL)) {
        return NULL;
    }

    skb = pctx->tx_tail_skb_all;
    *tx_list_len = pctx->tx_list_len_all;
    /* can be reused in xmit */
    pctx->tx_list_len_all = 0;
    pctx->tx_tail_skb_all = NULL;

    return skb;
}

STATIC void weth_ep_send_skb(struct weth_ep_ctx *pctx, struct sk_buff *skb,
    unsigned int len, int direct_free)
{
    unsigned long flags;

    if (unlikely(skb == NULL)) {
        pctx->stat_work_no_skb++;
        goto lock_out;
    }

    if (unlikely(direct_free)) {
        pctx->stat_vote_fail_free++;
        __weth_ep_free_skb_list(skb);
        goto lock_out;
    }

    pctx->cur_total_tx_pkg += len;
    weth_ep_dma_send_cnt(pctx, skb, len, g_send_cnt, WETH_CPU_PORT_NO);

lock_out:
    spin_lock_irqsave(&pctx->tx_lock_all, flags);
    pctx->vote_lock_opt--;
    spin_unlock_irqrestore(&pctx->tx_lock_all, flags);
}

STATIC void weth_xmit_work(struct work_struct *work)
{
    int ret;
    int direct_free = 0;
    struct sk_buff *queue_skb = NULL;
    unsigned int tx_list_len = 0;
    unsigned long flags;
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;

    spin_lock_irqsave(&pctx->tx_lock_all, flags);
    pctx->vote_lock_opt++;
    if (unlikely(!pctx->is_wakeup)) {
        pctx->stat_wait_vote_call++;
        spin_unlock_irqrestore(&pctx->tx_lock_all, flags);
        pctx->deep_sleep_vote++;
        weth_pr_dbg("bsp_pcie_ep_vote_lock 1\n");
        if (pctx->tx_tail_skb_all != NULL) {
            struct sk_buff *first_skb = pctx->tx_tail_skb_all->next;
            struct rx_cb_map_s *cb_map = (struct rx_cb_map_s *)&first_skb->cb;
            cb_map->userfield1 = WETH_EP_WAKEUP_FLAG;
        }
        ret = bsp_pcie_ep_vote_lock(PCIE_USER_ETH_DEV, 1);
        pctx->deep_sleep_vote--;
        weth_pr_dbg("bsp_pcie_ep_vote_lock 1 finish\n");

        spin_lock_irqsave(&pctx->tx_lock_all, flags);
        if (unlikely(ret != 0)) {
            pctx->stat_wait_vote_fail++;
            direct_free = 1;
        } else {
            pctx->is_wakeup = 1;
        }
        pctx->cur_total_tx_pkg += tx_list_len;
    }

    queue_skb = weth_ep_get_skb_to_send(pctx, &tx_list_len);
    spin_unlock_irqrestore(&pctx->tx_lock_all, flags);

    weth_ep_send_skb(pctx, queue_skb, tx_list_len, direct_free);

    return;
}

int weth_ep_tx_pre_check(struct weth_ep_ctx *pctx, struct sk_buff *skb)
{
    struct sk_buff *cur_skb = NULL;
    if (unlikely(!pctx->link)) {
        weth_pr_dbg("link is not up\n");
        pctx->tx_stats.ep_link_not_up++;
        __weth_ep_free_skb_list(skb);
        return -1;
    }

    if (unlikely(pctx->tx_list_len_all > WETH_VOTE_PROTECT_NUM)) {
        pctx->stat_work_protect++;
        cur_skb = skb->next;
        skb->next = NULL;

        __weth_ep_free_skb_list_by_null(cur_skb);
        return -1;
    }

    return 0;
}

void weth_ep_tx_restart_work(struct weth_ep_ctx *pctx, struct sk_buff *skb, unsigned int num)
{
    pctx->stat_nowait_vote_fail++;

    if (pctx->tx_tail_skb_all != NULL) {
        __skb_onelink_tail(pctx->tx_tail_skb_all, skb);
    }

    pctx->tx_tail_skb_all = skb;
    pctx->tx_list_len_all += num;
    pctx->vote_lock_opt--;

    queue_delayed_work(pctx->xmit_queue, &pctx->xmit_work, 0);
}

int weth_ep_submit_tx(struct sk_buff *skb, unsigned int num, int port_no)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    unsigned long flags;
    int ret;
    int vote;

    pctx->tx_stats.ep_submit_tx++;
    ret = weth_ep_tx_pre_check(pctx, skb);
    if (ret != 0) {
        return -1;
    }
    /* don't use lock in bulk packets */
    vote = 1;

    spin_lock_irqsave(&pctx->tx_lock_all, flags);
    /* may modify by other cpu/task */
    if (pctx->is_wakeup) {
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
        weth_pr_dbg("bsp_pcie_ep_vote_lock 0\n");
        ret = bsp_pcie_ep_vote_lock(PCIE_USER_ETH_DEV, 0);
    }

    if (unlikely(ret != 0)) {
        weth_ep_tx_restart_work(pctx, skb, num);
        spin_unlock_irqrestore(&pctx->tx_lock_all, flags);
        return 0;
    }
    pctx->cur_total_tx_pkg += num;
    pctx->is_wakeup = 1;

    spin_unlock_irqrestore(&pctx->tx_lock_all, flags);

    ret = weth_ep_dma_send_cnt(pctx, skb, num, g_send_cnt, port_no);

    spin_lock_irqsave(&pctx->tx_lock_all, flags);
    pctx->vote_lock_opt--;
    spin_unlock_irqrestore(&pctx->tx_lock_all, flags);

    return ret;
}

int bsp_weth_hds_transreport(struct weth_debug_info_s *debug_info)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    if (pctx == NULL || debug_info == NULL) {
        return -1;
    }

    debug_info->remote_eth_rx_total = pctx->tx_stats.remote_eth_send;
    debug_info->dma_send = pctx->tx_stats.dma_send;
    debug_info->dma_send_packets = pctx->tx_stats.dma_send_packets;
    debug_info->dma_send_tx_queue_full = pctx->tx_stats.tx_queue_full;
    debug_info->dmasend_drop = pctx->tx_stats.dmasend_drop;
    debug_info->remote_eth_tx_total = pctx->rx_stats.remote_eth_tx_total;

    return weth_spe_get_debug_info(debug_info);
}

int bsp_pnet_register_state_enable_cb(pnet_state_notify_cb_t cb)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    pctx->state_enable_cb = cb;

    return 0;
}

int bsp_pnet_register_state_disable_cb(pnet_state_notify_cb_t cb)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    pctx->state_disable_cb = cb;

    return 0;
}

void bsp_pnet_registe_get_mac_cb(ppp_ndis_get_mac_cb cb)
{
    struct weth_ep_ctx *pctx = &g_weth_ep_ctx;
    pctx->get_mac_cb = cb;
}

int bsp_pcie_support_5g_lan(unsigned int net_id)
{
    unsigned char bit_mask = 1;
    if (net_id >= WETH_STICK_NIC_NUM) {
        weth_pr_err("net id %d overflow\n", net_id);
        return 0;
    }
    bit_mask = bit_mask << net_id;
    return bit_mask & g_weth_ep_ctx.net_feature.ethernet_feature;
}
