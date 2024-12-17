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

#include "dkmd_log.h"

#include <linux/delay.h>
#include <dpu/soc_dpu_define.h>
#include "dpu_dacc.h"
#include "dpu_config_utils.h"
#include "dpu_comp_smmu.h"
#include "dpu_comp_online.h"
#include "dpu_comp_offline.h"
#include "dpu_comp_mgr.h"
#include "dpu_comp_init.h"
#include "dpu_comp_config_utils.h"
#include "dpu_isr_mdp.h"
#include "dpu_isr_sdp.h"
#include "dpu_isr_dvfs.h"
#include "dpu_sh_aod.h"
#include "dpu_comp_tui.h"
#include "dpu_comp_esd.h"
#include "dvfs.h"
#include "dpu_conn_mgr.h"
#include "dpu_comp_low_temp_handler.h"
#include "gfxdev_mgr.h"
#include "dkmd_connector.h"

int32_t media_regulator_enable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;

	ret = regulator_enable(comp_mgr->media1_subsys_regulator);
	if (ret)
		dpu_pr_err("regulator enable media1_subsys failed, error=%d!", ret);

	ret = regulator_enable(comp_mgr->vivobus_regulator);
	if (ret)
		dpu_pr_err("regulator enable vivobus failed, error=%d!", ret);

	return ret;
}

int32_t media_regulator_disable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;

	ret = regulator_disable(comp_mgr->vivobus_regulator);
	if (ret)
		dpu_pr_err("regulator disable vivobus failed, error=%d!", ret);

	ret = regulator_disable(comp_mgr->media1_subsys_regulator);
	if (ret)
		dpu_pr_err("regulator disable media1_subsys failed, error=%d!", ret);

	return ret;
}

void vivobus_autodiv_regulator_enable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;
	if (IS_ERR_OR_NULL(comp_mgr) || IS_ERR_OR_NULL(comp_mgr->vivobus_autodiv_regulator))
		return;

	ret = regulator_enable(comp_mgr->vivobus_autodiv_regulator);
	if (ret)
		dpu_pr_err("vivobus_autodiv_regulator enable failed, error=%d!", ret);

	return;
}

void vivobus_autodiv_regulator_disable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;
	if (IS_ERR_OR_NULL(comp_mgr) || IS_ERR_OR_NULL(comp_mgr->vivobus_autodiv_regulator))
		return;

	// multiple disable is allowed
	ret = regulator_disable(comp_mgr->vivobus_autodiv_regulator);
	if (ret)
		dpu_pr_info("vivobus_autodiv_regulator enable failed, error=%d!", ret);

	return;
}

static int32_t composer_regulator_enable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;

	if (!IS_ERR_OR_NULL(comp_mgr->dsssubsys_regulator)) {
		ret = regulator_enable(comp_mgr->dsssubsys_regulator);
		if (ret)
			dpu_pr_err("dsssubsys_regulator enable failed, error=%d!", ret);
	}

	if (!IS_ERR_OR_NULL(comp_mgr->disp_ch1subsys_regulator)) {
		dpu_pr_info("disp_ch1subsys_regulator enable + ");
		ret = regulator_enable(comp_mgr->disp_ch1subsys_regulator);
		if (ret)
			dpu_pr_err("disp_ch1subsys_regulator enable failed, error=%d!", ret);
	}

	if (!IS_ERR_OR_NULL(comp_mgr->regulator_smmu_tcu)) {
		ret = regulator_enable(comp_mgr->regulator_smmu_tcu);
		if (ret)
			dpu_pr_warn("smmu tcu regulator_enable failed, error=%d!", ret);
	}

	return ret;
}

static int32_t composer_regulator_disable(struct composer_manager *comp_mgr)
{
	int32_t ret = 0;

	if (!IS_ERR_OR_NULL(comp_mgr->regulator_smmu_tcu)) {
		ret = regulator_disable(comp_mgr->regulator_smmu_tcu);
		if (ret)
			dpu_pr_warn("smmu tcu regulator_disable failed, error=%d!", ret);
	}

	if (!IS_ERR_OR_NULL(comp_mgr->disp_ch1subsys_regulator)) {
		ret = regulator_disable(comp_mgr->disp_ch1subsys_regulator);
		if (ret)
			dpu_pr_err("disp_ch1subsys_regulator disable failed, error=%d!", ret);
	}

	if (!IS_ERR_OR_NULL(comp_mgr->dsssubsys_regulator)) {
		ret = regulator_disable(comp_mgr->dsssubsys_regulator);
		if (ret)
			dpu_pr_err("dsssubsys_regulator disable failed, error=%d!", ret);
	}

	return 0;
}

static void composer_manager_hiace_on(struct dpu_composer *dpu_comp)
{
	if (!dpu_comp->hiace_ctrl)
		return;

	dpu_comp->hiace_ctrl->init_params(dpu_comp);
	dpu_comp->hiace_ctrl->unblank(dpu_comp->hiace_ctrl);
}

static void composer_manager_hiace_off(struct dpu_composer *dpu_comp)
{
	if (!dpu_comp->hiace_ctrl)
		return;

	dpu_comp->hiace_ctrl->blank(dpu_comp->hiace_ctrl);
}

void composer_dpu_power_on(struct composer_manager *comp_mgr, struct composer *comp)
{
	dpu_dvfs_enable_core_clock(true);
	dpu_dvfs_direct_vote(comp->index, DPU_CORE_LEVEL_ON, false);
	composer_regulator_enable(comp_mgr);
	dpu_dvfs_qos_qic_media1_config(DPU_CORE_LEVEL_ON);
	dpu_power_on_state_for_ddr_dfs(comp_mgr->pctrl_base, true);
	vivobus_autodiv_regulator_enable(comp_mgr);

	/* Cmdlist preparation, need to use the CPU configuration */
	set_reg(comp_mgr->dpu_base + DPU_GLB_PM_CTRL_ADDR(DPU_GLB0_OFFSET), GLB_PM_CTRL_VALUE, 32, 0);
	set_reg(comp_mgr->dpu_base + DPU_DSI_PM_CTRL_ADDR(DPU_MIPI_DSI0_OFFSET), DSI_PM_CTRL_VALUE, 32, 0);
	set_reg(comp_mgr->dpu_base + DPU_DSI_PM_CTRL_ADDR(DPU_MIPI_DSI2_OFFSET), DSI_PM_CTRL_VALUE, 32, 0);

	set_reg(DPU_GLB_SDMA_DBG_RESERVED0_ADDR(comp_mgr->dpu_base + DPU_GLB0_OFFSET, 0), 1, 1, 20);
	set_reg(DPU_GLB_SDMA_DBG_RESERVED0_ADDR(comp_mgr->dpu_base + DPU_GLB0_OFFSET, 1), 1, 1, 20);
	set_reg(DPU_GLB_SDMA_DBG_RESERVED0_ADDR(comp_mgr->dpu_base + DPU_GLB0_OFFSET, 2), 1, 1, 20);
	set_reg(DPU_GLB_SDMA_DBG_RESERVED0_ADDR(comp_mgr->dpu_base + DPU_GLB0_OFFSET, 3), 1, 1, 20);

	/* Enable peri dvfs to make sure HW_DSS_DVFS receive frm_start signal */
	set_reg(DPU_GLB_REG_DVFS_CTRL_ADDR(comp_mgr->dpu_base + DPU_GLB0_OFFSET), 1, 1, 8);
}

void composer_dpu_power_on_sub(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	dpu_comp_smmuv3_on(comp_mgr, dpu_comp);
	composer_manager_hiace_on(dpu_comp);

	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, RESET_PARTIAL_UPDATE, NULL);
	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, INIT_DSC, pinfo);
	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, INIT_SPR, NULL);
	(void)pipeline_next_on(pinfo->conn_device, pinfo);
	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, ENABLE_ISR, &dpu_comp->isr_ctrl);
}

void composer_dpu_power_off(struct composer_manager *comp_mgr, struct composer *comp)
{
	vivobus_autodiv_regulator_disable(comp_mgr);
	dpu_power_on_state_for_ddr_dfs(comp_mgr->pctrl_base, false);
	composer_regulator_disable(comp_mgr);
	dpu_dvfs_direct_vote(comp->index, DPU_CORE_LEVEL_OFF, false);
	dpu_dvfs_disable_core_clock();
}

static void composer_dpu_pipesw_disconnect(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;
	int32_t primary_post_ch_offset = 0x0;
	uint32_t external_post_ch_offset = 0x0;

	dpu_pr_debug("enter");
	if ((!is_primary_panel(&pinfo->base)) && (!is_builtin_panel(&pinfo->base))) {
		dpu_pr_debug("comp index %u isnot primary_panel or builtin_panel", dpu_comp->comp.index);
		return;
	}
	if (pinfo->sw_post_chn_num == 0) {
		dpu_pr_err("comp index %u sw_post_chn_num invalid", dpu_comp->comp.index);
		return;
	}
	primary_post_ch_offset = (int32_t)pinfo->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX] * 0x1C;
	set_reg(DPU_PIPE_SW_SIG_CTRL_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, 0, 32, 0);
	set_reg(DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, 0, 32, 0);
	set_reg(DPU_PIPE_SW_DAT_CTRL_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, 0, 32, 0);
	set_reg(DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, 0, 32, 0);

	if (pinfo->sw_post_chn_num > EXTERNAL_CONNECT_CHN_IDX) {
		external_post_ch_offset = pinfo->sw_post_chn_idx[EXTERNAL_CONNECT_CHN_IDX] * 0x1C;
		set_reg(DPU_PIPE_SW_SIG_CTRL_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0, 32, 0);
		set_reg(DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0, 32, 0);
		set_reg(DPU_PIPE_SW_DAT_CTRL_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0, 32, 0);
		set_reg(DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(comp_mgr->dpu_base + DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0, 32, 0);
	}
}

void composer_dpu_power_off_sub(struct dpu_composer *dpu_comp)
{
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	if (dpu_tui_wait_quit(dpu_comp))
		dpu_pr_err("quit tui fail!");

	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, DISABLE_ISR, &dpu_comp->isr_ctrl);

	if ((!is_offline_panel(&pinfo->base)) && (!is_dp_panel(&pinfo->base)) &&
		(!is_hdmi_panel(&pinfo->base)) && (!dpu_comp->comp.is_fake_power_off))
		dpu_backlight_cancel(&dpu_comp->bl_ctrl);

	if (dpu_comp->comp.is_fake_power_off)
		(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, FAKE_POWER_OFF, pinfo);
	else
		(void)pipeline_next_off(pinfo->conn_device, pinfo);

	dpu_comp_smmuv3_off(comp_mgr, dpu_comp);
	composer_manager_hiace_off(dpu_comp);
	dpu_dvfs_reset_vote(dpu_comp->comp.index);

	dpu_comp->comp_idle_enable = false;
	dpu_comp->comp_idle_flag = 0;
	composer_dpu_pipesw_disconnect(dpu_comp);
}

int32_t composer_manager_set_fastboot(struct composer *comp)
{
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	if (comp_mgr->power_status.status == 0) {
		media_regulator_enable(comp_mgr);
		composer_dpu_power_on(comp_mgr, comp);
	}

	dpu_comp_smmuv3_on(comp_mgr, dpu_comp);
	composer_manager_hiace_on(dpu_comp);
	composer_present_power_on(dpu_comp);

	if (comp_mgr->power_status.status == 0) {
		dkmd_mdp_isr_enable(dpu_comp->comp_mgr);
		dkmd_sdp_isr_enable(dpu_comp->comp_mgr);
		dkmd_dvfs_isr_enable(dpu_comp->comp_mgr);
	}
	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, SET_FASTBOOT, (void *)&comp->fastboot_display_enabled);
	(void)pipeline_next_ops_handle(pinfo->conn_device, pinfo, ENABLE_ISR, &dpu_comp->isr_ctrl);

	/* need after isr request and composer power on */
	dpu_comp_init(dpu_comp);

	comp_mgr->power_status.refcount.value[comp->index]++;

	(void)dpu_sh_aod_blank(dpu_comp, DISP_BLANK_UNBLANK);

	restart_esd_timer(dpu_comp);

	reset_low_temperature_state(dpu_comp);

	return (int32_t)comp->fastboot_display_enabled;
}

int32_t composer_wait_for_blank(struct composer *comp)
{
	int32_t ret = 0;
	struct dpu_composer *dpu_comp = NULL;
	dpu_check_and_return(!comp, 0, err, "comp is NULL");
	dpu_comp = to_dpu_composer(comp);
	if ((!is_builtin_panel(&comp->base)) && (!is_primary_panel(&comp->base)))
		return 0;

	if (dpu_sh_need_fast_unblank(dpu_comp)) {
		dpu_pr_info("[aod] power up with fast unblank");
		return FAST_UNLOCK_RET;
	}

	dpu_wait_for_aod_stop(dpu_comp);

	ret = dpu_sh_aod_blank(dpu_comp, DISP_BLANK_UNBLANK);
	if (ret == FAST_UNLOCK_RET) {
		dpu_pr_info("AOD in fast unlock mode");
		return FAST_UNLOCK_RET;
	}

	dpu_aod_wait_for_blank(dpu_comp, DISP_BLANK_UNBLANK);
	return 0;
}

void composer_restore_fast_unblank_status(struct composer *comp)
{
	struct dpu_composer *dpu_comp = NULL;
	dpu_check_and_no_retval(!comp, err, "comp is NULL");
	dpu_comp = to_dpu_composer(comp);
	if ((!is_builtin_panel(&comp->base)) && (!is_primary_panel(&comp->base)))
		return;

	dpu_restore_fast_unblank_status(dpu_comp);
	dpu_aod_wait_for_blank(dpu_comp, DISP_BLANK_POWERDOWN);
}

static void composer_manager_alsc_on(struct dpu_composer *dpu_comp)
{
	if (!dpu_comp->alsc)
		return;

	if (!dpu_comp->alsc->comp_alsc_unblank)
		return;

	dpu_comp->alsc->comp_alsc_unblank(dpu_comp->alsc);
}

static void composer_manager_alsc_off(struct dpu_composer *dpu_comp)
{
	if (!dpu_comp->alsc)
		return;

	if (!dpu_comp->alsc->comp_alsc_blank)
		return;

	dpu_comp->alsc->comp_alsc_blank(dpu_comp->alsc);
}

void composer_manager_power_down(struct dpu_composer *dpu_comp)
{
	uint32_t index;
	struct composer *comp = NULL;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	if (unlikely(!comp_mgr))
		return;

	for (index = 0; index < DEVICE_COMP_MAX_COUNT; index++) {
		comp = &comp_mgr->dpu_comps[index]->comp;
		if (!comp)
			continue;

		down(&comp->blank_sem);
	}

	down(&comp_mgr->power_sem);
}

void composer_manager_power_up(struct dpu_composer *dpu_comp)
{
	int32_t index;
	struct composer *comp = NULL;
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	if (unlikely(!comp_mgr))
		return;

	up(&comp_mgr->power_sem);

	for (index = (DEVICE_COMP_MAX_COUNT - 1); index >= 0; index--) {
		comp = &comp_mgr->dpu_comps[index]->comp;
		if (!comp)
			continue;

		up(&comp->blank_sem);
	}
}

/* no lock function, need caller to lock power sem */
int32_t composer_manager_on_no_lock(struct composer *comp)
{
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;
	bool sensorhub_aod_hwlock_succ;

	dpu_pr_warn("power_status=0x%llx index:%u enter", comp_mgr->power_status.status, dpu_comp->comp.index);
	if (unlikely(comp->power_on))
		return 0;

	sensorhub_aod_hwlock_succ = dpu_sensorhub_aod_hw_lock(dpu_comp);

	dpu_comp_active_vsync(dpu_comp);

	// power_status will by protected by comp->blank_sem
	if (comp_mgr->power_status.status == 0) {
		/* public function need use public data as interface parameter, such as 'comp_mgr' */
		if (comp_mgr->power_on_stage == DPU_POWER_ON_NONE) {
			media_regulator_enable(dpu_comp->comp_mgr);
			composer_dpu_power_on(dpu_comp->comp_mgr, comp);
			dpu_dacc_load();
		}

		comp_mgr->power_status.refcount.value[comp->index]++;

		dkmd_mdp_isr_enable(dpu_comp->comp_mgr);
		dkmd_sdp_isr_enable(dpu_comp->comp_mgr);
		dkmd_dvfs_isr_enable(dpu_comp->comp_mgr);
		composer_present_power_on(dpu_comp);
		composer_dpu_power_on_sub(dpu_comp);
		dpu_comp_init(dpu_comp);
	} else {
		if (comp_mgr->power_status.refcount.value[comp->index] == 0) {
			composer_present_power_on(dpu_comp);
			composer_dpu_power_on_sub(dpu_comp);
			if ((is_primary_panel(&pinfo->base)) || (is_builtin_panel(&pinfo->base)))
				dpu_comp_init(dpu_comp);
		}
		comp_mgr->power_status.refcount.value[comp->index]++;
	}
	dpu_comp_deactive_vsync(dpu_comp);
	composer_manager_alsc_on(dpu_comp);

	start_esd_timer(dpu_comp);
	reset_low_temperature_state(dpu_comp);

	if (sensorhub_aod_hwlock_succ)
		dpu_sensorhub_aod_hw_unlock(dpu_comp);

	comp->power_on = true;
	dpu_pr_info("power_status=0x%llx exit", comp_mgr->power_status.status);
	return 0;
}

void composer_manager_release(struct composer *comp)
{
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);

	dpu_comp_release(dpu_comp);
}

int32_t composer_manager_on(struct composer *comp)
{
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;
	int32_t ret;

	if (unlikely(!comp_mgr))
		return -1;

	composer_manager_power_down(dpu_comp);
	if (unlikely(comp->power_on)) {
		composer_manager_power_up(dpu_comp);
		return 0;
	}

	ret = composer_manager_on_no_lock(comp);
	if (likely(ret == 0))
		comp->power_on = true;

	composer_manager_power_up(dpu_comp);

	return ret;
}

int32_t composer_manager_off_no_lock(struct composer *comp, int32_t off_mode)
{
	int32_t ret = 0;
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;
	bool sensorhub_aod_hwlock_succ;
	struct dpu_connector *connector = NULL;

	dpu_pr_warn("power_status=0x%llx index:%u enter, off_mode = %d",
				comp_mgr->power_status.status, dpu_comp->comp.index, off_mode);

	if (unlikely(!comp->power_on || !dpu_comp->conn_info))
		return 0;

	connector = get_primary_connector(dpu_comp->conn_info);
	sensorhub_aod_hwlock_succ = dpu_sensorhub_aod_hw_lock(dpu_comp);

	dpu_comp_active_vsync(dpu_comp);

	if (comp_mgr->power_status.refcount.value[comp->index] == 1) {
		composer_dpu_power_off_sub(dpu_comp);

		comp_mgr->power_status.refcount.value[comp->index]--;
		composer_present_power_off(dpu_comp, comp_mgr->power_status.status == 0);
		/* public function need use public data as interface parameter, such as 'comp_mgr' */
		if (comp_mgr->power_status.status == 0) {
			/* abort idle thread handle */
			mutex_lock(&comp_mgr->idle_lock);
			comp_mgr->idle_enable = false;
			comp_mgr->idle_func_flag = 0;
			mutex_unlock(&comp_mgr->idle_lock);

			dkmd_mdp_isr_disable(dpu_comp->comp_mgr);
			dkmd_sdp_isr_disable(dpu_comp->comp_mgr);
			dkmd_dvfs_isr_disable(dpu_comp->comp_mgr);
			/**
			 * After disable isr, in order to get right status for the first frame after dss power up,
			 * need to set is_vactive_end_recieved to true for ltpo panel.
			 */
			connector->is_vactive_end_recieved = VACTIVE_END_RECEIVED;
			if (comp_mgr->power_on_stage == DPU_POWER_ON_NONE) {
				composer_dpu_power_off(comp_mgr, comp);
				media_regulator_disable(comp_mgr);
			}
		}
	} else {
		if (comp_mgr->power_status.refcount.value[comp->index] != 0)
			comp_mgr->power_status.refcount.value[comp->index]--;
		dpu_pr_warn("%s set too many power down, refcount %d!", comp->base.name,
			comp_mgr->power_status.refcount.value[comp->index]);
	}

	dpu_comp_deactive_vsync(dpu_comp);
	composer_manager_alsc_off(dpu_comp);

	cancel_esd_timer(dpu_comp);
	reset_low_temperature_state(dpu_comp);

	(void)dpu_sh_aod_blank(dpu_comp, DISP_BLANK_POWERDOWN);
	if (sensorhub_aod_hwlock_succ)
		dpu_sensorhub_aod_hw_unlock(dpu_comp);

	comp->power_on = false;
	dpu_pr_info("power_status=0x%llx exit", comp_mgr->power_status.status);
	return ret;
}

int32_t composer_manager_off(struct composer *comp, int32_t off_mode)
{
	int32_t ret;
	struct dpu_composer *dpu_comp = to_dpu_composer(comp);
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;

	if (unlikely(!comp_mgr))
		return -1;

	composer_manager_power_down(dpu_comp);
	if (unlikely(!comp->power_on)) {
		comp->is_fake_power_off = false;
		composer_manager_power_up(dpu_comp);
		return 0;
	}

	ret = composer_manager_off_no_lock(comp, off_mode);
	if (likely(ret == 0))
		comp->power_on = false;

	composer_manager_timeline_resync(dpu_comp, off_mode);

	comp->is_fake_power_off = false;
	composer_manager_power_up(dpu_comp);
	return ret;
}

void composer_manager_power_restart_no_lock(struct dpu_composer *dpu_comp)
{
	struct composer_manager *comp_mgr = dpu_comp->comp_mgr;
	uint32_t index;
	struct composer *comp = NULL;
	int32_t ret;
	bool power_status[DEVICE_COMP_MAX_COUNT] = {false};

	dpu_pr_warn("index:%u enter", dpu_comp->comp.index);
	comp_mgr->is_power_restarting = true;
	for (index = 0; index < DEVICE_COMP_MAX_COUNT; index++) {
		comp = &comp_mgr->dpu_comps[index]->comp;
		if (!comp || !comp->power_on)
			continue;
		ret = composer_manager_off_no_lock(comp, COMPOSER_OFF_MODE_BLANK);
		if (ret)
			continue;
		power_status[index] = true;
	}

	for (index = 0; index < DEVICE_COMP_MAX_COUNT; index++) {
		comp = &comp_mgr->dpu_comps[index]->comp;
		if (comp == NULL)
			continue;
		if (power_status[index]) {
			ret = composer_manager_on_no_lock(comp);
			if (ret)
				dpu_pr_err("power on failed index:[%u]", index);
		}
	}
	send_reset_hardware_event(&dpu_comp->comp);
	comp_mgr->is_power_restarting = false;
	dpu_pr_warn("index:%u exit", dpu_comp->comp.index);
}

void composer_manager_reset_hardware(struct dpu_composer *dpu_comp)
{
	struct comp_online_present *present = NULL;
	composer_manager_power_restart_no_lock(dpu_comp);
	dpu_pr_warn("reset hardware finished!");
	if (!is_offline_panel(&dpu_comp->conn_info->base)) {
		present = (struct comp_online_present *)dpu_comp->present_data;
		present->vactive_start_flag = 1;
		present->frame_start_flag = 0;
	}
}
