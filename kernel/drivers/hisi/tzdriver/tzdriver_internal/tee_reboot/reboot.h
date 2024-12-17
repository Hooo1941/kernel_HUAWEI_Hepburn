/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function declaration for tee self-recovery, tee reboot
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
#ifndef REBOOT_H
#define REBOOT_H

#include <linux/err.h>

#define REBOOT_MAX_COUNT 100
#define REBOOT_SLEEP 20

#define MPIDR_MT              (1 << 24)
#define MPIDR_AFF_MASK        0xff00ffffffULL

#define MPIDR_AFF3_FIELD 32
#define MPIDR_AFF2_FIELD 16
#define MPIDR_AFF1_FIELD  8
#define AFF_MASK       0xff
#define SGI_AFF3_FIELD   48
#define SGI_AFF2_FIELD   32
#define SGI_AFF1_FIELD   16
#define SGI_ID_FIELD     24

#define GIC_AFF_LEVEL     4
#define GIC_CPU_RESCHEDULE   0

#define mrs(reg, v) asm volatile("mrs %0," reg : "=r"(v))
#define msr(reg, v) asm volatile("msr " reg ",%0" :: "r" (v))

#define define_sysreg_rd(name, reg)       \
static unsigned long get_##name(void)     \
{                                         \
	unsigned long val;                \
	mrs(#reg, val);                   \
	return val;                       \
}

#define define_sysreg_wr(name, reg)       \
static void set_##name(unsigned long val) \
{                                         \
	msr(#reg, val);                   \
}

define_sysreg_rd(mpidr_el1, mpidr_el1);
define_sysreg_wr(icc_sgi1r_el1, s3_0_c12_c11_5);

enum alarm_id {
	TEE_CRASH = 0,
	TEE_REBOOT_FAIL,
};
 
enum alarm_status {
	ALARM_REPORT = 0,
	ALARM_CLEAR,
};
 
typedef int32_t (*tee_alarm_func)(int32_t alarm_type, int32_t alarm_status);

#ifdef CONFIG_TEE_REBOOT
bool is_tee_rebooting(void);
int tee_init_reboot_thread(void);
int tee_wake_up_reboot(void);
void free_reboot_thread(void);
void get_teecd_pid(void);
void get_tlogcat_pid(void);
#endif

#endif
