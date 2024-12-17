/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Amax interface file.
 * Author: zhouxiang68@huawei.com
 * Create: 2023-03-31
 */

#ifndef _AMAX_MULTILINK_INTERFACE_H
#define _AMAX_MULTILINK_INTERFACE_H

#include <linux/types.h>
#include <linux/if.h>
#include <linux/inet.h>

typedef int32_t (*amax_tx_hook_func)(struct sk_buff *skb, struct net_device **dev);
typedef int32_t (*amax_tx_complete_func)(const struct sk_buff *net_buf);
typedef int32_t (*amax_rx_func_hook)(struct sk_buff *skb);

typedef int32_t (*hmac_tx_func)(struct sk_buff *netbuf, struct net_device *net_dev);
typedef int32_t (*hmac_rx_func)(struct sk_buff *netbuf, struct net_device *net_dev);

/* hmac驱动调用 */
extern void hmac_hook_driver_reg(hmac_tx_func tx_fun, hmac_rx_func rx_fun);
extern void hmac_hook_driver_unreg(void);
extern int32_t amax_hook_tx_data(struct sk_buff *netbuf, struct net_device **net_dev);
extern int32_t amax_hook_tx_complete(const struct sk_buff *netbuf);
extern int32_t amax_hook_rx_data(struct sk_buff *netbuf);

/* MultiLink驱动调用 */
extern void amax_hook_reg(amax_tx_hook_func tx_func, amax_tx_complete_func tx_cpt_func, amax_rx_func_hook rx_fun);
extern void amax_hook_unreg(void);
extern void hmac_hook_driver_tx_send(struct sk_buff *netbuf, struct net_device *net_dev);
extern void hmac_hook_driver_netif_rx(struct sk_buff *netbuf, struct net_device *net_dev);

#endif /* _AMAX_MULTILINK_INTERFACE_H */