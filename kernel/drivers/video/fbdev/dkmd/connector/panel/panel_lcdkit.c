/*
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
#include <securec.h>
#include "dkmd_log.h"
#include "dkmd_connector.h"
#include "panel_mgr.h"
#include "dkmd_peri.h"
#include "panel_drv.h"
#include "dpu_conn_mgr.h"
#include "dkmd_bl_factory.h"
#include "dsc_output_calc.h"
#include "panel_utils.h"
#include "dsc/dsc_algorithm_manager.h"
#include "dkmd_dsc_info.h"
#include "dkmd_object.h"

#define RAW_CONNECTER_ID_MIN 1
#define RAW_CONNECTER_ID_MAX 6

static int32_t lcdkit_panel_on(struct dkmd_connector_info *pinfo)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->on)
		return entry_pops->on();

	return -1;
}

static int32_t lcdkit_panel_off(struct dkmd_connector_info *pinfo)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->off)
		return entry_pops->off();
	return -1;
}

static int32_t lcdkit_panel_set_backlight(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	uint32_t bl_level = 0;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	bl_level = *((uint32_t *)value);

	if (entry_pops->set_backlight)
		return entry_pops->set_backlight(bl_level);
	return -1;
}

static int32_t lcdkit_panel_set_ppc_config_id(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	uint32_t ppc_config_id = 0;

	if (!is_ppc_support(&pinfo->base))
		return 0;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	ppc_config_id = *((uint32_t *)value);
	if (entry_pops->set_ppc_config_id)
		return entry_pops->set_ppc_config_id(ppc_config_id);
	return -1;
}

static int32_t lcdkit_panel_set_fastboot(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->set_fastboot)
		return entry_pops->set_fastboot();
	return -1;
}

static int32_t lcdkit_panel_fake_power_off(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->fake_power_off)
		return entry_pops->fake_power_off();
	return -1;
}

static int32_t lcd_check_status(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->check_lcd_status)
		return entry_pops->check_lcd_status();

	return -1;
}

static int32_t lcd_set_display_region(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	struct dkmd_rect *dirty = (struct dkmd_rect *)value;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->set_display_region)
		return entry_pops->set_display_region(dirty);
	return -1;
}

static int32_t lcd_regist_postprocess(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	struct device *dev = (struct device *)value;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");
	dpu_pr_info("enter\n");

	if (entry_pops->disp_postprocess)
		return entry_pops->disp_postprocess(dev);

	return -1;
}

static int32_t lcd_dump_exception_info(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");
	dpu_pr_info("enter\n");

	if (entry_pops->dump_exception_info)
		return entry_pops->dump_exception_info();

	return -1;
}

static int32_t lcd_set_refresh_statistic(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_return(!panel_info, -1, err, "panel info is null\n");

	panel_info->stat_rslt.stat_info = *(struct panel_refresh_statistic_info*)(value);

	return 0;
}

static int32_t lcd_get_stat_clear_flag(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	int32_t clear_flag;
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_return(!panel_info, -1, err, "panel info is null\n");

	clear_flag = atomic_read(&(panel_info->stat_rslt.clear_flag));
	atomic_set(&(panel_info->stat_rslt.clear_flag), 0);

	return clear_flag;
}

static int32_t lcdkit_trace_screen_bl(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	uint32_t bl_level = 0;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	bl_level = *((uint32_t *)value);

	if (entry_pops->trace_screen_bl) {
		dpu_pr_debug("trace screen bl, bl_level = %u\n", bl_level);
		return entry_pops->trace_screen_bl(bl_level);
	}
	return -1;
}

static int32_t lcdkit_send_cmds_at_vsync(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->send_cmds_at_vsync) {
		dpu_pr_debug("notify lcdkit to send bl/el cmds");
		return entry_pops->send_cmds_at_vsync();
	}
	return -1;
}

static int32_t lcd_update_safe_frame_rate(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	struct panel_update_safe_frm_rate_info *safe_frm_rate_info = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	safe_frm_rate_info = (struct panel_update_safe_frm_rate_info *)value;

	if (entry_pops->set_safe_frm_rate) {
		dpu_pr_debug("update safe frame rate to lcdkit");
		return entry_pops->set_safe_frm_rate(safe_frm_rate_info);
	}
	return -1;
}

static struct panel_ops_func_map g_panel_ops_func_table[PANEL_OPS_MAX] = {
	{SET_FASTBOOT, lcdkit_panel_set_fastboot},
	{SET_BACKLIGHT, lcdkit_panel_set_backlight},
	{CHECK_LCD_STATUS, lcd_check_status},
	{LCD_SET_DISPLAY_REGION, lcd_set_display_region},
	{REGIST_POSTPROCESS, lcd_regist_postprocess},
	{DUMP_EXCEPTION_INFO, lcd_dump_exception_info},
	{SET_REFRESH_STATISTIC, lcd_set_refresh_statistic},
	{UPDATE_SAFE_FRM_RATE, lcd_update_safe_frame_rate},
	{GET_STATISTIC_CLEAR_FLAG, lcd_get_stat_clear_flag},
	{FAKE_POWER_OFF, lcdkit_panel_fake_power_off},
	{TRACE_SCREEN_BL, lcdkit_trace_screen_bl},
	{SET_PPC_CONFIG_ID, lcdkit_panel_set_ppc_config_id},
	{SEND_CMDS_AT_VSYNC, lcdkit_send_cmds_at_vsync},
};

static struct panel_handle_adapter g_panel_handle  = {
	.on_func = lcdkit_panel_on,
	.off_func = lcdkit_panel_off,
	.panel_ops_func_table = g_panel_ops_func_table,
};

static void set_dsc_param(struct input_dsc_info *info, struct dsc_calc_info *dsc)
{
	struct dsc_algorithm_manager *dsc_algo = get_dsc_algorithm_manager_instance();

	if (!dsc_algo) {
		dpu_pr_err("pt is null!\n");
		return;
	}

	dsc->dsc_en = 1;
	dsc_algo->vesa_dsc_info_calc(info, &(dsc->dsc_info), NULL);
}

static int parse_connector_id(struct dkmd_connector_info *pinfo, uint8_t raw_connector_id)
{
	uint32_t i;
	uint32_t connector_cnt = 0;

	dpu_pr_info("raw connector id %d\n", raw_connector_id);
	if ((raw_connector_id < RAW_CONNECTER_ID_MIN) || (raw_connector_id > RAW_CONNECTER_ID_MAX)) {
		dpu_pr_err("raw connector id %u is error\n", raw_connector_id);
		return -1;
	}

	for (i = 0; i < MIPI_DSI_INDEX_MAX; i++) {
		if (BIT(i) & raw_connector_id) {
			if ((pinfo->base.type & PANEL_EXTERNAL) && i == CONNECTOR_ID_DSI0)
				pinfo->connector_idx[connector_cnt] = CONNECTOR_ID_DSI0_BUILTIN;
			else
				pinfo->connector_idx[connector_cnt] = i;
			pinfo->sw_post_chn_idx[connector_cnt] = i;
			connector_cnt++;
		}
	}

	pinfo->sw_post_chn_num = connector_cnt;
	if (pinfo->sw_post_chn_num > EXTERNAL_CONNECT_CHN_IDX) {
		get_primary_connector(pinfo)->bind_connector = get_external_connector(pinfo);
		dpu_pr_info("cphy 1+1 or dual-mipi mode or dual-connector!\n");
	}

	dpu_pr_info("connector idx:%u %u\n", pinfo->connector_idx[0], pinfo->connector_idx[1]);
	dpu_pr_info("sw post chn idx:%u %u num:%u\n",
		pinfo->sw_post_chn_idx[0], pinfo->sw_post_chn_idx[1], pinfo->sw_post_chn_num);
	return 0;
}

static bool check_entry_panel_info(struct dpu_panel_info *entry_pinfo)
{
	dpu_check_and_return(entry_pinfo->ifbc_type >= IFBC_TYPE_MAX, false,
		err, "invalid ifbc type:%u\n", entry_pinfo->ifbc_type);
	dpu_check_and_return(entry_pinfo->lcd_te_idx > 1, false,
		err, "invalid lcd te index:%u\n", entry_pinfo->lcd_te_idx);

	return true;
}

static int32_t prepare_panel_data(struct panel_drv_private *priv, uint32_t panel_id)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct dpu_panel_info *entry_pinfo = NULL;
	struct dpu_connector *connector = NULL;
	struct input_dsc_info input_dsc_info = {0};
	uint32_t sw_dvfs_frm_rate = 0;
	int32_t i = 0;

	entry_pinfo = get_panel_info(panel_id);
	dpu_check_and_return(!entry_pinfo, -1, err, "entr pinfo is null\n");
	dpu_check_and_return(!check_entry_panel_info(entry_pinfo), -1, err, "check entry panel info failed\n");

	pinfo = &priv->connector_info;

	/* base info */
	pinfo->base.lcd_name = entry_pinfo->lcd_name;
	pinfo->base.xres = entry_pinfo->xres;
	pinfo->base.yres = entry_pinfo->yres;
	pinfo->base.width = entry_pinfo->width;
	pinfo->base.height = entry_pinfo->height;
	pinfo->base.type = entry_pinfo->type;
	pinfo->base.fps = entry_pinfo->fps;
	pinfo->base.fps_of_longv_for_dvfs = entry_pinfo->fps_of_longv_for_dvfs;
	pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH0;
	pinfo->base.lcd_te_idx = entry_pinfo->lcd_te_idx;
	pinfo->bpp = entry_pinfo->bpp;
	pinfo->bgr_fmt = entry_pinfo->bgr_fmt;
	pinfo->orientation = entry_pinfo->orientation;
	pinfo->ifbc_type = entry_pinfo->ifbc_type;
	pinfo->aod_enable = entry_pinfo->aod_enable;
	pinfo->esd_enable = entry_pinfo->esd_enable;
	pinfo->vsync_ctrl_type = entry_pinfo->vsync_ctrl_type;
	pinfo->dirty_region_updt_support = 0;
	pinfo->base.fold_type = entry_pinfo->product_type;
	pinfo->base.hs_pkt_discontin_support = (uint8_t)entry_pinfo->hs_pkt_discontin_support;
	pinfo->base.panel_partial_ctrl_support = (uint8_t)entry_pinfo->panel_partial_ctrl_support;
	pinfo->base.id = panel_id;
	pinfo->base.fake_panel_flag = entry_pinfo->fake_panel_flag;
	pinfo->split_swap_enable = entry_pinfo->panel_split_swap_enable;

	for (i = 0; i < (int32_t)entry_pinfo->dfr_info.oled_info.fps_sup_num; ++i)
		sw_dvfs_frm_rate = max(sw_dvfs_frm_rate, entry_pinfo->dfr_info.oled_info.fps_sup_seq[i]);

	pinfo->base.sw_dvfs_frm_rate = min(sw_dvfs_frm_rate, SW_DVFS_FRM_RATE_LIMIT);

	get_panel_product_config(pinfo, entry_pinfo);

	priv->dfr_info = entry_pinfo->dfr_info;

	if (parse_connector_id(pinfo, entry_pinfo->connector_id) != 0)
		return -1;
	connector = get_primary_connector(pinfo);
	if (!connector) {
		dpu_pr_err("connector id=%u is not available\n", pinfo->connector_idx[PRIMARY_CONNECT_CHN_IDX]);
		return -1;
	}
	if (is_builtin_panel(&pinfo->base) && pinfo->base.fold_type == PANEL_FLIP) {
		dpu_pr_warn("buitin panel use itfch1\n");
		pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH1;
	}

	dpu_pr_info("panel id:%u, product type %d, pipe_sw_itfch_idx %d, ppc %u\n",
		panel_id, entry_pinfo->product_type, pinfo->base.pipe_sw_itfch_idx, pinfo->base.panel_partial_ctrl_support);

	/* dsc */
	input_dsc_info = entry_pinfo->input_dsc_info;
	if (is_ifbc_vesa_panel(pinfo->ifbc_type))
		set_dsc_param(&input_dsc_info, &connector->dsc);

	/* spr */
	if (is_spr_enabled(&entry_pinfo->spr)) {
		connector->spr = entry_pinfo->spr;
		connector->dsc.spr_en = 1;
	}

	/* mipi */
	connector->mipi = entry_pinfo->mipi;
	/* esd */
	connector->esd_info = entry_pinfo->esd_info;

	pinfo->bl_info.bl_min = entry_pinfo->bl_info.bl_min;
	pinfo->bl_info.bl_max = entry_pinfo->bl_info.bl_max;
	pinfo->bl_info.bl_default = entry_pinfo->bl_info.bl_default;
	pinfo->bl_info.bl_type = entry_pinfo->bl_info.bl_type;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV0;

	priv->user_pinfo = entry_pinfo->user_info;
	pinfo->dirty_region_updt_support = entry_pinfo->user_info.dirty_region_updt_support;
	pinfo->update_core_clk_support = entry_pinfo->update_core_clk_support;

	return 0;
}

int32_t lcdkit_panel_probe_sub(struct panel_drv_private *priv)
{
	static uint32_t panel_id = PANEL_ID_PRIMARY;

	dpu_check_and_return(!priv, -1, err, "panel priv data is null\n");

	dpu_pr_info("panel id = %d\n", panel_id);
	if (panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_err("lcdkit panel id err\n");
		return -1;
	}

	if (!is_panel_registed(panel_id)) {
		dpu_pr_err("lcdkit panel is not registed\n");
		return -EPROBE_DEFER;
	}

	if (prepare_panel_data(priv, panel_id) != 0) {
		dpu_pr_err("prepare data failed\n");
		return -1;
	}
	register_panel_handle(g_panel_handle, panel_id);
	panel_id++;

	return 0;
}
