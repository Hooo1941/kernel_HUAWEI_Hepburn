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

#ifndef __MINTP_TIMER_H__
#define __MINTP_TIMER_H__

#include "mintp.h"

#define MTP_RTX_TIMER_RETRANS		1 /* Retransmit timer */
#define MTP_RTX_TIMER_LOSS_PROBE	2 /* Tail loss probe timer */
#define MTP_RTX_TIMER_REO_TIMEOUT	3 /* Reordering timer */
#define MTP_RTX_TIMER_PROBE		4 /* Zero window probe timer */

#define MTP_KEEPALIVE_TIMER		5 /* keepalive timer */

#define MTP_THIN_PACKETS_OUT 4
#define MTP_THIN_LINEAR_RETRIES 6

static inline bool mtp_stream_is_thin(struct mtp_sock *msk)
{
	return msk->packets_out < MTP_THIN_PACKETS_OUT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
typedef struct timer_list *mtp_timer_ptr;
typedef void (*timer_func)(mtp_timer_ptr);
static inline void mtp_setup_timer(struct timer_list *timer, timer_func func)
{
	timer_setup(timer, func, 0);
}
#else
typedef unsigned long mtp_timer_ptr;
typedef void (*timer_func)(mtp_timer_ptr);
static inline void mtp_setup_timer(struct timer_list *timer, timer_func func)
{
	setup_timer(timer, func, (unsigned long)timer);
}
#endif

void mtp_init_xmit_timers(struct sock *sk);
void mtp_clear_xmit_timers(struct sock *sk);
void mtp_delack_timer_handler(struct sock *sk);
void mtp_write_timer_handler(struct sock *sk);

#endif /* __mtp_TIMER_H__ */
