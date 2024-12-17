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

#include "ps_bmi_rnic_ioctl.h"
#include "rnic_vnet_cfg.h"
#include "rnic_private.h"



int rnic_vnet_set_drop_time_stub(u32 vnet_id, u8 code)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);

	if (priv == NULL)
		return -EINVAL;

	if (priv->drop_code != code) {
		priv->drop_code = code;
	}

	return 0;
}

static void rnic_vnet_link_stub(const struct rnic_vnet_priv_s *priv, bool link_up,
				u8 addr_type, u8 sess_id, u8 modem_id)
{
	struct rnic_link_info_s info = {0};

	info.pif_id  = priv->pif_id;
	info.link_up = link_up;
	info.type    = (enum rnic_addr_type_e)addr_type;
	info.tft_info.pdu_session_id = sess_id;
	info.tft_info.modem_id       = modem_id;

	rnic_vnet_link_change(priv->handle, &info);
}

int rnic_vnet_link_up_stub(u32 vnet_id, u8 addr_type, u8 sess_id, u8 modem_id)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);

	if (priv == NULL)
		return -EINVAL;

	rnic_vnet_link_stub(priv, true, addr_type, sess_id, modem_id);
	return 0;
}

int rnic_vnet_link_down_stub(u32 vnet_id, u8 addr_type, u8 sess_id, u8 modem_id)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);

	if (priv == NULL)
		return -EINVAL;

	rnic_vnet_link_stub(priv, false, addr_type, sess_id, modem_id);
	return 0;
}

int rnic_vnet_show_drop_time(u32 vnet_id)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);

	if (priv == NULL)
		return -EINVAL;

	pr_err("vnet_id %d drop time is %d\n", vnet_id, priv->drop_code);
	return 0;
}

int rnic_vnet_show_pkt_stats(u32 vnet_id)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);
	u64 tx_drops = 0;
	u64 rx_drops = 0;

	if (priv == NULL)
		return -EINVAL;

	tx_drops += priv->stats.tx.tx_linear_errs;
	tx_drops += priv->stats.tx.tx_csum_errs;
	tx_drops += priv->stats.tx.tx_len_errs;
	tx_drops += priv->stats.tx.tx_link_errs;
	tx_drops += priv->stats.tx.tx_carr_errs;
	tx_drops += priv->stats.tx.tx_clone_errors;

	rx_drops += priv->stats.rx.rx_len_errs;
	rx_drops += priv->stats.rx.rx_link_errs;
	rx_drops += priv->stats.rx.rx_carr_errs;
	rx_drops += priv->stats.rx.rx_queue_errs;
	rx_drops += priv->stats.rx.rx_maa_errs;

	pr_err("[RMNET%d] tx_bytes        %llu\n", vnet_id, priv->stats.tx.tx_bytes);
	pr_err("[RMNET%d] tx_pkts         %llu\n", vnet_id, priv->stats.tx.tx_pkts);
	pr_err("[RMNET%d] tx_drops        %llu\n", vnet_id, tx_drops);
	pr_err("[RMNET%d] tx_linear_errs  %u\n", vnet_id, priv->stats.tx.tx_linear_errs);
	pr_err("[RMNET%d] tx_csum_errs    %u\n", vnet_id, priv->stats.tx.tx_csum_errs);
	pr_err("[RMNET%d] tx_len_errs     %u\n", vnet_id, priv->stats.tx.tx_len_errs);
	pr_err("[RMNET%d] tx_link_errs    %u\n", vnet_id, priv->stats.tx.tx_link_errs);
	pr_err("[RMNET%d] tx_carr_errs    %u\n", vnet_id, priv->stats.tx.tx_carr_errs);
	pr_err("[RMNET%d] tx_clone_errors %u\n", vnet_id, priv->stats.tx.tx_clone_errors);
	pr_err("[RMNET%d] rx_bytes        %llu\n", vnet_id, priv->stats.rx.rx_bytes);
	pr_err("[RMNET%d] rx_pkts         %llu\n", vnet_id, priv->stats.rx.rx_pkts);
	pr_err("[RMNET%d] rx_drops        %llu\n", vnet_id, rx_drops);
	pr_err("[RMNET%d] rx_len_errs     %u\n", vnet_id, priv->stats.rx.rx_len_errs);
	pr_err("[RMNET%d] rx_link_errs    %u\n", vnet_id, priv->stats.rx.rx_link_errs);
	pr_err("[RMNET%d] rx_carr_errs    %u\n", vnet_id, priv->stats.rx.rx_carr_errs);
	pr_err("[RMNET%d] rx_queue_errs   %u\n", vnet_id, priv->stats.rx.rx_queue_errs);
	pr_err("[RMNET%d] rx_maa_errs     %u\n", vnet_id, priv->stats.rx.rx_maa_errs);

	return 0;
}

int rnic_vnet_show_state_info(u32 vnet_id)
{
	struct rnic_vnet_priv_s *priv = rnic_vnet_get_priv(vnet_id);

	if (priv == NULL)
		return -EINVAL;

	pr_err("[RMNET%d] link state     %lu\n", vnet_id, priv->vnet_state);
	pr_err("[RMNET%d] carrier state  %lu\n", vnet_id, priv->link_state);
	pr_err("[RMNET%d] forward mode   %u\n",  vnet_id, priv->fw_mode);
	pr_err("[RMNET%d] pfa port id    %d\n",  vnet_id, priv->port_id);
	return 0;
}
