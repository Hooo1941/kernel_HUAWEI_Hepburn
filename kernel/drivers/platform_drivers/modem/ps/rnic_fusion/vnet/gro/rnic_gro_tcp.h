/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
*/

#ifndef RNIC_GRO_TCP_H
#define RNIC_GRO_TCP_H

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include "rnic_gro.h"
#include "rnic_gro_hash.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rnic_gro_tcp_pkt_s {
	struct net_device *netdev;
	union {
		struct rnic_gro_ip4_flow_keys_s keys4;
		struct rnic_gro_ip6_flow_keys_s keys6;
	};
	u32 hash;

	u32 thlen;
	u32 payload_offset;
	u32 payload_len;
	u32 seq;
	u32 next_seq;

	void *iph;
	struct tcphdr *th;

	bool is_v4;
	bool flush;
	bool merged;
	bool splice_head;

	struct sk_buff *skb;
};

struct rnic_gro_tcp_blks {
	struct list_head list;

	u32 payload_len;
	u32 truesize;

	u32 seq;
	u32 next_seq;
	__be32 flags;
	u16 gso_size;
	u16 count;
	u32 front;

	void *iph;
	struct tcphdr *th;

	struct sk_buff *first;
	struct sk_buff *last;
};

struct rnic_gro_tcp_flow_s {
	struct list_head list;

	struct net_device *netdev;
	union {
		struct rnic_gro_ip4_flow_keys_s keys4;
		struct rnic_gro_ip6_flow_keys_s keys6;
	};
	u32 hash;
	u32 blk_count;
	bool is_v4;
	struct list_head blk_head;
};

struct rnic_gro_tcp_tbl_s {
	struct rnic_gro_tcp_stats_s stats;
	u32 weight;
	u32 blk_cap;
	u32 blk_remain;

	void (*deliver_skb)(struct sk_buff *skb);

	unsigned long flow_bitmask;
	struct rnic_gro_list_s flow_hash[RNIC_GRO_HASH_SIZE];

	struct list_head flow_free_list;
	struct list_head blk_free_list;
	struct rnic_gro_tcp_flow_s *flows_buff;
	struct rnic_gro_tcp_blks *blks_buff;
} ____cacheline_aligned;

extern const struct rnic_gro_ops_s rnic_gro_tcp_ops;

#ifdef __cplusplus
}
#endif

#endif /* RNIC_GRO_TCP_H */
