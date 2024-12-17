/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hdmirx_phy.h"
#include <linux/delay.h>
#include "hdmirx_common.h"
#include "hdmirx_init.h"
#include "link/hdmirx_link_frl.h"
#include "packet/hdmirx_packet.h"

const char *g_phy_state_string[] = {
	"off",
	"handle eq",
	"auto eq",
	"dfe",
	"done",
	"max"
};

const char *g_halinput_type_string[] = {
	"OFF",
	"1.4",
	"2.0",
	"3L 3G",
	"3L 6G",
	"4L 6G",
	"4L 8G",
	"4L 10G",
	"4L 12G",
	"MAX"
};

const char *g_input_type_string[] = {
	"OFF",
	"1.4",
	"2.0",
	"3L 3G",
	"3L 6G",
	"4L 6G",
	"4L 8G",
	"4L 10G",
	"4L 12G",
	"max"
};

static hdmirx_phy_ctx g_hdmirx_phy_ctx;
static bool g_frl_ready = false;

static hdmirx_phy_ctx *hdmirx_phy_get_ctx()
{
	return &g_hdmirx_phy_ctx;
}

int hdmirx_phy_fgpa_reset(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x084, 0x3ff, 32, 0);
	mdelay(500);
	hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x084, 0x3b8, 32, 0);

	disp_pr_info("[hdmirx]phy init 0x3b8.\n");

	return 0;
}

int hdmirx_phy_asic_reset(struct hdmirx_ctrl_st *hdmirx)
{
	return 0;
}

int hdmirx_phy_init(struct hdmirx_ctrl_st *hdmirx)
{
	if (1) {
		hdmirx_phy_fgpa_reset(hdmirx);
	} else {
		hdmirx_phy_asic_reset(hdmirx);
	}

	return 0;
}


int hdmirx_phy_init_asic(struct hdmirx_ctrl_st *hdmirx)
{
	return 0;
}

static void phy_set_state(struct hdmirx_ctrl_st *hdmirx, phy_state state)
{
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx();
	phy_ctx->state = state;
	phy_ctx->wait_cnt = 0;
	phy_ctx->repeat_cnt = 0;
}

bool hdmirx_ctrl_get_tmds_ratio(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t flag;
	flag = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_TMDS_CONFIGURATION, TMDS_BIT_CLOCK_RATIO);
	return (flag > 0 ? TRUE : FALSE);
}

bool hdmirx_ctrl_get_scrbl_en(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t en, status;
	bool flag = FALSE;

	en = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_TMDS_CONFIGURATION, SCRAMBLING_ENABLE);
	status = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_TMDS_SCRAMBLER_STATUS, SCRAMBLER_STATUS);
	if (en == TRUE && status == TRUE)
		flag = TRUE;

	return flag;
}
void phy_clear_init(void)
{
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx();
	phy_ctx->state = PHY_STATE_OFF;
	phy_ctx->eq_state = PHY_EQ_STATE_OFF;
	phy_ctx->dfe_state = PHY_DFE_STATE_OFF;
	phy_ctx->run = FALSE;
	phy_ctx->wait_cnt = 0;
	phy_ctx->repeat_cnt = 0;
	phy_ctx->dfe_mode = HDMIRX_DFE_MODE_MAX;
	phy_ctx->type = HDMIRX_INPUT_MAX;
	phy_ctx->force_dfe = FALSE;
}

static void hdmirx_damix_set_dfe_en(struct hdmirx_ctrl_st *hdmirx, bool en)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG26, RG_ENDISC_L0, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG25, RG_ENDISC_L1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG24, RG_ENDISC_L2, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG28, RG_ENDISC_L3, en);
}

static void hdmirx_damix_clr_handle_dfe_code(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DTAP_OVR_LANE0, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DTAP_OVR_LANE1, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DTAP_OVR_LANE2, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DTAP_OVR_LANE3, FALSE);
}

static void hdmirx_damix_reduce_swing(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, bool en)
{
	bool cfg;

	if ((type == HDMIRX_INPUT_FRL4L12G) && (en == TRUE)) {
		cfg = TRUE;
	} else {
		cfg = FALSE;
	}
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG26, RG_EXTERNAL_DISCHARGE_L0, cfg);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG25, RG_EXTERNAL_DISCHARGE_L1, cfg);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG24, RG_EXTERNAL_DISCHARGE_L2, cfg);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG28, RG_EXTERNAL_DISCHARGE_L3, cfg);
}

static void hdmirx_damix_clear_ictrl_pfd(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD2, 0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD1, 0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD0, 0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN2, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN1, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN0, FALSE);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_ICTRL_PFD3, 0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_ICTRL_PFD_EN3, 0);
}

static void hdmirx_damix_set_ldo_power_supply_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	if (type <= HDMIRX_INPUT_20) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG29, EN_LDO_L3, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG1, EN_LDO_L2, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG2, EN_LDO_L1, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG3, EN_LDO_L0, FALSE);
	} else {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG29, EN_LDO_L3, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG1, EN_LDO_L2, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG2, EN_LDO_L1, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG3, EN_LDO_L0, TRUE);
	}
}

static hdmirx_clk_zone hdmirx_damix_clk_zone_detect_hdmirx_input_20(uint8_t value)
{
	hdmirx_clk_zone clk_zone = 0;
	if (value == 0x0) {
		clk_zone = HDMIRX_CLK_ZONE_TMDS20_85_100;
	} else if (value == 0x1) { /* 0x1: TMDS2.0 100M - 200M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS20_100_150;
	}
	return clk_zone;
}

static hdmirx_clk_zone hdmirx_damix_clk_zone_detect_hdmirx_input_frl(uint8_t value)
{
	hdmirx_clk_zone clk_zone = 0;
	if (value == 0x1) {        /* 0x1: FRL 3G */
		clk_zone = HDMIRX_CLK_ZONE_FRL_3G;
	} else if (value == 0x2) { /* 0x2: FRL 6G */
		clk_zone = HDMIRX_CLK_ZONE_FRL_6G;
	} else if (value == 0x3) { /* 0x3: FRL 8G */
		clk_zone = HDMIRX_CLK_ZONE_FRL_8G;
	} else if (value == 0x4) { /* 0x4: FRL 10G */
		clk_zone = HDMIRX_CLK_ZONE_FRL_10G;
	} else if (value == 0x5) { /* 0x5: FRL 12G */
		clk_zone = HDMIRX_CLK_ZONE_FRL_12G;
	}
	return clk_zone;
}

static hdmirx_clk_zone hdmirx_damix_clk_zone_detect_hdmirx_input_14(uint8_t value)
{
	hdmirx_clk_zone clk_zone = 0;

	if (value == 0x0) { /* 0x0: TMDS1.4 12.5M - 25M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_12P5_25;
	} else if (value == 0x1) { /* 0x1: TMDS1.4 25M - 42.5M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_25_42P5;
	} else if (value == 0x2) { /* 0x2: TMDS1.4 42.5M - 75M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_42P5_75;
	} else if (value == 0x3) { /* 0x3: TMDS1.4 75M - 150M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_75_150;
	} else if (value == 0x4) { /* 0x4: TMDS1.4 150M - 250M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_150_250;
	} else if (value == 0x5) { /* 0x5: TMDS1.4 250M - 340M */
		clk_zone = HDMIRX_CLK_ZONE_TMDS14_250_340;
	}

	return clk_zone;
}

hdmirx_clk_zone hdmirx_damix_clk_zone_detect(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone clk_zone = 0;
	uint8_t value;

	value = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_CLK_DETECT0, HDMI_ZONE_CTRL);
	switch (type) {
	case HDMIRX_INPUT_14:
		clk_zone = hdmirx_damix_clk_zone_detect_hdmirx_input_14(value);
		break;
	case HDMIRX_INPUT_20:
		clk_zone = hdmirx_damix_clk_zone_detect_hdmirx_input_20(value);
		break;
	case HDMIRX_INPUT_FRL3L3G:
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
	case HDMIRX_INPUT_FRL4L12G:
		clk_zone = hdmirx_damix_clk_zone_detect_hdmirx_input_frl(value);
		break;
	default:
		break;
	}
	return clk_zone;
}

static void hdmirx_cal_amp_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint8_t tmds_clk;

	tmds_clk = hdmirx_damix_clk_zone_detect(hdmirx, type);
	if (type == HDMIRX_INPUT_14) {
		if ((tmds_clk == HDMIRX_CLK_ZONE_TMDS14_12P5_25) ||
			(tmds_clk == HDMIRX_CLK_ZONE_TMDS14_25_42P5) ||
			(tmds_clk == HDMIRX_CLK_ZONE_TMDS14_42P5_75) ||
			(tmds_clk == HDMIRX_CLK_ZONE_TMDS14_75_150)) {
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base,  REG_ANALOG9, RG_EN_CAL_AMP, 0x0); /* 0x0: for HDMI14 low */
		} else {
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG9, RG_EN_CAL_AMP, 0x7); /* 0x7: for HDMI20 */
		}
	} else if ((type == HDMIRX_INPUT_20) || (type == HDMIRX_INPUT_FRL3L3G) ||
		(type == HDMIRX_INPUT_FRL3L6G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG9, RG_EN_CAL_AMP, 0x7); /* 0x7: for FRL 3L */
	} else if ((type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL4L8G) ||
		(type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG9, RG_EN_CAL_AMP, 0xf); /* 0xF: for FRL4L */
	}
}

void hdmirx_rterm_ctrl_set(struct hdmirx_ctrl_st *hdmirx, uint8_t rterm)
{
	if (rterm != 0xff) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG9, RG_RT_CTRL, rterm);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG11, RG_RT_CTRL_CK, rterm);
	}
}

static void hdmirx_damix_div_post_set(struct hdmirx_ctrl_st *hdmirx, bool div_post_en, hdmirx_input_type type)
{
	if ((div_post_en == FALSE) || (type > HDMIRX_INPUT_FRL3L3G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN0, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN1, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN2, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_DIVSEL_POST_EN3, FALSE);
	} else if (div_post_en == TRUE) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST0, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST1, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST2, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_DIVSEL_POST3, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN0, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN1, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A1, REG_DA_DIVSEL_POST_EN2, TRUE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_DIVSEL_POST_EN3, TRUE);
	}
}

static void hdmirx_pd_calibration_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG7, 0x4, TRUE); /* 0x4: ATOP bit Set */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG11, RG_CAL_START, FALSE);
	udelay(2); /* 2: delay 2us */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG11, RG_CAL_START, TRUE);
	udelay(10); /* 10: delay 10us */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG11, RG_CAL_START, FALSE);
	udelay(10); /* 10: delay 10us */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG7, 0x4, FALSE); /* 0x4: ATOP bit Set */
}

static void hdmirx_pll_ref_clk_set(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	if ((type == HDMIRX_INPUT_14) || (type == HDMIRX_INPUT_20)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L0, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L1, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L2, 0x0);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L3, 0x0);
	} else if ((type == HDMIRX_INPUT_FRL3L3G) || (type == HDMIRX_INPUT_FRL3L6G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L0, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L1, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L2, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L3, 0x0);
	} else if ((type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL4L8G) ||
		(type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L0, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L1, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L2, 0x1);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG0, RG_SEL_CLKIN_L3, 0x1);
	}
}

void hdmirx_damix_pre_set(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_damix_set_dfe_en(hdmirx, FALSE);
	hdmirx_damix_clr_handle_dfe_code(hdmirx);
	hdmirx_damix_reduce_swing(hdmirx, type, FALSE);
	hdmirx_damix_clear_ictrl_pfd(hdmirx);

	hdmirx_damix_set_ldo_power_supply_en(hdmirx, type);
	hdmirx_cal_amp_en(hdmirx, type);
	hdmirx_rterm_ctrl_set(hdmirx, g_efuse_1);   // handler
	hdmirx_damix_div_post_set(hdmirx, TRUE, type);
	hdmirx_pd_calibration_set(hdmirx);
	hdmirx_damix_div_post_set(hdmirx, FALSE, type);
	hdmirx_pll_ref_clk_set(hdmirx, type);
}

void hdmirx_phy_set_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, bool en)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	if (type >= HDMIRX_INPUT_MAX)
		return;

	phy_ctx = hdmirx_phy_get_ctx();

	phy_clear_init();
	phy_ctx->type = type;
	hdmirx_damix_pre_set(hdmirx, type);
	phy_ctx->run = en;
}

void phy_set_power(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	bool en = FALSE;
	if (type > HDMIRX_INPUT_MAX)
		return;

	en = g_reset_vco;
	if (type != HDMIRX_INPUT_OFF && en == TRUE) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A12, REG_DA_RESET_VCO_EN, 0xf);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A12, REG_DA_RESET_VCO, 0xf);
	}
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_RT, g_pd_cfg[type][0]);           /* 0: Pd RT */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_PHDAC, g_pd_cfg[type][1]);        /* 1: Pd PHDAC */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_LDO, g_pd_cfg[type][2]);          /* 2: Pd LDO */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_LANE, g_pd_cfg[type][3]);         /* 3: Pd LANE */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_CLK, g_pd_cfg[type][4]);          /* 4: Pd CLK */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_BG, g_pd_cfg[type][5]);           /* 5: Pd BG */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_PD_PLL, g_pd_cfg[type][6]);          /* 6: Pd PLL */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG33, RG_PD_LDO_NODIE, g_pd_cfg[type][7]);   /* 7: Pd NODIE */
}

bool hdmirx_ctrl_set_phy_power(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	bool chg = FALSE;
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();
	if (type != hdmirx->input_type) {
		g_frl_ready = false;
		phy_set_power(hdmirx, HDMIRX_INPUT_OFF);
		msleep(1);
		phy_set_power(hdmirx, type);
		msleep(1);
		hdmirx_phy_set_en(hdmirx, type, TRUE);
		hdmirx->input_type = type;
		chg = TRUE;
	}
	if ((!g_frl_ready) && (type >= HDMIRX_INPUT_FRL3L3G)) {
		phy_clear_init();
		phy_ctx->type = type;
		phy_ctx->run = TRUE;
	}
	return chg;
}

bool hdmirx_ctrl_power_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_input_type type;
	bool chg = FALSE;
	bool scrbl_en = FALSE;

	if (hdmirx_frl_get_rate(hdmirx) != HDMIRX_FRL_RATE_TMDS)
		return FALSE; // frl phy set in frl proc

	if (hdmirx_ctrl_get_tmds_ratio(hdmirx) == FALSE) {
		if (hdmirx_ctrl_get_scrbl_en(hdmirx) == TRUE) {
		} else {
		}
	}
	scrbl_en = hdmirx_ctrl_get_scrbl_en(hdmirx);

	type = ((hdmirx_ctrl_get_tmds_ratio(hdmirx) == TRUE) ||
		(scrbl_en == TRUE)) ? HDMIRX_INPUT_20 : HDMIRX_INPUT_14;
	chg = hdmirx_ctrl_set_phy_power(hdmirx, type);

	return chg;
}

bool hdmirx_damix_get_clk_stable(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t clk_stable;
	bool status;
	clk_stable = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_CLK_DETECT0, HDMI_CLK_STABLE);
	status = clk_stable > 0 ? TRUE : FALSE;
	return status;
}

void hdmirx_ctrl_phy_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_input_type type;
	hdmirx_frl_rate rate;
	bool scrbl_en = FALSE;

	rate = hdmirx_frl_get_rate(hdmirx);
	if (rate != HDMIRX_FRL_RATE_TMDS) {
		type = HDMIRX_INPUT_FRL3L3G + (rate - HDMIRX_FRL_RATE_FRL3L3G);
		if ((type < HDMIRX_INPUT_MAX) && (hdmirx_frl_is_ltpdone(hdmirx) == TRUE)) {
			/* dont need to do drv_hdmirx_phy_set_en(port, type, OSAL_TRUE); */
		}
		return;
	}

	if (hdmirx_ctrl_get_tmds_ratio(hdmirx) == FALSE)
		scrbl_en = hdmirx_ctrl_get_scrbl_en(hdmirx);

	scrbl_en = hdmirx_ctrl_get_scrbl_en(hdmirx);
	if ((hdmirx_ctrl_get_tmds_ratio(hdmirx) == TRUE) || (scrbl_en == TRUE)) {
		type = HDMIRX_INPUT_20;
	} else {
		type = HDMIRX_INPUT_14;
	}
	hdmirx_phy_set_en(hdmirx, type, TRUE);
}

void hdmirx_dphy_clear_ced_erro_cnt(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, 0xFFFFFFFF, 0xFFFFFFFF);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_SHADOW_CED_CLR, CFG_CNT_SW_CLR, TRUE);
	udelay(1);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base,  REG_SHADOW_CED_CLR, CFG_CNT_SW_CLR, FALSE);
	return;
}

void hdmirx_phy_ckdt_init(void)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();
	phy_ctx->is_ckdt = false;
}

void hdmirx_ctrl_phy_handler(struct hdmirx_ctrl_st *hdmirx)
{
	bool clk_statble = FALSE;
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();

	if (hdmirx->pwrstatus == TRUE)
		hdmirx_ctrl_power_set(hdmirx);

	clk_statble = hdmirx_damix_get_clk_stable(hdmirx);
	if (clk_statble != phy_ctx->is_ckdt) {
		if (clk_statble == TRUE){
			hdmirx_dphy_clear_ced_erro_cnt(hdmirx);
			hdmirx_ctrl_phy_set(hdmirx);
		}
		phy_ctx->is_ckdt = clk_statble;
	}
}
// phy process
static void phy_deskew_reset(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t deskew;
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_RX_PWD_SEL, CFG_FRL_MODE_EN, 1);
	mdelay(1);
	deskew = inp32(hdmirx->hdmirx_pwd_base + REG_CH_DESKEW_STATE);
	disp_pr_info("[HDMI]deskew 0x%x !", deskew);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_RX_PWD_SEL, CFG_DPHY_CHANNEL_FIFO_RST_LANE_EN, 1);
	disp_pr_info("[HDMI]if deskew fifo full, phy reset!");
}

static void phy_state_set_done(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();
	disp_pr_info("[HDMI]== phy state done!");
	phy_set_state(hdmirx, PHY_STATE_DONE);
	phy_ctx->run = FALSE;
	hdmirx->phy_done = true;
	hdmirx_dphy_clear_ced_erro_cnt(hdmirx);
	phy_deskew_reset(hdmirx);

	hdmirx_irq_enable(hdmirx);
	hdmirx_packet_vrr_mask(hdmirx);
}

static bool hdmirx_damix_hdmirx_input_14_check_eq(uint32_t eq_status0,
	uint32_t eq_status1, uint32_t eq_status2)
{
	bool result = 0;
	if ((eq_status0 == 0x100) && (eq_status1 == 0x100) && (eq_status2 == 0x100)) { /* 0x100: auto_eq ready */
		result = TRUE;
	} else {
		result = FALSE;
	}
	return result;
}

static bool hdmirx_damix_hdmirx_input_frl_check_eq(uint32_t eq_status0,
	uint32_t eq_status1, uint32_t eq_status2, uint32_t eq_status3)
{
	bool result = 0;
	if ((eq_status0 == 0x100) && (eq_status1 == 0x100) && (eq_status2 == 0x100)
		&& (eq_status3 == 0x100)) { /* 0x100: auto_eq ready */
		result = TRUE;
	} else {
		result = FALSE;
	}
	return result;
}

bool hdmirx_damix_get_auto_eq_result(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	bool result = 0;
	uint32_t eq_status0;
	uint32_t eq_status1;
	uint32_t eq_status2;
	uint32_t eq_status3;

	eq_status0 = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_AUTOEQ11, AUTO_EQ_ST_LANE0);
	eq_status1 = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_AUTOEQ11, AUTO_EQ_ST_LANE1);
	eq_status2 = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_AUTOEQ11, AUTO_EQ_ST_LANE2);
	eq_status3 = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_AUTOEQ20, AUTO_EQ_ST_LANE3);
	disp_pr_info("[HDMI]auto_eq_result %d   %d   %d   %d!", eq_status0, eq_status1, eq_status2, eq_status3);
	if ((type == HDMIRX_INPUT_14) || (type == HDMIRX_INPUT_20) ||
		(type == HDMIRX_INPUT_FRL3L3G) || (type == HDMIRX_INPUT_FRL3L6G)) {
		result = hdmirx_damix_hdmirx_input_14_check_eq(eq_status0, eq_status1, eq_status2);
	} else if ((type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL4L8G) ||
		(type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		result = hdmirx_damix_hdmirx_input_frl_check_eq(eq_status0, eq_status1, eq_status2, eq_status3);
	}
	return result;
}

bool hdmirx_damix_get_fix_eq_result(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint32_t fix_eq_state;
	fix_eq_state = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_STATE, FIXED_EQ_STATE);
	disp_pr_info("[HDMI]fix_eq_result %d!", fix_eq_state);
	return fix_eq_state == 4 ? TRUE : FALSE;
}

hdmirx_damix_status hdmirx_damix_get_status(struct hdmirx_ctrl_st *hdmirx,
	hdmirx_input_type input_type, hdmirx_damix_type damix_type)
{
	hdmirx_damix_status status = 0;

	switch (damix_type) {
	case HDMIRX_DAMIX_PLL:
		status = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_PLL_LOCK1, PLL_LOCK);
		break;
	case HDMIRX_DAMIX_CLK:
		status = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_CLK_DETECT0, HDMI_CLK_STABLE);
		break;
	case HDMIRX_DAMIX_AUTO_EQ:
		status = (hdmirx_damix_status)hdmirx_damix_get_auto_eq_result(hdmirx, input_type);
		break;
	case HDMIRX_DAMIX_FIX_EQ:
		status = (hdmirx_damix_status)hdmirx_damix_get_fix_eq_result(hdmirx, input_type);
		break;
	case HDMIRX_DAMIX_DFE:
		break;
	default:
		break;
	}
	return status;
}

static uint32_t phy_get_sleep_cnt(void)
{
	return SLEEP_TIME_CNT;
}

static void hdmirx_damix_set_kvco(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	switch (type) {
	case HDMIRX_INPUT_FRL3L3G:
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
	case HDMIRX_INPUT_FRL4L12G:
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG29, BIT3_0, g_frl_kvco[0]); /* 2: dig sel test clk */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG1, BIT3_0, g_frl_kvco[1]); /* 2: dig sel test clk */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG2, BIT3_0, g_frl_kvco[2]); /* 2: dig sel test clk */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG3, BIT3_0, g_frl_kvco[3]); /* 2: dig sel test clk */
		break;
	default:
		break;
	}
}

static void hdmirx_damix_set_ictrl(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint32_t ictrl;
	bool en = FALSE;

	if (type >= HDMIRX_INPUT_FRL3L3G && type <= HDMIRX_INPUT_FRL4L12G)
		en = TRUE;

	switch (type) {
	case HDMIRX_INPUT_FRL3L3G:
		ictrl = g_ictrl_pd0[0];
		break;
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
		ictrl = g_ictrl_pd0[0];
		break;
	case HDMIRX_INPUT_FRL4L8G:
		ictrl = g_ictrl_pd0[1];
		break;
	case HDMIRX_INPUT_FRL4L10G:
		ictrl = g_ictrl_pd0[2];
		break;
	case HDMIRX_INPUT_FRL4L12G:
		ictrl = g_ictrl_pd0[3];
		break;
	default:
		ictrl = g_ictrl_pd0[4];
		break;
	}
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD0, ictrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD1, ictrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD2, ictrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_ICTRL_PD3, ictrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD_EN0, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD_EN1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A4, REG_DA_ICTRL_PD_EN2, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A11, REG_DA_ICTRL_PD_EN3, en);
}

static void hdmirx_damix_ictrl_pfd_code_set(struct hdmirx_ctrl_st *hdmirx, uint8_t clk_zone)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD2, g_ictrl_pfd_code[2]); /* 3/2: channel 2 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD1, g_ictrl_pfd_code[1]); /* 3/1: channel 1 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD0, g_ictrl_pfd_code[0]); /* 3/0: channel 0 */
}

static void hdmirx_damix_set_ictrl_pfd(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	bool en = TRUE;
	uint8_t clk_zone;

	clk_zone = hdmirx_damix_clk_zone_detect(hdmirx, type);
	if (clk_zone > HDMIRX_CLK_ZONE_TMDS14_250_340)
		en = FALSE;

	if (en == TRUE)
		hdmirx_damix_ictrl_pfd_code_set(hdmirx, clk_zone);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN2, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A5, REG_DA_ICTRL_PFD_EN0, en);
}

static void hdmirx_damix_set_kvco_ratio(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint8_t tmds_clk;
	uint8_t kvco_value = g_kvco_ratio[2];

	if (type == HDMIRX_INPUT_14) {
		tmds_clk = hdmirx_damix_clk_zone_detect(hdmirx, type);
		if ((tmds_clk <= HDMIRX_CLK_ZONE_TMDS14_75_150))
			kvco_value = g_kvco_ratio[0]; /* 0x3: the kvco value */
	}

	if ((type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL3L6G))
		kvco_value = g_kvco_ratio[1];

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG29, TEST_KVCO_L3, kvco_value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG1, TEST_KVCO_L2, kvco_value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG2, TEST_KVCO_L1, kvco_value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG3, TEST_KVCO_L0, kvco_value);
}

static void hdmirx_damix_set_rctrl(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint8_t tmds_clk;
	bool en = FALSE;
	uint32_t rctrl = g_rctrl[2];

	switch (type) {
	case HDMIRX_INPUT_14:
		tmds_clk = hdmirx_damix_clk_zone_detect(hdmirx, type);
		if (tmds_clk == HDMIRX_CLK_ZONE_TMDS14_42P5_75 || tmds_clk == HDMIRX_CLK_ZONE_TMDS14_75_150) {
			rctrl = g_rctrl[0];
			en = TRUE;
		}
		break;
	case HDMIRX_INPUT_FRL3L3G:
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
		rctrl = g_rctrl[1];
		en = TRUE;
		break;
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
	case HDMIRX_INPUT_FRL4L12G:
		rctrl = g_rctrl[2];
		en = TRUE;
		break;
	case HDMIRX_INPUT_20:
	default:
		break;
	}
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL0, rctrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL1, rctrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL2, rctrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_RCTRL_PLL3, rctrl);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL_EN0, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL_EN1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A7, REG_DA_RCTRL_PLL_EN2, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_RCTRL_PLL_EN3, en);
}

static void hdmirx_damix_set_pll_bandwidth(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_damix_set_ictrl(hdmirx, type);
	hdmirx_damix_set_ictrl_pfd(hdmirx, type);
	hdmirx_damix_set_kvco_ratio(hdmirx, type);
	hdmirx_damix_set_rctrl(hdmirx, type);
}

void hdmirx_damix_common_cfg(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	if (g_reset_vco_a) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A12, REG_DA_RESET_VCO_EN, 0xf); /* 0xf: set 4ch enable */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A12, REG_DA_RESET_VCO, 0x0);
	}
	hdmirx_damix_set_kvco(hdmirx, type);
	hdmirx_damix_set_pll_bandwidth(hdmirx, type);

	if (g_reduce_swing)
		hdmirx_damix_reduce_swing(hdmirx, type, TRUE);
}

bool hdmirx_damix_need_autoeq_mode(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone tmds_clk;
	bool result = FALSE;
	/* get tmds clk for HDMI1.4 Eq mode */
	tmds_clk = hdmirx_damix_clk_zone_detect(hdmirx, type);

	/* if tmds_clk >= 1.5G, Auto Eq, else case use handle Eq */
	switch (type) {
	case HDMIRX_INPUT_20:
	case HDMIRX_INPUT_FRL3L3G:
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
	case HDMIRX_INPUT_FRL4L12G:
		result = TRUE;
		break;
	case HDMIRX_INPUT_14:
		if ((tmds_clk == HDMIRX_CLK_ZONE_TMDS14_150_250) ||
			(tmds_clk == HDMIRX_CLK_ZONE_TMDS14_250_340)) {
			result = TRUE;
		} else {
			result = FALSE;
		}
		break;
	default:
		break;
	}
	return result;
}


void hdmirx_damix_clr_handle_eq_code(struct hdmirx_ctrl_st *hdmirx)
{
	/* Port0 Handle Eq Enable */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN0, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN1, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN2, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN3, FALSE);
	/* Port0 Handle Eq Set Value */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE0, 0x0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE1, 0x0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE2, 0x0);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE3, 0x0);
}

static void hdmirx_damix_adaptive_eq_mode(struct hdmirx_ctrl_st *hdmirx, bool en)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A3, REG_DA_HDMI14_EQ_SEL0, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A3, REG_DA_HDMI14_EQ_SEL1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A3, REG_DA_HDMI14_EQ_SEL2, en);
}

void hdmirx_damix_fix_eq_init(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone clk_zone;
	const uint8_t len_max = HDMIRX_CLK_ZONE_TMDS14_150_250;

	clk_zone = hdmirx_damix_clk_zone_detect(hdmirx, type);
	if (clk_zone >= len_max)
		return;

	hdmirx_damix_clr_handle_eq_code(hdmirx);
	hdmirx_damix_adaptive_eq_mode(hdmirx, TRUE);

	hdmirx_fix_code_set();

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_EQ_SEL, FIX_EQ_CNT);
	/* 8 eq for different clk zone */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED0,
										 g_fix_code[clk_zone][0]); /* 0: fixeq 0 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED1,
										 g_fix_code[clk_zone][1]); /* 1: fixeq 1 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED2,
										 g_fix_code[clk_zone][2]); /* 2: fixeq 2 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED3,
										 g_fix_code[clk_zone][3]); /* 3: fixeq 3 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED4,
										 g_fix_code[clk_zone][4]); /* 4: fixeq 4 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG0, REG_EQ_FIXED5,
										 g_fix_code[clk_zone][5]); /* 5: fixeq 5 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_EQ_FIXED6,
										 g_fix_code[clk_zone][6]); /* 6: fixeq 6 */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_EQ_FIXED7,
										 g_fix_code[clk_zone][7]); /* 7: fixeq 7 */
	return;
}

void hdmirx_damix_set_fix_eq_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, bool en)
{
	if (en == TRUE) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_FIXED_SCAN_EQ_EN, FALSE);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_FIXED_SCAN_EQ_EN, TRUE);
	} else {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FIXED_EQ_CFG1, REG_FIXED_SCAN_EQ_EN, FALSE);
	}
}

static void hdmirx_damix_set_auto_eq_time(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	if ((type == HDMIRX_INPUT_FRL3L3G) || (type == HDMIRX_INPUT_FRL3L6G) ||
		(type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL4L8G) ||
		(type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ3, REG_SYMBOL_DET_DURATION, g_auto_eq[3]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ4, REG_EQ_UNDER_OVER_CHECK_DURATION, g_auto_eq[4]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ4, REG_EQ_UPDATE_DURATION, g_auto_eq[5]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_MHL_AUTO_EQ_EN, FALSE);
	} else {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ3, REG_SYMBOL_DET_DURATION, g_auto_eq[0]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ4, REG_EQ_UNDER_OVER_CHECK_DURATION, g_auto_eq[1]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ4, REG_EQ_UPDATE_DURATION, g_auto_eq[2]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_MHL_AUTO_EQ_EN, TRUE);
	}
}

static void hdmirx_damix_set_isel_eq(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	uint32_t isel = g_isel[0];

	switch (type) {
	case HDMIRX_INPUT_20: /* in case of HDMI20, FRL4L8G, FRL4L10G, FRL4L12G, the isel is 1 */
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
	case HDMIRX_INPUT_FRL4L12G:
		isel = g_isel[1];
		break;
	default: /* in case of HDMI14, FRL3L3G, FRL3L6G, FRL4L6G, the isel is 0 */
		break;
	}

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_ISEL_EQ_L0, isel);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_ISEL_EQ_L1, isel);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_ISEL_EQ_L2, isel);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_ANALOG8, RG_ISEL_EQ_L3, isel);
}

static uint32_t hdmirx_damix_set_bw_by_zone(hdmirx_clk_zone zone)
{
	uint32_t ret = g_eq_bandwidth[0];

	if (zone == HDMIRX_CLK_ZONE_TMDS14_150_250) {
		ret = g_eq_bandwidth[1]; /* 3: bandwidth value */
	} else if (zone == HDMIRX_CLK_ZONE_TMDS14_250_340) {
		ret = g_eq_bandwidth[2]; /* 2: bandwidth value */
	}
	return ret;
}

static uint32_t hdmirx_damix_set_bw_by_chiptype(hdmirx_input_type type)
{
	uint32_t ret;

	if (type == HDMIRX_INPUT_FRL4L8G) {
		ret = g_eq_bandwidth[6]; /* 5: bandwidth value for revision_a */
	} else {
		ret = g_eq_bandwidth[7]; /* 4: bandwidth value for revision_a */
	}
	return ret;
}

static void hdmirx_damix_sweep_clr(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_SWEEP_EQ_CFG, REG_EQ_SWEEP_MODE, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_SWEEP_EQ_CFG, REG_EQ_ERR_CHECK_MODE, FALSE);
}

static void hdmirx_damix_set_bandwidth(struct hdmirx_ctrl_st *hdmirx, uint32_t bandwidth)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL0, bandwidth);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL1, bandwidth);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL2, bandwidth);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL3, bandwidth);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL_EN0, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL_EN1, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL_EN2, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A10, REG_DA_BW_SEL_EN3, TRUE);
}

static void hdmirx_damix_set_eq_bandwidth(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone zone;
	uint32_t bandwidth = 0;

	zone = hdmirx_damix_clk_zone_detect(hdmirx, type);

	switch (type) {
	case HDMIRX_INPUT_14:
		bandwidth = hdmirx_damix_set_bw_by_zone(zone);
		break;
	case HDMIRX_INPUT_20:
		bandwidth = g_eq_bandwidth[3];  /* 6: bandwidth value */
		break;
	case HDMIRX_INPUT_FRL3L3G:
		bandwidth = g_eq_bandwidth[4];  /* 2: bandwidth value */
		break;
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
		bandwidth = g_eq_bandwidth[5];  /* 0: bandwidth value */
		break;
	case HDMIRX_INPUT_FRL4L8G:
	case HDMIRX_INPUT_FRL4L10G:
		bandwidth = hdmirx_damix_set_bw_by_chiptype(type);
		break;
	case HDMIRX_INPUT_FRL4L12G:
		bandwidth = g_eq_bandwidth[8]; /* 4: bandwidth value */
		break;
	default:
		break;
	}
	hdmirx_damix_set_bandwidth(hdmirx, bandwidth);
}

static void hdmirx_damix_sweep_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_SWEEP_EQ_CFG, REG_EQ_SWEEP_MODE, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_SWEEP_EQ_CFG, REG_EQ_ERR_CHECK_MODE, TRUE);
}

static void hdmirx_damix_set_coarse_tune_conditions(struct hdmirx_ctrl_st *hdmirx,
														hdmirx_input_type type, uint32_t coarse)
{
	if ((type == HDMIRX_INPUT_FRL3L3G) || (type == HDMIRX_INPUT_FRL3L6G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE0,
			CFG_COARSE_TUNE_CONDITIONS_LANE0, coarse);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE1,
			CFG_COARSE_TUNE_CONDITIONS_LANE1, coarse);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE2,
			CFG_COARSE_TUNE_CONDITIONS_LANE2, coarse);
	} else if ((type == HDMIRX_INPUT_FRL4L6G) ||
			   (type == HDMIRX_INPUT_FRL4L8G) ||
			   (type == HDMIRX_INPUT_FRL4L10G) ||
			   (type == HDMIRX_INPUT_FRL4L12G)) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE0,
			CFG_COARSE_TUNE_CONDITIONS_LANE0, coarse);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE1,
			CFG_COARSE_TUNE_CONDITIONS_LANE1, coarse);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE2,
			CFG_COARSE_TUNE_CONDITIONS_LANE2, coarse);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE3,
			CFG_COARSE_TUNE_CONDITIONS_LANE3, coarse);
	}
}

void hdmirx_damix_auto_eq_init(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone clk_zone;

	clk_zone = hdmirx_damix_clk_zone_detect(hdmirx, type);

	hdmirx_damix_clr_handle_eq_code(hdmirx);
	hdmirx_damix_set_fix_eq_en(hdmirx, type, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE0, CFG_FRL_EQ_MODE, 0x0);
	hdmirx_damix_set_auto_eq_time(hdmirx, type);
	hdmirx_damix_set_isel_eq(hdmirx, type);
	hdmirx_damix_set_eq_bandwidth(hdmirx, type);

	if (type == HDMIRX_INPUT_14) {
		if (g_auto_eq_ctl[0])
			hdmirx_damix_sweep_set(hdmirx);

		if (g_auto_eq_ctl[1])
			hdmirx_damix_adaptive_eq_mode(hdmirx, TRUE);

		/* 0x3f80,0x2,0xf: auto eq error detect cfg */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ10, REG_EQ_ERROR_CHECK_DURATION, g_auto_eq_init[0]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ10, REG_EQ_ERROR_STEP, g_auto_eq_init[1]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_PHASE_STABLE_THRESHOLD, g_auto_eq_init[2]);
		/* 0x3: eq strength */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_EQ_STRENGTH_STEP, g_auto_eq_init[3]);
		/* 0x0: start strength */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_START_EQ_STRENGTH, g_auto_eq_init[4]);
	} else if (type == HDMIRX_INPUT_20) {
		if (g_auto_eq_ctl[2])
			hdmirx_damix_sweep_clr(hdmirx);

		/* 0x3f80,0x2,0xf: auto eq error detect cfg */
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ10, REG_EQ_ERROR_CHECK_DURATION, g_auto_eq_init[0]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ10, REG_EQ_ERROR_STEP, g_auto_eq_init[1]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_PHASE_STABLE_THRESHOLD, g_auto_eq_init[2]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_EQ_STRENGTH_STEP, g_auto_eq_init[3]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_START_EQ_STRENGTH, g_auto_eq_init[4]);

		if (g_auto_eq_ctl[3])
			hdmirx_damix_adaptive_eq_mode(hdmirx, FALSE);
	} else if ((type == HDMIRX_INPUT_FRL3L3G) || (type == HDMIRX_INPUT_FRL3L6G) ||
			   (type == HDMIRX_INPUT_FRL4L6G) || (type == HDMIRX_INPUT_FRL4L8G) ||
			   (type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		if (g_auto_eq_ctl[4])
			hdmirx_damix_adaptive_eq_mode(hdmirx, TRUE);

		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_EQ_STRENGTH_STEP, g_auto_eq_init[5]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_START_EQ_STRENGTH, g_auto_eq_init[6]);
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_FRL_AUTO_EQ_CFG0_LANE0, CFG_FRL_EQ_MODE, 0x1);
		hdmirx_damix_set_coarse_tune_conditions(hdmirx, type, g_coarse); /* 0x2d: coarsetune conditions */
	}
	if (type == HDMIRX_INPUT_FRL4L12G) {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_EQ_START_LIMIT, g_auto_eq_init[7]);
	} else {
		hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ8, REG_EQ_START_LIMIT, g_auto_eq_init[8]);
	}
}

void hdmirx_damix_set_auto_eq_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, bool en)
{
	if (en == TRUE) {
		switch (type) {
		case HDMIRX_INPUT_FRL3L3G:
		case HDMIRX_INPUT_FRL3L6G:
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE0, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE1, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE2, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE3, FALSE);

			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE0, TRUE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE1, TRUE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE2, TRUE);
			break;
		case HDMIRX_INPUT_FRL4L6G:
		case HDMIRX_INPUT_FRL4L8G:
		case HDMIRX_INPUT_FRL4L10G:
		case HDMIRX_INPUT_FRL4L12G:
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE0, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE1, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE2, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE3, FALSE);

			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE0, TRUE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE1, TRUE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE2, TRUE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, CFG_FRL_AUTO_EQ_EN_LANE3, TRUE);
			break;
		case HDMIRX_INPUT_14:
		case HDMIRX_INPUT_20:
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_MHL_AUTO_EQ_EN, FALSE);
			hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_AUTOEQ0, REG_MHL_AUTO_EQ_EN, TRUE);
			break;
		default:
			break;
		}
	} else {
	}
}

static void phy_state_off_proc(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	uint32_t delay_time;
	hdmirx_input_type type;
	hdmirx_damix_status clk_stable, pll_stable;
	bool auto_eq_en;

	phy_ctx = hdmirx_phy_get_ctx();
	type = phy_ctx->type;
	clk_stable = hdmirx_damix_get_status(hdmirx, type, HDMIRX_DAMIX_CLK);
	pll_stable = hdmirx_damix_get_status(hdmirx, type, HDMIRX_DAMIX_PLL);
	delay_time = phy_get_sleep_cnt();

	if ((clk_stable == HDMIRX_DAMIX_STATUS_DONE) && (pll_stable == HDMIRX_DAMIX_STATUS_DONE)) {
		hdmirx_damix_common_cfg(hdmirx, type);
		auto_eq_en = hdmirx_damix_need_autoeq_mode(hdmirx, type);
		phy_ctx->wait_cnt = 0;
		if (auto_eq_en == FALSE) {
			disp_pr_info("[HDMI] into handle eq");
			phy_set_state(hdmirx, PHY_STATE_HANDLE_EQ);
			hdmirx_damix_fix_eq_init(hdmirx, type);
			hdmirx_damix_set_fix_eq_en(hdmirx, type, TRUE);
			phy_ctx->eq_state = PHY_EQ_STATE_HANDLE_CHECKING;
		} else {
			phy_set_state(hdmirx, PHY_STATE_AUTO_EQ);
			hdmirx_damix_auto_eq_init(hdmirx, type);
			hdmirx_damix_set_auto_eq_en(hdmirx, type, TRUE);
			phy_ctx->eq_state = PHY_EQ_STATE_AUTO_CHECKING;
		}
	} else {
		phy_ctx->wait_cnt += delay_time;
		if (phy_ctx->wait_cnt > PHY_STATE_OFF_TIME_CNT) {
			disp_pr_err("[HDMI] phy off timeout");
			phy_state_set_done(hdmirx);
		}
	}
}

void hdmirx_damix_set_handle_eq_code(struct hdmirx_ctrl_st *hdmirx, uint8_t eq_code)
{
	/* Port0 Handle Eq Set Value */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE0, eq_code);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE1, eq_code);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE2, eq_code);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE3, eq_code);

	/* change by lcg */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN0, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN1, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN2, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_D2A2, REG_DA_EQ_CODE_EN3, TRUE);
}

static void hdmirx_damix_set_set_eq_by_zone(struct hdmirx_ctrl_st *hdmirx, hdmirx_clk_zone zone)
{
	if ((zone == HDMIRX_CLK_ZONE_TMDS14_12P5_25) || (zone == HDMIRX_CLK_ZONE_TMDS14_25_42P5) ||
		(zone == HDMIRX_CLK_ZONE_TMDS14_42P5_75) || (zone == HDMIRX_CLK_ZONE_TMDS14_75_150)) {
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[0]); /* 0x8: HDMI14 low eq code */
	} else if ((zone == HDMIRX_CLK_ZONE_TMDS14_150_250) ||
			   (zone == HDMIRX_CLK_ZONE_TMDS14_250_340)) {
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[1]); /* 0xF: HDMI1.4 high eq code */
	}
}

void hdmirx_damix_set_force_eq(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_clk_zone zone;
	zone = hdmirx_damix_clk_zone_detect(hdmirx, type);

	switch (type) {
	case HDMIRX_INPUT_14:
		hdmirx_damix_set_set_eq_by_zone(hdmirx, zone);
		break;
	case HDMIRX_INPUT_20:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[2]); /* 0x12: HDMI20 low eq code */
		break;
	case HDMIRX_INPUT_FRL3L3G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[3]); /* 0x12: FRL 3G eq code */
		break;
	case HDMIRX_INPUT_FRL3L6G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[4]); /* 0xF: FRL 3L6G eq code */
		break;
	case HDMIRX_INPUT_FRL4L6G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[5]); /* 0xf: FRL 4L6G low eq code */
		break;
	case HDMIRX_INPUT_FRL4L8G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[6]); /* 0x16: FRL 8G low eq code */
		break;
	case HDMIRX_INPUT_FRL4L10G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[7]); /* 0x1f: FRL 10G eq code */
		break;
	case HDMIRX_INPUT_FRL4L12G:
		hdmirx_damix_set_handle_eq_code(hdmirx, g_force_eq_code[8]); /* 0x1e: FRL 12G eq code */
		break;
	default:
		break;
	}
}

static void phy_state_handle_eq_proc(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	uint32_t delay_time;
	bool result;
	hdmirx_input_type type;

	delay_time = phy_get_sleep_cnt();
	phy_ctx = hdmirx_phy_get_ctx();
	type = phy_ctx->type;
	if (phy_ctx->wait_cnt < PHY_STATE_HANDLE_EQ_TIME_CNT) {
		phy_ctx->wait_cnt += delay_time;
		result = hdmirx_damix_get_fix_eq_result(hdmirx, type);
		if (result == TRUE) {
			phy_ctx->eq_state = PHY_EQ_STATE_HANDLE_DONE;
			phy_state_set_done(hdmirx);
		}
		return;
	}
	phy_ctx->repeat_cnt++;
	if (phy_ctx->repeat_cnt < PHY_STATE_HANDLE_EQ_REPEAT_CNT) {
		disp_pr_info("time %u set handle eq fail,repeate agin!total:%u\n",
			phy_ctx->repeat_cnt, PHY_STATE_HANDLE_EQ_REPEAT_CNT);
		hdmirx_damix_set_fix_eq_en(hdmirx, type, TRUE);
		phy_ctx->wait_cnt = 0;
		return;
	}
	disp_pr_info( "[HDMI]== phy handle eq fail,set fixed eq!");
	hdmirx_damix_set_force_eq(hdmirx, type);
	phy_state_set_done(hdmirx);
	phy_ctx->eq_state = PHY_EQ_STATE_HANDLE_TIMEOUT;
}

static void phy_get_time_cnt_info(hdmirx_input_type type, uint32_t *wait_time, uint32_t *repeat_cnt)
{
	uint32_t cnt;
	cnt = PHY_STATE_21_AUTO_EQ_TIME_CNT;
	if (type <= HDMIRX_INPUT_20) {
		*wait_time = PHY_STATE_20_AUTO_EQ_TIME_CNT;
		*repeat_cnt = PHY_STATE_20_AUTO_EQ_REPEAT_CNT;
	} else {
		*wait_time = cnt;
		*repeat_cnt = PHY_STATE_21_AUTO_EQ_REPEAT_CNT;
	}
}

bool hdmirx_damix_need_dfe(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	if ((type == HDMIRX_INPUT_FRL4L10G) || (type == HDMIRX_INPUT_FRL4L12G)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void hdmirx_damix_dfe_init(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT14, 1);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT17, 1);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT20, 1);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT23, 1);

	/* 0xf, 0x3f, 0x3f, 0x3fc, 0x3: dfe cfg */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE0, CFG_DFE_STABLE_THRESHOD_LANE0, g_dfe_cfg[0]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE0, CFG_DTAP_THRESHOLD_LANE0, g_dfe_cfg[1]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE0, CFG_TTAP_THRESHOLD_LANE0, g_dfe_cfg[2]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE0, CFG_DFE_TIMEOUT_CNT_LANE0, g_dfe_cfg[3]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE0, CFG_DFE_WAIT_CNT_LANE0, g_dfe_cfg[4]);
	/* 0xf, 0x3f, 0x3f, 0x3fc, 0x3: dfe cfg */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE1, CFG_DFE_STABLE_THRESHOD_LANE1, g_dfe_cfg[0]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE1, CFG_DTAP_THRESHOLD_LANE1, g_dfe_cfg[1]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE1, CFG_TTAP_THRESHOLD_LANE1, g_dfe_cfg[2]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE1, CFG_DFE_TIMEOUT_CNT_LANE1, g_dfe_cfg[3]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE1, CFG_DFE_WAIT_CNT_LANE1, g_dfe_cfg[4]);
	/* 0xf, 0x3f, 0x3f, 0x3fc, 0x3: dfe cfg */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE2, CFG_DFE_STABLE_THRESHOD_LANE2, g_dfe_cfg[0]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE2, CFG_DTAP_THRESHOLD_LANE2, g_dfe_cfg[1]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE2, CFG_TTAP_THRESHOLD_LANE2, g_dfe_cfg[2]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE2, CFG_DFE_TIMEOUT_CNT_LANE2, g_dfe_cfg[3]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE2, CFG_DFE_WAIT_CNT_LANE2, g_dfe_cfg[4]);
	/* 0xf, 0x3f, 0x3f, 0x3fc, 0x3: dfe cfg */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE3, CFG_DFE_STABLE_THRESHOD_LANE3, g_dfe_cfg[0]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE3, CFG_DTAP_THRESHOLD_LANE3, g_dfe_cfg[1]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG1_LANE3, CFG_TTAP_THRESHOLD_LANE3, g_dfe_cfg[2]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE3, CFG_DFE_TIMEOUT_CNT_LANE3, g_dfe_cfg[3]);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG2_LANE3, CFG_DFE_WAIT_CNT_LANE3, g_dfe_cfg[4]);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DFE_JUDGE_MASK_LANE0, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DFE_JUDGE_MASK_LANE1, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DFE_JUDGE_MASK_LANE2, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DFE_JUDGE_MASK_LANE3, FALSE);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DFE_DONE_MODE_LANE3, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DFE_DONE_MODE_LANE3, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DFE_DONE_MODE_LANE3, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DFE_DONE_MODE_LANE3, FALSE);
}

static void hdmirx_damix_set_forec_dfe_ttap(struct hdmirx_ctrl_st *hdmirx, bool en, uint32_t value)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_TTAP_SW_LANE0, value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_TTAP_SW_LANE1, value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_TTAP_SW_LANE2, value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_TTAP_SW_LANE3, value);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_TTAP_OVR_LANE0, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_TTAP_OVR_LANE1, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_TTAP_OVR_LANE2, en);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_TTAP_OVR_LANE3, en);
}

static void hdmirx_damix_dfe_mode_cfg(struct hdmirx_ctrl_st *hdmirx, uint32_t mode)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DFE_MODE_LANE0, mode);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DFE_MODE_LANE1, mode);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DFE_MODE_LANE2, mode);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DFE_MODE_LANE3, mode);
}

static void hdmirx_damix_dfe_software_mode_start(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DFE_SW_START_LANE0, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DFE_SW_START_LANE1, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DFE_SW_START_LANE2, FALSE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DFE_SW_START_LANE3, FALSE);
	udelay(10); /* 10: delay 10 us */
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DFE_SW_START_LANE0, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DFE_SW_START_LANE1, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DFE_SW_START_LANE2, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DFE_SW_START_LANE3, TRUE);
}

void hdmirx_damix_set_handle_dfe_code(struct hdmirx_ctrl_st *hdmirx, uint8_t dfe_code)
{
	uint32_t gray_code, tmp_code;

	tmp_code = dfe_code;
	gray_code = (tmp_code >> 1) ^ tmp_code;

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DTAP_OVR_LANE0, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE0, CFG_DTAP_SW_LANE0, gray_code);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DTAP_OVR_LANE1, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE1, CFG_DTAP_SW_LANE1, gray_code);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DTAP_OVR_LANE2, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE2, CFG_DTAP_SW_LANE2, gray_code);

	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DTAP_OVR_LANE3, TRUE);
	hdmirx_reg_write(hdmirx->hdmirx_pwd_base, REG_DFE_CFG0_LANE3, CFG_DTAP_SW_LANE3, gray_code);
}

void hdmirx_damix_set_force_dfe(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type)
{
	switch (type) {
	case HDMIRX_INPUT_14:
	case HDMIRX_INPUT_20:
	case HDMIRX_INPUT_FRL3L3G:
	case HDMIRX_INPUT_FRL3L6G:
	case HDMIRX_INPUT_FRL4L6G:
	case HDMIRX_INPUT_FRL4L8G:
		/* in the case of HDMI14, HDMI20, FRL3L3G, FRL3L6G, FRL4L6G, FRL4L8G don't neet to set dfe */
		hdmirx_damix_set_handle_dfe_code(hdmirx, g_force_dfe[0]);
		break;
	case HDMIRX_INPUT_FRL4L10G:
		hdmirx_damix_set_handle_dfe_code(hdmirx, g_force_dfe[1]); /* 15: FRL 10G dfe code */
		break;
	case HDMIRX_INPUT_FRL4L12G:
		hdmirx_damix_set_handle_dfe_code(hdmirx, g_force_dfe[2]); /* 20: FRL 12G low dfe code */
		break;
	default:
		break;
	}
}

void hdmirx_damix_set_dfe_mode_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_dfe_mode mode)
{
	switch (mode) {
	case HDMIRX_DFE_MODE_HARDWARE:
		hdmirx_damix_set_forec_dfe_ttap(hdmirx, TRUE, g_ttap); /* 55: the dfe ttap value need to set */
		hdmirx_damix_set_dfe_en(hdmirx, TRUE);
		hdmirx_damix_dfe_mode_cfg(hdmirx, 0);
		hdmirx_damix_dfe_mode_cfg(hdmirx, 2); /* 2: logic control */
		break;
	case HDMIRX_DFE_MODE_SOFTWARE:
		hdmirx_damix_set_forec_dfe_ttap(hdmirx, TRUE, g_ttap); /* 55: the dfe ttap value need to set */

		hdmirx_damix_set_dfe_en(hdmirx, TRUE);
		hdmirx_damix_dfe_mode_cfg(hdmirx, 1);
		hdmirx_damix_dfe_software_mode_start(hdmirx);
		break;
	case HDMIRX_DFE_MODE_FORCE:
		hdmirx_damix_set_dfe_en(hdmirx, TRUE);
		hdmirx_damix_set_forec_dfe_ttap(hdmirx, FALSE, 0);
		hdmirx_damix_set_force_dfe(hdmirx, TRUE);
		break;
	default:
		hdmirx_damix_set_dfe_en(hdmirx, FALSE);
		hdmirx_damix_dfe_mode_cfg(hdmirx, 0);
		break;
	}
}

static bool phy_check_start_dfe_en(struct hdmirx_ctrl_st *hdmirx, hdmirx_input_type type, hdmirx_dfe_mode mode)
{
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx();
	if (hdmirx_damix_need_dfe(hdmirx, type) == TRUE) {
		hdmirx_damix_dfe_init(hdmirx, type);
		hdmirx_damix_set_dfe_mode_en(hdmirx, mode);
		phy_ctx->dfe_mode = mode;
		return TRUE;
	}
	phy_ctx->dfe_mode = HDMIRX_DFE_MODE_MAX;
	return FALSE;
}

static void phy_state_force_eq_set(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_input_type type;
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx();
	type = phy_ctx->type;
	hdmirx_damix_set_force_eq(hdmirx, type);
}

static void phy_state_auto_eq_proc(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t wait_time = 0;
	uint32_t repeat_cnt = 0;
	hdmirx_phy_ctx *phy_ctx = NULL;
	uint32_t delay_time;
	bool result;
	hdmirx_input_type type;

	phy_ctx = hdmirx_phy_get_ctx();
	type = phy_ctx->type;
	phy_get_time_cnt_info(type, &wait_time, &repeat_cnt);

	delay_time = phy_get_sleep_cnt();
	phy_ctx = hdmirx_phy_get_ctx();
	if (phy_ctx->wait_cnt < wait_time) {
		phy_ctx->wait_cnt += delay_time;
		result = hdmirx_damix_get_auto_eq_result(hdmirx, type);
		if (result == TRUE) {
			phy_ctx->eq_state = PHY_EQ_STATE_AUTO_DONE;
			if (type >= HDMIRX_INPUT_FRL3L3G) {
				disp_pr_info("g_frl_ready\n");
				g_frl_ready == true;
			}
			if (hdmirx_damix_need_dfe(hdmirx, type) == TRUE) {
				phy_ctx->dfe_state = PHY_DFE_STATE_CHECKING;
				phy_set_state(hdmirx, PHY_STATE_DFE);
				phy_check_start_dfe_en(hdmirx, type, HDMIRX_DFE_MODE_SOFTWARE);
			} else {
				phy_state_set_done(hdmirx);
			}
		}
		return;
	}
	phy_ctx->repeat_cnt++;
	if (phy_ctx->repeat_cnt < repeat_cnt) {
		disp_pr_info("time %u set auto eq fail,repeate agin!total:%u\n",
			phy_ctx->repeat_cnt, repeat_cnt);
		hdmirx_damix_set_auto_eq_en(hdmirx, type, TRUE);
		phy_ctx->wait_cnt = 0;
		return;
	}
	disp_pr_err("[HDMI] phy auto eq fail,set fixed eq!\n");
	phy_state_force_eq_set(hdmirx);
	/* no need to do hal_damix_set_force_eq(port, type); */
	phy_ctx->eq_state = PHY_EQ_STATE_AUTO_TIMEOUT;
	if (hdmirx_damix_need_dfe(hdmirx, type) == TRUE) {
		phy_ctx->dfe_state = PHY_DFE_STATE_CHECKING;
		phy_set_state(hdmirx, PHY_STATE_DFE);
		phy_check_start_dfe_en(hdmirx, type, HDMIRX_DFE_MODE_SOFTWARE);
	} else {
		phy_state_set_done(hdmirx);
	}
}

bool hdmirx_damix_get_dfe_result(struct hdmirx_ctrl_st *hdmirx)
{
	uint32_t lane0_dfe, lane1_dfe, lane2_dfe, lane3_dfe;

	lane0_dfe = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT14);
	lane1_dfe = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT17);
	lane2_dfe = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT20);
	lane3_dfe = hdmirx_reg_read(hdmirx->hdmirx_pwd_base, REG_DAMIX_INTR_STATE, DAMIX_INTR_STAT23);
	if ((lane0_dfe == FALSE) || (lane1_dfe == FALSE) ||
		(lane2_dfe == FALSE) || (lane3_dfe == FALSE)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

static void phy_state_dfe_proc(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	uint32_t delay_time;
	bool result;

	delay_time = phy_get_sleep_cnt();
	phy_ctx = hdmirx_phy_get_ctx();
	if (phy_ctx->wait_cnt < PHY_STATE_21_AUTO_DFE_TIME_CNT) {
		phy_ctx->wait_cnt += delay_time;
		result = hdmirx_damix_get_dfe_result(hdmirx);
		if (result == TRUE) {
			phy_ctx->dfe_state = PHY_DFE_STATE_DONE;
			disp_pr_info("[HDMI]==dfe done");
			phy_state_set_done(hdmirx);
		}
		return;
	}
	phy_ctx->dfe_state = PHY_DFE_STATE_TIMEOUT;
	disp_pr_info("[HDMI] dfe fail");
	if (phy_ctx->force_dfe == TRUE) {
		hdmirx_damix_set_handle_dfe_code(hdmirx, phy_ctx->dfe_code);
		disp_pr_info("froce dfe :%d\n", phy_ctx->dfe_code);
	} else {
		hdmirx_damix_set_force_dfe(hdmirx, phy_ctx->type);
	}
	phy_state_set_done(hdmirx);
}

void hdmirx_phy_proc(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx();
	if (phy_ctx->run == FALSE)
		return;

	switch (phy_ctx->state) {
	case PHY_STATE_OFF:
		phy_state_off_proc(hdmirx);
		break;
	case PHY_STATE_HANDLE_EQ:
		phy_state_handle_eq_proc(hdmirx);
		break;
	case PHY_STATE_AUTO_EQ:
		phy_state_auto_eq_proc(hdmirx);
		break;
	case PHY_STATE_DFE:
		phy_state_dfe_proc(hdmirx);
		break;
	default:
		break;
	}
}

int hdmirx_fpga_phy_clear(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx->input_type = HDMIRX_INPUT_OFF;
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();
	phy_ctx->fpga_phy_init = false;
	phy_ctx->fpga_phy_cnt = 0;
	phy_ctx->fpga_phy_status = 0;
	return 0;
}

void hdmirx_fpga_soft_reboot(struct hdmirx_ctrl_st *hdmirx)
{
	// FPGA soft reset
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x0020, 1, 1, 0);
	udelay(100);
	hdmirx_set_reg(hdmirx->hdmirx_pwd_base + 0x0020, 0, 1, 0);

	disp_pr_info("[hdmirx]hdmirx fpga soft reboot\n");
}

int hdmirx_fpga_phy_init(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;
	phy_ctx = hdmirx_phy_get_ctx();
	if (!(phy_ctx->fpga_phy_init)) {
		disp_pr_info("[hdmirx]init phy\n");
		hdmirx_phy_init(hdmirx);
		mdelay(4000);
		phy_ctx->fpga_phy_init = true;
	}

	if (phy_ctx->fpga_phy_status != 0x3f8) {
		phy_ctx->fpga_phy_cnt++;
		phy_ctx->fpga_phy_status = inp32(hdmirx->hdmirx_sysctrl_base + 0x084);
		if (phy_ctx->fpga_phy_status == 0x3f8) {
			hdmirx_fpga_soft_reboot(hdmirx);
			disp_pr_info("[hdmirx]phy init success. status 0x%x, irq mask\n", phy_ctx->fpga_phy_status);
			hdmirx_irq_enable(hdmirx);
		} else if (phy_ctx->fpga_phy_cnt >= 10) {
			phy_ctx->fpga_phy_cnt = 0;
			disp_pr_info("[hdmirx]phy init fail. %d, phy_status 0x%x\n", phy_ctx->fpga_phy_init, phy_ctx->fpga_phy_status);
		}
	}
	return 0;
}

phy_eq_state  hdmirx_phy_get_eq_state(struct hdmirx_ctrl_st *hdmirx)
{
	hdmirx_phy_ctx *phy_ctx = NULL;

	phy_ctx = hdmirx_phy_get_ctx(hdmirx);

	return phy_ctx->eq_state;
}
