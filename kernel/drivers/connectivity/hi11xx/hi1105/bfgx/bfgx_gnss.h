/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: bfgx_gnss.c header file
 * Author: @CompanyNameTag
 */

#ifndef BFGX_GNSS_H
#define BFGX_GNSS_H
/* 其他头文件包含 */
#include <linux/miscdevice.h>
#include "hw_bfg_ps.h"

/* 宏定义 */
#define DTS_COMP_HI110X_PS_ME_NAME "hisilicon,hisi_me"

/* 函数声明 */
struct bfgx_subsys_driver_desc *get_gnss_default_drv_desc(void);
#endif /* __BFGX_GNSS_H__ */
