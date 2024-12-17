/*
  * sih688x.c
  *
  * code for vibrator
  *
  * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/power_supply.h>
#include <linux/vmalloc.h>
#include <linux/pm_qos.h>

#include "sih688x_reg.h"
#include "sih688x.h"
#include "haptic.h"
#include "haptic_mid.h"
#include "haptic_regmap.h"

sih_chipid_reg_val_t sih688x_chip_id_list[] = {
	{SIH688X_CHIPID_REG_ADDR, SIH688X_CHIPID_REG_VALUE},
};

static void cont_config_seq0(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_seq1(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_seq2(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_asmooth(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_th_len(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_th_num(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_ampli(sih_haptic_t *sih_haptic, uint32_t value);
static void cont_config_go(sih_haptic_t *sih_haptic, uint32_t value);

cont_config_fp cont_config_sih688x[] = {
	cont_config_seq0,
	cont_config_seq1,
	cont_config_seq2,
	cont_config_asmooth,
	cont_config_th_len,
	cont_config_th_num,
	cont_config_ampli,
	cont_config_go
};

static void sih688x_ram_init(
	sih_haptic_t *sih_haptic,
	bool flag)
{
	hp_info("%s: enter!\n", __func__);
	if (flag == true) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_ENRAMINIT_MASK,
			SIH_SYSCTRL1_BIT_RAMINIT_EN);
	} else {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_ENRAMINIT_MASK,
			SIH_SYSCTRL1_BIT_RAMINIT_OFF);
	}
}

static void sih688x_set_go_enable(
	sih_haptic_t *sih_haptic,
	unsigned char play_mode)
{
	hp_info("%s: enter! go mode %d\n", __func__, play_mode);
	switch (play_mode) {
	case SIH_HAPTIC_RAM_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RAM_GO_MASK,
			SIH_GO_BIT_RAM_GO_ENABLE);
		break;
	case SIH_HAPTIC_RTP_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RTP_GO_MASK,
			SIH_GO_BIT_RTP_GO_ENABLE);
		break;
	case SIH_HAPTIC_CONT_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_F0_SEQ_GO_MASK,
			SIH_GO_BIT_F0_SEQ_GO_ENABLE);
		break;
	case SIH_HAPTIC_RAM_LOOP_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RAM_GO_MASK,
			SIH_GO_BIT_RAM_GO_ENABLE);
		break;
	default:
		hp_err("%s: play mode %d no need to go\n",
			__func__, sih_haptic->chip_ipara.play_mode);
		break;
	}
}

static void sih688x_set_go_disable(
	sih_haptic_t *sih_haptic,
	unsigned char play_mode)
{
	hp_info("%s: enter!\n", __func__);
	switch (play_mode) {
	case SIH_HAPTIC_RAM_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RAM_GO_MASK,
			SIH_GO_BIT_RAM_GO_DISABLE);
		break;
	case SIH_HAPTIC_RTP_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RTP_GO_MASK,
			SIH_GO_BIT_RTP_GO_DISABLE);
		break;
	case SIH_HAPTIC_CONT_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_F0_SEQ_GO_MASK,
			SIH_GO_BIT_F0_SEQ_GO_DISABLE);
		break;
	case SIH_HAPTIC_RAM_LOOP_MODE:
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_GO_BIT_RAM_GO_MASK,
			SIH_GO_BIT_RAM_GO_DISABLE);
		break;
	default:
		hp_err("%s: play mode %d no need to go\n",
			__func__, sih_haptic->chip_ipara.play_mode);
		break;
	}
}

static void sih688x_play_go(
	sih_haptic_t *sih_haptic,
	bool flag)
{
	hp_info("%s: enter, play_mode = %d, flag = %d\n",
		__func__, sih_haptic->chip_ipara.play_mode, flag);
	if (flag) {
		sih688x_set_go_enable(sih_haptic,
			sih_haptic->chip_ipara.play_mode);
		sih_haptic->chip_ipara.kpre_time = ktime_get();
	} else {
		sih_haptic->chip_ipara.kcur_time = ktime_get();
		sih_haptic->chip_ipara.interval_us =
			ktime_to_us(ktime_sub(sih_haptic->chip_ipara.kcur_time,
			sih_haptic->chip_ipara.kpre_time));
		if (sih_haptic->chip_ipara.interval_us < 2000) {
			hp_info("%s: sih688x->interval_us = %d < 2000\n",
				__func__, sih_haptic->chip_ipara.interval_us);
			usleep_range(1000, 1200);
		}
		sih688x_set_go_disable(sih_haptic,
			sih_haptic->chip_ipara.play_mode);
	}
}

#ifdef SIH_CHECK_RAM_DATA
static int sih688x_check_ram_data(
	struct sih_haptic *sih_haptic,
	uint8_t *cont_data,
	uint8_t *ram_data,
	uint32_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (ram_data[i] != cont_data[i]) {
			hp_err("%s:check err,addr=0x%02x,ram=0x%02x,file=0x%02x\n",
				__func__, i, ram_data[i], cont_data[i]);
			return -ERANGE;
		}
	}
	return 0;
}
#endif

static int sih688x_update_ram_config(
	sih_haptic_t *sih_haptic,
	haptic_container_t *sih_cont)
{
	uint8_t fifo_addr[4] = {0};
	uint32_t shift = 0;
	int i = 0;
	int len = 0;
	int ret = 0;
#ifdef SIH_CHECK_RAM_DATA
	char *ram_data = NULL;
#endif

	hp_info("%s: enter\n", __func__);

	mutex_lock(&sih_haptic->lock);

	sih_haptic->ram_para.baseaddr_shift = 2;
	sih_haptic->ram_para.ram_shift = 4;
	/* RAMINIT Enable */
	sih688x_ram_init(sih_haptic, true);

	/* base addr */
	shift = sih_haptic->ram_para.baseaddr_shift;
	sih_haptic->ram_para.base_addr =
		(uint32_t)(sih_cont->data[0 + shift] << 8) |
		(sih_cont->data[1 + shift]);
	fifo_addr[0] =
		(uint8_t)SIH_FIFO_AF_ADDR_L(sih_haptic->ram_para.base_addr);
	fifo_addr[1] =
		(uint8_t)SIH_FIFO_AE_ADDR_L(sih_haptic->ram_para.base_addr);
	fifo_addr[2] =
		(uint8_t)SIH_FIFO_AF_ADDR_H(sih_haptic->ram_para.base_addr);
	fifo_addr[3] =
		(uint8_t)SIH_FIFO_AE_ADDR_H(sih_haptic->ram_para.base_addr);

	hp_info("%s: base_addr = 0x%04x\n", __func__,
		sih_haptic->ram_para.base_addr);
	hp_info("%s: fifo_addr[0] = %d,fifo_addr[1] = %d\n",
		__func__, fifo_addr[0], fifo_addr[1]);
	hp_info("%s: fifo_addr[2] = %d,fifo_addr[3] = %d\n",
		__func__, fifo_addr[2], fifo_addr[3]);

	haptic_regmap_bulk_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_BASE_ADDRH,
		SIH_I2C_OPERA_BYTE_TWO,
		&sih_cont->data[shift]);
	haptic_regmap_bulk_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RTPCFG1,
		SIH_I2C_OPERA_BYTE_TWO,
		fifo_addr);
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RTPCFG3,
		SIH_RTPCFG3_BIT_FIFO_AFH_MASK,
		(fifo_addr[2]<<4));
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RTPCFG3,
		SIH_RTPCFG3_BIT_FIFO_AEH_MASK,
		fifo_addr[3]);

	/* ram */
	haptic_regmap_bulk_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RAMADDRH,
		SIH_I2C_OPERA_BYTE_TWO,
		&sih_cont->data[shift]);

	i = sih_haptic->ram_para.ram_shift;

	if (sih_cont->len > SIH_RAMDATA_WR_BUFFER_SIZE)
		sih_cont->len = SIH_RAMDATA_WR_BUFFER_SIZE;

	while (i < sih_cont->len) {
		if ((sih_cont->len - i) <= SIH_RAMDATA_WRITE_SIZE)
			len = sih_cont->len - i;
		else
			len = SIH_RAMDATA_WRITE_SIZE;

		ret = haptic_regmap_write(sih_haptic->regmapp.regmapping,
			SIH688X_REG_RAMDATA,
			len,
			&sih_cont->data[i]);
		if (ret < 0)
			hp_err("%s: write ram data err=%d\n",
				__func__, ret);
		i += len;
	}

#ifdef SIH_CHECK_RAM_DATA
	sih688x_ram_init(sih_haptic, false);
	sih688x_ram_init(sih_haptic, true);

	i = sih_haptic->ram_para.ram_shift;
	ram_data = vmalloc(SIH_RAMDATA_RD_BUFFER_SIZE);
	if (!ram_data) {
		hp_err("%s: ram_data vmalloc failed\n", __func__);
	} else {
		while (i < sih_cont->len) {
			if ((sih_cont->len - i) <= SIH_RAMDATA_READ_SIZE)
				len = sih_cont->len - i;
			else
				len = SIH_RAMDATA_READ_SIZE;

			haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_RAMDATA,
				len,
				ram_data);
			ret = sih688x_check_ram_data(sih_haptic,
				&sih_cont->data[i], ram_data, len);
			if (ret < 0)
				break;
			i += len;
		}
		if (ret)
			hp_err("%s: ram data check sum error\n", __func__);
		else
			hp_info("%s: ram data check sum pass\n", __func__);

		vfree(ram_data);
	}
#endif

	/* RAMINIT Disable */
	sih688x_ram_init(sih_haptic, false);

	mutex_unlock(&sih_haptic->lock);

	return ret;
}

static void sih688x_clear_interrupt_state(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINT,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	hp_info("%s: reg SYSINT = 0x%02x\n", __func__, reg_val);
}

static void sih688x_active(
	sih_haptic_t *sih_haptic)
{
	hp_info("%s: enter!\n", __func__);

	sih688x_clear_interrupt_state(sih_haptic);
}

static void sih688x_set_gain(
	sih_haptic_t *sih_haptic,
	uint8_t gain)
{
	hp_info("%s: set gain:0x%02x", __func__, gain);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_PWM_PRE_GAIN,
		SIH_I2C_OPERA_BYTE_ONE,
		&gain);
}

static void sih688x_get_vbat(
	sih_haptic_t *sih_haptic)
{
	uint8_t time = 10;
	uint8_t reg_val;
	uint8_t vbat_high;
	uint8_t vbat_low;

	/* DET VBAT detect go */
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RL_VBAT_CTRL,
		SIH_RL_VBAT_CTRL_BIT_DET_MODE_MASK,
		SIH_RL_VBAT_CTRL_BIT_DET_MODE_OFF);
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RL_VBAT_CTRL,
		SIH_RL_VBAT_CTRL_BIT_DET_GO_MASK,
		SIH_RL_VBAT_CTRL_BIT_DET_GO_EN);

	/* read 0x02 */
	while(time) {
		haptic_regmap_read(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSINT,
			SIH_I2C_OPERA_BYTE_ONE,
			&reg_val);
		/* detect done */
		if (reg_val == 0x04) {
			/* read raw data */
			haptic_regmap_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_ADC_OC_DATA_H,
				SIH_I2C_OPERA_BYTE_ONE,
				&vbat_high);
			haptic_regmap_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_ADC_VBAT_DATA_L,
				SIH_I2C_OPERA_BYTE_ONE,
				&vbat_low);
			sih_haptic->detect_para.detect_vbat_rawdata = (uint32_t)((vbat_high & 0xf0) << 4 | vbat_low);

			/* calc data */
			sih_haptic->detect_para.detect_vbat = (((sih_haptic->detect_para.detect_vbat_rawdata * SIH688X_ADC_VREF *
				SIH688X_VBAT_AMPLIFY_COE) / SIH688X_ADC_VREF_COE) *
				SIH688X_LPF_GAIN_COE) / SIH688X_RL_CALC_COE_DIV;
			break;
		}
		usleep_range(2000, 2500);
		time--;
	}

	hp_info("detect_vbat:%d\n", sih_haptic->detect_para.detect_vbat);
}

static void sih688x_ram_vbat_comp(
	sih_haptic_t *sih_haptic)
{
	uint32_t comp_gain = 0x80;

	hp_info("%s enter\n", __func__);
	sih688x_get_vbat(sih_haptic);
	comp_gain = SIH688X_STANDARD_VBAT * SIH688X_PRE_GAIN_VALUE / sih_haptic->detect_para.detect_vbat;
	sih688x_set_gain(sih_haptic, (uint8_t)comp_gain);
}

static void __maybe_unused sih688x_f0_cali(
	sih_haptic_t *sih_haptic)
{
	int32_t code = 0;

	if (sih_haptic->detect_para.tracking_f0 <= SIH_F0_MAX_THRESHOLD &&
		sih_haptic->detect_para.tracking_f0 >= SIH_F0_MIN_THRESHOLD) {
		code = (((int32_t)((((int32_t)sih_haptic->detect_para.tracking_f0 / 10) * SIH688X_F0_CAL_COE) /
			sih_haptic->detect_para.cali_target_value)) - SIH688X_F0_CAL_COE) / SIH688X_F0_DELTA;
	}

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIM1,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&code);
	hp_info("code = %d tracking_f0 = %d\n", code, sih_haptic->detect_para.tracking_f0);
}

static void __maybe_unused sih688x_trim_init(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;

	hp_info("%s: enter! \n", __func__);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIM1,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&reg_val);
}

static void sih688x_set_play_mode(
	sih_haptic_t *sih_haptic,
	uint8_t play_mode)
{
	uint8_t reg_val = 0;

	hp_info("%s: enter! play mode = %d\n", __func__, play_mode);

	switch (play_mode) {
	case SIH_HAPTIC_IDLE_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_IDLE_MODE;
		sih688x_trim_init(sih_haptic);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSINTM,
			SIH_SYSINT_BIT_UVP_FLAG_INT_MASK,
			SIH_SYSINT_BIT_UVP_FLAG_INT_OFF);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_STOP_MODE_MASK,
			SIH_SYSCTRL1_BIT_STOP_RIGHT_NOW);
		reg_val = SIH_GO_BIT_RAM_GO_DISABLE |
			SIH_GO_BIT_RTP_GO_DISABLE |
			SIH_GO_BIT_F0_SEQ_GO_DISABLE |
			SIH_GO_BIT_STOP_TRIG_EN;
		haptic_regmap_write(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GO,
			SIH_I2C_OPERA_BYTE_ONE,
			(char *)&reg_val);
		usleep_range(2000, 2500);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_STOP_MODE_MASK,
			SIH_SYSCTRL1_BIT_STOP_CUR_OVER);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL2,
			SIH_SYSCTRL2_BIT_BOOST_BYPASS_MASK,
			SIH_SYSCTRL2_BIT_BOOST_ENABLE);
		hp_info("%s:now chip is stanby\n", __func__);
		break;
	case SIH_HAPTIC_RAM_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_RAM_MODE;
		sih688x_f0_cali(sih_haptic);
		usleep_range(2000, 2500);
		sih688x_active(sih_haptic);
		break;
	case SIH_HAPTIC_RTP_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_RTP_MODE;
		reg_val = sih_haptic->rtp_para.rtp_start_thres;
		haptic_regmap_write(sih_haptic->regmapp.regmapping,
			SIH688X_REG_RTP_START_THRES,
			SIH_I2C_OPERA_BYTE_ONE,
			(char *)&reg_val);
		sih688x_active(sih_haptic);
		break;
	case SIH_HAPTIC_TRIG_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_TRIG_MODE;
		sih688x_active(sih_haptic);
		break;
	case SIH_HAPTIC_CONT_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_CONT_MODE;
		sih688x_active(sih_haptic);
		break;
	case SIH_HAPTIC_RAM_LOOP_MODE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_RAM_LOOP_MODE;
		sih688x_f0_cali(sih_haptic);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL2,
			SIH_SYSCTRL2_BIT_BOOST_BYPASS_MASK,
			SIH_SYSCTRL2_BIT_BOOST_BYPASS);

		sih688x_ram_vbat_comp(sih_haptic);
		sih688x_active(sih_haptic);
		break;
	default:
		hp_err("%s: play mode %d err\n", __func__, play_mode);
		break;
	}
}

static void sih688x_stop(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;
	uint32_t cnt = SIH688X_STANDBY_JUDGE_MAX_TIME;

	hp_info("%s: enter\n", __func__);
	sih688x_play_go(sih_haptic, false);
	sih688x_set_play_mode(sih_haptic, SIH_HAPTIC_IDLE_MODE);

	while (cnt--) {
		haptic_regmap_read(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSSST,
			SIH_I2C_OPERA_BYTE_ONE,
			&reg_val);
		if ((reg_val & 0x01) == SIH_SYSSST_BIT_STANDBY) {
			hp_info("%s: enterd standby! standby_mode = 0x%02x\n",
				__func__, reg_val);
			break;
		}
		usleep_range(2000, 2500);
		hp_info("%s: wait for standby, reg standby_mode = 0x%02x\n",
			__func__, reg_val);
	}
}

static void sih688x_set_wav_seq(
	sih_haptic_t *sih_haptic,
	uint8_t wav,
	uint8_t seq)
{
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_WAVESEQ1 + wav,
		SIH_I2C_OPERA_BYTE_ONE,
		&seq);
}

static void sih688x_get_wav_seq(
	sih_haptic_t *sih_haptic,
	uint32_t len)
{
	uint8_t i;
	uint8_t reg_val[SIH_HAPTIC_SEQUENCER_SIZE] = {0};

	if (len > SIH_HAPTIC_SEQUENCER_SIZE)
		len = SIH_HAPTIC_SEQUENCER_SIZE;

	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_WAVESEQ1,
		len,
		reg_val);
	for (i = 0; i < len; i++)
		sih_haptic->ram_para.seq[i] = reg_val[i];
}

static ssize_t sih688x_get_ram_data(
	sih_haptic_t *sih_haptic,
	char *buf)
{
	uint8_t *ram_data;
	int i = 0;
	int size = 0;
	ssize_t len = 0;

	ram_data = vmalloc(SIH_RAMDATA_RD_BUFFER_SIZE);
	if (!ram_data)
		return len;

	if (sih_haptic->ram_para.len < SIH_RAMDATA_RD_BUFFER_SIZE)
		size = sih_haptic->ram_para.len;
	else
		size = SIH_RAMDATA_RD_BUFFER_SIZE;

	while (i < size) {
		if ((size - i) <= SIH_RAMDATA_READ_SIZE)
			len = size - i;
		else
			len = SIH_RAMDATA_READ_SIZE;

		haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
			SIH688X_REG_RAMDATA,
			len,
			&ram_data[i]);

		i += len;
	}

	for (i = 1; i < size; i++)
		len += snprintf(buf + len, PAGE_SIZE - len,
			"0x%02x,", ram_data[i]);

	vfree(ram_data);

	return len;
}

static void sih688x_get_first_wave_addr(
	sih_haptic_t *sih_haptic,
	uint8_t *wave_addr)
{
	uint8_t reg_array[3] = {0};

	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RAMDATA,
		SIH_I2C_OPERA_BYTE_THREE,
		reg_array);

	wave_addr[0] = reg_array[1];
	wave_addr[1] = reg_array[2];

	hp_info("%s:wave_addr[0] = 0x%x wave_addr[1] = 0x%x\n",
		__func__, wave_addr[0], wave_addr[1]);
}

static void sih688x_set_ram_addr(
	sih_haptic_t *sih_haptic)
{
	uint8_t ram_addr[2] = {0};

	ram_addr[0] = (uint8_t)SIH_RAM_ADDR_H(sih_haptic->ram_para.base_addr);
	ram_addr[1] = (uint8_t)SIH_RAM_ADDR_L(sih_haptic->ram_para.base_addr);
	haptic_regmap_bulk_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_BASE_ADDRH,
		SIH_I2C_OPERA_BYTE_TWO,
		ram_addr);
}

static void sih688x_set_wav_internal_loop(
	sih_haptic_t *sih_haptic,
	uint8_t wav,
	uint8_t loop)
{
	uint8_t tmp = 0;

	hp_info("%s enter; wav = 0x%02x, loop = 0x%02x\n", __func__, wav, loop);

	if (wav % 2) {
		tmp = loop << 0;
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_WAVELOOP1 + (wav / 2),
			WAVELOOP_SEQ_EVEN_MASK,
			tmp);
	} else {
		tmp = loop << 4;
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_WAVELOOP1 + (wav / 2),
			WAVELOOP_SEQ_ODD_MASK,
			tmp);
	}
}

static size_t sih688x_get_wav_internal_loop(
	sih_haptic_t *sih_haptic,
	char *buf)
{
	uint8_t i;
	uint8_t reg_val[4] = {0};
	size_t count = 0;

	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_WAVELOOP1,
		SIH_I2C_OPERA_BYTE_FOUR,
		reg_val);

	for (i = 0; i < SIH_HAPTIC_SEQUENCER_LOOP_SIZE; i++) {
		sih_haptic->ram_para.loop[i * 2 + 0] = (reg_val[i] >> 4) & 0x0F;
		sih_haptic->ram_para.loop[i * 2 + 1] = (reg_val[i] >> 0) & 0x0F;
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d loop: 0x%02x\n", i * 2 + 1,
			sih_haptic->ram_para.loop[i * 2 + 0]);
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d loop: 0x%02x\n", i * 2 + 2,
			sih_haptic->ram_para.loop[i * 2 + 1]);
	}
	return count;
}

static void sih688x_set_wav_external_loop(
	sih_haptic_t *sih_haptic,
	uint8_t loop)
{
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_MAINLOOP,
		SIH_MAINLOOP_BIT_MAIN_LOOP_MASK,
		loop);
}

static size_t sih688x_get_wav_external_loop(
	sih_haptic_t *sih_haptic,
	char *buf)
{
	size_t count = 0;
	uint8_t reg_val = 0;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_MAINLOOP,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);
	sih_haptic->ram_para.external_loop = reg_val;
	count += snprintf(buf + count, PAGE_SIZE - count, "main loop:0x%02x\n",
		sih_haptic->ram_para.external_loop);

	return count;
}

static void sih688x_set_repeat_seq(
	sih_haptic_t *sih_haptic,
	uint8_t seq)
{
	sih688x_set_wav_seq(sih_haptic, 0x00, seq);
	sih688x_set_wav_internal_loop(sih_haptic,
		0x00, WAVELOOP_SEQ_ODD_INFINNTE_TIME);
}

static size_t sih688x_write_rtp_data(
	sih_haptic_t *sih_haptic,
	uint8_t *data,
	uint32_t len)
{
	int ret = 0;

	ret = haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RTPDATA,
		len,
		data);

	return ret;
}

static void sih688x_set_drv_bst_vol(
	sih_haptic_t *sih_haptic,
	uint32_t drv_bst)
{
	uint8_t bst_vol = 0;
	uint32_t bst_reg_val = 0;
	hp_info("%s enter!\n", __func__);
	bst_reg_val = ((drv_bst - SIH688X_DRV_BOOST_BASE) * SIH688X_DRV_BOOST_SETP_COE) / SIH688X_DRV_BOOST_SETP;

	haptic_regmap_write(sih_haptic->regmapp.regmapping, SIH688X_REG_ANA_CTRL3,
		SIH_I2C_OPERA_BYTE_ONE, (char *)&bst_reg_val);

	bst_vol = drv_bst / SIH_6887_DRV_VBOOST_COEFFICIENT;

	if (bst_vol >= SIH_ANA_CTRL_BST_LEVEL_6 &&
		bst_vol <= SIH_ANA_CTRL_BST_LEVEL_8) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_IOS_SEL_O_MASK,
			SIH_ANA_CTRL5_BST_IOS_SEL_6_8);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_OCP_VRSEL_O_MASK,
			SIH_ANA_CTRL5_BST_OCP_VRSEL_6_8);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL6, SIH_ANA_CTRL6_BST_ZCD_IOS_O_MASK,
			SIH_ANA_CTRL5_BST_ZCD_IOS_6_8);
	} else if (bst_vol > SIH_ANA_CTRL_BST_LEVEL_8 &&
		bst_vol <= SIH_ANA_CTRL_BST_LEVEL_10) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_IOS_SEL_O_MASK,
			SIH_ANA_CTRL5_BST_IOS_SEL_8_10);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_OCP_VRSEL_O_MASK,
			SIH_ANA_CTRL5_BST_OCP_VRSEL_8_10);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL6, SIH_ANA_CTRL6_BST_ZCD_IOS_O_MASK,
			SIH_ANA_CTRL5_BST_ZCD_IOS_8_11);
	} else if (bst_vol > SIH_ANA_CTRL_BST_LEVEL_10 &&
		bst_vol <= SIH_ANA_CTRL_BST_LEVEL_11) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_IOS_SEL_O_MASK,
			SIH_ANA_CTRL5_BST_IOS_SEL_10_11);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL5, SIH_ANA_CTRL5_BST_OCP_VRSEL_O_MASK,
			SIH_ANA_CTRL5_BST_OCP_VRSEL_10_11);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_ANA_CTRL6, SIH_ANA_CTRL6_BST_ZCD_IOS_O_MASK,
			SIH_ANA_CTRL5_BST_ZCD_IOS_8_11);
	}
}

static ssize_t __maybe_unused sih688x_get_all_reg(
	sih_haptic_t *sih_haptic,
	ssize_t len,
	char *buf)
{
	uint32_t i;
	uint8_t reg_val[SIH688X_REG_MAX+1] = {0};

	hp_info("%s enter\n", __func__);
	for (i = 0; i <= SIH688X_REG_MAX; i++) {
		haptic_regmap_read(sih_haptic->regmapp.regmapping,
			i,
			SIH_I2C_OPERA_BYTE_ONE,
			&reg_val[i]);
	}

	for (i = 0; i <= SIH688X_REG_MAX; i++)
		len += snprintf(buf + len, PAGE_SIZE - len,
			"reg:0x%02X = 0x%02X\n", i, reg_val[i]);

	return len;
}

static void __maybe_unused sih688x_get_lra_resistance(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val;
	uint8_t time = 10;
	uint8_t ana_ctrl1_old_val;
	uint8_t ana_ctrl2_old_val;
	uint8_t ana_cnt_old_val;
	uint8_t rl_high;
	uint8_t rl_low;

	hp_info("%s enter\n", __func__);

	/* ADC cnt */
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ADC_EN_CNT,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_cnt_old_val);
	reg_val = 0x40;
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ADC_EN_CNT,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);
	/* lpf gain = 8X */
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL1,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_ctrl1_old_val);
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL2,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_ctrl2_old_val);
	reg_val = 0x09;
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL1,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);
	reg_val = 0x35;
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL2,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);
	/* DET RL detect go */
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RL_VBAT_CTRL,
		SIH_RL_VBAT_CTRL_BIT_DET_MODE_MASK,
		SIH_RL_VBAT_CTRL_BIT_DET_MODE_EN);
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_RL_VBAT_CTRL,
		SIH_RL_VBAT_CTRL_BIT_DET_GO_MASK,
		SIH_RL_VBAT_CTRL_BIT_DET_GO_EN);

	/* read 0x02 */
	while (time) {
		haptic_regmap_read(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSINT,
			SIH_I2C_OPERA_BYTE_ONE,
			&reg_val);
		/* detect done */
		if (reg_val == 0x04) {
			/* read raw data */
			haptic_regmap_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_ADC_RL_DATA_H,
				SIH_I2C_OPERA_BYTE_ONE,
				&rl_high);
			haptic_regmap_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_ADC_RL_DATA_L,
				SIH_I2C_OPERA_BYTE_ONE,
				&rl_low);
			sih_haptic->detect_para.detect_rl_rawdata = (uint32_t)(rl_high << 8 | rl_low);
			hp_info("raw_data:%d\r\n", sih_haptic->detect_para.detect_rl_rawdata);
			/* calc data */
			sih_haptic->detect_para.detect_rl = (uint32_t)((((uint64_t)(sih_haptic->detect_para.detect_rl_rawdata *
				SIH688X_RL_CALC_COE_PLUS)) * SIH688X_RL_CALC_COE_PLUS_COE / SIH688X_RL_CALC_COE_DIV) /
				SIH688X_RL_CALC_COE_DIV_COE) - sih_haptic->detect_para.detect_rl_offset;
			break;
		}
		usleep_range(2000, 2500);
		time--;
	}
	/* recovery reg */
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ADC_EN_CNT,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_cnt_old_val);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL1,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_ctrl1_old_val);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL2,
		SIH_I2C_OPERA_BYTE_ONE,
		&ana_ctrl2_old_val);
	hp_info("detect_rl:%d\n", sih_haptic->detect_para.detect_rl);
}

static void __maybe_unused sih688x_set_auto_pvdd(
	sih_haptic_t *sih_haptic,
	bool flag)
{
	if (flag) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_AUTO_PVDD_MASK,
			SIH_SYSCTRL1_BIT_AUTO_PVDD_EN);
		sih_haptic->chip_ipara.auto_pvdd_en = true;
	} else {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_AUTO_PVDD_MASK,
			SIH_SYSCTRL1_BIT_AUTO_PVDD_OFF);
		sih_haptic->chip_ipara.auto_pvdd_en = false;
	}
}

/*****************************************************
 *
 * rtp
 *
 *****************************************************/
static void __maybe_unused sih688x_set_rtp_aei(
	sih_haptic_t *sih_haptic,
	bool flag)
{
	if (flag) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSINTM,
			SIH_SYSINTM_BIT_FF_AEI_MASK,
			SIH_SYSINTM_BIT_FF_AEI_EN);
	} else {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSINTM,
			SIH_SYSINTM_BIT_FF_AEI_MASK,
			SIH_SYSINTM_BIT_FF_AEI_OFF);
	}
}

static bool __maybe_unused sih688x_if_is_mode(
	sih_haptic_t *sih_haptic,
	uint8_t mode)
{
	uint8_t reg_val = 0;
	bool flag = false;

	haptic_regmap_read(sih_haptic->regmapp.regmapping, SIH688X_REG_SYSSST,
		SIH_I2C_OPERA_BYTE_ONE, &reg_val);

	switch (mode) {
	case SIH_HAPTIC_IDLE_MODE:
		if ((reg_val & SIH_SYSSST_BIT_STANDBY) == SIH_SYSSST_BIT_STANDBY)
			flag = true;
		else
			flag = false;

		break;
	case SIH_HAPTIC_RAM_MODE:
	case SIH_HAPTIC_RAM_LOOP_MODE:
		if ((reg_val & SIH_SYSSST_BIT_RAM_STATE) == SIH_SYSSST_BIT_RAM_STATE)
			flag = true;
		else
			flag = false;

		break;
	case SIH_HAPTIC_RTP_MODE:
		if ((reg_val & SIH_SYSSST_BIT_RTP_STATE) == SIH_SYSSST_BIT_RTP_STATE)
			flag = true;
		else
			flag = false;

		break;
	case SIH_HAPTIC_TRIG_MODE:
		if ((reg_val & SIH_SYSSST_BIT_TRIG_STATE) == SIH_SYSSST_BIT_TRIG_STATE)
			flag = true;
		else
			flag = false;

		break;
	case SIH_HAPTIC_CONT_MODE:
		if ((reg_val & SIH_SYSSST_BIT_F0_TRACK_STATE) == SIH_SYSSST_BIT_F0_TRACK_STATE)
			flag = true;
		else
			flag = false;

		break;
	default:
		hp_err("%s: err mode!\n", __func__);
		break;
	}

	return flag;
}

static uint8_t __maybe_unused sih688x_if_rtp_fifo_full(
	sih_haptic_t *sih_haptic)
{
	uint8_t ret = 0;
	uint8_t reg_val = 0;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSSST,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	reg_val &= SIH_SYSSST_BIT_FIF0_AF;
	ret = (reg_val >> 7);

	return ret;
}

static void __maybe_unused sih688x_upload_lra(
	sih_haptic_t *sih_haptic,
	uint32_t flag) {}

/*****************************************************
 *
 * haptic cont
 *
 *****************************************************/
static void cont_config_seq0(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint8_t write_value = 0;

	write_value = (uint8_t)(((value & 0xff) * SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq0_t_driver = write_value;
	write_value = (uint8_t)((((value >> 8) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq0_t_flush = write_value;
	write_value = (uint8_t)((((value >> 16) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq0_t_bemf = write_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ0_T_DRIVER,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq0_t_driver);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ0_T_FLUSH,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq0_t_flush);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ0_T_BEMF,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq0_t_bemf);
}

static void cont_config_seq1(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint8_t write_value = 0;

	write_value = (uint8_t)(((value & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq1_t_driver = write_value;
	write_value = (uint8_t)((((value >> 8) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq1_t_flush = write_value;
	write_value = (uint8_t)((((value >> 16) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq1_t_bemf = write_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ1_T_DRIVER,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq1_t_driver);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ1_T_FLUSH,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq1_t_flush);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ1_T_BEMF,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq1_t_bemf);
}

static void cont_config_seq2(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint8_t write_value = 0;

	write_value = (uint8_t)(((value & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq2_t_driver = write_value;
	write_value = (uint8_t)((((value >> 8) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq2_t_flush = write_value;
	write_value = (uint8_t)((((value >> 16) & 0xff) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.seq2_t_bemf = write_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ2_T_DRIVER,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq2_t_driver);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ2_T_FLUSH,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq2_t_flush);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SEQ2_T_BEMF,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&sih_haptic->cont_para.seq2_t_bemf);
}

static void cont_config_asmooth(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint32_t write_value = 0;

	write_value = (uint32_t)((value *
		SIH688X_SEQ_MUL_ASMOOTH_COE) /
		SIH688X_SEQ_DIV_COEFFICIENT);
	sih_haptic->cont_para.a_smooth = write_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SMOOTH_CONST_ALGO_DATA_0,
		SIH_I2C_OPERA_BYTE_THREE,
		(char *)&sih_haptic->cont_para.a_smooth);

	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_PWM_UP_SAMPLE_CTRL,
		SIH_PWM_UP_SAMPLE_CTRL_BIT_FIR_BRK_EN_MASK,
		SIH_PWM_UP_SAMPLE_CTRL_BIT_FIR_BRK_EN);
}

static void cont_config_th_len(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint32_t write_value = 0;

	write_value = (uint32_t)(((value * SIH688X_CLK_FS) *
		SIH688X_SEQ_MUL_COEFFICIENT) /
		SIH688X_THALF_DIV_COEFFICIENT);
	sih_haptic->cont_para.t_half_len = write_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_T_0,
		SIH_I2C_OPERA_BYTE_THREE,
		(char *)&sih_haptic->cont_para.t_half_len);
}

static void cont_config_th_num(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint8_t write_value[3] = {0};

	write_value[0] = (uint8_t)((value & 0xff));
	write_value[1] = (uint8_t)(((value >> 8) & 0xff));
	write_value[2] = (uint8_t)(((value >> 16) & 0xff));
	sih_haptic->cont_para.t_half_num = write_value[0] |
		(write_value[1] << 8) | (write_value[2] << 16);

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_CYCLE0,
		SIH_I2C_OPERA_BYTE_THREE,
		(char *)&sih_haptic->cont_para.t_half_num);
}

static void cont_config_ampli(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	uint8_t write_value[3] = {0};
	uint32_t mul_coe = 0;
	uint32_t div_coe = 0;

	mul_coe = SIH688X_VERF_COEFFICIENT *
		sih_haptic->chip_ipara.drv_vboost * SIH688X_VOLATE_COEFFICIENT;
	div_coe = SIH688X_VERF_DIV_COEFFICIENT *
		SIH_6887_DRV_VBOOST_COEFFICIENT;

	write_value[0] = (uint8_t)(((value & 0xff) *
		div_coe) / mul_coe);
	write_value[1] = (uint8_t)((((value >> 8) & 0xff) *
		div_coe) / mul_coe);
	write_value[2] = (uint8_t)((((value >> 16) & 0xff) *
		div_coe) / mul_coe);

	sih_haptic->cont_para.dst_ampli = write_value[0] |
		(write_value[1] << 8) | (write_value[2] << 16);
	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_VREF0,
		SIH_I2C_OPERA_BYTE_THREE,
		write_value);
}

static void cont_config_go(
	sih_haptic_t *sih_haptic,
	uint32_t value)
{
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_GO,
		SIH_GO_BIT_F0_SEQ_GO_MASK,
		SIH_GO_BIT_F0_SEQ_GO_ENABLE);

	hp_info("%s cont go!\n", __func__);
}

static void __maybe_unused sih688x_update_cont_config(
	sih_haptic_t *sih_haptic,
	uint8_t sel,
	uint32_t value)
{
	if (sel > SIH688X_CONT_CONFIG_MAX)
		hp_info("%s: invalid parameter\n", __func__);
	else
		cont_config_sih688x[sel](sih_haptic, value);
}

static void __maybe_unused sih688x_read_detect_fifo(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;
	uint8_t detect_array_max = SIH688X_DETECT_FIFO_ARRAY_MAX;
	uint8_t detect_num;
	uint32_t bemf_value;
	uint32_t f0_value;
	uint32_t lpf_gain_value;
	char buf[SIH688X_DETECT_FIFO_ARRAY_SIZE] = {0xff, 0xff,
		0xff, 0xff, 0xff};

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINT2,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&reg_val);

	reg_val &= SIH_SYSINT2_BIT_F0_DETECT_DONE_INT;
	if (SIH_SYSINT2_BIT_F0_DETECT_DONE_INT == reg_val) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_ENRAMINIT_MASK,
			SIH_SYSCTRL1_BIT_RAMINIT_EN);

		/* F0 detect read */
		for (detect_num = 0; detect_num <
			detect_array_max; ++detect_num) {
			haptic_regmap_read(sih_haptic->regmapp.regmapping,
				SIH688X_REG_RAMDATA,
				SIH688X_DETECT_FIFO_ARRAY_SIZE,
				(sih_haptic->detect_para.detect_fifo +
				detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE));

			hp_info("fifo data:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 0],
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 1],
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 2],
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 3],
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 4]);

			if (!memcmp(buf, (sih_haptic->detect_para.detect_fifo +
				detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE),
				SIH688X_DETECT_FIFO_ARRAY_SIZE)) {
				break;
			}
		}

		/* F0 detect calc */
		sih_haptic->detect_para.detect_drive_time = (((sih_haptic->detect_para.detect_fifo[2] << 16 |
			sih_haptic->detect_para.detect_fifo[3] << 8 |
			sih_haptic->detect_para.detect_fifo[4]) / SIH688X_DETECT_FOSC) / SIH688X_DETECT_FOSC_COE);

		haptic_regmap_read(sih_haptic->regmapp.regmapping, SIH688X_REG_ANA_CTRL1, SIH_I2C_OPERA_BYTE_ONE, &reg_val);
		switch (reg_val & SIH_ANA_CTRL1_LPF_CAP_O_MASK) {
			case SIH_ANA_CTRL1_LPF_CAP_O_0_82_GAIN:
				lpf_gain_value = SIH688X_LPF_CAP_GAIN_1 / SIH688X_LPF_CAP_GAIN_1_COE;
				break;
			case SIH_ANA_CTRL1_LPF_CAP_O_2_5_GAIN:
				lpf_gain_value = SIH688X_LPF_CAP_GAIN_2 / SIH688X_LPF_CAP_GAIN_2_COE;
				break;
			case SIH_ANA_CTRL1_LPF_CAP_O_4_GAIN:
				lpf_gain_value = SIH688X_LPF_CAP_GAIN_3;
				break;
			case SIH_ANA_CTRL1_LPF_CAP_O_6_GAIN:
				lpf_gain_value = SIH688X_LPF_CAP_GAIN_4;
				break;
			case SIH_ANA_CTRL1_LPF_CAP_O_8_GAIN:
				lpf_gain_value = SIH688X_LPF_CAP_GAIN_5;
				break;
			default:
				hp_err("no lpf matched 0x41:0x%02x\n", (reg_val & SIH_ANA_CTRL1_LPF_CAP_O_MASK));
				break;
		}
		for (detect_num = 1; detect_num < detect_array_max; ++detect_num) {
			bemf_value = (uint32_t)(sih_haptic->detect_para.detect_fifo[detect_num *
				SIH688X_DETECT_FIFO_ARRAY_SIZE] << 8 |
				sih_haptic->detect_para.detect_fifo[detect_num *
				SIH688X_DETECT_FIFO_ARRAY_SIZE + 1]);
			f0_value = (uint32_t)(sih_haptic->detect_para.detect_fifo[detect_num *
				SIH688X_DETECT_FIFO_ARRAY_SIZE + 2] << 16 |
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 3] << 8 |
				sih_haptic->detect_para.detect_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 4]);
			sih_haptic->detect_para.detect_bemf_value[detect_num - 1] =
				(((bemf_value * SIH688X_DETECT_ADCV) / SIH688X_DETECT_ADCV_COE)
				/ SIH688X_DETECT_BEMF_COE) / lpf_gain_value;
			sih_haptic->detect_para.detect_f0_value[detect_num - 1] =
				((SIH688X_DETECT_F0_COE * SIH688X_DETECT_FOSC * SIH688X_DETECT_FOSC_COE) /
				f0_value) / 2;
			hp_err("detect:bemf:%d f0:%d calc:bemf:%d f0:%d\n", bemf_value, f0_value,
				sih_haptic->detect_para.detect_bemf_value[detect_num - 1],
				sih_haptic->detect_para.detect_f0_value[detect_num - 1]);
			if (!memcmp(buf, (sih_haptic->detect_para.detect_fifo +
				detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE),
				SIH688X_DETECT_FIFO_ARRAY_SIZE)) {
				break;
			}
		}

		sih_haptic->detect_para.detect_f0 = (sih_haptic->detect_para.detect_f0_value[1] +
			sih_haptic->detect_para.detect_f0_value[2] +
			sih_haptic->detect_para.detect_f0_value[3] +
			sih_haptic->detect_para.detect_f0_value[4]) / 4;
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_ENRAMINIT_MASK,
			SIH_SYSCTRL1_BIT_RAMINIT_OFF);

		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SMOOTH_F0_WINDOW_OUT,
			SIH_SMOOTH_F0_DETECT_MASK,
			SIH_SMOOTH_F0_DETECT_OFF);
	}
}

static void __maybe_unused sih688x_read_tracking_f0(
	sih_haptic_t *sih_haptic)
{
	uint8_t data_val[3] = {0};
	uint8_t reg_val = 0;
	uint32_t tracking_f0_value;

	hp_info("%s:enter\n", __func__);
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINT2,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&reg_val);

	reg_val &= SIH_SYSINT2_BIT_F0_TRACKING_INT;
	if (reg_val == SIH_SYSINT2_BIT_F0_TRACKING_INT) {
		/* F0 detect calc */
		haptic_regmap_read(sih_haptic->regmapp.regmapping,
			SIH688X_REG_T_HALF_TRACKING_0,
			SIH_I2C_OPERA_BYTE_THREE,
			(char *)&data_val);
		tracking_f0_value = data_val[0] | data_val[1] << 8 | data_val[2] << 16;
		if (tracking_f0_value == 0) {
			sih_haptic->detect_para.tracking_f0 = 0;
		} else {
			sih_haptic->detect_para.tracking_f0 = (uint32_t)((((uint64_t)(SIH688X_DETECT_F0_COE * SIH688X_DETECT_FOSC *
				SIH688X_DETECT_FOSC_COE)) * 10 / tracking_f0_value)) / 2;
		}
		hp_info("tracking_raw_f0:%d  calc_tracking_f0:%d\n", tracking_f0_value, sih_haptic->detect_para.tracking_f0);
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SMOOTH_F0_WINDOW_OUT,
			SIH_SMOOTH_F0_DETECT_MASK,
			SIH_SMOOTH_F0_DETECT_OFF);
	}
}

static void __maybe_unused sih688x_read_bemf_fifo(
	sih_haptic_t *sih_haptic)
{
	uint8_t detect_array_max = SIH688X_DETECT_FIFO_ARRAY_MAX;
	uint8_t detect_num;
	uint32_t bemf_value;
	char buf[SIH688X_DETECT_FIFO_ARRAY_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff};

	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping, SIH688X_REG_SYSCTRL1,
		SIH_SYSCTRL1_BIT_ENRAMINIT_MASK, SIH_SYSCTRL1_BIT_RAMINIT_EN);

	/* F0 detect read */
	for (detect_num = 0; detect_num <
		detect_array_max; ++detect_num) {
		haptic_regmap_read(sih_haptic->regmapp.regmapping, SIH688X_REG_RAMDATA,
			SIH688X_DETECT_FIFO_ARRAY_SIZE,
			(sih_haptic->polar_cali_para.data_fifo +
			detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE));

		hp_info("fifo data:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			sih_haptic->polar_cali_para.data_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 0],
			sih_haptic->polar_cali_para.data_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 1],
			sih_haptic->polar_cali_para.data_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 2],
			sih_haptic->polar_cali_para.data_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 3],
			sih_haptic->polar_cali_para.data_fifo[detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE + 4]);

		if (!memcmp(buf, (sih_haptic->polar_cali_para.data_fifo +
			detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE),
			SIH688X_DETECT_FIFO_ARRAY_SIZE)) {
			break;
		}
	}

	for (detect_num = 1; detect_num < detect_array_max; ++detect_num) {

		if (!memcmp(buf, (sih_haptic->polar_cali_para.data_fifo +
			detect_num * SIH688X_DETECT_FIFO_ARRAY_SIZE),
			SIH688X_DETECT_FIFO_ARRAY_SIZE)) {
			break;
		}

		bemf_value = (uint32_t)(sih_haptic->polar_cali_para.data_fifo[detect_num *
			SIH688X_DETECT_FIFO_ARRAY_SIZE] << 8 |
			sih_haptic->polar_cali_para.data_fifo[detect_num *
			SIH688X_DETECT_FIFO_ARRAY_SIZE + 1]);

		sih_haptic->polar_cali_para.polar_bemf_value[detect_num - 1] = bemf_value;

		hp_err("detect:bemf:%d\n", bemf_value);
	}

	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSCTRL1,
		SIH_SYSCTRL1_BIT_ENRAMINIT_MASK,
		SIH_SYSCTRL1_BIT_RAMINIT_OFF);

	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SMOOTH_F0_WINDOW_OUT,
		SIH_SMOOTH_F0_DETECT_MASK,
		SIH_SMOOTH_F0_DETECT_OFF);
}

static void __maybe_unused sih688x_osc_trim_calibration(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;
	uint8_t freg_val = 0;
	int32_t code;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_PWM_UP_SAMPLE_CTRL,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	freg_val = (reg_val & SIH_PWM_UP_SAMPLE_CTRL_BIT_PWM_UP_RPT_MASK) >> 1;

	if (freg_val == 3) { /* 12k */
		sih_haptic->osc_para.theory_time = (sih_haptic->osc_para.osc_rtp_len * 1000) / 12;
	} else if (freg_val == 1) { /* 24k */
		sih_haptic->osc_para.theory_time = (sih_haptic->osc_para.osc_rtp_len * 1000) / 24;
	} else if (freg_val == 0) { /* 48k */
		sih_haptic->osc_para.theory_time = (sih_haptic->osc_para.osc_rtp_len * 1000) / 48;
	}

	hp_info("%s: microsecond:%d theory_time:%d\n", __func__,
		sih_haptic->osc_para.microsecond, sih_haptic->osc_para.theory_time);

	code = (((int32_t)((((int32_t)sih_haptic->osc_para.theory_time) /
		(sih_haptic->osc_para.microsecond / SIH688X_OSC_CALI_COE))
		- SIH688X_OSC_CALI_COE)) * SIH688X_OSC_CALI_COE) / SIH688X_F0_DELTA;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIM1,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&code);

	sih_haptic->osc_para.osc_data = (uint32_t)code;
}

static void __maybe_unused sih688x_interrupt_state_init(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;
	uint8_t reg_mask = 0;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINT,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	hp_info("%s: reg SYSINT = 0x%02x\n", __func__, reg_val);

	/* int enable */
	reg_mask = SIH_SYSINT_BIT_OCP_FLAG_INT_MASK |
		SIH_SYSINT_BIT_UVP_FLAG_INT_MASK |
		SIH_SYSINT_BIT_OTP_FLAG_INT_MASK |
		SIH_SYSINT_BIT_MODE_SWITCH_INT_MASK |
		SIH_SYSINT_BIT_BRK_LONG_TIMEOUT_MASK |
		SIH_SYSINTM_BIT_DONE_MASK |
		SIH_SYSINTM_BIT_FF_AEI_MASK |
		SIH_SYSINTM_BIT_FF_AFI_MASK;
	reg_val = SIH_SYSINT_BIT_OCP_FLAG_INT_OFF |
		SIH_SYSINT_BIT_UVP_FLAG_INT_OFF |
		SIH_SYSINT_BIT_OTP_FLAG_INT_OFF |
		SIH_SYSINT_BIT_MODE_SWITCH_INT_OFF |
		SIH_SYSINT_BIT_BRK_LONG_TIMEOUT_OFF |
		SIH_SYSINTM_BIT_DONE_OFF |
		SIH_SYSINTM_BIT_FF_AEI_OFF |
		SIH_SYSINTM_BIT_FF_AFI_OFF;
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINTM,
		reg_mask,
		reg_val);

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINTM,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	hp_info("%s: reg SYSINTM = 0x%02x\n", __func__, reg_val);
}

static int __maybe_unused sih688x_get_interrupt_state(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;
	uint8_t int_sts = 0;
	uint8_t status = 0;
	int ret = -1;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSSST,
		SIH_I2C_OPERA_BYTE_ONE,
		&int_sts);
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSINT,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_INT_STATUS2,
		SIH_I2C_OPERA_BYTE_ONE,
		&status);

	hp_info("%s: reg SYSST = 0x%02x reg SYSINT = 0x%02x reg STATUS2 = 0x%02x\n", __func__, int_sts, reg_val, status);

	/* dbg_state */

	if (reg_val & SIH_SYSINT_BIT_BRK_LONG_TIMEOUT)
		hp_err("%s: chip ltimeout int error\n", __func__);

	if (reg_val & SIH_SYSINT_BIT_DONE)
		hp_info("%s: chip playback done\n", __func__);

	if ((reg_val & SIH_SYSINT_BIT_FF_AEI) || (int_sts & 1 << 6)) {
		ret = 0;
		hp_err("%s: rtp mode fifo almost empty", __func__);
	}
	if (reg_val & SIH_SYSINT_BIT_FF_AFI && (int_sts & 1 << 7))
		hp_err("%s: rtp mode fifo almost full", __func__);

	return ret;
}

/*****************************************************
 *
 * device tree
 *
 *****************************************************/

static void __maybe_unused sih688x_set_low_power_mode(
	sih_haptic_t *sih_haptic,
	uint8_t val)
{
	if (val) {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_LOWPOWER_MASK,
			SIH_SYSCTRL1_BIT_LOWPOWER_EN);
		sih_haptic->chip_ipara.low_power = true;
	} else {
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_SYSCTRL1,
			SIH_SYSCTRL1_BIT_LOWPOWER_MASK,
			SIH_SYSCTRL1_BIT_LOWPOWER_OFF);
		sih_haptic->chip_ipara.low_power = false;
	}
	hp_info("sih_haptic->chip_ipara.low_power = %d\n",
		sih_haptic->chip_ipara.low_power);
}

static void __maybe_unused sih688x_trig_para_set(
	sih_haptic_t *sih_haptic,
	uint32_t *val)
{
	/* index enable polar trig_mode boost_bypass p_id n_id */
	uint8_t index = 0;
	uint8_t ctrl1 = 0;
	uint8_t ctrl2 = 0;
	uint8_t boost = 0;
	uint8_t pose_id = 0;
	uint8_t nege_id = 0;

	hp_info("%s: enter\n", __func__);
	if (val[0] == SIH688X_TRIG_INDEX_TRIG0) {
		index = SIH688X_TRIG_INDEX_TRIG0;
	} else if (val[0] == SIH688X_TRIG_INDEX_TRIG1) {
		index = SIH688X_TRIG_INDEX_TRIG1;
	} else if (val[0] == SIH688X_TRIG_INDEX_TRIG2) {
		index = SIH688X_TRIG_INDEX_TRIG2;
	} else {
		hp_err("error index=%d\n", val[0]);
		return;
	}

	if (val[1] == SIH688X_TRIG_ENABLE) {
		sih_haptic->trig_para[index].enable = SIH688X_TRIG_ENABLE;
		ctrl1 |= (uint8_t)(1 << index);
	} else if (val[1] == SIH688X_TRIG_DISABLE) {
		sih_haptic->trig_para[index].enable = SIH688X_TRIG_DISABLE;
		ctrl1 |= (uint8_t)(0 << index);
	} else {
		hp_err("error enable=%d\n", val[1]);
		return;
	}

	if (val[2] == SIH688X_TRIG_HIGH_ACTIVE) {
		sih_haptic->trig_para[index].polar = SIH688X_TRIG_HIGH_ACTIVE;
		ctrl1 |= (uint8_t)(0 << (index + 4));
	} else if (val[2] == SIH688X_TRIG_LOW_ACTIVE) {
		sih_haptic->trig_para[index].polar = SIH688X_TRIG_LOW_ACTIVE;
		ctrl1 |= (uint8_t)(1 << (index + 4));
	} else {
		hp_err("error polar=%d\n", val[2]);
		return;
	}

	if (val[3] == SIH688X_TRIG_POSEDGE_TRIGGER) {
		sih_haptic->trig_para[index].mode =
			SIH688X_TRIG_POSEDGE_TRIGGER;
		ctrl2 |= SIH688X_TRIG_POSEDGE_TRIGGER << index*2;
	} else if (val[3] == SIH688X_TRIG_NEGEDGE_TRIGGER) {
		sih_haptic->trig_para[index].mode =
			SIH688X_TRIG_NEGEDGE_TRIGGER;
		ctrl2 |= SIH688X_TRIG_NEGEDGE_TRIGGER << index*2;
	} else if (val[3] == SIH688X_TRIG_BOTHEDGE_TRIGGER) {
		sih_haptic->trig_para[index].mode =
			SIH688X_TRIG_BOTHEDGE_TRIGGER;
		ctrl2 |= SIH688X_TRIG_BOTHEDGE_TRIGGER << index*2;
	} else if (val[3] == SIH688X_TRIG_LEVEL_TRIGGER) {
		sih_haptic->trig_para[index].mode =
			SIH688X_TRIG_LEVEL_TRIGGER;
		ctrl2 |= SIH688X_TRIG_LEVEL_TRIGGER << index*2;
	} else {
		hp_err("error mode=%d\n", val[3]);
		return;
	}

	if (val[4] == SIH688X_TRIG_BOOST_EN) {
		sih_haptic->trig_para[index].boost_bypass =
			SIH688X_TRIG_BOOST_EN;
		boost |= 0 << 7;
	} else if (val[4] == SIH688X_TRIG_BOOST_BYPASS) {
		sih_haptic->trig_para[index].boost_bypass =
			SIH688X_TRIG_BOOST_BYPASS;
		boost |= 1 << 7;
	} else {
		hp_err("error boost_bypass = %d\n", val[4]);
		return;
	}

	pose_id = val[5];
	nege_id = val[6];

	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG_CTRL1,
		SIH_TRIG_CTRL1_BIT_TPOLAR0_MASK << index,
		ctrl1&(SIH_TRIG_CTRL1_BIT_TPOLAR0_MASK << index));
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG_CTRL1,
		SIH_TRIG_CTRL1_BIT_TRIG0_EN_MASK << index,
		ctrl1&(SIH_TRIG_CTRL1_BIT_TRIG0_EN_MASK << index));
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG_CTRL2,
		SIH_TRIG_CTRL2_BIT_TRIG0_MODE_MASK << index * 2,
		ctrl2&(SIH_TRIG_CTRL2_BIT_TRIG0_MODE_MASK << index * 2));
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG0_PACK_P + index * 2,
		SIH_TRIG_PACK_P_BIT_BOOST_BYPASS_MASK,
		boost&SIH_TRIG_PACK_P_BIT_BOOST_BYPASS_MASK);
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG0_PACK_P + index * 2,
		SIH_TRIG_PACK_P_BIT_TRIG_PACK_MASK,
		pose_id&SIH_TRIG_PACK_P_BIT_TRIG_PACK_MASK);
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG0_PACK_N + index * 2,
		SIH_TRIG_PACK_N_BIT_TRIG_PACK_MASK,
		nege_id&SIH_TRIG_PACK_N_BIT_TRIG_PACK_MASK);

	hp_info("ctrl1:0x%02x, ctrl2:0x%02x, boost:0x%02x, pose_id:0x%02x, nege_id:0x%02x\n",
		ctrl1, ctrl2, boost, pose_id, nege_id);
}

static size_t __maybe_unused sih688x_trig_para_get(
	sih_haptic_t *sih_haptic,
	char *buf)
{
	uint8_t trig_ctrl[2] = {0};
	size_t len = 0;

	hp_info("%s: enter\n", __func__);

	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_TRIG_CTRL1,
		SIH_I2C_OPERA_BYTE_TWO,
		trig_ctrl);

	len += snprintf(buf + len, PAGE_SIZE - len,
		"trig ctrl1:0x%02x, trig ctrl2:0x%02x\n",
		trig_ctrl[0], trig_ctrl[1]);

	return len;
}

static void __maybe_unused sih688x_set_ram_seq_gain(
	sih_haptic_t *sih_haptic,
	uint8_t wav,
	uint8_t gain)
{
	uint8_t tmp = 0;

	if (wav % 2 == 1) {
		tmp = gain << 4;
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GAIN_SET_SEQ1_0 + (wav / 2),
			WAVEGAIN_SEQ_ODD_MASK, tmp);
	} else {
		tmp = gain << 0;
		haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
			SIH688X_REG_GAIN_SET_SEQ1_0 + (wav / 2),
			WAVEGAIN_SEQ_EVEN_MASK, tmp);
	}
}

static size_t __maybe_unused sih688x_get_ram_seq_gain(
	sih_haptic_t *sih_haptic,
	char *buf)
{
	uint8_t i;
	uint8_t reg_val[4] = {0};
	size_t count = 0;

	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_GAIN_SET_SEQ1_0,
		SIH_I2C_OPERA_BYTE_FOUR,
		reg_val);

	for (i = 0; i < SIH_HAPTIC_SEQUENCER_GAIN_SIZE; i++) {
		sih_haptic->ram_para.gain[i * 2 + 0] = (reg_val[i] >> 0) & 0x0F;
		sih_haptic->ram_para.gain[i * 2 + 1] = (reg_val[i] >> 4) & 0x0F;
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d gain: 0x%02x\n", i * 2 + 0,
			sih_haptic->ram_para.gain[i * 2 + 0]);
		count += snprintf(buf + count, PAGE_SIZE - count,
			"seq%d gain: 0x%02x\n", i * 2 + 1,
			sih_haptic->ram_para.gain[i * 2 + 1]);
	}
	return count;
}

static int sih688x_read_id_code(
	sih_haptic_t *sih_haptic)
{
	int ret = -1;
	unsigned char loop_time = 0;
	int i;
	uint8_t success_time = 0;
	uint32_t chip_id_value = 0;

	while (loop_time < SIH_READ_CHIP_RETRY_TIME) {
		for (i = 0; i < sizeof(sih688x_chip_id_list) /
			sizeof(sih_chipid_reg_val_t); ++i) {
			ret = i2c_read_bytes(sih_haptic,
				(uint8_t)sih688x_chip_id_list[i].chip_id_reg,
				(uint8_t *)&chip_id_value,
				SIH_I2C_OPERA_BYTE_ONE);
			if (ret < 0)
				hp_err("%s i2c read id failed\n", __func__);
			if (ret >= 0 && sih688x_chip_id_list[i].reg_val == chip_id_value) {
				sih_haptic->chip_ipara.chip_id = chip_id_value;
				success_time++;
				hp_info("%s i2c read id success, id is %d\n",
					__func__, chip_id_value);
				break;
			}
		}

		if (!success_time)
			hp_err("%s: read chip id failed, restart\n", __func__);
		else
			return 0;

		loop_time++;

		usleep_range(SIH_READ_CHIP_RETRY_DELAY * 1000,
			SIH_READ_CHIP_RETRY_DELAY * 1000 + 500);
	}

	return -ENODEV;
}

static void sih688x_init_chip_func(
	sih_haptic_t *sih_haptic)
{
	sih_haptic->detect_para.detect_fifo =
		kmalloc(SIH_DETECT_FIFO_SIZE, GFP_KERNEL);
	if (sih_haptic->detect_para.detect_fifo == NULL) {
		hp_err("%s:detect fifo assign failed\n", __func__);
		sih_haptic->detect_para.detect_fifo_size = 0;
	} else {
		sih_haptic->detect_para.detect_fifo_size = SIH_DETECT_FIFO_SIZE;
	}

	sih_haptic->detect_para.detect_bemf_value =
		kmalloc(SIH_DETECT_FIFO_SIZE, GFP_KERNEL);
	if (sih_haptic->detect_para.detect_bemf_value == NULL) {
		hp_err("%s:detect fifo assign failed\n", __func__);
		sih_haptic->detect_para.detect_fifo_size = 0;
	} else {
		sih_haptic->detect_para.detect_fifo_size = SIH_DETECT_FIFO_SIZE;
	}

	sih_haptic->detect_para.detect_f0_value =
		kmalloc(SIH_DETECT_FIFO_SIZE, GFP_KERNEL);
	if (sih_haptic->detect_para.detect_f0_value == NULL) {
		hp_err("%s:detect fifo assign failed\n", __func__);
		sih_haptic->detect_para.detect_fifo_size = 0;
	} else {
		sih_haptic->detect_para.detect_fifo_size = SIH_DETECT_FIFO_SIZE;
	}

	sih_haptic->polar_cali_para.polar_bemf_value =
		kmalloc(SIH_DETECT_FIFO_SIZE, GFP_KERNEL);
	if (sih_haptic->polar_cali_para.polar_bemf_value == NULL) {
		hp_err("%s:polar fifo assign failed\n", __func__);
		sih_haptic->polar_cali_para.fifo_size = 0;
	} else {
		sih_haptic->polar_cali_para.fifo_size = SIH_DETECT_FIFO_SIZE;
	}

	sih_haptic->polar_cali_para.data_fifo =
		kmalloc(SIH_DETECT_FIFO_SIZE, GFP_KERNEL);
	if (sih_haptic->polar_cali_para.data_fifo == NULL) {
		hp_err("%s:polar fifo data assign failed\n", __func__);
		sih_haptic->polar_cali_para.fifo_size = 0;
	} else {
		sih_haptic->polar_cali_para.fifo_size = SIH_DETECT_FIFO_SIZE;
	}
}

static void sih688x_init_chip_parameter(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;

	sih_haptic->chip_ipara.support_attr.auto_pvdd = SIH_CHIP_ATTR_ENABLE;
	sih_haptic->chip_ipara.support_attr.auto_brake = SIH_CHIP_ATTR_ENABLE;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_IDLE_DEL_CNT, SIH_I2C_OPERA_BYTE_ONE, &reg_val);

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_PWM_PRE_GAIN, SIH_I2C_OPERA_BYTE_ONE, &reg_val);

	sih_haptic->chip_ipara.gain = (int)reg_val;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_ANA_CTRL3, SIH_I2C_OPERA_BYTE_ONE, &reg_val);

	switch (reg_val & SIH_ANA_CTRL3_BST_OUT_SEL_O_MASK) {
	case SIH_ANA_CTRL3_BST_OUT_SEL_6:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_6 *
		SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	case SIH_ANA_CTRL3_BST_OUT_SEL_7:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_7 *
			SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	case SIH_ANA_CTRL3_BST_OUT_SEL_8:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_8 *
			SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	case SIH_ANA_CTRL3_BST_OUT_SEL_9:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_9 *
			SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	case SIH_ANA_CTRL3_BST_OUT_SEL_10:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_10 *
			SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	case SIH_ANA_CTRL3_BST_OUT_SEL_11:
		sih_haptic->chip_ipara.drv_vboost = (int)SIH_ANA_CTRL_BST_LEVEL_11 *
			SIH_6887_DRV_VBOOST_COEFFICIENT;
		break;
	default:
		hp_err("%s: drv_vboost init failed\n", __func__);
	}

	sih688x_init_chip_func(sih_haptic);
}

static int sih688x_match_funclist(
	sih_haptic_t *sih_haptic)
{
	return sih688x_read_id_code(sih_haptic);
}

static void sih688x_update_chip_state(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val = 0;

	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SYSSST,
		SIH_I2C_OPERA_BYTE_ONE,
		&reg_val);

	if ((reg_val & SIH_SYSSST_BIT_STANDBY) == SIH_SYSSST_BIT_STANDBY)
		sih_haptic->chip_ipara.state = SIH_HAPTIC_STANDBY_MODE;
	else
		sih_haptic->chip_ipara.state = SIH_HAPTIC_ACTIVE_MODE;

	switch (reg_val & SIH_SYSSST_BIT_CENTRAL_STATE_MASK) {
	case SIH_SYSSST_BIT_RAM_STATE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_RAM_MODE;
		break;
	case SIH_SYSSST_BIT_RTP_STATE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_RTP_MODE;
		break;
	case SIH_SYSSST_BIT_TRIG_STATE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_TRIG_MODE;
		break;
	case SIH_SYSSST_BIT_F0_TRACK_STATE:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_CONT_MODE;
		break;
	default:
		sih_haptic->chip_ipara.play_mode = SIH_HAPTIC_IDLE_MODE;
		break;
	}
}

static void sih688x_efuse_check(
	sih_haptic_t *sih_haptic)
{
	uint8_t write_value = 0x02;
	uint8_t efuse_data[5] = {0};
	uint8_t crc_result = 0;
	uint8_t crc4_value;

	haptic_regmap_write(sih_haptic->regmapp.regmapping,
		SIH688X_REG_EFUSE_CTRL,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&write_value);
	haptic_regmap_bulk_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_EFUSE_RDATA0,
		SIH_I2C_OPERA_BYTE_FOUR,
		efuse_data);
	efuse_data[3] = efuse_data[3] & 0x0F;
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_EFUSE_RDATA4,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&efuse_data[4]);
	haptic_regmap_read(sih_haptic->regmapp.regmapping,
		SIH688X_REG_EFUSE_RDATA3,
		SIH_I2C_OPERA_BYTE_ONE,
		(char *)&crc4_value);
	efuse_data[4] = efuse_data[4] & 0x1F;
	crc4_value = (crc4_value & 0xF0) >> 4;
	crc_result = crc4_itu(efuse_data, sizeof(efuse_data)/sizeof(uint8_t));
	if (crc_result != crc4_value) {
		// default config
	}
	hp_info("crc_result:0x%02x crc_write:0x%02x\n", crc_result, crc4_value);
}

static int __maybe_unused sih688x_get_tracking_f0(
	sih_haptic_t *sih_haptic)
{
	uint8_t reg_val;
	uint8_t cnt = SIH688X_STANDBY_JUDGE_MAX_TIME;
	int i;
	uint8_t reg_config[][2] = {
		{0x8b, SIH688X_REG_T_LAST_MONITOR_L}, {0x36, SIH688X_REG_T_LAST_MONITOR_H},
		{0x4B, SIH688X_REG_VREF0}, {0x16, SIH688X_REG_VREF1}, {0x00, SIH688X_REG_VREF2},
		{0x04, SIH688X_REG_CYCLE0}, {0x3c, SIH688X_REG_CYCLE1}, {0x00, SIH688X_REG_CYCLE2},
		{0x24, SIH688X_REG_WATCHDOG_CNT_MAX}, {0x1a, SIH688X_REG_SEQ0_T_DRIVER},
		{0x06, SIH688X_REG_SEQ0_T_FLUSH}, {0x1a, SIH688X_REG_SEQ0_T_BEMF},
		{0x1a, SIH688X_REG_SEQ1_T_DRIVER}, {0x06, SIH688X_REG_SEQ1_T_FLUSH},
		{0x1a, SIH688X_REG_SEQ1_T_BEMF}, {0x1a, SIH688X_REG_SEQ2_T_DRIVER},
		{0x06, SIH688X_REG_SEQ2_T_FLUSH}, {0x1a, SIH688X_REG_SEQ2_T_BEMF},
	};
	/* enter standby mode */
	sih688x_stop(sih_haptic);
	/* f0 calibrate work mode */
	sih_haptic->detect_para.tracking_f0 = 0;

	for (i = 0 ; i < SIH688X_TRACKING_CONFIGE_SIZE; i++) {
		reg_val = reg_config[i][0];
		haptic_regmap_write(sih_haptic->regmapp.regmapping,
		reg_config[i][1], SIH_I2C_OPERA_BYTE_ONE, (char *)&reg_val);
		hp_info("reg_config 0:0x%02x reg_config 1:0x%02x\n", reg_config[i][0], reg_config[i][1]);
	}
	/* smooth f0 window ctrl off */
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SMOOTH_F0_WINDOW_OUT,
		SIH_SMOOTH_F0_DETECT_MASK,
		SIH_SMOOTH_F0_DETECT_OFF);
	/* f0 tracking en */
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_MAIN_STATE_CTRL,
		SIH_MODECTRL_BIT_TRACK_F0_MASK,
		SIH_MODECTRL_BIT_TRACK_F0_OFF);
	/* smooth f0 window ctrl en */
	haptic_regmap_update_bits(sih_haptic->regmapp.regmapping,
		SIH688X_REG_SMOOTH_F0_WINDOW_OUT,
		SIH_SMOOTH_F0_DETECT_MASK,
		SIH_SMOOTH_F0_DETECT_EN);
	/* cont go */
	cont_config_go(sih_haptic, 1);
	/* wait for standby */
	while (cnt--) {
		usleep_range(2000, 2500);
		if (sih688x_if_is_mode(sih_haptic, SIH_HAPTIC_IDLE_MODE))
			break;
	}
	/* read f0 data */
	sih688x_read_tracking_f0(sih_haptic);
	hp_info("tracking_f0:%d\n", sih_haptic->detect_para.tracking_f0);

	return 0;
}

haptic_func_t sih_688x_func_list = {
	.match_func_list = sih688x_match_funclist,
	.play_stop = sih688x_stop,
	.ram_init = sih688x_ram_init,
	.get_vbat = sih688x_get_vbat,
	.update_cont_config = sih688x_update_cont_config,
	.get_interrupt_state = sih688x_get_interrupt_state,
	.get_lra_resistance = sih688x_get_lra_resistance,
	.set_play_mode = sih688x_set_play_mode,
	.set_drv_bst_vol = sih688x_set_drv_bst_vol,
	.interrupt_state_init = sih688x_interrupt_state_init,
	.set_repeat_seq = sih688x_set_repeat_seq,
	.set_wav_seq = sih688x_set_wav_seq,
	.set_wav_internal_loop = sih688x_set_wav_internal_loop,
	.set_ram_addr = sih688x_set_ram_addr,
	.write_rtp_data = sih688x_write_rtp_data,
	.update_ram_config = sih688x_update_ram_config,
	.clear_interrupt_state = sih688x_clear_interrupt_state,
	.get_wav_internal_loop = sih688x_get_wav_internal_loop,
	.play_go = sih688x_play_go,
	.set_rtp_aei = sih688x_set_rtp_aei,
	.set_gain = sih688x_set_gain,
	.upload_lra = sih688x_upload_lra,
	.get_all_reg = sih688x_get_all_reg,
	.get_ram_data = sih688x_get_ram_data,
	.get_first_wave_addr = sih688x_get_first_wave_addr,
	.if_rtp_fifo_full = sih688x_if_rtp_fifo_full,
	.get_wav_seq = sih688x_get_wav_seq,
	.set_auto_pvdd = sih688x_set_auto_pvdd,
	.set_wav_external_loop = sih688x_set_wav_external_loop,
	.get_wav_external_loop = sih688x_get_wav_external_loop,
	.if_is_mode = sih688x_if_is_mode,
	.set_low_power_mode = sih688x_set_low_power_mode,
	.set_trig_para = sih688x_trig_para_set,
	.get_trig_para = sih688x_trig_para_get,
	.set_ram_seq_gain = sih688x_set_ram_seq_gain,
	.get_ram_seq_gain = sih688x_get_ram_seq_gain,
	.read_detect_fifo = sih688x_read_detect_fifo,
	.init_chip_parameter = sih688x_init_chip_parameter,
	.update_chip_state = sih688x_update_chip_state,
	.f0_cali = sih688x_f0_cali,
	.trim_init = sih688x_trim_init,
	.osc_trim_calibration = sih688x_osc_trim_calibration,
	.efuse_check = sih688x_efuse_check,
	.get_tracking_f0 = sih688x_get_tracking_f0,
};

haptic_func_t *sih_688x_func(void)
{
	return &sih_688x_func_list;
}