/*
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

#include "dkmd_dpu.h"
#include "dpu_conn_mgr.h"
#include "panel_dev.h"
#include "dkmd_blpwm.h"
#include "dkmd_bl_factory.h"
#include "panel_mgr.h"
#include "panel_utils.h"

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

/* LCD init step */
enum LCD_INIT_STEP {
	LCD_INIT_POWER_ON = 0,
	LCD_INIT_MIPI_LP_SEND_SEQUENCE,
	LCD_INIT_MIPI_HS_SEND_SEQUENCE,
};

/* LCD uninit step */
enum LCD_UNINIT_STEP {
	LCD_UNINIT_POWER_OFF = 0,
	LCD_UNINIT_MIPI_LP_SEND_SEQUENCE,
	LCD_UNINIT_MIPI_HS_SEND_SEQUENCE,
};

static struct regulator *vcc_lcdio;
static struct regulator *vcc_lcdanalog;

static struct vcc_desc lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 3100000, 3100000, WAIT_TYPE_MS, 0},
	/* io set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDIO_NAME, &vcc_lcdio, 1800000, 1800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};

/*******************************************************************************
** LCD IOMUX
*/
static struct pinctrl_data panel_pctrl;

static struct pinctrl_cmd_desc lcd_pinctrl_init_cmds[] = {
	{DTYPE_PINCTRL_GET, &panel_pctrl, 0},
	{DTYPE_PINCTRL_STATE_GET, &panel_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
	{DTYPE_PINCTRL_STATE_GET, &panel_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

static struct pinctrl_cmd_desc lcd_pinctrl_finit_cmds[] = {
	{DTYPE_PINCTRL_PUT, &panel_pctrl, 0},
};

static struct pinctrl_cmd_desc lcd_pinctrl_normal_cmds[] = {
	{DTYPE_PINCTRL_SET, &panel_pctrl, DTYPE_PINCTRL_STATE_DEFAULT},
};

static struct pinctrl_cmd_desc lcd_pinctrl_lowpower_cmds[] = {
	{DTYPE_PINCTRL_SET, &panel_pctrl, DTYPE_PINCTRL_STATE_IDLE},
};

/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_P5V5_ENABLE_NAME	"gpio_lcd_vsp_enable"
#define GPIO_LCD_N5V5_ENABLE_NAME "gpio_lcd_vsn_enable"
#define GPIO_LCD_RESET_NAME	"gpio_lcd_reset"
#define GPIO_LCD_BL_ENABLE_NAME	"gpio_lcd_bl_enable"
#define GPIO_LCD_TP1V8_NAME	"gpio_lcd_tp1v8"

static uint32_t gpio_lcd_vsp_enable;
static uint32_t gpio_lcd_vsn_enable;
static uint32_t gpio_lcd_reset;
static uint32_t gpio_lcd_bl_enable;
static uint32_t gpio_lcd_tp1v8;

static struct gpio_desc lcd_gpio_request_cmds[] = {
	/* AVDD_5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	/* AVEE_-5.5V */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	/* backlight enable */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
};

static struct gpio_desc lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 15,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 10,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 50,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 1},
};

static struct gpio_desc lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 3,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},

	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

static struct gpio_desc lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_BL_ENABLE_NAME, &gpio_lcd_bl_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_N5V5_ENABLE_NAME, &gpio_lcd_vsp_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_P5V5_ENABLE_NAME, &gpio_lcd_vsn_enable, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_US, 0,
		GPIO_LCD_RESET_NAME, &gpio_lcd_reset, 0},
};

struct panel_table {
	int32_t index;
	const char *panel_desc;
};

static struct panel_table s_lcd_tabel[] = {
	{ FAKE_PANEL_ID, DTS_COMP_FAKE_PANEL, },
	{ PANEL_NT35521_ID, DTS_COMP_PANEL_NT35521 },
	{ PANEL_TD4322_ID, DTS_COMP_PANEL_TD4322, },
	{ PANEL_NT37700P_ID, DTS_COMP_PANEL_NT37700P, },
	{ PANEL_NT37800A_ID, DTS_COMP_PANEL_NT37800A, },
	{ PANEL_NT37290_ID, DTS_COMP_PANEL_NT37290, },
	{ PANEL_NT37701_BRQ_ID, DTS_COMP_PANEL_NT37701_BRQ, },
	{ PANEL_NT37701A_ID, DTS_COMP_PANEL_NT37701A, },
	{ PANEL_RM69091_ID, DTS_COMP_PANEL_RM69091, },
	{ PANEL_HX5293_ID, DTS_COMP_PANEL_HX5293, },
	{ PANEL_NT36870_ID, DTS_COMP_PANEL_NT36870, },
	{ PANEL_VISIONOX_6P39_ID, DTS_COMP_PANEL_VISIONOX_6P39, },
	{ PANEL_BOE_6P39_ID, DTS_COMP_PANEL_BOE_6P39, },
	{ PANEL_VISIONOX310_ID, DTS_COMP_PANEL_VISIONOX310, },
	{ PANEL_BOE_7P847_ID, DTS_COMP_PANEL_BOE7P847, },
	{ PANEL_MEDIABOX_DSI0_ID, DTS_COMP_PANEL_MEDIABOX_DSI0, },
	{ PANEL_MEDIABOX_EDP0_ID, DTS_COMP_PANEL_MEDIABOX_EDP0, },
	{ PANEL_MEDIABOX_EDP1_ID, DTS_COMP_PANEL_MEDIABOX_EDP1, },
	{ PANEL_HX83121_ID, DTS_COMP_PANEL_HX83121, },
};

void panel_drv_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	if (!priv || !np) {
		dpu_pr_err("priv or np is nullptr!\n");
		return;
	}
	gpio_lcd_vsp_enable = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	gpio_lcd_vsn_enable = (uint32_t)of_get_named_gpio(np, "gpios", 1);
	gpio_lcd_reset = (uint32_t)of_get_named_gpio(np, "gpios", 2);
	gpio_lcd_bl_enable = (uint32_t)of_get_named_gpio(np, "gpios", 3);
	gpio_lcd_tp1v8 = (uint32_t)of_get_named_gpio(np, "gpios", 4);

	dpu_pr_info("used gpio:[vsp: %d, vsn: %d, rst: %d, bl_en: %d, tp1v8: %d]\n",
		gpio_lcd_vsp_enable, gpio_lcd_vsn_enable, gpio_lcd_reset, gpio_lcd_bl_enable, gpio_lcd_tp1v8);

	priv->lcd_init_step = LCD_INIT_POWER_ON;
	priv->lcd_uninit_step = LCD_UNINIT_MIPI_HS_SEND_SEQUENCE;
	priv->gpio_request_cmds = lcd_gpio_request_cmds;
	priv->gpio_request_cmds_len = ARRAY_SIZE(lcd_gpio_request_cmds);
	priv->gpio_free_cmds = lcd_gpio_free_cmds;
	priv->gpio_free_cmds_len = ARRAY_SIZE(lcd_gpio_free_cmds);

	priv->gpio_normal_cmds = lcd_gpio_normal_cmds;
	priv->gpio_normal_cmds_len = ARRAY_SIZE(lcd_gpio_normal_cmds);
	priv->gpio_lowpower_cmds = lcd_gpio_lowpower_cmds;
	priv->gpio_lowpower_cmds_len = ARRAY_SIZE(lcd_gpio_lowpower_cmds);

	if (priv->connector_info.base.fpga_flag == 0) {
		priv->vcc_enable_cmds = lcd_vcc_enable_cmds;
		priv->vcc_enable_cmds_len = ARRAY_SIZE(lcd_vcc_enable_cmds);
		priv->vcc_disable_cmds = lcd_vcc_disable_cmds;
		priv->vcc_disable_cmds_len = ARRAY_SIZE(lcd_vcc_disable_cmds);

		priv->pinctrl_normal_cmds = lcd_pinctrl_normal_cmds;
		priv->pinctrl_normal_cmds_len = ARRAY_SIZE(lcd_pinctrl_normal_cmds);
		priv->pinctrl_lowpower_cmds = lcd_pinctrl_lowpower_cmds;
		priv->pinctrl_lowpower_cmds_len = ARRAY_SIZE(lcd_pinctrl_lowpower_cmds);

		priv->pinctrl_init_cmds = lcd_pinctrl_init_cmds;
		priv->pinctrl_init_cmds_len = ARRAY_SIZE(lcd_pinctrl_init_cmds);
		priv->pinctrl_finit_cmds = lcd_pinctrl_finit_cmds;
		priv->pinctrl_finit_cmds_len = ARRAY_SIZE(lcd_pinctrl_finit_cmds);

		priv->vcc_init_cmds = lcd_vcc_init_cmds;
		priv->vcc_init_cmds_len = (uint32_t)ARRAY_SIZE(lcd_vcc_init_cmds);
		priv->vcc_finit_cmds = lcd_vcc_finit_cmds;
		priv->vcc_finit_cmds_len = (uint32_t)ARRAY_SIZE(lcd_vcc_finit_cmds);
	}
}

/* ----------------------- function pointor setup ------------------------ */

static int32_t panel_dev_lcd_power_on(struct dkmd_connector_info *pinfo)
{
	int32_t ret;
	struct panel_drv_private *priv = to_panel_private(pinfo);

	if (!priv) {
		dpu_pr_err("get panel drv private err!\n");
		return -EINVAL;
	}

	bl_factory_on(priv, pinfo->bl_info.bl_type);

	dpu_pr_info("enter!\n");
	if (priv->vcc_enable_cmds && (priv->vcc_enable_cmds_len > 0)) {
		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_enable_cmds, (int32_t)priv->vcc_enable_cmds_len);
		if (ret)
			dpu_pr_warn("vcc enable cmds handle fail!\n");
	}

	if (priv->pinctrl_normal_cmds && (priv->pinctrl_normal_cmds_len > 0)) {
		ret = peri_pinctrl_cmds_tx(priv->pdev, priv->pinctrl_normal_cmds, (int32_t)priv->pinctrl_normal_cmds_len);
		if (ret)
			dpu_pr_warn("pinctrl normal cmds handle fail!\n");
	}

	if (priv->gpio_normal_cmds && (priv->gpio_normal_cmds_len > 0)) {
		ret = peri_gpio_cmds_tx(priv->gpio_normal_cmds, (int32_t)priv->gpio_normal_cmds_len);
		if (ret)
			dpu_pr_warn("gpio normal cmds handle fail!\n");
	}
	dpu_pr_info("exit!\n");

	return 0;
}

static int32_t panel_dev_lcd_power_off(struct dkmd_connector_info *pinfo)
{
	int32_t ret = 0;
	struct panel_drv_private *priv = to_panel_private(pinfo);

	if (!priv) {
		dpu_pr_warn("get panel drv private err!\n");
		return -EINVAL;
	}

	bl_factory_off(priv, pinfo->bl_info.bl_type);

	dpu_pr_info("enter!\n");
	if (priv->vcc_disable_cmds && (priv->vcc_disable_cmds_len > 0)) {
		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_disable_cmds, (int32_t)priv->vcc_disable_cmds_len);
		if (ret)
			dpu_pr_warn("vcc disable cmds handle fail!\n");
	}

	if (priv->pinctrl_lowpower_cmds && (priv->pinctrl_lowpower_cmds_len > 0)) {
		ret = peri_pinctrl_cmds_tx(priv->pdev, priv->pinctrl_lowpower_cmds, (int32_t)priv->pinctrl_lowpower_cmds_len);
		if (ret)
			dpu_pr_warn("pinctrl lowpower handle fail!\n");
	}

	if (priv->gpio_lowpower_cmds && (priv->gpio_lowpower_cmds_len > 0)) {
		ret = peri_gpio_cmds_tx(priv->gpio_lowpower_cmds, (int32_t)priv->gpio_lowpower_cmds_len);
		if (ret)
			dpu_pr_warn("gpio lowpower handle fail!\n");
	}
	dpu_pr_info("exit!\n");

	return 0;
}

static int32_t lcd_initialize_code(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	int32_t ret = 0;
	struct dpu_connector *connector = NULL;

	dpu_pr_info("enter!\n");
	(void)value;
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL!");

	if (priv->disp_on_cmds && (priv->disp_on_cmds_len > 0)) {
		if (connector->bind_connector) {
			ret = mipi_dual_dsi_cmds_tx(priv->disp_on_cmds, (int32_t)priv->disp_on_cmds_len,
				connector->connector_base, priv->disp_on_cmds, (int32_t)priv->disp_on_cmds_len,
				connector->bind_connector->connector_base, EN_DSI_TX_NORMAL_MODE, true);
			dpu_pr_info("use dual mipi\n");
		} else {
			ret = mipi_dsi_cmds_tx(priv->disp_on_cmds, (int32_t)priv->disp_on_cmds_len, connector->connector_base);
		}

		dpu_pr_info("disp on cmds handle %d!\n", ret);
	}

	return ret;
}

/* lcd_check_status need be visited externelly, so get dsi_base internally */
static int32_t lcd_check_status(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	return lcd_check_power_status(get_primary_connector(pinfo));
}

static int32_t lcd_uninitialize_code(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	int32_t ret = 0;
	struct dpu_connector *connector = NULL;

	dpu_pr_info("enter!\n");
	(void)value;
	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL!");

	if (priv->disp_off_cmds && (priv->disp_off_cmds_len > 0)) {
		if (connector->bind_connector)
			ret = mipi_dual_dsi_cmds_tx(priv->disp_off_cmds, (int32_t)priv->disp_off_cmds_len,
				connector->connector_base, priv->disp_off_cmds, (int32_t)priv->disp_off_cmds_len,
				connector->bind_connector->connector_base, EN_DSI_TX_NORMAL_MODE, true);
		else
			ret = mipi_dsi_cmds_tx(priv->disp_off_cmds, (int32_t)priv->disp_off_cmds_len, connector->connector_base);
		dpu_pr_info("display off cmds handle ret=%d!\n", ret);
	}

	return ret;
}

static int32_t panel_dev_on(struct dkmd_connector_info *pinfo)
{
	struct panel_drv_private *priv = to_panel_private(pinfo);

	if (!priv) {
		dpu_pr_err("get panel drv private err!\n");
		return -EINVAL;
	}

	dpu_pr_info("+\n");
	if (priv->lcd_init_step == LCD_INIT_POWER_ON) {
		panel_dev_lcd_power_on(pinfo);
		priv->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
	} else if (priv->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		lcd_initialize_code(priv, pinfo, NULL);
		priv->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
	} else if (priv->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {
		priv->lcd_init_step = LCD_INIT_POWER_ON;
	} else {
		dpu_pr_err("failed to power on!\n");
		return -EINVAL;
	}
	dpu_pr_info("-\n");

	return 0;
}

static int32_t panel_dev_off(struct dkmd_connector_info *pinfo)
{
	struct panel_drv_private *priv = to_panel_private(pinfo);

	if (!priv) {
		dpu_pr_err("get panel drv private err!\n");
		return -EINVAL;
	}

	dpu_pr_info("+\n");
	if (priv->lcd_uninit_step == LCD_UNINIT_MIPI_HS_SEND_SEQUENCE) {
		priv->lcd_uninit_step = LCD_UNINIT_MIPI_LP_SEND_SEQUENCE;
	} else if (priv->lcd_uninit_step == LCD_UNINIT_MIPI_LP_SEND_SEQUENCE) {
		lcd_uninitialize_code(priv, pinfo, NULL);
		priv->lcd_uninit_step = LCD_UNINIT_POWER_OFF;
	} else if (priv->lcd_uninit_step == LCD_UNINIT_POWER_OFF) {
		panel_dev_lcd_power_off(pinfo);
		priv->lcd_uninit_step = LCD_UNINIT_MIPI_HS_SEND_SEQUENCE;
	} else {
		dpu_pr_err("failed to power off!\n");
		return -EINVAL;
	}
	dpu_pr_info("-\n");

	return 0;
}

static int32_t lcd_set_fastboot(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	int ret;

	if (!priv) {
		dpu_pr_err("get panel drv private err!\n");
		return -EINVAL;
	}

	bl_factory_on(priv, pinfo->bl_info.bl_type);

	dpu_pr_info("enter!\n");

	if (priv->pinctrl_normal_cmds && (priv->pinctrl_normal_cmds_len > 0)) {
		ret = peri_pinctrl_cmds_tx(priv->pdev, priv->pinctrl_normal_cmds, (int32_t)priv->pinctrl_normal_cmds_len);
		if (ret)
			dpu_pr_warn("pinctrl normal cmds handle fail!\n");
	}

	if (priv->gpio_request_cmds && (priv->gpio_request_cmds_len > 0)) {
		ret = peri_gpio_cmds_tx(priv->gpio_request_cmds, (int32_t)priv->gpio_request_cmds_len);
		if (ret)
			dpu_pr_info("gpio cmds request err: %d!\n", ret);
	}

	dpu_pr_info("exit!\n");
	return 0;
}

static int32_t lcd_set_display_region(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	static char lcd_disp_x[] = {
		0x2A,
		0x00, 0x00,0x04,0x37
	};
	static char lcd_disp_y[] = {
		0x2B,
		0x00, 0x00,0x07,0x7F
	};
	static struct dsi_cmd_desc set_display_address[] = {
		{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
			sizeof(lcd_disp_x), lcd_disp_x},
		{DTYPE_DCS_LWRITE, 0, 5, WAIT_TYPE_US,
			sizeof(lcd_disp_y), lcd_disp_y},
	};
	struct dkmd_rect *dirty = (struct dkmd_rect *)value;

	if (!dirty) {
		dpu_pr_err("dirty rect is null!");
		return -EINVAL;
	}

	dpu_pr_info("dirty_rect[%d %d %d %d]", dirty->x, dirty->y, dirty->w, dirty->h);

	lcd_disp_x[1] = ((uint32_t)(dirty->x) >> 8) & 0xff;
	lcd_disp_x[2] = ((uint32_t)(dirty->x)) & 0xff;
	lcd_disp_x[3] = ((uint32_t)((uint32_t)dirty->x + dirty->w - 1) >> 8) & 0xff;
	lcd_disp_x[4] = ((uint32_t)((uint32_t)dirty->x + dirty->w - 1)) & 0xff;
	lcd_disp_y[1] = ((uint32_t)(dirty->y) >> 8) & 0xff;
	lcd_disp_y[2] = ((uint32_t)(dirty->y)) & 0xff;
	lcd_disp_y[3] = ((uint32_t)((uint32_t)dirty->y + dirty->h - 1) >> 8) & 0xff;
	lcd_disp_y[4] = ((uint32_t)((uint32_t)dirty->y + dirty->h - 1)) & 0xff;

	mipi_dsi_cmds_tx(set_display_address,
		ARRAY_SIZE(set_display_address), get_primary_connector(pinfo)->connector_base);

	return 0;
}

static int32_t lcd_set_backlight(struct panel_drv_private *priv,
	struct dkmd_connector_info *pinfo, const void *value)
{
	uint32_t bl_level = *((uint32_t *)value);

	bl_factory_set_backlight(priv, pinfo->bl_info.bl_type, bl_level);

	return 0;
}

static struct panel_ops_func_map g_panel_ops_func_table[PANEL_OPS_MAX] = {
	{SET_FASTBOOT, lcd_set_fastboot},
	{CHECK_LCD_STATUS, lcd_check_status},
	{SET_BACKLIGHT, lcd_set_backlight},
	{LCD_SET_DISPLAY_REGION, lcd_set_display_region},
};

static struct panel_handle_adapter g_panel_handle = {
	.on_func = panel_dev_on,
	.off_func = panel_dev_off,
	.panel_ops_func_table = g_panel_ops_func_table,
};

static int32_t prepare_panel_data(struct panel_drv_private *priv)
{
	const struct panel_match_data *data = NULL;
	struct dkmd_connector_info *pinfo = NULL;

	data = of_device_get_match_data(&priv->pdev->dev);
	if (!data) {
		dpu_pr_err("get panel match data failed!\n");
		return -1;
	}

	if (data->lcd_id < PANEL_MAX_ID)
		dpu_pr_info("panel:%s !\n", s_lcd_tabel[data->lcd_id].panel_desc);

	if (data->of_device_setup && (data->of_device_setup(priv) != 0)) {
		dpu_pr_err("device setup failed!\n");
		return -1;
	}
	pinfo = &priv->connector_info;
	if (is_builtin_panel(&pinfo->base) && pinfo->base.fold_type == PANEL_FLIP) {
		dpu_pr_info("builtin panel use itfch1\n");
		pinfo->base.pipe_sw_itfch_idx = PIPE_SW_PRE_ITFCH1;
	}

	return 0;
}

int32_t panel_probe_sub(struct panel_drv_private *priv)
{
	static uint32_t g_panel_id = 0;

	dpu_check_and_return(!priv, -EINVAL, err, "priv is NULL!");
	dpu_check_and_return(prepare_panel_data(priv) != 0, -1, err, "panel_probe failed");
	register_panel_handle(g_panel_handle, g_panel_id);
	g_panel_id++;

	return 0;
}

MODULE_LICENSE("GPL");
