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

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_tun.h"

STATIC int rnic_tun_dev_init(struct net_device *netdev)
{
	struct rnic_tun_priv *priv = netdev_priv(netdev);

	priv->pcpu_stats = netdev_alloc_pcpu_stats(struct rnic_tun_pcpu_stats);
	if (priv->pcpu_stats == NULL)
		return -ENOMEM;

	return 0;
}

STATIC void rnic_tun_dev_uninit(struct net_device *netdev)
{
	struct rnic_tun_priv *priv = netdev_priv(netdev);

	free_percpu(priv->pcpu_stats);
}

STATIC netdev_tx_t rnic_tun_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct rnic_tun_priv *priv = netdev_priv(netdev);
	struct rnic_tun_pcpu_stats *pcpu_ptr = this_cpu_ptr(priv->pcpu_stats);

	u64_stats_update_begin(&pcpu_ptr->syncp);
	pcpu_ptr->tx_pkts++;
	pcpu_ptr->tx_bytes += skb->len;
	u64_stats_update_end(&pcpu_ptr->syncp);

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

STATIC void rnic_tun_get_stats64(struct net_device *netdev,
				 struct rtnl_link_stats64 *stats)
{
	struct rnic_tun_priv *priv = netdev_priv(netdev);
	int cpu;

	for_each_possible_cpu(cpu) {
		const struct rnic_tun_pcpu_stats *pcpu_ptr;
		u64 total_pkts, total_bytes;
		unsigned int start;

		pcpu_ptr = per_cpu_ptr(priv->pcpu_stats, cpu);
		do {
			start = u64_stats_fetch_begin_irq(&pcpu_ptr->syncp);
			total_pkts  = pcpu_ptr->tx_pkts;
			total_bytes = pcpu_ptr->tx_bytes;
		} while (u64_stats_fetch_retry_irq(&pcpu_ptr->syncp, start));

		stats->tx_packets += total_pkts;
		stats->tx_bytes   += total_bytes;
	}
}


STATIC const char rnic_tun_names[][IFNAMSIZ] = {
	"rmnet_tun00", "rmnet_tun01", "rmnet_tun02", "rmnet_tun03", "rmnet_tun04",
	"rmnet_tun10", "rmnet_tun11", "rmnet_tun12", "rmnet_tun13", "rmnet_tun14",
};
STATIC const u32 rnic_tun_num = ARRAY_SIZE(rnic_tun_names);

STATIC struct net_device **rnic_tun_pnetdevs;

STATIC const struct net_device_ops rnic_tun_ops = {
	.ndo_init		= rnic_tun_dev_init,
	.ndo_uninit		= rnic_tun_dev_uninit,
	.ndo_start_xmit		= rnic_tun_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_get_stats64	= rnic_tun_get_stats64,
};

STATIC int rnic_tun_init_one(const char *name, struct net_device **pnetdev)
{
	struct net_device *netdev = NULL;
	int err;

	netdev = alloc_etherdev(sizeof(struct rnic_tun_priv));
	if (netdev == NULL)
		return -ENOMEM;

	scnprintf(netdev->name, IFNAMSIZ, "%s", name);
	eth_hw_addr_random(netdev);
	netdev->netdev_ops = &rnic_tun_ops;

	err = register_netdev(netdev);
	if (err)
		goto err;

	*pnetdev = netdev;
	return 0;

err:
	free_netdev(netdev);
	return err;
}

STATIC void rnic_tun_cleanup(void)
{
	struct net_device *netdev = NULL;
	u32 i;

	if (rnic_tun_pnetdevs == NULL)
		return;

	for (i = 0; i < rnic_tun_num; i++) {
		netdev = rnic_tun_pnetdevs[i];
		if (netdev != NULL) {
			unregister_netdev(netdev);
			free_netdev(netdev);
		}
	}

	kfree(rnic_tun_pnetdevs);
	rnic_tun_pnetdevs = NULL;
}

int rnic_tun_module_init(void)
{
	u32 i;
	int err = 0;

	rnic_tun_pnetdevs = kcalloc(rnic_tun_num, sizeof(struct net_device *),
				    GFP_KERNEL);
	if (rnic_tun_pnetdevs == NULL) {
		RNIC_LOGE("alloc buffer failed");
		err = -ENOMEM;
		goto err_alloc_dev;
	}

	for (i = 0; i < rnic_tun_num && !err; i++)
		err = rnic_tun_init_one(rnic_tun_names[i], &rnic_tun_pnetdevs[i]);

	if (err) {
		RNIC_LOGE("init netdev failed, %d", err);
		goto err_init_dev;
	}

	return 0;

err_init_dev:
	rnic_tun_cleanup();
err_alloc_dev:
	return err;
}

void rnic_tun_module_exit(void)
{
	rnic_tun_cleanup();
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(rnic_tun_module_init);
module_exit(rnic_tun_module_exit);
#endif
