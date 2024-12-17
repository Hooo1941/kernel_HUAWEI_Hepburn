/*
 * npu_dfx_black_box_adapter.h
 *
 * about npu black box adapter
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __NPU_BLACK_BOX_ADAPTER_H
#define __NPU_BLACK_BOX_ADAPTER_H

#include <platform_include/basicplatform/linux/dfx_noc_modid_para.h>

#include "soc_pctrl_interface.h"
#include "mntn_subtype_exception.h"
#include "bbox/npu_dfx_black_box.h"


#define PERI_CRG_BASE   (0xFFB85000)
#define ACTRL_BASE      (0xFB214000)
#define PCTRL_BASE      (0xFF7CC000)
#define SCTRL_BASE      (0xFB21B000)

static struct rdr_exception_info_s npu_rdr_excetption_info[] = {
	{
		.e_modid = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
		.e_modid_end = (u32)EXC_TYPE_TS_AICORE_EXCEPTION,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_from_core = RDR_NPU,
		.e_process_priority = RDR_ERR,
		.e_reset_core_mask  = RDR_NPU,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_exce_subtype = AICORE_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "AICORE_EXCP",
	},
	{
		.e_modid = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
		.e_modid_end = (u32)EXC_TYPE_TS_AICORE_TIMEOUT,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_from_core = RDR_NPU,
		.e_process_priority = RDR_ERR,
		.e_reset_core_mask  = RDR_NPU,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_exce_subtype = AICORE_TIMEOUT,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "AICORE_TIMEOUT",
	},
	{
		.e_modid = (u32)EXC_TYPE_TS_AIV_EXCEPTION,
		.e_modid_end = (u32)EXC_TYPE_TS_AIV_EXCEPTION,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_from_core = RDR_NPU,
		.e_process_priority = RDR_ERR,
		.e_reset_core_mask  = RDR_NPU,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_exce_subtype = AIV_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "AIV_EXCP",
	},
	{
		.e_modid = (u32)EXC_TYPE_TS_AIV_TIMEOUT,
		.e_modid_end = (u32)EXC_TYPE_TS_AIV_TIMEOUT,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_from_core = RDR_NPU,
		.e_process_priority = RDR_ERR,
		.e_reset_core_mask  = RDR_NPU,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_exce_subtype = AIV_TIMEOUT,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "AIV_TIMEOUT",
	},
	{
		.e_modid = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
		.e_modid_end = (u32)RDR_EXC_TYPE_TS_RUNNING_TIMEOUT,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority  = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = TS_RUNNING_TIMEOUT,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "TS_RUNNING_TIMEOUT",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_TS_INIT_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = TS_INIT_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "TS_INIT_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_POWERUP_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = POWERUP_FAIL,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "POWERUP_FAIL",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_POWERDOWN_FAIL,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = POWERDOWN_FAIL,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "POWERDOWN_FAIL",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION,
		.e_modid_end = (u32)RDR_EXC_TYPE_NPU_SMMU_EXCEPTION,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = SMMU_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "SMMU_EXCP",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NOC_NPU2,
		.e_modid_end = (u32)RDR_EXC_TYPE_NOC_NPU2,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = NPU_AICORE0_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "NPU_AICORE0_NOC_ERR",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NOC_NPU5,
		.e_modid_end = (u32)RDR_EXC_TYPE_NOC_NPU5,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = NPU_TS_CPU_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "NPU_TS_CPU_NOC_ERR",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NOC_NPU6,
		.e_modid_end = (u32)RDR_EXC_TYPE_NOC_NPU6,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = NPU_TS_HWTS_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "NPU_TS_HWTS_NOC_ERR",
	}, {
		.e_modid = (u32)RDR_EXC_TYPE_NOC_NPU7,
		.e_modid_end = (u32)RDR_EXC_TYPE_NOC_NPU7,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask  = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = NPU_TCU_NOC_ERR,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "NPU_TCU_NOC_ERR",
	}, {
		.e_modid = (u32)RDR_TYPE_HWTS_BUS_ERROR,
		.e_modid_end = (u32)RDR_TYPE_HWTS_BUS_ERROR,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = HWTS_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "HWTS_EXCP",
	}, {
		.e_modid = (u32)RDR_TYPE_HWTS_EXCEPTION_ERROR,
		.e_modid_end = (u32)RDR_TYPE_HWTS_EXCEPTION_ERROR,
		.e_process_priority = RDR_ERR,
		.e_reboot_priority = RDR_REBOOT_NO,
		.e_notify_core_mask = RDR_NPU,
		.e_reset_core_mask = RDR_NPU,
		.e_from_core = RDR_NPU,
		.e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
		.e_exce_type = NPU_S_EXCEPTION,
		.e_exce_subtype = HWTS_EXCP,
		.e_upload_flag = (u32)RDR_UPLOAD_YES,
		.e_save_log_flags = RDR_SAVE_BL31_LOG | RDR_SAVE_LOGBUF | RDR_SAVE_DMESG | RDR_SAVE_CONSOLE_MSG,
		.e_from_module = "NPU",
		.e_desc = "HWTS_EXCP",
	},
};


#ifdef CONFIG_DFX_SEC_QIC_V100_MODID_REGISTER
static u32 npu_qic_modid_array[][2] = {
	{QIC_NPU_AICORE0, (u32)RDR_EXC_TYPE_NOC_NPU2},
	{QIC_NPU_TS_0,    (u32)RDR_EXC_TYPE_NOC_NPU5},
	{QIC_NPU_TS_1,    (u32)RDR_EXC_TYPE_NOC_NPU6},
	{QIC_NPU_TCU,     (u32)RDR_EXC_TYPE_NOC_NPU7}
};
#endif

static struct npu_dump_reg peri_regs[0];

#endif /* __NPU_BLACK_BOX_ADAPTER_H */
