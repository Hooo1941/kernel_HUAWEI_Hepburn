
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is detect whether the BPF
 *              and iptables rule drop the skb.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2019-04-18
 */

#include "hw_packet_filter_bypass.h"

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/uidgid.h>
#include <linux/sched/task.h>
#include <net/sock.h>
#include <net/inet_sock.h>
#include <securec.h>

#include "hw_booster_common.h"

#define NOTIFY_BUF_LEN 512
#define MAX_LISTENING_UID_NUM 10

#define REPORT_THRESHOLD_PKTS 1
#define IP_REPORT_THRESHOLD_PKTS 10
#define DETECT_PEIROID (30 * HZ)

enum report_hooks {
	HOOK_IP = 0,
	HOOK_EGRESS,
	HOOK_INGRESS,
	HOOK_LOCAL_OUT,
	HOOK_LOCAL_IN,
};

struct packet_drop_stats {
	int hook;
	u32 pkts;
	u32 dns_drop_count;
	unsigned long ts_reset;

	bool reported_drop;
	bool reported_recovery;
};

struct uid_info {
	int uid;
	bool bypass;

	struct packet_drop_stats ip_in_stats;
	struct packet_drop_stats local_out_drop_stats;
	struct packet_drop_stats bpf_egress_drop_stats;
	struct packet_drop_stats local_in_drop_stats;
	struct packet_drop_stats bpf_ingress_drop_stats;
};

struct listening_uids {
	struct uid_info infos[MAX_LISTENING_UID_NUM];
	int nums;

	spinlock_t lock;
};

static struct listening_uids listen_uids;
static struct detect_uids g_socket_close_detect_uids;
static notify_event *notifier = NULL;

static bool g_is_initial_socket_close_timer = false;
static int g_socket_close_index;
static struct chr_socket_close_info g_socket_close_uids_info[MAX_CHR_SOCKET_CLOSE_UID_NUM];
static struct timer_list g_socket_close_timer;
static spinlock_t g_socket_close_lock;

static void reset_uid_info(struct uid_info *info, int uid)
{
	memset(info, 0, sizeof(struct uid_info));
	info->uid = uid;
	info->ip_in_stats.hook = HOOK_IP;
	info->bpf_egress_drop_stats.hook = HOOK_EGRESS;
	info->bpf_ingress_drop_stats.hook = HOOK_INGRESS;
	info->local_in_drop_stats.hook = HOOK_LOCAL_IN;
	info->local_out_drop_stats.hook = HOOK_LOCAL_OUT;
}

static void reset_detect_uid()
{
	spin_lock_bh(&g_socket_close_detect_uids.lock);
	(void)memset_s(g_socket_close_detect_uids.uids, MAX_SOCKET_CLOSE_DETECT_NUM * sizeof(int), 0,
		MAX_SOCKET_CLOSE_DETECT_NUM * sizeof(int));
	spin_unlock_bh(&g_socket_close_detect_uids.lock);
}

static bool add_listening_uid(int uid)
{
	int i;

	if (listen_uids.nums >= MAX_LISTENING_UID_NUM)
		return false;
	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			reset_uid_info(&listen_uids.infos[i], uid);
			return true;
		}
	}
	i = listen_uids.nums;
	reset_uid_info(&listen_uids.infos[i], uid);
	++listen_uids.nums;
	return true;
}

static bool del_listening_uid(int uid)
{
	int i;

	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			for (; i < (listen_uids.nums - 1); ++i)
				listen_uids.infos[i] = listen_uids.infos[i + 1];
			--listen_uids.nums;
			return true;
		}
	}
	return false;
}

static bool upate_bypass_flag(int uid, bool bypass)
{
	int i;

	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid) {
			listen_uids.infos[i].bypass = bypass;
			return true;
		}
	}
	return false;
}


static void add_socket_close_detect_uid(int uid)
{
	int i = 0;

	for (i = 0; i < MAX_SOCKET_CLOSE_DETECT_NUM; ++i) {
		if (g_socket_close_detect_uids.uids[i] == 0) {
			g_socket_close_detect_uids.uids[i] = uid;
			break;
		}
	}
}

static void del_socket_close_detect_uid(int uid)
{
	int i = 0;

	for (i = 0; i < MAX_SOCKET_CLOSE_DETECT_NUM; ++i) {
		if (g_socket_close_detect_uids.uids[i] == uid) {
			g_socket_close_detect_uids.uids[i] = 0;
			break;
		}
	}
}

static void socket_close_detect_uids_change(int type, int num, char *p)
{
	int i, uid;

	spin_lock_bh(&g_socket_close_detect_uids.lock);
	for (i = 0; i < num; ++i) {
		uid = *(int *) p;
		if (uid < SYSTEM_UID_MAX)
			break;
		if (type == SOCKET_CLOSE_DETECT_UID_ADD)
			add_socket_close_detect_uid(uid);
		else
			del_socket_close_detect_uid(uid);
		p += sizeof(int);
	}
	spin_unlock_bh(&g_socket_close_detect_uids.lock);
}

static void update_listen_uids(int type, int num, char *p)
{
	int i, uid;
	bool ret = false;

	spin_lock_bh(&listen_uids.lock);
	for (i = 0; i < num; ++i) {
		uid = *(int *)p;
		if (uid < 0)
			break;
		switch (type) {
		case ADD_FG_UID:
			ret = add_listening_uid(uid);
			break;
		case DEL_FG_UID:
			ret = del_listening_uid(uid);
			break;
		case BYPASS_FG_UID:
			ret = upate_bypass_flag(uid, true);
			break;
		case NOPASS_FG_UID:
			ret = upate_bypass_flag(uid, false);
			break;
		default:
			ret = false;
			break;
		}
		if (!ret)
			break;
		p += sizeof(int);
	}
	spin_unlock_bh(&listen_uids.lock);
}

static void do_commands(struct req_msg_head *msg, u32 len)
{
	int num, size;
	char *p = NULL;

	if (!msg || msg->len <= sizeof(struct req_msg_head)) {
		pr_err("hw_packet_filter_bypass msg length too small msg len error!!!\n");
		return;
	}

	size = len - sizeof(struct req_msg_head);
	p = (char *)msg + sizeof(struct req_msg_head);
	if (size <= sizeof(int)) {
		pr_err("hw_packet_filter_bypass msg size too small msg len error!!! %d\n", size);
		return;
	}
	num = *(int *)p;
	size -= sizeof(int);
	p += sizeof(int);
	if (size < (sizeof(int) * num)) {
		pr_err("hw_packet_filter_bypass real length too small msg len error!!!\n");
		return;
	}

	if (msg->type == SOCKET_CLOSE_DETECT_UID_ADD || msg->type == SOCKET_CLOSE_DETECT_UID_DEL) {
		socket_close_detect_uids_change(msg->type, num, p);
		return;
	}

	update_listen_uids(msg->type, num, p);
}

static void notify_drop_event(int uid, struct packet_drop_stats *stats)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	// type
	assign_short(p, PACKET_FILTER_DROP_RPT);
	skip_byte(p, sizeof(s16));
	// 16 eq len(2B type + 2B len + 4B uid + 4B hook + 4B pkts)
	assign_short(p, 20);
	skip_byte(p, sizeof(s16));
	// uid
	assign_int(p, uid);
	skip_byte(p, sizeof(int));
	// hook
	assign_int(p, stats->hook);
	skip_byte(p, sizeof(int));
	// pkts
	assign_int(p, stats->pkts);
	skip_byte(p, sizeof(int));
	// dns is drop counts
	assign_int(p, stats->dns_drop_count);
	skip_byte(p, sizeof(int));

	notifier((struct res_msg_head *)event);
}

static void notify_recovery_event(int uid, struct packet_drop_stats *stats)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	// type
	assign_short(p, PACKET_FILTER_RECOVERY_RPT);
	skip_byte(p, sizeof(s16));
	// 12 eq len((2B type + 2B len + 4B uid + 4B hook))
	assign_short(p, 12);
	skip_byte(p, sizeof(s16));
	// uid
	assign_int(p, uid);
	skip_byte(p, sizeof(int));
	// hook
	assign_int(p, stats->hook);
	notifier((struct res_msg_head *)event);
}

static void update_packet_count(int uid, struct packet_drop_stats *stats, bool is_dns_drop)
{
	unsigned long now = jiffies;
	int threshold;

	if (stats->hook == HOOK_IP)
		threshold = IP_REPORT_THRESHOLD_PKTS;
	else
		threshold = REPORT_THRESHOLD_PKTS;
	/*
	 * If the dropped packets num exceeds the REPORT_THRESHOLD
	 *  within DETECTION_PERIOD seconds, think it as an exception
	 */
	stats->pkts = stats->pkts + 1;
	if (is_dns_drop)
		stats->dns_drop_count = stats->dns_drop_count + 1;
	if ((stats->pkts >= threshold) && (!stats->reported_drop)) {
		notify_drop_event(uid, stats);
		stats->reported_drop = true;
		stats->reported_recovery = false;
		stats->pkts = 0;
		stats->dns_drop_count = 0;
	}

	if (!stats->ts_reset)
		stats->ts_reset = now;
	if (time_after(now, stats->ts_reset + DETECT_PEIROID)) {
		stats->ts_reset = now + DETECT_PEIROID;
		if (stats->pkts >= threshold && stats->reported_drop)
			notify_drop_event(uid, stats);
		stats->pkts = 0;
		stats->dns_drop_count = 0;
	}
}

static int update_filter_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass, bool is_dns_drop)
{
	int ret = 0;

	if (pass == PASS) {
		stats->pkts = 0;
		stats->dns_drop_count = 0;
		/*
	 	 * cond1: already notify the drop event to user
	 	 * cond2: not notify the recovery event
	 	 */
		if (stats->reported_drop && !stats->reported_recovery) {
			notify_recovery_event(info->uid, stats);
			stats->reported_recovery = true;
			stats->reported_drop = false;
			stats->ts_reset = 0;
		}
	} else {
		update_packet_count(info->uid, stats, is_dns_drop);
		ret = info->bypass;
	}
	return ret;
}

static int update_ip_in_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass, bool is_dns_drop)
{
	if (pass != PASS)
		return 0;

	update_packet_count(info->uid, stats, is_dns_drop);
	return 0;
}

static int update_ip_out_pkts(struct uid_info *info,
	struct packet_drop_stats *stats, int pass)
{
	if (pass != PASS)
		return 0;

	stats->pkts = 0;
	stats->dns_drop_count = 0;
	/*
	 * cond1: already notify the drop event to user
	 * cond2: not notify the recovery event
	 * cond3: pass is not lead by bypass
	 */
	if (stats->reported_drop && (!stats->reported_recovery) && (!info->bypass)) {
		notify_recovery_event(info->uid, stats);
		stats->reported_recovery = true;
		stats->reported_drop = false;
		stats->ts_reset = 0;
	}
	return 0;
}

int hw_translate_hook_num(int af, int hook)
{
	int ret;

	switch (hook) {
	case NF_INET_PRE_ROUTING:
		ret = HW_PFB_INET_PRE_ROUTING;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_PRE_ROUTING;
		break;
	case NF_INET_LOCAL_IN:
		ret = HW_PFB_INET_LOCAL_IN;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_LOCAL_IN;
		break;
	case NF_INET_FORWARD:
		ret = HW_PFB_INET_FORWARD;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_FORWARD;
		break;
	case NF_INET_LOCAL_OUT:
		ret = HW_PFB_INET_LOCAL_OUT;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_LOCAL_OUT;
		break;
	case NF_INET_POST_ROUTING:
		ret = HW_PFB_INET_POST_ROUTING;
		if (af == AF_INET6)
			ret = HW_PFB_INET6_POST_ROUTING;
		break;
	default:
		ret = HW_PFB_HOOK_INVALID;
		break;
	}
	return ret;
}

int hw_translate_verdict(u32 verdict)
{
	if (verdict == NF_ACCEPT || verdict == NF_STOP)
		return PASS;
	else if ((verdict & NF_VERDICT_MASK) == NF_DROP)
		return DROP;
	else
		return IGNORE;
}

bool hw_hook_bypass_skb(int af, int hook, struct sk_buff *skb)
{
	/* 0 means do not influence the orignal procedure */
	struct net_device *in = NULL;
	struct net_device *out = NULL;
	struct dst_entry *dst = NULL;

	if (notifier == NULL)
		return 0;
	if (af != AF_INET && af != AF_INET6)
		return 0;
	if (!skb)
		return 0;

	switch (hook) {
	case NF_INET_LOCAL_OUT:
		dst = skb_dst(skb);
		if (likely(dst))
			out = dst->dev;
		break;
	case NF_INET_LOCAL_IN:
		out = skb->dev;
		break;
	default:
		break;
	}
	if (!in && !out)
		return 0;
	return hw_bypass_skb(af, HW_PFB_HOOK_ICMP, NULL, skb, in, out, PASS);
}

static int hw_pfb_hooks_info_judge(int i, int hook, int pass, bool is_dns_drop)
{
	int ret = 0;
	if (i < listen_uids.nums) {
		switch (hook) {
		case HW_PFB_INET_LOCAL_OUT:
		case HW_PFB_INET6_LOCAL_OUT:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].local_out_drop_stats, pass, is_dns_drop);
			break;
		case HW_PFB_INET_BPF_EGRESS:
		case HW_PFB_INET6_BPF_EGRESS:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].bpf_egress_drop_stats, pass, is_dns_drop);
			break;
		case HW_PFB_INET_IP_XMIT:
		case HW_PFB_INET6_IP_XMIT:
			ret = update_ip_in_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].ip_in_stats, pass, is_dns_drop);
			break;
		case HW_PFB_INET_DEV_XMIT:
		case HW_PFB_INET6_DEV_XMIT:
			ret = update_ip_out_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].ip_in_stats, pass);
			break;
		case HW_PFB_INET_LOCAL_IN:
		case HW_PFB_INET6_LOCAL_IN:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].local_in_drop_stats, pass, is_dns_drop);
			break;
		case HW_PFB_INET_BPF_INGRESS:
		case HW_PFB_INET6_BPF_INGRESS:
			ret = update_filter_pkts(&listen_uids.infos[i],
				&listen_uids.infos[i].bpf_ingress_drop_stats, pass, is_dns_drop);
			break;
		case HW_PFB_HOOK_ICMP:
			ret = listen_uids.infos[i].bypass;
			break;
		default:
			break;
		}
	}

	return ret;
}

static bool is_dns_dport(int protocol, int dport)
{
	if (protocol == IPPROTO_UDP)
		return dport == htons(UDP_DNS_PORT);

	if (protocol == IPPROTO_TCP)
		return dport == htons(TCP_DNS_PORT);

	return false;
}

bool is_dns_data_drop(int pass, int hook, int protocol, int dport)
{
	if (pass == PASS)
		return false;
	if (hook != HW_PFB_INET_LOCAL_OUT && hook != HW_PFB_INET6_LOCAL_OUT)
		return false;
	return is_dns_dport(protocol, dport);
}

/* if we should bypass this skb, return 1, else return 0 */
bool hw_bypass_skb(int af, int hook, const struct sock *sk,
	struct sk_buff *skb, struct net_device *in,
	struct net_device *out, int pass)
{
	uid_t uid;
	int i;
	int ret = 0;
	bool is_dns_drop = false;

	if (notifier == NULL)
		return ret;
	if (!skb)
		return ret;
	if (af != AF_INET && af != AF_INET6)
		return ret;
	if (pass == IGNORE)
		return ret;
	if (sk)
		sk = sk_to_full_sk((struct sock *)sk);
	else
		sk = sk_to_full_sk(skb->sk);
	uid = hw_get_sock_uid((struct sock *)sk);
	if (uid == overflowuid)
		return ret;
	if (!is_ds_rnic(in) && !is_ds_rnic(out))
		return ret;

	spin_lock_bh(&listen_uids.lock);
	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid)
			break;
	}

	if (is_dns_data_drop(pass, hook, sk->sk_protocol, sk->sk_dport))
		is_dns_drop = true;

	ret = hw_pfb_hooks_info_judge(i, hook, pass, is_dns_drop);
	spin_unlock_bh(&listen_uids.lock);

	return ret;
}

static void reset_socket_close_info(void)
{
	spin_lock_bh(&g_socket_close_lock);
	(void)memset_s(&g_socket_close_uids_info, sizeof(struct chr_socket_close_info), 0, sizeof(struct chr_socket_close_info));
	g_socket_close_index = SOCKET_CLOSE_INFO_INIT_INDEX;
	spin_unlock_bh(&g_socket_close_lock);
}

static void process_report_socket_close_info(int size, char *p)
{
	int i;

	if (size > MAX_CHR_SOCKET_CLOSE_UID_NUM || size < 0)
		return;

	assign_short(p, 4 + size * (SOCKET_CLOSE_INFO_SIZE));
	skip_byte(p, sizeof(s16));

	for (i = 0; i < size ; i++) {
		if (g_socket_close_uids_info[i].uid == 0)
			continue;

		assign_int(p, g_socket_close_uids_info[i].uid);
		skip_byte(p, sizeof(int));

		assign_int(p,  g_socket_close_uids_info[i].is_tcp);
		skip_byte(p, sizeof(int));

		assign_int(p,  g_socket_close_uids_info[i].net_id);
		skip_byte(p, sizeof(int));

		assign_int(p,  g_socket_close_uids_info[i].is_foreground);
		skip_byte(p, sizeof(int));

		(void)strncpy_s(p, IFNAMSIZ, g_socket_close_uids_info[i].ifname, IFNAMSIZ - 1);
		skip_byte(p, IFNAMSIZ);
	}
}

static void notify_socket_close_event(int size)
{
	char event[NOTIFY_BUF_LEN] = {0};
	char *p = event;

	if (!notifier)
		return;

	assign_short(p, SOCKET_CLOSE_CHR_MSG_ID);
	skip_byte(p, sizeof(s16));
	process_report_socket_close_info(size, p);
	notifier((struct res_msg_head *)event);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static void chr_socket_close_data_process_timer(struct timer_list* data)
#else
static void chr_socket_close_data_process_timer(unsigned long data)
#endif
{
	if (g_socket_close_index < 0)
		return;
	notify_socket_close_event(g_socket_close_index + 1);
	reset_socket_close_info();
}

static int get_update_uid_info_index(struct sock *sk)
{
	int i;
	int uid = sock_i_uid(sk).val;

	for (i = 0; i <= g_socket_close_index; i++) {
		if (g_socket_close_uids_info[i].uid == uid) {
			if (g_socket_close_uids_info[i].is_tcp == 1 && sk->sk_protocol == IPPROTO_UDP)
				return i;
			return INGONER_SOCKET_INDEX;
		}
	}
	return i >= MAX_CHR_SOCKET_CLOSE_UID_NUM ? INGONER_SOCKET_INDEX : i;
}

static void init_socket_close_timer(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	timer_setup(&g_socket_close_timer, chr_socket_close_data_process_timer, 0);
#else
	init_timer(&g_socket_close_timer);
	g_socket_close_timer.data = 0;
	g_socket_close_timer.function = chr_socket_close_data_process_timer;
#endif
	g_socket_close_timer.expires = jiffies + PROP_SOCKET_CLOSED_TIME;
	add_timer(&g_socket_close_timer);
}

static bool is_foreground_uid(int uid)
{
	int i;

	spin_lock_bh(&listen_uids.lock);
	for (i = 0; i < listen_uids.nums; ++i) {
		if (listen_uids.infos[i].uid == uid)
			break;
	}
	spin_unlock_bh(&listen_uids.lock);
	return i < listen_uids.nums;
}

static bool is_netd_process()
{
	char proc_name[PROCESS_NAME_LEN + 1] = {0};
	struct task_struct *task = NULL;
	struct pid_namespace *ns = NULL;
	int ret;

	if (current != NULL && current->group_leader != NULL) {
		rcu_read_lock();
		 ns = task_active_pid_ns(current);
		 if (ns == NULL) {
			rcu_read_unlock();
			return false;
		 }
		task = find_task_by_pid_ns(current->group_leader->pid, ns);
		if (task == NULL) {
			rcu_read_unlock();
			return false;
		}
		get_task_struct(task);
		rcu_read_unlock();
		if (task->active_mm != NULL && task->active_mm->exe_file != NULL && task->active_mm->exe_file->f_path.dentry != NULL) {
			ret = memcpy_s(proc_name, PROCESS_NAME_LEN, task->active_mm->exe_file->f_path.dentry->d_iname, PROCESS_NAME_LEN);
			if (ret != EOK) {
				put_task_struct(task);
				return false;
			}
		}
		put_task_struct(task);
		return strcmp(proc_name, NETD) == 0;
	}
	return false;
}

static void update_socket_close_uids_info(int index, struct sock *sk, int uid)
{
	char ifname[IFNAMSIZ] = {0};

	if (index >= MAX_CHR_SOCKET_CLOSE_UID_NUM || index < 0)
		return;

	spin_lock_bh(&g_socket_close_lock);
	g_socket_close_uids_info[index].uid = uid;
	g_socket_close_uids_info[index].is_foreground = is_foreground_uid(uid) ? 1 : 0;
	g_socket_close_uids_info[index].is_tcp = sk->sk_protocol == IPPROTO_TCP ? 1 : 0;
	if (sk->sk_bound_dev_if > 0 && netdev_get_name(sock_net(sk), ifname, sk->sk_bound_dev_if) == 0)
		(void)strncpy_s(g_socket_close_uids_info[index].ifname, IFNAMSIZ, ifname, IFNAMSIZ - 1);
	g_socket_close_uids_info[index].net_id = sk->sk_mark & NET_ID_MASK;
	if (index > g_socket_close_index)
		g_socket_close_index = index;
	spin_unlock_bh(&g_socket_close_lock);
}

static bool is_need_detect_uid(int uid)
{
	int i = 0;

	spin_lock_bh(&g_socket_close_detect_uids.lock);
	for (i = 0; i < MAX_SOCKET_CLOSE_DETECT_NUM; i++) {
		if (g_socket_close_detect_uids.uids[i] == uid) {
			spin_unlock_bh(&g_socket_close_detect_uids.lock);
			return true;
		}
	}
	spin_unlock_bh(&g_socket_close_detect_uids.lock);
	return false;
}

void socket_close_chr(struct sock *sk)
{
	int index;
	int uid;

	if (sk == NULL || (sk->sk_protocol != IPPROTO_TCP && sk->sk_protocol != IPPROTO_UDP))
		return;

	uid = sk->sk_uid.val;
	if (uid < SYSTEM_UID_MAX || current_uid().val >= SYSTEM_UID_MAX || !is_need_detect_uid(uid))
		return;

	if (is_dns_dport(sk->sk_protocol, sk->sk_dport))
		return;

	if (is_netd_process())
		return;

	if (!g_is_initial_socket_close_timer) {
		init_socket_close_timer();
		g_is_initial_socket_close_timer = true;
	}

	if (timer_pending(&g_socket_close_timer) == 0) {
		g_socket_close_timer.expires = jiffies + PROP_SOCKET_CLOSED_TIME;
		add_timer(&g_socket_close_timer);
	}

	index = get_update_uid_info_index(sk);
	update_socket_close_uids_info(index, sk, uid);
}

msg_process* __init hw_packet_filter_bypass_init(notify_event *notify)
{
	spin_lock_init(&g_socket_close_detect_uids.lock);
	spin_lock_init(&g_socket_close_lock);
	reset_socket_close_info();
	reset_detect_uid();
	if (notify == NULL) {
		pr_err("%s: notify parameter is null\n", __func__);
		return NULL;
	}
	spin_lock_init(&listen_uids.lock);
	notifier = notify;
	return do_commands;
}
