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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <securec.h>
#include "dpu_config_utils.h"
#include "dpu_comp_config_utils.h"
#include "dpu_comp_smmu.h"
#include "dpu_comp_mgr.h"
#include "gfxdev_mgr.h"
#include "dpu_comp_vsync.h"
#include "dvfs.h"
#include "dkmd_listener.h"
#include "dpu_comp_maintain.h"
#include "dpu_isr_mdp.h"
#include "dpu_isr_sdp.h"
#include "dpu_isr_dvfs.h"
#include "dkmd_display_engine.h"
#include "ddr_dvfs.h"
#include "dkmd_connector.h"
#include "dpu_connector.h"
#include "dksm_dmd.h"
#include "dpu_conn_mgr.h"
#include "dpu_ppc_status_control.h"

static ssize_t dfr_target_frame_rate_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	ssize_t ret;
	int32_t frame_rate;
	struct comp_online_present *present = NULL;
	struct dpu_composer *dpu_comp = NULL;
	ktime_t cur_vactive_timestamp = ktime_get();

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");

	dpu_comp = to_dpu_composer(get_comp_from_device(dev));
	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null\n");
		return -1;
	}

	present = (struct comp_online_present *)dpu_comp->present_data;
	dpu_check_and_return(!present, -1, err, "present_data is null pointer");

	ret = sscanf(buf, "%d", &frame_rate);
	if (ret == 0) {
		dpu_pr_err("get frame rate (%s) fail\n", buf);
		return -1;
	}

	present->frame_rate = (uint32_t)frame_rate;
	dpu_pr_info("frame_rate = %u, cur_time = %llu,",
		frame_rate,
		ktime_to_ns(cur_vactive_timestamp));
	return (ssize_t)count;
}

static inline int32_t vsync_timestamp_changed(struct dpu_vsync *vsync_ctrl, ktime_t prev_timestamp)
{
	return !(prev_timestamp == vsync_ctrl->timestamp);
}

static struct comp_online_present *get_present_data(struct device *dev)
{
	struct dpu_composer *dpu_comp = NULL;

	dpu_comp = to_dpu_composer(get_comp_from_device(dev));
	dpu_check_and_return(!dpu_comp, NULL, err, "dpu_comp is null pointer");

	return (struct comp_online_present *)dpu_comp->present_data;
}

static ssize_t vsync_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	ktime_t prev_timestamp;
	struct dpu_vsync *vsync_ctrl = NULL;
	struct comp_online_present *present = NULL;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");
	present = get_present_data(dev);
	dpu_check_and_return(!present, -1, err, "present_data is null pointer");

	vsync_ctrl = &present->vsync_ctrl;
	prev_timestamp = vsync_ctrl->timestamp;
	ret = wait_event_interruptible(vsync_ctrl->wait,
		((vsync_timestamp_changed(vsync_ctrl, prev_timestamp) != 0) && (vsync_ctrl->enabled != 0)));
	if (ret) {
		dpu_pr_warn("vsync wait event be interrupted abnormal!!!");
		return -1;
	}
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "VSYNC=%lld\n", ktime_to_ns(vsync_ctrl->timestamp));
	buf[strlen(buf) + 1] = '\0';
	return ret;
}

static ssize_t vsync_timestamp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	struct dpu_vsync *vsync_ctrl = NULL;
	struct comp_online_present *present = NULL;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");
	present = get_present_data(dev);
	dpu_check_and_return(!present, -1, err, "present_data is null pointer");

	vsync_ctrl = &present->vsync_ctrl;
	ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%lld\n", ktime_to_ns(vsync_ctrl->timestamp));
	buf[strlen(buf) + 1] = '\0';

	dpu_pr_info("buf:%s ", buf);

	return ret;
}

static ssize_t vsync_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	int32_t enable;
	struct dpu_vsync *vsync_ctrl = NULL;
	struct comp_online_present *present = NULL;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");
	present = get_present_data(dev);
	dpu_check_and_return(!present, -1, err, "present_data is null pointer");

	vsync_ctrl = &present->vsync_ctrl;
	ret = sscanf(buf, "%d", &enable);
	if (ret == 0) {
		dpu_pr_err("get buf (%s) enable fail\n", buf);
		return -1;
	}
	dpu_pr_info("dev->kobj.name: %s vsync enable=%d", dev->kobj.name, enable);
	dpu_vsync_enable(vsync_ctrl, enable);

	return (ssize_t)count;
}

static ssize_t dfr_real_frame_rate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	uint32_t real_frame_rate;
	struct dpu_composer *dpu_comp = NULL;
	struct composer *comp = NULL;
	struct comp_online_present *present = NULL;
	ktime_t curr_timestamp;
	uint64_t delta_time_us;

	dpu_check_and_return((!dev || !buf), -1, err, "input is null pointer");

	dpu_comp = to_dpu_composer(get_comp_from_device(dev));
	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null\n");
		return -1;
	}

	comp = &dpu_comp->comp;
	present = (struct comp_online_present *)dpu_comp->present_data;

	if (present->comp_maintain.self_refresh_period_us == 0) {
		dpu_pr_err("self refresh period == 0");
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", "error: self refresh period <= 0");
		buf[strlen(buf) + 1] = '\0';
		return ret;
	}

	down(&comp->blank_sem);
	dpu_pr_info("power_state: %d ", comp->power_on);
	if (comp->power_on && (present->dfr_ctrl.mode == DFR_MODE_TE_SKIP_BY_MCU ||
		present->dfr_ctrl.mode == DFR_MODE_LONGH_TE_SKIP_BY_MCU)) {
		curr_timestamp = ktime_get();
		delta_time_us = (uint64_t)(ktime_to_us(curr_timestamp) -
			ktime_to_us(present->comp_maintain.pre_refresh_timestamp));

		real_frame_rate = present->comp_maintain.real_frame_rate;
		if (delta_time_us >= present->comp_maintain.self_refresh_period_us)
			real_frame_rate = PERIOD_US_1HZ / present->comp_maintain.self_refresh_period_us;

		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u\n", real_frame_rate);
	} else {
		ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n",
			"current screen is powered off or is not a LTPO screen");
	}
	up(&comp->blank_sem);

	buf[strlen(buf) + 1] = '\0';

	dpu_pr_info("buf:%s ", buf);

	return ret;
}

static DEVICE_ATTR(vsync_event, 0440, vsync_event_show, NULL);
static DEVICE_ATTR(vsync_enable, 0200, NULL, vsync_enable_store);
static DEVICE_ATTR(vsync_timestamp, 0440, vsync_timestamp_show, NULL);
static DEVICE_ATTR(dfr_target_frame_rate, 0200, NULL, dfr_target_frame_rate_store);
static DEVICE_ATTR(dfr_real_frame_rate, 0440, dfr_real_frame_rate_show, NULL);

static uint32_t get_dpu_idle_status(struct composer_manager *comp_mgr)
{
	uint32_t dpu_idle_status = 0;
	uint32_t comp_index = 0;
	uint32_t power_on_comp_cnt = 0;
	uint32_t isr_off_comp_cnt = 0;
	uint32_t clk_off_comp_cnt = 0;

	for (comp_index = DEVICE_COMP_PRIMARY_ID; comp_index <= DEVICE_COMP_BUILTIN_ID; comp_index++) {
		if ((comp_mgr->dpu_comps[comp_index] != NULL) &&
			(comp_mgr->power_status.refcount.value[comp_index] != 0)) {
			power_on_comp_cnt++;
			if (((comp_mgr->dpu_comps[comp_index]->comp_idle_flag & VSYNC_IDLE_ISR_OFF) != 0))
				isr_off_comp_cnt++;
			if (((comp_mgr->dpu_comps[comp_index]->comp_idle_flag & VSYNC_IDLE_CLK_OFF) != 0))
				clk_off_comp_cnt++;
		}
	}
	if (power_on_comp_cnt == 0) {
		dpu_pr_warn("comp_mgr->power_status=0x%llx",comp_mgr->power_status.status);
		return dpu_idle_status;
	}
	if (power_on_comp_cnt == isr_off_comp_cnt)
		dpu_idle_status |= VSYNC_IDLE_ISR_OFF;
	if (power_on_comp_cnt == clk_off_comp_cnt)
		dpu_idle_status |= VSYNC_IDLE_CLK_OFF;
	return dpu_idle_status;
}

static bool dpu_comp_check_enter_vsync_idle(struct dpu_composer *dpu_comp)
{
	if ((dpu_comp->comp_mgr->active_status.refcount.value[dpu_comp->comp.index] == 0) &&
		(dpu_comp->comp_idle_expire_count == 0) && (dpu_comp->comp_idle_enable != 0) &&
		(!is_multi_device_active(dpu_comp->comp_mgr)))
		return true;
	return false;
}

static void dpu_comp_process_vsync_idle(struct dpu_composer *dpu_comp, struct dkmd_connector_info *pinfo)
{
	if (!dpu_comp_check_enter_vsync_idle(dpu_comp)) {
		if (dpu_comp->comp_idle_expire_count > 0)
			dpu_comp->comp_idle_expire_count--;
		return;
	}
	dpu_pr_debug("pinfo->vsync_ctrl_type=%d", pinfo->vsync_ctrl_type);

	if ((dpu_comp->comp_idle_flag & VSYNC_IDLE_RESET_VOTE) == 0) {
		dpu_pr_debug("enter dpu comp vsync idle, reset vote");
		dpu_dvfs_reset_vote(dpu_comp->comp.index);
		dpu_comp->comp_idle_flag |= VSYNC_IDLE_RESET_VOTE;
	}

	if (((pinfo->vsync_ctrl_type & VSYNC_IDLE_MIPI_ULPS) != 0) &&
		((dpu_comp->comp_idle_flag & VSYNC_IDLE_MIPI_ULPS) == 0)) {
		pipeline_next_ops_handle(pinfo->conn_device, pinfo,
			HANDLE_MIPI_ULPS, (void *)&dpu_comp->comp_idle_enable);
		dpu_comp->comp_idle_flag |= VSYNC_IDLE_MIPI_ULPS;
	}
	if (((pinfo->vsync_ctrl_type & VSYNC_IDLE_ISR_OFF) != 0) &&
		((dpu_comp->comp_idle_flag & VSYNC_IDLE_ISR_OFF) == 0)) {
		dpu_comp_smmuv3_off(dpu_comp->comp_mgr, dpu_comp);
		pipeline_next_ops_handle(pinfo->conn_device, pinfo, DISABLE_ISR, &dpu_comp->isr_ctrl);
		dpu_comp->comp_idle_flag |= VSYNC_IDLE_ISR_OFF;
	}
	if (((pinfo->vsync_ctrl_type & VSYNC_IDLE_CLK_OFF) != 0) &&
		((dpu_comp->comp_idle_flag & VSYNC_IDLE_CLK_OFF) == 0))
		dpu_comp->comp_idle_flag |= VSYNC_IDLE_CLK_OFF;
}

static void dpu_process_vsync_idle(struct composer_manager *comp_mgr,
	struct dpu_vsync *vsync_ctrl, struct dkmd_connector_info *pinfo)
{
	uint32_t dpu_idle_status = 0;

	if (!((comp_mgr->active_status.status == 0) && (comp_mgr->idle_enable)))
		return;
	dpu_idle_status = get_dpu_idle_status(comp_mgr);
	if (((dpu_idle_status & VSYNC_IDLE_ISR_OFF) != 0) &&
		((comp_mgr->idle_func_flag & VSYNC_IDLE_ISR_OFF) == 0)) {
			dkmd_dvfs_isr_disable(comp_mgr);
			dkmd_sdp_isr_disable(comp_mgr);
			dkmd_mdp_isr_disable(comp_mgr);
			comp_mgr->idle_func_flag |= VSYNC_IDLE_ISR_OFF;
	}
	if (((dpu_idle_status & VSYNC_IDLE_CLK_OFF) != 0) &&
		((comp_mgr->idle_func_flag & VSYNC_IDLE_CLK_OFF) == 0)) {
			vivobus_autodiv_regulator_disable(comp_mgr);
			dpu_dvfs_direct_vote(vsync_ctrl->dpu_comp->comp.index, DPU_CORE_LEVEL_OFF, false);
			dpu_dvfs_disable_core_clock();
			comp_mgr->idle_func_flag |= VSYNC_IDLE_CLK_OFF;
			dpu_ddr_bandwidth_release(!is_offline_panel(&pinfo->base));
	}

	// ppc status can be switched before first present of dpu idle status
	process_ppc_event(vsync_ctrl->dpu_comp, PPC_EVENT_DPU_IDLE);
}

static void vsync_trigger_handle_work(struct kthread_work *work)
{
	struct dpu_vsync *vsync_ctrl = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_composer *dpu_comp = NULL;

	vsync_ctrl = container_of(work, struct dpu_vsync, vsync_trigger_handle_work);
	dpu_check_and_no_retval(!vsync_ctrl, err, "vsync_ctrl is null pointer");
	dpu_comp = vsync_ctrl->dpu_comp;
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null pointer");
	pinfo = dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null pointer");

	pipeline_next_ops_handle(pinfo->conn_device, pinfo, SEND_CMDS_AT_VSYNC, NULL);
}

static void vsync_idle_handle_work(struct kthread_work *work)
{
	struct dpu_vsync *vsync_ctrl = NULL;
	struct composer_manager *comp_mgr = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct comp_online_present *present = NULL;

	vsync_ctrl = container_of(work, struct dpu_vsync, idle_handle_work);
	dpu_check_and_no_retval(!vsync_ctrl, err, "vsync_ctrl is null pointer");
	dpu_comp = vsync_ctrl->dpu_comp;
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null pointer");
	comp_mgr = dpu_comp->comp_mgr;
	dpu_check_and_no_retval(!comp_mgr, err, "comp_mgr is null pointer");
	pinfo = dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null pointer");
	present = (struct comp_online_present *)dpu_comp->present_data;
	dpu_check_and_no_retval(!present, err, "present is null pointer");

	if (g_debug_idle_enable == 0)
		return;
	mutex_lock(&comp_mgr->idle_lock);
	dpu_pr_debug("comp_mgr->active_status=0x%llx idle_func_flag=0x%x idle_expire_count=%d comp_idle_flag=0x%x comp=%u",
		comp_mgr->active_status.status, comp_mgr->idle_func_flag,
		dpu_comp->comp_idle_expire_count, dpu_comp->comp_idle_flag,
		dpu_comp->comp.index);
	/* enter vsync idle process directly in DFR_MODE_LONGH_TE_SKIP_BY_MCU mode */
	if (present->dfr_ctrl.mode != DFR_MODE_LONGH_TE_SKIP_BY_MCU && composer_mgr_get_dimming_status(comp_mgr) != 0) {
		mutex_unlock(&comp_mgr->idle_lock);
		return;
	}
	/* enter vsync idle */
	dpu_comp_process_vsync_idle(dpu_comp, pinfo);
	dpu_process_vsync_idle(comp_mgr, vsync_ctrl, pinfo);
	dpu_pr_debug("comp_mgr->active_status=0x%llx idle_func_flag=0x%x comp_idle_flag=0x%x comp=%u",
		comp_mgr->active_status.status, comp_mgr->idle_func_flag,
		dpu_comp->comp_idle_flag, dpu_comp->comp.index);
	mutex_unlock(&comp_mgr->idle_lock);
}

/**
 * @brief exit vsync idle right now
 *
 */
void dpu_comp_active_vsync(struct dpu_composer *dpu_comp)
{
	struct composer_manager *comp_mgr = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct comp_online_present *present = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null pointer");
	comp_mgr = dpu_comp->comp_mgr;
	dpu_check_and_no_retval(!comp_mgr, err, "comp_mgr is null pointer");
	pinfo = dpu_comp->conn_info;
	dpu_check_and_no_retval(!pinfo, err, "pinfo is null pointer");

	present = (struct comp_online_present *)dpu_comp->present_data;
	if (!present) {
		dpu_pr_err("present is null");
		return;
	}

	mutex_lock(&comp_mgr->idle_lock);
	/* Immediately exit the idle state */
	dpu_comp->comp_idle_enable = false;
	dpu_comp->comp_idle_expire_count = 0;
	comp_mgr->idle_enable = false;
	comp_mgr->active_status.refcount.value[dpu_comp->comp.index]++;

	dpu_pr_debug("comp_mgr->active_status=0x%llx idle_func_flag=0x%x comp_idle_flag=0x%x comp=%u",
		comp_mgr->active_status.status, comp_mgr->idle_func_flag, dpu_comp->comp_idle_flag, dpu_comp->comp.index);
	/* need exit vsync idle */
	if ((comp_mgr->idle_func_flag & VSYNC_IDLE_CLK_OFF) != 0) {
		dpu_dvfs_enable_core_clock(false);
		dpu_dvfs_direct_vote(dpu_comp->comp.index, DPU_CORE_LEVEL_ON, true);
		dpu_dacc_resume(comp_mgr->dpu_base);
		comp_mgr->idle_func_flag &= ~VSYNC_IDLE_CLK_OFF;
		dpu_ddr_bandwidth_recovery(!is_offline_panel(&pinfo->base));
		vivobus_autodiv_regulator_enable(comp_mgr);
	}

	if ((dpu_comp->comp_idle_flag & VSYNC_IDLE_RESET_VOTE) != 0)
		dpu_comp->comp_idle_flag &= ~VSYNC_IDLE_RESET_VOTE;

	if ((dpu_comp->comp_idle_flag & VSYNC_IDLE_CLK_OFF) != 0) {
		dpu_comp->comp_idle_flag &= ~VSYNC_IDLE_CLK_OFF;
		if (present->dfr_ctrl.exit_idle_status)
			present->dfr_ctrl.exit_idle_status(&present->dfr_ctrl);
	}

	if ((comp_mgr->idle_func_flag & VSYNC_IDLE_ISR_OFF) != 0) {
		dkmd_mdp_isr_enable(comp_mgr);
		dkmd_sdp_isr_enable(comp_mgr);
		dkmd_dvfs_isr_enable(comp_mgr);
		comp_mgr->idle_func_flag &= ~VSYNC_IDLE_ISR_OFF;
	}

	if ((dpu_comp->conn_info->vsync_ctrl_type & VSYNC_IDLE_CLK_OFF) == 0) {
		mutex_unlock(&comp_mgr->idle_lock);
		return;
	}

	if ((dpu_comp->comp_idle_flag & VSYNC_IDLE_ISR_OFF) != 0) {
		pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
			dpu_comp->conn_info, ENABLE_ISR, &dpu_comp->isr_ctrl);
		dpu_comp_smmuv3_on(comp_mgr, dpu_comp);
		dpu_comp->comp_idle_flag &= ~VSYNC_IDLE_ISR_OFF;
	}

	if ((dpu_comp->comp_idle_flag & VSYNC_IDLE_MIPI_ULPS) != 0) {
		pipeline_next_ops_handle(dpu_comp->conn_info->conn_device,
			dpu_comp->conn_info, HANDLE_MIPI_ULPS, (void *)&dpu_comp->comp_idle_enable);
		dpu_comp->comp_idle_flag &= ~VSYNC_IDLE_MIPI_ULPS;
	}

	mutex_unlock(&comp_mgr->idle_lock);
}

/**
 * @brief Restore vsync Idle Count
 *
 */
void dpu_comp_deactive_vsync(struct dpu_composer *dpu_comp)
{
	struct composer_manager *comp_mgr = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is null pointer");
	comp_mgr = dpu_comp->comp_mgr;
	dpu_check_and_no_retval(!comp_mgr, err, "comp_mgr is null pointer");

	mutex_lock(&comp_mgr->idle_lock);
	dpu_pr_debug("comp_mgr->active_status.status=0x%llx", comp_mgr->active_status.status);
	if (comp_mgr->active_status.refcount.value[dpu_comp->comp.index] > 0)
		comp_mgr->active_status.refcount.value[dpu_comp->comp.index]--;

	if (comp_mgr->active_status.refcount.value[dpu_comp->comp.index] == 0) {
		dpu_comp->comp_idle_enable = true;
		dpu_comp->comp_idle_expire_count = VSYNC_IDLE_EXPIRE_COUNT;
	}

	if (comp_mgr->active_status.status == 0)
		comp_mgr->idle_enable = true;

	mutex_unlock(&comp_mgr->idle_lock);
}

static void check_vsync_is_correct(uint64_t real_vsync, struct dpu_vsync *vsync_ctrl)
{
	uint64_t vsyncdiff = 0;
	uint64_t targetvsync = 0;
	struct comp_online_present *present = NULL;
	struct dfr_info *dinfo = NULL;

	present = (struct comp_online_present *)vsync_ctrl->dpu_comp->present_data;
	if (unlikely(present == NULL)) {
		dpu_pr_err("present is null");
		return;
	}

	if (present->dfr_ctrl.mode != DFR_MODE_TE_SKIP_BY_MCU &&
		present->dfr_ctrl.mode != DFR_MODE_LONGH_TE_SKIP_BY_MCU)
		return;

	if (unlikely(present->dfr_ctrl.dpu_comp == NULL)) {
		dpu_pr_err("dpu_comp is null");
		return;
	}

	dinfo = dkmd_get_dfr_info(present->dfr_ctrl.dpu_comp->conn_info);
	if (unlikely(dinfo == NULL)) {
		dpu_pr_err("dinfo is null");
		return;
	}

	if (dinfo->oled_info.ltpo_info.ver == PANEL_LTPO_V1 && present->dfr_ctrl.cur_frm_rate != 0) {
		// 1000000 = 1s
		targetvsync = 1000000 / present->dfr_ctrl.cur_frm_rate;
		vsyncdiff = (uint64_t)abs(real_vsync - targetvsync);
		if (vsyncdiff > VSYNC_MAX_TIME_OFFSET)
			vsync_ctrl->vsync_not_match_times++;
		else
			vsync_ctrl->vsync_not_match_times = 0;

		if (vsync_ctrl->vsync_not_match_times >= VSYNC_MAX_ERROR_TIMES) {
			dpu_pr_err("vsync no match times = %d", vsync_ctrl->vsync_not_match_times);
			present->dfr_ctrl.resend_switch_cmds = true;
			vsync_ctrl->vsync_not_match_times = 0;
		}
	}
}

static int32_t vsync_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = NULL;
	struct dpu_vsync *vsync_ctrl = NULL;
	ktime_t pre_vsync_timestamp;
	uint64_t real_vsync = 0;
	struct dpu_composer *dpu_comp = NULL;
	struct dpu_connector *connector = NULL;
	struct comp_online_present *present = NULL;

	dpu_check_and_return(!data, -EINVAL, err, "data is null pointer\n");
	listener_data = (struct dkmd_listener_data *)data;
	dpu_check_and_return(!listener_data, -EINVAL, err, "listener_data is null pointer\n");
	vsync_ctrl = (struct dpu_vsync *)(listener_data->data);
	dpu_check_and_return(!vsync_ctrl, -EINVAL, err, "vsync_ctrl is null pointer\n");
	dpu_comp = vsync_ctrl->dpu_comp;
	dpu_check_and_return(!dpu_comp, -EINVAL, err, "dpu_comp is null pointer\n");
	present = (struct comp_online_present *)dpu_comp->present_data;
	dpu_check_and_return(!present, -EINVAL, err, "present is null pointer\n");

	if ((present->frame_start_flag == 0) && is_ppc_status_wait_vsync(&present->ppc_ctrl)) {
		dpu_pr_info("ppc status switchable, can set active rect");
		process_ppc_event(dpu_comp, PPC_EVENT_VSYNC_OK);
		wake_up_interruptible_all(&present->ppc_ctrl.ppc_cmd_start_wq);
	}

	if (!vsync_ctrl->routine_enabled)
		return 0;

	if ((action & vsync_ctrl->listening_isr_bit) == 0)
		return 0;

	if (unlikely(!dpu_comp->conn_info))
		return 0;

	connector = get_primary_connector(dpu_comp->conn_info);
	pre_vsync_timestamp = vsync_ctrl->timestamp;
	vsync_ctrl->timestamp = listener_data->notify_timestamp;

	real_vsync = (uint64_t)ktime_us_delta(vsync_ctrl->timestamp, pre_vsync_timestamp);
	check_vsync_is_correct(real_vsync, vsync_ctrl);

	if (dpu_vsync_is_enabled(vsync_ctrl))
		wake_up_interruptible_all(&(vsync_ctrl->wait));

	if ((g_dpu_config_data.version.info.hw_type == DPU_HW_TYPE_ASIC) &&
		(is_mipi_cmd_panel(&dpu_comp->conn_info->base) || is_mipi_video_panel(&dpu_comp->conn_info->base)) &&
		connector->is_vactive_end_recieved != VACTIVE_END_MISS_REPORTED) {
		if (connector->is_vactive_end_recieved == VACTIVE_END_WAIT) {
			if (g_debug_dmd_report_vact_end_miss == 1)
				dksm_dmd_report_vactive_end_miss(dpu_comp->conn_info->connector_idx[PRIMARY_CONNECT_CHN_IDX]);
			dpu_pr_warn("NOTICE: do not receive vactive end itr of last frame, conn_id = %u",
				dpu_comp->conn_info->connector_idx[PRIMARY_CONNECT_CHN_IDX]);
			connector->is_vactive_end_recieved = VACTIVE_END_MISS_REPORTED;
		}
	}

	if (g_debug_vsync_dump)
		dpu_pr_info("VSYNC = %llu, time_diff = %llu us", ktime_to_ns(vsync_ctrl->timestamp),
			ktime_us_delta(vsync_ctrl->timestamp, pre_vsync_timestamp));

	if (vsync_ctrl->dpu_comp->conn_info->vsync_ctrl_type != 0)
		kthread_queue_work(&vsync_ctrl->dpu_comp->handle_worker, &vsync_ctrl->idle_handle_work);

	if (dpu_comp->bl_ctrl.bl_updated != 0)
		kthread_queue_work(&vsync_ctrl->dpu_comp->handle_worker, &vsync_ctrl->vsync_trigger_handle_work);

	dpu_display_engine_vsync_queue_work();
	return 0;
}

static struct notifier_block vsync_isr_notifier = {
	.notifier_call = vsync_isr_notify,
};

void dpu_vsync_init(struct dpu_vsync *vsync_ctrl, struct dkmd_attr *attrs)
{
	dpu_check_and_no_retval(!attrs, err, "attrs is null pointer");
	spin_lock_init(&(vsync_ctrl->spin_enable));
	init_waitqueue_head(&vsync_ctrl->wait);

	vsync_ctrl->enabled = 0;
	vsync_ctrl->routine_enabled = false;
	vsync_ctrl->notifier = &vsync_isr_notifier;
	kthread_init_work(&vsync_ctrl->idle_handle_work, vsync_idle_handle_work);
	kthread_init_work(&vsync_ctrl->vsync_trigger_handle_work, vsync_trigger_handle_work);
	vsync_ctrl->vsync_not_match_times = 0;

	dkmd_sysfs_attrs_append(attrs, &dev_attr_vsync_event.attr);
	dkmd_sysfs_attrs_append(attrs, &dev_attr_vsync_enable.attr);
	dkmd_sysfs_attrs_append(attrs, &dev_attr_vsync_timestamp.attr);
	dkmd_sysfs_attrs_append(attrs, &dev_attr_dfr_target_frame_rate.attr);
	dkmd_sysfs_attrs_append(attrs, &dev_attr_dfr_real_frame_rate.attr);
}
