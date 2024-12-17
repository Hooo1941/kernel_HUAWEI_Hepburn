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

#include <linux/skbuff.h>
#include <linux/static_key.h>
#include "securec.h"
#include "mdrv_diag.h"
#include "rnic_data.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "netfilter_ips_mntn.h"
#include "rnic_ltev_dpl.h"

STATIC struct ltev_dpl_s *ltev_dpl_ptr = NULL;
STATIC struct static_key ltev_dpl_key __read_mostly = STATIC_KEY_INIT_FALSE;

STATIC void ltev_dpl_key_set(bool val)
{
	RNIC_LOGI("set ltev dpl key to %d", val);

	if (val)
		static_key_enable(&ltev_dpl_key);
	else
		static_key_disable(&ltev_dpl_key);
}

STATIC void ltev_dpl_diag_notify_func(mdrv_diag_state_e state)
{
	if (state == DIAG_STATE_DISCONN)
		ltev_dpl_key_set(false);
}

STATIC void ltev_dpl_tx_trans(struct ltev_dpl_tx_s *tx)
{
	u32 size = sizeof(*tx) - (LTEV_DPL_TX_MAX - tx->pkt_num) *
		   sizeof(struct ltev_dpl_info_s);

	rnic_trans_report(ID_DIAG_RNIC_LTEV_DPL_TX, tx, size);
}

STATIC void ltev_dpl_rx_trans(struct ltev_dpl_rx_s *rx)
{
	u32 size = sizeof(*rx) - (LTEV_DPL_RX_MAX - rx->pkt_num) *
		   sizeof(struct ltev_dpl_info_s);

	rnic_trans_report(ID_DIAG_RNIC_LTEV_DPL_RX, rx, size);
}

STATIC void ltev_dpl_tx_rec(const struct sk_buff *skb, struct ltev_dpl_info_s *pinfo)
{
	unsigned char *data = NULL;
	u32 size = sizeof(pinfo->head);
	int ret;

	if (rnic_wan_frag_cb(skb)->first) {
		data = skb->data + skb->mac_len;
		if (size > skb->len - ETH_HLEN)
			size = skb->len - ETH_HLEN;

		ret = memcpy_s(pinfo->head, sizeof(pinfo->head), data, size);
		if (ret != EOK)
			RNIC_LOGE("memcpy failed, dst_len %lu, src_len %u",
				  sizeof(pinfo->head), size);
	} else {
		(void)memset_s(pinfo->head, sizeof(pinfo->head), 0, sizeof(pinfo->head));
	}

	pinfo->frag_info = rnic_wan_skb_cb(skb)->userfield1;
}

STATIC void ltev_dpl_rx_rec(const struct sk_buff *skb, struct ltev_dpl_info_s *pinfo)
{
	unsigned char *data = NULL;
	u32 size = sizeof(pinfo->head);
	int ret;

	if (rnic_map_frag_cb(skb)->first) {
		data = skb->data;
		if (size > skb->len)
			size = skb->len;

		ret = memcpy_s(pinfo->head, sizeof(pinfo->head), data, size);
		if (ret != EOK)
			RNIC_LOGE("memcpy failed, dst_len %lu, src_len %u",
				  sizeof(pinfo->head), size);
	} else {
		(void)memset_s(pinfo->head, sizeof(pinfo->head), 0, sizeof(pinfo->head));
	}

	pinfo->frag_info = rnic_map_skb_cb(skb)->userfield1;
}

void ltev_dpl_tx(const struct sk_buff *skb)
{
	struct ltev_dpl_tx_s *tx = NULL;

	if (!static_key_false(&ltev_dpl_key))
		return;

	tx = &ltev_dpl_ptr->tx;
	ltev_dpl_tx_rec(skb, &tx->pkt_info[tx->pkt_num]);

	if (++tx->pkt_num >= LTEV_DPL_TX_MAX) {
		ltev_dpl_tx_trans(tx);
		tx->pkt_num = 0;
	}
}

void ltev_dpl_rx(const struct sk_buff *skb)
{
	struct ltev_dpl_rx_s *rx = NULL;

	if (!static_key_false(&ltev_dpl_key))
		return;

	rx = &ltev_dpl_ptr->rx;
	ltev_dpl_rx_rec(skb, &rx->pkt_info[rx->pkt_num]);

	if (++rx->pkt_num >= LTEV_DPL_RX_MAX) {
		ltev_dpl_rx_trans(rx);
		rx->pkt_num = 0;
	}
}

void ltev_dpl_rx_cmplt(void)
{
	struct ltev_dpl_rx_s *rx = NULL;

	if (!static_key_false(&ltev_dpl_key))
		return;

	rx = &ltev_dpl_ptr->rx;

	if (rx->pkt_num != 0) {
		ltev_dpl_rx_trans(rx);
		rx->pkt_num = 0;
	}
}

void ltev_dpl_init(void)
{
	ltev_dpl_ptr =  kzalloc(sizeof(*ltev_dpl_ptr), GFP_KERNEL);
	if (ltev_dpl_ptr == NULL) {
		RNIC_LOGE("alloc ltev dpl failed");
		return;
	}

	(void)mdrv_diag_conn_state_notify_fun_reg(ltev_dpl_diag_notify_func);
	IPSMNTN_RegTraceCfgNotify(ltev_dpl_key_set);

	/* version 100 */
	ltev_dpl_ptr->tx.ver = 100;
	/* version 100 */
	ltev_dpl_ptr->rx.ver = 100;
}

void ltev_dpl_exit(void)
{
	if (ltev_dpl_ptr == NULL)
		return;

	kfree(ltev_dpl_ptr);
	ltev_dpl_ptr = NULL;
}
