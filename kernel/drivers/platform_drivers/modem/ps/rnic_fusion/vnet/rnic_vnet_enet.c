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

#include <net/sock.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/capability.h>
#include <linux/uaccess.h>
#include "mdrv_diag.h"
#include "bastet_rnic_interface.h"
#include "rnic_vnet.h"
#include "rnic_vnet_cfg.h"
#include "rnic_vnet_hdlr.h"
#include "rnic_vnet_ioctl.h"
#include "rnic_vnet_dpl_i.h"
#include "rnic_fragment_i.h"
#include "rnic_log.h"
#include "rnic_private.h"
#include "ps_bmi_rnic_ioctl.h"

STATIC struct rnic_vnet_ctx_s *rnic_vnet_ctx_ptr;
void (*rnic_gro_qack_thold_update_entry)(struct sock *) __rcu __read_mostly = NULL;
void (*rnic_gro_rpt_stats_entry)(void) __rcu __read_mostly = NULL;

struct rnic_vnet_priv_s *rnic_vnet_get_priv(u32 vnet_id)
{
	u32 pif_id = vnet_id + RNIC_PIFID_OFFSET;

	if (pif_id >= RNIC_MAX_PIF_NUM)
		return NULL;

	return rnic_vnet_ctx_ptr->privs[pif_id];
}

STATIC void rnic_vnet_qack_thold_update(struct sk_buff *skb)
{
	void (*qack_update)(struct sock *) = NULL;

	rcu_read_lock();
	qack_update = rcu_dereference(rnic_gro_qack_thold_update_entry);
	if (qack_update != NULL)
		qack_update(skb->sk);
	rcu_read_unlock();
}

STATIC int rnic_vnet_open(struct net_device *netdev)
{
	struct rnic_vnet_priv_s *priv = netdev_priv(netdev);

	mutex_lock(&priv->mutex);
	clear_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state);
	netif_start_queue(netdev);

	if ((priv->link_state & RNIC_ADDRF_ALL) == 0)
		goto out;

	rnic_vnet_port_bind(priv);

out:
	mutex_unlock(&priv->mutex);
	return 0;
}

STATIC int rnic_vnet_stop(struct net_device *netdev)
{
	struct rnic_vnet_priv_s *priv = netdev_priv(netdev);

	mutex_lock(&priv->mutex);
	set_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state);
	netif_stop_queue(netdev);

	if ((priv->link_state & RNIC_ADDRF_ALL) == 0)
		goto out;

	rnic_vnet_port_unbind(priv);

out:
	mutex_unlock(&priv->mutex);
	return 0;
}

STATIC netdev_tx_t rnic_vnet_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct rnic_vnet_priv_s *priv = netdev_priv(netdev);

	/* Tweak shift value to 4, means TSQ budget is ~64 ms of data. */
	sk_pacing_shift_update(skb->sk, 4);

	/* Tweak quick ack threshold. */
	rnic_vnet_qack_thold_update(skb);

	rnic_set_mem_flag(skb, 0);
	rnic_vnet_tx_data(priv, skb);
	return NETDEV_TX_OK;
}

STATIC int rnic_vnet_do_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct rnic_vnet_priv_s *priv = netdev_priv(netdev);
	int ret;

	if (ifr->ifr_ifru.ifru_data == NULL)
		return -EINVAL;

	switch (cmd) {
	case RNIC_IOCTL_PRIVATE:
		ret = rnic_vnet_ioctl_priv_handler(priv, ifr->ifr_ifru.ifru_data);
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

STATIC void rnic_vnet_get_stats64(struct net_device *netdev,
				  struct rtnl_link_stats64 *stats)
{
	struct rnic_vnet_priv_s *priv = netdev_priv(netdev);
	u64 rx_errs = 0;
	u64 tx_errs = 0;

	rx_errs += priv->stats.rx.rx_len_errs;
	rx_errs += priv->stats.rx.rx_link_errs;
	rx_errs += priv->stats.rx.rx_carr_errs;
	rx_errs += priv->stats.rx.rx_queue_errs;
	rx_errs += priv->stats.rx.rx_maa_errs;

	tx_errs += priv->stats.tx.tx_linear_errs;
	tx_errs += priv->stats.tx.tx_csum_errs;
	tx_errs += priv->stats.tx.tx_len_errs;
	tx_errs += priv->stats.tx.tx_link_errs;
	tx_errs += priv->stats.tx.tx_carr_errs;
	tx_errs += priv->stats.tx.tx_clone_errors;

	stats->rx_packets = priv->stats.rx.rx_pkts;
	stats->rx_bytes   = priv->stats.rx.rx_bytes;
	stats->tx_packets = priv->stats.tx.tx_pkts;
	stats->tx_bytes   = priv->stats.tx.tx_bytes;

	stats->rx_errors  = rx_errs;
	stats->rx_dropped = rx_errs;
	stats->tx_errors  = tx_errs;
	stats->tx_dropped = tx_errs;
}

int rnic_vnet_set_features(struct net_device *netdev, netdev_features_t features)
{
	return 0;
}

STATIC const struct net_device_ops rnic_vnet_netdev_ops = {
	.ndo_open		= rnic_vnet_open,
	.ndo_stop		= rnic_vnet_stop,
	.ndo_start_xmit		= rnic_vnet_start_xmit,
	.ndo_do_ioctl		= rnic_vnet_do_ioctl,
	.ndo_get_stats64	= rnic_vnet_get_stats64,
	.ndo_set_features	= rnic_vnet_set_features,
};

STATIC void rnic_vnet_set_name(struct rnic_vnet_priv_s *priv, u32 idx)
{
	char *table[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
			  "10", "11", "_d2d", "_mbs", "_ims00", "_ims10", "_emc0",
			  "_emc1", "_r_ims00", "_r_ims01", "_r_ims10",
			  "_r_ims11" };

	if (idx < ARRAY_SIZE(table))
		scnprintf(priv->netdev->name, IFNAMSIZ,
			  "%s%s", RNIC_VNET_NAME, table[idx]);
}

STATIC int rnic_vnet_set_frag_cap(struct rnic_vnet_priv_s *priv,
				  unsigned long long frag_mask)
{
	int ret = 0;

	if (!test_bit((int)priv->pif_id, (const unsigned long *)&frag_mask))
		goto out;

	ret = rnic_frag_create(priv->netdev);
	if (ret)
		goto out;

	priv->netdev->max_mtu = RNIC_MAX_MTU;
	priv->frag_en = true;
out:
	return ret;
}

STATIC int rnic_vnet_init_netdev(struct rnic_vnet_ctx_s *vctx, u32 pif_id, u32 idx)
{
	struct rnic_client_s *client = vctx->handle->client;
	struct rnic_vnet_priv_s *priv = NULL;
	struct net_device *netdev = NULL;
	int ret;

	netdev = alloc_etherdev(sizeof(struct rnic_vnet_priv_s));
	if (netdev == NULL) {
		RNIC_LOGE("alloc netdev %d failed", idx);
		return -ENOMEM;
	}

	netdev->netdev_ops = &rnic_vnet_netdev_ops;
	netdev->flags &= ~((u32)(IFF_BROADCAST | IFF_MULTICAST));
	netdev->features |= (u32)NETIF_F_SG | NETIF_F_GRO_FRAGLIST |
				 NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	netdev->hw_features |= netdev->features;
	netdev->mtu = RNIC_DEFAULT_MTU;
	netdev->dev_port = pif_id;
	eth_hw_addr_random(netdev);

	priv = netdev_priv(netdev);
	priv->handle = vctx->handle;
	priv->netdev = netdev;
	priv->dev = vctx->handle->dev;
	priv->back = vctx;
	set_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state);

	priv->pif_id = pif_id;
	priv->vnet_id = idx;
	priv->fw_mode = RNIC_FW_MODE_DIRECT;
	priv->port_id = -1;
	mutex_init(&priv->mutex);
	rnic_vnet_set_name(priv, idx);
	rnic_vnet_link_init(priv);

	ret = rnic_vnet_set_frag_cap(priv, client->frag_pifmask);
	if (ret) {
		RNIC_LOGE("set netdev %d frag cap failed, %d", idx, ret);
		goto err;
	}

	ret = register_netdev(netdev);
	if (ret) {
		RNIC_LOGE("register netdev %d failed, %d", idx, ret);
		goto err;
	}

	vctx->privs[pif_id] = priv;
	return 0;

err:
	free_netdev(netdev);
	return ret;
}

STATIC int rnic_vnet_create(struct rnic_vnet_ctx_s *vctx)
{
	const struct rnic_client_s *client = vctx->handle->client;
	const unsigned long *bitmasks = NULL;
	unsigned long bit;
	u32 idx = 0;
	int ret;

	bitmasks = (const unsigned long *)&client->pif_bitmask;
	for_each_set_bit(bit, bitmasks, (unsigned long)RNIC_MAX_PIF_NUM) {
		ret = rnic_vnet_init_netdev(vctx, (u32)bit, idx);
		if (ret)
			return ret;
		idx++;
	}

	return idx == 0 ? -EFAULT : 0;
}

STATIC void rnic_vnet_cleanup(struct rnic_vnet_ctx_s *vctx)
{
	struct rnic_vnet_priv_s *priv = NULL;
	u32 i;

	for (i = 0; i < RNIC_MAX_PIF_NUM; i++) {
		priv = vctx->privs[i];
		if (priv != NULL) {
			rnic_frag_destory(priv->netdev);
			unregister_netdev(priv->netdev);
			free_netdev(priv->netdev);
			vctx->privs[i] = NULL;
		}
	}
}

STATIC int rnic_vnet_init_inst(struct rnic_handle_s *handle)
{
	struct rnic_vnet_ctx_s *vctx = NULL;
	int ret;

	vctx = kzalloc(sizeof(*vctx), GFP_KERNEL);
	if (vctx == NULL) {
		RNIC_LOGE("alloc resource failed");
		return -ENOMEM;
	}

	rnic_vnet_ctx_ptr = vctx;
	vctx->handle = handle;

	ret = rnic_vnet_create(vctx);
	if (ret)
		goto err;

	rnic_vnet_dpl_init(handle->dev, (u8)handle->pfa_ver);

	__skb_queue_head_init(&vctx->rx_list);
	handle->priv = vctx;
	return 0;

err:
	rnic_vnet_cleanup(vctx);
	kfree(vctx);
	rnic_vnet_ctx_ptr = NULL;
	return ret;
}

STATIC void rnic_vnet_deinit_inst(struct rnic_handle_s *handle)
{
	struct rnic_vnet_ctx_s *vctx = handle->priv;

	rnic_vnet_dpl_exit();
	rnic_vnet_cleanup(vctx);
	kfree(vctx);
	rnic_vnet_ctx_ptr = NULL;
}

STATIC bool rnic_vnet_link_online(struct rnic_vnet_priv_s *priv)
{
	if (!test_bit(RNIC_VNET_STATE_DOWN, &priv->vnet_state))
		return true;
	if ((priv->link_state & RNIC_ADDRF_ALL) != 0)
		return true;
	return false;
}

STATIC void rnic_vnet_report_pkt_stats(struct rnic_vnet_priv_s *priv)
{
	struct rnic_vnet_pkt_stats_rpt_s info = {{0}};

	info.hdr.ver = 102; /* version 102 */
	info.hdr.id  = (u8)priv->vnet_id;
	info.stats   = priv->stats;
	rnic_trans_report(ID_DIAG_RNIC_VNET_PKT_STATS, &info, sizeof(info));
}

STATIC void rnic_vnet_report_gro_stats(void)
{
	void (*gro_rpt)(void) = NULL;

	rcu_read_lock();
	gro_rpt = rcu_dereference(rnic_gro_rpt_stats_entry);
	if (gro_rpt != NULL)
		gro_rpt();
	rcu_read_unlock();
}

STATIC void rnic_vnet_report_all_stats(struct rnic_handle_s *handle)
{
	struct rnic_vnet_ctx_s *vctx = handle->priv;
	struct rnic_vnet_priv_s *priv = NULL;
	unsigned long long pifmask = handle->client->pif_bitmask;
	const unsigned long *bitmasks = (const unsigned long *)&pifmask;
	unsigned long bit;

	rnic_vnet_report_gro_stats();

	for_each_set_bit(bit, bitmasks, (unsigned long)RNIC_MAX_PIF_NUM) {
		priv = vctx->privs[bit];
		if (!rnic_vnet_link_online(priv))
			continue;
		rnic_vnet_report_pkt_stats(priv);
		rnic_frag_report_stats(priv->netdev);
	}

	if (vctx->srs_ops != NULL)
		vctx->srs_ops->rpt_stats();
}

STATIC const struct rnic_client_ops_s rnic_vnet_client_ops = {
	.init_inst	= rnic_vnet_init_inst,
	.deinit_inst	= rnic_vnet_deinit_inst,
	.rx_data	= rnic_vnet_rx_data,
	.rx_cmplt	= rnic_vnet_rx_cmplt,
	.link_change	= rnic_vnet_link_change,
	.feature_cfg	= rnic_vnet_feature_cfg,
	.reset_notify	= rnic_vnet_reset_notify,
	.rpt_stats	= rnic_vnet_report_all_stats,
};

STATIC struct rnic_client_s rnic_vnet_client = {
	.list		= LIST_HEAD_INIT(rnic_vnet_client.list),
	.pif_bitmask	= RNIC_VNET_PIF_MASK,
	.frag_pifmask	= RNIC_VNET_FRAG_MASK,
	.type		= RNIC_CLIENT_VNET,
	.ops		= &rnic_vnet_client_ops,
};

int rnic_bst_get_modem_info(struct net_device *netdev, struct bst_rnic_modem_info *info)
{
	struct rnic_vnet_priv_s *priv = NULL;

	if (netdev == NULL || netdev->netdev_ops != &rnic_vnet_netdev_ops)
		return -EINVAL;

	if (info == NULL)
		return -EINVAL;

	priv = netdev_priv(netdev);
	if (test_bit(RNIC_ADDR_IPV4, &priv->link_state)) {
		info->ipv4_act = 1;
		info->modem_id = priv->tft_infos[RNIC_ADDR_IPV4].modem_id;
		info->ipv4_rab_id = priv->tft_infos[RNIC_ADDR_IPV4].pdu_session_id;
	}

	if (test_bit(RNIC_ADDR_IPV6, &priv->link_state)) {
		info->ipv6_act = 1;
		info->modem_id = priv->tft_infos[RNIC_ADDR_IPV6].modem_id;
		info->ipv6_rab_id = priv->tft_infos[RNIC_ADDR_IPV6].pdu_session_id;
	}

	return 0;
}

struct rnic_client_s *rnic_vnet_get_client(void)
{
	return &rnic_vnet_client;
}
