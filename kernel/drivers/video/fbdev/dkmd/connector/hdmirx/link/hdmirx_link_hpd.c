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

#include "hdmirx_link_hpd.h"
#include "hdmirx_common.h"

int hdmirx_link_set_power(struct hdmirx_ctrl_st *hdmirx, bool is_power_on)
{
	if (is_power_on) {
		disp_pr_info("powr on");
		hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x014, 1, 1, 0); // should follow powr 5v
	} else {
		disp_pr_info("powr off");
		hdmirx_set_reg(hdmirx->hdmirx_sysctrl_base + 0x014, 0, 1, 0); // should follow powr 5v
	}

	return 0;
}

int hdmirx_link_set_hpd_level(struct hdmirx_ctrl_st *hdmirx, bool is_high)
{
	if (is_high) {
		hdmirx_set_reg(hdmirx->hdmirx_aon_base + 0x1090, 0, 1, 1);
		hdmirx_set_reg(hdmirx->hdmirx_aon_base + 0x1090, 1, 1, 0);
	} else {
		hdmirx_set_reg(hdmirx->hdmirx_aon_base + 0x1090, 0, 1, 1);
		hdmirx_set_reg(hdmirx->hdmirx_aon_base + 0x1090, 0, 1, 0);
	}

	return 0;
}

int hdmirx_link_hpd_set(struct hdmirx_ctrl_st *hdmirx, bool is_high)
{
	if (is_high) {
		disp_pr_info("hpd high");
		hdmirx_link_set_power(hdmirx, is_high);
		hdmirx_link_set_hpd_level(hdmirx, is_high);
	} else {
		disp_pr_info("hpd low");
		hdmirx_link_set_hpd_level(hdmirx, is_high);
		hdmirx_link_set_power(hdmirx, is_high);
	}

	return 0;
}