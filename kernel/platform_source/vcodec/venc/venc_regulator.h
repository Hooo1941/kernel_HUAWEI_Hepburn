/*
 * venc_regulator.h
 *
 * This is venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
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

#ifndef __VENC_REGULATOR_H__
#define __VENC_REGULATOR_H__

#include <linux/regulator/consumer.h>
#include "drv_common.h"

#define MAX_NAME_LEN 20
#define VENC_CLK_RATE         "enc_clk_rate"
#define VENC_CLOCK_NAME       "clk_venc"
#define VENC_REGULATOR_NAME   "ldo_venc"
#define MEDIA_REGULATOR_NAME  "ldo_media"
#define VENC_POWER_MODE_PROFILE  "power_mode_loads"

#ifdef VCODECV700
#define VENC_LOW_POWER_MODE_PROFILE_LEN "low_power_mode_profile_len"
#define VENC_LOW_POWER_MODE_LOADS  "low_power_mode_loads"
#define VENC_LOW_POWER_MODE_LEVEL  "low_power_mode_level"
#endif

/* clock */
struct venc_clock {
	struct clk *venc_clk;
	struct clk *venc_aclk;
	venc_clk_t curr_clk_type;
};

/* regulator */
struct venc_regulator {
	struct regulator *media_regulator;
	struct regulator *venc_regulator[MAX_SUPPORT_CORE_NUM];
};

#ifdef VCODECV700
struct venc_performance_info {
	uint32_t loads;
	uint32_t core_num;
	uint32_t power_level;
};
#endif
/* config */
struct venc_config_common {
	uint32_t core_num;
	uint32_t fpga_flag;
	uint32_t qos_mode;
	uint32_t clk_rate[VENC_CLK_BUTT];
	uint32_t aclk_rate[VENC_CLK_BUTT];
	uint32_t power_off_clk_rate;
	uint32_t power_off_aclk_rate;
	uint32_t transit_clk_rate;
	uint32_t transit_aclk_rate;
	uint32_t valid_core_id;
	bool is_single_core;
	uint32_t mcore_code_base;
	uint32_t intr_hub_reg_base;
	uint32_t disable_cfg_tbu_max_tok_trans;
	uint32_t smmu_establish_link_robust;
#ifdef VCODECV700
	struct venc_performance_info power_mode[VENC_CLK_BUTT];
	uint32_t low_power_mode_len;
	struct venc_performance_info low_power_mode[5];
#endif
};

struct venc_config_priv {
	uint32_t reg_base_addr;
	uint32_t reg_range;
};

struct venc_config {
	struct venc_config_common venc_conf_com;
	struct venc_config_priv venc_conf_priv[MAX_SUPPORT_CORE_NUM];
#ifdef VENC_DPM_ENABLE
	struct venc_config_priv venc_conf_dpm[MAX_INNER_MODULE];
#endif
};

#ifdef SUPPORT_VENC_FREQ_CHANGE
extern uint32_t g_venc_freq;
extern struct mutex g_venc_freq_mutex;
#endif

int32_t venc_get_dts_config_info(struct platform_device *pdev);
int32_t venc_get_regulator_info(struct platform_device *pdev);
int32_t  venc_regulator_enable(void);
int32_t  venc_regulator_disable(void);
int32_t  venc_regulator_select_idle_core(vedu_osal_event_t *event);
int32_t  venc_regulator_wait_hardware_idle(vedu_osal_event_t *event);
int32_t  venc_regulator_update(struct clock_info *clock_info);
int32_t  venc_regulator_reset(void);
bool venc_regulator_is_fpga(void);
uint64_t  venc_get_smmu_ttb(void);
bool is_support_power_control_per_frame(void);
uint32_t venc_get_reg_base_addr(int32_t core_id);
uint32_t venc_get_core_num(void);
int32_t venc_regulator_disable_by_low_power(void);
void venc_set_clock_info(struct clock_info *info);
void venc_get_clock_info(struct clock_info *info);
uint32_t venc_get_mcore_code_base(void);
uint32_t venc_get_intr_hub_reg_base(void);
bool venc_get_disable_cfg_tbu_max_tok_trans(void);
bool venc_get_smmu_estbl_link_robust_enable(void);
struct venc_config *get_venc_config(void);
struct venc_clock *get_venc_clock_manager(uint32_t core_id);

#ifdef VCODECV700
uint32_t venc_get_power_profiles(uint32_t flag, struct venc_performance_info **performance_info);
#endif
#endif

