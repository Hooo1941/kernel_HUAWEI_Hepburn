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

#include "dpu_conn_mgr.h"

#ifdef CONFIG_DKMD_DPU_HDMI_TX
#include "hdmitx_ctrl_dev.h"
#endif

struct connector_dsi_match dsi_match[] = {
	{CONNECTOR_ID_DSI0, CONNECTOR_ID_DSI0},
	{CONNECTOR_ID_DSI1, CONNECTOR_ID_DSI1},
	{CONNECTOR_ID_DSI2, CONNECTOR_ID_DSI2},
	{CONNECTOR_ID_DP, CONNECTOR_ID_DP}, // dp
	{CONNECTOR_ID_EDP,CONNECTOR_ID_EDP}, // edp
	{CONNECTOR_ID_OFFLINE, CONNECTOR_ID_OFFLINE}, // offline
	{CONNECTOR_ID_HDMITX, CONNECTOR_ID_HDMITX}, // hdmitx
	{CONNECTOR_ID_DSI0_BUILTIN, CONNECTOR_ID_DSI0},
	/* add for v740 dp */
	{CONNECTOR_ID_DP1, CONNECTOR_ID_DP1}, // dp1
	{CONNECTOR_ID_DP2, CONNECTOR_ID_DP2}, // dp2
	{CONNECTOR_ID_DP3, CONNECTOR_ID_DP3}, // dp3
	{CONNECTOR_ID_EDP1,CONNECTOR_ID_EDP1}, // edp1
};

uint32_t get_connector_phy_id(uint32_t connector_id)
{
	if (connector_id >= CONNECTOR_ID_MAX) {
		dpu_pr_err("get primary connector id fail: %u\n", connector_id);
		return 0;
	}
	return dsi_match[connector_id].connector_phy_id;
}

void base_panel_connector_dts_parse(struct dkmd_connector_info *pinfo, struct device_node *np)
{
	int32_t ret;
	uint32_t connector_count = 0;
	uint32_t sw_post_chn_num = 0;
	uint32_t value = 0;
	const __be32 *p = NULL;
	struct property *prop = NULL;
	uint32_t scene_count = 0;

	if (unlikely(!pinfo || !np)) {
		dpu_pr_err("pinfo or np is nullptr!\n");
		return;
	}

	if (of_property_read_u32(np, "active_flag", &pinfo->active_flag) != 0)
		dpu_pr_info("get active flag failed");

	ret = of_property_read_u32(np, "enable_lbuf_reserve", &pinfo->base.enable_lbuf_reserve);
	if (ret) {
		dpu_pr_info("get enable_lbuf_reserve failed!\n");
		pinfo->base.enable_lbuf_reserve = 0;
	}
	dpu_pr_info("enable_lbuf_reserve=%u", pinfo->base.enable_lbuf_reserve);

	of_property_for_each_u32(np, "scene", prop, p, value) {
		if ((scene_count > ONLINE_SCENE_MAX) || (value > DPU_SCENE_ONLINE_3)) {
			scene_count = 0;
			dpu_pr_err("scene config error\n");
			break;
		}
		pinfo->base.scene_info.scene_id[scene_count++] = value;
		dpu_pr_info("scene_id=%#x\n", value);
	}
	pinfo->base.scene_info.scene_count = scene_count;
	dpu_pr_info("scene_count=%#x\n", pinfo->base.scene_info.scene_count);

	ret = of_property_read_u64(np, "opr_policy", &pinfo->base.opr_policy);
	if (ret)
		pinfo->base.opr_policy = DEFAULT_OPR_MASK;
	dpu_pr_info("opr_policy=%#llx", pinfo->base.opr_policy);

	ret = of_property_read_u8(np, "compose_policy", &pinfo->base.compose_policy);
	if (ret)
		pinfo->base.compose_policy = 0;
	dpu_pr_info("compose_policy=%#x\n", pinfo->base.compose_policy);

	of_property_for_each_u32(np, "connector_id", prop, p, value) {
		if (connector_count >= MAX_CONNECT_CHN_NUM)
			break;

		dpu_pr_info("connector id =%#x", value);
		pinfo->connector_idx[connector_count] = value;
		if (unlikely(!g_conn_manager)) {
			dpu_pr_err("g_conn_manager is NULL");
			return;
		}
		dpu_assert(!g_conn_manager->connector[value % CONNECTOR_ID_MAX]);
		++connector_count;
	}

	of_property_for_each_u32(np, "pipe_sw_post_chn_idx", prop, p, value) {
		if (sw_post_chn_num >= MAX_CONNECT_CHN_NUM)
			break;

		dpu_pr_info("connector sw_post_chn_idx=%#x is active_flag=%u", value, pinfo->active_flag);
		pinfo->sw_post_chn_idx[sw_post_chn_num] = value;
		pinfo->sw_post_chn_num = ++sw_post_chn_num;
	}

	dpu_pr_info("sw_post_chn_num=%u connector_count=%u", pinfo->sw_post_chn_num, connector_count);
	dpu_assert(pinfo->sw_post_chn_num != connector_count);
	if (connector_count > EXTERNAL_CONNECT_CHN_IDX){
		get_primary_connector(pinfo)->bind_connector = get_external_connector(pinfo);
		dpu_pr_info("cphy 1+1 or dual-mipi mode or dual-connector!\n");
	}
}

void dpu_connector_setup(struct dpu_connector *connector)
{
	if (!connector) {
		dpu_pr_err("connector is nullptr!");
		return;
	}
	dpu_pr_info("[dpu_connector_setup]connector->connector_id = %d", connector->connector_id);
	switch (connector->connector_id) {
	case CONNECTOR_ID_DSI0:
	case CONNECTOR_ID_DSI1:
	case CONNECTOR_ID_DSI2:
	case CONNECTOR_ID_DSI0_BUILTIN:
		mipi_dsi_default_setup(connector);
		break;
#ifdef CONFIG_DKMD_DPU_DP
	case CONNECTOR_ID_EDP:
	case CONNECTOR_ID_EDP1:
	case CONNECTOR_ID_DP:
	case CONNECTOR_ID_DP1:
	case CONNECTOR_ID_DP2:
	case CONNECTOR_ID_DP3:
		dp_ctrl_default_setup(connector);
		break;
#endif
	case CONNECTOR_ID_OFFLINE:
		dpu_offline_default_setup(connector);
		break;
#ifdef CONFIG_DKMD_DPU_HDMI_TX
	case CONNECTOR_ID_HDMITX:
		hdmitx_ctrl_default_setup(connector);
		break;
#endif
	default:
		dpu_pr_err("invalid pipe_sw post channel!");
		return;
	}
	connector->connector_base = connector->dpu_base + g_connector_offset[connector->connector_id];
	connector->dpp_base = connector->dpu_base + DPU_DPP0_OFFSET;
	connector->dsc_base = connector->dpu_base + DPU_DSC0_OFFSET;

	/* for dual-mipi or cphy 1+1 */
	if (connector->bind_connector &&
		(connector->connector_id != CONNECTOR_ID_DP) &&
		(connector->connector_id != CONNECTOR_ID_DP1) &&
		(connector->connector_id != CONNECTOR_ID_DP2) &&
		(connector->connector_id != CONNECTOR_ID_DP3) &&
		(connector->connector_id != CONNECTOR_ID_EDP) &&
		(connector->connector_id != CONNECTOR_ID_EDP))
		connector->bind_connector->connector_base = connector->dpu_base +
			g_connector_offset[connector->bind_connector->connector_id];

	if (connector->conn_info->base.pipe_sw_itfch_idx == PIPE_SW_PRE_ITFCH1) {
		dpu_pr_info("add for itfch1");
		connector->dpp_base = connector->dpu_base + DPU_DPP1_OFFSET;
		connector->dsc_base = NULL;
	}
}