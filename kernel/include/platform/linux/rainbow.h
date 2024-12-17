/*
 * rainbow.h
 *
 * This file is the header file for rainbow
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _RAINBOW_INTERFACE_H
#define _RAINBOW_INTERFACE_H

#define RB_MREASON_STR_MAX      32
#define RB_SREASON_STR_MAX      128

#define RB_REASON_STR_VALID     0x56414C44  /* VALD  0x56414C44 */
#define RB_REASON_STR_EMPTY     0

#define RB_TRACE_SECTION_SIZE 0x80000           /* 512kb */

#define RB_REGION_LOG_MAGIC_VALID   0x5A5A5A5A
#define RB_REGION_LOG_MAGIC_INVALID 0

#define RB_BL_LOG_SIZE 0x40000

enum rb_addr_offset_enum {
	RB_HEADER_OFFSET = 0,
	RB_PRE_LOG_OFFSET = 0x80000,
	RB_TRACE_IRQ_OFFSET = 0x100000,
	RB_TRACE_TASK_OFFSET = 0x180000,
};

enum himntn_id_enum {
	HIMNTN_ID_HEAD = 0,
	HIMNTN_ID_FIRST_BOOT,
	HIMNTN_ID_RESERVED,
	HIMNTN_ID_SUBSYS_REBOOT_SWITCH = 11,
	HIMNTN_ID_WARMRESET_SWITCH,
	HIMNTN_ID_PL_LOG_SWITCH,
	HIMNTN_ID_LK_LOG_SWITCH,
	HIMNTN_ID_LK_LOG_LEVEL,
	HIMNTN_ID_FASTBOOT_SWITCH,
	HIMNTN_ID_BOOTDUMP_SWITCH,
	HIMNTN_ID_FINAL_RELEASE,
	HIMNTN_ID_LK_WDT_SWITCH,
	HIMNTN_ID_LK_WDT_TIMEOUT,
	HIMNTN_ID_LKMSG_PRESS_SWITCH,
	HIMNTN_ID_UART_LOG_SWITCH,
	HIMNTN_ID_MRK_SWITCH,
	HIMNTN_ID_THREE_KEY_SWITCH,
	HIMNTN_ID_KEL_DEBUGLOG_SWITCH,
	HIMNTN_ID_PROC_PLLK_SWITCH,
	HIMNTN_ID_PROC_LKMSG_SWITCH,
	HIMNTN_ID_KERNEL_BUF_SWITCH,
	HIMNTN_ID_USER_LOG_SWITCH,
	HIMNTN_ID_SYSRQ_SWITCH,
	HIMNTN_ID_KERNEL_WDT_TIMEOUT,
	HIMNTN_ID_AEE_MODE,
	HIMNTN_ID_REBOOT_DB_SWITCH,
	HIMNTN_ID_EE_DB_SWITCH,
	HIMNTN_ID_OCP_DB_SWITCH,
	HIMNTN_ID_MRK_DB_SWITCH,
	HIMNTN_ID_SYSAPI_DB_SWITCH,
	HIMNTN_ID_KELAPI_DB_SWITCH,
	HIMNTN_ID_KMEMLEAK_SWITCH,
	HIMNTN_ID_BOTTOM = 40,
};

enum mreason_flag_enum {
	RB_M_UINIT = 0,
	RB_M_NORMAL,
	RB_M_APANIC,
	RB_M_BOOTFAIL,
	RB_M_AWDT,
	RB_M_POWERCOLLAPSE,
	RB_M_PRESS6S,
	RB_M_BOOTLOADER,
	RB_M_TZ,
	RB_M_UNKOWN,
};

void rb_mreason_set(uint32_t reason);
void rb_sreason_set(char *fmt);
void rb_attach_info_set(char *fmt);
void rb_kallsyms_set(const char *fmt);
void *rb_bl_log_get(unsigned int *size);
/* core interface for business module */
void cmd_himntn_item_switch(unsigned int index, bool *rtn);
bool cmd_himntn_bootdump_switch_fast(void);
void *get_rb_header_paddr(void);
void *get_rb_header(void);
#endif /* _RAINBOW_INTERFACE_H */
