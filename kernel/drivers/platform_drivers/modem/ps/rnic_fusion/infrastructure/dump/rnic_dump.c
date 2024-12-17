/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include <linux/tcp.h>
#include "securec.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "mdrv_om.h"
#include "mdrv_timer.h"
#include "rnic_dump.h"

#define RNIC_DUMP_BLK_NUM 256
#define RNIC_DUMP_MAX_SIZE (RNIC_DUMP_BLK_NUM * 16)
#define RNIC_DUMP_FLAG_MASK __constant_cpu_to_be32(0xFFFF0000)

#define rnic_dump_skb_len(len) __constant_cpu_to_be32(0x0000FFFF & (len))

struct rnic_dump_word4 {
	unsigned int word[4];
};

struct rnic_dump_info_s {
	unsigned short l4_sport;
	unsigned short l4_dport;
	unsigned int tcp_seq;
	unsigned int skb_count;
	unsigned short flags;
	unsigned short skb_len;
};

union rnic_tcp_dump_word_hdr {
	struct tcphdr th;
	struct rnic_dump_word4 words;
};

struct rnic_dump_ctx_s {
	u32 dump_count;
	struct rnic_dump_info_s dump_blk[RNIC_DUMP_BLK_NUM];
	unsigned char *dump_base;
};

STATIC struct rnic_dump_ctx_s *rnic_dpl_dump_ptr;

void rnic_save_tcp_skb_dump(struct tcphdr *th, u32 payload_len)
{
	struct rnic_dump_ctx_s *dump_ctx = rnic_dpl_dump_ptr;
	struct rnic_dump_word4 *words;
	unsigned int dcount, pos;

	dcount = dump_ctx->dump_count++;
	pos = dcount % RNIC_DUMP_BLK_NUM;
	words = (struct rnic_dump_word4 *)&dump_ctx->dump_blk[pos];

	*words = ((union rnic_tcp_dump_word_hdr *)(th))->words;

	words->word[2] = dcount;
	words->word[3] = (words->word[3] & RNIC_DUMP_FLAG_MASK) | rnic_dump_skb_len(payload_len);
}

STATIC void rnic_save_dump(void)
{
	struct rnic_dump_ctx_s *dump_ctx = rnic_dpl_dump_ptr;

	if (dump_ctx->dump_base == NULL) {
		RNIC_LOGE("dump field is NULL.");
		return;
	}

	(void)memcpy_s(dump_ctx->dump_base, RNIC_DUMP_MAX_SIZE,
		       dump_ctx->dump_blk, sizeof(dump_ctx->dump_blk));
}

bool rnic_register_dump_callback(void)
{
	struct rnic_dump_ctx_s *dump_ctx = NULL;

	dump_ctx = kzalloc(sizeof(*dump_ctx), GFP_KERNEL);
	if (dump_ctx == NULL) {
		RNIC_LOGE("alloc dump resource failed.");
		return false;
	}

	dump_ctx->dump_base = (unsigned char *)mdrv_om_register_field(OM_AP_RNIC,
		"rnic dump", RNIC_DUMP_MAX_SIZE, 0);
	if (dump_ctx->dump_base == NULL)
		RNIC_LOGE("register dump field is fail.");

	rnic_dpl_dump_ptr = dump_ctx;

	(void)mdrv_om_register_callback("rnic dump log", (dump_hook)rnic_save_dump);

	return true;
}