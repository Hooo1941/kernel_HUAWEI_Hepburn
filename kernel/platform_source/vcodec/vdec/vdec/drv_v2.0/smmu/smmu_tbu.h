/*
 * smmu_tbu.h
 *
 * This is for smmu tbu.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
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

#ifndef VCODEC_VDEC_SMMU_TBU_H
#define VCODEC_VDEC_SMMU_TBU_H

#include <linux/types.h>
#include "smmu_regs.h"

void smmu_tbu_ecc_clear_interrupts(void);
void smmu_init_one_sid(uint32_t reg_base_offset);
void smmu_init_entry(struct smmu_entry *entry);
int32_t smmu_init_one_tbu(uint32_t reg_base_offset);
int32_t smmu_deinit_one_tbu(uint32_t reg_base_offset);

#endif
