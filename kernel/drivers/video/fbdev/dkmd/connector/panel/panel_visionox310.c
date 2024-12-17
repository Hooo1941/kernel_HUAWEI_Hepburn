/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "dpu_conn_mgr.h"
#include "panel_mgr.h"
#include "panel_drv.h"
#include "dsc/dsc_output_calc.h"
#include "dsc/dsc_algorithm_manager.h"

#define DSC_1_1_EN

static char g_on_cmd0[] = {
	0xFE, 0xB6,
};

static char g_on_cmd1[] = {
	0x01, 0xC1,
};

static char g_on_cmd2[] = {
	0x02, 0x10,
};

static char g_on_cmd3[] = {
	0x03, 0x10,
};

static char g_on_cmd4[] = {
	0x04, 0x10,
};

static char g_on_cmd5[] = {
	0x05, 0x10,
};

static char g_on_cmd6[] = {
	0x06, 0x10,
};

static char g_on_cmd7[] = {
	0x07, 0x10,
};

static char g_on_cmd8[] = {
	0xFE, 0x16,
};

static char g_on_cmd9[] = {
	0x8A, 0x0C,
};

static char g_on_cmd10[] = {
	0xFE, 0x40,
};

static char g_on_cmd11[] = {
	0xBD, 0x00,
};

static char g_on_cmd12[] = {
	0xFE, 0xD2,
};

static char g_on_cmd13[] = {
	0x35, 0x0D,
};

static char g_on_cmd14[] = {
	0xFE, 0x00,
};

#ifdef DSC_1_1_EN
static char g_on_cmd15[] = {
	0xFA, 0x01,
};
#else
/* dsc enable: 0x01  dsc disable: 0x07 */
static char g_on_cmd15[] = {
	0xFA, 0x07,
};
#endif

static char g_on_cmd16[] = {
	0xC2, 0x08,
};

static char g_on_cmd17[] = {
	0x35, 0x00,
};

static char g_on_cmd18[] = {
	0x11,
};

static char g_on_cmd19[] = {
	0x29,
};

static char g_on_cmd20[] = {
	0x51, 0x0F, 0x00,
};
/* Power ON Sequence end */

#ifdef BIST_MODE
static char g_bist_cmd0[] = {
	0xFE, 0x00,
};

static char g_bist_cmd1[] = {
	0xFA, 0x07,
};

static char g_bist_cmd2[] = {
	0XFE, 0xD0,
};

static char g_bist_cmd3[] = {
	0x37, 0x90,
};

static char g_bist_cmd4[] = {
	0x43, 0x1F,
};

static char g_bist_cmd5[] = {
	0x44, 0x00,
};

static char g_bist_cmd6[] = {
	0x3A, 0xC8,
};
#endif

static struct dsi_cmd_desc lcd_display_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd0), g_on_cmd0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd1), g_on_cmd1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd2), g_on_cmd2},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd3), g_on_cmd3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd4), g_on_cmd4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd5), g_on_cmd5},

	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd6), g_on_cmd6},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd7), g_on_cmd7},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd8), g_on_cmd8},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd9), g_on_cmd9},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd10), g_on_cmd10},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd11), g_on_cmd11},
	{DTYPE_DCS_LWRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd12), g_on_cmd12},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd13), g_on_cmd13},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd14), g_on_cmd14},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd15), g_on_cmd15},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd16), g_on_cmd16},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd17), g_on_cmd17},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS, sizeof(g_on_cmd18), g_on_cmd18},
	{DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd19), g_on_cmd19},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_on_cmd20), g_on_cmd20},

#ifdef BIST_MODE
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd0), g_bist_cmd0},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd1), g_bist_cmd1},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd2), g_bist_cmd2},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd3), g_bist_cmd3},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd4), g_bist_cmd4},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd5), g_bist_cmd5},
	{DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_bist_cmd6), g_bist_cmd6},
#endif
};
/* Power OFF Sequence */
static char g_off_cmd0[] = {
	0xFE, 0x00,
};

static char g_off_cmd1[] = {
	0x28,
};

static char g_off_cmd2[] = {
	0x10,
};
static struct dsi_cmd_desc lcd_display_off_cmds[] = {
	{ DTYPE_DCS_WRITE1, 0, 10, WAIT_TYPE_US, sizeof(g_off_cmd0), g_off_cmd0 },
	{ DTYPE_DCS_WRITE, 0, 10, WAIT_TYPE_US, sizeof(g_off_cmd1), g_off_cmd1 },
	{ DTYPE_DCS_WRITE, 0, 100, WAIT_TYPE_MS, sizeof(g_off_cmd2), g_off_cmd2 },
};

/*******************************************************************************
 ** LCD GPIO
 */
#define GPIO_AMOLED_RESET_NAME  "gpio_amoled_reset"
#define GPIO_AMOLED_VCC1V2_NAME "gpio_amoled_vcc1v2"
#define GPIO_AMOLED_VCC1V8_NAME "gpio_amoled_vcc1v8"
#define GPIO_AMOLED_VCC3V1_NAME "gpio_amoled_vcc3v1"
#define GPIO_AMOLED_SW_NAME     "gpio_amoled_sw"
#define GPIO_AMOLED_TE0_NAME    "gpio_amoled_te0"

static uint32_t g_gpio_amoled_te0;
static uint32_t g_gpio_amoled_vcc1v8;
static uint32_t g_gpio_amoled_vcc1v2;
static uint32_t g_gpio_amoled_reset;
static uint32_t g_gpio_amoled_sw;
static uint32_t g_gpio_amoled_vcc3v1;

static struct gpio_desc g_asic_lcd_gpio_request_cmds[] = {
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_SW_NAME, &g_gpio_amoled_sw, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
};

static struct gpio_desc g_asic_lcd_gpio_normal_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_SW_NAME, &g_gpio_amoled_sw, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 20, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 35, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 1},
};

static struct gpio_desc g_asic_lcd_gpio_lowpower_cmds[] = {
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

static struct gpio_desc g_asic_lcd_gpio_free_cmds[] = {
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_RESET_NAME, &g_gpio_amoled_reset, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC3V1_NAME, &g_gpio_amoled_vcc3v1, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 5, GPIO_AMOLED_VCC1V2_NAME, &g_gpio_amoled_vcc1v2, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 10, GPIO_AMOLED_VCC1V8_NAME, &g_gpio_amoled_vcc1v8, 0},
};

/*******************************************************************************
 */
static void panel_drv_private_data_setup(struct panel_drv_private *priv, struct device_node *np)
{
	g_gpio_amoled_vcc3v1 = (uint32_t)of_get_named_gpio(np, "gpios", 0);
	g_gpio_amoled_vcc1v8 = (uint32_t)of_get_named_gpio(np, "gpios", 1);
	g_gpio_amoled_vcc1v2 = (uint32_t)of_get_named_gpio(np, "gpios", 2);
	g_gpio_amoled_reset = (uint32_t)of_get_named_gpio(np, "gpios", 3);
	g_gpio_amoled_te0 = (uint32_t)of_get_named_gpio(np, "gpios", 4);
	g_gpio_amoled_sw = 253;

	dpu_pr_info("used gpio:[rst: %d, vcc3v1: %d, vcc1v8: %d,vcc1v2: %d,te0: %d]\n",
		g_gpio_amoled_reset, g_gpio_amoled_vcc3v1, g_gpio_amoled_vcc1v8, g_gpio_amoled_vcc1v2, g_gpio_amoled_te0);

	priv->gpio_request_cmds = g_asic_lcd_gpio_request_cmds;
	priv->gpio_request_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_request_cmds);
	priv->gpio_free_cmds = g_asic_lcd_gpio_free_cmds;
	priv->gpio_free_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_free_cmds);

	priv->gpio_normal_cmds = g_asic_lcd_gpio_normal_cmds;
	priv->gpio_normal_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_normal_cmds);
	priv->gpio_lowpower_cmds = g_asic_lcd_gpio_lowpower_cmds;
	priv->gpio_lowpower_cmds_len = ARRAY_SIZE(g_asic_lcd_gpio_lowpower_cmds);
}

#if defined(DSC_1_1_EN)
static void dsc_param_set(struct dkmd_connector_info *pinfo, struct dsc_calc_info *dsc)
{
	pinfo->ifbc_type = IFBC_TYPE_VESA3_75X_DUAL;
	dsc->dsc_en = 1;
	dsc->dual_dsc_en = 1;

	dsc->dsc_info.dsc_version_major = 1;
	dsc->dsc_info.dsc_version_minor = 1;
	dsc->dsc_info.pic_height = 2224;
	dsc->dsc_info.pic_width = 2496;
	dsc->dsc_info.slice_height = 16;
	dsc->dsc_info.slice_width = 1248;
	dsc->dsc_info.chunk_size = 0x04e0;
	dsc->dsc_info.initial_dec_delay = 0x0370;
	dsc->dsc_info.initial_scale_value = 0x20;
	dsc->dsc_info.scale_increment_interval = 0x021d;
	dsc->dsc_info.scale_decrement_interval = 0x011;
	dsc->dsc_info.nfl_bpg_offset = 0x0667;
	dsc->dsc_info.slice_bpg_offset = 0x02c0;
	dsc->dsc_info.final_offset = 0x10f0;

	// dsc parameter info
	dsc->dsc_info.dsc_bpc = 10;
	dsc->dsc_info.dsc_bpp = 8;
	dsc->dsc_info.initial_xmit_delay = 0x200;
	dsc->dsc_info.first_line_bpg_offset = 0xc;

	// DSC_CTRL
	dsc->dsc_info.block_pred_enable = 1;
	dsc->dsc_info.linebuf_depth = 11;
	// RC_PARAM3
	dsc->dsc_info.initial_offset = 0x1800;
	// FLATNESS_QP_TH
	dsc->dsc_info.flatness_min_qp = 7;
	dsc->dsc_info.flatness_max_qp = 16;
	// DSC_PARAM4
	dsc->dsc_info.rc_edge_factor = 6;
	dsc->dsc_info.rc_model_size = 8192;
	// DSC_RC_PARAM5: 0x330b0b
	dsc->dsc_info.rc_tgt_offset_lo = 3;
	dsc->dsc_info.rc_tgt_offset_hi = 3;
	dsc->dsc_info.rc_quant_incr_limit1 = 15;
	dsc->dsc_info.rc_quant_incr_limit0 = 15;
	// DSC_RC_BUF_THRESH0: 0xe1c2a38
	dsc->dsc_info.rc_buf_thresh[0] = 14;
	dsc->dsc_info.rc_buf_thresh[1] = 28;
	dsc->dsc_info.rc_buf_thresh[2] = 42;
	dsc->dsc_info.rc_buf_thresh[3] = 56;
	// DSC_RC_BUF_THRESH1: 0x46546269
	dsc->dsc_info.rc_buf_thresh[4] = 70;
	dsc->dsc_info.rc_buf_thresh[5] = 84;
	dsc->dsc_info.rc_buf_thresh[6] = 98;
	dsc->dsc_info.rc_buf_thresh[7] = 105;
	// DSC_RC_BUF_THRESH2: 0x7077797b
	dsc->dsc_info.rc_buf_thresh[8] = 112;
	dsc->dsc_info.rc_buf_thresh[9] = 119;
	dsc->dsc_info.rc_buf_thresh[10] = 121;
	dsc->dsc_info.rc_buf_thresh[11] = 123;
	// DSC_RC_BUF_THRESH3: 0x7d7e0000
	dsc->dsc_info.rc_buf_thresh[12] = 125;
	dsc->dsc_info.rc_buf_thresh[13] = 126;
	// DSC_RC_RANGE_PARAM0: 0x1020100
	dsc->dsc_info.rc_range[0].min_qp = 0;
	dsc->dsc_info.rc_range[0].max_qp = 8;
	dsc->dsc_info.rc_range[0].offset = 2;
	dsc->dsc_info.rc_range[1].min_qp = 4;
	dsc->dsc_info.rc_range[1].max_qp = 8;
	dsc->dsc_info.rc_range[1].offset = 0;
	// DSC_RC_RANGE_PARAM1: 0x94009be
	dsc->dsc_info.rc_range[2].min_qp = 5;
	dsc->dsc_info.rc_range[2].max_qp = 9;
	dsc->dsc_info.rc_range[2].offset = 0;
	dsc->dsc_info.rc_range[3].min_qp = 5;
	dsc->dsc_info.rc_range[3].max_qp = 10;
	dsc->dsc_info.rc_range[3].offset = 62;
	// DSC_RC_RANGE_PARAM2, 0x19fc19fa
	dsc->dsc_info.rc_range[4].min_qp = 7;
	dsc->dsc_info.rc_range[4].max_qp = 11;
	dsc->dsc_info.rc_range[4].offset = 60;
	dsc->dsc_info.rc_range[5].min_qp = 7;
	dsc->dsc_info.rc_range[5].max_qp = 11;
	dsc->dsc_info.rc_range[5].offset = 58;
	// DSC_RC_RANGE_PARAM3, 0x19f81a38
	dsc->dsc_info.rc_range[6].min_qp = 7;
	dsc->dsc_info.rc_range[6].max_qp = 11;
	dsc->dsc_info.rc_range[6].offset = 56;
	dsc->dsc_info.rc_range[7].min_qp = 7;
	dsc->dsc_info.rc_range[7].max_qp = 12;
	dsc->dsc_info.rc_range[7].offset = 56;
	// DSC_RC_RANGE_PARAM4, 0x1a781ab6
	dsc->dsc_info.rc_range[8].min_qp = 7;
	dsc->dsc_info.rc_range[8].max_qp = 13;
	dsc->dsc_info.rc_range[8].offset = 56;
	dsc->dsc_info.rc_range[9].min_qp = 8;
	dsc->dsc_info.rc_range[9].max_qp = 14;
	dsc->dsc_info.rc_range[9].offset = 54;
	// DSC_RC_RANGE_PARAM5, 0x2ab62af4
	dsc->dsc_info.rc_range[10].min_qp = 9;
	dsc->dsc_info.rc_range[10].max_qp = 14;
	dsc->dsc_info.rc_range[10].offset = 54;
	dsc->dsc_info.rc_range[11].min_qp = 9;
	dsc->dsc_info.rc_range[11].max_qp = 15;
	dsc->dsc_info.rc_range[11].offset = 54;
	// DSC_RC_RANGE_PARAM6, 0x2af44b34
	dsc->dsc_info.rc_range[12].min_qp = 9;
	dsc->dsc_info.rc_range[12].max_qp = 15;
	dsc->dsc_info.rc_range[12].offset = 52;
	dsc->dsc_info.rc_range[13].min_qp = 12;
	dsc->dsc_info.rc_range[13].max_qp = 16;
	dsc->dsc_info.rc_range[13].offset = 52;
	// DSC_RC_RANGE_PARAM7, 0x63740000
	dsc->dsc_info.rc_range[14].min_qp = 16;
	dsc->dsc_info.rc_range[14].max_qp = 17;
	dsc->dsc_info.rc_range[14].offset = 52;
}
#endif

/* dsi param initialized value from panel spec */
static void mipi_lcd_init_dsi_param(struct dkmd_connector_info *pinfo, struct mipi_panel_info *mipi)
{
	mipi->hsa = 15;
	mipi->hbp = 15;
	mipi->dpi_hsize = 185;
	mipi->hline_time = 227;
	mipi->vsa = 4;
	mipi->vbp = 12;
	mipi->vfp = 16;

	mipi->pxl_clk_rate = 192 * 1000000UL;
	mipi->dsi_bit_clk = 600;

	mipi->dsi_bit_clk_upt = mipi->dsi_bit_clk;
	mipi->dsi_bit_clk_default = mipi->dsi_bit_clk;

	mipi->pxl_clk_rate_div = 1;
	mipi->dsi_bit_clk_upt_support = 0;

	mipi->clk_post_adjust = 215;
	mipi->lane_nums = DSI_2_LANES;
	mipi->color_mode = DSI_24BITS_1;
	mipi->dsi_version = DSI_1_1_VERSION;
	mipi->phy_mode = CPHY_MODE;
	mipi->vc = 0;
	mipi->max_tx_esc_clk = 10 * 1000000;
	mipi->burst_mode = DSI_BURST_SYNC_PULSES_1;
	mipi->non_continue_en = 1;
}

static int32_t panel_of_device_setup(struct panel_drv_private *priv)
{
	int32_t ret;
	struct dkmd_connector_info *pinfo = &priv->connector_info;
	struct device_node *np = priv->pdev->dev.of_node;

	dpu_pr_info("enter!\n");

	/* Inheritance based processing */
	panel_base_of_device_setup(priv);
	panel_drv_private_data_setup(priv, np);

	/* 1. config base object info
	 * would be used for framebuffer setup
	 */
	pinfo->base.xres = 2496;
	pinfo->base.yres = 2224;

	/* When calculating DPI needs the following parameters */
	pinfo->base.width = 158;
	pinfo->base.height = 141;

	// TODO: caculate fps by mipi timing para
	pinfo->base.fps = 60;

	/* 2. config connector info
	 * would be used for dsi & composer setup
	 */
	mipi_lcd_init_dsi_param(pinfo, &get_primary_connector(pinfo)->mipi);
#ifdef DSC_1_1_EN
	dsc_param_set(pinfo, &get_primary_connector(pinfo)->dsc);
#endif

	pinfo->dirty_region_updt_support = 0;
	pinfo->vsync_ctrl_type = 0;

	/* 3. config panel private info
	 * would be used for panel setup
	 */
	pinfo->bl_info.bl_min = 12;
	pinfo->bl_info.bl_max = 4095;
	pinfo->bl_info.bl_default = 2047;
	priv->mipi_brightness_para_type = MIPI_BL_PARA1_DBV8_AND_PARA2_DBV0;
	priv->disp_on_cmds = lcd_display_on_cmds;
	priv->disp_on_cmds_len = ARRAY_SIZE(lcd_display_on_cmds);
	priv->disp_off_cmds = lcd_display_off_cmds;
	priv->disp_off_cmds_len = ARRAY_SIZE(lcd_display_off_cmds);

	if (pinfo->base.fpga_flag == 0) {
		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_init_cmds, priv->vcc_init_cmds_len);
		if (ret != 0)
			dpu_pr_info("vcc init failed!\n");

		ret = peri_pinctrl_cmds_tx(priv->pdev, priv->pinctrl_init_cmds, priv->pinctrl_init_cmds_len);
		if (ret != 0)
			dpu_pr_info("pinctrl init failed\n");

		ret = peri_vcc_cmds_tx(priv->pdev, priv->vcc_enable_cmds, priv->vcc_enable_cmds_len);
		if (ret)
			dpu_pr_warn("vcc enable cmds handle fail!\n");
	}
	dpu_pr_info("exit!\n");
	return 0;
}

static void panel_of_device_release(struct panel_drv_private *priv)
{
	int32_t ret = 0;
	panel_base_of_device_release(priv);
	if (priv->gpio_free_cmds && (priv->gpio_free_cmds_len > 0)) {
		ret = peri_gpio_cmds_tx(priv->gpio_free_cmds, priv->gpio_free_cmds_len);
		if (ret)
			dpu_pr_info("gpio free handle err!\n");
	}
	dpu_pr_info("exit!\n");
}

panel_device_match_data(visionox310_panel_info, PANEL_VISIONOX310_ID, panel_of_device_setup, panel_of_device_release);

MODULE_LICENSE("GPL");
