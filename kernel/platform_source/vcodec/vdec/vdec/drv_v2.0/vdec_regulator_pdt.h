/*
 * vdec_regulator_pdt.h
 *
 * This is for vdec regulator
 *
 * Copyright (c) 2023-2023 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef VDEC_REGULATOR_PDT_H
#define VDEC_REGULATOR_PDT_H
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>

#define VDEC_CLOCK_NAME "clk_vdec"
#define VDEC_ACLOCK_NAME "aclk_vdec"
#define VDEC_CLK_RATE "clk_rate"
#define VDEC_TRANSITION_CLK_NAME "transi_clk_rate"
#define MEDIA_DEFAULT_CLK_NAME "default_clk_rate"

#define vdec_rd_vreg(base, reg, dat) \
	do { \
		(dat) = readl(((uint8_t *)(base) + (reg))); \
	} while (0)

#define vdec_wr_vreg(base, reg, dat) \
	do { \
		writel((dat), ((uint8_t *)(base) + (reg))); \
	} while (0)

enum {
	MEDIA_REGULATOR,
	VDEC_REGULATOR,
	SMMU_TCU_REGULATOR,
	MAX_REGULATOR,
};

typedef enum {
	VDEC_CLK_RATE_LOWER = 0,
	VDEC_CLK_RATE_LOW,
	VDEC_CLK_RATE_NORMAL,
	VDEC_CLK_RATE_HIGH,
#ifdef HIGHER_FREQUENCY_SUPPORT
	VDEC_CLK_RATE_HIGHER,
#endif
	VDEC_CLK_RATE_MAX,
} clk_rate_e;

struct vdec_regulator {
	struct clk *clk_vdec;
	struct clk *aclk_vdec;
	struct regulator *regulators[MAX_REGULATOR];
	uint32_t clk_values[VDEC_CLK_RATE_MAX];
	uint32_t transi_clk_rate;
	uint32_t default_clk_rate;
};

struct vdec_regulator_pdt_ops {
	int32_t (*get_clk)(struct device *dev, struct vdec_regulator *regulator_info);
	int32_t (*map_reg_info)(void);
	void (*unmap_reg_info)(void);
	int32_t (*open_clk)(void);
	void (*close_clk)(void);
	int32_t (*vdec_global_reset)(void);
	bool (*transition_clk_enable)(void);
};
struct vdec_regulator_pdt_ops *get_vdec_regulator_plat_ops(void);
#endif

