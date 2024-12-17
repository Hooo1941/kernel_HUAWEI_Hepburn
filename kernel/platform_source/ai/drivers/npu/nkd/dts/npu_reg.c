/*
 * npu_reg.c
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
#include "npu_reg.h"

#include <linux/io.h>
#include <securec.h>
#include "npu_log.h"

#define NPU_DUMP_REGION_NAME     "dump_region"

static u32 *npu_plat_get_reg_vaddr_offset(u32 reg_idx, u32 offset)
{
	struct npu_platform_info *plat_info = NULL;

	if (reg_idx >= NPU_REG_MAX_RESOURCE) {
		npu_drv_err("reg_idx = %x is error\n", reg_idx);
		return NULL;
	}

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_info failed\n");
		return NULL;
	}
	return (u32 *)((u8 *)plat_info->dts_info.reg_vaddr[reg_idx] + offset);
}

int npu_plat_map_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	int i;
	void __iomem *base = NULL;
	struct npu_mem_desc *mem_desc = NULL;
	int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_POWER_STATUS_BASE + 1;
	for (i = 0; i < max_regs; i++) {
		mem_desc = &plat_info->dts_info.reg_desc[i];
		npu_drv_debug("resource: base %pK\n",
			(void *)(uintptr_t)(u64)mem_desc->base);
		npu_drv_debug("resource: mem_desc.len %llx\n", mem_desc->len);

		if (mem_desc->base == 0 || mem_desc->len == 0) {
			plat_info->dts_info.reg_vaddr[i] = NULL;
			continue;
		}

		base = devm_ioremap(&pdev->dev, mem_desc->base, mem_desc->len);
		if (base == NULL) {
			npu_drv_err("platform_get_resource failed i = %d\n", i);
			goto map_platform_reg_fail;
		}
		plat_info->dts_info.reg_vaddr[i] = base;
	}

	return 0;
map_platform_reg_fail:
	(void)memset_s(plat_info->dts_info.reg_desc,
		sizeof(plat_info->dts_info.reg_desc),
		0, sizeof(plat_info->dts_info.reg_desc));
	(void)npu_plat_unmap_reg(pdev, plat_info);
	return -1;
}

int npu_plat_unmap_reg(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	int i;
	int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_POWER_STATUS_BASE + 1;

	for (i = 0; i < max_regs; i++) {
		if (plat_info->dts_info.reg_vaddr[i] == NULL)
			continue;

		devm_iounmap(&pdev->dev, plat_info->dts_info.reg_vaddr[i]);
	}
	return 0;
}

int npu_plat_parse_dump_region_desc(const struct device_node *root,
	struct npu_platform_info *plat_info)
{
	int ret;
	struct npu_mem_desc *mem_desc = &plat_info->dts_info.dump_region_desc[0];

	if (mem_desc == NULL) {
		npu_drv_err("dump_region desc is NULL");
		return -1;
	}
	ret = of_property_read_u64_array(root, NPU_DUMP_REGION_NAME,
		(u64 *)mem_desc,
		NPU_DUMP_REGION_MAX * (sizeof(struct npu_mem_desc) / sizeof(u64)));
	if (ret < 0) {
		npu_drv_warn("dump_regs base is not set, dump_regs is disabled\n");
		return ret;
	}
	npu_drv_debug("dump region: base=0x%llx len=0x%llx", mem_desc->base,
		mem_desc->len);
	return 0;
}

int npu_plat_parse_reg_desc(struct platform_device *pdev,
	struct npu_platform_info *plat_info)
{
	unsigned int i;
	struct resource *info = NULL;
	struct npu_mem_desc *mem_desc = NULL;
	unsigned int max_regs = NPU_REG_MAX_RESOURCE;

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_HWTS] == 0)
		max_regs = NPU_REG_POWER_STATUS_BASE + 1;
	for (i = 0; i < max_regs; i++) {
		info = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (info == NULL || info->end <= info->start) {
			npu_drv_err("info is invalid, i = %d\n", i);
			return -1;
		}
		mem_desc = &plat_info->dts_info.reg_desc[i];
		mem_desc->base = info->start;
		mem_desc->len = info->end - info->start + 1;
	}

	return 0;
}

u32 npu_plat_get_npu_power_status(void)
{
	u32 *addr = npu_plat_get_reg_vaddr_offset(NPU_REG_POWER_STATUS_BASE, 0);

	if (addr != NULL) {
		return (u32)readl(addr);
	} else {
		npu_drv_warn("invalid power status addr\n");
		return 0;
	}
}

void npu_plat_set_npu_power_status(u32 status)
{
	u32 *addr = npu_plat_get_reg_vaddr_offset(NPU_REG_POWER_STATUS_BASE, 0);

	if (addr != NULL)
		writel(status, addr);
	else
		npu_drv_warn("invalid power status addr\n");
}
EXPORT_SYMBOL(npu_plat_set_npu_power_status);
