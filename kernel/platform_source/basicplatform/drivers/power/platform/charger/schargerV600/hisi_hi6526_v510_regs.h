/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * hisi_scharger_v510_regs.h
 *
 * HI6526 charger register config header.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI6526_V510_REGS_H__
#define __HI6526_V510_REGS_H__
#include <linux/bitops.h>

#define V510_BASE (0)
#define V510_PAGE0_BASE                     (0x0080)
#define V510_PAGE1_BASE                     (0x0180)
#define V510_PAGE2_BASE                     (0x0280)

#define HI6526_V510_WATCHDOG_MSK            (BIT(0))
#define HI6526_V510_WATCHDOG_OK             (0x00)

#define V510_WATCHDOG_SOFT_RST_REG          ((V510_BASE) + (0x6C) + V510_PAGE0_BASE)
#define V510_WATCHDOG_CTRL_REG              ((V510_BASE) + (0x6D) + V510_PAGE0_BASE)

#define V510_CHG_IRQ_STATUS0                ((V510_BASE) + (0x52) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS1                ((V510_BASE) + (0x53) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS2                ((V510_BASE) + (0x54) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS3                ((V510_BASE) + (0x55) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS4                ((V510_BASE) + (0x56) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS5                ((V510_BASE) + (0x57) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS6                ((V510_BASE) + (0x58) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS7                ((V510_BASE) + (0x59) + V510_PAGE1_BASE)
#define V510_CHG_IRQ_STATUS8                ((V510_BASE) + (0x60) + V510_PAGE1_BASE)

#define V511_VCHG_MINOR_ADJUST_ADDR         ((V510_BASE) + (0x1D) + V510_PAGE2_BASE)
#define V511_VTERM_MINUTE_ADJUST_SHIFT      (4)
#define V511_VTERM_MINUTE_ADJUST_MASK       (0x7 << V511_VTERM_MINUTE_ADJUST_SHIFT)

#define V511_VCHG_MINOR_SHIFT               (7)
#define V511_VCHG_MINOR_MASK                (0x1 << V511_VCHG_MINOR_SHIFT)

#define CHG_FAST_VCHG_8MV                   8
#define CHG_FAST_VCHG_16MV                  16

#define V510_CHG_DCP_CFG_ADDR               ((V510_BASE) + (0x77) + V510_PAGE2_BASE)
#define V510_DCP_CFG_OK_MSK                 (BIT(0))

#define V510_CHG_DET_TOP_CFG_REG            ((V510_BASE) + (0x23) + V510_PAGE2_BASE)
#define V510_CHG_DET_TOP_CFG_SHIFT          (1)
#define V510_CHG_DET_TOP_CFG_MASK           (0x7F << V510_CHG_DET_TOP_CFG_SHIFT)
#define V510_CHG_DET_TOP_CFG_BASE           4000
#define V510_CHG_DET_TOP_CFG_STEP           10

#define V510_CHG_IR_COMP_MAX                 70
#define CHG_IR_COMP_STEP_8                   8

#define V510_CHG_FAST_VCHG_MIN              4000
#define V510_CHG_FAST_VCHG_MAX              4516

#define V511_CHG_FAST_VCHG_MIN              4216
#define V511_CHG_FAST_VCHG_MAX              4617

#define EFUSE_BYTE15                        15
#define EFUSE_BYTE24                        24
#define EFUSE_BYTE25                        25
#define EFUSE_BYTE26                        26
#define EFUSE_BYTE27                        27
#define EFUSE_BYTE28                        28
#define EFUSE_BYTE29                        29
#define EFUSE_BYTE30                        30
#define EFUSE_BYTE31                        31

#define V510_EFUSE_SEL_REG                 ((V510_BASE) + (0x6D) + V510_PAGE1_BASE)
#define V510_EFUSE_SEL_SHIFT               (0)
#define V510_EFUSE_SEL_MASK                (0x7f << V510_EFUSE_SEL_SHIFT)

#define V510_EFUSE_RDATA_REG               ((V510_BASE) + (0x6E) + V510_PAGE1_BASE)

#define V510_EFUSE_PDOB_SEL_REG            ((V510_BASE) + (0x62) + V510_PAGE1_BASE)
#define V510_EFUSE_PDOB_SEL_SHIFT           (0)
#define V510_EFUSE_PDOB_SEL_MASK            (0x1 << V510_EFUSE_PDOB_SEL_SHIFT)
#define V510_EFUSE_PDOB_SEL_CALI_EN         1
#define V510_EFUSE_PDOB_SEL_CALI_DIS        0

#define V510_EFUSE_EN_REG                   ((V510_BASE) + (0x6D) + V510_PAGE1_BASE)
#define V510_EFUSE_EN_SHIFT                 (7)
#define V510_EFUSE_EN_MASK                  (0x1 << V510_EFUSE_EN_SHIFT)
#define V510_EFUSE_EN                       1
#define V510_EFUSE_DIS                      0

#define V510_EFUSE_RD_CTRL                  ((V510_BASE) + (0x64) + V510_PAGE1_BASE)
#define V510_EFUSE_RD_SHIFT                 (0)
#define V510_EFUSE_RD_MASK                  (0x1 << V510_EFUSE_RD_SHIFT)
#define V510_EFUSE_RD_CTRL_EN               1
#define V510_EFUSE_RD_CTRL_DIS              0

#define V510_EFUSE_PDOB_PRE_ADDR_WE         ((V510_BASE) + (0x69) + V510_PAGE1_BASE)
#define V510_EFUSE_PDOB_PRE_ADDR_SHIFT      (0)
#define V510_EFUSE_PDOB_PRE_ADDR_MASK       (0x1f << V510_EFUSE_PDOB_PRE_ADDR_SHIFT)

#define V510_EFUSE_PDOB_PRE_WE_SHIFT        (7)
#define V510_EFUSE_PDOB_PRE_WE_MASK         (0x1 << V510_EFUSE_PDOB_PRE_WE_SHIFT)
#define V510_EFUSE_PDOB_PRE_WE_EN           1
#define V510_EFUSE_PDOB_PRE_WE_DIS          0

#define V510_EFUSE_PDOB_PRE_WDATA           ((V510_BASE) + (0x6A) + V510_PAGE1_BASE)
#define V510_EFUSE_PDOB_PRE_WDATA_SHIFT     (0)
#define V510_EFUSE_PDOB_PRE_WDATA_MASK      (0xFF << V510_EFUSE_PDOB_PRE_WDATA_SHIFT)

#define PLUS 1
#define SUB  0
#define V511_CV_INIT_REG_VAL 7
#define V511_VCHG_MINOR_REG_MAX_VAL 7
#define V511_BUCK_OVER_TEMP 45

int hi6526_efuse_write_2(u8 offset, u8 value, int pre_cali);
int hi6526_efuse_read_2(u8 offset, u8 *value, u8 pre_cali);
#endif