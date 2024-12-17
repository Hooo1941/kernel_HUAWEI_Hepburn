/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              collect ipv6 sync
 * Author: shenyujuan@huawei.com
 * Create: 2023-03-14
 */

#ifndef _IPV6_TCP_SYNC_COLLEC_H
#define _IPV6_TCP_SYNC_COLLEC_H

#include <linux/netfilter.h>
#include "netlink_handle.h"

#define CHECK_TIME (10 * HZ)
#define DS_NET "rmnet0"
#define DS_NET_SLAVE "rmnet3"
#define WIFI_NET "wlan0"
#define WIFI_NET_SLAVE "wlan1"
#define WIFI_NET_THIRD "wlan2"
#define DS_NET_MTK "ccmni"
#define DS_NET_MTK_PREFIX_LEN 5
#define DS_NET_MTK_LEN 6
#define DS_NET_LEN 6
#define DS_NET_QCOM "rmnet_data"
#define DS_NET_QCOM_PREFIX_LEN 10
#define DS_NET_QCOM_LEN 11
#define WIFI_NET_LEN 5
#define PHONE_SHELL_NET "usb0"
#define PHONE_SHELL_NET_LEN 4
#define CHANNEL_NUM 6
#define DS_INTERFACE_NAME_ID 6

enum app_status {
	UID_FOREGROUND = 1,
	UID_BACKGROUND = 2,
};

/* TCPIP protocol stack parameter structure body */
struct report_status {
	u32 uid; // The value 0 means does not filter
	u16 status; // bit0 ~ bit15 represent for channelId
};

/* Dynamic update current uid state */
struct cur_uid_info {
	int cur_uid; // current uid
	int cur_uid_state; // current uid state background or foreground
};

/* device id and modem id update */
struct link_property_info_ex {
	u32 modem_id;
	u32 interface_id; // numer of ccmni1, ccmni2...., rmnet0, rmnet1
};
/* Notification request issued by the upper layer is defined as: */
struct link_property_msg_ex {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct link_property_info_ex property;
};

msg_process *hw_ipv6_tcp_sync_collec_init(notify_event *notify);
void booster_update_data_tcp_rx_statistics(u_int8_t af, struct sk_buff *skb, const struct nf_hook_state *state);
void booster_update_data_tx_statistics(u_int8_t af, struct sk_buff *skb);
#endif
