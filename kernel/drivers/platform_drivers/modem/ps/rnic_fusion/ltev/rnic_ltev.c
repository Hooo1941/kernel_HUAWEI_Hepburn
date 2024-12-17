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

#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include "mdrv_pcv.h"
#include "mdrv_pfa.h"
#include "rnic_data.h"
#include "rnic_client.h"
#include "rnic_ltev_dpl.h"
#include "rnic_fragment_i.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "rnic_ltev.h"

STATIC struct ltev_ctx_s *ltev_ctx_ptr;

STATIC void ltev_fill_desc(struct ltev_ctx_s *lctx, struct sk_buff *skb)
{
	struct wan_info_s *winfo = rnic_wan_skb_cb(skb);
	u8 frag_value = rnic_wan_frag_cb(skb)->value;

	winfo->info = lctx->tft_info;

	winfo->userfield2  = 0;
	winfo->userfield1  = frag_value;
	winfo->userfield0  = PS_IFACE_ID_LTEV | RNIC_WAN_FIELD1_MASK;

	skb->mac_len  = ETH_HLEN;
}

STATIC bool ltev_txhdr_validate(struct sk_buff *skb)
{
	u32 hdr_len;

	if (unlikely(skb->len < PC5_HDR_VER_LEN))
		return false;

	hdr_len = pc5_get_version(skb) == PC5_HDR_VER_1 ?
		  PC5_TXHDR_V1_LEN : PC5_TXHDR_V2_LEN;
	if (unlikely(skb->len < hdr_len ||
		     skb->len > hdr_len + PC5_MAX_PKT_LEN))
		return false;

	return true;
}

STATIC int ltev_output(struct ltev_ctx_s *lctx, struct sk_buff *skb)
{
	struct rnic_handle_s *handle = lctx->handle;

	ltev_fill_desc(lctx, skb);
	ltev_dpl_tx(skb);

	handle->ops->tx_data(handle, skb);
	lctx->stats.tx_pkts++;
	return 0;
}

STATIC int ltev_fragment_finish(struct sk_buff *skb, struct net_device *netdev)
{
	struct ltev_ctx_s *lctx = netdev_priv(netdev);

	if (unlikely(skb_headroom(skb) < ETH_HLEN)) {
		if (pskb_expand_head(skb, ETH_HLEN, 0, GFP_ATOMIC)) {
			lctx->stats.tx_headroom_errs++;
			goto err_free;
		}
	}

	skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	skb_set_network_header(skb, ETH_HLEN);
	eth_hdr(skb)->h_proto = skb->protocol;
	return ltev_output(lctx, skb);

err_free:
	dev_kfree_skb_any(skb);
	return -ENOMEM;
}

STATIC int ltev_xmit(struct sk_buff *skb)
{
	struct ltev_ctx_s *lctx = ltev_ctx_ptr;
	int ret = 0;

	if (unlikely(!ltev_txhdr_validate(skb))) {
		ret = -EINVAL;
		lctx->stats.tx_length_errs++;
		goto err_free;
	}

	skb->protocol = 0;
	rnic_set_mem_flag(skb, 0);
	return rnic_fragment(skb, lctx->netdev, ltev_fragment_finish);

err_free:
	dev_kfree_skb_any(skb);
	return ret;
}

STATIC int ltev_input(struct ltev_ctx_s *lctx, struct sk_buff *skb)
{
	mdrv_pcv_xmit(skb);
	lctx->stats.rx_pkts++;
	return 0;
}

STATIC int ltev_defrag_finish(struct sk_buff *skb, struct net_device *netdev)
{
	struct ltev_ctx_s *lctx = netdev_priv(netdev);

	return ltev_input(lctx, skb);
}

STATIC int ltev_rx_data(struct rnic_handle_s *handle, struct sk_buff *skb)
{
	struct ltev_ctx_s *lctx = handle->priv;

	dma_unmap_single(lctx->dev, virt_to_phys(skb->data),
			 skb->len, DMA_FROM_DEVICE);

	ltev_dpl_rx(skb);
	return rnic_defrag(skb, lctx->netdev, ltev_defrag_finish);
}

STATIC void ltev_rx_cmplt(struct rnic_handle_s *handle)
{
	ltev_dpl_rx_cmplt();
}

STATIC int ltev_init_inst(struct rnic_handle_s *handle)
{
	struct ltev_ctx_s *lctx = NULL;
	struct net_device *netdev = NULL;
	int ret;

	netdev = alloc_etherdev(sizeof(struct ltev_ctx_s));
	if (netdev == NULL) {
		RNIC_LOGE("alloc netdev failed");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(netdev, handle->dev);
	netdev->dev_port = PS_IFACE_ID_LTEV;
	scnprintf(netdev->name, IFNAMSIZ, "%s", LTEV_NAME);

	lctx = netdev_priv(netdev);
	lctx->handle = handle;
	lctx->netdev = netdev;
	lctx->dev = handle->dev;

	lctx->tft_info.pdu_session_id = RNIC_INVALID_SESSID;
	lctx->tft_info.fc_head        = PFA_TFT_LTEV_ULFC;
	lctx->tft_info.modem_id       = 0;
	lctx->tft_info.td_mode_en     = 1;
	lctx->tft_info.td_mode        = PFA_TD_MOD_COPY_ONLY;
	lctx->tft_info.v2x_ind        = 1;

	ret = rnic_frag_create(netdev);
	if (ret) {
		RNIC_LOGE("frag create failed, %d", ret);
		goto err;
	}

	ltev_ctx_ptr = lctx;
	ltev_dpl_init();
	mdrv_pcv_cb_register(ltev_xmit);

	handle->priv = lctx;
	handle->ops->rx_cb_register(PS_IFACE_ID_LTEV, 1);
	return 0;

err:
	free_netdev(netdev);
	return ret;
}

STATIC void ltev_deinit_inst(struct rnic_handle_s *handle)
{
	struct ltev_ctx_s *lctx = handle->priv;

	mdrv_pcv_cb_register(NULL);
	ltev_dpl_exit();

	rnic_frag_destory(lctx->netdev);
	free_netdev(lctx->netdev);
	ltev_ctx_ptr = NULL;
}

STATIC void ltev_report_pkt_stats(struct ltev_ctx_s *lctx)
{
	struct ltev_pkt_stats_rpt_s info = {{0}};

	/* version 102 */
	info.hdr.ver = 102;
	info.stats   = lctx->stats;
	rnic_trans_report(ID_DIAG_RNIC_LTEV_PKT_STATS, &info, sizeof(info));
}

STATIC void ltev_report_all_stats(struct rnic_handle_s *handle)
{
	struct ltev_ctx_s *lctx = handle->priv;

	ltev_report_pkt_stats(lctx);
	rnic_frag_report_stats(lctx->netdev);
}

STATIC const struct rnic_client_ops_s ltev_client_ops = {
	.init_inst	= ltev_init_inst,
	.deinit_inst	= ltev_deinit_inst,
	.rx_data	= ltev_rx_data,
	.rx_cmplt	= ltev_rx_cmplt,
	.rpt_stats	= ltev_report_all_stats,
};

STATIC struct rnic_client_s ltev_client = {
	.list		= LIST_HEAD_INIT(ltev_client.list),
	.pif_bitmask	= BIT_ULL(PS_IFACE_ID_LTEV),
	.frag_pifmask	= BIT_ULL(PS_IFACE_ID_LTEV),
	.type		= RNIC_CLIENT_LTEV,
	.ops		= &ltev_client_ops,
};

struct rnic_client_s *ltev_get_client(void)
{
	return &ltev_client;
}

void ltev_show_stats(void)
{
	struct ltev_stats_s *stats = &ltev_ctx_ptr->stats;

	pr_err("rx_packets                     %10u\n", stats->rx_pkts);
	pr_err("tx_packets                     %10u\n", stats->tx_pkts);
	pr_err("tx_length_errors               %10u\n", stats->tx_length_errs);
	pr_err("tx_headroom_errors             %10u\n", stats->tx_headroom_errs);
}
