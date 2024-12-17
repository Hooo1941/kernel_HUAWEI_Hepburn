/**
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
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <securec.h>

#include "dkmd_log.h"
#include "dkmd_object.h"
#include "hdmitx_phy.h"
#include "hdmitx_common.h"
#include "hdmitx_frl.h"
#include "hdmitx_pwd_crg_config.h"
#include "dpu_connector.h"

#define FPGA_PHY_CTRL0 0x84
#define HDMI_PHY_MASK 0x1F77
#define HDMITX_PHY_NUM 2
#define HDMITX_PHY_REG_BASE 0x80000
#define FPGA_TMDS_FRL_SEL 0x98

struct hdmitx_phy *hdmitx_phy_init(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_phy *phy = NULL;

	phy = (struct hdmitx_phy *)kmalloc(sizeof(struct hdmitx_phy), GFP_KERNEL);
	if (phy == NULL) {
		dpu_pr_err("kmalloc fail");
		return NULL;
	}

	if (memset_s(phy, sizeof(*phy), 0, sizeof(*phy)) != EOK) {
		dpu_pr_err("memset_s fail");
		goto phy_init_failed;
	}

	phy->regs = hdmitx->base + HDMITX_PHY_REG_BASE;
	phy->crg_regs = hdmitx->sysctrl_base;
	hdmitx->phy = phy;
	dpu_pr_info("phy%u init", phy->phy_id);
	return phy;

phy_init_failed:
	kfree(phy);
	phy = NULL;
	return NULL;
}

void drv_hdmitx_phy_deinit(struct hdmitx_phy *phy)
{
	if (phy == NULL)
		return;

#ifndef CONFIG_SOCT_FPGA_SUPPORT
	hal_phy_reset(phy);
#endif
	dpu_pr_debug("phy deinit");
	kfree(phy);
	phy = NULL;
}

void drv_hdmitx_phy_on(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if (phy == NULL) {
		dpu_pr_err("null ptr");
		return;
	}

	if (hal_phy_is_on(phy))
		return;

	hal_phy_hw_oe_enable(phy, true);
	dpu_pr_debug("phy on");
#endif
}

void drv_hdmitx_phy_off(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if (phy == NULL) {
		dpu_pr_err("null ptr");
		return;
	}

	if (!hal_phy_is_on(phy))
		return;

	hal_phy_hw_oe_enable(phy, false);
	dpu_pr_debug("phy off");
#endif
}

void drv_hdmitx_phy_fcg_set(const struct hdmitx_ctrl *hdmitx)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	bool dsc_enable; // need fix accord dsc enable or disable
	struct phy_frl_tmds_clk frl_tmds_clk;

	if (hdmitx == NULL) {
		dpu_pr_err("hdmitx_ctrl ptr is null");
		return;
	}

	dsc_enable = false;
	frl_tmds_clk.pixel_clk =  hdmitx->select_mode.detail.pixel_clock;
	frl_tmds_clk.rate = hdmitx->frl->scdc.frl_rate;
	frl_tmds_clk.color_depth = (u32)g_hdmitx_color_depth_debug; // need fix accord real bpc
	if (dsc_enable)
		frl_tmds_clk.color_depth = HDMITX_BPC_24;

	drv_hdmitx_phy_configure_frl_tmds_clk(hdmitx->phy, &frl_tmds_clk);
#endif
	return;
}

void drv_hdmitx_crg_set(const struct hdmitx_ctrl *hdmitx)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	s32 encoder_mode;

	if (hdmitx == NULL) {
		dpu_pr_err("hdmitx_ctrl ptr is null");
		return;
	}

	encoder_mode = hdmitx->mode ? 1 : 0;
	hal_crg_config(hdmitx->base, encoder_mode);
#endif
}

void drv_hdmitx_phy_configure_tmds(struct hdmitx_phy *phy, const struct phy_tmds *tmds)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if ((phy == NULL) || (tmds == NULL)) {
		dpu_pr_err("null ptr!");
		return;
	}

	if (hal_phy_is_on(phy))
		dpu_pr_debug("phy is on");

	hal_phy_tmds_set(phy, tmds);
#endif
}

void drv_hdmitx_phy_configure_frl(struct hdmitx_phy *phy, const struct phy_frl *frl)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if ((phy == NULL) || (frl == NULL)) {
		dpu_pr_err("null ptr!");
		return;
	}

	if (hal_phy_is_on(phy))
		dpu_pr_debug("phy is on");

	hal_phy_frl_set(phy, frl);
#endif
}

void drv_hdmitx_phy_configure_frl_tmds_clk(const struct hdmitx_phy *phy,
	const struct phy_frl_tmds_clk *frl_tmds_clk)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if ((phy == NULL) || (frl_tmds_clk == NULL)) {
		dpu_pr_err("null ptr!");
		return;
	}

	if (hal_phy_is_on(phy))
		dpu_pr_debug("phy is on");

	hal_phy_frl_tmds_clock_set(phy, frl_tmds_clk);
#endif
}

// add for fpga phy configure
int drv_hdmitx_phy_configure_frl_fpga(struct hdmitx_ctrl *hdmitx)
{
	uint32_t val;
	uint32_t mask = HDMI_PHY_MASK;
	uint32_t cnt = 0;

	dpu_check_and_return(!hdmitx, -EINVAL, err, "[HDMI] hdmitx is NULL");

	/* FPGA PHY init process */
	hdmi_writel(hdmitx->sysctrl_base, FPGA_TMDS_FRL_SEL, 0x1);
	hdmi_writel(hdmitx->sysctrl_base, FPGA_PHY_CTRL0, 0x001FC000);
	hdmi_writel(hdmitx->sysctrl_base, FPGA_PHY_CTRL0, 0x0);

	do {
		val = hdmi_readl(hdmitx->sysctrl_base, FPGA_PHY_CTRL0);
		if ((val & mask) == mask) {
			dpu_pr_info("FPGA PHY init OK");
			return 0;
		}
		cnt++;
		dpu_pr_err("try cnt %u, val 0x%x", cnt, val);
		msleep(1000);
	} while (cnt <= 10);

	dpu_pr_err("FPGA PHY init timed out");
	return -1;
}

void drv_hdmitx_phy_configure_ssc(const struct hdmitx_phy *phy, const struct phy_ssc *ssc)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if ((phy == NULL) || (ssc == NULL)) {
		dpu_pr_err("null ptr!");
		return;
	}

	if (hal_phy_is_on(phy))
		dpu_pr_debug("phy is on");

	hal_phy_ssc_set(phy, ssc);
#endif
}

void drv_hdmitx_phy_reset(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	if (phy == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	hal_phy_reset(phy);
#endif
}

u32 drv_hdmitx_phy_get_pixel_clk(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	return hal_phy_get_pixel_clk(phy);
#else
	return 0;
#endif
}

u32 drv_hdmitx_phy_get_tmds_clk(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	return hal_phy_get_tmds_clk(phy);
#else
	return 0;
#endif
}

u32 drv_hdmitx_phy_get_frl_clk(const struct hdmitx_phy *phy)
{
#ifndef CONFIG_SOCT_FPGA_SUPPORT
	return hal_phy_get_frl_clk(phy);
#else
	return 0;
#endif
}
