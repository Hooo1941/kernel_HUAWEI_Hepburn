/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to collect ipv6 sync
 * Author: shenyujuan@huawei.com
 * Create: 2023-03-14
 */

#include "hw_ipv6_tcp_sync_collec.h"

#include <linux/net.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/time.h>
#include <net/tcp.h>
#include <securec.h>

#include "hw_booster_common.h"

#define NOTIFY_BUF_LEN 512
#define INVALID_LINK_PROPERTITY_INTERFACE_ID 0xFF
static DEFINE_SPINLOCK(g_ipv6_tcp_sync_collec_lock);

static struct cur_uid_info g_uid_info;
static bool g_other_rx[CHANNEL_NUM] = {0};
static bool g_ipv6_sync_ack_uid[CHANNEL_NUM] = {0};
static bool g_ipv6_sync_uid[CHANNEL_NUM] = {0};
static bool g_need_check = false;
static unsigned long g_start_time = 0;

static struct link_property_info_ex g_link_property[] = {
	{ DS_NET_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID },
	{ DS_NET_SLAVE_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID }};

static notify_event *notifier = NULL;

static u32 identify_network_protocol(struct sk_buff *skb,
	struct ipv6hdr **ip6h, struct iphdr **iph, struct tcphdr **tcph, u_int8_t af);
static int match_net_dev(struct sk_buff *skb, struct sock *sk, u32 *dev_id);

static uid_t get_uid_from_sock(struct sock *sk)
{
	if (sk == NULL)
		return 0;
	return sk->sk_uid.val;
}

static unsigned int get_mtk_net_id(const char *dev_name)
{
	unsigned int dev_id = 0;
	if (dev_name == NULL || strlen(dev_name) != DS_NET_MTK_LEN)
		return DS_NET_ID; // devName index is in [0, 9]
	if (dev_name[DS_NET_MTK_LEN - 1] < '0' || dev_name[DS_NET_MTK_LEN - 1] > '9')
		return DS_NET_ID;
	dev_id = (unsigned int)(dev_name[DS_NET_MTK_LEN - 1] - '0');
	if (g_link_property[DS_NET_SLAVE_ID].interface_id == dev_id)
		return DS_NET_SLAVE_ID;
	return DS_NET_ID;
}
static unsigned int get_qcom_net_id(const char *dev_name)

{
	unsigned int dev_id = 0;

	if (dev_name == NULL || strlen(dev_name) != DS_NET_QCOM_LEN)
		return DS_NET_ID; // devName index is in [0, 9]
	if (dev_name[DS_NET_QCOM_LEN - 1] < '0' || dev_name[DS_NET_QCOM_LEN - 1] > '9')
		return DS_NET_ID;
	dev_id = (unsigned int)(dev_name[DS_NET_QCOM_LEN - 1] - '0');
	if (g_link_property[DS_NET_SLAVE_ID].interface_id == dev_id)
		return DS_NET_SLAVE_ID;
	return DS_NET_ID;
}

static void restore()
{
	(void)memset_s(g_ipv6_sync_uid, sizeof(g_ipv6_sync_uid), 0, sizeof(g_ipv6_sync_uid));
	(void)memset_s(g_ipv6_sync_ack_uid, sizeof(g_ipv6_sync_ack_uid), 0, sizeof(g_ipv6_sync_ack_uid));
	(void)memset_s(g_other_rx, sizeof(g_other_rx), 0, sizeof(g_other_rx));
	(void)memset_s(&g_uid_info, sizeof(g_uid_info), 0, sizeof(g_uid_info));
	g_need_check = false;
	g_start_time = 0;
}

static void notify_tcp_collec_event(void)
{
	char event[NOTIFY_BUF_LEN];
	char *p = event;
	int i = 0;
	struct report_status report_status = {0};
	if (!notifier)
		return;
	for (i = 0; i < CHANNEL_NUM; i++) {
		if (!g_ipv6_sync_uid[i])
			continue;

		if (g_other_rx[i] && !g_ipv6_sync_ack_uid[i])
			report_status.status |= 1 << i;
	}

	report_status.uid = g_uid_info.cur_uid;
	restore();
	if (report_status.status == 0) {
		g_uid_info.cur_uid = report_status.uid;
		g_need_check = true;
		g_start_time = jiffies;
		return;
	}

	(void)memset_s(&event, NOTIFY_BUF_LEN, 0, NOTIFY_BUF_LEN);
	// type
	assign_short(p, IPV6_SYNC_ABNORMAL_REPORT_UID);
	skip_byte(p, sizeof(s16));

	// len(2B type + 2B len + 4B uid + 2B status
	assign_short(p, 10);
	skip_byte(p, sizeof(s16));

	// uid
	assign_int(p, report_status.uid);
	skip_byte(p, sizeof(int));

	// 2B status
	assign_short(p, report_status.status);
	skip_byte(p, sizeof(s16));
	notifier((struct res_msg_head *)event);
	pr_info("notify_tcp_collec_event skb\n");
}

void booster_update_data_tx_statistics(u_int8_t af, struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	u32 dev_id = 0;
	struct sock *sk = NULL;
	unsigned long cur_time = 0;

	/* identify the network protocol of an IP and tcp packet */
	spin_lock_bh(&g_ipv6_tcp_sync_collec_lock);
	if (!g_need_check)
		goto unlock;

	if (skb == NULL)
		goto unlock;

	if (skb->dev == NULL)
		goto unlock;

	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		goto unlock;

	cur_time = jiffies;
	if (g_start_time != 0 && cur_time >= g_start_time + CHECK_TIME) {
		notify_tcp_collec_event();
		goto unlock;
	}

	if (match_net_dev(skb, sk, &dev_id) != 0)
		goto unlock;

	if (g_ipv6_sync_uid[dev_id])
		goto unlock;

	if (identify_network_protocol(skb, &ip6h, &iph, &tcph, af) == -1)
		goto unlock;

	if ((ip6h != NULL) && (tcph->syn == 1) && (tcph->ack == 0) &&
		g_uid_info.cur_uid >= 10000 && get_uid_from_sock(sk) == g_uid_info.cur_uid) {
		pr_info("booster_update_data_tx_statistics dev_id=%d\n", dev_id);
		g_ipv6_sync_uid[dev_id] = true;
	}

unlock:
	spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
}

void booster_update_data_tcp_rx_statistics(u_int8_t af, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	u32 dev_id = 0;
	struct sock *sk = NULL;
	unsigned long cur_time = 0;

	/* identify the network protocol of an IP and tcp packet tcp */
	spin_lock_bh(&g_ipv6_tcp_sync_collec_lock);
	if (!g_need_check)
		goto unlock;

	if (skb->dev == NULL)
		goto unlock;

	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		goto unlock;

	if (match_net_dev(skb, sk, &dev_id) != 0)
		goto unlock;

	if (g_ipv6_sync_ack_uid[dev_id])
		goto unlock;

	if (identify_network_protocol(skb, &ip6h, &iph, &tcph, af) == -1)
		goto unlock;

	if ((ip6h != NULL) && (tcph->syn == 1) && (tcph->ack == 1) &&
		g_uid_info.cur_uid >= 10000 && get_uid_from_sock(sk) == g_uid_info.cur_uid) {
		g_ipv6_sync_ack_uid[dev_id] = true;
		pr_info("booster_update_data_tcp_rx_statistics dev_id=%d\n", dev_id);
		goto unlock;
	}

	g_other_rx[dev_id] = true;

unlock:
	spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
}

static int match_net_dev(struct sk_buff *skb, struct sock *sk, u32 *dev_id)
{
	char *dev_name = NULL;
	if (skb == NULL || sk == NULL || dev_id == NULL)
		return -1;
	if (skb->dev == NULL) {
#ifdef CONFIG_HW_WIFIPRO
		if (sk_fullsock(sk) && (sk->sk_family == AF_INET || sk->sk_family == AF_INET6) && sk->sk_protocol == IPPROTO_TCP)
			dev_name = sk->wifipro_dev_name;
#endif
	} else {
		dev_name = skb->dev->name;
	}
	if (dev_name == NULL) {
		pr_info("match_net_dev dev_name null\n");
		return -1;
	}

	*dev_id = 0;
	if (strncmp(dev_name, DS_NET, DS_NET_LEN) == 0)
		*dev_id = DS_NET_ID;
	else if (strncmp(dev_name, DS_NET_SLAVE, DS_NET_LEN) == 0)
		*dev_id = DS_NET_SLAVE_ID;
	else if (strncmp(dev_name, DS_NET_MTK, DS_NET_MTK_PREFIX_LEN) == 0)
		 *dev_id = get_mtk_net_id(dev_name);
	else if (strncmp(dev_name, DS_NET_QCOM, DS_NET_QCOM_PREFIX_LEN) == 0)
		*dev_id = get_qcom_net_id(dev_name);
	else if (strncmp(dev_name, WIFI_NET, WIFI_NET_LEN) == 0)
		*dev_id = WIFI_NET_ID;
	else if (strncmp(dev_name, WIFI_NET_SLAVE, WIFI_NET_LEN) == 0)
		*dev_id = WIFI_NET_SLAVE_ID;
	else if (strncmp(dev_name, WIFI_NET_THIRD, WIFI_NET_LEN) == 0)
		*dev_id = WIFI_NET_THIRD_ID;
	else if (strncmp(dev_name, PHONE_SHELL_NET, PHONE_SHELL_NET_LEN) == 0)
		*dev_id = PHONE_SHELL_NET_ID;
	else
		return -1;
	return 0;
}

static u32 identify_network_protocol(struct sk_buff *skb, struct ipv6hdr **ip6h, struct iphdr **iph, struct tcphdr **tcph, u_int8_t af)
{
	if (af == AF_INET6) {
		*ip6h = ipv6_hdr(skb);
		if (*ip6h == NULL || (*ip6h)->nexthdr != IPPROTO_TCP)
			return -1;
	} else {
		*iph = ip_hdr(skb);
		if (*iph == NULL || (*iph)->protocol != IPPROTO_TCP)
			return -1;
	}
	*tcph = tcp_hdr(skb);
	if (*tcph == NULL || skb->data == NULL || (*tcph)->doff == 0)
		return -1;
	return 0;
}

static unsigned int hook(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	u_int8_t af;

	if (skb == NULL || state == NULL)
		return NF_ACCEPT;
	if (state->pf == NFPROTO_IPV4)
		af = AF_INET;
	else if (state->pf == NFPROTO_IPV6)
		af = AF_INET6;
	else
		af = AF_UNSPEC;

	booster_update_data_tcp_rx_statistics(af, skb, state);
	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
};

static void update_uid(struct req_msg_head *msg, u32 len)
{
	unsigned int size;
	char *p = NULL;
	int tmp_uid = 0;
	int tmp_state = 0;

	if (msg == NULL || msg->len <= sizeof(struct req_msg_head) || len <= sizeof(struct req_msg_head)) {
		pr_err("update_uid msg length too small msg len error!!!\n");
		return;
	}

	size = len - sizeof(struct req_msg_head);
	p = (char *)msg + sizeof(struct req_msg_head);
	if (size < sizeof(int) * 2) {
		pr_err("update_uid msg size too small msg len error!!! %d\n", size);
		return;
	}

	tmp_uid = *(int *)p;
	p += sizeof(int);
	tmp_state = *(int *)p;
	if (tmp_uid < 10000 || (g_uid_info.cur_uid == tmp_uid && tmp_state == g_uid_info.cur_uid_state)) {
		pr_info("update_uid the same status\n");
		return;
	}
	spin_lock_bh(&g_ipv6_tcp_sync_collec_lock);
	if (tmp_state == UID_BACKGROUND && g_uid_info.cur_uid != tmp_uid) {
		spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
		return;
	}
	restore();
	if (tmp_state == UID_FOREGROUND) {
		g_uid_info.cur_uid = tmp_uid;
		g_uid_info.cur_uid_state = tmp_state;
		g_need_check = true;
		g_start_time = jiffies;
		pr_info("update_uid g_uid_info.cur_uid=%d\n", g_uid_info.cur_uid);
	}
	spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
	pr_info("update_uid g_need_check=%d\n", g_need_check);
}

static void process_link_property_update(struct req_msg_head *msg, u32 len)
{
	unsigned int size;

	if (msg == NULL || msg->len <= sizeof(struct req_msg_head) || len <= sizeof(struct req_msg_head)) {
		pr_err("process_link_property_update msg length too small msg len error!!!\n");
		return;
	}

	size = len - sizeof(struct req_msg_head);
	if (size < sizeof(int) * 2) {
		pr_err("process_link_property_update msg size too small msg len error!!! %d\n", size);
		return;
	}

	struct link_property_msg_ex *link_property = (struct link_property_msg_ex *)msg;
	u32 modem_id = link_property->property.modem_id;
	u32 interface_id = link_property->property.interface_id;
	pr_info("%s modem_id:%d, interface name id:%d\n", __func__, modem_id, interface_id);
	if (modem_id != DS_NET_ID && modem_id != DS_NET_SLAVE_ID)
		return;
	spin_lock_bh(&g_ipv6_tcp_sync_collec_lock);
	if (interface_id > DS_INTERFACE_NAME_ID)
		g_link_property[modem_id].interface_id = INVALID_LINK_PROPERTITY_INTERFACE_ID;
	g_link_property[modem_id].interface_id = interface_id;
	spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
}

static void update_screen_status(struct req_msg_head *msg, u32 len)
{
	unsigned int size;
	char *p = NULL;
	int screen_status = 0;

	if (msg == NULL || msg->len <= sizeof(struct req_msg_head) || len <= sizeof(struct req_msg_head)) {
		pr_err("update_screen_status msg length too small msg len error!!!\n");
		return;
	}

	size = len - sizeof(struct req_msg_head);
	if (size < sizeof(short)) {
		pr_err("update_screen_status msg size too small msg len error!!! %d\n", size);
		return;
	}
	p = (char *)msg + sizeof(struct req_msg_head);
	screen_status = *(short *)p;
	if (screen_status <= 0) {
		spin_lock_bh(&g_ipv6_tcp_sync_collec_lock);
		restore();
		spin_unlock_bh(&g_ipv6_tcp_sync_collec_lock);
	}
	pr_info("screen_status=%d\n", screen_status);
}

static void cmd_ipv6_sync_process(struct req_msg_head *msg, u32 len)
{
	pr_info("cmd_ipv6_sync_process  msg->type =%d\n", msg->type);
	switch (msg->type) {
	case KERNEL_MSG_IPV6_SYNC_UPDATE_UID:
		update_uid(msg, len);
		break;
	case KERNEL_MSG_UPDATE_INTERFACE_INFO_IPV6:
		process_link_property_update(msg, len);
		break;
	case KERNEL_MSG_SCREEN_STATUS_CMD:
		update_screen_status(msg, len);
		break;
	default:
		break;
	}
}

msg_process *hw_ipv6_tcp_sync_collec_init(notify_event *notify)
{
	int ret;
	pr_info("hw_ipv6_tcp_sync_collec_init \n");
	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}

	ret = nf_register_net_hooks(&init_net, net_hooks,
		ARRAY_SIZE(net_hooks));
	if (ret) {
		pr_info("nf_init_in error");
		goto init_error;
	}

	notifier = notify;
	return cmd_ipv6_sync_process;

init_error:
	return NULL;
}
