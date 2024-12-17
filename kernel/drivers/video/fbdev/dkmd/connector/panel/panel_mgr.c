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
#include <linux/slab.h>

#include "dkmd_log.h"
#include "dkmd_peri.h"

#include "panel_mgr.h"
#include "panel_drv.h"
#include "panel_dev.h"
#include "panel_lcdkit.h"
#include "panel_cdc.h"
#include "mipi_cdphy_utils.h"

#define SUPPORT_LCD_TYPE "support_lcd_type"
#define get_lcdkit_dev_id(panel_id) (0x1000 + (panel_id))

static struct dpu_panel_mgr panel_mgr[PANEL_ID_MAX_NUM];
static bool has_panel_registed[PANEL_ID_MAX_NUM];

bool is_panel_registed(uint32_t panel_id)
{
	if (panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_info("panel id is err %d\n", panel_id);
		return false;
	}

	return has_panel_registed[panel_id];
}

struct dpu_panel_info *get_panel_info(uint32_t panel_id)
{
	if (unlikely(panel_id >= PANEL_ID_MAX_NUM)) {
		dpu_pr_err("panel_id=%u is not of range", panel_id);
		return NULL;
	}

	dpu_check_and_return(!is_panel_registed(panel_id), NULL, err, "panel is not registered\n");
	return panel_mgr[panel_id].pinfo;
}

struct dpu_panel_ops *get_panel_ops(uint32_t panel_id)
{
	dpu_check_and_return(!is_panel_registed(panel_id), NULL, err, "panel is not registered\n");
	return panel_mgr[panel_id].pops;
}

int32_t register_panel(struct dpu_panel_ops *entry_pops, uint32_t panel_id)
{
	struct platform_device *pdev = NULL;
	struct dpu_panel_info *pinfo = NULL;

	dpu_check_and_return(!entry_pops, -1, err, "panel_ops is null\n");
	dpu_pr_info("+\n");

	if (is_panel_registed(panel_id)) {
		dpu_pr_err("lcdkit panel is already registed\n");
		return -1;
	}
	if (unlikely(panel_id >= PANEL_ID_MAX_NUM)) {
		dpu_pr_err("panel_id=%u is out of range", panel_id);
		return -1;
	}

	panel_mgr[panel_id].pops = entry_pops;
	pinfo = panel_mgr[panel_id].pops->get_panel_info();
	dpu_check_and_return(!pinfo, -1, err, "get panel info failed\n");
	mipi_dsi_convert_pxl2cycle(pinfo);
	panel_mgr[panel_id].pinfo = pinfo;
	has_panel_registed[panel_id] = true;

	// add dev
	pdev = platform_device_alloc("dsi_panel", (int32_t)get_lcdkit_dev_id(panel_id));
	if (pdev == NULL) {
		dpu_pr_err("failed to alloc dev\n");
		return -1;
	}

	if (platform_device_add(pdev)) {
		platform_device_put(pdev);
		dpu_pr_err("failed to add dev\n");
		return -1;
	}
	dpu_pr_info("-\n");
	return 0;
}

int32_t get_panel_refresh_count(uint32_t panel_id, struct panel_refresh_statistic_info *refresh_stat_info)
{
	struct dpu_panel_info *panel_info = NULL;
	dpu_check_and_return(!is_panel_registed(panel_id), -1, err, "panel is not registered\n");
	dpu_check_and_return(!refresh_stat_info, -1, err, "refresh_stat_info is null\n");

	panel_info = panel_mgr[panel_id].pinfo;
	dpu_check_and_return(!panel_info, -1, err, "panel_info is null\n");

	*refresh_stat_info = panel_info->stat_rslt.stat_info;
	atomic_set(&(panel_info->stat_rslt.clear_flag), 1);

	return 0;
}

static bool is_lcdkit_panel(uint32_t dev_id)
{
	if (dev_id >= get_lcdkit_dev_id(0) &&
		dev_id < get_lcdkit_dev_id(PANEL_ID_MAX_NUM))
		return true;

	return false;
}

int32_t prepare_panel_dev_data(struct panel_drv_private *priv)
{
	dpu_check_and_return(!priv, -1, err, "priv is null\n");

	dpu_pr_info("dev id = %d\n", priv->pdev->id);
#if defined(CONFIG_LCD_KIT_DRIVER)
	if (is_lcdkit_panel(priv->pdev->id)) {
		dpu_pr_info("in lcdkit probe\n");
		return lcdkit_panel_probe_sub(priv);
	}
#endif
#if defined(CONFIG_PRODUCT_CDC_ACE)
	dpu_pr_info("in cdc probe\n");
	return cdc_panel_probe_sub(priv);
#endif
	return panel_probe_sub(priv);
}

void register_panel_handle(struct panel_handle_adapter panel_ops, uint32_t panel_id)
{
	if (panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_info("panel id is err %d\n", panel_id);
		return;
	}

	panel_mgr[panel_id].ops_adapter = panel_ops;
}

static int32_t panel_dev_ops_handle(struct dkmd_connector_info *pinfo, uint32_t ops_cmd_id, void *value)
{
	int32_t i;
	struct panel_ops_func_map *ops_handle = NULL;
	struct panel_drv_private *priv = to_panel_private(pinfo);

	dpu_check_and_return(!priv, -1, err, "priv is null\n");
	dpu_check_and_return(!panel_mgr[pinfo->base.id].ops_adapter.panel_ops_func_table, -1, err,
		"panel ops table is null\n");

	dpu_pr_debug("ops_cmd_id = %d!\n", ops_cmd_id);

	for (i = 0; i < (int32_t)PANEL_OPS_MAX; i++) {
		ops_handle = &(panel_mgr[pinfo->base.id].ops_adapter.panel_ops_func_table[i]);
		if ((ops_cmd_id == ops_handle->ops_cmd_id) && ops_handle->handle_func)
			return ops_handle->handle_func(priv, pinfo, value);
	}

	return 0;
}

int setup_panel_dev_data(struct panel_drv_private *priv)
{
	struct dkmd_conn_handle_data *pdata = NULL;
	uint32_t panel_id = 0;

	dpu_check_and_return(!priv, -1, err, "priv is null\n");

	panel_id = priv->connector_info.base.id;
	dpu_pr_info("cur panel id = %d\n", panel_id);
	if (panel_id >= PANEL_ID_MAX_NUM) {
		dpu_pr_info("panel id is err %d\n", panel_id);
		return -1;
	}

	pdata = devm_kzalloc(&priv->pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dpu_pr_err("alloc panel device data failed!\n");
		return -1;
	}
	pdata->conn_info = &priv->connector_info;

	pdata->on_func = panel_mgr[panel_id].ops_adapter.on_func;
	pdata->off_func = panel_mgr[panel_id].ops_adapter.off_func;
	pdata->ops_handle_func = panel_dev_ops_handle;

	/* add panel handle data to platform device */
	if (platform_device_add_data(priv->pdev, pdata, sizeof(*pdata)) != 0) {
		dpu_pr_err("add dsi device data failed!\n");
		return -1;
	}

	return 0;
}

void get_panel_product_config(struct dkmd_connector_info *pinfo, struct dpu_panel_info *entry_pinfo)
{
	pinfo->base.enable_lbuf_reserve = entry_pinfo->enable_lbuf_reserve;
	pinfo->base.scene_info = entry_pinfo->scene_info;
	pinfo->base.opr_policy = entry_pinfo->opr_policy;
	pinfo->base.compose_policy = entry_pinfo->compose_policy;
}
