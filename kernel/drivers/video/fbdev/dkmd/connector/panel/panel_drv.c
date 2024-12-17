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
#include "panel_dev.h"
#include "panel_mgr.h"
#include "dkmd_dfr_info.h"
#ifdef CONFIG_HIBERNATION
#include <linux/pm.h>
#endif

#define PANEL_PRIMARY_NAME "gfx_primary"
#define PANEL_BUILTIN_NAME "gfx_builtin"

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param_for_fake_panel(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
	mipi->hsa = 20;
	mipi->hbp = 40;
	mipi->dpi_hsize = 812;
	mipi->hline_time = 1026;
	mipi->vsa = 4;
	mipi->vbp = 12;
	mipi->vfp = 14;

	mipi->pxl_clk_rate = 144 * 1000000UL;
	mipi->dsi_bit_clk = 480;

	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;

	mipi->pxl_clk_rate_div = 1;
	mipi->dsi_bit_clk_upt_support = 0;

	mipi->clk_post_adjust = 215;
	mipi->lane_nums = DSI_4_LANES;
	mipi->color_mode = DSI_24BITS_1;

	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_NON_BURST_SYNC_PULSES;
	mipi->non_continue_en = 0;
	mipi->phy_mode = DPHY_MODE;
}

int32_t panel_base_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;
	struct device_node *np = priv->pdev->dev.of_node;

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 1080;
	pinfo->base.yres = 1920;

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 65;
	pinfo->base.height = 115;

	// TODO: caculate fps by mipi timing para
	pinfo->base.fps = 60;

	/* 2. config connector info
	* would be used for dsi & composer setup
	*/
	if(is_fake_panel(&pinfo->base))
		mipi_lcd_init_dsi_param_for_fake_panel(pinfo, &get_primary_connector(pinfo)->mipi);

	ret = of_property_read_u32(np, FPGA_FLAG_NAME, &pinfo->base.fpga_flag);
	if (ret) {
		dpu_pr_info("get fpga_flag failed!\n");
		pinfo->base.fpga_flag = 0;
	}
	dpu_pr_info("fpga_flag=%#x", pinfo->base.fpga_flag);

	ret = of_property_read_u32(np, LCD_DFR_MODE_NAME, &priv->dfr_info.dfr_mode);
	if (ret) {
		dpu_pr_info("get lcd_dfr_mode failed!\n");
		priv->dfr_info.dfr_mode = DFR_MODE_CONSTANT;
	}
	dpu_pr_info("dfr_mode=%#x", priv->dfr_info.dfr_mode);

	ret = of_property_read_u32(np, LCD_DISP_TYPE_NAME, &pinfo->base.type);
	if (ret) {
		dpu_pr_info("get lcd_display_type failed!\n");
		pinfo->base.type = PANEL_MIPI_VIDEO;
	}
	dpu_pr_info("panel_type=%#x", pinfo->base.type);

	ret = of_property_read_u32(np, PIPE_SW_ITFSW_IDX_NAME, &pinfo->base.pipe_sw_itfch_idx);
	if (ret) {
		dpu_pr_info("get pipe_sw_itfch_idx failed!\n");
		pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH0;
	}
	dpu_pr_info("pipe_sw_pre_itfch_idx=%#x", pinfo->base.pipe_sw_itfch_idx);

	ret = of_property_read_u32(np, FAKE_PANEL_FLAG_NAME, &pinfo->base.fake_panel_flag);
	if (ret)
		pinfo->base.fake_panel_flag = 0;
	dpu_pr_info("fake_panel_flag=%#x", pinfo->base.fake_panel_flag);

	base_panel_connector_dts_parse(pinfo, np);

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;

	/* not be used by now */
	pinfo->orientation = LCD_PORTRAIT;

	ret = of_property_read_u32(np, LCD_IFBC_TYPE_NAME, &pinfo->ifbc_type);
	if (ret) {
		dpu_pr_info("get ifbc_type failed!\n");
		pinfo->ifbc_type = IFBC_TYPE_NONE;
	}
	dpu_pr_info("ifbc_type=%#x", pinfo->ifbc_type);
	pinfo->vsync_ctrl_type = 0; /* default disable */

	pinfo->bl_info.bl_min = 1;
	pinfo->bl_info.bl_max = 255;
	pinfo->bl_info.bl_default = 102;

	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, &pinfo->bl_info.bl_type);
	if (ret) {
		dpu_pr_info("get lcd_bl_type failed!\n");
		pinfo->bl_info.bl_type = BL_SET_BY_MIPI;
	}
	dpu_pr_info("bl_type=%#x", pinfo->bl_info.bl_type);

	ret = of_property_read_string_index(np, LCD_BLPWM_DEV_NAME, 0, &priv->bl_dev_name);
	if (ret)
		priv->bl_dev_name = NULL;
	dpu_pr_info("get bl dev name %s", priv->bl_dev_name);

	if (!is_fake_panel(&pinfo->base))
		panel_drv_data_setup(priv, np);

	return 0;
}

void panel_base_of_device_release(struct panel_drv_private *priv)
{
	dpu_pr_info("base release enter!\n");
	(void)priv;
}

panel_device_match_data(fake_panel_info, FAKE_PANEL_ID, panel_base_of_device_setup, panel_base_of_device_release);

static const struct of_device_id panel_device_match_table[] = {
	{
		.compatible = DTS_COMP_FAKE_PANEL,
		.data = &fake_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT35521,
		.data = &nt35521_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_TD4322,
		.data = &td4322_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT37700P,
		.data = &nt37700p_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT37800A,
		.data = &nt37800a_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT37290,
		.data = &nt37290_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT37701_BRQ,
		.data = &nt37701_brq_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT37701A,
		.data = &nt37701a_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_RM69091,
		.data = &rm69091_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_HX5293,
		.data = &hx5293_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_NT36870,
		.data = &nt36870_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_VISIONOX_6P39,
		.data = &visionox_6p39_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_BOE_6P39,
		.data = &boe_6p39_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_VISIONOX310,
		.data = &visionox310_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_BOE7P847,
		.data = &boe7p847_panel_info,
	},
	{
		.compatible = DTS_COMP_PANEL_HX83121,
		.data = &hx83121_panel_info,
	},
	{},
};
MODULE_DEVICE_TABLE(of, panel_device_match_table);

static void get_panel_user_info(struct dkmd_connector_info *pinfo, struct user_panel_info *user_pinfo)
{
	struct panel_drv_private *priv = to_panel_private(pinfo);

	*user_pinfo = priv->user_pinfo;
}

static void get_panel_ppc_config_id_rect_info(struct dkmd_connector_info *pinfo,
	struct dpu_ppc_config_id_rect_info *ppc_config_id_rect_info)
{
	uint32_t i = 0;
	struct dpu_panel_ops *entry_pops = NULL;
	struct dpu_panel_info *panel_info = NULL;

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_no_retval(!entry_pops, err, "entry pops is null\n");

	panel_info = entry_pops->get_panel_info();
	dpu_check_and_no_retval(!panel_info, err, "panel info is null\n");

	for (i = 0; i < PPC_CONFIG_ID_CNT; ++i) {
		ppc_config_id_rect_info[i] = panel_info->ppc_config_id_rect_info[i];
		dpu_pr_info("lcdkit panel fold state info(id, l, t, r, b): %u, %d, %d, %d, %d",
			panel_info->ppc_config_id_rect_info[i].id,
			panel_info->ppc_config_id_rect_info[i].rect.left, panel_info->ppc_config_id_rect_info[i].rect.top,
			panel_info->ppc_config_id_rect_info[i].rect.right, panel_info->ppc_config_id_rect_info[i].rect.bottom);
	}
}

static int32_t get_active_display_rect(struct dkmd_connector_info *pinfo, struct dkmd_rect *active_rect)
{
	struct dpu_ppc_config_id_rect_info ppc_rect_info[PPC_CONFIG_ID_CNT] = {0};
	struct dkmd_rect_coord *ppc_active_rect = NULL;

	dpu_check_and_return(!active_rect, -1, err, "active_rect is null");
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");

	/* not 3 fold panel */
	if (!pinfo->get_panel_ppc_config_id_rect_info) {
		active_rect->x = 0;
		active_rect->y = 0;
		active_rect->w = pinfo->base.xres;
		active_rect->h = pinfo->base.yres;
		return 0;
	}

	/* 3 fold panel */
	if (pinfo->ppc_config_id_active >= PPC_CONFIG_ID_CNT) {
		dpu_pr_err("ppc_config_id match error\n");
		return -1;
	}

	pinfo->get_panel_ppc_config_id_rect_info(pinfo, ppc_rect_info);
	ppc_active_rect = &ppc_rect_info[pinfo->ppc_config_id_active].rect;
	active_rect->x = (int32_t)ppc_active_rect->left;
	active_rect->y = (int32_t)ppc_active_rect->top;
	active_rect->w = ppc_active_rect->right - ppc_active_rect->left;
	active_rect->h = ppc_active_rect->bottom - ppc_active_rect->top;
	dpu_pr_debug("ppc_active_rect[l,t,r,b]=[%u, %u, %u, %u], id=%u\n", ppc_active_rect->left,
		ppc_active_rect->top, ppc_active_rect->right, ppc_active_rect->bottom,	pinfo->ppc_config_id_active);

	return 0;
}

static void get_dfr_info(struct dkmd_connector_info *pinfo, struct dfr_info **out)
{
	struct panel_drv_private *priv = to_panel_private(pinfo);

	*out = &priv->dfr_info;
}

static void get_ddic_cmds(struct dkmd_connector_info *pinfo,
	const struct disp_effect_params *effect_params, struct dsi_cmds *ddic_cmds)
{
	struct dpu_panel_ops *entry_pops = NULL;

	dpu_pr_debug("enter\n");

	entry_pops = get_panel_ops(pinfo->base.id);
	dpu_check_and_no_retval(!entry_pops, err, "panel ops is null\n");
	entry_pops->get_ddic_cmds(effect_params, ddic_cmds);

	dpu_pr_debug("exit\n");
}

static int32_t panel_probe(struct platform_device *pdev)
{
	struct panel_drv_private *priv = NULL;
	int32_t ret;

	if (unlikely(!pdev)) {
		dpu_pr_warn("pdev is null");
		return -1;
	}

	dpu_pr_info("enter\n");
	if (!is_connector_manager_available())
		return -EPROBE_DEFER;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dpu_pr_err("alloc panel driver private data failed!\n");
		return -EINVAL;
	}

	/* this is the tail device */
	priv->pdev = pdev;
	priv->connector_info.base.peri_device = pdev;
	priv->connector_info.get_dfr_info = get_dfr_info;
	priv->connector_info.get_ddic_cmds = get_ddic_cmds;
	platform_set_drvdata(pdev, priv);

	ret = prepare_panel_dev_data(priv);
	if (ret != 0) {
		dpu_pr_err("prepare_panel_dev_data failed\n");
		return ret;
	}

	ret = setup_panel_dev_data(priv);
	if (ret != 0) {
		dpu_pr_err("setup_panel_dev_data failed\n");
		return ret;
	}

	if (priv->connector_info.base.id == PANEL_ID_PRIMARY)
		priv->connector_info.base.name = PANEL_PRIMARY_NAME;
	else
		priv->connector_info.base.name = PANEL_BUILTIN_NAME;

	/* fake panel do not need TE, need force present */
	if (priv->connector_info.base.fake_panel_flag != 0)
		dpu_pr_warn("fake panel!\n");

	if (register_connector(&priv->connector_info) != 0) {
		dpu_pr_warn("device register failed, need retry!\n");
		return -EPROBE_DEFER;
	}

	priv->connector_info.get_panel_user_info = get_panel_user_info;
	priv->connector_info.get_active_display_rect = get_active_display_rect;

	if (is_ppc_support(&priv->connector_info.base)) {
		priv->connector_info.get_panel_ppc_config_id_rect_info = get_panel_ppc_config_id_rect_info;
		priv->connector_info.ppc_config_id_record = PPC_CONFIG_ID_G_MODE;
		priv->connector_info.ppc_config_id_active = PPC_CONFIG_ID_G_MODE;
	}

	dpu_pr_info("exit\n");
	return 0;
}

/**
 * @brief clear resource when device removed but not for devicetree device
 *
 * @param pdev
 * @return int32_t
 */
static int32_t panel_remove(struct platform_device *pdev)
{
	const struct panel_match_data *data = NULL;
	struct panel_drv_private *priv = NULL;

	priv = (struct panel_drv_private *)platform_get_drvdata(pdev);
	if (!priv) {
		dpu_pr_err("get dsi private data failed!\n");
		return -EINVAL;
	}
	unregister_connector(&priv->connector_info);

	data = of_device_get_match_data(&pdev->dev);
	if (data != NULL)
		data->of_device_release(priv);

	dpu_pr_info("%s panel remove complete!\n", priv->connector_info.base.name);
	return 0;
}

static void panel_shutdown(struct platform_device *pdev)
{
	struct panel_drv_private *priv = NULL;

	priv = (struct panel_drv_private *)platform_get_drvdata(pdev);
	if (!priv) {
		dpu_pr_err("get dsi private data failed!\n");
		return;
	}
	dpu_pr_info("enter +!\n");

	connector_device_shutdown(&priv->connector_info);
	dpu_pr_info("exit -!\n");
}

static int panel_pm_suspend(struct device *dev)
{
	struct panel_drv_private *priv = NULL;

	dpu_pr_info("enter +!\n");
	priv = (struct panel_drv_private *)dev_get_drvdata(dev);
	if (!priv) {
		dpu_pr_err("get dsi private data failed!\n");
		return 0;
	}

	connector_device_suspend(&priv->connector_info);
	dpu_pr_info("exit -!\n");
	return 0;
}

static int panel_pm_resume(struct device *dev)
{
	struct panel_drv_private *priv = NULL;

	dpu_pr_info("enter +!\n");
	priv = (struct panel_drv_private *)dev_get_drvdata(dev);
	if (!priv) {
		dpu_pr_err("get dsi private data failed!\n");
		return 0;
	}

	connector_device_resume(&priv->connector_info);
	dpu_pr_info("exit -!\n");
	return 0;
}

#ifdef CONFIG_HIBERNATION
static int panel_pm_freeze(struct device *dev)
{
	if (dpm_get_message_event() == PM_EVENT_QUIESCE)
		return 0;

	return panel_pm_suspend(dev);
}

static int panel_pm_restore(struct device *dev)
{
	return panel_pm_resume(dev);
}
#endif

static const struct dev_pm_ops panel_dev_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend = panel_pm_suspend,
	.resume = panel_pm_resume,
#endif
#ifdef CONFIG_HIBERNATION
	.freeze_noirq = panel_pm_freeze,
	.restore_noirq = panel_pm_restore,
#endif
};

static struct platform_driver panel_platform_driver = {
	.probe  = panel_probe,
	.remove = panel_remove,
	.shutdown = panel_shutdown,
	.driver = {
		.name  = "dsi_panel",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(panel_device_match_table),
		.pm = &panel_dev_pm_ops,
	}
};

static int32_t __init panel_register(void)
{
	return platform_driver_register(&panel_platform_driver);
}

static void __exit panel_unregister(void)
{
	platform_driver_unregister(&panel_platform_driver);
}

module_init(panel_register);
module_exit(panel_unregister);

MODULE_AUTHOR("Graphics Display");
MODULE_DESCRIPTION("Panel Module Driver");
MODULE_LICENSE("GPL");
