/*
 * ufshcd_hufs_extend.h
 *
 * The hufs extend interface for ufshcd.c
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __UFSHCD_HUFS_EXTEND_H__
#define __UFSHCD_HUFS_EXTEND_H__

#include "ufshcd.h"

#define QUERY_DESC_SNUM_XHFZ               6
#define QUERY_DESC_SNUM_XHFZ_SEC           18
#define XHFZ_SNUM_NUM_INDEX_FIRST          4
#define XHFZ_SNUM_NUM_INDEX_SEC            12
#define UFS_VENDOR_XHFZ                    0xA6B
#define UFS_VENDOR_ZX                      0xAEB
#define QUERY_DESC_SNUM_YMTC               (-9)
#define YMTC_SNUM_NUM_INDEX_FIRST          4
#define YMTC_SNUM_NUM_INDEX_SEC            6
#define UFS_VENDOR_YMTC                    0xA9B
#define UFS_VENDOR_CWHC                    0xFFFF

int hufs_get_device_info(struct ufs_hba *hba, struct ufs_dev_desc *dev_desc,
			     u8 *desc_buf);
void ufshcd_host_ops_register(struct ufs_hba *hba);
#endif
