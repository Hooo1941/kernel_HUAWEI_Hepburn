/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#ifndef _MODULE_TYPE_H_
#define _MODULE_TYPE_H_

enum module_type {
	MODULE_TYPE_KNL = 0,
	MODULE_TYPE_CTRL,
	MODULE_TYPE_STACK,
	MODULE_TYPE_FAST_CTRL,
	MODULE_TYPE_MAX
};

#endif

