/*
 * cloud_network_packet_match.h
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

#ifndef _CLOUD_NETWORK_HOOK_H_
#define _CLOUD_NETWORK_HOOK_H_
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/types.h>

void cloud_network_register_hook(void);
void cloud_network_unregister_hook(void);
bool set_mark_check(struct sk_buff *skb, struct net_device *tun_dev);
#endif /* _CLOUD_NETWORK_HOOK_H_ */