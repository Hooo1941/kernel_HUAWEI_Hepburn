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

#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/socket.h>
#include <linux/module.h>
#include <linux/version.h>
#include "rnic_data.h"
#include "rnic_gro.h"
#include "rnic_gro_tcp.h"
#include "rnic_gro_udp.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_gro_main.h"
#include "rnic_log.h"

STATIC struct rnic_gro_ctx_s *rnic_gro_ctx_ptr;

STATIC const struct rnic_gro_ops_s *rnic_gro_proto_base[] __read_mostly = {
	[RNIC_GRO_TCP] = &rnic_gro_tcp_ops,
	[RNIC_GRO_UDP] = &rnic_gro_udp_ops,
};

STATIC struct rnic_gro_weight_s rnic_gro_weight_cfg[RNIC_GRO_WEIGHT_CFG_NUM] __read_mostly = {
	[0] = { .pps = 0,       .weight = 8 },
	[1] = { .pps = 26800,   .weight = 8 },
	[2] = { .pps = 40200,   .weight = 8 },
	[3] = { .pps = 53600,   .weight = 32 },
	[4] = { .pps = 73800,   .weight = 64 },
	[5] = { .pps = U32_MAX, .weight = 64 },
	[6] = { .pps = U32_MAX, .weight = 64 },
	[7] = { .pps = U32_MAX, .weight = 64 },
};

STATIC int rnic_gro_qack_thold_val __read_mostly = RNIC_GRO_DEF_QACK_THOLD;
STATIC u32 rnic_gro_fix_weight __read_mostly = 0;
STATIC u32 rnic_gro_pkt_segs_num[RNIC_GRO_MAX_PKT_SEGS + 1] = {0};

STATIC void rnic_gro_pkt_segs_stats(u16 segs_num)
{
	u16 num = segs_num != 0 ? segs_num : 1;

	if (likely(num <= RNIC_GRO_MAX_PKT_SEGS))
		rnic_gro_pkt_segs_num[num]++;
	else
		rnic_gro_pkt_segs_num[RNIC_GRO_MAX_PKT_SEGS]++;
}

STATIC void rnic_gro_deliver_skb(struct sk_buff *skb)
{
	rnic_gro_pkt_segs_stats(skb_shinfo(skb)->gso_segs);
	__skb_queue_tail(rnic_gro_ctx_ptr->rx_list, skb);
}

STATIC int rnic_gro_map_type(struct sk_buff *skb)
{
	switch(rnic_map_skb_cb(skb)->packet_info.bits.l4_proto) {
	case IPPROTO_TCP:
		return RNIC_GRO_TCP;
	case IPPROTO_UDP:
		return RNIC_GRO_UDP;
	default:
		break;
	}

	return RNIC_GRO_TYPE_MAX;
}

STATIC void rnic_gro_rcv_burst(struct sk_buff_head *head)
{
	struct rnic_gro_ctx_s *gctx = rnic_gro_ctx_ptr;
	const struct rnic_gro_ops_s **ops = rnic_gro_proto_base;
	struct sk_buff *skb = NULL;
	struct sk_buff_head sub_list;
	int gro_type;
	int i;

	gctx->rx_list = head;
	__skb_queue_head_init(&sub_list);
	skb_queue_splice_tail_init(head, &sub_list);

	while ((skb = __skb_dequeue(&sub_list)) != NULL) {
		gro_type = rnic_gro_map_type(skb);
		if (unlikely(!test_bit(gro_type, &gctx->gro_types))) {
			__skb_queue_tail(gctx->rx_list, skb);
			continue;
		}

		gctx->flow_stats.rx_pkts++;
		ops[gro_type]->gro_receive(gctx->gro_tables[gro_type], skb);
	}

	for (i = 0; i < RNIC_GRO_TYPE_MAX; i++)
		ops[i]->gro_complete(gctx->gro_tables[i]);
	gctx->rx_list = NULL;
}

STATIC void rnic_gro_qack_thold_update(struct sock *sk)
{
#ifdef CONFIG_TCP_QUICKACK_TWEAK
	sk_quickack_thold_update(sk, rnic_gro_qack_thold_val);
#endif
}

STATIC u32 rnic_gro_calc_weight(u64 pps)
{
	struct rnic_gro_weight_s *cfg = &rnic_gro_weight_cfg[0];
	u32 weight = RNIC_GRO_TCP_DEF_WEIGHT;
	u32 i;

	if (unlikely(rnic_gro_fix_weight != 0))
		return rnic_gro_fix_weight;

	for (i = 0; i < RNIC_GRO_WEIGHT_CFG_NUM; i++) {
		if (pps < (u64)cfg[i].pps)
			break;
		weight = cfg[i].weight;
	}

	return weight;
}

STATIC void rnic_gro_set_weight(struct rnic_gro_ctx_s *gctx, u64 pps)
{
	const struct rnic_gro_ops_s **ops = rnic_gro_proto_base;
	u32 dst_weight;
	u32 i;

	dst_weight = rnic_gro_calc_weight(pps);
	if (dst_weight == gctx->curr_weight)
		return;

	for (i = 0; i< RNIC_GRO_TYPE_MAX; i++) {
		if (ops[i]->gro_set_weight != NULL)
			ops[i]->gro_set_weight(gctx->gro_tables[i], dst_weight);
	}

	gctx->curr_weight = dst_weight;
}

STATIC void rnic_gro_set_fix_weight(u32 weight)
{
	if (weight > RNIC_GRO_TCP_MAX_WEIGHT)
		rnic_gro_fix_weight = RNIC_GRO_TCP_MAX_WEIGHT;
	else
		rnic_gro_fix_weight = weight;
}

STATIC void rnic_gro_get_report_stats(struct rnic_gro_stats_rpt_s *info)
{
	struct rnic_gro_tcp_tbl_s *ttbl = NULL;
	struct rnic_gro_udp_tbl_s *utbl = NULL;

	ttbl = (struct rnic_gro_tcp_tbl_s *)rnic_gro_ctx_ptr->gro_tables[RNIC_GRO_TCP];
	info->tcp_stats = ttbl->stats;

	utbl = (struct rnic_gro_udp_tbl_s *)rnic_gro_ctx_ptr->gro_tables[RNIC_GRO_UDP];
	info->udp_stats = utbl->stats;
}

STATIC void rnic_gro_rpt_stats(void)
{
	struct rnic_gro_stats_rpt_s info = {{0}};
	info.hdr.ver = 102; /* version 101 */
	rnic_gro_get_report_stats(&info);
	rnic_trans_report(ID_DIAG_RNIC_VNET_GRO_STATS, &info, sizeof(info));
}

STATIC void rnic_gro_disable_gro(u32 disable)
{
	if (disable != 0) {
		rcu_assign_pointer(rnic_gro_rcv_burst_entry, NULL);
		rcu_assign_pointer(rnic_gro_qack_thold_update_entry, NULL);
		rcu_assign_pointer(rnic_gro_rpt_stats_entry, NULL);
	} else {
		rcu_assign_pointer(rnic_gro_rcv_burst_entry, rnic_gro_rcv_burst);
		rcu_assign_pointer(rnic_gro_qack_thold_update_entry, rnic_gro_qack_thold_update);
		rcu_assign_pointer(rnic_gro_rpt_stats_entry, rnic_gro_rpt_stats);
	}
}

void rnic_gro_refresh_flow_stats(struct timer_list *t)
{
	struct rnic_gro_ctx_s *gctx = rnic_gro_ctx_ptr;
	u64 pps;

	pps = gctx->flow_stats.rx_pkts - gctx->flow_stats.last_rx_pkts;
	rnic_gro_set_weight(gctx, pps);

	gctx->flow_stats.last_rx_pkts = gctx->flow_stats.rx_pkts;
	mod_timer(&gctx->fs_timer, RNIC_GRO_FS_TIMEOUT);
}

STATIC int rnic_gro_init(struct rnic_gro_ctx_s *gctx)
{
	const struct rnic_gro_ops_s **ops = rnic_gro_proto_base;
	int i;

	for (i = 0; i < RNIC_GRO_TYPE_MAX; i++) {
		gctx->gro_tables[i] = ops[i]->gro_init(rnic_gro_deliver_skb);
		if (gctx->gro_tables[i] == NULL)
			return -ENOMEM;
		__set_bit(i, &gctx->gro_types);
	}

	return 0;
}

STATIC void rnic_gro_deinit(struct rnic_gro_ctx_s *gctx)
{
	const struct rnic_gro_ops_s **ops = rnic_gro_proto_base;
	int i;

	for (i = 0; i < RNIC_GRO_TYPE_MAX; i++) {
		if (!test_bit(i, &gctx->gro_types))
			continue;
		ops[i]->gro_deinit(gctx->gro_tables[i]);
		gctx->gro_tables[i] = NULL;
	}
}

int rnic_gro_module_init(void)
{
	struct rnic_gro_ctx_s *gctx = NULL;
	int ret;

	gctx = kzalloc(sizeof(*gctx), GFP_KERNEL);
	if (gctx == NULL) {
		RNIC_LOGE("alloc resource failed");
		return -ENOMEM;
	}

	timer_setup(&gctx->fs_timer, rnic_gro_refresh_flow_stats, 0);
	gctx->curr_weight = RNIC_GRO_TCP_DEF_WEIGHT;

	ret = rnic_gro_init(gctx);
	if (ret) {
		rnic_gro_deinit(gctx);
		kfree(gctx);
		return ret;
	}

	rnic_gro_ctx_ptr = gctx;
	RCU_INIT_POINTER(rnic_gro_rcv_burst_entry, rnic_gro_rcv_burst);
	RCU_INIT_POINTER(rnic_gro_qack_thold_update_entry, rnic_gro_qack_thold_update);
	RCU_INIT_POINTER(rnic_gro_set_fix_weight_entry, rnic_gro_set_fix_weight);
	RCU_INIT_POINTER(rnic_gro_disable_gro_entry, rnic_gro_disable_gro);
	RCU_INIT_POINTER(rnic_gro_rpt_stats_entry, rnic_gro_rpt_stats);
	mod_timer(&gctx->fs_timer, RNIC_GRO_FS_TIMEOUT);

	return 0;
}

void rnic_gro_module_exit(void)
{
	struct rnic_gro_ctx_s *gctx = rnic_gro_ctx_ptr;

	if (gctx == NULL)
		return;

	RCU_INIT_POINTER(rnic_gro_rcv_burst_entry, NULL);
	RCU_INIT_POINTER(rnic_gro_qack_thold_update_entry, NULL);
	RCU_INIT_POINTER(rnic_gro_set_fix_weight_entry, NULL);
	RCU_INIT_POINTER(rnic_gro_disable_gro_entry, NULL);
	RCU_INIT_POINTER(rnic_gro_rpt_stats_entry, NULL);
	rnic_gro_deinit(gctx);

	kfree(gctx);
	rnic_gro_ctx_ptr = NULL;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(rnic_gro_module_init);
module_exit(rnic_gro_module_exit);
#endif

void rnic_gro_dbg_set_qack_thold_val(int val)
{
	rnic_gro_qack_thold_val = val;
	RNIC_LOGH("qack_thold_val is %d", rnic_gro_qack_thold_val);
}

void rnic_gro_dbg_set_pps_weight(u32 index, u32 pps, u32 weight)
{
	struct rnic_gro_weight_s *weight_cfg = NULL;

	if (index >= RNIC_GRO_WEIGHT_CFG_NUM) {
		RNIC_LOGE("index %d is invalid\n", index);
		return;
	}

	if (weight > RNIC_GRO_TCP_MAX_WEIGHT) {
		RNIC_LOGE("weight %d is invalid\n", weight);
		return;
	}

	weight_cfg = &rnic_gro_weight_cfg[index];
	weight_cfg->pps = pps;
	weight_cfg->weight = weight;
}

void rnic_gro_show_weight_config(void)
{
	u32 i;

	for (i = 0; i < RNIC_GRO_WEIGHT_CFG_NUM; i++)
		pr_err("[nas_rnic] gro weight_cfg[%d] %d %d \n", i,
		       rnic_gro_weight_cfg[i].pps, rnic_gro_weight_cfg[i].weight);
}

void rnic_gro_dbg_set_blk_per_flow(u32 num)
{
	struct rnic_gro_ctx_s *gctx = rnic_gro_ctx_ptr;
	struct rnic_gro_tcp_tbl_s *tbl = gctx->gro_tables[RNIC_GRO_TCP];

	tbl->blk_cap = num;
}

void rnic_gro_show_segs_stats(void)
{
	u32 i;

	for (i = 1; i <= RNIC_GRO_MAX_PKT_SEGS; i++)
		pr_err("[nas_rnic] gro segs_num[%d] %u \n", i, rnic_gro_pkt_segs_num[i]);
}

void rnic_gro_show_proto_stats(u32 gro_type)
{
	struct rnic_gro_ctx_s *gctx = rnic_gro_ctx_ptr;
	const struct rnic_gro_ops_s **ops = rnic_gro_proto_base;

	if (gctx == NULL) {
		RNIC_LOGE("gro context not exist\n");
		return;
	}

	if (gro_type >= RNIC_GRO_TYPE_MAX) {
		RNIC_LOGE("gro type %d is invalid\n", gro_type);
		return;
	}

	pr_err("[nas_rnic] [tcp] dyn_weight %d\n", gctx->curr_weight);
	pr_err("[nas_rnic] [tcp] fix_weight %d\n", rnic_gro_fix_weight);

	if (ops[gro_type]->gro_show_stats != NULL)
		ops[gro_type]->gro_show_stats(gctx->gro_tables[gro_type]);
}
