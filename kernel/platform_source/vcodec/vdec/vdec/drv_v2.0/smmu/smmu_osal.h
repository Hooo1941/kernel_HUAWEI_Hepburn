/*
 * smmu_osal.h
 *
 * This is for smmu osal.
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

#ifndef VCODEC_VDEC_SMMU_OSAL_H
#define VCODEC_VDEC_SMMU_OSAL_H

#include <linux/types.h>
#include "smmu_cfg.h"
#include "smmu_regs.h"

int32_t smmu_power_on_tcu(void);
void smmu_power_off_tcu(void);
int32_t smmu_map_reg_osal(struct smmu_entry *entry);
void smmu_unmap_reg_osal(struct smmu_entry *entry);

#endif

