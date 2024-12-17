/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mt5806_chip.h
 *
 * mt5806 registers, chip info, etc.
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

#ifndef _MT5806_CHIP_H_
#define _MT5806_CHIP_H_

#define MT5806_ADDR_LEN                      2
#define MT5806_CMD_RETRY_CNT                 3

/* chip_info: 0x0000 ~ 0x000C */
#define MT5806_CHIP_INFO_ADDR                0x0000
#define MT5806_CHIP_INFO_LEN                 14
/* chip id register */
#define MT5806_CHIP_ID_ADDR                  0x0000
#define MT5806_HW_CHIP_ID_ADDR               0x166C
#define MT5806_CHIP_ID_LEN                   2
#define MT5806_CHIP_ID                       0x5806
#define MT5806_HW_CHIP_ID                    0x5806
#define MT5806_CHIP_ID_AB                    0xFFFF /* abnormal chip id */
/* op mode register */
#define MT5806_OP_MODE_ADDR                  0x0005
#define MT5806_OP_MODE_LEN                   1
#define MT5806_OP_MODE_NA                    0x00
#define MT5806_OP_MODE_SA                    0x05 /* stand_alone */
#define MT5806_OP_MODE_RX                    BIT(0)
#define MT5806_OP_MODE_TX                    0x04
#define MT5806_SYS_MODE_MASK                 (BIT(0) | BIT(2))
#define MT5806_OP_MODE_START_TX              BIT(4)

/*
 * tx mode
 */

/* tx_cmd register */
#define MT5806_TX_CMD_ADDR                   0x0008
#define MT5806_TX_CMD_LEN                    4
#define MT5806_TX_CMD_VAL                    1
#define MT5806_TX_CMD_OPENLOOP               BIT(0)
#define MT5806_TX_CMD_OPENLOOP_SHIFT         0
#define MT5806_TX_CMD_RENORM                 BIT(1)
#define MT5806_TX_CMD_RENORM_SHIFT           1
#define MT5806_TX_CMD_SETPERIOD              BIT(2)
#define MT5806_TX_CMD_SETPERIOD_SHIFT        2
#define MT5806_TX_CMD_RST_SYS                BIT(4)
#define MT5806_TX_CMD_RST_SYS_SHIFT          4
#define MT5806_TX_CMD_CLEAR_INT              BIT(5)
#define MT5806_TX_CMD_CLEAR_INT_SHIFT        5
#define MT5806_TX_CMD_SEND_MSG               BIT(6)
#define MT5806_TX_CMD_SEND_MSG_SHIFT         6
#define MT5806_TX_CMD_OVP                    BIT(7)
#define MT5806_TX_CMD_OVP_SHIFT              7
#define MT5806_TX_CMD_OCP                    BIT(8)
#define MT5806_TX_CMD_OCP_SHIFT              8
#define MT5806_TX_CMD_PING_OCP               BIT(9)
#define MT5806_TX_CMD_PING_OCP_SHIFT         9
#define MT5806_TX_CMD_FOD_CTRL               BIT(10)
#define MT5806_TX_CMD_FOD_CTRL_SHIFT         10
#define MT5806_TX_CMD_LOW_POWER              BIT(11)
#define MT5806_TX_CMD_LOW_POWER_SHIFT        11
#define MT5806_TX_CMD_START_TX               BIT(12)
#define MT5806_TX_CMD_START_TX_SHIFT         12
#define MT5806_TX_CMD_STOP_TX                BIT(13)
#define MT5806_TX_CMD_STOP_TX_SHIFT          13
#define MT5806_TX_CMD_CLEAR_EPT              BIT(14)
#define MT5806_TX_CMD_CLEAR_EPT_SHIFT        14
#define MT5806_TX_CMD_FW_VERIFY              BIT(15)
#define MT5806_TX_CMD_FW_VERIFY_SHIFT        15
#define MT5806_TX_CMD_Q_SCAN                 BIT(16)
#define MT5806_TX_CMD_Q_SCAN_SHIFT           16
/* tc_cmd_flag register */
#define MT5806_TX_CMD_SYNC_ADDR              0x000F
#define MT5806_TX_CMD_SYNC_VAL               0x0055
/* tx_irq_en register */
#define MT5806_TX_IRQ_EN_ADDR                0x0010
#define MT5806_TX_IRQ_EN_LEN                 4
#define MT5806_TX_IRQ_EN_VAL                 0xE02C90F
/* tx_irq_latch register */
#define MT5806_TX_IRQ_ADDR                   0x0014
#define MT5806_TX_IRQ_LEN                    4
#define MT5806_TX_IRQ_SS_PKG_RCVD            BIT(0)
#define MT5806_TX_IRQ_ID_PKT_RCVD            BIT(1)
#define MT5806_TX_IRQ_CFG_PKT_RCVD           BIT(2)
#define MT5806_TX_IRQ_REMOVE_POWER           BIT(6)
#define MT5806_TX_IRQ_POWER_TRANS            BIT(8)
#define MT5806_TX_IRQ_POWERON                BIT(9)
#define MT5806_TX_IRQ_PP_PKT_RCVD            BIT(11)
#define MT5806_TX_IRQ_TX_DISABLE             BIT(13)
#define MT5806_TX_IRQ_TX_ENABLE              BIT(14)
#define MT5806_TX_IRQ_EPT_PKT_RCVD           BIT(15)
#define MT5806_TX_IRQ_START_PING             BIT(17)
#define MT5806_TX_IRQ_RX_ATTACH              BIT(25)
#define MT5806_TX_IRQ_RX_REMOVED             BIT(26)
#define MT5806_TX_IRQ_Q_CAIL                 BIT(27)
/* tx_irq_clr register */
#define MT5806_TX_IRQ_CLR_ADDR               0x0018
#define MT5806_TX_IRQ_CLR_LEN                4
#define MT5806_TX_IRQ_CLR_ALL                0xFFFFFFFF
/* tx_rcvd_msg_data register */
#define MT5806_TX_RCVD_MSG_HEADER_ADDR       0x0020
#define MT5806_TX_RCVD_MSG_CMD_ADDR          0x0021
#define MT5806_TX_RCVD_MSG_DATA_ADDR         0x0022
/* rcvd_msg: bit[0]:header, bit[1]:cmd, bit[2:5]:data */
#define MT5806_RCVD_MSG_DATA_LEN             4
#define MT5806_RCVD_MSG_PKT_LEN              6
#define MT5806_RCVD_PKT_BUFF_LEN             8
#define MT5806_RCVD_PKT_STR_LEN              64
/* tx_send_msg_data register */
#define MT5806_TX_SEND_MSG_HEADER_ADDR       0x0036
#define MT5806_TX_SEND_MSG_CMD_ADDR          0x0037
#define MT5806_TX_SEND_MSG_DATA_ADDR         0x0038
/* send_msg: bit[0]:header, bit[1]:cmd, bit[2:5]:data */
#define MT5806_SEND_MSG_DATA_LEN             4
#define MT5806_SEND_MSG_PKT_LEN              6
/* tx_max_fop, in kHz */
#define MT5806_TX_MAX_FOP_ADDR               0x004C
#define MT5806_TX_MAX_FOP_LEN                2
#define MT5806_TX_MAX_FOP                    145
#define MT5806_TX_FOP_STEP                   1
/* tx_min_fop, in kHz */
#define MT5806_TX_MIN_FOP_ADDR               0x004E
#define MT5806_TX_MIN_FOP_LEN                2
#define MT5806_TX_MIN_FOP                    120
/* tx_ping_freq, in kHz */
#define MT5806_TX_PING_FREQ_ADDR             0x0050
#define MT5806_TX_PING_FREQ_LEN              2
#define MT5806_TX_PING_FREQ                  130
#define MT5806_TX_PING_FREQ_MIN              100
#define MT5806_TX_PING_FREQ_MAX              150
#define MT5806_TX_PING_STEP                  1
/* tx ping ocp addr */
#define MT5806_TX_PING_OCP_TH_ADDR           0x0052
#define MT5806_TX_PING_OCP_TH_LEN            2
#define MT5806_TX_PING_OCP_TH                800
/* tx_ocp_thres register, in mA */
#define MT5806_TX_OCP_TH_ADDR                0x0054
#define MT5806_TX_OCP_TH_LEN                 2
#define MT5806_TX_OCP_TH                     2000
#define MT5806_TX_OCP_TH_STEP                1
/* tx_ovp_thres register, in mV */
#define MT5806_TX_OVP_TH_ADDR                0x0058
#define MT5806_TX_OVP_TH_LEN                 2
#define MT5806_TX_OVP_TH                     20000
#define MT5806_TX_OVP_TH_STEP                1
/* fod thd */
#define MT5806_TX_FOD_THD_ADDR               0x0060
/* tx_ping_duty_cycle register */
#define MT5806_TX_PT_DC_ADDR                 0x0063
#define MT5806_TX_HALF_BRIDGE_DC             255
#define MT5806_TX_FULL_BRIDGE_DC             150
/* tx ept_reason register */
#define MT5806_TX_EPT_REASON_ADDR            0x006A
#define MT5806_TX_EPT_REASON_LEN             2
/* tx ploss counter */
#define MT5806_TX_PLOSS_CNT_ADDR             0x006C
#define MT5806_TX_PLOSS_CNT_VAL              3
/* tx_oper_freq register, in 4Hz */
#define MT5806_TX_OP_FREQ_ADDR               0x006E
#define MT5806_TX_OP_FREQ_LEN                2
#define MT5806_TX_OP_FREQ_STEP               10
/* tx_clr_int_flag register */
#define MT5806_TX_IRQ_CLR_CTRL_ADDR          0x0070
#define MT5806_TX_IRQ_CLR_CTRL_LEN           1
#define MT5806_TX_IRQ_CLR_CTRL               1
/* pen on the pad status register  */
#define MT5806_TX_PEN_ON_THE_PAD_STA_ADDR    0x0076
#define MT5806_TX_PEN_ON_THE_PAD             0x55
#define MT5806_TX_PEN_LEAVE_PAD              0x00
/* tx_get_temp register */
#define MT5806_TX_GET_TEMP_ADDR              0x00D8
/* tx_vrect register, in mV */
#define MT5806_TX_VRECT_ADDR                 0x008A
#define MT5806_TX_VRECT_LEN                  2
/* tx_iin register, in mA */
#define MT5806_TX_IIN_ADDR                   0x008E
#define MT5806_TX_IIN_LEN                    2
/* tx_vin register, in mV */
#define MT5806_TX_VIN_ADDR                   0x0090
#define MT5806_TX_VIN_LEN                    2
/* tx_fod_rpp_thres0 register, in mW */
#define MT5806_TX_PLOSS_RPP_TH0_ADDR         0x00B6
#define MT5806_TX_PLOSS_RP_TH0_LEN           2
#define MT5806_TX_PLOSS_RP_TH0_VAL           900
/* tx_fod_th0 register */
#define MT5806_TX_PLOSS_TH0_ADDR             0x0094
#define MT5806_TX_PLOSS_TH0_LEN              2
#define MT5806_TX_PLOSS_TH0_VAL              2200
/* tx_fod_th1 register */
#define MT5806_TX_PLOSS_TH1_ADDR             0x0096
#define MT5806_TX_PLOSS_TH1_LEN              2
#define MT5806_TX_PLOSS_TH1_VAL              2800
/* tx_fod_th2 register */
#define MT5806_TX_PLOSS_TH2_ADDR             0x0098
#define MT5806_TX_PLOSS_TH2_LEN              2
#define MT5806_TX_PLOSS_TH2_VAL              3300
/* tx_cep_timeout */
#define MT5806_TX_CEP_TIMEOUT_ADDR           0x00AE
#define MT5806_TX_CEP_TIMEOUT_LEN            2
#define MT5806_TX_CEP_TIMEOUT_VAL            1200
/* tx_ping_interval, in ms */
#define MT5806_TX_PING_INTERVAL_ADDR         0x00A6
#define MT5806_TX_PING_INTERVAL_LEN          2
#define MT5806_TX_PING_INTERVAL_STEP         1
#define MT5806_TX_PING_INTERVAL_MIN          0
#define MT5806_TX_PING_INTERVAL_MAX          1000
#define MT5806_TX_PING_INTERVAL              120
/* tx_pwm_duty register */
#define MT5806_TX_PWM_DUTY_ADDR              0x00A8
#define MT5806_TX_PWM_DUTY_LEN               1
/* tx fsk depthoffset register */
#define MT5806_TX_FSK_DEPTH_ADDR             0x00A9
#define MT5806_TX_FSK_DEPTH_OFFSET           130
/* tx_ept_type register */
#define MT5806_TX_EPT_SRC_ADDR               0x00B0
#define MT5806_TX_EPT_SRC_LEN                4
#define MT5806_TX_EPT_SRC_CMD                BIT(0)
#define MT5806_TX_EPT_SRC_SS                 BIT(1)
#define MT5806_TX_EPT_SRC_ID                 BIT(2)
#define MT5806_TX_EPT_SRC_XID                BIT(3)
#define MT5806_TX_EPT_SRC_CFG_CNT            BIT(4)
#define MT5806_TX_EPT_SRC_PCH                BIT(5)
#define MT5806_TX_EPT_SRC_EPT_TIMEOUT        BIT(7)
#define MT5806_TX_EPT_SRC_CEP_TIMEOUT        BIT(8)
#define MT5806_TX_EPT_SRC_RPP_TIMEOUT        BIT(9)
#define MT5806_TX_EPT_SRC_OCP                BIT(10)
#define MT5806_TX_EPT_SRC_OVP                BIT(11)
#define MT5806_TX_EPT_SRC_LVP                BIT(12)
#define MT5806_TX_EPT_SRC_FOD                BIT(13)
#define MT5806_TX_EPT_SRC_OTP                BIT(14)
#define MT5806_TX_EPT_SRC_CFG                BIT(16)
#define MT5806_TX_EPT_SRC_PING_OVP           BIT(17)
#define MT5806_TX_EPT_SRC_PING_OCP           BIT(18)
#define MT5806_TX_EPT_SRC_PKTERR             BIT(19)
/* tx_q_cali_factory_cnt register */
#define MT5806_TX_CALI_FAC_CNT_ADDR          0x00E4
#define MT5806_TX_CALI_FAC_CNT_LEN           2
/* tx_q_cali_factory_width register */
#define MT5806_TX_CALI_FAC_WIDTH_ADDR        0x00E6
#define MT5806_TX_CALI_FAC_WIDTH_LEN         2
/* tx_q_no_rx_cnt register */
#define MT5806_TX_NO_RX_CNT_ADDR             0x00EC
#define MT5806_TX_NO_RX_CNT_LEN              2
/* tx_q_no_rx_width register */
#define MT5806_TX_NO_RX_WIDTH_ADDR           0x00EE
#define MT5806_TX_NO_RX_WIDTH_LEN            2
/* tx_q_no_rx_cnt_var register */
#define MT5806_TX_NO_RX_CNT_VAR_ADDR         0x00F0
#define MT5806_TX_NO_RX_CNT_VAR_LEN          2
/* tx_q_no_rx_width_var register */
#define MT5806_TX_NO_RX_WIDTH_VAR_ADDR       0x00F2
#define MT5806_TX_NO_RX_WIDTH_VAR_LEN        2
/* tx_q_cali_dynamic_cnt register */
#define MT5806_TX_CALI_DYN_CNT_ADDR          0x00F4
#define MT5806_TX_CALI_DYN_CNT_LEN           2
/* tx_q_cali_dynamic_width register */
#define MT5806_TX_CALI_DYN_WIDTH_ADDR        0x00F6
#define MT5806_TX_CALI_DYN_WIDTH_LEN         2

/*
 * mtp register
 */

#define MT5806_MTP_PGM_SIZE                  128
/* fw reverision */
#define MT5806_MTP_MINOR_ADDR_H              0x0003
#define MT5806_MTP_MINOR_ADDR                0x0004
#define MT5806_MTP_MAJOR_ADDR                0x0006
/* fw length register */
#define MT5806_FW_LENGTH_ADDR                0x00DA
/* fw crc16 value */
#define MT5806_FW_CRC16VALUE_ADDR            0x00DC
/* fw verify status */
#define MT5806_FW_VERIFY_STATUS_ADDR         0x00DE
#define MT5806_FW_VERIFY_STATUS_BUSY_VAL     0x0000
#define MT5806_FW_VERIFY_STATUS_FAIL_VAL     0x5555
#define MT5806_FW_VERIFY_STATUS_OK_VAL       0xAAAA
/* bootloader chipid addr */
#define MT5806_BOOTLOADER_CHIPID_ADDR        0x022C
/* bootloader start addr */
#define MT5806_BTLOADR_ADDR                  0x0800
/* bootloader ctrl */
#define MT5806_BOOT_CTRL_ADDR                0x0000
#define MT5806_BOOT_CTRL_WRITE_CMD           0x0110
#define MT5806_BOOT_CTRL_READ_CMD            0X0220
#define MT5806_BOOT_CTRL_MTP_ERASE_CMD       0x0440
#define MT5806_BOOT_CTRL_CRC_VERIFY_CMD      0x0550
#define MT5806_BOOT_CTRL_APP_ERASE_CMD       0x0880
#define MT5806_BOOT_CTRL_LIB_ERASE_CMD       0x0990
#define MT5806_BOOT_CTRL_FAC_DATA_CMD        0x0AA0
#define MT5806_BOOT_CTRL_DYN_DATA_CMD        0x0BB0
/* bootloader status addr */
#define MT5806_BOOT_STATUS_ADDR              0x0002
/* bootloader status value */
#define MT5806_BOOT_STATUS_WRITE_OK_VAL      0x0000
#define MT5806_BOOT_STATUS_WRITE_ERR_VAL     0x1111
#define MT5806_BOOT_STATUS_READ_OK_VAL       0x0000
#define MT5806_BOOT_STATUS_READ_ERR_VAL      0x2222
#define MT5806_BOOT_STATUS_ERASE_OK_VAL      0x0000
#define MT5806_BOOT_STATUS_ERASE_ERR_VAL     0x3333
#define MT5806_BOOT_STATUS_DATA_OK_VAL       0x0000
#define MT5806_BOOT_STATUS_DATA_ERR_VAL      0x4444
#define MT5806_BOOT_STATUS_CRC_OK_VAL        0x0000
#define MT5806_BOOT_STATUS_CRC_ERR_VAL       0x5555
#define MT5806_BOOT_STATUS_IDLE_VAL          0x0000
#define MT5806_BOOT_STATUS_BUSY_VAL          0xAAAA
/* bootloader pgm addr addr */
#define MT5806_BOOT_PGM_ADDR_ADDR            0x0004
/* bootloader pgm addr start value */
#define MT5806_BOOT_PGM_ADDR_START_VAL       0
/* bootloader pgm length addr */
#define MT5806_BOOT_PGM_LEN_ADDR             0x0008
/* bootloader pgm crc16 addr */
#define MT5806_BOOT_PGM_VERIFY_ADDR          0x000C
/* bootloader chip crc16 addr */
#define MT5806_BOOT_PGM_VERIFY_CHIP_ADDR     0x0010
/* bootloader pgm buffer addr */
#define MT5806_BOOT_PGM_BUFFER_ADDR          0x0014
/* load app start addr */
#define MT5806_BOOT_APP_ADDR                 0x0000
#define MT5806_BOOT_APP_ERASE_TIME_MS        0
/* load lib start addr */
#define MT5806_BOOT_LIB_ADDR                 0x3000
#define MT5806_BOOT_LIB_ERASE_TIME_MS        0
/* load q_val&F_val addr */
#define MT5806_BOOT_QF_BUFFER0_ADDR          0x7E00 /* dynamic calibrate */
#define MT5806_BOOT_QF_BUFFER1_ADDR          0x7F00 /* factory calibrate */
#define MT5806_BOOT_FACTOR_ERASE_TIME_MS     70
#define MT5806_BOOT_FACTOR_DATA_LEN          4
/* cortex M0 core */
#define MT5806_PMU_WDGEN_ADDR                0x1408
#define MT5806_WDG_DISABLE                   0x95
#define MT5806_WDG_ENABLE                    0x59
#define MT5806_PMU_FLAG_ADDR                 0x1400
#define MT5806_WDT_INTFALG                   0x01
#define MT5806_SYS_KEY_ADDR                  0x1244
#define MT5806_KEY_VAL                       0x57
#define MT5806_CODE_REMAP_ADDR               0x1208
#define MT5806_CODE_REMAP_VAL                0x11
#define MT5806_M0_CTRL_ADDR                  0x1200
#define MT5806_M0_HOLD_VAL                   0x20
#define MT5806_M0_RST_VAL                    0x80
#define MT5806_M0_CLK_ADDR                   0x1608
#define MT5806_M0_CLK_DISABLE_VAL            0x0F
#define MT5806_M0_PWM_ADDR                   0x4064
#define MT5806_M0_PWM_DISABLE_VAL            0x00
#define MT5806_M0_DMA_ADDR                   0x4200
#define MT5806_M0_DMA_DISABLE_VAL            0x00

/* pwm clk source 192M */
#define OSCCLK                               192000
#define MT5806_SHIFT_VAL                     2

#endif /* _MT5806_CHIP_H_ */
