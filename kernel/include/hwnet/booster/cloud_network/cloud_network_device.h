/*
 * cloud_network_device.h
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

#ifndef _CLOUD_NETWORK_DEVICE_H_
#define _CLOUD_NETWORK_DEVICE_H_

#include <linux/types.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>

#define DEV_HTABLE_NUM 128
#define DEV_HTABLE_LOCK_NUM 512
#define DEV_HTABLE_COUNT_MAX 64

#define DEVICE_NAME_MATCH 0x0001

#define PRIMARY_WIFI "wlan0"
#define TUN_NAME "tun0"

#define MODEM_NUMS (ROLLE_MAX - 1)

enum tx_rx_state_type {
	TX_RX_STATE_ATAR = 0, /* all available devices send and receive */
	TX_RX_STATE_2T2R = 1,
	TX_RX_STATE_1T2R = 2,
	TX_RX_STATE_1T1R = 3,
	TX_RX_STATE_MAX
};

struct cloud_network_device_tun {
	atomic_t ip_enable;
	spinlock_t lock;
	struct net_device *dev;
	__be32 ip;
};

struct dev_name_match {
	char name[IFNAMSIZ];
	u32 start;
	u32 end;
};

struct net_device_match {
	unsigned long match_mode; /* bit0:dev_name_match */
	struct dev_name_match dev_name_rule;
};

struct net_device_stat {
	struct hlist_node dev_hash;
	atomic_t dev_enable;
	atomic_t ip_enable;
	rwlock_t lock;
	u32 role;
	struct net_device *dev;
	union nf_inet_addr addr;
	unsigned char dest_eth_addr[ETH_ALEN]; /* destination eth addr */
};

int cloud_network_device_init(void);
void cloud_network_device_exit(void);
int get_role_id(struct net_device *dev);
__be32 cloud_network_inet_select_addr(__be32 dst, int role_id);
struct net_device *get_net_device(int role_id);
void set_net_device_mac(int path, char *data);
char *get_net_device_mac(int path);
u32 get_link_path(void);
void set_trigger_rx(int val);
int get_trigger_rx(void);
void set_tx_rx_state(int val);
int get_tx_rx_state(void);
void device_rx_pkt_statistic(int device_id);
bool tun_dev_match_by_name(struct net_device *dev);
bool cloud_network_tun_device_skb_match(struct sk_buff *skb);
#endif /* _CLOUD_NETWORK_DEVICE_H_ */
