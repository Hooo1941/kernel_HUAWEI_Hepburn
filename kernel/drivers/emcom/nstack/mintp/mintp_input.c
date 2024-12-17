/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * License terms: GNU General Public License (GPL)
 * Author: songqiubin
 *
 * MinTP layer2 Reliable Transmission Protocol.
 * Part of the code refers to the Linux tcpip stack
 */

#include "mintp_congestion.h"
#include "mintp_output.h"
#include "mintp_conn.h"
#include "mintp_rack.h"
#include "mintp_input.h"

static void mtp_fin(struct sock *sk);

#define MTP_BASE_OPTION_LEN 2
#define MTP_MAX_SKB_TRUESIZE (64 * 1024)

static void mtp_parse_options(const struct sk_buff *skb, const struct mtphdr *mtph)
{
	const unsigned char *ptr;
	struct mtp_skb_cb *cb = MTP_SKB_CB(skb);
	int length = cb->hdlen - sizeof(struct mtphdr);
	int opcode;
	int opsize;

	ptr = (const unsigned char *)(mtph + 1);

	while (length) {
		opcode = *ptr++;

		if (opcode == MTPOPT_NOP) {
			length--;
			continue;
		}

		opsize = *ptr++;
		if (opsize < MTP_BASE_OPTION_LEN)
			return;
		if (opsize > length)
			return;

		switch (opcode) {
		case MTPOPT_MSS:
			if (opsize == MTPOLEN_MSS && cb->type == MTPHDR_TYPE_HANDSHAKE)
				cb->mss = get_unaligned_be16(ptr);
			break;
		case MTPOPT_FLAGS:
			if (opsize == MTPOLEN_FLAGS)
				cb->opt_flags = get_unaligned_be16(ptr);
			break;
		case MTPOPT_MSG_SIZE:
			if (opsize == MTPOLEN_MSG_SIZE && cb->type == MTPHDR_TYPE_HANDSHAKE)
				cb->max_msg_size = get_unaligned_be16(ptr);
			break;
		case MTPOPT_SACK:
			if ((opsize >= (MTPOLEN_SACK_BASE + MTPOLEN_SACK_PERBLOCK)) &&
			   !((opsize - MTPOLEN_SACK_BASE) % MTPOLEN_SACK_PERBLOCK))
				cb->sacked = (ptr - MTP_BASE_OPTION_LEN) - (unsigned char *)mtph;
			break;
		default:
			mtp_warn("unsupport option %u\n", opcode);
			break;
		}
		ptr += opsize - MTP_BASE_OPTION_LEN;
		length -= opsize;
	}
}

static void mtp_rcv_fill_cb(struct sk_buff *skb, const struct mtphdr *mtph)
{
	struct mtp_skb_cb *cb = MTP_SKB_CB(skb);
	cb->seq = ntohl(mtph->seq);
	cb->end_seq = cb->seq + ntohs(mtph->len);
	cb->ack_seq = ntohl(mtph->ack_seq);
	cb->win = ntohs(mtph->win);
	cb->flags = MTP_FLAG_BYTE(mtph);
	cb->type = mtph->type;
	if (unlikely(cb->type == MTPHDR_TYPE_SYN || cb->type == MTPHDR_TYPE_HANDSHAKE ||
	    cb->type == MTPHDR_TYPE_FIN))
		cb->end_seq++;
	cb->hdlen = MTP_HL_GET(mtph);
	cb->sacked = 0;
	cb->opt_flags = 0;
	cb->max_msg_size = 0;
	mtp_parse_options(skb, mtph);
}

#define REXMIT_NONE	0 /* no loss recovery to do */
#define REXMIT_LOST	1 /* retransmit packets marked lost */
#define REXMIT_NEW	2 /* FRTO-style transmit of unsent/new packets */

struct mtp_sacktag_state {
	int	reord;
	int	fack_count;
	u64	first_sackt;
	u64	last_sackt;
	u32	flag;
};

static void mtp_rtt_estimator(struct sock *sk, long mrtt_us)
{
	struct mtp_sock *msk = mtp_sk(sk);
	long m = mrtt_us;
	u32 srtt = msk->srtt_us;

	if (srtt != 0) {
		m -= (srtt >> MTP_SRTT_WEIGHT);
		srtt += m;
		if (m < 0) {
			m = -m;
			m -= (msk->mdev_us >> 2); /* Standard RTT algorithm, mdev_us divided by 4 */
			if (m > 0)
				m >>= MTP_SRTT_WEIGHT;
		} else {
			m -= (msk->mdev_us >> 2); /* Standard RTT algorithm, mdev_us divided by 4 */
		}
		msk->mdev_us += m;
		if (msk->mdev_us > msk->mdev_max_us) {
			msk->mdev_max_us = msk->mdev_us;
			if (msk->mdev_max_us > msk->rttvar_us)
				msk->rttvar_us = msk->mdev_max_us;
		}
		if (after(msk->snd_una, msk->rtt_seq)) {
			if (msk->mdev_max_us < msk->rttvar_us)
				msk->rttvar_us -= (msk->rttvar_us - msk->mdev_max_us) >> 2;
			msk->rtt_seq = msk->snd_nxt;
			msk->mdev_max_us = MTP_RTO_MIN_US;
		}
	} else {
		srtt = m << MTP_SRTT_WEIGHT;
		msk->mdev_us = m << 1;
		msk->rttvar_us = max(msk->mdev_us, MTP_RTO_MIN_US);
		msk->mdev_max_us = msk->rttvar_us;
		msk->rtt_seq = msk->snd_nxt;
	}
	msk->srtt_us = max(1U, srtt);
	mtp_debug("%u:%u skb rtt_seq %u get srtt_us %u mdev_us %u snd_una %u\n",
		msk->src_port, msk->dst_port,
		msk->rtt_seq, msk->srtt_us, msk->mdev_us, msk->snd_una);
}

static inline void mtp_bound_rto(const struct sock *sk)
{
	if (mtp_sk(sk)->rtx.rto > MTP_RTO_MAX)
		mtp_sk(sk)->rtx.rto = MTP_RTO_MAX;
}

static void mtp_set_rto(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	msk->rtx.rto = mtp_get_rto(msk);
	mtp_bound_rto(sk);
	mtp_debug("%u:%u rto %u\n", msk->src_port, msk->dst_port, msk->rtx.rto);
}

static void mtp_update_rtt_min(struct sock *sk, u32 rtt_us)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 wlen = MTP_RTT_RECORD_MAX * HZ;

	minmax_running_min(&msk->rtt_min, wlen, MTP_JIFFIERS32,
			   rtt_us ? : jiffies_to_usecs(1));
}

static bool mtp_ack_update_rtt(struct sock *sk, long seq_rtt_us,
			       long sack_rtt_us, long ca_rtt_us)
{
	struct mtp_sock *msk = mtp_sk(sk);

	/* Prefer RTT measured from ACK's timing to TS-ECR. This is because
	 * broken middle-boxes or peers may corrupt TS-ECR fields. But
	 * Karn's algorithm forbids taking RTT if some retransmitted data
	 * is acked (RFC6298).
	 */
	if (seq_rtt_us < 0)
		seq_rtt_us = sack_rtt_us;

	if (seq_rtt_us < 0)
		return false;

	/* ca_rtt_us >= 0 is counting on the invariant that ca_rtt_us is
	 * always taken together with ACK, SACK, or TS-opts. Any negative
	 * values will be skipped with the seq_rtt_us < 0 check above.
	 */
	mtp_update_rtt_min(sk, ca_rtt_us);
	mtp_rtt_estimator(sk, seq_rtt_us);
	mtp_set_rto(sk);
	/* RFC6298: only reset backoff on valid RTT measurement. */
	msk->rtx.retransmits = 0;
	return true;
}

static u32 mtp_write_queue_skb_acked(struct sock *sk, struct sk_buff *skb,
	u64 *first_ackt, u64 *last_ackt)
{
	struct mtp_skb_cb *dcb = MTP_SKB_CB(skb);
	u8 sacked = dcb->sacked;
	u32 flag = 0;
	struct mtp_sock *msk = mtp_sk(sk);

	if (unlikely(sacked & MTPCB_RETRANS)) {
		if (sacked & MTPCB_SACKED_RETRANS)
			msk->retrans_out -= 1;
		flag |= MTP_ACK_FLAG_RETRANS_DATA_ACKED;
	} else if (!(sacked & MTPCB_SACKED_ACKED)) {
		*last_ackt = mtp_skb_timestamp_us(skb);
		WARN_ON_ONCE(*last_ackt == 0);
		if (!(*first_ackt))
			*first_ackt = *last_ackt;
		flag |= MTP_ACK_FLAG_IN_ORDER_ACKED;
		if (!after(dcb->end_seq, msk->high_seq))
			flag |= MTP_ACK_FLAG_ORIG_SACK_ACKED;
	}

	if (sacked & MTPCB_SACKED_ACKED) {
		msk->sacked_out -= 1;
	} else {
		msk->delivered += 1;
		mtp_rack_advance(msk, sacked, dcb->end_seq,
				 mtp_skb_timestamp_us(skb));
	}

	if (sacked & MTPCB_LOST)
		msk->lost_out -= 1;

	mtp_debug("%u:%u skb seq %u acked\n", msk->src_port, msk->dst_port, dcb->seq);
	msk->packets_out--;
	if (unlikely(dcb->type == MTPHDR_TYPE_SYN || dcb->type == MTPHDR_TYPE_HANDSHAKE))
		flag |= MTP_ACK_FLAG_SYN_ACKED;
	else
		flag |= MTP_ACK_FLAG_DATA_ACKED;

	if (unlikely(dcb->type == MTPHDR_TYPE_FIN)) {
		if (sk->sk_state == TCP_LAST_ACK) {
			mtp_unlink_write_queue(skb, sk);
			mtp_wmem_free_skb(sk, skb);
			mtp_done(sk);
			return flag;
		}
		sk_reset_timer(sk, &sk->sk_timer, jiffies + (msk->rtx.rto << 1));
	}

	mtp_unlink_write_queue(skb, sk);
	mtp_wmem_free_skb(sk, skb);
	if (unlikely(skb == msk->retransmit_skb_hint))
		msk->retransmit_skb_hint = NULL;
	if (unlikely(skb == msk->lost_skb_hint))
		msk->lost_skb_hint = NULL;
	return flag;
}

static long mtp_rtt_update(struct sock *sk, u32 flag, u64 first_ackt,
	u64 last_ackt, struct mtp_sacktag_state *sack)
{
	struct mtp_sock *msk = mtp_sk(sk);
	long sack_rtt_us = -1L;
	long seq_rtt_us = -1L;
	long ca_rtt_us = -1L;

	if (likely(first_ackt) && !(flag & MTP_ACK_FLAG_RETRANS_DATA_ACKED)) {
		seq_rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, first_ackt);
		ca_rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, last_ackt);

		if (seq_rtt_us < 0)
			return -1;
		mtp_update_rtt_min(sk, ca_rtt_us);
	}

	if (sack->first_sackt) {
		sack_rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, sack->first_sackt);
		ca_rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, sack->last_sackt);
	}

	if (mtp_ack_update_rtt(sk, seq_rtt_us, sack_rtt_us, ca_rtt_us))
		return sack_rtt_us;
	return -1;
}

static u32 mtp_write_queue_acked(struct sock *sk, int prior_fackets,
	u32 prior_snd_una, int *acked, struct mtp_sacktag_state *sack)
{
	struct sk_buff *skb;
	u32 flag = 0;
	struct mtp_sock *msk = mtp_sk(sk);
	u32 reord = msk->packets_out;
	u64 first_ackt, last_ackt;
	long sack_rtt_us;

	first_ackt = 0;
	while ((skb = mtp_write_queue_head(sk)) && skb != mtp_send_head(sk)) {
		if (after(MTP_SKB_CB(skb)->end_seq, msk->snd_una))
			break;

		flag |= mtp_write_queue_skb_acked(sk, skb, &first_ackt, &last_ackt);
		if (flag & MTP_ACK_FLAG_IN_ORDER_ACKED)
			reord = min((u32)(*acked), reord);
		*acked += 1;
	}

	sack_rtt_us = mtp_rtt_update(sk, flag, first_ackt, last_ackt, sack);
	if (flag & MTP_ACK_FLAG_ACKED) {
		flag |= MTP_ACK_FLAG_SET_XMIT_TIMER;  /* set TLP or RTO timer */

		/* Non-retransmitted hole got filled? That's reordering */
		if (reord < prior_fackets && reord <= msk->fackets_out)
			mtp_update_reordering(sk, msk->fackets_out - reord);
		msk->lost_cnt_hint -= min(msk->lost_cnt_hint, *acked);
		msk->fackets_out -= min((u32)(*acked), msk->fackets_out);
	} else if (skb && sack_rtt_us >= 0 &&
		   sack_rtt_us > mtp_stamp_us_delta(msk->cur_mstamp, mtp_skb_timestamp_us(skb))) {
		/* Do not re-arm RTO if the sack RTT is measured from data sent
		 * after when the head was last (re)transmitted. Otherwise the
		 * timeout may continue to extend in loss recovery.
		 */
		flag |= MTP_ACK_FLAG_SET_XMIT_TIMER;  /* set TLP or RTO timer */
	}

	return flag;
}

static int mtp_match_skb_to_sack(struct sock *sk, struct sk_buff *skb,
				  u32 start_seq, u32 end_seq)
{
	bool in_sack;

	in_sack = !after(start_seq, MTP_SKB_CB(skb)->seq) &&
		  !before(end_seq, MTP_SKB_CB(skb)->end_seq);
	return in_sack;
}

/* Mark the given newly-SACKed range as such, adjusting counters and hints. */
static u8 mtp_sacktag_one(struct sock *sk,
			  struct mtp_sacktag_state *state, u8 sacked,
			  u32 start_seq, u32 end_seq,
			  u64 xmit_time)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int fack_count = state->fack_count;

	/* Nothing to do; acked frame is about to be dropped (was ACKed). */
	if (!after(end_seq, msk->snd_una))
		return sacked;

	if (!(sacked & MTPCB_SACKED_ACKED)) {
		mtp_rack_advance(msk, sacked, end_seq, xmit_time);

		if (sacked & MTPCB_SACKED_RETRANS) {
			/* If the segment is not tagged as lost,
			 * we do not clear RETRANS, believing
			 * that retransmission is still in flight.
			 */
			if (sacked & MTPCB_LOST) {
				sacked &= ~(MTPCB_LOST | MTPCB_SACKED_RETRANS);
				msk->lost_out -= 1;
				msk->retrans_out -= 1;
			}
			goto check_next;
		}

		if (!(sacked & MTPCB_RETRANS)) {
			/* New sack for not retransmitted frame,
			 * which was in hole. It is reordering.
			 */
			if (before(start_seq, mtp_highest_sack_seq(msk)))
				state->reord = min(fack_count, state->reord);
			if (!after(end_seq, msk->high_seq))
				state->flag |= MTP_ACK_FLAG_ORIG_SACK_ACKED;
			if (state->first_sackt == 0)
				state->first_sackt = xmit_time;
			state->last_sackt = xmit_time;
		}

		if (sacked & MTPCB_LOST) {
			sacked &= ~MTPCB_LOST;
			msk->lost_out -= 1;
		}

check_next:
		sacked |= MTPCB_SACKED_ACKED;
		state->flag |= MTP_ACK_FLAG_DATA_SACKED;
		msk->sacked_out += 1;
		msk->delivered += 1;

		fack_count += 1;
		mtp_debug("%u:%u %u~%u MTPCB_SACKED_ACKED fack_count %d sacked_out %d\n",
			msk->src_port, msk->dst_port, start_seq, end_seq, fack_count, msk->sacked_out);
		if (fack_count > msk->fackets_out)
			msk->fackets_out = fack_count;
	}

	return sacked;
}

static struct sk_buff *mtp_sacktag_walk(struct sk_buff *skb, struct sock *sk,
					struct mtp_sacktag_state *state,
					u32 start_seq, u32 end_seq)
{
	struct mtp_sock *msk = mtp_sk(sk);

	mtp_for_write_queue_from(skb, sk) {
		int in_sack = 0;
		struct mtp_skb_cb *cb = MTP_SKB_CB(skb);

		if (skb == mtp_send_head(sk))
			break;

		/* queue is in-order => we can short-circuit the walk early */
		if (!before(MTP_SKB_CB(skb)->seq, end_seq))
			break;

		if (cb->sacked & MTPCB_SACKED_ACKED)
			continue;
		in_sack = mtp_match_skb_to_sack(sk, skb, start_seq, end_seq);
		if (in_sack) {
			cb->sacked = mtp_sacktag_one(sk,
						state,
						cb->sacked,
						cb->seq,
						cb->end_seq,
						mtp_skb_timestamp_us(skb));

			if (!before(cb->seq, mtp_highest_sack_seq(msk)))
				mtp_advance_highest_sack(sk, skb);
		}

		state->fack_count += 1;
	}
	return skb;
}

/* Avoid all extra work that is being done by sacktag while walking in
 * a normal way
 */
static struct sk_buff *mtp_sacktag_skip(struct sk_buff *skb, struct sock *sk,
					struct mtp_sacktag_state *state,
					u32 skip_to_seq)
{
	mtp_for_write_queue_from(skb, sk) {
		if (skb == mtp_send_head(sk))
			break;

		if (after(MTP_SKB_CB(skb)->end_seq, skip_to_seq))
			break;

		state->fack_count += 1;
	}
	return skb;
}

static bool mtp_is_sackblock_valid(const struct mtp_sock *msk, u32 start_seq, u32 end_seq)
{
	/* Too far in future, or reversed (interpretation is ambiguous) */
	if (after(end_seq, msk->snd_nxt) || !before(start_seq, end_seq))
		return false;

	/* Ignore very old stuff early */
	if (!after(end_seq, msk->snd_una))
		return false;

	/* Nasty start_seq wrap-around check (see comments above) */
	if (!before(start_seq, msk->snd_nxt))
		return false;

	/* In outstanding window? ...This is valid exit for D-SACKs too.
	 * start_seq == snd_una is non-sensical (see comments above)
	 */
	if (after(start_seq, msk->snd_una))
		return true;

	return false;
}

static int mtp_sack_cache_ok(const struct mtp_sock *msk, const struct mtp_sack_block *cache)
{
	return cache < msk->recv_sack_cache + ARRAY_SIZE(msk->recv_sack_cache);
}

/* return -1 to break, 1 to continue with next sack block, else(0) to continue with current sack block */
static int mtp_sacktag_block_process(struct sock *sk, struct mtp_sack_block **cache,
	struct sk_buff **skb, struct mtp_sack_block *sp, struct mtp_sacktag_state *state)
{
	struct mtp_sock *msk = mtp_sk(sk);

	/* Skip too early cached blocks */
	while (mtp_sack_cache_ok(msk, *cache) &&
	       !before(sp->start_seq, (*cache)->end_seq))
		(*cache)++;

	/* Can skip some work by looking recv_sack_cache? */
	if (mtp_sack_cache_ok(msk, *cache) &&
	    after(sp->end_seq, (*cache)->start_seq)) {
		/* Head todo? */
		if (before(sp->start_seq, (*cache)->start_seq)) {
			*skb = mtp_sacktag_skip(*skb, sk, state,
					       sp->start_seq);
			*skb = mtp_sacktag_walk(*skb, sk, state,
					       sp->start_seq,
					       (*cache)->start_seq);
		}

		/* Rest of the block already fully processed? */
		if (!after(sp->end_seq, (*cache)->end_seq))
			return 1;

		/* ...tail remains todo... */
		if (mtp_highest_sack_seq(msk) == (*cache)->end_seq) {
			/* ...but better entrypoint exists! */
			*skb = mtp_highest_sack(sk);
			if (!(*skb))
				return -1;

			state->fack_count = msk->fackets_out;
			(*cache)++;
			goto walk;
		}

		*skb = mtp_sacktag_skip(*skb, sk, state, (*cache)->end_seq);
		/* Check overlap against next cached too (past this one already) */
		(*cache)++;
		return 0;
	}

	if (!before(sp->start_seq, mtp_highest_sack_seq(msk))) {
		*skb = mtp_highest_sack(sk);
		if (!(*skb))
			return -1;
		state->fack_count = msk->fackets_out;
	}
	*skb = mtp_sacktag_skip(*skb, sk, state, sp->start_seq);

walk:
	*skb = mtp_sacktag_walk(*skb, sk, state, sp->start_seq, sp->end_seq);
	return 1;
}

static int mtp_sack_block_sort(const struct mtp_sock *msk, const struct sk_buff *ack_skb,
	struct mtp_sack_block *sp, const int sp_len)
{
	int used_sacks = 0;
	int i, j;
	const unsigned char *ptr = (skb_transport_header(ack_skb) +
				    MTP_SKB_CB(ack_skb)->sacked);
	/* add 2 to skip type&len */
	struct mtp_sack_block_wire *sp_wire = (struct mtp_sack_block_wire *)(ptr + 2);
	/* devided by 8(each sack block sizze) */
	int num_sacks = min(sp_len, (ptr[1] - MTPOLEN_SACK_BASE) >> 3);

	for (i = 0; i < num_sacks; i++) {
		sp[used_sacks].start_seq = get_unaligned_be32(&sp_wire[i].start_seq);
		sp[used_sacks].end_seq = get_unaligned_be32(&sp_wire[i].end_seq);

		mtp_debug("%u:%u recv sack block %d %u~%u\n", msk->src_port, msk->dst_port, used_sacks,
			sp[used_sacks].start_seq, sp[used_sacks].end_seq);
		if (!mtp_is_sackblock_valid(msk, sp[used_sacks].start_seq,
					    sp[used_sacks].end_seq))
			continue;

		used_sacks++;
	}

	/* Bubble Sort */
	for (i = used_sacks - 1; i > 0; i--) {
		for (j = 0; j < i; j++) {
			if (after(sp[j].start_seq, sp[j + 1].start_seq))
				swap(sp[j], sp[j + 1]);
		}
	}

	return used_sacks;
}

static struct mtp_sack_block *mtp_sack_get_cache(struct mtp_sock *msk)
{
	struct mtp_sack_block *cache = msk->recv_sack_cache;
	if (!msk->sacked_out) {
		/* It's already past, so skip checking against it */
		cache = msk->recv_sack_cache + ARRAY_SIZE(msk->recv_sack_cache);
	} else {
		cache = msk->recv_sack_cache;
		/* Skip empty blocks in at head of the cache */
		while (mtp_sack_cache_ok(msk, cache) && !cache->start_seq &&
		       !cache->end_seq)
			cache++;
	}
	return cache;
}

static int mtp_sacktag_write_queue(struct sock *sk, const struct sk_buff *ack_skb,
	struct mtp_sacktag_state *state)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_sack_block sp[MTP_NUM_SACKS];
	struct mtp_sack_block *cache;
	struct sk_buff *skb;
	int used_sacks;
	int i, j;

	state->flag = 0;
	state->reord = msk->packets_out;

	if (!msk->sacked_out) {
		if (WARN_ON(msk->fackets_out))
			msk->fackets_out = 0;
		mtp_highest_sack_reset(sk);
	}

	if (!msk->packets_out)
		goto out;

	used_sacks = mtp_sack_block_sort(msk, ack_skb, sp, MTP_NUM_SACKS);
	skb = mtp_write_queue_head(sk);
	state->fack_count = 0;
	cache = mtp_sack_get_cache(msk);

	i = 0;
	while (i < used_sacks) {
		int ret = mtp_sacktag_block_process(sk, &cache, &skb, &sp[i], state);
		if (ret == 1)
			i++;
		else if (ret == -1)
			break;
	}

	/* Clear the head of the cache sack blocks so we can skip it next time */
	for (i = 0; i < ARRAY_SIZE(msk->recv_sack_cache) - used_sacks; i++) {
		msk->recv_sack_cache[i].start_seq = 0;
		msk->recv_sack_cache[i].end_seq = 0;
	}
	for (j = 0; j < used_sacks; j++)
		msk->recv_sack_cache[i++] = sp[j];

	if ((state->reord < msk->fackets_out) &&
	    (msk->ca_state != MTP_CA_LOSS || msk->undo_marker))
		mtp_update_reordering(sk, msk->fackets_out - state->reord);

	WARN_ON(mtp_left_out(msk) > msk->packets_out);
out:
	WARN_ON((int)msk->sacked_out < 0);
	WARN_ON((int)msk->lost_out < 0);
	WARN_ON((int)msk->retrans_out < 0);
	WARN_ON((int)mtp_packets_in_flight(msk) < 0);
	return state->flag;
}

static bool mtp_sack_extend(struct mtp_sack_block *sp, u32 seq, u32 end_seq)
{
	if (!after(seq, sp->end_seq) && !after(sp->start_seq, end_seq)) {
		if (before(seq, sp->start_seq))
			sp->start_seq = seq;
		if (after(end_seq, sp->end_seq))
			sp->end_seq = end_seq;
		return true;
	}
	return false;
}

static void mtp_sack_maybe_coalesce(struct mtp_sock *msk)
{
	int this_sack, i;
	struct mtp_sack_block *sp = &msk->selective_acks[0];
	struct mtp_sack_block *swalk = sp + 1;

	for (this_sack = 1; this_sack < msk->num_sacks;) {
		if (mtp_sack_extend(sp, swalk->start_seq, swalk->end_seq)) {
			msk->num_sacks--;
			for (i = this_sack; i < msk->num_sacks; i++)
				sp[i] = sp[i + 1];
			continue;
		}
		this_sack++, swalk++;
	}
}

static void mtp_sack_new_ofo_skb(struct sock *sk, u32 seq, u32 end_seq)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_sack_block *sp = &msk->selective_acks[0];
	int this_sack;

	if (!msk->num_sacks)
		goto new_sack;

	for (this_sack = 0; this_sack < msk->num_sacks; this_sack++, sp++) {
		if (mtp_sack_extend(sp, seq, end_seq)) {
			for (; this_sack > 0; this_sack--, sp--)
				swap(*sp, *(sp - 1));
			if (msk->num_sacks > 1)
				mtp_sack_maybe_coalesce(msk);
			return;
		}
	}

	if (this_sack >= MTP_NUM_SACKS) {
		if (msk->delack.ofo_ack > MTP_FASTRETRANS_THRESH)
			mtp_send_ack(sk, 0);
		this_sack--;
		msk->num_sacks--;
		sp--;
	}
	for (; this_sack > 0; this_sack--, sp--)
		*sp = *(sp - 1);

new_sack:
	sp->start_seq = seq;
	sp->end_seq = end_seq;
	msk->num_sacks++;
}

static void mtp_sack_remove(struct mtp_sock *msk)
{
	struct mtp_sack_block *sp = &msk->selective_acks[0];
	int num_sacks = msk->num_sacks;
	int this_sack;

	/* Empty ofo queue, hence, all the SACKs are eaten. Clear. */
	if (RB_EMPTY_ROOT(&msk->out_of_order_queue)) {
		msk->num_sacks = 0;
		msk->delack.quick = 1;
		return;
	}

	for (this_sack = 0; this_sack < num_sacks;) {
		if (!before(msk->rcv_nxt, sp->start_seq)) {
			int i;

			if (before(msk->rcv_nxt, sp->end_seq))
				mtp_debug("%u:%u rcv_nxt %u must cover end_seq %u\n",
					msk->src_port, msk->dst_port, msk->rcv_nxt, sp->end_seq);

			for (i = this_sack + 1; i < num_sacks; i++)
				msk->selective_acks[i - 1] = msk->selective_acks[i];
			num_sacks--;
			msk->delack.quick = 1;
			continue;
		}
		this_sack++;
		sp++;
	}
	msk->num_sacks = num_sacks;
}

static inline bool mtp_ack_is_dubious(const struct sock *sk, const u32 flag)
{
	return !(flag & MTP_ACK_FLAG_NOT_DUP) || (flag & MTP_ACK_FLAG_CA_ALERT) ||
		mtp_sk(sk)->ca_state != MTP_CA_OPEN;
}

static void mtp_mark_head_lost(struct sock *sk, int packets, int mark_head)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	int cnt;
	/* Use SACK to deduce losses of new sequences sent during recovery */
	const u32 loss_high = msk->snd_nxt;
	u32 origin_lost_out = msk->lost_out;

	WARN_ON(packets > msk->packets_out);
	if (msk->lost_skb_hint) {
		skb = msk->lost_skb_hint;
		cnt = msk->lost_cnt_hint;
		/* Head already handled? */
		if (mark_head && skb != mtp_write_queue_head(sk))
			return;
	} else {
		skb = mtp_write_queue_head(sk);
		cnt = 0;
	}

	mtp_for_write_queue_from(skb, sk) {
		if (skb == mtp_send_head(sk))
			break;
		/* this is not the most efficient way to do this... */
		msk->lost_skb_hint = skb;
		msk->lost_cnt_hint = cnt;

		if (after(MTP_SKB_CB(skb)->end_seq, loss_high))
			break;

		if (cnt >= packets) {
			break;
		}

		cnt++;
		if (MTP_SKB_CB(skb)->sacked & MTPCB_SACKED_ACKED)
			continue;

		if (!(MTP_SKB_CB(skb)->sacked & (MTPCB_LOST | MTPCB_SACKED_ACKED))) {
			mtp_verify_retransmit_hint(msk, skb);

			msk->lost_out += 1;
			MTP_SKB_CB(skb)->sacked |= MTPCB_LOST;
			mtp_debug("%u:%u mark %u~%u lost cnt %d\n", msk->src_port, msk->dst_port,
				MTP_SKB_CB(skb)->seq, MTP_SKB_CB(skb)->end_seq, cnt);
		}

		if (mark_head)
			break;
	}
	WARN_ON(mtp_left_out(msk) > msk->packets_out);
	if (msk->lost_out > origin_lost_out)
		mtp_info("%u:%u origin lost_out %u now %u\n", msk->src_port, msk->dst_port,
			origin_lost_out, msk->lost_out);
}

static void mtp_update_scoreboard(struct sock *sk, int fast_rexmit)
{
	struct mtp_sock *msk = mtp_sk(sk);

	int lost = msk->fackets_out - MTP_FACKETS_OUT_THRESH;
	if (lost <= 0)
		lost = 1;
	mtp_mark_head_lost(sk, lost, 0);
}

/* Process an ACK in CA_Loss state. Move to CA_Open if lost data are
 * recovered or spurious. Otherwise retransmits more on partial ACKs.
 */
static void mtp_process_loss(struct sock *sk, int flag, bool is_dupack,
	int *rexmit)
{
	struct mtp_sock *msk = mtp_sk(sk);
	bool recovered = !before(msk->snd_una, msk->high_seq);

	if ((flag & MTP_ACK_FLAG_SND_UNA_ADVANCED) &&
	    mtp_try_undo_loss(sk, false))
		return;

	if (msk->rtx.frto) { /* F-RTO RFC5682 sec 3.1 (sack enhanced version). */
		/* Step 3.b. A timeout is spurious if not all data are
		 * lost, i.e., never-retransmitted data are (s)acked.
		 */
		if ((flag & MTP_ACK_FLAG_ORIG_SACK_ACKED) &&
		    mtp_try_undo_loss(sk, true))
			return;

		if (after(msk->snd_nxt, msk->high_seq)) {
			if ((flag & MTP_ACK_FLAG_DATA_SACKED) || is_dupack)
				msk->rtx.frto = 0; /* Step 3.a. loss was real */
		} else if ((flag & MTP_ACK_FLAG_SND_UNA_ADVANCED) && !recovered) {
			msk->high_seq = msk->snd_nxt;
			/* Step 2.b. Try send new data (but deferred until cwnd
			 * is updated in mtp_ack()). Otherwise fall back to
			 * the conventional recovery.
			 */
			if (mtp_send_head(sk)) {
				*rexmit = REXMIT_NEW;
				return;
			}
			msk->rtx.frto = 0;
		}
	}

	if (recovered) {
		/* F-RTO RFC5682 sec 3.1 step 2.a and 1st part of step 3.a */
		mtp_try_undo_recovery(sk);
		return;
	}
	*rexmit = REXMIT_LOST;
}

static int check_ca_state(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (msk->ca_state == MTP_CA_OPEN) {
		WARN_ON(msk->retrans_out != 0);
	} else if (!before(msk->snd_una, msk->high_seq)) {
		switch (msk->ca_state) {
		case MTP_CA_CWR:
			/* CWR is to be held something *above* high_seq
			 * is ACKed for CWR bit to reach receiver. */
			if (msk->snd_una != msk->high_seq) {
				mtp_end_cwnd_reduction(sk);
				msk->ca_loss = 0;
				mtp_set_ca_state(sk, MTP_CA_OPEN);
			}
			break;

		case MTP_CA_RECOVERY:
			if (mtp_try_undo_recovery(sk))
				return 1;
			mtp_end_cwnd_reduction(sk);
			break;
		default:
			break;
		}
	}
	return 0;
}

static void mtp_fastretrans_alert(struct sock *sk, const int acked,
	bool is_dupack, u32 *ack_flag, int *rexmit)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int fast_rexmit = 0;
	u32 flag = *ack_flag;
	bool do_lost = is_dupack || ((flag & MTP_ACK_FLAG_DATA_SACKED) &&
				    (msk->fackets_out > MTP_FACKETS_OUT_THRESH));

	if (!msk->packets_out && msk->sacked_out)
		msk->sacked_out = 0;
	if (!msk->sacked_out && msk->fackets_out)
		msk->fackets_out = 0;

	if (check_ca_state(sk))
		return;

	/* Process state. */
	switch (msk->ca_state) {
	case MTP_CA_RECOVERY:
		if (flag & MTP_ACK_FLAG_SND_UNA_ADVANCED) {
			if (mtp_try_undo_partial(sk, acked))
				return;
			/* Partial ACK arrived. Force fast retransmit. */
			do_lost = msk->fackets_out > msk->reordering;
		}
		if (mtp_rack_identify_loss(sk))
			*ack_flag |= MTP_ACK_FLAG_LOST_RETRANS;
		break;
	case MTP_CA_LOSS:
		mtp_process_loss(sk, flag, is_dupack, rexmit);
		if (mtp_rack_identify_loss(sk)) {
			*ack_flag |= MTP_ACK_FLAG_LOST_RETRANS;
			return;
		}
		if (msk->ca_state != MTP_CA_OPEN)
			return;
		/* fall through */
	default:
		if (mtp_rack_identify_loss(sk))
			*ack_flag |= MTP_ACK_FLAG_LOST_RETRANS;
		if (!msk->lost_out && msk->fackets_out <= MTP_FACKETS_OUT_THRESH) {
			mtp_try_to_open(sk);
			return;
		}

		/* Otherwise enter Recovery state */
		mtp_enter_recovery(sk);
		fast_rexmit = 1;
		break;
	}

	if (do_lost)
		mtp_update_scoreboard(sk, fast_rexmit);
	*rexmit = REXMIT_LOST;
}

static void mtp_xmit_recovery(struct sock *sk, int rexmit)
{
	if (rexmit == REXMIT_NONE)
		return;

	mtp_xmit_retransmit_queue(sk);
}

static void mtp_tlp_ack(struct sock *sk, u32 ack, int flag)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (before(ack, msk->tlp_high_seq))
		return;

	if (msk->ca_loss) {
		/* ACK advances: there was a loss, so reduce cwnd. Reset
		 * tlp_high_seq in mtp_init_cwnd_reduction()
		 */
		mtp_init_cwnd_reduction(sk);
		mtp_set_ca_state(sk, MTP_CA_CWR);
		mtp_end_cwnd_reduction(sk);
		mtp_try_keep_open(sk);
	} else {
		msk->tlp_high_seq = 0;
	}
}

static void mtp_check_write_space(struct sock *sk)
{
	smp_mb();
	if (sk->sk_socket &&
	    test_bit(SOCK_NOSPACE, &sk->sk_socket->flags))
		sk->sk_write_space(sk);
}

static void mtp_fin(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);

	sk->sk_shutdown |= RCV_SHUTDOWN;
	sock_set_flag(sk, SOCK_DONE);

	switch (sk->sk_state) {
	case TCP_ESTABLISHED:
		/* Move to CLOSE_WAIT */
		mtp_send_fin(sk, sk_gfp_mask(sk, GFP_ATOMIC));
		mtp_set_state(sk, TCP_LAST_ACK);
		sk->sk_shutdown |= SEND_SHUTDOWN;
		sk_reset_timer(sk, &sk->sk_timer, jiffies + (msk->rtx.rto << 1));
		break;
	case TCP_LAST_ACK:
		/* dup FIN */
		break;
	case TCP_FIN_WAIT1:
		msk->delack.ack_pend = 1;
		msk->delack.quick = 1;
		mtp_set_state(sk, TCP_FIN_WAIT2);
		sk_reset_timer(sk, &sk->sk_timer, jiffies + (msk->rtx.rto << 1));
		break;
	case TCP_FIN_WAIT2:
		/* dup FIN */
		msk->delack.ack_pend = 1;
		msk->delack.quick = 1;
		break;
	default:
		mtp_err("%u:%u Impossible, sk->sk_state=%d\n",
		       msk->src_port, msk->dst_port, sk->sk_state);
		break;
	}

	if (!sock_flag(sk, SOCK_DEAD)) {
		sk->sk_state_change(sk);
		/* Do not send POLL_HUP for half duplex close. */
		if (sk->sk_shutdown == SHUTDOWN_MASK || sk->sk_state == TCP_CLOSE)
			sk_wake_async(sk, SOCK_WAKE_WAITD, POLL_HUP);
		else
			sk_wake_async(sk, SOCK_WAKE_WAITD, POLL_IN);
	}
}

static inline void mtp_ack_ca_loss_check(struct mtp_sock *msk, const struct sk_buff *skb)
{
	if (unlikely((msk->ca_state != MTP_CA_OPEN || msk->tlp_high_seq) &&
		  (MTP_SKB_CB(skb)->opt_flags & MTPOPT_FLAG_REC)))
		msk->ca_loss = 1;
}

static int mtp_prior_ack_recv(struct sock *sk, const struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 prior_snd_una = msk->snd_una;
	u32 ack_seq = MTP_SKB_CB(skb)->ack_seq;
	u32 flag = 0;
	int acked = 0; /* Number of packets newly acked */
	bool is_dupack = false;
	int rexmit = REXMIT_NONE; /* Flag to (re)transmit to recover losses */
	struct mtp_sacktag_state sack_state;

	sack_state.first_sackt = 0;
	/* RFC 5961 5.2 [Blind Data Injection Attack].[Mitigation] */
	if (before(ack_seq, prior_snd_una - MTP_MAX_USHORT)) {
		mtp_send_ack(sk, 0);
		return -1;
	}

	mtp_ack_ca_loss_check(msk, skb);
	/* If data was SACKed, tag it and see if we should send more data. */
	if (MTP_SKB_CB(skb)->sacked) {
		flag |= mtp_sacktag_write_queue(sk, skb, &sack_state);
		mtp_fastretrans_alert(sk, acked, is_dupack, &flag, &rexmit);
		mtp_xmit_recovery(sk, rexmit);
	}
	return 0;
}

static int mtp_prior_ack_data(struct sock *sk, const struct sk_buff *skb,
	struct mtp_sacktag_state *sack_state, u32 *flag, u32 prior_fackets)
{
	struct mtp_sock *msk = mtp_sk(sk);
	int acked = 0; /* Number of packets newly acked */
	u32 prior_snd_una = msk->snd_una;
	u32 ack_seq = MTP_SKB_CB(skb)->ack_seq;
	u16 win = MTP_SKB_CB(skb)->win;

	*flag |= MTP_ACK_FLAG_SND_UNA_ADVANCED;
	msk->snd_una = ack_seq;
	msk->snd_wnd = win;
	*flag |= mtp_write_queue_acked(sk, prior_fackets, prior_snd_una,
		&acked, sack_state);
	mtp_check_write_space(sk);

	if (msk->tlp_high_seq)
		mtp_tlp_ack(sk, ack_seq, *flag);
	/* If needed, reset TLP/RTO timer; RACK may later override this. */
	if ((*flag) & MTP_ACK_FLAG_SET_XMIT_TIMER) {
		if (!mtp_schedule_loss_probe(sk, true))
			mtp_rearm_rto(sk);
	}

	return acked;
}

static int mtp_ack(struct sock *sk, const struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 flag = 0;
	struct mtp_sacktag_state sack_state;
	u32 prior_fackets;
	u32 delivered = msk->delivered;
	int acked = 0; /* Number of packets newly acked */
	int rexmit = REXMIT_NONE; /* Flag to (re)transmit to recover losses */

	sack_state.first_sackt = 0;

	prefetchw(sk->sk_write_queue.next);
	msk->stats.in_acks++;
	/* If the ack is older than previous acks
	 * then we can probably ignore it.
	 */
	if (before(MTP_SKB_CB(skb)->ack_seq, msk->snd_una))
		return mtp_prior_ack_recv(sk, skb);

	msk->keepalive_probes = 0;
	msk->rcv_tstamp = MTP_JIFFIERS32;
	prior_fackets = msk->fackets_out;
	if (MTP_SKB_CB(skb)->sacked)
		flag |= mtp_sacktag_write_queue(sk, skb, &sack_state);
	if (after(MTP_SKB_CB(skb)->ack_seq, msk->snd_una)) {
		acked = mtp_prior_ack_data(sk, skb, &sack_state, &flag, prior_fackets);
	} else if (MTP_SKB_CB(skb)->win > msk->snd_wnd) {
		msk->snd_wnd = MTP_SKB_CB(skb)->win;
		msk->rtx.probes = 0;
	}

	if (MTP_SKB_CB(skb)->seq != MTP_SKB_CB(skb)->end_seq)
		flag |= MTP_ACK_FLAG_DATA;

	if (mtp_ack_is_dubious(sk, flag)) {
		bool is_dupack = !(flag & (MTP_ACK_FLAG_SND_UNA_ADVANCED | MTP_ACK_FLAG_NOT_DUP));

		mtp_ack_ca_loss_check(msk, skb);
		mtp_fastretrans_alert(sk, acked, is_dupack, &flag, &rexmit);
	}

	mtp_debug("%u:%u flag %x rexmit %d\n", msk->src_port, msk->dst_port, flag, rexmit);
	delivered = msk->delivered - delivered; /* freshly ACKed or SACKed */
	mtp_cong_control(sk, MTP_SKB_CB(skb)->ack_seq, delivered, flag);
	mtp_xmit_recovery(sk, rexmit);
	return flag;
}

static void mtp_event_data_recv(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	msk->delack.ack_pend = 1;
}

static void mtp_drop(struct sock *sk, struct sk_buff *skb)
{
	MTP_INC_STATS(MTP_MIB_INERRS);
	sk_drops_add(sk, skb);
	__kfree_skb(skb);
}

static void mtp_rtx(struct mtp_sock *msk, const struct sk_buff *skb)
{
	if (unlikely(MTP_SKB_CB(skb)->flags & MTPHDR_RTX))
		msk->ca_recovery = 1;
}

static int mtp_receive_queue(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_skb_cb *mcb = MTP_SKB_CB(skb);

	if (unlikely(sock_flag(sk, SOCK_DEAD) && skb->len)) {
		mtp_err("sock is dead, can't recv data\n");
		return -1;
	}

	msk->cur_rcv_msg_size += skb->len;
	if (unlikely(msk->cur_rcv_msg_size > msk->max_msg_size)) {
		mtp_err("%u:%u cur_msg_size %u larger than %u\n", msk->src_port, msk->dst_port,
			msk->cur_rcv_msg_size, msk->max_msg_size);
		return -1;
	}

	__skb_queue_tail(&msk->reader_queue, skb);
	if (!sock_flag(sk, SOCK_DEAD) && (mcb->type == MTPHDR_TYPE_MSG_END ||
	    mcb->type == MTPHDR_TYPE_FIN)) {
		skb_queue_splice_tail_init(&msk->reader_queue, &sk->sk_receive_queue);
		sk->sk_data_ready(sk);
		mtp_debug("%u:%u sk_data_ready msg_size %u\n", msk->src_port, msk->dst_port, msk->cur_rcv_msg_size);
		msk->cur_rcv_msg_size = 0;
	}

	return 0;
}

static int mtp_ofo_queue(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb;
	struct rb_node *p;

	p = rb_first(&msk->out_of_order_queue);
	while (p) {
		skb = rb_to_skb(p);
		if (after(MTP_SKB_CB(skb)->seq, msk->rcv_nxt))
			break;

		p = rb_next(p);
		rb_erase(&skb->rbnode, &msk->out_of_order_queue);

		if (unlikely(!after(MTP_SKB_CB(skb)->end_seq, msk->rcv_nxt))) {
			mtp_debug("%u:%u ofo packet was already received\n", msk->src_port, msk->dst_port);
			mtp_drop(sk, skb);
			continue;
		}
		mtp_debug("ofo requeuing: %u:%u rcv_next %X seq %X - %X\n",
			  msk->src_port, msk->dst_port,
			  msk->rcv_nxt, MTP_SKB_CB(skb)->seq,
			  MTP_SKB_CB(skb)->end_seq);

		msk->rcv_nxt = MTP_SKB_CB(skb)->end_seq;
		if (skb == msk->ooo_last_skb)
			msk->ooo_last_skb = NULL;
		if (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_FIN) {
			mtp_fin(sk);
			if (!skb->len)
				break;
		}

		if (mtp_receive_queue(sk, skb))
			return -1;
	}
	return 0;
}

static int mtp_queue_rcv(struct sock *sk, struct sk_buff *skb, int hdrlen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	__skb_pull(skb, hdrlen);
	msk->rcv_nxt = MTP_SKB_CB(skb)->end_seq;
	mtp_rtx(msk, skb);
	skb_set_owner_r(skb, sk);

	return mtp_receive_queue(sk, skb);
}

static void mtp_ack_snd_check(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	if (!msk->delack.ack_pend)
		return;

	if (msk->delack.quick)
		goto send_now;

	if (unlikely(!RB_EMPTY_ROOT(&msk->out_of_order_queue))) {
		if (msk->delack.ofo_ack >= MTP_SEG_BURST)
			goto send_now;
		if (msk->delack.ofo_rcv_nxt != msk->rcv_nxt) {
			msk->delack.ofo_rcv_nxt = msk->rcv_nxt;
			if (msk->delack.ofo_ack > MTP_FASTRETRANS_THRESH)
				mtp_debug("%u:%u ofo_ack %u\n", msk->src_port, msk->dst_port, msk->delack.ofo_ack);
			msk->delack.ofo_ack = 0;
		}

		if (++msk->delack.ofo_ack <= MTP_FASTRETRANS_THRESH)
			goto send_now;
	}

	mtp_send_delayed_ack(sk);
	return;

send_now:
	mtp_send_ack(sk, 0);
}

int mtp_add_backlog(struct sock *sk, struct sk_buff *skb)
{
	u32 limit = sk->sk_rcvbuf + sk->sk_sndbuf;
	limit += MTP_MAX_SKB_TRUESIZE;
	return sk_add_backlog(sk, skb, limit);
}

static bool mtp_try_coalesce(struct sock *sk,
			     struct sk_buff *to,
			     struct sk_buff *from,
			     bool *fragstolen)
{
	int delta;

	*fragstolen = false;
	if (MTP_SKB_CB(from)->seq != MTP_SKB_CB(to)->end_seq)
		return false;

	if (MTP_SKB_CB(to)->type != MTPHDR_TYPE_DEFAULT)
		return false;

	if (!skb_try_coalesce(to, from, fragstolen, &delta))
		return false;

	atomic_add(delta, &sk->sk_rmem_alloc);
	sk_mem_charge(sk, delta);
	MTP_SKB_CB(to)->end_seq = MTP_SKB_CB(from)->end_seq;
	MTP_SKB_CB(to)->ack_seq = MTP_SKB_CB(from)->ack_seq;
	MTP_SKB_CB(to)->flags |= MTP_SKB_CB(from)->flags;
	MTP_SKB_CB(to)->type = MTP_SKB_CB(from)->type;
	return true;
}

static void mtp_create_first_sack(struct mtp_sock *msk, struct sk_buff *skb, u32 seq, u32 end_seq)
{
	struct rb_node **p = &msk->out_of_order_queue.rb_node;

	mtp_rtx(msk, skb);
	msk->num_sacks = 1;
	msk->selective_acks[0].start_seq = seq;
	msk->selective_acks[0].end_seq = end_seq;
	msk->ooo_last_skb = skb;
	rb_link_node(&skb->rbnode, NULL, p);
	rb_insert_color(&skb->rbnode, &msk->out_of_order_queue);
}

static void mtp_ofo_skb_merge_right(struct sock *sk, struct sk_buff *skb, u32 seq, u32 end_seq)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct sk_buff *skb1;

	/* Remove other segments covered by skb. */
	while ((skb1 = skb_rb_next(skb)) != NULL) {
		if (!after(end_seq, MTP_SKB_CB(skb1)->seq))
			break;
		if (before(end_seq, MTP_SKB_CB(skb1)->end_seq)) {
			break;
		}
		rb_erase(&skb1->rbnode, &msk->out_of_order_queue);
		mtp_drop(sk, skb1);
	}
	/* If there is no skb after us, we are the last_skb ! */
	if (!skb1)
		msk->ooo_last_skb = skb;
	mtp_sack_new_ofo_skb(sk, seq, end_seq);
}

#define MTP_DATA_OFO_RETURN 0
#define MTP_DATA_OFO_MERGE 1
#define MTP_DATA_OFO_COALESCE 2
#define MTP_DATA_OFO_CONTINUE 3

static int mtp_data_queue_ofo_walk(struct sock *sk, struct sk_buff *skb,
	struct rb_node **parent, struct rb_node ***p, bool *fragstolen)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 seq, end_seq;
	struct sk_buff *skb1;

	seq = MTP_SKB_CB(skb)->seq;
	end_seq = MTP_SKB_CB(skb)->end_seq;
	while (**p) {
		*parent = **p;
		skb1 = rb_to_skb(*parent);
		if (before(seq, MTP_SKB_CB(skb1)->seq)) {
			*p = &(*parent)->rb_left;
			continue;
		}
		if (before(seq, MTP_SKB_CB(skb1)->end_seq)) {
			if (!after(end_seq, MTP_SKB_CB(skb1)->end_seq)) {
				/* All the bits are present. Drop. */
				mtp_debug("%u:%u already recevied\n", msk->src_port, msk->dst_port);
				mtp_drop(sk, skb);
				return MTP_DATA_OFO_RETURN;
			}
			if (after(seq, MTP_SKB_CB(skb1)->seq)) {
				/* Partial overlap. */
				mtp_debug("%u:%u Partial overlap\n", msk->src_port, msk->dst_port);
			} else {
				mtp_rtx(msk, skb);
				/* skb's seq == skb1's seq and skb covers skb1.
				 * Replace skb1 with skb.
				 */
				rb_replace_node(&skb1->rbnode, &skb->rbnode,
						&msk->out_of_order_queue);
				mtp_drop(sk, skb1);
				return MTP_DATA_OFO_MERGE;
			}
		} else if (mtp_try_coalesce(sk, skb1, skb, fragstolen)) {
			return MTP_DATA_OFO_COALESCE;
		}
		*p = &(*parent)->rb_right;
	}

	return MTP_DATA_OFO_CONTINUE;
}

static void mtp_data_queue_ofo(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct rb_node **p, *parent;
	u32 seq, end_seq;
	bool fragstolen;
	int ret;

	msk->delack.ack_pend = 1;
	seq = MTP_SKB_CB(skb)->seq;
	end_seq = MTP_SKB_CB(skb)->end_seq;
	mtp_debug("out of order segment: %u:%u rcv_next %X seq %X - %X\n",
		   msk->src_port, msk->dst_port, msk->rcv_nxt, seq, end_seq);

	p = &msk->out_of_order_queue.rb_node;
	if (RB_EMPTY_ROOT(&msk->out_of_order_queue)) {
		mtp_create_first_sack(msk, skb, seq, end_seq);
		goto end;
	}

	if (mtp_try_coalesce(sk, msk->ooo_last_skb, skb, &fragstolen))
		goto coalesce_done;
	if (!before(seq, MTP_SKB_CB(msk->ooo_last_skb)->end_seq)) {
		parent = &msk->ooo_last_skb->rbnode;
		p = &parent->rb_right;
		goto insert;
	}

	parent = NULL;
	ret = mtp_data_queue_ofo_walk(sk, skb, &parent, &p, &fragstolen);
	if (ret == MTP_DATA_OFO_RETURN)
		return;
	else if (ret == MTP_DATA_OFO_MERGE)
		goto merge_right;
	else if (ret == MTP_DATA_OFO_COALESCE)
		goto coalesce_done;
insert:
	/* Insert segment into RB tree. */
	mtp_debug("%u:%u Insert out_of_order_queue\n", msk->src_port, msk->dst_port);
	mtp_rtx(msk, skb);
	rb_link_node(&skb->rbnode, parent, p);
	rb_insert_color(&skb->rbnode, &msk->out_of_order_queue);

merge_right:
	mtp_ofo_skb_merge_right(sk, skb, seq, end_seq);
end:
	skb_set_owner_r(skb, sk);
	return;

coalesce_done:
	mtp_debug("%u:%u mtp_try_coalesce\n", msk->src_port, msk->dst_port);
	mtp_rtx(msk, skb);
	kfree_skb_partial(skb, fragstolen);
	mtp_sack_new_ofo_skb(sk, seq, end_seq);
}

static void mtp_reset(struct sock *sk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	mtp_err("%u:%u recv reset on state %u\n", msk->src_port, msk->dst_port, sk->sk_state);
	switch (sk->sk_state) {
	case TCP_SYN_SENT:
		sk->sk_err = ECONNREFUSED;
		break;
	case TCP_CLOSE:
		return;
	default:
		sk->sk_err = ECONNRESET;
	}
	/* This barrier is coupled with smp_rmb() in mtp_sock_poll() */
	smp_wmb();

	mtp_write_queue_purge(sk);
	sk_mem_reclaim(sk);
	msk->lost_skb_hint = NULL;
	msk->retransmit_skb_hint = NULL;
	sk->sk_send_head = NULL;
	msk->packets_out = 0;
	msk->retrans_out = 0;
	mtp_done(sk);

	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_error_report(sk);
}

static bool mtp_reset_check(const struct sock *sk, u32 seq)
{
	struct mtp_sock *msk = mtp_sk(sk);

	if (seq == msk->rcv_nxt)
		return 1;

	if (unlikely(msk->num_sacks)) {
		struct mtp_sack_block *sp = &msk->selective_acks[0];
		int max_sack = sp[0].end_seq;
		int this_sack;

		for (this_sack = 1; this_sack < msk->num_sacks; ++this_sack)
			max_sack = after(sp[this_sack].end_seq,
					 max_sack) ?
					 sp[this_sack].end_seq : max_sack;

		if (seq == max_sack)
			return 1;
	}

	return unlikely((seq == (msk->rcv_nxt - 1)) &&
			((1 << sk->sk_state) & (TCPF_FIN_WAIT1 | TCPF_LAST_ACK |
						TCPF_FIN_WAIT2)));
}

static int mtp_syn_sent_rcv_check(struct sock *sk, const struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtp_skb_cb *mcb = MTP_SKB_CB(skb);
	u16 recv_mss = mcb->mss;
	u32 recv_msg_size = mcb->max_msg_size << MTP_RECV_MSG_SIZE_OFFSET;
	u32 ack = mcb->ack_seq;

	if (!after(ack, msk->snd_una) || after(ack, msk->snd_nxt)) {
		mtp_info("%u:%u ack_seq invalid ack %u snd_una %u snd_nxt %u\n",
			msk->src_port, msk->dst_port, ack, msk->snd_una, msk->snd_nxt);
		return -1;
	}

	if (mcb->type == MTPHDR_TYPE_RST) {
		mtp_info("%u:%u reset is set\n", msk->src_port, msk->dst_port);
		mtp_reset(sk);
		return -1;
	}

	if (mcb->type != MTPHDR_TYPE_HANDSHAKE) {
		mtp_info("%u:%u recv unexpect type %u\n", msk->src_port, msk->dst_port, mcb->type);
		return -1;
	}

	if (recv_mss < MTP_MIN_MSS) {
		mtp_info("%u:%u mss should at least %d\n", msk->src_port, msk->dst_port, MTP_MIN_MSS);
		mtp_reset(sk);
		return -1;
	}

	if (!recv_msg_size) {
		mtp_info("%u:%u recv_msg_size should be set\n", msk->src_port, msk->dst_port);
		mtp_reset(sk);
		return -1;
	}
	if (msk->mss > recv_mss)
		msk->mss = recv_mss;
	if (msk->max_msg_size > recv_msg_size)
		msk->max_msg_size = recv_msg_size;
	if (sk->sk_sndbuf < (msk->max_msg_size << 1)) {
		sk->sk_sndbuf = msk->max_msg_size << 1;
		sk->sk_rcvbuf = sk->sk_sndbuf;
	}
	mtp_debug("%u:%u max_msg_size %u sk_sndbuf %u\n", msk->src_port, msk->dst_port,
		msk->max_msg_size, sk->sk_sndbuf);
	return 0;
}

static int mtp_syn_sent_rcv(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	struct mtphdr *mtph = mtp_hdr(skb);
	struct sk_buff *syn_ack;
	u32 rtt_us;

	if (mtp_syn_sent_rcv_check(sk, skb))
		goto discard;

	msk->snd_wnd = ntohs(mtph->win);
	msk->rcv_wnd_scale = (u16)__order_base_2(msk->mss);
	msk->rcv_wnd = mtp_full_win(sk);
	mtp_debug("%u:%u rcv_scale %u rcv_wnd %u\n", msk->src_port, msk->dst_port, msk->rcv_wnd_scale, msk->rcv_wnd);
	msk->rcv_nxt = MTP_SKB_CB(skb)->seq + 1;
	WRITE_ONCE(msk->copied_seq, msk->rcv_nxt);
	msk->rcv_tstamp = MTP_JIFFIERS32;
	mtp_info("%u:%u ESTABLISHED\n", msk->src_port, msk->dst_port);

	syn_ack = mtp_write_queue_head(sk);
	rtt_us = mtp_stamp_us_delta(msk->cur_mstamp, mtp_skb_timestamp_us(syn_ack));
	mtp_update_rtt_min(sk, rtt_us);
	mtp_rtt_estimator(sk, rtt_us);
	mtp_set_rto(sk);
	mtp_send_synack(sk, msk->write_seq - 1, msk->rcv_nxt);
	if (sock_flag(sk, SOCK_KEEPOPEN))
		sk_reset_timer(sk, &sk->sk_timer, jiffies + msk->keepalive_time);
	smp_mb();
	mtp_set_state(sk, TCP_ESTABLISHED);
	msk->slow_start_mstamp = msk->cur_mstamp;
	if (!sock_flag(sk, SOCK_DEAD)) {
		sk->sk_state_change(sk);
		sk_wake_async(sk, SOCK_WAKE_IO, POLL_OUT);
	}
	return 0;

discard:
	mtp_drop(sk, skb);
	return 0;
}

static int do_rcv_seq(struct sock *sk, struct sk_buff *skb, u32 seq)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u8 is_fin = (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_FIN) ? 1 : 0;
	u32 len = MTP_SKB_CB(skb)->end_seq - seq - is_fin;

	if (seq == msk->rcv_nxt) {
		if (len || is_fin) {
			if (mtp_queue_rcv(sk, skb, MTP_SKB_CB(skb)->hdlen))
				goto disconnect;
			mtp_event_data_recv(sk);
			if (!RB_EMPTY_ROOT(&msk->out_of_order_queue) && mtp_ofo_queue(sk)) {
disconnect:
				mtp_send_reset(sk, msk->snd_nxt, msk->rcv_nxt, GFP_ATOMIC | __GFP_NOWARN);
				mtp_done(sk);
				return 1;
			}

			if (msk->num_sacks)
				mtp_sack_remove(msk);
			mtp_ack(sk, skb);
			if (is_fin)
				mtp_fin(sk);
		} else {
			mtp_ack(sk, skb);
			__kfree_skb(skb);
		}

		goto wake_send;
	} else if (after(seq, msk->rcv_nxt)) {
		if (after(seq, msk->rcv_nxt + msk->mss * msk->rcv_wnd))
			return -1;
		mtp_debug("%u:%u ofo seq %u rcv_nxt %u\n", msk->src_port, msk->dst_port, seq, msk->rcv_nxt);
		mtp_ack(sk, skb);
		if (len || is_fin) {
			__skb_pull(skb, MTP_SKB_CB(skb)->hdlen);
			mtp_data_queue_ofo(sk, skb);
		} else {
			__kfree_skb(skb);
		}
wake_send:
		if (mtp_send_need_sched(msk))
			msk->send_pend = true;
		else
			mtp_write_xmit(sk, 0, sk_gfp_mask(sk, GFP_ATOMIC));
	} else {
		if (after(seq + (msk->mss * msk->snd_wnd), msk->rcv_nxt)) {
			mtp_ack(sk, skb);
			msk->delack.ack_pend = 1;
		}
		mtp_drop(sk, skb);
	}
	return 0;
}

static void mtp_send_check(struct sock *sk)
{
	struct mtp_sock *msk = mtp_sk(sk);
	if (msk->send_pend) {
		unsigned long oval, nval;
		do {
			oval = READ_ONCE(sk->sk_tsq_flags);
			if (oval & MTPF_MSQ_QUEUED)
				goto send_finish;

			nval = oval | MTPF_MSQ_QUEUED | MTPF_MSQ_DEFERRED;
			nval = cmpxchg(&sk->sk_tsq_flags, oval, nval);
		} while (nval != oval);

		if (schedule_work_on(msk->cpu_id, &msk->worker))
			sock_hold(sk);
send_finish:
		msk->send_pend = false;
	}
}

int mtp_l2_do_rcv(struct sock *sk, struct sk_buff *skb)
{
	struct mtp_sock *msk = mtp_sk(sk);
	u32 seq, ack;
	int err;
	struct mtp_skb_cb *mcb = MTP_SKB_CB(skb);

	if (sk->sk_state == TCP_CLOSE)
		goto discard;
	mtp_mstamp_refresh(msk);

	if (unlikely(sk->sk_state == TCP_SYN_SENT))
		return mtp_syn_sent_rcv(sk, skb);

	if (unlikely(mcb->type == MTPHDR_TYPE_SYN || mcb->type == MTPHDR_TYPE_HANDSHAKE))
		goto discard;

	seq = mcb->seq;
	ack = mcb->ack_seq;
	if (after(ack, msk->snd_nxt)) {
		mtp_err("%u:%u ack is invalid, ack %u snd_nxt %u\n",
			msk->src_port, msk->dst_port, ack, msk->snd_nxt);
		goto discard;
	}

	if (unlikely(mcb->type == MTPHDR_TYPE_RST)) {
		if (mtp_reset_check(sk, seq)) {
			mtp_reset(sk);
			__kfree_skb(skb);
			return 0;
		} else {
			goto discard;
		}
	}

	if (unlikely(mcb->flags & MTPHDR_PROBE)) {
		if ((msk->cur_mstamp - msk->probe_rsp_mstamp) > MTP_PROBE_RSP_MIN_US) {
			msk->delack.ack_pend = 1;
			msk->delack.quick = 1;
			msk->probe_rsp_mstamp = msk->cur_mstamp;
		}
	}

	err = do_rcv_seq(sk, skb, seq);
	if (err == 1)
		return 0;
	else if (err == -1)
		goto discard;

	mtp_ack_snd_check(sk);
	mtp_send_check(sk);
	return 0;

discard:
	mtp_drop(sk, skb);
	return 0;
}

static struct sock *mtp_accept_queue_add(struct sock *sk, struct sock *nsk)
{
	struct mtp_sock *msk = (struct mtp_sock *)sk;

	spin_lock(&msk->accept_queue.lock);
	if (unlikely(sk->sk_state != TCP_LISTEN)) {
		nsk = NULL;
	} else {
		list_add_tail(&((struct mtp_sock *)nsk)->req_node, &msk->accept_queue.accept);
		sk_acceptq_added(sk);
		mtp_info("server %u sk_ack_backlog %d\n", msk->src_port, sk->sk_ack_backlog);
	}
	spin_unlock(&msk->accept_queue.lock);
	return nsk;
}

static void mtp_listen_rcv_syn(struct sock *sk, struct sk_buff *skb,
	const struct mtphdr *mtph)
{
	u32 isn;
	bool is_loopback = false;
	struct ethhdr *eth = eth_hdr(skb);
	struct mtp_sock *msk = mtp_sk(sk);
	struct net_device *dev = NULL;

	if (sk_acceptq_is_full(sk)) {
		mtp_err("server %u sk_acceptq_is_full %u\n", msk->src_port, sk->sk_ack_backlog);
		mtp_reply_reset(skb);
		mtp_drop(sk, skb);
		return;
	}

	dev = dev_get_by_mac(eth->h_source);
	if (dev) {
		dev_put(dev);
		is_loopback = true;
	}
	isn = mtp_create_syn_cookie(eth, mtph);
	local_bh_disable();
	(void)memcpy_s(msk->dst_mac, ETH_ALEN, eth->h_source, ETH_ALEN);
	msk->dst_port = mtph->src;
	msk->is_loopback = is_loopback;
	mtp_send_synack(sk, isn, MTP_SKB_CB(skb)->seq + 1);
	local_bh_enable();
	kfree_skb(skb);
}

static void mtp_listen_rcv_ack(struct sock *sk, struct sk_buff *skb)
{
	struct sock *nsk;
	u16 recv_mss = MTP_SKB_CB(skb)->mss;
	u32 recv_msg_size = MTP_SKB_CB(skb)->max_msg_size << MTP_RECV_MSG_SIZE_OFFSET;

	if (recv_mss < MTP_MIN_MSS) {
		mtp_err("server %u mss should at least %d\n", mtp_sk(sk)->src_port, MTP_MIN_MSS);
		goto discard;
	}

	if (!recv_msg_size) {
		mtp_err("server %u recv_msg_size should set\n", mtp_sk(sk)->src_port);
		goto discard;
	}

	nsk = mtp_get_cookie_sock(sk, skb);
	if (!nsk) {
		mtp_reply_reset(skb);
discard:
		MTP_INC_STATS(MTP_MIB_INERRS);
		mtp_drop(sk, skb);
		return;
	}

	(void)mtp_l2_do_rcv(nsk, skb);
	if (!mtp_accept_queue_add(sk, nsk)) {
		nsk->sk_prot->disconnect(nsk, O_NONBLOCK);
		sock_orphan(nsk);
		nsk->sk_prot->destroy(nsk);
		sock_put(nsk);
		bh_unlock_sock(nsk);

		sock_set_flag(nsk, SOCK_DEAD);
		mtp_done(nsk);
		return;
	}

	mtp_send_ack(nsk, 0);
	/* the lock and ref is in mtp_get_cookie_sock() */
	bh_unlock_sock(nsk);
	sock_put(nsk);

	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_data_ready(sk);
}

static void mtp_listen_rcv(struct sock *sk, struct sk_buff *skb,
	const struct mtphdr *mtph)
{
	if (unlikely(sk->sk_state != TCP_LISTEN))
		goto drop;
	if (mtph->len)
		goto drop;

	if (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_SYN) {
		mtp_listen_rcv_syn(sk, skb, mtph);
	} else if (MTP_SKB_CB(skb)->type == MTPHDR_TYPE_HANDSHAKE) {
		mtp_listen_rcv_ack(sk, skb);
	} else {
drop:
		mtp_err("server %u recv invalid packet\n", mtp_sk(sk)->src_port);
		mtp_drop(sk, skb);
	}
}

static int mtp_rcv_check(struct sk_buff *skb)
{
	struct mtphdr *mtph = NULL;
	u8 mtph_len;
	u16 payload_len;

	mtp_debug("------ recv skb len %u-------\n", skb->len);
	if (!pskb_may_pull(skb, MTP_HLEN_MIN)) {
		mtp_debug("pskb_may_pull failed, head len %u\n", skb_headlen(skb));
		return -1;
	}

	mtph = (struct mtphdr *)skb->data;
	mtph_len = MTP_HL_GET(mtph);
	payload_len = ntohs(mtph->len);

	if (unlikely(mtph->ver != 1)) {
		mtp_debug("mtph->ver %u unmatch\n", mtph->ver);
		mtp_reply_reset(skb);
		return -1;
	}
	if (unlikely(mtph->proto)) {
		mtp_debug("mtph->proto %u unmatch\n", mtph->proto);
		return -1;
	}

	if (unlikely(mtph->type > MTPHDR_TYPE_MAX)) {
		mtp_debug("mtph->type %u ussupport\n", mtph->type);
		return -1;
	}

	if (unlikely(mtph_len < MTP_HLEN_MIN)) {
		mtp_debug("mtph_len %u\n", mtph_len);
		return -1;
	}
	if (unlikely(!pskb_may_pull(skb, mtph_len))) {
		mtp_debug("pskb_may_pull fail mtph_len %u\n", mtph_len);
		return -1;
	}
	if (unlikely((mtph_len + payload_len) > skb->len)) {
		mtp_debug("mtph_len %u payload_len %u\n", mtph_len, payload_len);
		return -1;
	} else if (unlikely((mtph_len + payload_len) < skb->len)) {
		__skb_trim(skb, mtph_len + payload_len);
	}

	mtp_rcv_fill_cb(skb, mtph);
	skb_reset_transport_header(skb);
	return 0;
}

int mtp_l2_rcv(struct sk_buff *skb, struct net_device *dev,
	       struct packet_type *pt, struct net_device *orig_dev)
{
	struct sock *sk = NULL;
	struct mtphdr *mtph = NULL;

	/* Count it even if it's bad */
	__MTP_INC_STATS(MTP_MIB_INSEGS);

	if (mtp_rcv_check(skb))
		goto discard_it;

	mtph = mtp_hdr(skb);
	sk = mtp_lookup_skb(skb, mtph->dst, mtph->src);
	if (sk == NULL) {
		mtp_debug("can not find sock by port %u:%u\n", mtph->src, mtph->dst);
		mtp_reply_reset(skb);
		goto discard_it;
	}

	if (sk->sk_state == TCP_LISTEN) {
		local_bh_disable();
		bh_lock_sock_nested(sk);
		mtp_listen_rcv(sk, skb, mtph);
		bh_unlock_sock(sk);
		local_bh_enable();
		sock_put(sk);
		return NET_RX_SUCCESS;
	}

	local_bh_disable();
	bh_lock_sock_nested(sk);

	if (sock_owned_by_user(sk)) {
		if (mtp_add_backlog(sk, skb)) {
			mtp_err("mtp_add_backlog fail\n");
			goto discard_and_unlock;
		}
	} else {
		mtp_dev_update_bytes(mtp_sk(sk)->dev_node, skb->len);
		(void)mtp_l2_do_rcv(sk, skb);
	}
	bh_unlock_sock(sk);
	local_bh_enable();
	sock_put(sk);
	return NET_RX_SUCCESS;
discard_and_unlock:
	bh_unlock_sock(sk);
	local_bh_enable();
	sk_drops_add(sk, skb);
	sock_put(sk);
discard_it:
	MTP_INC_STATS(MTP_MIB_INERRS);
	kfree_skb(skb);
	return NET_RX_SUCCESS;
}
