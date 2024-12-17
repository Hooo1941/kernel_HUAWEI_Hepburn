/*
 * smmu_reg.c
 *
 * This is for smmu reg desc.
 *
 * Copyright (c) 2019-2022 Huawei Technologies CO., Ltd.
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

#include "vfmw_ext.h"
#include "smmu_osal.h"
#include "smmu_regs.h"

struct smmu_entry *smmu_get_entry(void)
{
	static struct smmu_entry g_smmu_entry;
	return &g_smmu_entry;
}

void set_tbu_reg(uint32_t addr, uint32_t val, uint32_t bw, uint32_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	rd_smmu_tbu_vreg(addr, tmp);
	tmp &= ~(mask << bs);
	wr_smmu_tbu_vreg(addr, tmp | ((val & mask) << bs));
}

void set_sid_reg(uint32_t addr, uint32_t val, uint32_t bw, uint32_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	rd_smmu_sid_vreg(addr, tmp);
	tmp &= ~(mask << bs);
	wr_smmu_sid_vreg(addr, tmp | ((val & mask) << bs));
}