/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#include <securec.h>
#include "amax_multilink_main.h"
#include "amax_multilink_tool.h"

void *k_memalloc(uint32_t size)
{
	int32_t l_flags = GFP_KERNEL;
	void *mem_space = NULL;

	/* 不睡眠或在中断程序中或者在禁止抢占上下文中标志置为GFP_ATOMIC */
	if (in_interrupt() || irqs_disabled() || in_atomic())
		l_flags = GFP_ATOMIC;

	if (unlikely(size == 0))
		return NULL;

	mem_space = kmalloc(size, l_flags);
	if (mem_space == NULL)
		return NULL;

	return mem_space;
}

int32_t is_udp_pkt(struct sk_buff *skb)
{
	struct ethhdr *p_macheader = NULL;
	struct iphdr *p_ipheader = NULL;

	p_macheader = (struct ethhdr *)(skb->data);
	p_ipheader = (struct iphdr *)(p_macheader + 1);
	if (p_ipheader->protocol == MAC_UDP_PROTOCAL)
		return true;
	return false;
}

uint32_t get_tcp_seq(struct sk_buff *netbuf, uint8_t offset)
{
	struct iphdr *p_ipheader = NULL;
	struct tcphdr *p_tcpheader = NULL;
	uint32_t tcp_seq = 0;

	p_ipheader = (struct iphdr *)((uint8_t *)(netbuf->data) + offset);
	if (p_ipheader->protocol == MAC_TCP_PROTOCAL) {
		p_tcpheader = (struct tcphdr *)(p_ipheader + 1);
		tcp_seq = htonl(p_tcpheader->seq);
	}

	return tcp_seq;
}

uint16_t get_ip_id(struct sk_buff *netbuf, uint8_t offset)
{
	struct iphdr *p_ipheader = NULL;

	p_ipheader = (struct iphdr *)((uint8_t *)(netbuf->data) + offset);
	return htons(p_ipheader->id);
}

bool eth_frame_is_dhcp(const struct iphdr *ip_header)
{
	struct udphdr *udp_header = NULL;

	if (unlikely(ip_header == NULL)) {
		amax_print(PRINT_ERROR, "ip_header is null.\n");
		return false;
	}
	/* DHCP判断标准: udp协议，ip头部fragment offset字段为0，目的端口号为67或68 */
	if (ip_header->protocol == MAC_UDP_PROTOCAL && ((ip_header->frag_off & 0xFF1F) == 0)) {
		udp_header = (struct udphdr *)(ip_header + 1);

		if (htons(udp_header->dest) == MAC_DHCP_DEST_PORT1 || htons(udp_header->dest) == MAC_DHCP_DEST_PORT2)
			return true;
	}

	return false;
}

bool eth_frame_is_hilink(const struct iphdr *ip_header)
{
	struct tcphdr *tcp_header = NULL;
	if (unlikely(ip_header == NULL)) {
		amax_print(PRINT_ERROR, "ip_header is null.\n");
		return false;
	}

	/* hilink识别标准：tcp协议，源端口号是37443 */
	if (ip_header->protocol != MAC_TCP_PROTOCAL)
		return false;

	tcp_header = (struct tcphdr *)(ip_header + 1);
	if ((htons(tcp_header->source) == MAC_HILINK_SRC_PORT) || (htons(tcp_header->dest) == MAC_HILINK_SRC_PORT))
		return true;

	return false;
}

bool eth_frame_is_amax_encaped(const struct sk_buff *netbuf)
{
	uint8_t *ptr = NULL;
	uint8_t *skb_tail = NULL;
	uint8_t *magic_sign = NULL;

	if (unlikely(netbuf == NULL)) {
		amax_print(PRINT_ERROR, "Error: empty netbuf.\n");
		return false;
	}

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb_tail = (uint8_t *)(netbuf->head + netbuf->tail);
#else
	skb_tail = (uint8_t *)netbuf->tail;
#endif
	ptr = (uint8_t *)(skb_tail - MAGIC_AMAX_LEN);
	magic_sign = get_magic_amax_sign();
	if (memcmp(ptr, magic_sign, MAGIC_AMAX_LEN) == 0)
		return true;

	return false;
}

bool eth_frame_is_fragmented(const struct iphdr *ip_header)
{
	if (unlikely(ip_header == NULL)) {
		amax_print(PRINT_ERROR, "ip_header is null.\n");
		return false;
	}

	if ((ip_header->frag_off & htons(IP_MF)) || (ip_header->frag_off & htons(IP_FRAG_OFFSET_IS_ZERO)))
		return true;

	return false;
}

uint8_t amax_get_tid(struct iphdr *ip_header)
{
	uint8_t tid = 0;

	if (ip_header == NULL) {
		amax_print(PRINT_ERROR, "ip_header is null.\n");
		return TID_ARR_NUM;
	}
	tid = ip_header->tos >> WLAN_IP_PRI_SHIFT;
#ifdef _PRE_WLAN_FEATURE_NETWORK_SLICING
	/* 判断是否为网络切片业务 */
	if (IS_IPV4_NS_TOS(ip_header->tos) == true)
		tid = WLAN_TIDNO_NETWORK_SLICING;
#endif

	return tid;
}

int32_t amax_check_tid_is_needed(uint8_t tid)
{
	if (tid == WLAN_TIDNO_BEST_EFFORT || tid == WLAN_TIDNO_NETWORK_SLICING ||
		tid == WLAN_TIDNO_VIDEO || tid == WLAN_TIDNO_VOICE)
		return SUCC;
	return FAIL;
}

uint8_t *amax_get_skb_tail(struct sk_buff *netbuf)
{
	uint8_t *skb_tail = NULL;

	if (netbuf == NULL)
		return NULL;

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb_tail = (uint8_t *)(netbuf->head + netbuf->tail);
#else
	skb_tail = (uint8_t *)netbuf->tail;
#endif

	return skb_tail;
}
