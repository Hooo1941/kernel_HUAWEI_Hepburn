/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: 前端依赖的外部OS接口.
 * Create: 2023-02-06
 */

#ifndef FE_DEPEND_LINUX_OS_IF_H
#define FE_DEPEND_LINUX_OS_IF_H

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#endif
#include "hisi_ini.h"

#endif /* end of __FE_DEPEND_LINUX_OS_IF_H__ */