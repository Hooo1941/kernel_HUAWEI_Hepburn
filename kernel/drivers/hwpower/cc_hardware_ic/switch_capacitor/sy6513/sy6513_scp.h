/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SY6513_protocol.h
 *
 * SY6513 protocol header file
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

#ifndef _SY6513_SCP_H_
#define _SY6513_SCP_H_

#include "sy6513.h"

/* SCP_PROTOCOL */
#define SY6513_SCP_ACK_RETRY_TIME                3
#define SY6513_SCP_RESTART_TIME                  4
#define SY6513_SCP_RETRY_TIME                    3
#define SY6513_SCP_POLL_TIME                     100
#define SY6513_SCP_DETECT_MAX_CNT                10
#define SY6513_SCP_PING_DETECT_MAX_CNT           15
#define SY6513_SCP_NO_ERR                        0
#define SY6513_SCP_IS_ERR                        1
#define SY6513_SCP_WRITE_OP                      0
#define SY6513_SCP_READ_OP                       1
#define SY6513_SCP_MULTI_READ_LEN                8
#define SY6513_SCP_MULTI_WRITE_LEN               8
#define SY6513_SCP_ACK_AND_CRC_LEN               2
#define SY6513_SCP_SBRWR_NUM                     1
#define SY6513_SCP_CMD_SBRRD                     0x0c
#define SY6513_SCP_CMD_SBRWR                     0x0b
#define SY6513_SCP_CMD_MBRRD                     0x1c
#define SY6513_SCP_CMD_MBRWR                     0x1b
#define SY6513_SCP_ACK                           0x08
#define SY6513_SCP_NACK                          0x03

/* fcp adapter vol value */
#define SY6513_FCP_ADAPTER_MAX_VOL               12000
#define SY6513_FCP_ADAPTER_MIN_VOL               5000
#define SY6513_FCP_ADAPTER_RST_VOL               5000
#define SY6513_FCP_ADAPTER_ERR_VOL               500
#define SY6513_FCP_ADAPTER_VOL_CHECK_TIME        10

/* DEVICE_ID reg=0xA0 */
#define SY6513_DEVICE_ID_REG                     0xA0
#define SY6513_REVISION_ID_MASK                  (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3))
#define SY6513_REVISION_ID_SHIFT                 3
#define SY6513_VENDOR_ID_MASK                    (BIT(2) | BIT(1) | BIT(0))
#define SY6513_VENDOR_ID_SHIFT                   O

/* INT1 reg=0xA1 */
#define SY6513_INT1_REG                          0xA1
#define SY6513_STIMER_INT_MASK                   BIT(7)
#define SY6513_STIMER_INT_SHIFT                  7
#define SY6513_INTB_WD_INT_MASK                  BIT(6)
#define SY6513_INTB_WD_INT_SHIFT                 6
#define SY6513_VBUS_CHANGE_MASK                  BIT(5)
#define SY6513_VBUS_CHANGE_SHIFT                 5
#define SY6513_DEVICE_CHANGE_MASK                BIT(4)
#define SY6513_DEVICE_CHANGE_SHIFT               4
#define SY6513_SCP_DM_DET_FLAG_MASK              BIT(3)
#define SY6513_SCP_DM_DET_FLAG_SHIFT             3
#define SY6513_SCP_DET_FAIL_FLAG_MASK            BIT(2)
#define SY6513_SCP_DET_FAIL_FLAG_SHIFT           2
#define SY6513_ENABLE_HAND_NO_RESPOND_MASK       BIT(1)
#define SY6513_ENABLE_HAND_NO_RESPOND_SHIFT      1
#define SY6513_TRANS_HAND_NO_RESPOND_MASK        BIT(0)
#define SY6513_TRANS_HAND_NO_RESPOND_SHIFT       0

/* INT MASK1 reg=0xA2 */
#define SY6513_INT1_MASK                         0xA2
#define SY6513_STIMER_INT_MASK_MASK              BIT(7)
#define SY6513_STIMER_INT_MASK_SHIFT             7
#define SY6513_INTB_WD_INT_MASK_MASK             BIT(6)
#define SY6513_INTB_WD_INT_MASK_SHIFT            6
#define SY6513_VBUS_CHANGE_MASK_MASK             BIT(5)
#define SY6513_VBUS_CHANGE_MASK_SHIFT            5
#define SY6513_DEVICE_CHANGE_MASK_MASK           BIT(4)
#define SY6513_DEVICE_CHANGE_MASK_SHIFT          4
#define SY6513_SCP_DM_DET_FLAG_MASK_MASK         BIT(3)
#define SY6513_SCP_DM_DET_FLAG_MASK_SHIFT        3
#define SY6513_SCP_DET_FAIL_FLAG_MASK_MASK       BIT(2)
#define SY6513_SCP_DET_FAIL_FLAG_MASK_SHIFT      2
#define SY6513_ENABLE_HAND_NO_RESPOND_MASK_MASK  BIT(1)
#define SY6513_ENABLE_HAND_NO_RESPOND_MASK_SHIFT 1
#define SY6513_TRANS_HAND_NO_RESPOND_MASK_MASK   BIT(0)
#define SY6513_TRANS_HAND_NO_RESPOND_MASK_SHIFT  0

/* DETECT_CONTROL reg=0xA3 */
#define SY6513_DETECT_CONTROL_REG                0xA3
#define SY6513_DPDM_DETC_EN_MASK                 BIT(7)
#define SY6513_DPDM_DETC_EN_SHIFT                7
#define SY6513_DETECT_DP_MASK                    (BIT(6) | BIT(5) | BIT(4))
#define SY6513_DETECT_DP_SHIFT                   4
#define SY6513_DETECT_DP_IS_0P6V                 2
#define SY6513_DETECT_DM_MASK                    (BIT(2) | BIT(1) | BIT(0))
#define SY6513_DETECT_DM_SHIFT                   0

/* BC12_CONTROL reg=0xA4 */
#define SY6513_BC12_CONTROL_REG                  0xA4
#define SY6513_DCD_TIMEOUT_SET_MASK              (BIT(5) | BIT(4))
#define SY6513_DCD_TIMEOUT_SET_SHIFT             4
#define SY6513_CHGDET_ON_TIME_MASK               BIT(3)
#define SY6513_CHGDET_ON_TIME_SHIFT              3
#define SY6513_BC12_AUTO_MASK                    BIT(2)
#define SY6513_BC12_AUTO_SHIFT                   2
#define SY6513_BC12_AUTO_ENABLE                  1

/* SCP_CONTROL reg=0xA5 */
#define SY6513_SCP_CONTROL_REG                   0xA5
#define SY6513_SCP_IO_SEL_MASK                   BIT(7)
#define SY6513_SCP_IO_SEL_SHIFT                  7
#define SY6513_SCP_EN_MASK                       BIT(6)
#define SY6513_SCP_EN_SHIFT                      6
#define SY6513_SET_DP_MASK                       (BIT(5) | BIT(4))
#define SY6513_SET_DP_SHIFT                      4
#define SY6513_SET_DM_MASK                       (BIT(3) | BIT(2))
#define SY6513_SET_DM_SHIFT                      2
#define SY6513_SET_DPDM_EN_MASK                  BIT(1)
#define SY6513_SET_DPDM_EN_SHIFT                 1
#define SY6513_SCP_DET_EN_MASK                   BIT(0)
#define SY6513_SCP_DET_EN_SHIFT                  0

/* SCP_TIMING reg=0xA6 */
#define SY6513_SCP_TIMING_REG                    0xA6
#define SY6513_EN_STIMER_MASK                    BIT(7)
#define SY6513_EN_STIMER_SHIFT                   7
#define SY6513_STIMER_MASK                       (BIT(6) | BIT(5))
#define SY6513_STIMER_SHIFT                      5
#define SY6513_INTB_WD_EN_MASK                   BIT(2)
#define SY6513_INTB_WD_EN_SHIFT                  2
#define SY6513_INTB_WATCHDOG_TIMER_MASK          (BIT(1) | BIT(0))
#define SY6513_INTB_WATCHDOG_TIMER_SHIFT         0

/* USB_STATUS1 reg=0xA7 */
#define SY6513_USB_STATUS1_REG                   0xA7
#define SY6513_RX_UI_TIME_CONTROL_MASK           (BIT(6) | BIT(5) | BIT(4))
#define SY6513_RX_UI_TIME_CONTROL_SHIFT          4
#define SY6513_DVC_MASK                          BIT(3)
#define SY6513_DVC_SHIFT                         3

/* USB_STATUS2 reg=0xA8 */
#define SY6513_USB_STATUS2_REG                   0xA8
#define SY6513_USB_STATUS_MASK                   (BIT(6) | BIT(5) | BIT(4))
#define SY6513_USB_STATUS_SHIFT                  4
#define SY6513_DCDT_MASK                         BIT(0)
#define SY6513_DCDT_SHIFT                        0

/* SCP_STATUS reg=0xA9 */
#define SY6513_SCP_STATUS_REG                    0xA9
#define SY6513_SCP_RDATA_READY_MASK              BIT(6)
#define SY6513_SCP_RDATA_READY_SHIFT             6
#define SY6513_DM_DET_DELAY_MASK                 (BIT(5) | BIT(4))
#define SY6513_DM_DET_DELAY_SHIFT                4
#define SY6513_DM_DET_TIME_MASK                  (BIT(3) | BIT(2))
#define SY6513_DM_DET_TIME_SHIFT                 2
#define SY6513_SLV_R_CPL_MASK                    BIT(1)
#define SY6513_SLV_R_CPL_SHIFT                   1
#define SY6513_RESET_SCP_MASK                    BIT(0)
#define SY6513_RESET_SCP_SHIFT                   0

/* SCP_Command_CONTROL reg=0xAA */
#define SY6513_SCP_CTRL_REG                      0xAA
#define SY6513_MSTR_RST_MASK                     BIT(2)
#define SY6513_MSTR_RST_SHIFT                    2
#define SY6513_SNDCMD_MASK                       BIT(0)
#define SY6513_SNDCMD_SHIFT                      0

/* SCP_CMD reg=0xAB */
#define SY6513_SCP_CMD_REG                       0xAB
#define SY6513_SCP_TX_CMD_MASK                   (BIT(7) | BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_SCP_TX_CMD_SHIFT                  0

/* SCP_ADDR reg=0xAC */
#define SY6513_SCP_ADDR_REG                      0xAC
#define SY6513_SCP_TX_ADDR_MASK                  (BIT(7) | BIT(6) | BIT(5) | BIT(4) | \
	BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_SCP_TX_ADDR_SHIFT                 0

/* SCP_NB reg=0xAD */
#define SY6513_SCP_NB_REG                        0xAD
#define SY6513_SCP_TX_NB_MASK                    (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define SY6513_SCP_TX_NB_SHIFT                   0

/* SCP_INT1 reg=0xAE */
#define SY6513_SCP_INT1_REG                      0xAE
#define SY6513_CMDCPL_MASK                       BIT(7)
#define SY6513_CMDCPL_SHIFT                      7
#define SY6513_ACK_MASK                          BIT(6)
#define SY6513_ACK_SHIFT                         6
#define SY6513_NACK_MASK                         BIT(4)
#define SY6513_NACK_SHIFT                        4
#define SY6513_SCP_PLGIN_MASK                    BIT(3)
#define SY6513_SCP_PLGIN_SHIFT                   3
#define SY6513_ERR_ACK_L_MASK                    BIT(2)
#define SY6513_ERR_ACK_L_SHIFT                   2
#define SY6513_CRCRX_MASK                        BIT(1)
#define SY6513_CRCRX_SHIFT                       1
#define SY6513_PAPRX_MASK                        BIT(0)
#define SY6513_PAPRX_SHIFT                       0

/* SCP_INT MASK1 reg=0xAF */
#define SY6513_SCP_INT_MASK1_REG                 0xAF
#define SY6513_CMDCPL_MASK_MASK                  BIT(7)
#define SY6513_CMDCPL_MASK_SHIFT                 7
#define SY6513_ACK_MASK_MASK                     BIT(6)
#define SY6513_ACK_MASK_SHIFT                    6
#define SY6513_SLV_R_CPL_MASK_MASK               BIT(5)
#define SY6513_SLV_R_CPL_MASK_SHIFT              5
#define SY6513_NACK_MASK_MASK                    BIT(4)
#define SY6513_NACK_MASK_SHIFT                   4
#define SY6513_SCP_PLGIN_MASK_MASK               BIT(3)
#define SY6513_SCP_PLGIN_MASK_SHIFT              3
#define SY6513_ERR_ACK_L_MASK_MASK               BIT(2)
#define SY6513_ERR_ACK_L_MASK_SHIFT              2
#define SY6513_CRCRX_MASK_MASK                   BIT(1)
#define SY6513_CRCRX_MASK_SHIFT                  1
#define SY6513_PAPRX_MASK_MASK                   BIT(0)
#define SY6513_PAPRX_MASK_SHIFT                  0

/* SCP_WDATA1 reg=0xB0 */
#define SY6513_SCP_WDATA1_REG                    0xB0
#define SY6513_SCP_WDATA_TX_DATA1_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA1_SHIFT          0

/* SCP_WDATA2 reg=0xB1 */
#define SY6513_SCP_WDATA2_REG                    0xB1
#define SY6513_SCP_WDATA_TX_DATA2_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA2_SHIFT          0

/* SCP_WDATA3 reg=0xB2 */
#define SY6513_SCP_WDATA3_REG                    0xB2
#define SY6513_SCP_WDATA_TX_DATA3_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA3_SHIFT          0

/* SCP_WDATA4 reg=0xB3 */
#define SY6513_SCP_WDATA4_REG                    0xB3
#define SY6513_SCP_WDATA_TX_DATA4_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA4_SHIFT          0

/* SCP_WDATA5 reg=0xB4 */
#define SY6513_SCP_WDATA5_REG                    0xB4
#define SY6513_SCP_WDATA_TX_DATA5_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA5_SHIFT          0

/* SCP_WDATA6 reg=0xB5 */
#define SY6513_SCP_WDATA6_REG                    0xB5
#define SY6513_SCP_WDATA_TX_DATA6_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA6_SHIFT          0

/* SCP_WDATA7 reg=0xB6 */
#define SY6513_SCP_WDATA7_REG                    0xB6
#define SY6513_SCP_WDATA_TX_DATA7_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA7_SHIFT          0

/* SCP_WDATA8 reg=0xB7 */
#define SY6513_SCP_WDATA8_REG                    0xB7
#define SY6513_SCP_WDATA_TX_DATA8_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA8_SHIFT          0

/* SCP_WDATA9 reg=0xB8 */
#define SY6513_SCP_WDATA9_REG                    0xB8
#define SY6513_SCP_WDATA_TX_DATA9_MASK           0xFF
#define SY6513_SCP_WDATA_TX_DATA9_SHIFT          0

/* SCP_WDATA10 reg=0xB9 */
#define SY6513_SCP_WDATA10_REG                   0xB9
#define SY6513_SCP_WDATA_TX_DATA10_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA10_SHIFT         0

/* SCP_WDATA11 reg=0xBA */
#define SY6513_SCP_WDATA11_REG                   0xBA
#define SY6513_SCP_WDATA_TX_DATA11_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA11_SHIFT         0

/* SCP_WDATA12 reg=0xBB */
#define SY6513_SCP_WDATA12_REG                   0xBB
#define SY6513_SCP_WDATA_TX_DATA12_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA12_SHIFT         0

/* SCP_WDATA13 reg=0xBC */
#define SY6513_SCP_WDATA13_REG                   0xBC
#define SY6513_SCP_WDATA_TX_DATA13_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA13_SHIFT         0

/* SCP_WDATA14 reg=0xBD */
#define SY6513_SCP_WDATA14_REG                   0xBD
#define SY6513_SCP_WDATA_TX_DATA14_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA14_SHIFT         0

/* SCP_WDATA15 reg=0xBE */
#define SY6513_SCP_WDATA15_REG                   0xBE
#define SY6513_SCP_WDATA_TX_DATA15_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA15_SHIFT         0

/* SCP_WDATA16 reg=0xBF */
#define SY6513_SCP_WDATA16_REG                   0xBF
#define SY6513_SCP_WDATA_TX_DATA16_MASK          0xFF
#define SY6513_SCP_WDATA_TX_DATA16_SHIFT         0

/* SCP_RDATA1 reg=0xC0 */
#define SY6513_SCP_RDATA1_REG                    0xC0
#define SY6513_SCP_RDATA_RX_DATA1_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA1_SHIFT          0

/* SCP_RDATA2 reg=0xC1 */
#define SY6513_SCP_RDATA2_REG                    0xC1
#define SY6513_SCP_RDATA_RX_DATA2_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA2_SHIFT          0

/* SCP_RDATA3 reg=0xC2 */
#define SY6513_SCP_RDATA3_REG                    0xC2
#define SY6513_SCP_RDATA_RX_DATA3_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA3_SHIFT          0

/* SCP_RDATA4 reg=0xC3 */
#define SY6513_SCP_RDATA4_REG                    0xC3
#define SY6513_SCP_RDATA_RX_DATA4_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA4_SHIFT          0

/* SCP_RDATA5 reg=0xC4 */
#define SY6513_SCP_RDATA5_REG                    0xC4
#define SY6513_SCP_RDATA_RX_DATA5_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA5_SHIFT          0

/* SCP_RDATA6 reg=0xC5 */
#define SY6513_SCP_RDATA6_REG                    0xC5
#define SY6513_SCP_RDATA_RX_DATA6_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA6_SHIFT          0

/* SCP_RDATA7 reg=0xC6 */
#define SY6513_SCP_RDATA7_REG                    0xC6
#define SY6513_SCP_RDATA_RX_DATA7_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA7_SHIFT          0

/* SCP_RDATA8 reg=0xC7 */
#define SY6513_SCP_RDATA8_REG                    0xC7
#define SY6513_SCP_RDATA_RX_DATA8_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA8_SHIFT          0

/* SCP_RDATA9 reg=0xC8 */
#define SY6513_SCP_RDATA9_REG                    0xC8
#define SY6513_SCP_RDATA_RX_DATA9_MASK           0xFF
#define SY6513_SCP_RDATA_RX_DATA9_SHIFT          0

/* SCP_RDATA10 reg=0xC9 */
#define SY6513_SCP_RDATA10_REG                   0xC9
#define SY6513_SCP_RDATA_RX_DATA10_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA10_SHIFT         0

/* SCP_RDATA11 reg=0xCA */
#define SY6513_SCP_RDATA11_REG                   0xCA
#define SY6513_SCP_RDATA_RX_DATA11_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA11_SHIFT         0

/* SCP_RDATA12 reg=0xCB */
#define SY6513_SCP_RDATA12_REG                   0xCB
#define SY6513_SCP_RDATA_RX_DATA12_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA12_SHIFT         0

/* SCP_RDATA13 reg=0xCC */
#define SY6513_SCP_RDATA13_REG                   0xCC
#define SY6513_SCP_RDATA_RX_DATA13_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA13_SHIFT         0

/* SCP_RDATA14 reg=0xCD */
#define SY6513_SCP_RDATA14_REG                   0xCD
#define SY6513_SCP_RDATA_RX_DATA14_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA14_SHIFT         0

/* SCP_RDATA15 reg=0xCE */
#define SY6513_SCP_RDATA15_REG                   0xCE
#define SY6513_SCP_RDATA_RX_DATA15_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA15_SHIFT         0

/* SCP_RDATA16 reg=0xCF */
#define SY6513_SCP_RDATA16_REG                   0xCF
#define SY6513_SCP_RDATA_RX_DATA16_MASK          0xFF
#define SY6513_SCP_RDATA_RX_DATA16_SHIFT         0

/* BC1.2 reg=0xF0 */
#define SY6513_SCP_BC12_REG                      0xF0
#define SY6513_SCP_SET_BC12                      0x20

int sy6513_protocol_ops_register(struct sy6513_device_info *di);
void sy6513_scp_adapter_default_vset(void *dev_data);
#endif /* _SY6513_SCP_H_ */
