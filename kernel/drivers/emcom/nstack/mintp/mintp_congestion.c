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
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#include "mintp_input.h"
#include "mintp_congestion.h"

#define MTP_INFINITE_SSTHRESH	0x7fffffff

static u32 mtp_reno_ssthresh(struct sock *sk);
static u32 mtp_reno_undo_cwnd(struct sock *sk);

static void mtp_debug_undo(struct sock *sk, const char *msg)
{
	struct mtp_sock *msk = mtp_sk(sk);

	mtp_info("%u:%u Undo %s c%u l%u ss%u/%u p%u\n",
		 msk->src_port, msk->dst_port, msg,
		 msk->snd_cwnd, mtp_left_out(msk),
		 msk->snd_ssthresh, msk->prior_ssthresh,
		 msk->packets_out);
}

/* The cwnd reduction in CWR and Recovery uses the PRR algorithm in RFC 6937.
 * It computes the number of packets to send (sndcnt) based on packets newly
 * delivered:
 *   1) If the packets in flight is larger than ssthresh, PRR spreads the
 *	cwnd reductions across a full RTT.
 *   2) Otherwise PRR uses packet conservation to send as much as delivered.
 *      But when the retransmits are acked without further losses, PRR
 *      slow starts cwnd up to ssthresh to speed up the recovery.
 */
void mtp_init_cwnd_reduction(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	msk->high_seq = msk->snd_nxt;
	msk->tlp_high_seq = 0;
	msk->snd_cwnd_cnt = 0;
	msk->prior_cwnd = msk->snd_cwnd;
	msk->prr_delivered = 0;
	msk->prr_out = 0;
	msk->snd_ssthresh = mtp_reno_ssthresh(sk);
	msk->stats.cwnd_reduction++;
	mtp_info("%u:%u snd_cwnd %u snd_ssthresh %u\n", msk->src_port, msk->dst_port,
		msk->snd_cwnd, msk->snd_ssthresh);
}

void mtp_cwnd_reduction(struct sock *sk, int newly_acked_sacked, u32 flag)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int sndcnt = 0;
	int delta = msk->snd_ssthresh - mtp_packets_in_flight(msk);

	if (newly_acked_sacked <= 0 || WARN_ON_ONCE(!msk->prior_cwnd))
		return;

	msk->prr_delivered += newly_acked_sacked;
	if (delta < 0) {
		u64 dividend = (u64)msk->snd_ssthresh * msk->prr_delivered +
			       msk->prior_cwnd - 1;
		sndcnt = div_u64(dividend, msk->prior_cwnd) - msk->prr_out;
	} else if ((flag & MTP_ACK_FLAG_RETRANS_DATA_ACKED) &&
		   !(flag & MTP_ACK_FLAG_LOST_RETRANS)) {
		sndcnt = min_t(int, delta,
			       max_t(int, msk->prr_delivered - msk->prr_out,
				     newly_acked_sacked) + 1);
	} else {
		sndcnt = min(delta, newly_acked_sacked);
	}
	/* Force a fast retransmit upon entering fast recovery */
	sndcnt = max(sndcnt, (msk->prr_out ? 0 : 1));
	msk->snd_cwnd = mtp_packets_in_flight(msk) + sndcnt;
	mtp_debug("%u:%u snd_cwnd %u snd_ssthresh %u prr_delivered %u prr_out %u\n",
		msk->src_port, msk->dst_port, msk->snd_cwnd, msk->snd_ssthresh,
		msk->prr_delivered, msk->prr_out);
}

static inline void mtp_ssthresh_undo(struct mtp_sock *msk)
{
	/* Hi1106 are easily lost in slow start, should Keep Slow Start in MTP_HI1106_WAKE_PEROID */
	if (msk->is_hi1106 &&
	    (mtp_stamp_us_delta(msk->cur_mstamp, msk->slow_start_mstamp) < MTP_HI1106_WAKE_PEROID))
		msk->snd_ssthresh = msk->prior_ssthresh;
	mtp_debug("%u:%u snd_cwnd %u snd_ssthresh %u\n", msk->src_port, msk->dst_port,
		msk->snd_cwnd, msk->snd_ssthresh);
}

void mtp_end_cwnd_reduction(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	/* Reset cwnd to ssthresh in CWR or Recovery (unless it's undone) */
	if (msk->snd_ssthresh < MTP_INFINITE_SSTHRESH &&
	    (msk->ca_state == MTP_CA_CWR || msk->undo_marker))
		msk->snd_cwnd = msk->snd_ssthresh;
	mtp_ssthresh_undo(msk);
}

static void mtp_undo_cwnd_reduction(struct sock *sk, bool unmark_loss)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (unmark_loss) {
		struct sk_buff *skb;

		mtp_for_write_queue(skb, sk) {
			if (skb == mtp_send_head(sk))
				break;
			MTP_SKB_CB(skb)->sacked &= ~MTPCB_LOST;
		}
		msk->lost_out = 0;
		msk->lost_skb_hint = NULL;
		msk->retransmit_skb_hint = NULL;
	}

	if (msk->prior_ssthresh) {
		msk->snd_cwnd = mtp_reno_undo_cwnd(sk);

		if (msk->prior_ssthresh > msk->snd_ssthresh)
			msk->snd_ssthresh = msk->prior_ssthresh;
	}
	msk->stats.cwnd_undo_reduction++;
	msk->undo_marker = 0;
}

static inline bool mtp_may_undo(const struct mtp_sock *msk)
{
	return msk->undo_marker && !msk->ca_loss;
}

bool mtp_try_undo_recovery(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (mtp_may_undo(msk)) {
		/* Happy end! We did not retransmit anything
		 * or our original transmission succeeded.
		 */
		mtp_debug_undo(sk, msk->ca_state == MTP_CA_LOSS ? "loss" : "retrans");
		mtp_undo_cwnd_reduction(sk, false);
	}

	mtp_ssthresh_undo(msk);
	mtp_set_ca_state(sk, MTP_CA_OPEN);
	msk->ca_loss = 0;
	return false;
}

static bool mtp_any_retrans_done(const struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;

	if (msk->retrans_out)
		return true;

	skb = mtp_write_queue_head(sk);
	if (unlikely(skb && (MTP_SKB_CB(skb)->sacked & MTPCB_EVER_RETRANS)))
		return true;

	return false;
}

void mtp_try_keep_open(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int state = MTP_CA_OPEN;

	if (mtp_left_out(msk) || mtp_any_retrans_done(sk))
		state = MTP_CA_DISORDER;

	if (msk->ca_state != state) {
		mtp_set_ca_state(sk, state);
		msk->high_seq = msk->snd_nxt;
		msk->ca_loss = 0;
	}
}

void mtp_try_to_open(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	WARN_ON(mtp_left_out(msk) > msk->packets_out);

	if (msk->ca_state != MTP_CA_CWR)
		mtp_try_keep_open(sk);
}

void mtp_enter_recovery(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	msk->prior_ssthresh = 0;
	mtp_init_undo(msk);

	if (!mtp_in_cwnd_reduction(sk)) {
		msk->prior_ssthresh = mtp_current_ssthresh(sk);
		mtp_init_cwnd_reduction(sk);
	}
	mtp_info("set prior_ssthresh to %u cwnd %u in_acks %u out_msq_limit %u\n",
		msk->prior_ssthresh, msk->snd_cwnd, msk->stats.in_acks, msk->stats.out_msq_limit);
	mtp_set_ca_state(sk, MTP_CA_RECOVERY);
}

void mtp_enter_loss(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	bool mark_lost;
	struct sk_buff *skb;
	bool new_recovery = msk->ca_state < MTP_CA_RECOVERY;

	msk->retrans_out = 0;
	msk->lost_out = 0;

	/* Reduce ssthresh if it has not yet been made inside this window. */
	if (msk->ca_state <= MTP_CA_DISORDER ||
	    !after(msk->high_seq, msk->snd_una) ||
	    (msk->ca_state == MTP_CA_LOSS && !msk->rtx.retransmits)) {
		msk->prior_ssthresh = mtp_current_ssthresh(sk);
		msk->prior_cwnd = msk->snd_cwnd;
		msk->snd_ssthresh = mtp_reno_ssthresh(sk);
		mtp_init_undo(msk);
	}
	msk->snd_cwnd = mtp_packets_in_flight(msk) + 1;
	msk->snd_cwnd_cnt = 0;
	mtp_set_ca_state(sk, MTP_CA_LOSS);
	msk->high_seq = msk->snd_nxt;
	msk->lost_skb_hint = NULL;
	msk->retransmit_skb_hint = NULL;
	mtp_for_write_queue(skb, sk) {
		if (skb == mtp_send_head(sk))
			break;

		mark_lost = !(MTP_SKB_CB(skb)->sacked & MTPCB_SACKED_ACKED);
		if (mark_lost) {
			MTP_SKB_CB(skb)->sacked &= ~(MTPCB_SACKED_ACKED | MTPCB_SACKED_RETRANS);
			MTP_SKB_CB(skb)->sacked |= MTPCB_LOST;
			msk->lost_out += 1;
			mtp_debug("mark %u~%u lost\n", MTP_SKB_CB(skb)->seq, MTP_SKB_CB(skb)->end_seq);
		}
	}
	WARN_ON(mtp_left_out(msk) > msk->packets_out);
	/* Timeout in disordered state after receiving substantial DUPACKs
	 * suggests that the degree of reordering is over-estimated.
	 */
	if (msk->ca_state <= MTP_CA_DISORDER &&
	    msk->sacked_out >= MTP_FASTRETRANS_THRESH)
		msk->reordering = MTP_FASTRETRANS_THRESH;

	/* F-RTO RFC5682 sec 3.1 step 1: retransmit SND.UNA if no previous
	 * loss recovery is underway except recurring timeout(s) on
	 * the same SND.UNA (sec 3.2).
	 */
	msk->rtx.frto = new_recovery || msk->rtx.retransmits;
}

void mtp_update_reordering(struct sock *sk, const int metric)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (metric < 0) {
		mtp_err("%u:%u metric %d\n", metric);
		return;
	}

	if (metric > msk->reordering) {
		msk->reordering = min(MTP_REORDERING_MAX, metric);

		mtp_info("%u:%u Disorder %s %u f%u s%u undo%u\n",
			 msk->src_port, msk->dst_port,
			 ca_state_str[msk->ca_state],
			 msk->reordering,
			 msk->fackets_out,
			 msk->sacked_out,
			 msk->undo_marker);
	}

	msk->rack.reord = 1;
}

/* Undo during fast recovery after partial ACK. */
bool mtp_try_undo_partial(struct sock *sk, int acked)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (msk->undo_marker && !msk->ca_loss) {
		/* Plain luck! Hole if filled with delayed
		 * packet, rather than with a retransmit.
		 */
		mtp_update_reordering(sk, msk->fackets_out + acked);

		/* We are getting evidence that the reordering degree is higher
		 * than we realized. If there are no retransmits out then we
		 * can undo. Otherwise we clock out new packets but do not
		 * mark more packets lost or retransmit more.
		 */
		if (msk->retrans_out)
			return true;

		mtp_debug_undo(sk, "partial recovery");
		mtp_undo_cwnd_reduction(sk, true);
		mtp_try_keep_open(sk);
		return true;
	}
	return false;
}

/* Undo during loss recovery after partial ACK or using F-RTO. */
bool mtp_try_undo_loss(struct sock *sk, bool frto_undo)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (frto_undo || mtp_may_undo(msk)) {
		mtp_undo_cwnd_reduction(sk, true);

		mtp_debug_undo(sk, "partial loss");
		msk->rtx.retransmits = 0;
		mtp_set_ca_state(sk, MTP_CA_OPEN);
		msk->ca_loss = 0;
		return true;
	}
	return false;
}

/* Slow start is used when congestion window is no greater than the slow start
 * threshold. We base on RFC2581 and also handle stretch ACKs properly.
 * We do not implement RFC3465 Appropriate Byte Counting (ABC) per se but
 * something better;) a packet is only considered (s)acked in its entirety to
 * defend the ACK attacks described in the RFC. Slow start processes a stretch
 * ACK of degree N as if N acks of degree 1 are received back to back except
 * ABC caps N to 2. Slow start exits when cwnd grows over ssthresh and
 * returns the leftover acks to adjust cwnd in congestion avoidance mode.
 */
static u32 mtp_slow_start(struct mtp_sock *msk, u32 acked)
{
	u32 cwnd = min(msk->snd_cwnd + acked, msk->snd_ssthresh);

	acked -= cwnd - msk->snd_cwnd;
	msk->snd_cwnd = min(cwnd, MTP_SND_CWND_CLAMP);

	return acked;
}

#define MTP_CONG_AVOID_RATIO 3

/* This is tp->snd_cwnd += 0.125 * tp->snd_cwnd (or alternative w),
 * for every send window that was ACKed.
 */
static void mtp_cong_avoid_ai(struct mtp_sock *msk, u32 w, u32 acked)
{
	u32 step;
	u32 ratio = msk->cong_cfg.cong_avoid_ratio ? : MTP_CONG_AVOID_RATIO;

	/* If credits accumulated at a higher w, apply them gently now. */
	if (msk->snd_cwnd_cnt >= w) {
		msk->snd_cwnd_cnt = 0;
		step = msk->snd_cwnd >> ratio;
		msk->snd_cwnd += step;
	}

	msk->snd_cwnd_cnt += acked;
	if (msk->snd_cwnd_cnt >= w) {
		u32 delta = msk->snd_cwnd_cnt / w;

		msk->snd_cwnd_cnt -= delta * w;
		step = msk->snd_cwnd >> ratio;
		msk->snd_cwnd += delta * step;
	}
	msk->snd_cwnd = min(msk->snd_cwnd, MTP_SND_CWND_CLAMP);
}

/* This is Jacobson's slow start and congestion avoidance.
 * SIGCOMM '88, p. 328.
 */
static void mtp_reno_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (!mtp_is_cwnd_limited(sk))
		return;

	/* In "safe" area, increase. */
	if (mtp_in_slow_start(msk)) {
		acked = mtp_slow_start(msk, acked);
		if (!acked)
			return;
	}
	/* In dangerous area, increase slowly. */
	mtp_cong_avoid_ai(msk, msk->snd_cwnd, acked);
}

static u32 mtp_reno_undo_cwnd(struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);
	return max(msk->snd_cwnd, msk->prior_cwnd);
}

static u32 mtp_reno_ssthresh(struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);

	return max((msk->snd_cwnd * 717U) / 1024U, 2U);
}

/* Decide wheather to run the increase function of congestion control. */
static inline bool mtp_may_raise_cwnd(const struct sock *sk, u32 flag)
{
	const struct mtp_sock *msk = mtp_sk(sk);
	/* If reordering is high then always grow cwnd whenever data is
	 * delivered regardless of its ordering. Otherwise stay conservative
	 * and only grow cwnd on in-order delivery (RFC5681). A stretched ACK w/
	 * new SACK or ECE mark may first advance cwnd here and later reduce
	 * cwnd in mtp_fastretrans_alert() based on more states.
	 */
	if (msk->reordering > MTP_FASTRETRANS_THRESH)
		return flag & MTP_ACK_FLAG_FORWARD_PROGRESS;

	return flag & MTP_ACK_FLAG_DATA_ACKED;
}

/* Set the sk_pacing_rate to allow proper sizing of TSO packets.
 * Note: TCP stack does not yet implement pacing.
 * FQ packet scheduler can be used to implement cheap but effective
 * TCP pacing, to smooth the burst on large writes when packets
 * in flight is significantly lower than cwnd (or rwin)
 */
int sysctl_mtp_pacing_ss_ratio __read_mostly = 200;
int sysctl_mtp_pacing_ca_ratio __read_mostly = 120;

static void mtp_update_pacing_rate(struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);
	u64 rate;

	/* set sk_pacing_rate to 200 % of current rate (mss * cwnd / srtt)
	 * left shifting 3 for srtt_us is 8x of the real value, divide 100 to change
	 * it to the percent value.
	 */
	rate = (u64)msk->mss * ((USEC_PER_SEC / 100) << 3);

	/* current rate is (cwnd * mss) / srtt
	 * In Slow Start [1], set sk_pacing_rate to 200 % the current rate.
	 * In Congestion Avoidance phase, set it to 120 % the current rate.
	 *
	 * [1] : Normal Slow Start condition is (msk->snd_cwnd < msk->snd_ssthresh)
	 *	 If snd_cwnd >= (msk->snd_ssthresh / 2), we are approaching
	 *	 end of slow start and should slow down.
	 */
	if (msk->snd_cwnd < msk->snd_ssthresh / 2)
		rate *= sysctl_mtp_pacing_ss_ratio;
	else
		rate *= sysctl_mtp_pacing_ca_ratio;

	rate *= max(msk->snd_cwnd, msk->packets_out);

	if (likely(msk->srtt_us))
		do_div(rate, msk->srtt_us);

	mtp_debug("%u:%u sk_pacing_rate %u\n", msk->src_port, msk->dst_port, rate);
	/* ACCESS_ONCE() is needed because sch_fq fetches sk_pacing_rate
	 * without any lock. We want to make sure compiler wont store
	 * intermediate values in this location.
	 */
	WRITE_ONCE(sk->sk_pacing_rate, rate);
}

void mtp_cong_control(struct sock *sk, u32 ack, u32 acked_sacked, u32 flag)
{
	if (mtp_in_cwnd_reduction(sk)) {
		/* Reduce cwnd if state mandates */
		mtp_cwnd_reduction(sk, acked_sacked, flag);
	} else if (mtp_may_raise_cwnd(sk, flag)) {
		/* Advance cwnd if state allows */
		mtp_reno_cong_avoid(sk, ack, acked_sacked);
	}
	mtp_update_pacing_rate(sk);
}

void mtp_init_congestion_control(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	msk->ca_state = MTP_CA_OPEN;
	msk->ca_recovery = 0;
	msk->ca_loss = 0;
	msk->ca_cwnd_limited = 0;

	msk->snd_cwnd = msk->cong_cfg.init_cwnd ? :
		(msk->is_hi1106 ? MTP_INIT_CWND_HI1106 : MTP_INIT_CWND);
	mtp_info("snd_cwnd init %u\n", msk->snd_cwnd);
	msk->snd_cwnd_cnt = 0;
	msk->prior_ssthresh = 0;
	msk->snd_ssthresh = MTP_INFINITE_SSTHRESH;
	msk->prior_cwnd = msk->snd_cwnd;
	msk->undo_marker = 0;
	msk->max_packets_out = 0;
	msk->max_packets_seq = msk->snd_nxt;
	msk->prr_delivered = 0;
	msk->prr_out = 0;
	msk->delivered = 0;
	msk->reordering = MTP_FASTRETRANS_THRESH;
}

