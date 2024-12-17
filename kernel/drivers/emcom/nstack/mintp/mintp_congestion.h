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

#ifndef _MINTP_CONGESTION_H
#define _MINTP_CONGESTION_H

#include "mintp.h"

enum mtp_ca_state {
	MTP_CA_OPEN = 0,
	MTP_CA_DISORDER = 1,
	MTP_CA_CWR = 2,
	MTP_CA_RECOVERY = 3,
	MTP_CA_LOSS = 4
};

#define MTPF_CA_OPEN		(1 << MTP_CA_OPEN)
#define MTPF_CA_DISORDER	(1 << MTP_CA_DISORDER)
#define MTPF_CA_CWR		(1 << MTP_CA_CWR)
#define MTPF_CA_RECOVERY	(1 << MTP_CA_RECOVERY)
#define MTPF_CA_LOSS		(1 << MTP_CA_LOSS)

static const char *ca_state_str[MTP_CA_LOSS + 1] = {
	"MTP_CA_OPEN",
	"MTP_CA_DISORDER",
	"MTP_CA_CWR",
	"MTP_CA_RECOVERY",
	"MTP_CA_LOSS",
};

#define MTP_INIT_CWND 128
#define MTP_INIT_CWND_HI1106 512 /* need a large burst to Trigger hi1106's performance Mode */
#define MTP_REORDERING_MAX 300
#define MTP_HI1106_WAKE_PEROID 3000000 /* hi1106 need some time change to performance Mode */
#define MTP_SND_CWND_CLAMP 10000u

static inline void mtp_set_ca_state(struct sock *sk, const u8 ca_state)
{
	struct mtp_sock *msk = mtp_sk(sk);

	mtp_info("%u:%u ca_state change from %s to %s\n", msk->src_port, msk->dst_port,
		 ca_state_str[msk->ca_state], ca_state_str[ca_state]);
	msk->ca_state = ca_state;
}

#define MTP_INFINITE_SSTHRESH	0x7fffffff

static inline bool mtp_in_slow_start(const struct mtp_sock *msk)
{
	return msk->snd_cwnd < msk->snd_ssthresh;
}

static inline bool mtp_in_initial_slowstart(const struct mtp_sock *msk)
{
	return msk->snd_ssthresh >= MTP_INFINITE_SSTHRESH;
}

static inline bool mtp_in_cwnd_reduction(const struct sock *sk)
{
	return (MTPF_CA_CWR | MTPF_CA_RECOVERY) &
	       (1 << ((const struct mtp_sock *)sk)->ca_state);
}

/* If cwnd > ssthresh, we may raise ssthresh to be half-way to cwnd.
 * The exception is cwnd reduction phase, when cwnd is decreasing towards
 * ssthresh.
 */
static inline __u32 mtp_current_ssthresh(const struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);

	/* if in cwnd reduction, return snd_ssthresh
	 * if not in cwnd reduction, return max(snd_ssthresh, 3/4 * snd_cwnd)
	 */
	if (mtp_in_cwnd_reduction(sk))
		return msk->snd_ssthresh;
	else
		return max(msk->snd_ssthresh,
			   ((msk->snd_cwnd >> 1) +
			    (msk->snd_cwnd >> 2)));
}

static inline void mtp_init_undo(struct mtp_sock *msk)
{
	msk->undo_marker = msk->snd_una;
}

static inline bool mtp_is_cwnd_limited(const struct sock *sk)
{
	const struct mtp_sock *msk = mtp_sk(sk);

	/* If in slow start, ensure cwnd grows to twice what was ACKed. */
	if (mtp_in_slow_start(msk))
		return msk->snd_cwnd < (msk->max_packets_out << 1);

	return msk->ca_cwnd_limited;
}

static inline unsigned int mtp_left_out(const struct mtp_sock *msk)
{
	return msk->sacked_out + msk->lost_out;
}

static inline unsigned int mtp_packets_in_flight(const struct mtp_sock *msk)
{
	return msk->packets_out - mtp_left_out(msk) + msk->retrans_out;
}

void mtp_init_congestion_control(struct sock *sk);
bool mtp_try_undo_loss(struct sock *sk, bool frto_undo);
bool mtp_try_undo_partial(struct sock *sk, int acked);
void mtp_enter_recovery(struct sock *sk);
void mtp_enter_loss(struct sock *sk);
void mtp_try_to_open(struct sock *sk);
void mtp_init_cwnd_reduction(struct sock *sk);
void mtp_try_keep_open(struct sock *sk);
void mtp_cong_control(struct sock *sk, u32 ack, u32 acked_sacked, u32 flag);
void mtp_cwnd_reduction(struct sock *sk, int newly_acked_sacked, u32 flag);
void mtp_end_cwnd_reduction(struct sock *sk);
bool mtp_try_undo_recovery(struct sock *sk);
void mtp_update_reordering(struct sock *sk, const int metric);

#endif /* _MINTP_CONGESTION_H */
