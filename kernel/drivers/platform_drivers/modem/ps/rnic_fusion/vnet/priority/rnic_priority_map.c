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

#include <linux/rcupdate.h>
#include <linux/cache.h>
#include "ps_bmi_rnic.h"
#include "rnic_private.h"
#include "rnic_log.h"
#include "rnic_data.h"
#include "rnic_dma_map.h"
#include "rnic_vnet.h"

#define RNIC_PRIORITY_CACHE_LEN	128
#define RNIC_MAX_PRIORITY	2

STATIC struct rnic_priority_map_info __rcu *rnic_pri_map_info[RNIC_MAX_PIF_NUM] __read_mostly = { 0 };

int rnic_register_priority_map(struct net_device *netdev, struct rnic_priority_map_info *info)
{
	struct rnic_vnet_priv_s *priv = NULL;
	struct rnic_priority_map_info *new_info = NULL;
	struct rnic_priority_map_info *old_info = NULL;

	if (netdev == NULL || info == NULL) {
		RNIC_LOGE("netdev or info is null");
		return -EINVAL;
	}

	if (strncmp(netdev->name, RNIC_VNET_NAME, strlen(RNIC_VNET_NAME))) {
		RNIC_LOGE("netdev name: %s, not dest", netdev->name);
		return -EINVAL;
	}

	priv = netdev_priv(netdev);
	if (priv->pif_id >= RNIC_MAX_PIF_NUM) {
		RNIC_LOGE("netdev pif_id: %d", priv->pif_id);
		return -EINVAL;
	}

	new_info = kzalloc(sizeof(*new_info), GFP_KERNEL);
	if (new_info == NULL) {
		RNIC_LOGE("alloc priority_map_info failed");
		return -ENOMEM;
	}
	new_info->func = info->func;
	new_info->cache_size = min((u32)L1_CACHE_ALIGN(info->cache_size), (u32)RNIC_PRIORITY_CACHE_LEN);

	old_info = rnic_pri_map_info[priv->pif_id];
	rcu_assign_pointer(rnic_pri_map_info[priv->pif_id], new_info);
	synchronize_rcu();

	if (old_info != NULL)
		kfree(old_info);

	return 0;
}
EXPORT_SYMBOL(rnic_register_priority_map);

u32 rnic_priority_map(struct rnic_vnet_priv_s *priv, struct sk_buff *skb)
{
	struct rnic_priority_map_info *info = NULL;
	u32 priority = 0;
	bool cache_mdf = false;

	rcu_read_lock();
	info = rcu_dereference(rnic_pri_map_info[priv->pif_id]);
	if (info != NULL && info->func != NULL) {
		rnic_dma_unmap(priv->dev, skb, info->cache_size, DMA_FROM_DEVICE);
		cache_mdf = info->func(skb, &priority);
		rnic_dma_map(priv->dev, skb, info->cache_size,
			     cache_mdf ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

		if (priority > RNIC_MAX_PRIORITY)
			priority = 0;
	}
	rcu_read_unlock();

	return priority;
}

#ifndef LLT_OS_VER
void rnic_priority_show_info(void)
{
	struct rnic_priority_map_info *info = NULL;
	u32 i;

	for (i = 0; i < RNIC_MAX_PIF_NUM; i++) {
		rcu_read_lock();
		info = rcu_dereference(rnic_pri_map_info[i]);
		if (info != NULL) {
			RNIC_LOGE("rnic_priority_show_info %d, cachesize: %d", i, info->cache_size);
		}
		rcu_read_unlock();
	}
}
#endif
