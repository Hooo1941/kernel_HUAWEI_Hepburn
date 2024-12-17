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

#include <linux/bitops.h>
#include <linux/rcupdate.h>
#include "securec.h"
#include "mdrv_pfa.h"
#include "ps_bmi_rnic_ioctl.h"
#include "rnic_log.h"
#include "rnic_vnet.h"
#include "rnic_vnet_hdlr.h"
#include "rnic_private.h"
#include "rnic_pkterr_i.h"
#include "rnic_dsm_msg_pif.h"
#include "rnic_vnet_cfg.h"

STATIC void rnic_vnet_eth_pfa_port_bind(struct rnic_vnet_priv_s *priv)
{
	struct wan_dev_info_s winfo = {
		.dev = priv->netdev,
		.get_fw_info = NULL,
	};
	mdrv_wan_dev_info_register(&winfo);
}

STATIC void rnic_vnet_eth_pfa_port_unbind(struct rnic_vnet_priv_s *priv)
{
	struct wan_dev_info_s winfo = {
		.dev = priv->netdev,
		.get_fw_info = NULL,
	};
	mdrv_wan_dev_info_deregister(&winfo);
}

STATIC void rnic_vnet_eth_rx_cb_regist(struct rnic_vnet_priv_s *priv, u32 pif_id)
{
	struct rnic_handle_s *handle = priv->handle;

	if (likely(handle->ops != NULL))
		handle->ops->rx_cb_register(pif_id, 1);
}

STATIC enum rnic_addr_type_e rnic_vnet_get_addrtype_by_fwtype(int fw_type)
{
	int fw_addr_type_map[PFA_FW_TYPE_END][2] = {
		{PFA_FW_TYPE_IPV4, RNIC_ADDR_IPV4},
		{PFA_FW_TYPE_IPV6, RNIC_ADDR_IPV6},
		{PFA_FW_TYPE_MAC,  RNIC_ADDR_ETH},
	};
	u32 i;

	for (i = 0; i < PFA_FW_TYPE_END; i++)
		if (fw_type == fw_addr_type_map[i][0])
			return fw_addr_type_map[i][1];

	return RNIC_ADDR_MAX;
}

STATIC void rnic_vnet_get_forward_info(struct net_device* dev, int fw_type, struct wan_info_s* info)
{
	struct rnic_vnet_priv_s *priv = NULL;
	int addr_type;

	if (dev == NULL || info == NULL || fw_type >= PFA_FW_TYPE_END)
		return;

	addr_type = rnic_vnet_get_addrtype_by_fwtype(fw_type);
	if (addr_type == RNIC_ADDR_MAX) {
		RNIC_LOGE("fw_type %d, is abnormal", fw_type);
		return;
	}

	(void)memset_s(info, sizeof(*info), 0, sizeof(*info));
	priv = netdev_priv(dev);
	info->info = priv->tft_infos[addr_type];
	info->userfield0 = priv->pif_id | (priv->drop_code << RNIC_WAN_DROP_SHIFT);
}

STATIC int rnic_vnet_direct_port_bind(const struct rnic_vnet_priv_s *priv)
{
	struct wan_dev_info_s winfo = {
		.dev = priv->netdev,
		.get_fw_info = rnic_vnet_get_forward_info,
	};
	return mdrv_wan_dev_info_register(&winfo);
}

STATIC int rnic_vnet_direct_port_unbind(const struct rnic_vnet_priv_s *priv)
{
	struct wan_dev_info_s winfo = {
		.dev = priv->netdev,
		.get_fw_info = NULL,
	};
	return mdrv_wan_dev_info_deregister(&winfo);
}

STATIC void rnic_vnet_ip_relay_port_bind(struct rnic_vnet_priv_s *priv)
{
	struct mdrv_pfa_port_info_s port_info = {0};
	int port_id;

	/* bind two times in pdu session ipv4v6, return in second bind */
	if (priv->port_id > -1) {
		RNIC_LOGE("pfa port is opened: %d", priv->port_id);
		return;
	}

	port_info.dev = priv->netdev;
	port_info.port_type = PFA_WWAN_RELAY;
	port_info.ops.rx = rnic_vnet_relay_xmit;
	port_info.ops.para = (void *)priv;

	port_id = mdrv_pfa_open_port(&port_info);
	if (port_id < 0) {
		RNIC_LOGE("open pfa port fail");
		return;
	}
	priv->port_id = port_id;
}

STATIC void rnic_vnet_ip_relay_port_unbind(struct rnic_vnet_priv_s *priv)
{
	if (priv->port_id >= 0) {
		mdrv_pfa_close_port(priv->port_id);
		priv->port_id = -1;
	}
}

void rnic_vnet_port_bind(struct rnic_vnet_priv_s *priv)
{
	if (test_bit(RNIC_ADDR_IPV4, &priv->link_state) ||
	    test_bit(RNIC_ADDR_IPV6, &priv->link_state))
		priv->fw_mode == RNIC_FW_MODE_DIRECT ? rnic_vnet_direct_port_bind(priv) :
						       rnic_vnet_ip_relay_port_bind(priv);
	else if (test_bit(RNIC_ADDR_ETH, &priv->link_state))
		rnic_vnet_eth_pfa_port_bind(priv);
	else if (test_bit(RNIC_ADDR_MBS, &priv->link_state))
		rnic_vnet_direct_port_bind(priv);
}

void rnic_vnet_port_unbind(struct rnic_vnet_priv_s *priv)
{
	if (test_bit(RNIC_ADDR_IPV4, &priv->link_state) ||
	    test_bit(RNIC_ADDR_IPV6, &priv->link_state))
		priv->fw_mode == RNIC_FW_MODE_DIRECT ? rnic_vnet_direct_port_unbind(priv) :
						       rnic_vnet_ip_relay_port_unbind(priv);
	else if (test_bit(RNIC_ADDR_ETH, &priv->link_state))
		rnic_vnet_eth_pfa_port_unbind(priv);
	else if (test_bit(RNIC_ADDR_MBS, &priv->link_state))
		rnic_vnet_direct_port_unbind(priv);
}

STATIC void rnic_vnet_eth_link_cfg(struct rnic_vnet_priv_s *priv)
{
	rnic_vnet_eth_rx_cb_regist(priv, priv->pif_id);
	priv->fill_desc = rnic_vnet_fill_eth_desc;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	rnic_vnet_eth_pfa_port_bind(priv);
}

STATIC void rnic_vnet_eth_link_rel(struct rnic_vnet_priv_s *priv)
{
	priv->fill_desc = NULL;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	rnic_vnet_eth_pfa_port_unbind(priv);
}

STATIC void rnic_vnet_ip_link_cfg(struct rnic_vnet_priv_s *priv)
{
	struct rnic_handle_s *handle = priv->handle;
	if (likely(handle->ops != NULL))
		handle->ops->rx_cb_register(priv->pif_id, 1);

	if (priv->frag_en)
		priv->fill_desc = rnic_vnet_fill_frag_desc;
	else
		priv->fill_desc = rnic_vnet_fill_ip_desc;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	priv->fw_mode == RNIC_FW_MODE_DIRECT ? rnic_vnet_direct_port_bind(priv) :
					       rnic_vnet_ip_relay_port_bind(priv);
}

STATIC void rnic_vnet_ip_link_rel(struct rnic_vnet_priv_s *priv)
{
	if (!test_bit(RNIC_ADDR_IPV4, &priv->link_state) &&
	    !test_bit(RNIC_ADDR_IPV6, &priv->link_state))
		priv->fill_desc = NULL;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	priv->fw_mode == RNIC_FW_MODE_DIRECT ? rnic_vnet_direct_port_unbind(priv) :
					       rnic_vnet_ip_relay_port_unbind(priv);
}

STATIC void rnic_vnet_mbs_link_cfg(struct rnic_vnet_priv_s *priv)
{
	struct rnic_handle_s *handle = priv->handle;
	if (likely(handle->ops != NULL))
		handle->ops->rx_cb_register(priv->pif_id, 1);

	priv->fill_desc = NULL;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	rnic_vnet_direct_port_bind(priv);
}

STATIC void rnic_vnet_mbs_link_rel(struct rnic_vnet_priv_s *priv)
{
	priv->fill_desc = NULL;

	if (test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return;

	rnic_vnet_direct_port_unbind(priv);
}

STATIC int rnic_vnet_link_cfg(struct rnic_vnet_priv_s *priv, const struct rnic_link_info_s *info)
{
	enum rnic_addr_type_e addr_type = info->type;

	mutex_lock(&priv->mutex);
	priv->copy_en = info->copy_en;
	priv->tft_infos[addr_type] = info->tft_info;

	if (addr_type == RNIC_ADDR_IPV4 || addr_type == RNIC_ADDR_IPV6)
		rnic_vnet_ip_link_cfg(priv);
	else if (addr_type == RNIC_ADDR_ETH)
		rnic_vnet_eth_link_cfg(priv);
	else
		rnic_vnet_mbs_link_cfg(priv);

	/*
	 * Attention:
	 * If set link state first, downlink pkt maybe send to tcpip before bind pfa port.
	 * Tcpip maybe set pfa ip entry use old pfa port when swith direct mode to relay mode.
	 * So here set link state after bind pfa port.
	 */
	set_bit(addr_type, &priv->link_state);
	mutex_unlock(&priv->mutex);
	return 0;
}

STATIC int rnic_vnet_link_rel(struct rnic_vnet_priv_s *priv, const struct rnic_link_info_s *info)
{
	enum rnic_addr_type_e type = info->type;

	mutex_lock(&priv->mutex);
	priv->tft_infos[type] = info->tft_info;
	clear_bit(type, &priv->link_state);

	if (type == RNIC_ADDR_IPV4 || type == RNIC_ADDR_IPV6)
		rnic_vnet_ip_link_rel(priv);
	else if (type == RNIC_ADDR_ETH)
		rnic_vnet_eth_link_rel(priv);
	else
		rnic_vnet_mbs_link_rel(priv);
	mutex_unlock(&priv->mutex);

	return 0;
}

void rnic_vnet_link_init(struct rnic_vnet_priv_s *priv)
{
	struct pfa_tft_info_s tft_info = {
		.pdu_session_id = RNIC_INVALID_SESSID,
		.fc_head        = RNIC_INVALID_FCHEAD,
		.modem_id       = RNIC_INVALID_MODEMID,
	};
	int i;

	for (i = 0; i < RNIC_ADDR_MAX; i++) {
		clear_bit(i, &priv->link_state);
		priv->tft_infos[i] = tft_info;
	}
}

int rnic_vnet_link_change(struct rnic_handle_s *handle, const struct rnic_link_info_s *info)
{
	struct rnic_vnet_ctx_s *vctx = handle->priv;

	if (info->link_up)
		rnic_vnet_link_cfg(vctx->privs[info->pif_id], info);
	else
		rnic_vnet_link_rel(vctx->privs[info->pif_id], info);

	return 0;
}

void rnic_vnet_feature_cfg(struct rnic_handle_s *handle, const void *cfg)
{
	struct rnic_vnet_ctx_s *vctx = handle->priv;

	if (vctx->srs_ops != NULL)
		vctx->srs_ops->set_feature_cfg(cfg);
}

void rnic_vnet_reset_notify(struct rnic_handle_s *handle)
{
	struct rnic_vnet_ctx_s *vctx = handle->priv;
	struct rnic_vnet_priv_s *priv = NULL;
	unsigned long long pifmask = handle->client->pif_bitmask;
	const unsigned long *bitmasks = (const unsigned long *)&pifmask;
	unsigned long bit;

	for_each_set_bit(bit, bitmasks, (unsigned long)RNIC_MAX_PIF_NUM) {
		priv = vctx->privs[bit];

		mutex_lock(&priv->mutex);
		rnic_vnet_port_unbind(priv);
		rnic_vnet_link_init(priv);
		mutex_unlock(&priv->mutex);

		priv->fill_desc = NULL;
	}
}
