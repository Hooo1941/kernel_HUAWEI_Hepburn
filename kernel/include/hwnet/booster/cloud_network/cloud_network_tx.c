/*
 * cloud_network_tx.c
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

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/gfp.h>
#include <linux/version.h>
#include <securec.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <linux/if_arp.h>
#include <net/arp.h>

#include "cloud_network_utils.h"
#include "cloud_network_device.h"
#include "cloud_network_rx.h"
#include "cloud_network_signature.h"
#include "cloud_network_param.h"
#include "cloud_network_honhdr.h"
#include "cloud_network_session.h"
#include "cloud_network_option.h"
#include "cloud_network_detect.h"
#include "cloud_network_packet_match.h"
#define HON_PACKET_TRANSMIT_MODULE "hon_packet_transmit_module"

#define SIGNATURE_KEY_SIZE 64
#define DNS_PORT 53

#define MSS_TEST 1300

int cloud_network_set_mac(struct sk_buff* skb, int path);
struct session *session_manage(struct sk_buff* skb);

int get_fid(int id)
{
	switch (id) {
	case ROLLE_PRIMARY_WIFI: /* primary WiFi roll id */
		return DETECT_PATH_VAL_WIFI_1; /* primary WiFi fid */
	case ROLLE_PRIMARY_MODEM: /* primary modem roll id */
		return DETECT_PATH_VAL_MODEM_1; /* primary modem fid */
	case ROLLE_SECONDARY_MODEM: /* secondary modem roll id */
		return DETECT_PATH_VAL_MODEM_2; /* secondary modem fid */
	default :
		return DETECT_PATH_VAL_ERROR;
	}
}

int set_skb_param_by_adapter(struct sk_buff *skb, int id)
{
	struct iphdr *iph = NULL;
	struct honhdr *honh = NULL;

	iph = (struct iphdr *)skb->data;
	honh = hon_hdr(skb);
	int fid = get_fid(id);
	if (fid == ERROR)
		return ERROR;
	honh->fid = fid;
	iph->saddr = cloud_network_inet_select_addr(iph->daddr, id);
	if (iph->saddr == 0)
		return ERROR;
	skb->dev = get_net_device(id);
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

	return SUCCESS;
}

void process_tcp(struct sk_buff *skb, struct tcphdr *tcph)
{
	if (unlikely(!skb))
		return;
	if (tcph->syn)
		tcp_mss_replace(skb, tcph);
}

void process_udp(struct sk_buff *skb, struct udphdr * udph)
{
	if (unlikely(!skb))
		return;
	udph->check = 0;
}

int process_header(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;

	if (unlikely(!skb))
		return ERROR;
	iph = (struct iphdr *)skb->data;
	if (iph->version == 4) {
		if (iph->protocol == IPPROTO_TCP) {
			tcph = (struct tcphdr *)(skb->data + iph->ihl * FOUR_BYTE);
			process_tcp(skb, tcph);
			return 0;
		} else if (iph->protocol == IPPROTO_UDP) {
			return 0;
		} else {
			return 1;
		}
	} else if (iph->version == 6) {
		return 1;
	} else {
		return 1;
	}
	return 0;
}

// warning: host is little endian
void hton24(u8 *p)
{
	u8 tmp = p[0];
	p[0] = p[2];
	p[2] = tmp;
}

void outer_header_init(struct session *ss, struct iphdr *inner_iph, struct iphdr *iph, struct udphdr *udph, struct honhdr *honh, struct hmachdr *hmach)
{
	struct protocol_info info;
	get_protocol_info(&info);
	memset_s(honh, sizeof(struct honhdr), 0, sizeof(struct honhdr));
	honh->version = CLOUD_NETWORK_HON_VERSION;
	honh->compress_flag = HON_CF_NONE;

	hton24(info.app_id);
	memcpy_s(honh->app_id, sizeof(honh->app_id), info.app_id, sizeof(honh->app_id));

	honh->hmac_id = info.hmac_id;
	honh->mp_policy = HON_MP_REDUNDANCY;
	memcpy_s(honh->client_id, sizeof(honh->client_id), info.client_id, sizeof(honh->client_id));
	u32 seq_tmp = atomic_inc_return(&ss->send_seq);
	if (seq_tmp >= MAX_SEQ || seq_tmp == 1) {
		atomic_set(&ss->send_seq, 1);
		seq_tmp = 1;
		honh->x = 1;
	}
	honh->seq = htonl(seq_tmp);
	honh->rsv = 0b10;
	memset_s(udph, sizeof(struct udphdr), 0, sizeof(struct udphdr));
	udph->source = 0x6b45;
	udph->dest = htons(info.compass_port);
	udph->check = 0;

	memset_s(iph, sizeof(struct iphdr), 0, sizeof(struct iphdr));
	iph->version = 4,
	iph->ihl = 5,
	iph->tos = inner_iph->tos,
	iph->id = inner_iph->id,
	iph->frag_off = htons(IP_DF),
	iph->ttl = 64,
	iph->protocol = IPPROTO_UDP,
	iph->check = 0,
	iph->saddr = 0,
	iph->daddr = info.compass_ip;
	memset_s(hmach, sizeof(struct hmachdr), 0, sizeof(struct hmachdr));
}

static int outer_header_set(struct sk_buff *skb, __u16 payload_len, __u8 frag_tot, __u8 frag_off, struct iphdr *iph, struct udphdr *udph, struct honhdr *honh, __u16 option_len, struct hmachdr *hmach)
{
	if (skb_headroom(skb) < MAC_SIZE + HON_EXTRA_SIZE + option_len + REVERSE_SIZE) {
		if (pskb_expand_head(skb, HON_EXTRA_SIZE + MAC_SIZE + option_len, 0, GFP_ATOMIC)) {
			log_err("pskb_expand_head error");
			return ERROR;
		}
	}
	honh->frag_off = frag_off;
	honh->frag_tot = frag_tot;
	honh->payload_length = htons(payload_len);
	udph->len = htons(UDP_SIZE + HON_SIZE + HMAC_SIZE + option_len + payload_len);
	iph->tot_len = htons(IP_SIZE + UDP_SIZE + HON_SIZE + option_len + HMAC_SIZE + payload_len);
	return SUCCESS;
}

void outer_header_push(struct sk_buff *skb, struct iphdr *iph, struct udphdr *udph, struct honhdr *honh, char *option, __u16 option_len, struct hmachdr *hmach)
{
	skb_push(skb, HMAC_SIZE);
	memcpy_s(skb->data, HMAC_SIZE, hmach, HMAC_SIZE);

	skb_push(skb, option_len);
	memcpy_s(skb->data, option_len, option, option_len);

	skb_push(skb, HON_SIZE);
	memcpy_s(skb->data, HON_SIZE, honh, HON_SIZE);

	skb_push(skb, UDP_SIZE);
	memcpy_s(skb->data, UDP_SIZE, udph, UDP_SIZE);
	skb_reset_transport_header(skb);

	skb_push(skb, IP_SIZE);
	memcpy_s(skb->data, IP_SIZE, iph, IP_SIZE);
	skb_reset_network_header(skb);
}

int make_packet_fragment(struct sk_buff *skb, struct iphdr *iph, struct udphdr *udph, struct honhdr *honh, char *option, u16 option_len, struct hmachdr *hmach, struct sk_buff*** skbs)
{
	const int packet_num = 1;

	*skbs = (struct sk_buff **)kmalloc(sizeof(struct sk_buff *) * (packet_num), GFP_ATOMIC);
	if (*skbs == NULL) {
		log_err("%s skbs kmalloc failed");
		dev_kfree_skb_any(skb);
		return 0;
	}
	(*skbs)[0] = skb;
	if (unlikely(outer_header_set((*skbs)[0], skb->len, packet_num, 0, iph,
			udph, honh, option_len, hmach) != SUCCESS)) {
		dev_kfree_skb_any(skb);
		return 0;
	}
	outer_header_push((*skbs)[0], iph, udph, honh, option, option_len, hmach);
	return packet_num;
}

int extend_header(struct session *ss, struct sk_buff *skb, struct sk_buff ***skbs)
{
	struct iphdr *iph = NULL;
	struct iphdr ip;
	struct udphdr udp;
	struct honhdr hon;
	char option[256] = {0};
	char option_len = 0;
	struct hmachdr hmac;
	int packet_num = 0;
	struct much_session_sync much_ss_sync;

	iph = ip_hdr(skb);
	if (atomic_read(&ss->send_seq) == 0 || atomic_read(&ss->send_seq) >= MAX_SEQ - 1) {
		much_ss_sync.cnt = 1;
		much_ss_sync.ss_sync.ip_type = 4;
		much_ss_sync.ss_sync.sip = ss->saddr;
		much_ss_sync.ss_sync.dip = ss->daddr;
		much_ss_sync.ss_sync.sport = ss->source;
		much_ss_sync.ss_sync.dport = ss->dest;
		much_ss_sync.ss_sync.proto_type = ss->protocol;
		option_len = generate_timestamp_option(option, option_len);
		option_len = generate_option(option, option_len, HONOPT_SESSION, sizeof(__u8) + sizeof(__u8) + sizeof(much_ss_sync), (char *)(&much_ss_sync));
	}

	outer_header_init(ss, iph, &ip, &udp, &hon, &hmac);

	if (skb_is_gso(skb))
		log_info("skb is gso!");
	packet_num = make_packet_fragment(skb, &ip, &udp, &hon, option, option_len, &hmac, skbs);

	return packet_num;
}

static struct sk_buff *src_config_set_by_path(int path, struct sk_buff *skb)
{
	int pathnum = -1;
	int source = 0;
	skb_unset_mac_header(skb);
	switch (path) {
	case PATH_WIFI_1:
		pathnum = ROLLE_PRIMARY_WIFI;
		source = 17771; /* WiFi src port */
		break;
	case PATH_MODEM_1:
		pathnum = ROLLE_PRIMARY_MODEM;
		source = 17772; /* modem1 src port */
		break;
	case PATH_MODEM_2:
		pathnum = ROLLE_SECONDARY_MODEM;
		source = 17773; /* modem2 src port */
		break;
	default:
		dev_kfree_skb_any(skb);
		return NULL;
	}
	if (set_skb_param_by_adapter(skb, pathnum)) {
		dev_kfree_skb_any(skb);
		return NULL;
	}
	struct iphdr* iph = (struct iphdr*)skb->data;
	struct udphdr* udph = (struct udphdr*)((((u8 *)iph) + iph->ihl * 4));
	udph->source = htons(source);
	udph->check = 0;
	return skb;
}

/*
 paths: 1(WIFI) 2(MODEM_PRIMARY) 4(MODEM_SECONDARY)
*/
static int skb_send_data_packet(struct sk_buff *skb, u32 paths, struct session *ss)
{
	char digest[BUFFER_SHA256];
	errno_t err;

	if (unlikely(!skb))
		return ERROR;
	if (unlikely(paths == 0)) {
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	skb->mac_len = 0;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	int time = hweight32(paths);
	for (int i = 0b100; i != 0; i >>= 1) {
		if (i & paths) {
			time--;
			struct sk_buff *transmit_skb = NULL;
			if (time > 0) {
				transmit_skb = skb_copy(skb, GFP_ATOMIC);
			} else if (time == 0) {
				transmit_skb = skb;
			} else {
				return ERROR;
			}
			if (unlikely(!transmit_skb))
				continue;
			transmit_skb = src_config_set_by_path(i, transmit_skb);
			if (!transmit_skb) {
				log_err("path %d send error", i);
				continue;
			}
			session_update_tx_dual(ss, transmit_skb, hon_hdr(transmit_skb));
			sign_hon_skb((char *)hon_hdr(transmit_skb), get_honhdr_totlen(transmit_skb) - HMAC_SIZE, digest);
			err = memcpy_s(hmac_hdr(transmit_skb)->hmac, HMAC_SIZE, digest, HMAC_SIZE);
			if (err != EOK)
				log_err("memcpy_s failed");

			if (cloud_network_set_mac(transmit_skb, i) == ERROR) {
				log_err("cloud network set mac error");
				dev_kfree_skb_any(transmit_skb);
				continue;
			}

			(void)check_hon_len(transmit_skb, true);
			if (dev_queue_xmit(transmit_skb) != NET_XMIT_SUCCESS)
				log_err("send skb loss, dev_type = %d", i);
		}
	}
	return SUCCESS;
}

int skb_send(struct sk_buff *skb, int type)
{
	return skb_send_data_packet(skb, type, NULL);
}

int transmit_packet(struct sk_buff *original_skb, struct sk_buff **skbs,
	int packet_num, struct session *ss)
{
	if (unlikely(skbs == NULL || original_skb == NULL))
		return ERROR;

	u32 paths = get_link_path();
	if ((paths & (PATH_MODEM_1 | PATH_MODEM_2)) != 0)
		ims_check_send(original_skb->sk->sk_uid.val);

	for (int i = 0; i < packet_num; i++) {
		if (unlikely(skbs[i] == NULL))
			continue;
		skb_send_data_packet(skbs[i], paths, ss);
	}
	return 0;
}

static int start_report()
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_started *msg_rept = NULL;

	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_started), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return ERROR;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_started);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = STARTED;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_started);

	msg_rept = (struct rept_started *)(msg_header->data);
	msg_rept->status = 4; /* modem + modem */
	log_info("event report data flow started");
	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return SUCCESS;
}

int cloud_network_tx(struct sk_buff *skb, struct net_device *dev)
{
	int packet_num = 0;
	struct sk_buff **skbs = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	if (unlikely(skb == NULL || dev == NULL))
		return ERROR;

	if (!cloud_network_tun_device_skb_match(skb))
		return ERROR;

	if (!skb_uid_match(skb->sk->sk_uid.val, skb))
		return ERROR;

	if (process_header(skb) != 0)
		return ERROR;

	if (!cloud_network_started())
		data_flow_compass_check_start();

	struct session *ss = session_manage(skb);
	if (unlikely(ss == NULL))
		return ERROR;

	iph = (struct iphdr *)(skb->data);
	if (iph->protocol == IPPROTO_UDP)
		udph = (struct udphdr *)(skb->data + iph->ihl * FOUR_BYTE);

	if (!get_data_flow_status() && ((iph->protocol == IPPROTO_UDP &&
		ntohs(udph->dest) != DNS_PORT) || iph->protocol == IPPROTO_TCP)) {
		set_data_flow_status_start();
		start_all_session_print();
		data_flow_compass_check_start();
		start_gain_detection();
		start_report();
	}

	packet_num = extend_header(ss, skb, &skbs);
	if (likely(packet_num != 0))
		transmit_packet(skb, skbs, packet_num, ss);
	kfree(skbs);
	session_put(ss);
	return SUCCESS;
}

static int cloud_network_set_last_mac(struct sk_buff* skb, int path)
{
	char *dest_eth_addr = NULL;

	if (unlikely(skb == NULL || skb->data == NULL || skb->dev == NULL))
		return ERROR;

	dest_eth_addr = get_net_device_mac(path);
	if (unlikely(dest_eth_addr == NULL)) {
		log_err("get net device mac failed, error");
		return ERROR;
	}
	skb->mac_len = MAC_SIZE;
	eth_header(skb, skb->dev, ETH_P_IP, dest_eth_addr, NULL, 0);
	return SUCCESS;
}

int cloud_network_set_mac(struct sk_buff* skb, int path)
{
	struct rtable *rt = NULL;
	struct neighbour *n = NULL;
	struct iphdr *iph = NULL;

	if (unlikely(skb == NULL || skb->data == NULL || skb->dev == NULL))
		return ERROR;

	iph = (struct iphdr*)(skb->data);
	rt = ip_route_output(dev_net(skb->dev), iph->daddr, 0, RT_TOS(0),
		skb->dev->ifindex);
	if ((IS_ERR(rt)) || rt == NULL || rt->rt_gw4 == 0) {
		log_err("ip route output failed, error");
		return cloud_network_set_last_mac(skb, path);
	}

	n = __ipv4_neigh_lookup(skb->dev, rt->rt_gw4);
	ip_rt_put(rt);
	if (IS_ERR(n)) {
		log_err("ipv4 neigh lookup failed, error");
		return cloud_network_set_last_mac(skb, path);
	}

	if (n->ha != 0xe0 && n->ha != 0xe8) {
		skb->mac_len = MAC_SIZE;
		eth_header(skb, skb->dev, ETH_P_IP, n->ha, NULL, 0);
		set_net_device_mac(path, n->ha);
		neigh_release(n);
	} else {
		log_err("neighbour param is invalid, error");
		return cloud_network_set_last_mac(skb, path);
	}

	return SUCCESS;
}

// warning: after session used over use session_put()
struct session *session_manage(struct sk_buff* skb)
{
	struct net* net = NULL;
	struct nf_conntrack_tuple_hash *tmp_tuple_hash = NULL;
	struct session *ss = NULL;
	struct iphdr *iph = NULL;
	struct nf_conntrack_tuple tmp_nf_tuple;

	iph = ip_hdr(skb);
	if ((iph->frag_off & htons(IP_DF)) == 0 && iph->frag_off & htons(IP_OFFSET)) {
			ss = fragment_tx_session_find_get(skb);
			if (ss == NULL) {
				log_err("fragment packet tx not find session, error");
				return NULL;
			}
			tmp_tuple_hash = ss->session_hash;
	} else {
		if (!get_skb_tuple_original(&tmp_nf_tuple, skb))
			return NULL;

		net = dev_net(skb->dev);
		if (net == NULL)
			return NULL;

		tmp_tuple_hash = session_find_get(net, &tmp_nf_tuple);
		if (tmp_tuple_hash != NULL) {
			ss = container_of(tmp_tuple_hash, struct session, session_hash[tmp_tuple_hash->tuple.dst.dir]);
		} else {
			ss = master_session_add_get(&tmp_nf_tuple, skb, skb->sk->sk_uid.val);
			if (ss == NULL)
				return NULL;
			tmp_tuple_hash = ss->session_hash;
		}
	}
	session_update_tx(ss, skb);
	return ss;
}
