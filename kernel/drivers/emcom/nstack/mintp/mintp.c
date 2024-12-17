/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: liyouyong songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#include "mintp_conn.h"
#include "mintp_input.h"
#include "mintp_output.h"
#include "mintp_congestion.h"
#include "mintp_timer.h"
#include "mintp_tls.h"

long sysctl_mtp_mem[3] __read_mostly;
int sysctl_mtp_rmem_min __read_mostly;
int sysctl_mtp_wmem_min __read_mostly;
atomic_long_t mtp_memory_allocated;

int g_mtp_loss = 0;
module_param(g_mtp_loss, int, S_IRUGO);

int sysctl_mtp_debug __read_mostly = LOGLEVEL_INFO;
int sysctl_mtp_queue __read_mostly = -1;
int sysctl_mtp_ack_timer __read_mostly = 4;
int sysctl_mtp_disable __read_mostly = 0;
int mtp_limit_output_bytes __read_mostly = 1000000;
#ifdef CONFIG_PROC_FS
static bool g_mtp_proc_init = false;
#endif /* CONFIG_PROC_FS */
static struct ctl_table_header *g_mtp_ctl_table = NULL;
static int mtp_sendmsg_segment(struct sock *sk, struct msghdr *msg, long timeo,
			       int *seg, bool *process_backlog);
static int mtp_recv_queue(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb);

bool mtp_mac_any(unsigned char *mac, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (mac[i])
			return false;
	}
	return true;
}

static struct sock *mtp_reqsk_remove(struct mtp_sock *msk)
{
	struct mtp_sock *req;

	spin_lock_bh(&msk->accept_queue.lock);
	req = list_first_entry_or_null(&msk->accept_queue.accept,
		struct mtp_sock, req_node);
	if (req) {
		sk_acceptq_removed((struct sock *)msk);
		list_del(&req->req_node);
		mtp_debug("server port %u sk_ack_backlog %d\n", msk->src_port,
			((struct sock *)msk)->sk_ack_backlog);
	}
	spin_unlock_bh(&msk->accept_queue.lock);
	return (struct sock *)req;
}

void mtp_set_state(struct sock *sk, int state)
{
	int oldstate = sk->sk_state;
	mtp_info("%u:%u sk_state from %u to %u\n", mtp_sk(sk)->src_port,
		mtp_sk(sk)->dst_port, oldstate, state);
	switch (state) {
	case TCP_ESTABLISHED:
		if (oldstate != TCP_ESTABLISHED)
			MTP_INC_STATS(MTP_MIB_CURRESTAB);
		break;
	case TCP_CLOSE:
		if (oldstate == TCP_CLOSE_WAIT || oldstate == TCP_ESTABLISHED)
			MTP_INC_STATS(MTP_MIB_ESTABRESETS);

		sk->sk_prot->unhash(sk);
		mtp_put_port(sk);
		fallthrough;
	default:
		if (oldstate == TCP_ESTABLISHED)
			MTP_DEC_STATS(MTP_MIB_CURRESTAB);
	}
	/* Change state AFTER socket is unhashed to avoid closed
	 * socket sitting in hash tables.
	 */
	mtp_state_store(sk, state);
}

void mtp_done(struct sock *sk)
{
	if (sk->sk_state == TCP_SYN_SENT)
		MTP_INC_STATS(MTP_MIB_ATTEMPTFAILS);
	sk->sk_prot->unhash(sk);
	mtp_put_port(sk);
	mtp_set_state(sk, TCP_CLOSE);
	mtp_clear_xmit_timers(sk);
	sk->sk_shutdown = SHUTDOWN_MASK;

	if (!sock_flag(sk, SOCK_DEAD)) {
		mtp_info("%u:%u sock is not dead\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
		sk->sk_state_change(sk);
	} else {
		mtp_info("%u:%u release the sk\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
		sk->sk_prot->destroy(sk);
		sk_stream_kill_queues(sk);
		sock_put(sk);
	}
}

static void mtp_listen_close(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct sock *req;

	mtp_set_state(sk, TCP_CLOSE);
	while ((req = mtp_reqsk_remove(msk)) != NULL) {
		local_bh_disable();
		bh_lock_sock(req);
		WARN_ON(sock_owned_by_user(req));
		sock_hold(req);

		req->sk_prot->disconnect(req, O_NONBLOCK);
		sock_orphan(req);
		req->sk_prot->destroy(req);
		sock_put(req);

		bh_unlock_sock(req);
		local_bh_enable();
		sock_put(req);

		cond_resched();
	}
	WARN_ON_ONCE(sk->sk_ack_backlog);
	mtp_info("server %u closed\n", mtp_sk(sk)->src_port);
}

static void mtp_prepare_close(struct sock *sk, u8 state)
{
	/* Now socket is owned by kernel and we acquire BH lock
	 *  to finish close. No need to check for user refs.
	 */
	local_bh_disable();
	bh_lock_sock(sk);

	WARN_ON(sock_owned_by_user(sk));

	/* Have we already been destroyed by a softirq or backlog? */
	if (state != TCP_CLOSE && sk->sk_state == TCP_CLOSE)
		goto out;

	if (sk->sk_state == TCP_CLOSE) {
		WARN_ON(!sk_unhashed(sk));
		sk->sk_prot->destroy(sk);
		sk_stream_kill_queues(sk);
		sock_put(sk);
	}
out:
	bh_unlock_sock(sk);
	local_bh_enable();
}

static void mtp_close(struct sock *sk, long timeout)
{
	u8 state;
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *tskb;

	mtp_info("%u:%u close timeout %ld\n",
		mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, timeout);
	lock_sock(sk);

	if (sk->sk_state == TCP_LISTEN) {
		mtp_listen_close(sk);
		goto adjudge_to_death;
	}

	if (msk->tls_ctx)
		mtp_tls_close(sk);

	sk->sk_shutdown = SHUTDOWN_MASK;
	/* If socket has been already reset (e.g. in mtp_reset()) - kill it. */
	if (sk->sk_state == TCP_CLOSE)
		goto adjudge_to_death;

	if (sock_flag(sk, SOCK_LINGER) && !sk->sk_lingertime)
		goto disconnect;

	tskb = mtp_write_queue_tail(sk);
	if (tskb && (MTP_SKB_CB(tskb)->type != MTPHDR_TYPE_MSG_END) &&
		(MTP_SKB_CB(tskb)->type != MTPHDR_TYPE_FIN)) {
disconnect:
		sk->sk_prot->disconnect(sk, 0);
		goto adjudge_to_death;
	}

	if (sk->sk_state == TCP_ESTABLISHED) {
		mtp_set_state(sk, TCP_FIN_WAIT1);
		mtp_send_fin(sk, sk->sk_allocation);
	} else if (sk->sk_state == TCP_SYN_SENT) {
		mtp_done(sk);
	}
	sk_stream_wait_close(sk, timeout);

adjudge_to_death:
	state = sk->sk_state;
	sock_hold(sk);
	sock_orphan(sk);
	release_sock(sk);
	mtp_prepare_close(sk, state);
	sock_put(sk);
}

static int mtp_connect_addr_check(struct sockaddr_mtp *addr, int addr_len)
{
	if (!addr || addr_len != sizeof(struct sockaddr_mtp) || addr->sa_family != AF_INET)
		return -EINVAL;

	if (!addr->port || mtp_mac_any(addr->mac.addr, ETH_ALEN)) {
		mtp_err("connect invalid address: mac "MACFMT" port:%u\n", MACDATA(addr->mac.addr), addr->port);
		return -EINVAL;
	}

	if (addr->type != MTP_ADDR_TYPE_MAC && addr->type != MTP_ADDR_TYPE_IPV4) {
		mtp_err("connect invalid addr type:%u\n", addr->type);
		return -EINVAL;
	}
	return 0;
}

static int mtp_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_mtp *addr = (struct sockaddr_mtp *)uaddr;
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct inet_sock *inet = (struct inet_sock *)sk;
	int err;

	err = mtp_connect_addr_check(addr, addr_len);
	if (err)
		return err;

	if (!msk->src_port) {
		mtp_err("no bind\n");
		return -EADDRNOTAVAIL;
	}

	if (addr->type == MTP_ADDR_TYPE_MAC) {
		(void)memcpy_s(msk->dst_mac, ETH_ALEN, addr->mac.addr, ETH_ALEN);
		if (memcmp(msk->dst_mac, msk->src_mac, ETH_ALEN) == 0)
			goto loopback;
	} else { /*  must be MTP_ADDR_TYPE_IPV4 */
		if (dev_get_mac_by_inetaddr(addr->ip.addr, msk->dst_mac)) {
			inet->inet_dport = htons((u16)addr->port);
			inet->inet_daddr = addr->ip.addr;
		} else if (memcmp(msk->dst_mac, msk->src_mac, ETH_ALEN) == 0) {
loopback:
			mtp_info("mac "MACFMT"is_loopback\n", MACDATA(msk->dst_mac));
			msk->is_loopback = true;
		}
	}
	msk->dst_port = addr->port;
	mtp_info("try to connect to mac "MACFMT" port:%u\n", MACDATA(addr->mac.addr), msk->dst_port);

	mtp_set_state(sk, TCP_SYN_SENT);
	err = sk->sk_prot->hash(sk);
	if (unlikely(err)) {
		msk->dst_port = 0;
		mtp_state_store(sk, TCP_CLOSE);
		mtp_err("connect err EADDRNOTAVAIL\n");
		return -EADDRNOTAVAIL;
	}

	MTP_INC_STATS(MTP_MIB_ACTIVEOPENS);
	err = mtp_send_syn(sk);
	if (unlikely(err)) {
		msk->dst_port = 0;
		sk->sk_prot->unhash(sk);
		mtp_set_state(sk, TCP_CLOSE);
		mtp_err("connect err %d\n", err);
		return err;
	}
	return 0;
}

static int mtp_bind_check(struct sockaddr_mtp *addr, int addr_len)
{
	if (addr_len < sizeof(struct sockaddr_mtp)) {
		mtp_err("addr_len %d less than sockaddr_mtp %u\n",
			  addr_len, sizeof(struct sockaddr_mtp));
		return -EINVAL;
	}

	if (addr->sa_family != AF_INET) {
		mtp_err("invalid addr family %u\n", addr->sa_family);
		return -EAFNOSUPPORT;
	}

	if (addr->type != MTP_ADDR_TYPE_MAC && addr->type != MTP_ADDR_TYPE_IPV4) {
		mtp_err("invalid addr type %u\n", addr->type);
		return -EINVAL;
	}

	return 0;
}

static int mtp_bind(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_mtp *addr = (struct sockaddr_mtp *)uaddr;
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct inet_sock *inet = (struct inet_sock *)sk;
	int err;

	err = mtp_bind_check(addr, addr_len);
	if (err)
		return err;

	lock_sock(sk);
	err = -EINVAL;
	/* Check these errors (active socket, double bind). */
	if (sk->sk_state != TCP_CLOSE || msk->src_port)
		goto out_release_sock;

	if (addr->type == MTP_ADDR_TYPE_MAC) {
		if (mtp_mac_any(addr->mac.addr, ETH_ALEN)) {
			mtp_err("mac addr is not set\n");
			err = -EADDRNOTAVAIL;
			goto out_release_sock;
		}
		(void)memcpy_s(msk->src_mac, ETH_ALEN, addr->mac.addr, ETH_ALEN);
		inet->inet_daddr = 0;
		inet->inet_rcv_saddr = inet->inet_saddr = 0;
	} else { /* It could only be MTP_ADDR_TYPE_IPV4 */
		if (dev_get_mac_by_inetaddr(addr->ip.addr, msk->src_mac)) {
			mtp_err("dev_get_mac_by_inetaddr failed\n");
			goto out_release_sock;
		}
		inet->inet_daddr = 0;
		inet->inet_rcv_saddr = inet->inet_saddr = addr->ip.addr;
	}

	mtp_info("try to bind to mac "MACFMT" port:%u\n", MACDATA(addr->mac.addr), addr->port);
	/* Make sure we are allowed to bind here. */
	err = sk->sk_prot->get_port(sk, addr->port);
	if (err != 0) {
		(void)memset_s(msk->src_mac, sizeof(msk->src_mac), 0, sizeof(msk->src_mac));
		mtp_err("get_port %u return err %d\n", addr->port, err);
		goto out_release_sock;
	}

	inet->inet_num = msk->src_port;
	inet->inet_sport = htons(inet->inet_num);
	inet->inet_dport = 0;
	if (!mtp_mac_any(msk->src_mac, ETH_ALEN))
		sk->sk_userlocks |= SOCK_BINDADDR_LOCK;
	if (msk->src_port)
		sk->sk_userlocks |= SOCK_BINDPORT_LOCK;

	err = 0;
out_release_sock:
	release_sock(sk);
	return err;
}

static int mtp_disconnect(struct sock *sk, int flags)
{
	int old_state = sk->sk_state;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	mtp_info("%u:%u disconnect\n", msk->src_port, msk->dst_port);
	mtp_send_reset(sk, msk->snd_nxt, msk->rcv_nxt, GFP_ATOMIC | __GFP_NOWARN);
	/* Change state AFTER socket is unhashed to avoid closed
	 * socket sitting in hash tables.
	 */
	if (old_state != TCP_CLOSE)
		mtp_set_state(sk, TCP_CLOSE);

	sk->sk_err = ESHUTDOWN;
	mtp_clear_xmit_timers(sk);
	__skb_queue_purge(&msk->reader_queue);
	mtp_write_queue_purge(sk);
	skb_rbtree_purge(&msk->out_of_order_queue);
	sk_mem_reclaim(sk);
	sk->sk_send_head = NULL;
	if (sk->sk_frag.page)
		mtp_warn("mtp should not using sk_frag\n");
	sk->sk_error_report(sk);
	return 0;
}

static int mtp_ioctl(struct sock *sk, int cmd, unsigned long arg)
{
	mtp_info("%u:%u ioctl\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
	return 0;
}

static void mtp_worker(struct work_struct *work)
{
	struct sock *sk = (struct sock *)container_of(work, struct mtp_sock, worker);

	smp_mb__before_atomic();
	clear_bit(MTP_MSQ_QUEUED, &sk->sk_tsq_flags);

	local_bh_disable();
	bh_lock_sock_nested(sk);

	if (!sock_owned_by_user(sk)) {
		clear_bit(MTP_MSQ_DEFERRED, &sk->sk_tsq_flags);
		mtp_msq_handler(sk);
	}
	bh_unlock_sock(sk);
	local_bh_enable();
	sock_put(sk);
}

void mtp_init_sock_base(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	msk->out_of_order_queue = RB_ROOT;
	msk->ooo_last_skb = NULL;
	skb_queue_head_init(&(msk->reader_queue));
	msk->rcv_wnd_free = 0;
	msk->srtt_us = 0;
	msk->mdev_us = jiffies_to_usecs(MTP_TIMEOUT_INIT);
	msk->rtx.rto = MTP_TIMEOUT_INIT;
	minmax_reset(&msk->rtt_min, MTP_JIFFIERS32, ~0U);

	msk->slow_start_mstamp = 0;
	msk->probe_rsp_mstamp = 0;
	msk->compress_probe = 0;
	msk->packets_out = 0;
	msk->retrans_out = 0;
	msk->lost_out = 0;
	msk->sacked_out = 0;
	msk->fackets_out = 0;
	msk->high_seq = msk->snd_nxt;
	msk->tlp_high_seq = 0;
	msk->lost_skb_hint = NULL;
	msk->retransmit_skb_hint = NULL;
	msk->d2d = false;
	msk->dev_node = NULL;
	msk->is_loopback = false;
	msk->cpu_id = -1;
	INIT_WORK(&msk->worker, mtp_worker);

	mtp_init_congestion_control(sk);
	mtp_init_xmit_timers(sk);
	sk->sk_pacing_rate = MTP_INIT_PACING_RATE;
	sk->sk_pacing_shift = MTP_PACING_SHIFT;
}

static void mtp_write_space(struct sock *sk)
{
	struct socket *sock = sk->sk_socket;
	struct socket_wq *wq;

	if ((sk->sk_sndbuf > sk->sk_wmem_queued) && sk_stream_is_writeable(sk) && sock) {
		clear_bit(SOCK_NOSPACE, &sock->flags);

		rcu_read_lock();
		wq = rcu_dereference(sk->sk_wq);
		if (skwq_has_sleeper(wq))
			wake_up_interruptible_poll(&wq->wait, POLLOUT |
						POLLWRNORM | POLLWRBAND);
		if (wq && wq->fasync_list && !(sk->sk_shutdown & SEND_SHUTDOWN))
			sock_wake_async(wq, SOCK_WAKE_SPACE, POLL_OUT);
		rcu_read_unlock();
	}
}

static int mtp_init_sock(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (sysctl_mtp_disable)
		return -EPROTONOSUPPORT;

	mtp_info("mtp socket create\n");

	mtp_init_sock_base(sk);
	sk->sk_no_check_tx = 1;
	sk->sk_no_check_rx = 1;
	msk->max_msg_size = MTP_DEFAULT_MSG_SIZE;
	msk->cur_rcv_msg_size = 0;
	msk->cur_snd_msg_size = 0;
	msk->sendpage_msg_size = 0;
	msk->xmit = mtp_direct_xmit;
	msk->segment = mtp_sendmsg_segment;
	msk->recv_queue = mtp_recv_queue;
	sk->sk_sndbuf = MTP_SND_BUF_SIZE;
	if (sk->sk_sndbuf > min(sysctl_rmem_max, sysctl_wmem_max))
		sk->sk_sndbuf = min(sysctl_rmem_max, sysctl_wmem_max);
	sk->sk_rcvbuf = sk->sk_sndbuf;
	sk->sk_userlocks |= (SOCK_SNDBUF_LOCK | SOCK_RCVBUF_LOCK);

	sk->sk_state = TCP_CLOSE;

	sk->sk_write_space = mtp_write_space;
	sock_set_flag(sk, SOCK_USE_WRITE_QUEUE);
	return 0;
}

static void mtp_destroy_sock(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	mtp_info("%u:%u destroy\n", msk->src_port, msk->dst_port);
	mtp_clear_xmit_timers(sk);
	__skb_queue_purge(&msk->reader_queue);
	mtp_write_queue_purge(sk);
	skb_rbtree_purge(&msk->out_of_order_queue);
	sk_mem_reclaim(sk);
	sk->sk_send_head = NULL;
	mtp_put_port(sk);
}

static int mtp_copy_int_from_sockptr(sockptr_t optval, unsigned int optlen, int *val)
{
	if (optlen >= sizeof(int)) {
		if (copy_from_sockptr(val, optval, sizeof(int)))
			return -EFAULT;
	} else if (optlen >= sizeof(char)) {
		unsigned char ucval;

		if (copy_from_sockptr(&ucval, optval, sizeof(ucval)))
			return -EFAULT;
		*val = (int) ucval;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int mtp_setsockopt_tos(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if (msk->tos != val) {
		mtp_info("set tos to %d\n", val);
		msk->tos = (__u8)val;
		sk->sk_priority = rt_tos2priority(val);
	}

	return 0;
}

static int mtp_setsockopt_keepidle(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if (val < 1 || val > MAX_MTP_KEEPIDLE) {
		mtp_info("%u:%u keepalive inval %d\n", msk->src_port, msk->dst_port, val);
		return -EINVAL;
	}
	msk->keepalive_time = val * HZ;
	mtp_info("%u:%u set keepalive_time %u\n", msk->src_port, msk->dst_port, msk->keepalive_time);
	if (sock_flag(sk, SOCK_KEEPOPEN) &&
	    !((1 << sk->sk_state) &
	      (TCPF_CLOSE | TCPF_LISTEN))) {
		u32 elapsed = MTP_JIFFIERS32 - msk->rcv_tstamp;
		if (msk->keepalive_time > elapsed)
			elapsed = msk->keepalive_time - elapsed;
		else
			elapsed = 0;
		sk_reset_timer(sk, &sk->sk_timer, jiffies + elapsed);
	}

	return 0;
}

static int mtp_setsockopt_max_msg_size(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if (val < 1) {
		mtp_err("local %u max_msg_size inval %d\n", msk->src_port, val);
		return -EINVAL;
	}
	if (sk->sk_state == TCP_CLOSE) {
		if (sk->sk_sndbuf < (val << (MTP_MSG_SHIFT + 1))) {
			sk->sk_sndbuf = val << (MTP_MSG_SHIFT + 1);
			sk->sk_rcvbuf = sk->sk_sndbuf << 1;
			mtp_info("local %u set sk_sndbuf %u sk_rcvbuf %u\n",
				msk->src_port, sk->sk_sndbuf, sk->sk_rcvbuf);
		}
		msk->max_msg_size = (u32)val << MTP_MSG_SHIFT;
		mtp_info("local %u set max_msg_size %u\n", msk->src_port, msk->max_msg_size);
	} else {
		mtp_err("local %u max_msg_size on state %u\n", msk->src_port, sk->sk_state);
		return -EPERM;
	}
	return 0;
}

static int mtp_setsockopt_d2d(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if (msk->src_port) {
		if (val && !msk->d2d) {
			ret = mtp_d2d_enable(msk);
			if (!ret)
				msk->d2d = true;
		} else if (!val && msk->d2d) {
			mtp_d2d_disable(msk);
			msk->d2d = false;
		}
	} else {
		ret = -ENODEV;
	}
	mtp_info("%u:%u dev_enable ret %d\n", msk->src_port, msk->dst_port, ret);
	return ret;
}

static int mtp_setsockopt_cpu_id(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;
	int cpu;
	int old_cpu;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if ((unsigned int)val >= nr_cpu_ids || !cpu_online(val)) {
		mtp_err("cpu_id %d not online\n", val);
		return -EINVAL;
	}

	cpu = val;
	old_cpu = msk->cpu_id;
	if (old_cpu == -1 && cpu != -1) {
		msk->cpu_id = cpu;
		mtp_info("%u:%u cpu id change from %d to %d\n", msk->src_port, msk->dst_port, old_cpu, cpu);
		return 0;
	}
	mtp_info("%u:%u already bind cpu\n", msk->src_port, msk->dst_port);
	return -EALREADY;
}

static int mtp_setsockopt_qdisc(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	mtp_info("enable MTP_QDISC %d\n", val);
	msk->xmit = (val ? dev_queue_xmit : mtp_direct_xmit);
	return 0;
}

static int mtp_setsockopt_cong_config(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_cong_config cfg;

	if (optlen < sizeof(msk->cong_cfg))
		return -EINVAL;

	if (sk->sk_state != TCP_CLOSE && sk->sk_state != TCP_LISTEN)
		return -ENOPROTOOPT;

	if (copy_from_sockptr(&cfg, optval, sizeof(cfg)))
		return -EFAULT;

	if (cfg.init_cwnd > (MTP_SND_CWND_CLAMP >> 1))
		return -EINVAL;
	if (cfg.cong_avoid_ratio > MTP_CONG_AVOID_RATIO_MAX)
		return -EINVAL;
	msk->cong_cfg = cfg;
	mtp_info("congestion init_cwnd %u cong_avoid_ratio %u\n", msk->cong_cfg.init_cwnd,
		msk->cong_cfg.cong_avoid_ratio);
	return 0;
}

static int mtp_setsockopt_sendpage_msg_size(struct sock *sk, sockptr_t optval, unsigned int optlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int ret;
	int val;

	if (sk->sk_state != TCP_ESTABLISHED)
		return -ENOTCONN;

	ret = mtp_copy_int_from_sockptr(optval, optlen, &val);
	if (ret)
		return ret;

	if ((val < (int)TLS_MAX_PAYLOAD_SIZE) || (val > (msk->max_msg_size >> 1)))
		return -EINVAL;

	mtp_info("set sendpage_msg_size to %d\n", val);
	msk->sendpage_msg_size = val;
	return 0;
}

typedef int (*mtp_setsockopt_hander)(struct sock *sk, sockptr_t optval, unsigned int optlen);

static mtp_setsockopt_hander setsockopt_handle[] = {
	mtp_setsockopt_tos, /* MTP_TOS */
	mtp_setsockopt_keepidle, /* MTP_KEEPIDLE */
	mtp_setsockopt_max_msg_size, /* MTP_MAX_MSG_SIZE */
	mtp_setsockopt_d2d, /* MTP_D2D */
	NULL, /* MTP_STATS */
	mtp_setsockopt_cpu_id, /* MTP_CPU_ID */
	mtp_setsockopt_qdisc, /* MTP_QDISC */
	mtp_setsockopt_cong_config, /* MTP_CONG_CONFIG */
	mtp_setsockopt_sendpage_msg_size, /* MTP_SENDPAGE_MSG_SIZE */
};

static int mtp_setsockopt(struct sock *sk, int level, int optname, sockptr_t optval,
		   unsigned int optlen)
{
	int err = 0;
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	mtp_setsockopt_hander handle;

	if (unlikely(level == SOL_TLS))
		return mtp_setsockopt_tls(sk, level, optname, optval, optlen);
	if (level != SOL_MTP)
		return -ENOPROTOOPT;

	if (optname <= 0 || optname > ARRAY_SIZE(setsockopt_handle)) {
		mtp_info("optname %d not supported\n", optname);
		return -ENOPROTOOPT;
	}

	lock_sock(sk);
	handle = setsockopt_handle[optname - 1];
	if (handle)
		err = handle(sk, optval, optlen);
	else
		err = -ENOPROTOOPT;
	release_sock(sk);
	mtp_debug("%u:%u setsockopt %d ret %d\n", msk->src_port, msk->dst_port, optname, err);
	return err;
}

static int mtp_copy_int_to_user(char __user *optval, int __user *optlen, int val, int len)
{
	if (len < sizeof(int) && len > 0 && val >= 0 && val <= 255) {
		unsigned char ucval = (unsigned char)val;
		len = 1;
		if (put_user(len, optlen))
			return -EFAULT;
		if (copy_to_user(optval, &ucval, 1))
			return -EFAULT;
	} else {
		len = min_t(unsigned int, sizeof(int), len);
		if (put_user(len, optlen))
			return -EFAULT;
		if (copy_to_user(optval, &val, len))
			return -EFAULT;
	}
	return 0;
}

static int mtp_getsockopt_tos(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	len = min_t(int, len, sizeof(int));
	if (len <= 0)
		return -EINVAL;

	val = msk->tos;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_keepidle(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	if (len < sizeof(int))
		return -EINVAL;
	len = sizeof(int);
	val = msk->keepalive_time / HZ;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_max_msg_size(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	if (len < sizeof(int))
		return -EINVAL;
	len = sizeof(int);
	val = msk->max_msg_size >> MTP_MSG_SHIFT;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_d2d(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	len = min_t(int, len, sizeof(int));
	if (len <= 0)
		return -EINVAL;

	val = msk->d2d;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_stats(struct sock *sk, char __user *optval, int __user *optlen)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct mtp_sock_stat stats = msk->stats;
	int len;

	if (get_user(len, optlen))
		return -EFAULT;

	len = min_t(int, sizeof(stats), len);
	if (put_user(len, optlen))
		return -EFAULT;

	if (len < sizeof(msk->stats))
		mtp_err("len %d should be %zd\n", len, sizeof(msk->stats));
	stats.cwnd = msk->snd_cwnd;
	stats.srtt = msk->srtt_us >> 8;
	if (copy_to_user(optval, &stats, len))
		return -EFAULT;
	return 0;
}

static int mtp_getsockopt_cpu_id(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	len = min_t(int, len, sizeof(int));
	if (len <= 0)
		return -EINVAL;

	val = msk->cpu_id;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_qdisc(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	len = min_t(int, len, sizeof(int));
	if (len <= 0)
		return -EINVAL;

	val = (msk->xmit == dev_queue_xmit);
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

static int mtp_getsockopt_cong_config(struct sock *sk, char __user *optval, int __user *optlen)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct mtp_cong_config cfg = msk->cong_cfg;
	int len;

	if (get_user(len, optlen))
		return -EFAULT;

	len = min_t(int, sizeof(cfg), len);
	if (put_user(len, optlen))
		return -EFAULT;

	if (len < sizeof(cfg))
		mtp_err("len %zd should be %zd\n", len, sizeof(cfg));
	if (copy_to_user(optval, &cfg, len))
		return -EFAULT;
	return 0;
}

static int mtp_getsockopt_sendpage_msg_size(struct sock *sk, char __user *optval, int __user *optlen)
{
	int val, len;
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	if (get_user(len, optlen))
		return -EFAULT;
	len = min_t(int, len, sizeof(int));
	if (len <= 0)
		return -EINVAL;

	val = (int)msk->sendpage_msg_size;
	return mtp_copy_int_to_user(optval, optlen, val, len);
}

typedef int (*mtp_getsockopt_hander)(struct sock *sk, char __user *optval, int __user *optlen);

static mtp_getsockopt_hander getsockopt_handle[] = {
	mtp_getsockopt_tos, /* MTP_TOS */
	mtp_getsockopt_keepidle, /* MTP_KEEPIDLE */
	mtp_getsockopt_max_msg_size, /* MTP_MAX_MSG_SIZE */
	mtp_getsockopt_d2d, /* MTP_D2D */
	mtp_getsockopt_stats, /* MTP_STATS */
	mtp_getsockopt_cpu_id, /* MTP_CPU_ID */
	mtp_getsockopt_qdisc, /* MTP_QDISC */
	mtp_getsockopt_cong_config, /* MTP_CONG_CONFIG */
	mtp_getsockopt_sendpage_msg_size, /* MTP_SENDPAGE_MSG_SIZE */
};

static int mtp_getsockopt(struct sock *sk, int level, int optname, char __user *optval,
			int __user *optlen)
{
	int err;
	mtp_getsockopt_hander handle;

	if (unlikely(level == SOL_TLS))
		return mtp_getsockopt_tls(sk, optname, optval, optlen);

	if (level != SOL_MTP)
		return -ENOPROTOOPT;

	if (optname <= 0 || optname > ARRAY_SIZE(getsockopt_handle)) {
		mtp_info("optname %d not supported\n", optname);
		return -ENOPROTOOPT;
	}

	lock_sock(sk);
	handle = getsockopt_handle[optname - 1];
	if (handle)
		err = handle(sk, optval, optlen);
	else
		err = -ENOPROTOOPT;
	release_sock(sk);
	return err;
}

static void mtp_cleanup_rbuf(struct sock *sk, u16 copied_skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u16 win_thresh;
	u16 block_win;
	u16 full_win = mtp_full_win(sk);

	msk->rcv_wnd_free += copied_skb;
	win_thresh = full_win >> MTP_RWIN_THRESH_SHIF;
	block_win = mtp_block_win(sk);
	/* if not send ack for 1/2 window since last send ack or the block_win is almost full(3/4 full window) */
	if (msk->rcv_wnd_free > (full_win >> 1) || block_win > (full_win - win_thresh)) {
		if (sk->sk_state == TCP_ESTABLISHED)
			mtp_send_ack(sk, 0);
	}
}

static int mtp_recvmsg_wait(struct sock *sk, long timeo)
{
	struct sk_buff *last;
	struct sk_buff_head *sk_queue = &sk->sk_receive_queue;
	struct mtp_sock *msk = mtp_sk(sk);
	int error;

	if (sk->sk_state == TCP_LISTEN)
		return -ENOTCONN;

	if (msk->tls_ctx && msk->tls_ctx->rx) {
		if (unlikely(msk->tls_ctx->rx->open_rec->error))
			return -EBADMSG;
		else if (msk->tls_ctx->rx->open_rec->user_data_len)
			return 0;
	}

	if (!skb_queue_empty(sk_queue))
		return 0;

	error = sock_error(sk);
	if (error)
		return error;

	if ((sk->sk_shutdown & RCV_SHUTDOWN) && skb_queue_empty(sk_queue))
		return -EPIPE;
	else if (sk->sk_state == TCP_CLOSE)
		return -ENOTCONN;

	while (skb_queue_empty(sk_queue)) {
		if (!timeo)
			return -EAGAIN;

		if (signal_pending(current)) {
			error = timeo ? sock_intr_errno(timeo) : -EAGAIN;
			return error;
		}
		last = skb_peek_tail(sk_queue);
		sk_wait_data(sk, &timeo, last);
		if (sock_flag(sk, SOCK_DONE) || (sk->sk_shutdown & RCV_SHUTDOWN)) {
			if (skb_peek(sk_queue))
				break;
			else
				return -EPIPE;
		}

		if (sk->sk_err)
			return sock_error(sk);
	}

	return 0;
}

static void mtp_recvmsg_addr(struct sock *sk, struct sockaddr_mtp *sin, int *addr_len)
{
	sin->sa_family = AF_INET;
	sin->port = mtp_sk(sk)->dst_port;
	sin->type = MTP_ADDR_TYPE_MAC;
	(void)memcpy_s(sin->mac.addr, ETH_ALEN, mtp_sk(sk)->dst_mac, ETH_ALEN);
	*addr_len = sizeof(*sin);
}

static int mtp_recv_queue(struct sock *sk, struct msghdr *msg, size_t len, u16 *copied_skb)
{
	struct sk_buff *skb, *tmp;
	int copied = 0;
	int err;
	struct mtp_sock *msk = mtp_sk(sk);

	skb_queue_walk_safe(&sk->sk_receive_queue, skb, tmp) {
		u32 offset = msk->copied_seq - MTP_SKB_CB(skb)->seq;
		unsigned long used;
		u8 type = MTP_SKB_CB(skb)->type;

		if (offset >= skb->len) {
			if (type == MTPHDR_TYPE_FIN) {
				copied = copied ? : -EPIPE;
				goto found_fin_ok;
			}
			mtp_err("%u:%u offset %u larger than skb len %u\n", mtp_sk(sk)->src_port,
				mtp_sk(sk)->dst_port, offset, skb->len);
			break;
		}

		used = min_t(unsigned long, skb->len - offset, len);
		if ((copied + used) > mtp_sk(sk)->max_msg_size) {
			mtp_err("%u:%u msg size too big, copied %d len %d\n max_msg_size %u", mtp_sk(sk)->src_port,
				mtp_sk(sk)->dst_port, copied, skb->len, mtp_sk(sk)->max_msg_size);
			break;
		}
		err = skb_copy_datagram_msg(skb, offset, msg, used);
		if (err) {
			mtp_err("%u:%u skb_copy_datagram_msg failed err %d\n",
				mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, err);
			/* Exception. Bailout! */
			if (!copied)
				copied = -EFAULT;
			break;
		}
		copied += used;
		len -= used;
		WRITE_ONCE(msk->copied_seq, msk->copied_seq + used);

		if (used + offset < skb->len)
			break;
found_fin_ok:
		*copied_skb += 1;
		sk_eat_skb(sk, skb);
		if (type == MTPHDR_TYPE_FIN) {
			WRITE_ONCE(msk->copied_seq, msk->copied_seq + 1);
			break;
		}
		if ((type == MTPHDR_TYPE_MSG_END) || len <= 0)
			break;
	}

	return copied;
}

static int mtp_recvmsg(struct sock *sk, struct msghdr *msg, size_t len, int noblock,
	int flags, int *addr_len)
{
	int err;
	int copied = 0;
	u16 copied_skb = 0;
	long timeo = sock_rcvtimeo(sk, noblock);
	DECLARE_SOCKADDR(struct sockaddr_mtp *, sin, msg->msg_name);
	struct mtp_sock *msk = mtp_sk(sk);

	if (unlikely(len <= 0))
		return -EINVAL;

	lock_sock(sk);
	err = mtp_recvmsg_wait(sk, timeo);
	if (err) {
		release_sock(sk);
		mtp_err("%u:%u error %d\n", msk->src_port, msk->dst_port, err);
		return err;
	}

	copied = msk->recv_queue(sk, msg, len, &copied_skb);
	if (likely(copied_skb > 0))
		mtp_cleanup_rbuf(sk, copied_skb);
	release_sock(sk);
	/* Copy the address. */
	if (copied > 0 && sin)
		mtp_recvmsg_addr(sk, sin, addr_len);
	mtp_debug("%u:%u mtp_recvmsg copied %d\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, copied);
	return copied;
}

void mtp_skb_entail(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_skb_cb *dcb = MTP_SKB_CB(skb);

	skb->csum    = 0;
	dcb->seq     = dcb->end_seq = msk->write_seq;
	dcb->flags   = 0;
	dcb->type = MTPHDR_TYPE_DEFAULT;
	dcb->opt_flags = 0;
	dcb->sacked  = 0;
	__skb_header_release(skb);
	__skb_queue_tail(&sk->sk_write_queue, skb);
	sk->sk_wmem_queued += skb->truesize;
	sk_mem_charge(sk, skb->truesize);

	/* Queue it, remembering where we must start sending. */
	if (sk->sk_send_head == NULL) {
		sk->sk_send_head = skb;

		if (msk->highest_sack == NULL)
			msk->highest_sack = skb;
	}
}

static void msk_forced_mem_schedule(struct sock *sk, int size)
{
	int amt;

	if (size <= sk->sk_forward_alloc)
		return;
	amt = sk_mem_pages(size);
	sk->sk_forward_alloc += amt * SK_MEM_QUANTUM;
	sk_memory_allocated_add(sk, amt);
}

struct sk_buff *msk_walloc_skb(struct sock *sk, int size, gfp_t gfp)
{
	struct sk_buff *skb;
	int max_header = sk->sk_prot->max_header;
	int tot_size;

	if (mtp_sk(sk)->is_hi1106)
		max_header -= (ALIGN(ETH_HLEN, ALIGIN_SIZE_4) - ETH_HLEN);
	tot_size = ALIGN(size + max_header, ALIGIN_SIZE_4);
	skb = alloc_skb_fclone(tot_size, gfp);
	if (likely(skb)) {
		msk_forced_mem_schedule(sk, skb->truesize);
		skb_reserve(skb, max_header);
		/*
		 * Make sure that we have exactly size bytes
		 * available to the caller, no more, no less.
		 */
		skb->reserved_tailroom = skb->end - skb->tail - size;
		return skb;
	} else {
		mtp_err("alloc_skb_fclone failed\n");
		sk_stream_moderate_sndbuf(sk);
	}
	return NULL;
}

static void mtp_remove_empty_skb(struct sock *sk, struct sk_buff *skb)
{
	if (skb && !skb->len &&
	    MTP_SKB_CB(skb)->end_seq == MTP_SKB_CB(skb)->seq) {
		mtp_unlink_write_queue(skb, sk);
		mtp_check_send_head(sk, skb);
		mtp_wmem_free_skb(sk, skb);
	}
}

static void mtp_sendmsg_segment_update(struct sock *sk, struct msghdr *msg,
				       struct sk_buff *skb, int seg, int copy)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_skb_cb *dcb;

	msk->write_seq += (u32)copy;
	dcb = MTP_SKB_CB(skb);
	dcb->end_seq += (u32)copy;
	msk->cur_snd_msg_size += (u32)copy;

	if (!msg_data_left(msg) && !(msg->msg_flags & MSG_SENDPAGE_NOTLAST)) {
		msk->stats.out_msgs++;
		dcb->type = MTPHDR_TYPE_MSG_END;
		msk->cur_snd_msg_size = 0;
		return;
	}

	/* if batch sending is enabled, always caches packet, even if list is empty */
	if (msk->is_hi1106) {
		if ((seg % MTP_BATCH_SENDING_MAX) == 0)
			mtp_write_xmit(sk, 0, sk->sk_allocation);

		return;
	}

	if ((seg % MTP_SEG_BURST) == 0)
		mtp_write_xmit(sk, 0, sk->sk_allocation);
	else if (skb == mtp_send_head(sk) && (seg == 1))
		mtp_write_xmit(sk, 1, sk->sk_allocation);
}

static int mtp_sendmsg_segment(struct sock *sk, struct msghdr *msg, long timeo,
			       int *seg, bool *process_backlog)
{
	struct sk_buff *skb;
	struct mtp_sock *msk = mtp_sk(sk);
	int copy = 0;
	int err;
	unsigned int mss = msk->mss;

	skb = mtp_write_queue_tail(sk);
	if (mtp_send_head(sk))
		copy = mss - skb->len;

	if (copy <= 0 || !mtp_skb_can_collapse_to(skb)) {
		if (*process_backlog && sk_flush_backlog(sk)) {
			*process_backlog = false;
			return -ERESTART;
		}
		/* get a new skb */
		skb = msk_walloc_skb(sk, mss, sk->sk_allocation);
		if (skb == NULL) {
			if (*seg)
				mtp_write_xmit(sk, 0, sk->sk_allocation);
			mtp_info("msk_walloc_skb failed\n");
			err = sk_stream_wait_memory(sk, &timeo);
			return err;
		}
		mtp_skb_entail(sk, skb);
		copy = (int)mss;
		*process_backlog = true;
		*seg += 1;
	}
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	/* Try to append data to the end of skb. */
	if (copy > msg_data_left(msg))
		copy = msg_data_left(msg);

	/* We have some space in skb head. Superb! */
	copy = min_t(int, copy, skb_availroom(skb));
	err = skb_add_data_nocache(sk, skb, &msg->msg_iter, copy);
	if (err)
		return err;

	mtp_sendmsg_segment_update(sk, msg, skb, *seg, copy);
	return copy;
}

static int mtp_sendmsg_wait(struct sock *sk, long *timeo)
{
	if (!sk_stream_memory_free(sk)) {
		mtp_debug("%u:%u wait_for_sndbuf\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port);
		set_bit(SOCK_NOSPACE, &sk->sk_socket->flags);
		return sk_stream_wait_memory(sk, timeo);
	}

	return 0;
}

static void mtp_sendpage_flag_handle(struct sock *sk, int flags)
{
	struct mtp_sock *msk = mtp_sk(sk);
	if (unlikely(flags & MSG_SENDPAGE_NOPOLICY)) {
		if (msk->tls_ctx && msk->tls_ctx->tx) {
			mtp_info("flush pending pages\n");
			mtp_tls_send_pending_page(sk);
		}
	}
}

static int mtp_sendmsg_locked(struct sock *sk, struct msghdr *msg, size_t len)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int copy;
	int copied = 0;
	int err;
	long timeo;
	bool process_backlog = false;
	int seg = 0;

	timeo = sock_sndtimeo(sk, msg->msg_flags & MSG_DONTWAIT);

	if ((1 << sk->sk_state) & (~TCPF_ESTABLISHED)) {
		err = sk_stream_wait_connect(sk, &timeo);
		if (err != 0)
			goto do_error;
	}

	if (len > msk->max_msg_size)
		return -EINVAL;

	err = mtp_sendmsg_wait(sk, &timeo);
	if (err)
		return err;

	mtp_sendpage_flag_handle(sk, msg->msg_flags);
	/* This should be in poll */
	sk_clear_bit(SOCKWQ_ASYNC_NOSPACE, sk);
restart:
	err = -EPIPE;
	if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN))
		goto do_error;
	while (msg_data_left(msg)) {
		copy = msk->segment(sk, msg, timeo, &seg, &process_backlog);
		if (likely(copy >= 0)) {
			copied += copy;
		} else if (copy == -ERESTART) {
			goto restart;
		} else {
			err = copy;
do_error:
			mtp_err("do_error err %d\n", err);
			mtp_remove_empty_skb(sk, mtp_write_queue_tail(sk));
			if (copied)
				goto out;
			err = sk_stream_error(sk, msg->msg_flags, err);
			/* make sure we wake any epoll edge trigger waiter */
			if (unlikely(skb_queue_len(&sk->sk_write_queue) == 0 && err == -EAGAIN))
				sk->sk_write_space(sk);
			return err;
		}
	}

out:
	mtp_write_xmit(sk, 0, sk->sk_allocation);
	mtp_debug("%u:%u send len %d\n", msk->src_port, msk->dst_port, copied);
	return copied;
}

static int mtp_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
	int ret;

	lock_sock(sk);
	ret = mtp_sendmsg_locked(sk, msg, len);
	release_sock(sk);

	return ret;
}

static int mtp_sock_listen(struct socket *sock, int backlog)
{
	struct sock *sk = sock->sk;
	struct mtp_sock *msk = mtp_sk(sk);
	unsigned char ostate;
	int err = -EINVAL;

	lock_sock(sk);
	if (sock->state != SS_UNCONNECTED)
		goto out;

	ostate = sk->sk_state;
	if (!((1 << ostate) & (TCPF_CLOSE | TCPF_LISTEN)))
		goto out;

	if (!msk->src_port)
		goto out;
	if (ostate != TCP_LISTEN) {
		spin_lock_init(&msk->accept_queue.lock);
		INIT_LIST_HEAD(&msk->accept_queue.accept);
		mtp_set_state(sk, TCP_LISTEN);
		err = sk->sk_prot->hash(sk);
		if (unlikely(err))
			goto out;

		sk->sk_ack_backlog = 0;
	}
	sk->sk_max_ack_backlog = backlog;
	err = 0;

out:
	release_sock(sk);
	mtp_info("server %u listen return %d\n", msk->src_port, err);
	return err;
}

static int mtp_accwpt_wait(struct sock *sk, long timeo)
{
	struct mtp_sock *msk = mtp_sk(sk);
	DEFINE_WAIT(wait);
	int err;

	for (;;) {
		prepare_to_wait_exclusive(sk_sleep(sk), &wait,
					  TASK_INTERRUPTIBLE);
		release_sock(sk);
		if (list_empty(&msk->accept_queue.accept))
			timeo = schedule_timeout(timeo);
		sched_annotate_sleep();
		lock_sock(sk);
		if (!list_empty(&msk->accept_queue.accept)) {
			err = 0;
			break;
		}
		if (sk->sk_state != TCP_LISTEN) {
			err = -EINVAL;
			break;
		}
		if (signal_pending(current)) {
			err = sock_intr_errno(timeo);
			break;
		}
		if (!timeo) {
			err = -EAGAIN;
			break;
		}
	}
	finish_wait(sk_sleep(sk), &wait);
	return err;
}

static struct sock *mtp_accept(struct sock *sk, int flags, int *err, bool kern)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sock *newsk = NULL;

	lock_sock(sk);

	if (sk->sk_state != TCP_LISTEN) {
		*err = -EINVAL;
		goto out;
	}

	if (list_empty(&msk->accept_queue.accept)) {
		long timeo = sock_rcvtimeo(sk, flags & O_NONBLOCK);
		if (!timeo) {
			*err = -EAGAIN;
			goto out;
		}

		*err = mtp_accwpt_wait(sk, timeo);
		if (*err)
			goto out;
	}
	newsk = mtp_reqsk_remove(msk);
	*err = 0;
out:
	release_sock(sk);
	mtp_info("server %u accept return %d\n", msk->src_port, *err);
	return newsk;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
#define MTP_NAME_ADDR struct sockaddr *uaddr
#else
#define MTP_NAME_ADDR  struct sockaddr *uaddr, int *uaddr_len
#endif
static int mtp_sock_getname(struct socket *sock, MTP_NAME_ADDR, int peer)
{
	struct sock *sk = sock->sk;
	struct mtp_sock *msk = mtp_sk(sk);
	struct sockaddr_mtp *addr = (struct sockaddr_mtp *)uaddr;

	(void)memset_s(addr, sizeof(*addr), 0, sizeof(*addr));
	addr->sa_family = AF_INET;
	addr->type = MTP_ADDR_TYPE_MAC;
	if (peer) {
		if (!msk->dst_port ||
		    (((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_SYN_SENT)) &&
		     peer == 1))
			return -ENOTCONN;
		addr->port = msk->dst_port;
		(void)memcpy_s(addr->mac.addr, ETH_ALEN, msk->dst_mac, ETH_ALEN);
	} else {
		addr->port = msk->src_port;
		(void)memcpy_s(addr->mac.addr, ETH_ALEN, msk->src_mac, ETH_ALEN);
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	return sizeof(*addr);
#else
	*uaddr_len = sizeof(*addr);
	return 0;
#endif
}

static unsigned int mtp_sock_poll(struct file *file, struct socket *sock, poll_table *wait)
{
	unsigned int mask = 0;
	struct sock *sk = sock->sk;
	const struct mtp_sock *msk = mtp_sk(sk);
	int state;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	sock_poll_wait(file, sock, wait);
#else
	sock_poll_wait(file, sk_sleep(sk), wait);
#endif
	state = mtp_state_load(sk);
	if (state == TCP_LISTEN)
		return list_empty(&msk->accept_queue.accept) ? 0 :
			(POLLIN | POLLRDNORM);

	if (sk->sk_shutdown == SHUTDOWN_MASK || state == TCP_CLOSE)
		mask |= POLLHUP;
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= POLLIN | POLLRDNORM | POLLRDHUP;

	if (state != TCP_SYN_SENT) {
		if (!skb_queue_empty_lockless(&sk->sk_receive_queue) || msk->tls_recv_pend)
			mask |= POLLIN | POLLRDNORM;

		if (!(sk->sk_shutdown & SEND_SHUTDOWN)) {
			/* writable? */
			if ((sk->sk_sndbuf > sk->sk_wmem_queued) && sk_stream_is_writeable(sk))
				mask |= POLLOUT | POLLWRNORM;
			else
				sk_set_bit(SOCKWQ_ASYNC_NOSPACE, sk);
		} else {
			mask |= POLLOUT | POLLWRNORM;
		}
	}
	/* This barrier is coupled with smp_wmb() in mtp_reset() */
	smp_rmb();
	if (sk->sk_err || !skb_queue_empty_lockless(&sk->sk_error_queue))
		mask |= POLLERR;

	return mask;
}

static int mtp_abort(struct sock *sk, int err)
{
	mtp_info("enter %s\n", __func__);
	return 0;
}

static int mtp_sendpage_locked(struct sock *sk, struct page *page, int offset,
			       size_t size, int flags)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (msk->tls_ctx && msk->tls_ctx->tx) {
		int ret;
		long timeo;

		timeo = sock_sndtimeo(sk, flags & MSG_DONTWAIT);

		if ((1 << sk->sk_state) & (~TCPF_ESTABLISHED)) {
			ret = sk_stream_wait_connect(sk, &timeo);
			if (ret)
				return ret;
		}

		ret = mtp_sendmsg_wait(sk, &timeo);
		if (ret)
			return ret;

		/* This should be in poll */
		sk_clear_bit(SOCKWQ_ASYNC_NOSPACE, sk);
		if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN)) {
			ret = sk_stream_error(sk, flags, -EPIPE);
			return ret;
		}
		return mtp_tls_sendpage_locked(sk, page, offset, size, flags);
	}

	return (int)sock_no_sendpage_locked(sk, page, offset, size, flags);
}

static int mtp_sendpage(struct sock *sk, struct page *page, int offset,
		 size_t size, int flags)
{
	int ret;

	lock_sock(sk);
	ret = mtp_sendpage_locked(sk, page, offset, size, flags);
	release_sock(sk);

	return ret;
}

static struct proto mtp_prot = {
	.name		 = "MTP",
	.owner		= THIS_MODULE,
	.close		= mtp_close,
	.connect	= mtp_connect,
	.bind		= mtp_bind,
	.disconnect     = mtp_disconnect,
	.accept 	= mtp_accept,
	.ioctl          = mtp_ioctl,
	.init           = mtp_init_sock,
	.destroy        = mtp_destroy_sock,
	.setsockopt     = mtp_setsockopt,
	.getsockopt     = mtp_getsockopt,
	.keepalive      = mtp_set_keepalive,
	.recvmsg        = mtp_recvmsg,
	.sendmsg        = mtp_sendmsg,
	.sendpage       = mtp_sendpage,
	.backlog_rcv    = mtp_l2_do_rcv,
	.release_cb     = mtp_release_cb,
	.hash           = mtp_lib_hash,
	.unhash         = mtp_lib_unhash,
	.get_port       = mtp_get_port,
	.memory_allocated = &mtp_memory_allocated,
	.sysctl_mem     = sysctl_mtp_mem,
	.sysctl_wmem    = &sysctl_mtp_wmem_min,
	.sysctl_rmem    = &sysctl_mtp_rmem_min,
	.max_header	= MAX_MTP_HEADER,
	.obj_size       = sizeof(struct mtp_sock),
	.slab_flags     = SLAB_TYPESAFE_BY_RCU,
	.h.hashinfo     = NULL,
	.no_autobind    = true,
	.diag_destroy   = mtp_abort,
};

/*
 * MTP splice context
 */
struct mtp_splice_state {
	struct pipe_inode_info *pipe;
	size_t len;
	unsigned int flags;
};

static int mtp_read_sock(struct sock *sk, read_descriptor_t *desc,
		  sk_read_actor_t recv_actor)
{
	struct sk_buff *skb, *tmp;
	struct mtp_sock *msk = mtp_sk(sk);
	u32 seq = msk->copied_seq;
	int copied = 0;
	u16 copied_skb = 0;

	skb_queue_walk_safe(&sk->sk_receive_queue, skb, tmp) {
		int used;
		u32 offset = seq - MTP_SKB_CB(skb)->seq;
		u8 type = MTP_SKB_CB(skb)->type;
		if (unlikely(type == MTPHDR_TYPE_SYN || type == MTPHDR_TYPE_HANDSHAKE)) {
			mtp_err("can't recv on a handshake skb\n");
			offset--;
		}

		if (offset >= skb->len) {
			/* This looks weird, but this can happen if TCP collapsing
			 * splitted a fat GRO packet, while we released socket lock
			 * in skb_splice_bits()
			 */
			sk_eat_skb(sk, skb);
			copied_skb++;
			continue;
		}
		used = recv_actor(desc, skb, offset, skb->len - offset);
		if (used <= 0) {
			if (!copied)
				copied = used;
			break;
		}

		copied += used;
		seq += used;
		if ((offset + used) < skb->len)
			break;

		copied_skb++;
		sk_eat_skb(sk, skb);
		if (type == MTPHDR_TYPE_FIN)
			++seq;

		if (!desc->count || type == MTPHDR_TYPE_MSG_END || type == MTPHDR_TYPE_FIN)
			break;
		WRITE_ONCE(msk->copied_seq, seq);
	}
	WRITE_ONCE(msk->copied_seq, seq);

	if (likely(copied_skb))
		mtp_cleanup_rbuf(sk, copied_skb);
	return copied;
}

static int mtp_splice_data_recv(read_descriptor_t *rd_desc, struct sk_buff *skb,
				unsigned int offset, size_t len)
{
	struct mtp_splice_state *tss = rd_desc->arg.data;
	int ret;

	ret = skb_splice_bits(skb, skb->sk, offset, tss->pipe,
			      min(rd_desc->count, len), tss->flags);
	if (ret > 0)
		rd_desc->count -= ret;
	return ret;
}

static ssize_t mtp_splice_read(struct socket *sock, loff_t *ppos,
	struct pipe_inode_info *pipe, size_t len, unsigned int flags)
{
	struct sock *sk = sock->sk;
	struct mtp_sock *msk = (struct mtp_sock *)sk;
	struct mtp_splice_state tss = {
		.pipe = pipe,
		.len = len,
		.flags = flags,
	};
	read_descriptor_t rd_desc;
	long timeo;
	int ret;

	/*
	 * We can't seek on a socket input
	 */
	if (unlikely(*ppos))
		return -ESPIPE;

	if (unlikely(len <= 0))
		return -EINVAL;
	lock_sock(sk);

	timeo = sock_rcvtimeo(sk, sock->file->f_flags & O_NONBLOCK);
	ret = mtp_recvmsg_wait(sk, timeo);
	if (ret) {
		release_sock(sk);
		mtp_err("%u:%u error %d\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, ret);
		return ret;
	}

	if (msk->tls_ctx && msk->tls_ctx->rx) {
		u16 copied_skb = 0;
		ret = mtp_tls_splice_read(sk, pipe, len, &copied_skb);
		if (likely(copied_skb))
			mtp_cleanup_rbuf(sk, copied_skb);
	} else {
		/* Store MTP splice context information in read_descriptor_t. */
		rd_desc.arg.data = &tss;
		rd_desc.count = tss.len;
		ret = mtp_read_sock(sk, &rd_desc, mtp_splice_data_recv);
	}
	release_sock(sk);
	mtp_debug("%u:%u mtp_splice_read ret %u\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, ret);
	return ret;
}

static const struct proto_ops mtp_ops = {
	.family		= PF_INET,
	.owner		= THIS_MODULE,
	.release	= inet_release,
	.bind		= inet_bind,
	.connect	= inet_stream_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= inet_accept,
	.getname	= mtp_sock_getname,
	.poll		= mtp_sock_poll,
	.ioctl		= inet_ioctl,
	.listen		= mtp_sock_listen,
	.shutdown	= inet_shutdown,
	.setsockopt	= sock_common_setsockopt,
	.getsockopt	= sock_common_getsockopt,
	.sendmsg	= inet_sendmsg,
	.sendmsg_locked	= mtp_sendmsg_locked,
	.recvmsg	= inet_recvmsg,
	.mmap		= sock_no_mmap,
	.sendpage	= inet_sendpage,
	.sendpage_locked = mtp_sendpage_locked,
	.splice_read	= mtp_splice_read,
	.read_sock	= mtp_read_sock,
	.set_peek_off	= sk_set_peek_off,
};

static struct inet_protosw mtp_protosw = {
	.type		= SOCK_DGRAM,
	.protocol	= IPPROTO_MINTP,
	.prot		= &mtp_prot,
	.ops		= &mtp_ops,
	.flags		= 0
};

static struct packet_type mtp_packet_type __read_mostly = {
	.type = cpu_to_be16(ETH_P_MTP),
	.func = mtp_l2_rcv,
};

static int mtp_notifier(struct notifier_block *this,
			unsigned long msg, void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);

	if (msg != NETDEV_DOWN)
		return NOTIFY_DONE;

	mtp_device_down(dev);
	return NOTIFY_DONE;
}

static struct notifier_block mtp_netdev_notifier = {
	.notifier_call = mtp_notifier,
};

static struct ctl_table net_core_mtp[] = {
	{
		.procname	= "mtp_debug",
		.data		= &sysctl_mtp_debug,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "mtp_queue",
		.data		= &sysctl_mtp_queue,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "mtp_ack_timer",
		.data		= &sysctl_mtp_ack_timer,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "mtp_limit_output_bytes",
		.data		= &mtp_limit_output_bytes,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "mtp_disable",
		.data		= &sysctl_mtp_disable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

static void mtp_mem_init(void)
{
	unsigned long limit;
	limit = nr_free_buffer_pages() / 8;
	limit = max(limit, 128UL);
	sysctl_mtp_mem[0] = limit / 4 * 3;
	sysctl_mtp_mem[1] = limit;
	sysctl_mtp_mem[2] = sysctl_mtp_mem[0] * 2;

	sysctl_mtp_rmem_min = SK_MEM_QUANTUM;
	sysctl_mtp_wmem_min = SK_MEM_QUANTUM;
}

static int32_t __init mtp_mod_init(void)
{
	int rc = mtp_tasklet_init();
	if (rc) {
		mtp_err("mtp_tasklet_init fail %d\n", rc);
		return rc;
	}

	rc = mtp_mib_init();
	if (rc) {
		mtp_err("mtp_mib_init fail %d\n", rc);
		goto mib_fail;
	}

	rc = proto_register(&mtp_prot, 1);
	if (rc) {
		mtp_err("Cannot register mtp protocol ret %d\n", rc);
		goto proto_fail;
	}
	mtp_hashinfo_init();
	rc = register_netdevice_notifier(&mtp_netdev_notifier);
	if (rc) {
		mtp_err("register_netdevice_notifier failed %d\n", rc);
		goto notifier_fail;
	}
	inet_register_protosw(&mtp_protosw);
	dev_add_pack(&mtp_packet_type);
	mtp_mem_init();

	g_mtp_ctl_table = register_net_sysctl(&init_net, "net/core", net_core_mtp);
	if (!g_mtp_ctl_table)
		mtp_warn("register_net_sysctl failed\n");
#ifdef CONFIG_PROC_FS
	rc = mtp_proc_init();
	if (rc)
		mtp_warn("mtp_proc_init failed\n");
	else
		g_mtp_proc_init = true;
#endif /* CONFIG_PROC_FS */
	mtp_info("mod init success\n");
	return 0;
notifier_fail:
	proto_unregister(&mtp_prot);
proto_fail:
	mtp_mib_exit();
mib_fail:
	mtp_tasklet_destroy();
	return rc;
}

static void __exit mtp_mod_exit(void)
{
#ifdef CONFIG_PROC_FS
	if (g_mtp_proc_init) {
		mtp_proc_exit();
		g_mtp_proc_init = false;
	}
#endif /* CONFIG_PROC_FS */
	if (g_mtp_ctl_table)
		unregister_net_sysctl_table(g_mtp_ctl_table);
	mtp_hashinfo_exit();
	unregister_netdevice_notifier(&mtp_netdev_notifier);
	inet_unregister_protosw(&mtp_protosw);
	proto_unregister(&mtp_prot);
	dev_remove_pack(&mtp_packet_type);
	mtp_mib_exit();
	mtp_tasklet_destroy();
	mtp_info("mod exit success\n");
}

module_init(mtp_mod_init);
module_exit(mtp_mod_exit);

MODULE_AUTHOR("Linux Kernel mtp developers <liyouyong@huawei.com>");
MODULE_AUTHOR("Linux Kernel mtp developers <songqiubin@huawei.com>");
MODULE_DESCRIPTION("Support for the mtp protocol");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.1");

