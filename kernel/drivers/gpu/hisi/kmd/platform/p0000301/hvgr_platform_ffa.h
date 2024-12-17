/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HVGR_PLATFORM_FFA_H
#define HVGR_PLATFORM_FFA_H

#include <linux/types.h>
#include "hvgr_defs.h"

uint64_t hvgr_platform_sh_runtime(struct hvgr_device *gdev, unsigned char *buf, unsigned long size);

#endif
