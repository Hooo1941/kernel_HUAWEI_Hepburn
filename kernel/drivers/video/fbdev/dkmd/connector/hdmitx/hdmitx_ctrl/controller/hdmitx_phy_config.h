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

#ifndef __HDMITX_PHY_CONFIG_H__
#define __HDMITX_PHY_CONFIG_H__

#include <linux/types.h>

#define SUCCESS 0
#define FAILURE (-1)

/* frl rate define */
#define FRL_RATE_3L_3G 1
#define FRL_RATE_3L_6G 2
#define FRL_RATE_4L_6G 3
#define FRL_RATE_4L_8G 4
#define FRL_RATE_4L_10G 5
#define FRL_RATE_4L_12G 6

/* color depth define */
#define DEEP_COLOR_24BIT 0
#define DEEP_COLOR_30BIT 1
#define DEEP_COLOR_36BIT 2

/* phy port id define */
#define PHY_ID_0 0
#define PHY_ID_1 1

/* pcb_len define */
#define PCB_LEN_1 1
#define PCB_LEN_2 2
#define PCB_LEN_3 3
#define PCB_LEN_4 4
#define PCB_LEN_5 5

/* mode define */
#define PHY_SET_MODE_TMDS 0
#define PHY_SET_MODE_FRL  1

struct phy_tmds {
	u32 pixel_clk;
	u32 tmds_clk; /* TMDS colck,in kHz */
	u32 color_depth; /* Deep color(color depth) */
	u8  pcb_len;
	bool yuv_420;
};

struct phy_frl {
	u32 frl_rate; /* Lane and Rate */
	u8 pcb_len;
};

struct phy_frl_tmds_clk {
	u32 pixel_clk;
	u32 color_depth;
	u32 rate;
};

struct phy_ssc {
	bool enable;
	u32 pixel_clk;
	u32 color_depth;
	u32 tmds_clk;
	u32 ssc_freq; /* kHz */
	u32 ssc_amp;
};

struct hdmitx_phy {
	void *regs;
	void *crg_regs;
	u32 phy_id;
	u32 mode;
};

enum aphy_addr_offset {
	APHY_OFFSET_0 = 0x0,
	APHY_OFFSET_1,
	APHY_OFFSET_2,
	APHY_OFFSET_3,
	APHY_OFFSET_4,
	APHY_OFFSET_5,
	APHY_OFFSET_6,
	APHY_OFFSET_7,
	APHY_OFFSET_8,
	APHY_OFFSET_9,
	APHY_OFFSET_A,
	APHY_OFFSET_B,
	APHY_OFFSET_C,
	APHY_OFFSET_D,
	APHY_OFFSET_E,
	APHY_OFFSET_F,
	APHY_OFFSET_MAX
};

void hal_phy_frl_set(struct hdmitx_phy *phy, const struct phy_frl *frl);
void hal_phy_frl_tmds_clock_set(const struct hdmitx_phy *phy, const struct phy_frl_tmds_clk *frl_tmds_clk);
void hal_phy_hw_oe_enable(const struct hdmitx_phy *phy, bool enable);
bool hal_phy_is_on(const struct hdmitx_phy *phy);
void hal_phy_reset(const struct hdmitx_phy *phy);
void hal_phy_ssc_set(const struct hdmitx_phy *phy, const struct phy_ssc *ssc);
void hal_phy_tmds_set(struct hdmitx_phy *phy, const struct phy_tmds *tmds);
u32 hal_phy_get_pixel_clk(const struct hdmitx_phy *phy);
u32 hal_phy_get_tmds_clk(const struct hdmitx_phy *phy);
u32 hal_phy_get_frl_clk(const struct hdmitx_phy *phy);
void stb_writeb(const struct hdmitx_phy *hdmi_phy, u32 cs, enum aphy_addr_offset offset, u8 data);
void stb_readb(const struct hdmitx_phy *hdmi_phy, u32 cs, enum aphy_addr_offset offset, u8 *data);
void stb_set_bits(const struct hdmitx_phy *hdmi_phy, u32 cs, enum aphy_addr_offset aphy_offset, u8 mask, u8 value);

#endif /* __HDMITX_PHY_CONFIG_H__ */
