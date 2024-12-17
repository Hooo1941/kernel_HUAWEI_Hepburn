/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 *
 */

#include "mintp.h"
#include "mintp_congestion.h"
#include "mintp_output.h"
#include "mintp_timer.h"
#include "mintp_rack.h"

#define DEF_REO_WND 1000

static void mtp_rack_mark_skb_lost(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);

	mtp_verify_retransmit_hint(msk, skb);

	if (!(MTP_SKB_CB(skb)->sacked & (MTPCB_LOST | MTPCB_SACKED_ACKED))) {
		msk->lost_out += 1;
		MTP_SKB_CB(skb)->sacked |= MTPCB_LOST;
		mtp_debug("%u:%u mark %u~%u lost, lost_out %d\n", msk->src_port, msk->dst_port,
			MTP_SKB_CB(skb)->seq, MTP_SKB_CB(skb)->end_seq, msk->lost_out);
	}

	if (MTP_SKB_CB(skb)->sacked & MTPCB_SACKED_RETRANS) {
		/* Account for retransmits that are lost again */
		MTP_SKB_CB(skb)->sacked &= ~MTPCB_SACKED_RETRANS;
		msk->retrans_out -= 1;
	}
}

static bool mtp_rack_sent_after(u64 t1, u64 t2, u32 seq1, u32 seq2)
{
	return t1 > t2 || (t1 == t2 && after(seq1, seq2));
}

static void mtp_rack_detect_loss(struct sock *sk, u32 *reo_timeout)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	u32 reo_wnd;
	u32 origin_lost_out = msk->lost_out;

	*reo_timeout = 0;
	reo_wnd = DEF_REO_WND;
	if ((msk->rack.reord || !msk->lost_out) && mtp_min_rtt(msk) != ~0U)
		reo_wnd = max(mtp_min_rtt(msk) >> 2, reo_wnd); /* divide by 4 */

	mtp_for_write_queue(skb, sk) {
		struct mtp_skb_cb *scb = MTP_SKB_CB(skb);

		if (skb == mtp_send_head(sk))
			break;

		/* Skip ones already (s)acked */
		if (!after(scb->end_seq, msk->snd_una) ||
		    (scb->sacked & MTPCB_SACKED_ACKED))
			continue;

		if (mtp_rack_sent_after(msk->rack.mstamp, mtp_skb_timestamp_us(skb),
					msk->rack.end_seq, scb->end_seq)) {
			/* Step 3 in draft-cheng-tcpm-rack-00.txt:
			 * A packet is lost if its elapsed time is beyond
			 * the recent RTT plus the reordering window.
			 */
			u32 elapsed = mtp_stamp_us_delta(msk->cur_mstamp,
							 mtp_skb_timestamp_us(skb));
			s32 remaining = msk->rack.rtt_us + reo_wnd - elapsed;

			if (remaining < 0) {
				mtp_rack_mark_skb_lost(sk, skb);
				continue;
			}

			/* Skip ones marked lost but not yet retransmitted */
			if ((scb->sacked & MTPCB_LOST) &&
			    !(scb->sacked & MTPCB_SACKED_RETRANS))
				continue;

			/* Record maximum wait time (+1 to avoid 0) */
			*reo_timeout = max_t(u32, *reo_timeout, 1 + remaining);
		} else if (!(scb->sacked & MTPCB_RETRANS)) {
			/* Original data are sent sequentially so stop early
			 * b/c the rest are all sent after rack_sent
			 */
			break;
		}
	}
	if (msk->lost_out > origin_lost_out)
		mtp_info("%u:%u origin lost_out %u now %u\n", msk->src_port, msk->dst_port,
			origin_lost_out, msk->lost_out);
}

static void mtp_rack_mark_lost(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 timeout;

	if (!msk->rack.advanced)
		return;

	/* Reset the advanced flag to avoid unnecessary queue scanning */
	msk->rack.advanced = 0;
	mtp_rack_detect_loss(sk, &timeout);
	if (timeout) {
		timeout = usecs_to_jiffies(timeout);
		if (timeout < MTP_TIMEOUT_MIN)
			timeout = MTP_TIMEOUT_MIN;
		else if (timeout > msk->rtx.rto)
			timeout = msk->rtx.rto;

		msk->rtx.pend = MTP_RTX_TIMER_REO_TIMEOUT;
		msk->rtx.timeout = jiffies + timeout;
		mtp_debug("%u:%u rack timeout %u\n", msk->src_port, msk->dst_port, timeout);
		sk_reset_timer(sk, &msk->rtx.timer, msk->rtx.timeout);
	}
}

int mtp_rack_identify_loss(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	/* Use RACK to detect loss */
	u32 prior_retrans = msk->retrans_out;

	mtp_rack_mark_lost(sk);
	if (prior_retrans > msk->retrans_out)
		return 1;
	return 0;
}

/* Record the most recently (re)sent time among the (s)acked packets
 * This is "6.2 Step 2: Update the state for the most recently sent segment that has
 * been delivered" from rfc8985
 */
void mtp_rack_advance(struct mtp_sock *msk, u8 sacked, u32 end_seq,
		      u64 xmit_time)
{
	u32 rtt_us;

	if (msk->rack.mstamp &&
	    !mtp_rack_sent_after(xmit_time, msk->rack.mstamp,
				 end_seq, msk->rack.end_seq))
		return;

	rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, xmit_time);
	if (sacked & MTPCB_RETRANS) {
		if (rtt_us < mtp_min_rtt(msk))
			return;
	}
	msk->rack.rtt_us = rtt_us;
	msk->rack.mstamp = xmit_time;
	msk->rack.end_seq = end_seq;
	msk->rack.advanced = 1;
}

/* We have waited long enough to accommodate reordering. Mark the expired
 * packets lost and retransmit them.
 */
void mtp_rack_reo_timeout(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 timeout, prior_inflight;

	prior_inflight = mtp_packets_in_flight(msk);
	mtp_rack_detect_loss(sk, &timeout);
	if (prior_inflight != mtp_packets_in_flight(msk)) {
		if (msk->ca_state != MTP_CA_RECOVERY) {
			mtp_enter_recovery(sk);
			mtp_cwnd_reduction(sk, 1, 0);
		}
		mtp_xmit_retransmit_queue(sk);
	}
	if (msk->rtx.pend != MTP_RTX_TIMER_RETRANS)
		mtp_rearm_rto(sk);
}

void mtp_verify_retransmit_hint(struct mtp_sock *msk, struct sk_buff *skb)
{
	if ((!msk->retransmit_skb_hint && msk->retrans_out >= msk->lost_out) ||
	    (msk->retransmit_skb_hint &&
	     before(MTP_SKB_CB(skb)->seq,
		    MTP_SKB_CB(msk->retransmit_skb_hint)->seq)))
		msk->retransmit_skb_hint = skb;
}