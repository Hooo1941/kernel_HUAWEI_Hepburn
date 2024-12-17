/*
 * npu_pm_internal.h
 *
 * about npu pm internal api
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_PM_INTERNAL_H__
#define __NPU_PM_INTERNAL_H__

#include <linux/platform_device.h>

#include "npu_ipc.h"

/* smc command flags[0-31]: x3 */
#define NPU_FLAGS_POWER_ON                  (0x1 << 0) /* bit0: npu subsys poweron/off */
#define NPU_FLAGS_POWER_OFF                 (0x1 << 1) /* bit1: npu subsys poweron/off */
#define NPU_FLAGS_PWONOFF_AIC0              (0x1 << 2) /* bit2: core0 poweron/off */
#define NPU_FLAGS_PWONOFF_AIC1              (0x1 << 3) /* bit3: core1 poweron/off */
#define NPU_FLAGS_PWONOFF_AIC0_1            (0x3 << 2) /* bit2&bit3: core0&core1 poweron/off */
#define NPU_FLAGS_INIT_SYSDMA               (0x1 << 4) /* bit4: sysdma init */
#define NPU_FLAGS_DISBALE_SYSDMA            (0x1 << 5) /* bit5: sysdma disable */

enum npu_gic_chipid {
	NPU_GIC_1  = 1
};

enum npuip {
	NPUIP_NONE = 0,
	NPUIP_NPU_SUBSYS,
	NPUIP_NPU_BUS,
	NPUIP_TS_SUBSYS,
	NPUIP_AICORE,
	NPUIP_NPUCPU_SUBSYS,
	NPUIP_SDMA,
	NPUIP_TCU,
	NPUIP_MAX
};

int npu_powerup_aicore(u64 work_mode, u32 aic_flag);

int npu_powerdown_aicore(u64 work_mode, u32 aic_flag);

// common pm func
int npu_plat_powerup_till_npucpu(u64 work_mode);

int npu_plat_powerup_till_ts(u32 work_mode, u32 offset);

int npu_plat_powerdown_ts(u32 offset, u32 work_mode);

int npu_plat_powerdown_npucpu(u32 expect_val, u32 mode);

int npu_plat_powerup_tbu(void);

int npu_plat_powerdown_tbu(u32 aic_flag);

int npu_plat_powerdown_nputop(void);

int npu_plat_aicore_get_disable_status(u32 core_id);

bool npu_plat_is_support_ispnn(void);

int npu_plat_send_ts_ctrl_core(uint32_t core_num);

int npu_sync_ts_time(void);

int send_ipc_msg_to_ts(u32 cmd_type, u32 msg_sync_type,
	const u8 *send_msg, u32 send_len, enum ipc_send_ack_type ack_type);
#endif
