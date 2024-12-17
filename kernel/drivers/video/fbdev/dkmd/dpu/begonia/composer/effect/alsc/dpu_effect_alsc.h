/* Copyright (c) 2023-2023, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#ifndef DPU_EFFECT_ALSC_H
#define DPU_EFFECT_ALSC_H

#include <linux/mutex.h>
#include "dkmd_alsc_interface.h"
#include "dkmd_comp.h"
#include "dpu_comp_mgr.h"

#define ALSC_MAX_DATA_LEN 3
#define ALSC_DEGAMMA_VAL_MASK ((1UL << 12) - 1)
#define ALSC_DEGAMMA_ODD_COEF_START_BIT 16

#define ALSC_PIC_WIDTH_SHIFT 13
#define ALSC_PIC_HEIGHT_MASK ((1UL << ALSC_PIC_WIDTH_SHIFT) - 1)

#define ALSC_ADDR_Y_SHIFT 13
#define ALSC_ADDR_X_MASK ((1UL << ALSC_ADDR_Y_SHIFT) - 1)
#define ALSC_SIZE_Y_SHIFT 8
#define ALSC_SIZE_X_MASK ((1UL << ALSC_SIZE_Y_SHIFT) - 1)

#define ALSC_NOISE_VALID 1

enum alsc_status {
	ALSC_UNINIT,
	ALSC_WORKING,
	ALSC_IDLE,
	ALSC_STATUS_MAX
};

enum alsc_action {
	ALSC_NO_ACTION,
	ALSC_ENABLE,
	ALSC_DISABLE,
	ALSC_UPDATE_BL,
	ALSC_UPDATE_DEGAMMA,
	ALSC_SET_REG,
	ALSC_BLANK,
	ALSC_UNBLANK,
	ALSC_HANDLE_ISR,
	ALSC_ACTION_MAX
};

struct dkmd_alsc_frame {
	struct dkmd_rect *disp_rect;
	uint32_t alsc_en;
};

struct dpu_alsc_cb_func {
	void (*send_data_func)(struct alsc_noise *, uint32_t);
	void (*send_ddic_alpha)(uint32_t, uint32_t);
};

struct dpu_alsc_data {
	struct alsc_noise noise;
	uint64_t ktimestamp;

	struct dpu_alsc_data *prev;
	struct dpu_alsc_data *next;
};

struct dpu_alsc {
	struct dkmd_alsc dkmd_alsc;

	bool is_inited;
	uint32_t status;
	uint32_t action;
	uint32_t panel_id;

	uint32_t degamma_en;
	uint32_t degamma_lut_sel;
	uint32_t alsc_en_by_dirty_region_limit;
	uint32_t bl_update_to_hwc;

	struct dkmd_rect alsc_phy_rect;

	struct mutex alsc_lock;
	char __iomem *alsc_base;

	struct dpu_alsc_data *data_head;
	struct dpu_alsc_data *data_tail;

	struct dpu_alsc_cb_func cb_func;

	struct workqueue_struct *alsc_send_data_wq;
	struct work_struct alsc_send_data_work;
	struct composer *comp;

	int32_t (*comp_alsc_set_reg)(struct dpu_composer *dpu_comp, struct dkmd_rect *disp_rect, uint32_t alsc_en);
	int32_t (*comp_alsc_blank)(struct dpu_alsc *alsc);
	int32_t (*comp_alsc_unblank)(struct dpu_alsc *alsc);
};

struct alsc_fsm {
	int32_t (*handle)(struct dpu_alsc *, const void * const);
};

int32_t dpu_effect_register_alsc_composer(struct composer *comp, uint32_t index, uint32_t dpp_offset);
int32_t dpu_effect_unregister_alsc_composer(struct composer *comp);
int32_t dpu_effect_alsc_store_data(struct dpu_alsc *alsc, uint32_t isr_flag);
int32_t dpu_effect_get_alsc_info(struct composer *comp, struct alsc_info *info);

void dpu_effect_update_alsc_coord(struct dpu_alsc *alsc);

#endif