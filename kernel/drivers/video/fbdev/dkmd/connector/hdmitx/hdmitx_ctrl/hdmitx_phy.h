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
#ifndef __HDMITX_PHY_H__
#define __HDMITX_PHY_H__

#include "hdmitx_ctrl.h"
#include "hdmitx_phy_config.h"

int drv_hdmitx_phy_configure_frl_fpga(struct hdmitx_ctrl *hdmitx);
struct hdmitx_phy *hdmitx_phy_init(struct hdmitx_ctrl *hdmitx);
void drv_hdmitx_phy_deinit(struct hdmitx_phy *phy);
void drv_hdmitx_phy_on(const struct hdmitx_phy *phy);
void drv_hdmitx_phy_off(const struct hdmitx_phy *phy);
void drv_hdmitx_phy_configure_tmds(struct hdmitx_phy *phy, const struct phy_tmds *tmds);
void drv_hdmitx_phy_configure_frl(struct hdmitx_phy *phy, const struct phy_frl *frl);
void drv_hdmitx_phy_configure_frl_tmds_clk(const struct hdmitx_phy *phy, const struct phy_frl_tmds_clk *frl_tmds_clk);
void drv_hdmitx_phy_configure_ssc(const struct hdmitx_phy *phy, const struct phy_ssc *ssc);
void drv_hdmitx_phy_reset(const struct hdmitx_phy *phy);
u32 drv_hdmitx_phy_get_pixel_clk(const struct hdmitx_phy *phy);
u32 drv_hdmitx_phy_get_tmds_clk(const struct hdmitx_phy *phy);
u32 drv_hdmitx_phy_get_frl_clk(const struct hdmitx_phy *phy);
void drv_hdmitx_crg_set(const struct hdmitx_ctrl *hdmitx);
void drv_hdmitx_phy_fcg_set(const struct hdmitx_ctrl *hdmitx);
#endif

