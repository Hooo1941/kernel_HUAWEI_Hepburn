/* SPDX-License-Identifier: GPL-2.0 */
/*
 * cps8601_chip.h
 *
 * cps8601 registers, chip info, etc.
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _CPS8601_CHIP_H_
#define _CPS8601_CHIP_H_

#define CPS8601_I2C_ADDR                      0x41
#define CPS8601_ADDR_LEN                      2
#define CPS8601_HW_ADDR_LEN                   4
#define CPS8601_CHIP_INFO_STR_LEN             128

/* chip id register */
#define CPS8601_CHIP_ID_ADDR                  0x0000
#define CPS8601_CHIP_ID_LEN                   2
#define CPS8601_CHIP_ID                       0x8601
/* mtp_version register */
#define CPS8601_MTP_VER_ADDR                  0x0002
#define CPS8601_MTP_VER_LEN                   2
/* op mode register */
#define CPS8601_OP_MODE_ADDR                  0x0004
#define CPS8601_OP_MODE_LEN                   1
#define CPS8601_OP_MODE_UNKOWN                0x00
#define CPS8601_OP_MODE_BP                    0x01
#define CPS8601_OP_MODE_TX                    0x02
/* send_msg_data register */
#define CPS8601_SEND_MSG_HEADER_ADDR          0x0049
#define CPS8601_SEND_MSG_CMD_ADDR             0x004A
#define CPS8601_SEND_MSG_DATA_ADDR            0x004B
/* send_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define CPS8601_SEND_MSG_DATA_LEN             4
#define CPS8601_SEND_MSG_PKT_LEN              6
/* rcvd_msg_data register */
#define CPS8601_RCVD_MSG_HEADER_ADDR          0x0040
#define CPS8601_RCVD_MSG_CMD_ADDR             0x0041
#define CPS8601_RCVD_MSG_DATA_ADDR            0x0042
/* rcvd_msg: bit[0]: header, bit[1]: cmd, bit[2:5]: data */
#define CPS8601_RCVD_MSG_DATA_LEN             4
#define CPS8601_RCVD_MSG_PKT_LEN              6
#define CPS8601_RCVD_PKT_BUFF_LEN             8
#define CPS8601_RCVD_PKT_STR_LEN              64

/*
 * tx mode
 */

/* tx_irq_en register */
#define CPS8601_TX_IRQ_EN_ADDR                0x0005
#define CPS8601_TX_IRQ_VAL                    0XFFFF
#define CPS8601_TX_IRQ_EN_LEN                 2
/* tx_irq_latch register */
#define CPS8601_TX_IRQ_ADDR                   0x0007
#define CPS8601_TX_IRQ_LEN                    2
#define CPS8601_TX_IRQ_TX_INIT                BIT(0)
#define CPS8601_TX_IRQ_START_PING             BIT(1)
#define CPS8601_TX_IRQ_SS_PKG_RCVD            BIT(2)
#define CPS8601_TX_IRQ_ID_PKT_RCVD            BIT(3)
#define CPS8601_TX_IRQ_CFG_PKT_RCVD           BIT(4)
#define CPS8601_TX_IRQ_ASK_PKT_RCVD           BIT(5)
#define CPS8601_TX_IRQ_EPT_PKT_RCVD           BIT(6)
#define CPS8601_TX_IRQ_RPP_TIMEOUT            BIT(7)
#define CPS8601_TX_IRQ_CEP_TIMEOUT            BIT(8)
#define CPS8601_TX_IRQ_RX_ATTACH              BIT(9)
#define CPS8601_TX_IRQ_RX_REMOVED             BIT(10)
#define CPS8601_TX_IRQ_OTHER_ERROR            BIT(11)
#define CPS8601_TX_IRQ_Q_CAIL                 BIT(12)
/* tx_irq_clr register */
#define CPS8601_TX_IRQ_CLR_ADDR               0x0009
#define CPS8601_TX_IRQ_CLR_LEN                2
#define CPS8601_TX_IRQ_CLR_ALL                0xFFFF
/* tx_cmd register */
#define CPS8601_TX_CMD_ADDR                   0x000B
#define CPS8601_TX_CMD_LEN                    2
#define CPS8601_TX_CMD_VAL                    1
#define CPS8601_TX_CMD_EN_DLP_MODE            BIT(0) /* DEEP LOW POWER */
#define CPS8601_TX_CMD_EN_DLP_MODE_SHIFT      0
#define CPS8601_TX_CMD_EN_TX                  BIT(1)
#define CPS8601_TX_CMD_EN_TX_SHIFT            1
#define CPS8601_TX_CMD_CRC_CHK                BIT(2)
#define CPS8601_TX_CMD_CRC_CHK_SHIFT          2
#define CPS8601_TX_CMD_DIS_TX                 BIT(3)
#define CPS8601_TX_CMD_DIS_TX_SHIFT           3
#define CPS8601_TX_CMD_SEND_MSG               BIT(4)
#define CPS8601_TX_CMD_SEND_MSG_SHIFT         4
#define CPS8601_TX_CMD_SYS_RST                BIT(5)
#define CPS8601_TX_CMD_SYS_RST_SHIFT          5
#define CPS8601_TX_CMD_Q_CAIL                 BIT(6)
#define CPS8601_TX_CMD_Q_CAIL_SHIFT           6
/* func_en register */
#define CPS8601_TX_FUNC_EN_ADDR               0x000C
#define CPS8601_TX_FUNC_EN_LEN                2
#define CPS8601_TX_FUNC_EN                    1
#define CPS8601_TX_FUNC_DIS                   0
#define CPS8601_TX_FOD_EN_MASK                BIT(0)
#define CPS8601_TX_FOD_EN_SHIFT               1
/* tx_q_cali_factory_cnt register */
#define CPS8601_TX_CALI_FAC_CNT_ADDR          0x000D
#define CPS8601_TX_CALI_FAC_CNT_LEN           1
/* tx_q_cali_factory_width register */
#define CPS8601_TX_CALI_FAC_WIDTH_ADDR        0x000E
#define CPS8601_TX_CALI_FAC_WIDTH_LEN         2
/* tx_q_no_rx_cnt register */
#define CPS8601_TX_NO_RX_CNT_ADDR             0x0010
#define CPS8601_TX_NO_RX_CNT_LEN              1
/* tx_q_no_rx_width register */
#define CPS8601_TX_NO_RX_WIDTH_ADDR           0x0011
#define CPS8601_TX_NO_RX_WIDTH_LEN            2
/* tx_q_no_rx_cnt_var register */
#define CPS8601_TX_NO_RX_CNT_VAR_ADDR         0x0013
#define CPS8601_TX_NO_RX_CNT_VAR_LEN          1
/* tx_q_no_rx_width_var register */
#define CPS8601_TX_NO_RX_WIDTH_VAR_ADDR       0x0014
#define CPS8601_TX_NO_RX_WIDTH_VAR_LEN        1
/* tx_q_cali_dynamic_cnt register */
#define CPS8601_TX_CALI_DYN_CNT_ADDR          0x0016
#define CPS8601_TX_CALI_DYN_CNT_LEN           1
/* tx_q_cali_dynamic_width register */
#define CPS8601_TX_CALI_DYN_WIDTH_ADDR        0x0017
#define CPS8601_TX_CALI_DYN_WIDTH_LEN         2
/* tx_q_rx_remove_cnt_delta register */
#define CPS8601_TX_RX_REMOVE_CNT_DELTA_ADDR   0x0019
#define CPS8601_TX_RX_REMOVE_CNT_DELTA_LEN    1
/* tx_q_rx_remove_width_delta register */
#define CPS8601_TX_RX_REMOVE_WIDTH_DELTA_ADDR 0x001A
#define CPS8601_TX_RX_REMOVE_WIDTH_DELTA_LEN  1
/* tx_ping_time, in ms */
#define CPS8601_TX_PING_TIME_ADDR             0x001B
#define CPS8601_TX_PING_TIME                  80
/* tx_ping_interval, in ms */
#define CPS8601_TX_PING_INTERVAL_ADDR         0x001C
#define CPS8601_TX_PING_INTERVAL_LEN          2
#define CPS8601_TX_PING_INTERVAL_MIN          0
#define CPS8601_TX_PING_INTERVAL_MAX          1000
#define CPS8601_TX_PING_INTERVAL              120
/* tx_pwm_duty register */
#define CPS8601_TX_PWM_DUTY_ADDR              0x001E
#define CPS8601_TX_PWM_DUTY_LEN               2
#define CPS8601_TX_PWM_DUTY_UNIT              10
/* tx_min_fop, in kHz */
#define CPS8601_TX_MIN_FOP_ADDR               0x0020
#define CPS8601_TX_MIN_FOP_LEN                2
#define CPS8601_TX_MIN_FOP                    113
/* tx_max_fop, in kHz */
#define CPS8601_TX_MAX_FOP_ADDR               0x0022
#define CPS8601_TX_MAX_FOP_LEN                2
#define CPS8601_TX_MAX_FOP                    145
/* tx_ping_freq, in kHz */
#define CPS8601_TX_PING_FREQ_ADDR             0x0024
#define CPS8601_TX_PING_FREQ_LEN              2
#define CPS8601_TX_PING_FREQ                  135
#define CPS8601_TX_PING_FREQ_MIN              100
#define CPS8601_TX_PING_FREQ_MAX              150
#define CPS8601_TX_PING_FREQ_UNIT             10
/* tx_ping_ocp, in mA */
#define CPS8601_TX_PING_OCP_ADDR              0x0026
#define CPS8601_TX_PING_OCP_LEN               2
#define CPS8601_TX_PING_OCP_TH                800
/* tx_ocp_thres register, in mA */
#define CPS8601_TX_OCP_TH_ADDR                0x0028
#define CPS8601_TX_OCP_TH_LEN                 2
#define CPS8601_TX_OCP_TH                     800
/* tx_ovp_thres register, in mV */
#define CPS8601_TX_OVP_TH_ADDR                0x002A
#define CPS8601_TX_OVP_TH_LEN                 2
#define CPS8601_TX_OVP_TH                     9000
/* tx_uvp_thres register, in mA */
#define CPS8601_TX_UVP_TH_ADDR                0x002C
#define CPS8601_TX_UVP_TH_LEN                 2
#define CPS8601_TX_UVP_TH                     4000
/* tx_fod_ploss_cnt register */
#define CPS8601_TX_PLOSS_CNT_ADDR             0x002E
#define CPS8601_TX_PLOSS_CNT_LEN              1
#define CPS8601_TX_PLOSS_CNT_VAL              5
/* tx_fod_ploss_rp_thres0 register, in mW */
#define CPS8601_TX_PLOSS_RP_TH0_ADDR          0x0052
#define CPS8601_TX_PLOSS_RP_TH0_LEN           2
#define CPS8601_TX_PLOSS_RP_TH0_VAL           900
/* tx_fod_ploss_thres0 register, in mW */
#define CPS8601_TX_PLOSS_TH0_ADDR             0x002F
#define CPS8601_TX_PLOSS_TH0_LEN              2
#define CPS8601_TX_PLOSS_TH0_VAL              2200
/* tx_rx_status register, in mW */
#define CPS8601_TX_RX_STATUS_ADDR             0x0031
#define CPS8601_TX_RX_STATUS_LEN              1
/* tx_oper_freq register, in Hz */
#define CPS8601_TX_OP_FREQ_ADDR               0x0032
#define CPS8601_TX_OP_FREQ_LEN                2
#define CPS8601_TX_OP_FREQ_UNIT               10
/* tx_vin register, in mV */
#define CPS8601_TX_VIN_ADDR                   0x0034
#define CPS8601_TX_VIN_LEN                    2
/* tx_vrect register, in mV */
#define CPS8601_TX_VRECT_ADDR                 0x0036
#define CPS8601_TX_VRECT_LEN                  2
/* tx_iin register, in mA */
#define CPS8601_TX_IIN_ADDR                   0x0038
#define CPS8601_TX_IIN_LEN                    2
/* tx_chip_temp register, in degC */
#define CPS8601_TX_CHIP_TEMP_ADDR             0x003A
#define CPS8601_TX_CHIP_TEMP_LEN              1
/* tx_receive_rx_ept_type register */
#define CPS8601_TX_RCVD_RX_EPT_ADDR           0x003B
#define CPS8601_TX_RCVD_RX_EPT_CLEAR          0
#define CPS8601_TX_RCVD_RX_EPT_LEN            1
/* crc val register */
#define CPS8601_CRC_ADDR                      0x003C
#define CPS8601_CRC_LEN                       2
/* tx_ept_type register */
#define CPS8601_TX_EPT_SRC_ADDR               0x003E
#define CPS8601_TX_EPT_SRC_CLEAR              0
#define CPS8601_TX_EPT_SRC_LEN                2
#define CPS8601_TX_EPT_SRC_WRONG_PKT          BIT(0)
#define CPS8601_TX_EPT_SRC_SSP                BIT(2)
#define CPS8601_TX_EPT_SRC_RX_EPT             BIT(3)
#define CPS8601_TX_EPT_SRC_CEP_TIMEOUT        BIT(4)
#define CPS8601_TX_EPT_SRC_OCP                BIT(6)
#define CPS8601_TX_EPT_SRC_OVP                BIT(7)
#define CPS8601_TX_EPT_SRC_UVP                BIT(8)
#define CPS8601_TX_EPT_SRC_FOD                BIT(9)
#define CPS8601_TX_EPT_SRC_OTP                BIT(10)
#define CPS8601_TX_EPT_SRC_POCP               BIT(11)
/* tx_fod_ploss_thres1 register, in mW */
#define CPS8601_TX_PLOSS_TH1_ADDR             0x0056
#define CPS8601_TX_PLOSS_TH1_LEN              2
#define CPS8601_TX_PLOSS_TH1_VAL              2800
/* tx_fod_ploss_thres2 register, in mW */
#define CPS8601_TX_PLOSS_TH2_ADDR             0x0058
#define CPS8601_TX_PLOSS_TH2_LEN              2
#define CPS8601_TX_PLOSS_TH2_VAL              3300
/* tx_cep_timeout */
#define CPS8601_TX_CEP_TIMEOUT_ADDR           0x005A
#define CPS8601_TX_CEP_TIMEOUT_LEN            2
#define CPS8601_TX_CEP_TIMEOUT_VAL            1200
/*
 * firmware register
 */

/* sram addr */
#define CPS8601_BOOTLOADER_BOOT_ADD           0x20000000
#define CPS8601_BOOTLOADER_CMD_ADD            0x200009FC
#define CPS8601_BOOTLOADER_FLAG_ADD           0x200009F8
#define CPS8601_BOOTLOADER_ADDR_BUFFER0       0x20000A00
#define CPS8601_BOOTLOADER_ADDR_BUFFER1       0x20000B00
/* sys set addr */
#define CPS8601_ACCESS_32BIT_REG_ADD          0xFFFFFF00
#define CPS8601_CLKCTRL_I2C_BUS_MODE          0x40040030
#define CPS8601_CLKCTRL_PASSWORD              0x40040070
#define CPS8601_CLKCTRL_SYSCONFIG             0x40040008
#define CPS8601_DMACTRL_CHANNEL               0x4000E000
#define CPS8601_CLKCTRL_WATCHDOG_LOAD         0x40040048
#define CPS8601_CLKCTRL_TRIM_DIS              0x40040014
/* sys data */
#define CPS8601_ENABLE_32BIT_ADD_ACCESS       0x0000000E
#define CPS8601_I2C_BUS_MODE_LITTLE_ENDIAN    0xFFFFFFFF
#define CPS8601_I2C_BUS_MODE_BIG_ENDIAN       0x00000000
#define CPS8601_ENABLE_PASSWORD               0x0000A061
#define CPS8601_RESET_MCU                     0x00000008
#define CPS8601_DISABLE_ALL_CHANNEL           0x00000000
#define CPS8601_WATCH_DOG_RELOAD_MAX          0x00000FFF
#define CPS8601_TRIMING_LOAD_DISABLED         0x00000001
#define CPS8601_SET_BOOT_SOURCE_SRAM          0x00000020
#define CPS8601_RESET_ALL_SYSTEM              0x00000001
#define CPS8601_SRAM_MTP_BUFF_SIZE            256
/* fw cmd addr */
#define CPS8601_CMD_PGM_BUFFER0               0x00000010
#define CPS8601_CMD_PGM_ERASER                0x00000070
#define CPS8601_CMD_PGM_WR_FLAG               0x00000080
#define CPS8601_CMD_CACL_CRC_MTP              0x00000090
#define CPS8601_CMD_CACL_CRC_TEST             0x000000B0
#define CPS8601_CMD_PGM_Q_FACTOR_FACTORY      0x000000F0
#define CPS8601_CMD_PGM_Q_FACTOR_DYNAMIC      0x000000F1
/* fw cmd check */
#define CPS8601_CHK_SUCC                      0x20
#define CPS8601_CHK_FAIL                      0x30
#define CPS8601_CHK_ILLEGAL                   0x40
/* fw crc calculation */
#define CPS8601_FW_CRC_SEED                   0x1021
#define CPS8601_FW_CRC_HIGHEST_BIT            0x8000

#endif /* _CPS8601_CHIP_H_ */
