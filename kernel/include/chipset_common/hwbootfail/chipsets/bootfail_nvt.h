/*
 * copyright: Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: define the nvt chipsets's interface enum/macros/interface
 */

#ifndef BOOTFAIL_NVT_H
#define BOOTFAIL_NVT_H

/* ---- includes ---- */
/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	EN_POWER_ON_EVENT_RESERVED_FOR_CUSTOMIZE_START = 0xA0,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_ERECOVERY = 0xA1,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_RECOVERY = 0xA2,

	EN_POWER_ON_EVENT_RESERVED_FOR_DM_VERITY_FAIL = 0xA3,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_SHELL = 0xA4,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_BOOTLOADER = 0xA5,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_RESCUEPARTY = 0xA6,

	EN_POWER_ON_EVENT_RESERVED_FOR_REBOOT_APEXD_FAILED = 0xA7,

	EN_POWER_ON_EVENT_RESERVED_FOR_CUSTOMIZE_END = 0xBF,
} ker_stbc_ipc_reboot_cmd;

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
