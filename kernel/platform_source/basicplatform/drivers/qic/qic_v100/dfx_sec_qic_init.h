/*
 * QIC V100 init functions head file.
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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
#ifndef __DFX_SEC_QIC_INIT_H
#define __DFX_SEC_QIC_INIT_H
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "../dfx_sec_qic.h"

#define QIC_TIMEOUT_EN_OFFSET      16
#define QIC_DEFAULT_MID_OFFSET     1
#define GIC_CTRL_SIZE     0x200

struct dfx_sec_qic_device {
	u32 irq_number;
	u32 *need_stop_cpu_buses;
	u32 need_stop_cpu_buses_num;
	struct qic_src_reg *irq_reg;
	u32 irq_reg_num;
	struct qic_src_reg *timeout_en_reg;
	u32 timeout_en_reg_num;
	struct qic_bus_info *bus_info;
	u32 bus_info_num;
	struct qic_master_info *mid_info;
	u32 mid_info_num;
	struct qic_master_info *acpu_core_info;
	u32 acpu_core_info_num;
	u64 *qic_share_base;
	void __iomem *gic_ctrl_base;
};

struct qic_bus_info {
	u32 bus_key;
	const char *bus_name;
	u32 mid_offset;
};

struct qic_master_info {
	u32 masterid_value;
	const char *masterid_name;
};

int dfx_sec_qic_smc_call(u64 function_id, u64 bus_key);
int dfx_sec_qic_common_init(const struct device_node *np, struct platform_device *pdev);
void dfx_sec_qic_free_source(struct dfx_sec_qic_device *qic_dev);
void qice_regs_print_init(void);
#endif
