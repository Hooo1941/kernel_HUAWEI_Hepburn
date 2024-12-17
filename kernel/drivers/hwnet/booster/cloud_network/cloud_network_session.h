/*
 * cloud_network_session.h
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

#ifndef _CLOUD_NETWORK_SESSION_H_
#define _CLOUD_NETWORK_SESSION_H_

#include <linux/types.h>
#include <linux/spinlock.h>
#include <net/netfilter/nf_conntrack.h>

#include "cloud_network_honhdr.h"

#define SESSION_NUM 2048
#define SESSION_LOCKS_NUM 2048
#define SESSION_COUNT_MAX 2048

#define UID_SESSION_NUM 512
#define UID_SESSION_LOCK_NUM 512
#define UID_SESSION_COUNT_MAX 512

#define MAX_SEQ 0xffffffff
#define SESSION_PCPU_LISTS_NULLS_VAL ((1 << 6) + 0)

#define SLIDE_WIN_SIZE 2048
#define SLIDE_HALF 2

#define RX_INTERVAL_STEP 25 /* ms */
#define RX_INTERVAL_NUM 41

struct session {
	atomic_t use;
	spinlock_t lock;
	struct nf_conntrack_tuple_hash session_hash[IP_CT_DIR_MAX];
	struct hlist_node uid_hash;
	possible_net_t session_net;
	u32 link_type;
	u32 uid;
	struct uid_session *uid_ss;

	u64 packet_rx_multi_path;
	u64 packet_rx_multi_path_gain;
	u64 packet_tx;
	u64 packet_tx_primary;
	u64 packet_tx_secondary;
	u64 packet_rx;
	u64 packet_rx_primary;
	u64 packet_rx_secondary;
	u64 traffic_tx;
	u64 traffic_tx_primary;
	u64 traffic_tx_secondary;
	u64 traffic_rx;
	u64 traffic_rx_primary;
	u64 traffic_rx_secondary;
	u64 timestamp_tx;
	u64 packet_rx_multi_path_last;
	u64 packet_rx_multi_path_gain_last;
	u64 packet_tx_last;
	u64 packet_tx_primary_last;
	u64 packet_tx_secondary_last;
	u64 packet_rx_last;
	u64 packet_rx_primary_last;
	u64 packet_rx_secondary_last;
	u64 traffic_tx_last;
	u64 traffic_tx_primary_last;
	u64 traffic_tx_secondary_last;
	u64 traffic_rx_last;
	u64 traffic_rx_primary_last;
	u64 traffic_rx_secondary_last;
	u32 per_start_rx_seq;
	u32 per_end_rx_seq;
	u32 per_packet_rx;
	u32 per_start_rx_seq_primary;
	u32 per_end_rx_seq_primary;
	u32 per_packet_rx_primary;
	u32 per_start_rx_seq_secondary;
	u32 per_end_rx_seq_secondary;
	u32 per_packet_rx_secondary;
	u64 rx_timestamp;
	u64 rx_timestamp_primary;
	u64 rx_timestamp_secondary;
	u32 rx_interval[RX_INTERVAL_NUM];
	u32 rx_interval_primary[RX_INTERVAL_NUM];
	u32 rx_interval_secondary[RX_INTERVAL_NUM];

	u32 saddr;
	u32 daddr;
	u16 cpu;
	__be16	id_tx;
	u16 source;
	u16 dest;
	int ss_window[SLIDE_WIN_SIZE];
	u64 begin;
	u64 end;
	atomic_t send_seq;
	__be16	id_rx;
	u8  protocol;
};

struct uid_session {
	atomic_t use;
	spinlock_t lock;
	struct rb_node node;
	struct rcu_head rcu;
	u32 uid;
	struct hlist_head *hash_table; /* the master sessions hash tableof this uid */
	unsigned int hash_table_size;
	spinlock_t hash_table_locks[UID_SESSION_LOCK_NUM];
	atomic_t hash_table_count;
};

struct session_pcpu {
	spinlock_t lock;
	struct hlist_nulls_head dying;
};

bool get_skb_tuple_reply(struct nf_conntrack_tuple *tuple,
	const struct sk_buff *skb);
bool get_skb_tuple_original(struct nf_conntrack_tuple *tuple,
	const struct sk_buff *skb);
bool is_session_existed(struct net *net,
	const struct nf_conntrack_tuple *tuple);
bool cloud_network_started(void);
struct nf_conntrack_tuple_hash *session_find_get(struct net *net,
	const struct nf_conntrack_tuple *tuple);
struct session *fragment_tx_session_find_get(struct sk_buff* skb);
struct session *fragment_rx_session_find_get(struct sk_buff* skb);
void session_put(struct session *ss);
struct session *master_session_add_get(struct nf_conntrack_tuple *tuple,
	struct sk_buff *skb, u32 uid);
void session_update_tx(struct session *ss, struct sk_buff *skb);
void session_update_tx_dual(struct session *ss, struct sk_buff *skb, struct honhdr *honh);
void session_update_rx(struct session *ss, struct sk_buff *skb,
	struct honhdr *honh, u64 timestamp);
void session_update_rx_dual(struct session *ss, struct sk_buff *skb,
	struct honhdr *honh, u64 timestamp);
void master_session_delete(struct nf_conntrack_tuple *tuple, struct net *net);
void start_all_session_print(void);
void stop_all_session_print(void);
void ims_check_send(u32 uid);
void ims_check_recv(struct session *ss);
void all_session_window_reset(void);
int cloud_network_session_init(void);
void cloud_network_session_reset(void);
void cloud_network_session_exit(void);
#endif /* _CLOUD_NETWORK_SESSION_H_ */
