/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_VERSION_H
#define HVGR_VERSION_H

#define HVGR_DIRVER_NAME    "hvgr"
#define HVGR_TIMELINE_NAME  HVGR_DIRVER_NAME"_timeline"

#define HVGR_DRIVER_VER_MAJOR   2
#define HVGR_DRIVER_VER_MINOR   0

#define hvgr_version(v)         (v)

#define hvgr_version_lt(v)      (HVGR_VERSION_CODE < (v))
#define hvgr_version_ge(v)      (HVGR_VERSION_CODE >= (v))
#define hvgr_version_eq(v)      (HVGR_VERSION_CODE == (v))

#endif
