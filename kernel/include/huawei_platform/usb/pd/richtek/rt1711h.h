/*
 * Copyright (C) 2016 Richtek Technology Corp.
 *
 * Author: TH <tsunghan_tsai@richtek.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_RT1711H_H
#define __LINUX_RT1711H_H

#include <linux/bitops.h>
#include <huawei_platform/usb/pd/richtek/std_tcpci_v10.h>
#include <huawei_platform/usb/pd/richtek/pd_dbg_info.h>

#define RTUSB_VIN_3V3 3300000
#define RT_PMIC_LDO_3 2

#define RT_CC_STATUS_FOR_DOUBLE_56K 5
#define RT_CC_STATUS_FOR_DOUBLE_22k 10
#define RT_CC_STATUS_FOR_DOUBLE_10k 15

#define CC_STATUS_MASK (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define RT1711_CC_STATUS 0x1d

/*show debug message or not */
#define ENABLE_RT1711_DBG	0

/* RT1711H Private RegMap */

#define RT1711H_REG_PHY_CTRL1				(0x80)
#define RT1711H_REG_CLK_CTRL2				(0x87)
#define RT1711H_REG_CLK_CTRL3				(0x88)

#define RT1711H_REG_BMC_CTRL				(0x90)
#define RT1711H_REG_BMCIO_VCONOCP			(0x92)
#define RT1711H_REG_BMCIO_RXDZSEL			(0x93)
#define RT1711H_REG_VCONN_CLIMITEN			(0x95)

#define RT1711H_REG_RT_STATUS				(0x97)
#define RT1711H_REG_RT_INT					(0x98)
#define RT1711H_REG_RT_MASK					(0x99)

#define RT1711H_REG_IDLE_CTRL				(0x9B)
#define RT1711H_REG_INTRST_CTRL				(0x9C)
#define RT1711H_REG_WATCHDOG_CTRL			(0x9D)
#define RT1711H_REG_I2CRST_CTRL				(0X9E)

#define RT1711H_REG_SWRESET				(0xA0)
#define RT1711H_REG_TTCPC_FILTER			(0xA1)
#define RT1711H_REG_DRP_TOGGLE_CYCLE		(0xA2)
#define RT1711H_REG_DRP_DUTY_CTRL			(0xA3)
#define RT1711H_REG_BMCIO_RXDZEN			(0xAF)

#define RT1711H_REG_CMD_REG1				(0xF0)
#define RT1711H_REG_CMD_REG2				(0xF1)
#define RT1711H_REG_ADJ_VSWING				(0xF3)

/* richtek serial */
#define RICHTEK_1711_VID	0x29cf
#define RICHTEK_1711_PID	0x1711

/* etek serial */
#define ETEK_ET7303_VID		0x6dcf
#define ETEK_ET7303_PID		0x1711

/* husb serial */
#define HUSB_311_VID		0x2e99
#define HUSB_311_PID		0x0311

#define VID_MASK	0x0ffff
#define DID_MASK	0x0ffff
#define SHIFT_16	16

/*
 * Device ID
 */

#define RT1711H_DID_A		0x2170
#define RT1711H_DID_B		0x2171
#define RT1711H_DID_C		0x2172
#define RT1711H_REG_PHY_CTRL1_SET( \
	retry_discard, toggle_cnt, bus_idle_cnt, rx_filter) \
	((retry_discard << 7) | (toggle_cnt << 4) | \
	(bus_idle_cnt << 2) | (rx_filter & 0x03))

/*
 * RT1711H_REG_CLK_CTRL2			(0x87)
 */

#define RT1711H_REG_CLK_DIV_600K_EN		(1<<7)
#define RT1711H_REG_CLK_BCLK2_EN		(1<<6)
#define RT1711H_REG_CLK_BCLK2_TG_EN		(1<<5)
#define RT1711H_REG_CLK_DIV_300K_EN		(1<<3)
#define RT1711H_REG_CLK_CK_300K_EN		(1<<2)
#define RT1711H_REG_CLK_BCLK_EN			(1<<1)
#define RT1711H_REG_CLK_BCLK_TH_EN		(1<<0)

/*
 * RT1711H_REG_CLK_CTRL3			(0x88)
 */

#define RT1711H_REG_CLK_OSCMUX_RG_EN	(1<<7)
#define RT1711H_REG_CLK_CK_24M_EN		(1<<6)
#define RT1711H_REG_CLK_OSC_RG_EN		(1<<5)
#define RT1711H_REG_CLK_DIV_2P4M_EN		(1<<4)
#define RT1711H_REG_CLK_CK_2P4M_EN		(1<<3)
#define RT1711H_REG_CLK_PCLK_EN			(1<<2)
#define RT1711H_REG_CLK_PCLK_RG_EN		(1<<1)
#define RT1711H_REG_CLK_PCLK_TG_EN		(1<<0)

/*
 * RT1711H_REG_BMC_CTRL				(0x90)
 */

#define RT1711H_REG_IDLE_EN				(1<<6)
#define RT1711H_REG_DISCHARGE_EN			(1<<5)
#define RT1711H_REG_BMCIO_LPRPRD			(1<<4)
#define RT1711H_REG_BMCIO_LPEN				(1<<3)
#define RT1711H_REG_BMCIO_BG_EN				(1<<2)
#define RT1711H_REG_VBUS_DET_EN				(1<<1)
#define RT1711H_REG_BMCIO_OSC_EN			(1<<0)

/*
 * RT1711H_REG_BMCIO_VCONOCP                         (0x92)
 */

#define RT1711H_REG_BMCIO_SOFTSTART_TIME_MASK		((1<<5) | (1<<4) | (1<<3))
#define RT1711H_REG_BMCIO_SOFTSTART_TIME		((1<<5) | (1<<4))
#define RT1711H_REG_ADJ_VBUS_MEASURE		(1<<6)

/*
 * RT1711H_REG_BMCIO_RXDZSEL                         (0x93)
 */

#define RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_800MA_DFT	((1<<7) | (1<<6) | (1<<0))
#define RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_800MA	((1<<7) | (1<<6))
#define RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_600MA_DFT   ((1<<7) | (1<<0))
#define RT1711H_REG_BMCIO_OCP_CURRENT_LEVEL_600MA       (1<<7)

/*
 * RT1711H_REG_RT_STATUS				(0x97)
 */

#define RT1711H_REG_RA_DETACH				(1<<5)
#define RT1711H_REG_VBUS_80				(1<<1)

/*
 * RT1711H_REG_RT_INT				(0x98)
 */

#define RT1711H_REG_INT_RA_DETACH			(1<<5)
#define RT1711H_REG_INT_WATCHDOG			(1<<2)
#define RT1711H_REG_INT_VBUS_80				(1<<1)
#define RT1711H_REG_INT_WAKEUP				(1<<0)

/*
 * RT1711H_REG_RT_MASK				(0x99)
 */

#define RT1711H_REG_M_RA_DETACH				(1<<5)
#define RT1711H_REG_M_WATCHDOG				(1<<2)
#define RT1711H_REG_M_VBUS_80				(1<<1)
#define RT1711H_REG_M_WAKEUP				(1<<0)

/*
 * RT1711H_REG_IDLE_CTRL				(0x9B)
 */

#define RT1711H_REG_CK_300K_SEL				(1<<7)
#define RT1711H_REG_SHIPPING_OFF			(1<<5)
#define RT1711H_REG_AUTOIDLE_EN				(1<<3)

/* timeout = (tout*2+1) * 6.4ms */
#define RT1711H_REG_IDLE_SET(ck300, ship_dis, auto_idle, tout) \
	((ck300 << 7) | (ship_dis << 5) | (auto_idle << 3) | (tout & 0x07))

/*
 * RT1711H_REG_INTRST_CTRL			(0x9C)
 */

#define RT1711H_REG_INTRST_EN				(1<<7)

/* timeout = (tout+1) * 0.2sec */
#define RT1711H_REG_INTRST_SET(en, tout) \
	((en << 7) | (tout & 0x03))

/*
 * RT1711H_REG_WATCHDOG_CTRL		(0x9D)
 */

#define RT1711H_REG_WATCHDOG_EN				(1<<7)

/* timeout = (tout+1) * 0.4sec */
#define RT1711H_REG_WATCHDOG_CTRL_SET(en, tout)	\
	((en << 7) | (tout & 0x07))

/*
 * RT1711H_REG_I2CRST_CTRL		(0x9E)
 */

#define RT1711H_REG_I2CRST_EN				(1<<7)

/* timeout = (tout+1) * 12.5ms */
#define RT1711H_REG_I2CRST_SET(en, tout)	\
	((en << 7) | (tout & 0x0f))

/*
 * RT1711H_REG_CMD_REG1		(0xF0)
 */

#define RT1711H_REG_CMD_REG1_ENABLE ((1<<7) | (1<<6) | (1<<1))
#define RT1711H_REG_CMD_REG1_CLEAN (0x00)

/*
 * RT1711H_REG_CMD_REG2		(0xF1)
 */

#define RT1711H_REG_CMD_REG2_ENABLE ((1<<5) | (1<<2) | (1<<1))
#define RT1711H_REG_CMD_REG2_CLEAN	(0x00)

/*
 * RT1711H_REG_ADJ_VSWING	(0xF3)
 */

#define RT1711H_REG_SETTING_VSWING_MASK	((1<<2) | (1<<1) | (1<<0))
#define RT1711H_REG_SETTING_VSWING ((1<<2) | (1<<0))

#if ENABLE_RT1711_DBG
#define RT1711H_INFO(format, args...) \
	pd_dbg_info("%s() line-%d: " format,\
	__func__, __LINE__, ##args)
#else
#define RT1711_INFO(format, args...) \
	pr_info("%s:" format, __func__, ##args)
#endif

extern void ls_i2c_mutex_lock(void);
extern void ls_i2c_mutex_unlock(void);

#endif /* #ifndef __LINUX_RT1711H_H */
