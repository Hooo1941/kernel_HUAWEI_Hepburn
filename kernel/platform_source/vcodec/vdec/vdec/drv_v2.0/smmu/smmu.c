/*
 * smmu.c
 *
 * This is for smmu driver.
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
#include "dbg.h"
#include "smmu.h"
#include "smmu_regs.h"
#include "smmu_osal.h"
#include "smmu_tbu.h"

int32_t smmu_map_reg(void)
{
	struct smmu_entry *entry = smmu_get_entry();
	(void)memset_s(entry, sizeof(*entry), 0, sizeof(*entry));
	return smmu_map_reg_osal(entry);
}
void smmu_unmap_reg(void)
{
	struct smmu_entry *entry = smmu_get_entry();
	smmu_unmap_reg_osal(entry);
}

static int32_t smmu_init_reg(struct smmu_entry *entry)
{
	int32_t ret;
	uint32_t reg_index;
	uint32_t reg_base_offset;
	uint32_t tbu_num = entry->tbu_info.mmu_tbu_num;
	uint32_t one_tbu_offset = entry->tbu_info.mmu_tbu_offset;
	uint32_t one_sid_offset = entry->tbu_info.mmu_sid_offset;

	dprint(PRN_CTRL, "tbu_num %u one_tbu_offset 0x%x one_sid_offset 0x%x\n", tbu_num, one_tbu_offset, one_sid_offset);

	for (reg_index = 0; reg_index < tbu_num; reg_index++) {
		reg_base_offset = reg_index * one_tbu_offset;
		ret = smmu_init_one_tbu(reg_base_offset);
		if (ret != SMMU_OK) {
			dprint(PRN_FATAL, "reg_index %u tbu and tcu connect failed\n", reg_index);
			return SMMU_ERR;
		}
		dprint(PRN_CTRL, "reg_index %u tbu and tcu connect success\n", reg_index);

		reg_base_offset = reg_index * one_sid_offset;
		smmu_init_one_sid(reg_base_offset);
	}

	return SMMU_OK;
}

static void smmu_deinit_reg(struct smmu_entry *entry)
{
	int32_t ret;
	uint32_t tbu_index;
	uint32_t tbu_num = entry->tbu_info.mmu_tbu_num;
	uint32_t one_tbu_offset = entry->tbu_info.mmu_tbu_offset;
	uint32_t reg_base_offset;

	for (tbu_index = 0; tbu_index < tbu_num; tbu_index++) {
		reg_base_offset = tbu_index * one_tbu_offset;
		ret = smmu_deinit_one_tbu(reg_base_offset);
		if (ret != SMMU_OK)
			dprint(PRN_ERROR, "reg_index %u tbu and tcu disconnect failed\n", tbu_index);
		else
			dprint(PRN_CTRL, "reg_index %u tbu and tcu disconnect success\n", tbu_index);
	}

	return;
}

int32_t smmu_init(void)
{
	struct smmu_entry *entry = smmu_get_entry();

	smmu_init_entry(entry);
	if (smmu_power_on_tcu()) {
		dprint(PRN_FATAL, "power on tcu failed\n");
		return SMMU_ERR;
	}
	if (smmu_init_reg(entry) != SMMU_OK) {
		smmu_power_off_tcu();
		return SMMU_ERR;
	}

	entry->smmu_init = 1;
	return SMMU_OK;
}

void smmu_deinit(void)
{
	struct smmu_entry *entry = smmu_get_entry();

	if (entry->smmu_init != 1) {
		dprint(PRN_ERROR, "smmu not init\n");
		return;
	}

	smmu_deinit_reg(entry);
	smmu_power_off_tcu();

	entry->smmu_init = 0;
}
