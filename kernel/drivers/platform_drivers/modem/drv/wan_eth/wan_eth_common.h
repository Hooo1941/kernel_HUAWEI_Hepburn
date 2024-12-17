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
#ifndef __WAN_ETH_COMMON_H__
#define __WAN_ETH_COMMON_H__
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <osl_types.h>
#include <linux/if_ether.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */

/* weth debug level */
#define WETH_LEVEL_ERR BIT(0)
#define WETH_LEVEL_WARN BIT(1)
#define WETH_LEVEL_DBG BIT(2)
#define WETH_LEVEL_PKG BIT(3)
#define WETH_LEVEL_RX BIT(4)
#define WETH_LEVEL_TX BIT(5)

#define STATIC

#define WETH_SMP_BIT_SCHED 0

#define WETH_DMA_READ PCIE_DMA_DIRECTION_READ
#define WETH_DMA_WRITE PCIE_DMA_DIRECTION_WRITE
#define WETH_SKB_MTU 1600
#define WETH_INVALID_PORT_NO 0xf
#define WETH_DEBUG_PORT_NO 0x8
#define WETH_CPU_PORT_NO 6
#define WETH_EP_WAKEUP_FLAG 0x1
#define WETH_ETH_MODE 0x455448

#define WETH_RC_HIGH_SPEED 1
#define WETH_RC_LOW_SPEED 0
#define WETH_LOW_SPEED_COUNT 3
#define WETH_RC_SPEED_THRESHOLD 200
#define WETH_RC_SPEED_UP_THRESHOLD 250
#define WETH_RC_SPEED_DOWN_THRESHOLD 150
#define WETH_RC_DEBUG_FLAG 0x5a
#define WETH_RC_DEBUG_PKT 0x5b
#define WETH_RC_HIDS_PKT_OPEN 0x55
#define WETH_EP_VOTE_COUNT 0x6a

#define WETH_TAB_ELEMENT_CNT    50     /* m in doc */
#define WETH_RC_RX_TAB_MAX_NUM        20     /* n in doc */
#define WETH_EP_RX_TAB_MAX_NUM        10    /* n in doc */
#define TABLE_REDUNDANCY_FOR_HEAD 2

#define MASK(n) (((1) << (n)) - 1)
#define WETH_HAL_IDX_SELF_INC(n, m) (((n) + 1) & MASK(m))
#define WETH_SPEED_BIT_TO_MB (1024 * 1024)
#define WETH_MACROSECOND_TO_MILLISECOND 1000
#define WETH_PACKET_ACCUMULATE_TIMEOUT 1000000 /* 1000000 nanoseconds is 1 millisecond */
#define WETH_MAX_TX_QUEUE_LEN (WETH_TAB_ELEMENT_CNT - TABLE_REDUNDANCY_FOR_HEAD)

#define pr_weth_fmt(args) "[weth]" args

struct common_info {
    int link_status;
    int table_sync_finish;
    int init_finish;
    int exit_finish;
    atomic_t bar_access;
    unsigned int link_down_rx;
    unsigned int link_down_tx;
    unsigned int wait_count;
    unsigned int wait_for_linkup;
    unsigned int wait_init_finish;
    unsigned int wait_table_finish;
};

struct weth_packet_dump {
    unsigned int sent_packets;
    unsigned int free_packets;
    unsigned int maa_free;
    unsigned int kernel_free;
};

struct weth_common_ctx {
    unsigned long long dl_speed;
    unsigned long long ul_speed;
    unsigned int speed_threshold;
    unsigned int speed_up_threshold;
    unsigned int speed_down_threshold;
    int cur_speed_level;
    unsigned int low_speed_count;
    unsigned int rx_packets_per_cpu[NR_CPUS];
};

extern unsigned int g_weth_debug_level;
extern int g_by_pass_mode;
extern struct common_info g_common_info;
extern struct weth_common_ctx g_common_ctx;
extern struct weth_packet_dump g_packet_dump;
extern int g_vote_count;

#define weth_pr_err(args, ...) do { \
        printk(KERN_EMERG pr_weth_fmt(args), ##__VA_ARGS__); \
} while (0)

#define weth_pr_warn(args, ...) do { \
    if (g_weth_debug_level & WETH_LEVEL_WARN)                \
        printk(KERN_EMERG pr_weth_fmt(args), ##__VA_ARGS__); \
} while (0)

#define weth_pr_dbg(args, ...) do { \
    if (g_weth_debug_level & WETH_LEVEL_DBG)                 \
        printk(KERN_EMERG pr_weth_fmt(args), ##__VA_ARGS__); \
} while (0)

#define weth_pr_rx(args, ...) do { \
    if (g_weth_debug_level & WETH_LEVEL_RX)                  \
        printk(KERN_EMERG pr_weth_fmt(args), ##__VA_ARGS__); \
} while (0)

#define weth_pr_tx(args, ...) do { \
    if (g_weth_debug_level & WETH_LEVEL_TX)                  \
        printk(KERN_EMERG pr_weth_fmt(args), ##__VA_ARGS__); \
} while (0)

#define MAX_PRINT_PKT_LEN 64
#define MAX_PRINT_PM_PKT_LEN 40
#define PRINT_BYTES_PER_LINE 16
#define WETH_NAPI_WEIGHT 64
#define BYTE_TO_BITS 8
#define BITS_TO_MEGABITS 20
#define NANOSECOND 1

static inline unsigned int weth_jiffies_sub(unsigned long jiffies1, unsigned long jiffies2)
{
    unsigned int jiffies_sub;

    if (jiffies1 >= jiffies2) {
        jiffies_sub = jiffies1 - jiffies2;
    } else {
        jiffies_sub = (unsigned int)((long)jiffies1 - (long)jiffies2);
    }

    return jiffies_sub;
}
static inline void weth_pr_pkg(unsigned char *buf, int len)
{
    int j, count;

    count = len > MAX_PRINT_PKT_LEN ? MAX_PRINT_PKT_LEN : len;
    for (j = 0; j < count; j += PRINT_BYTES_PER_LINE) {
        weth_pr_dbg("%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", j,
                    buf[j], buf[j + 1], buf[j + 2], buf[j + 3], buf[j + 4], buf[j + 5], buf[j + 6], buf[j + 7],
                    buf[j + 8], buf[j + 9], buf[j + 0xa], buf[j + 0xb], buf[j + 0xc], buf[j + 0xd], buf[j + 0xe],
                    buf[j + 0xf]);
    }
    weth_pr_dbg("\n");
}

static inline void weth_pr_pkg_pm(unsigned char *buf, int len)
{
    int j, count;

    count = len > MAX_PRINT_PKT_LEN ? MAX_PRINT_PKT_LEN : len;
    for (j = 0; j < count; j += PRINT_BYTES_PER_LINE) {
        weth_pr_err("%03x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", j,
                    buf[j], buf[j + 1], buf[j + 2], buf[j + 3], buf[j + 4], buf[j + 5], buf[j + 6], buf[j + 7],
                    buf[j + 8], buf[j + 9], buf[j + 0xa], buf[j + 0xb], buf[j + 0xc], buf[j + 0xd], buf[j + 0xe],
                    buf[j + 0xf]);
    }
    weth_pr_err("\n");
}

/* one link skb (the skb must be the last node in list) */
static inline void __skb_onelink_init(struct sk_buff *skb)
{
    skb->next = skb;
}

static inline void __skb_onelink_tail(struct sk_buff *prev_skb, struct sk_buff *new_skb)
{
    struct sk_buff *first_skb = new_skb->next;
    new_skb->next = prev_skb->next;
    prev_skb->next = first_skb;
}

static inline void skb_onelink_tail(struct sk_buff **skb_tail, struct sk_buff *skb)
{
    __skb_onelink_init(skb);

    if (*skb_tail) {
        __skb_onelink_tail(*skb_tail, skb);
    }

    *skb_tail = skb;
}

#define __skb_onelink_for_each(is_last, pos, tail) \
    for ((pos) = (tail)->next, (is_last) = 0; !(is_last); (is_last) = (int)((pos) == (tail)), (pos) = (pos)->next)

#define __skb_onelink_each_safe(is_last, pos, tail, tmp) \
    for ((pos) = (tail)->next, (is_last) = 0, (tmp) = (pos)->next; !(is_last); (is_last) = (int)((pos) == (tail)), \
        (pos) = (tmp), (tmp) = (pos)->next)

static inline int __skb_is_onelink(struct sk_buff *prev_skb)
{
    return (int)(prev_skb->next != NULL);
}

static inline struct sk_buff *__weth_alloc_skb(int len)
{
    struct sk_buff *skb;

    skb = netdev_alloc_skb_ip_align(NULL, len);
    if (NULL == skb) {
        weth_pr_err("skb is null\n");
        return skb;
    }

    skb_reserve(skb, ETH_HLEN);
    return skb;
}

static inline void __weth_free_skb(struct sk_buff *skb)
{
    dev_kfree_skb_any(skb);
}

static inline void __weth_free_skb_list(struct sk_buff *skb)
{
    struct sk_buff *cur_skb = NULL;
    struct sk_buff *next = NULL;
    int is_last;

    __skb_onelink_each_safe(is_last, cur_skb, skb, next) {
        dev_kfree_skb_any(cur_skb);
    }
    cur_skb = NULL;
    next = NULL;
}

static inline void __weth_free_skb_list_by_null(struct sk_buff *skb)
{
    struct sk_buff *next_skb = NULL;

    while (skb != NULL) {
        next_skb = skb->next;
        __weth_free_skb(skb);
        skb = next_skb;
    }
}

static inline unsigned int weth_rw_get_pos(unsigned int rw, unsigned int max)
{
    return (rw & (max - 1));
}

static inline unsigned int weth_rw_get_busy(unsigned int w, unsigned int r, unsigned int max)
{
    return (w >= r) ? (w - r) : (max - (r - w));
}

static inline unsigned int weth_rw_get_idle(unsigned int w, unsigned int r, unsigned int max)
{
    return (w >= r) ? (max - (w - r) - 1) : (r - w - 1);
}

void weth_print_pkt_info(unsigned char *data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __WAN_ETH_TABLE_H__ */
