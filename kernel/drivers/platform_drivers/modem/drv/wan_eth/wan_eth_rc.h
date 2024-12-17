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

#ifndef WAN_ETH_RC_H
#define WAN_ETH_RC_H

#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/version.h>

#include "mdrv_remote_eth.h"
#include "securec.h"
#include "bsp_pcie.h"
#include "wan_eth_event_buffer.h"
#include "wan_eth_table.h"
#include "bsp_wan_eth.h"

#define WETH_MAX_RX_NODE 2048
#define WETH_MAX_CPU_NUM 32
#define WETH_VOTE_TIMEOUT 100 /* ms unit */
#define WETH_NO_PKG_COUNT 15
#define WETH_VOTE_PROTECT_NUM 1024
#define WETH_CHN_NUM NR_CPUS
#define WETH_TX_CALC_TIME 10
#define WETH_MAX_NAP_QLEN 1024
#define WETH_RX_NUM 1024
#define VOTE_PER_SECOND (1000 / WETH_VOTE_TIMEOUT)

struct weth_stats {
    unsigned int stat_open;
    unsigned int stat_close;
    unsigned int stat_xmit_packets;
    unsigned int stat_len_err;
    unsigned int stat_liner_err;
    unsigned int stat_tx_timeout;
    unsigned int stat_set_mtu;
    unsigned int stat_set_mac;
    unsigned int stat_rx_done;
    unsigned int stat_poll;
    unsigned int stat_poll_none;
    unsigned int stat_tx_none;
    unsigned int stat_max_tx_num;
    unsigned int stat_cur_tx_num;
    unsigned int stat_max_rx_num;
    unsigned int stat_cur_rx_num;
    unsigned int stat_no_cmp_node;

    unsigned int stat_pcie_vote_fail;
    unsigned int stat_pcie_vote_work;

    unsigned int stat_tx_first_enq;
    unsigned int stat_tx_tail_enq;
    unsigned int stat_tx_submit_fail;

    unsigned int stat_rx_invalid_devid;
    unsigned int stat_rx_drop_by_invalid_devid;
    unsigned long long stat_rx_packets;
    unsigned long long stat_rx_bytes;
    unsigned long long stat_rx_errors;
    unsigned long long rx_packets_push;
    unsigned long long devid_mask;

    unsigned long long stat_tx_packets;
    unsigned long long stat_tx_bytes;
    unsigned long long stat_tx_errors;
    unsigned long long stat_tx_dropped;
    unsigned long long stat_tx_queue;
    unsigned long long tx_timer_start;
    unsigned long long tx_timer_cancel;
    unsigned long long tx_timer_running;
    unsigned long long force_schedule;
};

struct weth_chn {
    struct napi_struct napi;
    struct sk_buff *rx_napi_skb;
    spinlock_t rx_napi_lock;
    struct weth_ctx *pctx;
    unsigned int rx_qlen;
    unsigned int extra_qlen;
    int last_sum_num;
    unsigned long last_jiffies;
};

struct weth_ctx {
    struct net_device *ndev;
    spinlock_t rx_smp_lock;
    unsigned int devid;
    spinlock_t tx_lock;
    spinlock_t rx_lock;

    unsigned int rx_wpos;
    unsigned int rx_rpos;
    unsigned int rx_num;
    struct sk_buff *rx_skb[WETH_MAX_RX_NODE];
    unsigned int rx_pkg[WETH_MAX_RX_NODE];

    struct tasklet_struct xmit_push;
    struct delayed_work xmit_work;

    struct weth_chn chn[WETH_CHN_NUM];

    unsigned int tx_list_len;
    unsigned int rx_list_len;
    struct sk_buff *tx_tail_skb;
    struct sk_buff *rx_tail_skb;
    struct sk_buff *rx_skb_handle;

    struct hrtimer tx_timer;
    unsigned int tx_timer_duration;
    unsigned int max_list_len;
    unsigned long tx_last_jiff;

    int is_open;
    int gro_enable;
    struct weth_stats stats;
};

/* *********************Macro definition************************************** */
#define WETH_MUX_NUM 30
#define WETH_STICK_NIC_NUM 8

static inline unsigned int __skb_get_devid(struct sk_buff *skb)
{
    struct remote_eth_cb_map_s *map_info;

    map_info = (struct remote_eth_cb_map_s *)skb->cb;
    return (map_info->userfield0 & 0xFFFF);
}

/* *********************Macro definition****************************************** */
/* debug mode include */
#define PCIE_NIC_BAR_SIZE BAR_SIZE_256K
#define PCIE_NIC_BAR_MASK BAR_SIZE_256K
#define PCIE_NIC_BAR_NUM PCIE_BAR_ETH_DEV

#define MASK_16_BITS ((1 << 16) - 1)
#define MASK_8_BITS ((1 << 8) - 1)

#define ALIGN_64K(addr) (void *)(((addr) & ~PCIE_NIC_BAR_MASK) + PCIE_NIC_BAR_SIZE + 1)

/* skb buff MTU definition */
#define WETH_DMA_MASK 0xffffffffffffffff

#define NUM_R2E_DESC 4096
#define MIN_IO_SIZE 0x80

#define WETH_ETH_TYPE_IP 0x0008
#define WETH_ETH_TYPE_IPV6 0xDD86

/* ******************Structure definition********************************************* */
union weth_pcie_link_status {
    struct {
        unsigned int open : 1;
        unsigned int rsv : 31;
    } bits;
    unsigned int bit32;
};

struct weth_mac_addr {
    char has_mac;
    unsigned char mac_addr[ETH_ALEN];
};

struct weth_map_ep {
    union weth_pcie_link_status self_status;
    union weth_pcie_link_status peer_status;
    unsigned int enable_local_irq;
    unsigned int enable_remote_irq;

    /* add for table */
    struct weth_tab_addr_table_rc addr_table_rc;
    struct weth_tab_addr_table_ep addr_table_ep;

    /* event buffer */
    struct event_buffer tx_event;
    struct event_buffer rx_event;
    struct weth_mac_addr mac_addr[WETH_STICK_NIC_NUM];
} __attribute__((aligned(4)));

struct weth_map_rc {
    union weth_pcie_link_status peer_status;
    union weth_pcie_link_status self_status;
    unsigned int enable_remote_irq;
    unsigned int enable_local_irq;

    /* add for table */
    struct weth_tab_addr_table_rc addr_table_rc;
    struct weth_tab_addr_table_ep addr_table_ep;

    /* event buffer */
    struct event_buffer rx_event;
    struct event_buffer tx_event;
    struct weth_mac_addr mac_addr[WETH_STICK_NIC_NUM];
} __attribute__((aligned(4)));

union weth_map {
    struct weth_map_ep ep;
    struct weth_map_rc rc;
};

struct weth_rc_init_status {
    unsigned int bar_init_success;
    unsigned int bar_config_fail;
    unsigned int dma_ll_init_fail;
    unsigned int dma_ll_addr_alloc_fail;
    unsigned int dma_ll_addr_init_success;
    unsigned int msi_intr_init_fail;
    unsigned int pcie_desc_init_fail;
    unsigned int peth_desc_ep_init_fail;
    unsigned int weth_rc_init_fail;
    unsigned int pcie_dma_callback_register_fail;
    unsigned int tab_init_fail;
    unsigned int event_init_fail;
};

struct weth_rc_tx_status {
    unsigned long long rc_submit_tx;
    unsigned long long rc_link_not_up;
    unsigned long long rc_submit_skb_null;
    unsigned long long start_send;
    unsigned long long doing;
    unsigned long long unhandle;
    unsigned long long pending;
    unsigned long long dma_send;
    unsigned long long dma_send_packets;
    unsigned long long dma_send_bytes;
    unsigned long long dma_transfer_fail;
    unsigned long long dma_list_complete;
    unsigned long long dma_list_uncomplete;
    unsigned long long skb_pending;
    unsigned long long remote_eth_fail;
    unsigned long long remote_eth_drop;
    unsigned long long skb_free;
    unsigned long long send_data_complete;
    unsigned long long skb_pending_free;
    unsigned long long dma_write_callback;
    unsigned long long msi_send;
    unsigned long long no_capacity;
    unsigned long long send_pending;
    unsigned long long reschedule;
    unsigned long long data_send_irq;
    unsigned long long tx_queue_full;
    unsigned long long dma_send_pending;
    unsigned long long skb_free_due_to_tdfull;
    unsigned long long send_pending_write_cb;
    unsigned long long send_pending_resend;
    unsigned long long dma_resend;
    unsigned int phy_addr_null_drop;

    unsigned int debug_pkt_xmit;
    unsigned int debug_pkt_xmit_succ;
};

struct weth_rc_rx_status {
    unsigned long long dma_rx_start;
    unsigned long long packets_received;
    unsigned long long bytes_received;
    unsigned long long peer_dma_tx_complete;
    unsigned long long rx_done_cb;
    unsigned long long dma_read_callback;
    unsigned long long msi_send;
    unsigned long long no_packets_received;
    unsigned long long data_read_irq;
    unsigned long long rx_try;
    unsigned long long rx_skb_refill_fail;
    unsigned long long ep_td_available;
    unsigned long long rx_buget_reach;
    unsigned long long ep_td_available_rx_buget;
    unsigned long long rx_queue_full;
    unsigned long long total_rx_packets;
    unsigned long long total_rx_bytes;
};

struct weth_rc_pm_status {
    unsigned int suspend_fail;
    unsigned int suspend_sucess;
    unsigned int resume_sucess;
};

struct weth_rc_ctx {
    /* kernel dev */
    struct platform_device *pdev;
    struct device *pdev_for_map;
    int vote_count;
    struct weth_rc_debug_info_s weth_rc_debug;
    int is_first_init;
    /* resource lock */
    spinlock_t rx_lock;
    spinlock_t tx_lock;

    /* netdev status */
    union weth_pcie_link_status *local_stat;
    union weth_pcie_link_status *remote_stat;

    unsigned int rx_wpos;
    unsigned int rx_rpos;
    unsigned int rx_num;
    unsigned int invalid_devid;
    unsigned int not_opened;
    struct sk_buff *rx_skb[NUM_R2E_DESC];

    /* pcie link bar */
    unsigned int init;
    unsigned int link;
    union weth_map *desc;

    /* address */
    void *phys_addr;
    void *org_phys_addr;
    void *virt_addr;
    unsigned int buffer_size;
    struct semaphore weth_rc_init_sem;
    struct tasklet_struct send_pending_packets;

    struct tasklet_struct tx_push;
    struct hrtimer tx_timer;
    struct workqueue_struct *xmit_queue;
    struct delayed_work xmit_work;
    struct timer_list vote_timer;
    struct platform_driver *rc_platform_driver;

    struct sk_buff *tx_tail_skb_all;
    unsigned int tx_list_len_all;
    spinlock_t tx_vote_lock;

    int dynamic_napi_weight;
    unsigned int tx_timer_duration;
    unsigned int max_list_len;
    unsigned int init_weight;
    unsigned int first_weight;
    unsigned int second_weight;
    unsigned int third_weight;

    unsigned int first_weight_thru;
    unsigned int second_weight_thru;
    unsigned int third_weight_thru;

    unsigned int no_data_duration;
    unsigned int no_data_jiffies;

    unsigned int evaluate_duration;
    unsigned int evaluate_jiffies;

    unsigned int init_change;
    unsigned int first_change;
    unsigned int second_change;
    unsigned int third_change;

    unsigned int is_wakeup;
    unsigned int stat_wait_vote_call;
    unsigned int stat_wait_vote_fail;
    unsigned int stat_nowait_vote_call;
    unsigned int stat_nowait_vote_fail;
    unsigned int stat_unvote_call;
    unsigned int stat_unvote_fail;

    unsigned int stat_vote_fail_free;
    unsigned int stat_work_no_skb;
    unsigned int stat_work_protect;
    int first_resume;
    unsigned int first_resume_called;
    unsigned int real_resume;

    int vote_lock_opt;
    int deep_sleep_vote;
    unsigned int last_total_tx_pkg;
    unsigned int cur_total_tx_pkg;
    unsigned int last_total_rx_pkg;
    unsigned int cur_total_rx_pkg;
    unsigned long long no_pkg_count;
    unsigned int trans_report_enable;

    /* debug stats */
    struct weth_rc_init_status init_stats;
    struct weth_rc_tx_status tx_stats;
    struct weth_rc_rx_status rx_stats;
    struct weth_rc_pm_status pm_stats;
};

struct weth_rc_ctx *weth_get_rc_ctx(void);
struct weth_ctx *weth_get_ctx(unsigned int devid);

#endif
