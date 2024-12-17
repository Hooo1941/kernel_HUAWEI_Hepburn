/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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

#ifndef DPU_SH_AOD_H
#define DPU_SH_AOD_H

#include <linux/types.h>
#include <linux/semaphore.h>
#include "dkmd_dpu.h"
#include "peri/dkmd_connector.h"

/**
 * @brief private data for sensorhub aod
 */
struct dpu_composer;
struct sh_aod {
	// struct semaphore blank_aod_sem; // todo
	struct semaphore fast_unblank_sem;
	struct workqueue_struct *aod_ud_fast_unblank_workqueue;
	struct work_struct aod_ud_fast_unblank_work;
	bool is_fast_unblanking; /* in fast unblank status only for aod */
	bool enable_fast_unblank; /* support fast unblank and fb blank meanwhile for aod */
};

#if defined(CONFIG_DKMD_DPU_AOD)
/* sensorhub aod */
void dpu_sh_aod_init(struct dpu_composer *dpu_comp);
void dpu_sh_aod_deinit(struct dpu_composer *dpu_comp);
bool dpu_sensorhub_aod_hw_lock(struct dpu_composer *dpu_comp);
bool dpu_sensorhub_aod_hw_unlock(struct dpu_composer *dpu_comp);
int dpu_sensorhub_aod_unblank(uint32_t msg_no);
int dpu_sensorhub_aod_blank(uint32_t msg_no);
int dpu_sh_aod_blank(struct dpu_composer *dpu_comp, int blank_mode);
bool dpu_sh_need_fast_unblank(struct dpu_composer *dpu_comp);
uint32_t dpu_get_panel_product_type(void);
bool dpu_check_panel_product_type(uint32_t product_type);
void dpu_aod_wait_for_blank(struct dpu_composer *dpu_comp, int blank_mode);
void dpu_wait_for_aod_stop(struct dpu_composer *dpu_comp);
void dpu_restore_fast_unblank_status(struct dpu_composer *dpu_comp);
#else
static inline void dpu_sh_aod_init(struct dpu_composer *dpu_comp) {};
static inline void dpu_sh_aod_deinit(struct dpu_composer *dpu_comp) {};
static inline bool dpu_sensorhub_aod_hw_lock(struct dpu_composer *dpu_comp) { return true; };
static inline bool dpu_sensorhub_aod_hw_unlock(struct dpu_composer *dpu_comp) { return true; };
static inline int dpu_sensorhub_aod_unblank(uint32_t msg_no) { return 0; };
static inline int dpu_sensorhub_aod_blank(uint32_t msg_no) { return 0; };
static inline int dpu_sh_aod_blank(struct dpu_composer *dpu_comp, int blank_mode) { return 0; };
static inline bool dpu_sh_need_fast_unblank(struct dpu_composer *dpu_comp) { return false; };
static inline uint32_t dpu_get_panel_product_type(void) { return 0; };
static inline bool dpu_check_panel_product_type(uint32_t product_type) { return false; };
static inline void dpu_aod_wait_for_blank(struct dpu_composer *dpu_comp, int blank_mode) {};
static inline void dpu_wait_for_aod_stop(struct dpu_composer *dpu_comp) {};
static inline void dpu_restore_fast_unblank_status(struct dpu_composer *dpu_comp) {};
#endif

#endif