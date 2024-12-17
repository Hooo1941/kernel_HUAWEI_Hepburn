/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#ifndef AMAX_MULTILINK_TOOL_H_
#define AMAX_MULTILINK_TOOL_H_

#include <net/tcp.h>
#include <net/udp.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/ktime.h>
#include <huawei_platform/log/hw_log.h>

typedef enum {
	PRINT_ERROR = 1,
	PRINT_WARNING = 2,
	PRINT_INFO = 3,
	PRINT_DEBUG = 4
} PRINT_LEVEL;

#define HWLOG_TAG amax
HWLOG_REGIST();

#define PRINT_LOG_LEVEL PRINT_DEBUG

#define amax_print(level, format, ...)                                           \
	do {                                                                         \
		if (PRINT_LOG_LEVEL >= level) {                                          \
			printk("[amax][%s][%d]" format, __FUNCTION__, level, ##__VA_ARGS__); \
		}                                                                        \
	} while (0)

#define NETBUF_DATA(_pst_buf) ((_pst_buf)->data)

void *k_memalloc(uint32_t size);
int32_t is_udp_pkt(struct sk_buff *skb);
uint32_t get_tcp_seq(struct sk_buff *netbuf, uint8_t offset);
uint16_t get_ip_id(struct sk_buff *netbuf, uint8_t offset);

bool eth_frame_is_dhcp(const struct iphdr *ip_header);
bool eth_frame_is_hilink(const struct iphdr *ip_header);
bool eth_frame_is_amax_encaped(const struct sk_buff *netbuf);
bool eth_frame_is_fragmented(const struct iphdr *ip_header);

uint8_t amax_get_tid(struct iphdr *ip_header);
int32_t amax_check_tid_is_needed(uint8_t tid);
uint8_t *amax_get_skb_tail(struct sk_buff *netbuf);

#endif
