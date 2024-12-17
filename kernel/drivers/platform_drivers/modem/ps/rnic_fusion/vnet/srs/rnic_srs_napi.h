/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef RNIC_SRS_NAPI_H
#define RNIC_SRS_NAPI_H

#include <linux/cache.h>
#include <linux/cpuhotplug.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include "rnic_srs.h"
#include "rnic_dsm_msg_pif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SRS_NAPI_QLEN_CFG_UNIT	1000
#define SRS_NAPI_WEIGHT		64
#define SRS_MAX_PIF_NUM		RNIC_MAX_PIF_NUM
#define SRS_PIFID_OFFSET	RNIC_PIFID_OFFSET
#define SRS_PIF_MASK		RNIC_VNET_PIF_MASK
#define SRS_TCP_GRO_MASK	0x00000001
#define SRS_UDP_GRO_MASK	0x00000002

struct rnic_srs_napi_cfg_s {
	u32 napi_enable;
	u32 gro_enable;
	u32 weight_adj;
	u32 weight_level[SRS_PPS_LEVEL];
};

struct rnic_srs_napi_stats_s {
	u64 rx_pend_pkts;
	u64 rx_queue_errs;
	u32 poll_num;
	u32 poll_acc_num;
	u32 napi_cmpl_num;
};

struct rnic_srs_napi_priv_s {
	struct net_device *netdev;
	struct napi_struct napi ____cacheline_aligned;
	struct work_struct napi_work;
	struct sk_buff_head input_queue;
	struct sk_buff_head napi_queue;
	struct sk_buff_head process_queue;
	struct sk_buff_head rps_pend_queue;
	struct rnic_srs_napi_stats_s napi_stats;
	u64 pif_bitmask;
	u32 pif_id;
	u32 vif_id;

	u32 napi_enable;
	u32 gro_enable;
	u32 napi_weight;
	u32 que_len_max;
	u32 poll_acc_enable;
	u32 poll_num;
	u32 poll_max_num;
	atomic_t napi_cpu;
};

struct rnic_srs_napi_ctx_s {
	struct rnic_srs_ctx_s *back;
	struct rnic_srs_napi_cfg_s cfg;
	struct rnic_srs_napi_priv_s *priv[SRS_MAX_PIF_NUM];

	union {
		u64 rx_cmplt_bitmask;
		unsigned long rx_cmplt_mask[BITS_TO_LONGS(SRS_MAX_PIF_NUM)];
	};
};

static inline int rnic_srs_get_napi_weight(const struct rnic_srs_napi_priv_s *npriv)
{
	return npriv->napi.weight;
}

static inline void rnic_srs_set_napi_weight(struct rnic_srs_napi_priv_s *npriv,
					    int weight)
{
	npriv->napi.weight = weight;
}

static inline bool rnic_srs_napi_is_enabled(struct rnic_srs_napi_priv_s *npriv)
{
	return !test_bit(NAPI_STATE_SCHED, &npriv->napi.state) &&
	       !test_bit(NAPI_STATE_NPSVC, &npriv->napi.state);
}

static inline bool rnic_srs_napi_is_scheduled(struct rnic_srs_napi_priv_s *npriv)
{
	return npriv->napi.state & NAPIF_STATE_SCHED;
}

struct rnic_srs_napi_ctx_s *rnic_srs_get_nctx(void);
struct rnic_srs_napi_priv_s *rnic_srs_get_npriv(struct net_device *netdev);

bool rnic_srs_reset_napi_weight(struct rnic_srs_napi_priv_s *npriv);
void rnic_srs_set_napi_cfg(const struct rnic_vnet_fea_cfg_s *cfg);
void rnic_srs_set_napi_weight_level(struct rnic_srs_ctx_s *sctx, u32 level);

void rnic_srs_rx_skb_list(struct sk_buff_head *head);
void rnic_srs_disable_gro(u32 disable);

void rnic_srs_napi_enable(struct net_device *netdev);
void rnic_srs_napi_disable(struct net_device *netdev);
void rnic_srs_napi_add(struct net_device *netdev);
void rnic_srs_napi_del(struct net_device *netdev);
int rnic_srs_napi_init(struct rnic_srs_ctx_s *sctx);
void rnic_srs_napi_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_SRS_NAPI_H */
