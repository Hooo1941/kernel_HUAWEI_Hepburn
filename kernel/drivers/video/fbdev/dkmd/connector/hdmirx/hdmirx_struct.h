/* Copyright (c) 2022, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_HDMIRX_STRUCT_H
#define HISI_HDMIRX_STRUCT_H

#include <linux/platform_device.h>
#include <linux/types.h>
#include "chrdev/dkmd_chrdev.h"
#include "hdmirx_common.h"
#include "dkmd_rx.h"

enum {
	SCGT_HDMIMEDIA_PERI = 0,
	DIV_HDMIMEDIA_PERI  = 1,
	CLK_HDMI_MEDIA_PERI = 2,
	SC_GT_CLK_HDMI_AON  = 3,
	CLK_HDMI_AON_DIV    = 4,
	CLK_HDMI_AON        = 5,
	PCLK_HDMI           = 6,
	PCLK_HDMI_DJTAG     = 7,
	CLK_HDMI_AON_24M    = 8,
	SCGT_HDMI_MEDIA     = 9,
	CLK_HDMI_MEDIA_DIV  = 10,
	SEL_HDMI_MEDIA      = 11,
	CLK_HDMI_MEDIA      = 12,
	SC_GT_CLK_HDMI_LINK = 13,
	CLK_HDMI_LINK_DIV   = 14,
	CLK_HDMI_LINK       = 15,
	CLK_HDMIPHY_REF     = 16,
	CLK_HDMI_MAX_IDX    = 17,
};

struct hdmirx_ctrl_st {
	uint32_t hpd_gpio;
	uint32_t hpd_irq;

	uint32_t packet_irq;
	uint32_t hvchange_irq;

	// iomap
	void __iomem *hdmirx_aon_base; /* SOC_ACPU_HDMI_CTRL_AON_BASE_ADDR (0xEAB40000) */
	void __iomem *hdmirx_pwd_base; /* SOC_ACPU_HDMI_CTRL_PWD_BASE_ADDR (0xEAB00000) */
	void __iomem *hdmirx_sysctrl_base; /* SOC_ACPU_HDMI_CTRL_SYSCTRL_BASE_ADDR (0xEAB44000) */
	void __iomem *hdmirx_hsdt1_crg_base; /* SOC_ACPU_HSDT1_CRG_BASE_ADDR (0xEB045000) */
	void __iomem *hdmirx_ioc_base; /* SOC_ACPU_IOC_BASE_ADDR (0xFED02000) */
	void __iomem *hsdt1_sysctrl_base; /* SOC_ACPU_HSDT1_SCTRL_BASE_ADDR (0xEB040000) */
	void __iomem *hi_gpio14_base; /* SOC_ACPU_GPIO14_BASE_ADDR (0xFEC20000) */
	void __iomem *hi_gpio35_base; /* SOC_ACPU_GPIO35_BASE_ADDR (0xFA8B7000) */
	void __iomem *hdmirx_iocg_base; /* SOC_ACPU_IOC_BASE_ADDR (0xFED01000) */
	void __iomem *hdmirx_pericrg_base; /* (0xFFF05000) */

	void __iomem *hdmirx_gpio262_base_1; /* (0xFA8B4000) */
	void __iomem *hdmirx_gpio262_base_2; /* (0xFA89C000) */

	void __iomem *hdmirx_gpio010_base; /* (0xfec13000) */

	bool pwrstatus;
	hdmirx_input_type input_type;
	uint32_t fpga_flag;
	bool start_display_hdmi_ready;
	bool start_display_dss_ready;
	bool start_display;
	bool is_power_off;
	struct rx_interface interface_impl;

	struct clk *clk_hdmirx[CLK_HDMI_MAX_IDX];
	bool phy_done;
	uint32_t phy_cnt;
};

struct hdmirx_chr_dev_st {
	uint32_t index;
	struct platform_device *pdev;
	struct dkmd_chrdev chrdev;

	// hdmirx
	struct hdmirx_ctrl_st hdmirx_ctrl;
};

#endif

