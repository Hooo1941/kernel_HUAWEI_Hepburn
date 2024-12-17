/*
 * cloud_network_rx.h
 *
 * cloud network kernel module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _CLOUD_NETWORK_DATA_PROCESS_H_
#define _CLOUD_NETWORK_DATA_PROCESS_H_

#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <net/tcp.h>
#include "cloud_network.h"
#include "cloud_network_device.h"
#include "cloud_network_honhdr.h"
#include "cloud_network_session.h"
#define RECEIVE_WINDOW_DEFAULT 1024

#define OOO_SKB_MM_POOL_NUM 1024
#define OOO_SKB_MM_POOL_NUM_MAX 2048

#define CLOUD_NETWORK_TCP_MSS 1312

#define SKB_RECEIVE_TIMEOUT_MS 40
#define SKB_RECEIVE_TIMEOUT_TIMER_INTERVAL_MS 10
#define SKB_RECEIVE_DATA_STAT_TIMEOUT_MS (MS_IN_SECOND * SECOND_IN_MIN)
#define SECOND_IN_MIN 60
#define MS_IN_SECOND 1000ull

#define STEP_MS 5
#define EXPECTED_NETWORK_LATENCY_MS 200u
#define TABLE_SIZE (40 + 1) /* 200ms/5ms per 5ms a step and a step is over 200ms */
#define MAX_SKB_MM 100

#define HANG_TIME_MS 700 /* 700ms */

enum {
	RECEIVE_CONTROL_STATUS,
};

struct gain_stat_struct {
	struct hrtimer primary_hang_timer;
	struct hrtimer secondary_hang_timer;
	struct hrtimer mulit_hang_timer;
	struct timespec64 primary_modem_pre_skb_rx_ts;
	struct timespec64 secondary_modem_pre_skb_rx_ts;
	atomic_t status;
	spinlock_t lock;
};

struct hon_skb {
	struct sk_buff *skb;
	struct honhdr hon;
};

struct ooo_skb {
	struct rb_node rbnode;
	/* received continuous sequence packets */
	struct sk_buff_head skb_queue;
	u32 seq_start;
	u32 seq_end;
	/* timestamp for inserting the rbtree, recv_timeout hrtimer judgment use */
	u32 tstamp;
};

struct recv_ctl {
	unsigned long status;
	/* recv_wnd recv_nxt lock */
	spinlock_t recv_ctl_lock;
	/* Current receiver window */
	u32 recv_wnd;
	/* What we want to receive next */
	u32 recv_nxt;
	/* OOO segments skb go in this rbtree. lock must be held */
	struct rb_root out_of_order;
	spinlock_t out_of_order_lock;
	struct ooo_skb *ooo_last; /* cache out_of_order rbtree last item */
	struct hrtimer recv_timeout;
	struct tasklet_struct recv_timeout_handle_tasklet;
};

struct dev_data_info {
	atomic_t min_seq;
	atomic_t max_seq;
	atomic_t packets_rec;
};

struct time_info {
	atomic64_t last_ntimestamp;
	atomic64_t last_htimestamp;
	atomic_t last_seq;
};

struct statistics_data {
	struct dev_data_info info[ROLLE_MAX + 1];
	struct hrtimer recv_statistics_data;
};

int cloud_network_rx(struct sk_buff *skb, struct net_device *dev);
void tcp_mss_replace(struct sk_buff *skb, struct tcphdr *th);
int cloud_network_rx_init(void);
void cloud_network_data_rx_exit(void);
u64 ntohll(u64 val);
u64 htonll(u64 val);
u32 get_rel_time_us(void);
u64 get_rel_time_ms(void);
void prob_timer_state(int state);
void all_window_reset(struct session *ss);
void start_gain_detection(void);
void stop_gain_detection(void);
#endif /* _CLOUD_NETWORK_DATA_PROCESS_H_ */
