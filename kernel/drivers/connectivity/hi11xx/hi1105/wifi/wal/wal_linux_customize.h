/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : wal_linux_customize.c 的头文件
 * 作    者 : wifi3
 * 创建日期 : 2020年7月17日
 */

#ifndef WAL_LINUX_CUSTOMIZE_H
#define WAL_LINUX_CUSTOMIZE_H

#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CUSTOMIZE_H

uint32_t hwifi_force_refresh_rf_params(oal_net_device_stru *pst_net_dev);

#endif
