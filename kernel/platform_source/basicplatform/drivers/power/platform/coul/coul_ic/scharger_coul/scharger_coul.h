/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coulometer hardware driver headfile
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _SCHARGER_COUL_H_
#define _SCHARGER_COUL_H_

#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <platform_include/cee/linux/adc.h>
#include <linux/mfd/pmic_platform.h>

#include "scharger_v700_coul.h"

#define reg_spmi_read(regAddr) \
		pmic_read_reg(regAddr)
#define reg_spmi_write(regAddr, regval) \
		pmic_write_reg((int)(regAddr), (int)(regval))
#define reg_spmi_bulk_read(regAddr, buf, size) \
		pmic_array_read((int)(regAddr), (char *)(buf), (int)(size))
#define reg_spmi_bulk_write(regAddr, buf, size) \
		pmic_array_write((int)(regAddr), (char *)(buf), (int)(size))

#define BATT_L                                 0
#define BATT_H                                 1

#define PMU_ENABLE                              1
#define PMU_DISABLE                             0

#define LOCK                                    1
#define UNLOCK                                  0

#define R_COUL_UOHM                             10000 /* resisitance uohm */
#define INVALID_TEMP                            (-99)
#define COUL_HARDWARE                             0x36
#define COUL_PMIC6X21V700                         0x700
#define DEFAULT_BATTERY_VOL_2_PERCENT           3350
#define DEFAULT_BATTERY_VOL_0_PERCENT           3150
#define DEFAULT_I_GATE_VALUE                    5000 /* 5000 mA */
#define COUL_BIT_MASK                           0x800000
#define COUL_NEG_EXPAND                         0xff000000
#define COUL_FIFO_VOL_DEFAULT                   0xffffff
#define PERMILLAGE                              1000
#define SECONDS_PER_HOUR                        3600

#define ADC_CALI_TIMEOUT                        3000 /* 3s */
#define ADC_DELAY                               100  /* 100ms */

#define coul_hardware_err(fmt, args...) \
	printk(KERN_ERR    "[schargerV700_coul]" fmt, ## args)
#define coul_hardware_evt(fmt, args...) \
	printk(KERN_WARNING"[schargerV700_coul]" fmt, ## args)
#define coul_hardware_inf(fmt, args...) \
	printk(KERN_INFO   "[schargerV700_coul]" fmt, ## args)
#define coul_hardware_dbg(fmt, args...)

struct batt_regs {
	/* read only */
	unsigned int cl_in;
	unsigned int cl_out;

	/* read & write */
	unsigned int cl_in_rw;
	unsigned int cl_out_rw;

	unsigned int cl_in_time_ro;
	unsigned int cl_out_time_ro;
	unsigned int cl_in_time_rw;
	unsigned int cl_out_time_rw;

	unsigned int vol_fifo;
	unsigned int cur_fifo;
	unsigned int state;
	unsigned int adc_cali;
	unsigned int vol;
	unsigned int cur;
	unsigned int ate_a_b;

	unsigned int eco_vol_fifo;
	unsigned int eco_cur_fifo;
	unsigned int eco_temp_fifo;

	unsigned int eco_out_clin;
	unsigned int eco_out_clout;

	unsigned int irq_mask;
	unsigned int irq_flag;

	unsigned int  offset_voltage;
};

struct eco_leak_info {
	int rst_uah;
	int eco_uah;
	int cur_uah;
	s64 eco_in_uah;
	s64 eco_out_uah;
	s64 present_in_uah;
	s64 present_out_uah;
	u64 in_val;
	u64 out_val;
};

struct write_protect {
	unsigned int reg;
	unsigned char lock_val;
	unsigned char unlock_val;
};
struct coul_device_info {
	struct device *dev;
	struct regmap *regmap;
	struct work_struct irq_work;
	struct delayed_work chip_cali_work;
	struct mutex ops_lock;
	struct wakeup_source *wake_lock;
	int low_vol_mv;
	int irq;
	int batt_l_index;
	int batt_h_index;
	/* Registers address info */
	unsigned int irq_flag;
	unsigned int v_int;
	unsigned int i_in_gate;
	unsigned int i_out_gate;
	unsigned int bat_cnct_sel;

	unsigned int clj_ctrl;
	unsigned int clj_ctrl1;
	unsigned int clj_ctrl2;
	unsigned int clj_ctrl3;
	unsigned int clj_ctrl4;
	unsigned int coul_mstate;
	struct batt_regs h_batt;
	struct batt_regs l_batt;
	struct write_protect wp;

	struct coul_cali_params batt_h_cali_params;
	struct coul_cali_params batt_l_cali_params;

	u64 g_last_eco_in_h;
	u64 g_last_eco_out_h;

	u64 g_last_eco_in_l;
	u64 g_last_eco_out_l;
};

#endif
