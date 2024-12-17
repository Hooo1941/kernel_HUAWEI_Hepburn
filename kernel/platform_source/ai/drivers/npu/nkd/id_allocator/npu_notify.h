/*
 * npu_notify.h
 *
 * about npu notify
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __NPU_NOTIFY_H
#define __NPU_NOTIFY_H

#include <linux/types.h>
#include "npu_id_allocator.h"

struct npu_id_entity *npu_alloc_notify(u8 dev_id);

int npu_free_notify(u8 dev_id, u32 notify_id);

#endif
