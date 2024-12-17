/*
 * network_qos.c
 *
 * Network Qos schedule implementation
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "netqos_sched/netqos_sched.h"

#include <linux/sched.h>
#include <linux/tcp.h>
#include <net/inet_sock.h>
#include <net/sock.h>
#include <securec.h>

#ifdef CONFIG_HW_QOS_THREAD
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#define CREATE_TRACE_POINTS
#include <trace/events/netqos.h>

int sysctl_netqos_switch __read_mostly;
int sysctl_netqos_debug __read_mostly;
int sysctl_netqos_limit __read_mostly;
int sysctl_netqos_period __read_mostly = 200;
static unsigned long g_high_prio_flux_time;

enum {
	NETQOS_PRIO_LOW,
	NETQOS_PRIO_MID,
	NETQOS_PRIO_HIGH,
};

#define TIME_MS_PER_JIFFY (1000 / HZ)

#define MAX_STAT_TIME_JIFFIES (sysctl_netqos_period / TIME_MS_PER_JIFFY)
#define RCVWND_LEVEL_TIME_JIFFIES \
	(sysctl_netqos_period / 4 / TIME_MS_PER_JIFFY)

#define MAX_UPDATE_PRIO_JIFFIES (500 / TIME_MS_PER_JIFFY)
#define NETQOS_TRACE_INTERVAL (32 / TIME_MS_PER_JIFFY)

#define HIGH_PRIO_WIFI_MARK 0x5a
#define MIN_RCVWND_SIZE (4 * 1460)

#define MAX_LIMIT_UID 20
#define LIMIT_GROUP_BUF_MAX 300
#define HARD_LIMIT_MAX (10 * 1000 * 1000)
#define HARD_LIMIT_TIME_DIFF 100

struct limit_group_info {
	int num;
	int uids[MAX_LIMIT_UID];
	int times[MAX_LIMIT_UID];
};

static struct limit_group_info limit_group_infos = {
	.num = 0,
	.uids = {0, },
	.times = {0, },
};

static void build_limit_group_str(char *buf, int buf_len)
{
	int count;
	int i;
	int len;
	char *begin = buf;
	for (i = 0; i < limit_group_infos.num; ++i) {
		len = buf_len - (begin - buf);
		if (len <= 0)
			break;
		if (i == 0)
			count = snprintf_s(begin, len, len - 1, "%d,%d",
						limit_group_infos.uids[i],
						limit_group_infos.times[i]);
		else if (limit_group_infos.uids[i] != 0)
			count = snprintf_s(begin, len, len - 1, ";%d,%d",
						limit_group_infos.uids[i],
						limit_group_infos.times[i]);
		else
			continue;
		if (count < 0) {
			net_qos_debug("netqos %s failed, buf may not enough\n", __func__);
			break;
		}
		begin += count;
	}
}

static void parse_input_group_info(char *input_str)
{
	int uids[MAX_LIMIT_UID];
	int times[MAX_LIMIT_UID];
	int count = 0;
	int num;
	int i;
	char *pos;
	char *num_s;
	char *temp = input_str;
	(void)memset_s(uids, sizeof(uids), 0, sizeof(uids));
	(void)memset_s(times, sizeof(times), 0, sizeof(times));
	pos = strsep(&temp, ";");
	for (i = 0; i < MAX_LIMIT_UID; i++) {
		if (pos == NULL)
			break;
		num_s = strsep(&pos, ",");
		if (num_s != NULL)
			num = simple_strtol(num_s, NULL, 0);
		else
			num = 0;
		uids[i] = num;

		num_s = strsep(&pos, ",");
		if (num_s != NULL)
			num = simple_strtol(num_s, NULL, 0);
		else
			num = 0;
		times[i] = num;
		count++;
		pos = strsep(&temp, ";");
	}
	(void)memcpy_s(&limit_group_infos.uids, sizeof(uids), &uids, sizeof(uids));
	(void)memcpy_s(&limit_group_infos.times, sizeof(times), &times, sizeof(times));
	limit_group_infos.num = count;
}

int limit_group_handler(struct ctl_table *table, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	struct ctl_table tbl = {
		.maxlen = LIMIT_GROUP_BUF_MAX,
	};
	tbl.data = kmalloc(tbl.maxlen, GFP_USER);
	if (!tbl.data)
		return -ENOMEM;
	(void)memset_s(tbl.data, LIMIT_GROUP_BUF_MAX, 0, LIMIT_GROUP_BUF_MAX);
	if (!write) {
		build_limit_group_str((char *)tbl.data, LIMIT_GROUP_BUF_MAX);
	}
	ret = proc_dostring(&tbl, write, buffer, lenp, ppos);
	if (!write || ret) {
		kfree(tbl.data);
		return ret;
	}
	parse_input_group_info((char *)tbl.data);
	kfree(tbl.data);
	return ret;
}

int get_net_qos_level(struct task_struct *cur_task)
{
	int level = NETQOS_PRIO_MID;
#ifdef CONFIG_HW_QOS_THREAD
	int task_qos_level = get_task_qos(cur_task);

	switch (task_qos_level) {
	case VALUE_QOS_LOW:
		level = NETQOS_PRIO_LOW;
		break;
	case VALUE_QOS_NORMAL:
		level = NETQOS_PRIO_MID;
		break;
	case VALUE_QOS_HIGH:
	case VALUE_QOS_CRITICAL:
		level = NETQOS_PRIO_HIGH;
		break;
	default:
		net_qos_debug("netqos unknown task qos value %d uid %d\n",
			     task_qos_level, current_uid().val);
		break;
	}
#endif

	return level;
}

static int get_priority(struct sock *sk, struct task_struct *cur_task,
	unsigned long cur_time)
{
	if (unlikely(!sk))
		return NETQOS_PRIO_MID;

	if ((sk->sk_netqos_level != -1) &&
		(cur_time - sk->sk_netqos_time < MAX_UPDATE_PRIO_JIFFIES))
		return sk->sk_netqos_level;

	if (unlikely(!cur_task))
		return NETQOS_PRIO_MID;

	sk->sk_netqos_level = get_net_qos_level(cur_task);
	sk->sk_netqos_time = cur_time;
	net_qos_debug("netqos %s sk_netqos_level %d uid %d\n",
		__func__, sk->sk_netqos_level, current_uid().val);
	return sk->sk_netqos_level;
}

static inline void mark_high_prio_fluxing(unsigned long cur_time)
{
	g_high_prio_flux_time = cur_time;
}

static inline bool is_high_prio_fluxing(unsigned long cur_time)
{
	return g_high_prio_flux_time + MAX_STAT_TIME_JIFFIES > cur_time;
}

static void limit_speed(unsigned long cur_time)
{
	int sleeptime;

	if (!sysctl_netqos_switch || !sysctl_netqos_limit)
		return;

	sleeptime = (MAX_STAT_TIME_JIFFIES -
		(cur_time - g_high_prio_flux_time)) * TIME_MS_PER_JIFFY;
	if ((sleeptime > 0) && (sleeptime <= sysctl_netqos_period)) {
		net_qos_debug("netqos %s begin sleep %d for uid %d\n",
			__func__, sleeptime, current_uid().val);
		msleep_interruptible(sleeptime);
		net_qos_debug("netqos %s end sleep %d for uid %d\n",
			__func__, sleeptime, current_uid().val);
	}
}

static void limit_speed_hard(int hard_limit_time)
{
	if (!sysctl_netqos_switch)
		return;
	net_qos_debug("netqos %s hard_limit_time %d\n",
			__func__, hard_limit_time);
	if (hard_limit_time > 0 && hard_limit_time < HARD_LIMIT_MAX) {
		net_qos_debug("netqos %s begin sleep %d for uid %d pid %d\n",
			__func__, hard_limit_time, current_uid().val, current->tgid);
		usleep_range(hard_limit_time, hard_limit_time + HARD_LIMIT_TIME_DIFF);
		net_qos_debug("netqos %s end sleep %d for uid %d pid %d\n",
			__func__, hard_limit_time, current_uid().val, current->tgid);
	}
}

static int get_hard_limit_time()
{
	int i;
	int cur_uid;
	if (limit_group_infos.num <= 0)
		return 0;
	cur_uid = current_uid().val;
	for (i = 0; i < MAX_LIMIT_UID && i < limit_group_infos.num; i++) {
		if (limit_group_infos.uids[i] == cur_uid)
			return limit_group_infos.times[i];
	}
	return 0;
}

void netqos_sendrcv(struct sock *sk, int len, bool is_recv)
{
	int level;
	unsigned long cur_time;
	int hard_limit_time;

	if (!sysctl_netqos_switch)
		return;

	cur_time = jiffies;
	trace_netqos_trx(sk, len, is_recv, cur_time);
	level = get_priority(sk, current, cur_time);
	hard_limit_time = get_hard_limit_time();
	if (level == NETQOS_PRIO_HIGH)
		mark_high_prio_fluxing(cur_time);
	else if ((level == NETQOS_PRIO_LOW) && (hard_limit_time > 0))
		limit_speed_hard(hard_limit_time);
	else if ((level == NETQOS_PRIO_LOW) && is_high_prio_fluxing(cur_time))
		limit_speed(cur_time);
	else
		return;
}

int tcp_is_low_priority(struct sock *sk)
{
	return (get_priority(sk, NULL, jiffies) == NETQOS_PRIO_LOW);
}

static int check_netqos_rcvwnd_params(const struct tcp_sock *tp,
	const uint32_t *size)
{
	return (!sysctl_netqos_switch || !sysctl_netqos_limit || !tp || !size);
}

void netqos_rcvwnd(struct sock *sk, uint32_t *size)
{
	struct tcp_sock *tp = tcp_sk(sk);
	unsigned long cur_time;
	unsigned long time_aligned;
	u32 rcv_wnd_modify;

	if (check_netqos_rcvwnd_params(tp, size))
		return;

	if ((tp->rcv_rate.rcv_wnd == 0) || (tp->rcv_rate.rcv_wnd == ~0U) ||
		(*size <= MIN_RCVWND_SIZE))
		return;

	cur_time = jiffies;
	if ((get_priority(sk, NULL, cur_time) == NETQOS_PRIO_LOW) &&
		is_high_prio_fluxing(cur_time) &&
		(MAX_STAT_TIME_JIFFIES != 0)) {
		time_aligned = ((cur_time - g_high_prio_flux_time) /
			RCVWND_LEVEL_TIME_JIFFIES + 1) *
			RCVWND_LEVEL_TIME_JIFFIES;

		if (time_aligned > MAX_STAT_TIME_JIFFIES)
			time_aligned = MAX_STAT_TIME_JIFFIES;

		rcv_wnd_modify = (u32)(time_aligned * tp->rcv_rate.rcv_wnd /
			MAX_STAT_TIME_JIFFIES);
		if (rcv_wnd_modify < MIN_RCVWND_SIZE)
			rcv_wnd_modify = MIN_RCVWND_SIZE;

		net_qos_debug("netqos origin %d, rcv_wnd %u modify %u\n",
			*size, tp->rcv_rate.rcv_wnd, rcv_wnd_modify);
		*size = min_t(uint32_t, rcv_wnd_modify, *size);
	}
}

int netqos_qdisc_band(const struct sk_buff *skb, int orig_band)
{
	int level;

	if (!sysctl_netqos_switch || !skb)
		return orig_band;

	level = get_priority(skb->sk, NULL, jiffies);
	if (level == NETQOS_PRIO_HIGH) {
		net_qos_debug("netqos qdisc band update\n");
		return 0;
	}

	return orig_band;
}

bool netqos_trace_add_check(struct sock *sk, int len, bool is_recv,
	unsigned long cur_time)
{
	unsigned int *trx = NULL;

	if (unlikely(!sk || (len <= 0)))
		return false;

	trx = is_recv ? &sk->sk_netqos_rx : &sk->sk_netqos_tx;
	*trx += len;
	return cur_time - sk->sk_netqos_ttime >= NETQOS_TRACE_INTERVAL;
}

void netqos_trace_assign_set(struct sock *sk, int *tx, int *rx,
	unsigned long cur_time)
{
	if (unlikely(!sk || !tx || !rx))
		return;

	*tx = sk->sk_netqos_tx;
	*rx = sk->sk_netqos_rx;
	sk->sk_netqos_tx = 0;
	sk->sk_netqos_rx = 0;
	sk->sk_netqos_ttime = cur_time;
}
