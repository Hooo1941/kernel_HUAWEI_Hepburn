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

#include <platform_include/display/linux/dpu_dss_dp.h>
#include "dp_drv.h"
#include "dp_ctrl.h"
#include "dpu_conn_mgr.h"
#include "dpu_comp_mgr.h"
#include "dvfs.h"
#include "dpu_dp_dbg.h"
#include "dp_ctrl_config.h"

#define DP_PLUGOUT_INTER_COUNT (3)

static void dp_on_params_init(struct dp_ctrl *dptx)
{
	dptx->dptx_enable = true;
	dptx->detect_times = 0;
	dptx->current_link_rate = dptx->max_rate;
	dptx->current_link_lanes = dptx->max_lanes;
}

static int dp_on(struct dp_ctrl *dptx)
{
	struct dp_private *dp_priv = NULL;
	dp_priv = to_dp_private(dptx->connector->conn_info);

	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");

	if (dptx->video_transfer_enable) {
		dpu_pr_info("[DP] dptx has already on\n");
		return 0;
	}

	if (dptx->dptx_combophy_reset && (dptx->dptx_combophy_reset(dptx, true) != 0)) {
		dpu_pr_err("[DP] dptx PHY init failed!\n");
		return -1;
	}

	if (dptx->dp_dis_reset && (dptx->dp_dis_reset(dptx, true) != 0)) {
		dpu_pr_err("[DP] dptx dis reset failed!\n");
		return -1;
	}

	if (dptx->dptx_ctrl_clk_enable(dptx) != 0) {
		dpu_pr_err("[DP] dptx ctrl clk enable failed!\n");
		return -1;
	}

	if (dptx->dptx_core_on && (dptx->dptx_core_on(dptx) != 0)) {
		dpu_pr_err("[DP] dptx core on failed!\n");
		return -1;
	}
	dp_on_params_init(dptx);

	return 0;
}

static void dp_hpd_status_init(struct dp_ctrl *dptx, TYPEC_PLUG_ORIEN_E typec_orien)
{
	struct dp_private *dp_priv = NULL;

	dptx->same_source = dpu_dp_get_current_dp_source_mode();
	dptx->user_mode = 0;
	dptx->dptx_plug_type = typec_orien;
	dptx->user_mode_format = VCEA;

#if !defined(CONFIG_PRODUCT_CDC_ACE)
	dp_priv = to_dp_private(dptx->connector->conn_info);
	dpu_check_and_no_retval(!dp_priv, err, "DPTX is null!");
	/* DP HPD event must be delayed when system is booting */
	if (!dp_priv->dp_hpd_plug && !g_dpu_complete_start)
		wait_event_interruptible_timeout(dptx->dptxq, g_dpu_complete_start, msecs_to_jiffies(20000));
#endif
}

static int dp_get_lanes_mode(TCPC_MUX_CTRL_TYPE mode, uint8_t *dp_lanes)
{
	switch (mode) {
	case TCPC_DP:
		*dp_lanes = 4;
		dpu_pr_info("[DP] dp_get_lanes_mode lanes=4\n");
		break;
	case TCPC_USB31_AND_DP_2LINE:
		*dp_lanes = 2;
		dpu_pr_info("[DP] dp_get_lanes_mode lanes=2\n");
		break;
	default:
		*dp_lanes = 4;
		dpu_pr_err("[DP] not supported tcpc_mux_ctrl_type=%d\n", mode);
		return -EINVAL;
	}

	return 0;
}

static void dp_reset_max_params(struct dp_ctrl *dptx, TCPC_MUX_CTRL_TYPE mode)
{
	dptx->max_rate = (uint8_t)dptx->connector->conn_info->max_dptx_rate;
	dpu_pr_info("[DP] dp_reset_max_params rate=%u\n", dptx->max_rate);
	dp_get_lanes_mode(mode, &dptx->max_lanes);
}

static void dp_off_params_reset(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval(!dptx, err, "[DP] dptx is NULL!\n");
	dptx->detect_times = 0;
	dptx->dptx_vr = false;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx->dptx_plug_type = DP_PLUG_TYPE_NORMAL;
}

static int dp_off(struct dp_ctrl *dptx)
{
	int ret = 0;
	struct dp_private *dp_priv = NULL;
	dp_priv = to_dp_private(dptx->connector->conn_info);

	dpu_check_and_return(!dp_priv, -EINVAL, err, "[DP] dp_priv is null pointer\n");
	dp_off_params_reset(dptx);

	if (dptx->dptx_core_off)
		dptx->dptx_core_off(dptx);

	if (dptx->dp_dis_reset && (dptx->dp_dis_reset(dptx, false) != 0)) {
		dpu_pr_err("[DP] DPTX dis reset failed !!!\n");
		ret = -1;
	}
	if (dptx->dptx_combophy_reset && (dptx->dptx_combophy_reset(dptx, false) != 0)) {
		dpu_pr_err("[DP] dptx PHY close failed!\n");
		return -1;
	}

	/* disable  clk */
	if (dptx->dp_disable_clk)
		dptx->dp_disable_clk(dptx);

	/* reset dprx of 983 */
	if (dptx->dp_info && dptx->dp_info->is_dprx_ready && dptx->dp_info->dp_connect_ctrl_reset_dprx != NULL) {
		dpu_pr_info("[DP] dp_connect_ctrl_reset_dprx entry");
		dptx->dp_info->dp_connect_ctrl_reset_dprx(dptx->port_id);
	}

	/* set dp power saving mode */
	if (dptx->dptx_combophy_power_saving_handler)
		dptx->dptx_combophy_power_saving_handler(dptx);

	return ret;
}

static int dpu_dptx_handle_plugin(struct dp_ctrl *dptx, int mode, int typec_orien)
{
	int retval = 0;
	if (dptx->dptx_enable) {
		dpu_pr_info("[DP] dptx has already connected");
		return 0;
	}

	if (g_dp_debug_mode_enable)
		dptx_debug_set_params(dptx);

	dp_hpd_status_init(dptx, typec_orien);
	dp_reset_max_params(dptx, mode);
	if (dp_on(dptx) != 0) {
		dpu_pr_err("[DP] dp_on error\n");
		return -EINVAL;
	}

	mdelay(1);
	if (dptx->dptx_hpd_handler)
		retval = dptx->dptx_hpd_handler(dptx, true, dptx->max_lanes);

	if (retval) {
		dpu_pr_err("[DP] hpd handler failed ret: %d", retval);
		dptx->dptx_enable = false;
		return -EINVAL;
	}
	dpu_dvfs_increase_count_locked();
	return 0;
}

static int dpu_dptx_check_power(struct dp_ctrl *dptx, int mode, int typec_orien)
{
	if (!dptx->dptx_enable) {
		if (dptx->dp_disable_clk)
			dptx->dp_disable_clk(dptx);
		dpu_pr_info("[DP] dptx has already off");
		return -EINVAL;
	}
	/* already power off, need power on first */
	if (dptx->power_saving_mode && dptx->dptx_power_handler) {
		dpu_pr_info("[DP] dptx has already off, power on first");
		dptx->dptx_power_handler(dptx, true);
		dptx->power_saving_mode = false;
	}

	dp_hpd_status_init(dptx, typec_orien);
	dp_reset_max_params(dptx, mode);

	return 0;
}

static int dpu_dptx_handle_plugout(struct dp_ctrl *dptx, int mode, int typec_orien)
{
	dptx->is_pluging_out = true;
	if (dpu_dptx_check_power(dptx, mode, typec_orien) != 0) {
		dpu_pr_info("[DP] dptx check power error!");
		dptx->is_pluging_out = false;
		return -1;
	}

	if (dptx->dptx_hpd_handler)
		dptx->dptx_hpd_handler(dptx, false, dptx->max_lanes);

	mdelay(1);
	dp_off(dptx);
	dpu_dvfs_set_inter_vote_index_count(DP_PLUGOUT_INTER_COUNT);
	dptx->is_pluging_out = false;

	return 0;
}

static int dpu_dptx_handle_shot_plug(struct dp_ctrl *dptx, int mode, int typec_orien)
{
	if (dpu_dptx_check_power(dptx, mode, typec_orien) != 0)
		return -1;

	if (dptx->dptx_hpd_irq_handler)
		dptx->dptx_hpd_irq_handler(dptx);

	return 0;
}

int dpu_multi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode,
								TYPEC_PLUG_ORIEN_E typec_orien, uint32_t dp_id, int port_id)
{
	int ret = 0;
	struct dp_ctrl *dptx = NULL;
	struct dp_private *dp_priv = NULL;
	struct dkmd_connector_info *pinfo = NULL;
	struct dkmd_object_info *upload_pinfo = NULL;
	uint32_t dev_index = GFX_DP; // dev_index: 0 1 2
	struct composer *comp = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct comp_online_present *present = NULL;

	dpu_pr_info("[DP] dpu_multi_dptx_hpd_trigger entry\n");
	dptx_pixel_pll_preinit();

	if (dp_ctrl_is_bypass_by_pg((uint32_t)port_id)) {
		dpu_pr_err("[DP] port_id: %d. edp is unavailable by pg, so power on bypass", port_id);
		return -EINVAL;
	}

	if (dp_id >= MAX_DPTX_COUNT) {
		dpu_pr_err("[DP] input error dptx index\n");
		return -EINVAL;
	}

	ret = get_dp_devive_index(port_id, &dev_index);
	if (ret == -1) {
		dpu_pr_err("[DP] get_dp_devive_index: dev_index get error\n");
		return -EINVAL;
	}
	dpu_pr_info("[DP] get_dp_devive_index done: dev_index is %u\n", dev_index);

	dpu_check_and_return(!g_dkmd_dp_devive[dev_index], -EINVAL, err, "[DP] dp device is null pointer!");
	pinfo = platform_get_drvdata(g_dkmd_dp_devive[dev_index]);

	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] get connector info failed!");
	dp_priv = to_dp_private(pinfo);
	upload_pinfo = pinfo->base.comp_obj_info;

	comp = container_of(upload_pinfo, struct composer, base);
	if (!comp) {
		dpu_pr_err("[DP] comp is nullptr!");
		return -EINVAL;
	}
	dpu_comp = to_dpu_composer(comp);
	if (!dpu_comp) {
		dpu_pr_err("[DP] dpu_comp is nullptr");
		return -EINVAL;
	}
	present = (struct comp_online_present *)dpu_comp->present_data;
	if (!present) {
		dpu_pr_err("[DP] present is nullptr!");
		return -EINVAL;
	}

	dptx = &dp_priv->dp[dp_id];
	dpu_pr_info("[DP] DP HPD Type: %d, Mode: %d, Gate: %d", irq_type, mode, g_dpu_complete_start);

	if (!dptx->is_dual_tx && (dp_id == SLAVE_DPTX_IDX)) {
		dpu_pr_err("[DP] input error dptx index\n");
		return -EINVAL;
	}

	dptx->port_id = port_id;
	dptx->mode = mode;
	dptx->typec_orien = typec_orien;
	dpu_pr_info("[DP] trigger input port id is %d\n", dptx->port_id);

	dptx->is_double_tx_sync = false;
	dptx->port_id_sync = DPTX_INVALID_PORT_ID;
	dptx->dpc_index_selected = DPTX_DPC_NUM_0;

	dptx_init_dpc_num(dptx);

	mutex_lock(&dptx->dptx_mutex);
	switch (irq_type) {
	case TCA_IRQ_HPD_IN:
		/* connect_status need update before dp set uevent to hwc */
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_CONNECTED;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		ret = dpu_dptx_handle_plugin(dptx, mode, typec_orien);
		/* set dp power saving mode */
		if ((ret != 0) && dptx->dptx_combophy_power_saving_handler)
			dptx->dptx_combophy_power_saving_handler(dptx);
		break;
	case TCA_IRQ_HPD_OUT:
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_DISCONNECTED;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		ret = dpu_dptx_handle_plugout(dptx, mode, typec_orien);
		dpu_pr_info("[DP] portid: %d, dptx plugout success !\n", dptx->port_id);
		break;
	case TCA_IRQ_SHORT:
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_SHORTPLUG;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		ret = dpu_dptx_handle_shot_plug(dptx, mode, typec_orien);
		present->vactive_start_flag = 1;
		present->frame_start_flag = 0;
		mutex_lock(pinfo->base.pluggable_connect_mutex);
		upload_pinfo->connect_status = pinfo->base.connect_status = DP_CONNECTED;
		mutex_unlock(pinfo->base.pluggable_connect_mutex);
		break;
	default:
		dpu_pr_err("[DP] irq_type not matched! irq_type: %d\n", irq_type);
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&dptx->dptx_mutex);

	return ret;
}

int dpu_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode,
	TYPEC_PLUG_ORIEN_E typec_orien, int port_id)
{
	int ret = 0;

	ret = dpu_multi_dptx_hpd_trigger(irq_type, mode, typec_orien, MASTER_DPTX_IDX, port_id);

	return ret;
}

int dpu_dptx_notify_switch(void)
{
	return 0;
}

bool dpu_dptx_ready(void)
{
	return true;
}

int dpu_dptx_triger(bool benable)
{
	return 0;
}

int dpu_dptx_register_ext_disp_callback(struct dp_connect_ctrl_info *dp_info,
		uint32_t irq_type, uint32_t connect_type) {
	int ret = 0;
	uint32_t dev_index = GFX_DP; // dev_index: 0 1 2
	struct dkmd_connector_info *pinfo = NULL;
	struct dp_private *dp_priv = NULL;
	struct dp_ctrl *dptx = NULL;

	if (dp_info == NULL) {
		dpu_pr_err("[DP]dpu_dptx_register_ext_disp_callback failed !");
		return -EINVAL;
	}

	ret = get_dp_devive_index(dp_info->port_id, &dev_index);
	if (ret == -1) {
		dpu_pr_err("[DP] get_dp_devive_index: dev_index get error\n");
		return -EINVAL;
	}

	if (dev_index < GFX_DP || dev_index > GFX_DP2) {
		dpu_pr_err("[DP] dev_index is invalid! \n");
		return -EINVAL;
	}
	dpu_check_and_return(!g_dkmd_dp_devive[dev_index], -EINVAL, err, "[DP] dp device is null pointer!");

	pinfo = platform_get_drvdata(g_dkmd_dp_devive[dev_index]);
	dpu_check_and_return(!pinfo, -EINVAL, err, "[DP] get connector info failed!");
	pinfo->bl_info.bl_max = dp_info->max_brightness;
	dp_priv = to_dp_private(pinfo);
	dptx = &dp_priv->dp[0];

	if (dptx->dp_info == NULL) {
		dptx->dp_info = kzalloc(sizeof(struct dp_connect_ctrl_info), GFP_KERNEL);
		if (dptx->dp_info == NULL) {
			dpu_pr_err("[DP] dptx dp_info is NULL!");
			return -ENOMEM;
		}
	}

	dptx->dp_info->port_id = dp_info->port_id;
	dptx->dp_info->is_dprx_ready = dp_info->is_dprx_ready;
	dptx->dp_info->max_brightness = dp_info->max_brightness;
	dptx->dp_info->set_backlight = dp_info->set_backlight;

	dptx->dp_info->get_ext_disp_info = dp_info->get_ext_disp_info;
	dptx->dp_info->notify_ser_dp_vp_sync = dp_info->notify_ser_dp_vp_sync;
	dptx->dp_info->notify_ser_dp_unplug = dp_info->notify_ser_dp_unplug;
	dptx->dp_info->dp_connect_ctrl_reset_dprx = dp_info->dp_connect_ctrl_reset_dprx;

	dptx->port_id = dp_info->port_id;
	dptx->dp_info->edid_len = dp_info->edid_len;
	dptx->dp_info->screen_num = dp_info->screen_num;

	dpu_pr_info("[DP]dpu_dptx_register_ext_disp_callback start!");
	if (!dptx->dp_info->is_dprx_ready || !dptx->is_dptx_ready) {
		dpu_pr_err("[DP] dprx or dptx not ready!");
		return -EINVAL;
	}

	if (connect_type) {
		dpu_pr_info("[DP] dp%d enter!\n", dptx->port_id);
		mutex_lock(&dptx->dptx_mutex);
		if (!dptx->power_saving_mode && dptx->video_transfer_enable && dptx->dptx_power_handler) {
			dpu_pr_info("[DP] dptx %d has already connected, power off for blank!", dptx->port_id);
			dptx->dptx_power_handler(dptx, false);
			dptx->power_saving_mode = true;
			dptx->power_fake_saving_mode = true;
			msleep(100);
			dpu_pr_info("[DP] dptx has already blank, power on for unblank!");
			dptx->dptx_power_handler(dptx, true);
			dptx->power_saving_mode = false;
			dptx->power_fake_saving_mode = false;
		}
		dpu_pr_info("[DP] dptx_power_handler finish");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	ret = dpu_dptx_hpd_trigger(irq_type, TCPC_DP, DP_PLUG_TYPE_NORMAL, dptx->port_id);

	return ret;
}

#ifdef CONFIG_DKMD_DEBUG_ENABLE
EXPORT_SYMBOL(dpu_multi_dptx_hpd_trigger);
EXPORT_SYMBOL(dpu_dptx_hpd_trigger);
EXPORT_SYMBOL(dpu_dptx_notify_switch);
EXPORT_SYMBOL(dpu_dptx_ready);
EXPORT_SYMBOL(dpu_dptx_triger);
EXPORT_SYMBOL(dpu_dptx_register_ext_disp_callback);
#endif
