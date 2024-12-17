/*
 * cloud_network_rx.c
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

#include "cloud_network_rx.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/inetdevice.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <net/tcp.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <securec.h>
#include <linux/sort.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/gfp.h>
#include <net/xfrm.h>
#include <linux/jhash.h>
#include <linux/rtnetlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_seqadj.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <uapi/linux/netfilter/nf_nat.h>

#include "cloud_network_utils.h"
#include "cloud_network_param.h"
#include "cloud_network_tx.h"
#include "cloud_network_honhdr.h"
#include "cloud_network_signature.h"
#include "cloud_network_option.h"
#define MIN_REC_SEQ 64
#define NS_IN_MS 1000000

struct ooo_skb_ctl {
	struct list_queue work_queue;
	struct work_struct ooo_skb_handle_work;
};

struct recv_ctl *skb_recv_ctl = NULL;
struct memory_pool *ooo_skb_mm_poll = NULL;
static struct statistics_data *data_stat = NULL;
static atomic_t rx_time_table[ROLLE_MAX + 1][TABLE_SIZE] = {0};

static int cloud_network_gro_parse(struct sk_buff *skb);
static int cloud_network_rx_handle(struct hon_skb *hskb);

static struct gain_stat_struct gain_stats;

bool is_gain_state_start(void)
{
	return atomic_read(&gain_stats.status) == 1;
}

static void gain_stat_report(s64 gain, int path)
{
	struct netlink_head *msg_rsp = NULL;
	struct netlink_msg_head *msg_header = NULL;
	struct rept_gain_stat *msg_rept = NULL;

	if (!is_gain_state_start())
		return;

	msg_rsp = (struct netlink_head *)kmalloc(sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_gain_stat), GFP_ATOMIC);
	if (unlikely(msg_rsp == NULL)) {
		log_err("kmalloc failed");
		return;
	}

	msg_rsp->type = CLOUD_NETWORK_REPORT;
	msg_rsp->msg_len = sizeof(struct netlink_head) +
		sizeof(struct netlink_msg_head) +
		sizeof(struct rept_gain_stat);

	msg_header = (struct netlink_msg_head *)(msg_rsp->data);
	msg_header->type = GAIN_REPORT;
	msg_header->data_len = sizeof(struct netlink_msg_head) +
		sizeof(struct rept_gain_stat);

	msg_rept = (struct rept_gain_stat *)(msg_header->data);
	msg_rept->path = path;
	msg_rept->gain = (u32)(gain / NS_IN_MS);
	log_info("event report gain %lld ns, path %d", gain, path);
	netlink_report(msg_rsp, WORK_EXEC);
	kfree(msg_rsp);
	return;
}

static bool is_mulit_gain(struct hrtimer *path_hang_hrtimer)
{
	if (unlikely(path_hang_hrtimer == NULL))
		return false;
	if (!get_ims_status() ||
		(get_tx_rx_state() != TX_RX_STATE_2T2R))
		return false;

	if (!hrtimer_active(path_hang_hrtimer) &&
		hrtimer_active(&(gain_stats.mulit_hang_timer)))
		return true;
	return false;
}

static void restart_hang_timer(struct hrtimer *path_hang_hrtimer)
{
	if (unlikely(path_hang_hrtimer == NULL))
		return;
	hrtimer_cancel(path_hang_hrtimer);
	hrtimer_start(path_hang_hrtimer,
		ms_to_ktime(HANG_TIME_MS),
		HRTIMER_MODE_REL);
	return;
}

static void gain_stat_mulit_update(void)
{
	unsigned long flags = 0;

	if (!is_gain_state_start())
		return;

	if (!hrtimer_active(&(gain_stats.mulit_hang_timer))) {
		spin_lock_irqsave(&(gain_stats.lock), flags);
		if (hrtimer_active(&(gain_stats.primary_hang_timer)))
			gain_stats.secondary_modem_pre_skb_rx_ts =
				gain_stats.primary_modem_pre_skb_rx_ts;
		else if (hrtimer_active(&(gain_stats.secondary_hang_timer)))
			gain_stats.primary_modem_pre_skb_rx_ts =
				gain_stats.secondary_modem_pre_skb_rx_ts;
		else
			log_err("gain timer error");
		spin_unlock_irqrestore(&(gain_stats.lock), flags);
	}

	restart_hang_timer(&(gain_stats.mulit_hang_timer));
	return;
}

static void gain_stat_primary_modem_rx(struct timespec64 ts)
{
	struct timespec64 sub;
	s64 interval_gain_ns;
	unsigned long flags = 0;

	if (!is_gain_state_start())
		return;

	spin_lock_irqsave(&(gain_stats.lock), flags);
	if ((gain_stats.primary_modem_pre_skb_rx_ts.tv_sec != 0) &&
		is_mulit_gain(&(gain_stats.primary_hang_timer))) {
		sub = timespec64_sub(ts, gain_stats.primary_modem_pre_skb_rx_ts);
		interval_gain_ns = timespec64_to_ns(&sub);
		gain_stat_report(interval_gain_ns, DETECT_PATH_VAL_MODEM_2);
	}
	gain_stats.primary_modem_pre_skb_rx_ts = ts;
	spin_unlock_irqrestore(&(gain_stats.lock), flags);

	restart_hang_timer(&(gain_stats.primary_hang_timer));
	return;
}

static void gain_stat_secondary_modem_rx(struct timespec64 ts)
{
	struct timespec64 sub;
	s64 interval_gain_ns;
	unsigned long flags = 0;

	if (!is_gain_state_start())
		return;

	spin_lock_irqsave(&(gain_stats.lock), flags);
	if ((gain_stats.secondary_modem_pre_skb_rx_ts.tv_sec != 0) &&
		is_mulit_gain(&(gain_stats.secondary_hang_timer))) {
		sub = timespec64_sub(ts, gain_stats.secondary_modem_pre_skb_rx_ts);
		interval_gain_ns = timespec64_to_ns(&sub);
		gain_stat_report(interval_gain_ns, DETECT_PATH_VAL_MODEM_1);
	}

	gain_stats.secondary_modem_pre_skb_rx_ts = ts;
	spin_unlock_irqrestore(&(gain_stats.lock), flags);

	restart_hang_timer(&(gain_stats.secondary_hang_timer));
	return;
}

static void gain_stat_rx(struct honhdr *honh)
{
	struct timespec64 cur;

	if (unlikely(!honh))
		return;

	ktime_get_real_ts64(&cur);
	if (honh->fid == DETECT_PATH_VAL_MODEM_1) /* primary modem path */
		gain_stat_primary_modem_rx(cur);
	else if (honh->fid == DETECT_PATH_VAL_MODEM_2) /* secondary modem path */
		gain_stat_secondary_modem_rx(cur);

	return;
}

void start_gain_detection(void)
{
	if (get_ims_status() &&
		(get_tx_rx_state() == TX_RX_STATE_2T2R) &&
		get_data_flow_status()) {
		log_info("gain detection start");
		if (!hrtimer_active(&(gain_stats.primary_hang_timer)))
			hrtimer_start(&(gain_stats.primary_hang_timer),
				ms_to_ktime(HANG_TIME_MS),
				HRTIMER_MODE_REL);
		if (!hrtimer_active(&(gain_stats.secondary_hang_timer)))
			hrtimer_start(&(gain_stats.primary_hang_timer),
				ms_to_ktime(HANG_TIME_MS),
				HRTIMER_MODE_REL);
		if (!hrtimer_active(&(gain_stats.mulit_hang_timer)))
			hrtimer_start(&(gain_stats.mulit_hang_timer),
				ms_to_ktime(HANG_TIME_MS),
				HRTIMER_MODE_REL);
		atomic_set(&gain_stats.status, 1);
	}

	return;
}

static enum hrtimer_restart hang_timer(struct hrtimer *timer, int path)
{
	if (unlikely(timer == NULL)) {
		log_err("NULL Pointer, error");
		return HRTIMER_NORESTART;
	}

	log_info("timeout path %d", path);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart primary_hang_timer(struct hrtimer *timer)
{
	return hang_timer(timer, DETECT_PATH_VAL_MODEM_1);
}

static enum hrtimer_restart secondary_hang_timer(struct hrtimer *timer)
{
	return hang_timer(timer, DETECT_PATH_VAL_MODEM_2);
}

static enum hrtimer_restart mulit_hang_timer(struct hrtimer *timer)
{
	struct timespec64 sub;
	s64 interval_gain_ns = 0;
	unsigned long flags = 0;
	int gain_path = 0;

	spin_lock_irqsave(&(gain_stats.lock), flags);
	if (!((gain_stats.primary_modem_pre_skb_rx_ts.tv_sec == 0) ||
		(gain_stats.secondary_modem_pre_skb_rx_ts.tv_sec == 0))) {
		int timespec64_compare_ret =
			timespec64_compare(&(gain_stats.primary_modem_pre_skb_rx_ts),
				&(gain_stats.secondary_modem_pre_skb_rx_ts));
		if (timespec64_compare_ret > 0) {
			sub = timespec64_sub(gain_stats.primary_modem_pre_skb_rx_ts,
			gain_stats.secondary_modem_pre_skb_rx_ts);
			interval_gain_ns = timespec64_to_ns(&sub);
			gain_path = DETECT_PATH_VAL_MODEM_1;
		}

		if (timespec64_compare_ret < 0) {
			sub = timespec64_sub(gain_stats.secondary_modem_pre_skb_rx_ts,
			gain_stats.primary_modem_pre_skb_rx_ts);
			interval_gain_ns = timespec64_to_ns(&sub);
			gain_path = DETECT_PATH_VAL_MODEM_2;
		}
	}
	spin_unlock_irqrestore(&(gain_stats.lock), flags);

	if (gain_path != 0)
		gain_stat_report(interval_gain_ns, gain_path);
	return hang_timer(timer, 5);
}

void stop_gain_detection(void)
{
	log_info("stop gain stat schedule");
	atomic_set(&gain_stats.status, 0);
	hrtimer_cancel(&(gain_stats.primary_hang_timer));
	hrtimer_cancel(&(gain_stats.secondary_hang_timer));
	hrtimer_cancel(&(gain_stats.mulit_hang_timer));
	(void)memset_s(&(gain_stats.primary_modem_pre_skb_rx_ts),
		sizeof(struct timespec64), 0x00, sizeof(struct timespec64));
	(void)memset_s(&(gain_stats.secondary_modem_pre_skb_rx_ts),
		sizeof(struct timespec64), 0x00, sizeof(struct timespec64));

	return;
}

static void gain_stat_deinit(void)
{
	log_info("deinit gain stat schedule");
	stop_gain_detection();
	return;
}

static void gain_stat_init(void)
{
	log_info("init gain stat schedule");
	atomic_set(&gain_stats.status, 0);
	hrtimer_init(&(gain_stats.primary_hang_timer),
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gain_stats.primary_hang_timer.function =
		primary_hang_timer;

	hrtimer_init(&(gain_stats.secondary_hang_timer),
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gain_stats.secondary_hang_timer.function =
		secondary_hang_timer;

	hrtimer_init(&(gain_stats.mulit_hang_timer),
		CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gain_stats.mulit_hang_timer.function =
		mulit_hang_timer;

	(void)memset_s(&(gain_stats.primary_modem_pre_skb_rx_ts),
		sizeof(struct timespec64), 0x00, sizeof(struct timespec64));
	(void)memset_s(&(gain_stats.secondary_modem_pre_skb_rx_ts),
		sizeof(struct timespec64), 0x00, sizeof(struct timespec64));

	spin_lock_init(&(gain_stats.lock));
	return;
}

/* the table will be step by (0, 5], (5, 10], ..., (195, 200], (200, +infinite) */
static void table_set(u32 val, int id)
{
	if (val > EXPECTED_NETWORK_LATENCY_MS) {
		atomic_inc(&rx_time_table[id][TABLE_SIZE - 1]);
	} else if (val == 0) {
		atomic_inc(&rx_time_table[id][0]);
	} else {
		int step = (val - 1) / STEP_MS;
		atomic_inc(&rx_time_table[id][step]);
	}
}

u32 get_rel_time_us(void)
{
	u64 time_us = 0;
	struct timespec64 tv;
	(void)memset_s(&tv, sizeof(tv), 0, sizeof(tv));
	ktime_get_real_ts64(&tv);
	time_us = tv.tv_sec * USEC_PER_SEC + tv.tv_nsec / NSEC_PER_USEC;
	return (u32)time_us;
}

u64 get_rel_time_ms(void)
{
	u64 time_ms = 0;
	struct timespec64 tv;
	(void)memset_s(&tv, sizeof(tv), 0, sizeof(tv));
	ktime_get_real_ts64(&tv);
	time_ms = tv.tv_sec * MS_IN_SECOND + tv.tv_nsec / NSEC_PER_MSEC;
	return time_ms;
}

void cloud_data_stat(u32 seq, int id)
{
	if (id == ERROR) {
		log_err("dev is error");
		return;
	}
	if (data_stat == NULL) {
		log_err("data_stat == NULL");
		return;
	}
	u32 tmp_max = atomic_read(&data_stat->info[id].max_seq);
	u32 tmp_min = atomic_read(&data_stat->info[id].min_seq);
	atomic_inc(&data_stat->info[id].packets_rec);
	if (tmp_max < seq)
		atomic_set(&data_stat->info[id].max_seq, seq);
	if (tmp_min > seq)
		atomic_set(&data_stat->info[id].min_seq, seq);
	return;
}

static bool skb_rx_match(struct sk_buff *skb, struct net_device *dev)
{
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;

	if (skb->protocol != htons(ETH_P_IP))
		return false;

	if (!pskb_may_pull(skb, sizeof(*iph) + sizeof(*udph)))
		return false;

	iph = (struct iphdr *)(skb->data);
	if (iph->protocol != IPPROTO_UDP)
		return false;

	udph = (struct udphdr *)(skb->data + iph->ihl * 4);
	return protocol_info_match(iph->saddr, udph->source);
}

static inline unsigned int tcp_opt_len(const u_int8_t *opt,
	unsigned int offset)
{
	if (opt[offset] <= TCPOPT_NOP || opt[offset + 1] == 0)
		return 1;
	else
		return opt[offset + 1];
}

u64 ntohll(u64 val)
{
	return (((u64) ntohl(val)) << 32) + ntohl(val >> 32);
}

u64 htonll(u64 val)
{
	return (((u64) htonl(val)) << 32) + htonl(val >> 32);
}

void tcp_mss_replace(struct sk_buff *skb, struct tcphdr *th)
{
	int tcp_hdrlen;
	unsigned int i;
	u16 newmss;
	u8 *opt = NULL;

	if (unlikely(skb == NULL || th == NULL))
		return;

	tcp_hdrlen = th->doff * 4;
	newmss = CLOUD_NETWORK_TCP_MSS;

	opt = (u_int8_t *)th;
	for (i = sizeof(struct tcphdr); i <= tcp_hdrlen - TCPOLEN_MSS;
		i += tcp_opt_len(opt, i)) {
		if (opt[i] == TCPOPT_MSS && opt[i + 1] == TCPOLEN_MSS) {
			u_int16_t oldmss;
			/* get old MSS */
			oldmss = (opt[i + 2] << 8) | opt[i + 3];
			/* Never increase MSS */
			if (oldmss <= newmss)
				return;

			/* set new MSS */
			opt[i + 2] = (newmss & 0xff00) >> 8;
			opt[i + 3] = newmss & 0x00ff;

			inet_proto_csum_replace2(&th->check, skb,
						 htons(oldmss), htons(newmss),
						 false);
			return;
		}
	}
	return;
}

static void cloud_network_rx_deliver_to_stack(struct session *ss, struct sk_buff *skb)
{
	if (likely(ss != NULL))
		session_put(ss);

	if (unlikely(skb == NULL))
		return;

	netif_rx_any_context(skb);

	return;
}

static void recv_timeout_start(struct recv_ctl *ctl)
{
	if (unlikely(ctl == NULL))
		return;

	if ((RB_EMPTY_ROOT(&(ctl->out_of_order))) ||
		!test_bit(RECEIVE_CONTROL_STATUS, &ctl->status))
		return;

	if (!(hrtimer_active(&ctl->recv_timeout)))
		hrtimer_start(&ctl->recv_timeout,
			ms_to_ktime(SKB_RECEIVE_TIMEOUT_TIMER_INTERVAL_MS),
			HRTIMER_MODE_REL_PINNED);
	return;
}

struct session *session_reply_manage(struct sk_buff* skb)
{
	struct nf_conntrack_tuple tuple;
	struct session *ss = NULL;
	struct nf_conntrack_tuple_hash *tmp_tuple_hash = NULL;
	struct net* net = dev_net(skb->dev);
	if (!get_skb_tuple_reply(&tuple, skb)) {
		log_err("get skb reply tuple failed, error");
		return NULL;
	}

	tmp_tuple_hash = session_find_get(net, &tuple);
	if (tmp_tuple_hash == NULL) {
		log_err("tmp_tuple_hash == NULL");
		return NULL;
	}
	ss = container_of(tmp_tuple_hash, struct session, session_hash[tmp_tuple_hash->tuple.dst.dir]);
	return ss;
}

static void set_received(struct session *ss, u32 seq)
{
	int offset = seq % (SLIDE_WIN_SIZE);
	ss->ss_window[offset] = 1;
	return;
}

void all_window_reset(struct session *ss)
{
	(void)memset_s(ss->ss_window, sizeof(ss->ss_window), 0x00, sizeof(ss->ss_window));
	ss->begin = 0;
	ss->end = SLIDE_WIN_SIZE;
	return;
}

static void slid_half_window(struct session *ss)
{
	int offset = ss->begin % SLIDE_WIN_SIZE;
	log_info("begin %llu end %llu offset %d", ss->begin, ss->end, offset);
	(void)memset_s(&ss->ss_window[offset], sizeof(ss->ss_window) / SLIDE_HALF,
		0x00, sizeof(ss->ss_window) / SLIDE_HALF);
	ss->end += SLIDE_WIN_SIZE / SLIDE_HALF;
	ss->begin += SLIDE_WIN_SIZE / SLIDE_HALF;
	return;
}

static bool is_received(struct session *ss, u32 seq)
{
	int offset = seq % (SLIDE_WIN_SIZE);
	return ss->ss_window[offset] == 1;
}

static int cloud_network_rx_hon_seq(struct session *ss, struct hon_skb *hskb)
{
	struct sk_buff *skb = NULL;
	struct honhdr *honh = NULL;
	u64 timestamp;

	if (unlikely(ss == NULL))
		return GRO_DROP;
	if (unlikely(hskb == NULL || hskb->skb == NULL)) {
		session_put(ss);
		return GRO_DROP;
	}

	skb = hskb->skb;
	honh = &hskb->hon;
	timestamp = ktime_get_real_ns();
	gain_stat_rx(honh);

	spin_lock_bh(&ss->lock);
	if (is_compass_changing()) {
		spin_unlock_bh(&ss->lock);
		cloud_network_rx_deliver_to_stack(ss, skb);
		return GRO_DROP;
	}
	u32 seq = ntohl(honh->seq);
	if (seq < MIN_REC_SEQ || seq < ss->begin) {
		spin_unlock_bh(&ss->lock);
		cloud_network_rx_deliver_to_stack(ss, skb);
		gain_stat_mulit_update();
		return GRO_DROP;
	}
	session_update_rx_dual(ss, skb, honh, timestamp);
	while (seq >= ss->end)
		slid_half_window(ss);
	if (is_received(ss, seq)) {
		spin_unlock_bh(&ss->lock);
		session_put(ss);
		free_hon_skb(hskb);
		hskb = NULL;
		return GRO_DROP;
	}
	gain_stat_mulit_update();
	session_update_rx(ss, skb, honh, timestamp);
	set_received(ss, seq);
	spin_unlock_bh(&ss->lock);
	cloud_network_rx_deliver_to_stack(ss, skb);
	return GRO_DROP;
}

static int cloud_network_rx_handle(struct hon_skb *hskb)
{
	struct sk_buff *skb = NULL;
	struct net_device *dst_dev = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct session *ss = NULL;
	if (unlikely(hskb == NULL))
		return -1;

	skb = hskb->skb;
	iph = (struct iphdr *)skb->data;
	if (iph->protocol == IPPROTO_TCP) {
		tcph = (struct tcphdr *)(skb->data + iph->ihl * 4); /* get tcp head */
		if (tcph->syn)
			tcp_mss_replace(skb, tcph);
	}
	if (iph->daddr == 0) {
		log_err("daddr is invalid, error");
		free_hon_skb(hskb);
		hskb = NULL;
		return GRO_DROP;
	}

	dst_dev = __ip_dev_find(&init_net, iph->daddr, false);
	if (dst_dev == NULL) {
		log_err("rx cannot find dst dev");
		free_hon_skb(hskb);
		hskb = NULL;
		return GRO_DROP;
	}

	skb->dev = dst_dev;
	skb->skb_iif = dst_dev->ifindex;
	skb->mac_len = 0;
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, ip_hdr(skb)->ihl * 4);
	if ((iph->frag_off & htons(IP_DF)) == 0 && iph->frag_off & htons(IP_OFFSET)) {
		ss = fragment_rx_session_find_get(skb);
	} else {
		ss = session_reply_manage(skb);
	}
	if (unlikely(ss == NULL)) {
		log_err("rx session not found");
		free_hon_skb(hskb);
		hskb = NULL;
		return GRO_DROP;
	}
	ims_check_recv(ss);
	return cloud_network_rx_hon_seq(ss, hskb);
}

static bool skb_rx_valid_check(struct sk_buff *skb)
{
	if (unlikely(skb == NULL))
		return false;

	if (!check_sign_hon_skb((char *)hon_hdr(skb),
		sizeof(struct honhdr) + get_options_len(skb), hon_hdr(skb)->hmac_id,
		hmac_hdr(skb)))
		return false;

	if (!check_hon_len(skb, false))
		return false;

	return true;
}

static int cloud_network_normal_parse(struct sk_buff *skb, struct net_device *dev)
{
	struct hon_skb *hskb = NULL;

	if (unlikely(skb == NULL || dev == NULL))
		return GRO_NORMAL;

	if (!skb_rx_valid_check(skb)) {
		dev_kfree_skb_any(skb);
		skb = NULL;
		return GRO_DROP;
	}

	if (is_option(skb))
		option_handle_dispatch(hon_hdr(skb), hon_option(skb),
		get_options_len(skb));

	if (hon_hdr(skb)->payload_length == 0) {
		dev_kfree_skb_any(skb);
		skb = NULL;
		return GRO_DROP;
	}

	hskb = alloc_hon_skb(hon_hdr(skb), skb);
	if (unlikely(hskb == NULL)) {
		dev_kfree_skb_any(skb);
		skb = NULL;
		return GRO_DROP;
	}

	skb_pull(skb, IP_SIZE + UDP_SIZE + get_honhdr_totlen(skb));
	return cloud_network_rx_handle(hskb);
}

static int skb_gro_parse_no_option(struct sk_buff *skb, struct honhdr *honh,
	u32 offset)
{
	struct hmachdr hmach;

	if (unlikely(skb == NULL || honh == NULL))
		return LOOP_BREAK;

	if (honh->payload_length == 0) {
		log_err("payload_length is invalid, error");
		return LOOP_CONTINUE;
	}

	if (!check_hon_len_gro(honh))
		return LOOP_BREAK;

	if (skb_copy_bits(skb, offset + sizeof(struct honhdr), &hmach,
		HMAC_SIZE) != 0) {
		log_err("skb copy bits hmach failed, error");
		return LOOP_CONTINUE;
	}
	if (!check_sign_hon_skb((char *)honh, sizeof(struct honhdr),
		honh->hmac_id, &hmach))
		return LOOP_CONTINUE;

	return LOOP_SUCCESS;
}

static int skb_gro_parse_option(struct sk_buff *skb, struct honhdr *honh,
	u32 offset, u16 *option_len)
{
	struct hmachdr hmach;
	char *option = NULL;

	if (unlikely(skb == NULL || honh == NULL || option_len == NULL))
		return LOOP_BREAK;

	if (skb_copy_bits(skb, offset + sizeof(struct honhdr), option_len,
		sizeof(*option_len)) != 0) {
		log_err("skb copy bits option_len failed, error");
		return LOOP_BREAK;
	}

	*option_len = htons(*option_len);
	if (unlikely(*option_len < NUM_4)) {
		log_err("option_len is invalid, error");
		return LOOP_BREAK;
	}

	if (skb_copy_bits(skb, offset + sizeof(struct honhdr) + *option_len,
		&hmach, HMAC_SIZE) != 0) {
		log_err("skb copy bits hmach failed, error");
		return LOOP_CONTINUE;
	}

	option = (char *)kmalloc(sizeof(struct honhdr) + *option_len, GFP_ATOMIC);
	if (unlikely((option == NULL) ||
		(skb_copy_bits(skb, offset, option, sizeof(struct honhdr) +
		*option_len) != 0) ||
		!check_sign_hon_skb(option, sizeof(struct honhdr) + *option_len,
		honh->hmac_id, &hmach))) {
		log_err("kmalloc or skb copy bits or sign failed, error");
		if (option != NULL)
			kfree(option);
		return LOOP_CONTINUE;
	}

	option_handle_dispatch(honh, option + sizeof(struct honhdr), *option_len);
	kfree(option);
	option = NULL;

	if (honh->payload_length == 0)
		return LOOP_CONTINUE;

	return LOOP_SUCCESS;
}

static int skb_gro_parse_data(struct sk_buff *skb, struct honhdr *honh,
	u32 offset, u16 option_len, u32 payload_len, u32 head_len, u32 tail_len)
{
	if (unlikely(skb == NULL || honh == NULL))
		return LOOP_BREAK;

	struct sk_buff *skb_new = alloc_skb(payload_len + head_len + tail_len, GFP_ATOMIC);
	if (!skb_new) {
		log_err("alloc skb failed, error");
		return LOOP_BREAK;
	}

	skb_new->dev = skb->dev;
	skb_new->len = payload_len;
	skb_new->mac_len = 0;
	skb_new->protocol = skb->protocol;
	skb_new->sk = skb->sk;
	skb_new->data = skb_new->data + head_len;
	skb_new->tail = skb_new->tail + head_len + payload_len;

	if (skb_copy_bits(skb, offset + sizeof(struct honhdr) + option_len +
		HMAC_SIZE, skb_new->data, payload_len) != 0) {
		log_err("copy bits skb data failed, error");
		dev_kfree_skb_any(skb_new);
		return LOOP_CONTINUE;
	}

	struct hon_skb *hskb = alloc_hon_skb(honh, skb_new);
	if (hskb == NULL)
		dev_kfree_skb_any(skb_new);
	else
		cloud_network_rx_handle(hskb);

	return LOOP_SUCCESS;
}

static int cloud_network_gro_parse(struct sk_buff *skb)
{
	int len = skb->len - IP_SIZE - UDP_SIZE;
	u32 offset =  IP_SIZE + UDP_SIZE;
	int head_len = skb_headroom(skb);
	int tail_len = skb_tailroom(skb);
	struct honhdr honh;
	int ret;

	while (len > 0) {
		if (skb_copy_bits(skb, offset, &honh, sizeof(struct honhdr)) != 0) {
			log_err("skb copy bits honh failed, error");
			break;
		}

		u32 payload_len = ntohs(honh.payload_length);
		u16 option_len = 0;
		if (honh.x == 1)
			ret = skb_gro_parse_option(skb, &honh, offset, &option_len);
		else
			ret = skb_gro_parse_no_option(skb, &honh, offset);

		if (ret == LOOP_CONTINUE) {
			len -= sizeof(struct honhdr) + option_len + HMAC_SIZE + payload_len;
			offset += sizeof(struct honhdr) + option_len + HMAC_SIZE + payload_len;
			continue;
		} else if (ret == LOOP_BREAK) {
			break;
		}

		if ((payload_len + HON_SIZE + option_len + HMAC_SIZE + offset) > skb->len) {
			log_err("len is invalid, error");
			break;
		}

		ret = skb_gro_parse_data(skb, &honh, offset, option_len, payload_len, head_len, tail_len);
		if (ret == LOOP_BREAK)
			break;

		len -= sizeof(struct honhdr) + option_len + HMAC_SIZE + payload_len;
		offset += sizeof(struct honhdr) + option_len + HMAC_SIZE + payload_len;
	}

	if (len != 0)
		log_err("parse failed, error");

	dev_kfree_skb_any(skb);
	skb = NULL;
	return GRO_DROP;
}

int cloud_network_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct iphdr *iph = NULL;

	if (unlikely(skb == NULL || dev == NULL))
		return GRO_NORMAL;

	if (skb_rx_match(skb, dev) == false)
		return GRO_NORMAL;

	iph = (struct iphdr *)(skb->data);
	if (iph->frag_off & htons(IP_MF)) {
		log_err("receive fragment skb");
		dev_kfree_skb_any(skb);
		return GRO_DROP;
	}

	if (skb->data_len != 0)
		return cloud_network_gro_parse(skb);

	return cloud_network_normal_parse(skb, dev);
}

static void ooo_skb_queue_clear(struct ooo_skb *skb_ooo)
{
	struct sk_buff *skb = NULL;

	while (skb = __skb_dequeue(&(skb_ooo->skb_queue)))
		cloud_network_rx_deliver_to_stack(NULL, skb);
	return;
}

static void skb_rbtree_clear(struct recv_ctl *ctl)
{
	struct rb_node *p = NULL;

	if (unlikely(ctl == NULL))
		return;

	spin_lock_bh(&(ctl->out_of_order_lock));

	if (RB_EMPTY_ROOT(&(skb_recv_ctl->out_of_order))) {
		spin_unlock_bh(&(ctl->out_of_order_lock));
		return;
	}

	p = rb_first(&ctl->out_of_order);
	while (p) {
		struct ooo_skb *skb_ooo = rb_entry(p, struct ooo_skb, rbnode);
		p = rb_next(p);
		rb_erase(&(skb_ooo->rbnode), &ctl->out_of_order);
		ooo_skb_queue_clear(skb_ooo);
		memory_pool_free((void *)skb_ooo, ooo_skb_mm_poll);
	}
	ctl->ooo_last = NULL;
	spin_unlock_bh(&(ctl->out_of_order_lock));
	return;
}

static enum hrtimer_restart receive_timeout_handle(struct hrtimer *timer)
{
	struct recv_ctl *ctl = NULL;

	if (unlikely(timer == NULL))
		return HRTIMER_NORESTART;

	ctl = container_of(timer, struct recv_ctl, recv_timeout);
	if (unlikely(!test_bit(RECEIVE_CONTROL_STATUS, &ctl->status)))
		return HRTIMER_NORESTART;

	tasklet_schedule(&(ctl->recv_timeout_handle_tasklet));

	return HRTIMER_NORESTART;
}

static void recv_timeout_handle_tasklet(unsigned long arg)
{
	struct recv_ctl *ctl = NULL;
	struct rb_node *p = NULL;
	struct ooo_skb *skb_ooo = NULL;
	u32 time = 0;

	if (unlikely((struct recv_ctl *)arg == NULL))
		return;

	ctl = (struct recv_ctl *)arg;

	spin_lock_bh(&(ctl->out_of_order_lock));
	p = rb_first(&ctl->out_of_order);
	while (p) {
		skb_ooo = rb_entry_safe(p, struct ooo_skb, rbnode);

		time = jiffies_to_msecs(tcp_jiffies32 - skb_ooo->tstamp);
		if (time < SKB_RECEIVE_TIMEOUT_MS) {
			break;
		}
		ooo_skb_queue_clear(skb_ooo);
		memory_pool_free((void *)skb_ooo, ooo_skb_mm_poll);

		p = rb_next(p);
		rb_erase(&skb_ooo->rbnode, &ctl->out_of_order);
	}

	spin_unlock_bh(&(ctl->out_of_order_lock));
	recv_timeout_start(ctl);
	return;
}

void skb_recv_ctl_init(struct recv_ctl *ctl)
{
	log_info("init");

	if (unlikely(ctl == NULL))
		return;

	if (unlikely(test_bit(RECEIVE_CONTROL_STATUS, &ctl->status)))
		return;

	spin_lock_init(&(ctl->recv_ctl_lock));
	ctl->recv_wnd = RECEIVE_WINDOW_DEFAULT;
	ctl->recv_nxt = 0;
	ctl->out_of_order = RB_ROOT;
	spin_lock_init(&(ctl->out_of_order_lock));
	ctl->ooo_last = NULL;
	hrtimer_init(&ctl->recv_timeout, CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	ctl->recv_timeout.function = receive_timeout_handle;
	tasklet_init(&(ctl->recv_timeout_handle_tasklet),
		recv_timeout_handle_tasklet, (unsigned long)ctl);
	set_bit(RECEIVE_CONTROL_STATUS, &ctl->status);
	return;
}

void skb_recv_ctl_deinit(struct recv_ctl *ctl)
{
	log_info("deinit");

	if (unlikely(ctl == NULL))
		return;

	if (unlikely(!test_bit(RECEIVE_CONTROL_STATUS, &ctl->status)))
		return;

	clear_bit(RECEIVE_CONTROL_STATUS, &ctl->status);
	hrtimer_cancel(&(ctl->recv_timeout));
	skb_rbtree_clear(ctl);
	ctl->recv_wnd = 0;
	ctl->recv_nxt = 0;
	ctl->out_of_order = RB_ROOT;
	ctl->ooo_last = NULL;
	return;
}

void data_stat_deinit(void)
{
	log_info("data_stat deinit");

	if (data_stat == NULL)
		return;
	hrtimer_cancel(&data_stat->recv_statistics_data);
	kfree(data_stat);
	data_stat = NULL;
}

int skb_recv_ctl_create(struct recv_ctl **ctl)
{
	if (unlikely(ctl == NULL))
		return ERROR;

	if (likely(*ctl == NULL)) {
		log_err("receive control alloc");
		*ctl = (struct recv_ctl *)kmalloc(sizeof(struct recv_ctl),
			GFP_ATOMIC);
		if (unlikely(*ctl == NULL)) {
			log_err("kmalloc failed");
			return ERROR;
		}
		(void)memset_s(*ctl, sizeof(struct recv_ctl), 0x00,
			sizeof(struct recv_ctl));
	}
	clear_bit(RECEIVE_CONTROL_STATUS, &(*ctl)->status);
	return SUCCESS;
}

void skb_recv_ctl_destyoy(struct recv_ctl **ctl)
{
	log_info("destyoy");

	if (unlikely(ctl == NULL || *ctl == NULL))
		return;

	kfree(*ctl);
	*ctl = NULL;
	return;
}

static void ooo_skb_mm_poll_item_init(void *data, u32 data_size)
{
	struct ooo_skb *skb_ooo = NULL;

	if (unlikely(data == NULL || data_size != sizeof(struct ooo_skb)))
		return;

	skb_ooo = (struct ooo_skb *)data;

	RB_CLEAR_NODE(&(skb_ooo->rbnode));
	skb_queue_head_init(&(skb_ooo->skb_queue));
	skb_ooo->seq_start = 0;
	skb_ooo->seq_end = 0;
	skb_ooo->tstamp = 0;
}

void cloud_network_data_rx_exit(void)
{
	log_info("cloud network rx exit");
	gain_stat_deinit();
	return;
}

int cloud_network_rx_init(void)
{
	log_info("cloud network rx init");
	gain_stat_init();
	return SUCCESS;
}

struct hon_skb *combine_packets(struct hon_skb **hon_skbs, int nums)
{
	struct sk_buff *skb = NULL;
	struct hon_skb *hskb = NULL;
	u16 length = 0;
	int i = 0;
	bool error = false;
	for (i = 0; i < nums; i++) {
		if (hon_skbs == NULL || hon_skbs[i]->skb == NULL) {
			error = true;
			log_err("%s hon_skbs[i]->skb == NULL", __func__);
			break;
		}
		length += hon_skbs[i]->skb->len;
	}
	if (error) {
		for (i = 0; i < nums; i++) {
			if (hon_skbs[i]->skb != NULL) {
				dev_kfree_skb_any(hon_skbs[i]->skb);
				hon_skbs[i]->skb = NULL;
			}
		}
		return NULL;
	}

	if (nums == 1)
		return hon_skbs[0];

	skb = alloc_skb(length + REVERSE_SIZE, GFP_ATOMIC);
	hskb = alloc_hon_skb(&(hon_skbs[0]->hon), skb);
	if (hskb == NULL) {
		dev_kfree_skb_any(skb);
		return NULL;
	}

	hskb->hon.payload_length = htons(length);
	hskb->hon.frag_off = 0;
	hskb->hon.frag_tot = 1;
	for (i = 0; i < nums; i++) {
		memcpy_s(skb->tail, hon_skbs[i]->skb->len, hon_skbs[i]->skb->data, hon_skbs[i]->skb->len);
		skb_put(skb, hon_skbs[i]->skb->len);
		free_hon_skb(hon_skbs[i]);
		hon_skbs[i] = NULL;
	}
	return hskb;
}
