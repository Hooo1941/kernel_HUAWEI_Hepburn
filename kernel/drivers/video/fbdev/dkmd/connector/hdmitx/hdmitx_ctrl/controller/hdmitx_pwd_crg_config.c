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

#include <linux/types.h>

#include "dkmd_log.h"
#include "hdmitx_ctrl_reg.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_pwd_crg_config.h"

struct hdmitx_crg_reg_config {
	u32 dsc_clk_sel;
	u32 vidpath_clk_sel;
	u32 vidpath_dout_clk_sel;
	u32 clk_tmds_source_sel;
	u32 audio_clk_gen_sel;
	u32 audpath_acr_clk_sel;
	u32 clk_frl_source_sel;
};

struct hdmitx_crg_reg_config g_hdmitx_crg_default_configs[] = {
	/*
	 * TMDS mode(0~600M):
	 * 1. Video path
	 */
	{
		.dsc_clk_sel = 0,
		.vidpath_clk_sel = 0,
		.vidpath_dout_clk_sel = 0,
		.clk_tmds_source_sel = 0,
		.audio_clk_gen_sel = 0,
		.audpath_acr_clk_sel = 1,
		.clk_frl_source_sel = 0
	},
	/*
	 * frl nodsc mode(0~2.4G):
	 * 1. videopath clock is from hpll 1d4
	 * 2. (1) phy ref clock is from hpll 1d4
	 *	(2) hdmi top clock is from phy(tmds_1d4 which matchs ref 1d4)
	 * 3. frl top clock from phy frl
	 * 4. audio default i2s and acr clock from frl.
	 * 5. phy clock is configured in the following phy_config.
	 * note1:
	 *   when pixel rate 0~600m and 600m~2.4g.
	 *   when final pixel format is 420(original is alway 444).
	 * the 1d4 divider is different.
	 * note2:
	 *	   we can choose ppll as the default clock source instead of phy.
	 *	   then, the clk_tmds_source_sel should be 1.
	 *	   and ppll should be configured as the 1d4 tmds clock which
	 *	   matches the 1d4 pixel clock.it also needs to consider the
	 *	   600~2.4g and 420.
	 */
	{
		.dsc_clk_sel = 0,
		.vidpath_clk_sel = 0,
		.vidpath_dout_clk_sel = 1,
		.clk_tmds_source_sel = 0,
		.audio_clk_gen_sel = 0,
		.audpath_acr_clk_sel = 0,
		.clk_frl_source_sel = 0
	},
	/*
	 * frl dsc mode(600M~2.4G, 2.4G+):
	 * 1. videopath clock is from ppll or phy default
	 * 2. (1) phy ref clock is from 1d8
	 *	(2) hdmi top clock is the same with videopath.
	 * 3. frl top clock from phy frl
	 * 4. audio default i2s and acr clock from frl.
	 * 5. phy clock is configured in the following phy_config.
	 * note1:
	 *  The ppll out or phy out should match the dsc compressed pixel
	 *  clock. No need to consider 10bit.
	 *  if the ppll is the source, then we have to config hdmi
	 *  FREQDIV and the ppll divider to get a match compress rate.
	 *  if the phy is the source, only need to config the phy tmds
	 *  divider.
	 */
	{
		.dsc_clk_sel = 0,
		.vidpath_clk_sel = 0,
		.vidpath_dout_clk_sel = 1,
		.clk_tmds_source_sel = 0,
		.audio_clk_gen_sel = 0,
		.audpath_acr_clk_sel = 0,
		.clk_frl_source_sel = 0
	}
};

static void crg_hw_config_pwd(void *hdmi_reg_base, const struct hdmitx_crg_reg_config *reg_config)
{
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_DSC_CLK_SEL_M,
		reg_dsc_clk_sel(reg_config->dsc_clk_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_VIDPATH_CLK_SEL_M,
		reg_vidpath_clk_sel(reg_config->vidpath_clk_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_VIDPATH_DOUT_CLK_SEL_M,
		reg_vidpath_dout_clk_sel(reg_config->vidpath_dout_clk_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_CLK_TMDS_SOURCE_SEL_M,
		reg_clk_tmds_source_sel(reg_config->clk_tmds_source_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_AUD_CLK_GEN_SEL_M,
		reg_aud_clk_gen_sel(reg_config->audio_clk_gen_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_AUDPATH_ACR_CLK_SEL_M,
		reg_audpath_acr_clk_sel(reg_config->audpath_acr_clk_sel));
	hdmi_clrset(hdmi_reg_base, REG_TX_PWD_RST_CTRL, REG_CLK_FRL_SOURCE_SEL_M,
		reg_clk_frl_source_sel(reg_config->clk_frl_source_sel));
}

void hal_crg_config(void *hdmi_reg_base, s32 encoder_mode)
{
	struct hdmitx_crg_reg_config reg_config;

	if (hdmi_reg_base == NULL) {
		dpu_pr_err("null pointer");
		return;
	}

	dpu_pr_info("hal_crg_config: encoder_mode = %d", encoder_mode);

	if (encoder_mode == HDMITX_WORK_MODE_FRL) { /* frl mode */
		reg_config = g_hdmitx_crg_default_configs[1];
	} else if (encoder_mode == HDMITX_WORK_MODE_TMDS) { /* 0~600M TMDS */
		reg_config = g_hdmitx_crg_default_configs[0];
	} else {
		dpu_pr_err("unsupported encoder_mode, FRL/TMDS is supported");
		return;
	}

	crg_hw_config_pwd(hdmi_reg_base, &reg_config);
}