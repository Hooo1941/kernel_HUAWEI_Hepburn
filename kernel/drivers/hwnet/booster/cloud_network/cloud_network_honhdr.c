/*
 * cloud_network_honhdr.c
 *
 * cloud network kernel module implementation
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

#include "cloud_network_detect.h"
#include "cloud_network_utils.h"
#include "cloud_network_honhdr.h"
#include "cloud_network_rx.h"
#include "cloud_network_option.h"

int get_honhdr_payload_len(const struct sk_buff *skb)
{
	return ntohl(hon_hdr(skb)->payload_length);
}

int get_honhdr_totlen(const struct sk_buff *skb)
{
	return HON_SIZE + get_options_len(skb) + HMAC_SIZE;
}

int get_options_len(const struct sk_buff *skb)
{
	__u16 *len = NULL;

	if (!is_option(skb))
		return 0;

	len = hon_option(skb);
	return ntohs(*len);
}

int get_options_len_gro(char *data)
{
	__u16 *len = NULL;
	struct honhdr *honh = NULL;

	if (unlikely(data == NULL))
		return 0;

	honh = (struct honhdr *)data;
	if (honh->x == 0)
		return 0;

	len = (__u16 *)(data + HON_SIZE);
	return ntohs(*len);
}

bool is_option(const struct sk_buff* skb)
{
	struct honhdr *honh = NULL;

	if (unlikely(skb == NULL))
		return false;

	honh = hon_hdr(skb);
	if (unlikely(honh == NULL))
		return false;

	if (honh->x == 1)
		return true;
	return false;
}

struct hon_skb *alloc_hon_skb(struct honhdr *hon, struct sk_buff *skb)
{
	struct hon_skb *hskb = NULL;
	errno_t err;

	if (unlikely(hon == NULL || skb == NULL))
		return NULL;

	hskb = (struct hon_skb *)kmalloc(sizeof(struct hon_skb), GFP_ATOMIC);
	if (hskb == NULL)
		return NULL;

	err = memcpy_s(&(hskb->hon), sizeof(struct honhdr), hon,
		sizeof(struct honhdr));
	if (err != EOK) {
		kfree(hskb);
		return NULL;
	}
	hskb->skb = skb;
	return hskb;
}

void free_hon_skb(struct hon_skb *hskb)
{
	if (hskb == NULL)
		return ;
	if (hskb->skb != NULL) {
		dev_kfree_skb_any(hskb->skb);
		hskb->skb = NULL;
	}
	kfree(hskb);
}

void free_hon_skb_without_skb(struct hon_skb *hskb)
{
	if (hskb == NULL)
		return ;
	kfree(hskb);
}

static void hon_len_error_print(struct iphdr *iph, struct udphdr *udph,
	struct honhdr *honh)
{
	log_err("ip id %d totlen %d tos %d ihl %d ttl %d", ntohs(iph->id),
		ntohs(iph->tot_len), iph->tos, iph->ihl, iph->ttl);
	log_err("udp len %d", ntohs(udph->len));
	log_err("honh version %u flags %u appid %u %u %u hmacid %u"
		" payloadlength %u seq %u", honh->version, *(honh->__flags_offset),
		honh->app_id[NUM_0], honh->app_id[NUM_1], honh->app_id[NUM_2],
		honh->hmac_id, ntohs(honh->payload_length), ntohl(honh->seq));
	return;
}

bool check_hon_len(struct sk_buff *skb, bool has_mac_header)
{
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	struct honhdr *honh = NULL;
	int hon_option_len = 0;
	int offset = 0;

	if (has_mac_header)
		offset = MAC_SIZE;

	iph = (struct iphdr *)(skb->data + offset);
	udph = (struct udphdr *)(skb->data + offset + iph->ihl * 4);
	honh = (struct honhdr *)(skb->data + offset + iph->ihl * 4 + UDP_SIZE);

	if (honh->x == 1)
		hon_option_len = *(int *)(skb->data + offset + iph->ihl * 4 +
			sizeof(struct udphdr) + sizeof(struct honhdr));

	if (ntohs(honh->payload_length) != 0 &&
		ntohs(honh->payload_length) < IP_SIZE + UDP_SIZE) {
		log_err("honh payload len %d, error", ntohs(honh->payload_length));
		hon_len_error_print(iph, udph, honh);
		return false;
	}
	if (ntohs(udph->len) != UDP_SIZE + HON_SIZE + HMAC_SIZE +
		ntohs(hon_option_len) + ntohs(honh->payload_length)) {
		log_err("outter udp length is invalid, error");
		hon_len_error_print(iph, udph, honh);
		return false;
	}
	if (ntohs(honh->payload_length) == 0 && honh->x != 1) {
		log_err("honh payload length and option is invalid, error");
		hon_len_error_print(iph, udph, honh);
		return false;
	}
	return true;
}

bool check_hon_len_gro(struct honhdr *honh)
{
	if (unlikely(honh == NULL))
		return false;

	if (ntohs(honh->payload_length) != 0 &&
		ntohs(honh->payload_length) < IP_SIZE + UDP_SIZE) {
		log_err("honh version %u flags %u appid %u %u %u hmacid %u"
			" payloadlength %u seq %u", honh->version, *(honh->__flags_offset),
			honh->app_id[NUM_0], honh->app_id[NUM_1], honh->app_id[NUM_2],
			honh->hmac_id, ntohs(honh->payload_length), ntohl(honh->seq));
		return false;
	}
	return true;
}
