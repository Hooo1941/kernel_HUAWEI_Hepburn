/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: booster common interface.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2020-03-16
 */

#ifndef HW_BOOSTER_COMMON_H
#define HW_BOOSTER_COMMON_H

#include <linux/netdevice.h>
#include <linux/uidgid.h>
#include <net/sock.h>
#include "netlink_handle.h"

#define IFNAME_LEN_DOUBLE 2
#define IFNAME_LEN_TRIPLE 3

#define UDP_DNS_PORT 53
#define TCP_DNS_PORT 853

#define DS_NET_NAME "rmnet"
#define DS_NET_NAME_LEN 5
#define DS_NET_NAME_QCOM "rmnet_data"
#define DS_NET_NAME_QCOM_PREFIX_LEN 10

#define assign_short(p, val) (*(s16 *)(p) = (val))
#define assign_int(p, val) (*(s32 *)(p) = (val))
#define assign_uint(p, val) (*(u32 *)(p) = (val))
#define assign_long(p, val) (*(s64 *)(p) = (val))
#define skip_byte(p, num) ((p) = (p) + (num))

enum ifname_array_index_enum {
	ARRAY_INDEX_0 = 0,
	ARRAY_INDEX_1 = 1,
	ARRAY_INDEX_2 = 2,
	ARRAY_INDEX_3 = 3,
	ARRAY_INDEX_4 = 4,

	ARRAY_INDEX_BUTT
};

enum net_dev_id {
	DS_NET_ID = 0,
	DS_NET_SLAVE_ID,
	WIFI_NET_ID,
	WIFI_NET_SLAVE_ID,
	WIFI_NET_THIRD_ID,
	PHONE_SHELL_NET_ID,
};

#ifdef CONFIG_HW_BOOSTER_QCOM
enum qoe_version {
	QOE_VERSION_COMMON = 0,
	QOE_VERSION_ADVANCED = 1,
	QOE_VERSION_NUM_MAX
};

/* device id and modem id update */
struct interface_info {
	u32 modem_id;
	char name[IFNAMSIZ];
};

/* Notification request issued by the upper layer is defined as: */
struct interface_info_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct interface_info info;
};

/* Notification request issued by the upper layer is defined as: */
struct qoe_version_info_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	u32 qoe_version;
};
#endif

/* Notification request issued by the upper layer is defined as: */
struct close_udp_flow_info_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	u32 uid;
	u16 bind_mode;
	u16 ori_dev;
	u8 act;
};

msg_process *hw_booster_common_init(notify_event *notify);
bool is_ds_rnic(const struct net_device *dev);
#ifdef CONFIG_HW_BOOSTER_QCOM
bool is_qoe_version_advanced(void);
#endif
uid_t hw_get_sock_uid(struct sock *sk);
u8 *hw_get_payload_addr(const struct sk_buff *skb,
	u8 proto, u8 **vaddr, u32 *parse_len);
void hw_unmap_vaddr(u8 *vaddr);

#endif
