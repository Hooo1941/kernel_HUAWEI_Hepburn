/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#define RAW_CONNECTER_ID_MIN 0
#define RAW_CONNECTER_ID_MAX 6

static int32_t cdc_panel_on(struct dkmd_connector_info *pinfo)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->on)
		return entry_pops->on();

	return -1;
}

static int32_t cdc_panel_off(struct dkmd_connector_info *pinfo)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->off)
		return entry_pops->off();
	return -1;
}

static int32_t cdc_panel_set_backlight(struct panel_drv_private *priv,
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

static int32_t cdc_panel_set_fastboot(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");

	if (entry_pops->set_fastboot)
		return entry_pops->set_fastboot();
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

static int32_t cdc_notify_ser_vp_sync(struct panel_drv_private *priv,
		struct dkmd_connector_info *pinfo, const void *value)
{
	struct dpu_panel_ops *entry_pops = NULL;
	uint32_t idx;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_return(!entry_pops, -1, err, "panel ops is null\n");
	dpu_check_and_return(!value, -1, err, "value is null\n");
	idx = *((uint32_t *)value);

	if (entry_pops->notify_ser_dsi_vp_sync)
		return entry_pops->notify_ser_dsi_vp_sync(idx);

	dpu_pr_info("vp sync ops null\n");
	return 0;
}

static struct panel_ops_func_map g_panel_ops_func_table[PANEL_OPS_MAX] = {
	{SET_FASTBOOT, cdc_panel_set_fastboot},
	{SET_BACKLIGHT, cdc_panel_set_backlight},
	{LCD_SET_DISPLAY_REGION, lcd_set_display_region},
	{REGIST_POSTPROCESS, lcd_regist_postprocess},
	{DUMP_EXCEPTION_INFO, lcd_dump_exception_info},
	{NOTIFY_SER_VP_SYNC, cdc_notify_ser_vp_sync},
};

static struct panel_handle_adapter g_panel_handle  = {
	.on_func = cdc_panel_on,
	.off_func = cdc_panel_off,
	.panel_ops_func_table = g_panel_ops_func_table,
};

static int parse_connector_id(struct dkmd_connector_info *pinfo, uint8_t raw_connector_id)
{
	uint32_t connector_cnt = 0;

	dpu_pr_info("raw connector id %d\n", raw_connector_id);
	if ((raw_connector_id < RAW_CONNECTER_ID_MIN) || (raw_connector_id > RAW_CONNECTER_ID_MAX)) {
		dpu_pr_err("raw connector id %u is error\n", raw_connector_id);
		return -1;
	}

	pinfo->connector_idx[connector_cnt] = 0;
	pinfo->sw_post_chn_num = connector_cnt + 1;
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

	entry_pinfo = get_panel_info(panel_id);
	dpu_check_and_return(!entry_pinfo, -1, err, "entr pinfo is null\n");
	dpu_check_and_return(!check_entry_panel_info(entry_pinfo), -1, err, "check entry panel info failed\n");

	pinfo = &priv->connector_info;

	/* base info */
	pinfo->base.xres = entry_pinfo->xres;
	pinfo->base.yres = entry_pinfo->yres;
	pinfo->base.width = entry_pinfo->width;
	pinfo->base.height = entry_pinfo->height;
	pinfo->base.type = entry_pinfo->type;
	pinfo->base.fps = entry_pinfo->fps;
	pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH0;

	pinfo->bpp = entry_pinfo->bpp;
	pinfo->bgr_fmt = entry_pinfo->bgr_fmt;
	pinfo->orientation = entry_pinfo->orientation;
	pinfo->ifbc_type = entry_pinfo->ifbc_type;
	pinfo->vsync_ctrl_type = entry_pinfo->vsync_ctrl_type;
	pinfo->dirty_region_updt_support = entry_pinfo->user_info.dirty_region_updt_support;
	pinfo->base.id = panel_id;

	get_panel_product_config(pinfo, entry_pinfo);

	if (parse_connector_id(pinfo, (uint8_t)entry_pinfo->connector_id) != 0)
		return -1;
	connector = get_primary_connector(pinfo);
	if (!connector) {
		dpu_pr_err("connector id=%u is not available!\n", pinfo->connector_idx[PRIMARY_CONNECT_CHN_IDX]);
		return -1;
	}

	dpu_pr_info("panel id:%d, product type %d\n", panel_id, entry_pinfo->product_type);

	/* mipi */
	connector->mipi = entry_pinfo->mipi;
	/* esd */
	connector->esd_info = entry_pinfo->esd_info;

	pinfo->bl_info.bl_min = entry_pinfo->bl_info.bl_min;
	pinfo->bl_info.bl_max = entry_pinfo->bl_info.bl_max;
	pinfo->bl_info.bl_default = entry_pinfo->bl_info.bl_default;
	pinfo->bl_info.bl_type = entry_pinfo->bl_info.bl_type;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV0;

	return 0;
}

int32_t cdc_panel_probe_sub(struct panel_drv_private *priv)
{
	static uint32_t panel_id = PANEL_ID_PRIMARY;

	dpu_check_and_return(!priv, -1, err, "panel priv data is null\n");

	dpu_pr_info("panel id = %d\n", panel_id);
	if (panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_err("cdc panel id err\n");
		return -1;
	}

	if (!is_panel_registed(panel_id)) {
		dpu_pr_err("cdc panel is not registed\n");
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
