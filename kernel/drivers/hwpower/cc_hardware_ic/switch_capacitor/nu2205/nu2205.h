/* SPDX-License-Identifier: GPL-2.0 */
/*
 * nu2205.h
 *
 * nu2205 header file
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _NU2205_H_
#define _NU2205_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/raid/pq.h>
#include <linux/bitops.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>

enum nu2205_info {
	NU2205_INFO_IC_NAME = 0,
	NU2205_INFO_MAX_IBAT,
	NU2205_INFO_IBUS_OCP,
	NU2205_INFO_IBAT_OCP,
	NU2205_INFO_VBAT_OVP,
	NU2205_INFO_SW_FREQ,
	NU2205_INFO_TOTAL,
};

struct nu2205_mode_para {
	char ic_name[CHIP_DEV_NAME_LEN];
	int max_ibat;
	int ibus_ocp;
	int ibat_ocp;
	int vbat_ovp;
	int switching_frequency;
};

struct nu2205_dump_data {
	int vbus;
	int ibat;
	int vusb;
	int ibus;
	int temp;
};

struct nu2205_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data notify_data;
	struct dc_ic_ops sc_ops;
	struct dc_ic_ops lvc_ops;
	struct dc_batinfo_ops batinfo_ops;
	struct power_log_ops log_ops;
	struct nu2205_mode_para nu2205_lvc_para;
	struct nu2205_mode_para nu2205_sc_para;
	struct notifier_block otg_nb;
	char name[CHIP_DEV_NAME_LEN];
	int gpio_int;
	int irq_int;
	int chip_already_init;
	int device_id;
	bool first_rd;
	bool init_finished;
	bool int_notify_enabled;
	int sense_r_actual;
	int sense_r_config;
	int ovp_mos_switch_off;
	int manual_otg_mode;
	u32 ic_role;
};

enum irq_flag {
	NU2205_IRQ_AC1_OVP,
	NU2205_IRQ_AC2_OVP,
	NU2205_IRQ_FLT_FLAG,
	NU2205_IRQ_CONVERTER_STATE,
	NU2205_IRQ_IBUS_UCP_RISE_FLAG,
	NU2205_IRQ_IBUS_UCP_FALL_FLAG,
	NU2205_IRQ_VOUT_OVP,
	NU2205_IRQ_VBUS2VOUT_UVP_OVP,
	NU2205_IRQ_WD_TIMEOUT,
	NU2205_IRQ_END,
};

#define NU2205_COMP_MAX_NUM                    4
#define NU2205_BUF_LEN                         80
#define NU2205_SWITCHCAP_DISABLE               0
#define NU2205_ALM_DISABLE                     1
#define NU2205_ALM_ENABLE                      0

/* commom para init */
#define NU2205_VBAT_OVP_TH_INIT                10150 /* mv */
#define NU2205_IBAT_OCP_TH_INIT                8300 /* ma */
#define NU2205_VBUS_OVP_TH_INIT                21800 /* mv */
#define NU2205_IBUS_OCP_TH_INIT                4750 /* ma */
#define NU2205_VAC_OVP_TH_INIT                 22000 /* mv */
/* lvc para init */
#define NU2205_LVC_IBUS_OCP_TH_INIT            7000 /* ma */
/* sc para init */
#define NU2205_SC_IBUS_OCP_TH_INIT             5000 /* ma */

/* VBAT_OVP reg=0x00 */
#define NU2205_VBAT_OVP_REG                    0x00

#define NU2205_VBAT_OVP_DIS_MASK               BIT(7)
#define NU2205_VBAT_OVP_DIS_SHIFT              7
#define NU2205_VBAT_OVP_ENABLE                 0
#define NU2205_VBAT_OVP_DISABLE                1
#define NU2205_VBAT_OVP_MASK                   (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_VBAT_OVP_SHIFT                  0
#define NU2205_VBAT_OVP_STEP                   50 /* mv */
#define NU2205_VBAT_OVP_MAX                    10150 /* mv */
#define NU2205_VBAT_OVP_BASE                   7000 /* mv */
#define NU2205_VBAT_OVP_DEFAULT                8700 /* mv */

/* VBAT_OVP_ALM reg=0x01h */
#define NU2205_VBAT_OVP_ALM_REG                0x01

#define NU2205_VBAT_OVP_ALM_DIS_MASK           BIT(7)
#define NU2205_VBAT_OVP_ALM_DIS_SHIFT          7
#define NU2205_VBAT_OVP_ALM_ENABLE             0
#define NU2205_VBAT_OVP_ALM_DISABLE            1
#define NU2205_VBAT_OVP_ALM_MASK               (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_VBAT_OVP_ALM_SHIFT              0
#define NU2205_VBAT_OVP_ALM_BASE               7000
#define NU2205_VBAT_OVP_ALM_STEP               50

/* IBAT_OCP reg=0x02 */
#define NU2205_IBAT_OCP_REG                    0x02

#define NU2205_IBAT_OCP_DIS_MASK               BIT(7)
#define NU2205_IBAT_OCP_DIS_SHIFT              7
#define NU2205_IBAT_OCP_MASK                   (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define NU2205_IBAT_OCP_SHIFT                  0
#define NU2205_IBAT_OCP_MAX                    12000 /* ma */
#define NU2205_IBAT_OCP_BASE                   2000 /* ma */
#define NU2205_IBAT_OCP_DEFAULT                10100 /* ma */
#define NU2205_IBAT_OCP_STEP                   100 /* ma */

/* IBAT_OCP_ALM reg=0x03 */
#define NU2205_IBAT_OCP_ALM_REG                0x03

#define NU2205_IBAT_OCP_ALM_DIS_MASK           BIT(7)
#define NU2205_IBAT_OCP_ALM_DIS_SHIFT          7
#define NU2205_IBAT_OCP_ALM_ENABLE             0
#define NU2205_IBAT_OCP_ALM_DISABLE            1

#define NU2205_IBAT_OCP_ALM_MASK               (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define NU2205_IBAT_OCP_ALM_SHIFT              0
#define NU2205_IBAT_OCP_ALM_BASE               0
#define NU2205_IBAT_OCP_ALM_STEP               100 /* ma */

/* IBAT_UCP_ALM reg=0x04 */
#define NU2205_IBAT_UCP_ALM_REG                0x04

#define NU2205_IBAT_UCP_ALM_DIS_MASK           BIT(7)
#define NU2205_IBAT_UCP_ALM_DIS_SHIFT          7
#define NU2205_IBAT_UCP_ALM_ENABLE             0
#define NU2205_IBAT_UCP_ALM_DISABLE            1

#define NU2205_IBAT_UCP_ALM_MASK               (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_IBAT_UCP_ALM_SHIFT              0
#define NU2205_IBAT_UCP_ALM_BASE               0
#define NU2205_IBAT_UCP_ALM_STEP               50 /* ma */

/* AC1_PROTECTION reg=0x05 */
#define NU2205_AC1_OVP_REG                     0x05

#define NU2205_AC1_OVP_STAT_MASK               BIT(7)
#define NU2205_AC1_OVP_STAT_SHIFT              7
#define NU2205_AC1_OVP_FLAG_MASK               BIT(6)
#define NU2205_AC1_OVP_FLAG_SHIFT              6
#define NU2205_AC1_OVP_MASK_MASK               BIT(5)
#define NU2205_AC1_OVP_MASK_SHIFT              5
#define NU2205_AC1_VAC1_PD_EN_MASK             BIT(4)
#define NU2205_AC1_VAC1_PD_EN_SHIFT            4
#define NU2205_AC1_VAC1_PD_EN_ENABLE           1
#define NU2205_AC1_VAC1_PD_EN_DISABLE          0
#define NU2205_AC1_OVP_MASK                    BIT(2) | BIT(1) | BIT(0)
#define NU2205_AC1_OVP_SHIFT                   0

/* AC2_PROTECTION reg=0x06 */
#define NU2205_AC2_OVP_REG                     0x06

#define NU2205_AC2_OVP_STAT_MASK               BIT(7)
#define NU2205_AC2_OVP_STAT_SHIFT              7
#define NU2205_AC2_OVP_FLAG_MASK               BIT(6)
#define NU2205_AC2_OVP_FLAG_SHIFT              6
#define NU2205_AC2_OVP_MASK_MASK               BIT(5)
#define NU2205_AC2_OVP_MASK_SHIFT              5
#define NU2205_AC2_VAC2_PD_EN_MASK             BIT(4)
#define NU2205_AC2_VAC2_PD_EN_SHIFT            4
#define NU2205_AC2_VAC2_PD_EN_ENABLE           1
#define NU2205_AC2_VAC2_PD_EN_DISABLE          0
#define NU2205_AC2_OVP_MASK                    (BIT(2) | BIT(1) | BIT(0))
#define NU2205_AC2_OVP_SHIFT                   0

#define NU2205_AC_OVP_FBYPASS_STEP             250 /* mv */
#define NU2205_AC_OVP_FBYPASS_MAX              10000 /* mv */
#define NU2205_AC_OVP_FBYPASS_BASE             8500 /* mv */
#define NU2205_AC_OVP_F42SC_STEP               500 /* mv */
#define NU2205_AC_OVP_F42SC_MAX                22000 /* mv */
#define NU2205_AC_OVP_F42SC_BASE               19000 /* mv */
#define NU2205_AC_OVP_DFT                      0x07

/* VBUS_OVP reg=0x07
 * 4:2 mode, step size valid for 15.6V through 21.8 V only
 * 2:2 mode, step size valid for 7.8V through 10.9 V only
 */
#define NU2205_VBUS_OVP_REG                    0x07

#define NU2205_BUS_PD_EN_MASK                  BIT(7)
#define NU2205_BUS_PD_EN_SHIFT                 7
#define NU2205_BUS_OVP_DIS_MASK                BIT(6)
#define NU2205_BUS_OVP_DIS_MASK_SHIFT          6
#define NU2205_VBUS_OVP_MASK                   (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_VBUS_OVP_SHIFT                  0
#define NU2205_VBUS_OVP_FBYPASS_BASE           7800 /* mv */
#define NU2205_VBUS_OVP_FBYPASS_MAX            12500 /* mv */
#define NU2205_VBUS_OVP_FBYPASS_STEP           50 /* mv */
#define NU2205_VBUS_OVP_F42SC_BASE             15600 /* mv */
#define NU2205_VBUS_OVP_F42SC_MAX              25000 /* mv */
#define NU2205_VBUS_OVP_F42SC_STEP             100 /* mv */

/* VBUS_OVP_ALM reg=0x08 */
#define NU2205_VBUS_OVP_ALM_REG                0x08

#define NU2205_VBUS_OVP_ALM_DIS_MASK           BIT(7)
#define NU2205_VBUS_OVP_ALM_DIS_SHIFT          7
#define NU2205_VBUS_OVP_ALM_ENABLE             0
#define NU2205_VBUS_OVP_ALM_DISABLE            1
#define NU2205BUS_RCP_DIS_MASK                 BIT(6)
#define NU2205BUS_RCP_DIS_MASK_SHIFT           6
#define NU2205BUS_RCP_DISABLE                  1
#define NU2205BUS_RCP_ENABLE                   0

#define NU2205_VBUS_OVP_ALM_MASK               (BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_VBUS_OVP_ALM_SHIFT              0
#define NU2205_VBUS_OVP_ALM_FBYPASS_BASE       7300
#define NU2205_VBUS_OVP_ALM_FBYPASS_LSB        50
#define NU2205_VBUS_OVP_ALM_F42SC_BASE         14600
#define NU2205_VBUS_OVP_ALM_F42SC_LSB          100

/* IBUS_OCP_UCP reg=0x09 */
#define NU2205_IBUS_OCP_UCP_REG                0x09

#define NU2205_IBUS_OCP_DIS_MASK               BIT(7)
#define NU2205_IBUS_OCP_DIS_SHIFT              7
#define NU2205_IBUS_OCP_ENABLE                 0
#define NU2205_IBUS_OCP_DISABLE                1
#define NU2205_IBUS_UCP_RISE_FLAG              BIT(6)
#define NU2205_IBUS_UCP_RISE_FLAG_SHIFT        6
#define NU2205_IBUS_UCP_RISE_MASK_MASK         BIT(5)
#define NU2205_IBUS_UCP_RISE_MASK_SHIFT        5
#define NU2205_IBUS_UCP_RISE_ENABLE            0
#define NU2205_IBUS_UCP_RISE_DISABLE           1
#define NU2205_IBUS_UCP_DIS_MASK               BIT(4)
#define NU2205_IBUS_UCP_DIS_SHIFT              4
#define NU2205_IBUS_UCP_ENABLE                 0
#define NU2205_IBUS_UCP_DISABLE                1

#define NU2205_IBUS_OCP_MASK                   (BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_IBUS_OCP_SHIFT                  0
#define NU2205_IBUS_OCP_BASE                   2500 /* ma */
#define NU2205_IBUS_OCP_STEP                   250 /* ma */
#define NU2205_IBUS_OCP_MAX                    6250 /* ma */
#define NU2205_IBUS_OCP_DEFUALT                4750 /* ma */

/* IBUS_OCP_ALM reg=0x0A */
#define NU2205_IBUS_OCP_ALM_REG                0x0A

#define NU2205_IBUS_OCP_ALM_DIS_MASK           BIT(7)
#define NU2205_IBUS_OCP_ALM_DIS_SHIFT          7
#define NU2205_IBUS_OCP_ALM_ENABLE             0
#define NU2205_IBUS_OCP_ALM_DISABLE            1
#define NU2205_IBUS_UCP_FALL_FLAG              BIT(6)
#define NU2205_IBUS_UCP_FALL_FLAG_SHIFT        6
#define NU2205_IBUS_UCP_FALL_MASK_MASK         BIT(5)
#define NU2205_IBUS_UCP_FALL_MASK_SHIFT        5
#define NU2205_IBUS_UCP_FALL_ENABLE            0
#define NU2205_IBUS_UCP_FALL_DISABLE           1

#define NU2205_IBUS_OCP_ALM_MASK               (BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define NU2205_IBUS_OCP_ALM_SHIFT              0
#define NU2205_IBUS_OCP_ALM_BASE               2500 /* ma */
#define NU2205_IBUS_OCP_ALM_STEP               125 /* ma */
#define NU2205_IBUS_OCP_ALM_MAX                6375 /* ma */
#define NU2205_IBUS_OCP_ALM_DEFUALT            4500 /* ma */

/* VOUT_OVP reg = 0x0B */
#define NU2205_VOUT_OVP_REG                    0x0B

#define NU2205_VOUT_OVP_DIS_MASK               BIT(7)
#define NU2205_VOUT_OVP_DIS_SHIFT              7
#define NU2205_VOUT_OVP_ENABLE                 0
#define NU2205_VOUT_OVP_DISABLE                1
#define NU2205_VOUT_OVP_SET_MASK               BIT(6)
#define NU2205_VOUT_OVP_SET_SHIFT              6
#define NU2205_VOUT_OVP_9_8_V                  0 /* 0:9.8v */
#define NU2205_VOUT_OVP_10_8_V                 1 /* 1:10.8v */
#define NU2205_VOUT_OVP_STAT_MASK              BIT(2)
#define NU2205_VOUT_OVP_STAT_SHIFT             2
#define NU2205_VOUT_OVP_FLAG_MASK              BIT(1)
#define NU2205_VOUT_OVP_FLAG_SHIFT             1
#define NU2205_VOUT_OVP_MASK_MASK              BIT(0)
#define NU2205_VOUT_OVP_MASK_SHIFT             0
#define NU2205_VOUT_OVP_MASK_ENABLE            1
#define NU2205_VOUT_OVP_MASK_DISABLE           0

/* CONVERTER_STATE reg = 0x0C */
#define NU2205_CONVERTER_STATE_REG             0x0C

#define NU2205_TSD_FLAG_MASK                   BIT(7)
#define NU2205_TSD_FLAG_SHIFT                  7
#define NU2205_TSD_STAT_MASK                   BIT(6)
#define NU2205_TSD_STAT_SHIFT                  6
#define NU2205_VBUS_ERRORLO_STAT_MASK          BIT(5)
#define NU2205_VBUS_ERRORLO_STAT_SHIFT         5
#define NU2205_VBUS_ERRORHI_STAT_MASK          BIT(4)
#define NU2205_VBUS_ERRORHI_STAT_SHIFT         4
#define NU2205_SS_TIMEOUT_FLAG_MASK            BIT(3)
#define NU2205_SS_TIMEOUT_FLAG_SHIFT           3
#define NU2205_CONV_ACTIVE_STAT_MASK           BIT(2)
#define NU2205_CONV_ACTIVE_STAT_SHIFT          2
#define NU2205_PIN_DIAG_FAIL_FLAG_MASK         BIT(0)
#define NU2205_PIN_DIAG_FAIL_FLAG_SHIFT        0

/* CONTROL reg = 0x0D */
#define NU2205_CONTROL_REG                     0x0D
/* 0x34=disable watchdog, switching freq 500khz */
#define NU2205_CONTROL_REG_INIT                0x34

#define NU2205_REG_RST_MASK                    BIT(7)
#define NU2205_REG_RST_SHIFT                   7
#define NU2205_FSW_SET_MASK                    (BIT(6) | BIT(5) | BIT(4))
#define NU2205_FSW_SET_SHIFT                   4
#define NU2205_WD_TIMEOUT_FLAG_MASK            BIT(3)
#define NU2205_WD_TIMEOUT_FLAG_SHIFT           3
#define NU2205_WATCHDOG_DIS_MASK               BIT(2)
#define NU2205_WATCHDOG_DIS_SHIFT              2
#define NU2205_WATCHDOG_DISABLE                1
#define NU2205_WATCHDOG_ENABLE                 0
#define NU2205_WATCHDOG_CONFIG_MASK            (BIT(0) | BIT(1))
#define NU2205_WATCHDOG_CONFIG_SHIFT           0

#define NU2205_REG_RST_ENABLE                  1

#define NU2205_SW_FREQ_200KHZ                  200
#define NU2205_SW_FREQ_300KHZ                  300
#define NU2205_SW_FREQ_400KHZ                  400
#define NU2205_SW_FREQ_500KHZ                  500
#define NU2205_SW_FREQ_600KHZ                  600
#define NU2205_SW_FREQ_700KHZ                  700
#define NU2205_SW_FREQ_800KHZ                  800
#define NU2205_SW_FREQ_900KHZ                  900

#define NU2205_FSW_SET_SW_FREQ_200KHZ          0x0
#define NU2205_FSW_SET_SW_FREQ_300KHZ          0x1
#define NU2205_FSW_SET_SW_FREQ_400KHZ          0x2
#define NU2205_FSW_SET_SW_FREQ_500KHZ          0x3
#define NU2205_FSW_SET_SW_FREQ_600KHZ          0x4
#define NU2205_FSW_SET_SW_FREQ_700KHZ          0x5
#define NU2205_FSW_SET_SW_FREQ_800KHZ          0x6
#define NU2205_FSW_SET_SW_FREQ_900KHZ          0x7

#define NU2205_WTD_CONFIG_TIMING_500MS         500
#define NU2205_WTD_CONFIG_TIMING_1000MS        1000
#define NU2205_WTD_CONFIG_TIMING_5000MS        5000
#define NU2205_WTD_CONFIG_TIMING_30000MS       30000

#define NU2205_WTD_SET_500MS                   0
#define NU2205_WTD_SET_1000MS                  1
#define NU2205_WTD_SET_5000MS                  2
#define NU2205_WTD_SET_30000MS                 3

/* CHG_CTRL reg=0x0E */
#define NU2205_CHG_CTL_REG                     0x0E
#define NU2205_CHG_CTL_REG_INIT                0x00

#define NU2205_CHARGE_EN_MASK                  BIT(7)
#define NU2205_CHARGE_EN_SHIFT                 7
#define NU2205_CHG_ENABLE                      1
#define NU2205_CHG_DISABLE                     0
#define NU2205_CHARGE_MODE_MASK                (BIT(6) | BIT(5))
#define NU2205_CHARGE_MODE_SHIFT               5
#define NU2205_CHARGE_F42SC_MODE               0
#define NU2205_CHARGE_FBYPASS_MODE             1
#define NU2205_FREQ_SHIFT_MASK                 (BIT(4) | BIT(3))
#define NU2205_FREQ_SHIFT_SHIFT                3
#define NU2205_TSBUS_DIS_MASK                  BIT(2)
#define NU2205_TSBUS_DIS_SHIFT                 2
#define NU2205_TSBAT_DIS_MASK                  BIT(1)
#define NU2205_TSBAT_DIS_SHIFT                 1
#define NU2205_TDIE_DIS_MASK                   BIT(0)
#define NU2205_TDIE_DIS_SHIFT                  0

#define NU2205_TS_PROTECTION_ENABLE            0
#define NU2205_TS_PROTECTION_DISENABLE         1
#define NU2205_SW_FREQ_SHIFT_NORMAL            0
#define NU2205_SW_FREQ_SHIFT_P_P10             1 /* +10% */
#define NU2205_SW_FREQ_SHIFT_M_P10             2 /* -10% */
#define NU2205_SW_FREQ_SHIFT_MP_P10            3 /* +/-10% */

/* INT_STAT reg=0x0F */
#define NU2205_INT_STAT_REG                    0x0F

#define NU2205_BAT_OVP_ALM_STAT_MASK           BIT(7)
#define NU2205_BAT_OVP_ALM_STAT_SHIFT          7
#define NU2205_BAT_OCP_ALM_STAT_MASK           BIT(6)
#define NU2205_BAT_OCP_ALM_STAT_SHIFT          6
#define NU2205_BUS_OVP_ALM_STAT_MASK           BIT(5)
#define NU2205_BUS_OVP_ALM_STAT_SHIFT          5
#define NU2205_BUS_OCP_ALM_STAT_MASK           BIT(4)
#define NU2205_BUS_OCP_ALM_STAT_SHIFT          4
#define NU2205_BAT_UCP_ALM_STAT_MASK           BIT(3)
#define NU2205_BAT_UCP_ALM_STAT_SHIFT          3
#define NU2205_VOUT_NOT_PRES_STAT_MASK         BIT(2)
#define NU2205_VOUT_NOT_PRES_STAT_SHIFT        2
#define NU2205_VBAT_INSERT_STAT_MASK           BIT(1)
#define NU2205_VBAT_INSERT_STAT_SHIFT          1
#define NU2205_ADC_DONE_STAT_MASK              BIT(0)
#define NU2205_ADC_DONE_STAT_SHIFT             0

/* INT_FLAG reg=0x10 */
#define NU2205_INT_FLAG_REG                    0x10

#define NU2205_BAT_OVP_ALM_FLAG_MASK           BIT(7)
#define NU2205_BAT_OVP_ALM_FLAG_SHIFT          7
#define NU2205_BAT_OCP_ALM_FLAG_MASK           BIT(6)
#define NU2205_BAT_OCP_ALM_FLAG_SHIFT          6
#define NU2205_BUS_OVP_ALM_FLAG_MASK           BIT(5)
#define NU2205_BUS_OVP_ALM_FLAG_SHIFT          5
#define NU2205_BUS_OCP_ALM_FLAG_MASK           BIT(4)
#define NU2205_BUS_OCP_ALM_FLAG_SHIFT          4
#define NU2205_BAT_UCP_ALM_FLAG_MASK           BIT(3)
#define NU2205_BAT_UCP_ALM_FLAG_SHIFT          3
#define NU2205_VBAT_INSERT_FLAG_MASK           BIT(1)
#define NU2205_VBAT_INSERT_FLAG_SHIFT          1
#define NU2205_ADC_DONE_FLAG_MASK              BIT(0)
#define NU2205_ADC_DONE_FLAG_SHIFT             0

/* INT_MASK reg=0x11 */
#define NU2205_INT_MASK_REG                    0x11
#define NU2205_INT_MASK_REG_INIT               0x00

#define NU2205_BAT_OVP_ALM_MASK_MASK           BIT(7)
#define NU2205_BAT_OVP_ALM_MASK_SHIFT          7
#define NU2205_BAT_OCP_ALM_MASK_MASK           BIT(6)
#define NU2205_BAT_OCP_ALM_MASK_SHIFT          6
#define NU2205_BUS_OVP_ALM_MASK_MASK           BIT(5)
#define NU2205_BUS_OVP_ALM_MASK_SHIFT          5
#define NU2205_BUS_OCP_ALM_MASK_MASK           BIT(4)
#define NU2205_BUS_OCP_ALM_MASK_SHIFT          4
#define NU2205_BAT_UCP_ALM_MASK_MASK           BIT(3)
#define NU2205_BAT_UCP_ALM_MASK_SHIFT          3
#define NU2205_VBAT_INSERT_MASK_MASK           BIT(1)
#define NU2205_VBAT_INSERT_MASK_SHIFT          1
#define NU2205_ADC_DONE_MASK_MASK              BIT(0)
#define NU2205_ADC_DONE_MASK_SHIFT             0

/* FLT_STAT reg=0x12 */
#define NU2205_FLT_STAT_REG                    0x12

#define NU2205_BAT_OVP_FLT_STAT_MASK           BIT(7)
#define NU2205_BAT_OVP_FLT_STAT_SHIFT          7
#define NU2205_BAT_OCP_FLT_STAT_MASK           BIT(6)
#define NU2205_BAT_OCP_FLT_STAT_SHIFT          6
#define NU2205_BUS_OVP_FLT_STAT_MASK           BIT(5)
#define NU2205_BUS_OVP_FLT_STAT_SHIFT          5
#define NU2205_BUS_OCP_FLT_STAT_MASK           BIT(4)
#define NU2205_BUS_OCP_FLT_STAT_SHIFT          4
#define NU2205_BUS_RCP_FLT_STAT_MASK           BIT(3)
#define NU2205_BUS_RCP_FLT_STAT_SHIFT          3
#define NU2205_TS_ALM_STAT_MASK                BIT(2)
#define NU2205_TS_ALM_STAT_SHIFT               2
#define NU2205_TS_FLT_STAT_MASK                BIT(1)
#define NU2205_TS_FLT_STAT_SHIFT               1
#define NU2205_TDIE_ALM_STAT_MASK              BIT(0)
#define NU2205_TDIE_ALM_STAT_SHIFT             0

/* FLT_FLAG reg=0x13 */
#define NU2205_FLT_FLAG_REG                    0x13

#define NU2205_BAT_OVP_FLT_FLAG_MASK           BIT(7)
#define NU2205_BAT_OVP_FLT_FLAG_SHIFT          7
#define NU2205_BAT_OCP_FLT_FLAG_MASK           BIT(6)
#define NU2205_BAT_OCP_FLT_FLAG_SHIFT          6
#define NU2205_BUS_OVP_FLT_FLAG_MASK           BIT(5)
#define NU2205_BUS_OVP_FLT_FLAG_SHIFT          5
#define NU2205_BUS_OCP_FLT_FLAG_MASK           BIT(4)
#define NU2205_BUS_OCP_FLT_FLAG_SHIFT          4
#define NU2205_BUS_RCP_FLT_FLAG_MASK           BIT(3)
#define NU2205_BUS_RCP_FLT_FLAG_SHIFT          3
#define NU2205_TS_ALM_FLAG_MASK                BIT(2)
#define NU2205_TS_ALM_FLAG_SHIFT               2
#define NU2205_TS_FLT_FLAG_MASK                BIT(1)
#define NU2205_TS_FLT_FLAG_SHIFT               1
#define NU2205_TDIE_ALM_FLAG_MASK              BIT(0)
#define NU2205_TDIE_ALM_FLAG_SHIFT             0

/* FLT_MASK reg=0x14 */
#define NU2205_FLT_MASK_REG                    0x14
#define NU2205_FLT_MASK_REG_INIT               0x0F

#define NU2205_BAT_OVP_FLT_MASK_MASK           BIT(7)
#define NU2205_BAT_OVP_FLT_MASK_SHIFT          7
#define NU2205_BAT_OCP_FLT_MASK_MASK           BIT(6)
#define NU2205_BAT_OCP_FLT_MASK_SHIFT          6
#define NU2205_BUS_OVP_FLT_MASK_MASK           BIT(5)
#define NU2205_BUS_OVP_FLT_MASK_SHIFT          5
#define NU2205_BUS_OCP_FLT_MASK_MASK           BIT(4)
#define NU2205_BUS_OCP_FLT_MASK_SHIFT          4
#define NU2205_BUS_RCP_FLT_MASK_MASK           BIT(3)
#define NU2205_BUS_RCP_FLT_MASK_SHIFT          3
#define NU2205_TS_ALM_MASK_MASK                BIT(2)
#define NU2205_TS_ALM_MASK_SHIFT               2
#define NU2205_TS_FLT_MASK_MASK                BIT(1)
#define NU2205_TS_FLT_MASK_SHIFT               1
#define NU2205_TS_FLT_ENABLE                   0
#define NU2205_TS_FLT_DISABLE                  1
#define NU2205_TDIE_ALM_MASK_MASK              BIT(0)
#define NU2205_TDIE_ALM_MASK_SHIFT             0

/* ADC_CTRL reg=0x15 */
#define NU2205_ADC_CTRL_REG                    0x15
#define NU2205_ADC_CTRL_REG_INIT               0x00

#define NU2205_ADC_CTRL_EN_MASK                BIT(7)
#define NU2205_ADC_CTRL_EN_SHIFT               7
#define NU2205_ADC_ENABLE                      1
#define NU2205_ADC_DISABLE                     0
#define NU2205_ADC_RATE_MASK                   BIT(6)
#define NU2205_ADC_RATE_SHIFT                  6
#define NU2205_IBUS_ADC_DIS_MASK               BIT(1)
#define NU2205_IBUS_ADC_DIS_SHIFT              1
#define NU2205_VBUS_ADC_DIS_MASK               BIT(0)
#define NU2205_VBUS_ADC_DIS_SHIFT              0

/* ADC_FN_DISABLE reg=0x16 */
#define NU2205_ADC_FN_DIS_REG                  0x16
#define NU2205_ADC_FN_DIS_REG_INIT             0x00

#define NU2205_VAC1_ADC_DIS_MASK               BIT(7)
#define NU2205_VAC1_ADC_DIS_SHIFT              7
#define NU2205_VAC2_ADC_DIS_MASK               BIT(6)
#define NU2205_VAC2_ADC_DIS_SHIFT              6
#define NU2205_VOUT_ADC_DIS_MASK               BIT(5)
#define NU2205_VOUT_ADC_DIS_SHIFT              5
#define NU2205_VBAT_ADC_DIS_MASK               BIT(4)
#define NU2205_VBAT_ADC_DIS_SHIFT              4
#define NU2205_IBAT_ADC_DIS_MASK               BIT(3)
#define NU2205_IBAT_ADC_DIS_SHIFT              3
#define NU2205_TSBUS_ADC_DIS_MASK              BIT(2)
#define NU2205_TSBUS_ADC_DIS_SHIFT             2
#define NU2205_TSBAT_ADC_DIS_MASK              BIT(1)
#define NU2205_TSBAT_ADC_DIS_SHIFT             1
#define NU2205_TDIE_ADC_DIS_MASK               BIT(0)
#define NU2205_TDIE_ADC_DIS_SHIFT              0

/* IBUS_ADC1 reg=0x17 */
/* IBUS_ADC0 reg=0x18 */
#define NU2205_IBUS_ADC1_REG                   0x17
#define NU2205_IBUS_ADC0_REG                   0x18

#define NU2205_IBUS_ADC_STEP                   1 /* 1ma */

/* VBUS_ADC1 reg=0x19 */
/* VBUS_ADC0 reg=0x1A */
#define NU2205_VBUS_ADC1_REG                   0x19
#define NU2205_VBUS_ADC0_REG                   0x1A

#define NU2205_VBUS_ADC_STEP                   1 /* 1mv */

/* VAC1_ADC1 reg=0x1B */
/* VAC1_ADC0 reg=0x1C */
#define NU2205_VAC1_ADC1_REG                   0x1B
#define NU2205_VAC1_ADC0_REG                   0x1C

#define NU2205_VAC1_ADC_STEP                   1 /* 1mv */

/* VAC2_ADC1 reg=0x1D */
/* VAC2_ADC0 reg=0x1E */
#define NU2205_VAC2_ADC1_REG                   0x1D
#define NU2205_VAC2_ADC0_REG                   0x1E

#define NU2205_VAC2_ADC_STEP                   1 /* 1mv */

/* VOUT_ADC1 reg=0x1F */
/* VOUT_ADC0 reg=0x20 */
#define NU2205_VOUT_ADC1_REG                   0x1F
#define NU2205_VOUT_ADC0_REG                   0x20

#define NU2205_VOUT_ADC_STEP                   1 /* 1mv */

/* VBAT_ADC1 reg=0x21 */
/* VBAT_ADC0 reg=0x22 */
#define NU2205_VBAT_ADC1_REG                   0x21
#define NU2205_VBAT_ADC0_REG                   0x22

#define NU2205_VBAT_ADC_STEP                   1 /* 1mv */

/* IBAT_ADC1 reg=0x23 */
/* IBAT_ADC0 reg=0x24 */
#define NU2205_IBAT_ADC1_REG                   0x23
#define NU2205_IBAT_ADC0_REG                   0x24
#define NU2205_IBATADC_SIGN_MASK               BIT(7)
#define NU2205_IBATADC_NEGATIVE                (-1)
#define NU2205_IBATADC_POSITIVE                1

#define NU2205_IBAT_ADC_STEP                   1 /* 1ma */

/* TSBUS_ADC1 reg=0x25 */
/* TSBUS_ADC0 reg=0x26 */
#define NU2205_TSBUS_ADC1_REG                  0x25
#define NU2205_TSBUS_ADC1_MASK                 (BIT(7) | BIT(1) | BIT(0))

#define NU2205_TSBUS_ADC0_REG                  0x26

#define NU2205_TSBUS_ADC_STEP                  9766 /* 0.09766% */
#define NU2205_TSBUS_PER_MAX                   10000000

/* TSBAT_ADC1 reg=0x27 */
/* TSBAT_ADC0 reg=0x28 */
#define NU2205_TSBAT_ADC1_REG                  0x27
#define NU2205_TSBAT_ADC1_MASK                 (BIT(7) | BIT(1) | BIT(0))

#define NU2205_TSBAT_ADC0_REG                  0x28

#define NU2205_TSBAT_ADC_STEP                  9766 /* 0.09766% */
#define NU2205_TSBAT_PER_MAX                   10000000

/* TDIE_ADC1 reg=0x29 */
/* TDIE_ADC0 reg=0x2A */
#define NU2205_TDIE_ADC1_REG                   0x29
#define NU2205_TDIE_ADC0_REG                   0x2A

/* TSBUS_FLT reg=0x2B */
#define NU2205_TSBUS_FLT_REG                   0x2B

#define NU2205_TSBUS_FLT_MAX                   4980405 /* 49.80405% */
#define NU2205_TSBUS_FLT_BASE                  0
#define NU2205_TSBUS_FLT_DEFAULT               410151 /* 4.10151% */
#define NU2205_TSBUS_FLT_STEP                  19531 /* 0.19531% */

/* TSBAT_FLT reg=0x2C */
#define NU2205_TSBAT_FLT_REG                   0x2C

#define NU2205_TSBAT_FLT_MAX                   4980405 /* 49.80405% */
#define NU2205_TSBAT_FLT_BASE                  0
#define NU2205_TSBAT_FLT_DEFAULT               410151 /* 4.10151% */
#define NU2205_TSBAT_FLT_STEP                  19531 /* 0.19531% */

/* TDIE_ALM reg=0x2D */
#define NU2205_TDIE_ALM_REG                    0x2D

#define NU2205_TDIE_ALM_MAX                    1500 /* 150c */
#define NU2205_TDIE_ALM_BASE                   225  /* 22.5c */
#define NU2205_TDIE_ALM_DEFAULT                1200 /* 120c */
#define NU2205_TDIE_ALM_STEP                   5 /* 0.5c */

/* IBUS_UCP_RCP_THRESHOLD reg = 0x2E */
#define NU2205_IBUS_UCP_RCP_THRESHOLD_REG      0x2E

#define NU2105_SS_TIMEOUT_SET_MASK             (BIT(7) | BIT(6) | BIT(5))
#define NU2105_SS_TIMEOUT_SET_SHIFT            5
#define NU2105_SS_TIMEOUT_SET_100MS            4
#define NU2105_BUSSCP_FLAG_MASK                BIT(4)
#define NU2105_BUSSCP_FLAG_SHIFT               4
#define NU2105_BUSSCP_MASK_MASK                BIT(3)
#define NU2105_BUSSCP_MASK_SHIFT               3

#define NU2205_IBUS_UCP_THRES_MASK             BIT(2)
#define NU2205_IBUS_UCP_THRES_SHIFT            2
#define NU2205_UCP_RISE_500MA                  1 /* 500mA rising, 250mA falling */
#define NU2205_UCP_RISE_300MA                  0 /* 300 mA rising, 150 mA falling */
#define NU2205_IBAT_SNS_RES_MASK               BIT(1)
#define NU2205_IBAT_SNS_RES_SHIFT              1
#define NU2205_IBAT_SNS_RES_2MOHM              0
#define NU2205_IBAT_SNS_RES_1MOHM              1
#define NU2205_IBUS_RCP_THRES_MASK             BIT(0)
#define NU2205_IBUS_RCP_THRES_SHIFT            0

/* VAC_PRESENT_DET reg = 0x2F */
#define NU2205_VAC_PRESENT_DET_REG             0x2F

#define NU2205_VAC1PRESENT_STAT_MASK           BIT(7)
#define NU2205_VAC1PRESENT_STAT_SHIFT          7
#define NU2205_VAC1PRESENT_FLAG_MASK           BIT(6)
#define NU2205_VAC1PRESENT_FLAG_SHIFT          6
#define NU2205_VAC1PRESENT_MASK_MASK           BIT(5)
#define NU2205_VAC1PRESENT_MASK_SHIFT          5
#define NU2205_VAC2PRESENT_STAT_MASK           BIT(4)
#define NU2205_VAC2PRESENT_STAT_SHIFT          4
#define NU2205_VAC2PRESENT_FLAG_MASK           BIT(3)
#define NU2205_VAC2PRESENT_FLAG_SHIFT          3
#define NU2205_VAC2PRESENT_MASK_MASK           BIT(2)
#define NU2205_VAC2PRESENT_MASK_SHIFT          2
#define NU2205_EN_OTG_MASK                     BIT(1)
#define NU2205_EN_OTG_SHIFT                    1
#define NU2205_EN_OTG_ENABLE                   1
#define NU2205_EN_OTG_DISABLE                  0
#define NU2205_DIS_ACDRV_MASK                  BIT(0)
#define NU2205_DIS_ACDRV_SHIFT                 0
#define NU2205_DIS_ACDRV_ENABLE                1
#define NU2205_DIS_ACDRV_DISABLE               0

/* ACDRV_CTRL  reg = 0x30 */
#define NU2205_ACDRV_CTRL_REG                  0x30
#define NU2205_ACDRV_EN_ACDRV1_MASK            BIT(7)
#define NU2205_ACDRV_EN_ACDRV1_SHIFT           7
#define NU2205_ACDRV_EN_ACDRV1_ENABLE          1
#define NU2205_ACDRV_EN_ACDRV1_DISABLE         0

/* DEV_ID reg = 0x31 */
#define NU2205_DEVICE_ID_REG                   0x31
#define NU2205_DEVICE_ID_VALUE                 0x31

/* VBUS2VOUT_UVP_OVP reg = 0x32 */
#define NU2205_VBUS2VOUT_UVP_OVP_REG           0x32

#define NU2205_VBUS2VOUT_UVP_MASK              (BIT(7) | BIT(6))
#define NU2205_VBUS2VOUT_UVP_SHIFT             6
#define NU2205_VBUS2VOUT_OVP_MASK              (BIT(5) | BIT(4))
#define NU2205_VBUS2VOUT_OVP_SHIFT             4
#define NU2205_VBUS2VOUT_UVP_FLAG_MASK         BIT(3)
#define NU2205_VBUS2VOUT_UVP_FLAG_SHIFT        3
#define NU2205_VBUS2VOUT_OVP_FLAG_MASK         BIT(2)
#define NU2205_VBUS2VOUT_OVP_FLAG_SHIFT        2
#define NU2205_VBUS2VOUT_UVP_MASK_MASK         BIT(1)
#define NU2205_VBUS2VOUT_UVP_MASK_SHIFT        1
#define NU2205_VBUS2VOUT_OVP_MASK_MASK         BIT(0)
#define NU2205_VBUS2VOUT_OVP_MASK_SHIFT        0

#define NU2205_VBUS2VOUT_OVP_BASE              205
#define NU2205_VBUS2VOUT_OVP_MAX               220
#define NU2205_VBUS2VOUT_OVP_STEP              5

/* DEGLITCH reg = 0x33 */
#define NU2205_DEGLITCH_REG                    0x33

#define NU2205_VAC_PRESENT_DEG_MASK            BIT(7)
#define NU2205_VAC_PRESENT_DEG_SHIFT           7
#define NU2205_VAC_PRESENT_DEG_20MS            0
#define NU2205_VAC_PRESENT_DEG_10US            1
#define NU2205_VBUS_PRESENT_DEG_MASK           BIT(6)
#define NU2205_VBUS_PRESENT_DEG_SHIFT          6
#define NU2205_VBUS_PRESENT_DEG_20MS           0
#define NU2205_VBUS_PRESENT_DEG_10US           1
#define NU2205_IBUS_RCP_DEG_MASK               BIT(5)
#define NU2205_IBUS_RCP_DEG_SHIFT              5
#define NU2205_IBUS_RCP_DEG_8US                0
#define NU2205_IBUS_RCP_DEG_4MS                1
#define NU2205_CFLY_PRECHG_TIMEOUT_MASK        (BIT(4) | BIT(3))
#define NU2205_CFLY_PRECHG_TIMEOUT_SHIFT       3
#define NU2205_CFLY_PRECHG_TIMEOUT_20MS        0
#define NU2205_CFLY_PRECHG_TIMEOUT_40MS        1
#define NU2205_CFLY_PRECHG_TIMEOUT_60MS        2
#define NU2205_VBUS_VALID_DEG_MASK             BIT(2)
#define NU2205_VBUS_VALID_DEG_SHIFT            2
#define NU2205_VBUS_VALID_DEG_2MS              0
#define NU2205_VBUS_VALID_DEG_20MS             1
#define NU2205_IBUS_UCP_FALL_DG_SET_MASK       (BIT(1) | BIT(0))
#define NU2205_IBUS_UCP_FALL_DG_SET_SHIFT      0
#define NU2205_IBUS_UCP_FALL_DG_SET_10US       0
#define NU2205_IBUS_UCP_FALL_DG_SET_5MS        1
#define NU2205_IBUS_UCP_FALL_DG_SET_50MS       2
#define NU2205_IBUS_UCP_FALL_DG_SET_150MS      3

/* CP_OPTION reg = 0x34 */
#define NU2205_CP_OPTION_REG                   0x34

#define NU2205_POWER_NG_FLAG_MASK              BIT(7)
#define NU2205_POWER_NG_FLAG_SHIFT             7
#define NU2205_FORCE_SLEEP_MASK                BIT(6)
#define NU2205_FORCE_SLEEP_SHIFT               6
#define NU2205_BUSSCP_DIS_MASK                 BIT(5)
#define NU2205_BUSSCP_DIS_SHIFT                5
#define NU2205_PMID2VOUT_UVP_DIS_MASK          BIT(4)
#define NU2205_PMID2VOUT_UVP_DIS_SHIFT         4
#define NU2205_PMID2VOUT_OVP_DIS_MASK          BIT(3)
#define NU2205_PMID2VOUT_OVP_DIS_SHIFT         3
#define NU2205_VAC1_OVP_DIS_MASK               BIT(2)
#define NU2205_VAC1_OVP_DIS_SHIFT              2
#define NU2205_VAC2_OVP_DIS_MASK               BIT(1)
#define NU2205_VAC2_OVP_DIS_SHIFT              1


/* CP_OPTION2 reg = 0x37 */
#define NU2205_CP_OPTION2_REG                  0x37

#define NU2205_VBUS_PRESENT_STAT_MASK          BIT(7)
#define NU2205_VBUS_PRESENT_STAT_SHIFT         7
#define NU2205_VBUS_PRESENT_FLAG_MASK          BIT(6)
#define NU2205_VBUS_PRESENT_FLAG_SHIFT         6
#define NU2205_VBUS_PRESENT_MASK_MASK          BIT(5)
#define NU2205_VBUS_PRESENT_MASK_SHIFT         5
#define NU2205_POWER_SET_MASK                  BIT(4)
#define NU2205_POWER_SET_SHIFT                 4
#define NU2205_TDIE_FLT_MASK                   (BIT(2) | BIT(1) | BIT(0))
#define NU2205_TDIE_FLT_SHIFT                  0
#define NU2205_TDIE_FLT_BASE                   80
#define NU2205_TDIE_FLT_STEP                   10

#endif /* _NU2205_H_ */
