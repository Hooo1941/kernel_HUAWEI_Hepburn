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

#ifndef __HDMITX_CTRL_DEV_H__
#define __HDMITX_CTRL_DEV_H__

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <platform_include/basicplatform/linux/switch.h>
#include "dpu_connector.h"
#include "hdmitx_ctrl.h"

#define VCO_MIN_FREQ_OUTPUT    800000000UL /* 800 * 1000 *1000 */
#define SYS_IN_FREQ            38400000UL /* 38.4 * 1000 * 1000 */
#define POSTDIV_ARRAY_SIZE     30

#if (CONFIG_DKMD_DPU_VERSION == 740 || CONFIG_DKMD_DPU_VERSION == 741)
#   if (!defined(CONFIG_PRODUCT_CDC) && !defined(CONFIG_PRODUCT_CDC_ACE))
void hdmitx_ctrl_default_setup(struct dpu_connector *connector);
#   else
static void hdmitx_ctrl_default_setup(struct dpu_connector *connector) {}
#   endif
#else
static void hdmitx_ctrl_default_setup(struct dpu_connector *connector) {}
#endif

void hdmitx_switch_set_state(struct switch_dev *sdev, int state);
int hdmitx_dpc_pll_init(struct hdmitx_ctrl *hdmitx);
void hdmitx_dpc_pll_deinit(struct hdmitx_ctrl *hdmitx);
#endif
