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

#ifndef __MINTP_RACK_H__
#define __MINTP_RACK_H__

#include "mintp.h"

/* Minimum RTT in usec. ~0 means not available. */
static inline u32 mtp_min_rtt(const struct mtp_sock *msk)
{
	return minmax_get(&msk->rtt_min);
}

int mtp_rack_identify_loss(struct sock *sk);
void mtp_verify_retransmit_hint(struct mtp_sock *msk, struct sk_buff *skb);
void mtp_rack_reo_timeout(struct sock *sk);
void mtp_rack_advance(struct mtp_sock *msk, u8 sacked, u32 end_seq,
		      u64 xmit_time);
#endif /* __MINTP_RACK_H__ */
