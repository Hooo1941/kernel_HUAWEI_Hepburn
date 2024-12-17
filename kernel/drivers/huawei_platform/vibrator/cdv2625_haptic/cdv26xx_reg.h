/*
 * cdv26xx_reg.h
 *
 * code for vibrator
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef _CDV26XX_REG_H_
#define _CDV26XX_REG_H_

/* CDV2625_Registers_List_Deinition */
#define CDV2625_REG_CHIPID      0x00        /* Chip ID register. */
#define CDV2625_SYS_STATUS      0x01        /* System status register. */
#define CDV2625_SYS_INTR        0x02        /* System interrupt status register. */
#define CDV2625_SYS_INTM        0x03        /* System interrupt mask register. */
#define CDV2625_BST_CFG1        0x04        /* Boost configuration 1 register. */
#define CDV2625_BST_CFG2        0x05        /* Boost configuration 2 register. */
#define CDV2625_BST_CFG3        0x06        /* Boost configuration 3 register. */
#define CDV2625_GAIN            0x07        /* Data gain control register. */
#define CDV2625_SYS_CTRL        0x08        /* System control register. */
#define CDV2625_GO              0x09        /* Procedure control register. */
#define CDV2625_WAV_IDX1        0x0A        /* Wave index 1 register. */
#define CDV2625_WAV_IDX2        0x0B        /* Wave index 2 register. */
#define CDV2625_WAV_IDX3        0x0C        /* Wave index 3 register. */
#define CDV2625_WAV_IDX4        0x0D        /* Wave index 4 register. */
#define CDV2625_WAV_IDX5        0x0E        /* Wave index 5 register. */
#define CDV2625_WAV_IDX6        0x0F        /* Wave index 6 register. */
#define CDV2625_WAV_IDX7        0x10        /* Wave index 7 register. */
#define CDV2625_WAV_IDX8        0x11        /* Wave index 8 register. */
#define CDV2625_WAVLOOP1        0x12        /* Wave loop 1 register. */
#define CDV2625_WAVLOOP2        0x13        /* Wave loop 2 register. */
#define CDV2625_WAVLOOP3        0x14        /* Wave loop 3 register. */
#define CDV2625_WAVLOOP4        0x15        /* Wave loop 4 register. */
#define CDV2625_MAINLOOP        0x16        /* Main loop register. */
#define CDV2625_SRC_CTRL        0x18        /* PWM edge control register. */
#define CDV2625_CONT_CTRL3      0x1B        /* CONT mode control 3 register. */
#define CDV2625_CONT_CTRL4      0x1c        /* break_gain */
#define CDV2625_CONT_CTRL5      0x1D        /* Drive level control register. */
#define CDV2625_CONT_CTRL6      0x1E        /* Over drive level control register. */
#define CDV2625_CONT_CTRL7      0x1F        /* F0 trace configuration 1 register. */
#define CDV2625_CONT_CTRL8      0x20        /* F0 trace configuration 2 register. */
#define CDV2625_CONT_CTRL9      0x21        /* F0 trace configuration 3 register. */
#define CDV2625_CONT_CTRL10     0x22
#define CDV2625_CONT_CTRL11     0x23        /* Perset F0 value high 8 bits register. */
#define CDV2625_CONT_CTRL12     0x24        /* Perset F0 value low 8 bits register. */
#define CDV2625_CONT_CTRL13     0x25        /* F0 measurement value high 8 bits register. */
#define CDV2625_CONT_CTRL14     0x26        /* F0 measurement value low 8 bits register. */
#define CDV2625_CONT_CTRL15     0x27        /* F0 continuously drives the high 8 bits of the measured value. */
#define CDV2625_CONT_CTRL16     0x28        /* F0 continuously drives the low 8 bits of the measured value. */
#define CDV2625_CONT_CTRL17     0x29
#define CDV2625_CONT_CTRL18     0x2a
#define CDV2625_CONT_CTRL19     0x2b
#define CDV2625_CONT_CTRL20     0x2C        /* Drives time set register. */
#define CDV2625_RTP_CTRL1       0x2D        /* SRAM base address high 5 bits register. */
#define CDV2625_RTP_CTRL2       0x2E        /* SRAM base address low 8 bits register. */
#define CDV2625_RTP_CTRL3       0x2F        /* FIFO almost empty/full threshold high 4 bits register. */
#define CDV2625_RTP_CTRL4       0x30        /* FIFO almost empty threshold low 8 bits register. */
#define CDV2625_RTP_CTRL5       0x31        /* FIFO almost full threshold low 8 bits register. */
#define CDV2625_RTP_CTRL6       0x32        /* RTP mode data register. */
#define CDV2625_TRG_CTRL1       0x33        /* Trigger 1 rasing edge waveform control register. */
#define CDV2625_TRG_CTRL2       0x34        /* Trigger 2 rasing edge waveform control register. */
#define CDV2625_TRG_CTRL3       0x35        /* Trigger 3 rasing edge waveform control register. */
#define CDV2625_TRG_CTRL4       0x36        /* Trigger 1 falling edge waveform control register. */
#define CDV2625_TRG_CTRL5       0x37        /* Trigger 2 falling edge waveform control register. */
#define CDV2625_TRG_CTRL6       0x38        /* Trigger 3 falling edge waveform control register. */
#define CDV2625_TRG_CTRL7       0x39        /* Trigger pin configuration 1register. */
#define CDV2625_TRG_CTRL8       0x3A        /* Trigger pin configuration 2 register. */
#define CDV2625_TRG_CTRL9       0x3B        /* Trigger priority configuration register. */
#define CDV2625_TRG_CTRL10      0x3C        /* Trigger pin configuration 3 register. */
#define CDV2625_PAD_CTRL        0x3D        /* PAD configuration register. */
#define CDV2625_GLB_STATE       0x3F        /* Global status register. */
#define CDV2625_WAVE_DATA       0x40        /* Wave data register. */
#define CDV2625_WAVE_ADDRH      0x41        /* Wave address high 6 bits register. */
#define CDV2625_WAVE_ADDRL      0x42        /* Wave address low 8 bits register. */
#define CDV2625_DATA_CTRL       0x45        /* Global control data register. */
#define CDV2625_DBG_STAT        0x46        /* Debug status register. */
#define CDV2625_DPWM_CTRL1      0x47        /* PWM input DC detection control register. */
#define CDV2625_DPWM_CTRL2      0x48        /* PWM debug control register. */
#define CDV2625_DPWM_CTRL3      0x49        /* Waveform protection voltage configuration control register. */
#define CDV2625_DPWM_CTRL4      0x4A        /* Waveform guard period configuration control register. */
#define CDV2625_VDD_CTRL        0x4C        /* Gain adjustment control register control register. */
#define CDV2625_DET_CTRL1       0x4D        /* Detection control 1 register. */
#define CDV2625_DET_CTRL2       0x4E        /* Detection control 2 register. */
#define CDV2625_DET_CTRL3       0x4F        /* Detection control 3 register. */
#define CDV2625_DET_CTRL4       0x50        /* Detection control 4 register. */
#define CDV2625_DET_CTRL5       0x51        /* Detection control 5 register. */
#define CDV2625_ANA_CTRL0       0x5d
#define CDV2625_ANA_CTRL        0x5f        /* Configuration of analog parameters */
#define CDV2625_TRIM_OSC_LRA1   0x68        /* LRA calibration 1 register. */
#define CDV2625_TRIM_OSC_LRA2   0x69        /* LRA calibration 2 register. */
#define CDV2625_TRIM_EN         0x6A        /* TRIM enable pattern register. */
#define CDV2625_TRIM_OSC        0x6F
#define CDV2625_REG_QA          0x7c
#define CDV2625_CONT_CTRL       0x62

typedef enum {
	CS_RESET = 0x00,            /* A state is not triggered. */
	CS_SET   = !CS_RESET        /* A state is triggered. */
}cs_bit_status_t;

typedef enum {
	CS_DISABLE = 0x00,              /* Disable a function. */
	CS_ENABLE  = !CS_DISABLE,       /* Enable a function. */
}cs_enable_state_t;

/* CHIPID register  Bits definition */
#define CDV2625_BIT_CHIPID                      (0xFF)

/* SYS_STATUS register  Bits definition */
#define CDV2625_BIT_SYS_STS_BST_ERRS            (0x80)
#define CDV2625_BIT_SYS_STS_DPWMS               (0x40)
#define CDV2625_BIT_SYS_STS_UVLOS               (0x20)
#define CDV2625_BIT_SYS_STS_FIFO_AES            (0x10)
#define CDV2625_BIT_SYS_STS_FIFO_AFS            (0x08)
#define CDV2625_BIT_SYS_STS_OCS                 (0x04)
#define CDV2625_BIT_SYS_STS_OTS                 (0x02)
#define CDV2625_BIT_SYS_STS_DONES               (0x01)

/* SYS_INT register  Bits definition */
#define CDV2625_BIT_SYSINT_BST_ERRI             (0x80)
#define CDV2625_BIT_SYSINT_DPWMI                (0x40)
#define CDV2625_BIT_SYSINT_UVLOI                (0x20)
#define CDV2625_BIT_SYSINT_FF_AEI               (0x10)
#define CDV2625_BIT_SYSINT_FF_AFI               (0x08)
#define CDV2625_BIT_SYSINT_OCI                  (0x04)
#define CDV2625_BIT_SYSINT_OTI                  (0x02)
#define CDV2625_BIT_SYSINT_DONEI                (0x01)

/* @brief SYS_INTM register  Bits definition */
#define CDV2625_BIT_SYSINTM_BST_ERRM            (0x80)
#define CDV2625_BIT_SYSINTM_DPWMM               (0x40)
#define CDV2625_BIT_SYSINTM_UVLOM               (0x20)
#define CDV2625_BIT_SYSINTM_FF_AEM              (0x10)
#define CDV2625_BIT_SYSINTM_FF_AFM              (0x08)
#define CDV2625_BIT_SYSINTM_OCM                 (0x04)
#define CDV2625_BIT_SYSINTM_OTM                 (0x02)
#define CDV2625_BIT_SYSINTM_DONEM               (0x01)

/* @brief BST_CFG1 register  Bits definition */
#define CDV2625_BIT_BST_CFG1_AUTO_BST           (0xE0)
#define CDV2625_BIT_BST_CFG1_RTP_LP_TIME_TH     (0x18)
#define CDV2625_BIT_BST_CFG1_RTP_AUTO_LP        (0x04)
#define CDV2625_BIT_BST_CFG1_BST_BYPASS         (0x02)
#define CDV2625_BIT_BST_CFG1_BOOST_EN           (0x01)

/* @brief BST_CFG2 register  Bits definition */
#define CDV2625_BIT_BST_CFG2_BST_PRE_EN_TH      (0xC0)
#define CDV2625_BIT_BST_CFG2_H_SLEW_RATE        (0x38)
#define CDV2625_BIT_BST_CFG2_BST_MAX_PEAK_CUR   (0x07)

/* @brief BST_CFG3 register  Bits definition */
#define CDV2625_BIT_BST_CFG3_BYPASS_SLEW_EN     (0x80)
#define CDV2625_BIT_BST_CFG3_BST_VOLT           (0x7F)

/* @brief GAIN register  Bits definition */
#define CDV2625_BIT_GAIN_GAIN                   (0xFF)

/* @brief SYS_CTRL register  Bits definition */
#define CDV2625_BIT_SYS_CTRL_WAVE_UP_RATE       (0xC0)
#define CDV2625_BIT_SYS_CTRL_FIFO_RESET         (0x20)
#define CDV2625_BIT_SYS_CTRL_STOP_MODE          (0x10)
#define CDV2625_BIT_SYS_CTRL_STANDBY            (0x08)
#define CDV2625_BIT_SYS_CTRL_EXTRA_BRK          (0x04)
#define CDV2625_BIT_SYS_CTRL_PLAY_MODE          (0x03)

/* @brief GO register  Bits definition */
#define CDV2625_BIT_GO_GO                       (0x01)

/* @brief WAV_IDX1 register  Bits definition */
#define CDV2625_BIT_WAVIDX1_WAIT1               (0x80)
#define CDV2625_BIT_WAVIDX1_WAVE_IDX1           (0x7F)

/* @brief WAV_IDX2 register  Bits definition */
#define CDV2625_BIT_WAVIDX2_WAIT2               (0x80)
#define CDV2625_BIT_WAVIDX2_WAVE_IDX2           (0x7F)

/* @brief WAV_IDX3 register  Bits definition */
#define CDV2625_BIT_WAVIDX3_WAIT3               (0x80)
#define CDV2625_BIT_WAVIDX3_WAVE_IDX3           (0x7F)

/* @brief WAV_IDX4 register  Bits definition */
#define CDV2625_BIT_WAVIDX4_WAIT4               (0x80)
#define CDV2625_BIT_WAVIDX4_WAVE_IDX4           (0x7F)

/* @brief WAV_IDX5 register  Bits definition */
#define CDV2625_BIT_WAVIDX5_WAIT5               (0x80)
#define CDV2625_BIT_WAVIDX5_WAVE_IDX5           (0x7F)

/* @brief WAV_IDX6 register  Bits definition */
#define CDV2625_BIT_WAVIDX6_WAIT6               (0x80)
#define CDV2625_BIT_WAVIDX6_WAVE_IDX6           (0x7F)

/* @brief WAV_IDX7 register  Bits definition */
#define CDV2625_BIT_WAVIDX7_WAIT7               (0x80)
#define CDV2625_BIT_WAVIDX7_WAVE_IDX7           (0x7F)

/* @brief WAV_IDX8 register  Bits definition */
#define CDV2625_BIT_WAVIDX8_WAIT8               (0x80)
#define CDV2625_BIT_WAVIDX8_WAVE_IDX8           (0x7F)

/* @brief WAVLOOP1 register  Bits definition */
#define CDV2625_BIT_WAVLOOP1_IDX1_LOOP          (0xF0)
#define CDV2625_BIT_WAVLOOP1_IDX2_LOOP          (0x0F)

/* @brief WAVLOOP2 register  Bits definition */
#define CDV2625_BIT_WAVLOOP2_IDX3_LOOP          (0xF0)
#define CDV2625_BIT_WAVLOOP2_IDX4_LOOP          (0x0F)

/* @brief WAVLOOP3 register  Bits definition */
#define CDV2625_BIT_WAVLOOP3_IDX5_LOOP          (0xF0)
#define CDV2625_BIT_WAVLOOP3_IDX6_LOOP          (0x0F)

/* @brief WAVLOOP4 register  Bits definition */
#define CDV2625_BIT_WAVLOOP4_IDX7_LOOP          (0xF0)
#define CDV2625_BIT_WAVLOOP4_IDX8_LOOP          (0x0F)

/* @brief MAINLOOP register  Bits definition */
#define CDV2625_BIT_MAINLOOP_STDBY_TIME_TH      (0xC0)
#define CDV2625_BIT_MAINLOOP_WAIT_UNIT          (0x30)
#define CDV2625_BIT_MAINLOOP_MAINLOOP           (0x0F)

/* @brief SRC_CTRL register  Bits definition */
#define CDV2625_BIT_SRC_SRC_WAVE_LEN            (0xF0)
#define CDV2625_BIT_SRC_CTRL_FC_SEL             (0x0C)
#define CDV2625_BIT_SRC_CTRL_SRC_MODE           (0x02)
#define CDV2625_BIT_SRC_CTRL_SRC_EN             (0x01)

/* @brief CONT_CTRL3 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL3_EN_O2C           (0x80)
#define CDV2625_BIT_CONT_CTRL3_EN_CLOSE         (0x40)
#define CDV2625_BIT_CONT_CTRL3_CONT_END_M       (0x20)
#define CDV2625_BIT_CONT_CTRL3_F0_DET_MODE      (0x10)
#define CDV2625_BIT_CONT_CTRL3_BRK_CYCLE_HALF   (0x0F)

/* @brief CONT_CTRL5 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL5_DRV_CLAMP        (0x7F)

/* @brief CONT_CTRL6 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL6_OD_CLAMP         (0x7F)

/* @brief CONT_CTRL7 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL7_F0_OPEN_CYCLE    (0xF0)
#define CDV2625_BIT_CONT_CTRL7_F0_HD            (0x08)
#define CDV2625_BIT_CONT_CTRL7_F0_DET_TIMES     (0x07)

/* @brief CONT_CTRL8 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL8_BEMF_DET_CYC_HF  (0x30)
#define CDV2625_BIT_CONT_CTRL8_F0_FREE_CYC_HF   (0x0F)

/* @brief CONT_CTRL9 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL9_CONT_ERR         (0x78)
#define CDV2625_BIT_CONT_CTRL9_F0_CLS_CYC       (0x07)

/* @brief CONT_CTRL11 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL11_F0_PRE_H        (0x3F)

/* @brief CONT_CTRL12 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL12_F0_PRE_L        (0xFF)

/* @brief CONT_CTRL13 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL13_F0_FREE_H       (0xFF)

/* @brief CONT_CTRL14 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL14_F0_FREE_L       (0xFF)

/* @brief CONT_CTRL15 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL15_F0_DRIVE_H      (0xFF)

/* @brief CONT_CTRL16 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL16_F0_DRIVE_L      (0xFF)

/* @brief CONT_CTRL20 register  Bits definition */
#define CDV2625_BIT_CONT_CTRL20_CONT_TIME       (0xFF)

/* @brief RTP_CTRL1 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL1_BASE_ADDRH        (0x1F)

/* @brief RTP_CTRL2 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL2_BASE_ADDRL        (0xFF)

/* @brief RTP_CTRL3 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL3_FIFO_AEH          (0xF0)
#define CDV2625_BIT_RTP_CTRL3_FIFO_AFH          (0x0F)

/* @brief RTP_CTRL4 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL4_FIFO_AEL          (0xFF)

/* @brief RTP_CTRL5 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL5_FIFO_AFL          (0xFF)

/* @brief RTP_CTRL6 register  Bits definition */
#define CDV2625_BIT_RTP_CTRL6_RTP_DATA          (0xFF)

/* @brief TRG_CTRL1 register  Bits definition */
#define CDV2625_TRG_CTRL1_TRG1_PE               (0x80)
#define CDV2625_TRG_CTRL1_TRG1_IDX_P            (0x7F)

/* TRG_CTRL2: reg0x34 */
/* @brief CHIPID register  Bits definition */
#define CDV2625_TRG_CTRL2_TRG2_PE               (0x80)
#define CDV2625_TRG_CTRL2_TRG2_IDX_P            (0x7F)

/* @brief TRG_CTRL3 register  Bits definition */
#define CDV2625_TRG_CTRL3_TRG3_PE               (0x80)
#define CDV2625_TRG_CTRL3_TRG3_IDX_P            (0x7F)

/* @brief TRG_CTRL4 register  Bits definition */
#define CDV2625_TRG_CTRL4_TRG1_NE               (0x80)
#define CDV2625_TRG_CTRL4_TRG1_IDX_N            (0x7F)

/* @brief TRG_CTRL5 register  Bits definition */
#define CDV2625_TRG_CTRL5_TRG2_NE               (0x80)
#define CDV2625_TRG_CTRL5_TRG2_IDX_N            (0x7F)

/* @brief TRG_CTRL6 register  Bits definition */
#define CDV2625_TRG_CTRL6_TRG3_NE               (0x80)
#define CDV2625_TRG_CTRL6_TRG3_IDX_N            (0x7F)

/* @brief TRG_CTRL7 register  Bits definition */
#define CDV2625_TRG_CTRL7_TRG1_LVL              (0x80)
#define CDV2625_TRG_CTRL7_TRG1_LVL_EN           (0x40)
#define CDV2625_TRG_CTRL7_TRG1_BRK              (0x20)
#define CDV2625_TRG_CTRL7_TRG1_BST              (0x10)
#define CDV2625_TRG_CTRL7_TRG2_LVL              (0x08)
#define CDV2625_TRG_CTRL7_TRG2_LVL_EN           (0x04)
#define CDV2625_TRG_CTRL7_TRG2_BRK              (0x02)
#define CDV2625_TRG_CTRL7_TRG2_BST              (0x01)

/* @brief TRG_CTRL8 register  Bits definition */
#define CDV2625_TRG_CTRL8_TRG3_LVL              (0x80)
#define CDV2625_TRG_CTRL8_TRG3_LVL_EN           (0x40)
#define CDV2625_TRG_CTRL8_TRG3_BRK              (0x20)
#define CDV2625_TRG_CTRL8_TRG3_BST              (0x10)
#define CDV2625_TRG_CTRL8_TRG1_ONEWIRE          (0x08)
#define CDV2625_TRG_CTRL8_TRG3_STOP             (0x04)
#define CDV2625_TRG_CTRL8_TRG2_STOP             (0x02)
#define CDV2625_TRG_CTRL8_TRG1_STOP             (0x01)

/* @brief TRG_CTRL9 register  Bits definition */
#define CDV2625_TRG_CTRL9_GO_PRIO               (0xC0)
#define CDV2625_TRG_CTRL9_TRG3_PRIO             (0x30)
#define CDV2625_TRG_CTRL9_TRG2_PRIO             (0x0C)
#define CDV2625_TRG_CTRL9_TRG1_PRIO             (0x03)

/* @brief TRG_CTRL10 register  Bits definition */
#define CDV2625_TRG_CTRL10_TRG1_MODE            (0x40)
#define CDV2625_TRG_CTRL10_TRG3_PU              (0x20)
#define CDV2625_TRG_CTRL10_TRG2_PU              (0x10)
#define CDV2625_TRG_CTRL10_TRG1_PU              (0x08)
#define CDV2625_TRG_CTRL10_TRG3_PD              (0x04)
#define CDV2625_TRG_CTRL10_TRG2_PD              (0x02)
#define CDV2625_TRG_CTRL10_TRG1_PD              (0x01)

/* @brief PAD_CTRL register  Bits definition */
#define CDV2625_PAD_CTRL_I2C_LVL                (0xC0)
#define CDV2625_PAD_CTRL_PAD_DEGLITCH           (0x3F)

/* @brief GLB_STATE register  Bits definition */
#define CDV2625_GLB_STATE_PLAY_SW_MODE          (0x30)
#define CDV2625_GLB_STATE_GLB_STATE             (0x0F)

/* @brief WAVE_DATA register  Bits definition */
#define CDV2625_WAVE_DATA_WAVE_DATA             (0xFF)

/* @brief WAVE_ADDRH register  Bits definition */
#define CDV2625_WAVE_ADDRH_WAVE_ADDRH           (0x3F)

/* @brief WAVE_ADDRL register  Bits definition */
#define CDV2625_WAVE_ADDRL_WAVE_ADDRL           (0xFF)

/* @brief DATA_CTRL register  Bits definition */
#define CDV2625_DATA_CTRL_INT_OD                (0x20)
#define CDV2625_DATA_CTRL_INT_MODE              (0x1C)
#define CDV2625_DATA_CTRL_GAIN_SW_M             (0x01)

/* @brief DBG_STAT register  Bits definition */
#define CDV2625_DBG_STAT_BST_SCP                (0x40)
#define CDV2625_DBG_STAT_BST_OVP                (0x20)
#define CDV2625_DBG_STAT_FF_ERROR               (0x04)
#define CDV2625_DBG_STAT_FF_FULL                (0x02)
#define CDV2625_DBG_STAT_FF_EMPTY               (0x01)

/* @brief DPWM_CTRL1 register  Bits definition */
#define CDV2625_DPWM_CTRL1_DDEP_TH              (0xFE)
#define CDV2625_DPWM_CTRL1_DDEP_EN              (0x01)

/* @brief DPWM_CTRL2 register  Bits definition */
#define CDV2625_DPWM_CTRL2_PWM_WAVE             (0x80)
#define CDV2625_DPWM_CTRL2_PWMCLK_MODE          (0x60)
#define CDV2625_DPWM_CTRL2_PWM_EN               (0x10)
#define CDV2625_DPWM_CTRL2_PWM_FRC              (0x0E)
#define CDV2625_DPWM_CTRL2_PWM_OE               (0x01)

/* @brief DPWM_CTRL3 register  Bits definition */
#define CDV2625_DPWM_CTRL3_PWMI_PR_EN           (0x80)
#define CDV2625_DPWM_CTRL3_PWMI_PRLVL           (0x7F)

/* @brief DPWM_CTRL4 register  Bits definition */
#define CDV2625_DPWM_CTRL4_PWMI_PRTIME          (0xFF)

/* @brief VDD_CTRL register  Bits definition */
#define CDV2625_VDD_CTRL_VDD_AJ_MODE            (0x40)
#define CDV2625_VDD_CTRL_VDD_DET_M              (0x20)

/* @brief DET_CTRL1 register  Bits definition */
#define CDV2625_DET_CTRL1_DET_MODE              (0xF0)
#define CDV2625_DET_CTRL1_CLK_ADC_SEL           (0x0E)
#define CDV2625_DET_CTRL1_DET_GO                (0x01)

/* @brief DET_CTRL2 register  Bits definition */
#define CDV2625_DET_CTRL2_ALG_SEL               (0x20)
#define CDV2625_DET_CTRL2_ADC_FLT_COE           (0x18)
#define CDV2625_DET_CTRL2_PGA_GAIN              (0x07)

/* @brief DET_CTRL3 register  Bits definition */
#define CDV2625_DET_CTRL3_ADC_AVERAGE_H         (0xF0)
#define CDV2625_DET_CTRL3_ADC_DATA_H            (0x0F)

/* @brief DET_CTRL4 register  Bits definition */
#define CDV2625_DET_CTRL4_ADC_AVERAGE_L         (0xFF)

/* @brief DET_CTRL4 register  Bits definition */
#define CDV2625_DET_CTRL5_ADC_DATA_L            (0xFF)

/* @brief TRIM_OSC_LRA1 register  Bits definition */
#define CDV2625_TRIM_OSC_LRA1_RAMP              (0xC0)
#define CDV2625_TRIM_OSC_TRIM_OSC_TEMP          (0x38)
#define CDV2625_TRIM_OSC_TRIM_OSC_LRA_H         (0x07)

/* @brief TRIM_OSC_LRA2 register  Bits definition */
#define CDV2625_TRIM_OSC_LRA2_TRIM_OSC_LRA_L    (0xFF)

/* @brief TRIM_EN register  Bits definition */
#define CDV2625_TRIM_EN_PATTEN                  (0xFF)

/***************************************************************************************************
 * @ register desc
***************************************************************************************************/
/* @brief RC filter limit frequency selection. */
typedef enum {
	RC_FLT_LMT_FRQ_400  = 0x00, /* RC filter limit frequency is 400Hz. */
	RC_FLT_LMT_FRQ_600  = 0x04, /* RC filter limit frequency is 600Hz. */
	RC_FLT_LMT_FRQ_800  = 0x08, /* RC filter limit frequency is 800Hz. */
	RC_FLT_LMT_FRQ_1000 = 0x0C  /* RC filter limit frequency is 1000Hz. */
}rc_lim_freq_t;

/*! Checks RC filter limit frequency selection. */
#define RC_FLT_LMT_CHECK(frq)       (((frq) == RC_FLT_LMT_FRQ_400) || ((frq) == RC_FLT_LMT_FRQ_600) || \
									((frq) == RC_FLT_LMT_FRQ_800) || ((frq) == RC_FLT_LMT_FRQ_1000))

/* @brief PWM edge control mode selection. */
typedef enum {
	PWM_EDGE_MODE_SRAM    = 0x00, /* SRAM control mode. */
	PWM_EDGE_MODE_RC_FLT  = 0x02 /* RC filter control mode. */
}dege_mode_t;

/*! Checks PWM edge control mode selection. */
#define PWM_EDGE_MODE_CHECK(mode)       (((mode) == PWM_EDGE_MODE_SRAM) || ((mode) == PWM_EDGE_MODE_RC_FLT))

/* @brief PWM sample rate mode selection */
typedef enum {
	PWM_SAMPLE_48K = 0x00, /* PWM sample rate is 48K. */
	PWM_SAMPLE_24K = 0x02, /* PWM sample rate is 24K. */
	PWM_SAMPLE_12K = 0x03 /* PWM sample rate is 12K. */
}samp_rate_mode_t;

/* Checks PWM sample rate mode selection */
#define PWM_RATE_MODE_CHECK(sp)     (((sp) == PWM_SAMPLE_48K) || ((sp) == PWM_SAMPLE_24K) || ((sp) == PWM_SAMPLE_12K))

/* @brief PWM edge structure definition.   */
typedef struct {
	cs_enable_state_t      pwm_edge_en;          /* PWM edge control mode enable control. */
	uint8_t             wave_sample_num;        /* Number of edge waveform samples<value:0-0x0F */
	rc_lim_freq_t       freq_selection;         /* RC filter limit frequency selection. value from enum rc_lim_freq_t */
	dege_mode_t         mode_selection;         /* PWM edge control mode selection. value from enum dege_mode_t */
}edge_pwm_config_t;

/* @brief RAM wave library configuration structure definition.   */
typedef struct {
	uint16_t        base_addr;          /* RAM wave base address. */
	uint8_t         id;                 /* Wave ID. */
	uint8_t         version;            /* Wave library version. */
	uint16_t        start_addr;         /* Wave head start address. */
	uint16_t        end_addr;           /* Wave head end address. */
	uint8_t*        data;               /* Wave data. */
}wave_ram_data_t;

/* chip_flag */
#define CDV_FLAG_BST_ERRS       CDV2625_BIT_SYS_STS_BST_ERRS
#define CDV_FLAG_DPWMS          CDV2625_BIT_SYS_STS_DPWMS
#define CDV_FLAG_UVLOS          CDV2625_BIT_SYS_STS_UVLOS           /* Undervoltage lockout output signal. */
#define CDV_FLAG_FF_AES         CDV2625_BIT_SYS_STS_FIFO_AES        /* RTP FIFO almost empty. */
#define CDV_FLAG_FF_AFS         CDV2625_BIT_SYS_STS_FIFO_AFS        /* RTP FIFO almost full. */
#define CDV_FLAG_OCS            CDV2625_BIT_SYS_STS_OCS             /* Over-cur state. */
#define CDV_FLAG_OTS            CDV2625_BIT_SYS_STS_OTS             /* Over-temperature state. */
#define CDV_FLAG_DONES          CDV2625_BIT_SYS_STS_DONES           /* Play complete flag. */

/*! Checks Chip flag status. */
#define CDV_FLAG_CHECK(flag)        (((flag) == CDV_FLAG_BST_ERRS) || ((flag) == CDV_FLAG_DPWMS)  || \
									 ((flag) == CDV_FLAG_UVLOS)    || ((flag) == CDV_FLAG_FF_AES) || \
									 ((flag) == CDV_FLAG_FF_AFS)   || ((flag) == CDV_FLAG_OCS)    || \
									 ((flag) == CDV_FLAG_OTS)      || ((flag) == CDV_FLAG_DONES))

/* chip_interrupt */
/* Boost short circuit protection/overvoltage protection error interrupt. */
#define CDV_INTR_BST_ERRS       CDV2625_BIT_SYSINT_BST_ERRI
/* DPWM waveform data overflow protection or DPWM DC error interrupt. */
#define CDV_INTR_DPWMS          CDV2625_BIT_SYSINT_DPWMI
#define CDV_INTR_UVLOS          CDV2625_BIT_SYSINT_UVLOI        /* Undervoltage lockout output signal interrupt. */
#define CDV_INTR_FF_AES         CDV2625_BIT_SYSINT_FF_AEI       /* RTP FIFO almost empty interrupt. */
#define CDV_INTR_FF_AFS         CDV2625_BIT_SYSINT_FF_AFI       /* RTP FIFO almost full interrupt. */
#define CDV_INTR_OCS            CDV2625_BIT_SYSINT_OCI          /* Over-cur state interrupt. */
#define CDV_INTR_OTS            CDV2625_BIT_SYSINT_OTI          /* Over-temperature state interrupt. */
#define CDV_INTR_DONES          CDV2625_BIT_SYSINT_DONEI        /* Play complete interrupt. */

/*! Checks Chip interrupt. */
#define CDV_INTR_CHECK(intr)        (((intr) == CDV_INTR_BST_ERRS) || ((intr) == CDV_INTR_DPWMS)  || \
									 ((intr) == CDV_INTR_UVLOS)    || ((intr) == CDV_INTR_FF_AES) || \
									 ((intr) == CDV_INTR_FF_AFS)   || ((intr) == CDV_INTR_OCS)    || \
									 ((intr) == CDV_INTR_OTS)      || ((intr) == CDV_INTR_DONES))

/* @defgroup mode_auto_bst */
#define CDV_MODE_RAM            ((uint8_t)0x80)     /* RAM mode. */
#define CDV_MODE_RTP            ((uint8_t)0x40)     /* RTP mode. */
#define CDV_MODE_ONEWIRE        ((uint8_t)0x20)     /* One wire mode. */

/*! Checks mode related to auto boost enable. */
#define CDV_MODE_BST_EN_CHECK(mode)     (((mode)&0x1f) == 0)

/* @defgroup rtp_thr */
#define RTP_LP_TIME_TH_12CLK        ((uint8_t)0x00)     /* Time threshold is 12 12K-clock period. */
#define RTP_LP_TIME_TH_48CLK        ((uint8_t)0x08)     /* Time threshold is 48 12K-clock period. */
#define RTP_LP_TIME_TH_192CLK       ((uint8_t)0x10)     /* Time threshold is 192 12K-clock period. */
#define RTP_LP_TIME_TH_768CLK       ((uint8_t)0x18)     /* Time threshold is 768 12K-clock period. */

/*! Checks Time threshold. */
#define CDV_RTP_LP_TIME_TH_CHECK(time)      (((time) == RTP_LP_TIME_TH_12CLK) || ((time) == RTP_LP_TIME_TH_192CLK) || \
											 ((time) == RTP_LP_TIME_TH_48CLK) || ((time) == RTP_LP_TIME_TH_768CLK))

/* @defgroup bst_pre_thr */
#define BST_PRE_TIME_TH_6CLK        ((uint8_t)0x00)     /* Time threshold is 6 12K-clock period. */
#define BST_PRE_TIME_TH_8CLK        ((uint8_t)0x40)     /* Time threshold is 8 12K-clock period. */
#define BST_PRE_TIME_TH_12CLK       ((uint8_t)0x80)     /* Time threshold is 12 12K-clock period. */
#define BST_PRE_TIME_TH_16CLK       ((uint8_t)0xC0)     /* Time threshold is 16 12K-clock period. */

/*! Checks Time threshold. */
#define CDV_BST_PRE_TIME_TH_CHECK(time)     (((time) == BST_PRE_TIME_TH_6CLK)  || ((time) == BST_PRE_TIME_TH_8CLK) || \
											 ((time) == BST_PRE_TIME_TH_12CLK) || ((time) == BST_PRE_TIME_TH_16CLK))

/* @defgroup slew_rate_cfg */
#define BST_SLEW_RATE_R1314_F638        ((uint8_t)0x00)     /* Rasing 13.14ns and falling 6.38ns. */
#define BST_SLEW_RATE_R870_F477         ((uint8_t)0x08)     /* Rasing 8.7ns and falling 4.77ns. */
#define BST_SLEW_RATE_R668_F361         ((uint8_t)0x10)     /* Rasing 6.68ns and falling 3.61ns. */
#define BST_SLEW_RATE_R329_F205         ((uint8_t)0x18)     /* Rasing 3.29ns and falling 2.05ns. */

/*! Checks slew rate. */
#define CDV_SLEW_RATE_CHECK(slew)   (((slew) == BST_SLEW_RATE_R1314_F638) || ((slew) == BST_SLEW_RATE_R870_F477) || \
									 ((slew) == BST_SLEW_RATE_R668_F361)  || ((slew) == BST_SLEW_RATE_R329_F205))

/* @defgroup wave_serial_num */
#define CDV_WAVE_SERIAL_1        ((uint8_t)0x00)     /* First wave. */
#define CDV_WAVE_SERIAL_2        ((uint8_t)0x01)     /* Second wave. */
#define CDV_WAVE_SERIAL_3        ((uint8_t)0x02)     /* Thrid wave. */
#define CDV_WAVE_SERIAL_4        ((uint8_t)0x03)     /* Fourth wave. */
#define CDV_WAVE_SERIAL_5        ((uint8_t)0x04)     /* Fifth wave. */
#define CDV_WAVE_SERIAL_6        ((uint8_t)0x05)     /* Sixth wave. */
#define CDV_WAVE_SERIAL_7        ((uint8_t)0x06)     /* Seventh wave. */
#define CDV_WAVE_SERIAL_8        ((uint8_t)0x07)     /* Eighth wave. */

/*! Checks Time threshold. */
#define CDV_WAVE_IDX_CHECK(wave)        (((wave) == CDV_WAVE_SERIAL_1) || ((wave) == CDV_WAVE_SERIAL_2) || \
										 ((wave) == CDV_WAVE_SERIAL_3) || ((wave) == CDV_WAVE_SERIAL_4) || \
										 ((wave) == CDV_WAVE_SERIAL_5) || ((wave) == CDV_WAVE_SERIAL_6) || \
										 ((wave) == CDV_WAVE_SERIAL_7) || ((wave) == CDV_WAVE_SERIAL_8))

/* @defgroup chip_mode */
#define CDV_MODE_ACTIVE         ((uint8_t)0x00)                     /* Chip enter active mode. */
#define CDV_MODE_STANDBY        CDV2625_BIT_SYS_CTRL_STANDBY        /* Chip enter standby mode. */

/*! Checks chip mode. */
#define CDV_CHIP_MODE_CHECK(mode)        (((mode) == CDV_MODE_ACTIVE) || ((mode) == CDV_MODE_STANDBY))

/* @defgroup idle_time_thr */
#define IDLE_TIME_TH_12CLK      ((uint8_t)0x00)     /* Enter idle mode wait time threshold is 12 12K-clock period. */
#define IDLE_TIME_TH_24CLK      ((uint8_t)0x40)     /* Enter idle mode wait time threshold is 24 12K-clock period. */
#define IDLE_TIME_TH_48CLK      ((uint8_t)0x80)     /* Enter idle mode wait time threshold is 48 12K-clock period. */
#define IDLE_TIME_TH_96CLK      ((uint8_t)0xC0)     /* Enter idle mode wait time threshold is 96 12K-clock period. */

/*! Checks standby time threshold. */
#define CDV_STDBY_TIME_TH_CHECK(time)       (((time) == IDLE_TIME_TH_12CLK) || ((time) == IDLE_TIME_TH_24CLK) || \
											 ((time) == IDLE_TIME_TH_48CLK) || ((time) == IDLE_TIME_TH_96CLK))

/* @defgroup wait_time_unit */
#define WAIT_TIME_UNIT_SP_1         ((uint8_t)0x00)     /* Waiting time unit is 1 sample period. */
#define WAIT_TIME_UNIT_SP_8         ((uint8_t)0x10)     /* Waiting time unit is 8 sample period. */
#define WAIT_TIME_UNIT_SP_64        ((uint8_t)0x20)     /* Waiting time unit is 64 sample period. */
#define WAIT_TIME_UNIT_SP_512       ((uint8_t)0x30)     /* Waiting time unit is 512 sample period. */

/*! Checks waiting time. */
#define CDV_WAIT_TIME_SP_CHECK(time)        (((time) == WAIT_TIME_UNIT_SP_1)  || ((time) == WAIT_TIME_UNIT_SP_8) || \
											 ((time) == WAIT_TIME_UNIT_SP_64) || ((time) == WAIT_TIME_UNIT_SP_512))

/* @defgroup up_sample_rate */
#define UP_SP_RATE_1        ((uint8_t)0x40)     /* Single up sample rate. */
#define UP_SP_RATE_2        ((uint8_t)0x00)     /* Double up sample rate. */
#define UP_SP_RATE_4        ((uint8_t)0x80)     /* Quadruple up sample rate.  */

/*! Checks standby time threshold. */
#define CDV_UP_SP_RATE_CHECK(sp)      (((sp) == UP_SP_RATE_1) || ((sp) == UP_SP_RATE_2) || ((sp) == UP_SP_RATE_4))


/* @defgroup stop_mode */
#define MODE_WAIT_DRIVE_TO_0_STOP       ((uint8_t)0x00)     /* Wait for the drive to stop when it crosses zero. */
#define MODE_WAIT_PLAY_CMPL_STOP        ((uint8_t)0x10)     /* Wait for the wave playing finished before stopping. */

/*! Checks stop mode. */
#define CDV_STOP_MODE_CHECK(mode)      (((mode) == MODE_WAIT_DRIVE_TO_0_STOP) || ((mode) == MODE_WAIT_PLAY_CMPL_STOP))

/* @defgroup play_mode */
#define RAM_MODE        ((uint8_t)0x00)     /* RAM mode. */
#define RTP_MODE        ((uint8_t)0x01)     /* RTP mode. */
#define CONT_MODE       ((uint8_t)0x02)     /* Cont mode. */
#define TEST_MODE       ((uint8_t)0x03)     /* Test mode. */

/*! Checks play mode. */
#define CDV_PLAY_MODE_CHECK(mode)       (((mode) == RAM_MODE)  || ((mode) == RTP_MODE)  || ((mode) == CONT_MODE) || \
										 ((mode) == TEST_MODE))

/* @defgroup cont_play_time_mode */
#define CONT_PLAY_TIME      ((uint8_t)0x20)     /* CONT play time is controlled by register CONT_TIME. */
#define CONT_PLAY_GO        ((uint8_t)0x00)     /* CONT play time is controlled by GO. */

/*! Checks play time mode. */
#define CDV_PLAY_TIME_MODE_CHECK(mode)      (((mode) == CONT_PLAY_TIME) || ((mode) == CONT_PLAY_GO))

/* @defgroup bemf_period */
#define CDV_FREE_OSC_DET_HELF_NUM_2     ((uint8_t)0x00)     /* Free oscillation detection half cycle number is 2. */
#define CDV_FREE_OSC_DET_HELF_NUM_4     ((uint8_t)0x10)     /* Free oscillation detection half cycle number is 4. */
#define CDV_FREE_OSC_DET_HELF_NUM_8     ((uint8_t)0x20)     /* Free oscillation detection half cycle number is 8. */

/*! Checks free oscillation detection half cycle number. */
#define CDV_FREE_OSC_DET_NUM_CHECK(num)     (((num) == CDV_FREE_OSC_DET_HELF_NUM_2) || \
											 ((num) == CDV_FREE_OSC_DET_HELF_NUM_4) || \
											 ((num) == CDV_FREE_OSC_DET_HELF_NUM_8))

/* @defgroup err_stage */
#define CDV_ERR_STAGE_OPEN_LOOP         ((uint8_t)0x08)     /* Error occurs in open loop stage. */
#define CDV_ERR_STAGE_CLOSE_LOOP        ((uint8_t)0x10)     /* Error occurs in close loop stage. */
#define CDV_ERR_STAGE_BRAKE             ((uint8_t)0x20)     /* Error occurs in brake stage. */
#define CDV_ERR_STAGE_FREE_OSC          ((uint8_t)0x40)     /* Error occurs in free oscillation stage. */

/*! Checks error stage. */
#define CDV_ERR_STAGE_CHECK(stage)      (((stage) == CDV_ERR_STAGE_OPEN_LOOP)  || ((stage) == CDV_ERR_STAGE_BRAKE) || \
										 ((stage) == CDV_ERR_STAGE_CLOSE_LOOP) || ((stage) == CDV_ERR_STAGE_FREE_OSC))

/* @defgroup trigger_channel_selection */
#define TRIG_CH1        ((uint8_t)0x01)     /* Trigger channel NO.1. */
#define TRIG_CH2        ((uint8_t)0x02)     /* Trigger channel NO.2. */
#define TRIG_CH3        ((uint8_t)0x03)     /* Trigger channel NO.3. */

/*! Checks trigger channel selection. */
#define CDV_TRIG_CH_CHECK(ch)       (((ch) == TRIG_CH1) || ((ch) == TRIG_CH2) || ((ch) == TRIG_CH3))

/* @defgroup trigger_edge_selection */
#define TRIG_RASING         ((uint8_t)0x00)     /* Trigger rasing. */
#define TRIG_FALLING        ((uint8_t)0x03)     /* Trigger falling. */
#define TRIG_HIGH_LEVEL     ((uint8_t)0x10)     /* Trigger high level. */
#define TRIG_LOW_LEVEL      ((uint8_t)0x13)     /* Trigger low level. */

/*! Checks trigger edge selection. */
#define CDV_TRIG_CHECK(trig)        (((trig) == TRIG_RASING)     || ((trig) == TRIG_FALLING)  || \
									 ((trig) == TRIG_HIGH_LEVEL) || ((trig) == TRIG_LOW_LEVEL))

/* @defgroup level_edge_selection */
#define TRIGER_LOW_LEVEL        ((uint8_t)0x00)     /* Trigger low level. */
#define TRIGER_HIGH_LEVEL       ((uint8_t)0x01)     /* Trigger high level. */

/*! Checks trigger level selection. */
#define CDV_TRIG_LV_CHECK(edge)     (((edge) == TRIGER_LOW_LEVEL) || ((edge) == TRIGER_HIGH_LEVEL))

/* @defgroup trig_ch1_reuse_sel */
#define TRIGER_NORMAL       ((uint8_t)0x00)                 /* The pins of TRIG1 are multiplexed as normal trigger. */
#define TRIGER_GO           CDV2625_TRG_CTRL10_TRG1_MODE    /* The pins of TRIG1 are multiplexed as go. */

/*! Checks trigger channel resue selection. */
#define TRIG_CH1_RESUE_CHECK(trig)      (((trig) == TRIGER_NORMAL) || ((trig) == TRIGER_GO))

/* @defgroup trig_pupd_sel */
#define TRIGER_PULL_NONE        ((uint8_t)0x00)     /* Trigger floating . */
#define TRIGER_PULL_UP          ((uint8_t)0x08)     /* Trigger pull-up. */
#define TRIGER_PULL_DWON        ((uint8_t)0x01)     /* Trigger pull-down. */

/*! Checks trigger channel pull up/down selection. */
#define TRIG_PUPD_CHECK(pull)       (((pull) == TRIGER_PULL_NONE) || ((pull) == TRIGER_PULL_UP) || \
									 ((pull) == TRIGER_PULL_DWON))

/* @defgroup pga_iic_level */
#define PAD_IIC_LEVEL_1V8       ((uint8_t)0x00)     /* IIC level 1.8V or vdd. */
#define PAD_IIC_LEVEL_1V2       ((uint8_t)0x01)     /* IIC level 1.2V. */

/*! Checks PGA common cur. */
#define PAD_IIC_LEVEL_CHECK(value)      (((value) == PAD_IIC_LEVEL_1V8) || ((value) == PAD_IIC_LEVEL_1V2))

/* @defgroup io_type_sel */
#define CDV_IO_SCL          ((uint8_t)0x01)     /* I2c SCL. */
#define CDV_IO_SDA          ((uint8_t)0x02)     /* I2c SDA. */
#define CDV_IO_TRIG1        ((uint8_t)0x04)     /* IO trgger 1. */
#define CDV_IO_TRIG2        ((uint8_t)0x08)     /* IO trgger 2. */
#define CDV_IO_TRIG3        ((uint8_t)0x10)     /* IO trgger 3. */
#define CDV_IO_INTR         ((uint8_t)0x20)     /* IO interrupt. */

/*! Checks IO type selection. */
#define IO_TYPE_CHECK(io)       (((io) == CDV_IO_SCL)   || ((io) == CDV_IO_SDA)   || ((io) == CDV_IO_TRIG1) || \
								 ((io) == CDV_IO_TRIG2) || ((io) == CDV_IO_TRIG3) || ((io) == CDV_IO_INTR))

/* @defgroup io_mode */
#define CDV_IO_DEGLITCH          ((uint8_t)0x00)     /* I2c SCL. */
#define CDV_IO_FAST              ((uint8_t)0x01)     /* I2c SDA. */

/*! Checks IO mode selection. */
#define IO_MODE_CHECK(io)       (((io) == CDV_IO_DEGLITCH)   || ((io) == CDV_IO_DEGLITCH))

/* switch_mode */
/* Play a higher priority waveform after the cur waveform has been played. */
#define CDV_SW_PLAY_HIGH_WAVE       ((uint8_t)0x00)
/* After the cur waveform is played, the higher priority waveform is not played. */
#define CDV_SW_PLAY_NONE_WAVE       ((uint8_t)0x10)
/* Switch to a higher priority waveform while waiting for BEMF zero-crossing (or driving zero-crossing). */
#define CDV_SW_WAIT_BEMF_GO_0       ((uint8_t)0x20)

/*! Checks switch mode selection. */
#define CDV_SW_MODE_CHECK(mode)     (((mode) == CDV_SW_PLAY_HIGH_WAVE) || ((mode) == CDV_SW_PLAY_NONE_WAVE) || \
									 ((mode) == CDV_SW_WAIT_BEMF_GO_0))
/* @defgroup global state */
#define CDV_GLB_STATUS_STANDBY              0x00
#define CDV_GLB_STATUS_OTHERS               0x02
#define CDV_GLB_STATUS_RAM                  0x04
#define CDV_GLB_STATUS_RTP                  0x05
#define CDV_GLB_STATUS_TX                   0x06
#define CDV_GLB_STATUS_ONEWIRE              0x07
#define CDV_GLB_STATUS_CONT                 0x08
#define CDV_GLB_STATUS_BRAKE                0x09

/* @defgroup output_type_sel */
#define PIN_OUT_OD                          ((uint8_t)0x00)     /* Interrupt pin output type is open-drain. */
#define PIN_OUT_PP                          ((uint8_t)0x01)     /* Interrupt pin output type is push-pull. */

/*! Checks interrupt pin output type selection. */
#define CDV_PIN_OUTYPE_CHECK(mode)          (((mode) == PIN_OUT_OD) || ((mode) == PIN_OUT_PP))

/* @defgroup Interrupt_pin_trig_sel */
#define INTR_PIN_TIRG_LOW_LV        ((uint8_t)0x00)     /* Interrupt pin trigger type is low level. */
#define INTR_PIN_TIRG_HIGH_LV       ((uint8_t)0x04)     /* Interrupt pin trigger type is high level. */
#define INTR_PIN_TIRG_RASING        ((uint8_t)0x08)     /* Interrupt pin trigger type is rasing edge. */
#define INTR_PIN_TIRG_FALLING       ((uint8_t)0x0C)     /* Interrupt pin trigger type is falling edge. */
#define INTR_PIN_TIRG_BOTH_EDGE     ((uint8_t)0x10)     /* Interrupt pin trigger type is rasing and falling edge. */

/*! Checks interrupt pin trigger type selection. */
#define CDV_PIN_TRIG_CHECK(trig)        (((trig) == INTR_PIN_TIRG_LOW_LV) || ((trig) == INTR_PIN_TIRG_HIGH_LV) || \
										 ((trig) == INTR_PIN_TIRG_RASING) || ((trig) == INTR_PIN_TIRG_FALLING) || \
										 ((trig) == INTR_PIN_TIRG_BOTH_EDGE))

/* @defgroup PWM_modulate_sel */
#define PWM_MODULATE_TRIANGLE                   ((uint8_t)0x00)     /* PWM modulate mode is triangle. */
#define PWM_MODULATE_SAWTOOTH                   ((uint8_t)0x80)     /* PWM modulate mode is sawtooth. */

/*! Checks PWM modulate mode selection. */
#define PMW_MODULATE_CHECK(mode)                (((mode) == PWM_MODULATE_TRIANGLE) || \
												((mode) == PWM_MODULATE_SAWTOOTH))

/* @defgroup VDD_gain_adj_sel */
#define VDD_GAIN_ADJ_SOFTWARE                   ((uint8_t)0x00)     /* VDD gain software adjustment mode. */
#define VDD_GAIN_ADJ_HARDWARE                   ((uint8_t)0x40)     /* VDD gain hardware adjustment mode. */

/*! Checks VDD gain adjustment mode. */
#define VDD_GAIN_ADJ_CHECK(mode)                (((mode) == VDD_GAIN_ADJ_SOFTWARE) || \
												((mode) == VDD_GAIN_ADJ_HARDWARE))

/* @defgroup VDD_det_sel */
#define WAVE_PLAY_BEFORE_VDD_DET                ((uint8_t)0x00)
#define WAVE_PLAYING_VDD_DET                    ((uint8_t)0x20)

/*! Checks VDD detection mode. */
#define VDD_DET_CHECK(mode)                     (((mode) == WAVE_PLAY_BEFORE_VDD_DET) || \
												((mode) == WAVE_PLAYING_VDD_DET))

/* @defgroup det_mode_sel */
#define DET_MODE_PVDD                           ((uint8_t)0xB0)
#define DET_MODE_VDD                            ((uint8_t)0xA0)
#define DET_MODE_RL                             ((uint8_t)0x10)
#define DET_MODE_ADCOS                          ((uint8_t)0x00)

/*! Checks detection mode selection. */
#define DET_MODE_CHECK(mode)    (((mode) == DET_MODE_PVDD) || ((mode) == DET_MODE_VDD) || ((mode) == DET_MODE_RL) || \
								 ((mode) == DET_MODE_ADCOS))

/* @defgroup rc_flt_coe */
#define RC_FLT_COE_HELF             ((uint8_t)0x00)     /* RC filter coefficient is 1/2. */
#define RC_FLT_COE_QUARTER          ((uint8_t)0x08)     /* RC filter coefficient is 1/4. */
#define RC_FLT_COE_ONE_EIGHTH       ((uint8_t)0x10)     /* RC filter coefficient is 1/8. */
#define RC_FLT_COE_ONE_SIXTEEN      ((uint8_t)0x18)     /* RC filter coefficient is 1/16. */

/*! Checks RC filter coefficient selection. */
#define RC_FLT_COE_CHECK(freq)      (((freq) == RC_FLT_COE_HELF)       || ((freq) == RC_FLT_COE_QUARTER) || \
									((freq) == RC_FLT_COE_ONE_EIGHTH) || ((freq) == RC_FLT_COE_ONE_SIXTEEN))

/* adc_clk_frq_sel */

#define ADC_CLK_FRQ_3HZ072                      ((uint8_t)0x02)     /* ADC clock freguency is 3.072Hz. */
#define ADC_CLK_FRQ_1HZ536                      ((uint8_t)0x04)     /* ADC clock freguency is 1.536Hz. */
#define ADC_CLK_FRQ_0HZ768                      ((uint8_t)0x06)     /* ADC clock freguency is 0.768Hz. */

/*! Checks adc clock frequency selection. */
#define ADC_CLK_FRQ_CHECK(freq)                 (((freq) == ADC_CLK_FRQ_3HZ027) || ((freq) == ADC_CLK_FRQ_1HZ536) || \
												((freq) == ADC_CLK_FRQ_0HZ768))

/* @defgroup PGA_gain_sel */
#define PGA_GAIN_SINGLE                         ((uint8_t)0x00)     /* PGA gain is 1. */
#define PGA_GAIN_DOUBLE                         ((uint8_t)0x01)     /* PGA gain is 2. */
#define PGA_GAIN_4TIMES                         ((uint8_t)0x02)     /* PGA gain is 4. */
#define PGA_GAIN_8TIMES                         ((uint8_t)0x03)     /* PGA gain is 8. */
#define PGA_GAIN_10TIMES                        ((uint8_t)0x04)     /* PGA gain is 10. */
#define PGA_GAIN_16TIMES                        ((uint8_t)0x05)     /* PGA gain is 16. */
#define PGA_GAIN_20TIMES                        ((uint8_t)0x06)     /* PGA gain is 20. */
#define PGA_GAIN_40TIMES                        ((uint8_t)0x07)     /* PGA gain is 40. */

/*! Checks PGA gain selection. */
#define PGA_GAIN_CHECK(gain)                    (((gain) == PGA_GAIN_SINGLE)  || ((gain) == PGA_GAIN_DOUBLE)  || \
												((gain) == PGA_GAIN_4TIMES)  || ((gain) == PGA_GAIN_8TIMES)  || \
												((gain) == PGA_GAIN_10TIMES) || ((gain) == PGA_GAIN_16TIMES) || \
												((gain) == PGA_GAIN_20TIMES) || ((gain) == PGA_GAIN_40TIMES))

/* Checks wave sample number selection. */
#define EDGE_WAVE_SP_NUM_CHECK(num)             ((num) <= 0x0F)

/* Checks wave ID selection */
#define WAVE_ID_CHECK(num)                      (((num) <= 0x7F) && ((num) != 0))

#define CDV2625_QA_OK                           0xAA
#define CDV2625_RAM_TYPE                        ((uint8_t)0x08)

/* CDV2625_Registers access permission */
#define CDV2625_REG_MAX                         0xff
#define REG_RD_ACCESS                           (1 << 0)
#define REG_WR_ACCESS                           (1 << 1)

#endif
