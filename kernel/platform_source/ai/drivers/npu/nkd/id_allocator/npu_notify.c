/*
 * npu_notify.c
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
#include "npu_notify.h"
#include <linux/errno.h>
#include "npu_log.h"
#include "npu_common.h"
#include "npu_user_common.h"

struct npu_id_entity *npu_alloc_notify(u8 dev_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_error(dev_id >= NPU_DEV_NUM, NULL,
		"invalid id\n");
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, NULL, "cur_dev_ctx is null\n");
	id_allocator = &(cur_dev_ctx->id_allocator[NPU_ID_TYPE_NOTIFY]);

	return npu_id_allocator_alloc(id_allocator);
}

int npu_free_notify(u8 dev_id, u32 notify_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct npu_id_allocator *id_allocator = NULL;

	cond_return_error(dev_id >= NPU_DEV_NUM, -EINVAL,
		"invalid id\n");
	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx is null\n");
	id_allocator = &(cur_dev_ctx->id_allocator[NPU_ID_TYPE_NOTIFY]);

	return npu_id_allocator_free(id_allocator, notify_id);
}