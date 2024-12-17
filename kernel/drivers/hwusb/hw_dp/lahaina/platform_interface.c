/*
 * platform_interface.c
 *
 * interface for lahaina dp module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwusb/hw_dp/dp_interface.h>
#include <linux/module.h>
#include <drm/drm_dp_helper.h>
#include <huawei_platform/log/hw_log.h>
#include "../dp_dsm_internal.h"

#define HWLOG_TAG dp_interface
HWLOG_REGIST();

#define DP_CONFIGURE_MASK     0x3f
#define DP_HPD_STATE_MASK     0x40
#define DP_HPD_IRQ_MASK       0x80
#define DP_HPD_STATE_OFFSET   6
#define DP_HPD_IRQ_OFFSET     7

enum dp_phy_rate {
	HWDP_PHYIF_CTRL_RATE_RBR = 0,
	HWDP_PHYIF_CTRL_RATE_HBR,
	HWDP_PHYIF_CTRL_RATE_HBR2,
	HWDP_PHYIF_CTRL_RATE_HBR3,
};

enum dp_hdcp_state {
	HDCP_STATE_INACTIVE,
	HDCP_STATE_AUTHENTICATING,
	HDCP_STATE_AUTHENTICATED,
	HDCP_STATE_AUTH_FAIL,
};

static int dp_qcom_bw_to_phy_rate(uint8_t bw)
{
	switch (bw) {
	case DP_LINK_BW_1_62:
		return HWDP_PHYIF_CTRL_RATE_RBR;
	case DP_LINK_BW_2_7:
		return HWDP_PHYIF_CTRL_RATE_HBR;
	case DP_LINK_BW_5_4:
		return HWDP_PHYIF_CTRL_RATE_HBR2;
	case DP_LINK_BW_8_1:
		return HWDP_PHYIF_CTRL_RATE_HBR3;
	default:
		hwlog_err("[DP] Invalid bw 0x%x\n", bw);
		return -EINVAL;
	}
}

void qcom_dp_set_basic_info(struct dp_panel *panel, struct dp_link *link,
	struct dp_catalog *catalog)
{
	bool same_mode = true;
	int bw_val;
	int ret;

	if (!panel || !link || !catalog)
		return;

	huawei_dp_imonitor_set_param(DP_PARAM_WIDTH, &(panel->pinfo.h_active));
	huawei_dp_imonitor_set_param(DP_PARAM_HIGH, &(panel->pinfo.v_active));
	huawei_dp_imonitor_set_param(DP_PARAM_FPS, &(panel->pinfo.refresh_rate));
	huawei_dp_imonitor_set_param(DP_PARAM_PIXEL_CLOCK,
		&(panel->pinfo.pixel_clk_khz));
	bw_val = dp_qcom_bw_to_phy_rate(link->link_params.bw_code);
	huawei_dp_imonitor_set_param(DP_PARAM_LINK_RATE, &bw_val);
	huawei_dp_imonitor_set_param(DP_PARAM_LINK_LANES,
		&(link->link_params.lane_count));
	huawei_dp_imonitor_set_param(DP_PARAM_TU, &(catalog->panel.dp_tu));

	same_mode = huawei_dp_get_current_dp_source_mode();
	huawei_dp_imonitor_set_param(DP_PARAM_SOURCE_MODE, &(same_mode));

	if (link->hdcp_status.hdcp_state == HDCP_STATE_AUTH_FAIL) {
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_F, NULL);
	} else if (link->hdcp_status.hdcp_state == HDCP_STATE_AUTHENTICATED) {
		dp_imonitor_set_param(DP_PARAM_HDCP_VERSION,
			&link->hdcp_status.hdcp_version);
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_S, NULL);
	}

	/* for factory test */
	if (huawei_dp_factory_mode_is_enable()) {
		if (!huawei_dp_factory_is_4k_60fps(
			dp_qcom_bw_to_phy_rate(link->link_params.bw_code),
			link->link_params.lane_count, panel->pinfo.h_active,
			panel->pinfo.v_active, panel->pinfo.refresh_rate)) {
			hwlog_err("can't display when combinations is invalid in factory mode!\n");
			ret = -EINVAL;
			huawei_dp_imonitor_set_param(DP_PARAM_HOTPLUG_RETVAL, &ret);
		}
	}
}
EXPORT_SYMBOL_GPL(qcom_dp_set_basic_info);

void qcom_dp_set_sink_caps(struct dp_panel *dp_panel)
{
	uint8_t bw_code;
	int bw_val;
	int valid_extensions;
	int block;

	if (!dp_panel)
		return;

	/* sink rate and lane */
	bw_code = drm_dp_link_rate_to_bw_code(dp_panel->link_info.rate);
	bw_val = dp_qcom_bw_to_phy_rate(bw_code);
	hwlog_info("rate=%d lane=%d\n", bw_val, dp_panel->link_info.num_lanes);
	huawei_dp_imonitor_set_param(DP_PARAM_MAX_RATE, &bw_val);
	huawei_dp_imonitor_set_param(DP_PARAM_MAX_LANES,
		&dp_panel->link_info.num_lanes);

	/* edid */
	valid_extensions = dp_panel->edid_ctrl->edid[0].extensions;
	hwlog_debug("valid_extensions = %d\n", valid_extensions);
	for (block = 0; block <= valid_extensions; block++)
		huawei_dp_imonitor_set_param(DP_PARAM_EDID + block,
			&(dp_panel->edid_ctrl->edid[block]));

	huawei_dp_save_edid((uint8_t *)&dp_panel->edid_ctrl->edid[0],
		sizeof(struct edid));

#ifdef CONFIG_VR_DISPLAY
	bool dptx_vr = false;
	char monitor_name_info[DP_DSM_MONTIOR_INFO_SIZE] = {0};

	dp_set_monitor_info(&dp_panel->edid_ctrl->edid[0],
		DP_DSM_MONTIOR_INFO_SIZE, &monitor_name_info);
	if (!(strncmp("HUAWEIAV02", monitor_name_info, strlen("HUAWEIAV02"))) ||
		!(strncmp("HUAWEIAV03", monitor_name_info, strlen("HUAWEIAV03")))) {
		dptx_vr = true;
		DP_INFO("The display is VR\n");
	}
	huawei_dp_set_dptx_vr_status(dptx_vr);
#endif /* CONFIG_VR_DISPLAY */

	huawei_dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO,
		&(dp_panel->audio_supported));
	huawei_dp_imonitor_set_param(DP_PARAM_DPCD_RX_CAPS, &dp_panel->dpcd);
}
EXPORT_SYMBOL_GPL(qcom_dp_set_sink_caps);

void qcom_dp_set_pd_event(uint8_t cur_hpd_high, uint8_t connected,
	uint8_t dp_data, uint8_t orientation)
{
	uint8_t cur_mode = connected ? TCPC_DP : TCPC_NC;
	uint8_t pin, hpd_state, hpd_irq;
	uint8_t mode_type, irq_type;
	uint8_t dev_type = 0;

	pin = dp_data & DP_CONFIGURE_MASK;
	hpd_state = (dp_data & DP_HPD_STATE_MASK) >> DP_HPD_STATE_OFFSET;
	hpd_irq = (dp_data & DP_HPD_IRQ_MASK) >> DP_HPD_IRQ_OFFSET;

	mode_type = pin ? TCPC_DP : TCPC_NC;
	irq_type = !!hpd_state;

	if (!cur_hpd_high && hpd_state)
		dev_type = TCA_DP_IN;
	else if (cur_hpd_high && !hpd_state)
		dev_type = TCA_DP_OUT;

	if (hpd_irq)
		irq_type = TCA_IRQ_SHORT;

	huawei_dp_imonitor_set_param(DP_PARAM_IRQ_VECTOR, &hpd_irq);
	huawei_dp_imonitor_set_pd_event(irq_type, cur_mode, mode_type,
		dev_type, orientation);
}
EXPORT_SYMBOL_GPL(qcom_dp_set_pd_event);

