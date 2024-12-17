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

#ifndef __MINTP_OUTPUT_H__
#define __MINTP_OUTPUT_H__

#include "mintp.h"

#define MTP_RTO_MIN_US  (100 * 1000u)
#define MTP_PROBE_RSP_MIN_US  (1000u)

#define MTP_ATO_MAX	(1500U) /* maximal time to delay before sending an ACK */
#define MTP_ATO_MIN	1000U

#define MTP_TIMEOUT_MIN	(2U) /* Min timeout for MTP timers in jiffies */
#define MTP_TIMEOUT_INIT (MTP_RTO_MIN * 2)
#define MTP_SRTT_SHIFT 3

#define MTP_RETRY	15
#define MTP_PROBE_RETRY	6
#define MTP_SYN_RETRY	6

#define MTP_OPT_OFFSET_8 8
#define MTP_OPT_OFFSET_16 16
#define MTP_OPT_OFFSET_24 24
#define MTP_LOSS_RATE_MAX 1000
#define MTP_LOSS_PKT_NUM 2
#define MTP_LOSS_PROB_THRES 2

#define MTP_QUEUE_INDEX 5

enum mtp_enum {
	MTP_MSQ_THROTTLED,
	MTP_MSQ_QUEUED,
	MTP_MSQ_DEFERRED,	   /* mtp_tasklet_func() found socket was owned */
	MTP_WRITE_TIMER_DEFERRED,  /* mtp_write_timer() found socket was owned */
	MTP_DELACK_TIMER_DEFERRED, /* mtp_delack_timer() found socket was owned */
};

enum mtp_flags {
	MTPF_MSQ_THROTTLED		= (1UL << MTP_MSQ_THROTTLED),
	MTPF_MSQ_QUEUED			= (1UL << MTP_MSQ_QUEUED),
	MTPF_MSQ_DEFERRED		= (1UL << MTP_MSQ_DEFERRED),
	MTPF_WRITE_TIMER_DEFERRED	= (1UL << MTP_WRITE_TIMER_DEFERRED),
	MTPF_DELACK_TIMER_DEFERRED	= (1UL << MTP_DELACK_TIMER_DEFERRED),
};

#define MTP_DEFERRED_ALL (MTPF_MSQ_DEFERRED | \
			  MTPF_WRITE_TIMER_DEFERRED | \
			  MTPF_DELACK_TIMER_DEFERRED)

#define GET_NEXT_MTP_FRAME(skb) \
	((struct sk_buff **)((skb)->head))[0]

#define RESET_MTP_FRAME_PTR(skb) \
	((struct sk_buff **)((skb)->head))[0] = NULL

#define CACHE_MTP_FRAME(list, skb) \
	((struct sk_buff **)((list)->head))[0] = (skb)

static inline u32 mtp_get_rto(const struct mtp_sock *msk)
{
	if (msk->srtt_us == 0)
		return (MTP_RTO_MIN << 1);
	return usecs_to_jiffies((msk->srtt_us >> MTP_SRTT_SHIFT) + msk->rttvar_us);
}

/* At how many usecs into the future should the RTO fire? */
static inline s64 mtp_rto_delta_us(const struct sock *sk)
{
	const struct sk_buff *skb = mtp_write_queue_head(sk);
	u32 rto = mtp_sk(sk)->rtx.rto;
	u64 rto_time_stamp_us = mtp_skb_timestamp_us(skb) + jiffies_to_usecs(rto);

	return rto_time_stamp_us - mtp_sk(sk)->cur_mstamp;
}

void mtp_rearm_rto(struct sock *sk);
int mtp_tasklet_init(void);
void mtp_tasklet_destroy(void);
struct net_device *mtp_odev_get(struct mtp_sock *msk);
int mtp_retransmit_skb(struct sock *sk, struct sk_buff *skb);
void mtp_xmit_retransmit_queue(struct sock *sk);
void mtp_write_xmit(struct sock *sk, int push_one, gfp_t gfp);
bool mtp_schedule_loss_probe(struct sock *sk, bool advancing_rto);
void mtp_send_loss_probe(struct sock *sk);
void mtp_send_ack(struct sock *sk, u8 probe);
int mtp_send_syn(struct sock *sk);
void mtp_send_fin(struct sock *sk, gfp_t priority);
void mtp_send_synack(struct sock *sk, u32 seq, u32 ack_seq);
void mtp_send_reset(struct sock *sk, u32 seq, u32 ack_seq, gfp_t priority);
void mtp_reply_reset(struct sk_buff *skb);
void mtp_send_delayed_ack(struct sock *sk);
void mtp_release_cb(struct sock *sk);
void mtp_msq_handler(struct sock *sk);
int mtp_direct_xmit(struct sk_buff *skb);

#endif /* __MINTP_OUTPUT_H__ */
