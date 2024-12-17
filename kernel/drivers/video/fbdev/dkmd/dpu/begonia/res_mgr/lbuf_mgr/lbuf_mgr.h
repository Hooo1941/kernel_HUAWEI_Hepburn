/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef LBUF_MGR_H
#define LBUF_MGR_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/semaphore.h>
#include "dkmd_res_mgr.h"
#include "dkmd_log.h"

void dpu_res_register_lbuf_mgr(struct list_head *resource_head);
#endif