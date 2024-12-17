/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111INVALID_INDEX307, USA
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

#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include <net/udp.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include "securec.h"
#include "rnic_gro.h"
#include "rnic_gro_hash.h"
#include "rnic_gro_checksum.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_gro_ip.h"
#include "rnic_gro_udp.h"

STATIC void rnic_gro_udp_rebuild_skb_flist(struct sk_buff *skb,
				     struct rnic_gro_udp_blk_s *blk)
{
	if (skb->ip_summed == CHECKSUM_UNNECESSARY) {
		if (skb->csum_level < SKB_MAX_CSUM_LEVEL)
			skb->csum_level++;
	} else {
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->csum_level = 0;
	}

	skb_shinfo(skb)->gso_size = blk->gso_size;
	skb_shinfo(skb)->gso_segs = blk->count;
	skb_shinfo(skb)->gso_type |= (SKB_GSO_FRAGLIST | SKB_GSO_UDP_L4);
}

STATIC void rnic_gro_udp_reskb_segment(struct sk_buff *skb,
				       struct udphdr *uh,
				       struct rnic_gro_udp_blk_s *blk)
{
	skb->csum_start = (unsigned char *)uh - skb->head;
	skb->csum_offset = offsetof(struct udphdr, check);
	skb->ip_summed = CHECKSUM_PARTIAL;

	skb_shinfo(skb)->gso_size = blk->gso_size;
	skb_shinfo(skb)->gso_segs = blk->count;
	skb_shinfo(skb)->gso_type |= SKB_GSO_UDP_L4;
}

STATIC void rnic_gro_udp4_reskb(struct sk_buff *skb,
				struct rnic_gro_udp_blk_s *blk)
{
	struct iphdr *iph = (struct iphdr *)blk->iph;
	struct udphdr *uh = blk->uh;
	__be16 newlen;
	u32 hdrlen;

	iph = blk->iph;
	newlen = htons(skb->len);
	csum_replace2(&iph->check, iph->tot_len, newlen);
	iph->tot_len = newlen;
	hdrlen = sizeof(*iph);

	uh->len = htons(skb->len - hdrlen);

	if (blk->is_flist) {
		rnic_gro_udp_rebuild_skb_flist(skb, blk);
		return;
	}

	if (uh->check)
		uh->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
			skb->len - sizeof(*iph), IPPROTO_UDP, 0);


	rnic_gro_udp_reskb_segment(skb, uh, blk);
	return;
}

STATIC void rnic_gro_udp6_reskb(struct sk_buff *skb,
				struct rnic_gro_udp_blk_s *blk)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)blk->iph;
	struct udphdr *uh = blk->uh;
	u32 hdrlen = sizeof(struct ipv6hdr);

	ip6h->payload_len = htons(skb->len - hdrlen);

	uh->len = htons(skb->len - hdrlen);

	if (blk->is_flist) {
		rnic_gro_udp_rebuild_skb_flist(skb, blk);
		return;
	}

	if (uh->check)
		uh->check = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
			skb->len - hdrlen, IPPROTO_UDP, 0);

	rnic_gro_udp_reskb_segment(skb, uh, blk);
	return;
}

STATIC struct sk_buff *rnic_gro_udp_rebuild_skb(
       struct rnic_gro_udp_flow_s *flow)
{
	struct rnic_gro_udp_blk_s *blk = &flow->blk;
	struct sk_buff *skb = blk->first;

	if (blk->count <= 1)
		return skb;

	if (skb->protocol == htons(ETH_P_IP))
		rnic_gro_udp4_reskb(skb, blk);
	else
		rnic_gro_udp6_reskb(skb, blk);

	return skb;
}

STATIC struct rnic_gro_udp_flow_s *rnic_gro_udp_flush_oldest(
       struct rnic_gro_udp_tbl_s *tbl, struct rnic_gro_list_s *list)
{
	struct rnic_gro_udp_flow_s *flow =
		list_last_entry(&list->head, struct rnic_gro_udp_flow_s, list);

	/*
	 * Do not adjust tbl->flow_hash[].count, caller is adding a new
	 * flow to the chain.
	 */
	__list_del_entry(&flow->list);
	tbl->deliver_skb(rnic_gro_udp_rebuild_skb(flow));
	tbl->stats.flush_flow++;

	return flow;
}

STATIC void rnic_gro_udp_flush_chain(struct rnic_gro_udp_tbl_s *tbl, u32 idx)
{
	struct list_head *head = &tbl->flow_hash[idx].head;
	struct list_head *pos = NULL;
	struct rnic_gro_udp_flow_s *flow = NULL;

	list_for_each_prev(pos, head) {
		flow = list_entry(pos, struct rnic_gro_udp_flow_s, list);
		tbl->deliver_skb(rnic_gro_udp_rebuild_skb(flow));
	}

	list_splice_tail_init(head, &tbl->free_list);
	tbl->flow_hash[idx].count = 0;
}

STATIC void rnic_gro_udp_flush_all(struct rnic_gro_udp_tbl_s *tbl)
{
	unsigned long bitmask = tbl->flow_bitmask;
	unsigned int base = ~0U;
	unsigned int i;

	while ((i = (unsigned int)ffs((int)bitmask)) != 0) {
		bitmask >>= i;
		base += i;
		rnic_gro_udp_flush_chain(tbl, base);
		__clear_bit((int)base, &tbl->flow_bitmask);
	}
}

STATIC bool rnic_gro_udp4hdr_valid(struct sk_buff *skb, struct udphdr *uh)
{
	u32 ulen;

	if (unlikely(skb->len < sizeof(struct iphdr) + sizeof(struct udphdr)))
		return false;

	ulen = ntohs(uh->len);
	if (unlikely(ulen <= sizeof(*uh) || ulen != skb->len - sizeof(struct iphdr)))
		return false;

	return true;
}

STATIC bool rnic_gro_udp4_dissect_skb(struct sk_buff *skb,
				      struct rnic_gro_udp_pkt_s *pkt)
{
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct rnic_gro_ip4_flow_keys_s *keys = &pkt->keys4;
	struct udphdr *uh = NULL;

	if (unlikely(!rnic_gro_ip4hdr_valid(skb, iph)))
		return false;

	if (ipv4_is_multicast(iph->daddr) || ipv4_is_lbcast(iph->daddr))
		return false;

	uh = (struct udphdr *)(iph + 1);
	if (unlikely(!rnic_gro_udp4hdr_valid(skb, uh)))
		return false;

	pkt->is_flist = true;
	pkt->netdev = skb->dev;

	keys->n_proto = ETH_P_IP;
	keys->ip_proto = iph->protocol;
	keys->saddr = iph->saddr;
	keys->daddr = iph->daddr;
	keys->ports = *(__be32 *)&uh->source;
	pkt->hash = rnic_gro_ip4_flow_hash(skb, keys);
	__skb_set_sw_hash(skb, pkt->hash, true);

	pkt->ulen = ntohs(uh->len);
	pkt->payload_offset = sizeof(*iph) + sizeof(*uh);
	pkt->payload_len = skb->len - sizeof(*iph) - sizeof(*uh);

	pkt->iph = iph;
	pkt->uh  = uh;
	pkt->skb = skb;

	return true;
}

STATIC bool rnic_gro_udp6_dissect_skb(struct sk_buff *skb,
				      struct rnic_gro_udp_pkt_s *pkt)
{
	struct ipv6hdr *iph = (struct ipv6hdr *)skb->data;
	struct rnic_gro_ip6_flow_keys_s *keys = &pkt->keys6;
	struct udphdr *uh = NULL;
	u32 payload_len = ntohs(iph->payload_len);
	u32 hash;

	if (!rnic_gro_ip6hdr_valid(skb, payload_len, sizeof(*uh)))
		return false;

	if (ipv6_addr_is_multicast(&iph->daddr))
		return false;

	pkt->is_flist = true;

	uh = (struct udphdr *)(iph + 1);
	pkt->ulen = ntohs(uh->len);
	if (unlikely(pkt->ulen < sizeof(*uh)))
		return false;

	pkt->skb = skb;
	pkt->netdev = skb->dev;
	pkt->payload_offset = sizeof(*iph) + sizeof(*uh);
	pkt->payload_len = payload_len - sizeof(*uh);

	keys->n_proto = ETH_P_IPV6;
	keys->first_word = *(__be32 *)iph & htonl(0xF00FFFFF);
	*(u16 *)&keys->nexthdr = *(u16 *)&iph->nexthdr;
	keys->saddr = iph->saddr;
	keys->daddr = iph->daddr;
	keys->ports = *(u32 *)&uh->source;

	hash = rnic_gro_ip6_flow_hash(skb, keys);
	__skb_set_sw_hash(skb, hash, true);
	pkt->hash = hash;
	pkt->uh = uh;
	pkt->iph = iph;

	return true;
}

STATIC bool rnic_gro_udp_dissect_skb(struct sk_buff *skb,
				     struct rnic_gro_udp_pkt_s *pkt)
{
	pkt->is_v4 = (skb->protocol == htons(ETH_P_IP)) ? true : false;

	return pkt->is_v4 ? rnic_gro_udp4_dissect_skb(skb, pkt) :
			    rnic_gro_udp6_dissect_skb(skb, pkt);
}

STATIC struct sock *rnic_gro_udp_lib_lookup(struct sk_buff *skb,
					    struct rnic_gro_udp_pkt_s *pkt)
{
	struct udphdr *uh = pkt->uh;

	return pkt->is_v4 ? udp4_lib_lookup_skb(skb, uh->source, uh->dest) :
			    udp6_lib_lookup_skb(skb, uh->source, uh->dest);
}

STATIC bool rnic_gro_udp_csum_valid(struct sk_buff *skb,
				    struct rnic_gro_udp_pkt_s *pkt)
{
	struct udphdr *uh = pkt->uh;
	if (!uh->check)
		return true;

	return pkt->is_v4 ?
	       rnic_gro_ip_csum_valid(skb, (struct iphdr *)pkt->iph,
		pkt->payload_len + sizeof(struct udphdr), IPPROTO_UDP) :
	       rnic_gro_ipv6_csum_valid(skb, (struct ipv6hdr *)pkt->iph,
		pkt->payload_len + sizeof(struct udphdr), IPPROTO_UDP);
}

STATIC bool rnic_gro_udp_is_sk_gro(struct sk_buff *skb,
				   struct rnic_gro_udp_pkt_s *pkt,
				   struct rnic_gro_udp_stats_s *stats)
{
	struct sock *sk = NULL;

	skb_set_network_header(skb, 0);

	rcu_read_lock();
	sk = rnic_gro_udp_lib_lookup(skb, pkt);

	if (sk) {
		if (udp_sk(sk)->gro_receive) {
			rcu_read_unlock();
			return true;
		}

		if (udp_sk(sk)->gro_enabled) {
			pkt->is_flist = false;
			stats->sk_gro_enable++;
		}
	}

	rcu_read_unlock();
	return false;
}

STATIC void rnic_gro_udp_add_flow(struct rnic_gro_udp_tbl_s *tbl,
				  struct rnic_gro_list_s *list,
				  struct rnic_gro_udp_pkt_s *pkt)
{
	struct rnic_gro_udp_flow_s *flow = NULL;
	struct rnic_gro_udp_blk_s *blk = NULL;

	if (list->count >= RNIC_GRO_MAX_FLOW_PER_BUCKET) {
		/* The flow struct will be reused below. */
		flow = rnic_gro_udp_flush_oldest(tbl, list);
		tbl->stats.bucket_full++;
		goto update_flow;
	} else {
		list->count++;
	}

	/*
	 * Since we limit max flow number in each bucket, we can
	 * make sure the free list won't be empty.
	 */
	flow = list_first_entry(&tbl->free_list, struct rnic_gro_udp_flow_s, list);
	__list_del_entry(&flow->list);

update_flow:
	flow->hash = pkt->hash;
	flow->keys6 = pkt->keys6;
	flow->netdev = pkt->netdev;
	flow->is_v4 = pkt->is_v4;
	list_add(&flow->list, &list->head);
	tbl->stats.new_flow++;

	blk = &flow->blk;
	blk->ulen = pkt->ulen;
	blk->gso_size = (u16)pkt->payload_len;
	blk->count = 1;
	blk->iph = pkt->iph;
	blk->uh = pkt->uh;
	blk->first = pkt->skb;
	blk->last = NULL;
	blk->is_flist = pkt->is_flist;
}

STATIC bool rnic_gro_udp_flows_same(struct rnic_gro_udp_flow_s *flow,
				    struct rnic_gro_udp_pkt_s *pkt)
{
	if (flow->netdev != pkt->netdev || flow->hash != pkt->hash ||
	    pkt->is_v4 ^ flow->is_v4)
		return false;

	return pkt->is_v4 ?
		rnic_gro_ip4_flows_same(&flow->keys4, &pkt->keys4) :
		rnic_gro_ip6_flows_same(&flow->keys6, &pkt->keys6);
}

STATIC struct rnic_gro_udp_flow_s *rnic_gro_udp_match_flow(
       struct rnic_gro_list_s *list, struct rnic_gro_udp_pkt_s *pkt)
{
	struct list_head *pos = NULL;
	struct rnic_gro_udp_flow_s *flow = NULL;

	/* Search for same flow */
	list_for_each(pos, &list->head) {
		flow = list_entry(pos, struct rnic_gro_udp_flow_s, list);
		if (rnic_gro_udp_flows_same(flow, pkt))
			return flow;
	}

	return NULL;
}

STATIC void rnic_gro_udp_update_flow_mask(struct rnic_gro_udp_tbl_s *tbl,
					  int idx)
{
	if (tbl->flow_hash[idx].count) {
		if (!test_bit(idx, &tbl->flow_bitmask))
			__set_bit(idx, &tbl->flow_bitmask);
	} else if (test_bit(idx, &tbl->flow_bitmask)) {
		__clear_bit(idx, &tbl->flow_bitmask);
	}
}

STATIC int rnic_gro_udp_merge(struct rnic_gro_udp_tbl_s *tbl,
			      struct rnic_gro_udp_blk_s *blk,
			      struct rnic_gro_udp_pkt_s *pkt)
{
	struct sk_buff *first = blk->first;
	struct sk_buff *last = blk->last;
	struct sk_buff *skb = pkt->skb;

	if (unlikely(first->len + pkt->payload_len >= U16_MAX))
		return -E2BIG;

	skb_set_network_header(skb, 0);
	__skb_pull(skb, pkt->payload_offset);

	if (!blk->is_flist)
		__skb_header_release(skb);

	first->len += pkt->payload_len;
	first->data_len += pkt->payload_len;
	first->truesize += skb->truesize;

	if (last != NULL)
		last->next = skb;
	else
		skb_shinfo(first)->frag_list = skb;

	blk->last = skb;
	blk->count++;
	pkt->merged = true;

	return 0;
}

STATIC struct rnic_gro_udp_flow_s *rnic_gro_udp_try_to_merge(
       struct rnic_gro_udp_tbl_s *tbl, struct rnic_gro_list_s *list,
       struct rnic_gro_udp_pkt_s *pkt)
{
	struct rnic_gro_udp_flow_s *flow = NULL;
	unsigned int ulen1 = pkt->ulen;
	unsigned int ulen2;
	u32 flush = 0;

	flow = rnic_gro_udp_match_flow(list, pkt);
	if (flow == NULL)
		goto out;

	if (pkt->is_v4)
		flush |= rnic_gro_cmp_ip4hdr(pkt->iph, flow->blk.iph);
	else
		flush |= rnic_gro_cmp_ip6hdr(pkt->iph, flow->blk.iph);

	ulen2 = flow->blk.ulen;
	if (ulen1 > ulen2) {
		flush = 1;
		goto out;
	}

	if (flush || (rnic_gro_udp_merge(tbl, &flow->blk, pkt) ||
		      ulen1 != ulen2 || flow->blk.count >= RNIC_GRO_UDP_MAX_WEIGHT))
		flush = 1;

out:
	return (flow != NULL && (flush || !pkt->merged)) ? flow : NULL;
}

STATIC void rnic_gro_udp_receive_internal(struct rnic_gro_udp_tbl_s *tbl,
					  struct rnic_gro_udp_pkt_s *pkt)
{
	u32 idx = pkt->hash & RNIC_GRO_HASH_MASK;
	struct rnic_gro_list_s *list = &tbl->flow_hash[idx];
	struct rnic_gro_udp_flow_s *flow = NULL;

	flow = rnic_gro_udp_try_to_merge(tbl, list, pkt);
	if (flow != NULL) {
		__list_del_entry(&flow->list);
		list->count--;

		tbl->deliver_skb(rnic_gro_udp_rebuild_skb(flow));
		tbl->stats.flush_flow++;

		list_add(&flow->list, &tbl->free_list);
	}

	if (pkt->merged) {
		tbl->stats.merged++;
		goto update_mask;
	}

	if (rnic_gro_udp_is_sk_gro(pkt->skb, pkt, &tbl->stats)) {
		tbl->stats.sk_gro_rcv++;
		tbl->deliver_skb(pkt->skb);
		goto update_mask;
	}

	rnic_gro_udp_add_flow(tbl, list, pkt);

update_mask:
	rnic_gro_udp_update_flow_mask(tbl, (int)idx);
}

STATIC void rnic_gro_udp_receive(void *table, struct sk_buff *skb)
{
	struct rnic_gro_udp_tbl_s *tbl = table;
	struct rnic_gro_udp_pkt_s info = {0};
	struct rnic_gro_udp_pkt_s *pkt = &info;

	if (unlikely(!rnic_gro_udp_dissect_skb(skb, pkt))) {
		tbl->stats.hdr_errs++;
		goto flush;
	}

	if (unlikely(!rnic_gro_udp_csum_valid(skb, pkt))) {
		tbl->stats.csum_errs++;
		goto flush;
	}

	rnic_gro_udp_receive_internal(tbl, pkt);
	return;

flush:
	tbl->deliver_skb(skb);
	tbl->stats.flush_single++;
}

STATIC void rnic_gro_udp_complete(void *table)
{
	struct rnic_gro_udp_tbl_s *tbl = table;
	rnic_gro_udp_flush_all(tbl);
	tbl->stats.flush_all++;
}

STATIC void rnic_gro_udp_show_stats(void *table)
{
	struct rnic_gro_udp_tbl_s *tbl = table;

	if (tbl == NULL)
		return;

	pr_err("[nas_rnic] [udp] hdr_errs      %u \n", tbl->stats.hdr_errs);
	pr_err("[nas_rnic] [udp] csum_errs     %u \n", tbl->stats.csum_errs);
	pr_err("[nas_rnic] [udp] sk_gro_rcv    %u \n", tbl->stats.sk_gro_rcv);
	pr_err("[nas_rnic] [udp] sk_gro_enable %u \n", tbl->stats.sk_gro_enable);

	pr_err("[nas_rnic] [udp] new_flow     %u \n", tbl->stats.new_flow);
	pr_err("[nas_rnic] [udp] merged       %u \n", tbl->stats.merged);
	pr_err("[nas_rnic] [udp] bucket_full  %u \n", tbl->stats.bucket_full);
	pr_err("[nas_rnic] [udp] flush_flow   %u \n", tbl->stats.flush_flow);
	pr_err("[nas_rnic] [udp] flush_single %u \n", tbl->stats.flush_single);
	pr_err("[nas_rnic] [udp] flush_all    %u \n", tbl->stats.flush_all);
}

STATIC void *rnic_gro_udp_init(rnic_gro_deliver_fn deliver_func)
{
	struct rnic_gro_udp_tbl_s *tbl = NULL;
	struct rnic_gro_udp_flow_s *flow = NULL;
	size_t size;
	u32 flow_num;
	u32 i;

	tbl = kzalloc(sizeof(*tbl), GFP_KERNEL);
	if (tbl == NULL) {
		RNIC_LOGE("alloc tbl failed");
		goto err_alloc_tbl;
	}

	for (i = 0; i < RNIC_GRO_HASH_SIZE; i++) {
		INIT_LIST_HEAD(&tbl->flow_hash[i].head);
		tbl->flow_hash[i].count = 0;
	}
	tbl->flow_bitmask = 0;

	(void)memset_s(&tbl->stats, sizeof(tbl->stats), 0 ,sizeof(tbl->stats));

	flow_num = RNIC_GRO_HASH_SIZE * RNIC_GRO_MAX_FLOW_PER_BUCKET;
	size = sizeof(struct rnic_gro_udp_flow_s) * flow_num;
	tbl->flows_buff = kzalloc(size, GFP_KERNEL);
	if (tbl->flows_buff == NULL) {
		RNIC_LOGE("alloc flows failed");
		goto err_alloc_flow;
	}

	INIT_LIST_HEAD(&tbl->free_list);
	for (i = 0; i < flow_num; i++) {
		flow = tbl->flows_buff + i;
		list_add_tail(&flow->list, &tbl->free_list);
	}

	tbl->deliver_skb = deliver_func;
	return tbl;

err_alloc_flow:
	kfree(tbl);
err_alloc_tbl:
	return NULL;
}

STATIC void rnic_gro_udp_deinit(void *table)
{
	struct rnic_gro_udp_tbl_s *tbl = table;

	if (tbl == NULL)
		return;

	/* Flush all flows holded. */
	rnic_gro_udp_flush_all(tbl);

	kfree(tbl->flows_buff);
	kfree(tbl);
}

const struct rnic_gro_ops_s rnic_gro_udp_ops = {
	.gro_init	= rnic_gro_udp_init,
	.gro_deinit	= rnic_gro_udp_deinit,
	.gro_receive	= rnic_gro_udp_receive,
	.gro_complete	= rnic_gro_udp_complete,
	.gro_show_stats	= rnic_gro_udp_show_stats,
};