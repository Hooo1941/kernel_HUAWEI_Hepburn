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

#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include "res_mgr.h"
#include "dkmd_object.h"
#include "dvfs.h"
#include "dpu_dvfs.h"
#include "dpu_config_utils.h"

#define PERF_INIT_VALUE 0xFFFF
#define DVFS_PMCTRL_PERI_CTRL4_TEMPERATURE_MASK GENMASK(27, 26)
#define DVFS_PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT 26
#define DVFS_MIDIA_PERI_CTRL4 0x350

// g_dvfs_mgr protected by sem lock in dvfs.c
struct dpu_dvfs *g_dvfs_mgr = NULL;

uint32_t dpu_dvfs_check_low_temperature(void)
{
	uint32_t perictrl4;
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	dpu_check_and_return(!pmctrl_base, false, err, "pmctrl_base is NULL!\n");
	perictrl4 = inp32(pmctrl_base + DVFS_MIDIA_PERI_CTRL4);
	perictrl4 &= DVFS_PMCTRL_PERI_CTRL4_TEMPERATURE_MASK;
	perictrl4 = (perictrl4 >> DVFS_PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT);
	dpu_pr_debug("Get current temperature: %d\n", perictrl4);
	return perictrl4;
}

static uint64_t dpu_dvfs_get_final_vote_freq(uint32_t comp_index, struct intra_frame_dvfs_info *info)
{
	uint32_t i = 0;
	uint64_t final_total_freq = 0;

	g_dvfs_mgr->user_info[comp_index].vote_freq_info = info->vote_freq_info;
	for (; i < DEVICE_COMP_MAX_COUNT; i++) {
		final_total_freq += g_dvfs_mgr->user_info[i].vote_freq_info.sdma_freq;
		dpu_pr_debug("sdma freq[%d] is %llu", i, g_dvfs_mgr->user_info[i].vote_freq_info.sdma_freq);
	}

	for (i = 0; i < DEVICE_COMP_MAX_COUNT; i++) {
		final_total_freq = max(final_total_freq, g_dvfs_mgr->user_info[i].vote_freq_info.current_total_freq);
		dpu_pr_debug("current total freq[%d] is %llu", i, g_dvfs_mgr->user_info[i].vote_freq_info.current_total_freq);
	}

	dpu_pr_debug("final total freq is %llu", final_total_freq);

	return final_total_freq;
}

static uint32_t dpu_dvfs_get_vote_level(void)
{
	uint32_t i = 0;
	uint32_t voted_level = 0;
	uint32_t max_voted_level = 0;

	for (; i < DEVICE_COMP_MAX_COUNT; ++i) {
		voted_level = g_dvfs_mgr->user_info[i].voted_level;
		dpu_pr_debug("voted_level=%u max_voted_level=%u", voted_level, max_voted_level);
		max_voted_level = voted_level > max_voted_level ? voted_level : max_voted_level;
	}

	return max_voted_level;
}

// debug for switch to other dvfs type
static void dpu_dvfs_debug_vote_level(uint32_t comp_index, uint32_t perf_level,
	bool need_config_qos, bool direct_vote)
{
	uint32_t i = 0;
	uint32_t vote_level = 0;
	if (comp_index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("comp index out of range!");
		return;
	}

	g_dvfs_mgr->user_info[comp_index].voted_level = perf_level;

	if (perf_level == DPU_CORE_LEVEL_OFF) {
		for (; i < (uint32_t)DEVICE_COMP_MAX_COUNT; ++i)
			g_dvfs_mgr->user_info[i].voted_level = DPU_CORE_LEVEL_OFF;
		vote_level = DPU_CORE_LEVEL_OFF;
	} else {
		if (g_debug_dpu_perf_level == 0 || g_debug_dpu_perf_level > DPU_PERF_LEVEL_MAX)
			vote_level = direct_vote ? perf_level : dpu_dvfs_get_vote_level();
		else
			vote_level = g_debug_dpu_perf_level;
	}

	if (vote_level == g_dvfs_mgr->voted_level)
		return;

	if (g_debug_dvfs_type == DVFS_INTER_HW_TYPE)
		dpu_dvfs_direct_process(vote_level, need_config_qos);
	else if (g_debug_dvfs_type == DVFS_INTER_SW_TYPE)
		dpu_legacy_inter_frame_dvfs_vote(vote_level, need_config_qos);

	g_dvfs_mgr->voted_level = vote_level;
}

void dpu_dvfs_inter_frame_vote(uint32_t comp_index, struct intra_frame_dvfs_info *info)
{
	dpu_pr_debug("+");

	if (unlikely(info == NULL) || unlikely(g_dvfs_mgr == NULL))
		return;

	if (unlikely(!is_dpu_dvfs_enable()))
		return;

	if (comp_index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("comp index out of range!");
		return;
	}

	down(&g_dvfs_mgr->sem);

	info->perf_level = dpu_config_get_perf_level(dpu_dvfs_get_final_vote_freq(comp_index, info));
	dpu_pr_debug("perf_level=%u", info->perf_level);

	if((dpu_dvfs_check_low_temperature() != 0) && (info->perf_level == DPU_PERF_LEVEL_MAX)) {
		dpu_pr_debug("low temp and perf_level=%u", DPU_PERF_LEVEL_MAX);
		info->perf_level = DPU_PERF_LEVEL_MAX - 1;
	}
	// debug for switch to other dvfs type
	if (g_debug_dvfs_type != DVFS_INTRA_TYPE) {
		dpu_dvfs_debug_vote_level(comp_index, info->perf_level, true, false);
		up(&g_dvfs_mgr->sem);
		return;
	}

	if (g_debug_dpu_perf_level != 0 && g_debug_dpu_perf_level <= DPU_PERF_LEVEL_MAX)
		info->perf_level = g_debug_dpu_perf_level;

	g_dvfs_mgr->user_info[comp_index].voted_level = info->perf_level;
	// if copybit vote LEVEL_1, do not need vote perf level
	if (info->perf_level == DPU_CORE_LEVEL_ON) {
		up(&g_dvfs_mgr->sem);
		return;
	}

	dpu_dvfs_inter_process(info->perf_level, true);
	g_dvfs_mgr->inter_dvfs_locked_count++;

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

void dpu_dvfs_intra_frame_vote(uint32_t comp_index, struct intra_frame_dvfs_info *info,
	bool need_direct_vote)
{
	bool inter_locked = false;

	dpu_pr_debug("+");

	dpu_check_and_no_retval(!info, err, "info is NULL!\n");

	if (unlikely(info == NULL) || unlikely(g_dvfs_mgr == NULL))
		return;

	if (unlikely(!is_dpu_dvfs_enable()))
		return;

	if (comp_index >= DEVICE_COMP_MAX_COUNT) {
		dpu_pr_err("comp index out of range!");
		return;
	}

	down(&g_dvfs_mgr->sem);

	info->perf_level = dpu_config_get_perf_level(dpu_dvfs_get_final_vote_freq(comp_index, info));
	dpu_pr_debug("perf_level=%u", info->perf_level);

	// debug for switch to other dvfs type
	if (g_debug_dvfs_type != DVFS_INTRA_TYPE) {
		dpu_dvfs_debug_vote_level(comp_index, info->perf_level, true, false);
		up(&g_dvfs_mgr->sem);
		return;
	}

	if (g_debug_dpu_perf_level != 0 && g_debug_dpu_perf_level <= DPU_PERF_LEVEL_MAX)
		info->perf_level = g_debug_dpu_perf_level;

	if (need_direct_vote || !info->is_supported_intra_dvfs) {
		dpu_dvfs_inter_process(info->perf_level, false);
		up(&g_dvfs_mgr->sem);
		return;
	}

	dpu_pr_debug("inter_dvfs_locked_count=%d", g_dvfs_mgr->inter_dvfs_locked_count);

	if (g_dvfs_mgr->inter_dvfs_locked_count != 0)
		inter_locked = true;

	dpu_dvfs_intra_process(info, inter_locked);

	if ((inter_locked) && (g_dvfs_mgr->inter_dvfs_locked_count != 0))
		g_dvfs_mgr->inter_dvfs_locked_count--;

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

void dpu_dvfs_enable_core_clock(bool need_config_qos)
{
	dpu_pr_debug("+");

	if (unlikely(g_dvfs_mgr == NULL)) {
		dpu_pr_err("g_dvfs_mgr is NULL");
		return;
	}

	if (!is_dpu_dvfs_enable())
		return;

	down(&g_dvfs_mgr->sem);

	dpu_enable_core_clock(need_config_qos);

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
	return;
}

void dpu_dvfs_disable_core_clock(void)
{
	dpu_pr_debug("+");

	if (unlikely(g_dvfs_mgr == NULL)) {
		dpu_pr_err("g_dvfs_mgr is NULL");
		return;
	}

	if (!is_dpu_dvfs_enable())
		return;

	down(&g_dvfs_mgr->sem);
	dpu_disable_core_clock();
	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

/*
 * Note: direct vote dvfs not consider level descending control, must be used carefully
 * Usually used in power on/off
*/
void dpu_dvfs_direct_vote(uint32_t comp_index, uint32_t perf_level, bool need_config_qos)
{
	dpu_pr_debug("+");

	dpu_pr_debug("perf_level=%u", perf_level);

	if (unlikely(g_dvfs_mgr == NULL)) {
		dpu_pr_err("g_dvfs_mgr is NULL");
		return;
	}

	down(&g_dvfs_mgr->sem);

	/* for first vote */
	if (unlikely(g_dvfs_mgr->voted_level == PERF_INIT_VALUE))
		g_dvfs_mgr->voted_level = DPU_CORE_LEVEL_OFF;

	// debug for switch to other dvfs type
	if (g_debug_dvfs_type != DVFS_INTRA_TYPE) {
		dpu_dvfs_debug_vote_level(comp_index, perf_level, need_config_qos, true);
		up(&g_dvfs_mgr->sem);
		return;
	}

	if (g_debug_dpu_perf_level != 0 && g_debug_dpu_perf_level <= DPU_PERF_LEVEL_MAX)
		perf_level = perf_level != DPU_CORE_LEVEL_OFF ? g_debug_dpu_perf_level : DPU_CORE_LEVEL_OFF;

	dpu_dvfs_direct_process(perf_level, need_config_qos);

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

void dpu_dvfs_reset_comp_vote(uint32_t comp_index)
{
	dpu_pr_debug("+");

	if (unlikely(!g_dvfs_mgr)) {
		dpu_pr_err("g_dvfs_mgr is NULL");
		return;
	}

	if (unlikely(comp_index >= DEVICE_COMP_MAX_COUNT)) {
		dpu_pr_err("comp_index=%u is out of range", comp_index);
		return;
	}

	down(&g_dvfs_mgr->sem);

	// copybit finish need reset perf level, unless vote LEVEL_1 before
	if (g_dvfs_mgr->user_info[comp_index].voted_level != DPU_CORE_LEVEL_ON) {
		dpu_dvfs_inter_process(DPU_CORE_LEVEL_ON, true);
		if (g_dvfs_mgr->inter_dvfs_locked_count != 0)
			g_dvfs_mgr->inter_dvfs_locked_count--;
		else
			dpu_pr_warn("dvfs lock count is %d", g_dvfs_mgr->inter_dvfs_locked_count);
	}

	g_dvfs_mgr->user_info[comp_index].voted_level = DPU_CORE_LEVEL_OFF;
	g_dvfs_mgr->user_info[comp_index].vote_freq_info.current_total_freq = 0;
	g_dvfs_mgr->user_info[comp_index].vote_freq_info.sdma_freq = 0;

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

void dpu_dvfs_reset_vote(uint32_t comp_index)
{
	dpu_pr_debug("+");

	if (unlikely(!g_dvfs_mgr)) {
		dpu_pr_err("g_dvfs_mgr is NULL");
		return;
	}

	if (unlikely(comp_index >= DEVICE_COMP_MAX_COUNT)) {
		dpu_pr_err("comp_index=%u is out of range", comp_index);
		return;
	}

	down(&g_dvfs_mgr->sem);

	g_dvfs_mgr->user_info[comp_index].voted_level = DPU_CORE_LEVEL_OFF;
	g_dvfs_mgr->user_info[comp_index].vote_freq_info.current_total_freq = 0;
	g_dvfs_mgr->user_info[comp_index].vote_freq_info.sdma_freq = 0;

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");
}

void dpu_dvfs_qos_qic_media1_config(uint32_t level)
{
	dpu_qos_qic_media1_config(level);
}

static void* dvfs_init(struct dpu_res_data *rm_data)
{
	struct dpu_dvfs *dvfs_mgr = NULL;

	dpu_check_and_return(!rm_data, NULL, err, "res_data is nullptr!");
	dvfs_mgr = (struct dpu_dvfs *)kzalloc(sizeof(*dvfs_mgr), GFP_KERNEL);
	dpu_check_and_return(!dvfs_mgr, NULL, err, "dvfs_mgr alloc failed!");

	sema_init(&dvfs_mgr->sem, 1);
	dvfs_mgr->voted_level = PERF_INIT_VALUE;
	dvfs_mgr->inter_dvfs_locked_count = 0;
	g_dvfs_mgr = dvfs_mgr;

	return dvfs_mgr;
}

static void dvfs_deinit(void *dvfs)
{
	struct dpu_dvfs *dvfs_mgr = (struct dpu_dvfs *)dvfs;

	dpu_check_and_no_retval(!dvfs_mgr, err, "dvfs_mgr is nullptr!");
	kfree(dvfs_mgr);
	g_dvfs_mgr = NULL;
	dvfs = NULL;
}

void dpu_res_register_dvfs(struct list_head *res_head)
{
	struct dpu_res_resouce_node *dvfs_node = NULL;

	dvfs_node = kzalloc(sizeof(*dvfs_node), GFP_KERNEL);
	if (unlikely(!dvfs_node))
		return;

	dvfs_node->res_type = RES_DVFS;
	atomic_set(&dvfs_node->res_ref_cnt, 0);
	dvfs_node->init = dvfs_init;
	dvfs_node->deinit = dvfs_deinit;
	dvfs_node->ioctl = NULL;

	list_add_tail(&dvfs_node->list_node, res_head);
}

void dpu_dvfs_decrease_count_locked(void)
{
	dpu_pr_debug("+");

	if (unlikely(g_dvfs_mgr == NULL))
		return;

	if (!is_dpu_dvfs_enable())
		return;

	if (g_debug_dvfs_type != DVFS_INTRA_TYPE)
		return;

	down(&g_dvfs_mgr->sem);

	if (g_dvfs_mgr->inter_dvfs_locked_count != 0)
		g_dvfs_mgr->inter_dvfs_locked_count--;
	else
		dpu_pr_warn("dvfs lock count is %d", g_dvfs_mgr->inter_dvfs_locked_count);

	dpu_pr_debug("inter_dvfs_locked_count=%u", g_dvfs_mgr->inter_dvfs_locked_count);

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");

	return;
}

void dpu_dvfs_increase_count_locked(void)
{
	dpu_pr_debug("+");

	if (unlikely(g_dvfs_mgr == NULL))
		return;

	if (!is_dpu_dvfs_enable())
		return;

	if (g_debug_dvfs_type != DVFS_INTRA_TYPE)
		return;

	down(&g_dvfs_mgr->sem);

	g_dvfs_mgr->inter_dvfs_locked_count++;
	dpu_pr_debug("inter_dvfs_locked_count=%u", g_dvfs_mgr->inter_dvfs_locked_count);

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");

	return;
}

void dpu_dvfs_set_inter_vote_index_count(uint32_t count)
{
	dpu_pr_debug("+");

	if (unlikely(g_dvfs_mgr == NULL))
		return;

	if (!is_dpu_dvfs_enable())
		return;

	if (g_debug_dvfs_type != DVFS_INTRA_TYPE)
		return;

	down(&g_dvfs_mgr->sem);

	g_dvfs_mgr->inter_dvfs_locked_count = count;
	dpu_pr_debug("inter_dvfs_locked_count=%u", g_dvfs_mgr->inter_dvfs_locked_count);

	up(&g_dvfs_mgr->sem);

	dpu_pr_debug("-");

	return;
}
