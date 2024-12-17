/*
 * npu_arch_exception.h
 *
 * about npu proc hwts exception
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_ARCH_EXCEPTION_H
#define __NPU_ARCH_EXCEPTION_H

#include "npu_common.h"
#include "npu_except_mgr.h"

int npu_exception_svc_init(struct npu_dev_ctx *dev_ctx);
void npu_exception_svc_deinit(struct npu_dev_ctx *dev_ctx);
int npd_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er);

#endif