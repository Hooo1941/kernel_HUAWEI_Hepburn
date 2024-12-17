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
#include <linux/delay.h>
#include <securec.h>

#include "dkmd_log.h"
#include "hdmitx_phy_config.h"
#include "hdmitx_dphy_reg.h"
#include "hdmitx_ctrl.h"
#include "hdmitx_dbg.h"

/* aphy cs define */
#define APHY_CS_0      0x1
#define APHY_CS_1      0x2
#define APHY_CS_2      0x4
#define APHY_CS_3      0x8
#define APHY_CS_012    0x7
#define APHY_CS_0123   0xf
#define APHY_CS_8      0x100
#define APHY_CS_9      0x200

#define APHY_CS_MAX    10

#define AC_CTRL_BUS_SRST_REQ_M       (0x1 << 7)
#define AC_CTRL_SRST_REQ_M           (0x1 << 6)
#define PHY_SRST_REQ_M               (0x1 << 4)
#define CFG_HDMITX_PHY_BUS_SRST_REQ_M  (0x1 << 2)

#define RATE_3G      1
#define RATE_6G      2
#define RATE_8G      3
#define RATE_10G     4
#define RATE_12G     5

#define FRL_DATA_LANE_0   0
#define FRL_DATA_LANE_1   1
#define FRL_DATA_LANE_2   2
#define FRL_DATA_LANE_3   3

#define MAX_FFE_LEVEL   4
#define FRL_DATA_LANE_NUM   4

#define aphy_refclksel(x)   (((x) & 0x7) << 4)
#define APHY_REFCLKSEL_M    (0x7 << 4)
#define aphy_refclkdiv(x)   (((x) & 0x7) << 0)
#define APHY_REFCLKDIV_M    (0x7 << 0)

#define aphy_data_channel_en(x) (((x) & 0x1) << 1)
#define APHY_DATA_CHANNEL_EN_M  (0x1 << 1)

#define PIXEL_CLK_37500   37500
#define PIXEL_CLK_75000   75000
#define PIXEL_CLK_50000   50000
#define PIXEL_CLK_150000  150000
#define PIXEL_CLK_300000  300000
#define PIXEL_CLK_600000  600000
#define PIXEL_CLK_1188000 1188000

#define COEFFICIENT_VALUE 1024

#define PHY_TMDS_OUTEN 0x14

#define MAX_DTS_COMPATIB_NAME_LEN 25

struct fcg_div_params {
	u32 tmds_cnt_val;
	u32 cnt1_target;
	u8 fcg_divm;
	u8 fcg_divp;
	u8 ki;
	u8 lock_th;
	u8 fcg_postdiv;
};

struct dphy_spec_en {
	bool drv_post2_en;
	bool drv_post1_en;
	bool drv_pre_en;
};

struct dphy_spec_params {
	u8 drv_post2;
	u8 drv_post1;
	u8 drv_main;
	u8 drv_pre;
};

struct aphy_spec_params {
	u8 offset_0;
	u8 offset_1;
	u8 offset_2;
	u8 offset_3;
	u8 offset_4;
	u8 offset_5;
	u8 offset_6;
	u8 offset_7;
	u8 offset_8;
	u8 offset_9;
	u8 offset_a;
	u8 offset_b;
};

struct spec_params {
	struct dphy_spec_en en;
	struct dphy_spec_params dphy;
	struct aphy_spec_params aphy;
};

struct tmds_spec_params {
	u32 min_tmds_clk;
	u32 max_tmds_clk;
	struct spec_params data;
	struct spec_params clock;
};

struct frl_spec_params {
	u32 rate;
	struct dphy_spec_en en;
	struct aphy_spec_params aphy[FRL_DATA_LANE_NUM];
	struct dphy_spec_params ffe[MAX_FFE_LEVEL];
};

static struct tmds_spec_params g_tmds_spec[] = {
	{
		/* min_tmds_clk, max_tmds_clk */
		25000, 100000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en,pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		100000, 165000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		165000, 340000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x03, 0x22, 0x02 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0xf7, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		340000, 600000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x3f, 0x3f, 0x12 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x8a, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, true, true },
			/* post2, post1, main, pre */
			{ 0x00, 0x3f, 0x3f, 0x12 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x8a, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},
};

static struct tmds_spec_params g_tmds_spec_a[] = {
	{
		/* min_tmds_clk, max_tmds_clk */
		25000, 100000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en,pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		100000, 165000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		165000, 340000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x14, 0x00 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},

	{
		/* min_tmds_clk, max_tmds_clk */
		340000, 600000,
		/* data */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x28, 0x14 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		/* clock */
		{
			/* post2_en, post1_en, pre_en */
			{ false, false, false },
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x28, 0x14 },
			/* offset */
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
	},
};

static struct frl_spec_params g_frl_spec[] = {
	{
		RATE_3G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe0 */
			{ 0x00, 0x3f, 0x3f, 0x08 }, /* ffe1 */
			{ 0x00, 0x3f, 0x3f, 0x10 }, /* ffe2 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe3 */
		},
	},

	{
		RATE_6G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe0 */
			{ 0x00, 0x3f, 0x3f, 0x08 }, /* ffe1 */
			{ 0x00, 0x3f, 0x3f, 0x10 }, /* ffe2 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe3 */
		},
	},

	{
		RATE_8G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe0 */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe1 */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe2 */
			{ 0x00, 0x19, 0x20, 0x11 }, /* ffe3 */
		},
	},

	{
		RATE_10G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x8d, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8d, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8d, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8d, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe0 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe1 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe2 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe3 */
		},
	},

	{
		RATE_12G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x8a, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8d, 0x14, 0x00, 0xa0, 0x40, 0x72, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8a, 0x14, 0x00, 0xa0, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x8f, 0x14, 0x00, 0xa0, 0x40, 0x73, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe0 */
			{ 0x00, 0x3f, 0x3f, 0x14 }, /* ffe1 */
			{ 0x00, 0x3f, 0x3f, 0x12 }, /* ffe2 */
			{ 0x00, 0x3f, 0x3f, 0x13 }, /* ffe3 */
		},
	}
};

static struct frl_spec_params g_frl_spec_a[] = {
	{
		RATE_3G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x26,  0x08 }, /* ffe0 */
			{ 0x00, 0x00, 0x26,  0x08 }, /* ffe1 */
			{ 0x00, 0x00, 0x26,  0x08 }, /* ffe2 */
			{ 0x00, 0x00, 0x26,  0x08 }, /* ffe3 */
		},
	},

	{
		RATE_6G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x00, 0x26, 0x0a }, /* ffe0 */
			{ 0x00, 0x00, 0x26, 0x0a }, /* ffe1 */
			{ 0x00, 0x00, 0x26, 0x0a }, /* ffe2 */
			{ 0x00, 0x00, 0x26, 0x0a }, /* ffe3 */
		},
	},

	{
		RATE_8G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x04, 0x24, 0x0c }, /* ffe0 */
			{ 0x00, 0x04, 0x24, 0x0c }, /* ffe1 */
			{ 0x00, 0x04, 0x24, 0x0c }, /* ffe2 */
			{ 0x00, 0x04, 0x24, 0x0c }, /* ffe3 */
		},
	},

	{
		RATE_10G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x08, 0x25, 0x16 }, /* ffe0 */
			{ 0x00, 0x08, 0x25, 0x16 }, /* ffe1 */
			{ 0x00, 0x08, 0x25, 0x16 }, /* ffe2 */
			{ 0x00, 0x08, 0x25, 0x16 }, /* ffe3 */
		},
	},

	{
		RATE_12G,
		/* post2_en, post1_en, pre_en */
		{ false, true, true },
		/* offset */
		{
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
			{ 0x3f, 0x7f, 0x3e, 0x77, 0x14, 0x00, 0x60, 0x40, 0x74, 0x8c, 0xc0, 0x55 },
		},
		{
			/* post2, post1, main, pre */
			{ 0x00, 0x0a, 0x28, 0x2f}, /* ffe0 */
			{ 0x00, 0x0a, 0x28, 0x2f}, /* ffe1 */
			{ 0x00, 0x0a, 0x28, 0x2f}, /* ffe2 */
			{ 0x00, 0x0a, 0x28, 0x2f}, /* ffe3 */
		},
	}
};

static u32 phy_pow(u32 base, u8 exp)
{
	u16 i;
	u32 val = 1;

	for (i = 0; i < exp; i++)
		val = val * base;

	return val;
}

void stb_writeb(const struct hdmitx_phy *hdmi_phy, u32 cs, enum aphy_addr_offset offset, u8 data)
{
	/* CS reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M);
	/* WR  en */
	hdmi_clr(hdmi_phy->regs, REG_PHY_WR, REG_STB_WEN_M);
	/* WDATA set */
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_WDATA, REG_STB_WDATA_M, data);
	/* ADDR set */
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_ADDR, REG_STB_ADDR_M, offset);
	/* WR set */
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_WR, REG_STB_WEN_M, 0x1);
	/* CS set */
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M, cs);
	/* delay time 1 */
	udelay(1);
	/* CS reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M);
	/* WR reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_WR, REG_STB_WEN_M);
}

void stb_readb(const struct hdmitx_phy *hdmi_phy, u32 cs, enum aphy_addr_offset offset, u8 *data)
{
	/* cs reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M);
	/* wr reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_WR, REG_STB_WEN_M);
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_ADDR, REG_STB_ADDR_M, offset);
	udelay(1);
	/* cs set */
	hdmi_write_bits(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M, cs);
	udelay(1);
	/* rdata read */
	*data = hdmi_readl(hdmi_phy->regs, REG_PHY_RDATA);
	/* cs reset */
	hdmi_clr(hdmi_phy->regs, REG_PHY_CSEN, REG_STB_CS_EN_M);
}

void stb_set_bits(const struct hdmitx_phy *hdmi_phy, u32 cs,
	enum aphy_addr_offset aphy_offset, u8 mask, u8 value)
{
	u8 i;
	u32 cs_tmp = 0;
	u8 tmp = 0;

	for (i = 0; i < APHY_CS_MAX; i++) {
		if ((cs >> i) & 0x1) {
			cs_tmp = 0x1 << i;
			break;
		}
	}

	if (i == APHY_CS_MAX) {
		dpu_pr_err("cs is zero!");
		return;
	}

	stb_readb(hdmi_phy, cs_tmp, aphy_offset, &tmp);
	tmp = (tmp & ~mask) | (value & mask);
	stb_writeb(hdmi_phy, cs, aphy_offset, tmp);
}

static u32 get_cs_str(u32 aphy_cs)
{
	u8 ret = 0;
	switch (aphy_cs) {
	case APHY_CS_0:
		ret = 0;
		break;
	case APHY_CS_1:
		ret = 1;
		break;
	case APHY_CS_2:
		ret = 2;
		break;
	case APHY_CS_3:
		ret = 3;
		break;
	case APHY_CS_012:
		ret = 12;
		break;
	case APHY_CS_0123:
		ret = 123;
		break;
	case APHY_CS_8:
		ret = 8;
		break;
	case APHY_CS_9:
		ret = 9;
		break;
	default:
		break;
	}
	return ret;
}

static void dump_aphy_cs0123_values(const struct hdmitx_phy *hdmi_phy, u32 aphy_cs)
{
	u8 data[APHY_OFFSET_D] = {0};
	u32 i;
	for (i = 0; i < APHY_OFFSET_D; i++)
		stb_readb(hdmi_phy, aphy_cs, i, &data[i]);

	dpu_pr_info("APHY_CS_%u: offset0 = 0x%02x, offset1 = 0x%02x, offset2 = 0x%02x, offset3 = 0x%02x, offset4 = 0x%02x,"
			"\n\t offset5 = 0x%02x, offset6 = 0x%02x, offset7 = 0x%02x, offset8 = 0x%02x, offset9 = 0x%02x, offsetA = 0x%02x,"
			"\n\t offsetB = 0x%02x, offsetC = %02x",
			get_cs_str(aphy_cs), data[APHY_OFFSET_0], data[APHY_OFFSET_1], data[APHY_OFFSET_2], data[APHY_OFFSET_3],
			data[APHY_OFFSET_4], data[APHY_OFFSET_5], data[APHY_OFFSET_6], data[APHY_OFFSET_7], data[APHY_OFFSET_8],
			data[APHY_OFFSET_9], data[APHY_OFFSET_A], data[APHY_OFFSET_B], data[APHY_OFFSET_C]);
}

static void dump_aphy_cs89_values(const struct hdmitx_phy *hdmi_phy, u32 aphy_cs)
{
	u8 data[APHY_OFFSET_MAX] = {0};
	u32 i;
	for (i = 0; i < APHY_OFFSET_MAX; i++)
		stb_readb(hdmi_phy, aphy_cs, i, &data[i]);

	dpu_pr_info("APHY_CS_%u: offset0 = %02x, offset1 = %02x, offset2 = %02x, offset3 = %02x, offset4 = %02x,"
			"\n\t offset5 = %02x, offset6 = %02x, offset7 = %02x, offset8 = %02x, offset9 = %02x, offsetA = %02x,"
			"\n\t offsetB = %02x, offsetC = %02x, offsetD = %02x, offsetE = %02x, offsetF = %02x",
			get_cs_str(aphy_cs), data[APHY_OFFSET_0], data[APHY_OFFSET_1], data[APHY_OFFSET_2], data[APHY_OFFSET_3],
			data[APHY_OFFSET_4], data[APHY_OFFSET_5], data[APHY_OFFSET_6], data[APHY_OFFSET_7], data[APHY_OFFSET_8],
			data[APHY_OFFSET_9], data[APHY_OFFSET_A], data[APHY_OFFSET_B], data[APHY_OFFSET_C], data[APHY_OFFSET_D],
			data[APHY_OFFSET_E], data[APHY_OFFSET_F]);
}

static void get_tmds_spec_params_by_pcb_len(u8 pcb_len, bool flag, struct tmds_spec_params **ret, u32 *len)
{
	struct tmds_spec_params *tmds = NULL;
	struct tmds_spec_params *tmds_a = NULL;

	if (pcb_len == PCB_LEN_1) {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	} else if (pcb_len == PCB_LEN_2) {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	} else if (pcb_len == PCB_LEN_3) {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	} else if (pcb_len == PCB_LEN_4) {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	} else if (pcb_len == PCB_LEN_5) {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	} else {
		tmds = &g_tmds_spec[0];
		tmds_a = &g_tmds_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_tmds_spec) : ARRAY_SIZE(g_tmds_spec_a);
	}
	*ret = flag ? tmds : tmds_a;
}

static struct tmds_spec_params *get_tmds_spec_params(u32 tmds_clk, u8 pcb_len)
{
	u32 i;
	u32 len;
	struct tmds_spec_params *tmp = NULL;
	bool flag = true;

	dpu_pr_info("get_tmds_spec_params: tmds_clk = %u", tmds_clk);
	get_tmds_spec_params_by_pcb_len(pcb_len, flag, &tmp, &len);
	for (i = 0; i < len; i++) {
		if (tmds_clk >= tmp[i].min_tmds_clk && tmds_clk < tmp[i].max_tmds_clk)
			return &tmp[i];
	}

	dpu_pr_err("tmds clock exceed max tmds clock!");
	return &tmp[0];
}

static void get_frl_spec_params_by_pcb_len(u8 pcb_len, bool flag, struct frl_spec_params **ret, u32 *len)
{
	struct frl_spec_params *frl = NULL;
	struct frl_spec_params *frl_a = NULL;

	if (pcb_len == PCB_LEN_1) {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	} else if (pcb_len == PCB_LEN_2) {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	} else if (pcb_len == PCB_LEN_3) {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	} else if (pcb_len == PCB_LEN_4) {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	} else if (pcb_len == PCB_LEN_5) {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	} else {
		frl = &g_frl_spec[0];
		frl_a = &g_frl_spec_a[0];
		*len = flag ? ARRAY_SIZE(g_frl_spec) : ARRAY_SIZE(g_frl_spec_a);
	}
	*ret = flag ? frl : frl_a;
}

static struct frl_spec_params *get_frl_spec_params(u32 rate, u8 pcb_len)
{
	u32 i;
	u32 len;
	struct frl_spec_params *tmp = NULL;
	bool flag = true;

	dpu_pr_info("get_frl_spec_params: rate = %u", rate);
	get_frl_spec_params_by_pcb_len(pcb_len, flag, &tmp, &len);
	for (i = 0; i < len; i++) {
		if (rate == tmp[i].rate)
			return &tmp[i];
	}

	dpu_pr_err("frl rate exceed max frl rate!");
	return &tmp[0];
}

static u32 get_ref_clk_div(u32 pixel_clk)
{
	u32 ref_clk_div = 0;

	if (pixel_clk < PIXEL_CLK_37500) {
		ref_clk_div = 0x0;
	} else if (pixel_clk >= PIXEL_CLK_37500 && pixel_clk < PIXEL_CLK_75000) {
		ref_clk_div = 0x1;
	} else if (pixel_clk >= PIXEL_CLK_75000 && pixel_clk < PIXEL_CLK_150000) {
		ref_clk_div = 0x2;
	} else if (pixel_clk >= PIXEL_CLK_150000 && pixel_clk < PIXEL_CLK_300000) {
		ref_clk_div = 0x3;
	} else if (pixel_clk >= PIXEL_CLK_300000 && pixel_clk < PIXEL_CLK_600000) {
		ref_clk_div = 0x4;
	}

	return ref_clk_div;
}

static u32 get_tmds_clk_div(u32 tmds_clk)
{
	u32 tmds_clk_div = 0;

	if (tmds_clk < PIXEL_CLK_37500) {
		tmds_clk_div = 0x4;
	} else if (tmds_clk >= PIXEL_CLK_37500 && tmds_clk < PIXEL_CLK_75000) {
		tmds_clk_div = 0x3;
	} else if (tmds_clk >= PIXEL_CLK_75000 && tmds_clk < PIXEL_CLK_150000) {
		tmds_clk_div = 0x2;
	} else if (tmds_clk >= PIXEL_CLK_150000 && tmds_clk < PIXEL_CLK_300000) {
		tmds_clk_div = 0x1;
	} else if (tmds_clk >= PIXEL_CLK_300000 &&  tmds_clk < PIXEL_CLK_600000) {
		tmds_clk_div = 0x0;
	}

	return tmds_clk_div;
}

static u32 get_tmds_fcon_param(const struct hdmitx_phy *hdmi_phy, u32 tmds_clk)
{
	u32 pll_vco_freq;
	u32 tmds_divsel;
	u32 fccntr0;
	u32 i_ref_cnt;
	u64 temp;

	fccntr0 = hdmi_read_bits(hdmi_phy->regs, REG_FCCNTR0, REG_I_REF_CNT_LEN_M);
	tmds_divsel = get_tmds_clk_div(tmds_clk);

	/* formula:pll_vco_freq equal tmds_clk * 5 * phy_pow(2, tmds_divsel) */
	temp = tmds_clk * 5 * phy_pow(2, tmds_divsel);
	pll_vco_freq = (u32)temp;

	/* 24M formula:i_ref_cnt equal (pll_vco_freq * 2 * fccntr0) / (1000 * 20 * 3) */
	/* 38.4M formula[24M to 38.4M]:i_ref_cnt equal (pll_vco_freq * fccntr0) / (1000 * 48) */
	i_ref_cnt = (pll_vco_freq  * fccntr0) / (1000 * 48);

	dpu_pr_info("get_tmds_fcon_param: fccntr0 = %u, tmds_divsel = %u, pll_vco_freq = %u, i_ref_cnt = %u",
				fccntr0, tmds_divsel, pll_vco_freq, i_ref_cnt);
	return i_ref_cnt;
}

static u32 get_frl_fcon_param(const struct hdmitx_phy *hdmi_phy, u32 frl_rate)
{
	u32 i_ref_cnt = 0;
	u32 i_ref_cnt_length;
	u64 temp;

	i_ref_cnt_length = hdmi_read_bits(hdmi_phy->regs, REG_FCCNTR0, REG_I_REF_CNT_LEN_M);

	switch (frl_rate) {
	case FRL_RATE_3L_3G:
	case FRL_RATE_3L_6G:
	case FRL_RATE_4L_6G:
	case FRL_RATE_4L_12G:
		/* kirin formula[24M to 38.4M]:i_ref_cnt equal i_ref_cnt_length * 125 / 2 */
		temp = (i_ref_cnt_length * 125) / 2;
		i_ref_cnt = (u32)temp;
		break;
	case FRL_RATE_4L_8G:
		/* kirin formula[24M to 38.4M]:i_ref_cnt equal (i_ref_cnt_length * 125) / 3 */
		i_ref_cnt = (i_ref_cnt_length * 125) / 3;
		break;
	case FRL_RATE_4L_10G:
		/* kirin formula[24M to 38.4M]:i_ref_cnt equal (i_ref_cnt_length * 625) / 12 */
		i_ref_cnt = (i_ref_cnt_length * 625) / 12;
		break;
	default:
		break;
	}

	dpu_pr_info("get_frl_fcon_param: i_ref_cnt = %u, i_ref_cnt_length = %u", i_ref_cnt, i_ref_cnt_length);
	return i_ref_cnt;
}

static u32 get_tmds_frac_param(u32 tmds_clk, u32 pixel_clk, u32 color_depth, u32 *m_value, u32 *n_value)
{
	u32 m, n;
	u32 mn_value;
	u8 ref_clk_div;
	u8 tmds_divsel;
	u32 k;

	tmds_divsel = get_tmds_clk_div(tmds_clk);
	ref_clk_div = get_ref_clk_div(pixel_clk);

	/*
	 * pll_refclk equals to pixel_clk / phy_pow(2, ref_clk_div)
	 * pll_vco_clk equals to tmds_clk * 5 * phy_pow(2, tmds_divsel)
	 * mn_value equals to pll_vco_clk / pll_refclk,
	 * equals to tmds_clk * 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) /pixel_clk,
	 * equals to 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * (depth_color)
	 */
	switch (color_depth) {
	case DEEP_COLOR_24BIT:
		/* mn_value equal 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 1 */
		mn_value = 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 1;
		break;
	case DEEP_COLOR_30BIT:
		/* mn_value equal 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 5 / 4 */
		mn_value = 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 5 / 4;
		break;
	case DEEP_COLOR_36BIT:
		/* mn_value equal 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 3 / 2 */
		mn_value = 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 3 / 2;
		break;
	default:
		/* mn_value equal 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 1 */
		mn_value = 5 * phy_pow(2, tmds_divsel) * phy_pow(2, ref_clk_div) * 1;
		break;
	}

	n = mn_value % 10; /* Find the remainder of 10 */

	if (n == 0) {
		k = 2; /* k is assigned a value of 2 */
		n = 10; /* n is assigned a value of 10 */
	} else {
		k = 1;
	}

	m = mn_value / 10 - k; /* m equal mn_value / 10 - k */

	*m_value = m;
	*n_value = n;

	dpu_pr_info("get_tmds_frac_param: tmds_divsel = %u, ref_clk_div = %u, m = %u, n = %u, k = %u, mn_value = %u",
				tmds_divsel, ref_clk_div, m, n, k, mn_value);

	return mn_value;
}

static void get_frl_frac_param(u32 frl_rate, u32 *m_value, u32 *n_value, u32 *x_value)
{
	switch (frl_rate) {
	case FRL_RATE_3L_3G:
	case FRL_RATE_3L_6G:
	case FRL_RATE_4L_6G:
	case FRL_RATE_4L_12G:
		*m_value = 0x6;
		*n_value = 0x8;
		*x_value = 0x200000;
		break;
	case FRL_RATE_4L_8G:
		*m_value = 0x4;
		*n_value = 0x2;
		*x_value = 0x155555;
		break;
	case FRL_RATE_4L_10G:
		*m_value = 0x5;
		*n_value = 0x5;
		*x_value = 0x1aaaaa;
		break;
	default:
		break;
	}
	dpu_pr_info("get_frl_frac_param: m_value = 0x%x, n_value = 0x%x, x_value = 0x%x", *m_value, *n_value, *x_value);
}

static u32 get_frl_vco_freq(u32 rate)
{
	u32 vco_freq;

	switch (rate) {
	case FRL_RATE_3L_3G:
	case FRL_RATE_3L_6G:
	case FRL_RATE_4L_6G:
	case FRL_RATE_4L_12G:
		vco_freq = 3000000; /* vco_freq is 3000000 */
		break;
	case FRL_RATE_4L_8G:
		vco_freq = 2000000; /* vco_freq is 2000000 */
		break;
	case FRL_RATE_4L_10G:
		vco_freq = 2500000; /* vco_freq is 2500000 */
		break;
	default:
		vco_freq = 3000000; /* vco_freq is 3000000 */
		break;
	}

	dpu_pr_info("get_frl_vco_freq: vco_freq = %u", vco_freq);
	return vco_freq;
}

static u32 get_disable_dsc_fcg_div(const struct phy_frl_tmds_clk *frl_tmds_clk)
{
	u32 fcg_div;
	u32 ref_pixel_clk;
	u32 vco_clk;

	if (frl_tmds_clk->pixel_clk < 4) { /* can't be zero after divided by 4 */
		dpu_pr_err("pixel clock is zero!");
		return 0;
	}

	vco_clk = get_frl_vco_freq(frl_tmds_clk->rate);
	ref_pixel_clk = frl_tmds_clk->pixel_clk / 4; /* ref_pixel_clk equal frl_tmds_clk->pixel_clk / 4 */

	/*
	 * tmds_cnt_val equals to (dsc_target / ref_pixel_clk) * cnt1_target;
	 * dsc_target = org_dsc_target * n(color_depth: 24bit, n = 1;
	 * color_depth: 30bit, n = 5 /4 ; color_depth: 36bit, n = 3 / 2;)
	 * fcg_div equals to vco_clk / tmds_clk - 1
	 * tmds_clk = ref_pixel_clk * n(color_depth: 24bit, n = 1;
	 * color_depth: 30bit, n = 5 /4 ; color_depth: 36bit, n = 3 / 2;)
	 */
	switch (frl_tmds_clk->color_depth) {
	case DEEP_COLOR_24BIT:
		fcg_div = vco_clk / ref_pixel_clk - 1;
		break;
	case DEEP_COLOR_30BIT:
		fcg_div = vco_clk * 4 / (ref_pixel_clk * 5) - 1; /* fcg_div equal vco_clk * 4 / (ref_pixel_clk * 5) - 1 */
		break;
	case DEEP_COLOR_36BIT:
		fcg_div = vco_clk * 2 / (ref_pixel_clk * 3) - 1; /* fcg_div equal vco_clk * 2 / (ref_pixel_clk * 3) - 1 */
		break;
	default:
		fcg_div = vco_clk / ref_pixel_clk - 1;
		break;
	}

	dpu_pr_info("get_disable_dsc_fcg_div: fcg_div = %u, ref_pixel_clk = %u, vco_clk = %u",
				fcg_div, ref_pixel_clk, vco_clk);
	return fcg_div;
}

static void get_disable_dsc_tmds_cnt_val(struct fcg_div_params *phy_fcg, const struct phy_frl_tmds_clk *frl_tmds_clk)
{
	const u32 cnt1_target = 0x400;
	u32 tmds_cnt_val;
	u32 ref_pixel_clk;
	u32 org_dsc_target;
	u32 ratio;
	u64 temp;

	if (frl_tmds_clk->pixel_clk < 4) { /* can't be zero after divided by 4 */
		dpu_pr_err("pixel clock is zero!");
		return;
	}

	ref_pixel_clk = frl_tmds_clk->pixel_clk / 4; /* ref_pixel_clk equal frl_tmds_clk->pixel_clk / 4 */
	org_dsc_target = frl_tmds_clk->pixel_clk / 4; /* org_dsc_target equal frl_tmds_clk->pixel_clk / 4 */
	ratio = org_dsc_target / ref_pixel_clk;

	/*
	 * tmds_cnt_val equals to (dsc_target / ref_pixel_clk) * cnt1_target;
	 * dsc_target = org_dsc_target * n(color_depth: 24bit, n = 1;
	 * color_depth: 30bit, n = 5 /4 ; color_depth: 36bit, n = 3 / 2;)
	 * fcg_div equals to vco_clk / tmds_clk - 1
	 * tmds_clk = ref_pixel_clk * n(color_depth: 24bit, n = 1;
	 * color_depth: 30bit, n = 5 /4 ; color_depth: 36bit, n = 3 / 2;)
	 */
	switch (frl_tmds_clk->color_depth) {
	case DEEP_COLOR_24BIT:
		temp = ratio * cnt1_target;
		tmds_cnt_val = (u32)temp;
		break;
	case DEEP_COLOR_30BIT:
		tmds_cnt_val = ratio * 5 * cnt1_target / 4; /* tmds_cnt_val equal ratio * 5 * cnt1_target / 4 */
		break;
	case DEEP_COLOR_36BIT:
		tmds_cnt_val = ratio * 3 * cnt1_target / 2; /* tmds_cnt_val equal ratio * 3 * cnt1_target / 2 */
		break;
	default:
		temp = ratio * cnt1_target;
		tmds_cnt_val = (u32)temp;
		break;
	}

	phy_fcg->cnt1_target = cnt1_target;
	phy_fcg->tmds_cnt_val = tmds_cnt_val;

	dpu_pr_info("get_disable_dsc_tmds_cnt_val: tmds_cnt_val = %u, ref_pixel_clk = %u, org_dsc_target = %u, ratio = %u",
				tmds_cnt_val, ref_pixel_clk, org_dsc_target, ratio);
}

static u32 get_fcg_post_div(u32 fcg_div, struct fcg_div_params *phy_fcg)
{
	u32 fcg_div_tmp;

	if (fcg_div <= 112) { /* Fixed parameter value 112 */
		phy_fcg->fcg_postdiv = 0x0;
		fcg_div_tmp = fcg_div;
	} else if (fcg_div > 112 && fcg_div <= 224) { /* Fixed parameter value 112,224 */
		phy_fcg->fcg_postdiv = 0x1;
		fcg_div_tmp = fcg_div / 2; /* Fixed parameter value 2 */
	} else if (fcg_div > 224 && fcg_div <= 448) { /* Fixed parameter value 224,448 */
		phy_fcg->fcg_postdiv = 0x2;
		fcg_div_tmp = fcg_div / 4; /* Fixed parameter value 4 */
	} else {
		phy_fcg->fcg_postdiv = 0x3;
		fcg_div_tmp = fcg_div / 8; /* Fixed parameter value 8 */
	}

	dpu_pr_info("get_fcg_post_div: fcg_div_tmp = %u, fcg_div = %u", fcg_div_tmp, fcg_div);
	return fcg_div_tmp;
}

static void get_fcg_div_params(u32 fcg_div, struct fcg_div_params *phy_fcg)
{
	u8 fcg_div_tmp;

	fcg_div_tmp = get_fcg_post_div(fcg_div, phy_fcg);
	if (fcg_div_tmp > 112) {
		phy_fcg->fcg_divm = 0;
		phy_fcg->fcg_divp = 0;
	} else if (fcg_div_tmp > 96) {
		phy_fcg->fcg_divm = 12;
		phy_fcg->fcg_divp = 3;
	} else if (fcg_div_tmp > 48) {
		phy_fcg->fcg_divp = 3;
		if ((fcg_div_tmp >> 3) <= 6)
			phy_fcg->fcg_divm = 6;
		else
			phy_fcg->fcg_divm = fcg_div_tmp >> 3;
	} else if (fcg_div_tmp > 24) {
		phy_fcg->fcg_divp = 2;
		if ((fcg_div_tmp >> 2) <= 6)
			phy_fcg->fcg_divm = 6;
		else
			phy_fcg->fcg_divm = fcg_div_tmp >> 2;
	} else if (fcg_div_tmp > 12) {
		phy_fcg->fcg_divp = 1;
		if ((fcg_div_tmp >> 1) <= 6)
			phy_fcg->fcg_divm = 6;
		else
			phy_fcg->fcg_divm = fcg_div_tmp >> 1;
	} else if (fcg_div_tmp > 4) {
		phy_fcg->fcg_divm = fcg_div_tmp;
		phy_fcg->fcg_divp = 0;
	} else if (fcg_div_tmp > 0) {
		phy_fcg->fcg_divm = 4;
		phy_fcg->fcg_divp = 0;
	}
	dpu_pr_info("get_fcg_div_params: fcg_divm = %u, fcg_divp = %u", phy_fcg->fcg_divm, phy_fcg->fcg_divp);
}

static u8 get_fcg_ki(u32 cnt1_target)
{
	u8 ki;

	if (cnt1_target < COEFFICIENT_VALUE) {
		ki = 0xC;
	} else if (cnt1_target >= COEFFICIENT_VALUE && cnt1_target < COEFFICIENT_VALUE * 2) { /* multiple 2 */
		ki = 0xB;
	} else if (cnt1_target >= COEFFICIENT_VALUE * 2 && cnt1_target < COEFFICIENT_VALUE * 4) { /* multiple 2,4 */
		ki = 0xA;
	} else if (cnt1_target >= COEFFICIENT_VALUE * 4 && cnt1_target < COEFFICIENT_VALUE * 8) { /* multiple 4,8 */
		ki = 0x9;
	} else if (cnt1_target >= COEFFICIENT_VALUE * 8 && cnt1_target < COEFFICIENT_VALUE * 16) { /* multiple 8,16 */
		ki = 0x8;
	} else {
		ki = 0x7;
	}

	dpu_pr_info("get_fcg_ki: cnt1_target = %u, ki = 0x%x", cnt1_target, ki);
	return ki;
}

static void get_disable_dsc_fcg_params(struct fcg_div_params *phy_fcg, const struct phy_frl_tmds_clk *frl_tmds_clk)
{
	u8 ki;
	u32 fcg_div;

	/* get fcg div && tmds cnt val */
	fcg_div = get_disable_dsc_fcg_div(frl_tmds_clk);
	get_disable_dsc_tmds_cnt_val(phy_fcg, frl_tmds_clk);
	/* get divm divp param */
	get_fcg_div_params(fcg_div, phy_fcg);
	/* get ki && lock value */
	ki = get_fcg_ki(phy_fcg->cnt1_target);

	phy_fcg->ki = ki;
	phy_fcg->lock_th = ki - 0x5;
}

static void get_tmds_ssc_params(const struct phy_ssc *ssc, u16 *mod_n, u16 *mod_d)
{
	u32 m = 0;
	u32 n = 0;
	u32 mn;
	u32 mod_dn;
	u32 ref_clk_div;
	u32 pll_ref_clk;
	u32 tmp_mod_n;
	u32 tmp_mod_d;
	u32 rem;

	if (ssc->ssc_freq == 0) {
		dpu_pr_err("Ssc Freq is zero");
		return;
	}

	mn = get_tmds_frac_param(ssc->tmds_clk, ssc->pixel_clk, ssc->color_depth, &m, &n);

	ref_clk_div = get_ref_clk_div(ssc->pixel_clk);
	/* pll_ref_clk equal ssc->pixel_clk / phy_pow(2, ref_clk_div) */
	pll_ref_clk = ssc->pixel_clk / phy_pow(2, ref_clk_div);

	mod_dn = pll_ref_clk * 2500 / ssc->ssc_freq; /* mod_dn equal pll_ref_clk * 2500 / ssc->ssc_freq */
	if (mod_dn % 10000 >= 5000) /* mod_dn % 10000 greater or equal to 5000 */
		tmp_mod_n = mod_dn / 10000 + 1; /* tmp_mod_d equal tmp_mod_d / 100000 + 1 */
	else
		tmp_mod_n = mod_dn / 10000; /* tmp_mod_d equal tmp_mod_d / 100000 */

	/*
	 * Relative deviation equal (MOD_D(0x80A0[15:0])*
	 * MOD_N(0x809C[31:16])) / (65536*(((M+1)*10)+(N+X)))
	 */
	if (tmp_mod_n == 0) {
		dpu_pr_err("tmp_mod_n is zero");
		return;
	}

	tmp_mod_d = (u32)div_u64((u64)phy_pow(2, 24) * mn * ssc->ssc_amp, tmp_mod_n); /* number 2,24 */
	rem = tmp_mod_d % 100000; /* rem equal tmp_mod_d % 100000 */
	if (rem >= 50000) /* rem greater or equal to 50000 */
		tmp_mod_d = tmp_mod_d / 100000 + 1; /* tmp_mod_d equal tmp_mod_d / 100000 + 1 */
	else
		tmp_mod_d = tmp_mod_d / 100000; /* tmp_mod_d equal tmp_mod_d / 100000 */

	*mod_n = tmp_mod_n;
	*mod_d = tmp_mod_d;

	dpu_pr_debug("mod_n = %x, mod_d = %x", tmp_mod_n, tmp_mod_d);
}

static void phy_hw_bus_reset(const struct hdmitx_phy *hdmi_phy)
{
	void *crg_reg = hdmi_phy->crg_regs;

	if (crg_reg == NULL) {
		dpu_pr_err("null ptr err, phy_id = %u", hdmi_phy->phy_id);
		return;
	}

	hdmi_set(crg_reg, 0x8, CFG_HDMITX_PHY_BUS_SRST_REQ_M);
	udelay(1);
	hdmi_clr(crg_reg, 0x8, CFG_HDMITX_PHY_BUS_SRST_REQ_M);
}

static void phy_hw_reset(const struct hdmitx_phy *hdmi_phy)
{
	/* step 1: APB reset */
	phy_hw_bus_reset(hdmi_phy);

	/* step 2: Dphy reset */
	hdmi_set(hdmi_phy->regs, REG_SWRESET, REG_GLOBAL_RESET_M);
	udelay(1);
	hdmi_clr(hdmi_phy->regs, REG_SWRESET, REG_GLOBAL_RESET_M);

	/* step 3: Aphy reset  */
	hdmi_clr(hdmi_phy->regs, REG_RESETN, REG_STB_RESETN_M);
	udelay(1);
	hdmi_set(hdmi_phy->regs, REG_RESETN, REG_STB_RESETN_M);
}

static void phy_hw_init(const struct hdmitx_phy *hdmi_phy, u32 pixel_clk, u32 rate)
{
	u8 ref_clk_sel;
	ref_clk_sel = (hdmi_phy->mode == PHY_SET_MODE_TMDS) ?  0x0 : 0x3;
	/*
	 * step1: Dphy clock internal set;
	 * 1).dac clock gate enable;
	 * 2).select mod clk;
	 * 3).select 38.4MHz clock as reference clock;
	 */
	hdmi_set(hdmi_phy->regs, REG_SWRESET, REG_DAC_CLOCK_GAT_M);
	hdmi_set(hdmi_phy->regs, REG_CLKSET, REG_P_MODCLK_SEL_M);
	hdmi_set(hdmi_phy->regs, REG_REFCLKSEL, REG_I_REF_CLK_SEL_M);

	/* step2:Aphy access mode set */
	hdmi_write_bits(hdmi_phy->regs, REG_STBOPT, REG_STB_DELAY0_M, 0x2);
	hdmi_write_bits(hdmi_phy->regs, REG_STBOPT, REG_STB_DELAY1_M, 0x2);
	hdmi_write_bits(hdmi_phy->regs, REG_STBOPT, REG_STB_DELAY2_M, 0x2);

	/*
	 * Aphy PLL setting
	 * step 3: aphy internel block enable
	 */
	if ((hdmi_phy->mode == PHY_SET_MODE_FRL) && (rate == FRL_RATE_4L_10G || rate == FRL_RATE_4L_12G))
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_F, 0xc);

	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_E, 0x11);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_0, 0xff);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_1, 0x31);

	/*
	 * step 4: Aphy ref clock select.
	 * Note: we must make the source clock is stable before we run to here
	 */
	if (hdmi_phy->mode == PHY_SET_MODE_TMDS)
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_2,
			aphy_refclksel(ref_clk_sel) | aphy_refclkdiv(get_ref_clk_div(pixel_clk)));
	else
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_2, aphy_refclksel(ref_clk_sel) | aphy_refclkdiv(0x00));

	/*
	 * step 5: Aphy pll loop parameter setting,
	 * should be provided by hw test guys
	 */
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_3, 0x1);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_4, 0x81);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_5, 0x32);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_6, 0x12);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_7, 0x9);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_8, 0xa4);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_9, 0x80);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_A, 0x5);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_C, 0x0c);
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_D, 0xff);

	/* step 7: Aphy pll */
	stb_writeb(hdmi_phy, APHY_CS_9, APHY_OFFSET_0, 0x07);
	/* step 8: Aphy pll tmds divider setting */
	if (hdmi_phy->mode == PHY_SET_MODE_TMDS)
		stb_writeb(hdmi_phy, APHY_CS_9, APHY_OFFSET_7, (PHY_TMDS_OUTEN << 3) | get_tmds_clk_div(rate)); /* shift 3 */
	else
		stb_writeb(hdmi_phy, APHY_CS_9, APHY_OFFSET_7, 0x20);

	/* step 9: Aphy frl clock output enable */
	if (hdmi_phy->mode == PHY_SET_MODE_FRL)
		stb_writeb(hdmi_phy, APHY_CS_9, APHY_OFFSET_E, 0x0c);
	/*
	 * step 10: Turn off the PHY to wait until VSYNC outputs data
	 * after OE is turned on.
	 */
	if (g_aphy_cs_8_debug) {
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_0, g_aphy_cs_8_offset[APHY_OFFSET_0]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_1, g_aphy_cs_8_offset[APHY_OFFSET_1]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_3, g_aphy_cs_8_offset[APHY_OFFSET_3]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_4, g_aphy_cs_8_offset[APHY_OFFSET_4]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_5, g_aphy_cs_8_offset[APHY_OFFSET_5]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_6, g_aphy_cs_8_offset[APHY_OFFSET_6]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_7, g_aphy_cs_8_offset[APHY_OFFSET_7]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_8, g_aphy_cs_8_offset[APHY_OFFSET_8]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_9, g_aphy_cs_8_offset[APHY_OFFSET_9]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_A, g_aphy_cs_8_offset[APHY_OFFSET_A]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_C, g_aphy_cs_8_offset[APHY_OFFSET_C]);
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_E, g_aphy_cs_8_offset[APHY_OFFSET_E]);
	}
	hdmi_write_bits(hdmi_phy->regs, REG_HDMI_OE_CFG, REG_CFG_OE_SYNC_EN_M, false);
	dpu_pr_info("phy_hw_init end!");
	dump_aphy_cs89_values(hdmi_phy, APHY_CS_8);
	dump_aphy_cs89_values(hdmi_phy, APHY_CS_9);
}

static void phy_hw_tmds_clk_set(const struct hdmitx_phy *hdmi_phy, const struct phy_tmds *tmds)
{
	u32 p_div_h20;
	bool hdmi2_en = false;
	u32 m, n;

	get_tmds_frac_param(tmds->tmds_clk, tmds->pixel_clk, tmds->color_depth, &m, &n);
	p_div_h20 = get_tmds_clk_div(tmds->tmds_clk);
	hdmi2_en = (tmds->tmds_clk > 340000) ? true : false; /* 340000 means tmds clock is 340M */
	hdmi_write_bits(hdmi_phy->regs, REG_FCDSTEPSET, REG_P_DIVN_H20_M, p_div_h20);
	hdmi_clr(hdmi_phy->regs, REG_FCDSTEPSET, REG_FCDSTEPSET_UNUSED_M);
	hdmi_clr(hdmi_phy->regs, REG_FCDSTEPSET, REG_UP_SAMPLER_RATIO_SEL_M);

	hdmi_write_bits(hdmi_phy->regs, REG_FDIVMANUAL, REG_I_MANUAL_EN_M, 0x0e);
	hdmi_write_bits(hdmi_phy->regs, REG_FDIVMANUAL, REG_I_MDIV_M, m);
	hdmi_write_bits(hdmi_phy->regs, REG_FDIVSTAT1, REG_I_FDIV_IN_N_M, n);
	hdmi_write_bits(hdmi_phy->regs, REG_FDIVSTAT1, REG_I_FDIV_IN_X_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_HDMI_MODE, REG_REG_HDMI_MODE_EN_M, hdmi2_en);
	/* FDIV init */ /* set n,x to aphy */
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M);

	hdmi_set(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M);
	/* delay 1us */
	udelay(1);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M);
}

static bool phy_hw_wait_fcon_done(const struct hdmitx_phy *hdmi_phy)
{
	u8 fcon_result[4] = {0}; /* array size is 4 */
	u32 count = 0;
	bool ret;

	while (count <= 3) { /* 3 cycles */
		fcon_result[0] = hdmi_read_bits(hdmi_phy->regs, REG_FCSTAT, REG_CLK_OK_M); /* array index 0 */
		fcon_result[1] = hdmi_read_bits(hdmi_phy->regs, REG_FCSTAT, REG_BUSY_M); /* array index 1 */
		fcon_result[2] = hdmi_read_bits(hdmi_phy->regs, REG_FCSTAT, REG_DONE_M); /* array index 2 */
		fcon_result[3] = hdmi_read_bits(hdmi_phy->regs, REG_FCSTAT, REG_ERROR_M); /* array index 3 */
		if (fcon_result[1] == 0)
			break;

		msleep(1);
		count++;
	}

	if (fcon_result[3] == 0 && fcon_result[2] == 1 && /* array index 3, array index 2 */
		fcon_result[1] == 0 && fcon_result[0] == 1) {
		dpu_pr_info("set fcon is done!");
		ret = true;
	} else if (fcon_result[3] == 1) { /* array index 3 */
		dpu_pr_info("set fcon error!");
		ret = false;
	} else {
		dpu_pr_info("set fcon failed!");
		ret = false;
	}

	return ret;
}

static int phy_hw_fcon_param_set(const struct hdmitx_phy *hdmi_phy, u32 rate)
{
	int ret = 0;

	/* step1: */
	hdmi_write_bits(hdmi_phy->regs, REG_CTSET0, REG_I_ENABLE_M, true);
	hdmi_clr(hdmi_phy->regs, REG_CTSET0, REG_I_RUN_M);
	hdmi_clr(hdmi_phy->regs, REG_CTSET0, REG_CTSET0_UNUSED_M);

	/* step 2: */
	hdmi_write_bits(hdmi_phy->regs, REG_FCCNTR0, REG_I_VCO_ST_WAIT_LEN_M, 0x50);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCNTR0, REG_I_VCO_END_WAIT_LEN_M, 0x50);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCNTR0, REG_I_REF_CNT_LEN_M, 0x80);
	hdmi_write_bits(hdmi_phy->regs, REG_FCOPT, REG_I_CT_SEL_M, true);
	hdmi_write_bits(hdmi_phy->regs, REG_FCOPT, REG_I_CLKDET_SEL_M, true);
	hdmi_write_bits(hdmi_phy->regs, REG_FCOPT, REG_I_CT_MODE_M, 0x2);
	hdmi_clr(hdmi_phy->regs, REG_FCOPT, REG_FCOPT_UNUSED_1_M);
	hdmi_write_bits(hdmi_phy->regs, REG_FCOPT, REG_I_CT_EN_M, 0x1);
	hdmi_clr(hdmi_phy->regs, REG_FCOPT, REG_FCOPT_UNUSED_2_M);
	hdmi_clr(hdmi_phy->regs, REG_FCOPT, REG_I_CT_IDX_SEL_M);
	hdmi_clr(hdmi_phy->regs, REG_FCOPT, REG_FCOPT_UNUSED_M);
	if (hdmi_phy->mode == PHY_SET_MODE_TMDS)
		hdmi_write_bits(hdmi_phy->regs, REG_FCCNTR1, REG_I_REF_CNT_M, get_tmds_fcon_param(hdmi_phy, rate));
	else
		hdmi_write_bits(hdmi_phy->regs, REG_FCCNTR1, REG_I_REF_CNT_M, get_frl_fcon_param(hdmi_phy, rate));

	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET1, REG_P_CONTIN_UPD_RATE_M, 0x960);
	hdmi_clr(hdmi_phy->regs, REG_FCCONTINSET1, REG_P_CONTIN_UPD_TIME_M);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET2, REG_P_CONTIN_UPD_TH_DN_M, 0x5);
	hdmi_clr(hdmi_phy->regs, REG_FCCONTINSET2, REG_FCCONTINSET2_UNUSED_M);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET2, REG_P_CONTIN_UPD_TH_UP_M, 0x3ff);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET0, REG_P_CONTIN_UPD_EN_M, 0x1);
	hdmi_clr(hdmi_phy->regs, REG_FCCONTINSET0, REG_P_CONTIN_UPD_OPT_M);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET0, REG_P_CONTIN_UPD_POL_M, 0x1);
	hdmi_clr(hdmi_phy->regs, REG_FCCONTINSET0, REG_FCCONTINSET0_UNUSED_M);
	hdmi_write_bits(hdmi_phy->regs, REG_FCCONTINSET0, REG_P_CONTIN_UPD_STEP_M, 0x2);
	hdmi_write_bits(hdmi_phy->regs, REG_CTSET0, REG_I_ENABLE_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_CTSET0, REG_I_RUN_M, 0x1);
	hdmi_clr(hdmi_phy->regs, REG_CTSET0, REG_CTSET0_UNUSED_M);
	if (!phy_hw_wait_fcon_done(hdmi_phy))
		ret = -1;

	return ret;
}

static void phy_hw_tmds_aphy_spec_set(const struct hdmitx_phy *hdmi_phy, const struct tmds_spec_params *tmds)
{
	const struct aphy_spec_params *data = &tmds->data.aphy;
	const struct aphy_spec_params *clk = &tmds->clock.aphy;
	u32 i;

	/* data drv set */
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_0, data->offset_0);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_1, data->offset_1);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_2, data->offset_2);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_3, data->offset_3);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_4, data->offset_4);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_5, data->offset_5);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_6, data->offset_6);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_7, data->offset_7);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_8, data->offset_8);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_9, data->offset_9);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_A, data->offset_a);
	stb_writeb(hdmi_phy, APHY_CS_012, APHY_OFFSET_B, data->offset_b);
	/* clk drv set */
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_0, clk->offset_0);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_1, clk->offset_1);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_2, clk->offset_2);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_3, clk->offset_3);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_4, clk->offset_4);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_5, clk->offset_5);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_6, clk->offset_6);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_7, clk->offset_7);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_8, clk->offset_8);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_9, clk->offset_9);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_A, clk->offset_a);
	stb_writeb(hdmi_phy, APHY_CS_3, APHY_OFFSET_B, clk->offset_b);

	if (g_tmds_aphy_cs_012_debug) {
		for (i = 0; i < APHY_OFFSET_C; i++)
			stb_writeb(hdmi_phy, APHY_CS_012, i, g_tmds_aphy_cs_012_offset[i]);
	}

	if (g_tmds_aphy_cs_3_debug) {
		for (i = 0; i < APHY_OFFSET_C; i++)
			stb_writeb(hdmi_phy, APHY_CS_3, i, g_tmds_aphy_cs_012_offset[i]);
	}

	dump_aphy_cs0123_values(hdmi_phy, APHY_CS_3);
}

static void phy_hw_tmds_dphy_spec_set(const struct hdmitx_phy *hdmi_phy, const struct tmds_spec_params *tmds)
{
	const struct dphy_spec_params *data = &tmds->data.dphy;
	const struct dphy_spec_params *clk = &tmds->clock.dphy;
	const struct dphy_spec_en *data_en = &tmds->data.en;
	const struct dphy_spec_en *clk_en = &tmds->clock.en;

	/* select dphy drv set mode */
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_HDMI_FFE_SEL_M, 0x1);
	/* dphy data drv set */
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_POST2_CH0_M, data->drv_post2);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_POST1_CH0_M, data->drv_post1);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_M_CH0_M, data->drv_main);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_PRE_CH0_M, data->drv_pre);

	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_POST2_CH1_M, data->drv_post2);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_POST1_CH1_M, data->drv_post1);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_M_CH1_M, data->drv_main);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_PRE_CH1_M, data->drv_pre);

	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_POST2_CH2_M, data->drv_post2);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_POST1_CH2_M, data->drv_post1);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_M_CH2_M, data->drv_main);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_PRE_CH2_M, data->drv_pre);

	/* dphy clk drv set */
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_POST2_CH3_M, clk->drv_post2);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_POST1_CH3_M, clk->drv_post1);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_M_CH3_M, clk->drv_main);
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_PRE_CH3_M, clk->drv_pre);

	/* dphy data drv enable */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_PRE_EN_M, data_en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST1_EN_M, data_en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST2_EN_M, data_en->drv_post2_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_PRE_EN_M, data_en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST1_EN_M, data_en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST2_EN_M, data_en->drv_post2_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_PRE_EN_M, data_en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST1_EN_M, data_en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST2_EN_M, data_en->drv_post2_en);

	/* dphy clock drv enable */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_PRE_EN_M, clk_en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST1_EN_M, clk_en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST2_EN_M, clk_en->drv_post2_en);

	if (g_tmds_data_ffe_debug) {
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_POST2_CH0_M, g_tmds_data_dphy_ffe[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_POST1_CH0_M, g_tmds_data_dphy_ffe[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_M_CH0_M, g_tmds_data_dphy_ffe[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_DRV_PRE_CH0_M, g_tmds_data_dphy_ffe[3]);

		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_POST2_CH1_M, g_tmds_data_dphy_ffe[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_POST1_CH1_M, g_tmds_data_dphy_ffe[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_M_CH1_M, g_tmds_data_dphy_ffe[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH1, REG_CFG_DRV_PRE_CH1_M, g_tmds_data_dphy_ffe[3]);

		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_POST2_CH2_M, g_tmds_data_dphy_ffe[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_POST1_CH2_M, g_tmds_data_dphy_ffe[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_M_CH2_M, g_tmds_data_dphy_ffe[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH2, REG_CFG_DRV_PRE_CH2_M, g_tmds_data_dphy_ffe[3]);

		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_PRE_EN_M, g_tmds_data_dphy_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST1_EN_M, g_tmds_data_dphy_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST2_EN_M, g_tmds_data_dphy_en[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_PRE_EN_M, g_tmds_data_dphy_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST1_EN_M, g_tmds_data_dphy_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST2_EN_M, g_tmds_data_dphy_en[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_PRE_EN_M, g_tmds_data_dphy_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST1_EN_M, g_tmds_data_dphy_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST2_EN_M, g_tmds_data_dphy_en[0]);
	}

	if (g_tmds_clk_ffe_debug) {
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_POST2_CH3_M, g_tmds_clk_dphy_ffe[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_POST1_CH3_M, g_tmds_clk_dphy_ffe[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_M_CH3_M, g_tmds_clk_dphy_ffe[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH3, REG_CFG_DRV_PRE_CH3_M, g_tmds_clk_dphy_ffe[3]);

		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_PRE_EN_M, g_tmds_clk_dphy_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST1_EN_M, g_tmds_clk_dphy_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST2_EN_M, g_tmds_clk_dphy_en[0]);
	}
}

static void phy_hw_frl_aphy_spec_set(const struct hdmitx_phy *hdmi_phy, const struct frl_spec_params *frl)
{
	const struct aphy_spec_params *data = &frl->aphy[0];
	u32 aphy_cs;
	u32 i;

	/* data drv set */
	for (i = 0; i < FRL_DATA_LANE_NUM; i++) {
		switch (i) {
		case FRL_DATA_LANE_0:
			aphy_cs = APHY_CS_0;
			break;
		case FRL_DATA_LANE_1:
			aphy_cs = APHY_CS_1;
			break;
		case FRL_DATA_LANE_2:
			aphy_cs = APHY_CS_2;
			break;
		case FRL_DATA_LANE_3:
			aphy_cs = APHY_CS_3;
			break;
		default:
			break;
		}
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_0, data[i].offset_0);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_1, data[i].offset_1);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_2, data[i].offset_2);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_3, data[i].offset_3);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_4, data[i].offset_4);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_5, data[i].offset_5);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_6, data[i].offset_6);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_7, data[i].offset_7);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_8, data[i].offset_8);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_9, data[i].offset_9);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_A, data[i].offset_a);
		stb_writeb(hdmi_phy, aphy_cs, APHY_OFFSET_B, data[i].offset_b);
	}

	if (g_frl_aphy_cs_0123_debug) {
		for (i = 0; i < APHY_OFFSET_C; i++)
			stb_writeb(hdmi_phy, APHY_CS_0123, i, g_frl_aphy_cs_0123_offset[i]);
	}
}

static void phy_hw_frl_dphy_spec_set(const struct hdmitx_phy *hdmi_phy, const struct frl_spec_params *frl)
{
	const struct dphy_spec_en *en = &frl->en;
	const struct dphy_spec_params *ffe = &frl->ffe[0];

	/* select dphy drv set mode */
	hdmi_write_bits(hdmi_phy->regs, REG_TMDS_DRV_CFG_CH0, REG_CFG_HDMI_FFE_SEL_M, 0x0);

	/* dphy ffe0~3 drv set */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_POST2_M, ffe[0].drv_post2); /* ffe0 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_POST1_M, ffe[0].drv_post1); /* ffe0 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_M_M, ffe[0].drv_main); /* ffe0 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_PRE_M, ffe[0].drv_pre); /* ffe0 */

	hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_POST2_M, ffe[1].drv_post2); /* ffe1 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_POST1_M, ffe[1].drv_post1); /* ffe1 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_M_M, ffe[1].drv_main); /* ffe1 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_PRE_M, ffe[1].drv_pre); /* ffe1 */

	hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_POST2_M, ffe[2].drv_post2); /* ffe2 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_POST1_M, ffe[2].drv_post1); /* ffe2 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_M_M, ffe[2].drv_main); /* ffe2 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_PRE_M, ffe[2].drv_pre); /* ffe2 */

	hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_POST2_M, ffe[3].drv_post2); /* ffe3 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_POST1_M, ffe[3].drv_post1); /* ffe3 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_M_M, ffe[3].drv_main); /* ffe3 */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_PRE_M, ffe[3].drv_pre); /* ffe3 */

	/* dphy data drv enable */
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_PRE_EN_M, en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST1_EN_M, en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST2_EN_M, en->drv_post2_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_PRE_EN_M, en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST1_EN_M, en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST2_EN_M, en->drv_post2_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_PRE_EN_M, en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST1_EN_M, en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST2_EN_M, en->drv_post2_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_PRE_EN_M, en->drv_pre_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST1_EN_M, en->drv_post1_en);
	hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST2_EN_M, en->drv_post2_en);
	if (g_frl_ffe_debug) {
		/* dphy ffe0~3 drv set */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_POST2_M, g_frl_dphy_ffe0[0]); /* ffe0 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_POST1_M, g_frl_dphy_ffe0[1]); /* ffe0 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_M_M, g_frl_dphy_ffe0[2]); /* ffe0 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE0_CFG, REG_CFG_FFE0_DRV_PRE_M, g_frl_dphy_ffe0[3]); /* ffe0 */

		hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_POST2_M, g_frl_dphy_ffe1[0]); /* ffe1 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_POST1_M, g_frl_dphy_ffe1[1]); /* ffe1 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_M_M, g_frl_dphy_ffe1[2]); /* ffe1 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE1_CFG, REG_CFG_FFE1_DRV_PRE_M, g_frl_dphy_ffe1[3]); /* ffe1 */

		hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_POST2_M, g_frl_dphy_ffe2[0]); /* ffe2 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_POST1_M, g_frl_dphy_ffe2[1]); /* ffe2 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_M_M, g_frl_dphy_ffe2[2]); /* ffe2 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE2_CFG, REG_CFG_FFE2_DRV_PRE_M, g_frl_dphy_ffe2[3]); /* ffe2 */

		hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_POST2_M, g_frl_dphy_ffe3[0]); /* ffe3 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_POST1_M, g_frl_dphy_ffe3[1]); /* ffe3 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_M_M, g_frl_dphy_ffe3[2]); /* ffe3 */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE3_CFG, REG_CFG_FFE3_DRV_PRE_M, g_frl_dphy_ffe3[3]); /* ffe3 */

		/* dphy data drv enable */
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_PRE_EN_M, g_frl_dphy_spec_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST1_EN_M, g_frl_dphy_spec_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C3_POST2_EN_M, g_frl_dphy_spec_en[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_PRE_EN_M, g_frl_dphy_spec_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST1_EN_M, g_frl_dphy_spec_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C2_POST2_EN_M, g_frl_dphy_spec_en[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_PRE_EN_M, g_frl_dphy_spec_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST1_EN_M, g_frl_dphy_spec_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C1_POST2_EN_M, g_frl_dphy_spec_en[0]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_PRE_EN_M, g_frl_dphy_spec_en[2]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST1_EN_M, g_frl_dphy_spec_en[1]);
		hdmi_write_bits(hdmi_phy->regs, REG_FFE_EN_CFG, REG_CFG_C0_POST2_EN_M, g_frl_dphy_spec_en[0]);
	}
}

static void phy_hw_data_channel_enable(const struct hdmitx_phy *hdmi_phy, u32 rate)
{
	/* hs fifo enable hs link external */
	hdmi_write_bits(hdmi_phy->regs, REG_HSSET, REG_P_HSSET_M, 0x3);
	/* transmitter fifo and up sampler enable */
	hdmi_write_bits(hdmi_phy->regs, REG_TXFIFOSET0, REG_P_PR_EN_H20_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_TXFIFOSET0, REG_P_ENABLE_H20_M, 0x1);
	hdmi_clr(hdmi_phy->regs, REG_TXFIFOSET0, REG_TXFIFOSET0_UNUSED_M);

	if (hdmi_phy->mode == PHY_SET_MODE_TMDS) {
		/* hs input data selection */
		hdmi_clr(hdmi_phy->regs, REG_TXDATAOUTSEL, REG_P_CH_OUT_SEL_M);
		stb_set_bits(hdmi_phy, APHY_CS_0123, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(0));
		stb_set_bits(hdmi_phy, APHY_CS_0123, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(1));
	} else {
		hdmi_write_bits(hdmi_phy->regs, REG_TXDATAOUTSEL, REG_P_CH_OUT_SEL_M, 0x1);
		switch (rate) {
		case FRL_RATE_3L_3G:
		case FRL_RATE_3L_6G:
			stb_set_bits(hdmi_phy, APHY_CS_0123, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(0));
			stb_set_bits(hdmi_phy, APHY_CS_012, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(1));
			break;
		case FRL_RATE_4L_6G:
		case FRL_RATE_4L_8G:
		case FRL_RATE_4L_10G:
		case FRL_RATE_4L_12G:
			stb_set_bits(hdmi_phy, APHY_CS_0123, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(0));
			stb_set_bits(hdmi_phy, APHY_CS_0123, APHY_OFFSET_0, APHY_DATA_CHANNEL_EN_M, aphy_data_channel_en(1));
			break;
		default:
			break;
		}
	}
	dump_aphy_cs0123_values(hdmi_phy, APHY_CS_012);
	dump_aphy_cs0123_values(hdmi_phy, APHY_CS_0123);
}

static void phy_hw_ch0_dcc_qec_set(const struct hdmitx_phy *hdmi_phy)
{
	/* channel 0 dcc setting */
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC3_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC3_VALUE_CH0_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC2_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC2_VALUE_CH0_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC1_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC1_VALUE_CH0_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC0_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH0, REG_CFG_DCC0_VALUE_CH0_M, 0x28);
	/* channel 0 qec setting */
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC3_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC3_VALUE_CH0_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC2_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC2_VALUE_CH0_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC1_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC1_VALUE_CH0_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC0_OVR_CH0_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH0, REG_CFG_QEC0_VALUE_CH0_M, 0x1f);
}

static void phy_hw_ch1_dcc_qec_set(const struct hdmitx_phy *hdmi_phy)
{
	/* channel 1 dcc setting */
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC3_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC3_VALUE_CH1_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC2_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC2_VALUE_CH1_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC1_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC1_VALUE_CH1_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC0_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH1, REG_CFG_DCC0_VALUE_CH1_M, 0x28);
	/* channel 1 qec setting */
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC3_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC3_VALUE_CH1_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC2_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC2_VALUE_CH1_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC1_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC1_VALUE_CH1_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC0_OVR_CH1_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH1, REG_CFG_QEC0_VALUE_CH1_M, 0x1f);
}

static void phy_hw_ch2_dcc_qec_set(const struct hdmitx_phy *hdmi_phy)
{
	/* channel 2 dcc setting */
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC3_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC3_VALUE_CH2_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC2_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC2_VALUE_CH2_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC1_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC1_VALUE_CH2_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC0_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH2, REG_CFG_DCC0_VALUE_CH2_M, 0x28);
	/* channel 2 qec setting */
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC3_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC3_VALUE_CH2_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC2_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC2_VALUE_CH2_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC1_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC1_VALUE_CH2_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC0_OVR_CH2_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH2, REG_CFG_QEC0_VALUE_CH2_M, 0x1f);
}

static void phy_hw_ch3_dcc_qec_set(const struct hdmitx_phy *hdmi_phy)
{
	/* channel 3 dcc setting */
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC3_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC3_VALUE_CH3_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC2_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC2_VALUE_CH3_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC1_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC1_VALUE_CH3_M, 0x28);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC0_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_DCC_OVERWRITE_CFG_CH3, REG_CFG_DCC0_VALUE_CH3_M, 0x28);
	/* channel 3 qec setting */
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC3_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC3_VALUE_CH3_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC2_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC2_VALUE_CH3_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC1_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC1_VALUE_CH3_M, 0x0);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC0_OVR_CH3_M, 0x1);
	hdmi_write_bits(hdmi_phy->regs, REG_QEC_OVERWRITE_CFG_CH3, REG_CFG_QEC0_VALUE_CH3_M, 0x1f);
}

static void phy_hw_frl_clk_set(const struct hdmitx_phy *hdmi_phy, u32 rate)
{
	u32 m = 0;
	u32 n = 0;
	u32 x = 0;

	get_frl_frac_param(rate, &m, &n, &x);
	hdmi_write_bits(hdmi_phy->regs, REG_FDIVMANUAL, REG_I_MANUAL_EN_M, 0xe);

	hdmi_write_bits(hdmi_phy->regs, REG_FDIVMANUAL, REG_I_MDIV_M, m);

	hdmi_write_bits(hdmi_phy->regs, REG_FDIVSTAT1, REG_I_FDIV_IN_N_M, n);
	hdmi_write_bits(hdmi_phy->regs, REG_FDIVSTAT1, REG_I_FDIV_IN_X_M, x);

	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M);
	hdmi_set(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M); /* need confirm */
	udelay(1);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M);

	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M);
	hdmi_clr(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M);
	hdmi_set(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M);

	switch (rate) {
	case FRL_RATE_3L_3G:
		hdmi_write_bits(hdmi_phy->regs, REG_FCDSTEPSET, REG_P_DIVN_H20_M, 0x2);
		break;
	case FRL_RATE_3L_6G:
	case FRL_RATE_4L_6G:
		hdmi_write_bits(hdmi_phy->regs, REG_FCDSTEPSET, REG_P_DIVN_H20_M, 0x1);
		break;
	case FRL_RATE_4L_8G:
	case FRL_RATE_4L_10G:
	case FRL_RATE_4L_12G:
		hdmi_write_bits(hdmi_phy->regs, REG_FCDSTEPSET, REG_P_DIVN_H20_M, 0x0);
		break;
	default:
		dpu_pr_err("unknow frl rate!");
		break;
	}

	hdmi_clr(hdmi_phy->regs, REG_FCDSTEPSET,  REG_FCDSTEPSET_UNUSED_M | REG_UP_SAMPLER_RATIO_SEL_M); /* need confirm */
}

static void phy_hw_fcg_params_set(const struct hdmitx_phy *hdmi_phy, const struct fcg_div_params *phy_fcg)
{
	u32 dlf_lock;
	u32 count = 0;

	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_B, 0x0);

	hdmi_clr(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_EN_M);
	hdmi_clr(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_DIF_EN_M);
	hdmi_clr(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_DITHER_EN_M);
	hdmi_clr(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_LOCK_EN_M);

	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_B,
		((phy_fcg->fcg_divm & 0xf) << 4) | ((phy_fcg->fcg_divp & 0x3) << 2) | 3); /* left shift 4 bit and 2 bit or 3 */
	stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_A,
		((phy_fcg->fcg_postdiv & 0x3) << 6) | 0x3); /* left shift 6 bit */
	if (g_aphy_cs_8_debug)
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_A,
				((phy_fcg->fcg_postdiv & 0x3) << 6) | (g_aphy_cs_8_offset[APHY_OFFSET_A] & 0x1f));

	hdmi_write_bits(hdmi_phy->regs, REG_FCGCNT, REG_P_TMDS_CNT_VAL_M, phy_fcg->tmds_cnt_val);
	hdmi_write_bits(hdmi_phy->regs, REG_FCGCNT, REG_P_CNT1_TARGET_M, phy_fcg->cnt1_target);
	hdmi_write_bits(hdmi_phy->regs, REG_FCGPARAM, REG_P_LOCK_CNT_M, 0x3);
	hdmi_write_bits(hdmi_phy->regs, REG_FCGPARAM, REG_P_LOCK_TH_M, phy_fcg->lock_th);
	hdmi_write_bits(hdmi_phy->regs, REG_FCGPARAM, REG_P_KI_M, phy_fcg->ki);
	hdmi_write_bits(hdmi_phy->regs, REG_FCGPARAM, REG_P_LOCK_MODE_M, 0x0);

	hdmi_set(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_EN_M);
	hdmi_set(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_DIF_EN_M);
	hdmi_set(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_DITHER_EN_M);
	hdmi_set(hdmi_phy->regs, REG_FCGSET, REG_P_FCG_LOCK_EN_M);

	dump_aphy_cs89_values(hdmi_phy, APHY_CS_8);

	do {
		dlf_lock = hdmi_read_bits(hdmi_phy->regs, REG_FCGSTATE, REG_O_DLF_LOCK_M);
		if (dlf_lock) {
			dpu_pr_info("fcg lock successful: lock = %u", dlf_lock);
			break;
		}

		count++;
		msleep(1);

		if (count >= 300) { /* if count over or equal to 300, loack failed */
			dpu_pr_err("fcg lock failed: lock = %u", dlf_lock);
			break;
		}
	} while (1);
}

static void phy_hw_tmds_ssc_set(const struct hdmitx_phy *hdmi_phy,
	bool enable, u16 mod_n, u16 mod_d)
{
	if (enable) {
		stb_writeb(hdmi_phy, APHY_CS_8, APHY_OFFSET_D, 0xFD);
		/* MOD_N MOD_T */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET3, REG_MOD_LEN_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET3, REG_MOD_T_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET3, REG_MOD_N_M, mod_n);
		/* MOD_D */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET4, REG_MOD_D_M, mod_d);
		/* FDIV init */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M, 0x0);
		/* FDIV init */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x1);
		udelay(1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M, 0x0);
		udelay(1); /* 150 nsec  */
		/* FDIV control */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M, 0x0);
		/* FDIV control */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M, 0x0);
		/* FDIV control */
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_SDM_M, 0x1);
	} else {
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x1);
		udelay(1);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_INIT_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_CTRL_M, 0x0);
		hdmi_write_bits(hdmi_phy->regs, REG_FDIVSET0, REG_EN_MOD_M, 0x0);
	}
}

void hal_phy_hw_oe_enable(const struct hdmitx_phy *phy, bool enable)
{
	if ((phy == NULL) || (phy->regs == NULL)) {
		dpu_pr_err("null ptr!");
		return;
	}

	hdmi_write_bits(phy->regs, REG_HDMI_OE_CFG, REG_CFG_HDMI_OE_CH3_M, enable);
	hdmi_write_bits(phy->regs, REG_HDMI_OE_CFG, REG_CFG_HDMI_OE_CH2_M, enable);
	hdmi_write_bits(phy->regs, REG_HDMI_OE_CFG, REG_CFG_HDMI_OE_CH1_M, enable);
	hdmi_write_bits(phy->regs, REG_HDMI_OE_CFG, REG_CFG_HDMI_OE_CH0_M, enable);
	/* Turn on the PHY to wait until VSYNC stops outputting data after OE is turned off. */
	if (enable)
		hdmi_write_bits(phy->regs, REG_HDMI_OE_CFG, REG_CFG_OE_SYNC_EN_M, true);
}

static void phy_tmds_spec_set(const struct hdmitx_phy *hdmi_phy, u32 tmds_clk, u8 pcb_len)
{
	struct tmds_spec_params *tmds = NULL;

	tmds = get_tmds_spec_params(tmds_clk, pcb_len);
	phy_hw_tmds_aphy_spec_set(hdmi_phy, tmds);
	phy_hw_tmds_dphy_spec_set(hdmi_phy, tmds);
}

static void phy_frl_spec_set(const struct hdmitx_phy *hdmi_phy, u32 frl_rate, u8 pcb_len)
{
	u32 rate;
	struct frl_spec_params *frl = NULL;

	switch (frl_rate) {
	case FRL_RATE_3L_3G:
		rate = RATE_3G;
		break;
	case FRL_RATE_3L_6G:
	case FRL_RATE_4L_6G:
		rate = RATE_6G;
		break;
	case FRL_RATE_4L_8G:
		rate = RATE_8G;
		break;
	case FRL_RATE_4L_10G:
		rate = RATE_10G;
		break;
	case FRL_RATE_4L_12G:
		rate = RATE_12G;
		break;
	default:
		rate = RATE_3G;
		break;
	}

	frl = get_frl_spec_params(rate, pcb_len);
	phy_hw_frl_aphy_spec_set(hdmi_phy, frl);
	phy_hw_frl_dphy_spec_set(hdmi_phy, frl);
}

static void phy_tmds_clock_set(const struct hdmitx_phy *hdmi_phy, const struct phy_tmds *tmds)
{
	phy_hw_tmds_clk_set(hdmi_phy, tmds);
	phy_hw_fcon_param_set(hdmi_phy, tmds->tmds_clk);
}

static void phy_frl_clock_set(const struct hdmitx_phy *hdmi_phy, u32 frl_rate)
{
	phy_hw_frl_clk_set(hdmi_phy, frl_rate);
	phy_hw_fcon_param_set(hdmi_phy, frl_rate);
}

static void phy_dcc_qec_set(const struct hdmitx_phy *hdmi_phy)
{
	phy_hw_ch0_dcc_qec_set(hdmi_phy);
	phy_hw_ch1_dcc_qec_set(hdmi_phy);
	phy_hw_ch2_dcc_qec_set(hdmi_phy);
	phy_hw_ch3_dcc_qec_set(hdmi_phy);
}

static void phy_tmds_ssc_set(const struct hdmitx_phy *hdmi_phy, const struct phy_ssc *ssc)
{
	u16 mod_n = 0;
	u16 mod_d = 0;

	if (ssc->tmds_clk > 340000) { /* 340000 means 340M clock freq */
		dpu_pr_err("more than 340M clock freq have no ssc!");
		return;
	}

	if (ssc->enable) {
		get_tmds_ssc_params(ssc, &mod_n, &mod_d);
		phy_hw_tmds_ssc_set(hdmi_phy, true, mod_n, mod_d);
	} else {
		phy_hw_tmds_ssc_set(hdmi_phy, false, 0, 0);
	}
}

void hal_phy_tmds_set(struct hdmitx_phy *phy, const struct phy_tmds *tmds)
{
	u32 tmds_clk;
	u32 pixel_clk;
	struct phy_ssc ssc = {};
	u8 pcb_len;

	if (phy == NULL || tmds == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	pcb_len = tmds->pcb_len;
	tmds_clk = tmds->tmds_clk;
	pixel_clk = tmds->pixel_clk;
	phy->mode = PHY_SET_MODE_TMDS;

	/* reset phy */
	phy_hw_reset(phy);
	/* SET PLL  */
	phy_hw_init(phy, pixel_clk, tmds_clk);
	/* set phy tmds clock */
	phy_tmds_clock_set(phy, tmds);
	/* dcc qec */
	phy_dcc_qec_set(phy);
	/* scc set */
	ssc.enable = false;
	phy_tmds_ssc_set(phy, &ssc);
	/* set spec */
	phy_tmds_spec_set(phy, tmds_clk, pcb_len);
	/* data  path enable */
	phy_hw_data_channel_enable(phy, 0);
}

void hal_phy_frl_set(struct hdmitx_phy *phy, const struct phy_frl *frl)
{
	u32 frl_rate;
	u8 pcb_len;

	if (phy == NULL || frl == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	pcb_len = frl->pcb_len;
	frl_rate = frl->frl_rate;
	phy->mode = PHY_SET_MODE_FRL;

	/* reset phy */
	phy_hw_reset(phy);
	/* SET PLL */
	phy_hw_init(phy, 0, frl_rate);
	/* set phy frl clock */
	phy_frl_clock_set(phy, frl_rate);
	/* DCC QEC */
	phy_dcc_qec_set(phy);
	/* set spec */
	phy_frl_spec_set(phy, frl_rate, pcb_len);
	/* data  path enable */
	phy_hw_data_channel_enable(phy, frl_rate);
}

void hal_phy_frl_tmds_clock_set(const struct hdmitx_phy *phy, const struct phy_frl_tmds_clk *frl_tmds_clk)
{
	struct fcg_div_params fcg = {};

	if (phy == NULL || frl_tmds_clk == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	/* calculate fcg param */
	get_disable_dsc_fcg_params(&fcg, frl_tmds_clk);

	/* set fcg param */
	phy_hw_fcg_params_set(phy, &fcg);
}

void hal_phy_ssc_set(const struct hdmitx_phy *phy, const struct phy_ssc *ssc)
{
	if (phy == NULL || ssc == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	phy_tmds_ssc_set(phy, ssc);
}

bool hal_phy_is_on(const struct hdmitx_phy *phy)
{
	u32 val;

	if ((phy == NULL) || (phy->regs == NULL)) {
		dpu_pr_err("null ptr!");
		return false;
	}

	val = hdmi_readl(phy->regs, REG_HDMI_OE_CFG);
	if ((val & REG_CFG_HDMI_OE_CH0_M) &&
		(val & REG_CFG_HDMI_OE_CH1_M) &&
		(val & REG_CFG_HDMI_OE_CH2_M) &&
		(val & REG_CFG_HDMI_OE_CH3_M))
		return true;
	return false;
}

void hal_phy_reset(const struct hdmitx_phy *phy)
{
	if (phy == NULL) {
		dpu_pr_err("null ptr!");
		return;
	}

	phy_hw_reset(phy);
}

u32 hal_phy_get_pixel_clk(const struct hdmitx_phy *phy)
{
	if ((phy == NULL) || (phy->regs == NULL)) {
		dpu_pr_err("Null ptr err!");
		return 0;
	}
	hdmi_writel(phy->regs, REG_CLKSET, 0x40);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 0);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	hdmi_writel(phy->regs, REG_FDSRCFREQ, 0x1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 0x1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	msleep(1);
	return hdmi_read_bits(phy->regs, REG_FDSRCRES, REG_SRC_CNT_OUT_M);
}

u32 hal_phy_get_tmds_clk(const struct hdmitx_phy *phy)
{
	if ((phy == NULL) || (phy->regs == NULL)) {
		dpu_pr_err("Null ptr err!");
		return 0;
	}
	hdmi_writel(phy->regs, REG_CLKSET, 0x43);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 0);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	hdmi_writel(phy->regs, REG_FDSRCFREQ, 0x1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	msleep(1);
	return hdmi_read_bits(phy->regs, REG_FDSRCRES, REG_SRC_CNT_OUT_M);
}

u32 hal_phy_get_frl_clk(const struct hdmitx_phy *phy)
{
	if ((phy == NULL) || (phy->regs == NULL)) {
		dpu_pr_err("Null ptr err!");
		return 0;
	}
	hdmi_writel(phy->regs, REG_CLKSET, 0x42);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 0);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	hdmi_writel(phy->regs, REG_FDSRCFREQ, 0x1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_ENABLE_M, 1);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_CNT_M, 0x2);
	hdmi_write_bits(phy->regs, REG_FDSRCPARAM, REG_SRC_LOCK_VAL_M, 0x4);

	msleep(1);
	return hdmi_read_bits(phy->regs, REG_FDSRCRES, REG_SRC_CNT_OUT_M);
}
