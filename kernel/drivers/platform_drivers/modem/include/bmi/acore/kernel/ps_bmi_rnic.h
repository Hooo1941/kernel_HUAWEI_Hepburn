/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * foss@huawei.com
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
 *
 */

#ifndef PS_BMI_RNIC_H
#define PS_BMI_RNIC_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "product_config_ps_ap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rnic_pktcap_fn)(struct sk_buff *skb, unsigned int param, void *priv);

/**
 * @brief register packet capture callback func.
 * @attention  Since the third parameter of the current callback function
 *    (void (*rnic_pktcap_fn)(struct sk_buff *skb, unsigned int param, void *priv)) is not used,
 *    it will be confirmed after discussing with the caller of the registered function.
 *    There is no need to provide an additional third parameter(void *priv),
 *    and directly fill in NULL for processing.
 * @param[in] param, user self param.
 * @param[in] func, the packet capture callback func.
 */
#ifdef CONFIG_RNIC_PKTCAP
int rnic_register_pktcap(unsigned int param, rnic_pktcap_fn func);
#else
static inline int rnic_register_pktcap(unsigned int param, rnic_pktcap_fn func)
{
	return 0;
}
#endif

typedef void (*rnic_wakeup_fn)(void);

/**
 * @brief Please register wakeup callback func if you want to change wake up hold mechanism.
 *    Default wake up hold for 500ms after data packets are received.
 * @attention The callback may be called in the interrupt\soft interrupt\work queue context.
 * @param[in] wakeup, the wakeup callback func.
 */
#ifdef CONFIG_RNIC_WAKEUP
void rnic_register_wakeup(rnic_wakeup_fn wakeup);
#else
static inline void rnic_register_wakeup(rnic_wakeup_fn wakeup) { return; }
#endif

/**
 * @brief Type of priority map function.
 * @attention
 * @param[in] skb, the skb to be maped.
 * @param[out] priority, return the maped priority, Value range:[0-2], default is lowest priority: 0.
 * @retval true: the skb data cache was modified.
 *        false: the skb data cache was not modified.
 */
typedef bool (*rnic_priority_map_fn)(struct sk_buff *skb, unsigned int *priority);

/**
 * @attention
 * cache_size indicate max size of skb data cache that can be modified by rnic_priority_map_fn func.
 * cache_size must be an integer multiple of L1_CACHE_BYTES and must be less than 128.
 */
struct rnic_priority_map_info {
	rnic_priority_map_fn func;
	unsigned int cache_size;
};

/**
 * @brief Register priority map information for the specified device.
 * @attention
 * @param[in] netdev, nic dev.
 * @param[in] info, priority map info.
 * @retval 0: register success.
 *  non zero: register fail, maybe incorrect parameter or memory alloc fail.
 */
#ifdef CONFIG_RNIC_PRIORITY_MAP
int rnic_register_priority_map(struct net_device *netdev, struct rnic_priority_map_info *info);
#else
static inline int rnic_register_priority_map(struct net_device *netdev, struct rnic_priority_map_info *info)
{
	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* PS_BMI_RNIC_H */
