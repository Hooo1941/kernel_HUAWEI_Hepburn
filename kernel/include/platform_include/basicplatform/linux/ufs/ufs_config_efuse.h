/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: ufs bootrom high speed efuse set header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __UFS_CONFIG_EFUSE_H__
#define __UFS_CONFIG_EFUSE_H__

#include <linux/types.h>

#define ERR_UFS_EFUSE_RW_FAIL 1
#define ERR_UFS_NOT_HISI_CHARGER_IC 2

u32 ufs_bootrom_mode_sel_efuse_set(void);

#endif /* __UFS_CONFIG_EFUSE_H__ */