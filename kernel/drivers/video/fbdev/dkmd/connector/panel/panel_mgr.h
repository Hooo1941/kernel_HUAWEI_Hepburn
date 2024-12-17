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

#ifndef PANEL_MGR_H
#define PANEL_MGR_H

#include <linux/atomic.h>
#include "dkmd_lcd_interface.h"
#include "dkmd_peri.h"
#include "dkmd_connector.h"

struct panel_drv_private;

#define DTS_COMP_FAKE_PANEL "dkmd,mipi_fake_panel"
#define DTS_COMP_PANEL_NT35521 "dkmd,panel_nt35521"
#define DTS_COMP_PANEL_TD4322 "dkmd,panel_td4322"
#define DTS_COMP_PANEL_NT37700P "dkmd,panel_nt37700p"
#define DTS_COMP_PANEL_NT37800A "dkmd,panel_nt37800a"
#define DTS_COMP_PANEL_NT37290 "dkmd,panel_nt37290"
#define DTS_COMP_PANEL_NT37701_BRQ "dkmd,panel_nt37701_brq"
#define DTS_COMP_PANEL_NT37701A "dkmd,panel_nt37701a"
#define DTS_COMP_PANEL_RM69091 "dkmd,panel_rm69091"
#define DTS_COMP_PANEL_HX5293 "dkmd,panel_hx5293"
#define DTS_COMP_PANEL_NT36870 "dkmd,panel_nt36870"
#define DTS_COMP_PANEL_VISIONOX310 "dkmd,panel_visionox310"
#define DTS_COMP_PANEL_BOE7P847 "dkmd,panel_boe_7p847"
#define DTS_COMP_PANEL_VISIONOX_6P39 "dkmd,panel_visionox_6p39"
#define DTS_COMP_PANEL_BOE_6P39 "dkmd,panel_boe_6p39"
#define DTS_COMP_PANEL_HX83121 "dkmd,panel_hx83121"
#define DTS_COMP_PANEL_LCDKIT "huawei,lcd_panel_type"
#define DTS_COMP_PANEL_MEDIABOX_DSI0 "dkmd,panel_mediabox_dsi0"
#define DTS_COMP_PANEL_MEDIABOX_EDP0 "dkmd,panel_mediabox_edp0"
#define DTS_COMP_PANEL_MEDIABOX_EDP1 "dkmd,panel_mediabox_edp1"

/* Record the screen ID information, natural growth */
enum {
	FAKE_PANEL_ID = 0,
	PANEL_NT35521_ID,
	PANEL_TD4322_ID,
	PANEL_NT37700P_ID,
	PANEL_NT37800A_ID,
	PANEL_NT37290_ID,
	PANEL_NT37701_BRQ_ID,
	PANEL_NT37701A_ID,
	PANEL_RM69091_ID,
	PANEL_HX5293_ID,
	PANEL_NT36870_ID,
	PANEL_VISIONOX_6P39_ID,
	PANEL_BOE_6P39_ID,
	PANEL_VISIONOX310_ID,
	PANEL_BOE_7P847_ID,
	PANEL_MEDIABOX_DSI0_ID,
	PANEL_MEDIABOX_EDP0_ID,
	PANEL_MEDIABOX_EDP1_ID,
	PANEL_HX83121_ID,
	PANEL_MAX_ID,
};

struct panel_ops_func_map {
	uint32_t ops_cmd_id;
	int32_t (*handle_func)(struct panel_drv_private *priv,
		struct dkmd_connector_info *pinfo, const void *value);
};

struct panel_handle_adapter {
	int32_t (*on_func)(struct dkmd_connector_info *pinfo);
	int32_t (*off_func)(struct dkmd_connector_info *pinfo);
	struct panel_ops_func_map *panel_ops_func_table;
};

struct dpu_panel_mgr {
	struct dpu_panel_info *pinfo;
	struct dpu_panel_ops *pops;
	struct panel_handle_adapter ops_adapter;
};

struct dpu_panel_info *get_panel_info(uint32_t panel_id);
struct dpu_panel_ops *get_panel_ops(uint32_t panel_id);
bool is_panel_registed(uint32_t panel_id);
int32_t init_panel_mgr(void);
void deinit_panel_mgr(void);
int32_t register_panel(struct dpu_panel_ops *entry_pops, uint32_t panel_id);
int32_t get_panel_refresh_count(uint32_t panel_id, struct panel_refresh_statistic_info *refresh_stat_info);
void register_panel_handle(struct panel_handle_adapter panel_ops, uint32_t panel_id);
int32_t prepare_panel_dev_data(struct panel_drv_private *priv);
int setup_panel_dev_data(struct panel_drv_private *priv);
void get_panel_product_config(struct dkmd_connector_info *pinfo, struct dpu_panel_info *entry_pinfo);

#endif
