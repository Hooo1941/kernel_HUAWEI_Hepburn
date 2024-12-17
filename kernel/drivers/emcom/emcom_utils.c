/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2023. All rights reserved.
 * Description: utils of xengine module
 * Author: lianghenghui lianghenghui@huawei.com
 * Create: 2017-07-24
 */

#include "emcom_utils.h"
#include <linux/inetdevice.h>
#include <linux/module.h>
#include <linux/types.h>

#undef HWLOG_TAG
#define HWLOG_TAG emcom_utils
HWLOG_REGIST();
MODULE_LICENSE("GPL");

emcom_support_enum g_modem_emcom_support = MODEM_NOT_SUPPORT_EMCOM;

void emcom_ind_modem_support(uint8_t en_support)
{
	emcom_logd("g_modem_emcom_support:%d\n", g_modem_emcom_support);
	g_modem_emcom_support = (emcom_support_enum)en_support;
}

bool emcom_is_modem_support(void)
{
	if (g_modem_emcom_support == MODEM_NOT_SUPPORT_EMCOM)
		return false;
	else
		return true;
}

bool emcom_is_valid_inetaddr(struct net_device *dev)
{
	struct in_device *in_dev = NULL;
	struct in_ifaddr *ifa = NULL;

	if (!dev) {
		emcom_logd("get inet addr dev is null");
		return false;
	}

	in_dev = __in_dev_get_rcu(dev);
	if (!in_dev) {
		emcom_logd("get inet addr in_dev is null dev: %s", dev->name);
		return false;
	}

	ifa = in_dev->ifa_list;
	if (ifa != NULL)
		return true;

	return false;
}

bool emcom_is_valid_dev_if(struct net *net, int ifindex)
{
	if ((ifindex != 0) && net) {
		struct net_device *dev = NULL;
		rcu_read_lock();
		dev = dev_get_by_index_rcu(net, ifindex);
		if (!dev || (dev->name[0] == '\0') || !emcom_is_valid_inetaddr(dev)) {
			rcu_read_unlock();
			return false;
		}
		rcu_read_unlock();
		return true;
	}

	return false;
}

