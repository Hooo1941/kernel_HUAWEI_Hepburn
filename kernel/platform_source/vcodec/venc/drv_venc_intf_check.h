/*
 * drv_venc_intf_check.h
 *
 * This is for venc drv param check .
 *
 * Copyright (c) 2023-2023 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DRV_VENC_INTF_CHECK_H__
#define __DRV_VENC_INTF_CHECK_H__

#include "drv_common.h"

int32_t venc_check_encode_info(struct encode_info *encode_info);

#endif
