/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: This module is to support device-pipe synergy data frame sending and reveiving, framwork communication.
 * Create: 2023-11-23
 */

#include "dps_data_txrx.h"
#include "amax_multilink_tool.h"
#include <../net/wireless/nl80211.h>
#include <huawei_platform/net/amax/amax_multilink_interface.h>

#define WLAN_CMD_VDR_COMMON 0xCB

#define ETH_IP_UDP_LEN 42
#define IP_UDP_LEN 28
#define IPV4_VERSION 4
#define IPV4_HEADER_LEN 5 /* 默认长度为20Byte，此处以4Byte为1个单位 */
#define IPV4_TOS_AC_VO 0xc0 /* VO优先级队列 */
#define IPV4_HEADER_TTL 64
#define DPS_GROUP_ALL_SET 3
/* 使用特殊固定的端口号，用以区分灵犀端管管理帧 */
#define DPS_DATA_SRC_PORT 11111
#define DPS_DATA_DST_PORT 22222

static const uint8_t g_dev_name_main[] = "wlan0";
static const uint8_t g_dev_name_assist[] = "wlan1";
static uint8_t g_dev_mac_addr_assist[ETHER_ADDR_LEN] = {0};
static uint8_t g_dev_mac_addr_main[ETHER_ADDR_LEN] = {0};
/* 端管管理报文使用广播ip地址发送 */
static uint8_t g_broadcast_ip[IPV4_ADDR_SIZE] = { 255, 255, 255, 255 };

static struct net_device *dps_get_netdev(uint8_t dev)
{
	struct net_device *net_dev = NULL;
	if (dev == DPS_DEV_MAIN) {
		net_dev = dev_get_by_name(&init_net, g_dev_name_main);
	} else if (dev == DPS_DEV_ASSIST) {
		net_dev = dev_get_by_name(&init_net, g_dev_name_assist);
	} else {
		printk("[DPS] dev [%u] is invalid", dev);
		return NULL;
	}
	return net_dev;
}

static uint8_t *dps_get_dev_mac_addr(uint8_t dev)
{
	int32_t ret;
	struct net_device *net_dev = NULL;

	if (dev == DPS_DEV_MAIN) {
		net_dev = dev_get_by_name(&init_net, g_dev_name_main);
		if (net_dev != NULL) {
			if (memcpy_s(g_dev_mac_addr_main, ETHER_ADDR_LEN, net_dev->dev_addr, ETHER_ADDR_LEN) != EOK)
				return NULL;
			return g_dev_mac_addr_main;
		}
	} else if (dev == DPS_DEV_ASSIST) {
		net_dev = dev_get_by_name(&init_net, g_dev_name_assist);
		if (net_dev != NULL) {
			if (memcpy_s(g_dev_mac_addr_assist, ETHER_ADDR_LEN, net_dev->dev_addr, ETHER_ADDR_LEN) != EOK)
				return NULL;
			return g_dev_mac_addr_assist;
		}
	} else {
		printk("[DPS] dev [%u] is invalid", dev);
		return NULL;
	}

	return NULL;
}

/* 检查udp时，注意钩包时data指针的位置，当前钩包位于协议栈，data指针处于ip头 */
uint32_t check_pkt_is_dps_data(struct sk_buff *netbuf)
{
	struct iphdr *ip_header = NULL;
	struct udphdr *udp_header = NULL;
	llc_header *llc_hdr = NULL;
	uint8_t *skb_tail = NULL;

	if (unlikely(netbuf == NULL))
		return FAIL;

	skb_tail = amax_get_skb_tail(netbuf);
	llc_hdr = (llc_header *)(skb_tail - MAGIC_AMAX_LEN - sizeof(llc_header));

	/* 灵犀端管管理数据帧，reset和group三个bit均为1，且llc_id应该为0 */
	if (llc_hdr->reset != 1 || llc_hdr->group != DPS_GROUP_ALL_SET || llc_hdr->llc_id != 0)
		return FAIL;

	/* 灵犀端管管理数据帧 udp有固定的端口号 */
	ip_header = (struct iphdr *)(netbuf->data);
	udp_header = (struct udphdr *)(ip_header + 1);
	if (udp_header->source != ntohs(DPS_DATA_SRC_PORT) || udp_header->dest != ntohs(DPS_DATA_DST_PORT))
		return FAIL;

	/* 校验通过 */
	return SUCC;
}

static void devicepipe_rx_data_report(uint8_t *buf, size_t len)
{
	struct net_device *netdev = NULL;
	struct wireless_dev *wdev = NULL;

	netdev = dps_get_netdev(DPS_DEV_MAIN);
	if (netdev != NULL) {
		wdev = netdev->ieee80211_ptr;
	} else {
		printk("[DPS] devicepipe_rx_data_report netdev is NULL");
		return;
	}
	if (wdev != NULL) {
		cfg80211_mgmt_tx_vdr_common(wdev, buf, len, GFP_ATOMIC, WLAN_CMD_VDR_COMMON);
	} else {
		printk("[DPS] devicepipe_rx_data_report wdev is NULL");
		return;
	}
	printk("[DPS] report a data msg to fwk. len: %u", len);
}

void dps_report_data_to_userspace(struct sk_buff *netbuf)
{
	uint8_t *msg = NULL;
	uint8_t *data_start = NULL;
	int32_t msg_len;
	int32_t l_flags = GFP_KERNEL;

	if (unlikely(netbuf == NULL))
		return;

	/* 保证收到的skb是线性的 */
	if (netbuf->data_len) {
		printk("[DPS] netbuf is no linear.\n");
		goto freeskb;
	}
	/* 获取端管管理数据部分的实际长度 */
	msg_len = netbuf->len - IP_UDP_LEN - sizeof(llc_header) - MAGIC_AMAX_LEN;
	if (msg_len <= 0) {
		printk("[DPS] invalid msg_len\n");
		goto freeskb;
	}

	/* 不睡眠或在中断程序中或者在禁止抢占上下文中标志置为GFP_ATOMIC */
	if (in_interrupt() || irqs_disabled() || in_atomic())
		l_flags = GFP_ATOMIC;
	/* 申请并填充msg */
	msg = (uint8_t *)kmalloc(msg_len, l_flags);
	if (msg == NULL) {
		printk("[DPS] kmalloc msg failed\n");
		goto freeskb;
	}
	data_start = (uint8_t *)((uint8_t *)netbuf->data + IP_UDP_LEN);
	if (memcpy_s(msg, msg_len, data_start, msg_len) != EOK) {
		printk("[DPS] memcpy_s msg failed\n");
		kfree(msg);
		goto freeskb;
	}
	/* 消息传递至用户态 */
	devicepipe_rx_data_report(msg, msg_len);

	/* 释放msg及skb */
	kfree(msg);
freeskb:
	if (netbuf != NULL)
		dev_kfree_skb_any(netbuf);
}

static int32_t dps_fill_mac_header(struct ethhdr *mac_header, dps_mgmt_msg *dps_data_info)
{
	int32_t sec_ret;
	uint8_t *sta_mac = NULL;

	sta_mac = dps_get_dev_mac_addr(dps_data_info->send_dev);
	if (sta_mac == NULL) {
		printk("[DPS] sta mac get failed\n");
		return -1;
	}
	sec_ret = memcpy_s(mac_header->h_source, ETHER_ADDR_LEN, sta_mac, ETHER_ADDR_LEN);
	if (sec_ret != EOK)
		return sec_ret;
	sec_ret = memcpy_s(mac_header->h_dest, ETHER_ADDR_LEN, dps_data_info->bssid, ETHER_ADDR_LEN);
	if (sec_ret != EOK)
		return sec_ret;
	mac_header->h_proto = ntohs(ETHER_TYPE_IP);
	return 0;
}

static int32_t dps_fill_ip_header(struct iphdr *ip_header, uint32_t data_len)
{
	int32_t sec_ret;

	ip_header->version = IPV4_VERSION;
	ip_header->ihl = IPV4_HEADER_LEN;
	ip_header->tos = IPV4_TOS_AC_VO;
	// tot_len填实际值，以免有校验
	ip_header->tot_len = ntohs(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len +
		sizeof(llc_header) + MAGIC_AMAX_LEN);
	ip_header->ttl = IPV4_HEADER_TTL;
	ip_header->protocol = MAC_UDP_PROTOCAL;

	sec_ret = memcpy_s((void *)&ip_header->saddr, IPV4_ADDR_SIZE, g_broadcast_ip, IPV4_ADDR_SIZE);
	if (sec_ret != EOK)
		return sec_ret;
	sec_ret = memcpy_s((void *)&ip_header->daddr, IPV4_ADDR_SIZE, g_broadcast_ip, IPV4_ADDR_SIZE);
	if (sec_ret != EOK)
		return sec_ret;
	return 0;
}

static void dps_fill_udp_header(struct udphdr *udp_header, uint32_t data_len)
{
	udp_header->source = ntohs(DPS_DATA_SRC_PORT);
	udp_header->dest = ntohs(DPS_DATA_DST_PORT);
	// udp->len 填实际值，以免有校验
	udp_header->len = ntohs(sizeof(struct udphdr) + data_len + sizeof(llc_header) + MAGIC_AMAX_LEN);
	return;
}

static int32_t dps_fill_skb(struct sk_buff *netbuf, dps_mgmt_msg *dps_data_info, unsigned int data_len)
{
	struct ethhdr *mac_header = NULL;
	struct iphdr *ip_header = NULL;
	struct udphdr *udp_header = NULL;
	uint8_t *dps_ptr = NULL;
	uint8_t *magic_hdr = NULL;
	llc_header *llc_hdr = NULL;
	int32_t ret;

	// 获取各个头部指针
	mac_header = (struct ethhdr *)(netbuf->data);
	ip_header = (struct iphdr *)(mac_header + 1);
	udp_header = (struct udphdr *)(ip_header + 1);
	dps_ptr = (uint8_t *)(udp_header + 1);
	llc_hdr = (llc_header *)(dps_ptr + data_len);
	magic_hdr = (uint8_t *)(llc_hdr + 1);
	/* 填充mac头 */
	ret = dps_fill_mac_header(mac_header, dps_data_info);
	if (ret != 0) {
		printk("[DPS] fill mac header failed!");
		return ret;
	}
	/* 填充ip头 */
	ret = dps_fill_ip_header(ip_header, data_len);
	if (ret != 0) {
		printk("[DPS] fill ip header failed!");
		return ret;
	}
	/* 填充udp头 */
	dps_fill_udp_header(udp_header, data_len);
	/* 填充端管管理内容 */
	ret = memcpy_s(dps_ptr, data_len, dps_data_info->data, data_len);
	if (ret != 0) {
		printk("[DPS] fill dps data body failed!");
		return ret;
	}
	/* 填充llc头 将重置位和多核编号这3个bit置1，表示端管管理报文 */
	llc_hdr->reset = 1;
	llc_hdr->group = DPS_GROUP_ALL_SET;
	/* 填充MAGIC */
	ret = memcpy_s(magic_hdr, MAGIC_AMAX_LEN, get_magic_amax_sign(), MAGIC_AMAX_LEN);
	if (ret != 0) {
		printk("[DPS] fill dps data body failed!");
		return ret;
	}
	return 0;
}

static struct sk_buff *dps_zalloc_data_frame(unsigned int data_len)
{
	struct sk_buff *netbuf = NULL;
	uint32_t len = 0;

	len = ETH_IP_UDP_LEN + data_len + sizeof(llc_header) + MAGIC_AMAX_LEN;

	netbuf = dev_alloc_skb(len);
	if (unlikely(netbuf == NULL)) {
		printk("[DPS] skb alloc failed\n");
		return NULL;
	}
	if (unlikely(netbuf->data == NULL)) {
		dev_kfree_skb_any(netbuf);
		printk("[DPS] netbuf->data is NULL\n");
		return NULL;
	}
	(void)memset_s(netbuf->data, len, 0, len);
	/* tail指针后移 */
	skb_put(netbuf, len);
	netbuf->prev = NULL;
	netbuf->next = NULL;
	return netbuf;
}

static void dps_alloc_and_send_frame(dps_mgmt_msg *dps_data_info, unsigned int msg_len)
{
	struct sk_buff *netbuf = NULL;
	unsigned int data_len;
	int32_t ret;

	data_len = msg_len - sizeof(dps_data_info->send_dev) - sizeof(dps_data_info->bssid);
	netbuf = dps_zalloc_data_frame(data_len);
	if (netbuf == NULL)
		return;
	/* 填充报文头部，端管管理内容以及灵犀llc及magic */
	ret = dps_fill_skb(netbuf, dps_data_info, data_len);
	if (ret != 0) {
		dev_kfree_skb_any(netbuf);
		return;
	}
	/* 根据fn获取dev */
	netbuf->dev = dps_get_netdev(dps_data_info->send_dev);
	/* 复用amax的发送函数，将报文发出 */
	if (netbuf->dev == NULL) {
		printk("[DPS] dev get failed\n");
		dev_kfree_skb_any(netbuf);
		return;
	}
	hmac_hook_driver_tx_send(netbuf, netbuf->dev);

	printk("[DPS] send a dps data frame successfully\n");
}

void dps_ioctl_msg_proc(uint8_t *msg_info, unsigned int msg_len)
{
	dps_mgmt_msg *dps_data_info = NULL;

	if (msg_info == NULL) {
		printk("[DPS] msg_info is NULL\n");
		return;
	}

	dps_data_info = (dps_mgmt_msg *)msg_info;
	if (msg_len < sizeof(dps_mgmt_msg)) {
		printk("[DPS] error msg_len %u\n", msg_len);
		return;
	}

	printk("[DPS] dps ioctl msg. len[%u]\n", msg_len);

	dps_alloc_and_send_frame(dps_data_info, msg_len);

	return;
}