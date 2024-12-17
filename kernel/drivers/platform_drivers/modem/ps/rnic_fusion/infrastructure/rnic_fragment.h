/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RNIC_FRAGMENT_H
#define RNIC_FRAGMENT_H

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_FRAG_QUE_MAX_LEN	8

/*
 * fragment queue flags
 *
 * @RNIC_FRAG_FIRST_IN: first fragment has arrived
 * @RNIC_FRAG_LAST_IN: final fragment has arrived
 * @RNIC_FRAG_COMPLETE: frag queue has been processed
 */
enum {
	RNIC_FRAG_FIRST_IN	= BIT(0),
	RNIC_FRAG_LAST_IN	= BIT(1),
	RNIC_FRAG_COMPLETE	= BIT(2),
};

struct rnic_frag_stats_s {
	/* detailed fragment */
	u32 frag_oks;
	u32 frag_alloc_errs;

	/* detailed reasm */
	u32 reasm_oks;
	u32 reasm_seq_errs;
	u32 reasm_order_errs;
	u32 reasm_alloc_errs;
};

struct rnic_frag_pkt_stats_rpt_s {
	struct {
		u8 ver;
		u8 reserved[7];
	} hdr;
	char name[IFNAMSIZ];
	struct rnic_frag_stats_s stats;
};

struct rnic_frag_queue_s {
	struct list_head list;
	struct net_device *netdev;

	struct rnic_frag_stats_s stats;
	struct sk_buff_head frag_list;
	unsigned long flags;
	u8 seq_next;
};

#ifdef __cplusplus
}
#endif

#endif /* RNIC_FRAGMENT_H */
