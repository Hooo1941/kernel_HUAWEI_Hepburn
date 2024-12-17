/*
 * asp_codec_comm.h -- codec driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __ASP_CODEC_COMM_H__
#define __ASP_CODEC_COMM_H__

#include <sound/soc.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>

#define CLEAR_FIFO_DELAY_LEN_US 1

enum asp_codec_platform_type {
	FPGA_PLATFORM = 0,
	UDP_PLATFORM,
	PLATFORM_CNT,
};

enum sample_rate_index {
	SAMPLE_RATE_INDEX_8K  = 0x0,
	SAMPLE_RATE_INDEX_16K = 0x1,
	SAMPLE_RATE_INDEX_32K = 0x2,
	SAMPLE_RATE_INDEX_48K = 0x3,
	SAMPLE_RATE_INDEX_96K = 0x4,
};

struct codec_pm {
	bool have_dapm;
	bool asp_pd;
	bool pm_runtime_support;
};

struct codec_resource {
	struct regulator *asp_regulator;
	struct clk *asp_subsys_clk;
	struct clk *asp_codec_clk;
	struct pinctrl *pctrl;
	struct pinctrl_state *pin_default;
	struct pinctrl_state *pin_idle;
};

struct codec_ops {
	void (*priv_data_init)(struct snd_soc_component *codec);
	void (*priv_data_deinit)(struct snd_soc_component *codec);
	void (*asp_reg_init)(struct snd_soc_component *codec);
	void (*reset_asp_codec)(struct snd_soc_component *codec);
	void (*shutdown)(struct platform_device *);
	void (*fpga_pinctrl_init)(struct snd_soc_component *codec);
};

struct asp_codec_reg_page {
	void __iomem *reg_base_addr;
	unsigned int page_reg_begin;
	unsigned int page_reg_end;
	bool is_virtual_reg;
	bool is_asp_reg_type;
};

struct codec_data {
	struct snd_soc_component *codec;
	struct codec_pm pm;
	struct codec_resource resource;
	struct codec_ops ops;
	spinlock_t lock;
	enum asp_codec_platform_type platform_type;
	unsigned int *v_codec_reg;
	unsigned int v_codec_num;
	struct asp_codec_reg_page *platform_reg_array;
	unsigned int platform_reg_size;
};

#endif

