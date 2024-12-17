/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 * Description: Contexthub 9030 driver.
 * Create: 2023-01-16
 */

#include <linux/types.h>
#include <platform_include/smart/linux/base/ap/protocol.h>
#include <linux/export.h>
#include "sts.h"

/* sync ipc */
int sts_reg_op(sts_reg_op_t *data)
{
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	return sts_do_reg_op(data);
#else
	return 0;
#endif
}
EXPORT_SYMBOL(sts_reg_op);