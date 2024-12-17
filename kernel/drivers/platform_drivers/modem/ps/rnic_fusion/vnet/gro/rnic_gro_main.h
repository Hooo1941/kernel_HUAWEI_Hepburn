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

#ifndef RNIC_GRO_MAIN_H
#define RNIC_GRO_MAIN_H

#include <linux/jiffies.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_GRO_WEIGHT_CFG_NUM 8
#define RNIC_GRO_MAX_PKT_SEGS 64
#define RNIC_GRO_DEF_QACK_THOLD 8
#define RNIC_GRO_FS_TIMEOUT (jiffies + msecs_to_jiffies(1000))

enum {
	RNIC_GRO_TCP,
	RNIC_GRO_UDP,
	RNIC_GRO_TYPE_MAX
};

#define RNIC_GRO_TYPEF_TCP (1UL << RNIC_GRO_TCP)
#define RNIC_GRO_TYPEF_UDP (1UL << RNIC_GRO_UDP)

struct rnic_gro_weight_s {
	u32 pps;
	u32 weight;
};

struct rnic_gro_flow_stats_s {
	u64 rx_pkts;
	u64 last_rx_pkts;
};

struct rnic_gro_ctx_s {
	struct rnic_gro_flow_stats_s flow_stats;
	struct timer_list fs_timer;
	u32 curr_weight;

	struct sk_buff_head *rx_list;
	unsigned long gro_types;
	void *gro_tables[RNIC_GRO_TYPE_MAX];
} ____cacheline_aligned;

extern void (*rnic_gro_rcv_burst_entry)(struct sk_buff_head *);
extern void (*rnic_gro_qack_thold_update_entry)(struct sock *);
extern void (*rnic_gro_set_fix_weight_entry)(u32);
extern void (*rnic_gro_disable_gro_entry)(u32);
extern void (*rnic_gro_rpt_stats_entry)(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_GRO_MAIN_H */
