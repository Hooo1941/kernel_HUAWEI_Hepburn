/*
 * smmu_tbu.c
 *
 * This is for smmu tbu.
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

#include <linux/delay.h>
#include "vcodec_vdec_plat.h"
#include "vcodec_vdec_regulator.h"
#include "vfmw_ext.h"
#include "smmu_cfg.h"
#include "smmu_tbu.h"

#define TBU_ACK_TIMEOUT 1000

static void smmu_tbu_ecc_enable(uint32_t reg_base_offset)
{
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_S_DR, 0x1, 1, 25);
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_S_SR, 0x1, 1, 25);
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_T_DR, 0x1, 1, 15);
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_T_SR, 0x1, 1, 15);
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_B_DR, 0x1, 1, 13);
	set_tbu_reg(reg_base_offset + SMMU_TBU_MEM_CTRL_B_SR, 0x1, 1, 13);

	/* enable interrupts */
	set_tbu_reg(reg_base_offset + SMMU_TBU_IRPT_MASK_NS, 0x0, 2, 4);
}

void smmu_tbu_ecc_clear_interrupts(void)
{
	uint32_t reg_index;
	uint32_t reg_base_offset;
	uint32_t tbu_num;
	uint32_t one_tbu_offset;
	struct smmu_entry *entry = smmu_get_entry();

	if (entry->smmu_init != 1) {
		dprint(PRN_ERROR, "smmu is not init\n");
		return;
	}

	tbu_num = entry->tbu_info.mmu_tbu_num;
	one_tbu_offset = entry->tbu_info.mmu_tbu_offset;

	for (reg_index = 0; reg_index < tbu_num; reg_index++) {
		reg_base_offset = reg_index * one_tbu_offset;
		set_tbu_reg(reg_base_offset + SMMU_TBU_IRPT_CLR_NS, 0x3, 2, 4);
	}
}

void smmu_init_one_sid(uint32_t reg_base_offset)
{
	set_sid_reg(reg_base_offset + SMMU_NORM_RSID, SMMU_NORM_RSID_VALUE, 8, 0); /* 8: bit width */
	set_sid_reg(reg_base_offset + SMMU_NORM_WSID, SMMU_NORM_WSID_VALUE, 8, 0); /* 8: bit width */

	set_sid_reg(reg_base_offset + SMMU_NORM_RSSID, 0, 1, 0);
	set_sid_reg(reg_base_offset + SMMU_NORM_WSSID, 0, 1, 0);

	set_sid_reg(reg_base_offset + SMMU_NORM_RSSIDV, 1, 1, 0);
	set_sid_reg(reg_base_offset + SMMU_NORM_WSSIDV, 1, 1, 0);
}

int32_t smmu_init_one_tbu(uint32_t reg_base_offset)
{
	uint32_t i;
	uint32_t rel;
	uint32_t rel_cr;
	uint32_t ack_timeout;
	vdec_plat_ops * plat_ops = NULL;

	vdec_dts *dts_info = &(vdec_plat_get_entry()->dts_info);
	ack_timeout = dts_info->is_fpga ? TBU_ACK_TIMEOUT * 10 : TBU_ACK_TIMEOUT;

	plat_ops = get_vdec_plat_ops();
	if (plat_ops->smmu_need_enable_tbu_ecc())
		smmu_tbu_ecc_enable(reg_base_offset);

	if (plat_ops->smmu_need_cfg_max_tok_trans())
		set_tbu_reg(reg_base_offset + SMMU_TBU_CR, 0x17, 8, 8);
	set_tbu_reg(reg_base_offset + SMMU_TBU_CR, 0x1, 1, 0);

	for (i = 0; i < ack_timeout; i++) {
		udelay(1);
		rd_smmu_tbu_vreg((reg_base_offset + SMMU_TBU_CRACK), rel);
		if ((rel & 0x3) == 0x3)
			break;
	}

	if (i == ack_timeout)
		return SMMU_ERR;

	rd_smmu_tbu_vreg((reg_base_offset + SMMU_TBU_CR), rel_cr);
	if ((rel & 0xff00) < (rel_cr & 0xff00)) {
		dprint(PRN_FATAL, "check tok_trans_gnt failed\n");
		return SMMU_ERR;
	}

	return SMMU_OK;
}

int32_t smmu_deinit_one_tbu(uint32_t reg_base_offset)
{
	uint32_t i;
	uint32_t rel;
	uint32_t ack_timeout;

	vdec_dts *dts_info = &(vdec_plat_get_entry()->dts_info);
	ack_timeout = dts_info->is_fpga ? TBU_ACK_TIMEOUT * 10 : TBU_ACK_TIMEOUT;

	set_tbu_reg(reg_base_offset + SMMU_TBU_CR, 0x0, 1, 0);
	for (i = 0; i < ack_timeout; i++) {
		udelay(1);
		rd_smmu_tbu_vreg((reg_base_offset + SMMU_TBU_CRACK), rel);
		if ((rel & 0x3) == 0x1)
			break;
	}

	if (i == ack_timeout)
		return SMMU_ERR;

	return SMMU_OK;
}

void smmu_init_entry(struct smmu_entry *entry)
{
	struct smmu_tbu_info *tbu_info = &(vdec_plat_get_entry()->smmu_info);
	entry->tbu_info.mmu_tbu_num = tbu_info->mmu_tbu_num;
	entry->tbu_info.mmu_tbu_offset = tbu_info->mmu_tbu_offset;
	entry->tbu_info.mmu_sid_offset = tbu_info->mmu_sid_offset;
}
