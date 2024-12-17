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

#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/spinlock.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/etherdevice.h>
#include <net/ipv6.h>
#include "securec.h"
#include "mdrv_pfa.h"
#include "rnic_srs_cc.h"
#include "rnic_srs_lb.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_srs_napi.h"

STATIC struct rnic_srs_napi_ctx_s rnic_srs_napi_ctx;

struct rnic_srs_napi_ctx_s *rnic_srs_get_nctx(void)
{
	return &rnic_srs_napi_ctx;
}

struct rnic_srs_napi_priv_s *rnic_srs_get_npriv(struct net_device *netdev)
{
	u32 pif_id = netdev->dev_port;

	return pif_id >= SRS_MAX_PIF_NUM ? NULL : rnic_srs_napi_ctx.priv[pif_id];
}

bool rnic_srs_reset_napi_weight(struct rnic_srs_napi_priv_s *npriv)
{
	if (npriv != NULL && npriv->napi.weight != (int)npriv->napi_weight) {
		npriv->napi.weight = (int)npriv->napi_weight;
		return true;
	}

	return false;
}

void rnic_srs_set_napi_cfg(const struct rnic_vnet_fea_cfg_s *cfg)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;
	struct rnic_srs_napi_priv_s *npriv = NULL;
	struct rnic_srs_napi_cfg_s *napi_cfg = &nctx->cfg;
	u32 i;

	napi_cfg->napi_enable = cfg->napi_enable;
	napi_cfg->gro_enable = cfg->gro_enable;
	napi_cfg->weight_adj = cfg->napi_weight_adj;

	for (i = 0; i < SRS_PPS_LEVEL; i++)
		napi_cfg->weight_level[i] = cfg->level_cfg[i].napi_weight;

	for (i = 0; i < SRS_MAX_PIF_NUM; i++) {
		npriv = nctx->priv[i];
		if (npriv != NULL) {
			npriv->napi_enable = cfg->napi_enable;
			npriv->gro_enable = cfg->gro_enable;
			if (cfg->napi_weight_adj)
				npriv->napi_weight = napi_cfg->weight_level[0];
			else
				npriv->napi_weight = cfg->napi_weight;
			npriv->que_len_max = cfg->napi_que_len_max * SRS_NAPI_QLEN_CFG_UNIT;
			npriv->poll_acc_enable = cfg->napi_poll_acc_enable;
			npriv->poll_max_num = cfg->napi_poll_max;
		}
	}
}

void rnic_srs_set_napi_weight_level(struct rnic_srs_ctx_s *sctx, u32 level)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;
	struct rnic_srs_napi_priv_s *npriv = NULL;
	struct rnic_srs_rx_queue_s *rxq_node = NULL;
	struct list_head *tmp = NULL;
	u32 weight = nctx->cfg.weight_level[level];

	if (nctx->cfg.weight_adj == 0)
		return;

	mutex_lock(&sctx->mutex);
	list_for_each(tmp, sctx->rxq_list) {
		rxq_node = list_entry(tmp, struct rnic_srs_rx_queue_s, list);
		npriv = rxq_node->npriv;
		if (npriv != NULL) {
			npriv->napi.weight = (int)weight;
			npriv->napi_weight = weight;
		}
	}
	mutex_unlock(&sctx->mutex);
}

STATIC void rnic_srs_set_gro_cfg(u32 enable)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;
	struct rnic_srs_napi_priv_s *npriv = NULL;
	int i;

	for (i = 0; i < SRS_MAX_PIF_NUM; i++) {
		npriv = nctx->priv[i];
		if (npriv == NULL)
			continue;

		if (npriv->gro_enable != enable)
			npriv->gro_enable = enable;
	}
}

void rnic_srs_disable_gro(u32 disable)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;

	if (disable != 0)
		rnic_srs_set_gro_cfg(0);
	else
		rnic_srs_set_gro_cfg(nctx->cfg.gro_enable);
}

STATIC bool rnic_srs_check_mcast(struct sk_buff *skb)
{
	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = ip_hdr(skb);
		if (ipv4_is_multicast(iph->daddr) || ipv4_is_lbcast(iph->daddr))
			return true;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		if (ipv6_addr_is_multicast(&ip6h->daddr))
			return true;
	}

	return false;
}

STATIC bool rnic_srs_check_skb_gro(struct rnic_srs_napi_priv_s *npriv, struct sk_buff *skb)
{
	u32 l4_proto = rnic_map_skb_cb(skb)->packet_info.bits.l4_proto;

	if (l4_proto == IPPROTO_TCP) {
		if (!!(npriv->gro_enable & SRS_TCP_GRO_MASK))
			return true;
	} else if (l4_proto == IPPROTO_UDP) {
		if (!!(npriv->gro_enable & SRS_UDP_GRO_MASK) &&
		    !rnic_srs_check_mcast(skb))
			return true;
	}

	return false;
}

STATIC void rnic_srs_set_ethhdr(struct rnic_srs_napi_priv_s *npriv, struct sk_buff *skb)
{
	if (!skb_mac_header_was_set(skb)) {
		struct ethhdr *ethh = (struct ethhdr *)(skb->data - ETH_HLEN);
		struct net_device *netdev = npriv->netdev;

		ether_addr_copy(ethh->h_dest, netdev->dev_addr);
		ether_addr_copy(ethh->h_source, netdev->dev_addr);
		ethh->h_proto = skb->protocol;

		skb_set_mac_header(skb, -ETH_HLEN);
		skb_set_network_header(skb, 0);
		skb->pkt_type = PACKET_HOST;
	}
}

STATIC bool rnic_srs_napi_deliver_skb(struct rnic_srs_napi_priv_s *npriv,
				      struct sk_buff *skb)
{
	rnic_srs_set_ethhdr(npriv, skb);
	if (rnic_srs_check_skb_gro(npriv, skb)) {
		napi_gro_receive(&npriv->napi, skb);
	} else {
		if (rnic_map_skb_cb(skb)->packet_info.bits.l4_proto == IPPROTO_UDP &&
		    rnic_srs_rx_cqcc(npriv->netdev, skb)) {
			__skb_queue_tail(&npriv->rps_pend_queue, skb);
			return false;
		} else {
			netif_receive_skb(skb);
		}
	}

	return true;
}

STATIC int rnic_srs_napi_rx_skb(struct rnic_srs_napi_priv_s *npriv, int budget)
{
	struct sk_buff *skb = NULL;
	int rx_packets = 0;
	bool again = true;

	while (again) {
		while ((skb = __skb_dequeue(&npriv->process_queue)) != NULL) {
			if (rnic_srs_napi_deliver_skb(npriv, skb) &&
			    ++rx_packets >= budget)
				goto rx_completed;
		}

		spin_lock_bh(&npriv->napi_queue.lock);
		if (skb_queue_empty(&npriv->napi_queue))
			again = false;
		else
			skb_queue_splice_tail_init(&npriv->napi_queue, &npriv->process_queue);
		spin_unlock_bh(&npriv->napi_queue.lock);
	}

rx_completed:
	if (!skb_queue_empty(&npriv->rps_pend_queue)) {
		npriv->napi_stats.rx_pend_pkts += skb_queue_len(&npriv->rps_pend_queue);
		skb_queue_splice_tail_init(&npriv->process_queue, &npriv->rps_pend_queue);
		spin_lock_bh(&npriv->napi_queue.lock);
		skb_queue_splice_init(&npriv->rps_pend_queue, &npriv->napi_queue);
		spin_unlock_bh(&npriv->napi_queue.lock);
	}

	return rx_packets;
}

STATIC int rnic_srs_napi_poll(struct napi_struct *napi, int budget)
{
	struct rnic_srs_napi_priv_s *npriv =
		container_of(napi, struct rnic_srs_napi_priv_s, napi);
	int work;

	npriv->napi_stats.poll_num++;
	if (unlikely(budget <= 0))
		return budget;

	work = rnic_srs_napi_rx_skb(npriv, budget);
	if ((npriv->poll_acc_enable &&
	     ++npriv->poll_num > npriv->poll_max_num) && work >= budget) {
		work = budget - 1;
		npriv->napi_stats.poll_acc_num++;
	}

	if (work < budget) {
		napi_complete(&npriv->napi);
		npriv->napi_stats.napi_cmpl_num++;
		npriv->poll_num = 0;

		if (skb_queue_empty(&npriv->input_queue) &&
		    (!skb_queue_empty(&npriv->napi_queue) ||
		     !skb_queue_empty(&npriv->process_queue))) {
			atomic_set(&npriv->napi_cpu, rnic_srs_rx_tcpu());
			schedule_work(&npriv->napi_work);
		}
	}

	rnic_srs_cqcc_clear_pend();
	return work;
}

STATIC void rnic_srs_napi_schedule(void *info)
{
	struct rnic_srs_napi_priv_s *npriv = info;

	if (napi_schedule_prep(&npriv->napi))
		__napi_schedule(&npriv->napi);
}

STATIC void rnic_srs_napi_schedule_wq(struct rnic_srs_napi_priv_s *npriv)
{
	local_bh_disable();
	rnic_srs_napi_schedule(npriv);
	local_bh_enable();
}

STATIC void rnic_srs_napi_work(struct work_struct *work)
{
	struct rnic_srs_napi_priv_s *npriv = container_of(work,
							struct rnic_srs_napi_priv_s,
							napi_work);
	int napi_cpu = atomic_read(&npriv->napi_cpu);
	int cur_cpu = (int)smp_processor_id();

	get_online_cpus();
	if (!cpu_online(napi_cpu) || (cur_cpu == napi_cpu)) {
		rnic_srs_napi_schedule_wq(npriv);
	} else if (!rnic_srs_napi_is_scheduled(npriv)) {
		smp_call_function_single(napi_cpu, rnic_srs_napi_schedule, npriv, 0);
	}
	put_online_cpus();
}

STATIC void rnic_srs_napi_rx_enqueue(struct rnic_srs_napi_priv_s *npriv)
{
	if (skb_queue_len(&npriv->napi_queue) + skb_queue_len(&npriv->process_queue) +
	    skb_queue_len(&npriv->rps_pend_queue) < npriv->que_len_max) {
		spin_lock_bh(&npriv->napi_queue.lock);
		skb_queue_splice_tail_init(&npriv->input_queue, &npriv->napi_queue);
		spin_unlock_bh(&npriv->napi_queue.lock);
	} else {
		npriv->napi_stats.rx_queue_errs += skb_queue_len(&npriv->input_queue);
		skb_queue_purge(&npriv->input_queue);
	}
}

STATIC void rnic_srs_rx_cmplt(struct rnic_srs_napi_ctx_s *nctx)
{
	struct rnic_srs_napi_priv_s *npriv = NULL;
	u64 bitmask;
	unsigned int bit;
	unsigned int base = ~0U;
	u32 cur_len;

	bitmask = nctx->rx_cmplt_bitmask;
	while (bitmask) {
		bit = (unsigned int)__ffs64(bitmask) + 1;
		bitmask >>= bit;
		base += bit;

		npriv = nctx->priv[base];
		rnic_srs_napi_rx_enqueue(npriv);

		cur_len = skb_queue_len(&npriv->input_queue) +
			  skb_queue_len(&npriv->napi_queue) +
			  skb_queue_len(&npriv->process_queue) +
			  skb_queue_len(&npriv->rps_pend_queue);
		rnic_srs_rx_nqcc(npriv, cur_len, npriv->que_len_max);

		schedule_work(&npriv->napi_work);
		atomic_set(&npriv->napi_cpu, rnic_srs_rx_tcpu());
	}
	nctx->rx_cmplt_bitmask = 0;
}

void rnic_srs_rx_skb_list(struct sk_buff_head *head)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;
	struct rnic_srs_flow_stats_s *fs = &nctx->back->flow_stats;
	struct rnic_srs_napi_priv_s *npriv = NULL;
	struct sk_buff *skb = NULL;
	struct sk_buff *tmp = NULL;
	u32 pkt_num;

	if (unlikely(nctx->cfg.napi_enable == 0))
		return;

	skb_queue_walk_safe(head, skb, tmp) {
		npriv = nctx->priv[rnic_map_pifid(skb)];
		if (unlikely(npriv == NULL))
			continue;

		pkt_num = skb_shinfo(skb)->gso_segs ? skb_shinfo(skb)->gso_segs : 1;
		fs->rx_packets += pkt_num;
		rnic_map_skb_cb(skb)->packet_info.bits.l4_proto == IPPROTO_TCP ?
			fs->rx_tcp_pkts += pkt_num : (fs->rx_non_tcp_pkts += pkt_num);

		__skb_unlink(skb, head);
		__skb_queue_tail(&npriv->input_queue, skb);
		nctx->rx_cmplt_bitmask |= npriv->pif_bitmask;
	}

	if (nctx->rx_cmplt_bitmask != 0)
		rnic_srs_rx_cmplt(nctx);
}

void rnic_srs_napi_enable(struct net_device *netdev)
{
	struct rnic_srs_napi_priv_s *npriv = rnic_srs_get_npriv(netdev);

	if (npriv != NULL && npriv->napi_enable) {
		npriv->napi.weight = (int)npriv->napi_weight;
		napi_enable(&npriv->napi);
	}
}

void rnic_srs_napi_disable(struct net_device *netdev)
{
	struct rnic_srs_napi_priv_s *npriv = rnic_srs_get_npriv(netdev);

	if (npriv != NULL && npriv->napi_enable) {
		napi_disable(&npriv->napi);
		skb_queue_purge(&npriv->napi_queue);
	}
}

void rnic_srs_napi_add(struct net_device *netdev)
{
	struct rnic_srs_napi_priv_s *npriv = rnic_srs_get_npriv(netdev);

	if (npriv != NULL) {
		npriv->netdev = netdev;
		netif_napi_add(netdev, &npriv->napi, rnic_srs_napi_poll, SRS_NAPI_WEIGHT);
	}
}

void rnic_srs_napi_del(struct net_device *netdev)
{
	struct rnic_srs_napi_priv_s *npriv = rnic_srs_get_npriv(netdev);

	if (npriv != NULL)
		netif_napi_del(&npriv->napi);
}

int rnic_srs_napi_init(struct rnic_srs_ctx_s *sctx)
{
	struct rnic_srs_napi_priv_s *npriv = NULL;
	unsigned long long pifmask = SRS_PIF_MASK;
	const unsigned long *bitmasks = (const unsigned long *)&pifmask;
	unsigned long bit;
	u32 idx = 0;

	for_each_set_bit(bit, bitmasks, (unsigned long)SRS_MAX_PIF_NUM) {
		npriv = kzalloc(sizeof(*npriv), GFP_KERNEL);
		if (npriv == NULL) {
			RNIC_LOGE("alloc napi ctx failed");
			goto err_alloc_mem;
		}

		atomic_set(&npriv->napi_cpu, 0);
		INIT_WORK(&npriv->napi_work, rnic_srs_napi_work);
		skb_queue_head_init(&npriv->input_queue);
		skb_queue_head_init(&npriv->napi_queue);
		skb_queue_head_init(&npriv->process_queue);
		skb_queue_head_init(&npriv->rps_pend_queue);
		npriv->pif_bitmask = BIT_ULL(bit);
		npriv->pif_id = (u32)bit;
		npriv->vif_id = idx++;

		rnic_srs_napi_ctx.priv[bit] = npriv;
	}

	rnic_srs_napi_ctx.back = sctx;
	return 0;

err_alloc_mem:
	rnic_srs_napi_exit();
	return -ENOMEM;
}

void rnic_srs_napi_exit(void)
{
	struct rnic_srs_napi_ctx_s *nctx = &rnic_srs_napi_ctx;
	struct rnic_srs_napi_priv_s *npriv = NULL;
	int i;

	for (i = 0; i < SRS_MAX_PIF_NUM; i++) {
		npriv = nctx->priv[i];
		if (npriv == NULL)
			continue;

		if (npriv->napi_work.func != NULL)
			cancel_work_sync(&npriv->napi_work);

		kfree(npriv);
		nctx->priv[i] = NULL;
	}
}
