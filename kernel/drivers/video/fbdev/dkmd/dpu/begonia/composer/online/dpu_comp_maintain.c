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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "dpu_config_utils.h"
#include "dpu_comp_smmu.h"
#include "dpu_comp_mgr.h"
#include "gfxdev_mgr.h"
#include "dpu_comp_maintain.h"
#include "dkmd_listener.h"
#include "dpu_comp_config_utils.h"

#define LTPO_90HZ_TE 90
#define MIN_STAT_DURATION_US 2000

static struct frm_rate_gears valid_frm_rate[FRM_RATE_GEARS_NUM] = {{FRM_RATE_1HZ, 1, 1000000},
	{FRM_RATE_5HZ, 5, 200000}, {FRM_RATE_10HZ, 10, 100000}, {FRM_RATE_15HZ, 15, 66667}, {FRM_RATE_24HZ, 24, 41667},
	{FRM_RATE_30HZ, 30, 33333}, {FRM_RATE_45HZ, 45, 22222}, {FRM_RATE_60HZ, 60, 16667}, {FRM_RATE_90HZ, 90, 11111},
	{FRM_RATE_120HZ, 120, 8333}, {FRM_RATE_180HZ, 180, 5556}};

static uint32_t find_closest_fps(uint64_t val) {
	uint32_t idx;
	uint32_t distans = UINT_MAX;
	uint32_t final_fps_type = 0;

	for (idx = 0; idx < FRM_RATE_GEARS_NUM; idx++) {
		if (abs(val - (uint64_t)valid_frm_rate[idx].period_us) < (uint64_t)distans) {
			distans = (uint32_t)abs(val - (uint64_t)valid_frm_rate[idx].period_us);
			final_fps_type = valid_frm_rate[idx].type;
		}
	}

	return final_fps_type;
}

static void refresh_stat_count_ddic_dimming(uint64_t *delta_time_us, struct dpu_comp_maintain *comp_maintain)
{
	uint32_t dim_idx;
	uint32_t repeat_idx;
	uint32_t dim_seq_num;
	struct dimming_node curr_dimming_node;
	uint32_t curr_dimming_fps_type;
	uint32_t last_dimming_fps_type;
	struct dfr_info *dfr_info = dkmd_get_dfr_info(comp_maintain->dpu_comp->conn_info);

	if (!dfr_info) {
		dpu_pr_err("dfr_info is null\n");
		return;
	}

	if (dfr_info->oled_info.ltpo_info.dimming_mode != DIMMING_MODE_DDIC)
		return;

	last_dimming_fps_type = find_closest_fps((uint64_t)comp_maintain->self_refresh_period_us);

	dim_seq_num = dfr_info->oled_info.ltpo_info.dimming_sequence[DIMMING_NORMAL].dimming_seq_num;
	for (dim_idx = 0; dim_idx < dim_seq_num; dim_idx++) {
		curr_dimming_node = dfr_info->oled_info.ltpo_info.dimming_sequence[DIMMING_NORMAL].dimming_seq_list[dim_idx];

		if (curr_dimming_node.frm_rate == valid_frm_rate[last_dimming_fps_type].frm_rate)
			break;

		for (repeat_idx = 0; repeat_idx < curr_dimming_node.repeat_num; repeat_idx++) {
			curr_dimming_fps_type = find_closest_fps((uint64_t)(PERIOD_US_1HZ / curr_dimming_node.frm_rate));
			if (*delta_time_us > valid_frm_rate[curr_dimming_fps_type].period_us) {
				*delta_time_us = *delta_time_us - valid_frm_rate[curr_dimming_fps_type].period_us;
				comp_maintain->refresh_stat_info.refresh_cnt[curr_dimming_fps_type]++;
				dpu_pr_debug("dimming fps = %u Hz", valid_frm_rate[curr_dimming_fps_type].frm_rate);
			}
		}
	}

	return;
}

static int32_t refresh_stat_count(uint64_t delta_time_us, struct dpu_comp_maintain *comp_maintain)
{
	uint32_t final_fps_type;
	uint32_t self_refresh_cnt;
	uint32_t tmp_period_us;

	tmp_period_us = comp_maintain->self_refresh_period_us;
	/* self refresh rate is 90Hz when in 90Hz TE case */
	if (comp_maintain->cur_te_rate == LTPO_90HZ_TE)
		comp_maintain->self_refresh_period_us = PERIOD_US_1HZ / comp_maintain->cur_te_rate;

	if (comp_maintain->self_refresh_period_us == 0) {
		dpu_pr_err("self refresh period == 0");
		return -1;
	}

	dpu_pr_debug("delta time = %llu us", delta_time_us);
	refresh_stat_count_ddic_dimming(&delta_time_us, comp_maintain);

	/* handle the self-refresh situation */
	self_refresh_cnt = (uint32_t)(delta_time_us / (uint64_t)comp_maintain->self_refresh_period_us);
	if (self_refresh_cnt > 0) {
		final_fps_type = find_closest_fps((uint64_t)comp_maintain->self_refresh_period_us);
		comp_maintain->refresh_stat_info.refresh_cnt[final_fps_type] += self_refresh_cnt;
		comp_maintain->real_frame_rate = (uint32_t)(PERIOD_US_1HZ / comp_maintain->self_refresh_period_us);
		dpu_pr_debug("self refresh period = %u us, self refresh cnt = %u",
			comp_maintain->self_refresh_period_us, self_refresh_cnt);
	}

	/* handle the non self-refresh situation */
	delta_time_us = delta_time_us - self_refresh_cnt * comp_maintain->self_refresh_period_us;
	if (delta_time_us > MIN_STAT_DURATION_US) {
		final_fps_type = find_closest_fps(delta_time_us);
		comp_maintain->refresh_stat_info.refresh_cnt[final_fps_type]++;
		comp_maintain->real_frame_rate = (uint32_t)(PERIOD_US_1HZ / delta_time_us);

		dpu_pr_debug("approximate fps = %u Hz, final fps = %u Hz", comp_maintain->real_frame_rate,
			valid_frm_rate[final_fps_type].frm_rate);
	}

	comp_maintain->self_refresh_period_us = tmp_period_us;
	return 0;
}

static void stat_clear_case_process(struct dpu_comp_maintain *comp_maintain)
{
	int32_t clear_flag;

	// timeout clear
	if (comp_maintain->total_count_time_us > MAX_RECODE_DURATION_US) {
		dpu_pr_info("total_count_time_us > MAX_RECODE_DURATION_US, total_count_time_us = %llu us",
			comp_maintain->total_count_time_us);

		comp_maintain->total_count_time_us = 0;
		memset_s(&comp_maintain->refresh_stat_info, sizeof(comp_maintain->refresh_stat_info),
			0, sizeof(comp_maintain->refresh_stat_info));
		return;
	}

	// clear flag clear
	clear_flag = pipeline_next_ops_handle(comp_maintain->dpu_comp->conn_info->conn_device,
		comp_maintain->dpu_comp->conn_info, GET_STATISTIC_CLEAR_FLAG, &comp_maintain->refresh_stat_info);
	if (clear_flag) {
		comp_maintain->total_count_time_us = 0;
		memset_s(&comp_maintain->refresh_stat_info, sizeof(comp_maintain->refresh_stat_info),
			0, sizeof(comp_maintain->refresh_stat_info));

		dpu_pr_info("refresh_stat_info clear");
		return;
	}

	return;
}

static void refresh_stat_isr_handle_work(struct kthread_work *work)
{
	uint64_t power_off_duration_us;
	uint64_t refresh_duration_us;
	uint64_t delta_time_us;
	uint32_t multiple_cnt;
	uint32_t te_duration_us;
	struct dpu_comp_maintain *comp_maintain = NULL;
	struct composer *comp = NULL;

	comp_maintain = container_of(work, struct dpu_comp_maintain, isr_handle_work);
	dpu_check_and_no_retval(!comp_maintain, err, "comp_maintain is null pointer");
	comp = &comp_maintain->dpu_comp->comp;
	dpu_check_and_no_retval(!comp, err, "comp is null pointer");

	mutex_lock(&comp_maintain->maintain_lock);
	if (comp_maintain->first_count_flag) {
		comp_maintain->pre_refresh_timestamp = comp_maintain->curr_refresh_timestamp;
		comp_maintain->pre_refresh_timestamp_hw = comp_maintain->curr_refresh_timestamp_hw;
		comp_maintain->first_count_flag = 0;
		mutex_unlock(&comp_maintain->maintain_lock);
		dpu_pr_info("first count case");
		return;
	}

	refresh_duration_us = (uint64_t)(ktime_to_us(comp_maintain->curr_refresh_timestamp) -
		ktime_to_us(comp_maintain->pre_refresh_timestamp));
	if (comp->power_off_timestamp > comp_maintain->pre_refresh_timestamp) {
		power_off_duration_us = (uint64_t)(ktime_to_us(comp->power_on_timestamp) -
			ktime_to_us(comp->power_off_timestamp));
		dpu_pr_debug("power off duration = %llu us", power_off_duration_us);
		delta_time_us = refresh_duration_us - power_off_duration_us;
	} else {
		delta_time_us = (uint64_t)abs(ktime_to_us(comp_maintain->curr_refresh_timestamp_hw) -
			ktime_to_us(comp_maintain->pre_refresh_timestamp_hw));
	}

	comp_maintain->total_count_time_us += delta_time_us;

	if (comp_maintain->cur_te_rate > 0) {
		te_duration_us = PERIOD_US_1HZ / comp_maintain->cur_te_rate;
		/* multiple_cnt need round off */
		multiple_cnt = (uint32_t)(delta_time_us / (uint64_t)te_duration_us);
		if ((delta_time_us - multiple_cnt * te_duration_us) > te_duration_us / 2)
			multiple_cnt += 1;

		delta_time_us = multiple_cnt * te_duration_us;
	}

	if (refresh_stat_count(delta_time_us, comp_maintain) != 0) {
		mutex_unlock(&comp_maintain->maintain_lock);
		dpu_pr_err("calculation encountered an error");
		return;
	}

	// set refresh count
	pipeline_next_ops_handle(comp_maintain->dpu_comp->conn_info->conn_device,
		comp_maintain->dpu_comp->conn_info, SET_REFRESH_STATISTIC, &comp_maintain->refresh_stat_info);

	stat_clear_case_process(comp_maintain);

	comp_maintain->pre_refresh_timestamp = comp_maintain->curr_refresh_timestamp;
	comp_maintain->pre_refresh_timestamp_hw = comp_maintain->curr_refresh_timestamp_hw;

	mutex_unlock(&comp_maintain->maintain_lock);
}

static int32_t refresh_stat_isr_notify(struct notifier_block *self, unsigned long action, void *data)
{
	struct dkmd_listener_data *listener_data = (struct dkmd_listener_data *)data;
	struct dpu_comp_maintain *comp_maintain = (struct dpu_comp_maintain *)(listener_data->data);

	if (!comp_maintain->routine_enabled)
		return 0;

	comp_maintain->curr_refresh_timestamp = ktime_get();
	comp_maintain->curr_refresh_timestamp_hw = get_te_hw_timestamp_us(comp_maintain->dpu_comp->comp_mgr->dpu_base,
		comp_maintain->dpu_comp->conn_info->base.pipe_sw_itfch_idx);

	dpu_pr_debug("timestamp = %llu us, timestamp_hw = %llu us", ktime_to_us(comp_maintain->curr_refresh_timestamp),
		ktime_to_us(comp_maintain->curr_refresh_timestamp_hw));

	kthread_queue_work(&comp_maintain->dpu_comp->handle_worker, &comp_maintain->isr_handle_work);
	return 0;
}

static struct notifier_block refresh_isr_notifier = {
	.notifier_call = refresh_stat_isr_notify,
};

void comp_mntn_refresh_stat_init(struct dkmd_isr *isr,
	struct dpu_comp_maintain *comp_maintain, uint32_t listening_bit)
{
	init_waitqueue_head(&comp_maintain->wait);

	comp_maintain->routine_enabled = false;
	comp_maintain->first_count_flag = 1;
	if (comp_maintain->dpu_comp->comp.index == DEVICE_COMP_PRIMARY_ID)
		comp_maintain->self_refresh_period_us = PRIMARY_SELF_REFRESH_PERIOD_US;

	if (comp_maintain->dpu_comp->comp.index == DEVICE_COMP_BUILTIN_ID)
		comp_maintain->self_refresh_period_us = BUILTIN_SELF_REFRESH_PERIOD_US;

	dkmd_isr_register_listener(isr, &refresh_isr_notifier, listening_bit, comp_maintain);
	kthread_init_work(&comp_maintain->isr_handle_work, refresh_stat_isr_handle_work);

	return;
}

void comp_mntn_refresh_stat_deinit(struct dkmd_isr *isr, uint32_t listening_bit)
{
	dkmd_isr_unregister_listener(isr, &refresh_isr_notifier, listening_bit);
}
