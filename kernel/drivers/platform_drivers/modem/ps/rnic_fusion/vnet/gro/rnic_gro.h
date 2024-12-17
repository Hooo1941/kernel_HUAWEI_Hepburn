/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#ifndef RNIC_GRO_H
#define RNIC_GRO_H

#include <linux/jiffies.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_GRO_HASH_SIZE 16
#define RNIC_GRO_HASH_MASK (RNIC_GRO_HASH_SIZE - 1)
#define RNIC_GRO_MAX_FLOW_PER_BUCKET 8
#define RNIC_GRO_MAX_BLOCK_PER_FLOW 8

#define RNIC_GRO_TCP_DEF_WEIGHT 8
#define RNIC_GRO_TCP_MAX_WEIGHT 64

#define RNIC_GRO_UDP_MAX_WEIGHT 64

struct rnic_gro_tcp_stats_s {
	u32 hdr_errs;
	u32 csum_errs;

	u32 new_flow;
	u32 merged;
	u32 reorder;
	u32 blk_full;
	u32 blk_exhaust;
	u32 bucket_full;

	u32 flush_flow;
	u32 flush_single;
	u32 flush_all;
};

struct rnic_gro_udp_stats_s {
	u32 hdr_errs;
	u32 csum_errs;
	u32 sk_gro_rcv;
	u32 sk_gro_enable;

	u32 new_flow;
	u32 merged;
	u32 bucket_full;
	u32 flush_flow;
	u32 flush_single;
	u32 flush_all;
};

struct rnic_gro_stats_rpt_s {
	struct {
		u8 ver;
		u8 reserved[7];
	} hdr;
	struct rnic_gro_tcp_stats_s tcp_stats;
	struct rnic_gro_udp_stats_s udp_stats;
};

struct rnic_gro_list_s {
	struct list_head head;
	u32 count;
};

typedef void (*rnic_gro_deliver_fn)(struct sk_buff *skb);

struct rnic_gro_ops_s {
	void *(*gro_init)(rnic_gro_deliver_fn deliver_func);
	void (*gro_deinit)(void *table);
	void (*gro_receive)(void *table, struct sk_buff *skb);
	void (*gro_complete)(void *table);
	void (*gro_set_weight)(void *table, u32 weight);
	void (*gro_show_stats)(void *table);
};

#ifdef __cplusplus
}
#endif

#endif /* RNIC_GRO_H */
