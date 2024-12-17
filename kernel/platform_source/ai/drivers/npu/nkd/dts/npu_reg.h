/*
 * npu_reg.h
 *
 * about npu reg
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
#ifndef __NPU_REG_H
#define __NPU_REG_H

#include <linux/platform_device.h>
#include "npu_platform.h"

#define DRV_NPU_POWER_OFF_FLAG    0
#define DRV_NPU_POWER_ON_FLAG     0x22222222
/* the power status is only for v200.
 * C4 is totally different with 1A and 2B in binary
 */
#define DRV_NPU_POWER_ON_SEC_FLAG 0x44444444

int npu_plat_map_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info);

int npu_plat_unmap_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info);

int npu_plat_parse_reg_desc(struct platform_device *pdev,
	struct npu_platform_info *plat_info);

int npu_plat_parse_dump_region_desc(const struct device_node *root,
	struct npu_platform_info *plat_info);

void npu_plat_set_npu_power_status(u32 status);
u32 npu_plat_get_npu_power_status(void);

#endif
