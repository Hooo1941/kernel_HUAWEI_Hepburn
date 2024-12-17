/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: pmic6421v900 coulometer driver headfile
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

#ifndef _SCHARGER_V700_COUL_H_
#define _SCHARGER_V700_COUL_H_

#include <soc_schargerV700_interface.h>
#include <pmic_interface.h>

#ifndef BIT
#define BIT(x)                          (1 << (x))
#endif

/* vol offset a/b value */
#define VOL_OFFSET_A_STEP               39
#define VOL_OFFSET_B_STEP               78125

#define VOL_OFFSET_A_BASE               1000000
#define VOL_OFFSET_B_BASE               (0)

#define VOL_OFFSET_B_VALID_MASK         0xFE
#define VOL_OFFSET_A_HIGH_VALID_MASK    0x1FE
#define VOL_OFFSET_A_LOW_VALID_MASK     0x001
#define VOL_OFFSET_A_VALID_MASK         0x1FF

#define ECO_DELAY_500MS                 0
#define ECO_DELAY_2S                    1
#define ECO_DELAY_5S                    2
#define ECO_DELAY_10S                   3
#define ECO_DELAY_ENTER_EN                    BIT(1)
#define ECO_DELAY_OUT_EN                    BIT(0)
#define ECO_ENTER                       BIT(2)

#define TEMP_EN                         BIT(0)
#define TEMP_RDY                        BIT(1)
#define VOUT_RDY                        BIT(2)

#define ECO_FIFO_DEPTH                  4

/* coul reserverd regs use */
#define COUL_HARDWARE_BATTERY_MOVE_ADDR        PMIC_HRST_REG0_ADDR(0)

#define BATTERY_MOVE_MAGIC_NUM          0xc3
#define BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM 0x18

#define COUL_HARDWARE_OCV_CHOOSE               PMIC_HRST_REG1_ADDR(0) /* use bit 5 */
#define COUL_HARDWARE_TEMP_PROTECT        PMIC_HRST_REG1_ADDR(0) /* use bit 4 */
#define COUL_HARDWARE_DELTA_RC_SCENE           PMIC_HRST_REG1_ADDR(0) /* use bit 3 */
#define COUL_HARDWARE_PD_BY_OCV_WRONG          PMIC_HRST_REG1_ADDR(0) /* use bit 2 */
#define COUL_HARDWARE_NV_READ_SUCCESS          PMIC_HRST_REG1_ADDR(0) /* use bit 1 */
#define COUL_HARDWARE_NV_SAVE_SUCCESS          PMIC_HRST_REG1_ADDR(0) /* use bit 0 */

#define USE_SAVED_OCV_FLAG              BIT(5)
#define TEMP_PROTECT_BITMASK            BIT(4)
#define DELTA_RC_SCENE_BITMASK          BIT(3)
#define PD_BY_OCV_WRONG_BIT             BIT(2)
#define NV_READ_BITMASK                 BIT(1)
#define NV_SAVE_BITMASK                 BIT(0)

#define COUL_HARDWARE_SAVE_OCV_ADDR            PMIC_HRST_REG2_ADDR(0) /* use 2byte,reserved3 and reserved4 */
#define COUL_HARDWARE_SAVE_OCV_RESERVED        PMIC_HRST_REG3_ADDR(0)
#define INVALID_TO_UPDATE_FCC           0x8000

#define COUL_HARDWARE_SAVE_OCV_TEMP_ADDR       PMIC_HRST_REG4_ADDR(0) /* OCV TEMP saved use 2bytes */
#define COUL_HARDWARE_SAVE_OCV_TEMP_RESERVED   PMIC_HRST_REG5_ADDR(0)

/* record last soc */
#define COUL_HARDWARE_SAVE_LAST_SOC            PMIC_HRST_REG6_ADDR(0) /* last soc 0-6bit */
#define COUL_HARDWARE_SAVE_LAST_SOC_VAILD      PMIC_HRST_REG6_ADDR(0) /* last soc vaild 7bit */
#define SAVE_LAST_SOC (BIT(6) | BIT(5) | BIT(4) | BIT(3) | \
	BIT(2) | BIT(1) | BIT(0))
#define SAVE_LAST_SOC_FLAG              BIT(7)
#define CLEAR_LAST_SOC_FLAG             0x7F

#define COUL_HARDWARE_OCV_LEVEL_ADDR           PMIC_HRST_REG7_ADDR(0) /* last soc 2-5bit */
#define SAVE_OCV_LEVEL                  (BIT(5) | BIT(4) | BIT(3) | BIT(2))
#define OCV_LEVEL_SHIFT                 2

#define COUL_HARDWARE_ECO_OCV_ADDR             PMIC_HRST_REG7_ADDR(0) /* 6-7bit */
#define EN_ECO_SAMPLE                   BIT(6)
#define CLR_ECO_SAMPLE                  BIT(7)
#define EN_ECO_SAMPLE_FLAG              1
#define OUT_ECO_SAMPLE_FLAG             0

#define DRAINED_BATTERY_FLAG_ADDR       PMIC_HRST_REG12_ADDR(0)
#define DRAINED_BATTERY_FLAG_BIT        BIT(0)
#define BOOT_OCV_ADDR                   PMIC_HRST_REG12_ADDR(0) /* bit 1 */
#define EN_BOOT_OCV_SAMPLE              BIT(1)

/******new**********/

#define COUL_HARDWARE_BATTERY_MOVE_ADDR_H        PMIC_HRST_REG24_ADDR(0)

#define COUL_HARDWARE_OCV_CHOOSE_H               PMIC_HRST_REG25_ADDR(0) /* use bit 5 */
#define COUL_HARDWARE_TEMP_PROTECT_H             PMIC_HRST_REG25_ADDR(0) /* use bit 4 */
#define COUL_HARDWARE_DELTA_RC_SCENE_H           PMIC_HRST_REG25_ADDR(0) /* use bit 3 */
#define COUL_HARDWARE_PD_BY_OCV_WRONG_H          PMIC_HRST_REG25_ADDR(0) /* use bit 2 */
#define COUL_HARDWARE_NV_READ_SUCCESS_H          PMIC_HRST_REG25_ADDR(0) /* use bit 1 */
#define COUL_HARDWARE_NV_SAVE_SUCCESS_H          PMIC_HRST_REG25_ADDR(0) /* use bit 0 */

#define COUL_HARDWARE_SAVE_OCV_ADDR_H            PMIC_HRST_REG26_ADDR(0) /* use 2byte,reserved3 and reserved4 */
#define COUL_HARDWARE_SAVE_OCV_RESERVED_H        PMIC_HRST_REG27_ADDR(0)

#define COUL_HARDWARE_SAVE_OCV_TEMP_ADDR_H       PMIC_HRST_REG28_ADDR(0) /* OCV TEMP saved use 2bytes */
#define COUL_HARDWARE_SAVE_OCV_TEMP_RESERVED_H   PMIC_HRST_REG29_ADDR(0)

#define COUL_HARDWARE_OCV_LEVEL_ADDR_H           PMIC_HRST_REG30_ADDR(0) /* last soc 2-5bit */
#define COUL_HARDWARE_ECO_OCV_ADDR_H             PMIC_HRST_REG30_ADDR(0) /* 6-7bit */

#define DRAINED_BATTERY_FLAG_ADDR_H       PMIC_HRST_REG31_ADDR(0)
#define BOOT_OCV_ADDR_H                   PMIC_HRST_REG31_ADDR(0) /* bit 1 */

/* coul register of smartstar */
#define COUL_CALI_ENABLE                BIT(4)

#define REG_NUM                         3
#define CC_REG_NUM                      5

#define COUL_ECO_FLT_200MS              (BIT(1)|BIT(2))
#define COUL_ALL_REFLASH                0
#define COUL_ECO_REFLASH                BIT(0)
#define DEFAULT_COUL_CTRL_VAL (COUL_ECO_FLT_200MS | COUL_ALL_REFLASH)
#define ECO_COUL_CTRL_VAL     (COUL_ECO_FLT_200MS | COUL_ECO_REFLASH )

#define CALI_AUTO_ONOFF_CTRL            BIT(4)
#define CALI_AUTO_TIME_15S              0
#define CALI_AUTO_TIME_60S              BIT(2)
#define CALI_AUTO_TIME_8MIN             BIT(3)
#define CALI_AUTO_TIME_32MIN            (BIT(2) | BIT(3))

/* Warning: bit change */
#define CALI_CLJ_DEFAULT_VALUE (CALI_AUTO_TIME_4MIN | CALI_AUTO_ONOFF_CTRL)
#define MASK_CALI_AUTO_OFF     (CALI_AUTO_ONOFF_CTRL)

#define WAIT_COMP_EN      (REG_CLJ_CTRL_REGS3_wait_comp_en_START)


#define COUL_CLEAR_IRQ                  0x3F
#define COUL_I_OUT_MASK                 BIT(5)
#define COUL_I_IN_MASK                  BIT(4)
#define COUL_VBAT_INT_MASK              BIT(3)
#define COUL_CL_IN_MASK                 BIT(2)
#define COUL_CL_OUT_MASK                BIT(1)
#define COUL_CL_INT_MASK                BIT(0)
#define COUL_INT_MASK_ALL (COUL_I_OUT_MASK | COUL_I_IN_MASK | \
	COUL_VBAT_INT_MASK | COUL_CL_IN_MASK | \
	COUL_CL_OUT_MASK | COUL_CL_INT_MASK)

#define COUL_WORKING                    0x5
#define COUL_CALI_ING                   0x4

#define COUL_MSTATE_MASK                0x0f

#define FIFO_DEPTH                      8

/* register write lock/unlock */
#define COUL_WRITE_LOCK                 0x5a
#define COUL_WRITE_UNLOCK               0xac

#define SOH_DIS                         0
#define SOH_EN                          1

#define ECO_FIFO_CLEAR              BIT(REG_CLJ_CTRL_REGS4_eco_data_clr_START)
#define ECO_FIFO_EN                 BIT(REG_CLJ_CTRL_REGS4_data_eco_en_START)

#define VERSION_VAL   0x36

// 0x510
#define COUL_REG_DEBUG_WRITE_PRO_ADDR      REG_DEBUG_WRITE_PRO_ADDR(REG_COUL_NOPWR0_BASE)
// 0x330
#define COUL_REG_BAT_CNCT_SEL_ADDR      REG_BAT_CNCT_SEL_ADDR(REG_GLB_BASE)
// 0x51a
#define COUL_REG_V_INT0_ADDR     REG_V_INT0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X51D
#define COUL_REG_I_OUT_GATE0_ADDR  REG_I_OUT_GATE0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X520
#define COUL_REG_I_IN_GATE0_ADDR           REG_I_IN_GATE0_ADDR(REG_COUL_NOPWR0_BASE)
// 0x400
#define COUL_REG_CLJ_CTRL_REG_ADDR        REG_CLJ_CTRL_REG_ADDR(REG_COUL_PWR_BASE)
// 0X509
#define COUL_REG_CLJ_CTRL_REG1_ADDR        REG_CLJ_CTRL_REG1_ADDR(REG_COUL_NOPWR0_BASE)
// 0X50A
#define COUL_REG_CLJ_CTRL_REGS2_ADDR REG_CLJ_CTRL_REGS2_ADDR(REG_COUL_NOPWR0_BASE)
// 0X401
#define COUL_REG_CLJ_CTRL_REGS3_ADDR   REG_CLJ_CTRL_REGS3_ADDR(REG_COUL_PWR_BASE)
// 0X402
#define COUL_REG_CLJ_CTRL_REGS4_ADDR  REG_CLJ_CTRL_REGS4_ADDR(REG_COUL_PWR_BASE)
// 0X50C
#define COUL_REG_STATE_TEST_ADDR           REG_STATE_TEST_ADDR(REG_COUL_NOPWR0_BASE)
// 0X500
#define COUL_REG_COUL_IRQ_FLAG_ADDR        REG_COUL_IRQ_FLAG_ADDR(REG_COUL_NOPWR0_BASE)
// 0X6A3
#define COUL_REG_L_OFFSET_VOLTAGE0_ADDR    REG_L_OFFSET_VOLTAGE0_ADDR(REG_COUL_NOPWR1_BASE)
// 0x552
#define COUL_REG_L_CL_OUT_RO0_ADDR         REG_L_CL_OUT_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X557
#define COUL_REG_L_CL_IN_RO0_ADDR          REG_L_CL_IN_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X55C
#define COUL_REG_L_CHG_TIMER_RO0_ADDR      REG_L_CHG_TIMER_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X560
#define COUL_REG_L_LOAD_TIMER_RO0_ADDR     REG_L_LOAD_TIMER_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X68C
#define COUL_REG_L_CHG_TIMER0_ADDR         REG_L_CHG_TIMER0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X690
#define COUL_REG_L_LOAD_TIMER0_ADDR        REG_L_LOAD_TIMER0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X69D
#define COUL_REG_L_V_OUT_0_ADDR            REG_L_V_OUT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X69A
#define COUL_REG_L_CURRENT_0_ADDR          REG_L_CURRENT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X6B6
#define COUL_REG_L_V_PRE0_OUT0_ADDR        REG_L_V_PRE0_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X6CE
#define COUL_REG_L_CURRENT_PRE0_OUT0_ADDR  REG_L_CURRENT_PRE0_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X6AC
#define COUL_REG_L_ECO_OUT_CLIN_0_ADDR     REG_L_ECO_OUT_CLIN_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0x6b1
#define COUL_REG_L_ECO_OUT_CLOUT_0_ADDR    REG_L_ECO_OUT_CLOUT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X505
#define COUL_REG_COUL_L_IRQ_MASK_ADDR      REG_COUL_L_IRQ_MASK_ADDR(REG_COUL_NOPWR0_BASE)
// 0X502
#define COUL_REG_COUL_L_IRQ_FLAG_ADDR      REG_COUL_L_IRQ_FLAG_ADDR(REG_COUL_NOPWR0_BASE)
// 0X682
#define COUL_REG_L_CL_OUT0_ADDR            REG_L_CL_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X687
#define COUL_REG_L_CL_IN0_ADDR             REG_L_CL_IN0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X627
#define COUL_REG_H_OFFSET_VOLTAGE0_ADDR    REG_H_OFFSET_VOLTAGE0_ADDR(REG_COUL_NOPWR1_BASE)
// 0x540
#define COUL_REG_H_CL_OUT_RO0_ADDR         REG_H_CL_OUT_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0x545
#define COUL_REG_H_CL_IN_RO0_ADDR          REG_H_CL_IN_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X54A
#define COUL_REG_H_CHG_TIMER_RO0_ADDR      REG_H_CHG_TIMER_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0X54E
#define COUL_REG_H_LOAD_TIMER_RO0_ADDR     REG_H_LOAD_TIMER_RO0_ADDR(REG_COUL_NOPWR0_BASE)
// 0x60a
#define COUL_REG_H_CHG_TIMER0_ADDR         REG_H_CHG_TIMER0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X60E
#define COUL_REG_H_LOAD_TIMER0_ADDR        REG_H_LOAD_TIMER0_ADDR(REG_COUL_NOPWR1_BASE)
// 0x61e
#define COUL_REG_H_V_OUT_0_ADDR            REG_H_V_OUT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X61B
#define COUL_REG_H_CURRENT_0_ADDR          REG_H_CURRENT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X63A
#define COUL_REG_H_V_PRE0_OUT0_ADDR        REG_H_V_PRE0_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X652
#define COUL_REG_H_CURRENT_PRE0_OUT0_ADDR  REG_H_CURRENT_PRE0_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X630
#define COUL_REG_H_ECO_OUT_CLIN_0_ADDR     REG_H_ECO_OUT_CLIN_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X635
#define COUL_REG_H_ECO_OUT_CLOUT_0_ADDR    REG_H_ECO_OUT_CLOUT_0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X504
#define COUL_REG_COUL_H_IRQ_MASK_ADDR       REG_COUL_H_IRQ_MASK_ADDR(REG_COUL_NOPWR0_BASE)
// 0X600
#define COUL_REG_H_CL_OUT0_ADDR            REG_H_CL_OUT0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X605
#define COUL_REG_H_CL_IN0_ADDR             REG_H_CL_IN0_ADDR(REG_COUL_NOPWR1_BASE)
// 0X501
#define COUL_REG_COUL_H_IRQ_FLAG_ADDR       REG_COUL_H_IRQ_FLAG_ADDR(REG_COUL_NOPWR0_BASE)

#endif
