/*
 *
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_IOCTL_H
#define HVGR_IOCTL_H

#include "linux/types.h"

#define HVGR_IOCTL_MODULE_DATAN               0x50u
#define HVGR_IOCTL_MODULE_DEV                 0x51u
#define HVGR_IOCTL_MODULE_PM                  0x52u
#define HVGR_IOCTL_MODULE_MEM                 0x53u
#define HVGR_IOCTL_MODULE_SCH                 0x54u

/* submodule of sch */
#define HVGR_IOCTL_SCH_SUB_MODULE_SCHED       0u
#define HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB     128u

/* 2^HVGR_IOCTL_SCH_SUB_MODULE_SPLIT = HVGR_IOCTL_SCH_SUB_MODULE_SOFTJOB */
#define HVGR_IOCTL_SCH_SUB_MODULE_SPLIT       7u

#endif
