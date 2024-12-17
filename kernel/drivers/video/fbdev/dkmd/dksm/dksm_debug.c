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
#include <linux/module.h>
#include "dkmd_log.h"

uint32_t g_dkmd_log_level = DPU_LOG_LVL_INFO;
uint32_t g_debug_fence_timeline = 0;
uint32_t g_ldi_data_gate_en = 1;
uint32_t g_debug_present_hold;
uint32_t g_debug_vsync_dump = 0;
uint32_t g_debug_force_update = 0;
uint32_t g_debug_vsync_delay_time = 0;
uint32_t g_debug_dpu_clear_enable = 1;
uint32_t g_debug_idle_enable = 1;
uint32_t g_debug_m1_qic_enable = 0;
uint32_t g_debug_dpu_qos_enable = 1;
uint32_t g_debug_dpu_send_dcs_cmds = 0;
uint32_t g_debug_dpu_perf_level = 0;
int32_t g_debug_underflow_dump_enable = 1;
uint32_t g_debug_dump_reg = 0;
uint32_t g_debug_dmd_report_vact_end_miss = 0;
// Set this type in screen off, in order to switch to other dvfs type
// 0: hw channel dvfs 1: sw channel dvfs 2: sw regulator dvfs
uint32_t g_debug_dvfs_type = 0;
uint32_t g_debug_dvfs_isr_enable = 0;
int32_t g_cmdlist_dump_enable = 0;
uint32_t g_debug_dpu_safe_frm_rate = 0;
uint32_t g_pan_display_frame_index;
uint32_t g_debug_dpu_frm_rate = 0;
bool g_dpu_complete_start;
uint32_t g_vsync_vactive_time_gap = 2000; // us
uint32_t g_debug_underflow_itr_enable = 0;
uint32_t g_debug_perfstat_init_data[6] = {50, 0x100C0, 0x0, 28, 2, 1};
int g_debug_perfstat_init_data_len = 0;
int32_t g_debug_underflow_fail = 0;


#ifdef CONFIG_DKMD_DEBUG_ENABLE
module_param_named(debug_msg_level, g_dkmd_log_level, uint, 0640);
MODULE_PARM_DESC(debug_msg_level, "dpu msg level");

module_param_named(debug_fence_timeline, g_debug_fence_timeline, uint, 0640);
MODULE_PARM_DESC(debug_fence_timeline, "dpu fence timeline debug");

module_param_named(enable_ldi_data_gate, g_ldi_data_gate_en, uint, 0640);
MODULE_PARM_DESC(enable_ldi_data_gate, "dpu dsi ldi data gate enable");

module_param_named(debug_present_hold, g_debug_present_hold, uint, 0640);
MODULE_PARM_DESC(debug_present_hold, "dpu present hold debug");

module_param_named(debug_vsync_dump, g_debug_vsync_dump, uint, 0640);
MODULE_PARM_DESC(debug_vsync_dump, "debug vsync dump timestamp");

module_param_named(debug_force_update, g_debug_force_update, uint, 0640);
MODULE_PARM_DESC(debug_force_update, "debug force update");

module_param_named(debug_vsync_delay, g_debug_vsync_delay_time, uint, 0640);
MODULE_PARM_DESC(debug_vsync_delay, "debug vsync delay time");

module_param_named(debug_dpu_clear, g_debug_dpu_clear_enable, uint, 0640);
MODULE_PARM_DESC(debug_dpu_clear, "debug dpu composer clear config");

module_param_named(debug_idle_enable, g_debug_idle_enable, uint, 0640);
MODULE_PARM_DESC(debug_idle_enable, "dpu idle enable");

module_param_named(debug_m1_qic_enable, g_debug_m1_qic_enable, uint, 0640);
MODULE_PARM_DESC(debug_m1_qic_enable, "dpu m1 qic enable");

module_param_named(debug_dpu_qos_enable, g_debug_dpu_qos_enable, uint, 0640);
MODULE_PARM_DESC(debug_dpu_qos_enable, "dpu qos enable");

module_param_named(debug_dpu_send_dcs_cmds, g_debug_dpu_send_dcs_cmds, uint, 0640);
MODULE_PARM_DESC(debug_dpu_send_dcs_cmds, "dpu send dcs cmds");

module_param_named(debug_dpu_perf_level, g_debug_dpu_perf_level, uint, 0640);
MODULE_PARM_DESC(debug_dpu_perf_level, "dpu perf level");

module_param_named(debug_dump_reg, g_debug_dump_reg, uint, 0640);
MODULE_PARM_DESC(debug_dump_reg, "dpu dump reg");

module_param_named(debug_dvfs_type, g_debug_dvfs_type, uint, 0640);
MODULE_PARM_DESC(debug_dvfs_type, "debug dvfs type");

module_param_named(debug_dvfs_isr_enable, g_debug_dvfs_isr_enable, uint, 0640);
MODULE_PARM_DESC(debug_dvfs_isr_enable, "enable dvfs isr");

module_param_named(cmdlist_dump_enable, g_cmdlist_dump_enable, int, 0600);
MODULE_PARM_DESC(cmdlist_dump_enable, "enable cmdlist config dump");

module_param_named(debug_dpu_safe_frm_rate, g_debug_dpu_safe_frm_rate, uint, 0640);
MODULE_PARM_DESC(debug_dpu_safe_frm_rate, "dpu safe frm rate");

module_param_named(underflow_dump_enable, g_debug_underflow_dump_enable, int, 0600);
MODULE_PARM_DESC(underflow_dump_enable, "enable underflow dump");

module_param_named(dmd_report_vact_end_miss, g_debug_dmd_report_vact_end_miss, int, 0600);
MODULE_PARM_DESC(dmd_report_vact_end_miss, "enable dmd report vactive endmiss");
module_param_named(vsync_vactive_time_gap, g_vsync_vactive_time_gap, int, 0640);
MODULE_PARM_DESC(time_gap, "vsync time diff");

module_param_named(debug_dpu_frm_rate, g_debug_dpu_frm_rate, uint, 0640);
MODULE_PARM_DESC(debug_dpu_frm_rate, "dpu frm rate");
module_param_named(debug_underflow_itr_enable, g_debug_underflow_itr_enable, uint, 0640);
MODULE_PARM_DESC(debug_underflow_itr_enable, "dpu underflow itr enable");
module_param_array_named(debug_perfstat_init_data, g_debug_perfstat_init_data, uint,
						&g_debug_perfstat_init_data_len, 0640);
module_param_named(debug_underflow_fail, g_debug_underflow_fail, int, 0600);
MODULE_PARM_DESC(debug_underflow_fail, "debug underflow fail");

#endif
