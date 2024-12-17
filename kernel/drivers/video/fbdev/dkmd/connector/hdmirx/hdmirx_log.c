/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include "hdmirx_log.h"


uint32_t g_hdmirx_log_level = HDMIRX_LOG_LVL_MAX;
#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_named(hdmirx_debug_msg_level, g_hdmirx_log_level, uint, 0640);
MODULE_PARM_DESC(hdmirx_debug_msg_level, "hdmirx msg level");
#endif

void set_hdmirx_debug_level(uint32_t level)
{
	g_hdmirx_log_level = level;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL_GPL(set_hdmirx_debug_level);
#endif
