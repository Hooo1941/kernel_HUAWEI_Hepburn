/*
 * cloud_network_packet_match.c
 *
 * cloud network module implementation
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

#include "cloud_network_packet_match.h"
#include <net/netfilter/nf_conntrack.h>
#include <linux/inetdevice.h>
#include "cloud_network_device.h"
#include "cloud_network_utils.h"
#include "cloud_network_param.h"
#define TUN_NAME "tun0"
#define PACKET_MARK 0x11

static unsigned int change_packet_dev_to_tun_hook_func(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct nf_conn *ct = NULL;
	struct net_device *tun_dev = NULL;

	if (!is_cloud_network_enable())
		return NF_ACCEPT;

	ct = (struct nf_conn *)skb_nfct(skb);
	if (ct == NULL)
		return NF_ACCEPT;

	if (ct->mark == PACKET_MARK) {
		tun_dev = dev_get_by_name(&init_net, TUN_NAME);
		if (tun_dev == NULL)
			return NF_ACCEPT;

		skb->dev = tun_dev;
		dev_put(tun_dev);
	}
	return NF_ACCEPT;
}

bool set_mark_check(struct sk_buff *skb, struct net_device *tun_dev)
{
	struct iphdr *iph = NULL;

	if (!is_cloud_network_enable())
		return false;

	if (unlikely(skb == NULL || tun_dev == NULL))
		return false;

	iph = ip_hdr(skb);
	if (skb->protocol == htons(ETH_P_IP) &&
		((iph->protocol != IPPROTO_TCP &&
		iph->protocol != IPPROTO_UDP) ||
		!is_compass_available()) &&
		tun_dev->ip_ptr &&
		tun_dev->ip_ptr->ifa_list &&
		iph->saddr == tun_dev->ip_ptr->ifa_list->ifa_address)
		return true;

	return false;
}

static unsigned int set_mark_hook_func(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct net_device *tun_dev = NULL;
	unsigned int err;

	if (!is_cloud_network_enable())
		return NF_ACCEPT;

	if (skb && skb->sk && !skb_uid_match(skb->sk->sk_uid.val, skb))
		return NF_ACCEPT;
	tun_dev = dev_get_by_name(&init_net, TUN_NAME);
	if (!tun_dev)
		return NF_ACCEPT;

	if (set_mark_check(skb, tun_dev)) {
		skb->mark = PACKET_MARK;
		err = ip_route_me_harder(state->net, state->sk, skb, RTN_UNSPEC);
		if (err < 0) {
			dev_put(tun_dev);
			return NF_DROP_ERR(err);
		}
	}
	dev_put(tun_dev);
	return NF_ACCEPT;
}

static struct nf_hook_ops change_packet_dev_to_tun_hook = {
	.hook = change_packet_dev_to_tun_hook_func,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_IN,
	.priority = NF_IP_PRI_FILTER - 1
};

static struct nf_hook_ops set_mark_hook = {
	.hook = set_mark_hook_func,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_LOCAL_OUT,
	.priority = NF_IP_PRI_MANGLE - 1
};

void cloud_network_register_hook(void)
{
	nf_register_net_hook(&init_net, &change_packet_dev_to_tun_hook);
	nf_register_net_hook(&init_net, &set_mark_hook);
}

void cloud_network_unregister_hook(void)
{
	nf_unregister_net_hook(&init_net, &change_packet_dev_to_tun_hook);
	nf_unregister_net_hook(&init_net, &set_mark_hook);
}
