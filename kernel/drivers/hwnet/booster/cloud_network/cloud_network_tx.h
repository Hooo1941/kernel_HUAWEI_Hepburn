/*
 * cloud_network_tx.h
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

#include <linux/skbuff.h>
#ifndef _CLOUD_NETWORK_PACKET_TRANSMIT_H_
#define _CLOUD_NETWORK_PACKET_TRANSMIT_H_

int cloud_network_tx(struct sk_buff *skb, struct net_device *dev);
int set_skb_param_by_adapter(struct sk_buff *skb, int id);
int cloud_network_set_mac(struct sk_buff* skb, int path);
int skb_send(struct sk_buff *skb, int type);
struct session *session_manage(struct sk_buff* skb);
void hton24(u8 *p);
int get_fid(int id);
#endif
