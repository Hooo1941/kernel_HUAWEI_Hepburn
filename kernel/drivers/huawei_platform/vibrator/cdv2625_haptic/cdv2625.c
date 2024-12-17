/*
 * cdv2625.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/power_supply.h>
#include <linux/vmalloc.h>
#include <linux/pm_qos.h>

#include "haptic_if.h"
#include "cdv26xx_reg.h"

const unsigned char cdv_reg_access[CDV2625_REG_MAX] = {
	[CDV2625_REG_CHIPID] = REG_RD_ACCESS,
	[CDV2625_SYS_STATUS] = REG_RD_ACCESS,
	[CDV2625_SYS_INTR] = REG_RD_ACCESS,
	[CDV2625_SYS_INTM] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_BST_CFG1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_BST_CFG2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_BST_CFG3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_GAIN] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_SYS_CTRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_GO] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX4] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX5] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX6] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX7] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAV_IDX8] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVLOOP1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVLOOP2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVLOOP3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVLOOP4] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_MAINLOOP] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_SRC_CTRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL5] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL6] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL7] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL8] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL9] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL11] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL12] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_CONT_CTRL13] = REG_RD_ACCESS,
	[CDV2625_CONT_CTRL14] = REG_RD_ACCESS,
	[CDV2625_CONT_CTRL15] = REG_RD_ACCESS,
	[CDV2625_CONT_CTRL16] = REG_RD_ACCESS,
	[CDV2625_CONT_CTRL20] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL4] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL5] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_RTP_CTRL6] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL3] = REG_RD_ACCESS,
	[CDV2625_TRG_CTRL4] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL5] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL6] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL7] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL8] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL9] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRG_CTRL10] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_PAD_CTRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_GLB_STATE] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVE_DATA] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVE_ADDRH] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_WAVE_ADDRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DATA_CTRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DBG_STAT] = REG_RD_ACCESS,
	[CDV2625_DPWM_CTRL1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DPWM_CTRL2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DPWM_CTRL3] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DPWM_CTRL4] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_VDD_CTRL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DET_CTRL1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DET_CTRL2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_DET_CTRL3] = REG_RD_ACCESS,
	[CDV2625_DET_CTRL4] = REG_RD_ACCESS,
	[CDV2625_DET_CTRL5] = REG_RD_ACCESS,
	[CDV2625_TRIM_OSC_LRA1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRIM_OSC_LRA2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[CDV2625_TRIM_EN] = REG_RD_ACCESS | REG_WR_ACCESS,
};

/***************************************************************************************************
 * @ cdv2625 driver functions
***************************************************************************************************/
static int cdv2625_chip_qualify_check(struct haptic *cs_haptic)
{
	int ret = -1;
	unsigned char reg_val = 0;

	log_info("enter");
	ret = cs_i2c_byte_read(cs_haptic, CDV2625_REG_QA, &reg_val, CS_I2C_BYTE_ONE);
	if (ret < 0)
		return ret;
	if (reg_val != CDV2625_QA_OK) {
		log_info("chip was not test in mass production, reg_val = %x", reg_val);
		return -ERANGE;
	}
	return 0;
}

cs_bit_status_t cdv2625_flag_status_get(struct haptic *cdv2625, uint8_t flag)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_STATUS, &temreg, 0x01);
	if (temreg & flag)
		return CS_SET;

	return CS_RESET;
}

void cdv2625_soft_rst(struct haptic *cdv2625)
{
	uint8_t reg_rst = 0xaa; // CHIPID reg reset

	cs_i2c_byte_write(cdv2625, CDV2625_REG_CHIPID, &reg_rst, 0x01);
}

cs_bit_status_t cdv2625_interrupt_status_get(struct haptic *cdv2625, uint8_t intr)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_INTR, &temreg, 0x01);
	if (temreg & intr)
		return CS_SET;
	return CS_RESET;
}

void cdv2625_interrupt_enable_ctrl(struct haptic *cdv2625, uint8_t intr, cs_enable_state_t state)
{
	uint8_t reg_temp = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_INTM, &reg_temp, 0x01);
	if (state == CS_DISABLE)
		reg_temp |= intr;
	else
		reg_temp &= ~intr;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_INTM, &reg_temp, 0x01);
}

void cdv2625_auto_bst_enable_ctrl(struct haptic *cdv2625, uint8_t mode, uint8_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= mode;
	else
		buf &= ~mode;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
}

void cdv2625_rtp_enter_lowpower_thr_set(struct haptic *cdv2625, uint8_t time)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	buf &= ~CDV2625_BIT_BST_CFG1_RTP_LP_TIME_TH;
	buf |= time;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
}

void cdv2625_rpt_auto_enter_lowpower_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_BST_CFG1_RTP_AUTO_LP;
	else
		buf &= ~CDV2625_BIT_BST_CFG1_RTP_AUTO_LP;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
}

void cdv2625_bst_bypass_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	if (state != CS_DISABLE)
		buf &= ~CDV2625_BIT_BST_CFG1_BST_BYPASS;
	else
		buf |= CDV2625_BIT_BST_CFG1_BST_BYPASS;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
}

void cdv2625_bst_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_BST_CFG1_BOOST_EN;
	else
		buf &= ~CDV2625_BIT_BST_CFG1_BOOST_EN;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
}

uint8_t cdv2625_bst_status_get(struct haptic *cdv2625)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG1, &buf, 0x01);
	return buf;
}

void cdv2625_bst_pre_en_thr_set(struct haptic *cdv2625, uint8_t time)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG2, &buf, 0x01);
	buf &= ~CDV2625_BIT_BST_CFG2_BST_PRE_EN_TH;
	buf |= time;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG2, &buf, 0x01);
}

void cdv2625_soft_start_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG3, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_BST_CFG3_BYPASS_SLEW_EN;
	else
		buf &= ~CDV2625_BIT_BST_CFG3_BYPASS_SLEW_EN;
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG3, &buf, 0x01);
}

void cdv2625_pvdd_out_vol_set(struct haptic *cdv2625, uint8_t vol)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_BST_CFG3, &buf, 0x01);
	buf &= ~CDV2625_BIT_BST_CFG3_BST_VOLT;
	buf |= (vol & 0x7F);
	cs_i2c_byte_write(cdv2625, CDV2625_BST_CFG3, &buf, 0x01);
}

void cdv2625_set_gain(struct haptic *cdv2625, uint8_t gain)
{
	uint8_t buf = gain;

	log_info("gain = %u", gain);
	cs_i2c_byte_write(cdv2625, CDV2625_GAIN, &buf, 0x01);
}

uint8_t cdv2625_get_gain(struct haptic *cdv2625)
{
	uint8_t buf = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_GAIN, &buf, 0x01);
	log_info("read gain = %u", buf);
	return buf;
}

void cdv2625_up_sample_rate_config(struct haptic *cdv2625, uint8_t sample_rate)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	buf &= ~CDV2625_BIT_SYS_CTRL_WAVE_UP_RATE;
	buf |= sample_rate;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_fifo_reset(struct haptic *cdv2625)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	buf |= CDV2625_BIT_SYS_CTRL_FIFO_RESET;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_stop_or_play_switching_mode_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_SYS_CTRL_STOP_MODE;
	else
		buf &= ~CDV2625_BIT_SYS_CTRL_STOP_MODE;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_active_mode_set(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	if (mode != CDV_MODE_ACTIVE)
		buf |= CDV2625_BIT_SYS_CTRL_STANDBY;
	else
		buf &= ~CDV2625_BIT_SYS_CTRL_STANDBY;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_extra_brk_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_SYS_CTRL_EXTRA_BRK;
	else
		buf &= ~CDV2625_BIT_SYS_CTRL_EXTRA_BRK;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_wave_play_mode_set(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
	buf &= ~CDV2625_BIT_SYS_CTRL_PLAY_MODE;
	buf |= mode;
	cs_i2c_byte_write(cdv2625, CDV2625_SYS_CTRL, &buf, 0x01);
}

void cdv2625_wave_play_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t regtemp = 0;

	if (state != CS_DISABLE)
		regtemp = CDV2625_BIT_GO_GO;
	cs_i2c_byte_write(cdv2625, CDV2625_GO, &regtemp, 0x01);
}

cs_bit_status_t cdv2625_play_finish_status_get(struct haptic *cdv2625)
{
	uint8_t  reg_val = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_GO, &reg_val, 0x01);
	if (reg_val & CDV2625_BIT_GO_GO)
		return CS_RESET;
	return CS_SET;
}

void cdv2625_wave_wait_time_set(struct haptic *cdv2625, uint8_t wave_serial_num, uint8_t time)
{
	uint8_t buf = time | 0x80;

	cs_i2c_byte_write(cdv2625, (wave_serial_num + 0x09), &buf, 0x01);
}

void cdv2625_wave_id_set(struct haptic *cdv2625, uint8_t wave_serial_num, uint8_t wave_id)
{
	uint8_t buf = wave_id & 0x7F;

	cs_i2c_byte_write(cdv2625, (wave_serial_num + 0x0a), &buf, 0x01);
}

void cdv2625_wave_loop_set(struct haptic *cdv2625, uint8_t wave_serial_num, uint8_t loop_time)
{
	uint8_t loopbuf = 0;
	uint8_t reg_addr = 0;

	reg_addr = wave_serial_num / 2 + 0x12;
	cs_i2c_byte_read(cdv2625, reg_addr, &loopbuf, 0x01);

	if (wave_serial_num % 2) {
		loopbuf &= 0xF0;
		loop_time = (loop_time & 0x0F);
		loopbuf = loop_time | loopbuf;
	} else {
		loopbuf &= 0x0F;
		loop_time = loop_time << 4;
		loopbuf = (loop_time & 0xF0) | loopbuf;
	}
	cs_i2c_byte_write(cdv2625, reg_addr, &loopbuf, 0x01);
}

void cdv2625_enter_idle_times_thr_set(struct haptic *cdv2625, uint8_t time)
{
	uint8_t buf = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
	buf &= ~CDV2625_BIT_MAINLOOP_STDBY_TIME_TH;
	buf |= (time & CDV2625_BIT_MAINLOOP_STDBY_TIME_TH);
	cs_i2c_byte_write(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
}

void cdv2625_wait_unit_set(struct haptic *cdv2625, uint8_t time)
{
	uint8_t buf = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
	buf &= ~CDV2625_BIT_MAINLOOP_WAIT_UNIT;
	buf |= (time & CDV2625_BIT_MAINLOOP_WAIT_UNIT);
	cs_i2c_byte_write(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
}

void cdv2625_main_loop_set(struct haptic *cdv2625, uint8_t loop_times)
{
	uint8_t buf = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
	buf &= ~CDV2625_BIT_MAINLOOP_MAINLOOP;
	buf |= (loop_times & CDV2625_BIT_MAINLOOP_MAINLOOP);
	cs_i2c_byte_write(cdv2625, CDV2625_MAINLOOP, &buf, 0x01);
}

void cdv2625_pwm_edge_init(struct haptic *cdv2625, edge_pwm_config_t * edge_pwm_config)
{
	uint8_t buf = 0;

	buf = (edge_pwm_config->wave_sample_num << 4) | ((uint8_t)(edge_pwm_config->mode_selection)) |
		  ((uint8_t)edge_pwm_config->freq_selection);
	if (edge_pwm_config->pwm_edge_en != CS_DISABLE)
		buf |= CDV2625_BIT_SRC_CTRL_SRC_EN;
	else
		buf &= ~CDV2625_BIT_SRC_CTRL_SRC_EN;
	cs_i2c_byte_write(cdv2625, CDV2625_SRC_CTRL, &buf, 0x01);
}

void cdv2625_closed_loop_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);

	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_CONT_CTRL3_EN_CLOSE;
	else
		buf &= ~CDV2625_BIT_CONT_CTRL3_EN_CLOSE;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
}

void cdv2625_cont_play_end_mode_config(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
	buf &= ~CDV2625_BIT_CONT_CTRL3_CONT_END_M;
	buf |= mode;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
}

void cdv2625_pow_on_f0_detect_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
	if (state != CS_DISABLE)
		buf |= CDV2625_BIT_CONT_CTRL3_F0_DET_MODE;
	else
		buf &= ~CDV2625_BIT_CONT_CTRL3_F0_DET_MODE;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
}

void cdv2625_max_brake_cycle_set(struct haptic *cdv2625, uint8_t num)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
	if (!num)
		num = 0x04; /* num can't be 0, if num is 0, Set it reset value. */
	buf &= ~CDV2625_BIT_CONT_CTRL3_BRK_CYCLE_HALF;
	buf |= num;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL3, &buf, 0x01);
}

void cdv2625_norm_dri_volt_set(struct haptic *cdv2625, uint8_t vol)
{
	uint8_t buf = vol;

	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL5, &buf, 0x01);
}

void cdv2625_over_dri_volt_set(struct haptic *cdv2625, uint8_t vol)
{
	uint8_t buf = vol;

	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL6, &buf, 0x01);
}

void cdv2625_f0_track_config(struct haptic *cdv2625, uint8_t perid_num, cs_enable_state_t sam_mode_en, uint8_t rep_time)
{
	uint8_t buf = (perid_num << 4) | rep_time;

	if (sam_mode_en != CS_DISABLE)
		buf |= CDV2625_BIT_CONT_CTRL7_F0_HD;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL7, &buf, 0x01);
}

void cdv2625_bemf_cyc_hf_set(struct haptic *cdv2625, uint8_t perid_num)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL8, &buf, 0x01);
	buf &= ~CDV2625_BIT_CONT_CTRL8_BEMF_DET_CYC_HF;
	buf |= perid_num;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL8, &buf, 0x01);
}

void cdv2625_f0_free_cyc_hf_set(struct haptic *cdv2625, uint8_t perid_num)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL8, &buf, 0x01);
	buf &= ~CDV2625_BIT_CONT_CTRL8_F0_FREE_CYC_HF;
	buf |= perid_num;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL8, &buf, 0x01);
}

void cdv2625_f0_close_cyc_set(struct haptic *cdv2625, uint8_t perid_num)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL9, &buf, 0x01);
	buf &= ~CDV2625_BIT_CONT_CTRL9_F0_CLS_CYC;
	buf |= (perid_num & 0x07);
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL9, &buf, 0x01);
}

void cdv2625_perset_f0_set(struct haptic *cdv2625, uint16_t value)
{
	uint8_t buf[2] = {0, 0};

	log_info("enter! 0x%x", value);
	buf[0] = value >> 8;
	buf[1] = value;
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL11, &buf[0], 0x01);
	cs_i2c_byte_write(cdv2625, CDV2625_CONT_CTRL12, &buf[1], 0x01);
}

uint16_t cdv2625_preset_f0_get(struct haptic *cdv2625)
{
	uint8_t buf[2] = {0, 0};

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL11, &buf[0], 0x01);
	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL12, &buf[1], 0x01);
	return (((uint16_t)buf[0] << 8) | buf[1]);
}

uint16_t cdv2625_free_f0_get(struct haptic *cdv2625)
{
	uint8_t buf[2] = {0, 0};

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL13, &buf[0], 0x01);
	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL14, &buf[1], 0x01);
	return (((uint16_t)buf[0] << 8) | buf[1]);
}

uint16_t cdv2625_cont_driver_f0_get(struct haptic *cdv2625)
{
	uint8_t buf[2] = {0, 0};

	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL15, &buf[0], 0x01);
	cs_i2c_byte_read(cdv2625, CDV2625_CONT_CTRL16, &buf[1], 0x01);
	return (((uint16_t)buf[0] << 8) | buf[1]);
}

void cdv2625_sram_base_addr_set(struct haptic *cdv2625, uint16_t addr)
{
	uint8_t buf[2] = {0, 0};

	buf[0] = addr >> 8;
	buf[1] = addr;
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL1, &buf[0], 0x01);
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL2, &buf[1], 0x01);
}

void cdv2625_fifo_thr_set(struct haptic *cdv2625, uint16_t empty_thr, uint16_t full_thr)
{
	uint8_t buf[3] = {0, 0, 0};

	buf[0] = (empty_thr >> 4) & 0xF0;
	buf[1] = (full_thr >> 8) & 0x0F;
	buf[2] = buf[0] | buf[1];
	buf[0] = (uint8_t)(empty_thr & 0x0ff);
	buf[1] = (uint8_t)(full_thr & 0x0ff);
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL3, &buf[2], 0x01);
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL4, &buf[0], 0x01);
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL5, &buf[1], 0x01);
}

void cdv2625_set_rtp_data(struct haptic *cdv2625, uint8_t *data, uint32_t len)
{
	cs_i2c_byte_write(cdv2625, CDV2625_RTP_CTRL6, data, len);
}

void cdv2625_edge_triger_waveid_set(struct haptic *cdv2625, uint8_t chx, uint8_t trigger, uint8_t wave_id)
{
	uint8_t temreg = 0;
	uint8_t addr = chx + 0x32 + (trigger & 0x0F);

	cs_i2c_byte_read(cdv2625, addr, &temreg, 0x01);
	temreg &= ~CDV2625_TRG_CTRL1_TRG1_IDX_P;
	temreg |= wave_id;
	cs_i2c_byte_write(cdv2625, addr, &temreg, 0x01);
}

void cdv2625_triger_mode_config(struct haptic *cdv2625, uint8_t chx, uint8_t trigger, cs_enable_state_t state)
{
	uint8_t temreg = 0;
	uint8_t temp = 0;
	uint8_t addr = 0;

	/* dege trigger */
	if ((trigger == TRIG_RASING) || (trigger == TRIG_FALLING)) {
		/* Sets trigger mode is dege trigger. */
		addr = 0x39 + (chx / 3);
		cs_i2c_byte_read(cdv2625, addr, &temp, 0x01);
		temp &= ~(CDV2625_TRG_CTRL7_TRG1_LVL_EN >> (((chx + 1) % 2) * 4));
		cs_i2c_byte_write(cdv2625, addr, &temp, 0x01);

		/* Sets rasing or falling trigger. */
		addr = chx + 0x32 + (trigger & 0x0F);
		cs_i2c_byte_read(cdv2625, addr, &temreg, 0x01);

		if (state != CS_DISABLE)
			temreg |= CDV2625_TRG_CTRL1_TRG1_PE;
		else
			temreg &= ~CDV2625_TRG_CTRL1_TRG1_PE;
		cs_i2c_byte_write(cdv2625, addr, &temreg, 0x01);
	} else { /* level trigger */
		if (state != CS_DISABLE) {
			/* Sets trigger mode is level trigger. */
			addr = 0x39 + (chx / 3);
			cs_i2c_byte_read(cdv2625, addr, &temp, 0x01);
			temp |= (CDV2625_TRG_CTRL7_TRG1_LVL_EN >> (((chx + 1) % 2) * 4));
			if (trigger == TRIG_HIGH_LEVEL)
				temp |= (CDV2625_TRG_CTRL7_TRG1_LVL >> (((chx + 1) % 2) * 4));
			else
				temp &= ~(CDV2625_TRG_CTRL7_TRG1_LVL >> (((chx + 1) % 2) * 4));
			cs_i2c_byte_write(cdv2625, addr, &temp, 0x01);
		} else {
			/* Sets trigger mode is dege trigger. */
			addr = 0x39 + (chx / 3);
			cs_i2c_byte_read(cdv2625, addr, &temp, 0x01);
			temp &= ~(CDV2625_TRG_CTRL7_TRG1_LVL_EN >> (((chx + 1) % 2) * 4));
			cs_i2c_byte_write(cdv2625, addr, &temp, 0x01);
		}
	}
}

void cdv2625_triger_brk_enable_ctrl(struct haptic *cdv2625, uint8_t chx, cs_enable_state_t state)
{
	uint8_t temreg = 0;
	uint8_t addr = 0;

	addr = 0x39 + (chx / 3);
	cs_i2c_byte_read(cdv2625, addr, &temreg, 0x01);
	if (state != CS_DISABLE)
		temreg |= (CDV2625_TRG_CTRL7_TRG1_BRK >> (((chx + 1) % 2) * 4));
	else
		temreg &= ~(CDV2625_TRG_CTRL7_TRG1_BRK >> (((chx + 1) % 2) * 4));
	cs_i2c_byte_write(cdv2625, addr, &temreg, 0x01);
}

void cdv2625_triger_bst_enable_ctrl(struct haptic *cdv2625, uint8_t chx, cs_enable_state_t state)
{
	uint8_t temreg = 0;
	uint8_t addr = 0;

	addr = 0x39 + (chx / 3);
	cs_i2c_byte_read(cdv2625, addr, &temreg, 0x01);
	if (state != CS_DISABLE)
		temreg |= (CDV2625_TRG_CTRL7_TRG1_BST >> (((chx + 1) % 2) * 4));
	else
		temreg &= ~(CDV2625_TRG_CTRL7_TRG1_BST >> (((chx + 1) % 2) * 4));
	cs_i2c_byte_write(cdv2625, addr, &temreg, 0x01);
}

void cdv2625_triger_play_enable_ctrl(struct haptic *cdv2625, uint8_t chx, cs_enable_state_t state)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_TRG_CTRL8, &temreg, 0x01);
	if (state != CS_DISABLE)
		temreg &= ~(1 << (chx - 1));
	else
		temreg |= (1 << (chx - 1));
	cs_i2c_byte_write(cdv2625, CDV2625_TRG_CTRL8, &temreg, 0x01);
}

uint8_t cdv2625_global_state_get(struct haptic *cdv2625)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_GLB_STATE, &temreg, 0x01);
	return temreg & CDV2625_GLB_STATE_GLB_STATE;
}

int cdv2625_wave_data_set(struct haptic *cdv2625, uint8_t *data, uint16_t len)
{
	return cs_write_sram(cdv2625, CDV2625_WAVE_DATA, data, len);
}

int cdv2625_wave_addr_set(struct haptic *cdv2625, uint16_t addr)
{
	uint8_t buf[2] = {0, 0};

	buf[0] = addr >> 8;
	buf[1] = addr;
	return cs_i2c_byte_write(cdv2625, CDV2625_WAVE_ADDRH, buf, 0x02);
}

void cdv2625_intr_pin_config(struct haptic *cdv2625, uint8_t out_type, uint8_t inter_type)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DATA_CTRL, &temreg, 0x01);
	temreg &= ~(CDV2625_DATA_CTRL_INT_OD | CDV2625_DATA_CTRL_INT_MODE);
	if (out_type != PIN_OUT_OD)
		temreg |= (CDV2625_DATA_CTRL_INT_OD | inter_type);
	cs_i2c_byte_write(cdv2625, CDV2625_DATA_CTRL, &temreg, 0x01);
}

void cdv2625_gain_switch_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t  state)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DATA_CTRL, &temreg, 0x01);
	if (state != CS_DISABLE)
		temreg |= CDV2625_DATA_CTRL_GAIN_SW_M;
	else
		temreg &= ~CDV2625_DATA_CTRL_GAIN_SW_M;
	cs_i2c_byte_write(cdv2625, CDV2625_DATA_CTRL, &temreg, 0x01);
}

void cdv2625_pwm_sample_rate_config(struct haptic *cdv2625, samp_rate_mode_t pwm_type)
{
	uint8_t buf = 0x00;

	cs_i2c_byte_read(cdv2625, CDV2625_DPWM_CTRL2, &buf, 0x01);
	buf &= ~CDV2625_DPWM_CTRL2_PWMCLK_MODE;
	buf |= ((uint8_t)pwm_type << 5);
	cs_i2c_byte_write(cdv2625, CDV2625_DPWM_CTRL2, &buf, 0x01);
}

void cdv2625_pwm_in_prot_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DPWM_CTRL3, &temreg, 0x01);
	if (state != CS_DISABLE)
		temreg |= CDV2625_DPWM_CTRL3_PWMI_PR_EN;
	else
		temreg &= ~CDV2625_DPWM_CTRL3_PWMI_PR_EN;
	cs_i2c_byte_write(cdv2625, CDV2625_DPWM_CTRL3, &temreg, 0x01);
}

void cdv2625_pwm_in_prot_vol_set(struct haptic *cdv2625, uint8_t vol)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DPWM_CTRL3, &temreg, 0x01);
	temreg &= ~CDV2625_DPWM_CTRL3_PWMI_PRLVL;
	temreg |= (vol & 0x7F);
	cs_i2c_byte_write(cdv2625, CDV2625_DPWM_CTRL3, &temreg, 0x01);
}

void cdv2625_vdd_gain_adj_mode_config(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_VDD_CTRL, &temreg, 0x01);
	temreg &= ~CDV2625_VDD_CTRL_VDD_AJ_MODE;
	temreg |= mode;
	cs_i2c_byte_write(cdv2625, CDV2625_VDD_CTRL, &temreg, 0x01);
}

void cdv2625_vdd_det_mode_config(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_VDD_CTRL, &temreg, 0x01);
	temreg &= ~CDV2625_VDD_CTRL_VDD_DET_M;
	temreg |= mode;
	cs_i2c_byte_write(cdv2625, CDV2625_VDD_CTRL, &temreg, 0x01);
}

void cdv2625_det_mode_config(struct haptic *cdv2625, uint8_t mode)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
	temreg &= ~CDV2625_DET_CTRL1_DET_MODE;
	temreg |= mode;
	cs_i2c_byte_write(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
}

void cdv2625_adc_clk_config(struct haptic *cdv2625, uint8_t clk)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
	temreg &= ~CDV2625_DET_CTRL1_CLK_ADC_SEL;
	temreg |= clk;
	cs_i2c_byte_write(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
}

void cdv2625_det_enable_ctrl(struct haptic *cdv2625, cs_enable_state_t state)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);

	if (state != CS_DISABLE)
		temreg |= CDV2625_DET_CTRL1_DET_GO;
	else
		temreg &= ~CDV2625_DET_CTRL1_DET_GO;

	cs_i2c_byte_write(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
}

uint8_t cdv26xx_get_det_enable_status(struct haptic *cdv2625)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL1, &temreg, 0x01);
	return (temreg & CDV2625_DET_CTRL1_DET_GO);
}

void cdv2625_adc_flt_coe_config(struct haptic *cdv2625, uint8_t coe)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL2, &temreg, 0x01);
	temreg &= ~(CDV2625_DET_CTRL2_ADC_FLT_COE | CDV2625_DET_CTRL2_ALG_SEL);
	temreg |= coe;
	cs_i2c_byte_write(cdv2625, CDV2625_DET_CTRL2, &temreg, 0x01);
}

void cdv2625_pga_gain_config(struct haptic *cdv2625, uint8_t gain)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL2, &temreg, 0x01);
	temreg &= ~CDV2625_DET_CTRL2_PGA_GAIN;
	temreg |= gain;
	cs_i2c_byte_write(cdv2625, CDV2625_DET_CTRL2, &temreg, 0x01);
}

uint8_t cdv2625_pga_gain_get(struct haptic *cdv2625)
{
	uint8_t temreg = 0;
	uint8_t pga_map[] = {1, 2, 4, 8, 10, 16, 20, 40};

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL2, &temreg, 0x01);
	temreg &=CDV2625_DET_CTRL2_PGA_GAIN;
	return pga_map[temreg];
}


uint16_t cdv2625_adc_average_get(struct haptic *cdv2625)
{
	uint8_t buf[2] = {0, 0};

	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL3, &buf[0], 0x01);
	cs_i2c_byte_read(cdv2625, CDV2625_DET_CTRL4, &buf[1], 0x01);
	return ((((uint16_t)buf[0] << 4) & 0x0F00) | buf[1]);
}

void cdv2625_f0_osc_adj_set(struct haptic *cdv2625, uint16_t value)
{
	uint8_t temreg = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_TRIM_OSC_LRA1, &temreg, 0x01);
	temreg &= ~CDV2625_TRIM_OSC_TRIM_OSC_LRA_H;
	temreg |=  ((value >> 8) & CDV2625_TRIM_OSC_TRIM_OSC_LRA_H);
	cs_i2c_byte_write(cdv2625, CDV2625_TRIM_OSC_LRA1, &temreg, 0x01);
	temreg = value;
	cs_i2c_byte_write(cdv2625, CDV2625_TRIM_OSC_LRA2, &temreg, 0x01);
}

void cdv2625_delay_us(uint32_t xus)
{
	usleep_range(xus, xus + 500);
}

static int cdv2625_standby_wait(struct haptic *cdv2625)
{
	unsigned int cnt = CS_INTO_STANDBY_RETRIES;
	unsigned char glb_val = 0;

	while (cnt--) {
		glb_val = cdv2625_global_state_get(cdv2625);
		if (glb_val == CDV_GLB_STATUS_STANDBY) {
			log_info("wait for standby, current status = 0x%02x", glb_val);
			return 0;
		}
		usleep_range(2000, 2200);
	}
	log_err("wait into standby failed, current status = 0x%02x", glb_val);
	return -1;
}

void cdv2625_stop(struct haptic *cdv2625)
{
	unsigned char glb_val = 0;

	log_info("enter");
	if (cdv2625->play_mode == CS_RAM_MODE) {
		while (1) {
			glb_val = cdv2625_global_state_get(cdv2625);
			if (glb_val == CDV_GLB_STATUS_STANDBY)
				break;
			cdv2625->chip_ipara.kcur_time = ktime_get();
			cdv2625->chip_ipara.interval_us =
				ktime_to_us(ktime_sub(cdv2625->chip_ipara.kcur_time, cdv2625->chip_ipara.kpre_time));
			if (cdv2625->chip_ipara.interval_us > 30000)
				break;
			log_info("play time us = %u, less than 30ms, wait", cdv2625->chip_ipara.interval_us);
			usleep_range(5000, 5500);
		}
	}
	if (!cdv2625_play_finish_status_get(cdv2625)) {
		cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
		cdv2625_wave_play_enable_ctrl(cdv2625, CS_DISABLE);
		if (cdv2625_standby_wait(cdv2625) != 0) {
			cdv2625->play_mode = CS_STANDBY_MODE;
			cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
			cdv2625_soft_rst(cdv2625);
		}
	}
}

static void cdv2625_ram_init(struct haptic *cdv2625, bool flag)
{
	log_info("enter");
	if (flag)
		cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	else
		cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
}

void cdv2625_det_go_wait(struct haptic *cdv2625, uint8_t state, uint16_t time_ms)
{
	uint16_t cnt = 0;

	while (cdv26xx_get_det_enable_status(cdv2625) != state) {
		cnt+=5;
		msleep(5);
		if (cnt > time_ms)
			break;
	}
}

uint8_t cdv2625_check_status_wait(struct haptic *cdv2625, uint8_t status, cs_bit_status_t state, uint16_t time_ms)
{
	uint16_t cnt = 0;
	uint8_t res = 0;

	while (cdv2625_flag_status_get(cdv2625, status) != state) {
		cnt += 5;
		msleep(5);
		if (cnt > time_ms) {
			res = 1;
			break;
		}
	}
	return res;
}

void cdv2625_playmode(struct haptic *cdv2625, uint8_t playmode)
{
	cdv2625->play_mode = playmode;
	switch (playmode) {
	case CS_STANDBY_MODE:
		log_info("enter standby mode");
		cdv2625_stop(cdv2625);
		break;
	case CS_RAM_MODE:
		log_info("enter ram mode");
		cdv2625_wave_play_mode_set(cdv2625, RAM_MODE);
		if (cdv2625->index >= HAPTIC_RAM_BRK_DISABLE_ID_1)
			cdv2625_extra_brk_enable_ctrl(cdv2625, CS_DISABLE);
		else
			cdv2625_extra_brk_enable_ctrl(cdv2625, CS_ENABLE);
		cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL10, LONG_RAM_MAX_BRK_VOL);
		break;
	case CS_RAM_LOOP_MODE:
		log_info("enter ram loop mode");
		cdv2625_wave_play_mode_set(cdv2625, RAM_MODE);
		cdv2625_auto_bst_enable_ctrl(cdv2625, CDV_MODE_RAM, CS_DISABLE);
		cdv2625_extra_brk_enable_ctrl(cdv2625, CS_ENABLE);
		cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL10, SPECIAL_RAM_MAX_BRK_VOL);
		cdv2625_set_gain(cdv2625, HAPTIC_MAX_GAIN_REG);
		break;
	case CS_RTP_MODE:
		log_info("enter rtp mode");
		cdv2625_wave_play_mode_set(cdv2625, RTP_MODE);
		cdv2625_auto_bst_enable_ctrl(cdv2625, CDV_MODE_RTP, CS_ENABLE);
		cdv2625_extra_brk_enable_ctrl(cdv2625, CS_DISABLE);
		break;
	case CS_CONT_MODE:
		log_info("enter cont mode");
		cdv2625_wave_play_mode_set(cdv2625, CONT_MODE);
		cdv2625_extra_brk_enable_ctrl(cdv2625, CS_DISABLE);
		break;
	case CS_TRIG_MODE:
		log_info("enter trig mode");
		break;
	default:
		log_info("enter standby mode");
		cdv2625_stop(cdv2625);
		break;
	}
}

static void cdv2625_set_repeat_seq(struct haptic *cs_haptic, uint8_t seq)
{
	log_info("enter");
	cdv2625_wave_id_set(cs_haptic, CDV_WAVE_SERIAL_1, seq);
	cdv2625_wave_loop_set(cs_haptic, CDV_WAVE_SERIAL_1, 0x0F);
}

static void cdv2625_vbat_mode_config(struct haptic *cs_haptic, uint8_t flag)
{
	log_info("enter");
	if (flag == CS_CONT_VBAT_HW_COMP_MODE)
		cdv2625_vdd_gain_adj_mode_config(cs_haptic, VDD_GAIN_ADJ_HARDWARE);
	else
		cdv2625_vdd_gain_adj_mode_config(cs_haptic, VDD_GAIN_ADJ_SOFTWARE);
}

static void cdv2625_set_ram_addr(struct haptic *cs_haptic)
{
	uint16_t base_addr = (uint16_t)cs_haptic->ram.base_addr;

	cdv2625_wave_addr_set(cs_haptic, base_addr);
}

void cdv2625_pre_wave(struct haptic *cdv2625)
{
	uint8_t edge_wave[] = { 0x07, 0x0E, 0x15, 0x1C, 0x23, 0x2A, 0x31, 0x37, 0x3D, 0x44, 0x49, 0x4F, 0x55, 0x5A, 0x5F,
							0x63, 0x68, 0x6C, 0x6F, 0x72, 0x75, 0x78, 0x7A, 0x7C, 0x7D, 0x7E, 0x7F, 0x7F };
	uint16_t t_len = 0;
	edge_pwm_config_t edge_pwm_config_struct;
	int i = 0;

	t_len = sizeof(edge_wave);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	for (i = 0; i < CS_I2C_RETRY_TIMES; i++) {
		if (cdv2625_wave_addr_set(cdv2625, cdv2625->ram_id - t_len) >= 0) {
			if (cdv2625_wave_data_set(cdv2625, edge_wave, t_len) >= 0)
				break;
		}
	}
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
	edge_pwm_config_struct.freq_selection = RC_FLT_LMT_FRQ_600;
	edge_pwm_config_struct.mode_selection = PWM_EDGE_MODE_SRAM;
	edge_pwm_config_struct.pwm_edge_en = CS_ENABLE;
	edge_pwm_config_struct.wave_sample_num = (t_len / 4 - 1);
	cdv2625_pwm_edge_init(cdv2625, &edge_pwm_config_struct);
}

#ifdef CHECK_RAM_DATA
static int cdv2625_check_ram_data(struct haptic *cdv2625, struct haptic_container *cs_cont)
{
	uint8_t ram_data[CDV2625_RAM_SINGLE_WRITE_LEN] = {0};
	uint16_t i = cdv2625->ram.ram_shift;
	uint16_t start_addr = cdv2625->ram.base_addr;
	uint16_t len = 0;
	int j = 0;

	while (i < cs_cont->len) {
		if ((cs_cont->len - i) < CDV2625_RAM_SINGLE_WRITE_LEN)
			len = cs_cont->len - i;
		else
			len = CDV2625_RAM_SINGLE_WRITE_LEN;
		cdv2625_wave_addr_set(cdv2625, start_addr);
		cs_i2c_byte_read(cdv2625, CDV2625_WAVE_DATA, ram_data, len);
		for (j = 0; j < len; j++) {
			if (ram_data[j] != cs_cont->data[i + j]) {
				log_info("check ramdata error, addr=0x%04x, ram_data=0x%02x, file_data=0x%02x",
						start_addr, ram_data[j], cs_cont->data[i + j]);
				return -ERANGE;
			}
		}
		i += len;
		start_addr += len;
	}
	return 0;
}
#endif

static int cdv2625_ram_data_write(struct haptic *cdv2625, struct haptic_container *cs_cont)
{
	uint16_t i = 0;
	uint16_t start_address = 0;
	uint8_t addr_fail_cnt = 0;
	uint8_t data_fail_cnt = 0;
	uint16_t len = 0;
	uint8_t ram_data[4] = {0};
	int i2c_err = 0;

	log_info("enter");
	start_address = cdv2625->ram.base_addr;
	i = cdv2625->ram.ram_shift;
	while (i < cs_cont->len) {
		if ((data_fail_cnt > CS_I2C_RETRY_TIMES) || (addr_fail_cnt > CS_I2C_RETRY_TIMES)) {
			log_err("ram data write error\n");
			break;
		}
		if ((cs_cont->len - i) < CDV2625_RAM_SINGLE_WRITE_LEN)
			len = cs_cont->len - i;
		else
			len = CDV2625_RAM_SINGLE_WRITE_LEN;
		if (cdv2625_wave_addr_set(cdv2625, start_address) < 0) {
			addr_fail_cnt++;
			continue;
		}
		if (cdv2625_wave_data_set(cdv2625, &cs_cont->data[i], len) < 0) {
			data_fail_cnt++;
			continue;
		}
		if (cs_i2c_byte_read(cdv2625, CDV2625_DATA_CTRL, ram_data, 4) >= 0) {
			if (ram_data[0] != 0 || ram_data[3] != 0x11)
				i2c_err = -1;
		} else {
			i2c_err = -1;
		}
		if (i2c_err == -1) {
			log_info("i2c err\n");
			break;
		}
		i += len;
		start_address += len;
	}
	return i2c_err;
}

/* Initializes RAM wave library */
int cdv2625_container_update(struct haptic *cdv2625, struct haptic_container *cs_cont)
{
	int ret = 0;

	log_info("enter");
	cdv2625->ram.baseaddr_shift = CS_RAMLIB_BASEADDR_OFFSET;
	cdv2625->ram.ram_shift = CS_RAMLIB_DATA_OFFSET;

	cdv2625_stop(cdv2625);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	cdv2625->ram.base_addr = (uint16_t)cs_cont->data[cdv2625->ram.baseaddr_shift] << 8 |
		cs_cont->data[cdv2625->ram.baseaddr_shift + 1];
	cdv2625_sram_base_addr_set(cdv2625, cdv2625->ram.base_addr);

	if (cdv2625_ram_data_write(cdv2625, cs_cont) < 0) {
		cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
		return -1;
	}

#ifdef CHECK_RAM_DATA
	ret = cdv2625_check_ram_data(cdv2625, cs_cont);
	if (ret < 0)
		log_info("check ramdata error");
#endif
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
	return ret;
}

static void cdv2625_protect_config(struct haptic *cs_haptic, uint8_t addr, uint8_t val)
{
	log_info("enter");

	if (addr == PROTECT_ENABLE)
		cdv2625_pwm_in_prot_enable_ctrl(cs_haptic, CS_ENABLE);
	else if (addr == PROTECT_DISABLE)
		cdv2625_pwm_in_prot_enable_ctrl(cs_haptic, CS_DISABLE);
	else if (addr == PROTECT_SET_VOL)
		cdv2625_pwm_in_prot_vol_set(cs_haptic, val);
	else
		cdv2625_pwm_in_prot_enable_ctrl(cs_haptic, CS_DISABLE);
}

static void cdv2625_misc_para_init(struct haptic *cdv2625)
{
	uint8_t temp_value = 0;

	cdv2625_sram_base_addr_set(cdv2625, CS_BASE_ADDR);
	cdv2625->ram.base_addr = CS_BASE_ADDR;
	cdv2625_fifo_thr_set(cdv2625, cdv2625->ram.base_addr >> 1,
		cdv2625->ram.base_addr - (cdv2625->ram.base_addr >> 2));
	cdv2625->info.bst_vol_ram = CS_DEFAULT_BOOST_VOL;
	cdv2625->info.bst_vol_rtp = CS_DEFAULT_BOOST_VOL;
	cdv2625_interrupt_enable_ctrl(cdv2625, CDV_INTR_FF_AFS, CS_ENABLE);
	cdv2625_f0_close_cyc_set(cdv2625, F0_CLOSE_CYCLE);
	cs_i2c_one_byte_write(cdv2625, CDV2625_PAD_CTRL, NO_PAD_DEGLITCH);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL, CONT_BEMF_TIME);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL19, CONT_BEMF_STARTUP_TIME);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL18, CONT_BRK_THRESHOLD);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL17, CONT_BEMF_TRACK_TIME);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL4, CONT_BRK_GAIN);
	cs_i2c_one_byte_write(cdv2625, CDV2625_BST_CFG2, BST_CFG);
	cs_i2c_one_byte_write(cdv2625, CDV2625_ANA_CTRL, LOW_POWER_CFG);
	cs_i2c_one_byte_write(cdv2625, CDV2625_CONT_CTRL10, LONG_RAM_MAX_BRK_VOL);
	cdv2625_adc_flt_coe_config(cdv2625, RC_FLT_COE_ONE_SIXTEEN);
	cdv2625_pga_gain_config(cdv2625, PGA_GAIN_10TIMES);
	cdv2625_max_brake_cycle_set(cdv2625, HAPTIC_BREAK_CYCLE);
	cdv2625_extra_brk_enable_ctrl(cdv2625, CS_ENABLE);
	if (cdv2625->info.bst_vol_default <= RAM_BOOST_VOL_9V)
		cdv2625->vmax = cdv2625->info.bst_vol_default;
	else
		cdv2625->vmax = RAM_BOOST_VOL_9V;

	cdv2625_pvdd_out_vol_set(cdv2625, cdv2625->vmax);
	cdv2625_pre_wave(cdv2625);
	cdv2625_perset_f0_set(cdv2625, (uint16_t)(F0_CONV_FACTOR / cdv2625->info.f0_pre));
}

static void cdv2625_set_rtp_aei(struct haptic *cs_haptic, bool flag)
{
	log_info("enter");
	if (flag)
		cdv2625_interrupt_enable_ctrl(cs_haptic, CDV_INTR_FF_AES, CS_ENABLE);
	else
		cdv2625_interrupt_enable_ctrl(cs_haptic, CDV_INTR_FF_AES, CS_DISABLE);
}

static ssize_t cdv2625_get_reg(struct haptic *cs_haptic, ssize_t len, char *buf)
{
	uint8_t i = 0;
	uint8_t reg_val = 0;

	log_info("enter");
	for (i = 0; i < CDV2625_REG_MAX; i++) {
		if (!(cdv_reg_access[i] & REG_RD_ACCESS))
			continue;
		cs_i2c_byte_read(cs_haptic, i, &reg_val, 0x01);
		len += snprintf(buf + len, PAGE_SIZE - len, "reg:0x%02x=0x%02x\n", i, reg_val);
	}
	return len;
}

static uint8_t cdv2625_get_prctmode(struct haptic *cs_haptic)
{
	uint8_t reg_val = 0;

	log_info("enter");
	cs_i2c_byte_read(cs_haptic, CDV2625_DPWM_CTRL3, &reg_val, CS_I2C_BYTE_ONE);
	reg_val &= 0x80;
	return reg_val;
}

ssize_t cdv2625_get_ram_data(struct haptic *cs_haptic, char *buf)
{
	int i = 0;
	int j = 0;
	uint32_t len = 0;
	int size = 0;
	uint8_t ram_data[CS_RAMDATA_RD_BUFFER_SIZE] = {0};

	if (buf == NULL)
		return 0;
	cdv2625_active_mode_set(cs_haptic, CDV_MODE_ACTIVE);
	while (i < cs_haptic->ram.len) {
		cdv2625_wave_addr_set(cs_haptic, cs_haptic->ram.base_addr + i);
		if ((cs_haptic->ram.len - i) < CDV2625_RAM_SINGLE_WRITE_LEN)
			size = cs_haptic->ram.len - i;
		else
			size = CDV2625_RAM_SINGLE_WRITE_LEN;
		cs_i2c_byte_read(cs_haptic, CDV2625_WAVE_DATA, ram_data, size);
		for (j = 0; j < size; j++)
			len += snprintf(buf + len, PAGE_SIZE - len, "0x%02X,", ram_data[j]);
		i += size;
	}
	cdv2625_active_mode_set(cs_haptic, CDV_MODE_STANDBY);
	return len;
}

static void cdv2625_get_first_wave_addr(struct haptic *cs_haptic, uint8_t *wave_addr)
{
	uint8_t reg_array[3] = {0};

	log_info("enter");
	cdv2625_active_mode_set(cs_haptic, CDV_MODE_ACTIVE);
	cdv2625_wave_addr_set(cs_haptic, cs_haptic->ram.base_addr);
	cs_i2c_byte_read(cs_haptic, CDV2625_WAVE_DATA, reg_array, 3);
	cdv2625_active_mode_set(cs_haptic, CDV_MODE_STANDBY);
	wave_addr[0] = reg_array[1];
	wave_addr[1] = reg_array[2];
}

static uint8_t cdv2625_rtp_get_fifo_afs(struct haptic *cs_haptic)
{
	uint8_t reg_val = 0;

	log_info("enter");
	reg_val = cdv2625_flag_status_get(cs_haptic, CDV_FLAG_FF_AFS);
	return reg_val;
}

static uint8_t cdv2625_rtp_get_fifo_afi(struct haptic *cs_haptic)
{
	uint8_t reg_val = 0;

	reg_val = cdv2625_interrupt_status_get(cs_haptic, CDV2625_BIT_SYSINT_FF_AFI);
	return reg_val;
}

static void cdv2625_get_wav_seq(struct haptic *cs_haptic, uint32_t len)
{
	uint8_t i = 0;
	uint8_t reg_val[CS_SEQUENCER_SIZE] = {0};

	log_info("enter");
	if (len > CS_SEQUENCER_SIZE)
		len = CS_SEQUENCER_SIZE;
	cs_i2c_byte_read(cs_haptic, CDV2625_WAV_IDX1, reg_val, len);
	for (i = 0; i < len; i++)
		cs_haptic->seq[i] = reg_val[i];
}


static void cdv2625_read_f0(struct haptic *cs_haptic)
{
	uint16_t f0_free_reg = 0;
	unsigned long f0_tmp = 0;

	log_info("enter");
	f0_free_reg = cdv2625_free_f0_get(cs_haptic);
	if (!f0_free_reg) {
		log_err("lra_f0 is error, f0_reg=0");
		return;
	}
	f0_tmp = F0_CONV_FACTOR / (uint32_t)f0_free_reg; // 0.1hz
	cs_haptic->f0 = (uint32_t)f0_tmp;
	log_info("lra_f0=%u", cs_haptic->f0);
	f0_free_reg = cdv2625_cont_driver_f0_get(cs_haptic);
	if (!f0_free_reg) {
		log_err("cont_f0 is error, f0_reg=0");
		return;
	}
	f0_tmp = F0_CONV_FACTOR / (uint32_t)f0_free_reg; // 0.1hz
	cs_haptic->cont_f0 = (uint32_t)f0_tmp;
	log_info("cont_f0=%u", cs_haptic->cont_f0);
}

static int cdv2625_free_f0_detect(struct haptic *cdv2625)
{
	uint8_t gain_val = 0;
	uint8_t vddmode_val = 0;

	log_info("enter");
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
	cs_i2c_byte_read(cdv2625, 0x4c, &vddmode_val, 0x01);
	cs_i2c_byte_read(cdv2625, 0x07, &gain_val, 0x01);

	cdv2625_cont_play_end_mode_config(cdv2625, CONT_PLAY_TIME);
	cdv2625_vdd_det_mode_config(cdv2625, WAVE_PLAY_BEFORE_VDD_DET);
	cdv2625_vdd_gain_adj_mode_config(cdv2625, VDD_GAIN_ADJ_SOFTWARE);
	cdv2625_wave_play_mode_set(cdv2625, CONT_MODE);
	cdv2625_set_gain(cdv2625, HAPTIC_GAIN_FOR_F0);

	cdv2625_closed_loop_enable_ctrl(cdv2625, CS_DISABLE);
	cdv2625_pow_on_f0_detect_enable_ctrl(cdv2625, CS_ENABLE);
	cdv2625_f0_track_config(cdv2625, HAPTIC_F0_OPEN_CYCLE, CS_DISABLE, HAPTIC_F0_CHECK_TIMES - 1);
	cdv2625_adc_flt_coe_config(cdv2625, RC_FLT_COE_ONE_SIXTEEN);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);

	cdv2625_wave_play_enable_ctrl(cdv2625, CS_ENABLE);
	if (cdv2625_check_status_wait(cdv2625, CDV_FLAG_DONES, CS_SET, F0_WAIT_TIME_MS)) {
		cdv2625->cont_f0 = 0;
		cdv2625->f0 = 0;
		cdv2625_soft_rst(cdv2625);
		log_info("get f0 err");
	} else {
		cdv2625_read_f0(cdv2625);
	}
	cs_i2c_byte_write(cdv2625, 0x07, &gain_val, 0x01);
	cs_i2c_byte_write(cdv2625, 0x4c, &vddmode_val, 0x01);
	cdv2625_vdd_gain_adj_mode_config(cdv2625, vddmode_val);
	cdv2625_wave_play_mode_set(cdv2625, RAM_MODE);

	return 0;
}

static int cdv2625_get_irq_state(struct haptic *cs_haptic)
{
	int ret = -1;
	uint8_t reg_val = 0;

	log_info("enter");
	cs_i2c_byte_read(cs_haptic, CDV2625_SYS_INTR, &reg_val, 0x01);
	log_info("reg SYSINT = 0x%02X", reg_val);

	if (reg_val & CDV_INTR_BST_ERRS)
		cs_haptic->rtp_routine_on = 0;
	if (reg_val & CDV_INTR_UVLOS) {
		cs_haptic->rtp_routine_on = 0;
		log_err("chip uvlo int error");
	}
	if (reg_val & CDV_INTR_OCS) {
		cs_haptic->rtp_routine_on = 0;
		log_err("chip over current int error");
	}
	if (reg_val & CDV_INTR_OTS) {
		cs_haptic->rtp_routine_on = 0;
		log_err("chip over temperature int error");
	}
	if (reg_val & CDV_INTR_DONES) {
		cs_haptic->rtp_routine_on = 0;
		log_err("chip playback done");
	}
	if (reg_val & CDV_INTR_FF_AES) {
		ret = 0;
		log_info("rtp mode fifo almost empty!");
	}
	if (reg_val & CDV_INTR_FF_AFS)
		log_info("rtp mode fifo almost full!");
	return ret;
}

static int cdv2625_juge_rtp_going(struct haptic *cs_haptic)
{
	uint8_t glb_state = 0;
	uint8_t rtp_state = 0;

	glb_state = cdv2625_global_state_get(cs_haptic);
	if (cs_haptic->rtp_routine_on || (glb_state == CDV_GLB_STATUS_RTP)) {
		rtp_state = 1; /* is going on */
		log_info("rtp_routine_on");
	}
	return rtp_state;
}

static void cdv2625_vbat_check(struct haptic *cdv2625)
{
	unsigned int vdd_val = 0;

	log_info("enter");
	cdv2625_vdd_det_mode_config(cdv2625, WAVE_PLAY_BEFORE_VDD_DET);

	cdv2625_det_mode_config(cdv2625, DET_MODE_VDD);
	cdv2625_adc_clk_config(cdv2625, ADC_CLK_FRQ_3HZ072);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	cdv2625_det_enable_ctrl(cdv2625, CS_ENABLE);
	cdv2625_det_go_wait(cdv2625, CS_DISABLE, DET_VDD_TIME_MS);
	vdd_val = cdv2625_adc_average_get(cdv2625);
	vdd_val = vdd_val * 238 / 100;
	cdv2625->vbat = vdd_val;
	if (cdv2625->vbat > CS_VBAT_MAX) {
		cdv2625->vbat = CS_VBAT_MAX;
		log_info("vbat max limit = %umV", cdv2625->vbat);
	}
	if (cdv2625->vbat < CS_VBAT_MIN) {
		cdv2625->vbat = CS_VBAT_MIN;
		log_info("vbat min limit = %umV", cdv2625->vbat);
	}
	log_info("vbat = %umV, vdd_reg = 0x%02X", cdv2625->vbat, vdd_val);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
}

int16_t cdv2625_os_detect(struct haptic *cdv2625)
{
	uint8_t temp_value = 0x10;
	uint8_t temp_pga = 0;
	uint16_t temp_adc_val = 0;
	int16_t temp_os_val = 0;

	cdv2625_det_enable_ctrl(cdv2625, CS_DISABLE);
	cs_i2c_byte_read(cdv2625, 0x4e, &temp_pga, 1);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	temp_value = 0x50;
	cs_i2c_byte_write(cdv2625, 0x63, &temp_value, 0x01);
	temp_value = 0x14;
	cs_i2c_byte_write(cdv2625, 0x4E, &temp_value, 0x01);
	temp_value = 0x03;
	cs_i2c_byte_write(cdv2625, 0x4D, &temp_value, 0x01);
	cdv2625_det_go_wait(cdv2625, CS_DISABLE, DET_OS_TIME_MS);
	temp_value = temp_pga;
	cs_i2c_byte_write(cdv2625, 0x4E, &temp_value, 0x01);
	temp_adc_val = cdv2625_adc_average_get(cdv2625);
	if (temp_adc_val >= 0x800)
		temp_os_val = (-1) * (((~temp_adc_val) + 1) & 0x0fff);
	else
		temp_os_val = (int16_t)temp_adc_val;
	return temp_os_val;
}

static void cdv2625_resistance_detect(struct haptic *cdv2625)
{
	uint8_t temp = 0;
	uint8_t gain = 0;
	int16_t os_value = 0;
	uint16_t rl_value = 0;
	uint32_t res = 0;
	uint8_t temp_pga = 0;

	log_info("enter");
	mutex_lock(&cdv2625->lock);
	cdv2625_vdd_gain_adj_mode_config(cdv2625, VDD_GAIN_ADJ_SOFTWARE);
	/* OS detection. */
	os_value = cdv2625_os_detect(cdv2625);
	cdv2625_vbat_check(cdv2625);
	/* RL detection. */
	cs_i2c_byte_read(cdv2625, 0x4e, &temp_pga, 1);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);

	cdv2625_pga_gain_config(cdv2625, PGA_GAIN_10TIMES);
	cdv2625_det_enable_ctrl(cdv2625, CS_DISABLE);
	cdv2625_det_mode_config(cdv2625, DET_MODE_RL);
	temp = 0x60;
	cs_i2c_byte_write(cdv2625, 0x63, &temp, 0x01);
	temp = 0x01;
	cs_i2c_byte_write(cdv2625, 0x61, &temp, 0x01);
	cdv2625_det_enable_ctrl(cdv2625, CS_ENABLE);
	cdv2625_delay_us(100000);
	gain = cdv2625_pga_gain_get(cdv2625);
	rl_value = cdv2625_adc_average_get(cdv2625);
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
	temp = 0x00;
	cs_i2c_byte_write(cdv2625, 0x61, &temp, 0x01);
	cdv2625_vdd_gain_adj_mode_config(cdv2625, VDD_GAIN_ADJ_HARDWARE);
	cs_i2c_byte_write(cdv2625, 0x4e, &temp_pga, 0x01);
	res = (rl_value - os_value) * 60 / gain;
	cdv2625->lra = res * 10;
	log_info("cdv2625->lra = %u rl_value = %u os_value = %d", cdv2625->lra, rl_value, os_value);
	mutex_unlock(&cdv2625->lock);
}

static void cdv2625_set_pwm(struct haptic *cs_haptic, uint8_t mode)
{
	log_info("enter");
	switch (mode) {
	case CS_PWM_48K:
		cdv2625_up_sample_rate_config(cs_haptic, UP_SP_RATE_1);
		break;
	case CS_PWM_24K:
		cdv2625_up_sample_rate_config(cs_haptic, UP_SP_RATE_2);
		break;
	case CS_PWM_12K:
		cdv2625_up_sample_rate_config(cs_haptic, UP_SP_RATE_4);
		break;
	default:
		log_err("error param!");
		break;
	}
	cdv2625_pwm_sample_rate_config(cs_haptic, PWM_SAMPLE_48K);
}

static void cdv2625_upload_lra(struct haptic *cdv2625, uint32_t flag)
{
	log_info("enter");

	switch (flag) {
	case CS_F0_CALI_DEFAULT:
		log_info("write default");
		cdv2625_f0_osc_adj_set(cdv2625, 0);
		break;
	case CS_F0_CALI_LRA:
		log_info("write cali to trim_lra = 0x%x", cdv2625->f0_cali_data);
		cdv2625_f0_osc_adj_set(cdv2625, cdv2625->f0_cali_data);
		break;
	default:
		log_err("error param ");
		break;
	}
}

void cdv2625_cont_config(struct haptic *cdv2625)
{
	log_info("enter");
	/* work mode */
	cdv2625_active_mode_set(cdv2625, CDV_MODE_STANDBY);
	cdv2625_wave_play_mode_set(cdv2625, CONT_MODE);
	cdv2625_pow_on_f0_detect_enable_ctrl(cdv2625, CS_DISABLE);
	cdv2625_cont_play_end_mode_config(cdv2625, CONT_PLAY_GO);
	/* cont play go */
	cdv2625_active_mode_set(cdv2625, CDV_MODE_ACTIVE);
	cdv2625_wave_play_enable_ctrl(cdv2625, CS_ENABLE);
}

static void cdv2625_irq_clear(struct haptic *cdv2625)
{
	uint8_t reg_val = 0;

	cs_i2c_byte_read(cdv2625, CDV2625_SYS_INTR, &reg_val, 0x01);
	log_info("reg SYSINT=0x%02X\n", reg_val);
}

static void cdv2625_interrupt_setup(struct haptic *cdv2625)
{
	/* edge int mode */
	cdv2625_intr_pin_config(cdv2625, PIN_OUT_OD, INTR_PIN_TIRG_FALLING);
	/* int enable */
	cdv2625_interrupt_enable_ctrl(cdv2625, CDV_INTR_DPWMS, CS_DISABLE);
	cdv2625_interrupt_enable_ctrl(cdv2625, CDV_INTR_UVLOS, CS_DISABLE);
	cdv2625_interrupt_enable_ctrl(cdv2625, CDV_INTR_OCS, CS_DISABLE);
	cdv2625_interrupt_enable_ctrl(cdv2625, CDV_INTR_OTS, CS_DISABLE);
}

/*****************************************************
 *
 * device tree
 *
 *****************************************************/
static void cdv2625_parse_dt(struct device *dev, struct haptic *cs_haptic, struct device_node *np)
{
	uint8_t trig_config_temp[24] = {1, 0, 1, 1, 1, 2, 0, 0, 1, 0, 0, 1, 0, 2, 0, 0, 1, 0, 0, 1, 0, 2, 0, 0};
	uint32_t val = 0;

	val = of_property_read_u8(np, "max_bst_vol", &cs_haptic->info.max_bst_vol);
	if (val != 0)
		log_info("max_bst_vol not found");
	val = of_property_read_u8(np, "mode", &cs_haptic->info.mode);
	if (val != 0)
		log_info("vib_mode not found");

	val = of_property_read_u32(np, "f0_pre", &cs_haptic->info.f0_pre);
	if (val != 0)
		log_info("vib_f0_pre not found");
	val = of_property_read_u8(np, "f0_cali_percent", &cs_haptic->info.f0_cali_percent);
	if (val != 0)
		log_info("vib_f0_cali_percent not found");
	val = of_property_read_u8_array(np, "trig_config", trig_config_temp, ARRAY_SIZE(trig_config_temp));
	if (val != 0)
		log_info("vib_trig_config not found");
	memcpy(cs_haptic->info.trig_cfg, trig_config_temp, sizeof(trig_config_temp));
	val = of_property_read_u8(np, "bst_vol_default", &cs_haptic->info.bst_vol_default);
	if (val != 0)
		log_info("vib_bst_vol_default not found");
	val = of_property_read_u8(np, "bst_vol_ram", &cs_haptic->info.bst_vol_ram);
	if (val != 0)
		log_info("vib_bst_vol_ram not found");
	val = of_property_read_u8(np, "bst_vol_rtp", &cs_haptic->info.bst_vol_rtp);
	if (val != 0)
		log_info("vib_bst_vol_rtp not found");
	cs_haptic->info.is_enabled_auto_bst = of_property_read_bool(np, "is_enabled_auto_bst");
	log_info("is_enabled_auto_bst = %d", cs_haptic->info.is_enabled_auto_bst);
	cs_haptic->info.is_enabled_one_wire = of_property_read_bool(np, "is_enabled_one_wire");
	log_info("is_enabled_one_wire = %d", cs_haptic->info.is_enabled_one_wire);
	cs_haptic->info.is_enabled_richtap_core = of_property_read_bool(np,
		"cdv2625_is_enabled_richtap_core");
	log_info("cs_haptic->info.cdv2625_is_enabled_richtap_core = %u",
		cs_haptic->info.is_enabled_richtap_core);
}

static size_t cdv2625_get_wav_loop(struct haptic *cs_haptic, char *buf)
{
	uint8_t i = 0;
	uint8_t reg_val[CS_SEQUENCER_LOOP_SIZE] = {0};
	size_t count = 0;

	cs_i2c_byte_read(cs_haptic, CDV2625_WAVLOOP1, reg_val, CS_SEQUENCER_LOOP_SIZE);

	for (i = 0; i < CS_SEQUENCER_LOOP_SIZE; i++) {
		cs_haptic->loop[i * 2 + 0] = (reg_val[i] >> 4) & 0x0F;
		cs_haptic->loop[i * 2 + 1] = (reg_val[i] >> 0) & 0x0F;
		count += snprintf(buf + count, PAGE_SIZE - count, "seq%d loop: 0x%02x\n",
						  i * 2 + 1, cs_haptic->loop[i * 2 + 0]);
		count += snprintf(buf + count, PAGE_SIZE - count, "seq%d loop: 0x%02x\n",
						i * 2 + 2, cs_haptic->loop[i * 2 + 1]);
	}
	return count;
}

static void cdv2625_play_go(struct haptic *cs_haptic, bool flag)
{
	log_info("enter");
	if (flag) {
		cdv2625_active_mode_set(cs_haptic, CDV_MODE_ACTIVE);
		cdv2625_wave_play_enable_ctrl(cs_haptic, CS_ENABLE);
		cs_haptic->chip_ipara.kpre_time = ktime_get();
	} else {
		cs_haptic->chip_ipara.kcur_time = ktime_get();
		cs_haptic->chip_ipara.interval_us =
			ktime_to_us(ktime_sub(cs_haptic->chip_ipara.kcur_time, cs_haptic->chip_ipara.kpre_time));
		if (cs_haptic->chip_ipara.interval_us < 2000) {
			log_info("cs_haptic->interval_us = %u < 2000", cs_haptic->chip_ipara.interval_us);
			usleep_range(1000, 1200);
		}
		cdv2625_wave_play_enable_ctrl(cs_haptic, CS_DISABLE);
	}
}

static uint16_t cdv2625_get_trim_osc(struct haptic *cs_haptic)
{
	uint16_t reg = 0;
	uint8_t reg_val[2] = {0};

	cs_i2c_byte_read(cs_haptic, CDV2625_TRIM_OSC, reg_val, 2);
	reg = reg_val[0] & 0x07;
	reg = (reg << 8) | reg_val[1];
	log_info("trim osc: %04x", reg);
	return reg;
}

struct haptic_func cdv2625_func_list = {
	.play_stop = cdv2625_stop,
	.ram_init = cdv2625_ram_init,
	.get_vbat = cdv2625_vbat_check,
	.get_f0 = cdv2625_free_f0_detect,
	.cont_config = cdv2625_cont_config,
	.read_f0 = cdv2625_read_f0,
	.get_irq_state = cdv2625_get_irq_state,
	.check_qualify = cdv2625_chip_qualify_check,
	.juge_rtp_going = cdv2625_juge_rtp_going,
	.get_lra_resistance = cdv2625_resistance_detect,
	.set_pwm = cdv2625_set_pwm,
	.play_mode = cdv2625_playmode,
	.set_bst_vol = cdv2625_pvdd_out_vol_set,
	.interrupt_setup = cdv2625_interrupt_setup,
	.set_repeat_seq = cdv2625_set_repeat_seq,
	.auto_bst_enable = cdv2625_auto_bst_enable_ctrl,
	.vbat_mode_config = cdv2625_vbat_mode_config,
	.set_wav_seq = cdv2625_wave_id_set,
	.set_wav_loop = cdv2625_wave_loop_set,
	.set_ram_addr = cdv2625_set_ram_addr,
	.set_rtp_data = cdv2625_set_rtp_data,
	.container_update = cdv2625_container_update,
	.protect_config = cdv2625_protect_config,
	.parse_dt = cdv2625_parse_dt,
	.irq_clear = cdv2625_irq_clear,
	.get_wav_loop = cdv2625_get_wav_loop,
	.play_go = cdv2625_play_go,
	.misc_para_init = cdv2625_misc_para_init,
	.set_rtp_aei = cdv2625_set_rtp_aei,
	.set_gain = cdv2625_set_gain,
	.get_gain = cdv2625_get_gain,
	.upload_lra = cdv2625_upload_lra,
	.get_reg = cdv2625_get_reg,
	.get_prctmode = cdv2625_get_prctmode, // null
	.get_ram_data = cdv2625_get_ram_data,
	.get_first_wave_addr = cdv2625_get_first_wave_addr,
	.get_glb_state = cdv2625_global_state_get,
	.rtp_get_fifo_afs = cdv2625_rtp_get_fifo_afs,
	.get_wav_seq = cdv2625_get_wav_seq,
	.get_trim_osc = cdv2625_get_trim_osc,
	.enable_gain = cdv2625_gain_switch_enable_ctrl,
	.rtp_get_fifo_afi = cdv2625_rtp_get_fifo_afi,
	.fifo_reset = cdv2625_fifo_reset,
};

struct haptic_func *cdv2625_func(void)
{
	return &cdv2625_func_list;
}

