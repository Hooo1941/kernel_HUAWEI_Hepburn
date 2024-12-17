/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef COMPOSER_PRESENT_H
#define COMPOSER_PRESENT_H

#include <linux/types.h>

void composer_present_timeline_resync(struct dpu_composer *dpu_comp);
void composer_present_data_setup(struct dpu_composer *dpu_comp, bool inited);
void composer_present_data_release(struct dpu_composer *dpu_comp, bool is_poweroff);
void composer_present_power_on(struct dpu_composer *dpu_comp);
void composer_present_power_off(struct dpu_composer *dpu_comp, bool is_dpu_poweroff);
void composer_present_dfr_setup(struct dpu_composer *dpu_comp);
void composer_present_dfr_release(struct dpu_composer *dpu_comp);
void composer_present_ppc_release(struct dpu_composer *dpu_comp);
int32_t composer_present_dfr_send_dcs_by_riscv(struct dpu_composer *dpu_comp, uint32_t bl_level);
int32_t composer_present_set_safe_frm_rate(struct dpu_composer *dpu_comp,
	uint32_t safe_frm_rate);
int32_t composer_present_set_active_rect(struct dpu_composer *dpu_comp, uint32_t ppc_config_id);
#endif
