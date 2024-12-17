/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This module is to collect TCP/IP stack parameters
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#include "ip_para_collec.h"

#include <linux/net.h>
#include <linux/list.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "hw_booster_common.h"
#include "ip_para_collec_ex.h"
#include "netlink_handle.h"
#include "data_dubai_collect.h"
#include "tcp_para_collec.h"
#include <securec.h>
#ifdef CONFIG_HW_NETWORK_SLICE
#include "network_slice_management.h"
#endif

#ifdef CONFIG_STREAM_DETECT
#include "stream_detect.h"
#endif

#define TCP_HDR_DOFF_QUAD 4
#define TCP_HDR_IHL_QUAD 4
#define INVALID_LINK_PROPERTITY_INTERFACE_ID 0xFF

#define HEAD_DATA_LEN 12

static struct app_ctx *g_app_ctx = NULL;
static struct link_property_info g_link_property[] = {
	{ DS_NET_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID },
	{ DS_NET_SLAVE_ID, INVALID_LINK_PROPERTITY_INTERFACE_ID }
};
static DEFINE_SPINLOCK(g_ip_para_collec_lock);
static DEFINE_SPINLOCK(g_tcp_reset_lock);
static DEFINE_SPINLOCK(g_tcp_recover_lock);
static u32 g_foreground_uid = 0;
static struct tcp_reset_info g_tcp_reset_info;
bool g_is_enable_stats = false;
int g_dev_id_network_type[NETWORK_NUM] = {-1, -1, -1}; // -1 means main card, second card or wifi is closed
traffic_node_ptr g_traffic_info_head = NULL;
traffic_node_ptr g_traffic_info_tail = NULL;
u32 g_list_num = 0;
u32 g_top_uid[MAX_TOP_UID_NUM] = {0};
u32 g_top_uid_num = 0;
u64 g_total_traffic_len = 0;
u32 g_last_report_time = 0;
unsigned long g_last_recover_time = 0;
static long cache_dns_tx_pkt_sum = 0;
static long cache_dns_rx_pkt_sum = 0;

static int match_net_dev(struct sk_buff *skb, struct sock *sk, u32 *dev_id);
inline u32 get_sock_uid(struct sock *sk);

struct app_ctx *get_ip_para_collec_app_ctx(void)
{
	return g_app_ctx;
}

u32 get_ip_para_collec_foreground_uid(void)
{
	return g_foreground_uid;
}

/*
 * create and init head node of list
 */
static bool create_list()
{
	traffic_node_ptr p_head = NULL;

	if (g_traffic_info_head != NULL) {
		pr_info("[IP_PARA]list has existed, can not create list");
		return false;
	}

	p_head = (traffic_node_ptr)kmalloc(sizeof(traffic_node), GFP_ATOMIC);
	if (p_head == NULL) {
		pr_info("[IP_PARA]create head node failed");
		return false;
	}

	(void)memset_s(p_head, sizeof(traffic_node), 0, sizeof(traffic_node));
	p_head->uid = OTHER_UID;
	p_head->next = NULL;
	g_traffic_info_head = p_head;
	g_traffic_info_tail = p_head;

	pr_info("[IP_PARA]create list success");
	return true;
}

/*
 * search for node which have the same id
 */
static traffic_node_ptr find_list(int id)
{
	traffic_node_ptr p = g_traffic_info_head->next;
	while (p != NULL && p->uid != id)
		p = p->next;
	return p;
}

/*
 * insert a node at the end of list with certain id
 */
static traffic_node_ptr insert_tail(int id)
{
	traffic_node_ptr tmp = NULL;

	if (g_traffic_info_tail == NULL) {
		pr_info("[IP_PARA]tail node is not found");
		return NULL;
	}

	tmp = (traffic_node_ptr)kmalloc(sizeof(traffic_node), GFP_ATOMIC);
	if (tmp == NULL) {
		pr_info("[IP_PARA]insert tail failed");
		return NULL;
	}

	(void)memset_s(tmp, sizeof(traffic_node), 0, sizeof(traffic_node));
	tmp->uid = id;
	tmp->next = NULL;

	g_traffic_info_tail->next = tmp;
	g_traffic_info_tail = tmp;
	g_list_num++;

	pr_info("[IP_PARA]insert tail success");
	return tmp;
}

/*
 * delete all of node except the head node
 */
static void delete_list()
{
	traffic_node_ptr p = NULL;
	traffic_node_ptr tmp = NULL;

	if (g_traffic_info_head == NULL) {
		pr_info("[IP_PARA]tail node is not found");
		return;
	}

	p = g_traffic_info_head;
	tmp = g_traffic_info_head->next;
	while (tmp != NULL) {
		p->next = tmp->next;
		kfree(tmp);
		tmp = p->next;
	}

	(void)memset_s(g_traffic_info_head, sizeof(traffic_node), 0, sizeof(traffic_node));
	g_traffic_info_head->uid = OTHER_UID;
	g_traffic_info_head->next = NULL;
	g_traffic_info_tail = g_traffic_info_head;
	g_list_num = 0;
	pr_info("[IP_PARA]delete list success");
}

/*
 * send traffic info msg to the booster, and free the memory
 */
static void notify_traffic_info_process(char *event)
{
	if (event != NULL && g_app_ctx->fn)
		g_app_ctx->fn((struct res_msg_head *)event);
}

/*
 * send traffic info msg to the booster, and delete the list of traffic info
 */
static void notify_traffic_info(char *p, u16 notify_len)
{
	int i;
	u32 tmp_traffic_len, tmp_traffic_duration;
	traffic_node_ptr cur_traffic_info_ptr = NULL;

	// type
	assign_short(p, TRAFFIC_STATS_INFO_RPT);
	skip_byte(p, sizeof(u16));
	pr_info("[IP_PARA]notify_type: %d", TRAFFIC_STATS_INFO_RPT);
	// len(2B type + 2B len + 4B uid_num + 52B * (g_list_num + 1) traffic_info)
	assign_short(p, notify_len);
	skip_byte(p, sizeof(u16));
	assign_uint(p, g_list_num + 1);
	skip_byte(p, sizeof(u32));

	cur_traffic_info_ptr = g_traffic_info_head;
	while (cur_traffic_info_ptr != NULL) {
		assign_uint(p, cur_traffic_info_ptr->uid);
		skip_byte(p, sizeof(u32));
		for (i = 0; i < RAT_NUM_MAX; i++) {
			tmp_traffic_len = (u32)(cur_traffic_info_ptr->traffic_len[i] / LEN_ONE_KILOBYTE);
			tmp_traffic_duration = cur_traffic_info_ptr->traffic_duration[i] / DURATION_ONE_SEC;
			if (tmp_traffic_len == 0 || tmp_traffic_duration == 0) {
				tmp_traffic_len = 0;
				tmp_traffic_duration = 0;
			}
			assign_uint(p, tmp_traffic_len);
			skip_byte(p, sizeof(u32));
			assign_uint(p, tmp_traffic_duration);
			skip_byte(p, sizeof(u32));
		}
		cur_traffic_info_ptr = cur_traffic_info_ptr->next;
	}

	delete_list();
	g_total_traffic_len = 0;
	g_last_report_time = jiffies_to_msecs(jiffies_64);
}

/*
 * judge if socket is ipv6 packet
 */
bool is_v6_sock(struct sock *sk)
{
	if (sk->sk_family == AF_INET6 &&
		!(ipv6_addr_type(&sk->sk_v6_daddr) & IPV6_ADDR_MAPPED) &&
		!(ipv6_addr_type(&sk->sk_v6_rcv_saddr) & IPV6_ADDR_MAPPED))
		return true;
	return false;
}

/*
 *  update traffic info of each different uid
 */
static void update_traffic_info_process(const traffic_node_ptr match_traffic_info_ptr, u32 dev_id, int len)
{
	u32 cur_time = jiffies_to_msecs(jiffies_64);
	u32 duration;

	// 0 sa, 1 nsa, 2 nsa_no_endc, 3 lte, 4 other, 5 wifi
	match_traffic_info_ptr->traffic_len[g_dev_id_network_type[dev_id]] += len;
	duration = cur_time - match_traffic_info_ptr->last_pkt_time[dev_id];
	if (duration > 0 && duration < MAX_TRAFFIC_DURATION &&
		match_traffic_info_ptr->last_pkt_time[dev_id] != 0)
		match_traffic_info_ptr->traffic_duration[g_dev_id_network_type[dev_id]] += duration;
	match_traffic_info_ptr->last_pkt_time[dev_id] = cur_time;

	g_total_traffic_len += len;
}

/*
 *  update traffic info from tethering
 */
static void update_tethering_traffic_info(struct sk_buff *skb)
{
	u32 dev_id;
	traffic_node_ptr match_traffic_info_ptr = NULL;
	spin_lock_bh(&g_ip_para_collec_lock);
	if (g_dev_id_network_type[WIFI_NET_ID] == -1)
		dev_id = DS_NET_ID;
	else
		dev_id = WIFI_NET_ID;

	if (g_dev_id_network_type[dev_id] < 0 || g_dev_id_network_type[dev_id] >= RAT_NUM_MAX) {
		pr_info("[IP_PARA]update tethering traffic info dev id network type error");
		spin_unlock_bh(&g_ip_para_collec_lock);
		return;
	}

	match_traffic_info_ptr = g_traffic_info_head;
	if (match_traffic_info_ptr == NULL) {
		pr_info("[IP_PARA]match node is not found");
		spin_unlock_bh(&g_ip_para_collec_lock);
		return;
	}
	update_traffic_info_process(match_traffic_info_ptr, dev_id, skb->len);
	spin_unlock_bh(&g_ip_para_collec_lock);
}

/*
 * update traffic info from TCP and UDP
 */
static void update_traffic_info(struct sk_buff *skb, struct sock *sk, int len, u32 uid)
{
	int i;
	u32 dev_id;
	bool is_top_uid = false;
	traffic_node_ptr match_traffic_info_ptr = NULL;

	if (match_net_dev(skb, sk, &dev_id) != RTN_SUCC) {
		pr_info("[IP_PARA]update traffic info get dev id failed");
		return;
	}
	if (dev_id == WIFI_NET_SLAVE_ID)
		dev_id = WIFI_NET_ID; // statistics wifi regardless of main and second
	if (dev_id < 0 || dev_id > WIFI_NET_ID) {
		pr_info("[IP_PARA]update traffic info dev id error");
		return;
	}
	if (g_dev_id_network_type[dev_id] < 0 || g_dev_id_network_type[dev_id] >= RAT_NUM_MAX) {
		pr_info("[IP_PARA]update traffic info dev id network type error");
		return;
	}

	uid = uid % UID_MASK; // do not distinguish userid, the application clone is counted to the main application
	for (i = 0; i < g_top_uid_num; i++) {
		if (uid == g_top_uid[i]) {
			is_top_uid = true;
			break;
		}
	}

	if (is_top_uid == false) {
		match_traffic_info_ptr = g_traffic_info_head; // if not found, the statistics are otherapp in the head node
	} else {
		match_traffic_info_ptr = find_list(uid);
		if (match_traffic_info_ptr == NULL)
			match_traffic_info_ptr = insert_tail(uid);
	}

	if (match_traffic_info_ptr == NULL) {
		pr_info("[IP_PARA]match node is not found");
		return;
	}
	update_traffic_info_process(match_traffic_info_ptr, dev_id, len);
}

/*
 * hook packet from certain hook point, and judge if it's time to send msg to booster
 */
static void statistics_traffic(u8 protocol, struct sk_buff *skb, struct sock *sk, u8 direction)
{
	char *event = NULL;
	u32 uid = get_sock_uid(sk);
	u32 cur_time;
	u16 notify_len;

	spin_lock_bh(&g_ip_para_collec_lock);
	if (protocol == IPPROTO_TCP && (direction == NF_INET_LOCAL_IN || direction == NF_INET_POST_ROUTING)) {
		update_traffic_info(skb, sk, skb->len, uid);
	} else if (protocol == IPPROTO_UDP) {
		if (direction == NF_INET_UDP_IN_HOOK) {
			int len = (is_v6_sock(sk) == false) ? (skb->len + IPV4_HEAD_LEN) : (skb->len + IPV6_HEAD_LEN);
			update_traffic_info(skb, sk, len, uid);
		} else if (direction == NF_INET_POST_ROUTING) {
			update_traffic_info(skb, sk, skb->len, uid);
		}
	}

	cur_time = jiffies_to_msecs(jiffies_64);
	if ((g_total_traffic_len > REPORT_TRAFFIC_LEN_THRESHOLD &&
		cur_time - g_last_report_time > REPORT_TRAFFIC_DURATION_THRESHOLD) ||
		cur_time - g_last_report_time > MAX_REPORT_DURATION_CYCLE) {
		pr_info("[IP_PARA]notify after meeting the threshold");
		notify_len = sizeof(u16) + sizeof(u16) + sizeof(u32) + LEN_PER_UID * (g_list_num + 1);
		event = (char *)kmalloc(notify_len, GFP_ATOMIC);
		if (event != NULL) {
			(void)memset_s(event, notify_len, 0, notify_len);
			notify_traffic_info(event, notify_len);
		}
	}
	spin_unlock_bh(&g_ip_para_collec_lock);

	notify_traffic_info_process(event);
	if (event != NULL)
		kfree(event);
}

inline u32 get_sock_uid(struct sock *sk)
{
	return (u32)sk->sk_uid.val;
}

#if defined(CONFIG_HUAWEI_KSTATE)
inline u32 get_sock_pid(struct sock *sk)
{
	if (sk->sk_socket == NULL)
		return 0;
	return (u32)sk->sk_socket->pid;
}
#endif

static u32 get_current_srtt(struct tcp_sock *tp)
{
	u32 rtt_ms;
	u32 rcv_rtt_us = 0;
	u32 time;

	if (tp == NULL)
		return 0;

	if (tp->bytes_received < tp->content_length &&
		tp->key_flow &&
		tp->rx_opt.rcv_tsecr)
			rcv_rtt_us = tp->rcv_rtt_est.rtt_us;

	rtt_ms = tp->srtt_us > rcv_rtt_us ?
			tp->srtt_us : rcv_rtt_us;

	time = jiffies_to_msecs(tcp_jiffies32 - tp->rtt_update_tstamp);
	if (time < TIME_TEN_SEC)
		rtt_ms = rtt_ms > tp->prior_srtt_us ?
			rtt_ms : tp->prior_srtt_us;

	rtt_ms = rtt_ms / MULTIPLE_OF_RTT / US_MS;
	rtt_ms = (rtt_ms > MAX_RTT) ? MAX_RTT : rtt_ms;
	return rtt_ms;
}

static bool is_local_or_lan_addr(__be32 ip_addr)
{
	if (ipv4_is_loopback(ip_addr) ||
		ipv4_is_multicast(ip_addr) ||
		ipv4_is_local_multicast(ip_addr) ||
		ipv4_is_lbcast(ip_addr) ||
		ipv4_is_private_10(ip_addr) ||
		ipv4_is_private_172(ip_addr) ||
		ipv4_is_private_192(ip_addr) ||
		ipv4_is_linklocal_169(ip_addr))
		return true;
	else
		return false;
}

static bool is_private_v6_addr(struct sock *sk)
{
	const unsigned int clat_ipv4_index = 3;
	__be32 addr;
	int addr_type;
#if IS_ENABLED(CONFIG_IPV6)
	if (sk == NULL)
		return false;
	addr_type = ipv6_addr_type(&sk->sk_v6_daddr);
	if ((sk->sk_family == AF_INET6) &&
		(addr_type & IPV6_ADDR_MAPPED)) {
		addr = sk->sk_v6_daddr.s6_addr32[clat_ipv4_index];
		return is_local_or_lan_addr(addr);
	}
#endif
	return false;
}

static bool is_sk_daddr_private(struct sock *sk)
{
	if (sk->sk_family == AF_INET6)
		return is_private_v6_addr(sk);
	else if (sk->sk_family == AF_INET)
		return is_local_or_lan_addr(sk->sk_daddr);

	return false;
}

static u32 is_first_pkt_of_period(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	tp = tcp_sk(sk);
	if (tp == NULL || g_app_ctx == NULL ||
		g_app_ctx->report_tstamp == 0)
		return false;

	if (tp->last_pkt_tstamp == 0 && sk->sk_state == TCP_SYN_SENT)
		return true;

	return before(tp->last_pkt_tstamp, g_app_ctx->report_tstamp);
}

static bool is_sk_fullsock(struct sock *sk)
{
	if (sk != NULL && sk_fullsock(sk))
		return true;

	return false;
}

static bool match_key_processes_flow(unsigned short key_pid, struct sock * sk)
{
#if defined(CONFIG_HUAWEI_KSTATE)
	u32 pid = get_sock_pid(sk);
	if (key_pid == 0 || pid == 0)
		return false;
	if (pid == key_pid)
		return true;
#endif
	return false;
}

static bool match_flow_type(struct sock *sk, struct sk_buff *skb, u16 hook_type, u32 flow_type)
{
	struct tcphdr *th = tcp_hdr(skb);

	if (!(hook_type == NF_INET_LOCAL_IN))
		return false;

	if (th == NULL || skb->data == NULL)
		return false;

#ifdef CONFIG_STREAM_DETECT
	u32 payloadlen = 0;
	u8 *vaddr = NULL;
	u8 *payload = hw_get_payload_addr(skb, IPPROTO_TCP, &vaddr, &payloadlen); // need unmap_vaddr
	if (payloadlen == 0 || payload == NULL) {
		hw_unmap_vaddr(vaddr);
		return false;
	}
	struct stream_info info = {0};
	info.dest_port = (u32)htons(th->source);
	bool match = is_target_stream(sk, payload, payloadlen, flow_type, info);
	hw_unmap_vaddr(vaddr);
	return match;
#else
	return false;
#endif
}

static bool match_booster_key_flow_ip(struct sk_buff *skb, struct booster_key_flow *info)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ipv6h = NULL;

	if (skb == NULL || iph == NULL || info == NULL) {
		pr_err("[IP_PARA] %s:skb or iphdr or booster info == NULL\n", __func__);
		return false;
	}

	if (skb->protocol == ntohs(ETH_P_IP)) {
		iph = ip_hdr(skb);
		if (iph->daddr == info->dest_addr.ip4_addr) {
			pr_info("[IP_PARA]ipv4 address already matched");
			return true;
		}
	} else if (skb->protocol == ntohs(ETH_P_IPV6)) {
		ipv6h = ipv6_hdr(skb);
		if (ipv6_addr_equal(&ipv6h->daddr, &info->dest_addr.ip6_addr)) {
			pr_info("[IP_PARA]ipv4 address already matched");
			return true;
		}
	} else {
		pr_info("[IP_PARA]abnormal protocol type");
		return false;
	}
	return false;
}

static bool match_udp_port(struct sk_buff *skb, struct udphdr *uh, struct booster_key_flow *info)
{
	if (skb == NULL || uh == NULL || info == NULL) {
		pr_err("[IP_PARA] %s:udph or booster info == NULL\n", __func__);
		return false;
	}

	pr_info("[IP_PARA] kernel port is : %d", uh->dest);
	pr_info("[IP_PARA] booster port is : %d", info->dest_port);
	if (uh->dest == info->dest_port) {
		pr_info("[IP_PARA]UDP port already matched");
		return match_booster_key_flow_ip(skb, info);
	}
	return false;
}

static bool match_tcp_port(struct sk_buff *skb, struct tcphdr *th, struct booster_key_flow *info)
{
	if (skb == NULL || th == NULL || info == NULL) {
		pr_err("[IP_PARA] %s:tcph or booster info == NULL\n", __func__);
		return false;
	}

	if (th->dest == info->dest_port) {
		pr_info("[IP_PARA]TCP port already matched");
		return match_booster_key_flow_ip(skb, info);
	}
	return false;
}

static bool match_booster_key_flow(struct sock *sk, struct sk_buff *skb, struct booster_key_flow *info)
{
	u32 uid;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;

	if (sk == NULL || skb == NULL || info == NULL) {
		pr_err("[IP_PARA] %s:sock or skb or booster info == NULL\n", __func__);
		return false;
	}

	uid = get_sock_uid(sk);
	if (uid != info->uid)
		return false;

	if (sk->sk_protocol == IPPROTO_TCP) {
		th = tcp_hdr(skb);
		return match_tcp_port(skb, th, info);
	} else if (sk->sk_protocol == IPPROTO_UDP) {
		uh = udp_hdr(skb);
		return match_udp_port(skb, uh, info);
	}
	return false;
}

static bool match_key_flow(struct key_flow_info *info, struct sock *sk, struct sk_buff *skb, u16 hook_type)
{
	switch (info->type) {
	case VIDEO_FLOW:
	case IMAGE_FLOW:
		return match_flow_type(sk, skb, hook_type, info->type);
	case KEY_PROCESSES_FLOW:
		return match_key_processes_flow(info->field.pid, sk);
	case BOOSTER_KEY_FLOW:
		return false;
	default:
		return false;
	}
}

static bool is_key_flow(struct sock *sk, struct sk_buff *skb, u16 hook_type, u8 protocol)
{
	struct tcp_sock *tp = NULL;
	u32 uid;
	u32 i;
	struct key_flow *flow = &g_app_ctx->flow;
	struct booster_key_flow *new_flow = &g_app_ctx->new_flow;

	if (!is_sk_fullsock(sk) || protocol != IPPROTO_TCP)
		return false;

	uid = get_sock_uid(sk);
	tp = tcp_sk(sk);

	if (uid != g_app_ctx->fore_app_uid)
		return false;

	if (tp->key_flow)
		return true;

	for (i = 0; i < flow->cnt; i++)
		if (match_key_flow(&flow->info[i], sk, skb, hook_type)) {
			tp->key_flow = 1;
			pr_info("[IP_PARA]%s:already match key flow:%u\n",
			__func__, flow->info[i].type);
			return true;
		}

	if (match_booster_key_flow(sk, skb, new_flow)) {
		tp->key_flow = 1;
		pr_info("[IP_PARA]: already match booster key flow:");
		return true;
	}
	return false;
}

static bool is_key_flow_new(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = NULL;
	struct booster_key_flow *new_flow = &g_app_ctx->new_flow;

	if (sk == NULL || skb == NULL)
		return false;

	if (is_sk_fullsock(sk) || sk->sk_state != TCP_ESTABLISHED)
		return false;

	tp = tcp_sk(sk);
	if (match_booster_key_flow(sk, skb, new_flow)) {
		tp->key_flow = 1;
		pr_info("[IP_PARA]: already match booster key flow");
		return true;
	}
	pr_info("[IP_PARA]: can not match booster key flow");
	return false;
}

static bool need_update_rtt_ofo(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	u32 time;

	if (!is_sk_fullsock(sk) || is_sk_daddr_private(sk))
		return false;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return false;

	tp = tcp_sk(sk);
	time = jiffies_to_usecs(tcp_jiffies32 - tp->ofo_tstamp) *
		MULTIPLE_OF_RTT;
	if (tp->ofo_tstamp == 0)
		return false;

	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue)) {
		if (before(tp->rcv_nxt_ofo, tp->rcv_nxt) &&
			tp->rcv_nxt_ofo != 0)
			return true;
		else if (tp->prior_srtt_us < MAX_RTT_US &&
			time > tp->prior_srtt_us + OFO_RTT_UPDATE_THRESHOLD_US)
			return true;
	} else {
		if (tp->rcv_nxt_ofo != 0 &&
			tp->ofo_tstamp != 0)
			return true;
	}

	return false;
}

void update_ofo_rtt_for_qoe(struct sock *sk)
{
	if (!need_update_rtt_ofo(sk))
		return;
	update_tcp_para_without_skb(sk, NF_INET_OFO_HOOK);
}

void dec_sk_num_for_qoe(struct sock *sk, int new_state)
{
	int old_state;

	if (!is_sk_fullsock(sk))
		return;

	old_state = sk->sk_state;
	if ((old_state == TCP_ESTABLISHED ||
		old_state == TCP_SYN_SENT) &&
		new_state != TCP_ESTABLISHED &&
		new_state != TCP_SYN_SENT)
		update_tcp_para_without_skb(sk, NF_INET_DELETE_SK_HOOK);
}

static void increace_sk_num(struct sock *sk, struct tcp_res_info *stat)
{
	struct tcp_sock *tp = NULL;
	u32 rtt_ms;

	if (!is_sk_fullsock(sk))
		return;
	if (stat == NULL)
		return;
	tp = tcp_sk(sk);
	if (tp == NULL)
		return;

	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;

	if (is_first_pkt_of_period(sk)) {
		if (tp->last_pkt_tstamp != g_app_ctx->report_tstamp)
			tp->last_pkt_tstamp = g_app_ctx->report_tstamp;

		stat->tcp_sk_num++;
		rtt_ms = get_current_srtt(tp);
		tp->prior_srtt_us = rtt_ms * MULTIPLE_OF_RTT * US_MS;
		stat->sk_rtt_sum += rtt_ms;
	}
}

static void clear_prior_rtt(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);
	if (tp->rtt_update_tstamp == 0)
		return;
	tp->prior_srtt_us = 0;
	tp->rtt_update_tstamp = 0;
}

static void update_sk_rtt_sum_common(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;

	tp = tcp_sk(sk);
	tp->prior_srtt_us = (tp->srtt_us > MAX_RTT_US) ?
		MAX_RTT_US : tp->srtt_us;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

static void update_sk_rtt_retrans_common(struct sock *sk)
{
	struct inet_connection_sock *icsk = NULL;
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;
	tp = tcp_sk(sk);
	icsk = inet_csk(sk);
	if (icsk == NULL)
		return;

	tp->prior_srtt_us = jiffies_to_usecs(icsk->icsk_rto) * MULTIPLE_OF_RTT;
	tp->prior_srtt_us = (tp->prior_srtt_us > MAX_RTT_US_RETRANS) ?
		MAX_RTT_US_RETRANS : tp->prior_srtt_us;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

static void update_sk_rtt_ofo_common(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	u32 time;

	if (!is_sk_fullsock(sk))
		return;
	if (sk->sk_state != TCP_ESTABLISHED &&
		sk->sk_state != TCP_SYN_SENT)
		return;
	tp = tcp_sk(sk);

	time = jiffies_to_usecs(tcp_jiffies32 - tp->ofo_tstamp) *
		MULTIPLE_OF_RTT;
	tp->prior_srtt_us = (time > MAX_RTT_US) ?
		MAX_RTT_US : time;
	tp->rtt_update_tstamp = tcp_jiffies32;
}

static void tcp_should_enter_recover(struct sock *sk)
{
	struct tcp_sock *tp = NULL;
	struct sk_buff *ooo_first_skb = NULL;

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);
	if (tp == NULL)
		return;

	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue)
		&& (tp->ooo_last_skb != NULL)
		&& (tp->ofo_tstamp != 0)
		&& (tp->last_rcv_nxt != 0)
		&& before(tp->last_rcv_nxt, tp->rcv_nxt)
		&& before(tp->ofo_tstamp, tcp_jiffies32)) {
		ooo_first_skb = skb_rb_first(&tp->out_of_order_queue);
		if (ooo_first_skb == NULL || !before(tp->rcv_nxt, TCP_SKB_CB(ooo_first_skb)->seq)) {
			tp->should_recover = 0;
			tp->should_enter_recover = 0;
			return;
		}

		if (jiffies_to_msecs(tcp_jiffies32 - tp->ofo_tstamp) > DURATION_ONE_SEC) {
			u32 estimate_time =
			(TCP_SKB_CB(ooo_first_skb)->seq - tp->rcv_nxt) /
			(tp->rcv_nxt - tp->last_rcv_nxt) * jiffies_to_msecs(tcp_jiffies32 - tp->ofo_tstamp);
			pr_err("tcp_recover:ooo_first_skb seq:%u ooo_last_skb seq:%u rcv_nxt:%u last_rcv_nxt:%u "
				"usedtime:%u estimatetime:%u\n", TCP_SKB_CB(ooo_first_skb)->seq,
				TCP_SKB_CB(tp->ooo_last_skb)->seq, tp->rcv_nxt, tp->last_rcv_nxt,
				jiffies_to_msecs(tcp_jiffies32 - tp->ofo_tstamp), estimate_time);
			if (estimate_time > TCP_RECOVER_ESTIMATE_TIME_THR) {
				tp->should_recover = 1;
				spin_lock_bh(&g_tcp_recover_lock);
				g_last_recover_time = jiffies;
				spin_unlock_bh(&g_tcp_recover_lock);
				return;
			} else {
				tp->should_recover = 0;
				tp->should_enter_recover = 0;
			}
		}
	} else {
		tp->should_recover = 0;
		tp->should_enter_recover = 0;
	}
	return;
}

void update_should_enter_recover(struct sock *sk, u32 seq)
{
	struct tcp_sock *tp = NULL;
	struct tcphdr *th = NULL;
	u32 rtt_ms = 0;
	u32 ofo_rtt_ms = 0;

	spin_lock_bh(&g_tcp_recover_lock);
	if (g_last_recover_time != 0 && time_before(jiffies, g_last_recover_time + TCP_RECOVER_FORBIDDEN_TIME)) {
		spin_unlock_bh(&g_tcp_recover_lock);
		return;
	}
	spin_unlock_bh(&g_tcp_recover_lock);

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);
	if (tp == NULL)
		return;

	if (sk->sk_state != TCP_ESTABLISHED)
		return;

	if (tp->should_recover)
		return;

	if (tp->should_enter_recover)
		goto recover_check;

	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue) && tp->ofo_tstamp != 0 && tp->ooo_last_skb != NULL) {
		th = tcp_hdr(tp->ooo_last_skb);
		if (th == NULL || ntohs(th->source) != TCP_RECOVER_PORT)
			return;
		rtt_ms = tp->srtt_us / MULTIPLE_OF_RTT / US_MS;
		ofo_rtt_ms = tp->prior_srtt_us / MULTIPLE_OF_RTT / US_MS;

		if (rtt_ms > 0 && rtt_ms < TIME_MAX_RECOVER_SRTT && ofo_rtt_ms > TIME_MAX_RECOVER_OFO_RTT) {
			pr_info("tcp_recover: should enter_recover.seq:%u rcv_nxt:%u last_rcv_nxt:%u ofo_rtt_ms:%u "
				"rtt_ms:%u port:%u \n", seq, tp->rcv_nxt, tp->last_rcv_nxt, ofo_rtt_ms, rtt_ms,
				sk->sk_num);
			goto recover_check;
		}
	}

	tp->should_enter_recover = 0;
	tp->should_recover = 0;
	return;

recover_check:
	tp->should_enter_recover = 1;
	tcp_should_enter_recover(sk);
}

void update_ofo_tstamp_for_qoe(struct sock *sk)
{
	struct tcp_sock *tp = NULL;

	if (!is_sk_fullsock(sk))
		return;

	tp = tcp_sk(sk);
	if (!RB_EMPTY_ROOT(&tp->out_of_order_queue)) {
		if (tp->rcv_nxt_ofo == 0 &&
			tp->ofo_tstamp == 0) {
			tp->rcv_nxt_ofo = tp->rcv_nxt + tp->rcv_wnd;
			tp->ofo_tstamp = tcp_jiffies32;
			tp->last_rcv_nxt = tp->rcv_nxt;
		} else if (before(tp->rcv_nxt_ofo, tp->rcv_nxt) &&
			tp->rcv_nxt_ofo != 0) {
			tp->rcv_nxt_ofo = tp->rcv_nxt + tp->rcv_wnd;
			tp->ofo_tstamp = tcp_jiffies32;
			tp->last_rcv_nxt = tp->rcv_nxt;
		}
	} else {
		if (tp->ofo_tstamp != 0) {
			tp->rcv_nxt_ofo = 0;
			tp->ofo_tstamp = 0;
			tp->last_rcv_nxt = 0;
		}
	}
}

static bool is_dns_sk(struct sock *sk, u8 protocol)
{
	if (sk == NULL)
		return false;

	if (protocol != IPPROTO_UDP)
		return false;

	if (htons(inet_sk(sk)->inet_dport) == DNS_PORT)
		return true;

	return false;
}

/*
 * the udp pkts from hooks which pid and uid can get.
 *
 * hooks 1  2  3  4
 * sock  0  0  Y  Y
 * UID   N  N  y  y
 * PID   N  N  N  N
 *
 * the tcp pkts from hooks which pid and uid can get.
 *
 * hooks 1  2  3  4
 * sock  1  0  1  1
 * UID   y  N  Y  Y
 * PID   N  N  Y  Y
 */
static u32 match_app(struct sock *sk, struct sk_buff *skb,
	struct tcp_res *stat, u8 protocol, u16 hook_type)
{
	u32 uid;

	if (sk == NULL || skb == NULL || stat == NULL)
		return FORGROUND_UNMACH;

	// forward
	if (is_key_flow_new(sk, skb))
		return FORGROUND_MACH; // booster-key flow
	if (stat->uid == 0) // 1-background uid: 0
		return BACKGROUND;
	if (stat->uid == DNS_UID && is_dns_sk(sk, protocol)) // 2-DNS protocol port:53(only UDP)
		return FORGROUND_MACH;
	if ((protocol == IPPROTO_TCP) && (!sk_fullsock(sk)))
		return FORGROUND_UNMACH; // ignore timewait or request socket
	if (stat->uid == KEY_FLOW_UID && is_key_flow(sk, skb, hook_type, protocol))
		return FORGROUND_MACH; // 3-key flow
	uid = get_sock_uid(sk);
	if (uid == stat->uid)
		return FORGROUND_MACH; // 4-others and uid, foreground

	return FORGROUND_UNMACH;
}

static void set_dev_name(struct sock *sk, const struct sk_buff *skb)
{
#ifdef CONFIG_HW_WIFIPRO
	unsigned int dev_max_len;
#endif

	if (sk == NULL || skb == NULL || skb->dev == NULL)
		return;

#ifdef CONFIG_HW_WIFIPRO
	if ((1 << sk->sk_state) & TCPF_SYN_SENT) {
		dev_max_len = strnlen(skb->dev->name,
			IFNAMSIZ - 1);
		strncpy(sk->wifipro_dev_name, skb->dev->name,
			dev_max_len);
		sk->wifipro_dev_name[dev_max_len] = '\0';
	}
#endif
}

void update_stats_srtt(struct tcp_sock *tp, u32 *rtt)
{
	u32 rtt_ms;
	if (rtt == NULL || tp == NULL)
		return;

	rtt_ms = get_current_srtt(tp);
	*rtt += rtt_ms;
}

static void update_post_routing_para(struct tcp_res_info *stat,
	struct sk_buff *skb, struct sock *sk, struct tcphdr *th, s32 tcp_len)
{
	struct tcp_sock *tp = NULL;

	tp = (struct tcp_sock *)sk;
	if (sk->sk_state == TCP_ESTABLISHED) {
		stat->out_pkt++;
		stat->out_len += skb->len;
	} else if (sk->sk_state == TCP_SYN_SENT) {
		if (th->syn == 1) {
			stat->tcp_syn_pkt++;
			set_dev_name(sk, skb);
		}
	} else {
		if (th->fin == 1)
			stat->tcp_fin_pkt++;
	}
	if (tp->snd_nxt != 0 &&
		before(ntohl(th->seq), tp->snd_nxt) &&
		(sk->sk_state == TCP_ESTABLISHED ||
		sk->sk_state == TCP_SYN_SENT)) {
		stat->out_rts_pkt++;
		if (sk->sk_state == TCP_SYN_SENT)
			stat->tcp_syn_rts_pkt++;
	}
	increace_sk_num(sk, stat);
}

static int get_sock_index(struct sock *sk)
{
	int i;

	if (g_tcp_reset_info.record_index >= MAX_TCP_SOCK_INFO_SIZE - 1)
		return -1;

	for (i = 0; i <= g_tcp_reset_info.record_index; i++) {
		if (g_tcp_reset_info.sock_info[i] == sk)
			return -1;
	}
	return i;
}

static void update_tcp_reset_count(const struct sk_buff *skb, struct sock *sk, const struct tcphdr *th)
{
	int index = -1;
	int uid = 0;

	if (skb == NULL || sk == NULL || th == NULL)
		return;

	if (!(sk->sk_state == TCP_ESTABLISHED || sk->sk_state == TCP_SYN_SENT) || th->rst != 1 || !is_ds_rnic(skb->dev))
		return;

	uid = sk->sk_uid.val;
	spin_lock_bh(&g_tcp_reset_lock);
	if (uid < IGNORE_UID || uid != g_foreground_uid) {
		spin_unlock_bh(&g_tcp_reset_lock);
		return;
	}

	if (g_tcp_reset_info.report_cnt >= MAX_REPORT_TCP_RESET_CNT) {
		spin_unlock_bh(&g_tcp_reset_lock);
		return;
	}

	index = get_sock_index(sk);
	if (index < 0 || index >= MAX_TCP_SOCK_INFO_SIZE) {
		spin_unlock_bh(&g_tcp_reset_lock);
		return;
	}

	if (is_v6_sock(sk)) {
		if (sk->sk_state == TCP_ESTABLISHED)
			g_tcp_reset_info.established_v6_count++;
		else
			g_tcp_reset_info.syn_v6_count++;
	} else {
		if (sk->sk_state == TCP_ESTABLISHED)
			g_tcp_reset_info.established_v4_count++;
		else
			g_tcp_reset_info.syn_v4_count++;
	}
	if (get_sock_uid(sk) == g_foreground_uid)
		data_dubai_update_syn(sk);
	g_tcp_reset_info.record_index = index;
	g_tcp_reset_info.sock_info[index] = sk;
	spin_unlock_bh(&g_tcp_reset_lock);
}

/*
 * tcp pkt_in pid=0(default),uid is ok, pkt_out pid,uid is ok.
 */
static void update_tcp_para(struct tcp_res_info *stat, struct sk_buff *skb,
	struct sock *sk, u8 direction, u16 payload_len)
{
	struct tcphdr *th = tcp_hdr(skb);
	struct tcp_sock *tp = NULL;
	s32 tcp_len;
	u32 rtt_ms;

	if (direction == NF_INET_FORWARD) // sk == NULL
		stat->out_all_pkt++;
	if (sk == NULL)
		return;
	if (th == NULL)
		return;
	if (direction == NF_INET_BUFHOOK)
		return;
	tp = (struct tcp_sock *)sk;
	if (tp == NULL)
		return;
	tcp_len = payload_len - th->doff * TCP_HDR_DOFF_QUAD; // 32bits to byte
	if (tcp_len < 0)
		return;
	if (direction == NF_INET_LOCAL_IN) {
		update_tcp_reset_count(skb, sk, th);
		increace_sk_num(sk, stat);
		if (sk->sk_state != TCP_ESTABLISHED)
			return;
		stat->in_pkt++;
		stat->in_len += skb->len;
		if (before(ntohl(th->seq) + tcp_len - 1, tp->rcv_nxt) &&
			tcp_len > 0)
			stat->in_rts_pkt++;

		update_stats_srtt(tp, &stat->rtt);
		if (get_sock_uid(sk) == g_foreground_uid) {
			data_dubai_update_tcp_in_pkt();
			rtt_ms = get_current_srtt(tp);
			data_dubai_update_rtt(rtt_ms);
		}
	} else if (direction == NF_INET_POST_ROUTING) {
		update_post_routing_para(stat, skb, sk, th, tcp_len);
	} else if (direction == NF_INET_LOCAL_OUT) {
		stat->out_all_pkt++;
	}
}

/*
 * udp udp_in uid=0, udp_out pid=0(or any pid,uid is ok),uid is ok
 */
void update_udp_para(struct tcp_res_info *stat, struct sk_buff *skb,
	u8 direction)
{
	if (direction == NF_INET_UDP_IN_HOOK) {
		stat->in_udp_pkt++;
		stat->in_udp_len += skb->len;
	} else if (direction == NF_INET_POST_ROUTING) {
		stat->out_udp_pkt++;
		stat->out_udp_len += skb->len;
	} else if (direction == NF_INET_FORWARD || direction == NF_INET_LOCAL_OUT) {
		stat->out_all_pkt++;
	}
}

static unsigned int get_mtk_net_id(const char *dev_name)
{
	unsigned int dev_id = 0;

	if (dev_name == NULL || strlen(dev_name) != DS_NET_MTK_LEN)
		return DS_NET_ID;

	// devName index is in [0, 9]
	if (dev_name[DS_NET_MTK_LEN - 1] < '0' ||
		dev_name[DS_NET_MTK_LEN - 1] > '9')
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
		return DS_NET_ID;

	// devName index is in [0, 9]
	if (dev_name[DS_NET_QCOM_LEN - 1] < '0' ||
		dev_name[DS_NET_QCOM_LEN - 1] > '9')
		return DS_NET_ID;
	dev_id = (unsigned int)(dev_name[DS_NET_QCOM_LEN - 1] - '0');
	if (g_link_property[DS_NET_SLAVE_ID].interface_id == dev_id)
		return DS_NET_SLAVE_ID;
	return DS_NET_ID;
}

static int fill_dev_id(char *dev_name, u32 *dev_id)
{
	if (dev_name == NULL)
		return NET_DEV_ERR;

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
		return NET_DEV_ERR;
	return RTN_SUCC;
}

static int match_net_dev(struct sk_buff *skb, struct sock *sk, u32 *dev_id)
{
	char *dev_name = NULL;

	if (skb == NULL || sk == NULL || dev_id == NULL)
		return NET_DEV_ERR;
	if (skb->dev == NULL || skb->dev->name == NULL) {
#ifdef CONFIG_HW_WIFIPRO
		if (sk_fullsock(sk) &&
			(sk->sk_family == AF_INET ||
			sk->sk_family == AF_INET6) &&
			sk->sk_protocol == IPPROTO_TCP)
			dev_name = sk->wifipro_dev_name;
#endif
	} else {
		dev_name = skb->dev->name;
	}

	return fill_dev_id(dev_name, dev_id);
}

static struct app_list *get_match_node_from_list(struct sock *sk,
	struct sk_buff *skb, unsigned int cpu, u8 protocol, u16 hook_type)
{
	struct app_stat *app = NULL;
	struct app_list *pos = NULL;
	struct app_list *select = NULL;
	u32 app_type;

	if (sk == NULL)
		return NULL;

	if (cpu < 0 || cpu >= CONFIG_NR_CPUS)
		return NULL;

	app = &g_app_ctx->cur[cpu];

	if (list_empty_careful(&app->head))
		return NULL;

	list_for_each_entry(pos, &app->head, head) {
		app_type = match_app(sk, skb, &pos->stat, protocol, hook_type);
		if (app_type == BACKGROUND) {
			select = pos;
		} else if (app_type == FORGROUND_MACH) {
			select = pos;
			break;
		}
	}
	return select;
}

static void update_tcp_rtt_sk_num(struct sock *sk,
	struct tcp_res_info *stat, unsigned int direction)
{
	if (stat == NULL)
		return;
	if (direction == NF_INET_DELETE_SK_HOOK) {
		clear_prior_rtt(sk);
	} else if (direction == NF_INET_UPDATE_RTT_HOOK) {
		update_sk_rtt_sum_common(sk);
	} else if (direction == NF_INET_RETRANS_TIMER_HOOK) {
		update_sk_rtt_retrans_common(sk);
	} else if (direction == NF_INET_OFO_HOOK) {
		update_sk_rtt_ofo_common(sk);
	}
}

static void process_stat_info_tcp_layer(struct sock *sk,
	struct app_list *pos_selec, unsigned int direction, u32 dev_id)
{
	if (dev_id < 0 ||
		dev_id >= CHANNEL_NUM ||
		pos_selec == NULL)
		return;

	if (direction >= NF_INET_NEW_SK_HOOK && direction <= NF_INET_OFO_HOOK) {
		update_tcp_rtt_sk_num(sk,
			&pos_selec->stat.info[dev_id],
			direction);
	} else if (direction == NF_INET_TCP_DUPACK_IN_HOOK) {
		pos_selec->stat.info[dev_id].in_dupack_pkt++;
	}
}

static void stat_proces(struct sk_buff *skb, struct sock *sk,
	const struct nf_hook_state *state, u8 protocol, u16 payload_len)
{
	struct app_stat *app = NULL;
	struct app_list *pos_selec = NULL;
	unsigned int cpu = raw_smp_processor_id();
	u32 dev_id;

	if (skb == NULL || sk == NULL)
		return;
	if (cpu < 0 || cpu >= CONFIG_NR_CPUS)
		return;
	if (state == NULL)
		return;

	if (g_is_enable_stats == true)
		statistics_traffic(protocol, skb, sk, state->hook);
	app = &g_app_ctx->cur[cpu];
	spin_lock_bh(&app->lock);

	if (match_net_dev(skb, sk, &dev_id) != RTN_SUCC)
		goto unlock;

	pos_selec = get_match_node_from_list(sk, skb, cpu,
		protocol, state->hook);
	if (pos_selec == NULL)
		goto unlock;

	if (state->hook >= NF_INET_TCP_DUPACK_IN_HOOK) {
		process_stat_info_tcp_layer(sk,
			pos_selec, state->hook, dev_id);
	} else {
		if (protocol == IPPROTO_TCP)
			update_tcp_para(&pos_selec->stat.info[dev_id], skb, sk,
				state->hook, payload_len);
		else if (protocol == IPPROTO_UDP)
			update_udp_para(&pos_selec->stat.info[dev_id], skb,
				state->hook);
	}

unlock:
	spin_unlock_bh(&app->lock);
}

void update_dupack_num(struct sock *sk,
	bool is_dupack, unsigned int hook)
{
	if (is_dupack)
		update_tcp_para_without_skb(sk, NF_INET_TCP_DUPACK_IN_HOOK);
}

static void update_tcp_para_with_skb(struct sk_buff *skb, struct sock *sk,
	unsigned int hook)
{
	struct nf_hook_state state;
	if (skb == NULL || !is_sk_fullsock(sk) || is_sk_daddr_private(sk))
		return;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = hook;
	stat_proces(skb, sk, &state, IPPROTO_TCP, (u16)skb->len);
}

static bool is_tcp_udp_sock(struct sock *sk)
{
	if (!is_sk_fullsock(sk))
		return false;

	return (sk->sk_type == SOCK_STREAM || sk->sk_type == SOCK_DGRAM);
}

static unsigned int hook4(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct iphdr *iph = NULL;
	struct sock *sk = NULL;
	u16 payload_len;

	if (state == NULL)
		return NF_ACCEPT;
	if (g_is_enable_stats == true && skb != NULL && state->hook == NF_INET_FORWARD)
		update_tethering_traffic_info(skb);

#ifdef CONFIG_HW_NETWORK_SLICE
	if (state->hook == NF_INET_LOCAL_OUT)
		slice_ip_para_report(skb, AF_INET);
#endif

#ifdef CONFIG_STREAM_DETECT
	if (state->hook == NF_INET_POST_ROUTING)
		stream_process_hook_out(skb, AF_INET);
	if (state->hook == NF_INET_LOCAL_IN)
		stream_process_hook_in(skb, AF_INET);
#endif

	iph = ip_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->protocol == IPPROTO_UDP && state->hook == NF_INET_LOCAL_IN)
		return NF_ACCEPT;
	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (!is_tcp_udp_sock(sk))
		return NF_ACCEPT;

	if (iph->protocol == IPPROTO_TCP &&
		is_local_or_lan_addr(sk->sk_daddr))
		return NF_ACCEPT;

	payload_len = ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD; // 32bits to byte
	if (iph->protocol == IPPROTO_TCP || iph->protocol == IPPROTO_UDP)
		stat_proces(skb, sk, state, iph->protocol, payload_len);
	return NF_ACCEPT;
}

void update_tcp_para_without_skb(struct sock *sk, unsigned int hook)
{
	struct sk_buff skb;

	if (sk == NULL)
		return;

	memset(&skb, 0, sizeof(struct sk_buff));
	skb.len = IP_TCP_HEAD_LEN;
	update_tcp_para_with_skb(&skb, sk, hook);
}

void udp_in_hook(struct sk_buff *skb, struct sock *sk)
{
	const struct iphdr *iph = NULL;
	u16 payload_len;
	struct nf_hook_state state;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = NF_INET_UDP_IN_HOOK;
	if (skb == NULL)
		return;
	iph = ip_hdr(skb);
	if (iph == NULL)
		return;
	payload_len = ntohs(iph->tot_len) - iph->ihl * TCP_HDR_IHL_QUAD; // 32bits to byte
	stat_proces(skb, sk, &state, iph->protocol, payload_len);
}

void udp6_in_hook(struct sk_buff *skb, struct sock *sk)
{
	const struct ipv6hdr *iph = NULL;
	u16 payload_len;
	struct nf_hook_state state;

	memset(&state, 0, sizeof(struct nf_hook_state));
	state.hook = NF_INET_UDP_IN_HOOK;

	if (skb == NULL || sk == NULL)
		return;
	iph = ipv6_hdr(skb);
	if (iph == NULL)
		return;
	payload_len = ntohs(iph->payload_len);
	stat_proces(skb, sk, &state, iph->nexthdr, payload_len);
}

static unsigned int hook6(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	const struct ipv6hdr *iph = NULL;
	struct sock *sk = NULL;
	u16 payload_len;

	if (state == NULL)
		return NF_ACCEPT;
	if (g_is_enable_stats == true && skb != NULL && state->hook == NF_INET_FORWARD)
		update_tethering_traffic_info(skb);

#ifdef CONFIG_HW_NETWORK_SLICE
	if (state->hook == NF_INET_LOCAL_OUT)
		slice_ip_para_report(skb, AF_INET6);
#endif

#ifdef CONFIG_STREAM_DETECT
	if (state->hook == NF_INET_POST_ROUTING)
		stream_process_hook_out(skb, AF_INET6);
	if (state->hook == NF_INET_LOCAL_IN)
		stream_process_hook_in(skb, AF_INET6);
#endif

	iph = ipv6_hdr(skb);
	if (iph == NULL)
		return NF_ACCEPT;
	if (iph->nexthdr == IPPROTO_UDP && state->hook == NF_INET_LOCAL_IN)
		return NF_ACCEPT;
	sk = skb_to_full_sk(skb);
	if (sk == NULL)
		sk = state->sk;
	if (!is_tcp_udp_sock(sk))
		return NF_ACCEPT;

	if (iph->nexthdr == IPPROTO_TCP &&
		is_private_v6_addr(sk))
		return NF_ACCEPT;

	payload_len = ntohs(iph->payload_len);
	if (iph->nexthdr == IPPROTO_TCP || iph->nexthdr == IPPROTO_UDP)
		stat_proces(skb, sk, state, iph->nexthdr, payload_len);

	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook4,
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_FORWARD,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook6,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

static void add_to_res(struct tcp_res *add, struct tcp_res *sum)
{
	int i;

	for (i = 0; i < CHANNEL_NUM; i++) {
		sum->info[i].in_rts_pkt += add->info[i].in_rts_pkt;
		sum->info[i].in_pkt += add->info[i].in_pkt;
		sum->info[i].in_len += add->info[i].in_len;
		sum->info[i].out_rts_pkt += add->info[i].out_rts_pkt;
		sum->info[i].out_pkt += add->info[i].out_pkt;
		sum->info[i].out_len += add->info[i].out_len;
		sum->info[i].rtt += add->info[i].rtt;
		sum->info[i].out_all_pkt += add->info[i].out_all_pkt;
		sum->info[i].in_udp_pkt += add->info[i].in_udp_pkt;
		sum->info[i].out_udp_pkt += add->info[i].out_udp_pkt;
		sum->info[i].in_udp_len += add->info[i].in_udp_len;
		sum->info[i].out_udp_len += add->info[i].out_udp_len;
		sum->info[i].tcp_syn_pkt += add->info[i].tcp_syn_pkt;
		sum->info[i].tcp_fin_pkt += add->info[i].tcp_fin_pkt;
		sum->info[i].in_dupack_pkt += add->info[i].in_dupack_pkt;
		sum->info[i].tcp_sk_num += add->info[i].tcp_sk_num;
		sum->info[i].sk_rtt_sum += add->info[i].sk_rtt_sum;
		sum->info[i].tcp_syn_rts_pkt += add->info[i].tcp_syn_rts_pkt;
		if (sum->info[i].in_len > MAX_U32_VALUE)
			sum->info[i].in_len = MAX_U32_VALUE;
		if (sum->info[i].out_len > MAX_U32_VALUE)
			sum->info[i].out_len = MAX_U32_VALUE;
		if (sum->info[i].rtt > MAX_U32_VALUE)
			sum->info[i].rtt = MAX_U32_VALUE;
		if (sum->info[i].in_udp_len > MAX_U32_VALUE)
			sum->info[i].in_udp_len = MAX_U32_VALUE;
		if (sum->info[i].out_udp_len > MAX_U32_VALUE)
			sum->info[i].out_udp_len = MAX_U32_VALUE;
	}
}

static void reset_stat(struct tcp_res *entry)
{
	int i;

	for (i = 0; i < CHANNEL_NUM; i++) {
		entry->info[i].in_rts_pkt = 0;
		entry->info[i].in_pkt = 0;
		entry->info[i].in_len = 0;
		entry->info[i].out_rts_pkt = 0;
		entry->info[i].out_pkt = 0;
		entry->info[i].out_len = 0;
		entry->info[i].rtt = 0;
		entry->info[i].out_all_pkt = 0;
		entry->info[i].in_udp_pkt = 0;
		entry->info[i].out_udp_pkt = 0;
		entry->info[i].in_udp_len = 0;
		entry->info[i].out_udp_len = 0;
		entry->info[i].tcp_syn_pkt = 0;
		entry->info[i].tcp_fin_pkt = 0;
		entry->info[i].in_dupack_pkt = 0;
		entry->info[i].tcp_sk_num = 0;
		entry->info[i].sk_rtt_sum = 0;
		entry->info[i].tcp_syn_rts_pkt = 0;
	}
}

static void rm_list(struct list_head *list)
{
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct app_list *pos = NULL;

	list_for_each_safe(p, n, list) {
		pos = list_entry(p, struct app_list, head);
		list_del_init(p);
		if (pos == NULL)
			return;
		kfree(pos);
	}
	list->prev = list;
	list->next = list;
}

void update_dns_statistics(struct res_msg *res, u16 cnt)
{
	u32 i;
	u32 delta_dns_tx = 0;
	u32 delta_dns_rx = 0;
	long current_dns_tx = 0;
	long current_dns_rx = 0;

	current_dns_tx = get_dns_tx_pkt_num();
	current_dns_rx = get_dns_rx_pkt_num();

	if (current_dns_tx < cache_dns_tx_pkt_sum || current_dns_rx < cache_dns_rx_pkt_sum){
		current_dns_tx = 1;
		current_dns_rx = 1;
	} else {
		delta_dns_tx = (u32)(current_dns_tx - cache_dns_tx_pkt_sum);
		delta_dns_rx = (u32)(current_dns_rx - cache_dns_rx_pkt_sum);
	}
	cache_dns_tx_pkt_sum = current_dns_tx;
	cache_dns_rx_pkt_sum = current_dns_rx;

	for (i = 0; i < cnt; i++) {
		if (res->res.tcp.res[i].uid == DNS_UID) {
			res->res.tcp.res[i].info[0].out_pkt = delta_dns_tx;
			res->res.tcp.res[i].info[0].in_pkt = delta_dns_rx;
			break;
		}
	}
}

static void cum_cpus_stat(struct app_stat *cur,
	struct tcp_collect_res *tcp, u16 cnt)
{
	struct app_list *pos = NULL;
	int i;
	bool need_add = true;

	spin_lock_bh(&cur->lock);
	list_for_each_entry(pos, &cur->head, head) {
		need_add = true;
		for (i = 0; i < tcp->cnt; i++) {
			if (pos->stat.pid == tcp->res[i].pid &&
				pos->stat.uid == tcp->res[i].uid) {
				add_to_res(&pos->stat, &tcp->res[i]);
				reset_stat(&pos->stat);
				need_add = false;
				break;
			}
		}
		if (need_add) {
			if (tcp->cnt >= cnt)
				break;
			tcp->res[tcp->cnt].pid = pos->stat.pid;
			tcp->res[tcp->cnt].uid = pos->stat.uid;
			add_to_res(&pos->stat, &tcp->res[tcp->cnt]);
			reset_stat(&pos->stat);
			tcp->cnt++;
		}
	}
	spin_unlock_bh(&cur->lock);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void stat_report(unsigned long sync)
#else
static void stat_report(struct timer_list* sync)
#endif
{
	struct res_msg *res = NULL;
	u16 cnt;
	u16 len;
	u32 i;
	u32 j;

	spin_lock_bh(&g_app_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_app_ctx->timer.data = sync + 1; // number of reports
#endif
	g_app_ctx->timer.function = stat_report;
	mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	cnt = g_app_ctx->cnt;
	data_dubai_report();
	len = sizeof(struct res_msg) + cnt * sizeof(struct tcp_res);
	if (len > MAX_REQ_DATA_LEN) {
		pr_err("[IP_PARA]%s data = %d is too long!", __func__, len);
		goto report_end;
	}
	res = kmalloc(len, GFP_ATOMIC);
	if (res == NULL)
		goto report_end;

	memset(res, 0, len);
	res->type = APP_QOE_RPT;
	res->len = len;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	res->res.tcp.sync = (u16)sync;
#endif

	for (i = 0; i < cnt; i++) {
		for (j = 0; j < CHANNEL_NUM; j++)
			res->res.tcp.res[i].info[j].dev_id = j;
	}
	for (i = 0; i < CONFIG_NR_CPUS; i++)
		cum_cpus_stat(g_app_ctx->cur + i, &res->res.tcp, cnt);
	update_dns_statistics(res, cnt);
	g_app_ctx->report_tstamp = tcp_jiffies32;
	if (g_app_ctx->fn)
		g_app_ctx->fn((struct res_msg_head *)res);
		
	kfree(res);

report_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void reset_tcp_sock_info(void)
{
	g_tcp_reset_info.record_index = -1;
	g_tcp_reset_info.syn_v4_count = 0;
	g_tcp_reset_info.syn_v6_count = 0;
	g_tcp_reset_info.established_v4_count = 0;
	g_tcp_reset_info.established_v6_count = 0;
	(void)memset_s(g_tcp_reset_info.sock_info, MAX_TCP_SOCK_INFO_SIZE, 0, MAX_TCP_SOCK_INFO_SIZE);
	if (time_after(jiffies, g_tcp_reset_info.report_stamp + TCP_RESET_CHR_FORBIDDEN_REPORT_TIME))
		g_tcp_reset_info.report_cnt = 0;
}

static void report_tcp_reset_chr(int last_foreground_uid)
{
	char event[TCP_RESET_NOTIFY_LEN] = {0};
	char *p = event;

	if (last_foreground_uid < IGNORE_UID)
		return;

	if (g_tcp_reset_info.report_cnt >= MAX_REPORT_TCP_RESET_CNT)
		return;

	if (!(g_tcp_reset_info.syn_v4_count > 0 || g_tcp_reset_info.syn_v6_count > 0 ||
		g_tcp_reset_info.established_v4_count > 0 || g_tcp_reset_info.established_v6_count > 0))
		return;

	assign_short(p, TCP_RESET_CHR_MSG_ID);
	skip_byte(p, sizeof(u16));
	assign_short(p, TCP_RESET_CHR_RPT_LEN);
	skip_byte(p, sizeof(u16));
	assign_int(p, last_foreground_uid);
	skip_byte(p, sizeof(int));
	assign_int(p, g_tcp_reset_info.syn_v4_count);
	skip_byte(p, sizeof(int));
	assign_int(p, g_tcp_reset_info.syn_v6_count);
	skip_byte(p, sizeof(int));
	assign_int(p, g_tcp_reset_info.established_v4_count);
	skip_byte(p, sizeof(int));
	assign_int(p, g_tcp_reset_info.established_v6_count);
	skip_byte(p, sizeof(int));
	if (g_app_ctx->fn) {
		g_app_ctx->fn((struct res_msg_head *) event);
		g_tcp_reset_info.report_cnt++;
		if (g_tcp_reset_info.report_cnt == 1)
			g_tcp_reset_info.report_stamp = jiffies;
	}
}

static void record_fore_app_uid(u32 uid)
{
	if (uid == BACKGROUND_UID ||
		uid == DNS_UID ||
		uid == KEY_FLOW_UID)
		return;

	g_app_ctx->fore_app_uid = uid;
}

static void sync_apps_list(struct app_id *id, u16 cnt,
	struct app_stat *stat)
{
	u16 i;
	bool need_new = true;
	struct list_head tmp_head;
	struct list_head *p = NULL;
	struct list_head *n = NULL;
	struct app_list *pos = NULL;

	tmp_head.next = &tmp_head;
	tmp_head.prev = &tmp_head;
	for (i = 0; i < cnt; i++) {
		need_new = true;
		list_for_each_safe(p, n, &stat->head) {
			pos = list_entry(p, struct app_list, head);
			if (pos->stat.uid != id[i].uid ||
				pos->stat.pid != id[i].pid)
				continue;
			list_move_tail(p, &tmp_head);
			need_new = false;
			break;
		}
		if (need_new) {
			pos = kmalloc(sizeof(struct app_list), GFP_ATOMIC);
			if (pos == NULL)
				goto list_end;
			memset(pos, 0, sizeof(struct app_list));
			pos->stat.pid = id[i].pid;
			pos->stat.uid = id[i].uid;
			list_add_tail(&pos->head, &tmp_head);
		}
	}
	record_fore_app_uid(id[cnt - 1].uid);
	rm_list(&stat->head);
	stat->head.prev = tmp_head.prev;
	stat->head.next = tmp_head.next;
	tmp_head.prev->next = &stat->head;
	tmp_head.next->prev = &stat->head;
	return;

list_end:
	rm_list(&tmp_head);
}

static void sync_to_cpus(struct tcp_collect_req *req)
{
	int i;
	struct app_stat *stat = g_app_ctx->cur;

	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		spin_lock_bh(&stat[i].lock);
		sync_apps_list(req->id, req->cnt, &stat[i]);
		spin_unlock_bh(&stat[i].lock);
	}
	g_app_ctx->cnt = req->cnt;
}

static u32 ms_convert_jiffies(u32 cycle)
{
	return cycle / JIFFIES_MS;
}

static void process_sync(struct req_msg *msg)
{
	if (msg->len != HEAD_DATA_LEN)
		return;

	struct tcp_collect_req *req = &msg->req.tcp_req;
	u16 flag = req->req_flag;
	u32 cycle = req->rpt_cycle;

	pr_info("[IP_PARA]%s data: %d %d", __func__, req->req_flag, req->rpt_cycle);
	spin_lock_bh(&g_app_ctx->lock);
	if (flag & RESTART_SYNC) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
		g_app_ctx->timer.data = 0;
#endif
		g_app_ctx->timer.function = stat_report;
		g_app_ctx->jcycle = ms_convert_jiffies(cycle);
		mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	} else {
		if (!timer_pending(&g_app_ctx->timer))
			goto sync_end;
		del_timer(&g_app_ctx->timer);
		reset_app_stat();
	}
sync_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void sync_key_flow(struct key_flow *new_flow)
{
	u32 i;
	u32 cnt_num = new_flow->cnt;
	struct key_flow *flow = &g_app_ctx->flow;
	if (cnt_num > MAX_KEY_FLOW_TYPE_NUM)
		cnt_num = MAX_KEY_FLOW_TYPE_NUM;
	flow->cnt = cnt_num;
	pr_info("[IP_PARA] %s,cnt=%u", __func__, cnt_num);
	for (i = 0; i < cnt_num; i++)
		flow->info[i] = new_flow->info[i];
}

static void add_key_flow(struct booster_key_flow *new_booster_flow)
{
	pr_info("[IP_PARA]: add_key_flow START");
	struct booster_key_flow *flow = &g_app_ctx->new_flow;
	flow->uid = new_booster_flow->uid;
	flow->dest_port = htons(new_booster_flow->dest_port);
	flow->dest_addr = new_booster_flow->dest_addr;
}

static void process_app_update(struct req_msg *msg)
{
	if (msg->len < sizeof(struct req_msg) ||
		msg->len != msg->req.tcp_req.cnt * sizeof(struct app_id) + HEAD_DATA_LEN + sizeof(struct key_flow))
		return;

	struct tcp_collect_req *req = &msg->req.tcp_req;
	u16 i = 0;
	u16 flag = req->req_flag;
	u32 cycle = req->rpt_cycle;

	pr_info("[IP_PARA] %s data :flag=%d,cycle=%d,cnt=%d", __func__, req->req_flag, req->rpt_cycle, req->cnt);

	spin_lock_bh(&g_app_ctx->lock);

	if (req->cnt > MAX_APP_LIST_LEN)
		goto app_end;

	if (req->cnt >= 3 &&
		(req->id[0].uid != DNS_UID || req->id[1].uid != KEY_FLOW_UID || req->id[2].uid != BACKGROUND_UID))
		pr_info("[IP_PARA]%s 0:PID = %d, UID = %d; 1:PID = %d, UID = %d; 2:PID = %d, UID = %d;", __func__,
		req->id[0].pid, req->id[0].uid, req->id[1].pid, req->id[1].uid, req->id[2].pid, req->id[2].uid);

	for (i = 0; i < req->cnt; i++) {
		if (i >= 3)
			pr_info("[IP_PARA]%s i: %d, PID = %d, UID = %d", __func__, i, req->id[i].pid, req->id[i].uid);
	}

	if ((flag & FORFROUND_STAT) || (flag & BACKGROUND_STAT)) {
		sync_to_cpus(req);
		sync_key_flow(&req->key_flow);
		reset_app_stat();
	}

	if (flag & RESTART_SYNC) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
		g_app_ctx->timer.data = 0;
#endif
		g_app_ctx->timer.function = stat_report;
		g_app_ctx->jcycle = ms_convert_jiffies(cycle);
		mod_timer(&g_app_ctx->timer, jiffies + g_app_ctx->jcycle);
	}

app_end:
	spin_unlock_bh(&g_app_ctx->lock);
}

static void process_game_app_update(struct new_key_flow_req *msg)
{
	if (msg->len != sizeof(struct new_key_flow_req))
		return;

	pr_info("[IP_PARA]: process_game_app_update START");
	spin_lock_bh(&g_app_ctx->lock);

	if (msg->command == 0) {
		pr_info("[IP_PARA]: add key flow");
		add_key_flow(&msg->booster_key_flow_info);
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_app_ctx->timer.data = 0;
#endif
	g_app_ctx->timer.function = stat_report;
	spin_unlock_bh(&g_app_ctx->lock);
}

static void process_link_property_update(struct link_property_msg *msg)
{
	if (msg->len != sizeof(struct link_property_msg))
		return;

	struct link_property_info *property = &msg->property;
	u32 modem_id = property->modem_id;
	u32 interface_id = property->interface_id;

	pr_info("[LINK PROPERTY] %s modem_id:%d, interface name id:%d\n", __func__,
		modem_id, interface_id);

	if (modem_id != DS_NET_ID && modem_id != DS_NET_SLAVE_ID)
		return;
	if (interface_id > DS_INTERFACE_NAME_ID) {
		g_link_property[modem_id].interface_id = INVALID_LINK_PROPERTITY_INTERFACE_ID;
		return;
	}
	g_link_property[modem_id].interface_id = interface_id;
}

/*
 * handle top uid list msg
 */
static void process_uid_list(const struct uid_list_msg_head *msg)
{
	if (msg->len < sizeof(struct uid_list_msg_head) ||
		msg->len != sizeof(struct uid_list_msg_head) + msg->uid_num * sizeof(u32))
		return;

	u32 *p = NULL;
	int i;

	pr_info("[IP_PARA]process uid list uid_num: %u", msg->uid_num);
	if (msg->uid_num <= 0 || msg->uid_num > MAX_TOP_UID_NUM) {
		pr_info("[IP_PARA]uid num is invalid, process uid list failed");
		return;
	}
	spin_lock_bh(&g_ip_para_collec_lock);
	if (g_top_uid_num != 0) {
		pr_info("[IP_PARA]top uid has existed");
		spin_unlock_bh(&g_ip_para_collec_lock);
		return;
	}

	if (create_list() == false) {
		pr_info("[IP_PARA]create list failed");
		spin_unlock_bh(&g_ip_para_collec_lock);
		return;
	}
	p = (u32 *)msg;
	p = p + sizeof(struct uid_list_msg_head) / sizeof(u32);

	for (i = 0; i < msg->uid_num; i++) {
		g_top_uid[g_top_uid_num] = *p;
		g_top_uid_num++; // g_top_uid_num represents the number of elements in g_top_uid
		p++;
	}

	if (g_top_uid_num > 0) {
		g_is_enable_stats = true;
		g_last_report_time = jiffies_to_msecs(jiffies_64);
	}

	spin_unlock_bh(&g_ip_para_collec_lock);
	return;
}

/*
 * handle install app msg
 */
static void process_uid_add_update(const struct uid_change_msg *msg)
{
	pr_info("[IP_PARA]process uid add update uid: %u", msg->uid);
	spin_lock_bh(&g_ip_para_collec_lock);
	if (g_is_enable_stats == false) {
		if (create_list() == false) {
			pr_info("[IP_PARA]create list failed");
			spin_unlock_bh(&g_ip_para_collec_lock);
			return;
		}
		g_is_enable_stats = true;
		g_last_report_time = jiffies_to_msecs(jiffies_64);
	}

	if (g_top_uid_num < MAX_TOP_UID_NUM) {
		g_top_uid[g_top_uid_num] = msg->uid;
		g_top_uid_num++;
	}

	spin_unlock_bh(&g_ip_para_collec_lock);
	return;
}

/*
 * handle uninstall app msg
 */
static void process_uid_del_update(const struct uid_change_msg *msg)
{
	traffic_node_ptr del_traffic_info_ptr = NULL;
	char *event = NULL;
	int pos;
	int i;
	u16 notify_len;

	spin_lock_bh(&g_ip_para_collec_lock);
	if (g_is_enable_stats == false) {
		pr_info("[IP_PARA]process uid del update failed");
		spin_unlock_bh(&g_ip_para_collec_lock);
		return;
	}
	pr_info("[IP_PARA]process uid del update uid: %u", msg->uid);

	del_traffic_info_ptr = find_list(msg->uid);
	if (del_traffic_info_ptr != NULL) {
		pr_info("[IP_PARA]notify if del uid is in list");
		notify_len = sizeof(u16) + sizeof(u16) + sizeof(u32) + LEN_PER_UID * (g_list_num + 1);
		event = (char *)kmalloc(notify_len, GFP_ATOMIC);
		if (event != NULL) {
			(void)memset_s(event, notify_len, 0, notify_len);
			notify_traffic_info(event, notify_len);
		}
	}

	pos = g_top_uid_num;
	for (i = 0; i < g_top_uid_num; i++) {
		if (msg->uid == g_top_uid[i]) {
			pos = i;
			break;
		}
	}
	if (pos == g_top_uid_num) {
		spin_unlock_bh(&g_ip_para_collec_lock);
		notify_traffic_info_process(event);
		if (event != NULL)
			kfree(event);
		return;
	}
	for (i = pos; i < g_top_uid_num - 1; i++)
		g_top_uid[i] = g_top_uid[i + 1];
	g_top_uid[g_top_uid_num - 1] = 0;
	g_top_uid_num--;

	spin_unlock_bh(&g_ip_para_collec_lock);

	notify_traffic_info_process(event);
	if (event != NULL)
		kfree(event);
	return;
}

/*
 * handle network type msg
 */
static void process_cellular_network_update(const struct cellular_network_type_msg *msg)
{
	pr_info("[IP_PARA]process network update modem0: %d, modem1: %d",
		msg->networktype_modem0, msg->networktype_modem1);

	spin_lock_bh(&g_ip_para_collec_lock);
	if (msg->networktype_modem0 < 0 || msg->networktype_modem0 >= STATS_NETWORK_TYPE_WIFI)
		g_dev_id_network_type[DS_NET_ID] = -1;
	else
		g_dev_id_network_type[DS_NET_ID] = msg->networktype_modem0;

	if (msg->networktype_modem1 < 0 || msg->networktype_modem1 >= STATS_NETWORK_TYPE_WIFI)
		g_dev_id_network_type[DS_NET_SLAVE_ID] = -1;
	else
		g_dev_id_network_type[DS_NET_SLAVE_ID] = msg->networktype_modem1;

	spin_unlock_bh(&g_ip_para_collec_lock);
	return;
}

/*
 * handle wifi state msg
 */
static void process_wifi_update(const struct wifi_state_msg *msg)
{
	pr_info("[IP_PARA]process wifi update wifi_state: %d", msg->wifi_state);

	spin_lock_bh(&g_ip_para_collec_lock);
	if (msg->wifi_state == WIFI_STATE_ON)
		g_dev_id_network_type[WIFI_NET_ID] = STATS_NETWORK_TYPE_WIFI;
	else
		g_dev_id_network_type[WIFI_NET_ID] = -1;

	spin_unlock_bh(&g_ip_para_collec_lock);
	return;
}

/*
 * handle virtual sim state msg
 */
static void process_virtual_sim_update(const struct virtual_sim_state_msg *msg)
{
	pr_info("[IP_PARA]process virtual sim update virtual_sim_state: %d", msg->virtual_sim_state);

	spin_lock_bh(&g_ip_para_collec_lock);
	if (msg->virtual_sim_state == VIRTUAL_SIM_STATE_ON) {
		g_dev_id_network_type[DS_NET_ID] = -1;
		g_dev_id_network_type[DS_NET_SLAVE_ID] = -1;
	}

	spin_unlock_bh(&g_ip_para_collec_lock);
	return;
}

static void set_foreground_uid(const struct uid_change_msg *msg)
{
	if (msg == NULL || msg->len != sizeof(struct uid_change_msg) || msg->uid <= 0)
		return;
	spin_lock_bh(&g_tcp_reset_lock);
	if (g_foreground_uid != msg->uid) {
		report_tcp_reset_chr(g_foreground_uid);
		reset_tcp_sock_info();
		g_foreground_uid = msg->uid;
		update_dubai_top_idx((u16)g_foreground_uid);
	}
	spin_unlock_bh(&g_tcp_reset_lock);
}

/*
 * handle msg from booster
 */
static void booster_msg_process(const struct req_msg_head *msg)
{
	switch (msg->type) {
	case SYNC_TOP_UID_LIST:
		process_uid_list((struct uid_list_msg_head *)msg);
		break;
	case ADD_TOP_UID:
		if (msg->len == sizeof(struct uid_change_msg))
			process_uid_add_update((struct uid_change_msg *)msg);
		break;
	case DEL_TOP_UID:
		if (msg->len == sizeof(struct uid_change_msg))
			process_uid_del_update((struct uid_change_msg *)msg);
		break;
	case UPDATE_CELLULAR_MODE:
		if (msg->len == sizeof(struct cellular_network_type_msg))
			process_cellular_network_update((struct cellular_network_type_msg *)msg);
		break;
	case UPDATE_WIFI_STATE:
		if (msg->len == sizeof(struct wifi_state_msg))
			process_wifi_update((struct wifi_state_msg *)msg);
		break;
	case UPDATE_VIRTUAL_SIM_STATE:
		if (msg->len == sizeof(struct virtual_sim_state_msg))
			process_virtual_sim_update((struct virtual_sim_state_msg *)msg);
		break;
	default:
		break;
	}
}

static void cmd_process(struct req_msg_head *msg, u32 len)
{
	if (len < sizeof(struct req_msg_head) || msg->len > MAX_REQ_DATA_LEN)
		return;

	if (msg->len != len) {
		pr_err("ip_para_collec msg len error!!! left = %d, right = %d", msg->len, len);
		return;
	}
	if (msg->type == APP_QOE_SYNC_CMD) {
		process_sync((struct req_msg *)msg);
	} else if (msg->type == GAME_FLOW_QOE_CMD) {
		process_game_app_update((struct new_key_flow_req *)msg);
	} else if (msg->type == UPDATE_APP_INFO_CMD) {
		process_app_update((struct req_msg *)msg);
	} else if (msg->type == UPDAT_INTERFACE_NAME) {
		process_link_property_update((struct link_property_msg *)msg);
	} else if (msg->type == FG_CHANGE_UID) {
		set_foreground_uid((struct uid_change_msg *) msg);
	} else if (msg->type == UPDATE_TOP_APP_UID_LIST) {
		update_top_app_uids((struct dubai_update_top_idx *) msg);
	} else if (msg->type == TRIGGER_TOP_APP_ABN_REPORT) {
		top_app_abn_stat_report();
	} else if (msg->type >= ADD_TOP_UID && msg->type <= UPDATE_VIRTUAL_SIM_STATE) {
		booster_msg_process(msg);
	} else {
		pr_info("[IP_PARA]map msg error\n");
	}
}

static void init_reset_sock_info(void)
{
	spin_lock_bh(&g_tcp_reset_lock);
	(void)memset_s(&g_tcp_reset_info, sizeof(struct tcp_reset_info), 0, sizeof(struct tcp_reset_info));
	g_tcp_reset_info.record_index = -1;
	spin_unlock_bh(&g_tcp_reset_lock);
}

/*
 * Initialize internal variables and external interfaces
 */
msg_process *ip_para_collec_init(notify_event *fn)
{
	int i;
	int ret;

	if (fn == NULL)
		return NULL;
	init_reset_sock_info();
	init_data_dubai_info();
	update_dubai_top_idx((u16)g_foreground_uid);
	g_app_ctx = kmalloc(sizeof(struct app_ctx), GFP_KERNEL);
	if (g_app_ctx == NULL)
		return NULL;
	memset(g_app_ctx, 0, sizeof(struct app_ctx));
	g_app_ctx->fn = fn;
	g_app_ctx->cur = kmalloc(sizeof(struct app_stat) * CONFIG_NR_CPUS,
				 GFP_KERNEL);
	if (g_app_ctx->cur == NULL)
		goto init_error;
	memset(g_app_ctx->cur, 0, sizeof(struct app_stat) * CONFIG_NR_CPUS);
	for (i = 0; i < CONFIG_NR_CPUS; i++) {
		spin_lock_init(&g_app_ctx->cur[i].lock);
		g_app_ctx->cur[i].head.prev = &g_app_ctx->cur[i].head;
		g_app_ctx->cur[i].head.next = &g_app_ctx->cur[i].head;
	}
	spin_lock_init(&g_app_ctx->lock);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	init_timer(&g_app_ctx->timer);
#else
	timer_setup(&g_app_ctx->timer, stat_report, 0);
#endif
	g_app_ctx->report_tstamp = tcp_jiffies32;
	ret = nf_register_net_hooks(&init_net, net_hooks,
				    ARRAY_SIZE(net_hooks));
	if (ret) {
		pr_info("[IP_PARA]nf_init_in ret=%d  ", i);
		goto init_error;
	}
	return cmd_process;

init_error:
	if (g_app_ctx->cur != NULL)
		kfree(g_app_ctx->cur);
	if (g_app_ctx != NULL)
		kfree(g_app_ctx);
	g_app_ctx = NULL;
	return NULL;
}

void ip_para_collec_exit(void)
{
	if (g_app_ctx->cur != NULL)
		kfree(g_app_ctx->cur);
	if (g_app_ctx != NULL)
		kfree(g_app_ctx);
	g_app_ctx = NULL;
}
