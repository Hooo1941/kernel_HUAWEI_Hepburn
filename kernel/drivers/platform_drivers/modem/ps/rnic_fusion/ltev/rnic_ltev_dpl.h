/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#ifndef RNIC_LTEV_DPL_H
#define RNIC_LTEV_DPL_H

#include <linux/skbuff.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LTEV_DPL_TX_MAX 1
#define LTEV_DPL_RX_MAX 1
#define LTEV_DPL_HEAD_LEN 32

struct ltev_dpl_info_s {
	u8  head[LTEV_DPL_HEAD_LEN]; /* default 32 bytes, decoded by ELF struct */
	u32 frag_info; /* fragment info */
};

struct ltev_dpl_tx_s {
	u8  ver;
	u8  rsv;
	u16 pkt_num;
	struct ltev_dpl_info_s pkt_info[LTEV_DPL_TX_MAX];
};

struct ltev_dpl_rx_s {
	u8  ver;
	u8  rsv;
	u16 pkt_num;
	struct ltev_dpl_info_s pkt_info[LTEV_DPL_RX_MAX];
};

struct ltev_dpl_s {
	struct ltev_dpl_tx_s tx;
	struct ltev_dpl_rx_s rx;
};

void ltev_dpl_tx(const struct sk_buff *skb);
void ltev_dpl_rx(const struct sk_buff *skb);
void ltev_dpl_rx_cmplt(void);
void ltev_dpl_init(void);
void ltev_dpl_exit(void);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_LTEV_DPL_H */
