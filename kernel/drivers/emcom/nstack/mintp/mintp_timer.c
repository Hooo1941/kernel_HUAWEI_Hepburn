/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Description: Huawei mtp timer
 * Author: songqiubin
 * Create: 2020-10-27
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#include "mintp_congestion.h"
#include "mintp_output.h"
#include "mintp_rack.h"
#include "mintp_timer.h"

static void mtp_write_err(struct sock *sk)
{
	sk->sk_err = sk->sk_err_soft ? : ETIMEDOUT;
	mtp_info("%u:%u sk_err %d\n", mtp_sk(sk)->src_port, mtp_sk(sk)->dst_port, sk->sk_err);
	sk->sk_error_report(sk);

	mtp_write_queue_purge(sk);
	sk->sk_send_head = NULL;
	mtp_done(sk);
}

static void mtp_retransmit_timer(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev;
	struct sk_buff *skb;

	if (msk->rtx.retransmits >= MTP_RETRY ||
		(sk->sk_state == TCP_SYN_SENT &&
		 msk->rtx.retransmits >= MTP_SYN_RETRY)) {
		mtp_write_err(sk);
		return;
	}

	msk->tlp_high_seq = 0;
	mtp_enter_loss(sk);
	dev = mtp_odev_get(msk);
	if (unlikely(!dev))
		return;

	skb = mtp_write_queue_head(sk);
	skb->dev = dev;
	if (mtp_retransmit_skb(sk, skb) > 0) {
		dev_put(dev);
		/* Retransmission failed because of local congestion,
		 * do not backoff.
		 */
		if (!msk->rtx.retransmits)
			msk->rtx.retransmits = 1;

		msk->rtx.timeout = jiffies + MTP_RTO_MIN;
		msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
		sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
		return;
	}
	dev_put(dev);

	if (sk->sk_state == TCP_ESTABLISHED && mtp_stream_is_thin(msk) &&
	    msk->rtx.retransmits <= MTP_THIN_LINEAR_RETRIES) {
		msk->rtx.backoff = 0;
		msk->rtx.rto = min(mtp_get_rto(msk), MTP_RTO_MAX);
	} else {
		msk->rtx.backoff++;
		msk->rtx.rto = min(msk->rtx.rto << 1, MTP_RTO_MAX);
	}

	msk->rtx.retransmits++;
	msk->rtx.timeout = jiffies + msk->rtx.rto;
	msk->rtx.pend = MTP_RTX_TIMER_RETRANS;
	sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
}

static void mtp_probe_timer(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	__u32 expires;

	if (msk->packets_out || !mtp_send_head(sk)) {
		msk->rtx.probes = 0;
		return;
	}

	if (sock_flag(sk, SOCK_DEAD) || sk->sk_state == TCP_CLOSE ||
	    msk->rtx.probes >= MTP_PROBE_RETRY) {
		mtp_write_err(sk);
		return;
	}

	/* Only send another probe if we didn't close things up. */
	mtp_send_ack(sk, 1);
	msk->rtx.probes++;

	expires = min(msk->rtx.rto << msk->rtx.probes, MTP_RTO_MAX);
	msk->rtx.timeout = jiffies + expires;
	msk->rtx.pend = MTP_RTX_TIMER_PROBE;
	mtp_info("send probe %u rto %u expires %u\n",
		msk->rtx.probes, msk->rtx.rto, expires);
	sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
}

/* Called with BH disabled */
void mtp_write_timer_handler(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	if (((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_LISTEN)) || !msk->rtx.pend)
		return;

	if (time_after(msk->rtx.timeout, jiffies)) {
		sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
		goto out;
	}

	mtp_mstamp_refresh(msk);
	switch (msk->rtx.pend) {
	case MTP_RTX_TIMER_RETRANS:
		mtp_retransmit_timer(sk);
		break;
	case MTP_RTX_TIMER_LOSS_PROBE:
		mtp_send_loss_probe(sk);
		break;
	case MTP_RTX_TIMER_REO_TIMEOUT:
		mtp_rack_reo_timeout(sk);
		break;
	case MTP_RTX_TIMER_PROBE:
		mtp_probe_timer(sk);
		break;
	default:
		break;
	}

out:
	sk_mem_reclaim(sk);
}

static void mtp_write_timer(mtp_timer_ptr data)
{
	struct sock *sk = (struct sock *)container_of((struct timer_list *)data, struct mtp_sock, rtx.timer);

	bh_lock_sock(sk);
	if (!sock_owned_by_user(sk)) {
		mtp_write_timer_handler(sk);
	} else {
		/* deleguate our work to mtp_release_cb() */
		if (!test_and_set_bit(MTP_WRITE_TIMER_DEFERRED, &sk->sk_tsq_flags))
			sock_hold(sk);
	}
	bh_unlock_sock(sk);
	sock_put(sk);
}

static void mtp_keepalive_timer(mtp_timer_ptr data)
{
	struct sock *sk = (struct sock *)container_of((struct timer_list *)data, struct mtp_sock, inet.sk.sk_timer);
	struct mtp_sock *msk = mtp_sk(sk);
	u32 elapsed;
	/* Only process if socket is not in use. */
	bh_lock_sock(sk);
	if (sock_owned_by_user(sk)) {
		/* Try again later. */
		sk_reset_timer(sk, &sk->sk_timer, jiffies + 1);
		goto out;
	}

	if (sk->sk_state == TCP_LISTEN) {
		goto out;
	}

	if (sk->sk_state == TCP_FIN_WAIT1 || sk->sk_state == TCP_FIN_WAIT2 || sk->sk_state == TCP_LAST_ACK) {
		mtp_info("%u:%u keepalive expire on state %u\n", msk->src_port, msk->dst_port, sk->sk_state);
		mtp_done(sk);
		goto out;
	}

	if (!sock_flag(sk, SOCK_KEEPOPEN) ||
	    ((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_SYN_SENT)))
		goto out;

	/* It is alive without keepalive */
	if (msk->packets_out || mtp_send_head(sk)) {
		elapsed = msk->keepalive_time;
		goto resched;
	}

	elapsed = MTP_JIFFIERS32 - msk->rcv_tstamp;
	if (elapsed >= msk->keepalive_time) {
		if (msk->keepalive_probes >= MTP_PROBE_RETRY) {
			mtp_info("keepalive_probes %u exceed, reset connection\n", msk->keepalive_probes);
			mtp_send_reset(sk, msk->snd_nxt, msk->rcv_nxt, GFP_ATOMIC);
			mtp_write_err(sk);
			goto out;
		}
		mtp_info("send keepalive_probes %u\n", msk->keepalive_probes);
		mtp_send_ack(sk, 1);
		elapsed = min(MTP_KEEPALIVE_INTVL << msk->keepalive_probes, MTP_RTO_MAX);
		msk->keepalive_probes++;
	} else {
		elapsed = msk->keepalive_time - elapsed;
	}

resched:
	sk_reset_timer(sk, &sk->sk_timer, jiffies + elapsed);
out:
	bh_unlock_sock(sk);
	sock_put(sk);
}

/* Called with BH disabled */
void mtp_delack_timer_handler(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	mtp_debug("%u:%u delay ack send\n", msk->src_port, msk->dst_port);
	sk_mem_reclaim_partial(sk);

	if (time_after(msk->delack.timeout, jiffies)) {
		sk_reset_timer(sk, &msk->delack.timer, msk->delack.timeout);
		return;
	}

	msk->delack.enable = 0;
	if (msk->delack.ack_pend)
		mtp_send_ack(sk, 0);
}

static void mtp_delack_timer(mtp_timer_ptr data)
{
	struct sock *sk = (struct sock *)container_of((struct timer_list *)data, struct mtp_sock, delack.timer);

	bh_lock_sock(sk);
	if (!sock_owned_by_user(sk)) {
		mtp_delack_timer_handler(sk);
	} else {
		struct mtp_sock *msk = mtp_sk(sk);
		msk->delack.quick = 1;
		/* deleguate our work to mtp_release_cb() */
		if (!test_and_set_bit(MTP_DELACK_TIMER_DEFERRED, &sk->sk_tsq_flags))
			sock_hold(sk);
	}
	bh_unlock_sock(sk);
	sock_put(sk);
}

void mtp_init_xmit_timers(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	mtp_setup_timer(&msk->delack.timer, mtp_delack_timer);
	msk->delack.ack_pend = 0;
	msk->delack.enable = 0;
	msk->delack.quick = 0;
	msk->delack.ofo_ack = 0;
	msk->delack.compress_ack = 0;
	msk->delack.ofo_rcv_nxt = msk->rcv_nxt;

	mtp_setup_timer(&msk->rtx.timer, mtp_write_timer);
	msk->rtx.rto = MTP_TIMEOUT_INIT;
	msk->rtx.pend = 0;
	msk->rtx.retransmits = 0;
	msk->rtx.backoff = 0;

	msk->keepalive_time = MTP_DEFAULT_KEEPALIVE_TIME;
	mtp_setup_timer(&sk->sk_timer, mtp_keepalive_timer);
}

void mtp_clear_xmit_timers(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	sk_stop_timer(sk, &msk->delack.timer);
	sk_stop_timer(sk, &msk->rtx.timer);
	sk_stop_timer(sk, &sk->sk_timer);
}

void mtp_set_keepalive(struct sock *sk, int val)
{
	if ((1 << sk->sk_state) & (TCPF_CLOSE | TCPF_LISTEN))
		return;

	if (val && !sock_flag(sk, SOCK_KEEPOPEN))
		sk_reset_timer(sk, &sk->sk_timer, jiffies + mtp_sk(sk)->keepalive_time);
	else if (!val)
		sk_stop_timer(sk, &sk->sk_timer);
}
