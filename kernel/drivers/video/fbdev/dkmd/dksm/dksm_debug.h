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

#ifndef _DKSM_DEBUG_H_
#define _DKSM_DEBUG_H_

#include <linux/types.h>

extern uint32_t g_dkmd_log_level;
extern uint32_t g_debug_fence_timeline;
extern uint32_t g_ldi_data_gate_en;
extern uint32_t g_debug_present_hold;
extern uint32_t g_debug_vsync_dump;
extern uint32_t g_debug_force_update;
extern uint32_t g_debug_vsync_delay_time;
extern uint32_t g_debug_dpu_clear_enable;
extern uint32_t g_debug_idle_enable;
extern uint32_t g_pan_display_frame_index;
extern bool g_dpu_complete_start;
extern uint32_t g_debug_m1_qic_enable;
extern uint32_t g_debug_dpu_qos_enable;
extern uint32_t g_debug_dpu_send_dcs_cmds;
extern uint32_t g_debug_dpu_perf_level;
extern uint32_t g_debug_dvfs_type;
extern uint32_t g_debug_dvfs_isr_enable;
extern int32_t g_cmdlist_dump_enable;
extern uint32_t g_debug_dpu_safe_frm_rate;
extern uint32_t g_debug_dpu_frm_rate;
extern int32_t g_debug_underflow_dump_enable;
extern uint32_t g_debug_dump_reg;
extern uint32_t g_debug_dmd_report_vact_end_miss;
extern uint32_t g_vsync_vactive_time_gap;
extern uint32_t g_debug_underflow_itr_enable;
extern uint32_t g_debug_perfstat_init_data[6];
extern int32_t g_debug_underflow_fail;

#endif /* _DKSM_DEBUG_H_ */
