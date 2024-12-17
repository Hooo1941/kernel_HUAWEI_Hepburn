/*
 * cloud_network_option.c
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

#include "cloud_network_option.h"
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include "cloud_network_device.h"
#include "cloud_network_detect.h"
#include "cloud_network_utils.h"
#include "cloud_network_param.h"
#include "cloud_network_rx.h"
#include "cloud_network_tx.h"

#define OPTION_TIMEOUT_THRESHOLD 2000

enum option_work_type {
	SEND_OPTION = 0,
	OPTION_TIMEOUT = 1,
	RECV_OPTION_ACK = 2,
	OPTION_WORK_TYPE_MAX
};

struct option_ctrl {
	struct hrtimer option_timeout_timer;
	struct list_queue option_queue;
	struct work_struct option_handle_work;
	struct list_head option_packet_list;
	spinlock_t option_packet_list_lock;
};

struct option_struct {
	u32 type;
	u32 msg_len;
	char data[0];
};

struct option_recv_struct {
	u32 type;
	u32 msg_len;
	u64 timestamp;
};

struct option_ctrl option_mgr;

static void option_put_nop(struct sk_buff *skb)
{
	__u16 len;
	__u16 extra = 0;
	char *tail = NULL;
	__u16 *option_len = NULL;
	__u16 i = 0;

	len = get_options_len(skb) - sizeof(__u16);
	if (len % 4 == 0)
		return;
	extra = (len / 4 + 1) * 4 - len;
	tail = skb_tail_pointer(skb);
	for (i = 0; i < extra; i++)
		tail[i] = 1;
	skb_put(skb, extra);
	option_len = hon_option(skb);
	*option_len = htons(ntohs(*option_len) + extra);
	return;
}

__u16 generate_option(char *option, __u16 option_len, __u8 type, __u8 len, char *data)
{
	__u16 extra = 0;
	errno_t err;

	if (option_len == 0) {
		option_len += sizeof(option_len);
		*((__u16 *)option) = htons(2);
	}
	if (len % 4 != 0)
		extra = len / 4 * 4 + 4 - len;
	*((__u16 *)option) = htons(ntohs(*((__u16 *)option)) + len + extra);
	*(option + option_len) = type;
	*(option + option_len + sizeof(type)) = len;
	err = memcpy_s(option + option_len + sizeof(type) + sizeof(len),
		len - sizeof(type) - sizeof(len), data,
		len - sizeof(type) - sizeof(len));
	if (err != EOK)
		log_err("memcpy_s failed");

	option_len += (len + extra);
	for (int i = 1; i <= extra; i++)
		*(option + option_len - i) = HONOPT_NOP;
	return option_len;
}

__u16 generate_timestamp_option(char *option, __u16 option_len)
{
	struct option_timestamp_struct option_timestamp;
	option_timestamp.timestamp = htonl(get_rel_time_us());

	if (unlikely(option == NULL)) {
		log_err("option is NULL, error");
		return 0;
	}

	return generate_option(option, option_len, HONOPT_DETECT_REQUEST, TIMESTAMP_OPTION_LEN, (char *)&option_timestamp);
}

static void free_opt_skb(struct opt_skb *opt)
{
	if (unlikely(opt == NULL)) {
		log_err("opt is NULL, error");
		return;
	}
	if (opt->skb != NULL)
		dev_kfree_skb_any(opt->skb);
	kfree(opt);
	return;
}

static void option_outer_header_set(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	struct honhdr *honh = NULL;
	__u16 *option_len = NULL;
	struct protocol_info info;

	if (unlikely(skb == NULL)) {
		log_err("skb is NULL, error");
		return;
	}
	get_protocol_info(&info);
	skb_put(skb, IP_SIZE + UDP_SIZE + HON_SIZE);
	skb_set_network_header(skb, 0);
	skb_set_transport_header(skb, IP_SIZE);
	iph = ip_hdr(skb);
	(void)memset_s(iph, IP_SIZE, 0, IP_SIZE);
	iph->protocol = IPPROTO_UDP;
	iph->tot_len = 0;
	iph->version = 4;
	iph->ihl = 5;
	iph->ttl = 64;
	iph->daddr = info.compass_ip;
	iph->frag_off = htons(IP_DF);
	udph = udp_hdr(skb);
	(void)memset_s(udph, UDP_SIZE, 0, UDP_SIZE);
	udph->dest = htons(info.compass_port);
	udph->source = htons(17771);
	honh = hon_hdr(skb);
	(void)memset_s(honh, HON_SIZE, 0, HON_SIZE);
	honh->rsv = 0b10;
	honh->x = 1;
	honh->version = CLOUD_NETWORK_HON_VERSION;
	honh->compress_flag = HON_CF_NONE;
	honh->hmac_id = info.hmac_id;
	honh->mp_policy = HON_MP_REDUNDANCY;
	honh->frag_off = 0;
	honh->frag_tot = 1;
	hton24(info.app_id);
	memcpy_s(honh->app_id, sizeof(honh->app_id), info.app_id, sizeof(honh->app_id));
	memcpy_s(honh->client_id, sizeof(honh->client_id), info.client_id, sizeof(honh->client_id));
	option_len = (__u16 *)skb_tail_pointer(skb);
	*option_len = htons(sizeof(__u16));
	skb_put(skb, sizeof(__u16));
	return;
}

static enum hrtimer_restart cloud_option_ack_timeout(struct hrtimer *timer)
{
	struct list_node *node = NULL;
	struct option_struct opt_struct;

	if (unlikely(timer == NULL))
		return HRTIMER_NORESTART;

	if (!is_cloud_network_enable())
		return HRTIMER_NORESTART;

	opt_struct.type = OPTION_TIMEOUT;
	opt_struct.msg_len = sizeof(opt_struct);
	node = list_node_alloc((void *)&opt_struct, sizeof(opt_struct));
	if (unlikely(node == NULL)) {
		log_err("node is NULL");
		return HRTIMER_NORESTART;
	}

	enqueue_list_queue(&option_mgr.option_queue, node);
	schedule_work(&(option_mgr.option_handle_work));
	hrtimer_forward_now(timer, ms_to_ktime(OPTION_TIMEOUT_THRESHOLD));
	return HRTIMER_RESTART;
}

static void send_option_handle(struct opt_skb *opt_orig)
{
	struct sk_buff *skb = NULL;
	struct opt_skb *opt = NULL;
	u32 paths;
	errno_t err;

	opt = (struct opt_skb *)kmalloc(sizeof(struct opt_skb), GFP_ATOMIC);
	if (opt == NULL)
		return;

	err = memcpy_s(opt, sizeof(struct opt_skb), opt_orig,
		sizeof(struct opt_skb));
	if (err != EOK) {
		kfree(opt);
		return;
	}

	if (unlikely(opt->skb == NULL)) {
		log_err("opt_skb is error");
		kfree(opt);
		return;
	}

	opt->type = OPTION_TIMEOUT;
	if (!hrtimer_active(&option_mgr.option_timeout_timer))
		hrtimer_start(&option_mgr.option_timeout_timer,
		ms_to_ktime(OPTION_TIMEOUT_THRESHOLD), HRTIMER_MODE_REL);
	spin_lock(&option_mgr.option_packet_list_lock);
	list_add(&opt->head, &option_mgr.option_packet_list);
	spin_unlock(&option_mgr.option_packet_list_lock);
	skb = skb_copy(opt->skb, GFP_ATOMIC);
	if (unlikely(skb == NULL)) {
		log_err("skb_copy failed");
		kfree(opt);
		return;
	}
	paths = get_link_path();
	add_path_timestamp_send(paths, opt->timestamp);
	skb_send(skb, paths);
	return;
}

static void option_timeout_handle()
{
	struct sk_buff *skb = NULL;
	struct opt_skb *opt = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	u32 paths = 0;
	u32 timestamp = get_rel_time_us();

	spin_lock(&option_mgr.option_packet_list_lock);
	if (list_empty(&option_mgr.option_packet_list)) {
		log_info("stop option_timeout_timer");
		hrtimer_cancel(&option_mgr.option_timeout_timer);
		spin_unlock(&option_mgr.option_packet_list_lock);
		return;
	}
	list_for_each_safe(pos, n, &option_mgr.option_packet_list) {
		opt = list_entry(pos, struct opt_skb, head);
		if (unlikely(opt == NULL || opt->skb == NULL)) {
			log_err("opt_skb is error");
			return;
		}

		if (unlikely(!is_cloud_network_enable())) {
			list_del(pos);
			free_opt_skb(opt);
			hrtimer_cancel(&option_mgr.option_timeout_timer);
			continue;
		}

		if (timestamp - opt->timestamp > 3 * 1000 * 1000) {
			list_del(pos);
			free_opt_skb(opt);
			continue;
		}
		if (timestamp - opt->timestamp < OPTION_TIMEOUT_THRESHOLD * 1000) {
			continue;
		}

		skb = skb_copy(opt->skb, GFP_ATOMIC);
		if (unlikely(skb == NULL)) {
			log_err("skb_copy failed");
			continue;
		}
		paths = get_link_path();
		skb_send(skb, paths);
	}
	spin_unlock(&option_mgr.option_packet_list_lock);
	return;
}

static void recv_option_ack_handle(struct option_recv_struct *opt_rcv)
{
	struct opt_skb *opt = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	if (unlikely(opt_rcv == NULL)) {
		log_err("opt_rcv is NULL, error");
		return;
	}

	spin_lock(&option_mgr.option_packet_list_lock);
	list_for_each_safe(pos, n, &option_mgr.option_packet_list) {
		opt = list_entry(pos, struct opt_skb, head);
		if (opt->timestamp == opt_rcv->timestamp) {
			list_del(pos);
			free_opt_skb(opt);
			spin_unlock(&option_mgr.option_packet_list_lock);
			return;
		}
	}
	spin_unlock(&option_mgr.option_packet_list_lock);
	return;
}

static void cloud_option_work_handle(struct work_struct *work)
{
	log_info("cloud option work handle start");
	struct list_node *node = NULL;

	if (unlikely(work == NULL)) {
		log_err("work NULL Pointer");
		return;
	}

	if (!is_cloud_network_enable())
		return;

	while (list_queue_empty(&(option_mgr.option_queue)) == 0) {
		node = dequeue_list_queue(&(option_mgr.option_queue));
		if (node == NULL) {
			log_err("dequeue a null node");
			break;
		}
		struct option_struct *opt_struct = (struct option_struct *)(node->data);
		switch (opt_struct->type) {
		case SEND_OPTION:
			log_info("SEND_OPTION");
			send_option_handle((struct opt_skb *)(node->data));
			break;
		case OPTION_TIMEOUT:
			log_info("OPTION_TIMEOUT");
			option_timeout_handle();
			break;
		case RECV_OPTION_ACK:
			recv_option_ack_handle((struct option_recv_struct *)(node->data));
			break;
		default:
			log_err("opt_struct type unknown");
		}
		list_node_free(node);
		node = NULL;
	}
	log_info("cloud option work handle end");
	return;
}

void option_append(struct sk_buff *skb, __u8 type, __u8 len, char *data)
{
	__u8 *tail = NULL;
	__u16 *option_len = NULL;
	errno_t err;

	tail = skb_tail_pointer(skb);
	if (unlikely(skb == NULL)) {
		log_err("skb is NULL, error");
		return;
	}
	if (unlikely(tail == NULL)) {
		log_err("tail is NULL, error");
		return;
	}

	option_len = hon_option(skb);
	*option_len = htons(ntohs(*option_len) + len);
	tail[0] = type;
	tail[1] = len;
	err = memcpy_s(tail + sizeof(type) + sizeof(len),
		len - sizeof(type) - sizeof(len), data,
		len - sizeof(type) - sizeof(len));
	if (err != EOK)
		log_err("memcpy_s failed");

	skb_put(skb, len);
	option_put_nop(skb);
	return;
}

u32 option_timestamp_append(struct sk_buff *skb, u32 timestamp)
{
	struct option_timestamp_struct option_timestamp;
	if (timestamp != 0)
		option_timestamp.timestamp = htonl(timestamp);
	else
		option_timestamp.timestamp = htonl(get_rel_time_us());
	if (unlikely(skb == NULL)) {
		log_err("skb is NULL, error");
		return 0;
	}

	option_append(skb, HONOPT_DETECT_REQUEST, TIMESTAMP_OPTION_LEN, (char *)&option_timestamp);
	return ntohl(option_timestamp.timestamp);
}

struct sk_buff *alloc_option_skb(int option_size)
{
	struct sk_buff *skb = NULL;
	skb = alloc_skb(HON_EXTRA_SIZE + TIMESTAMP_OPTION_LEN +
		(option_size + sizeof(__u16)) + REVERSE_SIZE, GFP_ATOMIC);
	if (unlikely(skb == NULL)) {
		log_err("skb is NULL, error");
		return NULL;
	}

	skb->ip_summed = CHECKSUM_NONE;
	skb->csum = 0;
	skb_reset_mac_header(skb);
	skb->mac_len = 0;
	skb->data_len = 0;
	skb->protocol = htons(ETH_P_IP);
	skb_reserve(skb, REVERSE_SIZE / 2);
	skb_reset_tail_pointer(skb);
	option_outer_header_set(skb);
	return skb;
}

void recv_option_ack(u64 timestamp)
{
	struct option_recv_struct opt_recv;
	struct list_node *node = NULL;

	if (!is_cloud_network_enable())
		return;

	opt_recv.type = RECV_OPTION_ACK;
	opt_recv.msg_len = sizeof(struct option_recv_struct);
	opt_recv.timestamp = timestamp;
	node = list_node_alloc((void *)(&opt_recv), sizeof(struct option_recv_struct));
	if (node == NULL) {
		log_err("node is NULL, error");
		return;
	}
	enqueue_list_queue(&option_mgr.option_queue, node);
	schedule_work(&(option_mgr.option_handle_work));
	return;
}

void send_reset_option(void)
{
	char reset = 1;
	send_option_packet(HONOPT_RESET, sizeof(__u8) + sizeof(__u8) + sizeof(__u8), (char *)(&reset));
	return;
}

void send_session_synchronize_v4(__u8 ip_type, __u32 sip, __u32 dip, __u16 sport, __u16 dport, __u8 proto_type)
{
	struct much_session_sync much_ss_sync;
	much_ss_sync.cnt = 1;
	much_ss_sync.ss_sync.ip_type = ip_type;
	much_ss_sync.ss_sync.sip = sip;
	much_ss_sync.ss_sync.dip = dip;
	much_ss_sync.ss_sync.sport = sport;
	much_ss_sync.ss_sync.dport = dport;
	much_ss_sync.ss_sync.proto_type = proto_type;
	send_option_packet(HONOPT_SESSION, sizeof(__u8) + sizeof(__u8) + sizeof(much_ss_sync), (char *)(&much_ss_sync));
	return;
}

static int device_option_information_report(struct cloud_option_tlv *tlv)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	errno_t err;

	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		tlv->length, GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("msg_rsp is NULL, error");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) + tlv->length;
	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = OPTION_INFORM_EXCEPTION;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		tlv->length;
	err = memcpy_s(msg_header->data, tlv->length, (char *)tlv, tlv->length);
	if (err != EOK) {
		log_err("memcpy_s failed");
		kfree(msg_rsp);
		return ERROR;
	}
	log_info("event report option inform exception");
	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

void recv_exception_information(char *data)
{
	if (!is_cloud_network_enable())
		return;

	device_option_information_report((struct cloud_option_tlv *)data);
}

void recv_aging_information(void)
{
	return;
}

void send_fid_control(__u8 path, __u8 operation, char *data, int length)
{
	struct fid_control fid_ctrl;
	int fid;

	if (!is_cloud_network_enable())
		return;

	fid = get_fid(path);
	if (fid == DETECT_PATH_VAL_ERROR)
		return;

	log_info("path %u operation %u fid %d", path, operation, fid);
	switch (operation) {
	case 0:
		fid_ctrl.fid = fid;
		fid_ctrl.operation = operation;
		send_option_packet(HONOPT_LINK_MANAGE, sizeof(__u8) + sizeof(__u8) + sizeof(fid_ctrl), (char *)(&fid_ctrl));
		break;
	case 1:
		break;
	default:
		return;
	}
	return;
}

void send_option_packet(__u8 type, __u8 len, char *data)
{
	struct sk_buff *skb = NULL;
	struct opt_skb opt;
	u32 timestamp = 0;
	struct list_node *node = NULL;

	if (type <= HONOPT_NOP || type >= HONOPT_MAX || data == NULL || len == 0) {
		log_err("input param is invlaid, error");
		return;
	}

	skb = alloc_option_skb(len);
	if (unlikely(skb == NULL))
		return;

	timestamp = option_timestamp_append(skb, 0);
	if (unlikely(timestamp == 0)) {
		log_err("timestamp is 0");
		dev_kfree_skb_any(skb);
		return;
	}

	option_append(skb, type, len, data);
	skb_put(skb, HMAC_SIZE);
	struct iphdr *iph = ip_hdr(skb);
	struct udphdr *udph = udp_hdr(skb);
	iph->tot_len = htons(IP_SIZE + UDP_SIZE + HON_SIZE + get_options_len(skb) + HMAC_SIZE);
	udph->len = htons(UDP_SIZE + HON_SIZE + get_options_len(skb) + HMAC_SIZE);
	opt.type = SEND_OPTION;
	opt.msg_len = sizeof(struct opt_skb);
	opt.skb = skb;
	opt.timestamp = timestamp;
	node = list_node_alloc((void *)(&opt), sizeof(struct opt_skb));
	if (unlikely(node == NULL)) {
		log_err("node is NULL failed");
		dev_kfree_skb_any(skb);
		return;
	}
	enqueue_list_queue(&option_mgr.option_queue, node);
	schedule_work(&(option_mgr.option_handle_work));
	return;
}

void send_ack_respond(__u64 timestamp)
{
	__u8 val = 1;
	if (unlikely(timestamp == 0)) {
		log_info("timestamp is 0. recv option packet no timestamp or timestamp is 0");
		return;
	}
	send_option_packet(HONOPT_RESPOND, sizeof(__u8) + sizeof(__u8) + sizeof(val), (char *)(&val));
	return;
}

// WARNING: Do not process data in the function.The function only dispatch data.
void option_handle_dispatch(struct honhdr *honh, u8 *option, u16 tot_len)
{
	int offset = sizeof(tot_len);
	u8 type = 0;
	u8 len = 0;
	u64 timestamp = 0;
	struct hon_option_detect *detect = NULL;

	if (!is_cloud_network_enable())
		return;

	if (unlikely(option == NULL || honh == NULL))
		return;

	while (offset < tot_len) {
		type = *(option + offset);
		len = *(option + offset + 1);
		switch (type) {
		case HONOPT_NOP:
			offset++;
			continue;
		case HONOPT_DETECT_RESPOND:
			detect = (struct hon_option_detect *)(option + offset);
			timestamp = ntohl(detect->timestamp);
			detect_packet_recv(honh->fid, (struct hon_option_detect *)(option + offset));
			break;
		case HONOPT_RESPOND:
			log_info("HONOPT_RESPOND");
			recv_option_ack(timestamp);
			break;
		case HONOPT_INFORM_EXCEPTION:
			recv_exception_information(option + offset);
			break;
		case HONOPT_SESSION:
		case HONOPT_FLOW_CONTROL:
		case HONOPT_RESET:
		case HONOPT_DETECT_REQUEST:
		case HONOPT_INFORM_AGING:
		case HONOPT_LINK_MANAGE:
		case HONOPT_SESSION_MP_MANAGE:
		case HONOPT_QOE_SYNC_QOE:
		case HONOPT_QOE_SYNC_ALGORITHM:
		case HONOPT_RETRANSMIT_REQUEST:
		case HONOPT_SHORT_HMAC_START:
		case HONOPT_SHORT_HMAC_STOP:
			break;
		default:
			break;
		}
		offset += len;
	}
	return;
}

void cloud_network_option_stop()
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct opt_skb *opt = NULL;
	log_info("cloud network option stop");
	hrtimer_cancel(&option_mgr.option_timeout_timer);
	cancel_work_sync(&(option_mgr.option_handle_work));
	clean_list_queue(&(option_mgr.option_queue));

	spin_lock(&option_mgr.option_packet_list_lock);
	list_for_each_safe(pos, n, &option_mgr.option_packet_list) {
		opt = list_entry(pos, struct opt_skb, head);
		list_del(pos);
		free_opt_skb(opt);
	}
	spin_unlock(&option_mgr.option_packet_list_lock);
	return;
}

void cloud_network_option_exit()
{
	log_info("cloud network option exit");
	hrtimer_cancel(&option_mgr.option_timeout_timer);
	cancel_work_sync(&(option_mgr.option_handle_work));
	clean_list_queue(&(option_mgr.option_queue));
	return;
}

int cloud_network_option_init()
{
	log_info("cloud network option init");

	init_list_queue(&(option_mgr.option_queue));
	INIT_WORK(&(option_mgr.option_handle_work), cloud_option_work_handle);
	INIT_LIST_HEAD(&option_mgr.option_packet_list);
	spin_lock_init(&(option_mgr.option_packet_list_lock));
	hrtimer_init(&option_mgr.option_timeout_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	option_mgr.option_timeout_timer.function = cloud_option_ack_timeout;
	return SUCCESS;
}
