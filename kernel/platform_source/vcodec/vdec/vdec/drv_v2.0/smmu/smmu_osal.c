/*
 * smmu_osal.c
 *
 * This is for smmu linux osal.
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
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

#include <linux/iommu/mm_iommu.h>
#include "dbg.h"
#include "smmu_regs.h"
#include "smmu_osal.h"
#include "vfmw_ext.h"
#include "vcodec_vdec_regulator.h"


int32_t smmu_power_on_tcu(void)
{
	struct regulator *tcu_regulator =
		vdec_plat_get_entry()->regulator_info.regulators[SMMU_TCU_REGULATOR];

	return regulator_enable(tcu_regulator);
}

void smmu_power_off_tcu(void)
{
	int32_t ret;
	struct regulator *tcu_regulator =
		vdec_plat_get_entry()->regulator_info.regulators[SMMU_TCU_REGULATOR];

	ret = regulator_disable(tcu_regulator);
	if (ret)
		dprint(PRN_ERROR, "power off tcu failed\n");
}

int32_t smmu_map_reg_osal(struct smmu_entry *entry)
{
	UADDR reg_phy;
	uint8_t *reg_vaddr = VCODEC_NULL;
	vdec_dts *dts_info = &(vdec_plat_get_entry()->dts_info);

	reg_phy = dts_info->module_reg[MMU_SID_MOUDLE].reg_phy_addr;
	reg_vaddr = (uint8_t *)IO_REMAP_NO_CACHE(reg_phy, dts_info->module_reg[MMU_SID_MOUDLE].reg_range);
	if (!reg_vaddr) {
		dprint(PRN_ERROR, "map mmu sid reg failed\n");
		return SMMU_ERR;
	}

	entry->reg_info.smmu_sid_reg_vir = reg_vaddr;

	reg_phy = dts_info->module_reg[MMU_TBU_MODULE].reg_phy_addr;
	reg_vaddr = (uint8_t *)IO_REMAP_NO_CACHE(reg_phy, dts_info->module_reg[MMU_TBU_MODULE].reg_range);
	if (!reg_vaddr) {
		dprint(PRN_ERROR, "map mmu tbu reg failed\n");
		iounmap(entry->reg_info.smmu_sid_reg_vir);
		entry->reg_info.smmu_sid_reg_vir = VCODEC_NULL;
		return SMMU_ERR;
	}

	entry->reg_info.smmu_tbu_reg_vir = reg_vaddr;

	return SMMU_OK;
}

void smmu_unmap_reg_osal(struct smmu_entry *entry)
{
	if (entry->reg_info.smmu_tbu_reg_vir) {
		iounmap(entry->reg_info.smmu_tbu_reg_vir);
		entry->reg_info.smmu_tbu_reg_vir = VCODEC_NULL;
	}

	if (entry->reg_info.smmu_sid_reg_vir) {
		iounmap(entry->reg_info.smmu_sid_reg_vir);
		entry->reg_info.smmu_sid_reg_vir = VCODEC_NULL;
	}
}
