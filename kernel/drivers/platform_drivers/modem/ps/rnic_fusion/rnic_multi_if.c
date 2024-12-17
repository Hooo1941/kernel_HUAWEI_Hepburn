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

#include <linux/socket.h>
#include "mdrv_pfa.h"
#include "rnic_private.h"
#include "rnic_multi_if.h"

STATIC void rnic_mif_get_tft_info(struct rnic_wwan_ctx_s *wctx,
	struct sk_buff *skb, struct rnic_mif_tft_rslt_s *tft_rslt)
{
	struct rx_cb_map_s *map = rnic_map_skb_cb(skb);

	switch (wctx->pfa_ver) {
	case PFA_VERSION_V0:
		tft_rslt->modem_id = map->pfa_tft_result.bits.modem_id;
		tft_rslt->pdu_session_id = map->pfa_tft_result.bits.pdu_session_id;
		tft_rslt->ip_type = map->pfa_tft_result.bits.ip_type;
		tft_rslt->pf_type = map->pfa_tft_result.bits.pf_type;
		tft_rslt->valid_bid_qosid = (map->pfa_tft_result.bits.bid_qosid != 0xff);
		break;
	case PFA_VERSION_V100:
		tft_rslt->modem_id = map->pfa_tft_result.bits_v100.modem_id;
		tft_rslt->pdu_session_id = map->pfa_tft_result.bits_v100.pdu_session_id;
		tft_rslt->ip_type = map->pfa_tft_result.bits_v100.ip_type;
		tft_rslt->pf_type = map->pfa_tft_result.bits_v100.pf_type;
		tft_rslt->valid_bid_qosid = (map->pfa_tft_result.bits_v100.bid_qosid != 0x7f);
		break;
	case PFA_VERSION_V200:
	default:
		break;
	}
}

STATIC void rnic_mif_fill_desc(struct sk_buff *skb, struct rnic_mif_tft_rslt_s *tft_rslt)
{
	struct rx_cb_map_s map = *rnic_map_skb_cb(skb);
	struct wan_info_s *winfo = rnic_wan_skb_cb(skb);
	struct pfa_tft_info_s tft_info = {0};

	tft_info.modem_id = tft_rslt->modem_id;
	tft_info.pdu_session_id = tft_rslt->pdu_session_id;
	tft_info.fc_head = rnic_get_fc_head(tft_rslt->modem_id);
	tft_info.td_mode_en = 1;
	tft_info.td_mode = PFA_TD_MOD_FILTER_ONLY;

	winfo->info = tft_info;
	winfo->userfield2 = 0;
	winfo->userfield1 = 0;
	winfo->userfield0 = map.userfield0 & RNIC_MIF_FIELD0_LB_MASK;
}

STATIC void rnic_mif_tx(struct sk_buff *skb, struct rnic_mif_tft_rslt_s *tft_rslt)
{
	rnic_mif_fill_desc(skb, tft_rslt);

	skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	skb_set_network_header(skb, ETH_HLEN);
	eth_hdr(skb)->h_proto = skb->protocol;
	skb->mac_len = ETH_HLEN;

	mdrv_wan_tx(skb, 0);
}

bool rnic_mif_need_redirect(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb)
{
	struct rnic_mif_tft_rslt_s tft = {0};

	rnic_mif_get_tft_info(wctx, skb, &tft);

	/* Should loop back ipv4 fragment packets(include ppp/ndis/vnet) and
	 * ppp packets.
	 */
	if ((tft.ip_type == 0 && tft.pf_type != 0) ||
	    (rnic_map_pifid(skb) == PS_IFACE_ID_PPP0))
		return true;

	return false;
}

void rnic_mif_correct_pifid(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb)
{
	struct rnic_mif_tft_rslt_s tft_rslt = {0};
	struct rx_cb_map_s *map = rnic_map_skb_cb(skb);
	u32 net_id = rnic_map_netid(skb);

	rnic_mif_get_tft_info(wctx, skb, &tft_rslt);

	/* Should replace the pifid with net_id when net_id is valid. */
	if (!(tft_rslt.ip_type == 0 && tft_rslt.pf_type != 0) &&
	    (tft_rslt.valid_bid_qosid && net_id < PS_IFACE_ID_BUTT)) {
		map->userfield0 &= ~RNIC_MAP_PIFID_MASK;
		map->userfield0 |= net_id;
	}
}

void rnic_mif_redirect(struct rnic_wwan_ctx_s *wctx, struct sk_buff *skb)
{
	struct rnic_mif_tft_rslt_s tft_rslt = {0};

	rnic_mif_get_tft_info(wctx, skb, &tft_rslt);
	skb->protocol = tft_rslt.ip_type == 0 ? htons(ETH_P_IP) : htons(ETH_P_IPV6);
	rnic_mif_tx(skb, &tft_rslt);
	wctx->stats.tx_mif_loop_pkts++;
	return;
}
