/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HDMIRX_INIT_H
#define HDMIRX_INIT_H

#include "hdmirx_struct.h"

#define HISI_HDMIRX_CHR_NAME    "hdmi_rx0"
#define HISI_CONNECTOR_HDMIRX_NAME    "hisi_disp_hdmi_rx"
#define DTS_NAME_HDMIRX0 "dkmd,dpu_hdmi_receiver0"

#define GPIO_NUM        8
#define GPI0_GROUP_14   14
#define GPI0_GROUP_35   35
#define GPI0_PIN_2      2
#define GPI0_PIN_7      7


enum {
	HDMIRX_AON_BASE_ID = 0,
	HDMIRX_PWD_BASE_ID,
	HDMIRX_SYSCTRL_BASE_ID,
	HDMIRX_HSDT1_CRG_BASE_ID,
	HDMIRX_HSDT1_SYSCTRL_BASE_ID,
	HDMIRX_IOC_BASE_ID,
	HDMIRX_HI_GPIO14_BASE_ID,
	HDMIRX_HI_GPIO35_BASE_ID,
	HDMIRX_IOCG_BASE_ID,
	HDMIRX_PERICRG_BASE_ID,
	HDMIRX_GPIO262_BASE_ID,
	HDMIRX_GPIO262_BASE_2_ID,
	HDMIRX_GPIO010_BASE_ID,
	HDMIRX_BASE_ADDR_MAX,
};
void hdmirx_display_hdmi_ready(struct hdmirx_ctrl_st *hdmirx);
uint32_t hdmirx_get_layer_fmt(void);
int hdmirx_irq_enable(struct hdmirx_ctrl_st *hdmirx);
#endif