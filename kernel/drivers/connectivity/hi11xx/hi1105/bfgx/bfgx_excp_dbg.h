/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_excp_dbg.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_EXCP_DBG_H
#define BFGX_EXCP_DBG_H

/* 其他头文件包含 */
#include <linux/miscdevice.h>
#include "plat_type.h"

ssize_t bfgx_excp_dbg_show_comm(char *buf);
ssize_t bfgx_excp_dbg_store_comm(struct device *dev, const char *buf, size_t count);
#endif
