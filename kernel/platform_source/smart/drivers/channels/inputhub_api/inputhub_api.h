/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: Contexthub IPC send api.
 * Create: 2017-06-08
 */

#ifndef __INPUTHUB_API_H__
#define __INPUTHUB_API_H__
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/err.h>
#include <platform_include/smart/linux/base/ap/protocol.h>
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include <platform_include/smart/linux/iomcu_ipc.h>

#define CONTEXTHUB_HEADER_SIZE (sizeof(struct pkt_header) + sizeof(unsigned int))

#endif
