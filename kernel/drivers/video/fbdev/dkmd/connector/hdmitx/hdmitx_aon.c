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
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "dkmd_log.h"
#include "dkmd_object.h"
#include "hdmitx_drv.h"
#include "hdmitx_connector.h"
#include "hdmitx_core_config.h"
#include "dpu_connector.h"

#define AON_INTR_MASK         0x30
#define AON_INTR_STATE        0x34
#define AON_STATE             0x28
#define HDMI_APB_PERRSTDIS0   0x64
#define HDMI_APB_PERRSTEN0    0x60
#define HDMI_PCLK_PERDIS0     0x4
#define HDMI_PCLK_PEREN0      0x0
#define HDMI_CRG_PERRSTDIS3   0x88
#define HDMI_CRG_PERRSTEN3    0x84

struct hdmitx_ctrl *g_hdmitx = NULL;

static irqreturn_t hdmi_hpd_gpio_thread_irq(int32_t irq, void *ptr)
{
	bool isr_status;
	uint32_t is_pluged = 0;
	struct hdmitx_ctrl *hdmitx = NULL;
	struct hdmitx_private *hdmitx_priv = NULL;

	hdmitx = (struct hdmitx_ctrl *)ptr;
	dpu_check_and_return(!hdmitx, IRQ_NONE, err, "hdmitx is null!");

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	dpu_check_and_return(!hdmitx_priv, IRQ_NONE, err, "hdmitx_priv is null!");

	is_pluged = (uint32_t)gpio_get_value(hdmitx_priv->hpd_gpio);
	dpu_pr_info("[HDMITX] is_pluged %d", is_pluged);

	if (is_pluged == 1) {
		mdelay(100);
		is_pluged = (uint32_t)gpio_get_value(hdmitx_priv->hpd_gpio);
		if (is_pluged != 1) {
			dpu_pr_info("[HDMITX] ignore the irq is_pluged %u", is_pluged);
			return IRQ_NONE;
		}
		dpu_pr_info("[HDMITX] hotplug in!");
		isr_status = true;
	} else {
		mdelay(100);
		is_pluged = (uint32_t)gpio_get_value(hdmitx_priv->hpd_gpio);
		if (is_pluged != 0) {
			dpu_pr_info("[HDMITX] ignore the irq is_pluged %u", is_pluged);
			return IRQ_NONE;
		}
		dpu_pr_info("[HDMITX] hotplug out!");
		isr_status = false;
	}

	mutex_lock(&hdmitx->hdmitx_mutex);
	if (isr_status)
		hdmitx_hotplug();
	else
		hdmitx_hotunplug();
	mutex_unlock(&hdmitx->hdmitx_mutex);
	return IRQ_NONE;
}

int32_t hdmitx_hpd_gpio_setup(struct hdmitx_private *hdmitx_priv)
{
	int32_t ret;
	mutex_init(&hdmitx_priv->hdmitx.hdmitx_mutex);
	g_hdmitx = &hdmitx_priv->hdmitx;

	dpu_pr_info("hpd_gpio=%#x,+", hdmitx_priv->hpd_gpio);

	ret = gpio_request(hdmitx_priv->hpd_gpio, "hpd_gpio");
	if (ret) {
		dpu_pr_err("[HDMITX] Fail[%d] request gpio:%d", ret, hdmitx_priv->hpd_gpio);
		return ret;
	}

	ret = gpio_direction_input(hdmitx_priv->hpd_gpio);
	if (ret < 0) {
		gpio_free(hdmitx_priv->hpd_gpio);
		dpu_pr_err("[HDMITX] Failed to set gpio direction");
		return ret;
	}

	hdmitx_priv->hpd_irq_no = (uint32_t)gpio_to_irq(hdmitx_priv->hpd_gpio);
	if (hdmitx_priv->hpd_irq_no < 0) {
		gpio_free(hdmitx_priv->hpd_gpio);
		dpu_pr_err("[HDMITX] Failed to get dp_cable_det_gpio irq");
		return -EINVAL;
	}

	ret = request_threaded_irq(hdmitx_priv->hpd_irq_no, NULL, hdmi_hpd_gpio_thread_irq, IRQF_TRIGGER_RISING |
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "hdmitx_hpd_gpio", (void *)&hdmitx_priv->hdmitx);
	if (ret) {
		gpio_free(hdmitx_priv->hpd_gpio);
		dpu_pr_err("Failed to request press interupt handler!");
		return ret;
	}

	/* default enable irq */
	disable_irq(hdmitx_priv->hpd_irq_no);
	dpu_pr_info("-");
	return ret;
}

int hdmitx_aon_init(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_return(!hdmitx, -1, err, "hdmitx is null!");
	dpu_pr_info("+");

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	dpu_check_and_return(!hdmitx_priv, -1, err, "hdmitx_priv is null!");

	//  apb and hdmi reset
	set_reg(hdmitx_priv->hsdt1_sub_harden_base + HDMI_APB_PERRSTDIS0, 0x3, 2, 11);

	//  hsdt1 crg reset
	set_reg(hdmitx_priv->hsdt1_crg_base + HDMI_CRG_PERRSTDIS3, 0x1, 1, 28);

	// pclk enable
	set_reg(hdmitx_priv->hsdt1_sub_harden_base + HDMI_PCLK_PEREN0, 0x1, 1, 15);

	//  AON 24MHz clock, use clk_prepare_enable enable 24M clock
	clk_prepare_enable(hdmitx->connector->connector_clk[CLK_DPCTRL_PIXEL]);
	dpu_pr_info("-");
	return 0;
}

int hdmitx_aon_deinit(struct hdmitx_ctrl *hdmitx)
{
	struct hdmitx_private *hdmitx_priv = NULL;
	dpu_check_and_return(!hdmitx, -1, err, "hdmitx is null!");

	hdmitx_priv = to_hdmitx_private(hdmitx->connector->conn_info);
	dpu_check_and_return(!hdmitx_priv, -1, err, "hdmitx_priv is null!");

	//  AON 24MHz clock, use clk_disable_unprepare disable 24M clock
	clk_disable_unprepare(hdmitx->connector->connector_clk[CLK_DPCTRL_PIXEL]);

	// pclk disable
	set_reg(hdmitx_priv->hsdt1_sub_harden_base + HDMI_PCLK_PERDIS0, 0x1, 1, 15);

	//  hsdt1 crg set
	set_reg(hdmitx_priv->hsdt1_crg_base + HDMI_CRG_PERRSTEN3, 0x1, 1, 28);

	//  apb and hdmi set
	set_reg(hdmitx_priv->hsdt1_sub_harden_base + HDMI_APB_PERRSTEN0, 0x3, 2, 11);
	return 0;
}