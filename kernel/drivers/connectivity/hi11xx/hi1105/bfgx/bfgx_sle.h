/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_platform_msg_handle.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_SLE_H
#define BFGX_SLE_H

/* 其他头文件包含 */
#include <linux/miscdevice.h>
#include "hw_bfg_ps.h"

/* 函数声明 */
struct bfgx_subsys_driver_desc *get_sle_default_drv_desc(void);
#endif /* __BFGX_SLE_H__ */
