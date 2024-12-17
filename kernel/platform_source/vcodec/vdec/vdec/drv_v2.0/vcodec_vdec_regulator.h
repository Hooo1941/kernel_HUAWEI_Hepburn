/*
 * vcodec_vdec_refulator.h
 *
 * This is for vdec regulator
 *
 * Copyright (c) 2019-2023 Huawei Technologies CO., Ltd.
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

#ifndef VCODEC_VDEC_REGULATOR_H
#define VCODEC_VDEC_REGULATOR_H

#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/iommu/mm_iommu.h>
#include "vcodec_types.h"
#include "vfmw_ext.h"
#include "smmu_regs.h"
#include "vdec_regulator_pdt.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#define IO_REMAP_NO_CACHE ioremap
#else
#define IO_REMAP_NO_CACHE ioremap_nocache
#endif

typedef struct {
	clk_rate_e clk_rate;
	uint64_t lower_limit;
	uint64_t upper_limit;
} perf_load_range;

typedef struct {
	uint64_t load;
	clk_rate_e base_rate;
	perf_load_range load_range_map[VDEC_CLK_RATE_MAX];
} performance_params_s;

typedef struct {
	struct file *file;
	clk_rate_e base_rate;
	uint64_t load;
} performance_info;

typedef struct {
	/* The current frequency may be invalid when a power-off operation has been performed.
	   The frequency needs to be reset. */
	int32_t clk_flag;
	clk_rate_e current_clk; /* Current decoder frequency */
	clk_rate_e dynamic_clk; /* Set by the performance node. (/sys/class/vdec_class/hi_vdec/vdec_freq) */
	clk_rate_e static_clk;  /* Set this parameter based on the load in user mode. */
} vdec_clk_ctrl;

typedef struct {
	struct mutex vdec_plat_mutex;
	struct vdec_regulator regulator_info;
	vdec_dts dts_info;
	int32_t power_flag;
	int32_t plt_init;
	vdec_clk_ctrl clk_ctrl;
	struct smmu_tbu_info smmu_info;
	struct device *dev;
} vdec_plat;

vdec_plat *vdec_plat_get_entry(void);

int32_t vdec_plat_init(struct platform_device *plt_dev);
int32_t vdec_plat_deinit(void);

int32_t vdec_plat_regulator_enable(void);
void vdec_plat_regulator_disable(void);

void vdec_plat_set_dynamic_clk_rate(clk_rate_e clk_rate);
void vdec_plat_get_dynamic_clk_rate(clk_rate_e *clk_rate);
void vdec_plat_set_static_clk_rate(clk_rate_e clk_rate);
void vdec_plat_get_static_clk_rate(clk_rate_e *clk_rate);
void vdec_plat_get_target_clk_rate(clk_rate_e *clk_rate);
int32_t vdec_plat_regulator_set_clk_rate(clk_rate_e dst_clk_type);
int32_t vdec_plat_regulator_global_reset(void);
int32_t vdec_plat_regulator_map_reg(void);
void vdec_plat_regulator_unmap_reg(void);
#endif

