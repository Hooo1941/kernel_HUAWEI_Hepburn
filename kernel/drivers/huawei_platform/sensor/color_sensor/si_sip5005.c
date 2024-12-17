/*
 * si_sip5005.c
 *
 * code for SI sip5005 sensor
 *
 * Copyright (c) 2024 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/unistd.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>

#include "si_sip5005.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

/*
* defines for special tests
* define EXTENDED_LOG for log test
* define TEST_COMPARE_SMUX_CONFIG for Test
*/

#define EXTENDED_LOG
#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static bool fd_gain_adjusted;
static bool fd_timer_stopped;
static bool color_calibrate_result = true;
static bool report_calibrate_result;
static bool skip_first_rgb_data;
static u16 ring_buffer[MAX_BUFFER_SIZE];
static u32 read_out_buffer[MAX_BUFFER_SIZE];
static u16 *buffer_ptr = &ring_buffer[0];
static u16 *head_ptr = &ring_buffer[0];
static u16 *tail_ptr = &ring_buffer[0];
static u8 first_circle_end;
static DEFINE_MUTEX(ring_buffer_mutex);
static DEFINE_MUTEX(enable_handle_mutex);
static struct color_chip *p_chip;
static int enable_status_before_cali;
static UINT8 report_logcount;
static UINT32 algo_support;
static UINT32 get_oeminfo_data_support;
static u32 max_flicker;
static UINT16 highest_digits;
static bool cal_normal;
static enum sip5005_sensor_id_t sensor_type;

extern int (*color_default_enable)(bool enable);
extern UINT32 flicker_support;
extern int color_report_val[MAX_REPORT_LEN];
static void read_fd_data(struct color_chip *chip);

static uint8_t find_highest_bit(uint32_t value)
{
	int8_t i;

	if (value == 0)
		return 0;

	for (i = INTEGER_BIT_SIZE - 1; i >= 0; i--) {
		if ((value >> (uint8_t)i) & 1)
			return (uint8_t)i;
	}
	return 0;
}

static void sip5005_get_optimized_gain(UINT32 maximum_adc, UINT32 highest_adc, UINT8 *p_gain,
	UINT8 *p_saturation)
{
	UINT32 gain_change;

	if (highest_adc == 0)
		highest_adc = 1;

	if (highest_adc >= maximum_adc) {
		/* saturation detected */
		if (*p_gain > LOW_AUTO_GAIN_VALUE)
			*p_gain /= AUTO_GAIN_DIVIDER;
		else
			*p_gain = LOWEST_GAIN_ID;
		*p_saturation = IS_SATURATION;
	} else {
		/* value too low, increase the gain to 80% maximum */
		gain_change = (SATURATION_LOW_PERCENT * maximum_adc) / (SATURATION_HIGH_PERCENT * highest_adc);
		if (gain_change == 0 && *p_gain != 0) {
			(*p_gain)--;
		} else {
			gain_change = find_highest_bit(gain_change);
			if (((uint32_t)(*p_gain) + gain_change) > MAX_GAIN_ID)
				*p_gain = MAX_GAIN_ID;
			else
				*p_gain += (uint8_t)gain_change;
		}
		*p_saturation = !IS_SATURATION;
	}
}

static void sip5005_get_field(struct i2c_client *handle, UINT8 reg, UINT8 *data, UINT8 mask)
{
	if (!handle || !data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	color_sensor_get_byte(handle, reg, data);
	*data &= mask;
}

static void sip5005_set_field(struct i2c_client *handle, UINT8 reg, UINT8 data, UINT8 mask)
{
	UINT8 original_data = 0;
	UINT8 new_data;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&enable_handle_mutex);
	color_sensor_get_byte(handle, reg, &original_data);
	new_data = original_data & (~mask);
	new_data |= (data & mask);
	color_sensor_set_byte(handle, reg, new_data);
	mutex_unlock(&enable_handle_mutex);
}

static int sip5005_rgb_report_type(void)
{
	return AWB_SENSOR_RAW_SEQ_TYPE_SIP5005;
}

static int sip5005_report_data(int value[])
{
	return ap_color_report(value, SI_REPORT_DATA_LEN * sizeof(int));
}

static enum sip5005_sensor_id_t sip5005_get_chipid(struct i2c_client *handle)
{
	UINT8 chipid = 0;
	UINT8 i;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return SI_UNKNOWN_DEVICE;
	}
	color_sensor_get_byte(handle, SIP5005_ID_REG, &chipid);
	for (i = 0; i < ARRAY_SIZE(sip5005_ids); i++) {
		if (sip5005_ids[i] == chipid) {
			hwlog_info("get this chip info, %s: Chip is SIP5005\n", __func__);
			sensor_type = SI_SIP5005_REV0;
			return sensor_type;
		}
	}
	return SI_UNKNOWN_DEVICE;
}
/* added modify idac config and updata some idac value , there are five channels of idac */
static void sip5005_set_configuration_for_channels(struct sip5005_ctx *ctx,
	struct sip5005_channel_idac *ch_idac)
{
	UINT8 idac[6] = {0};
	UINT8 temp_code = 0;

	sip5005_set_field(ctx->handle, SIP5005_AL_IDAC_MODE, 0x1F, 0x1F);

	temp_code =	 ((ch_idac_config[0].ch_idac_high) << 0) | ((ch_idac_config[1].ch_idac_high) << 1)
				| ((ch_idac_config[2].ch_idac_high) << 2) | ((ch_idac_config[3].ch_idac_high) << 3)
				| ((ch_idac_config[4].ch_idac_high) << 4);

	color_sensor_set_byte(ctx->handle, SIP5005_AL_IDAC_MSB, temp_code);
	color_sensor_set_byte(ctx->handle, SIP5005_AL0_IDAC, ch_idac[0].ch_idac_low);
	color_sensor_set_byte(ctx->handle, SIP5005_AL1_IDAC, ch_idac[1].ch_idac_low);
	color_sensor_set_byte(ctx->handle, SIP5005_AL2_IDAC, ch_idac[2].ch_idac_low);
	color_sensor_set_byte(ctx->handle, SIP5005_AL3_IDAC, ch_idac[3].ch_idac_low);
	color_sensor_set_byte(ctx->handle, SIP5005_AL4_IDAC, ch_idac[4].ch_idac_low);
	color_sensor_set_byte(ctx->handle, SIP5005_AL_AZ_AUTO_NUM, 0x00);
	color_sensor_read_fifo(ctx->handle, SIP5005_AL_IDAC_MSB, &idac[0], SIP5005_IDAC_MSB_NUM);
	hwlog_info("idac new reg[0x66~0x6B], = [0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]", idac[0],
		idac[1], idac[2], idac[3], idac[4], idac[5]);
	color_sensor_set_byte(ctx->handle, SIP5005_AL_AZ_AUTO_NUM, 0x00);
}

static void sip5005_adjust_offset_config(struct sip5005_ctx *ctx)
{
	UINT8 idac[6] = {0};
	int ch_idac_temp = 0;
	int i = 0;

	color_sensor_read_fifo(ctx->handle, SIP5005_AL_IDAC_MSB, &idac[0], SIP5005_IDAC_MSB_NUM);
	hwlog_info("idac reg[0x66~0x6B], = [0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]",
		idac[0], idac[1], idac[2], idac[3], idac[4], idac[5]);

	ch_idac_config[0].ch_idac_low = idac[1];
	ch_idac_config[1].ch_idac_low = idac[2];
	ch_idac_config[2].ch_idac_low = idac[3];
	ch_idac_config[3].ch_idac_low = idac[4];
	ch_idac_config[4].ch_idac_low = idac[5];

	ch_idac_config[0].ch_idac_high = (idac[0] & 0x01);
	ch_idac_config[1].ch_idac_high = (idac[0] & 0x02);
	ch_idac_config[2].ch_idac_high = (idac[0] & 0x04);
	ch_idac_config[3].ch_idac_high = (idac[0] & 0x08);
	ch_idac_config[4].ch_idac_high = (idac[0] & 0x10);

	for (i = 0; i < ALSPS_CFG_MAX_CHANNELS; i++) {
		if (ch_idac_config[i].ch_idac_high > 0)
			ch_idac_temp = (-1) * ch_idac_config[i].ch_idac_low;
		else
			ch_idac_temp = ch_idac_config[i].ch_idac_low;

		ch_idac_temp -= 5;

		if (ch_idac_temp < 0)
			ch_idac_config[i].ch_idac_high = 1;
		else
			ch_idac_config[i].ch_idac_high = 0;

		ch_idac_config[i].ch_idac_low = abs(ch_idac_temp);
	}
	/* Set the configuration for the channels */
	sip5005_set_configuration_for_channels(ctx, &ch_idac_config[0]);
}

static int sip5005_modify_als_enable(struct sip5005_ctx *ctx, UINT8 mask, UINT8 val)
{
	int ret = 0;
	UINT8 als_enable = 0;

	color_sensor_get_byte(ctx->handle, SIP5005_AL_CTRL0, &als_enable);
	sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, 0, AL_EN_ALL);
	sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, val, mask);
	if ((als_enable & AL_EN) == 0) {
		if ((val & AL_EN_ALL) == AL_EN_ALL) {
			mdelay(SIP5005_ALS_ENABLE_TIME); /* delay 2ms */
			sip5005_adjust_offset_config(ctx);
		}
	}
	return ret;
}

static INT32 sip5005_als_get_gain(const void *ctx)
{
	UINT8 cfg1_reg = 0;
	INT32 gain = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	sip5005_get_field(((struct sip5005_ctx *)ctx)->handle, SIP5005_AL_GAIN_1_0, &cfg1_reg, 0x0F);

	if (cfg1_reg <= ARRAY_SIZE(sip5005_als_gain))
		gain = sip5005_als_gain[cfg1_reg - 1];

	hwlog_info("now the gain val = %d\n", gain);
	return gain;
}

static void sip5005_als_set_gain(struct sip5005_ctx *p_sip5005, UINT8 gain_id)
{
	UINT32 gain_value = 0;
	UINT8 cfg1 = 0;
	UINT8 enable = 0;

	if (!p_sip5005) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (gain_id > MAX_GAIN_ID) {
		hwlog_err("%s: gain_id out of range: %u\n", __func__, gain_id);
		return;
	}

	if (gain_id == LOWEST_GAIN_ID)
		gain_value = LOWEST_GAIN_VALUE;
	else
		gain_value = (1 << (gain_id - 1)) * SIP5005_GAIN_SCALE;

	cfg1 = gain_id + 1;

	color_sensor_set_byte(p_sip5005->handle, SIP5005_AL_GAIN_1_0, cfg1 | (cfg1 << 4));
	color_sensor_set_byte(p_sip5005->handle, SIP5005_AL_GAIN_3_2, cfg1 | (cfg1 << 4));
	sip5005_set_field(p_sip5005->handle, SIP5005_AL_GAIN_5_4, cfg1, AGAIN_MASK);
	color_sensor_get_byte(p_sip5005->handle, SIP5005_AL_CTRL0, &enable);

	if (enable & AL_EN_ALL) {
		sip5005_set_field(p_sip5005->handle, SIP5005_AL_CTRL0, 0, AL_EN_ALL);
		sip5005_set_field(p_sip5005->handle, SIP5005_AL_CTRL0, AL_EN_ALL, AL_EN_ALL);
	}
	
	p_sip5005->alg_ctx.gain_id = gain_id;
	p_sip5005->alg_ctx.gain = gain_value;
	hwlog_info("%s gain = %d, id = %u, cfg1 = 0x%x\n", __func__, gain_value, gain_id, cfg1);
}

static UINT8 sip5005_als_get_saved_gain(struct sip5005_ctx *p_sip5005)
{
	if (!p_sip5005) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	return p_sip5005->alg_ctx.gain_id;
}

static void sip5005_als_set_itime(struct sip5005_ctx *p_sip5005, int itime_ms)
{
	INT32 als_time;
	uint8_t low_8_bit = 0;
	uint8_t middle_8_bit = 0;
	uint8_t high_8_bit = 0;

	if (!p_sip5005) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (itime_ms < 0) {
		hwlog_err("%s: itime_ms out of range: %d\n", __func__, itime_ms);
		return;
	}

	als_time = (uint32_t)((uint32_t)(itime_ms * SIP5005_ROUNDING_VAL) / ATIME_DUTY_CYCLE);
	high_8_bit = (uint8_t)(als_time >> SIP5005_HIGH_8_BIT);
	middle_8_bit = (uint8_t)((als_time >> SIP5005_MIDDLE_8_BIT) & SIP5005_MIDDLE_8_BIT_OFFSET);
	low_8_bit = (uint8_t)(als_time & SIP5005_LOW_8_BIT_OFFSET);

	color_sensor_set_byte(p_sip5005->handle, SIP5005_ALS_INTE_TIME_H, high_8_bit);
	color_sensor_set_byte(p_sip5005->handle, SIP5005_ALS_INTE_TIME_M, middle_8_bit);
	color_sensor_set_byte(p_sip5005->handle, SIP5005_ALS_INTE_TIME_L, low_8_bit);

	p_sip5005->alg_ctx.itime_ms = itime_ms;
	p_sip5005->alg_ctx.astep = als_time;
	hwlog_info("%s als_itime: %dms', als_atime: %d\n", __func__, itime_ms, p_sip5005->alg_ctx.astep);
}

static UINT16 sip5005_als_get_fullscale(struct sip5005_ctx *p_sip5005)
{
	UINT32 value;

	if (!p_sip5005) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}

	value = ((UINT32)(p_sip5005->alg_ctx.astep) + 1);

	if (value > MAX_ADC_COUNT)
		value = MAX_ADC_COUNT;
	return (uint16_t)value;
}

static void sip5005_log_buffer(UINT8 *p_name, UINT32 *p_buf, UINT8 size)
{
	if (!p_name || !p_buf) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (size != SI_REPORT_DATA_LEN)
		return;

	hwlog_info("%s : %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", p_name,
				p_buf[0], p_buf[1], p_buf[2], p_buf[3], p_buf[4], p_buf[5],
				p_buf[6], p_buf[7], p_buf[8], p_buf[9]);
}

static void calculate_fd_sat_threshold(struct sip5005_ctx *ctx)
{
	UINT8 data[2] = {0};
	UINT16 saturation;

	if (!ctx) {
		hwlog_err("%s NULL poniter\n", __func__);
		return;
	}

	color_sensor_get_byte(ctx->handle, SIP5005_FD_INTE_TIME_H, &data[0]);
	color_sensor_get_byte(ctx->handle, SIP5005_FD_INTE_TIME_L, &data[1]);
	saturation = (((UINT16)(data[0] << 8) | data[1]) + 2);

	max_flicker = (saturation * SATURATION_LOW_PERCENT) / SATURATION_HIGH_PERCENT;
	hwlog_info("%s , max_flicker threshold: %u\n", __func__, max_flicker);
}

/* Ten channel values are placed in four registers */
static void sip5005_updata_ch_otp_value(UINT8 *otp)
{
	UINT8 i = 0;

	ch_scale[0].value = (otp[0] & 0x07);
	ch_scale[1].value = (otp[0] >> 3) & 0x07;
	ch_scale[2].value = ((otp[0] >> 6) & 0x03) | ((otp[1] & 0x01) << 2);
	ch_scale[3].value = ((otp[1] >> 1) & 0x07);
	ch_scale[4].value = ((otp[1] >> 4) & 0x07);
	ch_scale[5].value = ((otp[1] >> 7) & 0x01) | ((otp[2] & 0x03) << 1);
	ch_scale[6].value = ((otp[2] >> 2) & 0x07);
	ch_scale[7].value = ((otp[2] >> 5) & 0x07);
	ch_scale[8].value = (otp[3] & 0x07);
	ch_scale[9].value = (otp[3] >> 3) & 0x07;

	for (i = 0; i < SI_REPORT_DATA_LEN; i++)
		hwlog_info("%s, ch_scale_value: %u\n", __func__, ch_scale[i].value);
}

static void sip5005_cal_scale(struct sip5005_ctx *ctx)
{
	UINT8 otp[4] = {0};
	UINT8 otp_id = 0;
	UINT8 i = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	/* OTOD0 stores the feature values of different chips. */
	color_sensor_get_byte(ctx->handle, SIP5005_OTPD0, &otp_id);
	color_sensor_read_fifo(ctx->handle, SIP5005_OTPD4, &otp[0], 4);
	sip5005_updata_ch_otp_value(otp);

	if ((otp_id & 0x1F) != 0x1F) {  /* different chips are calibrated in different ways. */
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			ch_scale[i].scale = -175 + 50 * (ch_scale[i].value & 0x07);  /* offsets and coefficients */
	} else {
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			ch_scale[i].scale = -100 + 25 * (ch_scale[i].value & 0x07);
	}

	for (i = 0; i < SI_REPORT_DATA_LEN; i++)
		ch_scale[i].scale = SIP5005_INTEGER_VAL * SIP5005_INTEGER_VAL /
		(SIP5005_INTEGER_VAL + ch_scale[i].scale);
}

static void sip5005_reset_regs(struct sip5005_ctx *ctx)
{
	UINT8 i;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(sip5005_settings); i++)
		color_sensor_set_byte(ctx->handle, sip5005_settings[i].reg, sip5005_settings[i].value);

	sip5005_als_set_gain(ctx, DEFAULT_ALS_GAIN_ID);
	sip5005_als_set_itime(ctx, SIP5005_ITIME_DEFAULT);
	calculate_fd_sat_threshold(ctx);

	ctx->alg_ctx.load_calibration = true;
	/* initialize calibration default values */
	for (i = 0; i < SI_REPORT_DATA_LEN; i++) {
		ctx->cal_ctx.ratio[i] = SIP5005_CAL_RATIO;
		ctx->cal_ctx.target[i] = 0;
	}
}

static UINT8 sip5005_get_fd_gain_reg_value(int gain)
{
	UINT8 cfg1;

	switch (gain) {
	case ALS_GAIN_VALUE_1:
		cfg1 = FD_GAIN_0_5X;
		break;
	case ALS_GAIN_VALUE_2:
		cfg1 = FD_GAIN_1X;
		break;
	case ALS_GAIN_VALUE_3:
		cfg1 = FD_GAIN_2X;
		break;
	case ALS_GAIN_VALUE_4:
		cfg1 = FD_GAIN_4X;
		break;
	case ALS_GAIN_VALUE_5:
		cfg1 = FD_GAIN_8X;
		break;
	case ALS_GAIN_VALUE_6:
		cfg1 = FD_GAIN_16X;
		break;
	case ALS_GAIN_VALUE_7:
		cfg1 = FD_GAIN_32X;
		break;
	case ALS_GAIN_VALUE_8:
		cfg1 = FD_GAIN_64X;
		break;
	case ALS_GAIN_VALUE_9:
		cfg1 = FD_GAIN_128X;
		break;
	case ALS_GAIN_VALUE_10:
		cfg1 = FD_GAIN_256X;
		break;
	case ALS_GAIN_VALUE_11:
		cfg1 = FD_GAIN_512X;
		break;
	case ALS_GAIN_VALUE_12:
		cfg1 = FD_GAIN_1024X;
		break;
	default:
		cfg1 = FD_GAIN_2048X;
		break;
	}
	return cfg1;
}

static INT32 sip5005_set_fd_gain(struct sip5005_ctx *ctx, int gain)
{
	UINT8 cfg1;
	UINT32 old_gain;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	old_gain = ctx->flc_ctx.fd_gain;
	cfg1 = sip5005_get_fd_gain_reg_value(gain);

	// close FD_EN
	sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, 0, FD_EN);
	sip5005_set_field(ctx->handle, SIP5005_FIFO_CFG2, FIFO_CLEAR, FIFO_CLEAR);
	// open FDEN
	sip5005_set_field(ctx->handle, SIP5005_AL_GAIN_5_4, cfg1 << 4, AGAIN_MASK << 4);
	sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, FD_EN, FD_EN);

	ctx->flc_ctx.fd_gain = gain;
	hwlog_info("%s old = %u, new = %d, cfg1 = 0x%02x\n", __func__, old_gain, gain, cfg1);
	return 0;
}

static UINT8 get_rgb_fd_enable_status(struct i2c_client *handle)
{
	UINT8 enable_sta = 0;

	if (!handle) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	color_sensor_get_byte(handle, SIP5005_AL_CTRL0, &enable_sta);
	return enable_sta;
}

static INT32 sip5005_set_als_onoff(struct sip5005_ctx *ctx, UINT8 data, bool cal_mode)
{
	INT32 result = 0;
	UINT8 all_al_int = 0;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	hwlog_info("%s set_als_onoff data is %u\n", __func__, data);

	/* reset interrupt status */
	color_sensor_get_byte(ctx->handle, SIP5005_ALL_AL_INT, &all_al_int);

	if (data == 0) {
		/* disable als measurement */
		sip5005_set_field(ctx->handle, SIP5005_AL_IDAC_MODE, 0x00, 0x1F);
		color_sensor_set_byte(ctx->handle, SIP5005_AL_AZ_AUTO_NUM, 0xFF);
		sip5005_modify_als_enable(ctx, AL_EN_ALL, 0);
		hwlog_info("%s call modify_als_enable data is %u\n", __func__, data);
	} else {
		/* For first integration after AEN */
		ctx->alg_ctx.meas_steps = STEP_DEFAULT_MEASURE;
		ctx->alg_ctx.als_update = 0;
		if (!cal_mode) {
			ctx->alg_ctx.meas_steps = STEP_FAST_AUTO_GAIN;
			sip5005_als_set_itime(ctx, SIP5005_ITIME_FOR_FIRST_DATA);
			sip5005_als_set_gain(ctx, SIP5005_AGAIN_FOR_FIRST_DATA);
		} else {
			sip5005_als_set_itime(ctx, SIP5005_ITIME_DEFAULT);
			sip5005_als_set_gain(ctx, DEFAULT_CALIB_SIP5005_GAIN_ID);
		}
		/* enable als measurement */
		hwlog_info("%s call modify_als_enable data is %u\n", __func__, data);
		sip5005_modify_als_enable(ctx, AL_EN_ALL, AL_EN_ALL);
	}

	hwlog_info("%s set_als_onoff is enable status %u\n", __func__, data);
	return result;
}

static void sip5005_handle_first_integration(struct sip5005_ctx *ctx)
{
	UINT8 old_gain_id;
	UINT8 gain_id;
	UINT8 saturation;

	gain_id = sip5005_als_get_saved_gain(ctx);
	old_gain_id = gain_id;

	sip5005_get_optimized_gain(sip5005_als_get_fullscale(ctx), highest_digits, &gain_id, &saturation);
	if (old_gain_id != gain_id) {
		hwlog_info("first gain adjust - old gain: %u, saturation: %u\n", old_gain_id, saturation);
		sip5005_als_set_gain(ctx, gain_id);
	}

	if (!saturation) {
		sip5005_als_set_itime(ctx, SIP5005_ITIME_FAST);
		ctx->alg_ctx.meas_steps = STEP_FAST_MEASURE;
	}
}

static bool sip5005_handle_block(struct sip5005_ctx *ctx, bool cal_mode)
{
	UINT8 old_gain_id;
	UINT8 gain_id;
	UINT8 saturation;
	bool re_enable = false;
	UINT16 maximum;

	if (cal_mode) {
		ctx->alg_ctx.als_update |= SI_SIP5005_FEATURE_ALS;
	} else {
		gain_id = sip5005_als_get_saved_gain(ctx);
		old_gain_id = gain_id;
		maximum = highest_digits;
		sip5005_get_optimized_gain(sip5005_als_get_fullscale(ctx), maximum, &gain_id, &saturation);
		if (old_gain_id != gain_id) {
			hwlog_info("gain adjust - old: %u, sat: %u\n", old_gain_id, saturation);
			sip5005_als_set_gain(ctx, gain_id);
		}

		if (saturation)
			re_enable = true;
		/* in case gain change but als just output raw */
		else if (old_gain_id == gain_id)
			ctx->alg_ctx.als_update |= SI_SIP5005_FEATURE_ALS;
	}
	return re_enable;
}

static void adjust_raw_values_on_itime(struct sip5005_ctx *ctx)
{
	int i;

	if (!(cal_normal && (sensor_type == SI_SIP5005_REV0))) {
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			ctx->alg_ctx.data[i] =
				(ctx->alg_ctx.data[i] * SIP5005_ITIME_DEFAULT * 11) / (SIP5005_ITIME_FAST * 10);
		sip5005_als_set_itime(ctx, SIP5005_ITIME_DEFAULT);
	}
	sip5005_log_buffer("INT_ADJUST", ctx->alg_ctx.data, SI_REPORT_DATA_LEN);
	ctx->alg_ctx.meas_steps = STEP_DEFAULT_MEASURE;
}

static void sip5005_save_data(struct sip5005_ctx *ctx, UINT16 *p_adc_data)
{
	ctx->alg_ctx.data[CHAN_F2] = p_adc_data[0];
	ctx->alg_ctx.data[CHAN_FZ] = p_adc_data[1];
	ctx->alg_ctx.data[CHAN_F3] = p_adc_data[2];
	ctx->alg_ctx.data[CHAN_F4] = p_adc_data[3];
	ctx->alg_ctx.data[CHAN_FY] = p_adc_data[4];
	ctx->alg_ctx.data[CHAN_FX] = p_adc_data[5];
	ctx->alg_ctx.data[CHAN_F6] = p_adc_data[6];
	ctx->alg_ctx.data[CHAN_F7] = p_adc_data[7];
	ctx->alg_ctx.data[CHAN_NIR] = p_adc_data[8];
	ctx->alg_ctx.data[CHAN_VIS] = p_adc_data[9];
}

static void switch_channel_sequence(UINT16 *si_channel, UINT16 *standard_channel)
{
	standard_channel[0] = si_channel[5];
	standard_channel[1] = si_channel[1];
	standard_channel[2] = si_channel[6];
	standard_channel[3] = si_channel[7];
	standard_channel[4] = si_channel[0];
	standard_channel[5] = si_channel[2];
	standard_channel[6] = si_channel[8];
	standard_channel[7] = si_channel[3];
	standard_channel[8] = si_channel[4];
	standard_channel[9] = si_channel[9];
}

static bool sip5005_handle_als(struct sip5005_ctx *ctx, bool cal_mode)
{
	UINT8 adc_data[ALS_CHANNELS * 2] = {0};
	UINT16 channel_data[ALS_CHANNELS] = {0};
	UINT16 channel_adjust[ALS_CHANNELS] = {0};
	bool re_enable = false;
	UINT8 i;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return true;
	}

	color_sensor_read_fifo(ctx->handle, SIP5005_AL0_DATA_H, &(adc_data[0]), RAW_DATA_BYTE_NUM);
	for (i = 0; i < RAW_DATA_BYTE_NUM; i += 2)
		channel_data[i / 2] = (UINT16)(adc_data[i] << 8) | adc_data[i + 1];

	for (i = 0; i < SI_REPORT_DATA_LEN; i++)
		channel_data[i] = (channel_data[i] * ch_scale[i].scale) / SIP5005_INTEGER_VAL;

	switch_channel_sequence(channel_data, channel_adjust);
	
	highest_digits = channel_data[0];
	for (i = 1; i < ALS_CHANNELS - 3; i++)
		if (highest_digits < channel_data[i])
			highest_digits = channel_data[i];

	if (ctx->alg_ctx.meas_steps == STEP_FAST_AUTO_GAIN) {
		sip5005_handle_first_integration(ctx);
		re_enable = true;
	} else {
		re_enable = sip5005_handle_block(ctx, cal_mode);
	}

	if (!re_enable) {
		sip5005_save_data(ctx, channel_adjust);
		if ((ctx->alg_ctx.meas_steps == STEP_FAST_MEASURE) && ctx->alg_ctx.als_update)
			adjust_raw_values_on_itime(ctx);
	}

	if (ctx->alg_ctx.enabled)
		sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, AL_EN_ALL, AL_EN_ALL);
	return re_enable;
}

static bool sip5005_handle_als_event(struct sip5005_ctx *ctx, bool cal_mode)
{
	bool ret = false;

	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return false;
	}

	color_sensor_get_byte(ctx->handle, SIP5005_ALL_AL_INT, &ctx->alg_ctx.status);
	color_sensor_get_byte(ctx->handle, SIP5005_DATA_VALID, &ctx->alg_ctx.data_valid);

	if (ctx->alg_ctx.data_valid & ALS_DATA_VALID)
		ret = sip5005_handle_als(ctx, cal_mode);  /* only get available staus, then read data */
	else
		hwlog_info("%s: No new als data were available\n", __func__);
	return ret;
}

static UINT8 sip5005_get_als_update(struct sip5005_ctx *ctx)
{
	if (!ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return 0;
	}
	return ctx->alg_ctx.als_update;
}

static void sip5005_get_als_data(struct sip5005_ctx *ctx, UINT32 *p_data, UINT32 num)
{
	if (!ctx || !p_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	if (num != SI_REPORT_DATA_LEN) {
		hwlog_err("%s: length out of range: %u\n", __func__, num);
		return;
	}

	memcpy_s(p_data, num * sizeof(UINT32), ctx->alg_ctx.data, num * sizeof(UINT32));
	ctx->alg_ctx.als_update &= ~(SI_SIP5005_FEATURE_ALS);
}

void sip5005_als_timer_wrk(struct timer_list *data)
{
	struct color_chip *chip = from_timer(chip, data, work_timer);

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	queue_work(system_power_efficient_wq, &chip->als_work);
}

void sip5005_flc_timer_wrk(struct timer_list *data)
{
	struct color_chip *chip = from_timer(chip, data, fd_timer);

	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	queue_work(system_power_efficient_wq, &chip->fd_work);
}

static ssize_t sip5005_als_set_enable(struct color_chip *chip, uint8_t en)
{
	UINT32 i;
	struct sip5005_ctx *ctx = NULL;
	UINT32 autozero_offset = 0;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return 0;
	}

	ctx = (struct sip5005_ctx *)chip->device_ctx;
	hwlog_info("%s = %u\n", __func__, en);

	if (ctx->alg_ctx.enabled && (en == 1)) {
		hwlog_info("%s, rgb already enabled, return\n", __func__);
		return 0;
	}

	if (!ctx->alg_ctx.enabled && (en == 0)) {
		hwlog_info("%s, rgb already disabled, return\n", __func__);
		return 0;
	}

	ctx->alg_ctx.enabled = en;  /* inconsistent status update status */
	if (sip5005_set_als_onoff(chip->device_ctx, en, chip->in_cal_mode))
		autozero_offset = AUTOZERO_TIMER_OFFSET;

	if (en == 0) {
		hwlog_info("%s: close rgb, del_timer\n", __func__);
		return 0;
	}

	/* set default report val to -1 when enable */
	color_report_val[0] = SI_REPORT_DATA_LEN;
	for (i = 1; i < MAX_REPORT_LEN; i++)
		color_report_val[i] = -1;

	mod_timer(&chip->work_timer, jiffies + msecs_to_jiffies(autozero_offset +
		((struct sip5005_ctx *)chip->device_ctx)->alg_ctx.itime_ms + HIGH_TIMER_OFFSET));

	report_logcount = 0;
	return 0;
}

static int get_cal_para_from_nv(struct sip5005_calib_ctx_t *p_calib)
{
	int ret;
	int i;

	if (!p_calib) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: read calib data with size %d\n", __func__,
			   sizeof(struct sip5005_calib_ctx_t));
	ret = read_color_calibrate_data_from_nv(RGBAP_CALI_DATA_NV_NUM,
											sizeof(struct sip5005_calib_ctx_t),
											"RGBAP", (char *)p_calib);
	if (ret < 0) {
		hwlog_err("%s: failed with error code %d\n", __func__, ret);
		return -ENODATA;
	}
	for (i = 0; i < SI_REPORT_DATA_LEN; i++) {
		if (p_calib->ratio[i] == 0) {
			hwlog_err("%s: read calib ratio from mem %d\n", __func__, i);
			return -EILSEQ;
		}
	}
	return ret;
}

static int save_cal_para_to_nv(struct sip5005_calib_ctx_t *p_calib)
{
	int ret;

	if (!p_calib) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: write calib data with size %d\n", __func__,
			   sizeof(struct sip5005_calib_ctx_t));
	ret = write_color_calibrate_data_to_nv(RGBAP_CALI_DATA_NV_NUM,
										   sizeof(struct sip5005_calib_ctx_t),
										   "RGBAP", (char *)p_calib);
	if (ret < 0)
		hwlog_err("%s: fail, error code\n", __func__);
	return ret;
}

static int sip5005_cal_als(struct color_chip *chip)
{
	struct sip5005_ctx *ctx = NULL;
	UINT32 raw_data[SI_REPORT_DATA_LEN];
	UINT32 curr_gain;
	int i;

	hwlog_info("%s, comes into cali step\n", __func__);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	ctx = chip->device_ctx;
	curr_gain = (ctx->alg_ctx.gain / SIP5005_GAIN_SCALE);

	sip5005_get_als_data(chip->device_ctx, raw_data, SI_REPORT_DATA_LEN);
	sip5005_log_buffer("cal_target", ctx->cal_ctx.target, SI_REPORT_DATA_LEN);
	sip5005_log_buffer("cal_raw", raw_data, SI_REPORT_DATA_LEN);

	hwlog_info("%s: state = %d, count = %u, curr_gain = %u\n", __func__,
			   chip->cali_ctx.cal_state, chip->cali_ctx.cal_raw_count, curr_gain);

	for (i = 0; i < SI_REPORT_DATA_LEN; i++)
		if (raw_data[i] == 0) {
			/* can"t devide by zero, use the default scaling factor */
			ctx->cal_ctx.ratio[i] = SIP5005_CAL_RATIO;
			color_calibrate_result = false;
			hwlog_err("%s, raw[%d] == 0, can not devide by zero\n", __func__, i);
		} else {
			ctx->cal_ctx.ratio[i] = ctx->cal_ctx.target[i] *
									(curr_gain * SI_SIP5005_FLOAT_TO_FIX /
									 SI_SIP5005_GAIN_OF_GOLDEN) / raw_data[i];
		}

	sip5005_log_buffer("cal_ratio", ctx->cal_ctx.ratio, SI_REPORT_DATA_LEN);
	hwlog_info("cal_result : %d", color_calibrate_result);

	if (color_calibrate_result)
		save_cal_para_to_nv(&(ctx->cal_ctx));

	report_calibrate_result = true;
	chip->in_cal_mode = false;

	if (sensor_type == SI_SIP5005_REV0) {
		if (enable_status_before_cali != 1)
			sip5005_als_set_enable(chip, SIDRIVER_ALS_DISABLE);
		else
			hwlog_info("color sensor enabled before calibrate\n");
	}
	hwlog_info("%s done\n", __func__);

	return 0;
}

static void sip5005_report_als(struct color_chip *chip)
{
	UINT32 raw_data[SI_REPORT_DATA_LEN];
	UINT32 curr_gain;
	UINT8 i;
	struct sip5005_ctx *ctx = NULL;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	sip5005_get_als_data(chip->device_ctx, raw_data, SI_REPORT_DATA_LEN);
	curr_gain = ctx->alg_ctx.gain / SIP5005_GAIN_SCALE;
	if (curr_gain == 0)
		return;

	/* adjust the report data when the calibrate ratio is acceptable */
	/* kun remove some code and for SIP5005 donot need to mul the cal_ctx.ratio */
	if (sensor_type == SI_SIP5005_REV0) {
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			raw_data[i] = raw_data[i] * ctx->cal_ctx.ratio[i] / SI_SIP5005_FLOAT_TO_FIX;
	}

	/* send data without length information */
	if (skip_first_rgb_data) {
		skip_first_rgb_data = false;
	} else {
		color_report_val[0] = SI_REPORT_DATA_LEN;
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			color_report_val[i + 1] = raw_data[i] * SI_SIP5005_GAIN_OF_GOLDEN / curr_gain;
		sip5005_report_data(color_report_val + 1);
	}
	report_logcount++;
	if ((report_logcount > SI_REPORT_DATA_LEN) && (report_logcount < SI_REPORT_LOG_COUNT_NUM))
		return;

	if (report_logcount == SI_REPORT_LOG_COUNT_NUM)
		report_logcount = MAX_LOG_COUNT;

	hwlog_info("color_report_val: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, curr gain: %u\n",
				color_report_val[1], color_report_val[2], color_report_val[3],
				color_report_val[4], color_report_val[5], color_report_val[6],
				color_report_val[7], color_report_val[8], color_report_val[9],
				color_report_val[10], curr_gain);
}

int sip5005_enable_rgb(bool enable)
{
	struct color_chip *chip = p_chip;

	hwlog_info("%s: enter\n", __func__);
	if (!chip) {
		hwlog_err("%s: chip Pointer is NULL\n", __func__);
		return 0;
	}

	if (enable)
		sip5005_als_set_enable(chip, SIDRIVER_ALS_ENABLE);
	else
		sip5005_als_set_enable(chip, SIDRIVER_ALS_DISABLE);

	return SET_ENABLE_COMPLETE;
}
EXPORT_SYMBOL_GPL(sip5005_enable_rgb);

void sip5005_show_calibrate(struct color_chip *chip, struct at_color_sensor_output_t *out)
{
	UINT32 i;
	UINT32 j;
	struct sip5005_ctx *ctx = NULL;

	if (!out || !chip || !chip->device_ctx) {
		hwlog_err("%s input para NULL\n", __func__);
		return;
	}

	ctx = chip->device_ctx;
	if (chip->in_cal_mode == false)
		hwlog_err("%s not in cali mode\n", __func__);

	out->result = (UINT32)report_calibrate_result;
	hwlog_info("%s result = %u\n", __func__, out->result);

	out->gain_arr = CAL_STATE_GAIN_LAST;
	out->color_arr = SI_REPORT_DATA_LEN;
	memcpy_s(out->cali_gain, sizeof(out->cali_gain), sip5005_als_gain_lvl, sizeof(out->cali_gain));

	for (i = 0; i < CAL_STATE_GAIN_LAST; i++) {
		for (j = 0; j < MAX_RAW_DATA_LEN; j++) {
			if (j < SI_REPORT_DATA_LEN)
				out->cali_rst[j][i] = ctx->cal_ctx.ratio[j];
			else
				out->cali_rst[j][i] = 0;
		}

		hwlog_info("%s i = %d: cali= %u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", __func__,
			sip5005_als_gain_lvl[i], out->cali_rst[0][i], out->cali_rst[1][i],
			out->cali_rst[2][i], out->cali_rst[3][i], out->cali_rst[4][i],
			out->cali_rst[5][i], out->cali_rst[6][i], out->cali_rst[7][i],
			out->cali_rst[8][i], out->cali_rst[9][i]);
	}
}

void sip5005_store_calibrate(struct color_chip *chip, struct at_color_sensor_input_t *in)
{
	struct sip5005_ctx *ctx = NULL;
	UINT8 rgb_enable_status;

	if (!chip || !chip->device_ctx || !in) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	report_calibrate_result = false;
	hwlog_info("%s get tar_FY_FZ_XL_F6_CLR = %u, %u, %u, %u, %u\n", __func__,
			   in->reserverd[0], in->reserverd[1], in->reserverd[2],
			   in->reserverd[8], in->reserverd[9]);

	if (in->enable && chip->in_cal_mode) {
		hwlog_info("%s already in cali mode\n", __func__);
		return;
	}

	if (in->enable) {
		ctx = chip->device_ctx;
		hwlog_info("%s start calibration mode\n", __func__);
		chip->cali_ctx.cal_raw_count = 0;

		/* copy target value */
		memcpy_s(ctx->cal_ctx.target, SI_REPORT_DATA_LEN * sizeof(UINT32),
				in->reserverd, SI_REPORT_DATA_LEN * sizeof(UINT32));

		/* disable running measurement and safe state */
		rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
		hwlog_info("%s rgb_enable_status = %u\n", __func__, rgb_enable_status);
		if (ctx->alg_ctx.enabled) {
			/* enabled before calibrate */
			enable_status_before_cali = 1;
			hwlog_info("%s: enabled before calibrate\n", __func__);
			sip5005_als_set_enable(chip, SIDRIVER_ALS_DISABLE);
			mdelay(SIP5005_DISABLE_TIME);	 /* sleep 10 ms to make sure disable timer */
		} else {
			/* disabled before calibrate */
			enable_status_before_cali = 0;
			hwlog_info("%s: disabled before calibrate\n", __func__);
		}
		chip->in_cal_mode = true;
		/* make the calibrate_result true, it will be reset on any error */
		color_calibrate_result = true;

		/* start calibration process */
		sip5005_als_set_enable(chip, SIDRIVER_ALS_ENABLE);
	} else {
		hwlog_info("%s stop calibration mode\n", __func__);
		chip->in_cal_mode = false;
	}
}

void sip5005_show_enable(struct color_chip *chip, int *state)
{
	UINT8 rgb_enable_status;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	rgb_enable_status = get_rgb_fd_enable_status(chip->client);
	if (rgb_enable_status & AL_EN_ALL)
		*state = 1;
	else
		*state = 0;
}

void sip5005_store_enable(struct color_chip *chip, int state)
{
	hwlog_info("%s: enter\n", __func__);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	skip_first_rgb_data = true;
	if (state)
		sip5005_als_set_enable(chip, SIDRIVER_ALS_ENABLE);
	else
		sip5005_als_set_enable(chip, SIDRIVER_ALS_DISABLE);
}

static void fd_enable_set(struct color_chip *chip, UINT8 en)
{
	struct sip5005_ctx *ctx = NULL;
	UINT8 fd_enable_status;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	ctx = (struct sip5005_ctx *)chip->device_ctx;

	hwlog_info("%s = %u\n", __func__, en);
	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);

	if (en) {
		if ((fd_enable_status & FD_EN) != FD_EN) {
			/* set default gain */
			sip5005_set_fd_gain(ctx, DEFAULT_FD_GAIN);

			/* enable flicker */
			sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, FD_EN, FD_EN);

			ctx->flc_ctx.first_fd_inte = true;
			/* first enable flicker timer */
			mod_timer(&chip->fd_timer, jiffies + msecs_to_jiffies(FIRST_FLK_TIMER));
			fd_timer_stopped = false;
			hwlog_info("fd_enable 6ms for a gain quickly\n");
		} else {
			hwlog_info("fd_enable fd already been enabled\n");
		}
	} else {
		if ((fd_enable_status & FD_EN) == FD_EN) {
			/* disable flicker */
			sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, 0, FD_EN);

			fd_timer_stopped = true;
			/* clear ring_buffer when close the fd */
			memset_s(ring_buffer, MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT, 0,
					MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT);
			buffer_ptr = &ring_buffer[0];
			head_ptr = &ring_buffer[0];
			tail_ptr = &ring_buffer[0];
			hwlog_info("fd_enable now disable fd sensor\n");
		} else {
			hwlog_info("fd_enable has already been disabled\n");
		}
	}
}

void sip5005_fd_show_enable(struct color_chip *chip, int *state)
{
	UINT8 fd_enable_status;

	if (!chip || !state) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	fd_enable_status = get_rgb_fd_enable_status(chip->client);
	if ((fd_enable_status & FD_EN) == FD_EN)
		*state = 1;
	else
		*state = 0;
}

void sip5005_fd_store_enable(struct color_chip *chip, int state)
{
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}
	if (state)
		/* check the calibration process, disable fd function */
		fd_enable_set(chip, SIDRIVER_FD_ENABLE);
	else
		fd_enable_set(chip, SIDRIVER_FD_DISABLE);

	hwlog_info("%s enable = %d success\n", __func__, state);
}

static void sip5005_initialize_calibration(struct sip5005_ctx *ctx)
{
	UINT32 i;

	if (get_cal_para_from_nv(&(ctx->cal_ctx)) < 0) {
		hwlog_info("set default calibration data\n");
		/* initialize calibration default values */
		for (i = 0; i < SI_REPORT_DATA_LEN; i++)
			ctx->cal_ctx.ratio[i] = SIP5005_CAL_RATIO;
	} else {
		hwlog_info("successful reading of calibration data, CALIB %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
			ctx->cal_ctx.ratio[0], ctx->cal_ctx.ratio[1],
			ctx->cal_ctx.ratio[2], ctx->cal_ctx.ratio[3],
			ctx->cal_ctx.ratio[4], ctx->cal_ctx.ratio[5],
			ctx->cal_ctx.ratio[6], ctx->cal_ctx.ratio[7],
			ctx->cal_ctx.ratio[8], ctx->cal_ctx.ratio[9]);
	}
}

static void sip5005_als_work(struct work_struct *work)
{
	bool re_enable = false;
	UINT8 rgb_data_status;
	UINT8 rgb_enable_status;
	struct sip5005_ctx *ctx = NULL;
	struct color_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	chip = container_of(work, struct color_chip, als_work);
	if (!chip || !chip->device_ctx) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}
	ctx = (struct sip5005_ctx *)chip->device_ctx;

	/* read persistent data here, because it is not available during probe */
	if (ctx->alg_ctx.load_calibration) {
		sip5005_initialize_calibration(ctx);
		ctx->alg_ctx.load_calibration = false;
	}

	re_enable = sip5005_handle_als_event(ctx, chip->in_cal_mode);

	rgb_data_status = sip5005_get_als_update(ctx);
	if ((rgb_data_status & SI_SIP5005_FEATURE_ALS) && !re_enable) {
		if (chip->in_cal_mode == false)
			sip5005_report_als(chip);
		else
			sip5005_cal_als(chip);
	}

	rgb_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if ((rgb_enable_status & AL_EN_ALL) != AL_EN_ALL) {
		hwlog_info("%s: rgb already disabled, del timer\n", __func__);
		if (ctx->alg_ctx.enabled) {
			sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, AL_EN_ALL, AL_EN_ALL);
			hwlog_info("%s: alg_ctx enabled: status=%d\n", __func__, ctx->alg_ctx.enabled);
		} else {
			return;
		}
	}

	mod_timer(&chip->work_timer, jiffies + msecs_to_jiffies(ctx->alg_ctx.itime_ms + HIGH_TIMER_OFFSET));
}

void sip5005_get_read_out_buffer(void)
{
	UINT32 i;
	uint32_t head_ptr_delta;
	uint16_t *temp_ptr = &ring_buffer[0];
	uint16_t *temp_head_ptr = head_ptr;

	head_ptr_delta = (uint32_t)((uintptr_t)temp_head_ptr - (uintptr_t)(&ring_buffer[0]));
	head_ptr_delta = head_ptr_delta / ASTEP_LEN;

	if (head_ptr_delta > MAX_BUFFER_SIZE - 1)
		head_ptr_delta = MAX_BUFFER_SIZE - 1;

	memset_s(read_out_buffer, sizeof(read_out_buffer), 0, sizeof(read_out_buffer));

	for (i = 0; i < (MAX_BUFFER_SIZE - head_ptr_delta); i++) {
		read_out_buffer[i] = *temp_head_ptr;
		temp_head_ptr++;
	}

	for (i = (MAX_BUFFER_SIZE - head_ptr_delta); i < MAX_BUFFER_SIZE; i++) {
		read_out_buffer[i] = *temp_ptr;
		temp_ptr++;
	}
}

void sip5005_ring_buffer_process(uint8_t fifo_lvl, uint16_t *buf_16)
{
	UINT32 i;

	if (!buf_16) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&ring_buffer_mutex);
	for (i = 0; i < fifo_lvl; i++) {
		*buffer_ptr = buf_16[i];
		tail_ptr = buffer_ptr;
		buffer_ptr++;
		if (buffer_ptr == &ring_buffer[MAX_BUFFER_SIZE]) {
			buffer_ptr = &ring_buffer[0];
			first_circle_end = 1;
		}
	}
	if ((tail_ptr == &ring_buffer[MAX_BUFFER_SIZE - 1]) || (first_circle_end == 0))
		head_ptr = &ring_buffer[0];

	if ((first_circle_end == 1) && (tail_ptr != &ring_buffer[MAX_BUFFER_SIZE - 1]))
		head_ptr = (tail_ptr + 1);

	mutex_unlock(&ring_buffer_mutex);
}

static void fd_auto_gain_first(struct sip5005_ctx *ctx, uint16_t buf_16_bit_max)
{
	if (buf_16_bit_max <= BUF_16_MAX_LEVEL5)
		/* if min 9x gain auto lev5 */
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL4)
		/* if min 9x gain auto lev4 */
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL3)
		/* if min 9x gain auto lev3 */
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
	else if (buf_16_bit_max <= BUF_16_MAX_LEVEL2)
		hwlog_info("%s, keep the init 4x gain\n", __func__);
	else
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_1);

	hwlog_info("%s, come into first_fd_inte, gain = %u\n", __func__, ctx->flc_ctx.fd_gain);
}

static bool fd_auto_gain_normal(struct sip5005_ctx *ctx, UINT16 buf_16_bit_max)
{
	bool fd_saturation_check;
	bool fd_insufficience_check;
	bool gain_adjusted = false;

	fd_saturation_check = buf_16_bit_max >= max_flicker;
	fd_insufficience_check = buf_16_bit_max < SIP5005_FD_LOW_LEVEL;

	if (fd_saturation_check)
		hwlog_info("%s, FD sat: %d, max_fd: %u, thresh: %u\n", __func__,
				fd_saturation_check, buf_16_bit_max, max_flicker);

	if (fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2)) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_1);
		gain_adjusted = true;
	} else if ((fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3)) ||
				(fd_insufficience_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_1))) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_2);
		gain_adjusted = true;
	} else if ((fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4)) ||
				(fd_insufficience_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_2))) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_3);
		gain_adjusted = true;
	} else if ((fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_5)) ||
				(fd_insufficience_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_3))) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_4);
		gain_adjusted = true;
	} else if ((fd_saturation_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_6)) ||
				(fd_insufficience_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_4))) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_5);
		gain_adjusted = true;
	} else if (fd_insufficience_check && (ctx->flc_ctx.fd_gain == FD_GAIN_LEVEL_5)) {
		sip5005_set_fd_gain(ctx, FD_GAIN_LEVEL_6);
		gain_adjusted = true;
	}
	return gain_adjusted;
}

static bool fd_auto_gain_control(struct sip5005_ctx *ctx, uint16_t *buf_16_bit, int len)
{
	UINT8 i;
	UINT16 buf_16_bit_max;

	if (!ctx || !buf_16_bit) {
		hwlog_err("%s, pointer is null\n", __func__);
		return false;
	}

	if (len == 0)
		return false;

	if (len > MAX_AUTOGAIN_CHECK)
		len = MAX_AUTOGAIN_CHECK;

	fd_gain_adjusted = false;
	buf_16_bit_max = buf_16_bit[0];
	for (i = 1; i < len; i++)
		if (buf_16_bit_max < buf_16_bit[i])
			buf_16_bit_max = buf_16_bit[i];

	/* first flicker detect data is special */
	if (ctx->flc_ctx.first_fd_inte) {
		fd_auto_gain_first(ctx, buf_16_bit_max);
		ctx->flc_ctx.first_fd_inte = false;
		fd_gain_adjusted = true;
	} else {
		if (fd_auto_gain_normal(ctx, buf_16_bit_max))
			fd_gain_adjusted = true;
	}
	return fd_gain_adjusted;
}

static bool check_flicker_status(struct sip5005_ctx *ctx)
{
	UINT8 fd_status1 = 0;
	bool error_detected = false;

	color_sensor_get_byte(ctx->handle, SIP5005_FIFO_STATUS1, &fd_status1);
	if (fd_status1 & FIFO_FULL) {
		error_detected = true;
		hwlog_err("ERROR: Flicker FIFO overflow detected");
	}
	return error_detected;
}

static void pull_fd_data_to_buffer(struct sip5005_ctx *ctx, UINT16 *buf, UINT8 num)
{
	UINT8 fd_enable_status;
	UINT8 i;

	fd_enable_status = get_rgb_fd_enable_status(ctx->handle);
	if ((fd_enable_status & FD_EN) == FD_EN) {
		for (i = 0; i < num; i++)  /* normalization */
			buf[i] = (UINT16)(((UINT32)(buf[i]) * SI_SIP5005_FD_GAIN_OF_GOLDEN) / ctx->flc_ctx.fd_gain);
		sip5005_ring_buffer_process(num, buf);
	}
}

static void reset_fd_buffer(struct sip5005_ctx *ctx, bool error_detected)
{
	/* if fd_gain is adjusted */
	/* clear the ringbuffer and reset the related pointer */
	mutex_lock(&ring_buffer_mutex);
	memset_s(ring_buffer, MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT, 0, MAX_BUFFER_SIZE * BUF_RATIO_8_16BIT);
	/* clear ring_buffer when close the fd */
	buffer_ptr = &ring_buffer[0];
	head_ptr = &ring_buffer[0];
	tail_ptr = &ring_buffer[0];
	mutex_unlock(&ring_buffer_mutex);

	if (error_detected) {
		sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, 0, FD_EN);
		sip5005_set_field(ctx->handle, SIP5005_FIFO_CFG2, FIFO_CLEAR, FIFO_CLEAR);
		sip5005_set_field(ctx->handle, SIP5005_AL_CTRL0, FD_EN, FD_EN);
	}
}

static UINT16 check_fifo_len(struct sip5005_ctx *ctx)
{
	UINT8 size_low = {0};
	UINT8 size_high = {0};
	UINT8 size_value[2] = {0};
	UINT16 fifo_level = 0;

	color_sensor_read_fifo(ctx->handle, SIP5005_FIFO_STATUS0, &size_value[0], 2);
	size_low = size_value[0];
	size_high = size_value[1] & FIFO_LVL_H_MASK;
	fifo_level = (size_high << 8) | size_low;
	return fifo_level;
}

static void read_fd_data(struct color_chip *chip)
{
	UINT16 buf_16_bit[REPORT_FIFO_LEN] = {0};
	UINT8 buf_8_bit[REPORT_FIFO_LEN * 2] = {0};
	UINT16 fifo_level = 0;
	UINT16 i = 0;
	struct sip5005_ctx *ctx = NULL;
	bool auto_gain = false;
	bool error_detected = false;

	if (!chip || !chip->device_ctx) {
		hwlog_err("%s, pointer is null\n", __func__);
		return;
	}
	ctx = (struct sip5005_ctx *)chip->device_ctx;
	mutex_lock(&ctx->flc_ctx.lock);

	error_detected = check_flicker_status(ctx);
	fifo_level = check_fifo_len(ctx);
	if (fifo_level >= MAX_FIFO_LEVEL)
		error_detected = true;

	if (fd_timer_stopped == false)
		mod_timer(&chip->fd_timer, jiffies + msecs_to_jiffies(FD_POLLING_TIME));
	else
		hwlog_info("fd has been disabled, del fd work timer\n");

	/* cal fd_ratio */
	if (fifo_level > 0 && !error_detected) {
		color_sensor_read_fifo(ctx->handle, SIP5005_FD_DATA, buf_8_bit, fifo_level);
		for (i = 0; i < fifo_level; i += 2)
			buf_16_bit[i / 2] = (UINT16)(buf_8_bit[i]) | ((UINT16)(buf_8_bit[i + 1]) << 8);

		auto_gain = fd_auto_gain_control(ctx, buf_16_bit, fifo_level / 2);
		if (auto_gain == false)
			pull_fd_data_to_buffer(ctx, buf_16_bit, fifo_level / 2);
	} else {
		hwlog_info("%s: FIFO_LVL: %u, error: %d\n", __func__, fifo_level, error_detected);
	}

	if (auto_gain == true || error_detected) {
		hwlog_info("%s: FD reset, auto_gain: %d, error: %d\n", __func__, auto_gain, error_detected);
		reset_fd_buffer(ctx, error_detected);
	}
	mutex_unlock(&ctx->flc_ctx.lock);
}

static void sip5005_fd_work(struct work_struct *work)
{
	struct color_chip *chip = NULL;

	if (!work) {
		hwlog_err("%s: Pointer work is NULL\n", __func__);
		return;
	}
	chip = container_of(work, struct color_chip, fd_work);
	if (!chip) {
		hwlog_err("%s: Pointer chip is NULL\n", __func__);
		return;
	}
	mutex_lock(&chip->lock);
	read_fd_data(chip);
	mutex_unlock(&chip->lock);
}

static void sip5005_get_fd_data(struct color_chip *chip, char *si_fd_data)
{
	if (!chip || !si_fd_data) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return;
	}

	mutex_lock(&ring_buffer_mutex);
	sip5005_get_read_out_buffer();
	memcpy_s(si_fd_data, sizeof(read_out_buffer), read_out_buffer, sizeof(read_out_buffer));
	mutex_unlock(&ring_buffer_mutex);
}

static char *sip5005_chip_name(void)
{
	return "si_sip5005";
}

static char *sip5005_algo_type(void)
{
	if (sensor_type == SI_SIP5005_REV0)
		return "sip5005_nor";
	else
		return "unsupport";
}

static char *sip5005_get_oeminfo_data(void)
{
	if (get_oeminfo_data_support == 1) // dts get_oeminfo_data_support = <1>
		return "get_oeminfo_data_support";
	else
		return "unsupport";
}

int sip5005_set_external_gain(void *ctx, int gain)
{
	if (gain > FULL_BYTE)
		return -EINVAL;

	sip5005_als_set_gain(ctx, (UINT8)gain);
	return 0;
}

static void configure_functions(struct color_chip *chip)
{
	chip->at_color_show_calibrate_state = sip5005_show_calibrate;
	chip->at_color_store_calibrate_state = sip5005_store_calibrate;
	chip->color_enable_show_state = sip5005_show_enable;
	chip->color_enable_store_state = sip5005_store_enable;
	chip->color_sensor_get_gain = sip5005_als_get_gain;
	chip->color_sensor_set_gain = sip5005_set_external_gain;
	chip->get_flicker_data = sip5005_get_fd_data;
	chip->flicker_enable_show_state = sip5005_fd_show_enable;
	chip->flicker_enable_store_state = sip5005_fd_store_enable;
	chip->color_report_type = sip5005_rgb_report_type;
	chip->color_chip_name = sip5005_chip_name;
	chip->color_algo_type = sip5005_algo_type;
	chip->get_oeminfo_data = sip5005_get_oeminfo_data;
	color_default_enable = sip5005_enable_rgb;
}

static void read_options_parameter(struct device *dev)
{
	int rc;

	rc = of_property_read_u32(dev->of_node, "flicker_support", &flicker_support);
	if (rc < 0) {
		hwlog_info("%s, get flicker_support failed\n", __func__);
		flicker_support = 1;  /* default support flk */
	}
	rc = of_property_read_u32(dev->of_node, "algo_support", &algo_support);
	if (rc < 0) {
		hwlog_info("%s, get algo_support failed\n", __func__);
		algo_support = SI_UNSUPPORT_ALGO;  /* default not support algo */
	}
	rc = of_property_read_u32(dev->of_node, "get_oeminfo_data_support",
		&get_oeminfo_data_support);
	if (rc < 0) {
		hwlog_warn("%s, get_oeminfo_data_support failed\n", __func__);
		get_oeminfo_data_support = 0; // not support get oeminfo data
	}

	hwlog_info("%s flicker_support = %u, algo_support = %u, get_oeminfo_data_support = %u\n", __func__,
		flicker_support, algo_support, get_oeminfo_data_support);

	hwlog_info("%s flicker_support = %u, algo_support = %u\n", __func__, flicker_support, algo_support);
}

static void init_library(struct color_chip *chip)
{
	int ret;

	timer_setup(&chip->work_timer, sip5005_als_timer_wrk, 0);
	INIT_WORK(&chip->als_work, sip5005_als_work);

	timer_setup(&chip->fd_timer, sip5005_flc_timer_wrk, 0);
	INIT_WORK(&chip->fd_work, sip5005_fd_work);

	configure_functions(chip);

	p_chip = chip;
	ret = color_register(chip);
	if (ret < 0)
		hwlog_err("%s color_register fail\n", __func__);

	hwlog_info("rgb sensor %s ok\n", __func__);
}

int sip5005_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	int ret = -1;
	struct device *dev = NULL;
	static struct color_chip *chip = NULL;
	struct sip5005_ctx *ctx = NULL;

	if (!client)
		return -EFAULT;

	color_notify_support();	 /* declare support sip5005 */
	dev = &client->dev;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto init_failed;

	chip = kzalloc(sizeof(struct color_chip), GFP_KERNEL);
	if (!chip)
		goto malloc_failed;

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = dev->platform_data;
	i2c_set_clientdata(chip->client, chip);

	chip->in_cal_mode = false;
	chip->cali_ctx.cal_state = 0;

	if (sip5005_get_chipid(chip->client) == SI_UNKNOWN_DEVICE) {
		hwlog_info("%s failed: SI_UNKNOWN_DEVICE\n", __func__);
		goto id_failed;
	}
	cal_normal = false;
	read_options_parameter(dev);

	chip->device_ctx = kzalloc(sizeof(struct sip5005_ctx), GFP_KERNEL);
	if (!chip->device_ctx)
		goto id_failed;

	ctx = chip->device_ctx;
	ctx->handle = chip->client;
	mutex_init(&(ctx->flc_ctx.lock));

	sip5005_reset_regs(ctx);
	sip5005_cal_scale(ctx);
	init_library(chip);
	return 0;

id_failed:
	if (chip->device_ctx)
		kfree(chip->device_ctx);

	i2c_set_clientdata(client, NULL);
	hwlog_err("%s id_failed\n", __func__);
malloc_failed:
	if (chip) kfree(chip);
init_failed:
	hwlog_err("%s Probe failed\n", __func__);
	color_notify_absent();
	return ret;
}

int sip5005_suspend(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s\n", __func__);
	return 0;
}

int sip5005_resume(struct device *dev)
{
	struct color_chip *chip = NULL;

	if (!dev) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -EFAULT;
	}
	hwlog_info("%s\n", __func__);
	return 0;
}

int si_sip5005_remove(struct i2c_client *client)
{
	struct color_chip *chip = NULL;

	if (!client) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}
	chip = i2c_get_clientdata(client);
	if (!chip) {
		hwlog_err("%s: Pointer is NULL\n", __func__);
		return -1;
	}

	free_irq(client->irq, chip);
	i2c_set_clientdata(client, NULL);
	kfree(chip->device_ctx);
	kfree(chip);
	return 0;
}

static struct i2c_device_id sidriver_idtable[] = {{"si_sip5005", 0}, {}};
MODULE_DEVICE_TABLE(i2c, sidriver_idtable);

static const struct dev_pm_ops si_sip5005_pm_ops = {
	.suspend = sip5005_suspend,
	.resume = sip5005_resume,
};

static const struct of_device_id sidriver_of_id_table[] = {
	{.compatible = "si,sip5005"},
	{},
};

static struct i2c_driver si_sip5005_driver = {
	.driver =
		{
			.name = "si_sip5005",
			.owner = THIS_MODULE,
			.of_match_table = sidriver_of_id_table,
		},
	.id_table = sidriver_idtable,
	.probe = sip5005_probe,
	.remove = si_sip5005_remove,
};

static int __init sip5005_init(void) {
	int rc;

	hwlog_info("%s\n", __func__);

	rc = i2c_add_driver(&si_sip5005_driver);

	hwlog_info("%s %d\n", __func__, rc);
	return rc;
}

static void __exit sip5005_exit(void) {
	hwlog_info("%s\n", __func__);
	i2c_del_driver(&si_sip5005_driver);
}

module_init(sip5005_init);
module_exit(sip5005_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("color_sensor driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
