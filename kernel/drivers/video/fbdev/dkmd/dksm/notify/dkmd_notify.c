/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "dkmd_notify.h"

#include <linux/export.h>

static BLOCKING_NOTIFIER_HEAD(dkmd_notifier_list);

int dkmd_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&dkmd_notifier_list, nb);
}

int dkmd_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&dkmd_notifier_list, nb);
}

int dkmd_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&dkmd_notifier_list, val, v);
}