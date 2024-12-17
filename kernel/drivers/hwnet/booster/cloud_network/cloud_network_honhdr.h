/*
 * cloud_network_honhdr.h
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

#ifndef _CLOUD_NETWORK_HONHDR_H_
#define _CLOUD_NETWORK_HONHDR_H_
#include <linux/skbuff.h>
#include <linux/types.h>
#include <net/udp.h>

#define OPTION_LEN_SIZE 2
#define SIGNATURE_SIZE 10
#define CLOUD_NETWORK_HON_VERSION 4

#define HON_CF_NONE 0b0
#define HON_CF_HON_COMPRESS 0b0001
#define HON_CF_INNER_IP_COMPRESS 0b0010
#define HON_CF_INNER_TCP_UDP_COMPRESS 0b0100
#define HON_CF_INNER_APPLICATION_COMPRESS 0b1000

#define HON_MP_REDUNDANCY 1
#define HON_MP_AGGREGATION 2

#define HONOPT_NOP 1
#define HONOPT_DETECT_REQUEST 2
#define HONOPT_DETECT_RESPOND 3
#define HONOPT_RESET 4
#define HONOPT_RESPOND 5
#define HONOPT_SESSION 6
#define HONOPT_FLOW_CONTROL 7
#define HONOPT_INFORM_EXCEPTION 8
#define HONOPT_INFORM_AGING 9
#define HONOPT_LINK_MANAGE 10
#define HONOPT_SESSION_MP_MANAGE 11
#define HONOPT_QOE_SYNC_QOE 12
#define HONOPT_QOE_SYNC_ALGORITHM 13
#define HONOPT_RETRANSMIT_REQUEST 14
#define HONOPT_SHORT_HMAC_START 15
#define HONOPT_SHORT_HMAC_STOP 16
#define HONOPT_MAX 17

#define MAC_SIZE 14
#define IP_SIZE 20
#define UDP_SIZE 8
#define HON_SIZE 22
#define HMAC_SHA256_SIGNATURE 32
#define HMAC_SIZE 10
#define REVERSE_SIZE 100
#define HON_EXTRA_SIZE (IP_SIZE + UDP_SIZE + HON_SIZE + HMAC_SIZE)

#define OPTION_LEN_MAX 1500
#pragma pack(1)
struct honhdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	compress_flag : 4,
			version : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	version : 4,
			compress_flag : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	frag_tot : 4,
			frag_off : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	frag_off : 4,
			frag_tot : 4;
#endif
	/* private */
	__u16	__flags_offset[0];
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16	pr : 1,
			fs : 1,
			fp : 1,
			v6 : 1,
			smp : 1,
			etp : 1,
			rsv : 2,
			s : 1,
			m : 2,
			p : 1,
			o : 1,
			g : 1,
			x : 1,
			px : 1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16	rsv : 2,
			etp : 1,
			smp : 1,
			v6 : 1,
			fp : 1,
			fs : 1,
			pr : 1,
			px : 1,
			x : 1,
			g : 1,
			o : 1,
			p : 1,
			m : 2,
			s : 1;
#endif
	__u8	app_id[3];
	__u8	hmac_id;
	__u16	payload_length;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	fid : 4,
			mp_policy : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	mp_policy : 4,
			fid : 4;
#endif
	__u8	res1;
	__u8	client_id[6];
	union {
		__u32   seq;
		__u32   ack_seq;
	};
	/* The options start here. */
};
#pragma pack()
struct hmachdr {
	__u8 hmac[SIGNATURE_SIZE];
};

enum {
	HON_OPT_HANDLE_FAILED = -1,
	HON_OPT_HANDLE_SUCCESS = 0
};

int get_honhdr_payload_len(const struct sk_buff *skb);
int get_honhdr_totlen(const struct sk_buff *skb);
int get_options_len(const struct sk_buff *skb);
int get_options_len_gro(char *data);
bool is_option(const struct sk_buff* skb);
struct hon_skb *alloc_hon_skb(struct honhdr *hon, struct sk_buff *skb);
void free_hon_skb(struct hon_skb *hskb);
void free_hon_skb_without_skb(struct hon_skb *hskb);
bool check_hon_len(struct sk_buff *skb, bool has_mac_header);
bool check_hon_len_gro(struct honhdr *honh);
static inline void *get_payload_start(const struct sk_buff *skb)
{
	return (void *)(skb->data + IP_SIZE + sizeof(struct udphdr) + sizeof(struct honhdr) + get_options_len(skb) + sizeof(struct hmachdr));
}

static inline struct honhdr *hon_hdr(const struct sk_buff *skb)
{
	return (struct honhdr *)(skb->data + IP_SIZE + sizeof(struct udphdr));
}

static inline void *hon_option(const struct sk_buff *skb)
{
	return (void *)(skb->data + IP_SIZE + sizeof(struct udphdr) + sizeof(struct honhdr));
}

static inline struct hmachdr *hmac_hdr(const struct sk_buff *skb)
{
	return (struct hmachdr *)(skb->data + IP_SIZE + sizeof(struct udphdr) + sizeof(struct honhdr) + get_options_len(skb));
}
#endif /* _CLOUD_NETWORK_HONHDR_H_ */
