/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sy6513.h
 *
 * sy6513 header file
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

#ifndef _SY6513_H_
#define _SY6513_H_

#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/raid/pq.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>
#include <chipset_common/hwpower/hardware_channel/power_sw.h>

#define SY6513_NOT_USED                                0
#define SY6513_USED                                    1
#define SY6513_NOT_INIT                                0
#define SY6513_INIT_FINISH                             1
#define SY6513_HOST_MODE                               0
#define SY6513_SLAVE_MODE                              1
#define SY6513_STANDALONE_MODE                         0
#define SY6513_ENABLE_INT_NOTIFY                       1
#define SY6513_DISABLE_INT_NOTIFY                      0
#define SY6513_BUF_LEN                                 80
#define SY6513_DEVICE_ID_GET_FAIL                      (-1)
#define SY6513_IRQ_FLAG_REG_NUM                        6
#define SY6513_DEVICE_ID                               0x11
#define IBAT_IBUS_SYMBOL_BIT                           15

#define SY6513_SCP_MAX_DATA_LEN                        32
#define SY6513_NOT_USED                                0
#define SY6513_USED                                    1
#define SY6513_INIT_FINISH                             1
#define SY6513_BUF_LEN                                 80
#define SY6513_DEVICE_ID_GET_FAIL                      (-1)
#define SY6513_SWITCHCAP_DISABLE                       0
#define SY6513_SWITCHCAP_ENABLE                        1
#define SY6513_GPIO_DISABLE                            0
#define SY6513_GPIO_ENABLE                             1
#define SY6513_CHIP_ID_REG_NUM                         3

/* DEVICE_VER reg=0x00 */
#define SY6513_DEVICE_VER_REG                          0x00
#define SY6513_DEVICE_VER_MASK                         0xFF
#define SY6513_DEVICE_VER_SHIFT                        0

/* DEVICE1_VER reg=0x01 */
#define SY6513_DEVICE1_VER_REG                         0x01
#define SY6513_DEVICE1_VER_MASK                        0xFF
#define SY6513_DEVICE1_VER_SHIFT                       0

/* DEVICE2_VER reg=0x02 */
#define SY6513_DEVICE2_VER_REG                         0x02
#define SY6513_DEVICE2_VER_MASK                        0xFF
#define SY6513_DEVICE2_VER_SHIFT                       0

/* Charge MODE C ontrol1 reg=0x03 */
#define SY6513_CHARGE_MODE_CONTROL1_REG                0x03
#define SY6513_REG_RST_ENABLE_MASK                     BIT(7)
#define SY6513_REG_RST_ENABLE_SHIFT                    7
#define SY6513_CHG_MODE_MASK                           (BIT(6) | BIT(5) | BIT(4))
#define SY6513_CHG_MODE_SHIFT                          4
#define SY6513_WATCHDOG_ENABLE_MASK                    BIT(3)
#define SY6513_WATCHDOG_ENABLE_SHIFT                   3
#define SY6513_WATCHDOG_TIMER_SET_MASK                 (BIT(2) | BIT(1) | BIT(0))
#define SY6513_WATCHDOG_TIMER_SET_SHIFT                0
#define SY6513_REG_RST_DISABLE                         0
#define SY6513_CHG_FBYPASS_MODE                        0
#define SY6513_CHG_F21SC_MODE                          1
#define SY6513_CHG_F41SC_MODE                          2
#define SY6513_CHG_RBYPASS_MODE                        3
#define SY6513_CHG_R12SC_MODE                          4
#define SY6513_CHG_R14SC_MODE                          5
#define SY6513_WATCHDOG_TIMER_5000MS                   5000
#define SY6513_WATCHDOG_TIMER_2000MS                   2000
#define SY6513_WATCHDOG_TIMER_1000MS                   1000
#define SY6513_WATCHDOG_TIMER_500MS                    500
#define SY6513_WATCHDOG_DISABLE                        1
#define SY6513_WATCHDOG_ENABLE                         0
#define SY6513_WATCHDOG_TIMER_0P5S                     0
#define SY6513_WATCHDOG_TIMER_1S                       1
#define SY6513_WATCHDOG_TIMER_2S                       2
#define SY6513_WATCHDOG_TIMER_5S                       3

/* Charge_MODE_Control2 reg=0x04 */
#define SY6513_CHARGE_MODE_CONTROL2_REG                0x04
#define SY6513_CHARGE_ENABLE_MASK                      BIT(7)
#define SY6513_CHARGE_ENABLE_SHIFT                     7
#define SY6513_CP_SWITCHING_STAT_MASK                  BIT(4)
#define SY6513_CP_SWITCHING_STAT_SHIFT                 4
#define SY6513_USB_OVP_MODE_MASK                       BIT(3)
#define SY6513_USB_OVP_MODE_SHIFT                      3
#define SY6513_USB_OVP_DRIVER_ENABLE_MASK              BIT(2)
#define SY6513_USB_OVP_DRIVER_ENABLE_SHIFT             2
#define SY6513_WPC_OVP_MODE_MASK                       BIT(1)
#define SY6513_WPC_OVP_MODE_SHIFT                      1
#define SY6513_WPC_OVP_DRIVER_ENABLE_MASK              BIT(0)
#define SY6513_WPC_OVP_DRIVER_ENABLE_SHIFT             0
#define SY6513_CHARGE_DISABLE                          0
#define SY6513_CHARGE_ENABLE                           1
#define SY6513_CP_SWITCHING_STAT_DISABLE               0
#define SY6513_CP_SWITCHING_STAT_ENABLE                1
#define SY6513_USB_OVP_MODE_FORWARD                    0
#define SY6513_USB_OVP_MODE_REVERSE                    1
#define SY6513_USB_OVP_DRIVER_ENABLE                   1
#define SY6513_WPC_OVP_MODE_FORWARD                    0
#define SY6513_WPC_OVP_MODE_REVERSE                    1
#define SY6513_WPC_OVP_DRIVER_ENABLE                   1

/* USB_OVP_ENABLE reg=0x05 */
#define SY6513_USB_OVP_ENABLE_REG                      0x05
#define SY6513_VUSB_OVP_ALM_RNG_MASK                   (BIT(5) | BIT(4) | BIT(3))
#define SY6513_VUSB_OVP_ALM_RNG_SHIFT                  3
#define SY6513_VUSB_OVP_RNG_MASK                       (BIT(2) | BIT(1) | BIT(0))
#define SY6513_VUSB_OVP_RNG_SHIFT                      0
#define SY6513_VUSB_OVP_ALM_6P5V                       0
#define SY6513_VUSB_OVP_ALM_10V                        1
#define SY6513_VUSB_OVP_ALM_11V                        2
#define SY6513_VUSB_OVP_ALM_12V                        3
#define SY6513_VUSB_OVP_ALM_19V                        4
#define SY6513_VUSB_OVP_ALM_20V                        5
#define SY6513_VUSB_OVP_ALM_21V                        6
#define SY6513_VUSB_OVP_ALM_22V                        7
#define SY6513_VUSB_OVP_7P5V                           0
#define SY6513_VUSB_OVP_11V                            1
#define SY6513_VUSB_OVP_12V                            2
#define SY6513_VUSB_OVP_13V                            3
#define SY6513_VUSB_OVP_20V                            4
#define SY6513_VUSB_OVP_21V                            5
#define SY6513_VUSB_OVP_22V                            6
#define SY6513_VUSB_OVP_23V                            7

/* WPC OVP ENABLE reg=0x06 */
#define SY6513_WPC_OVP_ENABLE_REG                      0x06
#define SY6513_VWPC_OVP_ALM_RNG_MASK                   (BIT(5) | BIT(4) | BIT(3))
#define SY6513_VWPC_OVP_ALM_RNG_SHIFT                  3
#define SY6513_VWPC_OVP_RNG_MASK                       (BIT(2) | BIT(1) | BIT(0))
#define SY6513_VWPC_OVP_RNG_SHIFT                      0
#define SY6513_VWPC_OVP_ALM_6P5V                       0
#define SY6513_VWPC_OVP_ALM_10V                        1
#define SY6513_VWPC_OVP_ALM_11V                        2
#define SY6513_VWPC_OVP_ALM_12V                        3
#define SY6513_VWPC_OVP_ALM_19V                        4
#define SY6513_VWPC_OVP_ALM_20V                        5
#define SY6513_VWPC_OVP_ALM_21V                        6
#define SY6513_VWPC_OVP_ALM_22V                        7
#define SY6513_VWPC_OVP_7P5V                           0
#define SY6513_VWPC_OVP_11V                            1
#define SY6513_VWPC_OVP_12V                            2
#define SY6513_VWPC_OVP_13V                            3
#define SY6513_VWPC_OVP_20V                            4
#define SY6513_VWPC_OVP_21V                            5
#define SY6513_VWPC_OVP_22V                            6
#define SY6513_VWPC_OVP_23V                            7

/* SC control reg=0x07 */
#define SY6513_SC_CONTROL_REG                          0x07
#define SY6513_SWITCHING_FREQUENCY_MASK                (BIT(7) | BIT(6) | BIT(5))
#define SY6513_SWITCHING_FREQUENCY_SHIFT               5
#define SY6513_CHG_FSHIFT_MASK                         (BIT(4) | BIT(3))
#define SY6513_CHG_FSHIFT_SHIFT                        3
#define SY6513_CHG_FSHIFT_EN_MASK                      BIT(2)
#define SY6513_CHG_FSHIFT_EN_SHIFT                     2
#define SY6513_SWITCHING_FREQUENCY_400KHZ              0
#define SY6513_SWITCHING_FREQUENCY_500KHZ              1
#define SY6513_SWITCHING_FREQUENCY_600KHZ              2
#define SY6513_SWITCHING_FREQUENCY_750KHZ              3
#define SY6513_SWITCHING_FREQUENCY_850KHZ              4
#define SY6513_SWITCHING_FREQUENCY_1000KHZ             5
#define SY6513_SWITCHING_FREQUENCY_300KHZ              6
#define SY6513_SWITCHING_FREQUENCY_350KHZ              7
#define SY6513_SWITCHING_FREQUENCY_MIN                 300
#define SY6513_SWITCHING_FREQUENCY_MAX                 1000
#define SY6513_SWITCHING_FREQUENCY_LEVEL_NUM           8
#define SY6513_CHG_FSHIFT_NOMINAL_FREQUENCY            0
#define SY6513_CHG_FSHIFT_ADD10_PERSENT                1
#define SY6513_CHG_FSHIFT_SUB10_PERSENT                2
#define SY6513_CHG_FSHIFT_DISABLE                      0
#define SY6513_CHG_FSHIFT_ENABLE                       1

/* VBUS_VOUT_OVP reg=0x08 */
#define SY6513_VBUS_VOUT_OVP_REG                       0x08
#define SY6513_41SC_VBUS_OVP_MASK                      BIT(7)
#define SY6513_41SC_VBUS_OVP_SHIFT                     7
#define SY6513_21SC_VBUS_OVP_MASK                      BIT(6)
#define SY6513_21SC_VBUS_OVP_SHIFT                     6
#define SY6513_BPS_VBUS_OVP_MASK                       BIT(5)
#define SY6513_BPS_VBUS_OVP_SHIFT                      5
#define SY6513_VOUT_OVP_MASK                           (BIT(4) | BIT(3))
#define SY6513_VOUT_OVP_SHIFT                          3
#define SY6513_VOUT_OVP_TDB_MASK                       BIT(2)
#define SY6513_VOUT_OVP_TDB_SHIFT                      2
#define SY6513_41SC_VBUS_OVP_MIN                       21
#define SY6513_41SC_VBUS_OVP_MAX                       22
#define SY6513_21SC_VBUS_OVP_MIN                       12
#define SY6513_21SC_VBUS_OVP_MAX                       13
#define SY6513_21SC_VBUS_OVP_STEP                      1000
#define SY6513_BPS_VBUS_OVP_MIN                        5600
#define SY6513_BPS_VBUS_OVP_MAX                        6000
#define SY6513_BPS_VBUS_OVP_5P6V                       0
#define SY6513_BPS_VBUS_OVP_6V                         1
#define SY6513_VOUT_OVP_MIN                            4600
#define SY6513_VOUT_OVP_INIT                           4800
#define SY6513_VOUT_OVP_MAX                            5200
#define SY6513_VOUT_OVP_STEP                           200
#define SY6513_VOUT_OVP_TDB_100US                      0
#define SY6513_VOUT_OVP_TDB_1MS                        1
#define SY6513_VBUS_OVP_FBYPASS_5600MV                 0
#define SY6513_VBUS_OVP_FBYPASS_6000MV                 1
#define SY6513_VBUS_OVP_F21SC_12000MV                  0
#define SY6513_VBUS_OVP_F21SC_13000MV                  1
#define SY6513_VBUS_OVP_F41SC_21000MV                  0
#define SY6513_VBUS_OVP_F41SC_22000MV                  1

/* MOSFET OCP reg=0x09 */
#define SY6513_MOSFET_OCP_REG                          0x09
#define SY6513_IBUS_OCP_PEAK_MASK                      (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SY6513_IBUS_OCP_PEAK_SHIFT                     4
#define SY6513_CVT_OCP_PEAK_MASK                       (BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_CVT_OCP_PEAK_SHIFT                      0
#define SY6513_IBUS_OCP_PEAK_MIN                       7500
#define SY6513_IBUS_OCP_PEAK_INIT                      10500
#define SY6513_IBUS_OCP_PEAK_MAX                       15000
#define SY6513_IBUS_OCP_PEAK_STEP                      500
#define SY6513_CVT_OCP_PEAK_MIN                        2000
#define SY6513_CVT_OCP_PEAK_INIT                       11000
#define SY6513_CVT_OCP_PEAK_MAX                        15500
#define SY6513_CVT_OCP_PEAK_STEP                       900

/* SC_VDROP_OVP reg=0x0A */
#define SY6513_SC_VDROP_OVP_REG                        0x0A
#define SY6513_VDROP_MIN_MASK                          (BIT(7) | BIT(6))
#define SY6513_VDROP_MIN_SHIFT                         6
#define SY6513_VDROP_MIN_TDB_MASK                      BIT(5)
#define SY6513_VDROP_MIN_TDB_SHIFT                     5
#define SY6513_VDROP_OVP_MASK                          (BIT(4) | BIT(3))
#define SY6513_VDROP_OVP_SHIFT                         3
#define SY6513_VDROP_OVP_TDB_MASK                      BIT(2)
#define SY6513_VDROP_OVP_TDB_SHIFT                     2
#define SY6513_RBPS_IBUS_OCP_MASK                      (BIT(1) | BIT(0))
#define SY6513_RBPS_IBUS_OCP_SHIFT                     0
#define SY6513_VDROP_MIN_MIN                           (-400)
#define SY6513_VDROP_MIN_INIT                          (-200)
#define SY6513_VDROP_MIN_MAX                           (-100)
#define SY6513_VDROP_MIN_STEP                          100
#define SY6513_VDROP_MIN_TDB_8US                       0
#define SY6513_VDROP_MIN_TDB_1MS                       1
#define SY6513_VDROP_OVP_MIN                           600
#define SY6513_VDROP_OVP_INIT                          600
#define SY6513_VDROP_OVP_MAX                           1200
#define SY6513_VDROP_OVP_STEP                          200
#define SY6513_VDROP_OVP_TDB_0US                       0
#define SY6513_VDROP_OVP_TDB_128US                     1
#define SY6513_RBPS_IBUS_OCP_0P5A                      0
#define SY6513_RBPS_IBUS_OCP_1A                        1
#define SY6513_RBPS_IBUS_OCP_2A                        2
#define SY6513_RBPS_IBUS_OCP_3A                        3

/* IBUS_PROTECTION reg=0x0B */
#define SY6513_IBUS_PROTECTION_REG                     0x0B
#define SY6513_41SC_IBUS_OCP_MASK                      BIT(7)
#define SY6513_41SC_IBUS_OCP_SHIFT                     7
#define SY6513_21SC_IBUS_OCP_MASK                      BIT(6)
#define SY6513_21SC_IBUS_OCP_SHIFT                     6
#define SY6513_14SC_IBUS_OCP_MASK                      BIT(5)
#define SY6513_14SC_IBUS_OCP_SHIFT                     5
#define SY6513_12SC_IBUS_OCP_MASK                      BIT(4)
#define SY6513_12SC_IBUS_OCP_SHIFT                     4
#define SY6513_IBUS_OCP_TDB_MASK                       BIT(3)
#define SY6513_IBUS_OCP_TDB_SHIFT                      3
#define SY6513_IBUS_UCP_TDB_MASK                       BIT(2)
#define SY6513_IBUS_UCP_TDB_SHIFT                      2
#define SY6513_IBUS_UCP_HYS_TDLY_MASK                  (BIT(1) | BIT(0))
#define SY6513_IBUS_UCP_HYS_TDLY_SHIFT                 0
#define SY6513_41SC_IBUS_OCP_3750MA                    0
#define SY6513_41SC_IBUS_OCP_3750MA_VALUE              3750
#define SY6513_41SC_IBUS_OCP_4000MA                    1
#define SY6513_21SC_IBUS_OCP_6000MA                    0
#define SY6513_21SC_IBUS_OCP_6000MA_VALUE              6000
#define SY6513_21SC_IBUS_OCP_7000MA                    1
#define SY6513_14SC_IBUS_OCP_1250MA                    0
#define SY6513_14SC_IBUS_OCP_1500MA                    1
#define SY6513_12SC_IBUS_OCP_2500MA                    0
#define SY6513_12SC_IBUS_OCP_3000MA                    1
#define SY6513_IBUS_OCP_TDB_4US                        0
#define SY6513_IBUS_OCP_TDB_64US                       1
#define SY6513_IBUS_UCP_TDB_32MS                       0
#define SY6513_IBUS_UCP_TDB_256MS                      1
#define SY6513_IBUS_UCP_HYS_TDLY_100MS                 0
#define SY6513_IBUS_UCP_HYS_TDLY_200MS                 1
#define SY6513_IBUS_UCP_HYS_TDLY_400MS                 2
#define SY6513_IBUS_UCP_HYS_TDLY_800MS                 3

/* VBUS_PROTECTION reg=0x0C */
#define SY6513_VBUS_PROTECTION_REG                     0x0C
#define SY6513_TDIE_OTP_MASK                           (BIT(7) | BIT(6))
#define SY6513_TDIE_OTP_SHIFT                          6
#define SY6513_TDIE_OTP_ALM_MASK                       (BIT(5) | BIT(4))
#define SY6513_TDIE_OTP_ALM_SHIFT                      4
#define SY6513_FWD_VBUS_UVLO_TDB_MASK                  BIT(3)
#define SY6513_FWD_VBUS_UVLO_TDB_SHIFT                 3
#define SY6513_IBUS_RCP_TDB_MASK                       BIT(2)
#define SY6513_IBUS_RCP_TDB_SHIFT                      2
#define SY6513_41FWD_VBUS_UVP_MASK                     BIT(1)
#define SY6513_41FWD_VBUS_UVP_SHIFT                    1
#define SY6513_21FWD_VBUS_UVP_MASK                     BIT(0)
#define SY6513_21FWD_VBUS_UVP_SHIFT                    0
#define SY6513_TDIE_OTP_MIN                            80
#define SY6513_TDIE_OTP_INIT                           140
#define SY6513_TDIE_OTP_MAX                            140
#define SY6513_TDIE_OTP_STEP                           20
#define SY6513_TDIE_OTP_ALM_MIN                        60
#define SY6513_TDIE_OTP_ALM_INIT                       80
#define SY6513_TDIE_OTP_ALM_MAX                        100
#define SY6513_TDIE_OTP_ALM_STEP                       20
#define SY6513_FWD_VBUS_UVLO_TDB_10MS                  0
#define SY6513_FWD_VBUS_UVLO_TDB_20MS                  1
#define SY6513_IBUS_RCP_TDB_100NS                      0
#define SY6513_IBUS_RCP_TDB_4US                        1
#define SY6513_41FWD_VBUS_UVP_12V                      0
#define SY6513_41FWD_VBUS_UVP_13V                      1
#define SY6513_21FWD_VBUS_UVP_6V                       0
#define SY6513_21FWD_VBUS_UVP_6P5V                     1

/* VBAT_PROTECTION reg=0x0D */
#define SY6513_VBAT_PROTECTION_REG                     0x0D
#define SY6513_FWD_VBAT1_OVP_MASK                      (BIT(7) | BIT(6) | BIT(5))
#define SY6513_FWD_VBAT1_OVP_SHIFT                     5
#define SY6513_FWD_VBAT1_OVP_TDB_MASK                  BIT(4)
#define SY6513_FWD_VBAT1_OVP_TDB_SHIFT                 4
#define SY6513_FWD_VBAT2_OVP_MASK                      (BIT(3) | BIT(2) | BIT(1))
#define SY6513_FWD_VBAT2_OVP_SHIFT                     1
#define SY6513_FWD_VBAT2_OVP_TDB_MASK                  BIT(0)
#define SY6513_FWD_VBAT2_OVP_TDB_SHIFT                 0
#define SY6513_FWD_VBAT1_OVP_MIN                       4500
#define SY6513_FWD_VBAT1_OVP_INIT                      4600
#define SY6513_FWD_VBAT1_OVP_MAX                       5200
#define SY6513_FWD_VBAT1_OVP_STEP                      100
#define SY6513_FWD_VBAT1_OVP_TDB_0US                   0
#define SY6513_FWD_VBAT1_OVP_TDB_128US                 1
#define SY6513_FWD_VBAT2_OVP_MIN                       4500
#define SY6513_FWD_VBAT2_OVP_INIT                      4600
#define SY6513_FWD_VBAT2_OVP_MAX                       5200
#define SY6513_FWD_VBAT2_OVP_STEP                      100
#define SY6513_FWD_VBAT2_OVP_TDB_0US                   0
#define SY6513_FWD_VBAT2_OVP_TDB_128US                 1

/* IBAT_PROTECTION reg=0x0E */
#define SY6513_IBAT_PROTECTION_REG                     0x0E
#define SY6513_FWD_IBAT_OCP_MASK                       (BIT(7) | BIT(6) | BIT(5) | BIT(4))
#define SY6513_FWD_IBAT_OCP_SHIFT                      4
#define SY6513_RSENSE_MASK                             (BIT(3) | BIT(2))
#define SY6513_RSENSE_SHIFT                            2
#define SY6513_PLACEMENT_RSENSE_MASK                   BIT(1)
#define SY6513_PLACEMENT_RSENSE_SHIFT                  1
#define SY6513_FWD_IBAT_OCP_INIT                       8500
#define SY6513_FWD_IBAT_OCP_5000MA                     0
#define SY6513_FWD_IBAT_OCP_5500MA                     1
#define SY6513_FWD_IBAT_OCP_6000MA                     2
#define SY6513_FWD_IBAT_OCP_6500MA                     3
#define SY6513_FWD_IBAT_OCP_8500MA                     4
#define SY6513_FWD_IBAT_OCP_9000MA                     5
#define SY6513_FWD_IBAT_OCP_9500MA                     6
#define SY6513_FWD_IBAT_OCP_10000MA                    7
#define SY6513_FWD_IBAT_OCP_11000MA                    8
#define SY6513_FWD_IBAT_OCP_12000MA                    9
#define SY6513_FWD_IBAT_OCP_12500MA                    10
#define SY6513_FWD_IBAT_OCP_13000MA                    11
#define SY6513_FWD_IBAT_OCP_13500MA                    12
#define SY6513_FWD_IBAT_OCP_14000MA                    13
#define SY6513_FWD_IBAT_OCP_14500MA                    14
#define SY6513_FWD_IBAT_OCP_15000MA                    15
#define SY6513_RSENSE_0P5MOHM                          0
#define SY6513_RSENSE_1MOHM                            1
#define SY6513_RSENSE_2MOHM                            2
#define SY6513_PLACEMENT_RSENSE_LOW                    0
#define SY6513_PLACEMENT_RSENSE_HIGH                   1

/* PULL_DOWN_CONFIG reg=0x0F */
#define SY6513_PULL_DOWN_CONFIG_REG                    0x0F
#define SY6513_FWD_IBAT_OCP_TDB_MASK                   BIT(7)
#define SY6513_FWD_IBAT_OCP_TDB_SHIFT                  7
#define SY6513_VUSB_PULL_DOWN_MASK                     BIT(6)
#define SY6513_VUSB_PULL_DOWN_SHIFT                    6
#define SY6513_VWPC_PULL_DOWN_MASK                     BIT(5)
#define SY6513_VWPC_PULL_DOWN_SHIFT                    5
#define SY6513_VBUS_PULL_DOWN_MASK                     BIT(4)
#define SY6513_VBUS_PULL_DOWN_SHIFT                    4
#define SY6513_OVP_G_EN_REVERSE_MODE_MASK              BIT(3)
#define SY6513_OVP_G_EN_REVERSE_MODE_SHIFT             3
#define SY6513_WPC_G_EN_REVERSE_MODE_MASK              BIT(2)
#define SY6513_WPC_G_EN_REVERSE_MODE_SHIFT             2
#define SY6513_FWD_IBAT_OCP_TDB_100NS                  0
#define SY6513_VUSB_PULL_DOWN_DISABLE                  0
#define SY6513_VWPC_PULL_DOWN_DISABLE                  0
#define SY6513_VUSB_PULL_DOWN_DISABLE                  0
#define SY6513_OVP_G_EN_REVERSE_MODE_DISABLE           0
#define SY6513_WPC_G_EN_REVERSE_MODE_DISABLE           0

/* OVPFET_DRIVER_CURRENT_CONFIG reg=0x10 */
#define SY6513_OVPFET_DRIVER_CURRENT_CONFIG_REG        0x10
#define SY6513_OVPFET_DRIVER_CURRENT_MASK              (BIT(7) | BIT(6) | BIT(5))
#define SY6513_OVPFET_DRIVER_CURRENT_SHIFT             5
#define SY6513_OVPFET_DRIVER_CURRENT_MIN               500
#define SY6513_OVPFET_DRIVER_CURRENT_INIT              4000
#define SY6513_OVPFET_DRIVER_CURRENT_MAX               4000
#define SY6513_OVPFET_DRIVER_CURRENT_STEP              500

/* FLT_FLAG1 reg=0x11 */
#define SY6513_FLT_FLAG1_REG                           0x11
#define SY6513_VUSB_OVP_FLAG_MASK                      BIT(7)
#define SY6513_VUSB_OVP_FLAG_SHIFT                     7
#define SY6513_VUSB_PULLDOWN_FLAG_MASK                 BIT(6)
#define SY6513_VUSB_PULLDOWN_FLAG_SHIFT                6
#define SY6513_VWPC_OVP_FLAG_MASK                      BIT(5)
#define SY6513_VWPC_OVP_FLAG_SHIFT                     5
#define SY6513_VWPC_PULLDOWN_FLAG_MASK                 BIT(4)
#define SY6513_VWPC_PULLDOWN_FLAG_SHIFT                4
#define SY6513_VBUS_OVP_FLAG_MASK                      BIT(3)
#define SY6513_VBUS_OVP_FLAG_SHIFT                     3
#define SY6513_VBUS_PULLDOWN_FLAG_MASK                 BIT(2)
#define SY6513_VBUS_PULLDOWN_FLAG_SHIFT                2
#define SY6513_VUSB_UVLO_FLAG_MASK                     BIT(1)
#define SY6513_VUSB_UVLO_FLAG_SHIFT                    1
#define SY6513_VWPC_UVLO_FLAG_MASK                     BIT(0)
#define SY6513_VWPC_UVLO_FLAG_SHIFT                    0

/* FLT_INT_MASK1 reg=0x12 */
#define SY6513_FLT_INT_MASK1_REG                       0x12
#define SY6513_VUSB_OVP_MASK_MASK                      BIT(7)
#define SY6513_VUSB_OVP_MASK_SHIFT                     7
#define SY6513_VUSB_PULLDOWN_MASK_MASK                 BIT(6)
#define SY6513_VUSB_PULLDOWN_MASK_SHIFT                6
#define SY6513_VWPC_OVP_MASK_MASK                      BIT(5)
#define SY6513_VWPC_OVP_MASK_SHIFT                     5
#define SY6513_VWPC_PULLDOWN_MASK_MASK                 BIT(4)
#define SY6513_VWPC_PULLDOWN_MASK_SHIFT                4
#define SY6513_VBUS_OVP_MASK_MASK                      BIT(3)
#define SY6513_VBUS_OVP_MASK_SHIFT                     3
#define SY6513_VBUS_PULLDOWN_MASK_MASK                 BIT(2)
#define SY6513_VBUS_PULLDOWN_MASK_SHIFT                2
#define SY6513_VUSB_UVLO_MASK_MASK                     BIT(1)
#define SY6513_VUSB_UVLO_MASK_SHIFT                    1
#define SY6513_VWPC_UVLO_MASK_MASK                     BIT(0)
#define SY6513_VWPC_UVLO_MASK_SHIFT                    0
#define SY6513_VUSB_UVLO_MASK_EN                       1
#define SY6513_VWPC_UVLO_MASK_EN                       1

/* FLT_FLAG2 reg=0x13 */
#define SY6513_FLT_FLAG2_REG                           0x13
#define SY6513_OVP_FWD_OVP_VOUT_FLAG_MASK              BIT(7)
#define SY6513_OVP_FWD_OVP_VOUT_FLAG_SHIFT             7
#define SY6513_OVP_FWD_WPC_VOUT_FLAG_MASK              BIT(6)
#define SY6513_OVP_FWD_WPC_VOUT_FLAG_SHIFT             6
#define SY6513_SC_VDROP_MIN_FLAG_MASK                  BIT(5)
#define SY6513_SC_VDROP_MIN_FLAG_SHIFT                 5
#define SY6513_SC_VDROP_OVP_FLAG_MASK                  BIT(4)
#define SY6513_SC_VDROP_OVP_FLAG_SHIFT                 4
#define SY6513_41SC_IBUS_OCP_FLAG_MASK                 BIT(3)
#define SY6513_41SC_IBUS_OCP_FLAG_SHIFT                3
#define SY6513_21SC_OR_FBPS_IBUS_OCP_FLAG_MASK         BIT(2)
#define SY6513_21SC_OR_FBPS_IBUS_OCP_FLAG_SHIFT        2
#define SY6513_14SC_IBUS_OCP_FLAG_MASK                 BIT(1)
#define SY6513_14SC_IBUS_OCP_FLAG_SHIFT                1
#define SY6513_12SC_OR_RBPS_IBUS_OCP_FLAG_MASK         BIT(0)
#define SY6513_12SC_OR_RBPS_IBUS_OCP_FLAG_SHIFT        0

/* FLT_INT_MASK2 reg=0x14 */
#define SY6513_FLT_INT_MASK2_REG                       0x14
#define SY6513_OVP_FWD_OVP_VOUT_MASK_MASK              BIT(7)
#define SY6513_OVP_FWD_OVP_VOUT_MASK_SHIFT             7
#define SY6513_OVP_FWD_WPC_VOUT_MASK_MASK              BIT(6)
#define SY6513_OVP_FWD_WPC_VOUT_MASK_SHIFT             6
#define SY6513_SC_VDROP_MIN_MASK_MASK                  BIT(5)
#define SY6513_SC_VDROP_MIN_MASK_SHIFT                 5
#define SY6513_SC_VDROP_OVP_MASK_MASK                  BIT(4)
#define SY6513_SC_VDROP_OVP_MASK_SHIFT                 4
#define SY6513_41SC_IBUS_OCP_MASK_MASK                 BIT(3)
#define SY6513_41SC_IBUS_OCP_MASK_SHIFT                3
#define SY6513_21SC_AND_FBPS_IBUS_OCP_MASK_MASK        BIT(2)
#define SY6513_21SC_AND_FBPS_IBUS_OCP_MASK_SHIFT       2
#define SY6513_14SC_IBUS_OCP_MASK_MASK                 BIT(1)
#define SY6513_14SC_IBUS_OCP_MASK_SHIFT                1
#define SY6513_12SC_AND_RBPS_IBUS_OCP_MASK_MASK        BIT(0)
#define SY6513_12SC_AND_RBPS_IBUS_OCP_MASK_SHIFT       0

/* FLT_FLAG3 reg=0x15 */
#define SY6513_FLT_FLAG3_REG                           0x15
#define SY6513_VBAT1_OVP_FLAG_MASK                     BIT(7)
#define SY6513_VBAT1_OVP_FLAG_SHIFT                    7
#define SY6513_VBAT2_OVP_FLAG_MASK                     BIT(6)
#define SY6513_VBAT2_OVP_FLAG_SHIFT                    6
#define SY6513_IBAT_OCP_FLAG_MASK                      BIT(5)
#define SY6513_IBAT_OCP_FLAG_SHIFT                     5
#define SY6513_OVP_MOS_SOFTSTART_TIMEROUT_MASK         BIT(4)
#define SY6513_OVP_MOS_SOFTSTART_TIMEROUT_SHIFT        4
#define SY6513_TDIE_OTP_FLAG_MASK                      BIT(1)
#define SY6513_TDIE_OTP_FLAG_SHIFT                     1
#define SY6513_SC_VOUT_OVP_FLAG_MASK                   BIT(0)
#define SY6513_SC_VOUT_OVP_FLAG_SHIFT                  0

/* FLT_INT_MASK3 reg=0x16 */
#define SY6513_FLT_INT_MASK3_REG                       0x16
#define SY6513_VBAT1_OVP_MASK_MASK                     BIT(7)
#define SY6513_VBAT1_OVP_MASK_SHIFT                    7
#define SY6513_VBAT2_OVP_MASK_MASK                     BIT(6)
#define SY6513_VBAT2_OVP_MASK_SHIFT                    6
#define SY6513_IBAT_OCP_MASK_MASK                      BIT(5)
#define SY6513_IBAT_OCP_MASK_SHIFT                     5
#define SY6513_OVP_MOS_SOFTSTART_TIMEROUT_MASK         BIT(4)
#define SY6513_OVP_MOS_SOFTSTART_TIMEROUT_SHIFT        4
#define SY6513_TDIE_OTP_MASK_MASK                      BIT(1)
#define SY6513_TDIE_OTP_MASK_SHIFT                     1
#define SY6513_SC_VOUT_OVP_MASK_MASK                   BIT(0)
#define SY6513_SC_VOUT_OVP_MASK_SHIFT                  0

/* FLT_FLAG4 reg=0x17 */
#define SY6513_FLT_FLAG4_REG                           0x17
#define SY6513_IBUS_OCP_FLAG_MASK                      BIT(7)
#define SY6513_IBUS_OCP_FLAG_SHIFT                     7
#define SY6513_SC_CVT_OCP_FLAG_MASK                    BIT(6)
#define SY6513_SC_CVT_OCP_FLAG_SHIFT                   6
#define SY6513_IBUS_UCP_FLAG_MASK                      BIT(5)
#define SY6513_IBUS_UCP_FLAG_SHIFT                     5
#define SY6513_IBUS_RCP_FLAG_MASK                      BIT(4)
#define SY6513_IBUS_RCP_FLAG_SHIFT                     4
#define SY6513_41FWD_VBUS_UVP_FLAG_MASK                BIT(3)
#define SY6513_41FWD_VBUS_UVP_FLAG_SHIFT               3
#define SY6513_21FWD_VBUS_UVP_FLAG_MASK                BIT(2)
#define SY6513_21FWD_VBUS_UVP_FLAG_SHIFT               2
#define SY6513_VUSB_INSERT_FLAG_MASK                   BIT(1)
#define SY6513_VUSB_INSERT_FLAG_SHIFT                  1
#define SY6513_VWPC_INSERT_FLAG_MASK                   BIT(0)
#define SY6513_VWPC_INSERT_FLAG_SHIFT                  0

/* FLT_INT_MASK4 reg=0x18 */
#define SY6513_FLT_INT_MASK4_REG                       0x18
#define SY6513_IBUS_OCP_MASK_MASK                      BIT(7)
#define SY6513_IBUS_OCP_MASK_SHIFT                     7
#define SY6513_SC_CVT_OCP_MASK_MASK                    BIT(6)
#define SY6513_SC_CVT_OCP_MASK_SHIFT                   6
#define SY6513_IBUS_UCP_MASK_MASK                      BIT(5)
#define SY6513_IBUS_UCP_MASK_SHIFT                     5
#define SY6513_IBUS_RCP_MASK_MASK                      BIT(4)
#define SY6513_IBUS_RCP_MASK_SHIFT                     4
#define SY6513_41FWD_VBUS_UVP_MASK_MASK                BIT(3)
#define SY6513_41FWD_VBUS_UVP_MASK_SHIFT               3
#define SY6513_21FWD_VBUS_UVP_MASK_MASK                BIT(2)
#define SY6513_21FWD_VBUS_UVP_MASK_SHIFT               2
#define SY6513_VUSB_INSERT_MASK_MASK                   BIT(1)
#define SY6513_VUSB_INSERT_MASK_SHIFT                  1
#define SY6513_VWPC_INSERT_MASK_MASK                   BIT(0)
#define SY6513_VWPC_INSERT_MASK_SHIFT                  0

/* FLT_FLAG5 reg=0x19 */
#define SY6513_FLT_FLAG5_REG                           0x19
#define SY6513_VBUS_INSERT_FLAG_MASK                   BIT(7)
#define SY6513_VBUS_INSERT_FLAG_SHIFT                  7
#define SY6513_VBAT_INSERT_FLAG_MASK                   BIT(6)
#define SY6513_VBAT_INSERT_FLAG_SHIFT                  6
#define SY6513_WATCHDOG_TIMEOUT_FLAG_MASK              BIT(5)
#define SY6513_WATCHDOG_TIMEOUT_FLAG_SHIFT             5
#define SY6513_VBUS_UVLO_FLAG_MASK                     BIT(4)
#define SY6513_VBUS_UVLO_FLAG_SHIFT                    4
#define SY6513_IBUS_UCP_TIMEOUT_FLAG_MASK              BIT(3)
#define SY6513_IBUS_UCP_TIMEOUT_FLAG_SHIFT             3
#define SY6513_ADC_DONE_FLAG_MASK                      BIT(2)
#define SY6513_ADC_DONE_FLAG_SHIFT                     2
#define SY6513_FLYING_CAPACITOR_SHORT_OPEN_FLAG_MASK   BIT(1)
#define SY6513_FLYING_CAPACITOR_SHORT_OPEN_FLAG_SHIFT  1
#define SY6513_REVERSE_MODE_VOUT_UVLO_FLAG_MASK        BIT(0)
#define SY6513_REVERSE_MODE_VOUT_UVLO_FLAG_SHIFT       0

/* FLT_INT_MASK4 reg=0x1A */
#define SY6513_FLT_INT_MASK5_REG                       0x1A
#define SY6513_VBUS_INSERT_MASK_MASK                   BIT(7)
#define SY6513_VBUS_INSERT_MASK_SHIFT                  7
#define SY6513_VBAT_INSERT_MASK_MASK                   BIT(6)
#define SY6513_VBAT_INSERT_MASK_SHIFT                  6
#define SY6513_WATCHDOG_TIMEOUT_MASK_MASK              BIT(5)
#define SY6513_WATCHDOG_TIMEOUT_MASK_SHIFT             5
#define SY6513_VBUS_UVLO_MASK_MASK                     BIT(4)
#define SY6513_VBUS_UVLO_MASK_SHIFT                    4
#define SY6513_IBUS_UCP_TIMEOUT_MASK_MASK              BIT(3)
#define SY6513_IBUS_UCP_TIMEOUT_MASK_SHIFT             3
#define SY6513_ADC_DONE_MASK_MASK                      BIT(2)
#define SY6513_ADC_DONE_MASK_SHIFT                     2
#define SY6513_FLYING_CAPACITOR_SHORT_OPEN_MASK_MASK   BIT(1)
#define SY6513_FLYING_CAPACITOR_SHORT_OPEN_MASK_SHIFT  1
#define SY6513_REVERSE_MODE_VOUT_UVLO_MASK_MASK        BIT(0)
#define SY6513_REVERSE_MODE_VOUT_UVLO_MASK_SHIFT       0

/* FLT_FLAG6 reg=0x1B */
#define SY6513_FLT_FLAG6_REG                           0x1B
#define SY6513_VUSB_OVP_ALM_FLAG_MASK                  BIT(7)
#define SY6513_VUSB_OVP_ALM_FLAG_SHIFT                 7
#define SY6513_VWPC_OVP_ALM_FLAG_MASK                  BIT(6)
#define SY6513_VWPC_OVP_ALM_FLAG_SHIFT                 6
#define SY6513_TDIE_OTP_ALM_FLAG_MASK                  BIT(5)
#define SY6513_TDIE_OTP_ALM_FLAG_SHIFT                 5

/* FLT_INT_MASK6 reg=0x1C */
#define SY6513_FLT_INT_MASK6_REG                       0x1C
#define SY6513_VUSB_OVP_ALM_MASK_MASK                  BIT(7)
#define SY6513_VUSB_OVP_ALM_MASK_SHIFT                 7
#define SY6513_VWPC_OVP_ALM_MASK_MASK                  BIT(6)
#define SY6513_VWPC_OVP_ALM_MASK_SHIFT                 6
#define SY6513_TDIE_OTP_ALM_MASK_MASK                  BIT(5)
#define SY6513_TDIE_OTP_ALM_MASK_SHIFT                 5

/* ADC_CTRL reg=0x20 */
#define SY6513_ADC_CTRL_REG                            0x20
#define SY6513_ADC_ENABLE_MASK                         BIT(7)
#define SY6513_ADC_ENABLE_SHIFT                        7
#define SY6513_ADC_RATE_MASK                           BIT(6)
#define SY6513_ADC_RATE_SHIFT                          6
#define SY6513_VBUS_ADC_DIS_MASK                       BIT(5)
#define SY6513_VBUS_ADC_DIS_SHIFT                      5
#define SY6513_VUSB_ADC_DIS_MASK                       BIT(4)
#define SY6513_VUSB_ADC_DIS_SHIFT                      4
#define SY6513_VWPC_ADC_DIS_MASK                       BIT(3)
#define SY6513_VWPC_ADC_DIS_SHIFT                      3
#define SY6513_IBUS_ADC_DIS_MASK                       BIT(2)
#define SY6513_IBUS_ADC_DIS_SHIFT                      2
#define SY6513_ADC_ENABLE                              1
#define SY6513_ADC_DISABLE                             0

/* ADC_CTRL reg=0x21 */
#define SY6513_ADC_CTRL1_REG                           0x21
#define SY6513_VBAT1_ADC_DIS_MASK                      BIT(7)
#define SY6513_VBAT1_ADC_DIS_SHIFT                     7
#define SY6513_VBAT2_ADC_DIS_MASK                      BIT(6)
#define SY6513_VBAT2_ADC_DIS_SHIFT                     6
#define SY6513_VOUT_ADC_DIS_MASK                       BIT(5)
#define SY6513_VOUT_ADC_DIS_SHIFT                      5
#define SY6513_IBAT_ADC_DIS_MASK                       BIT(4)
#define SY6513_IBAT_ADC_DIS_SHIFT                      4
#define SY6513_TDIE_ADC_DIS_MASK                       BIT(3)
#define SY6513_TDIE_ADC_DIS_SHIFT                      3
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_MASK  (BIT(1) | BIT(0))
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_SHIFT 0
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_0     0
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_2     1
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_4     2
#define SY6513_AVERAGE_TIMES_FOR_SAMPLING_OF_ADC_8     3

/* VBUS_ADC1 reg=0x22 */
#define SY6513_VBUS_ADC1_REG                           0x22
#define SY6513_VBUS_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_VBUS_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_VBUS_ADC1_VBUS_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VBUS_ADC1_VBUS_ADC_SHIFT                0

/* VBUS_ADC0 reg=0x23 */
#define SY6513_VBUS_ADC0_REG                           0x23
#define SY6513_VBUS_ADC0_VBUS_ADC_MASK                 0xFF
#define SY6513_VBUS_ADC0_VBUS_ADC_SHIFT                0

/* VUSB_ADC1 reg=0x24 */
#define SY6513_VUSB_ADC1_REG                           0x24
#define SY6513_VUSB_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_VUSB_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_VUSB_ADC1_VUSB_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VUSB_ADC1_VUSB_ADC_SHIFT                0

/* VUSB_ADC0 reg=0x25 */
#define SY6513_VUSB_ADC0_REG                           0x25
#define SY6513_VUSB_ADC0_VUSB_ADC_MASK                 0xFF
#define SY6513_VUSB_ADC0_VUSB_ADC_SHIFT                0

/* VWPC_ADC1 reg=0x26 */
#define SY6513_VWPC_ADC1_REG                           0x26
#define SY6513_VWPC_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_VWPC_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_VWPC_ADC1_VWPC_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VWPC_ADC1_VWPC_ADC_SHIFT                0

/* VWPC_ADC0 reg=0x27 */
#define SY6513_VWPC_ADC0_REG                           0x27
#define SY6513_VWPC_ADC0_VWPC_ADC_MASK                 0xFF
#define SY6513_VWPC_ADC0_VWPC_ADC_SHIFT                0

/* IBUS_ADC1 reg=0x28 */
#define SY6513_IBUS_ADC1_REG                           0x28
#define SY6513_IBUS_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_IBUS_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_IBUS_ADC1_IBUS_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_IBUS_ADC1_IBUS_ADC_SHIFT                0

/* IBUS_ADC0 reg=0x29 */
#define SY6513_IBUS_ADC0_REG                           0x29
#define SY6513_IBUS_ADC0_IBUS_ADC_MASK                 0xFF
#define SY6513_IBUS_ADC0_IBUS_ADC_SHIFT                0

/* VBAT1_ADC1 reg=0x2E */
#define SY6513_VBAT1_ADC1_REG                          0x2E
#define SY6513_VBAT1_ADC_SIGN_BIT_MASK                 BIT(7)
#define SY6513_VBAT1_ADC_SIGN_BIT_SHIFT                7
#define SY6513_VBAT1_ADC1_VBAT1_ADC_MASK               (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VBAT1_ADC1_VBAT1_ADC_SHIFT              0

/* VBAT1_ADC0 reg=0x2F */
#define SY6513_VBAT1_ADC0_REG                          0x2F
#define SY6513_VBAT1_ADC0_VBAT1_ADC_MASK               0xFF
#define SY6513_VBAT1_ADC0_VBAT1_ADC_SHIFT              0

/* VBAT2_ADC1 reg=0x30 */
#define SY6513_VBAT2_ADC1_REG                          0x30
#define SY6513_VBAT2_ADC_SIGN_BIT_MASK                 BIT(7)
#define SY6513_VBAT2_ADC_SIGN_BIT_SHIFT                7
#define SY6513_VBAT2_ADC1_VBAT2_ADC_MASK               (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VBAT2_ADC1_VBAT2_ADC_SHIFT              0

/* VBAT2_ADC0 reg=0x31 */
#define SY6513_VBAT2_ADC0_REG                          0x31
#define SY6513_VBAT2_ADC0_VBAT2_ADC_MASK               0xFF
#define SY6513_VBAT2_ADC0_VBAT2_ADC_SHIFT              0

/* VOUT_ADC1 reg=0x32 */
#define SY6513_VOUT_ADC1_REG                           0x32
#define SY6513_VOUT_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_VOUT_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_VOUT_ADC1_VOUT_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_VOUT_ADC1_VOUT_ADC_SHIFT                0

/* VOUT_ADC0 reg=0x33 */
#define SY6513_VOUT_ADC0_REG                           0x33
#define SY6513_VOUT_ADC0_VOUT_ADC_MASK                 0xFF
#define SY6513_VOUT_ADC0_VOUT_ADC_SHIFT                0

/* IBAT_ADC1 reg=0x34 */
#define SY6513_IBAT_ADC1_REG                           0x34
#define SY6513_IBAT_ADC_SIGN_BIT_MASK                  BIT(7)
#define SY6513_IBAT_ADC_SIGN_BIT_SHIFT                 7
#define SY6513_IBAT_ADC1_IBAT_ADC_MASK                 (BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_IBAT_ADC1_IBAT_ADC_SHIFT                0

/* IBAT_ADC0 reg=0x35 */
#define SY6513_IBAT_ADC0_REG                           0x35
#define SY6513_IBAT_ADC0_IBAT_ADC_MASK                 0xFF
#define SY6513_IBAT_ADC0_IBAT_ADC_SHIFT                0

/* TDIE_ADC reg=0x36 */
#define SY6513_TDIE_REG                                0x36
#define SY6513_TDIE_ADC_MASK                           0xFF
#define SY6513_TDIE_ADC_SHIFT                          0

/* PROTECT_CONTROL1 reg=0x37 */
#define SY6513_PROTECT_CONTROL1_REG                    0x37
#define SY6513_VUSB_UVLO_ENABLE_MASK                   BIT(7)
#define SY6513_VUSB_UVLO_ENABLE_SHIFT                  7
#define SY6513_VUSB_NVOUT_VDROP_OVP_ENABLE_MASK        BIT(6)
#define SY6513_VUSB_NVOUT_VDROP_OVP_ENABLE_SHIFT       6
#define SY6513_VUSB_OVP_ENABLE_MASK                    BIT(5)
#define SY6513_VUSB_OVP_ENABLE_SHIFT                   5
#define SY6513_VWPC_UVLO_ENABLE_MASK                   BIT(4)
#define SY6513_VWPC_UVLO_ENABLE_SHIFT                  4
#define SY6513_VWPC_NVOUT_VDROP_OVP_ENABLE_MASK        BIT(3)
#define SY6513_VWPC_NVOUT_VDROP_OVP_ENABLE_SHIFT       3
#define SY6513_VWPC_OVP_ENABLE_MASK                    BIT(2)
#define SY6513_VWPC_OVP_ENABLE_SHIFT                   2
#define SY6513_VBUS_OVP_ENABLE_MASK                    BIT(1)
#define SY6513_VBUS_OVP_ENABLE_SHIFT                   1
#define SY6513_VBUS_OVP_PEAK_ENABLE_MASK               BIT(0)
#define SY6513_VBUS_OVP_PEAK_ENABLE_SHIFT              0

/* PROTECT_CONTROL2 reg=0x38 */
#define SY6513_PROTECT_CONTROL2_REG                    0x38
#define SY6513_VOUT_OVP_ENABLE_MASK                    BIT(7)
#define SY6513_VOUT_OVP_ENABLE_SHIFT                   7
#define SY6513_IBUS_OCP_PEAK_ENABLE_MASK               BIT(6)
#define SY6513_IBUS_OCP_PEAK_ENABLE_SHIFT              6
#define SY6513_CONVERTER_OCP_PEAK_ENABLE_MASK          BIT(5)
#define SY6513_CONVERTER_OCP_PEAK_ENABLE_SHIFT         5
#define SY6513_VBUS_NVOUT_VDROP_MIN_ENABLE_MASK        BIT(4)
#define SY6513_VBUS_NVOUT_VDROP_MIN_ENABLE_SHIFT       4
#define SY6513_VBUS_NVOUT_VDROP_OVP_ENABLE_MASK        BIT(3)
#define SY6513_VBUS_NVOUT_VDROP_OVP_ENABLE_SHIFT       3
#define SY6513_IBUS_OCP_ENABLE_MASK                    BIT(2)
#define SY6513_IBUS_OCP_ENABLE_SHIFT                   2
#define SY6513_IBUS_UCP_ENABLE_MASK                    BIT(1)
#define SY6513_IBUS_UCP_ENABLE_SHIFT                   1
#define SY6513_IBUS_RCP_PEAK_ENABLE_MASK               BIT(0)
#define SY6513_IBUS_RCP_PEAK_ENABLE_SHIFT              0
#define SY6513_IBUS_RCP_PEAK_ENABLE                    0
#define SY6513_IBUS_RCP_PEAK_DISABLE                   1
#define SY6513_IBUS_UCP_ENABLE                         0
#define SY6513_IBUS_UCP_DISABLE                        1

/* PROTECT_CONTROL3 reg=0x39 */
#define SY6513_PROTECT_CONTROL3_REG                    0x39
#define SY6513_VBUS_UVLO_ENABLE_MASK                   BIT(7)
#define SY6513_VBUS_UVLO_ENABLE_SHIFT                  7
#define SY6513_VBUS_UVP_ENABLE_MASK                    BIT(6)
#define SY6513_VBUS_UVP_ENABLE_SHIFT                   6
#define SY6513_VBAT1_OVP_ENABLE_MASK                   BIT(5)
#define SY6513_VBAT1_OVP_ENABLE_SHIFT                  5
#define SY6513_VBAT2_OVP_ENABLE_MASK                   BIT(4)
#define SY6513_VBAT2_OVP_ENABLE_SHIFT                  4
#define SY6513_IBAT_OCP_ENABLE_MASK                    BIT(3)
#define SY6513_IBAT_OCP_ENABLE_SHIFT                   3
#define SY6513_VUSB_OVP_ALM_ENABLE_MASK                BIT(2)
#define SY6513_VUSB_OVP_ALM_ENABLE_SHIFT               2
#define SY6513_VWPC_OVP_ALM_ENABLE_MASK                BIT(1)
#define SY6513_VWPC_OVP_ALM_ENABLE_SHIFT               1
#define SY6513_TDIE_OTP_ALM_ENABLE_MASK                BIT(0)
#define SY6513_TDIE_OTP_ALM_ENABLE_SHIFT               0

/* PROTECT_CONTROL4 reg=0x3A */
#define SY6513_PROTECT_CONTROL4_REG                    0x3A
#define SY6513_TDIE_OTP_ENABLE_MASK                    BIT(7)
#define SY6513_TDIE_OTP_ENABLE_SHIFT                   7
#define SY6513_REVERSE_VOUT_UVLO_ENABLE_MASK           BIT(6)
#define SY6513_REVERSE_VOUT_UVLO_ENABLE_SHIFT          6

/* device_id reg=0xa0 */
#define SY6513_DEVICE_ID_REG                           0xA0

struct sy6513_device_info {
	struct i2c_client *client;
	struct work_struct irq_work;
	struct device *dev;
	struct workqueue_struct *int_wq;
	struct mutex scp_detect_lock;
	struct mutex accp_adapter_reg_lock;
	struct nty_data nty_data;
	struct dc_ic_ops lvc_ops;
	struct dc_ic_ops sc_ops;
	struct dc_ic_ops sc4_ops;
	struct dc_batinfo_ops batinfo_ops;
	struct power_log_ops log_ops;
	struct power_sw_attr wpcsw_attr;
	struct power_sw_attr ovpsw_attr;
	char name[CHIP_DEV_NAME_LEN];
	int gpio_int;
	int gpio_enable;
	int gpio_reset;
	int gpio_lpm;
	int irq_int;
	int irq_active;
	int chip_already_init;
	int device_id;
	u32 ic_role;
	int get_id_time;
	int init_finish_flag;
	int int_notify_enable_flag;
	int switching_frequency;
	int sense_r_actual;
	int sense_r_config;
	int dts_scp_support;
	int dts_fcp_support;
	u32 sc_ibus_ocp;
	u32 sc4_ibus_ocp;
	u32 sc_ibat_ocp;
	u32 sc4_ibat_ocp;
	u32 watchdog_time;
	u32 rvs_cp_support;
	u32 parallel_mode;
	u32 slave_mode;
	u8 charge_mode;
	u32 scp_error_flag;
	u8 scp_data[SY6513_SCP_MAX_DATA_LEN];
	u8 scp_op;
	u8 scp_op_num;
	bool fcp_support;
	bool scp_trans_done;
	bool i2c_is_working;
};

struct sy6513_dump_value {
	int vbat;
	int ibus;
	int vbus;
	int ibat;
	int vusb;
	int vout;
	int temp;
};

enum irq_flag {
	SY6513_IRQ_FLT_FLAG1,
	SY6513_IRQ_FLT_FLAG2,
	SY6513_IRQ_FLT_FLAG3,
	SY6513_IRQ_FLT_FLAG4,
	SY6513_IRQ_FLT_FLAG5,
	SY6513_IRQ_FLT_FLAG6,
	SY6513_IRQ_END,
};

struct sy6513_thre_configs {
	int mode;
	int reg;
	int mask;
	int shift;
	int value;
};

static const int usb_ovp_vals[] = {
	SY6513_VUSB_OVP_7P5V,
	SY6513_VUSB_OVP_13V,
	SY6513_VUSB_OVP_23V,
	SY6513_VUSB_OVP_7P5V,
	SY6513_VUSB_OVP_13V,
	SY6513_VUSB_OVP_23V
};

static const int wpc_ovp_vals[] = {
	SY6513_VWPC_OVP_7P5V,
	SY6513_VWPC_OVP_13V,
	SY6513_VWPC_OVP_23V,
	SY6513_VWPC_OVP_7P5V,
	SY6513_VWPC_OVP_13V,
	SY6513_VWPC_OVP_23V
};

int sy6513_config_vusb_ovp_th_mv(struct sy6513_device_info *di, int ovp_th);
int sy6513_config_vbus_ovp_th_mv(struct sy6513_device_info *di, int ovp_th, int mode);
int sy6513_get_vbus_mv(int *vbus, void *dev_data);

#endif /* _SY6513_H_ */
