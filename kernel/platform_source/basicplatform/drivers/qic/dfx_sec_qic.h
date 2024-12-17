/*
 * QIC Mntn Module.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_SEC_QIC_H
#define __DFX_SEC_QIC_H

#if defined(CONFIG_DFX_SEC_QIC_V100)
#include "qic_v100/dfx_sec_qic_init.h"
#elif defined(CONFIG_DFX_SEC_QIC_V300)
#include "qic_v300/dfx_sec_qic_init.h"
#else
#endif

#define QIC_MODULE_NAME "DFX_SEC_QIC"
#define QIC_REG_FIELD_NAME_SIZE    32

struct qic_src_reg {
	void __iomem *reg_base;
	u32 *reg_mask;
	u32 *offsets;
	u32 len;
};

struct dfx_sec_qic_device *dfx_sec_qic_get_dev(void);
int dfx_sec_qic_get_component_reg(const struct device_node *qic_node, struct platform_device *pdev,
				     struct qic_src_reg **component_reg, u32 *component_reg_num, const char *name);

#endif

