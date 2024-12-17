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
#include <linux/if_ether.h>
#include <linux/in.h>
#include "rnic_gro.h"
#include "rnic_gro_hash.h"
#include "rnic_gro_checksum.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_gro_ip.h"
#include "rnic_gro_tcp.h"
#include "rnic_dump.h"

STATIC void rnic_gro_tcp4_fill_check(struct sk_buff *skb,
	struct tcphdr *th, void *iph, u32 payload_len)
{
	struct iphdr *ip4h = (struct iphdr *)iph;
	u32 iphdr_len = sizeof(*ip4h);

	/* tcp header length unit is 4 bytes (32bit) */
	u32 add_len = payload_len + iphdr_len + th->doff * 4 - skb->len;
	__be16 newlen;

	skb->len += add_len;
	skb->data_len += add_len;

	newlen = htons(skb->len);
	csum_replace2(&ip4h->check, ip4h->tot_len, newlen);
	ip4h->tot_len = newlen;

	th->check = ~csum_tcpudp_magic(ip4h->saddr, ip4h->daddr,
		skb->len - iphdr_len, IPPROTO_TCP, 0);
}

STATIC void rnic_gro_tcp6_fill_check(struct sk_buff *skb,
	struct tcphdr *th, void *iph, u32 payload_len)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)iph;
	u32 iphdr_len = sizeof(*ip6h);

	/* tcp header length unit is 4 bytes (32bit) */
	u32 add_len = payload_len + iphdr_len + th->doff * 4 - skb->len;

	skb->len += add_len;
	skb->data_len += add_len;

	ip6h->payload_len = htons(skb->len - iphdr_len);

	th->check = ~tcp_v6_check((int)(skb->len - iphdr_len),
		&ip6h->saddr, &ip6h->daddr, 0);
}

STATIC bool rnic_gro_tcphdr_valid(struct sk_buff *skb, struct tcphdr *th)
{
	if (unlikely(skb->len < sizeof(struct iphdr) + sizeof(struct tcphdr)))
		return false;

	/* tcp header length unit is 4 bytes (32bit) */
	if (unlikely(th->doff < sizeof(struct tcphdr) / 4))
		return false;

	/* tcp header length unit is 4 bytes (32bit) */
	if (unlikely(skb->len < sizeof(struct iphdr) + th->doff * 4))
		return false;

	return true;
}

STATIC bool rnic_gro_tcp4_dissect_skb(struct sk_buff *skb,
				       struct rnic_gro_tcp_pkt_s *pkt)
{
	struct iphdr *iph = (struct iphdr *)skb->data;
	struct tcphdr *th = NULL;
	struct rnic_gro_ip4_flow_keys_s *keys = &pkt->keys4;

	if (unlikely(!rnic_gro_ip4hdr_valid(skb, iph)))
		return false;

	th = (struct tcphdr *)(iph + 1);
	if (unlikely(!rnic_gro_tcphdr_valid(skb, th)))
		return false;

	pkt->netdev = skb->dev;

	keys->n_proto = ETH_P_IP;
	keys->ip_proto = iph->protocol;
	keys->saddr = iph->saddr;
	keys->daddr = iph->daddr;
	keys->ports = *(u32 *)&th->source;
	pkt->hash = rnic_gro_ip4_flow_hash(skb, keys);
	__skb_set_sw_hash(skb, pkt->hash, true);

	/* tcp header length unit is 4 bytes (32bit) */
	pkt->thlen = th->doff * 4;
	pkt->payload_offset = sizeof(struct iphdr) + pkt->thlen;
	pkt->payload_len = skb->len - sizeof(struct iphdr) - pkt->thlen;
	pkt->seq = ntohl(th->seq);
	pkt->next_seq = ntohl(th->seq) + pkt->payload_len;

	pkt->iph = iph;
	pkt->th  = th;
	pkt->skb = skb;

	return true;
}

STATIC bool rnic_gro_tcp6_dissect_skb(struct sk_buff *skb,
				       struct rnic_gro_tcp_pkt_s *pkt)
{
	struct ipv6hdr *iph = (struct ipv6hdr *)skb->data;
	struct tcphdr *th = NULL;
	struct rnic_gro_ip6_flow_keys_s *keys = &pkt->keys6;
	u32 payload_len = ntohs(iph->payload_len);
	u32 hash;

	if (!rnic_gro_ip6hdr_valid(skb, payload_len, sizeof(*th)))
		return false;

	th = (struct tcphdr *)(iph + 1);
	pkt->thlen = __tcp_hdrlen(th);
	if (unlikely(pkt->thlen < sizeof(*th)))
		return false;

	pkt->skb = skb;
	pkt->netdev = skb->dev;
	pkt->iph = iph;
	pkt->th = th;
	pkt->payload_offset = sizeof(*iph) + pkt->thlen;
	pkt->payload_len = payload_len - pkt->thlen;
	pkt->seq = ntohl(th->seq);
	pkt->next_seq = pkt->seq + pkt->payload_len;

	keys->n_proto = ETH_P_IPV6;
	keys->first_word = *(__be32 *)iph & htonl(0xF00FFFFF);
	*(u16 *)&keys->nexthdr = *(u16 *)&iph->nexthdr;
	keys->saddr = iph->saddr;
	keys->daddr = iph->daddr;
	keys->ports = *(u32 *)&th->source;

	hash = rnic_gro_ip6_flow_hash(skb, keys);
	__skb_set_sw_hash(skb, hash, true);
	pkt->hash = hash;

	return true;
}

STATIC struct sk_buff *rnic_gro_tcp_rebuild_skb(
	struct rnic_gro_tcp_blks *blk)
{
	struct sk_buff *skb = blk->first;
	struct sk_buff *next_skb = skb->next;
	struct tcphdr *th = NULL;

	if (blk->count <= 1)
		return skb;

	skb_shinfo(skb)->frag_list = next_skb;
	skb->next = NULL;

	th = blk->th;

	if (skb->protocol == htons(ETH_P_IP))
		rnic_gro_tcp4_fill_check(skb, th, blk->iph, blk->payload_len);
	else
		rnic_gro_tcp6_fill_check(skb, th, blk->iph, blk->payload_len);

	tcp_flag_word(th) |= blk->flags;

	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->csum_start = (__u16)((unsigned char *)th - skb->head);
	skb->csum_offset = offsetof(struct tcphdr, check);
	skb->truesize = blk->truesize;

	skb_shinfo(skb)->gso_size = blk->gso_size;
	skb_shinfo(skb)->gso_segs = blk->count;
	skb_shinfo(skb)->gso_type = (skb->protocol == htons(ETH_P_IP)) ?
		SKB_GSO_TCPV4 : SKB_GSO_TCPV6;

	return skb;
}

STATIC void rnic_gro_tcp_deliver_flow(struct rnic_gro_tcp_tbl_s *tbl,
				      struct rnic_gro_tcp_flow_s *flow)
{
	struct list_head *pos = NULL;
	struct rnic_gro_tcp_blks *blk = NULL;

	list_for_each_prev(pos, &flow->blk_head) {
		blk = list_entry(pos, struct rnic_gro_tcp_blks, list);
		tbl->deliver_skb(rnic_gro_tcp_rebuild_skb(blk));
		tbl->blk_remain++;
	}
	flow->blk_count = 0;
	list_splice_tail_init(&flow->blk_head, &tbl->blk_free_list);
}

STATIC struct rnic_gro_tcp_flow_s *rnic_gro_tcp_flush_oldest(
	struct rnic_gro_tcp_tbl_s *tbl, struct rnic_gro_list_s *list)
{
	struct rnic_gro_tcp_flow_s *flow =
		list_last_entry(&list->head, struct rnic_gro_tcp_flow_s, list);

	/*
	 * Do not adjust tbl->flow_hash[].count, caller is adding a new
	 * flow to the chain.
	 */
	__list_del_entry(&flow->list);
	rnic_gro_tcp_deliver_flow(tbl, flow);

	return flow;
}

STATIC void rnic_gro_tcp_flush_chain(struct rnic_gro_tcp_tbl_s *tbl, u32 idx)
{
	struct list_head *head = &tbl->flow_hash[idx].head;
	struct list_head *pos = NULL;
	struct rnic_gro_tcp_flow_s *flow = NULL;

	list_for_each_prev(pos, head) {
		flow = list_entry(pos, struct rnic_gro_tcp_flow_s, list);
		rnic_gro_tcp_deliver_flow(tbl, flow);
	}

	list_splice_tail_init(head, &tbl->flow_free_list);
	tbl->flow_hash[idx].count = 0;
}

STATIC void rnic_gro_tcp_flush_all(struct rnic_gro_tcp_tbl_s *tbl)
{
	unsigned long bitmask = tbl->flow_bitmask;
	unsigned int base = ~0U;
	unsigned int i;

	while ((i = (unsigned int)ffs((int)bitmask)) != 0) {
		bitmask >>= i;
		base += i;
		rnic_gro_tcp_flush_chain(tbl, base);
		__clear_bit((int)base, &tbl->flow_bitmask);
	}
}

STATIC void rnic_gro_tcp_fill_blk(struct rnic_gro_tcp_pkt_s *pkt,
				  struct rnic_gro_tcp_blks *blk)
{
	blk->seq = pkt->seq;
	blk->next_seq = pkt->next_seq;

	blk->payload_len = pkt->payload_len;
	blk->truesize = pkt->skb->truesize;
	blk->gso_size = (u16)pkt->payload_len;
	blk->count = 1;
	blk->front = 0;

	blk->iph = pkt->iph;
	blk->th  = pkt->th;

	blk->flags = tcp_flag_word(blk->th);

	blk->first = pkt->skb;
	blk->last  = pkt->skb;
}

STATIC struct rnic_gro_tcp_blks *rnic_gro_tcp_get_blk(
	struct rnic_gro_tcp_tbl_s *tbl, struct rnic_gro_list_s *list)
{
	struct rnic_gro_tcp_blks *blk = NULL;

	if (unlikely(!tbl->blk_remain)) {
		rnic_gro_tcp_flush_all(tbl);
		tbl->stats.blk_exhaust++;
	}

	blk = list_first_entry(&tbl->blk_free_list, struct rnic_gro_tcp_blks, list);
	__list_del_entry(&blk->list);

	tbl->blk_remain--;

	return blk;
}

STATIC bool rnic_gro_tcp_flows_same(struct rnic_gro_tcp_flow_s *flow,
				     struct rnic_gro_tcp_pkt_s *pkt)
{
	if (pkt->is_v4 ^ flow->is_v4)
		return false;

	return pkt->is_v4 ?
		rnic_gro_ip4_flows_same(&flow->keys4, &pkt->keys4) :
		rnic_gro_ip6_flows_same(&flow->keys6, &pkt->keys6);
}

STATIC struct rnic_gro_tcp_flow_s *rnic_gro_tcp_match_flow(
	struct rnic_gro_list_s *list, struct rnic_gro_tcp_pkt_s *pkt)
{
	struct list_head *pos = NULL;
	struct rnic_gro_tcp_flow_s *flow = NULL;

	/* Search for same flow */
	list_for_each(pos, &list->head) {
		flow = list_entry(pos, struct rnic_gro_tcp_flow_s, list);
		if (flow->netdev != pkt->netdev || flow->hash != pkt->hash)
			continue;

		if (rnic_gro_tcp_flows_same(flow, pkt))
			return flow;
	}

	return NULL;
}

STATIC void rnic_gro_tcp_add_flow(struct rnic_gro_tcp_tbl_s *tbl,
				  struct rnic_gro_list_s *list,
				  struct rnic_gro_tcp_pkt_s *pkt)
{
	struct rnic_gro_tcp_flow_s *flow = NULL;
	struct rnic_gro_tcp_blks *blk = NULL;

	blk = rnic_gro_tcp_get_blk(tbl, list);

	rnic_gro_tcp_fill_blk(pkt, blk);

	if (unlikely(list->count >= RNIC_GRO_MAX_FLOW_PER_BUCKET)) {
		/* The flow struct will be reused below. */
		flow = rnic_gro_tcp_flush_oldest(tbl, list);
		tbl->stats.bucket_full++;
		goto update_flow;
	} else {
		list->count++;
	}

	/*
	 * Since we limit max flow number in each bucket, we can
	 * make sure the free list won't be empty.
	 */
	flow = list_first_entry(&tbl->flow_free_list, struct rnic_gro_tcp_flow_s, list);
	__list_del_entry(&flow->list);

update_flow:
	flow->hash = pkt->hash;
	flow->keys6 = pkt->keys6;
	flow->netdev = pkt->netdev;
	flow->is_v4 = pkt->is_v4;
	INIT_LIST_HEAD(&flow->blk_head);
	list_add(&flow->list, &list->head);

	list_add(&blk->list, &flow->blk_head);
	flow->blk_count = 1;

	tbl->stats.new_flow++;
}

STATIC struct rnic_gro_tcp_blks *rnic_gro_tcp_insert_blk(
	struct rnic_gro_tcp_tbl_s *tbl,
	struct rnic_gro_tcp_flow_s *flow,
	struct list_head *pos)
{
	struct rnic_gro_tcp_blks *blk = NULL;

	if (unlikely(flow->blk_count >= tbl->blk_cap)) {
		tbl->stats.blk_full++;
		return NULL;
	}

	if (unlikely(!tbl->blk_remain)) {
		tbl->stats.blk_exhaust++;
		return NULL;
	}

	blk = list_first_entry(&tbl->blk_free_list, struct rnic_gro_tcp_blks, list);
	__list_del_entry(&blk->list);

	blk->first = NULL;
	list_add_tail(&blk->list, pos->next);
	flow->blk_count++;
	tbl->blk_remain--;

	return blk;
}

STATIC bool rnic_gro_tcp_splice_valid(struct rnic_gro_tcp_tbl_s *tbl,
	struct rnic_gro_tcp_pkt_s *pkt, struct rnic_gro_tcp_blks *blk)
{
	const struct tcphdr *th = pkt->th;
	const struct tcphdr *th2 = blk->th;
	u32 invaild;
	u32 thlen;
	u32 i;

	if (unlikely(pkt->splice_head)) {
		invaild = pkt->payload_len < (u32)blk->gso_size;
		invaild |= (pkt->payload_len > (u32)blk->gso_size) && (blk->count > 1);
		invaild |= (__force u32)tcp_flag_word(pkt->th) &
			   (TCP_FLAG_FIN | TCP_FLAG_PSH);
	} else {
		/*
		 * If there are already data packets of different sizes in the
		 * current blk, the blk->front flag will be set, indicating that
		 * the follow-up can only be combined forward
		 */
		invaild = blk->front;
		invaild |= pkt->payload_len > (u32)blk->gso_size;
		invaild |= (__force u32)blk->flags & TCP_FLAG_PSH;
	}

	for (i = sizeof(*th), thlen = pkt->thlen; i < thlen; i += sizeof(u32))
		invaild |= (*(u32 *)((u8 *)th + i) ^ *(u32 *)((u8 *)th2 + i));

	invaild |= blk->count >= tbl->weight;
	invaild |= blk->payload_len + pkt->payload_offset +
		     pkt->payload_len >= U16_MAX;

	if (unlikely(invaild))
		return false;

	return true;
}

STATIC struct rnic_gro_tcp_blks *rnic_gro_tcp_find_blk(
	struct rnic_gro_tcp_tbl_s *tbl, struct rnic_gro_tcp_flow_s *flow,
	struct rnic_gro_tcp_pkt_s *pkt, struct list_head *insert_pos)
{
	struct rnic_gro_tcp_blks *blk = NULL;
	struct list_head *pos = NULL;
	int splice_tail;
	int splice_head;

	list_for_each(pos, &flow->blk_head) {
		blk = list_entry(pos, struct rnic_gro_tcp_blks, list);
		insert_pos->next = &blk->list;

		splice_tail = pkt->seq - blk->next_seq;
		if (splice_tail == 0)
			return rnic_gro_tcp_splice_valid(tbl, pkt, blk) ?
				blk : NULL;

		tbl->stats.reorder++;

		if (splice_tail > 0)
			return NULL;

		splice_head = pkt->next_seq - blk->seq;
		if (splice_head < 0)
			continue;

		if (splice_head == 0) {
			insert_pos->next = blk->list.next;
			pkt->splice_head = true;
			return rnic_gro_tcp_splice_valid(tbl, pkt, blk) ?
				blk : NULL;
		}

		// dup skb
		if (pkt->seq <= blk->seq)
			insert_pos->next = blk->list.next;

		return NULL;
	}

	insert_pos->next = &flow->blk_head;
	return NULL;
}

STATIC void rnic_gro_tcp_merged_skb(struct rnic_gro_tcp_tbl_s *tbl,
				    struct rnic_gro_tcp_pkt_s *pkt,
				    struct rnic_gro_tcp_blks *blk)
{
	struct sk_buff *head_skb = NULL;
	struct sk_buff *head_last = NULL;
	struct sk_buff *next_skb = NULL;
	struct sk_buff *next_last = NULL;

	if (unlikely(blk == NULL))
		return;

	if (blk->first == NULL) {
		rnic_gro_tcp_fill_blk(pkt, blk);
		pkt->merged = true;
		return;
	}

	if (unlikely(pkt->payload_len - (u32)blk->gso_size))
		blk->front = 1;

	if (unlikely(pkt->splice_head)) {
		head_skb = pkt->skb;
		head_last = pkt->skb;

		next_skb = blk->first;
		next_last = blk->last;

		blk->th = pkt->th;
		blk->iph = pkt->iph;

		blk->gso_size = (u16)pkt->payload_len;
		blk->seq = pkt->seq;
	} else {
		head_skb = blk->first;
		head_last = blk->last;

		next_skb = pkt->skb;
		next_last = pkt->skb;

		blk->next_seq = pkt->next_seq;
	}

	// release next skb_header
	__skb_pull(next_skb, pkt->payload_offset);
	__skb_header_release(next_skb);

	head_last->next = next_skb;

	blk->payload_len += pkt->payload_len;
	blk->truesize += pkt->skb->truesize;
	blk->count++;
	blk->flags |= tcp_flag_word(pkt->th) & (TCP_FLAG_FIN | TCP_FLAG_PSH);

	blk->first = head_skb;
	blk->last = next_last;

	pkt->merged = true;
}

STATIC struct rnic_gro_tcp_flow_s *rnic_gro_tcp_try_to_merge(
	struct rnic_gro_tcp_tbl_s *tbl, struct rnic_gro_list_s *list,
	struct rnic_gro_tcp_pkt_s *pkt)
{
	struct rnic_gro_tcp_blks *blk = NULL;
	struct rnic_gro_tcp_flow_s *flow = NULL;
	const struct tcphdr *th = pkt->th;
	const struct tcphdr *th2 = NULL;
	struct list_head pos;
	u32 flush = 0;
	__be32 flags = tcp_flag_word(th);

	flow = rnic_gro_tcp_match_flow(list, pkt);
	if (flow == NULL)
		goto out_check;

	blk = list_first_entry(&flow->blk_head,
				struct rnic_gro_tcp_blks, list);

	if (pkt->is_v4)
		flush |= (ntohs(((struct iphdr *)pkt->iph)->frag_off) ^ IP_DF) |
			rnic_gro_cmp_ip4hdr(pkt->iph, blk->iph);
	else
		flush |= rnic_gro_cmp_ip6hdr(pkt->iph, blk->iph);

	th2 = blk->th;
	flush |= (__force u32)(flags & TCP_FLAG_CWR);
	flush |= (__force u32)((flags ^ tcp_flag_word(th2)) &
			       ~(TCP_FLAG_CWR | TCP_FLAG_FIN | TCP_FLAG_PSH));
	flush |= (__force u32)(th->ack_seq ^ th2->ack_seq);

	flush |= pkt->payload_len < 1;
	if (flush)
		goto out_check;

	INIT_LIST_HEAD(&pos);
	blk = rnic_gro_tcp_find_blk(tbl, flow, pkt, &pos);
	if (blk == NULL)
		blk = rnic_gro_tcp_insert_blk(tbl, flow, &pos);

	rnic_gro_tcp_merged_skb(tbl, pkt, blk);

out_check:
	flush = pkt->payload_len < 1;
	flush |= (__force u32)(flags & (TCP_FLAG_URG | TCP_FLAG_RST |
					TCP_FLAG_SYN | TCP_FLAG_FIN |
					TCP_FLAG_CWR));

	pkt->flush = (flush != 0);
	return (flow != NULL && (!(pkt->merged) || flush)) ? flow : NULL;
}

STATIC void rnic_gro_tcp_update_flow_mask(struct rnic_gro_tcp_tbl_s *tbl, u32 idx)
{
	if (tbl->flow_hash[idx].count) {
		if (!test_bit((int)idx, &tbl->flow_bitmask))
			__set_bit((int)idx, &tbl->flow_bitmask);
	} else if (test_bit((int)idx, &tbl->flow_bitmask)) {
		__clear_bit((int)idx, &tbl->flow_bitmask);
	}
}

STATIC void rnic_gro_tcp_receive_internal(struct rnic_gro_tcp_tbl_s *tbl,
					  struct rnic_gro_tcp_pkt_s *pkt)
{
	u32 idx = pkt->hash & RNIC_GRO_HASH_MASK;
	struct rnic_gro_list_s *list = &tbl->flow_hash[idx];
	struct rnic_gro_tcp_flow_s *flow = NULL;

	flow = rnic_gro_tcp_try_to_merge(tbl, list, pkt);
	if (flow != NULL) {
		__list_del_entry(&flow->list);
		rnic_gro_tcp_deliver_flow(tbl, flow);
		tbl->stats.flush_flow++;

		list->count--;
		list_add_tail(&flow->list, &tbl->flow_free_list);
	}

	if (pkt->merged) {
		tbl->stats.merged++;
		goto out_update;
	}

	if (pkt->flush) {
		tbl->deliver_skb(pkt->skb);
		tbl->stats.flush_single++;
		goto out_update;
	}

	rnic_gro_tcp_add_flow(tbl, list, pkt);

out_update:
	rnic_gro_tcp_update_flow_mask(tbl, idx);
}

STATIC bool rnic_gro_tcp_csum_valid(struct sk_buff *skb,
	struct rnic_gro_tcp_pkt_s *pkt)
{
	if (pkt->is_v4)
		return rnic_gro_ip_csum_valid(skb, (struct iphdr *)pkt->iph,
			pkt->payload_len + pkt->thlen, IPPROTO_TCP);

	return rnic_gro_ipv6_csum_valid(skb, (struct ipv6hdr *)pkt->iph,
			pkt->payload_len + pkt->thlen, IPPROTO_TCP);
}

STATIC bool rnic_gro_tcp_dissect_skb(struct sk_buff *skb,
	struct rnic_gro_tcp_pkt_s *pkt)
{
	pkt->is_v4 = (skb->protocol == htons(ETH_P_IP)) ? true : false;

	if (pkt->is_v4)
		return rnic_gro_tcp4_dissect_skb(skb, pkt);

	return rnic_gro_tcp6_dissect_skb(skb, pkt);
}

STATIC void rnic_gro_tcp_receive(void *table, struct sk_buff *skb)
{
	struct rnic_gro_tcp_tbl_s *tbl = table;
	struct rnic_gro_tcp_pkt_s info = {0};
	struct rnic_gro_tcp_pkt_s *pkt = &info;

	if (unlikely(!rnic_gro_tcp_dissect_skb(skb, pkt))) {
		tbl->stats.hdr_errs++;
		goto flush;
	}

	if (unlikely(!rnic_gro_tcp_csum_valid(skb, pkt))) {
		tbl->stats.csum_errs++;
		goto flush;
	}

	rnic_save_tcp_skb_dump(pkt->th, pkt->payload_len);

	rnic_gro_tcp_receive_internal(tbl, pkt);

	return;

flush:
	tbl->deliver_skb(skb);
}

STATIC void rnic_gro_tcp_complete(void *table)
{
	struct rnic_gro_tcp_tbl_s *tbl = table;
	rnic_gro_tcp_flush_all(tbl);
	tbl->stats.flush_all++;
}

STATIC void rnic_gro_tcp_set_weight(void *table, u32 weight)
{
	struct rnic_gro_tcp_tbl_s *tbl = table;
	tbl->weight = weight;
}

STATIC void rnic_gro_tcp_show_stats(void *table)
{
	struct rnic_gro_tcp_tbl_s *tbl = table;

	if (tbl == NULL)
		return;

	pr_err("[nas_rnic] [tcp] hdr_errs     %u \n", tbl->stats.hdr_errs);
	pr_err("[nas_rnic] [tcp] csum_errs    %u \n", tbl->stats.csum_errs);
	pr_err("[nas_rnic] [tcp] new_flow     %u \n", tbl->stats.new_flow);
	pr_err("[nas_rnic] [tcp] merged       %u \n", tbl->stats.merged);
	pr_err("[nas_rnic] [tcp] reorder      %u \n", tbl->stats.reorder);
	pr_err("[nas_rnic] [tcp] blk_full     %u \n", tbl->stats.blk_full);
	pr_err("[nas_rnic] [tcp] blk_exhaust  %u \n", tbl->stats.blk_exhaust);
	pr_err("[nas_rnic] [tcp] bucket_full  %u \n", tbl->stats.bucket_full);
	pr_err("[nas_rnic] [tcp] flush_flow   %u \n", tbl->stats.flush_flow);
	pr_err("[nas_rnic] [tcp] flush_single %u \n", tbl->stats.flush_single);
	pr_err("[nas_rnic] [tcp] flush_all    %u \n", tbl->stats.flush_all);
}

STATIC bool rnic_gro_tcp_init_blk(struct rnic_gro_tcp_tbl_s *tbl)
{
	struct rnic_gro_tcp_blks *blk = NULL;
	const u32 blk_num = (RNIC_GRO_HASH_SIZE * RNIC_GRO_MAX_FLOW_PER_BUCKET *
		RNIC_GRO_MAX_BLOCK_PER_FLOW) >> 1;
	u32 i;

	tbl->blks_buff = kzalloc(sizeof(*blk) * blk_num, GFP_KERNEL);
	if (tbl->blks_buff == NULL) {
		RNIC_LOGE("tcp gro alloc blk failed");
		return false;
	}

	INIT_LIST_HEAD(&tbl->blk_free_list);
	for (i = 0; i < blk_num; i++) {
		blk = tbl->blks_buff + i;
		list_add_tail(&blk->list, &tbl->blk_free_list);
	}

	tbl->blk_remain = blk_num;
	tbl->blk_cap = RNIC_GRO_MAX_BLOCK_PER_FLOW;

	return true;
}

STATIC bool rnic_gro_tcp_init_flow(struct rnic_gro_tcp_tbl_s *tbl)
{
	struct rnic_gro_tcp_flow_s *flow = NULL;
	const u32 max_flow_num = RNIC_GRO_HASH_SIZE * RNIC_GRO_MAX_FLOW_PER_BUCKET;
	u32 i;

	for (i = 0; i < RNIC_GRO_HASH_SIZE; i++) {
		INIT_LIST_HEAD(&tbl->flow_hash[i].head);
		tbl->flow_hash[i].count = 0;
	}
	tbl->flow_bitmask = 0;

	tbl->flows_buff = kzalloc(sizeof(*flow) * max_flow_num, GFP_KERNEL);
	if (tbl->flows_buff == NULL) {
		RNIC_LOGE("tcp gro alloc flows failed");
		return false;
	}

	INIT_LIST_HEAD(&tbl->flow_free_list);
	for (i = 0; i < max_flow_num; i++) {
		flow = tbl->flows_buff + i;
		list_add_tail(&flow->list, &tbl->flow_free_list);
	}

	return true;
}

STATIC void *rnic_gro_tcp_init(rnic_gro_deliver_fn deliver_func)
{
	struct rnic_gro_tcp_tbl_s *tbl = NULL;

	tbl = kzalloc(sizeof(*tbl), GFP_KERNEL);
	if (tbl == NULL) {
		RNIC_LOGE("tcp gro alloc tbl failed");
		goto err_alloc_tbl;
	}
	tbl->weight = RNIC_GRO_TCP_DEF_WEIGHT;

	if (!rnic_gro_tcp_init_flow(tbl))
		goto err_alloc_flow;

	if (!rnic_gro_tcp_init_blk(tbl))
		goto err_alloc_blk;

	if (!rnic_register_dump_callback())
		goto err_alloc_dump;

	tbl->deliver_skb = deliver_func;
	return tbl;

err_alloc_dump:
	kfree(tbl->blks_buff);
err_alloc_blk:
	kfree(tbl->flows_buff);
err_alloc_flow:
	kfree(tbl);
err_alloc_tbl:
	return NULL;
}

STATIC void rnic_gro_tcp_deinit(void *table)
{
	struct rnic_gro_tcp_tbl_s *tbl = table;

	if (tbl == NULL)
		return;

	/* Flush all flows holded. */
	rnic_gro_tcp_flush_all(tbl);

	kfree(tbl->blks_buff);
	kfree(tbl->flows_buff);
	kfree(tbl);
}

const struct rnic_gro_ops_s rnic_gro_tcp_ops = {
	.gro_init	= rnic_gro_tcp_init,
	.gro_deinit	= rnic_gro_tcp_deinit,
	.gro_receive	= rnic_gro_tcp_receive,
	.gro_complete	= rnic_gro_tcp_complete,
	.gro_set_weight	= rnic_gro_tcp_set_weight,
	.gro_show_stats	= rnic_gro_tcp_show_stats,
};