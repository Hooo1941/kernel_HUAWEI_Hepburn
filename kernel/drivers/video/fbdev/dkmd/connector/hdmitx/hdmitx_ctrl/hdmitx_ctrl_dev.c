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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>

#include <platform_include/basicplatform/linux/switch.h>
#include <securec.h>
#include <platform_include/see/bl31_smc.h>
#include <platform_include/display/linux/dpu_drmdriver.h>

#include "dkmd_connector.h"
#include "dpu_connector.h"
#include "dpu_conn_mgr.h"

#include "hdmitx_drv.h"
#include "hdmitx_aon.h"
#include "hdmitx_ddc.h"
#include "hdmitx_connector.h"
#include "hdmitx_frl.h"
#include "hdmitx_phy.h"
#include "hdmitx_reg.h"
#include "hdmitx_ctrl_dev.h"
#include "hdmitx_core_config.h"
#include "hdmitx_avgen.h"

#define AON_BASE_OFFSET 0x44000
#define SYSCTRL_BASE_OFFSET 0x50000
#define TRAINING_BASE_OFFSET 0x20000
#define WORKQUEUE_NAME_SIZE 128
#define UEVENT_BUF_LEN 120

#define HDMI_INT_CTRL 0x30
#define HDMI_INTR_UNMASK GENMASK(8, 5)
#define HDMI_INTR_CTRL_ALL GENMASK(4, 0)

#define HDMI_INTR_VSTART BIT(1)
#define HDMI_INTR_VSYNC BIT(2)
#define HDMI_INTR_UNDERFLOW BIT(3)
#define HDMI_INTR_HDCP BIT(4)

#define HSDT1_TZPC_GRP0_LOCK_EN_OFFSET 0xc00
#define TZ_SECURE_N_HSDT1_CRG_S_SET_MASK BIT(1)
#define HSDT1_TZPC_LOCK_OFFSET 0xd00

#define DPC_PLL_STABLE_TIME 100
#define DPC_PLL_CONTINUE_LOCK_CNT 20

struct dpc_pll_cfg_param {
	uint64_t refdiv;
	uint64_t fbdiv;
	uint64_t frac;
	uint64_t postdiv1;
	uint64_t postdiv2;
	int post_div;
};

int hdmitx_ceil(uint64_t a, uint64_t b)
{
	if (b == 0)
		return -1;

	if (a % b != 0)
		return a / b + 1;

	return a / b;
}

static uint64_t hdmitx_dpc_pll_param_calc(uint64_t pixel_clock_cur, struct dpc_pll_cfg_param *cfg)
{
	int i, ceil_temp;
	uint64_t pll_freq_divider, vco_freq_output;
	int freq_divider_list[POSTDIV_ARRAY_SIZE] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		12, 14, 15, 16, 18, 20, 21, 24,
		25, 28, 30, 32, 35, 36, 40, 42,
		48, 49, 56, 64
	};
	int postdiv1_list[POSTDIV_ARRAY_SIZE] = {
		1, 2, 3, 4, 5, 6, 7, 8, 3, 5, 6,
		7, 5, 8, 6, 5, 7, 8, 5, 7, 6, 8,
		7, 6, 8, 7, 8, 7, 8, 8
	};
	int postdiv2_list[POSTDIV_ARRAY_SIZE] = {
		1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2,
		2, 3, 2, 3, 4, 3, 3, 5, 4, 5, 4,
		5, 6, 5, 6, 6, 7, 7, 8
	};

	/* Fractional PLL can not output the so small clock */
	cfg->post_div = 1;
	if (pixel_clock_cur * (uint64_t)freq_divider_list[POSTDIV_ARRAY_SIZE - 1] < VCO_MIN_FREQ_OUTPUT) {
		cfg->post_div = 2;
		pixel_clock_cur *= 2; /* multiple frequency */
	}
	dpu_pr_info("[HDMITX] post_div=0x%x", cfg->post_div);

	ceil_temp = hdmitx_ceil(VCO_MIN_FREQ_OUTPUT, pixel_clock_cur);
	if (ceil_temp < 0)
		return pixel_clock_cur;

	pll_freq_divider = (uint64_t)ceil_temp;
	for (i = 0; i < POSTDIV_ARRAY_SIZE; i++) {
		if (freq_divider_list[i] >= (int)pll_freq_divider) {
			pll_freq_divider = (uint64_t)freq_divider_list[i];
			cfg->postdiv1 = (uint64_t)postdiv1_list[i] - 1;
			cfg->postdiv2 = (uint64_t)postdiv2_list[i] - 1;
			dpu_pr_info("[HDMITX] postdiv1=0x%llx, postdiv2=0x%llx", cfg->postdiv1, cfg->postdiv2);
			break;
		}
	}

	vco_freq_output = pll_freq_divider * pixel_clock_cur;
	if (vco_freq_output == 0)
		return pixel_clock_cur;
	dpu_pr_info("[HDMITX] vco_freq_output=%llu", vco_freq_output);

	ceil_temp = hdmitx_ceil(VCO_MIN_FREQ_OUTPUT, vco_freq_output);
	if (ceil_temp < 0)
		return pixel_clock_cur;

	cfg->refdiv =  1;
	dpu_pr_info("[HDMITX] refdiv=0x%llx", cfg->refdiv);

	cfg->fbdiv = vco_freq_output * (uint64_t)ceil_temp * cfg->refdiv / SYS_IN_FREQ;
	dpu_pr_info("[HDMITX] fbdiv=0x%llx", cfg->fbdiv);

	cfg->frac = ((uint64_t)ceil_temp * vco_freq_output - SYS_IN_FREQ / \
		cfg->refdiv * cfg->fbdiv) * cfg->refdiv * 0x1000000; /* 0x1000000 is 2^24 */
	cfg->frac = (uint64_t)cfg->frac / SYS_IN_FREQ;

	dpu_pr_info("[HDMITX] frac=0x%llx", cfg->frac);

	return pixel_clock_cur;
}

static void hdmitx_dpc_pll_param_set(char __iomem *hsdt1_crg_base, struct dpc_pll_cfg_param *cfg)
{
	uint32_t dpc_pll_ctrl0, dpc_pll_ctrl1;
	uint32_t dpc_pll_ctrl0_val, dpc_pll_ctrl1_val;

	dpc_pll_ctrl0 = inp32(hsdt1_crg_base + DPC_PLL0_CTRL0);
	dpc_pll_ctrl0 &= ~DPC_PLL_FREQ_DEVIDER_MASK;

	dpc_pll_ctrl0_val = 0x0;
	dpc_pll_ctrl0_val |= (uint32_t)((cfg->postdiv2 << 23) | (cfg->postdiv1 << 20) | \
		(cfg->fbdiv << 8) | (cfg->refdiv << 2));
	dpc_pll_ctrl0_val &= DPC_PLL_FREQ_DEVIDER_MASK;

	dpc_pll_ctrl0 |= dpc_pll_ctrl0_val;
	outp32(hsdt1_crg_base + DPC_PLL0_CTRL0, dpc_pll_ctrl0);

	dpc_pll_ctrl1 = inp32(hsdt1_crg_base + DPC_PLL0_CTRL1);
	dpc_pll_ctrl1 &= ~DPC_PLL_FRAC_MODE_MASK;

	dpc_pll_ctrl1_val = 0x0;
	dpc_pll_ctrl1_val |= (uint32_t)(1 << 25 | 0 << 24 | cfg->frac);
	dpc_pll_ctrl1_val &= DPC_PLL_FRAC_MODE_MASK;

	dpc_pll_ctrl1 |= dpc_pll_ctrl1_val;
	outp32(hsdt1_crg_base + DPC_PLL0_CTRL1, dpc_pll_ctrl1);
	udelay(1);
}

static int32_t hdmitx_dpc_pll_select_and_config(char __iomem *hsdt1_crg_base, struct dpc_pll_cfg_param *cfg)
{
	uint32_t val;
	uint32_t delay_time = 0;
	uint32_t continue_lock_cnt = 0;
	// disable gate ~ gt_clk_dpc_pll0
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL1, GT_CLK_DPC_PLL_MASK, 0x0);

	// disable dpc_pll0_en
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_EN_MASK, 0x0);

	// enable bypass
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_BP_MASK, 0x1);

	// select dpc_pll
	hdmi_write_bits(hsdt1_crg_base, CLKDIV11, SEL_PLL_HDMI0_MASK, 0x1800000);

	// config postdiv
	val = (((uint32_t)cfg->post_div - 1) | (0x3f << 16));
	hdmi_write_bits(hsdt1_crg_base, CLKDIV11, DIV_PLL_HDMI0_MASK, val);
	dpu_pr_info("[HDMI]CLKDIV11 value is 0x%x", inp32(hsdt1_crg_base + CLKDIV11));

	hdmitx_dpc_pll_param_set(hsdt1_crg_base, cfg);

	// enable dpc_pll0_en
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_EN_MASK, 0x1);

	// enable bypass
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_BP_MASK, 0x1);
	udelay(20);

	// wait for dpc pll locked
	do {
		delay_time++;
		val = hdmi_readl(hsdt1_crg_base, DPC_PLL0_CTRL0);
		if ((val & DPC_PLL_LOCK_MASK) == DPC_PLL_LOCK_MASK) {
			continue_lock_cnt++;
		} else {
			continue_lock_cnt = 0;
		}
		udelay(1);
		if (delay_time > DPC_PLL_STABLE_TIME) {
			dpu_pr_err("[HDMI]dpc pll lock timeout");
			return -1;
		}
	} while (continue_lock_cnt < DPC_PLL_CONTINUE_LOCK_CNT);

	dpu_pr_info("[HDMI]dpc pll lock done!");

	// enable dpc_pll0_en
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_EN_MASK, 0x1);

	// disable bypass
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_BP_MASK, 0x0);

	// enable gate ~ gt_clk_dpc_pll0
	hdmi_write_bits(hsdt1_crg_base, DPC_PLL0_CTRL1, GT_CLK_DPC_PLL_MASK, 0x1);

	// enable gate ~ sc_gt_clk_pll_hdmi0
	hdmi_write_bits(hsdt1_crg_base, CLKDIV11, SC_GT_CLK_PLL_HDMI0_MASK, 0x400000);
	return 0;
}

int32_t hdmitx_dpc_pll_init(struct hdmitx_ctrl *hdmitx)
{
	int32_t ret = 0;
	uint64_t pixel_clock;
	uint64_t pixel_clock_cur;
	struct dpc_pll_cfg_param cfg;
	struct hdmitx_private *hdmitx_priv = NULL;

	dpu_check_and_return(!hdmitx, -1, err, "null pointer\n");

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	// pixel_clock need compute according to frl or tmds, if frl need divided 2
	pixel_clock = hdmitx->connector->ldi.pxl_clk_rate;
	if (hdmitx->mode == HDMITX_WORK_MODE_FRL)
		pixel_clock = pixel_clock / 2;

	pixel_clock_cur = hdmitx_dpc_pll_param_calc(pixel_clock, &cfg);

	// config dpc pll
	ret = hdmitx_dpc_pll_select_and_config(hdmitx_priv->hsdt1_crg_base, &cfg);
	dpu_pr_info("clk_hdmictrl_pixel:ori[%llu]->[%llu]", pixel_clock, pixel_clock_cur);

	return ret;
}

void hdmitx_dpc_pll_deinit(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_no_retval(!hdmitx, err, "null pointer\n");
	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	// disable dpc_pll0_en
	hdmi_write_bits(hdmitx_priv->hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_EN_MASK, 0x0);

	// enable bypass
	hdmi_write_bits(hdmitx_priv->hsdt1_crg_base, DPC_PLL0_CTRL0, DPC_PLL_BP_MASK, 0x1);

	// disable gate ~ gt_clk_dpc_pll0
	hdmi_write_bits(hdmitx_priv->hsdt1_crg_base, DPC_PLL0_CTRL1, GT_CLK_DPC_PLL_MASK, 0x0);
}

void hdmitx_switch_set_state(struct switch_dev *sdev, int state)
{
	char *envp[3];
	char name_buf[UEVENT_BUF_LEN];
	char state_buf[UEVENT_BUF_LEN];
	int length;
	dpu_pr_info("hdmitx_switch_set_state +");
	if (sdev == NULL) {
		dpu_pr_err("sdev is NULL");
		return;
	}
	if (strlen(sdev->name) >= UEVENT_BUF_LEN) {
		dpu_pr_err("sdev->name size %u over buf max size %u", strlen(sdev->name), UEVENT_BUF_LEN);
		return;
	}

	length = snprintf_s(name_buf, sizeof(name_buf), sizeof(name_buf) - 1, "SWITCH_NAME=%s\n", sdev->name);
	if (length > 0) {
		if (name_buf[length - 1] == '\n')
			name_buf[length - 1] = 0;
	}

	length = snprintf_s(state_buf, sizeof(state_buf), sizeof(state_buf) - 1, "SWITCH_STATE=%d\n", state);
	if (length > 0) {
		if (state_buf[length - 1] == '\n')
			state_buf[length - 1] = 0;
	}

	envp[0] = name_buf;
	envp[1] = state_buf;
	envp[2] = NULL;
	kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);
	dpu_pr_info("hdmitx_switch_set_state -");
}

static irqreturn_t dpu_hdmitx_isr(int32_t irq, void *ptr)
{
	uint32_t intr_status;
	struct dpu_connector *connector = NULL;
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)ptr;
	struct hdmitx_private *hdmitx_priv = NULL;
	struct hdmitx_ctrl *hdmitx = NULL;

	dpu_check_and_return(!isr_ctrl, IRQ_NONE, err, "[HDMI] isr_ctrl is null pointer");
	connector = (struct dpu_connector *)isr_ctrl->parent;
	hdmitx_priv = to_hdmitx_private(connector->conn_info);
	dpu_check_and_return(!hdmitx_priv, IRQ_NONE, err, "[HDMI] hdmitx_priv is null pointer");
	hdmitx = &hdmitx_priv->hdmitx;

	intr_status = inp32(hdmitx->sysctrl_base + HDMI_INT_CTRL);
	outp32(hdmitx->sysctrl_base + HDMI_INT_CTRL, intr_status);

	if (intr_status & HDMI_INTR_VSYNC) {
		dpu_pr_debug("vsync intr");
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VSYNC);
	}

	if (intr_status & HDMI_INTR_UNDERFLOW) {
		dpu_pr_err("HDMI underflow");
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_UNDER_FLOW);
	}

	if (intr_status & HDMI_INTR_VSTART) {
		dpu_pr_debug("vstart intr");
		dkmd_isr_notify_listener(isr_ctrl, DSI_INT_VACT0_START);
	}

	if (intr_status & HDMI_INTR_HDCP) {
		dpu_pr_info("random request update");
		queue_work(hdmitx->hdmitx_wq, &hdmitx->hdcp_random_work);
	}

	return IRQ_HANDLED;
}

static void hdmitx_ctrl_isr_handle(struct dpu_connector *connector, struct dkmd_isr *isr_ctrl, bool enable)
{
	struct hdmitx_private *hdmitx_priv = to_hdmitx_private(connector->conn_info);
	struct hdmitx_ctrl *hdmitx = &hdmitx_priv->hdmitx;
	uint32_t mask = 0x0;

	mutex_lock(&hdmitx->hdmitx_mutex);
	if (!hdmitx->video_transfer_enable) {
		dpu_pr_info("[HDMI] hdmitx has already off");
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);
		mutex_unlock(&hdmitx->hdmitx_mutex);
		return;
	}
	/* 1. interrupt mask */
	outp32(hdmitx->sysctrl_base + HDMI_INT_CTRL, mask);

	if (enable) {
		/* 2. enable irq */
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_ENABLE);

		/* 3. interrupt clear */
		outp32(hdmitx->sysctrl_base + HDMI_INT_CTRL, HDMI_INTR_CTRL_ALL);

		/* 4. interrupt umask */
		outp32(hdmitx->sysctrl_base + HDMI_INT_CTRL, HDMI_INTR_UNMASK);
	} else {
		/* 2. disable irq */
		isr_ctrl->handle_func(isr_ctrl, DKMD_ISR_DISABLE);

		/* 3. interrupt clear  */
		outp32(hdmitx->sysctrl_base + HDMI_INT_CTRL, HDMI_INTR_CTRL_ALL);
	}
	mutex_unlock(&hdmitx->hdmitx_mutex);
}

static int32_t hdmitx_ctrl_isr_enable(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;
	dpu_pr_info("+");
	hdmitx_ctrl_isr_handle(connector, isr_ctrl, true);

	return 0;
}

static int32_t hdmitx_ctrl_isr_disable(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;
	dpu_pr_info("+");
	hdmitx_ctrl_isr_handle(connector, isr_ctrl, false);

	return 0;
}

static int32_t hdmitx_ctrl_isr_setup(struct dpu_connector *connector, const void *value)
{
	struct dkmd_isr *isr_ctrl = (struct dkmd_isr *)value;

	if (unlikely((!connector) || (!value))) {
		dpu_pr_err("connector or value is null\n");
		return -1;
	}

	dpu_pr_info("+");
	(void)snprintf_s(isr_ctrl->irq_name, sizeof(isr_ctrl->irq_name),
		ISR_NAME_SIZE - 1, "irq_hdmi_%u", connector->connector_id);
	isr_ctrl->irq_no = connector->connector_irq;
	isr_ctrl->isr_fnc = dpu_hdmitx_isr;
	isr_ctrl->parent = connector;
	/* fake int mask for hdmitx */
	isr_ctrl->unmask = ~(DSI_INT_VSYNC | DSI_INT_VACT0_START | DSI_INT_UNDER_FLOW);

	return 0;
}

static void enable_hdmitx_timing_gen(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);
	struct hdmitx_private *hdmitx_priv = to_hdmitx_private(connector->conn_info);
	struct hdmitx_ctrl *hdmitx = &hdmitx_priv->hdmitx;
	dpu_pr_info("+");

	if (hdmitx->video_transfer_enable)
		hdmitx_timing_gen_enable(hdmitx);
}

static void disable_hdmitx_timing_gen(struct dkmd_connector_info *pinfo)
{
	struct dpu_connector *connector = get_primary_connector(pinfo);
	struct hdmitx_private *hdmitx_priv = to_hdmitx_private(connector->conn_info);
	struct hdmitx_ctrl *hdmitx = &hdmitx_priv->hdmitx;
	if (hdmitx->video_transfer_enable)
		hdmitx_timing_gen_disable(hdmitx);
}

static int hdmitx_ctrl_on(struct dkmd_connector_info *pinfo)
{
	dpu_pr_info("[HDMI] +");
	return 0;
}

static int hdmitx_ctrl_off(struct dkmd_connector_info *pinfo)
{
	dpu_pr_info("[HDMI] +\n");
	disable_hdmitx_timing_gen(pinfo);
	return 0;
}

static int32_t hdmitx_ctrl_hdmitx_reset(struct dpu_connector *connector, const void *value)
{
	struct hdmitx_private *hdmitx_priv = to_hdmitx_private(connector->conn_info);
	struct hdmitx_ctrl *hdmitx = &hdmitx_priv->hdmitx;

	dpu_pr_info("[HDMI] clear enter");

	mutex_lock(&hdmitx->hdmitx_mutex);
	if (!hdmitx->video_transfer_enable) {
		dpu_pr_info("[HDMITX] hdmitx has already off");
		hdmitx->hdmitx_underflow_clear = false;
		mutex_unlock(&hdmitx->hdmitx_mutex);
		return 0;
	}
	msleep(100);
	enable_hdmitx_timing_gen(connector->conn_info);
	hdmitx->hdmitx_underflow_clear = false;
	mutex_unlock(&hdmitx->hdmitx_mutex);

	dpu_pr_info("[HDMI] clear exit");

	return 0;
}

static struct connector_ops_handle_data hdmitx_ops_table[] = {
	{ SETUP_ISR, hdmitx_ctrl_isr_setup },
	{ ENABLE_ISR, hdmitx_ctrl_isr_enable },
	{ DISABLE_ISR, hdmitx_ctrl_isr_disable },
	{ DO_CLEAR, hdmitx_ctrl_hdmitx_reset },
};

static int32_t hdmitx_ctrl_ops_handle(struct dkmd_connector_info *pinfo, uint32_t ops_cmd_id, void *value)
{
	struct dpu_connector *connector = NULL;
	dpu_check_and_return(!pinfo, -EINVAL, err, "pinfo is NULL!");

	connector = get_primary_connector(pinfo);
	dpu_check_and_return(!connector, -EINVAL, err, "connector is NULL!");

	return dkdm_connector_hanlde_func(hdmitx_ops_table, ARRAY_SIZE(hdmitx_ops_table), ops_cmd_id, connector, value);
}

static void hdmitx_ctrl_ldi_setup(struct dpu_connector *connector)
{
	struct dkmd_connector_info *pinfo = connector->conn_info;

	/* set default ldi parameter */
	pinfo->base.width = 16000;
	pinfo->base.height = 9000;
	pinfo->base.fps = 60;
	pinfo->base.is_pluggable = 1;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	if (pinfo->base.fpga_flag == 1) {
		pinfo->base.xres = 1280;
		pinfo->base.yres = 720;

		connector->ldi.h_back_porch = 220;
		connector->ldi.h_front_porch = 110;
		connector->ldi.h_pulse_width = 40;
		connector->ldi.hsync_plr = 1;

		connector->ldi.v_back_porch = 20;
		connector->ldi.v_front_porch = 30;
		connector->ldi.v_pulse_width = 5;
		connector->ldi.vsync_plr = 1;

		connector->ldi.pxl_clk_rate = 27000000UL;
		connector->ldi.pxl_clk_rate_div = 1;
	} else {
		pinfo->base.xres = 1920;
		pinfo->base.yres = 1080;

		connector->ldi.h_back_porch = 148;
		connector->ldi.h_front_porch = 88;
		connector->ldi.h_pulse_width = 44;
		connector->ldi.hsync_plr = 1;

		connector->ldi.v_back_porch = 36;
		connector->ldi.v_front_porch = 4;
		connector->ldi.v_pulse_width = 5;
		connector->ldi.vsync_plr = 1;

		connector->ldi.pxl_clk_rate = 148500000UL;
		connector->ldi.pxl_clk_rate_div = 1;
	}
}

static void hdmitx_ctrl_clk_setup(struct dpu_connector *connector)
{
	int ret = 0;

	ret = clk_set_rate(connector->connector_clk[CLK_DPCTRL_PIXEL], 24000000UL);
	if (ret < 0)
		dpu_pr_err("[HDMITX] CLK_DPCTRL_PIXEL clk_set_rate failed, error=%d", ret);

	// disable dpc pll register secure access, modify to non-secure for kenel to config
	configure_dss_service_security(FID_BL31_DISPLAY_HSDT_PLL_NS_CONFIG, 0, 0, 0);
}

void hdmitx_hdcp_random_handle(struct work_struct *work)
{
	struct hdmitx_ctrl *hdmitx = NULL;

	hdmitx = container_of(work, struct hdmitx_ctrl, hdcp_random_work);
	hdmitx_switch_set_state(&hdmitx->sdev, HOT_PLUG_HDCP_RANDOM);
}

static void hdmitx_wq_init(struct hdmitx_ctrl *hdmitx)
{
	char wq_name[WORKQUEUE_NAME_SIZE] = {0};

	(void)snprintf_s(wq_name, WORKQUEUE_NAME_SIZE, WORKQUEUE_NAME_SIZE - 1, "hdmitx_wq");

	hdmitx->hdmitx_wq = create_singlethread_workqueue(wq_name);
	if (hdmitx->hdmitx_wq == NULL)
		dpu_pr_err("hdmitx workqueue failed");
	INIT_WORK(&hdmitx->hdcp_random_work, hdmitx_hdcp_random_handle);
}

static void hdmitx_resource_init(struct dpu_connector *connector, struct hdmitx_ctrl *hdmitx)
{
	hdmitx->base = connector->connector_base;
	hdmitx->aon_base = hdmitx->base + AON_BASE_OFFSET;
	hdmitx->sysctrl_base = hdmitx->base + SYSCTRL_BASE_OFFSET;
	hdmitx->training_base = hdmitx->base + TRAINING_BASE_OFFSET;
}

void hdmitx_ctrl_default_setup(struct dpu_connector *connector)
{
	int ret = 0;
	struct hdmitx_private *hdmitx_priv = NULL;
	struct hdmitx_ctrl *hdmitx = NULL;

	dpu_check_and_no_retval(!connector, err, "null pointer\n");

	hdmitx_priv = to_hdmitx_private(connector->conn_info);
	hdmitx = &hdmitx_priv->hdmitx;
	hdmitx->connector = connector;
	hdmitx->irq = (uint32_t)(connector->connector_irq);
	connector->connector_base = hdmitx_priv->hdmitx_base;

	hdmitx_resource_init(connector, hdmitx);
	hdmitx_wq_init(hdmitx);
	hdmitx_ctrl_ldi_setup(connector);
	hdmitx_ctrl_clk_setup(connector);

	ret = hdmitx_hpd_gpio_setup(hdmitx_priv);
	if(ret)
		dpu_pr_err("gpio setup failed, ret = %d", ret);

	hdmitx_ddc_init(hdmitx);
	hdmitx_connector_init(hdmitx);
	hdmitx_phy_init(hdmitx);

	connector->on_func = hdmitx_ctrl_on;
	connector->off_func = hdmitx_ctrl_off;
	connector->ops_handle_func = hdmitx_ctrl_ops_handle;
	connector->conn_info->enable_ldi = enable_hdmitx_timing_gen;
	connector->conn_info->disable_ldi = disable_hdmitx_timing_gen;
}
