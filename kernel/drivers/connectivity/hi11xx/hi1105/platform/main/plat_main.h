/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:plat_main.h header file
 * Author: @CompanyNameTag
 */

#ifndef PLAT_MAIN_H
#define PLAT_MAIN_H

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "sdt_drv.h"
#endif
#include "frw_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_MAIN_H

/* 函数声明 */
extern int32_t plat_init(void);
extern void plat_exit(void);

#endif
