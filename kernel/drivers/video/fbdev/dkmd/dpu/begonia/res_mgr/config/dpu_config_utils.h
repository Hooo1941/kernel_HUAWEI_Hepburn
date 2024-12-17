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

#ifndef DISP_CONFIG_H
#define DISP_CONFIG_H

#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <dpu/soc_dpu_define.h>
#include <dpu/dpu_lbuf.h>
#include <platform_include/display/dkmd/dkmd_res_mgr.h>
#include <platform_include/display/dkmd/dkmd_dpu.h>
#include "dksm_debug.h"

#ifndef max_of_three
#define max_of_three(a, b, c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))
#endif

#ifndef array_len
#define array_len(array) (sizeof(array) / sizeof((array)[0]))
#endif

#define DTS_OFFLINE_SCENE_ID_MAX 3

#define PERCENT 100
#define HZ_TO_US 1000000

struct dpu_config {
	char __iomem *ip_base_addrs[DISP_IP_MAX];
	uint32_t lbuf_size;
	union dpu_version version;
	uint32_t xres;
	uint32_t yres;
	uint32_t offline_scene_id_count;
	uint32_t offline_scene_ids[DTS_OFFLINE_SCENE_ID_MAX];

	uint32_t pmctrl_dvfs_enable;
	struct clk *clk_gate_edc;
};

struct lbuf_node_config_info {
	int32_t hw_node_id;
	uint32_t max_lb;
};

struct lbuf_pool_config_info {
	uint32_t node_size;
	struct lbuf_node_config_info *node_config_info;
};

struct lbuf_pool_node_config_info {
	uint32_t pool_size;
	struct lbuf_pool_config_info *lb_cfg_info;
};

struct lbuf_conn_pool_config_info {
	int32_t hw_pool_id;
	uint32_t node_size;
	int32_t *hw_node_ids;
};

enum lbuf_conn_pool_config_info_index {
	CONN_POOL_INDEX_0,
	CONN_POOL_INDEX_1,
	CONN_POOL_INDEX_2,
	CONN_POOL_INDEX_3,
	CONN_POOL_INDEX_4,
	CONN_POOL_INDEX_5,
	CONN_POOL_INDEX_6,
	CONN_POOL_INDEX_7,
	CONN_POOL_INDEX_MAX,
};

struct lbuf_conn_pool_node_config_info {
	uint32_t conn_pool_index;
	struct lbuf_conn_pool_config_info *lb_conn_pool_cfg_info;
};

struct lbuf_conn_config_info {
	int32_t cur_opr;
	int32_t nxt_opr;
	uint32_t conn_pool_size;
	struct lbuf_conn_pool_node_config_info *lb_conn_pool_node_cfg_info;
};

struct dbuf_config_info {
	uint32_t addr_offset[DPU_DBUF_REG_NUM];
	uint32_t value[DPU_DBUF_REG_NUM];
};

extern struct dpu_config g_dpu_config_data;

int32_t dpu_init_config(struct platform_device *device);

static inline bool dpu_config_enable_pmctrl_dvfs(void)
{
	return (g_dpu_config_data.pmctrl_dvfs_enable == 1);
}

static inline char __iomem *dpu_config_get_ip_base(uint32_t ip)
{
	if (ip >= DISP_IP_MAX)
		return NULL;

	return g_dpu_config_data.ip_base_addrs[ip];
}

static inline uint32_t dpu_config_get_lbuf_size(void)
{
	return g_dpu_config_data.lbuf_size;
}

static inline uint64_t dpu_config_get_version_value(void)
{
	return g_dpu_config_data.version.value;
}

static inline void dpu_config_set_screen_info(uint32_t xres, uint32_t yres)
{
	if (g_dpu_config_data.xres == 0 && g_dpu_config_data.yres == 0) {
		g_dpu_config_data.xres = xres;
		g_dpu_config_data.yres = yres;
	}
}

static inline void dpu_config_get_screen_info(uint32_t *xres, uint32_t *yres)
{
	*xres = g_dpu_config_data.xres;
	*yres = g_dpu_config_data.yres;
}

static inline uint32_t* dpu_config_get_offline_scene_ids(uint32_t *count)
{
	*count = g_dpu_config_data.offline_scene_id_count;
	return g_dpu_config_data.offline_scene_ids;
}

uint32_t *dpu_config_get_scf_lut_addr_tlb(uint32_t *length);
uint32_t *dpu_config_get_arsr_lut_addr_tlb(uint32_t *length);
void dpu_config_dvfs_vote_exec(uint64_t clk_rate);
uint32_t dpu_config_get_version(void);

struct lbuf_pool_node_config_info *dpu_get_lb_pool_node_config_info(void);
struct lbuf_conn_pool_node_config_info *dpu_get_lb_conn_pool_node_config_info(int32_t cur_opr,
	int32_t nxt_opr, uint32_t *pool_size);
uint32_t dpu_get_pool_a_lb_max_size(void);
uint32_t dpu_get_pool_b_srot_lb_num(void);
uint32_t dpu_get_pool_b_lb_constraint(uint32_t lb_num);
uint32_t dpu_is_support_ebit(void);
void dpu_print_dvfs_vote_status(void);
void dpu_skip_intra_frame_idle_dvfs(void);
struct dbuf_config_info *dbuf_get_config_info(uint32_t xres, uint32_t yres,
	uint32_t dsc_en, uint32_t dsc_out_width, uint32_t fps);

#endif /* DISP_CONFIG_H */
