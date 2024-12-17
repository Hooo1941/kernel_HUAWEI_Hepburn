/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef RNIC_MULTI_IF_H
#define RNIC_MULTI_IF_H

#include "rnic_wwan.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_MIF_FIELD0_LB_MASK		(RNIC_MAP_PIFID_MASK | \
					 RNIC_MAP_PKTTYPE_MASK | \
					 RNIC_MAP_MULTIPIF_MASK)

#define rnic_get_fc_head(modem_id) \
	((modem_id) == 0 ? PFA_TFT_MODEM0_ULFC : (modem_id) ==  1 ? \
	 PFA_TFT_MODEM1_ULFC : PFA_TFT_MODEM2_ULFC)

struct rnic_mif_tft_rslt_s {
	unsigned int modem_id;
	unsigned int pdu_session_id;
	unsigned int ip_type;
	unsigned int pf_type;
	bool valid_bid_qosid;
};

bool rnic_mif_need_redirect(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb);
void rnic_mif_correct_pifid(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb);
void rnic_mif_redirect(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_MULTI_IF_H */
