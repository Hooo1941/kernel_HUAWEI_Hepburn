/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : main.c 的头文件
 * 作    者 : wifi
 * 创建日期 : 2014年10月10日
 */

#ifndef MAIN_H
#define MAIN_H

#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_H

int32_t mpxx_host_main_init(void);
void mpxx_host_main_exit(void);

#endif
