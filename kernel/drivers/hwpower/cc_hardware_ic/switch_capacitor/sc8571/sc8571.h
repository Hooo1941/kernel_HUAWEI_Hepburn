/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sc8571.h
 *
 * sc8571 header file
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

#ifndef _SC8571_H_
#define _SC8571_H_

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

enum sc8571_info {
	SC8571_INFO_IC_NAME = 0,
	SC8571_INFO_MAX_IBAT,
	SC8571_INFO_IBUS_OCP,
	SC8571_INFO_IBAT_OCP,
	SC8571_INFO_VBAT_OVP,
	SC8571_INFO_SW_FREQ,
	SC8571_INFO_TOTAL,
};

struct sc8571_mode_para {
	char ic_name[CHIP_DEV_NAME_LEN];
	int max_ibat;
	int ibus_ocp;
	int ibat_ocp;
	int vbat_ovp;
	int switching_frequency;
};

struct sc8571_dump_data {
	int vbus;
	int ibat;
	int vusb;
	int ibus;
	int temp;
};

struct sc8571_device_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct nty_data notify_data;
	struct dc_ic_ops sc_ops;
	struct dc_ic_ops lvc_ops;
	struct dc_batinfo_ops batinfo_ops;
	struct power_log_ops log_ops;
	struct sc8571_mode_para sc8571_lvc_para;
	struct sc8571_mode_para sc8571_sc_para;
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

#define SC8571_COMP_MAX_NUM                    4
#define SC8571_BUF_LEN                         80
#define SC8571_SWITCHCAP_DISABLE               0
#define BQ25980_CHARGER_CONFIG_ENABLE          1
#define BQ25980_CHARGER_CONFIG_DISABLE         0

/* commom para init */
#define SC8571_VBAT_OVP_TH_INIT                8800 /* mv */
#define SC8571_IBAT_OCP_TH_INIT                8300 /* ma */
#define SC8571_VBUS_OVP_TH_INIT                22000 /* mv */
#define SC8571_IBUS_OCP_TH_INIT                3000 /* ma */
#define SC8571_VAC_OVP_TH_INIT                 22000 /* mv */

/* lvc para init */
#define SC8571_LVC_IBUS_OCP_TH_INIT            7000 /* ma */
/* sc para init */
#define SC8571_SC_IBUS_OCP_TH_INIT             5000 /* ma */

/* VBAT_OVP reg=0x00 */
#define SC8571_VBAT_OVP_REG                    0x00
#define SC8571_VBAT_OVP_DIS_MASK               BIT(7)
#define SC8571_VBAT_OVP_DIS_SHIFT              7
#define	SC8571_VBAT_OVP_ENABLE                 0
#define	SC8571_VBAT_OVP_DISABLE                1
#define SC8571_VBAT_OVP_MASK                   (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC8571_VBAT_OVP_SHIFT                  0
#define SC8571_VBAT_OVP_STEP                   20 /* mv */
#define SC8571_VBAT_OVP_MAX                    9540 /* mv */
#define SC8571_VBAT_OVP_BASE                   7000 /* mv */
#define SC8571_VBAT_OVP_DEFAULT                8800 /* mv */

/* VBAT_OVP_ALM reg=0x01h */
#define SC8571_VBAT_OVP_ALM_REG                0x01
#define SC8571_VBAT_OVP_ALM_DIS_MASK           BIT(7)
#define SC8571_VBAT_OVP_ALM_DIS_SHIFT          7
#define SC8571_VBAT_OVP_ALM_ENABLE             0
#define SC8571_VBAT_OVP_ALM_DISABLE            1
#define SC8571_VBAT_OVP_ALM_MASK               (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC8571_VBAT_OVP_ALM_SHIFT              0
#define SC8571_VBAT_OVP_ALM_BASE               7000
#define SC8571_VBAT_OVP_ALM_STEP               20

/* IBAT_OCP reg=0x02 */
#define SC8571_IBAT_OCP_REG                    0x02
#define SC8571_IBAT_OCP_DIS_MASK               BIT(7)
#define SC8571_IBAT_OCP_DIS_SHIFT              7
#define SC8571_IBAT_OCP_MASK                   (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC8571_IBAT_OCP_SHIFT                  0
#define SC8571_IBAT_OCP_MAX                    12700 /* ma */
#define SC8571_IBAT_OCP_BASE                   0 /* ma */
#define BQ25980_IBAT_OCP_BASE                  2000 /* ma */
#define SC8571_IBAT_OCP_DEFAULT                8100 /* ma */
#define SC8571_IBAT_OCP_STEP                   100 /* ma */

/* IBAT_OCP_ALM reg=0x03 */
#define SC8571_IBAT_OCP_ALM_REG                0x03
#define SC8571_IBAT_OCP_ALM_DIS_MASK           BIT(7)
#define SC8571_IBAT_OCP_ALM_DIS_SHIFT          7
#define SC8571_IBAT_OCP_ALM_ENABLE             0
#define SC8571_IBAT_OCP_ALM_DISABLE            1

#define SC8571_IBAT_OCP_ALM_MASK               (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC8571_IBAT_OCP_ALM_SHIFT              0
#define SC8571_IBAT_OCP_ALM_BASE               0
#define SC8571_IBAT_OCP_ALM_STEP               100

/* CHARGER_CONFIG 1 reg=0x04 */
#define BQ25980_CHARGER_CONFIG_1_REG           0x04
#define BQ25980_CHARGER_CONFIG_1_MASK          BIT(7)
#define BQ25980_CHARGER_CONFIG_1_SHIFT         7

/* CHARGER_CONTROL 1 reg=0x05 */
#define BQ25980_CHARGER_CONTROL_1_REG          0x05
#define BQ25980_IBUS_UCP_DIS_MASK              BIT(7)
#define BQ25980_IBUS_UCP_DIS_SHIFT             7
#define BQ25980_IBUS_UCP_ENABLE                0
#define BQ25980_IBUS_UCP_MASK                  BIT(6)
#define BQ25980_IBUS_UCP_SHIFT                 6
#define BQ25980_IBUS_UCP_250MA                 1
#define BQ25980_IBUS_RCP_DIS_MASK              BIT(5)
#define BQ25980_IBUS_RCP_DIS_SHIFT             5
#define BQ25980_IBUS_RCP_ENABLE                0
#define BQ25980_IBUS_RCP_MASK                  BIT(4)
#define BQ25980_IBUS_RCP_SHIFT                 4
#define BQ25980_IBUS_RCP_900MA                 0
#define BQ25980_CHARGER_CONFIG_2_MASK          BIT(3)
#define BQ25980_CHARGER_CONFIG_2_SHIFT         3
#define BQ25980_VBUS_ERRHI_DIS_MASK            BIT(2)
#define BQ25980_VBUS_ERRHI_DIS_SHIFT           2
#define BQ25980_VBUS_ERRHI_DIS_ENABLE          0
#define BQ25980_VBUS_ERRHI_DIS_DISABLE         1
#define BQ25980_CHARGER_CONFIG_3_MASK          (BIT(1) | BIT(0))
#define BQ25980_CHARGER_CONFIG_3_SHIFT         1

/* VBUS_OVP reg=0x06 */
#define SC8571_VBUS_OVP_REG                    0x06
#define SC8571_BUS_PD_EN_MASK                  BIT(7)
#define SC8571_BUS_PD_EN_SHIFT                 7
#define SC8571_VBUS_OVP_MASK                   (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SC8571_VBUS_OVP_SHIFT                  0
#define SC8571_VBUS_OVP_1_1_MAX                13350 /* mv */
#define SC8571_VBUS_OVP_1_1_BASE               7000 /* mv */
#define SC8571_VBUS_OVP_1_1_DEFAULT            8900 /* mv */
#define SC8571_VBUS_OVP_1_1_STEP               50 /* mv */
#define SC8571_VBUS_OVP_2_1_BASE               14000 /* mv */
#define SC8571_VBUS_OVP_2_1_MAX                26700 /* mv */
#define SC8571_VBUS_OVP_2_1_STEP               100 /* mv */

#define BQ25980_VBUS_OVP_1_1_MAX               12570 /* mv */
#define BQ25980_VBUS_OVP_2_1_MAX               22000 /* mv */

/* VBUS_OVP_ALM reg=0x07 */
#define SC8571_VBUS_OVP_ALM_REG                0x07
#define SC8571_VBUS_OVP_ALM_DIS_MASK           0x80
#define SC8571_VBUS_OVP_ALM_DIS_SHIFT          7
#define SC8571_VBUS_OVP_ALM_ENABLE             0
#define SC8571_VBUS_OVP_ALM_DISABLE            1

#define SC8571_VBUS_OVP_ALM_MASK               0x7F
#define SC8571_VBUS_OVP_ALM_SHIFT              0
#define SC8571_VBUS_OVP_ALM_1_1_BASE           7000
#define SC8571_VBUS_OVP_ALM_1_1_LSB            50
#define SC8571_VBUS_OVP_ALM_2_1_BASE           14000
#define SC8571_VBUS_OVP_ALM_2_1_LSB            100

/* IBUS_OCP reg=0x08 */
#define SC8571_IBUS_OCP_REG                    0x08
#define SC8571_IBUS_OCP_DIS_MASK               BIT(7)
#define SC8571_IBUS_OCP_DIS_SHIFT              7
#define SC8571_IBUS_OCP_ENABLE                 0
#define SC8571_IBUS_OCP_DISABLE                1
#define SC8571_IBUS_OCP_MASK                   (BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SC8571_IBUS_OCP_SHIFT                  0
#define SC8571_IBUS_OCP_BASE                   1000 /* ma */
#define SC8571_IBUS_OCP_STEP                   250 /* ma */
#define SC8571_IBUS_OCP_MAX                    8000 /* ma */
#define SC8571_IBUS_OCP_DEFUALT                4250 /* ma */

#define BQ25980_IBUS_OCP_BASE                  1050 /* ma */
#define BQ25980_IBUS_OCP_STEP                  262 /* ma */
#define BQ25980_IBUS_OCP_1_1_MAX               8925 /* ma */
#define BQ25980_IBUS_OCP_2_1_MAX               6037 /* ma */
#define BQ25980_IBUS_OCP_DEFUALT               4462 /* ma */

/* CHARGER_CONFIG 4 reg=0x09 */
#define BQ25980_CHARGER_CONFIG_4_REG           0x09
#define BQ25980_CHARGER_CONFIG_4_MASK          BIT(7)
#define BQ25980_CHARGER_CONFIG_4_SHIFT         7

/* Over temperature protection reg=0x0A */
#define SC8571_TEPMP_CTRL_REG                  0x0A
#define SC8571_TSHUT_DIS_MASK                  BIT(7)
#define SC8571_TSHUT_DIS_SHIFT                 7
#define SC8571_TSHUT_ENABLE                    0
#define SC8571_TSHUT_DISABLE                   1

#define SC8571_TDIE_ALM_DIS_MASK               BIT(4)
#define SC8571_TDIE_ALM_DIS_SHIFT              4
#define SC8571_TDIE_ALM_ENABLE                 0
#define SC8571_TDIE_ALM_DISABLE                1

#define SC8571_TSBUS_FLT_DIS_MASK              BIT(3)
#define SC8571_TSBUS_FLT_DIS_SHIFT             3
#define SC8571_TSBUS_FLT_ENABLE                0
#define SC8571_TSBUS_FLT_DISABLE               1

#define SC8571_TSBAT_FLT_DIS_MASK              BIT(2)
#define SC8571_TSBAT_FLT_DIS_SHIFT             2
#define SC8571_TSBAT_FLT_ENABLE                0
#define SC8571_TSBAT_FLT_DISABLE               1

/* VAC_CTRL reg=0x0E */
#define SC8571_VAC_OVP_REG                     0x0E
#define SC8571_VAC1_OVP_MASK                   (BIT(7) | BIT(6) | BIT(5))
#define SC8571_VAC1_OVP_SHIFT                  5
#define SC8571_VAC1_OVP_BASE                   6500 /* mv */
#define SC8571_VAC1_OVP_MAX                    24000 /* mv */
#define SC8571_VAC1_OVP_STEP                   2500 /* mv */
#define SC8571_VAC1_PD_EN_MASK                 BIT(1)
#define SC8571_VAC1_PD_EN_SHIFT                1
#define SC8571_VAC1_PD_ENABLE                  1
#define SC8571_VAC1_PD_DISABLE                 0

#define SC8571_VAC2_OVP_MASK                   (BIT(4) | BIT(3) | BIT(2))
#define SC8571_VAC2_OVP_SHIFT                  2
#define SC8571_VAC2_OVP_MAX                    24000 /* mv */
#define SC8571_VAC2_OVP_BASE                   6500 /* mv */
#define SC8571_VAC2_OVP_DEFAULT                18000 /* actual 6.5v */
#define SC8571_VAC2_OVP_STEP                   2500 /* mv */
#define SC8571_VAC2_PD_EN_MASK                 BIT(0)
#define SC8571_VAC2_PD_EN_SHIFT                0
#define SC8571_VAC2_PD_ENABLE                  1
#define SC8571_VAC2_PD_DISABLE                 0

#define BQ25980_VAC_OVP_6500MV                 6500
#define BQ25980_VAC_OVP_10500MV                10500
#define BQ25980_VAC_OVP_12000MV                12000
#define BQ25980_VAC_OVP_14000MV                14000
#define BQ25980_VAC_OVP_16000MV                16000
#define BQ25980_VAC_OVP_18000MV                18000
#define BQ25980_VAC_OVP_22000MV                22000
#define BQ25980_VAC_OVP_24000MV                24000

#define BQ25980_VAC_OVP_SET_6500MV             0
#define BQ25980_VAC_OVP_SET_10500MV            1
#define BQ25980_VAC_OVP_SET_12000MV            2
#define BQ25980_VAC_OVP_SET_14000MV            3
#define BQ25980_VAC_OVP_SET_16000MV            4
#define BQ25980_VAC_OVP_SET_18000MV            5
#define BQ25980_VAC_OVP_SET_22000MV            6
#define BQ25980_VAC_OVP_SET_24000MV            7

/* CTRL2 reg=0x0F */
#define SC8571_CTRL2_REG                       0x0F
#define SC8571_REG_RST_MASK                    BIT(7)
#define SC8571_REG_RST_SHIFT                   7
#define SC8571_REG_RST_NOT_RESET               0
#define SC8571_REG_RST_RESET                   1

#define SC8571_OTG_EN_MASK                     BIT(5)
#define SC8571_OTG_EN_SHIFT                    5
#define SC8571_OTG_ENABLE                      1
#define SC8571_OTG_DISABLE                     0

#define SC8571_CHG_EN_MASK                     BIT(4)
#define SC8571_CHG_EN_SHIFT                    4
#define SC8571_CHG_ENABLE                      1
#define SC8571_CHG_DISABLE                     0

#define SC8571_CHARGE_MODE_MASK                BIT(3)
#define SC8571_CHARGE_MODE_SHIFT               3
#define SC8571_CHARGE_MODE_2_1                 0
#define SC8571_CHARGE_MODE_1_1                 1 /* bypass mode */

#define SC8571_DIS_ACDRV_BOTH_MASK             BIT(2)
#define SC8571_DIS_ACDRV_BOTH_SHIFT            2
#define SC8571_DIS_ACDRV_BOTH_OFF              0
#define SC8571_DIS_ACDRV_BOTH_ON               1

#define SC8571_ACDRV1_STAT_MASK                BIT(1)
#define SC8571_ACDRV1_STAT_SHIFT               1
#define SC8571_ACDRV1_OFF                      0
#define SC8571_ACDRV1_ON                       1

#define SC8571_ACDRV2_STAT_MASK                BIT(0)
#define SC8571_ACDRV2_STAT_SHIFT               0
#define SC8571_ACDRV2_OFF                      0
#define SC8571_ACDRV2_ON                       1

/* CTRL3 reg=0x10 */
#define SC8571_CTRL3_REG                       0x10
/* 0x84=disable watchdog, switching freq 500khz */
#define SC8571_CONTROL_REG_INIT                0x84
#define SC8571_FSW_SET_MASK                    (BIT(7) | BIT(6) | BIT(5))
#define SC8571_FSW_SET_SHIFT                   5
#define SC8571_SW_FREQ_300KHZ                  300
#define SC8571_SW_FREQ_350KHZ                  350
#define SC8571_SW_FREQ_400KHZ                  400
#define SC8571_SW_FREQ_450KHZ                  450
#define SC8571_SW_FREQ_500KHZ                  500
#define SC8571_SW_FREQ_550KHZ                  550
#define SC8571_SW_FREQ_600KHZ                  600
#define SC8571_SW_FREQ_750KHZ                  750
#define SC8571_FSW_SET_300KHZ                  0x0
#define SC8571_FSW_SET_350KHZ                  0x1
#define SC8571_FSW_SET_400KHZ                  0x2
#define SC8571_FSW_SET_450KHZ                  0x3
#define SC8571_FSW_SET_500KHZ                  0x4
#define SC8571_FSW_SET_550KHZ                  0x5
#define SC8571_FSW_SET_600KHZ                  0x6
#define SC8571_FSW_SET_750KHZ                  0x7

#define BQ25980_SW_FREQ_187P5KHZ               187
#define BQ25980_SW_FREQ_250KHZ                 250
#define BQ25980_SW_FREQ_300KHZ                 300
#define BQ25980_SW_FREQ_350KHZ                 350
#define BQ25980_SW_FREQ_375KHZ                 375
#define BQ25980_SW_FREQ_500KHZ                 500
#define BQ25980_SW_FREQ_550KHZ                 550
#define BQ25980_SW_FREQ_750KHZ                 750
#define BQ25980_FSW_SET_SW_FREQ_187P5KHZ       0x0
#define BQ25980_FSW_SET_SW_FREQ_250KHZ         0x1
#define BQ25980_FSW_SET_SW_FREQ_300KHZ         0x2
#define BQ25980_FSW_SET_SW_FREQ_375KHZ         0x3
#define BQ25980_FSW_SET_SW_FREQ_500KHZ         0x4
#define BQ25980_FSW_SET_SW_FREQ_750KHZ         0x5

#define SC8571_WATCHDOG_MASK                   (BIT(4) | BIT(3))
#define SC8571_WATCHDOG_SHIFT                  3
#define SC8571_WATCHDOG_650MS                  0
#define SC8571_WATCHDOG_1300MS                 1
#define SC8571_WATCHDOG_6500MS                 2
#define SC8571_WATCHDOG_39000MS                3

#define SC8571_WATCHDOG_DIS_MASK               BIT(2)
#define SC8571_WATCHDOG_DIS_SHIFT              2
#define SC8571_WATCHDOG_ENABLE                 0
#define SC8571_WATCHDOG_DISABLE                1

#define SC8571_WATCHDOG_CONFIG_TIMING_650MS    650
#define SC8571_WATCHDOG_CONFIG_TIMING_1300MS   1300
#define SC8571_WATCHDOG_CONFIG_TIMING_6500MS   6500
#define SC8571_WATCHDOG_CONFIG_TIMING_39000MS  39000

#define BQ25980_WATCHDOG_CONFIG_TIMING_500MS   500
#define BQ25980_WATCHDOG_CONFIG_TIMING_1000MS  1000
#define BQ25980_WATCHDOG_CONFIG_TIMING_5000MS  5000
#define BQ25980_WATCHDOG_CONFIG_TIMING_30000MS 30000

#define BQ25980_WATCHDOG_SET_500MS             0
#define BQ25980_WATCHDOG_SET_1000MS            1
#define BQ25980_WATCHDOG_SET_5000MS            2
#define BQ25980_WATCHDOG_SET_30000MS           3

/* CTRL4 reg=0x11 */
#define SC8571_CTRL4_REG                       0x11
#define SC8571_IBAT_SNS_RES_MASK               BIT(7)
#define SC8571_IBAT_SNS_RES_SHIFT              7
#define SC8571_IBAT_SNS_RES_2MHM               0
#define SC8571_IBAT_SNS_RES_5MHM               1

#define SC8571_SS_TIMEOUT_SET_MASK             (BIT(6) | BIT(5) | BIT(4))
#define SC8571_SS_TIMEOUT_SET_SHIFT            4
#define SC8571_SS_TIMEOUT_6P5MS                0
#define SC8571_SS_TIMEOUT_13MS                 1
#define SC8571_SS_TIMEOUT_26MS                 2
#define SC8571_SS_TIMEOUT_52MS                 3
#define SC8571_SS_TIMEOUT_104MS                4
#define SC8571_SS_TIMEOUT_416MS                5
#define SC8571_SS_TIMEOUT_1600MS               6
#define SC8571_SS_TIMEOUT_13000MS              7

#define SC8571_IBUS_UCP_FALL_DEG_MASK          (BIT(3) | BIT(2))
#define SC8571_IBUS_UCP_FALL_DEG_SHIFT         2
#define SC8571_IBUS_UCP_FALL_10US              0
#define SC8571_IBUS_UCP_FALL_5MS               1
#define SC8571_IBUS_UCP_FALL_50MS              2
#define SC8571_IBUS_UCP_FALL_150MS             3

/* CTRL5 reg=0x12 */
#define SC8571_CTRL5_REG                       0x12
#define SC8571_VOUT_OVP_DIS_MASK               BIT(7)
#define SC8571_VOUT_OVP_DIS_SHIFT              7
#define SC8571_VOUT_OVP_ENABLE                 0
#define SC8571_VOUT_OVP_DISABLE                1

#define SC8571_VOUT_OVP_MASK                   (BIT(6) | BIT(5))
#define SC8571_VOUT_OVP_SHIFT                  5
#define SC8571_VOUT_OVP_9_4V                   0
#define SC8571_VOUT_OVP_9_6V                   1
#define SC8571_VOUT_OVP_9_8V                   2
#define SC8571_VOUT_OVP_10V                    3

#define SC8571_FREQ_SHIFT_MASK                 (BIT(4) | BIT(3))
#define SC8571_FREQ_SHIFT_SHIFT                3
#define SC8571_FREQ_SHIFT_NORMINAL             0
#define SC8571_FREQ_SHIFT_POSITIVE10           1
#define SC8571_FREQ_SHIFT_NEGATIVE10           2
#define SC8571_FREQ_SHIFT_SPREAD_SPECTRUM      3
#define BQ25980_FREQ_SHIFT_NORMAL              0
#define BQ25980_FREQ_SHIFT_P_P10               1 /* +10% */
#define BQ25980_FREQ_SHIFT_M_P10               2 /* -10% */

#define SC8571_MS_MASK                         (BIT(1) | BIT(0))
#define SC8571_MS_SHIFT                        0
#define SC8571_MS_STANDALONE                   0
#define SC8571_MS_SLAVE                        1
#define SC8571_MS_MASTER                       2

/* STAT5 reg=0x17 */
#define SC8571_STAT5_REG                       0x17
#define SC8571_CP_SWITCHING_STAT_MASK          BIT(6)
#define SC8571_CP_SWITCHING_STAT_SHIFT         6

#define SC8571_VBUS_ERR_HI_STAT_MASK           BIT(4)
#define SC8571_VBUS_ERR_HI_STAT_SHIFT          4

#define SC8571_VBUS_ERR_LO_STAT_MASK           BIT(3)
#define SC8571_VBUS_ERR_LO_STAT_SHIFT          3

/* FLAG1 reg=0x18 */
#define SC8571_FLAG1_REG                       0x18
#define SC8571_VBAT_OVP_FLAG_MASK              BIT(7)
#define SC8571_VBAT_OVP_FLAG_SHIFT             7

#define SC8571_VBAT_OVP_ALM_FLAG_MASK          BIT(6)
#define SC8571_VBAT_OVP_ALM_FLAG_SHIFT         6

#define SC8571_OUT_OVP_FLAG_MASK               BIT(5)
#define SC8571_OUT_OVP_FLAG_SHIFT              5

#define SC8571_IBAT_OCP_FLAG_MASK              BIT(4)
#define SC8571_IBAT_OCP_FLAG_SHIFT             4

#define SC8571_IBAT_OCP_ALM_FLAG_MASK          BIT(3)
#define SC8571_IBAT_OCP_ALM_FLAG_SHIFT         3

#define SC8571_VBUS_OVP_FLAG_MASK              BIT(1)
#define SC8571_VBUS_OVP_FLAG_SHIFT             1

#define SC8571_VBUS_OVP_ALM_FLAG_MASK          BIT(0)
#define SC8571_VBUS_OVP_ALM_FLAG_SHIFT         0

/* FLAG2 reg=0x19 */
#define SC8571_FLAG2_REG                       0x19
#define SC8571_IBUS_OCP_FLAG_MASK              BIT(7)
#define SC8571_IBUS_OCP_FLAG_SHIFT             7

#define SC8571_BUS_UCP_FALL_FLAG_MASK          BIT(5)
#define SC8571_BUS_UCP_FALL_FLAG_SHIFT         5

#define SC8571_PIN_DIAG_FALL_FLAG_MASK         BIT(2)
#define SC8571_PIN_DIAG_FALL_FLAG_SHIFT        2

/* FLAG3 reg=0x1A */
#define SC8571_FLAG3_REG                       0x1A
#define SC8571_VAC1_OVP_FLAG_MASK              BIT(7)
#define SC8571_VAC1_OVP_FLAG_SHIFT             7

#define SC8571_VAC2_OVP_FLAG_MASK              BIT(6)
#define SC8571_VAC2_OVP_FLAG_SHIFT             6

#define SC8571_VBAT_INSERT_FLAG_MASK           BIT(5)
#define SC8571_VBAT_INSERT_FLAG_SHIFT          5

/* FLAG4 reg=0x1B */
#define SC8571_FLAG4_REG                       0x1B
#define SC8571_SS_TIMEOUT_FLAG_MASK            BIT(6)
#define SC8571_SS_TIMEOUT_FLAG_SHIFT           6

#define SC8571_TSBUS_TSBAT_ALM_FLAG_MASK       BIT(5)
#define SC8571_TSBUS_TSBAT_ALM_FLAG_SHIFT      5

#define SC8571_TSBUS_FLT_FLAG_MASK             BIT(4)
#define SC8571_TSBUS_FLT_FLAG_SHIFT            4

#define SC8571_TSBAT_FLT_FLAG_MASK             BIT(3)
#define SC8571_TSBAT_FLT_FLAG_SHIFT            3

#define SC8571_TSHUT_FLT_FLAG_MASK             BIT(2)
#define SC8571_TSHUT_FLT_FLAG_SHIFT            2

#define SC8571_TDIE_ALM_FLAG_MASK              BIT(1)
#define SC8571_TDIE_ALM_FLAG_SHIFT             1

#define SC8571_WD_TIMEOUT_FLAG_MASK            BIT(0)
#define SC8571_WD_TIMEOUT_FLAG_SHIFT           0

/* FLAG5 reg=0x1C */
#define SC8571_FLAG5_REG                       0x1C
#define SC8571_VBUS_ERR_HI_FLAG_MASK           BIT(4)
#define SC8571_VBUS_ERR_HI_FLAG_SHIFT          4

#define SC8571_VBUS_ERR_LO_FLAG_MASK           BIT(3)
#define SC8571_VBUS_ERR_LO_FLAG_SHIFT          3

/* MASK1 reg=0x1D */
#define SC8571_MASK1_REG                       0x1D
#define SC8571_BAT_OVP_ALM_MASK                BIT(6)
#define SC8571_BAT_OVP_ALM_MASK_SHIFT          6
#define SC8571_BAT_OVP_ALM_MASK_DISABLE        1

#define SC8571_BAT_OCP_ALM_MASK                BIT(3)
#define SC8571_BAT_OCP_ALM_MASK_SHIFT          3
#define SC8571_BAT_OCP_ALM_DISALLE             1

#define SC8571_BUS_OVP_ALM_MASK                BIT(0)
#define SC8571_BUS_OVP_ALM_MASK_SHIFT          0
#define SC8571_BUS_OVP_ALM_MASK_DISABLE        1

/* MASK4 reg=0x20 */
#define SC8571_MASK4_REG                       0x20
#define SC8571_TSBUS_TSBAT_ALM_MASK            BIT(5)
#define SC8571_TSBUS_TSBAT_ALM_MASK_SHIFT      5
#define SC8571_TSBUS_TSBAT_ALM_DISABLE         1

#define SC8571_TDIE_ALM_MASK                   BIT(1)
#define SC8571_TDIE_ALM_MASK_SHIFT             1
#define SC8571_TDIE_ALM_DISABLE                1

/* Device id info reg=0x22 */
#define SC8571_DEVICE_ID_REG                   0x22
#define SC8571_DEVICE_ID_VALUE                 0x41
#define BQ25980_DEVICE_ID_VALUE                0x12

/* ADC_CTRL reg=0x23 */
#define SC8571_ADC_CTRL_REG                    0x23
#define SC8571_ADC_EN_MASK                     BIT(7)
#define SC8571_ADC_EN_SHIFT                    7
#define SC8571_ADC_ENABLE                      1
#define SC8571_ADC_DISABLE                     0

#define SC8571_IBUS_ADC_DIS_MASK               BIT(1)
#define SC8571_IBUS_ADC_DIS_SHIFT              1
#define SC8571_IBUS_ADC_ENABLE                 0
#define SC8571_IBUS_ADC_DISABLE                1

#define SC8571_VBUS_ADC_DIS_MASK               BIT(0)
#define SC8571_VBUS_ADC_DIS_SHIFT              0
#define SC8571_VBUS_ADC_ENABLE                 0
#define SC8571_VBUS_ADC_DISABLE                1

/* BQ25980 ADC reg base ratio */
#define BQ25980_BASE_RATIO_UNIT                10000

/* IBUS_ADC1 0x25 */
#define SC8571_IBUS_ADC1_REG                   0x25
#define SC8571_IBUS_POL_H_MASK                 (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define BQ25980_IBUS_ADC_STEP                  10700 /* 1.07ma */

/* VBUS_ADC1 reg=0x27 */
#define SC8571_VBUS_ADC1_REG                   0x27
#define SC8571_VBUS_POL_H_MASK                 (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define BQ25980_VBUS_ADC_STEP                  10066 /* 1.0066mv */

/* VAC2_ADC1 reg=0x2B */
#define SC8571_VAC2_ADC1_REG                   0x2B
#define SC8571_VAC2_POL_H_MASK                 (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define BQ25980_VAC_ADC_STEP                   10056 /* 1.0056mv */

/* VBAT_ADC1 reg=0x2F */
#define SC8571_VBAT_ADC1_REG                   0x2F
#define SC8571_VBAT_POL_H_MASK                 (BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define BQ25980_VBAT_ADC_STEP                  10055 /* 1.0055mv */

/* IBAT_ADC1 reg=0x31 */
#define SC8571_IBAT_ADC1_REG                   0x31
#define SC8571_IBAT_POL_H_MASK                 (BIT(3) | BIT(2) | BIT(1) | BIT(0))

/* TDIE_ADC1 reg=0x37 */
#define SC8571_TDIE_ADC1_REG                   0x37
#define SC8571_TDIE_POL_H_MASK                 BIT(0)

/* INTERNAL1 reg=0x40 */
#define SC8571_INTERNAL1_REG                   0x40
#define SC8571_ACDRV_MANUAL_EN_MASK            BIT(6)
#define SC8571_ACDRV_MANUAL_EN_SHIFT           6
#define SC8571_ACDRV_MANUAL_EN                 1
#define SC8571_ACDRV_MANUAL_OFF                0

#define SC8571_ACDRV1_EN_MASK                  BIT(5)
#define SC8571_ACDRV1_EN_SHIFT                 5
#define SC8571_ACDRV1_EN_ENABLE                1
#define SC8571_ACDRV1_EN_DISABLE               0

/* PMID2OUT_OVP_UVP reg=0x42 */
#define SC8571_PMID2OUT_OVP_UVP_REG            0x42
#define SC8571_PMID2OUT_OVP_UVP_INIT           0x5C

#define BQ25980_TDIE_ADC_STEP                  5150 /* 0.515C */

#endif /* _SC8571_H_ */
