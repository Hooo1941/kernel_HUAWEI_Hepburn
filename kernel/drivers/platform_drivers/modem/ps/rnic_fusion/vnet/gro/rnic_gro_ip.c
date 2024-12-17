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
#include "rnic_gro.h"
#include "rnic_gro_ip.h"

int rnic_gro_cmp_ip4hdr(void *iph1, void *iph2)
{
	const struct iphdr *ip4h1 = iph1;
	const struct iphdr *ip4h2 = iph2;
	return (ip4h1->ttl ^ ip4h2->ttl) | (ip4h1->tos ^ ip4h2->tos) |
	       ((ip4h1->frag_off ^ ip4h2->frag_off) & htons(IP_DF));
}

int rnic_gro_cmp_ip6hdr(void *iph1, void *iph2)
{
	const struct ipv6hdr *ip6h1 = iph1;
	const struct ipv6hdr *ip6h2 = iph2;
	return (*(__be32 *)ip6h1 ^ *(__be32 *)ip6h2) & htonl(0x0FF00000);
}

bool rnic_gro_ip6_flows_same(struct rnic_gro_ip6_flow_keys_s *keys1,
			     struct rnic_gro_ip6_flow_keys_s *keys2)
{
	/*
	 * ipv6 with non-upper-layer nxthdr wouldn't be recognized as
	 * tcp via pfa, so we wouldn't compare nxthdr here, e.g.
	 * memcmp(iph + 1, iph2 + 1, nlen - sizeof(*iph)).
	 */
	return ipv6_addr_equal(&keys1->saddr, &keys2->saddr) &&
	       ipv6_addr_equal(&keys1->daddr, &keys2->daddr) &&
	       ((keys1->ports ^ keys2->ports) |
	       (keys1->first_word ^ keys2->first_word) |
	       (keys1->hop_limit ^ keys2->hop_limit)) == 0;
}

bool rnic_gro_ip4_flows_same(struct rnic_gro_ip4_flow_keys_s *keys1,
			     struct rnic_gro_ip4_flow_keys_s *keys2)
{
	return ((keys1->saddr ^ keys2->saddr) | (keys1->daddr ^ keys2->daddr) |
		(keys1->ports ^ keys2->ports)) == 0;
}

bool rnic_gro_ip4hdr_valid(struct sk_buff *skb, struct iphdr *iph)
{
	u32 len;

	if (unlikely(skb->len < sizeof(struct iphdr)))
		return false;

	if (unlikely(*(u8 *)iph != 0x45))
		return false;

	if (unlikely(ip_is_fragment(iph)))
		return false;

	/* ip header length value is 5 */
	if (unlikely(ip_fast_csum((u8 *)iph, 5)))
		return false;

	len = ntohs(iph->tot_len);
	if (unlikely(skb->len < len))
		return false;
	else if (unlikely(len < sizeof(struct iphdr)))
		return false;

	/* remove padding if exist */
	if (unlikely(skb->len > len))
		__skb_trim(skb, len);

	return true;
}

bool rnic_gro_ip6hdr_valid(struct sk_buff *skb, u32 payload_len, u32 l4hdr_len)
{
	u32 ip6hdr_len = sizeof(struct ipv6hdr);

	/*
	 * IPv6 with non-upper-layer nxthdr at the begining would
	 * not be recognized as tcp6 by pfa.
	 */
	if (unlikely(skb->len < ip6hdr_len + l4hdr_len))
		return false;

	if (unlikely(payload_len < l4hdr_len))
		return false;

	/* padding */
	if (unlikely(skb->len > payload_len + ip6hdr_len))
		skb->len = payload_len + ip6hdr_len;

	/*
	 * If padding, next conidtion would always be false.
	 * Since there would be checksum later, these skb are allowed to move on.
	 */
	if (unlikely(payload_len != (skb->len - ip6hdr_len)))
		return false;

	return true;
}