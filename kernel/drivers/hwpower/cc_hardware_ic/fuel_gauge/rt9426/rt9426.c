// SPDX-License-Identifier: GPL-2.0
/*
 * rt9426.c
 *
 * driver for rt9426 battery fuel gauge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "rt9426.h"
#include <chipset_common/hwpower/common_module/power_supply.h>
#ifdef CONFIG_COUL_DRV
#include "platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h"
#endif /* CONFIG_COUL_DRV */
#include <chipset_common/hwpower/battery/battery_adc_monitor.h>

#define HWLOG_TAG rt9426
HWLOG_REGIST();

static int g_rt9426_t_v_table[][2] = {
	{ -400, 1148 }, { -350, 1132 }, { -300, 1111 }, { -250, 1086 },
	{ -200, 1056 }, { -150, 1021 }, { -100, 981 }, { -50, 935 },
	{ 0, 886 }, { 50, 832 }, { 100, 775 }, { 150, 717 },
	{ 200, 658 }, { 250, 600 }, { 300, 544 }, { 350, 490 },
	{ 400, 440 }, { 450, 393 }, { 500, 350 }, { 550, 311 },
	{ 600, 277 }, { 650, 245 }, { 700, 218 }, { 750, 193 },
	{ 800, 171 }, { 850, 152 }, { 900, 135 }, { 0, 0 },
};

#ifdef CONFIG_COUL_DRV
static struct rt9426_chip *g_rt9426_chip[RT9426_IC_TYPE_MAX];

struct rt9426_chip *rt9426_get_chip(int idx)
{
	if ((idx < 0) || (idx >= RT9426_IC_TYPE_MAX))
		return NULL;

	return g_rt9426_chip[idx];
}
#endif /* CONFIG_COUL_DRV */

static int rt9426_read_device_onetime(void *client, u32 reg, int len, void *dst)
{
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;

	if (len > 1) {
		ret = i2c_smbus_read_i2c_block_data(i2c, reg, len, dst);
	} else {
		ret = i2c_smbus_read_byte_data(i2c, reg);
		if (ret < 0)
			return ret;
		*(u8 *)dst = (u8)ret;
	}
	return ret;
}

static int rt9426_write_device_onetime(void *client, u32 reg, int len, const void *src)
{
	const u8 *data = NULL;
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;

	if (len > 1) {
		ret = i2c_smbus_write_i2c_block_data(i2c, reg, len, src);
	} else {
		data = src;
		ret = i2c_smbus_write_byte_data(i2c, reg, *data);
	}
	return ret;
}

static int rt9426_read_device(void *client, u32 reg, int len, void *dst)
{
	int ret;
	int retry_cnt = 5; /* 5: retry times */
	struct rt9426_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->retry_lock);

	do {
		ret = rt9426_read_device_onetime(client, reg, len, dst);
		if (ret >= 0)
			goto out;
		mdelay(50); /* 50: delay 50ms */
	} while (retry_cnt-- > 0);

out:
	mutex_unlock(&chip->retry_lock);
	return ret;
}

static int rt9426_write_device(void *client, u32 reg, int len, const void *src)
{
	int ret;
	int retry_cnt = 5; /* 5: retry times */
	struct rt9426_chip *chip = i2c_get_clientdata(client);

	mutex_lock(&chip->retry_lock);

	do {
		ret = rt9426_write_device_onetime(client, reg, len, src);
		if (ret >= 0)
			goto out;
		mdelay(50); /* 50: delay 50ms */
	} while (retry_cnt-- > 0);

out:
	mutex_unlock(&chip->retry_lock);
	return ret;
}

static int rt9426_block_read(struct i2c_client *i2c, u8 reg, int len, void *dst)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);
	int ret;

	ret = rt9426_read_device(i2c, reg, len, dst);
	if (ret < 0)
		dev_notice(chip->dev, "rt9426 block read 0x%02x fail\n", reg);
	return ret;
}

static int rt9426_block_write(struct i2c_client *i2c,
	u8 reg, int len, const void *src)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);
	int ret;

	ret = rt9426_write_device(i2c, reg, len, src);
	if (ret < 0)
		dev_notice(chip->dev, "rt9426 block write 0x%02x fail\n", reg);
	return ret;
}

static int rt9426_reg_read_word(struct i2c_client *i2c, u8 reg)
{
	u16 data = 0;
	int ret;

	ret = rt9426_block_read(i2c, reg, 2, &data);
	return (ret < 0) ? ret : (s32)le16_to_cpu(data);
}

static int rt9426_reg_write_word(struct i2c_client *i2c, u8 reg, u16 data)
{
	data = cpu_to_le16(data);
	return rt9426_block_write(i2c, reg, 2, (uint8_t *)&data);
}

static int __maybe_unused rt9426_reg_write_word_with_check(struct rt9426_chip *chip,
	u8 reg, u16 data)
{
	int retry_times = 2; /* sometimes need retry */
	int r_data = 0;

	while (retry_times) {
		rt9426_reg_write_word(chip->i2c, reg, data);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
		mdelay(5); /* delay 5ms */
		r_data = rt9426_reg_read_word(chip->i2c, reg);
		if (data == r_data) {
			dev_info(chip->dev,
				"TWrite REG_0x%.2x Successful\n", reg);
			break;
		}
		retry_times--;
		if (retry_times == 0)
			dev_notice(chip->dev, "Write REG_0x%.2x fail\n", reg);
	}
	return r_data;
}

static int rt9426_regmap_init(struct rt9426_chip *chip)
{
	return 0;
}

static void rt9426_read_page_cmd(struct rt9426_chip *chip, uint8_t page)
{
	uint16_t read_page_cmd = RT9426_READ_PAGE_CMD;

	read_page_cmd += page;
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, read_page_cmd);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, read_page_cmd);
	mdelay(5); /* delay 5ms */
}

static void rt9426_write_page_cmd(struct rt9426_chip *chip, uint8_t page)
{
	uint16_t write_page_cmd = RT9426_WRITE_PAGE_CMD;

	write_page_cmd += page;
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, write_page_cmd);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, write_page_cmd);
	mdelay(5); /* delay 5ms */
}

static int rt9426_unseal_wi_retry(struct rt9426_chip *chip)
{
	int i, regval, retry_times;

	retry_times = 3; /* sometimes need retry */
	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
		if (regval < 0)
			return -EPERM;
		if ((regval & RT9426_UNSEAL_MASK) == RT9426_UNSEAL_STATUS) {
			dev_info(chip->dev, "RT9426 Unseal Pass\n");
			return RT9426_UNSEAL_PASS;
		}
		dev_info(chip->dev, "RT9426 Unseal Fail Cnt = %d\n", i + 1);
		if (i >= 2) { /* 2: retry time */
			dev_info(chip->dev, "RT9426 Unseal Fail after 3 retries\n");
			return RT9426_UNSEAL_FAIL;
		} else if (i > 0) {
			dev_info(chip->dev, "delay 1000ms before next Unseal retry\n");
			mdelay(1000); /* delay 1s */
		}

		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
			(RT9426_UNSEAL_KEY & POWER_MASK_WORD)); /* lower 2 bytes */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
			(RT9426_UNSEAL_KEY >> 16)); /* higher 2 bytes */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		usleep_range(5000, 5100); /* delay 5ms */
	}
	return RT9426_UNSEAL_FAIL;
}

static void rt9426_hibernate_duty_set(struct rt9426_chip *chip, uint16_t data)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
		if (regval < 0)
			return;
		regval = ((regval & 0xfff8) | (data & 0x0007)); /* mask bit [2-0] and set */
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7, regval);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
		mdelay(10); /* delay 10ms */
	}
}

static void rt9426_hibernate_duty_read(struct rt9426_chip *chip)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
		if (regval < 0)
			return;
		regval = (regval & 0x0007); /* set bit[2-0] */
		dev_info(chip->dev, "HIBERNATE_DUTTY = 2^%d sec)\n", regval);
	}
}

static void rt9426_enter_hibernate(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_ENTR_SLP_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_exit_hibernate(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXIT_SLP_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_temperature_set(struct rt9426_chip *chip, int data)
{
	dev_info(chip->dev, "%s: temp = %d oC\n", __func__, data);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_TEMP, ((data * 10) + 2732)); /* to K temp */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_reset(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x0041); /* reset regs */
}

static int rt9426_get_volt(struct rt9426_chip *chip)
{
	if (chip->pdata->volt_source)
		chip->bvolt = rt9426_reg_read_word(chip->i2c, chip->pdata->volt_source);
	return chip->bvolt;
}

static inline bool rt9426_is_reg_temp_abnormal(int reg_temp)
{
	return reg_temp == -107; /* -107: cpld error return value, 'abnormal' refer specifically to it */
}

static inline void rt9426_set_reg_temp_cache(struct rt9426_chip *chip, int reg_temp)
{
	chip->cached_reg_data.temp = reg_temp;
}

static inline int rt9426_get_reg_temp_cache(struct rt9426_chip *chip)
{
	hwlog_info("role = %u, get cached reg temp:%d", chip->pdata->ic_role, chip->cached_reg_data.temp);
	return chip->cached_reg_data.temp;
}

static int rt9426_get_temp(struct rt9426_chip *chip)
{
	int reg_temp;

	if (chip->pdata->temp_source) {
		reg_temp = rt9426_reg_read_word(chip->i2c, chip->pdata->temp_source);
		if (chip->pdata->cache_reg_temp_en) {
			if (rt9426_is_reg_temp_abnormal(reg_temp))
				reg_temp = rt9426_get_reg_temp_cache(chip);
			else
				rt9426_set_reg_temp_cache(chip, reg_temp);
		}
		chip->btemp = reg_temp - 2732; /* to K temp */
	}

	if (chip->pdata->ground_loop_comp_en) {
		chip->pdata->glc_data.ibat = rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
		chip->pdata->glc_data.vadc = power_lookup_table_linear_trans_dichotomy(g_rt9426_t_v_table,
			RT9426_T_V_ARRAY_LEN, chip->btemp, 0);
		dev_info(chip->dev, "ibat%d, vadc%d, vpullup%d, rpullup%d, rcomp%d, temp%d\n",
			chip->pdata->glc_data.ibat, chip->pdata->glc_data.vadc, chip->pdata->glc_data.vpullup,
			chip->pdata->glc_data.rpullup, chip->pdata->glc_data.rcomp, chip->btemp);
		chip->btemp = ground_loop_compensate_get_temp(&chip->pdata->glc_data);
	}

	return chip->btemp;
}

static unsigned int rt9426_get_cyccnt(struct rt9426_chip *chip)
{
	int ret;
	unsigned int cyccnt = 0;

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_CYC);
	if (ret < 0)
		dev_notice(chip->dev, "%s: read cycle count fail\n", __func__);
	else
		cyccnt = ret;
	return cyccnt;
}

static unsigned int rt9426_get_avg_current(struct rt9426_chip *chip)
{
	chip->avg_curr = rt9426_reg_read_word(chip->i2c, RT9426_REG_AI);
	if (chip->avg_curr < 0)
		return -EIO;
	if (chip->avg_curr > 0x7FFF) { /* handle negative situation */
		chip->avg_curr = 0x10000 - chip->avg_curr;
		chip->avg_curr = 0 - chip->avg_curr;
	}

	return chip->avg_curr;
}

static unsigned int rt9426_get_current(struct rt9426_chip *chip)
{
	chip->bcurr = rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR);
	if (chip->bcurr < 0)
		return -EIO;
	if (chip->bcurr > 0x7FFF) { /* handle negative situation */
		chip->bcurr = 0x10000 - chip->bcurr;
		chip->bcurr = 0 - chip->bcurr;
	}

	return chip->bcurr;
}

static unsigned int rt9426_get_rm(struct rt9426_chip *chip)
{
	chip->rm = rt9426_reg_read_word(chip->i2c, RT9426_REG_RM);

	return chip->rm;
}

static int rt9426_get_offset(struct rt9426_chip *chip, int soc, int temp);

static int rt9426_get_soc(struct rt9426_chip *chip)
{
	int regval;
	int capacity;
	int btemp;

	regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_SOC);
	if (regval < 0) {
		dev_notice(chip->dev, "read soc value fail\n");
		return -EIO;
	}
	capacity = (regval * 10); /* 10 precision */
	dev_dbg(chip->dev, "capacity before offset = %d\n", capacity);
	btemp = rt9426_get_temp(chip);
	dev_dbg(chip->dev, "TEMP = %d\n", btemp);
	chip->soc_offset = rt9426_get_offset(chip, capacity, btemp);
	dev_dbg(chip->dev, "SOC_OFFSET = %d\n", chip->soc_offset);
	capacity += chip->soc_offset;
	dev_dbg(chip->dev, "capacity after offset = %d\n", capacity);
	if (capacity > 0)
		capacity = DIV_ROUND_UP(capacity, 10); /* 10 precision */
	else
		capacity = 0;
	if (capacity > 100) /* full capacity */
		capacity = 100;
	dev_dbg(chip->dev, "SYS_SOC = %d\n", capacity);
	return capacity;
}

static void set_low_volt_smooth(struct rt9426_chip *chip)
{
	int regval;

	regval = rt9426_unseal_wi_retry(chip);
	if (regval == RT9426_UNSEAL_FAIL) {
		dev_info(chip->dev,
			"%s: Unseal failed. Unable to do lv smooth\n", __func__);
	} else {
		mutex_lock(&chip->update_lock);
		rt9426_write_page_cmd(chip, RT9426_PAGE_5);
		/* low voltage smooth, enable: 0x00FF, disable: 0x0032 */
		if (chip->low_v_smooth_en)
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, 0x00FF);
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, 0x0032);
		mutex_unlock(&chip->update_lock);
	}
}

static void check_for_lv_smooth(struct rt9426_chip *chip)
{
	int vbat = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
	int temp = rt9426_get_temp(chip);
	int soc = rt9426_get_soc(chip);
	int curr = rt9426_get_current(chip);

	if ((temp >= LV_SMOOTH_T_MIN) && (temp <= LV_SMOOTH_T_MAX)) {
		if (curr <= LV_SMOOTH_I_TH) {
			/* dsg current <= -1000mA, disable lv_smooth */
			if (chip->low_v_smooth_en) {
				chip->low_v_smooth_en = false;
				set_low_volt_smooth(chip);
			}
		} else {
			/* dsg current > -1000mA, keep checking vbat & soc condition */
			if ((vbat <= LV_SMOOTH_V_TH) && (soc > LV_SMOOTH_S_TH)) {
				/* vbat & soc condition meet, enable lv_smooth */
				if (!chip->low_v_smooth_en) {
					chip->low_v_smooth_en = true;
					set_low_volt_smooth(chip);
				}
			} else {
				/* vbat & soc condition don't meet, disable lv_smooth */
				if (chip->low_v_smooth_en) {
					chip->low_v_smooth_en = false;
					set_low_volt_smooth(chip);
				}
			}
		}
	} else {
		/* temperature condition don't meet, disable lv_smooth */
		if (chip->low_v_smooth_en) {
			chip->low_v_smooth_en = false;
			set_low_volt_smooth(chip);
		}
	}
}

static void check_for_lowtemp_edv(struct rt9426_chip *chip)
{
	int regval;
	int volt_thr = 50; /* 50: volt threshold value */
	int vbat = rt9426_reg_read_word(chip->i2c, RT9426_REG_AV);
	int temp = rt9426_get_temp(chip);

	if (!chip->pdata->lowtemp_en)
		return;

	if ((chip->lowtemp_edv == 1) && (temp < chip->pdata->lowtemp_t_thr) &&
		(vbat < RT9426_LOWTEMP_V_THR)) {
		regval = rt9426_unseal_wi_retry(chip);
		if (regval == RT9426_UNSEAL_FAIL) {
			dev_info(chip->dev,
				"%s: Unseal failed. Unable to do lowtemp EDV\n", __func__);
		} else {
			mutex_lock(&chip->update_lock);
			rt9426_write_page_cmd(chip, RT9426_PAGE_5);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, 0x1200 |
				chip->pdata->lowtemp_edv_thr); /* fixed value, do not change it */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
			chip->lowtemp_edv = 2; /* 2: do lowtemp EDV */
			mutex_unlock(&chip->update_lock);
		}
	} else if (((chip->lowtemp_edv == 2) &&
		((temp >= (chip->pdata->lowtemp_t_thr + volt_thr)) ||
		(vbat >= (RT9426_LOWTEMP_V_THR + volt_thr)))) ||
		(chip->lowtemp_edv == 0)) {
		regval = rt9426_unseal_wi_retry(chip);
		if (regval == RT9426_UNSEAL_FAIL) {
			dev_info(chip->dev,
				"%s: Unseal failed. Unable to do lowtemp EDV\n", __func__);
		} else {
			mutex_lock(&chip->update_lock);
			rt9426_read_page_cmd(chip, RT9426_PAGE_5);
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
			if (chip->pdata->fd_vth[chip->ocv_index] != (regval & POWER_MASK_BYTE)) {
				rt9426_write_page_cmd(chip, RT9426_PAGE_5);
				rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, 0x1200 |
					chip->pdata->fd_vth[chip->ocv_index]); /* fixed value, do not change it */
				rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
			}
			chip->lowtemp_edv = 1; /* 1: not do lowtemp EDV */
			mutex_unlock(&chip->update_lock);
		}
	}
}

int rt9426_get_display_data(struct rt9426_chip *di, int index)
{
	int val = 0;

	switch (index) {
	case RT9426_DISPLAY_TEMP:
		val = rt9426_get_temp(di);
		break;
	case RT9426_DISPLAY_VBAT:
		val = rt9426_get_volt(di);
		break;
	case RT9426_DISPLAY_IBAT:
		val = rt9426_get_current(di);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_AVG_IBAT:
		val = rt9426_get_avg_current(di);
			if (di->pdata->rs_schematic)
				val = ((s64)val * di->pdata->rs_ic_setting) /
					di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_RM:
		val = rt9426_get_rm(di);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_SOC:
		val = rt9426_get_soc(di);
		check_for_lv_smooth(di);
		if (!di->pdata->dnm_cfg_edv_disable)
			check_for_lowtemp_edv(di);
		break;
	case RT9426_DISPLAY_DISIGN_FCC:
		val = di->pdata->design_capacity;
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_FCC:
		val = rt9426_reg_read_word(di->i2c, RT9426_REG_FCC);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_TTE:
		val = rt9426_reg_read_word(di->i2c, RT9426_REG_TTE);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_DC:
		val = rt9426_reg_read_word(di->i2c, RT9426_REG_DSNCAP);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	default:
		break;
	}

	return val;
}

static int rt9426_get_log_head(char *buffer, int size, void *dev_data)
{
	struct rt9426_chip *di = dev_data;

	if (!buffer || !di)
		return -EPERM;

	if (di->pdata->ic_role == RT9426_IC_TYPE_MAIN)
		snprintf(buffer, size,
			"Temp   Vbat   Ibat   AIbat   Rm     Soc    Fcc    flag2   flag3   addr_70   ");
	else
		snprintf(buffer, size,
			"Temp1  Vbat1  Ibat1  AIbat1  Rm1    Soc1   Fcc1   flag2_1 flag3_1 addr_70_1 ");

	return 0;
}

static void rt9426_dump_register(struct rt9426_chip *di)
{
	int val;

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_RSVD_FLAG);
	if (val >= 0)
		dev_info(di->dev, "RSVD = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG1);
	if (val >= 0)
		dev_info(di->dev, "flag1 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG2);
	if (val >= 0)
		dev_info(di->dev, "flag2 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG3);
	if (val >= 0)
		dev_info(di->dev, "flag3 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_DSNCAP);
	if (val >= 0)
		dev_info(di->dev, "DC = %d\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FCC);
	if (val >= 0)
		dev_info(di->dev, "FCC = %d\n", val);
}

static int rt9426_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct rt9426_chip *di = dev_data;
	struct rt9426_display_data g_dis_data;
	int flag2, flag3, addr_70;

	if (!buffer || !di)
		return -EPERM;

	g_dis_data.vbat = rt9426_get_display_data(di, RT9426_DISPLAY_VBAT);
	g_dis_data.ibat = rt9426_get_display_data(di, RT9426_DISPLAY_IBAT);
	g_dis_data.avg_ibat = rt9426_get_display_data(di, RT9426_DISPLAY_AVG_IBAT);
	g_dis_data.rm = rt9426_get_display_data(di, RT9426_DISPLAY_RM);
	g_dis_data.temp = rt9426_get_display_data(di, RT9426_DISPLAY_TEMP);
	g_dis_data.soc = rt9426_get_display_data(di, RT9426_DISPLAY_SOC);
	g_dis_data.fcc = rt9426_get_display_data(di, RT9426_DISPLAY_FCC);

	flag2 = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG2);
	flag3 = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG3);
	addr_70 = rt9426_reg_read_word(di->i2c, RT9426_REG_UN_FLT_SOC);

	snprintf(buffer, size, "%-7d%-7d%-7d%-8d%-7d%-7d%-7d%-8x%-8x%-10x",
		g_dis_data.temp, g_dis_data.vbat, g_dis_data.ibat,
		g_dis_data.avg_ibat, g_dis_data.rm, g_dis_data.soc,
		g_dis_data.fcc, flag2, flag3, addr_70);

	return 0;
}

static struct power_log_ops rt9426_fg_ops = {
	.dev_name = "rt9426",
	.dump_log_head = rt9426_get_log_head,
	.dump_log_content = rt9426_dump_log_data,
};

static struct power_log_ops rt9426_aux_fg_ops = {
	.dev_name = "rt9426_aux",
	.dump_log_head = rt9426_get_log_head,
	.dump_log_content = rt9426_dump_log_data,
};

/* add to get both ocv checksum and replace "qs_en" */
static unsigned int rt9426_get_ocv_checksum(struct rt9426_chip *chip)
{
	int regval = rt9426_unseal_wi_retry(chip);

	if (regval == RT9426_UNSEAL_FAIL) {
		chip->ocv_checksum_ic = 0xFFFF;
	} else {
		/* get ocv checksum from ic */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0xCA09);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0xCA09);
		mdelay(5); /* delay 5ms */
		chip->ocv_checksum_ic = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	}
	return chip->ocv_checksum_ic;
}

#ifndef CONFIG_COUL_DRV
static void rt9426_dynamic_change_term_setting_reg_op(struct rt9426_chip *chip,
	struct rt9426_fc_setting *fc_setting)
{
	int regval;

	fc_setting->fc_ith_spd = fc_setting->fc_ith_spd * (int)chip->pdata->rs_schematic;
	fc_setting->fc_ith_spd = DIV_ROUND_UP(fc_setting->fc_ith_spd, (int)chip->pdata->rs_ic_setting);
	dev_info(chip->dev, "fc_th_setting_new%d, fc_ith_spd_setting%d\n", fc_setting->fc_th,
		fc_setting->fc_ith_spd);

	mutex_lock(&chip->update_lock);
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, fc_setting->fc_th);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	regval = (regval & POWER_MASK_BYTE) | ((fc_setting->fc_ith_spd & POWER_MASK_BYTE) << 8);
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5, regval);

	/* lock the register for avoiding to write */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x0020);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5); /* delay 5ms for run success */
	mutex_unlock(&chip->update_lock);
}

static void rt9426_get_fc_setting_from_fg(struct rt9426_chip *chip,
	struct rt9426_fc_setting *fc_setting)
{
	fc_setting->fc_vth = chip->pdata->fc_vth[chip->ocv_index];
	fc_setting->fc_ith = ((fc_setting->fc_th >> POWER_BITS_PER_BYTE) &
		POWER_MASK_BYTE) * (int)chip->pdata->rs_ic_setting * 4; /* unit of setting is 4mA */
	fc_setting->fc_ith = DIV_ROUND_UP(fc_setting->fc_ith,
		(int)chip->pdata->rs_schematic);
	mutex_lock(&chip->update_lock);
	rt9426_read_page_cmd(chip, RT9426_PAGE_5);
	fc_setting->fc_ith_spd = rt9426_reg_read_word(chip->i2c,
		RT9426_REG_SWINDOW5);
	mutex_unlock(&chip->update_lock);
	fc_setting->fc_ith_spd = ((fc_setting->fc_ith_spd >> POWER_BITS_PER_BYTE) &
		POWER_MASK_BYTE) * (int)chip->pdata->rs_ic_setting;
	fc_setting->fc_ith_spd = DIV_ROUND_UP(fc_setting->fc_ith_spd,
		(int)chip->pdata->rs_schematic);
}

static void rt9426_fc_vth_low_curr_adjust(struct rt9426_chip *chip,
	struct ffc_ctrl_charge_info *ndata, struct rt9426_fc_setting *fc_setting)
{
	int cur = rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
	int vol = rt9426_get_display_data(chip, RT9426_DISPLAY_VBAT);

	if (!chip || !fc_setting || !ndata ||
		!chip->pdata->fc_low_curr_vth[chip->ocv_index])
		return;

	if (!chip->pdata->dynamic_change_iterm_en)
		rt9426_get_fc_setting_from_fg(chip, fc_setting);

	dev_info(chip->dev, "low_curr_vth condition: cur=%d, (0,%d]\n", cur,
		fc_setting->fc_ith + fc_setting->fc_ith_spd + chip->pdata->fc_low_curr_idelta);
	dev_info(chip->dev, "low_curr_vth condition: vol=%d, (%d,%d]\n", vol,
		RT9426_FC_VTH(fc_setting->fc_vth) - chip->pdata->fc_low_curr_vdelta,
		RT9426_FC_VTH(fc_setting->fc_vth));

	if ((cur <= (fc_setting->fc_ith + fc_setting->fc_ith_spd + chip->pdata->fc_low_curr_idelta)) &&
		(vol > (RT9426_FC_VTH(fc_setting->fc_vth) - chip->pdata->fc_low_curr_vdelta)) &&
		(cur > 0) && (vol <= RT9426_FC_VTH(fc_setting->fc_vth))) {
		if (chip->pdata->fc_low_curr_cnt < chip->pdata->fc_low_curr_cnt_th)
			chip->pdata->fc_low_curr_cnt++;
	}

	if (chip->pdata->fc_low_curr_cnt >= chip->pdata->fc_low_curr_cnt_th) {
		if (ndata->ffc_charge_flag)
			fc_setting->fc_vth = chip->pdata->fc_low_curr_vth[chip->ocv_index];
		else
			fc_setting->fc_vth = chip->pdata->fc_low_curr_vth_non_ffc[chip->ocv_index];
	}

	fc_setting->fc_th = (fc_setting->fc_vth & POWER_MASK_BYTE) |
		(fc_setting->fc_th & POWER_MASK_HIGH_BYTE);
	dev_info(chip->dev, "low_curr_vth: fc_vth=%d\n", RT9426_FC_VTH(fc_setting->fc_vth));
}

static void rt9426_fc_vth_setting(struct rt9426_chip *chip,
	struct ffc_ctrl_charge_info *ndata, struct rt9426_fc_setting *fc_setting)
{
	if (!chip || !ndata || !fc_setting ||
		!chip->pdata->dynamic_change_iterm_en)
		return;

	fc_setting->fc_vth = chip->pdata->fc_vth[chip->ocv_index];

	if (!ndata->ffc_charge_flag)
		fc_setting->fc_vth = chip->pdata->non_ffc_fc_vth[chip->ocv_index];

	fc_setting->fc_th = (fc_setting->fc_vth & POWER_MASK_BYTE) |
		(fc_setting->fc_th & POWER_MASK_HIGH_BYTE);
	dev_info(chip->dev, "dynamic_change_vterm: fc_vth=%d\n",
		RT9426_FC_VTH(fc_setting->fc_vth));
}

static void rt9426_fc_ith_setting(struct rt9426_chip *chip,
	struct ffc_ctrl_charge_info *ndata, struct rt9426_fc_setting *fc_setting)
{
	int fc_ith, fc_ith_spd, precisionA, precisionB;

	if (!chip || !ndata || !fc_setting || (ndata->iterm <= 0) ||
		!chip->pdata->dynamic_change_iterm_en)
		return;

	fc_setting->ffc_charge_flag = ndata->ffc_charge_flag;
	if (fc_setting->ffc_charge_flag) {
		if (ndata->dc_mode) {
			precisionA = chip->pdata->ffc_dc_para_A;
			precisionB = chip->pdata->ffc_dc_para_B;
		} else {
			precisionA = chip->pdata->ffc_buck_para_A;
			precisionB = chip->pdata->ffc_buck_para_B;
		}
	} else {
		precisionA = chip->pdata->non_ffc_para_A;
		precisionB = chip->pdata->non_ffc_para_B;
	}

	fc_ith = ((ndata->iterm * chip->pdata->iterm_ratio / RT9426_ITERM_RATIO_BASE *
		(RT9426_CURRENT_PRECISION_BASE + precisionA) / RT9426_CURRENT_PRECISION_BASE) +
		precisionB) * RT9426_CURRENT_PRECISION / RT9426_CURRENT_PRECISION_BASE;
	dev_info(chip->dev, "dynamic_change_vterm: fc_ith=%d\n", fc_ith);
	fc_setting->fc_ith = fc_ith;

	/* choose the proper val of speed window by fc_ith */
	if (fc_ith <= chip->pdata->fc_delta_ith_for_spd_th)
		fc_ith_spd = chip->pdata->fc_delta_ith_for_non_ffc_spd;
	else
		fc_ith_spd = chip->pdata->fc_delta_ith_for_ffc_spd;

	fc_ith = (fc_ith * (int)chip->pdata->rs_schematic);
	/* unit of setting is 4mA */
	fc_ith = DIV_ROUND_UP(fc_ith, (int)chip->pdata->rs_ic_setting * 4);

	/* recombine to fc_setting by bit operation */
	fc_setting->fc_th = (fc_setting->fc_th & POWER_MASK_BYTE) |
		((fc_ith & POWER_MASK_BYTE) << 8);
	fc_setting->fc_ith_spd = fc_ith_spd;
}

static int rt9426_get_fc_th(struct rt9426_chip *chip)
{
	int fc_th;

	if (!chip)
		return 0;

	mutex_lock(&chip->update_lock);
	rt9426_read_page_cmd(chip, RT9426_PAGE_5);
	fc_th = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	mutex_unlock(&chip->update_lock);

	return fc_th;
}

static void rt9426_dynamic_change_term_setting(struct rt9426_chip *chip,
	struct ffc_ctrl_charge_info *ndata)
{
	struct rt9426_fc_setting fc_setting;
	int fc_th_old, regval;

	if (!chip || !ndata)
		return;

	/* handling dynamic iterm setting only when charging */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG1);
	if (regval & BIT(0))
		return;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return;

	fc_th_old = rt9426_get_fc_th(chip);
	fc_setting.fc_th = fc_th_old;

	rt9426_fc_ith_setting(chip, ndata, &fc_setting);
	rt9426_fc_vth_setting(chip, ndata, &fc_setting);
	rt9426_fc_vth_low_curr_adjust(chip, ndata, &fc_setting);

	if (fc_setting.fc_th != fc_th_old)
		rt9426_dynamic_change_term_setting_reg_op(chip, &fc_setting);
}

static void rt9426_set_fast_fcc_lr(struct rt9426_chip *chip)
{
	int regval, fcc_ln_cond1, fcc_ln_cond2;

	mutex_lock(&chip->update_lock);
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	fcc_ln_cond1 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW8);
	/* Self-learning charge requirement changed to greater than 25% */
	regval = (fcc_ln_cond1 & POWER_MASK_HIGH_BYTE) | 0x0004;

	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, regval);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5);

	rt9426_read_page_cmd(chip, RT9426_PAGE_6);
	fcc_ln_cond2 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2);
	/* Self-learning accuracy changed to (2%, 4%, 8%) */
	regval = (fcc_ln_cond2 & 0x0F00) | 0x8024;

	rt9426_write_page_cmd(chip, RT9426_PAGE_6);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2, regval);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5);

	dev_info(chip->dev, "fcc_ln_cond_before: 0x%x, 0x%x", fcc_ln_cond1, fcc_ln_cond2);
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	fcc_ln_cond1 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW8);
	rt9426_read_page_cmd(chip, RT9426_PAGE_6);
	fcc_ln_cond2 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2);
	mutex_unlock(&chip->update_lock);
	dev_info(chip->dev, "fcc_ln_cond_after: 0x%x, 0x%x", fcc_ln_cond1, fcc_ln_cond2);
}

static void rt9426_set_slow_fcc_lr(struct rt9426_chip *chip)
{
	mutex_lock(&chip->update_lock);
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8,
		chip->pdata->extreg_update.extreg_update_data[3].extreg_data);
	mdelay(5);

	rt9426_write_page_cmd(chip, RT9426_PAGE_6);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2,
		chip->pdata->extreg_update.extreg_update_data[9].extreg_data);
	mdelay(5);
	mutex_unlock(&chip->update_lock);
	dev_info(chip->dev, "fcc_ln_condition: 0x%x, 0x%x",
		chip->pdata->extreg_update.extreg_update_data[3].extreg_data,
		chip->pdata->extreg_update.extreg_update_data[9].extreg_data);
}

static void rt9426_adjust_fcc_learning_rate(struct rt9426_chip *chip)
{
	int cycle, fcc_ratio, cur_fcc;
	int design_fcc = POWER_SUPPLY_DEFAULT_CHARGE_FULL * POWER_UA_PER_MA;

	if (!chip->pdata->fcc_lr_adjust_en)
		return;

	power_supply_get_int_property_value("battery",
		POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &design_fcc);
	cycle = rt9426_get_cyccnt(chip);
	cur_fcc = rt9426_get_display_data(chip, RT9426_DISPLAY_FCC);
	fcc_ratio = cur_fcc * RT9426_FCC_RATIO_BASE / (design_fcc / POWER_UA_PER_MA);
	dev_info(chip->dev, "%s: cycle=%d, fcc_ratio=%d, fcc_lr_status=%d\n",
		__func__, cycle, fcc_ratio, chip->pdata->fcc_lr_status);

	if ((fcc_ratio >= 0) && (fcc_ratio < chip->pdata->fcc_lr_ratio_th) &&
		(cycle >= 0) && (cycle < chip->pdata->fcc_lr_cycle_th)) {
		if ((chip->pdata->fcc_lr_status != RT9426_LR_SLOW) ||
			(rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)) {
			dev_info(chip->dev, "failed to adjust fcc_lr\n");
			return;
		}

		rt9426_set_fast_fcc_lr(chip);
		chip->pdata->fcc_lr_status = RT9426_LR_FAST;
	} else {
		if ((chip->pdata->fcc_lr_status != RT9426_LR_FAST) ||
			(rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)) {
			dev_info(chip->dev, "failed to adjust fcc_lr\n");
			return;
		}

		rt9426_set_slow_fcc_lr(chip);
		chip->pdata->fcc_lr_status = RT9426_LR_SLOW;
	}
}
#endif /* CONFIG_COUL_DRV */

static void rt9426_event_notifier_handler(struct rt9426_chip *chip,
	unsigned long event, void *data)
{
	switch (event) {
#ifndef CONFIG_COUL_DRV
	case POWER_NE_BUCK_FFC_CHARGE:
		rt9426_adjust_fcc_learning_rate(chip);
		rt9426_dynamic_change_term_setting(chip, data);
		break;
#endif /* CONFIG_COUL_DRV */
	case POWER_NE_CHARGING_START:
		chip->pdata->fc_low_curr_cnt = 0;
		break;
	default:
		return;
	}

	hwlog_info("receive event=%lu, term_setting done\n", event);
}

static int rt9426_buck_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct rt9426_chip *chip = container_of(nb,
		struct rt9426_chip, event_buck_nb);

	if (!chip)
		return NOTIFY_OK;

	rt9426_event_notifier_handler(chip, event, data);

	return NOTIFY_OK;
}

static int rt9426_charging_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct rt9426_chip *chip = container_of(nb,
		struct rt9426_chip, event_nb);

	if (!chip)
		return NOTIFY_OK;

	rt9426_event_notifier_handler(chip, event, data);

	return NOTIFY_OK;
}

static int rt9426_reg_write_ocv_table(struct rt9426_chip *chip, const u32 *pval)
{
	int i;
	int j;
	int regval;

	if (!pval)
		return -EPERM;
	for (i = 0; i < (RT9426_OCV_ROW_SIZE - 1); i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
			RT9426_REG_OCV_WRITE + i);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
			RT9426_REG_OCV_WRITE + i);
		for (j = 0; j < RT9426_OCV_COL_SIZE; j++) {
			/* write ocv_table[index][i][j] */
			rt9426_reg_write_word(chip->i2c,
				RT9426_REG_OCV_COL_ADDRESS + j * POWER_WORD_LEN,
				*(pval + i * RT9426_OCV_COL_SIZE + j));
			dev_info(chip->dev, "0x%x ",
				*(pval + i * RT9426_OCV_COL_SIZE + j));
			mdelay(1);  /* delay 1ms */
		}
		dev_info(chip->dev, "\n");
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
		RT9426_REG_OCV_WRITE + i);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
		RT9426_REG_OCV_WRITE + i);
	/* write ocv_table last line fine elements */
	for (i = 0; i < RT9426_OCV_TABLE_IN_OCV_COL; i++) {
		/* write ocv_table[index][9][i] */
		rt9426_reg_write_word(chip->i2c,
			RT9426_REG_OCV_COL_ADDRESS + i * POWER_WORD_LEN, *(pval +
			(RT9426_OCV_ROW_SIZE - 1) * RT9426_OCV_COL_SIZE + i));
		dev_info(chip->dev, "0x%x ",
			*(pval + (RT9426_OCV_ROW_SIZE - 1) *
			RT9426_OCV_COL_SIZE + i));
		mdelay(1); /* delay 1ms */
	}
	dev_info(chip->dev, "\n");
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(10); /* delay 10ms */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval & RT9426_USR_TBL_USED_MASK) {
		dev_info(chip->dev, "OCV Table Write Successful\n");
		return 0;
	}
	return -EPERM;
}

static int rt9426_write_ocv_table(struct rt9426_chip *chip)
{
	int i;
	int retry_times = 3; /* try write 3 times */
	const u32 *pval = (u32 *)chip->pdata->ocv_table +
		chip->ocv_index * RT9426_OCV_DATA_TOTAL_SIZE;

	if (*pval != RT9426_OCV_TABLE_FIRST)
		return -EPERM;

	dev_info(chip->dev, "Write NEW OCV Table\n");
	for (i = 0; i < retry_times; i++) {
		if (!rt9426_reg_write_ocv_table(chip, pval))
			return 0;
	}

	dev_notice(chip->dev, "Set OCV Table fail\n");
	return -EPERM;
}

static void rt9426_update_info(struct rt9426_chip *chip)
{
	int regval;
	int ret;
	struct power_supply *batt_psy = NULL;

	dev_dbg(chip->dev, "%s\n", __func__);

	/* get battery temp from battery power supply */
	if (chip->pdata->platform_support)
		batt_psy = power_supply_get_by_name("Battery");
	else
		batt_psy = power_supply_get_by_name("battery");

	if (!batt_psy)
		return;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return;

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);

	dev_dbg(chip->dev, "OPCFG(0x%x 0x%x 0x%x 0x%x 0x%x)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5));

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
	if (regval < 0)
		return;
	regval = (regval & 0x0300) >> 8; /* get defined state */
	if (((ret & 0x0800) >> 11) == 1) { /* get OCV */
		dev_dbg(chip->dev, "OCV table define by User\n");
	} else {
		if (regval == 0)
			dev_dbg(chip->dev, "OCV(4200) Zero_Point(3200)\n");
		else if (regval == 1)
			dev_dbg(chip->dev, "OCV(4350) Zero_Point(3200)\n");
		else if (regval == 2)
			dev_dbg(chip->dev, "OCV(4400) Zero_Point(3200)\n");
		else
			dev_dbg(chip->dev, "OCV(4350) Zero_Point(3400)\n");
	}
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	dev_dbg(chip->dev, "CSCOMP4(%d)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	dev_dbg(chip->dev, "CSCOMP5(%d)\n", regval);

	dev_dbg(chip->dev, "DSNCAP(%d) FCC(%d)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC));

	dev_dbg(chip->dev,
		"VOLT_SOURCE(0x%x) INPUT_VOLT(%d) FG_VBAT(%d) FG_OCV(%d) FG_AV(%d)\n",
		chip->pdata->volt_source, rt9426_get_volt(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_OCV),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AV));
	dev_dbg(chip->dev,
		"CURR_SOURCE(0x%x) INPUT_CURR(%d) FG_CURR(%d) FG_AI(%d)\n",
		chip->pdata->curr_source, rt9426_get_current(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AI));
	dev_dbg(chip->dev, "TEMP_SOURCE(0x%x) INPUT_TEMP(%d) FG_TEMP(%d)\n",
		chip->pdata->temp_source, rt9426_get_temp(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_TEMP));
	dev_dbg(chip->dev, "FG_FG_INTT(%d) FG_AT(%d)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_INTT),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AT));

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG1);
	dev_dbg(chip->dev, "FLAG1(0x%x)\n", regval);
	if (regval < 0)
		return;
	if (((regval & 0x0200) >> 9) == 1) /* get FC */
		dev_dbg(chip->dev, "FC = 1\n");
	else
		dev_dbg(chip->dev, "FC = 0\n");

	if (((regval & 0x0004) >> 2) == 1) /* get FD */
		dev_dbg(chip->dev, "FD = 1\n");
	else
		dev_dbg(chip->dev, "FD = 0\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	dev_dbg(chip->dev, "FLAG2(0x%x)\n", regval);
	if (regval < 0)
		return;

	if (((regval & 0xE000) >> 13) == 0) /* get Active mode */
		dev_dbg(chip->dev, "Power_Mode (Active)\n");
	else if (((regval & 0xE000) >> 13) == 1) /* get FST_RSP_ACT mode */
		dev_dbg(chip->dev, "Power_Mode (FST_RSP_ACT)\n");
	else if (((regval & 0xE000) >> 13) == 2) /* get Shutdown mode */
		dev_dbg(chip->dev, "Power_Mode (Shutdown)\n");
	else
		dev_dbg(chip->dev, "Power_Mode (Sleep)\n");

	if (((regval & 0x0800) >> 11) == 1) /* get User_Define_Table state */
		dev_dbg(chip->dev, "User_Define_Table (IN USE)\n");
	else
		dev_dbg(chip->dev, "User_Define_Table (NOT IN USE)\n");
	if (((regval & 0x0040) >> 6) == 1) /* get Battery_Status state */
		dev_dbg(chip->dev, "Battery_Status (Inserted)\n");
	else
		dev_dbg(chip->dev, "Battery_Status (Removed)\n");

	if (((regval & 0x0001)) == 1) /* get RLX mode */
		dev_dbg(chip->dev, "RLX = 1\n");
	else
		dev_dbg(chip->dev, "RLX = 0\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	dev_dbg(chip->dev, "FLAG3(0x%x)\n", regval);
	if (regval < 0)
		return;
	if (((regval & 0x0100) >> 8) == 1) /* get RI mode */
		dev_dbg(chip->dev, "RI = 1\n");
	else
		dev_dbg(chip->dev, "RI = 0\n");

	if (((regval & 0x0001)) == 1) /* get Seal mode */
		dev_dbg(chip->dev, "RT9426 (Unseal)\n");
	else
		dev_dbg(chip->dev, "RT9426 (Seal)\n");

	dev_dbg(chip->dev, "CYCCNT(%d)\n", rt9426_get_cyccnt(chip));

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_VGCOMP12);
	dev_dbg(chip->dev, "VGCOMP12(0x%x)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_VGCOMP34);
	dev_dbg(chip->dev, "VGCOMP34(0x%x)\n", regval);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	dev_dbg(chip->dev, "FCC(%d)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	dev_dbg(chip->dev, "DSNCAP(%d)\n", regval);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_UN_FLT_SOC);
	dev_dbg(chip->dev, "UNFSOC(%d)\n", regval);
	chip->capacity = rt9426_get_soc(chip);

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	regval = chip->capacity;
	regval = regval * ret;
	regval = DIV_ROUND_UP(regval, 100); /* rm: capacity * fcc / 100 */
	dev_dbg(chip->dev, "RM(%d)\n", regval);

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	ret = ret * 100; /* full capacity */
	regval = DIV_ROUND_UP(ret, regval);
	dev_dbg(chip->dev, "SOH(%d)\n", regval);

	power_supply_changed(batt_psy);
	power_supply_put(batt_psy);
}

static int rt9426_apply_pdata(struct rt9426_chip *);
static irqreturn_t rt9426_irq_handler(int irqno, void *param)
{
	struct rt9426_chip *chip = (struct rt9426_chip *)param;
	u16 irq_val;

	if (!chip)
		return IRQ_HANDLED;

	irq_val = rt9426_reg_read_word(chip->i2c, RT9426_REG_IRQ);
	/* 0x02: bit flag for UV_IRQ */
	if (irq_val & 0x02)
		power_event_bnc_notify(POWER_BNT_COUL, POWER_NE_COUL_LOW_VOL, NULL);

	return IRQ_HANDLED;
}

static void new_vgcomp_soc_offset_datas(struct device *dev, int type,
	struct rt9426_platform_data *pdata, int size_x, int size_y, int size_z)
{
	switch (type) {
	case SOC_OFFSET:
		if (pdata->soc_offset.soc_offset_data) {
			devm_kfree(dev,
				pdata->soc_offset.soc_offset_data);
			pdata->soc_offset.soc_offset_data = NULL;
		}
		if ((size_x != 0) && (size_y != 0))
			pdata->soc_offset.soc_offset_data =
				devm_kzalloc(dev, size_x * size_y * sizeof(struct data_point),
					GFP_KERNEL);
		if (pdata->soc_offset.soc_offset_data) {
			pdata->soc_offset.soc_voltnr = size_x;
			pdata->soc_offset.tempnr = size_y;
		} else {
			pdata->soc_offset.soc_voltnr = 0;
			pdata->soc_offset.tempnr = 0;
		}
		break;
	case EXTREG_UPDATE:
		if (pdata->extreg_update.extreg_update_data) {
			devm_kfree(dev,
				pdata->extreg_update.extreg_update_data);
			pdata->extreg_update.extreg_update_data = NULL;
		}
		if (size_x != 0)
			pdata->extreg_update.extreg_update_data =
				devm_kzalloc(dev, size_x * sizeof(struct extreg_data_point),
					GFP_KERNEL);
		break;
	default:
		break;
	}
}

static inline const struct data_point *get_mesh_data(
	int i, int j, int k,
	const struct data_point *mesh, int xnr, int ynr)
{
	return mesh + k * ynr * xnr + j * xnr + i;
}

static int get_sub_mesh(int state, struct data_point *mesh_buffer,
	struct submask_condition *condition)
{
	int i, j;
	int k = 0;
	int x = condition->x;
	int y = condition->y;
	int z = condition->z;

	for (i = 0; i < condition->xnr; ++i) {
		if (get_mesh_data(i, 0, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->x >= x)
			break;
	}
	for (; i >= 0 && i < condition->xnr; --i) {
		if (get_mesh_data(i, 0, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->x <= x)
			break;
	}

	for (j = 0; j < condition->ynr; ++j) {
		if (get_mesh_data(0, j, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->y >= y)
			break;
	}
	for (; j >= 0 && j < condition->ynr; --j) {
		if (get_mesh_data(0, j, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->y <= y)
			break;
	}

	if (state == FG_COMP) {
		for (k = 0; k < condition->znr; ++k) {
			if (get_mesh_data(0, 0, k, condition->mesh_src,
				condition->xnr, condition->ynr)->z >= z)
				break;
		}
		for (; k >= 0 && k < condition->znr; --k) {
			if (get_mesh_data(0, 0, k, condition->mesh_src,
				condition->xnr, condition->ynr)->z <= z)
				break;
		}
	}

	i -= ((condition->order_x - 1) / 2);
	j -= ((condition->order_y - 1) / 2);
	k -= ((condition->order_z - 1) / 2);

	if (i <= 0)
		i = 0;
	if (j <= 0)
		j = 0;
	if (k <= 0)
		k = 0;
	if ((i + condition->order_x) > condition->xnr)
		i = condition->xnr - condition->order_x;
	if ((j + condition->order_y) > condition->ynr)
		j = condition->ynr - condition->order_y;
	if ((k + condition->order_z) > condition->znr)
		k = condition->znr - condition->order_z;

	if (state == FG_COMP) {
		for (z = 0; z < condition->order_z; ++z) {
			for (y = 0; y < condition->order_y; ++y) {
				for (x = 0; x < condition->order_x; ++x)
					*(mesh_buffer + z * condition->order_y *
						condition->order_z +
						y * condition->order_x + x)
						= *get_mesh_data(i + x, j + y,
							k + z,
							condition->mesh_src,
							condition->xnr,
							condition->ynr);
			}
		}
	} else {
		for (y = 0; y < condition->order_y; ++y) {
			for (x = 0; x < condition->order_x; ++x)
				*(mesh_buffer + y * condition->order_x + x)
					= *get_mesh_data(i + x, j + y, 0,
						condition->mesh_src,
						condition->xnr,
						condition->ynr);
		}
	}
	return 0;
}

static int offset_li(int xnr, int ynr,
	const struct data_point *mesh, int x, int y)
{
	long long retval = 0;
	int i, j, k;
	long long wm, wd;
	const struct data_point *cache = NULL;

	for (i = 0; i < xnr; ++i) {
		for (j = 0; j < ynr; ++j) {
			wm = wd = 1;
			cache = get_mesh_data(i, j, 0, mesh, xnr, ynr);
			for (k = 0; k < xnr; ++k) {
				if (i != k) {
					wm *= (x - get_mesh_data(k, j, 0,
						mesh, xnr, ynr)->x);
					wd *= (cache->x - get_mesh_data(k, j, 0,
						mesh, xnr, ynr)->x);
				}
			}
			for (k = 0; k < ynr; ++k) {
				if (j != k) {
					wm *= (y - get_mesh_data(i, k, 0,
							mesh, xnr, ynr)->y);
					wd *= (cache->y - get_mesh_data(i, k, 0,
							mesh, xnr, ynr)->y);
				}
			}
			retval += div64_s64(
				((cache->w * wm) << PRECISION_ENHANCE), wd);
		}
	}
	return (int)((retval + (1 << (PRECISION_ENHANCE - 1)))
		>> PRECISION_ENHANCE);
}

static int rt9426_get_offset(struct rt9426_chip *chip, int soc_val, int temp)
{
	const int ip_x = chip->pdata->offset_interpolation_order[0];
	const int ip_y = chip->pdata->offset_interpolation_order[1];
	struct data_point *sub_mesh = NULL;
	int xnr, ynr;
	int offset;
	struct soc_offset_table *offset_table = NULL;
	struct submask_condition condition = {
		.x = soc_val,
		.y = temp,
	};

	sub_mesh = kzalloc(ip_x * ip_y * sizeof(struct data_point), GFP_KERNEL);
	if (!sub_mesh)
		return 0;
	mutex_lock(&chip->var_lock);
	offset_table = &chip->pdata->soc_offset;
	xnr = offset_table->soc_voltnr;
	ynr = offset_table->tempnr;
	if ((xnr == 0) || (ynr == 0)) {
		mutex_unlock(&chip->var_lock);
		kfree(sub_mesh);
		return 0;
	}
	condition.order_x = min(ip_x, xnr);
	condition.order_y = min(ip_y, ynr);
	condition.xnr = xnr;
	condition.ynr = ynr;
	condition.mesh_src = offset_table->soc_offset_data;
	get_sub_mesh(SOC_OFFSET, sub_mesh, &condition);
	offset = offset_li(condition.order_x, condition.order_y, sub_mesh,
		soc_val, temp);
	mutex_unlock(&chip->var_lock);
	kfree(sub_mesh);
	return offset;
}

/*
 * In order to ensure the accuracy of the low coulometer at different temperatures,
 * tl table should be adjusted when fd_th changes. tl table consists of 13 words,
 * 5 in window 4 of page 7, 8 in window 1 of page 8.
 */
static int rt9426_write_tl_table_one_time(struct rt9426_chip *chip, const u32 *pval)
{
	int i;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return -EIO;

	rt9426_write_page_cmd(chip, RT9426_PAGE_7);
	for (i = 0; i < RT9426_TL_TABLE_IN_WINDOW4; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4 +
			i * POWER_WORD_LEN, *(pval + i));
		mdelay(1);
	}

	rt9426_write_page_cmd(chip, RT9426_PAGE_8);
	for (i = 0; i < RT9426_TL_TABLE_IN_WINDOW1; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1 +
			i * POWER_WORD_LEN, *(pval + 5 + i));
		mdelay(1);
	}

	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5);

	return 0;
}

static int rt9426_write_tl_table_check(struct rt9426_chip *chip, const u32 *pval)
{
	int i;
	int read_array[RT9426_TL_DATA_TOTAL_SIZE] = { 0 };

	rt9426_read_page_cmd(chip, RT9426_PAGE_7);
	for (i = 0; i < RT9426_TL_TABLE_IN_WINDOW4; i++)
		read_array[i] = rt9426_reg_read_word(chip->i2c,
			RT9426_REG_SWINDOW4 + i * POWER_WORD_LEN);

	rt9426_read_page_cmd(chip, RT9426_PAGE_8);
	for (i = 0; i < RT9426_TL_TABLE_IN_WINDOW1; i++)
		read_array[i + 5] = rt9426_reg_read_word(chip->i2c,
			RT9426_REG_SWINDOW1 + i * POWER_WORD_LEN);

	if (memcmp(read_array, pval, 13 * POWER_DWORD_LEN) == 0) {
		dev_notice(chip->dev, "Set TL Table PASS\n");
		return RT9426_WRITE_TL_PASS;
	}

	return RT9426_WRITE_TL_FAIL;
}

static int rt9426_write_tl_table(struct rt9426_chip *chip)
{
	int retry_times = 3;
	int ret;
	const u32 *pval = (u32 *)chip->pdata->tl_table + chip->ocv_index *
		RT9426_TL_DATA_TOTAL_SIZE;

	while (retry_times > 0) {
		rt9426_write_tl_table_one_time(chip, pval);
		ret = rt9426_write_tl_table_check(chip, pval);
		if (ret == RT9426_WRITE_TL_PASS) {
			dev_notice(chip->dev, "Set TL Table PASS\n");
			return ret;
		}

		retry_times--;
	}
	dev_notice(chip->dev, "Set TL Table FAIL\n");

	return ret;
}

static void rt9426_write_fd_th(struct rt9426_chip *chip)
{
	int fd_vth_target;
	int fd_spd_target;

	rt9426_read_page_cmd(chip, RT9426_PAGE_5);
	fd_vth_target = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
	fd_spd_target = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW8);

	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6,
		(chip->pdata->fd_vth[chip->ocv_index]) |
		(fd_vth_target & POWER_MASK_HIGH_BYTE));
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8,
		(chip->pdata->fd_spd_vth[chip->ocv_index]) |
		(fd_spd_target & POWER_MASK_HIGH_BYTE));
}

static int rt9426_set_si_ocv_table(struct rt9426_chip *chip, int ocv_index)
{
	u32 regval;

	if (!chip)
		return -EPERM;

	mutex_lock(&chip->update_lock);
	chip->ocv_index = ocv_index;
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
	if ((regval & POWER_MASK_WORD) == chip->pdata->fcc[chip->ocv_index]) {
		dev_info(chip->dev, "RT9426 ocv table is already write to reg\n");
		goto set_ocv_table_end;
	}

	/* write aging ocv table */
	rt9426_write_ocv_table(chip);
	/* write aging tl table */
	rt9426_write_tl_table(chip);
	/* write aging fcc */
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		chip->pdata->fcc[chip->ocv_index]);
	/* write aging fc_vth */
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3,
		(chip->pdata->fc_vth[chip->ocv_index]) |
		(chip->pdata->fc_ith << POWER_BITS_PER_BYTE));
	/* write aging fd_vth and fd_spd */
	rt9426_write_fd_th(chip);

	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5);

set_ocv_table_end:
	mutex_unlock(&chip->update_lock);
	return 0;
}

static int rt9426_set_ocv_table(struct rt9426_chip *chip, int ocv_index)
{
	u32 regval;

	if (!chip)
		return -EPERM;

	mutex_lock(&chip->update_lock);
	chip->ocv_index = ocv_index;
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
	if ((regval & POWER_MASK_WORD) == chip->pdata->fcc[chip->ocv_index]) {
		dev_info(chip->dev, "RT9426 ocv table is already write to reg\n");
		goto set_ocv_table_end;
	}

	/* write aging ocv table */
	(void)rt9426_write_ocv_table(chip);
	/* write aging fcc */
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		chip->pdata->fcc[chip->ocv_index]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5); /* delay 5ms */
	/* write aging fc_vth */
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	/* write fc_vth and fc_ith */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3,
		chip->pdata->fc_vth[chip->ocv_index] |
		(chip->pdata->fc_ith << 8));

set_ocv_table_end:
	mutex_unlock(&chip->update_lock);
	return 0;
}

static int get_parameters(char *buf, long *param, int num_of_par)
{
	char *token = NULL;
	int base, cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16;
			else
				base = 10;

			if (kstrtol(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static bool rt9426_check_illegal_input(const char *buf, int value)
{
	int x = 0;

	if ((sscanf(buf, "%x\n", &x) != 1) || (x != value))
		return true;

	return false;
}

#ifdef CONFIG_DEBUG_FS
static int dentry_id_to_comp_offset_type[] = {
	[RT9426FG_SOC_OFFSET_SIZE] = SOC_OFFSET,
	[RT9426FG_SOC_OFFSET_DATA] = SOC_OFFSET,
	[RT9426FG_PARAM_LOCK] = -1, /* dummy */
	[RT9426FG_OFFSET_IP_ORDER] = -1, /* dummy */
	[RT9426FG_FIND_OFFSET_TEST] = -1, /* dummy */
	[RT9426FG_PARAM_CHECK] = -1, /* dummy */
};

struct rt9426_dbg_private_data {
	struct rt9426_chip *chip;
	int id;
	int counter;
	int temp;
	int volt;
	int curr;
	int soc_val;
	int temp2;
};

static ssize_t rt9426_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	struct seq_file *s = filp->private_data;
	struct rt9426_dbg_private_data *prv_data = s->private;
	struct rt9426_dbg_private_data *prv_data_header = NULL;
	struct rt9426_chip *chip = prv_data->chip;
	struct data_point *data = NULL;
	char lbuf[RT9426_WRITE_BUF_LEN];
	int rc;
	int index;
	int comp_offset_type;
	long param[RT9426_MAX_PARAMS];

	if (cnt > (sizeof(lbuf) - 1))
		return -EINVAL;
	rc = copy_from_user(lbuf, ubuf, cnt);
	if (rc)
		return -EFAULT;
	lbuf[cnt] = '\0';
	comp_offset_type = dentry_id_to_comp_offset_type[prv_data->id];
	prv_data_header = prv_data - prv_data->id;
	switch (prv_data->id) {
	case RT9426FG_SOC_OFFSET_SIZE:
		rc = get_parameters(lbuf, param, RT9426_SOC_OFFSET_SIZE);
		if (rc < 0)
			return rc;
		new_vgcomp_soc_offset_datas(chip->dev, SOC_OFFSET,
			chip->pdata, param[0], param[1], 0);
		prv_data_header[prv_data->id + 1].counter = 0;
		break;
	case RT9426FG_SOC_OFFSET_DATA:
		index = comp_offset_type - SOC_OFFSET;
		rc = get_parameters(lbuf, param, RT9426_SOC_OFFSET_DATA_SIZE);
		if (rc < 0)
			return rc;
		data = chip->pdata->soc_offset.soc_offset_data + prv_data->counter;
		data->voltage = param[0];
		data->temperature = param[1];
		data->offset = param[2];

		prv_data->counter++;
		break;
	case RT9426FG_PARAM_LOCK:
		rc = get_parameters(lbuf, param, RT9426FG_PARAM_LOCK_SIZE);
		if (rc < 0)
			return rc;
		if (param[0]) {
			if (prv_data->counter == 0)
				mutex_lock(&chip->var_lock);
			prv_data->counter = 1;
		} else {
			if (prv_data->counter == 1)
				mutex_unlock(&chip->var_lock);
			prv_data->counter = 0;
		}
		break;
	case RT9426FG_OFFSET_IP_ORDER:
		rc = get_parameters(lbuf, param, RT9426FG_OFFSET_IP_ORDER_SIZE);
		if (rc < 0)
			return rc;
		chip->pdata->offset_interpolation_order[0] = param[0];
		chip->pdata->offset_interpolation_order[1] = param[1];
		break;
	case RT9426FG_FIND_OFFSET_TEST:
		rc = get_parameters(lbuf, param, RT9426FG_FIND_OFFSET_TEST_SIZE);
		if (rc < 0)
			return rc;
		prv_data->soc_val = param[0];
		prv_data->temp2 = param[1];
		break;
	case RT9426FG_PARAM_CHECK:
		cnt = -ENOTSUPP;
		break;
	default:
		break;
	}

	return cnt;
}

static int rt9426_debug_read(struct seq_file *s, void *unused)
{
	struct rt9426_dbg_private_data *prv_data = s->private;
	struct rt9426_chip *chip = prv_data->chip;
	struct rt9426_dbg_private_data *prv_data_header;
	struct data_point *data = NULL;
	int i;
	int offset;
	int data_size;

	prv_data_header = prv_data - prv_data->id;
	switch (prv_data->id) {
	case RT9426FG_SOC_OFFSET_SIZE:
		seq_printf(s, "%d %d\n", chip->pdata->soc_offset.soc_voltnr,
			chip->pdata->soc_offset.tempnr);
		break;
	case RT9426FG_SOC_OFFSET_DATA:
		data_size = chip->pdata->soc_offset.soc_voltnr *
			chip->pdata->soc_offset.tempnr;
		if (data_size == 0)
			seq_puts(s, "no data\n");

		data = chip->pdata->soc_offset.soc_offset_data;
		for (i = 0; i < data_size; i++, data++)
			seq_printf(s, "%d %d %d\n", data->voltage,
				data->temperature, data->offset);
		break;
	case RT9426FG_PARAM_LOCK:
		seq_printf(s, "%d\n", prv_data->counter);
		break;
	case RT9426FG_OFFSET_IP_ORDER:
		seq_printf(s, "%d %d\n",
			chip->pdata->offset_interpolation_order[0],
			chip->pdata->offset_interpolation_order[1]);
		break;
	case RT9426FG_FIND_OFFSET_TEST:
		offset = rt9426_get_offset(chip, prv_data->soc_val,
			prv_data->temp2);
		seq_printf(s, "<%d %d> : offset = <%d>\n",
			prv_data->soc_val, prv_data->temp2, offset);
		break;
	case RT9426FG_PARAM_CHECK:
		break;
	default:
		break;
	}
	return 0;
}

static int rt9426_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, rt9426_debug_read, inode->i_private);
}

static const struct file_operations rt9426_debug_ops = {
	.open = rt9426_debug_open,
	.write = rt9426_debug_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const char * const dbgfs_names[] = {
	[RT9426FG_SOC_OFFSET_SIZE] = "soc_offset_size",
	[RT9426FG_SOC_OFFSET_DATA] = "soc_offset_data",
	[RT9426FG_PARAM_LOCK] = "param_lock",
	[RT9426FG_OFFSET_IP_ORDER] = "offset_ip_order",
	[RT9426FG_FIND_OFFSET_TEST] = "find_offset_test",
	[RT9426FG_PARAM_CHECK] = "param_check",
};

#define decl_rt9426fg_prv_data(_id) \
{ \
	.id = _id, \
	.counter = 0, \
}

static struct rt9426_dbg_private_data rt9426_dbg_private_data[] = {
	decl_rt9426fg_prv_data(RT9426FG_SOC_OFFSET_SIZE),
	decl_rt9426fg_prv_data(RT9426FG_SOC_OFFSET_DATA),
	decl_rt9426fg_prv_data(RT9426FG_PARAM_LOCK),
	decl_rt9426fg_prv_data(RT9426FG_OFFSET_IP_ORDER),
	decl_rt9426fg_prv_data(RT9426FG_FIND_OFFSET_TEST),
	decl_rt9426fg_prv_data(RT9426FG_PARAM_CHECK),
};

static void rt9426_create_debug_files(struct rt9426_chip *chip)
{
	int i;

	if (chip->pdata->ic_role == RT9426_IC_TYPE_MAIN)
		chip->dir_dentry = debugfs_create_dir("rt9426fg_table", 0);
	else
		chip->dir_dentry = debugfs_create_dir("rt9426fg_table_aux", 0);
	if (IS_ERR(chip->dir_dentry)) {
		dev_notice(chip->dev,
			"%s : cannot create rt9426fg_table\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(dbgfs_names); ++i) {
		rt9426_dbg_private_data[i].chip = chip;
		chip->file_dentries[i] =
			debugfs_create_file(dbgfs_names[i],
				S_IFREG | 0444,
				chip->dir_dentry,
				&rt9426_dbg_private_data[i],
				&rt9426_debug_ops);
	}
}
#else
static void rt9426_create_debug_files(struct rt9426_chip *chip)
{
	dev_notice(chip->dev, "Not support debugfs\n");
}
#endif /* CONFIG_DEBUG_FS */

/* Add for calibration */
static int rt9426_get_calibration_para(struct rt9426_chip *chip, u8 *curr_offs,
	u8 *curr_gain, u8 *volt_gain)
{
	return 0;
}

static void rt9426_check_system_tick(struct rt9426_chip *chip)
{
	int j;
	int tick_old;
	int tick_new;

	/* Check System Tick before entering Calibration */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SYS_TICK_ON_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);

	usleep_range(5000, 5100); /* delay 5ms */

	tick_old = rt9426_reg_read_word(chip->i2c, RT9426_REG_ADV);
	for (j = 0; j < 1000; j++) { /* try 1000 times */
		tick_new = rt9426_reg_read_word(chip->i2c, RT9426_REG_ADV);
		if (tick_old != tick_new) {
			mdelay(300); /* delay 300ms */
			break;
		}
		usleep_range(5000, 5100); /* delay 5ms */
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CALI_ENTR_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SYS_TICK_OFF_CMD);
}

static int rt9426_enter_calibration_mode(struct rt9426_chip *chip)
{
	int i;
	int regval;
	int retry_times = 3; /* 3:calibration retry times */

	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -EPERM;
		if ((regval & RT9426_CALI_MODE_MASK) == RT9426_CALI_MODE_MASK) {
			dev_info(chip->dev, "RT9426 is in Calibration Mode\n");
			return RT9426_CALI_MODE_PASS;
		}
		if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
			rt9426_check_system_tick(chip);
			if (i >= (retry_times - 1)) {
				dev_info(chip->dev,
					"RT9426 Enter Calibration Mode Fail after 3 retries\n");
				return RT9426_CALI_MODE_FAIL;
			}
			usleep_range(5000, 5100); /* delay 5ms */
		}
	}
	return RT9426_CALI_MODE_FAIL;
}

static void rt9426_exit_calibration_mode(struct rt9426_chip *chip)
{
	int regval;
	int retry_times = 3; /* retry 3 times */

	do {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CALI_EXIT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);

		usleep_range(5000, 5100); /* delay 5ms */

		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (!(regval & RT9426_CALI_MODE_MASK)) {
			dev_info(chip->dev, "Success to exit calibration mode\n");
			break;
		}
	} while (--retry_times);
}

static void rt9426_apply_sense_resistor(struct rt9426_chip *chip)
{
	int op_config1;
	int rsense;

	switch (chip->pdata->rs_ic_setting) {
	case 250: /* 2.5mR */
		rsense = 0;
		break;
	case 500: /* 5mR */
		rsense = 1;
		break;
	case 2000: /* 20mR */
		rsense = 3;
		break;
	case 1000: /* 10mR */
	default:
		rsense = 2;
		break;
	}
	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		/* get op_config1 */
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		op_config1 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
		if (op_config1 < 0)
			return;
		/* update rsense to op_config1 */
		op_config1 = (op_config1 & 0xFF3F) | ((rsense & 0x3) << 6);
		/* apply op_config1 */
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, op_config1);
		mdelay(5); /* 5: delay 5ms for write success */
	}
}

static void rt9426_apply_calibration_para(struct rt9426_chip *chip, u8 curr_offs,
	u8 curr_gain, u8 volt_gain)
{
	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_write_page_cmd(chip, RT9426_PAGE_0);
		/* set Current system gain & offset */
		if ((curr_gain != 0x00) && (curr_gain != 0xFF) &&
			(curr_offs != 0x00) && (curr_offs != 0xFF))
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1,
				curr_gain | (curr_offs << 8));
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8080);
		/* set Voltage system gain */
		if ((volt_gain != 0x00) && (volt_gain != 0xFF))
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
				0x88 | (volt_gain << 8));
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7, 0x8088);
		mdelay(5); /* 5: delay 5ms */
	}
}

static int rt9426_get_curr_by_conversion(struct rt9426_chip *chip)
{
	int regval = 0;

	if (rt9426_enter_calibration_mode(chip) == RT9426_CALI_MODE_PASS) {
		/* Start current convertion */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CURR_CONVERT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(50);

		/* Get convert result */
		regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR);
		if (regval < 0)
			return -EIO;
		if (regval > 0x7FFF) {
			regval = 0x10000 - regval;
			regval = 0 - regval;
		}
	}
	/* scaling for the current, 2020-08-07 */
	regval = regval * (int)chip->pdata->rs_ic_setting / (int)chip->pdata->rs_schematic;
	dev_info(chip->dev, "CALIB_CURRENT = %d mA\n", regval);

	return regval;
}

static int rt9426_get_volt_by_conversion(struct rt9426_chip *chip)
{
	int regval = 0;

	if (rt9426_enter_calibration_mode(chip) == RT9426_CALI_MODE_PASS) {
		/* Start current convertion */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_VOLT_CONVERT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(50); /* 50: delay 50ms */

		/* Get voltage result */
		regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
	}
	return regval;
}

/* Add subroutine to handle enter/exit shutdown mode, 2020-08-05 */
static int rt9426_request_charging_inhibit(bool need_inhibit)
{
	return 0;
}

static int rt9426_enter_shutdown_mode(struct rt9426_chip *chip)
{
	int regval, loop;

	for (loop = 0; loop < 5; loop++) { /* 5: retry 5 times */
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		dev_info(chip->dev, "flag2 = 0x%04x\n", regval);
		if (regval < 0)
			return -EPERM;
		if (regval & RT9426_SHDN_MASK)
			break;

		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SHDN_ENTR_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(20); /* 20: delsy 20ms */
	}

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval < 0)
		return -EPERM;
	if (!(regval & RT9426_SHDN_MASK)) {
		dev_info(chip->dev, "RT9426 Enter Shutdown Fail\n");
		return -EPERM;
	}

	dev_info(chip->dev, "RT9426 Enter Shutdown Success\n");
	return 0;
}

static int rt9426_exit_shutdown_mode(struct rt9426_chip *chip)
{
	int regval, loop;
	int cmd_cnt = 0;

	for (loop = 0; loop < 5; loop++) { /* 5: retry 5 times */
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -EPERM;
		if (!(regval & RT9426_SHDN_MASK))
			break;
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SHDN_EXIT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(250); /* 250: delay 250ms */
		dev_info(chip->dev, "RT9426 Send Exit Shutdown Cmd Count = %d\n", ++cmd_cnt);
	}
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval < 0)
		return -EPERM;
	if (regval & RT9426_SHDN_MASK) {
		dev_info(chip->dev, "RT9426 is in Shutdown\n");
		return -EPERM;
	}
	dev_info(chip->dev, "RT9426 is not in Shutdown\n");

	if (cmd_cnt == 0)
		return 0;

	/* Power path control check */
	regval = rt9426_get_current(chip);
	if (regval > 0) {
		rt9426_request_charging_inhibit(true); /* Disable power path */
		dev_info(chip->dev, "RT9426 request to enable charging inhibit\n");
		mdelay(1250); /* 1250: delay 1250ms */
	}
	/* Send QS Command to get INI SOC */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x4000);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	dev_info(chip->dev, "Send QS after exiting Shutdown\n");
	mdelay(5); /* 5: delay 5ms */
	/* Power path recover check */
	if (regval > 0) {
		rt9426_request_charging_inhibit(false); /* Enable power path */
		dev_info(chip->dev, "RT9426 request to disable charging inhibit\n");
	}

	return 0;
}

static ssize_t rt9426_show_attrs(struct device *, struct device_attribute *,
	char *);
static ssize_t rt9426_store_attrs(struct device *, struct device_attribute *,
	const char *, size_t count);

#define rt9426_attr(_name) \
{ \
	.attr = {.name = #_name, .mode = 0664}, \
	.show = rt9426_show_attrs, \
	.store = rt9426_store_attrs, \
}

static struct device_attribute rt9426_fuelgauge_attrs[] = {
	rt9426_attr(fg_temp),
	rt9426_attr(volt),
	rt9426_attr(curr),
	rt9426_attr(update),
	rt9426_attr(ocv_table),
	rt9426_attr(enter_hibernate),
	rt9426_attr(exit_hibernate),
	rt9426_attr(set_hibernate_duty),
	rt9426_attr(DBP0),
	rt9426_attr(DBP1),
	rt9426_attr(DBP2),
	rt9426_attr(DBP3),
	rt9426_attr(DBP4),
	rt9426_attr(DBP5),
	rt9426_attr(DBP6),
	rt9426_attr(DBP7),
	rt9426_attr(DBP8),
	rt9426_attr(DBP9),
	rt9426_attr(WCNTL),
	rt9426_attr(WEXTCNTL),
	rt9426_attr(WSW1),
	rt9426_attr(WSW2),
	rt9426_attr(WSW3),
	rt9426_attr(WSW4),
	rt9426_attr(WSW5),
	rt9426_attr(WSW6),
	rt9426_attr(WSW7),
	rt9426_attr(WSW8),
	rt9426_attr(WTEMP),
	rt9426_attr(UNSEAL),
	rt9426_attr(FG_SET_TEMP),
	rt9426_attr(FG_RESET),
	rt9426_attr(CALIB_SECURE),
	rt9426_attr(CALIB_RSENSE),
	rt9426_attr(CALIB_CURRENT),
	rt9426_attr(CALIB_VOLTAGE),
	rt9426_attr(CALIB_FACTOR),
	rt9426_attr(FORCE_SHUTDOWN),
};

enum {
	FG_TEMP = 0,
	FG_VOLT,
	FG_CURR,
	FG_UPDATE,
	OCV_TABLE,
	ENTER_HIBERNATE,
	EXIT_HIBERNATE,
	SET_HIBERNATE_DUTY,
	DBP0,
	DBP1,
	DBP2,
	DBP3,
	DBP4,
	DBP5,
	DBP6,
	DBP7,
	DBP8,
	DBP9,
	WCNTL,
	WEXTCNTL,
	WSW1,
	WSW2,
	WSW3,
	WSW4,
	WSW5,
	WSW6,
	WSW7,
	WSW8,
	WTEMP,
	UNSEAL,
	FG_SET_TEMP,
	FG_RESET,
	CALIB_SECURE,
	CALIB_RSENSE,
	CALIB_CURRENT,
	CALIB_VOLTAGE,
	CALIB_FACTOR,
	FORCE_SHUTDOWN,
};

static ssize_t rt9426_fg_temp_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", chip->btemp);
}

static ssize_t rt9426_fg_volt_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", chip->bvolt);
}

static ssize_t rt9426_fg_curr_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", chip->bcurr);
}

static ssize_t rt9426_fg_base_value_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}

	switch (offset) {
	case FG_TEMP:
		chip->btemp = val;
		break;
	case FG_VOLT:
		chip->bvolt = val;
		break;
	case FG_CURR:
		chip->bcurr = val;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static ssize_t rt9426_fg_update_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	rt9426_update_info(chip);
	return 0;
}

static ssize_t rt9426_ocv_table_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	int temp[RT9426_OCV_COL_SIZE];
	int x;
	int y;

	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	for (x = 0; x < RT9426_OCV_ROW_SIZE; x++) {
		rt9426_reg_write_word(chip->i2c,
			RT9426_REG_EXTREGCNTL, 0xCA00 + x);
		rt9426_reg_write_word(chip->i2c,
			RT9426_REG_EXTREGCNTL, 0xCA00 + x);
		for (y = 0; y < RT9426_OCV_COL_SIZE; y++)
			temp[y] = rt9426_reg_read_word(chip->i2c,
				RT9426_REG_SWINDOW1 + y * 2);

		dev_info(dev, "fg_ocv_table_%d\n", x);
		dev_info(dev, "<0x%x 0x%x 0x%x 0x%x>\n",
			temp[0], temp[1], temp[2], temp[3]);
		dev_info(dev, "<0x%x 0x%x 0x%x 0x%x>\n",
			temp[4], temp[5], temp[6], temp[7]);
	}
	return 0;
}

static ssize_t rt9426_enter_hivernate_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	rt9426_enter_hibernate(chip);
	dev_info(dev, "SLP_STS = %d\n", rt9426_reg_read_word(
		chip->i2c, RT9426_REG_FLAG2) >> 15);
	return 0;
}

static ssize_t rt9426_exit_hibernate_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	rt9426_exit_hibernate(chip);
	dev_info(dev, "SLP_STS = %d\n", rt9426_reg_read_word(
		chip->i2c, RT9426_REG_FLAG2) >> 15);
	return 0;
}

static ssize_t rt9426_set_hibernate_duty_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	rt9426_hibernate_duty_set(chip, val);
	rt9426_hibernate_duty_read(chip);

	return 0;
}

static ssize_t rt9426_dbp_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	uint8_t page = offset - DBP0;
	int temp[RT9426_OCV_COL_SIZE];
	int x;

	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	rt9426_read_page_cmd(chip, page);
	if (offset == DBP9)
		usleep_range(5000, 5100); /* delay 5ms */
	for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
		temp[x] = rt9426_reg_read_word(chip->i2c,
			RT9426_REG_SWINDOW1 + x * 2);
	return 0;
}

static ssize_t rt9426_wcntl_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, val);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */

	return 0;
}

static ssize_t rt9426_wextcntl_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, val);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, val);

	return 0;
}

static ssize_t rt9426_wsw_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;
	uint8_t reg_offset = offset - WSW1;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1 + reg_offset * 2, val);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */

	return 0;
}

static ssize_t rt9426_unseal_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	int ret;

	if (rt9426_check_illegal_input(buf, 1)) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (ret < 0)
		return ret;

	if ((ret & RT9426_UNSEAL_STATUS) != 0) {
		dev_info(dev, "RT9426 Unseal Pass\n");
		return 0;
	}

	/* Unseal RT9426 */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
		(RT9426_UNSEAL_KEY & POWER_MASK_WORD)); /* get low 2 bytes */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
		(RT9426_UNSEAL_KEY >> 16)); /* get high 2 bytes */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */

	mdelay(10); /* delay 10ms */

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (ret < 0)
		return ret;
	if ((ret & RT9426_UNSEAL_STATUS) == 0)
		dev_info(dev, "RT9426 Unseal Fail\n");
	else
		dev_info(dev, "RT9426 Unseal Pass\n");
	
	return 0;
}

static ssize_t rt9426_fg_set_temp_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	long val = 0;

	if (get_parameters((char *)buf, &val, 1) < 0) {
		dev_notice(dev, "get paremters fail\n");
		return -EINVAL;
	}
	rt9426_temperature_set(chip, val);

	return 0;
}

static ssize_t rt9426_fg_reset_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	rt9426_reset(chip);
	return 0;
}

static ssize_t rt9426_calib_secure_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	return scnprintf(buf, PAGE_SIZE, chip->calib_unlock ? "Unlocked\n" : "Locked\n");
}

static ssize_t rt9426_calib_secure_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	int x = 0;

	if (kstrtoint(buf, 0, &x))
		return -EINVAL;
	chip->calib_unlock = (x == RT9426_CALIB_UNLOCK_KEY);

	return 0;
}

static ssize_t rt9426_calib_rsense_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	if (rt9426_check_illegal_input(buf, 1))
		return -EINVAL;

	if (!chip->calib_unlock)
		return -EACCES;
	rt9426_apply_sense_resistor(chip); /* set rsense according to dtsi */
	rt9426_apply_calibration_para(chip, 0x80, 0x80, 0x80);

	return 0;
}

static ssize_t rt9426_calib_current_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	int ret;

	if (!chip->calib_unlock)
		return -EACCES;
	rt9426_enter_calibration_mode(chip);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n", rt9426_get_curr_by_conversion(chip));
	rt9426_exit_calibration_mode(chip);
	return ret;
}

static ssize_t rt9426_calib_voltage_show(struct device *dev, struct rt9426_chip *chip,
	char *buf)
{
	int ret;

	if (!chip->calib_unlock)
		return -EACCES;
	rt9426_enter_calibration_mode(chip);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n", rt9426_get_volt_by_conversion(chip));
	rt9426_exit_calibration_mode(chip);
	return ret;
}

static ssize_t rt9426_calib_factor_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	int temp[RT9426_OCV_COL_SIZE];

	if (!chip->calib_unlock)
		return -EACCES;

	if (sscanf(buf, "%d,%d,%d", &temp[0], &temp[1], &temp[2]) != 3)
		return -EINVAL;

	rt9426_apply_calibration_para(chip, temp[0], temp[1], temp[2]);

	return 0;
}

static ssize_t rt9426_force_shutdown_store(struct device *dev, struct rt9426_chip *chip,
	const char *buf, const uint8_t offset)
{
	int x = -1;
	int ret;

	if (kstrtoint(buf, 0, &x) || x < 0 || x > 1)
		return -EINVAL;

	ret = x ? rt9426_enter_shutdown_mode(chip) : rt9426_exit_shutdown_mode(chip);
	return ret;
}

typedef ssize_t (*rt9426_dbg_show_action)(struct device *, struct rt9426_chip *,
		char *);
typedef ssize_t (*rt9426_dbg_store_action)(struct device *, struct rt9426_chip *,
		const char *, const uint8_t);
struct rt9426_dbg_struct {
	rt9426_dbg_show_action show_action;
	rt9426_dbg_store_action store_action;
};

static struct rt9426_dbg_struct g_rt9426_dbg_tbl[] = {
	[FG_TEMP] = { rt9426_fg_temp_show, rt9426_fg_base_value_store },
	[FG_VOLT] = { rt9426_fg_volt_show, rt9426_fg_base_value_store },
	[FG_CURR] = { rt9426_fg_curr_show, rt9426_fg_base_value_store },
	[FG_UPDATE] = { NULL, rt9426_fg_update_store },
	[OCV_TABLE] = { NULL, rt9426_ocv_table_store },
	[ENTER_HIBERNATE] = { NULL, rt9426_enter_hivernate_store },
	[EXIT_HIBERNATE] = { NULL, rt9426_exit_hibernate_store },
	[SET_HIBERNATE_DUTY] = { NULL, rt9426_set_hibernate_duty_store },
	[DBP0] = { NULL, rt9426_dbp_store },
	[DBP1] = { NULL, rt9426_dbp_store },
	[DBP2] = { NULL, rt9426_dbp_store },
	[DBP3] = { NULL, rt9426_dbp_store },
	[DBP4] = { NULL, rt9426_dbp_store },
	[DBP5] = { NULL, rt9426_dbp_store },
	[DBP6] = { NULL, rt9426_dbp_store },
	[DBP7] = { NULL, rt9426_dbp_store },
	[DBP8] = { NULL, rt9426_dbp_store },
	[DBP9] = { NULL, rt9426_dbp_store },
	[WCNTL] = { NULL, rt9426_wcntl_store },
	[WEXTCNTL] = { NULL, rt9426_wextcntl_store },
	[WSW1] = { NULL, rt9426_wsw_store },
	[WSW2] = { NULL, rt9426_wsw_store },
	[WSW3] = { NULL, rt9426_wsw_store },
	[WSW4] = { NULL, rt9426_wsw_store },
	[WSW5] = { NULL, rt9426_wsw_store },
	[WSW6] = { NULL, rt9426_wsw_store },
	[WSW7] = { NULL, rt9426_wsw_store },
	[WSW8] = { NULL, rt9426_wsw_store },
	[WTEMP] = { NULL, rt9426_fg_set_temp_store },
	[UNSEAL] = { NULL, rt9426_unseal_store },
	[FG_SET_TEMP] = { NULL, rt9426_fg_set_temp_store },
	[FG_RESET] = { NULL, rt9426_fg_reset_store },
	[CALIB_SECURE] = { rt9426_calib_secure_show, rt9426_calib_secure_store },
	[CALIB_RSENSE] = { NULL, rt9426_calib_rsense_store },
	[CALIB_CURRENT] = { rt9426_calib_current_show, NULL },
	[CALIB_VOLTAGE] = { rt9426_calib_voltage_show, NULL },
	[CALIB_FACTOR] = { NULL, rt9426_calib_factor_store },
	[FORCE_SHUTDOWN] = { NULL, rt9426_force_shutdown_store },
};

#define RT9426_DBG_ATTRS_SIZE   ARRAY_SIZE(g_rt9426_dbg_tbl)

static ssize_t rt9426_show_attrs(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);
	const ptrdiff_t offset = attr - rt9426_fuelgauge_attrs;

	if (!chip || !dev || offset >= RT9426_DBG_ATTRS_SIZE ||
		offset < 0 || !g_rt9426_dbg_tbl[offset].show_action)
		return -EINVAL;

	return g_rt9426_dbg_tbl[offset].show_action(dev, chip, buf);
}

static ssize_t rt9426_store_attrs(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);
	const ptrdiff_t offset = attr - rt9426_fuelgauge_attrs;
	int ret;

	if (!chip || !dev || offset >= RT9426_DBG_ATTRS_SIZE ||
		offset < 0 || !g_rt9426_dbg_tbl[offset].store_action)
		return -EINVAL;

	ret = g_rt9426_dbg_tbl[offset].store_action(dev, chip, buf, offset);
	return (ret == 0) ? count : ret;
}

static int rt9426_create_attrs(struct device *dev)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(rt9426_fuelgauge_attrs); i++) {
		rc = device_create_file(dev, &rt9426_fuelgauge_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	dev_notice(dev, "%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &rt9426_fuelgauge_attrs[i]);
create_attrs_succeed:
	return rc;
}

static int rt9426_irq_enable(struct rt9426_chip *chip, bool enable)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		mutex_lock(&chip->update_lock);
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2,
			enable ? chip->pdata->op_config[1] : 0);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		mdelay(5); /* delay 5ms */
		/* if disable, force clear irq status */
		if (!enable) {
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_IRQ);
			dev_info(chip->dev, "previous irq status 0x%04x\n", regval);
		}
		mutex_unlock(&chip->update_lock);
	}
	return 0;
}

static int rt9426_irq_deinit(struct rt9426_chip *chip)
{
	device_init_wakeup(chip->dev, false);
	return 0;
}

static bool rt9426_is_need_update_pdata(struct rt9426_chip *chip)
{
	int regval;
	int dc;
	int op_config[RT9426_OP_CONFIG_SIZE] = {0};

	/* 1st check version */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
	/* mask high bits */
	if ((regval & 0x7FFF) != chip->pdata->rt_config_ver)
		return true;
	/* 2nd check dc */
	dc = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	if (dc != chip->pdata->design_capacity)
		return true;
	/* 3rd check op config */
	if (rt9426_unseal_wi_retry(chip) != RT9426_UNSEAL_PASS)
		return false;

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);
	op_config[0] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
	op_config[1] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2);
	op_config[2] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	op_config[3] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	if (op_config[0] != chip->pdata->op_config[0] ||
		op_config[1] != chip->pdata->op_config[1] ||
		op_config[2] != chip->pdata->op_config[2] ||
		op_config[3] != chip->pdata->op_config[3])
		return true;

	return false;
}

static int rt9426_apply_pdata(struct rt9426_chip *chip)
{
	int i;
	int ret;
	int regval;
	int volt_now = 0;
	int fd_vth_now;
	int fd_threshold = 0;
	int retry_times_all;
	int fc_target1;
	int fc_target2;
	int fd_target1;
	int fd_target2;
	u8 target_curr_offs = 0;
	u8 target_curr_gain = 0;
	u8 target_volt_gain = 0;
	int op_config_reading[RT9426_OP_CONFIG_SIZE] = { 0 };
	int retry_times = 30; /* sometimes need retry */
	bool need_update = false;

	dev_info(chip->dev, "%s\n", __func__);
	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -EPERM;
		if (regval & RT9426_RDY_MASK) {
			rt9426_exit_shutdown_mode(chip);
			rt9426_get_ocv_checksum(chip);
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
			if (regval < 0)
				return -EPERM;
			if (!(regval & RT9426_RI_MASK)) {
				regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
				need_update = rt9426_is_need_update_pdata(chip);
				if (!need_update) { /* mask high bits */
					dev_info(chip->dev, "RI=0, bypass initial phase\n");
					goto out_apply;
				} else {
					dev_info(chip->dev, "force init due to DRIVER_VER\n");
					mdelay(60); /* delay 60ms */
					if ((regval & 0x7FFF) <= chip->pdata->need_reset_version) {
						rt9426_reset(chip);
						mdelay(1000); /* delay 1000ms */
					}
					break;
				}
			} else {
				dev_info(chip->dev, "initialization due to RI\n");
				mdelay(60); /* delay 60ms */
				break;
			}
		}
		mdelay(10); /* delay 10ms */
	}

	/* add 3 times retry for overall flow */
	retry_times_all = 3;

init_start:
	if (--retry_times_all < 0) {
		dev_err(chip->dev, "Fail for the retry times 3\n");
		return -EFAULT;
	}

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return -EINVAL;

	rt9426_reg_read_word(chip->i2c,	RT9426_REG_IRQ);
	/* set OCV Table */
	(void)rt9426_write_ocv_table(chip);

	rt9426_get_calibration_para(chip, &target_curr_offs, &target_curr_gain, &target_volt_gain);
	rt9426_apply_calibration_para(chip, target_curr_offs, target_curr_gain, target_volt_gain);
	/* set alert threshold */
	rt9426_write_page_cmd(chip, RT9426_PAGE_3);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, chip->pdata->otc_tth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2, chip->pdata->otc_chg_ith);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, chip->pdata->otd_tth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4, chip->pdata->otd_dchg_ith);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5, chip->pdata->uv_ov_threshold);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, (0x4600 | (chip->pdata->us_threshold))); /* set us mask */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	/* set OCV type */
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	if (chip->pdata->battery_type == 4400) { /* 4400: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8200);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8200\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4352) { /* 4352: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8100);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8100\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4354) { /* 4354: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8300);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8300\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4200) { /* 4200: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8000);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8000\n",
			__func__, chip->pdata->battery_type);
	} else {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8100);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8100\n",
			__func__, chip->pdata->battery_type);
	}

	/* set design capacity */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6,
		chip->pdata->design_capacity);

	/* set fcc */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		chip->pdata->fcc[chip->ocv_index]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
	if (ret != chip->pdata->design_capacity) {
		dev_info(chip->dev,
			"%s: design capacity mismatched\n", __func__);
		goto init_start;
	}
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
	dev_info(chip->dev, "%s: fcc setting = %d, fcc reading = %d\n",
		__func__, chip->pdata->fcc[chip->ocv_index], ret);
	if (ret != chip->pdata->fcc[chip->ocv_index]) {
		dev_info(chip->dev, "%s: fcc mismatched\n", __func__);
		goto init_start;
	}
	/* set all 5 op config */
	rt9426_write_page_cmd(chip, RT9426_PAGE_1);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1,
		chip->pdata->op_config[0]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2,
		chip->pdata->op_config[1]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3,
		chip->pdata->op_config[2]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4,
		chip->pdata->op_config[3]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5,
		chip->pdata->op_config[4]);
	/* set curr deadband */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		(0x0012 | (chip->pdata->curr_db << 8)));
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);
	/* read all 5 op_config for check */
	for (i = 0; i < RT9426_OP_CONFIG_IN_WINDOW1; i++)
		op_config_reading[i] =
			rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1 + (i * 2));

	/* compare OPCFG1 & show result */
	if (op_config_reading[0] != chip->pdata->op_config[0]) {
		dev_info(chip->dev, "%s: OPCFG1 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG1 matched\n", __func__);

		if (((op_config_reading[0] & 0x00C0) >> 6) == 0)
			dev_info(chip->dev, "Rsense(2.5)mohm\n");
		else if (((op_config_reading[0] & 0x00C0) >> 6) == 1)
			dev_info(chip->dev, "Rsense(5.0)mohm\n");
		else if (((op_config_reading[0] & 0x00C0) >> 6) == 2)
			dev_info(chip->dev, "Rsense(10)mohm\n");
		else
			dev_info(chip->dev, "Rsense(20)mohm\n");

		if (((op_config_reading[0] & 0xC000) >> 14) == 0)
			dev_info(chip->dev, "Temperature (NTC)\n");
		else if (((op_config_reading[0] & 0xC000) >> 14) == 1)
			dev_info(chip->dev, "Temperature (AP)\n");
		else
			dev_info(chip->dev, "Temperature (INT_Temp)\n");
	}

	/* compare OPCFG2 & show result */
	if (op_config_reading[1] != chip->pdata->op_config[1]) {
		dev_info(chip->dev, "%s: OPCFG2 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG2 matched\n", __func__);

		/* 0x4000: voltage alert IRQ mask */
		if (op_config_reading[1] & 0x4000)
			dev_info(chip->dev, "voltage alert IRQ enable\n");
		else
			dev_info(chip->dev, "voltage alert IRQ disable\n");

		if (((op_config_reading[1] & 0x0080) >> 7) == 1)
			dev_info(chip->dev, "Auto_Sleep (Enable)\n");
		else
			dev_info(chip->dev, "Auto_Sleep (Disable)\n");

		if (((op_config_reading[1] & 0x0040) >> 6) == 1)
			dev_info(chip->dev, "Sleep_Mode (Enable)\n");
		else
			dev_info(chip->dev, "Sleep_Mode (Disable)\n");

		if (((op_config_reading[1] & 0x0020) >> 5) == 1)
			dev_info(chip->dev, "Shutdown_Mode (Enable)\n");
		else
			dev_info(chip->dev, "Shutdown_Mode (Disable)\n");

		if (((op_config_reading[1] & 0x0001)) == 1)
			dev_info(chip->dev, "Battery_Detection (Enable)\n");
		else
			dev_info(chip->dev, "Battery_Detection (Disable)\n");

		if (((op_config_reading[1] & 0x2000) >> 13) == 1)
			dev_info(chip->dev, "SOC_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "SOC_IRQ (Disable)\n");

		if (((op_config_reading[1] & 0x1000) >> 12) == 1)
			dev_info(chip->dev, "Battery_Det_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "Battery_Det_IRQ (Disable)\n");

		if (((op_config_reading[1] & 0x0200) >> 9) == 1)
			dev_info(chip->dev, "SC_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "SC_IRQ (Disable)\n");
	}

	/* compare OPCFG3 & show result */
	if (op_config_reading[2] != chip->pdata->op_config[2]) {
		dev_info(chip->dev, "%s: OPCFG3 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG3 matched\n", __func__);

		if (((op_config_reading[2] & 0x00E0) >> 4) == 0xE)
			dev_info(chip->dev, "FC+FD+RLX_Det (Enable)\n");
		else
			dev_info(chip->dev, "FC+FD+RLX_Det (Disable)\n");

		if (((op_config_reading[2] & 0x0018) >> 3) == 0x3)
			dev_info(chip->dev, "TLCOMP (Enable)\n");
		else
			dev_info(chip->dev, "TLCOMP (Disable)\n");

		if (((op_config_reading[2] & 0x0007)) == 0x7) /* set bit[2-0] */
			dev_info(chip->dev, "BCCOMP (Enable)\n");
		else
			dev_info(chip->dev, "BCCOMP (Disable)\n");

		if (((op_config_reading[2] & 0x0200) >> 9) == 1)
			dev_info(chip->dev, "FC_LOCK (Enable)\n");
		else
			dev_info(chip->dev, "FC_LOCK (Disable)\n");
	}
	/* compare OPCFG4 & show result */
	if (op_config_reading[3] != chip->pdata->op_config[3]) {
		dev_info(chip->dev, "%s: OPCFG4 mismatched\n", __func__);
		goto init_start;
	}

	/* compare OPCFG5 & show result */
	if (op_config_reading[4] != chip->pdata->op_config[4]) {
		dev_info(chip->dev, "%s: OPCFG5 mismatched\n", __func__);
		goto init_start;
	}

	/* set fc_vth + fc_ith */
	fc_target1 = (chip->pdata->fc_vth[chip->ocv_index]) |
		(chip->pdata->fc_ith << 8);
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, fc_target1);

	/* set fc_sth */
	fc_target2 = 0x4100 | (chip->pdata->fc_sth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4, fc_target2);

	/* set fd_vth */
	ret = bat_model_get_bat_cathode_type();
	if (ret == BAT_MODEL_BAT_CATHODE_TYPE_SILICON) {
		rt9426_write_tl_table(chip);
		rt9426_read_page_cmd(chip, RT9426_PAGE_5);
		fd_target2 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW8);
		fd_target2 = (chip->pdata->fd_spd_vth[chip->ocv_index]) |
			(fd_target2 & POWER_MASK_HIGH_BYTE);
		rt9426_write_page_cmd(chip, RT9426_PAGE_5);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, fd_target2);
	}
	fd_target1 = 0x1200 | (chip->pdata->fd_vth[chip->ocv_index]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, fd_target1);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	usleep_range(5000, 5100); /* delay 5ms */
	/* add read back check for fc_vth/fc_ith/fc_sth/fd_vth/fd_spd_vth */
	rt9426_read_page_cmd(chip, RT9426_PAGE_5);

	/* check fc_vth & fc_ith */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	if (ret != fc_target1) {
		dev_info(chip->dev, "%s: fc_vth & fc_ith mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}

	/* check fd_spd_vth */
	ret = bat_model_get_bat_cathode_type();
	if (ret == BAT_MODEL_BAT_CATHODE_TYPE_SILICON) {
		ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW8);
		dev_info(chip->dev, "%s: fd_spd_vth (%d,%d)\n", __func__, ret, fd_target2);
	}

	/* check fc_sth */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	if (ret != fc_target2) {
		dev_info(chip->dev, "%s: fc_sth mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}

	/* check fd_vth */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
	if (ret != fd_target1) {
		dev_info(chip->dev, "%s: fd_vth mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}
	/* set EXTREG */
	ret = chip->pdata->extreg_size;
	for (i = 0; i < ret; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6550 +
		(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6550 +
		(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		usleep_range(1000, 1100); /* delay 1ms */
		rt9426_reg_write_word(chip->i2c,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_addr,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_data);
		usleep_range(5000, 5100); /* delay 5ms */
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	/* check EXTREG: 0x6500 for write page cmd */
	ret = chip->pdata->extreg_size;
	for (i = 0; i < ret; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6500 +
			(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6500 +
			(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		usleep_range(5000, 5100); /* delay 5ms */
		regval = rt9426_reg_read_word(chip->i2c,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_addr);
		/* check the readback value */
		if (regval != chip->pdata->extreg_update.extreg_update_data[i].extreg_data) {
			dev_info(chip->dev, "%s: data[%d] mismatched\n", __func__, i);
			/* add retry after read back check failed */
			goto init_start;
		}
	}

	if (chip->ocv_checksum_ic != chip->ocv_checksum_dtsi) {
		if (rt9426_get_current(chip) > 0) {
			volt_now = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
			fd_vth_now = chip->pdata->fd_vth[chip->ocv_index];
			fd_threshold = RT9426_FD_BASE + 5 * (fd_vth_now);

			if (volt_now > fd_threshold) {
				/* disable battery charging path before QS command */
				rt9426_request_charging_inhibit(true);
				dev_info(chip->dev, "Enable Charging Inhibit and delay 1250ms\n");
				mdelay(1250);
			}
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x4000);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(5);
		dev_info(chip->dev, "OCV checksum are different, QS is done\n");
		/* Power path recover check */
		if (volt_now > fd_threshold) {
			/* enable battery charging path after QS command */
			rt9426_request_charging_inhibit(false);
			dev_info(chip->dev, "Disable Charging Inhibit\n");
		}
	} else {
		dev_info(chip->dev, "OCV checksum are the same, bypass QS\n");
	}
	/* clear RI, set 0 to RI bits */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (regval < 0)
		return -EPERM;
	regval = (regval & ~RT9426_RI_MASK);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_FLAG3, regval);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (regval < 0)
		return -EPERM;
	if (((regval & RT9426_RI_MASK) >> 8) == 0)
		dev_info(chip->dev, "RT9426 RI=0\n");
	else
		dev_info(chip->dev, "RT9426 RI=1\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
	if (regval < 0)
		return -EPERM;
	if ((regval & 0x7FFF) != chip->pdata->rt_config_ver) {
		regval = (regval & 0x8000) | chip->pdata->rt_config_ver;
		rt9426_reg_write_word(chip->i2c, RT9426_REG_RSVD_FLAG, regval);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	}

	/* update setting of rsense location if it's necessary; */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_DEVICE_ID);
	if (regval == 0x426A) {
		rt9426_write_page_cmd(chip, RT9426_PAGE_11);  /* Write Page11 */
		/* if it' s rt9426A, turn off the default advanced self-learning function */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, 0x290B);

		if (chip->pdata->rs_high_side == 1) {
			/* if it's rt9426A & high side rsense is used, then set opcfg6[7] = 1 */
			rt9426_write_page_cmd(chip, RT9426_PAGE_10);
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, (regval | 0x0080));
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
			mdelay(5);
		}
	}

out_apply:
	chip->online = 1;
	return 0;
}

struct dt_offset_params {
	int data[RT9426_DT_OFFSET_PARA_SIZE];
};

struct dt_extreg_params {
	int edata[RT9426_DT_EXTREG_PARA_SIZE];
};

static struct device_node *rt9426_get_child_node(struct device *dev,
	struct rt9426_platform_data *pdata)
{
	const char *battery_name = NULL;
	const char *batt_model_name = NULL;
	struct device_node *np = dev->of_node;
	struct device_node *child_node = NULL;
	struct device_node *default_node = NULL;

	batt_model_name = bat_model_name();
	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG),
			child_node, "batt_name", &battery_name)) {
			dev_info(dev, "childnode without batt_name property");
			continue;
		}
		if (!battery_name)
			continue;
		if (!default_node)
			default_node = child_node;
		dev_info(dev, "search battery data, battery_name: %s\n", battery_name);
		if (!batt_model_name || !strcmp(battery_name, batt_model_name))
			break;
	}

	if (!child_node) {
		if (default_node) {
			dev_info(dev, "cannt match childnode, use first\n");
			child_node = default_node;
		} else {
			dev_info(dev, "cannt find any childnode, use father\n");
			child_node = np;
		}
	}

	return child_node;
}

static void rt9426_parse_source(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,volt_source", &pdata->volt_source, RT9426_REG_AV);

	if (pdata->volt_source == RT9426_VOLT_SOURCE_NONE) {
		pdata->volt_source = 0;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_VBAT) {
		pdata->volt_source = RT9426_REG_VBAT;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_OCV) {
		pdata->volt_source = RT9426_REG_OCV;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_AV) {
		pdata->volt_source = RT9426_REG_AV;
	} else {
		dev_notice(dev, "pdata->volt_source is out of range, use 3\n");
		pdata->volt_source = RT9426_REG_AV;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,temp_source", &pdata->temp_source, 0);

	if (pdata->temp_source == RT9426_TEMP_SOURCE_NONE)
		pdata->temp_source = 0;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_TEMP)
		pdata->temp_source = RT9426_REG_TEMP;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_INIT)
		pdata->temp_source = RT9426_REG_INTT;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_AT)
		pdata->temp_source = RT9426_REG_AT;
	else
		pdata->temp_source = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,curr_source", &pdata->curr_source, 0);

	if (pdata->curr_source == RT9426_CURR_SOURCE_NONE)
		pdata->curr_source = 0;
	else if (pdata->curr_source == RT9426_CURR_SOURCE_CURR)
		pdata->curr_source = RT9426_REG_CURR;
	else if (pdata->curr_source == RT9426_CURR_SOURCE_AI)
		pdata->curr_source = RT9426_REG_AI;
	else
		pdata->curr_source = RT9426_REG_AI;
}

static void rt9426_parse_threshold(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otc_tth", &pdata->otc_tth, RT9426_OTC_TTH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otc_chg_ith", &pdata->otc_chg_ith, RT9426_OTC_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otd_tth", &pdata->otd_tth, RT9426_OTD_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otd_dchg_ith", &pdata->otd_dchg_ith, RT9426_OTD_DCHG_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_uvov_threshold", &pdata->uv_ov_threshold, RT9426_UV_OV_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_us_threshold", &pdata->us_threshold, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_ith", &pdata->fc_ith, RT9426_FC_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_sth", &pdata->fc_sth, RT9426_FC_STH_DEFAULT_VAL);
}

static int rt9426_parse_soc_offset(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	struct dt_offset_params *offset_params = NULL;
	int sizes[RT9426_SOC_OFFSET_SIZE + 1] = { 0 }; /* 1:for boundary protection */

	ret = of_property_read_u32_array(np, "rt,offset_interpolation_order",
		pdata->offset_interpolation_order, RT9426_OFFSET_INTERPLO_SIZE);
	if (ret < 0)
		pdata->offset_interpolation_order[0] =
			pdata->offset_interpolation_order[1] = 2; /* 2: interpolation order */

	sizes[0] = sizes[1] = 0;
	ret = of_property_read_u32_array(np, "rt,soc_offset_size",
		sizes, RT9426_SOC_OFFSET_SIZE);
	if (ret < 0)
		dev_notice(dev, "cannt get prop soc_offset_size %d\n", ret);

	new_vgcomp_soc_offset_datas(dev, SOC_OFFSET, pdata, sizes[0], sizes[1], 0);
	if (!pdata->soc_offset.soc_offset_data)
		return 0;

	offset_params = devm_kzalloc(dev,
		sizes[0] * sizes[1] * sizeof(struct dt_offset_params), GFP_KERNEL);
	if (!offset_params)
		return -EPERM;

	of_property_read_u32_array(np, "rt,soc_offset_data",
		(u32 *)offset_params,
		sizes[0] * sizes[1] * (RT9426_SOC_OFFSET_SIZE + 1));
	for (i = 0; i < sizes[0] * sizes[1]; i++) {
		pdata->soc_offset.soc_offset_data[i].x =
			offset_params[i].data[0];
		pdata->soc_offset.soc_offset_data[i].y =
			offset_params[i].data[1];
		pdata->soc_offset.soc_offset_data[i].offset =
			offset_params[i].data[2];
	}
	devm_kfree(dev, offset_params);

	return 0;
}

static int rt9426_parse_extreg(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	struct dt_extreg_params *extreg_params = NULL;

	ret = of_property_read_u32(np, "rt,extreg_size", &pdata->extreg_size);
	if (ret < 0)
		dev_notice(dev, "cannt get prop extreg_size %d\n", ret);

	new_vgcomp_soc_offset_datas(dev, EXTREG_UPDATE, pdata, pdata->extreg_size, 0, 0);
	if (!pdata->extreg_update.extreg_update_data)
		return 0;

	extreg_params = devm_kzalloc(dev,
		pdata->extreg_size * sizeof(struct dt_extreg_params), GFP_KERNEL);
	if (!extreg_params)
		return -EPERM;

	of_property_read_u32_array(np, "rt,extreg_data",
		(u32 *)extreg_params, pdata->extreg_size * 3);
	for (i = 0; i < pdata->extreg_size;  ++i) {
		pdata->extreg_update.extreg_update_data[i].extreg_page =
			extreg_params[i].edata[0];
		pdata->extreg_update.extreg_update_data[i].extreg_addr =
			extreg_params[i].edata[1];
		pdata->extreg_update.extreg_update_data[i].extreg_data =
			extreg_params[i].edata[2];
	}
	devm_kfree(dev, extreg_params);

	return 0;
}

static void rt9426_parse_ocv_table(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	char prop_name[RT9426_WRITE_BUF_LEN + 1] = { 0 };

	if (pdata->ocv_vterm_size > 1)
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			"rt,fg_ocv_table0",
			(u32 *)pdata->ocv_table, RT9426_OCV_DATA_TOTAL_SIZE);
	else
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			"rt,fg_ocv_table",
			(u32 *)pdata->ocv_table, RT9426_OCV_DATA_TOTAL_SIZE);
	if (ret < 0) {
		memset32((u32 *)pdata->ocv_table, 0, RT9426_OCV_DATA_TOTAL_SIZE);
		return;
	}
	for (i = 1; i < pdata->ocv_vterm_size; i++) {
		snprintf(prop_name, RT9426_WRITE_BUF_LEN, "rt,fg_ocv_table%d", i);
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			prop_name, (u32 *)pdata->ocv_table + i *
			RT9426_OCV_DATA_TOTAL_SIZE, RT9426_OCV_DATA_TOTAL_SIZE);
		if (ret < 0)
			memset32((u32 *)pdata->ocv_table + i *
				RT9426_OCV_DATA_TOTAL_SIZE, 0,
				RT9426_OCV_DATA_TOTAL_SIZE);
	}
}

static void rt9426_parse_tl_table(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	char prop_name[RT9426_WRITE_BUF_LEN + 1] = { 0 };

	if (pdata->ocv_vterm_size == 1) {
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			"rt,fg_tl_table", (u32 *)pdata->tl_table, RT9426_TL_DATA_TOTAL_SIZE);
		if (ret < 0)
			memset32((u32 *)pdata->tl_table, 0, RT9426_TL_DATA_TOTAL_SIZE);

		return;
	}

	for (i = 0; i < pdata->ocv_vterm_size; i++) {
		snprintf(prop_name, RT9426_WRITE_BUF_LEN, "rt,fg_ocv_table%d", i);
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			prop_name, (u32 *)pdata->tl_table + i *
			RT9426_TL_DATA_TOTAL_SIZE, RT9426_TL_DATA_TOTAL_SIZE);
		if (ret < 0)
			memset32((u32 *)pdata->tl_table + i * RT9426_TL_DATA_TOTAL_SIZE, 0,
				RT9426_TL_DATA_TOTAL_SIZE);
	}
}

static void rt9426_parse_op_config(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int ret;

	ret = of_property_read_u32_array(np, "rt,fg_op_config",
		(u32 *)pdata->op_config, RT9426_OP_CONFIG_SIZE);
	if (ret < 0) {
		dev_notice(dev, "no fg op config proeprty, use default\n");
		pdata->op_config[0] = RT9426_OP_CONFIG_0_DEFAULT_VAL;
		pdata->op_config[1] = RT9426_OP_CONFIG_1_DEFAULT_VAL;
		pdata->op_config[2] = RT9426_OP_CONFIG_2_DEFAULT_VAL;
		pdata->op_config[3] = RT9426_OP_CONFIG_3_DEFAULT_VAL;
		pdata->op_config[4] = RT9426_OP_CONFIG_4_DEFAULT_VAL;
	}
}

static void rt9426_parse_iterm_setting(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,ffc_buck_para_A",
		&pdata->ffc_buck_para_A, RT9426_FFC_BUCK_PARA_A);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,ffc_buck_para_B",
		&pdata->ffc_buck_para_B, RT9426_FFC_BUCK_PARA_B);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,ffc_dc_para_A",
		&pdata->ffc_dc_para_A, RT9426_FFC_DC_PARA_A);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,ffc_dc_para_B",
		&pdata->ffc_dc_para_B, RT9426_FFC_DC_PARA_B);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,non_ffc_para_A",
		&pdata->non_ffc_para_A, RT9426_NON_FFC_PARA_A);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,non_ffc_para_B",
		&pdata->non_ffc_para_B, RT9426_NON_FFC_PARA_B);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fg_fc_low_curr_cnt_th",
		&pdata->fc_low_curr_cnt_th, RT9426_FC_LOW_CURR_CNT_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fg_iterm_ratio",
		&pdata->iterm_ratio, RT9426_ITERM_RATIO_BASE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fg_fc_low_curr_idelta",
		&pdata->fc_low_curr_idelta, RT9426_FC_LOW_CURR_IDELTA);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fg_fc_low_curr_vdelta",
		&pdata->fc_low_curr_vdelta, RT9426_FC_LOW_CURR_VDELTA);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fc_delta_ith_for_ffc_spd",
		&pdata->fc_delta_ith_for_ffc_spd, RT9426_FC_DELTA_ITH_FOR_FFC_SPD);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fc_delta_ith_for_non_ffc_spd",
		&pdata->fc_delta_ith_for_non_ffc_spd, RT9426_FC_DELTA_ITH_FOR_NON_FFC_SPD);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fc_delta_ith_for_spd_th",
		&pdata->fc_delta_ith_for_spd_th, RT9426_FC_DELTA_ITH_FOR_SPD_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,dynamic_change_iterm_en",
		&pdata->dynamic_change_iterm_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fcc_lr_adjust_en",
		&pdata->fcc_lr_adjust_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fcc_lr_cycle_th",
		&pdata->fcc_lr_cycle_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt,fcc_lr_ratio_th",
		&pdata->fcc_lr_ratio_th, 0);
}

static void rt9426_parse_fc_setting(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int ret;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_vth", pdata->fc_vth, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fc_vth property, use default 4200mV\n");
		memset32(pdata->fc_vth, RT9426_FC_VTH_DEFAULT_VAL,
			pdata->ocv_vterm_size);
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_low_curr_vth", pdata->fc_low_curr_vth, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fc_low_curr_vth property, use default\n");
		memset32(pdata->fc_low_curr_vth, RT9426_FC_LOW_CURR_VTH,
			pdata->ocv_vterm_size);
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_low_curr_vth_non_ffc", pdata->fc_low_curr_vth_non_ffc, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fc_low_curr_vth_non_ffc property, use default\n");
		memset32(pdata->fc_low_curr_vth_non_ffc, RT9426_FC_LOW_CURR_VTH,
			pdata->ocv_vterm_size);
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_non_ffc_fc_vth", pdata->non_ffc_fc_vth, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no non_ffc_fc_vth property, use default 4200mV\n");
		memset32(pdata->non_ffc_fc_vth, RT9426_FC_VTH_DEFAULT_VAL,
			pdata->ocv_vterm_size);
	}
}

/*
 * parse the props with flexible len, 1 or sz
 */
static void rt9426_parse_flexible_u32_array(struct device *dev, struct device_node *np,
	u32 *arr, u32 sz, u32 def, const char *prop)
{
	int ret;
	u32 tmp = def;

	ret = of_property_count_u32_elems(np, prop);
	if (ret == 1) {
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, prop, &tmp, def);
		memset32(arr, tmp, sz);
		return;
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np, prop, arr, sz);
	if (ret < 0) {
		dev_notice(dev, "no %s, use default %d\n", prop, def);
		memset32(arr, def, sz);
	}
}

static void rt9426_parse_fd_setting(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	rt9426_parse_flexible_u32_array(dev, np, pdata->fd_vth, pdata->ocv_vterm_size,
		RT9426_FD_VTH_DEFAULT_VAL, "rt,fg_fd_vth");
	rt9426_parse_flexible_u32_array(dev, np, pdata->fd_spd_vth, pdata->ocv_vterm_size,
		RT9426_FD_SPD_VTH_DEFAULT_VAL, "rt,fg_fd_spd_vth");

	/*
	 * config fd_th may conflict with the logics
	 * of dynamically changeing edv, pls be careful
	 */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,dnm_cfg_edv_disable", &pdata->dnm_cfg_edv_disable, 0);
}

static int rt9426_parse_base(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int ret;
	int i;

	ret = of_property_count_u32_elems(np, "rt,fcc");
	if (ret < 1)
		ret = 1;
	pdata->ocv_vterm_size = ret;
	if (pdata->ocv_vterm_size > RT9426_MAX_VTERM_SIZE) {
		dev_notice(dev, "the number of arrays is invalid\n");
		return -EINVAL;
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,fcc", pdata->fcc, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fcc property, use default 2000\n");
		memset32(pdata->fcc, RT9426_DESIGN_FCC_VAL,
			pdata->ocv_vterm_size);
	}

	ret = of_property_read_u32_array(np, "rt,dtsi_version",
		pdata->dtsi_version, RT9426_DTSI_VER_SIZE);
	if (ret < 0)
		pdata->dtsi_version[0] = pdata->dtsi_version[1] = 0;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"rt,vterm_th_arr", pdata->vterm_th_arr, pdata->ocv_vterm_size - 1);
	if (ret < 0) {
		dev_notice(dev, "no vterm_th_arr, use default 0,50,100,150,...\n");
		for (i = 0; i < pdata->ocv_vterm_size - 1; ++i)
			pdata->vterm_th_arr[i] = RT9426_VTERM_INCREASE * i;
	}

	of_property_read_u32(np, "rt,rs_ic_setting", &pdata->rs_ic_setting);
	of_property_read_u32(np, "rt,rs_schematic", &pdata->rs_schematic);
	dev_info(dev, "rs_ic_setting = %d\n", pdata->rs_ic_setting);
	dev_info(dev, "rs_schematic = %d\n", pdata->rs_schematic);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,battery_type", &pdata->battery_type, 4352); /* 4352: default battery type */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,design_capacity", &pdata->design_capacity, RT9426_DESIGN_CAP_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_curr_db", &pdata->curr_db, RT9426_CURR_DB_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,rs_high_side", &pdata->rs_high_side, 0); /* 0: low-side(default) */

	return 0;
}

static int rt9426_parse_sub_param(struct device *dev,
	struct rt9426_platform_data *pdata)
{
	int ret;
	struct device_node *child_node = rt9426_get_child_node(dev, pdata);

	ret = rt9426_parse_base(dev, child_node, pdata);
	if (ret)
		return ret;
	rt9426_parse_threshold(dev, child_node, pdata);
	rt9426_parse_source(dev, child_node, pdata);
	rt9426_parse_ocv_table(dev, child_node, pdata);
	rt9426_parse_tl_table(dev, child_node, pdata);
	rt9426_parse_op_config(dev, child_node, pdata);
	rt9426_parse_iterm_setting(dev, child_node, pdata);
	rt9426_parse_fc_setting(dev, child_node, pdata);
	rt9426_parse_fd_setting(dev, child_node, pdata);
	rt9426_parse_soc_offset(dev, child_node, pdata);
	rt9426_parse_extreg(dev, child_node, pdata);

	return ret;
}

static int rt9426_parse_dt(struct device *dev, struct rt9426_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	const char *dts_batt_name = NULL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "disable_under_vol_irq",
		&pdata->disable_under_vol_irq, 0); /* 0: enable irq by default */
	dev_info(dev, "disable_under_vol_irq is %d\n", pdata->disable_under_vol_irq);

	if (!pdata->disable_under_vol_irq) {
		pdata->intr_gpio = of_get_named_gpio(np, "rt9426,gpio-intb", 0);
		if (!gpio_is_valid(pdata->intr_gpio)) {
			dev_info(dev, "get intr_gpio fail\n");
			return -EPERM;
		}
		dev_info(dev, "get intr_gpio %d\n", pdata->intr_gpio);
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt_config_ver",
		&pdata->rt_config_ver, RT9426_DRIVER_VER);

	pdata->force_use_aux_cali_para =
		of_property_read_bool(np, "force_use_aux_cali_para");

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "lowtemp_en",
		&pdata->lowtemp_en, 0); /* 0: default lowtemp is closed */
	(void)power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np,
		"lowtemp_t_thr", &pdata->lowtemp_t_thr, RT9426_LOWTEMP_T_THR);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"lowtemp_edv_thr", &pdata->lowtemp_edv_thr, RT9426_LOWTEMP_EDV_THR);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		&pdata->ic_role, RT9426_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "need_reset_version",
		&pdata->need_reset_version, 4); /* 4 default value */
	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np, "rt,bat_name",
		&dts_batt_name))
		strncpy(pdata->batt_name, "rt9426", RT9426_BATT_NAME_LEN - 1);
	else
		strncpy(pdata->batt_name, dts_batt_name, RT9426_BATT_NAME_LEN - 1);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_en",
		&pdata->ground_loop_comp_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_vpullup",
		&pdata->glc_data.vpullup, 1200); /* default pull-up voltage 1200 mV */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_rpullup",
		&pdata->glc_data.rpullup, 10000); /* default pull-up r 10 KOhm */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_rcomp",
		&pdata->glc_data.rcomp, 30); /* default compensate r 3.0 mOhm */

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ir_comp_en",
		&pdata->ir_comp_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "compensation_r",
		&pdata->compensation_r, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "cache_reg_temp_en",
		&pdata->cache_reg_temp_en, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "platform_support",
		&pdata->platform_support, 0); /* 0 default qcom */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "connect_type",
		&pdata->connect_type, 0); /* 0 default 1s1p */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "vbat_max",
		&pdata->vbat_max, 4400); /* vbat_max default 4400mV */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "adc_sample_type",
		&pdata->adc_sample_type, 0);

	return rt9426_parse_sub_param(dev, pdata);
}

static int rt9426_i2c_chipid_check(struct i2c_client *i2c)
{
	u16 ver = 0;
	int ret;

	ret = i2c_smbus_read_i2c_block_data(i2c, RT9426_REG_VER, 2, (u8 *)&ver);
	if (ret < 0)
		return ret;
	ver = le16_to_cpu(ver);
	if ((ver & 0xff00) != 0x0000) { /* get version */
		dev_notice(&i2c->dev, "chip id not match\n");
		return -ENODEV;
	}
	return ver;
}

static int rt9426_irq_register(struct rt9426_chip *chip)
{
	int ret;

	ret = devm_gpio_request_one(chip->dev, chip->pdata->intr_gpio,
		GPIOF_IN, "rt9426_irq_gpio");
	if (ret < 0) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_to_irq(chip->pdata->intr_gpio);
	if (ret < 0) {
		hwlog_err("irq mapping fail\n");
		return ret;
	}
	chip->alert_irq = ret;
	hwlog_info("irq = %d\n", chip->alert_irq);

	/* Request threaded IRQ */
	ret = devm_request_threaded_irq(chip->dev, chip->alert_irq, NULL,
		rt9426_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"rt9246_irq", chip);
	if (ret < 0) {
		hwlog_err("request thread irq fail\n");
		gpio_free(chip->pdata->intr_gpio);
		return ret;
	}
	device_init_wakeup(chip->dev, true);

	return 0;
}

static int rt9426_is_ready(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return 1;
}

static int rt9426_read_battery_temperature(void *dev_data)
{
	if (!dev_data)
		return 0;

	return rt9426_get_display_data(dev_data, RT9426_DISPLAY_TEMP);
}

#ifdef CONFIG_HLTHERM_RUNTEST
static int rt9426_is_battery_exist(void *dev_data)
{
	return 0;
}
#else
static int rt9426_is_battery_exist(void *dev_data)
{
	int temp;

	temp = rt9426_read_battery_temperature(dev_data);
	if ((temp <= RT9426_TEMP_ABR_LOW) || (temp >= RT9426_TEMP_ABR_HIGH)) {
		hwlog_err("battery not exist, temp = %d\n", temp);
		return 0;
	}

	return 1;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int rt9426_read_battery_soc(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_SOC);
}

static int rt9426_read_battery_vol(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;
	int cur, vol;

	if (!chip)
		return 0;

	vol = rt9426_get_display_data(chip, RT9426_DISPLAY_VBAT);
	if (chip->pdata->ir_comp_en) {
		cur = rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
		vol -= cur * chip->pdata->compensation_r / POWER_UV_PER_MV;
		dev_info(chip->dev, "vbatt_comp=%d, cur=%d, compr=%d\n",
			vol, cur, chip->pdata->compensation_r);
	}

	return vol;
}

static int rt9426_read_battery_current(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
}

static int rt9426_read_battery_avg_current(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_AVG_IBAT);
}

static int rt9426_read_battery_fcc(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_FCC);
}

static int rt9426_read_battery_cycle(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_cyccnt(chip);
}

static int rt9426_read_battery_rm(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_RM);
}

static int rt9426_read_battery_charge_counter(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_RM);
}

static int rt9426_set_battery_low_voltage(int val, void *dev_data)
{
	int uv_set;
	int reg_val;
	u16 wr_val;
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	/* 2400: val = 2400mV + 10mV* uv_set */
	uv_set = (val - 2400) / 10;
	if (uv_set < 0)
		return -EPERM;

	mutex_lock(&chip->update_lock);
	rt9426_read_page_cmd(chip, RT9426_PAGE_3);
	reg_val = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	if (reg_val < 0) {
		mutex_unlock(&chip->update_lock);
		return -EPERM;
	}
	wr_val = ((unsigned int)uv_set << RT9426_BYTE_BITS) & RT9426_HIGH_BYTE_MASK;
	wr_val |= (unsigned int)reg_val & RT9426_LOW_BYTE_MASK;

	dev_notice(chip->dev, "uv_set=0x%x, reg_val=0x%x, wr_val=0x%x\n",
		uv_set, reg_val, wr_val);
	rt9426_write_page_cmd(chip, RT9426_PAGE_3);
	reg_val = rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5, wr_val);
	mutex_unlock(&chip->update_lock);
	return reg_val;
}

static int rt9426_set_last_capacity(int capacity, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	if ((capacity > RT9426_FULL_CAPCACITY) || (capacity < 0) || !chip)
		return 0;

	return rt9426_reg_write_word(chip->i2c, RT9426_EXTEND_REG, capacity);
}

static int rt9426_get_last_capacity(void *dev_data)
{
	int last_cap = 0;
	int cap;
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return last_cap;

	last_cap = rt9426_reg_read_word(chip->i2c, RT9426_EXTEND_REG);
	cap = rt9426_read_battery_soc(dev_data);

	dev_notice(chip->dev, "%s read cap=%d, last_cap=%d\n",
		__func__, cap, last_cap);

	if ((last_cap <= 0) || (cap <= 0))
		return cap;

	if (abs(last_cap - cap) >= RT9426_CAPACITY_TH)
		return cap;

	/* reset last capacity */
	rt9426_reg_write_word(chip->i2c, RT9426_EXTEND_REG, 0);

	return last_cap;
}

static int rt9426_set_vterm_dec(int vterm, void *dev_data)
{
	int ocv_index;
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	for (ocv_index = 0; ocv_index < chip->pdata->ocv_vterm_size - 1; ++ocv_index) {
		if (vterm <= chip->pdata->vterm_th_arr[ocv_index])
			break;
	}

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return -EIO;

	if (bat_model_get_bat_cathode_type() == BAT_MODEL_BAT_CATHODE_TYPE_SILICON)
		return rt9426_set_si_ocv_table(chip, ocv_index);

	return rt9426_set_ocv_table(chip, ocv_index);
}

static const char *rt9426_get_coul_model(void *dev_data)
{
	return "rt94xxx";
}

#ifdef CONFIG_COUL_DRV
static int rt9426_get_battery_voltage_uv(void *dev_data)
{
	return rt9426_read_battery_vol(dev_data) * POWER_UV_PER_MV;
}

static int rt9426_get_battery_current(void *dev_data)
{
	return rt9426_read_battery_current(dev_data);
}

static int rt9426_get_battery_avg_current(void *dev_data)
{
	return rt9426_read_battery_avg_current(dev_data);
}

static int rt9426_get_battery_resistance(void *dev_data)
{
	return 0;
}

static int rt9426_get_battery_fcc_design(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip || !chip->pdata || !chip->pdata->rs_schematic)
		return 0;

	return chip->pdata->design_capacity * chip->pdata->rs_ic_setting /
		chip->pdata->rs_schematic;
}

static int rt9426_get_battery_temperature(void *dev_data)
{
	return rt9426_read_battery_temperature(dev_data) / 10;
}

static int rt9426_get_battery_health(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;
	int health;

	if (!chip)
		return POWER_SUPPLY_HEALTH_UNKNOWN;

	if (chip->btemp == RT9426_TEMP_UNKNOWN) {
		health = POWER_SUPPLY_HEALTH_UNKNOWN;
	} else if (chip->btemp < RT9426_COLD_TEMP) {
		health = POWER_SUPPLY_HEALTH_COLD;
	} else if (chip->btemp > RT9426_OVERHEAT_TEMP) {
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	} else {
		health = POWER_SUPPLY_HEALTH_GOOD;
	}
	return health;
}

static int rt9426_get_battery_vbat_max(void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip || !chip->pdata)
		return 0;

	return chip->pdata->vbat_max;
}

static int rt9426_get_battery_cc(void *dev_data)
{
	return 0;
}

static int rt9426_update_basp_policy(void *dev_data, unsigned int level,
	unsigned int nondc_volt_dec)
{
	return rt9426_set_vterm_dec(nondc_volt_dec, dev_data);
}

static int rt9426_coul_set_cali(void *data, struct coul_batt_cali_info *info)
{
	struct coul_cali_coul_drv_ops *cali_ops = coul_cali_get_ops();
	int ret = 0;

	if (!cali_ops || !cali_ops->coul_set_mode || !cali_ops->coul_set_cali)
		return -EPERM;

	if (!data || !info)
		return -EPERM;

	if (info->batt_index == RT9426_IC_TYPE_MAIN)
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_AUX, cali_ops->dev_data);
	else
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_MAIN, cali_ops->dev_data);

	ret += cali_ops->coul_set_cali(cali_ops->dev_data, info);
	return ret;
}

static int rt9426_coul_get_cali(void *data, struct coul_batt_cali_info *info)
{
	struct coul_cali_coul_drv_ops *cali_ops = coul_cali_get_ops();
	int ret = 0;

	if (!cali_ops || !cali_ops->coul_set_mode || !cali_ops->coul_get_cali)
		return -EPERM;

	if (!data || !info)
		return -EPERM;

	if (info->batt_index == RT9426_IC_TYPE_MAIN)
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_AUX, cali_ops->dev_data);
	else
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_MAIN, cali_ops->dev_data);

	ret += cali_ops->coul_get_cali(cali_ops->dev_data, info);
	return ret;
}

static int rt9426_enable_cali_mode(int enable, void *dev_data);
static int rt9426_set_cali_adc_disable(void *data, int state)
{
	return rt9426_enable_cali_mode(state, data);
}

static struct coul_merge_drv_ops rt9426_ops = {
	.is_battery_exist = rt9426_is_battery_exist,
	.battery_voltage = rt9426_read_battery_vol,
	.battery_voltage_uv = rt9426_get_battery_voltage_uv,
	.battery_current = rt9426_get_battery_current,
	.battery_resistance = rt9426_get_battery_resistance,
	.fifo_avg_current = rt9426_get_battery_avg_current,
	.battery_current_avg = rt9426_read_battery_avg_current,
	.battery_capacity = rt9426_read_battery_soc,
	.battery_rm = rt9426_read_battery_rm,
	.battery_temperature = rt9426_get_battery_temperature,
	.battery_fcc = rt9426_read_battery_fcc,
	.battery_fcc_design = rt9426_get_battery_fcc_design,
	.battery_health = rt9426_get_battery_health,
	.battery_vbat_max = rt9426_get_battery_vbat_max,
	.battery_cycle_count = rt9426_read_battery_cycle,
	.get_battery_limit_fcc = rt9426_read_battery_fcc,
	.battery_cc = rt9426_get_battery_cc,
	.get_qmax = rt9426_get_battery_fcc_design,
	.charger_event_rcv = NULL,
	.update_basp_policy = rt9426_update_basp_policy,
	.coul_set_cali = rt9426_coul_set_cali,
	.coul_get_cali = rt9426_coul_get_cali,
	.set_cali_adc_disable = rt9426_set_cali_adc_disable,
};
#else
static struct coul_interface_ops rt9426_ops = {
	.type_name = "main",
	.is_coul_ready = rt9426_is_ready,
	.is_battery_exist = rt9426_is_battery_exist,
	.get_battery_capacity = rt9426_read_battery_soc,
	.get_battery_voltage = rt9426_read_battery_vol,
	.get_battery_current = rt9426_read_battery_current,
	.get_battery_avg_current = rt9426_read_battery_avg_current,
	.get_battery_temperature = rt9426_read_battery_temperature,
	.get_battery_fcc = rt9426_read_battery_fcc,
	.get_battery_cycle = rt9426_read_battery_cycle,
	.set_battery_low_voltage = rt9426_set_battery_low_voltage,
	.set_battery_last_capacity = rt9426_set_last_capacity,
	.get_battery_last_capacity = rt9426_get_last_capacity,
	.get_battery_rm = rt9426_read_battery_rm,
	.get_battery_charge_counter = rt9426_read_battery_charge_counter,
	.set_vterm_dec = rt9426_set_vterm_dec,
	.get_coul_model = rt9426_get_coul_model,
};

static struct coul_interface_ops rt9426_aux_ops = {
	.type_name = "aux",
	.is_coul_ready = rt9426_is_ready,
	.is_battery_exist = rt9426_is_battery_exist,
	.get_battery_capacity = rt9426_read_battery_soc,
	.get_battery_voltage = rt9426_read_battery_vol,
	.get_battery_current = rt9426_read_battery_current,
	.get_battery_avg_current = rt9426_read_battery_avg_current,
	.get_battery_temperature = rt9426_read_battery_temperature,
	.get_battery_fcc = rt9426_read_battery_fcc,
	.get_battery_cycle = rt9426_read_battery_cycle,
	.set_battery_low_voltage = rt9426_set_battery_low_voltage,
	.set_battery_last_capacity = rt9426_set_last_capacity,
	.get_battery_last_capacity = rt9426_get_last_capacity,
	.get_battery_rm = rt9426_read_battery_rm,
	.get_battery_charge_counter = rt9426_read_battery_charge_counter,
	.set_vterm_dec = rt9426_set_vterm_dec,
	.get_coul_model = rt9426_get_coul_model,
};
#endif /* CONFIG_COUL_DRV */

static int rt9426_get_data_from_int(int val)
{
	return RT9426_GAIN_DEFAULT_VAL + (((s64)(val) * RT9426_GAIN_BASE_VAL) /
		RT9426_COUL_DEFAULT_VAL - RT9426_GAIN_BASE_VAL);
}

static int rt9426_get_offset_from_int(int offset, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return RT9426_GAIN_DEFAULT_VAL;

	return RT9426_GAIN_DEFAULT_VAL - ((s64)offset * chip->pdata->rs_schematic) /
		chip->pdata->rs_ic_setting / POWER_MA_PER_A;
}

static int rt9426_get_calibration_curr(int *val, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!val || !chip)
		return -EPERM;

	*val = rt9426_get_curr_by_conversion(chip);
	dev_notice(chip->dev, "cali cur %d\n", *val);
	return 0;
}

static int rt9426_get_calibration_vol(int *val, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!val || !chip)
		return -EPERM;

	*val = rt9426_get_volt_by_conversion(chip);
	*val *= POWER_UV_PER_MV; /* mv to uv */
	dev_notice(chip->dev, "cali vol %d\n", *val);
	return 0;
}

static int rt9426_set_current_gain(unsigned int val, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	val = rt9426_get_data_from_int(val);

	chip->c_gain = val;
	rt9426_apply_calibration_para(chip, RT9426_GAIN_DEFAULT_VAL,
		chip->c_gain, chip->v_gain);
	dev_notice(chip->dev, "cur gain %d\n", val);
	return 0;
}

static int rt9426_set_voltage_gain(unsigned int val, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	val = rt9426_get_data_from_int(val);

	chip->v_gain = val;
	rt9426_apply_calibration_para(chip, RT9426_GAIN_DEFAULT_VAL,
		chip->c_gain, chip->v_gain);
	dev_notice(chip->dev, "voltage gain %d\n", val);
	return 0;
}

static int rt9426_set_current_offset(int val, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	val = rt9426_get_offset_from_int(val, chip);

	chip->c_offset = val;
	rt9426_apply_calibration_para(chip, chip->c_offset,
		chip->c_gain, chip->v_gain);
	dev_notice(chip->dev, "cur offset %d\n", val);
	return 0;
}

static int rt9426_enable_cali_mode(int enable, void *dev_data)
{
	struct rt9426_chip *chip = dev_data;

	if (!chip)
		return -EPERM;

	if (enable)
		rt9426_enter_calibration_mode(chip);
	else
		rt9426_exit_calibration_mode(chip);
	return 0;
}

static struct coul_cali_ops rt9426_cali_ops = {
	.dev_name = "aux",
	.get_current = rt9426_get_calibration_curr,
	.get_voltage = rt9426_get_calibration_vol,
	.set_current_offset = rt9426_set_current_offset,
	.set_current_gain = rt9426_set_current_gain,
	.set_voltage_gain = rt9426_set_voltage_gain,
	.set_cali_mode = rt9426_enable_cali_mode,
};

/* main battery gauge use aux calibration data for compatible */
static struct coul_cali_ops rt9426_aux_cali_ops = {
	.dev_name = "main",
	.get_current = rt9426_get_calibration_curr,
	.get_voltage = rt9426_get_calibration_vol,
	.set_current_gain = rt9426_set_current_gain,
	.set_voltage_gain = rt9426_set_voltage_gain,
	.set_cali_mode = rt9426_enable_cali_mode,
};

static int rt9426_calibration_para_invalid(int c_gain, int v_gain)
{
	return ((c_gain < RT9426_TBATICAL_MIN_A) ||
		(c_gain > RT9426_TBATICAL_MAX_A) ||
		(v_gain < RT9426_TBATCAL_MIN_A) ||
		(v_gain > RT9426_TBATCAL_MAX_A));
}

static int rt9426_calibration_offset_invalid(int c_offset)
{
	return ((c_offset < RT9426_TBATICAL_MIN_B) ||
		(c_offset > RT9426_TBATICAL_MAX_B));
}

static void rt9426_init_calibration_para(struct rt9426_chip *chip)
{
	int c_a = 0;
	int c_b = 0;
	int v_a = 0;

	chip->c_gain = RT9426_GAIN_DEFAULT_VAL;
	chip->v_gain = RT9426_GAIN_DEFAULT_VAL;
	chip->c_offset = RT9426_GAIN_DEFAULT_VAL;

	/* main battery gauge use aux calibration data for compatible */
	if (chip->pdata->ic_role == RT9426_IC_TYPE_MAIN) {
		coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_B, &c_b);
		coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_VOL_A, &v_a);
	} else {
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_B, &c_b);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_VOL_A, &v_a);
	}

	if (rt9426_calibration_para_invalid(c_a, v_a))
			goto update;

	if (!rt9426_calibration_offset_invalid(c_b))
		chip->c_offset = rt9426_get_offset_from_int(c_b, chip);

	chip->c_gain = rt9426_get_data_from_int(c_a);
	chip->v_gain = rt9426_get_data_from_int(v_a);

	dev_notice(chip->dev, "c_offset %d, c_gain %d, v_gain %d\n",
		chip->c_offset, chip->c_gain, chip->v_gain);
update:
	rt9426_apply_calibration_para(chip, chip->c_offset, chip->c_gain, chip->v_gain);
}

static int rt9426_irq_init(struct rt9426_chip *chip)
{
	int ret;

	/* mask irq before irq register */
	ret = rt9426_irq_enable(chip, false);
	if (ret < 0) {
		dev_notice(chip->dev, "scirq mask fail\n");
		return ret;
	}

	ret = rt9426_irq_register(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "irq init fail\n");
		return ret;
	}

	ret = rt9426_irq_enable(chip, true);
	if (ret < 0) {
		dev_notice(chip->dev, "scirq mask fail\n");
		free_irq(chip->alert_irq, chip);
		gpio_free(chip->pdata->intr_gpio);
	}

	return ret;
}

static void rt9426_register_power_log(struct rt9426_chip *chip)
{
	struct rt9426_platform_data *pdata = chip->pdata;

	if (pdata->ic_role == RT9426_IC_TYPE_MAIN) {
		rt9426_fg_ops.dev_data = (void *)chip;
		power_log_ops_register(&rt9426_fg_ops);
	} else {
		rt9426_aux_fg_ops.dev_data = (void *)chip;
		power_log_ops_register(&rt9426_aux_fg_ops);
	}
}

#ifdef CONFIG_COUL_DRV
static void rt9426_register_coul_ops(struct rt9426_chip *chip)
{
	coul_merge_drv_coul_ops_register(&rt9426_ops,
		chip, chip->pdata->ic_role, chip->pdata->batt_name);
}
#else
static void rt9426_register_coul_ops(struct rt9426_chip *chip)
{
	struct rt9426_platform_data *pdata = chip->pdata;

	if (pdata->ic_role == RT9426_IC_TYPE_MAIN) {
		rt9426_ops.dev_data = (void *)chip;
		coul_interface_ops_register(&rt9426_ops);
	} else {
		rt9426_aux_ops.dev_data = (void *)chip;
		coul_interface_ops_register(&rt9426_aux_ops);
	}
}
#endif /* CONFIG_COUL_DRV */

static void rt9426_register_cali_ops(struct rt9426_chip *chip)
{
	struct rt9426_platform_data *pdata = chip->pdata;

	rt9426_init_calibration_para(chip);
	if (pdata->ic_role == RT9426_IC_TYPE_MAIN) {
		rt9426_cali_ops.dev_data = (void *)chip;
		coul_cali_ops_register(&rt9426_cali_ops);
	} else {
		rt9426_aux_cali_ops.dev_data = (void *)chip;
		coul_cali_ops_register(&rt9426_aux_cali_ops);
	}
}

static void rt9426_register_power_device(struct rt9426_chip *chip)
{
	struct power_devices_info_data *power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = chip->dev->driver->name;
		power_dev_info->dev_id = chip->ic_ver;
		power_dev_info->ver_id = 0;
	}
}

static void rt9426_init_func(struct rt9426_chip *chip)
{
	int ret;
	struct rt9426_platform_data *pdata = NULL;
	struct i2c_client *i2c = NULL;

	if (!chip) {
		hwlog_info("chip NULL\n");
		return;
	}

	pdata = chip->pdata;
	i2c = chip->i2c;
	if (!pdata || !i2c) {
		dev_err(chip->dev, "pdata or i2c NULL\n");
		return;
	}

	/* apply platform data */
	ret = rt9426_apply_pdata(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "apply pdata fail\n");
		return;
	}

	rt9426_create_attrs(&i2c->dev);

	if (!pdata->disable_under_vol_irq) {
		ret = rt9426_irq_init(chip);
		if (ret < 0) {
			dev_notice(chip->dev, "init irq fail\n");
			return;
		}
	}

	rt9426_create_debug_files(chip);
	rt9426_register_power_log(chip);
	if (pdata->adc_sample_type) {
		if (pdata->adc_sample_type & BIT(BATT_CURRENT))
			batt_adc_monitor_ops_register(pdata->ic_role, BATT_CURRENT, rt9426_read_battery_current, chip);
	} else {
		rt9426_register_coul_ops(chip);
	}

	rt9426_register_cali_ops(chip);
	rt9426_dump_register(chip);

	chip->event_buck_nb.notifier_call = rt9426_buck_event_notifier_call;
	power_event_bnc_register(POWER_BNT_BUCK_CHARGE, &chip->event_buck_nb);
	chip->event_nb.notifier_call = rt9426_charging_event_notifier_call;
	power_event_bnc_register(POWER_BNT_CHARGING, &chip->event_nb);

	dev_info(chip->dev, "chip ver = 0x%04x\n", chip->ic_ver);
	rt9426_register_power_device(chip);
}

#ifndef CONFIG_COUL_DRV
static void rt9426_init_work_func(struct work_struct *work)
{
	struct rt9426_chip *chip = container_of(work, struct rt9426_chip,
		init_work);
	rt9426_init_func(chip);
}
#endif /* CONFIG_COUL_DRV */

static int rt9426_i2c_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct rt9426_platform_data *pdata = i2c->dev.platform_data;
	struct rt9426_chip *chip = NULL;
	u16 ic_ver;
	int ret;

	/* check chip id first */
	ret = rt9426_i2c_chipid_check(i2c);
	if (ret < 0) {
		dev_notice(&i2c->dev, "chip id check fail\n");
		return ret;
	}
	ic_ver = (u16)ret;
	/* alloc memory */
	chip = devm_kzalloc(&i2c->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	ret = rt9426_parse_dt(&i2c->dev, pdata);
	if (ret < 0) {
		dev_notice(&i2c->dev, "parse_dt fail\n");
		return ret;
	}
	chip->pdata = i2c->dev.platform_data = pdata;

	chip->i2c = i2c;
	chip->dev = &i2c->dev;
	chip->btemp = RT9426_BAT_TEMP_VAL;
	chip->bvolt = RT9426_BAT_VOLT_VAL;
	chip->bcurr = RT9426_BAT_CURR_VAL;
	chip->ic_ver = ic_ver;
	chip->ocv_checksum_ic = 0;
	chip->lowtemp_edv = 0;
	chip->ocv_checksum_dtsi = pdata->ocv_table[9].data[4];
	chip->low_v_smooth_en = false;
	chip->pdata->fc_low_curr_cnt = 0;

	mutex_init(&chip->update_lock);
	mutex_init(&chip->var_lock);
	mutex_init(&chip->retry_lock);
	i2c_set_clientdata(i2c, chip);
	rt9426_apply_sense_resistor(chip);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	if (pdata->ic_role == RT9426_IC_TYPE_MAIN)
		set_hw_dev_flag(DEV_I2C_GAUGE_IC);
	else
		set_hw_dev_flag(DEV_I2C_GAUGE_IC_AUX);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	/* rt regmap init */
	ret = rt9426_regmap_init(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "regmap init fail\n");
		return ret;
	}

#ifdef CONFIG_COUL_DRV
	rt9426_init_func(chip);
#else
	INIT_WORK(&chip->init_work, rt9426_init_work_func);
	queue_work(system_highpri_wq, &chip->init_work);
#endif /* CONFIG_COUL_DRV */

#ifdef CONFIG_COUL_DRV
	if (pdata->ic_role == RT9426_IC_TYPE_MAIN)
		g_rt9426_chip[RT9426_IC_TYPE_MAIN] = chip;
	else
		g_rt9426_chip[RT9426_IC_TYPE_AUX] = chip;

	if (pdata->platform_support)
		rt9426_adapt_kirin_platform(chip);
#endif /* CONFIG_COUL_DRV */

	return 0;
}

static int rt9426_i2c_remove(struct i2c_client *i2c)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);

	dev_info(chip->dev, "%s\n", __func__);
	if (!chip->pdata->disable_under_vol_irq) {
		rt9426_irq_enable(chip, false);
		rt9426_irq_deinit(chip);
	}
	mutex_destroy(&chip->var_lock);
	mutex_destroy(&chip->retry_lock);
	return 0;
}

static int rt9426_i2c_suspend(struct device *dev)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);

	dev_dbg(chip->dev, "%s\n", __func__);
	if (!chip->pdata->disable_under_vol_irq && device_may_wakeup(dev))
		enable_irq_wake(chip->alert_irq);
	return 0;
}

static int rt9426_i2c_resume(struct device *dev)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);

	dev_dbg(chip->dev, "%s\n", __func__);
	if (!chip->pdata->disable_under_vol_irq && device_may_wakeup(dev))
		disable_irq_wake(chip->alert_irq);
	return 0;
}

static SIMPLE_DEV_PM_OPS(rt9426_pm_ops, rt9426_i2c_suspend, rt9426_i2c_resume);

static const struct i2c_device_id rt9426_i2c_id[] = {
	{ "rt9426", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, rt9426_i2c_id);

static const struct of_device_id rt9426_match_table[] = {
	{ .compatible = "richtek,rt9426", },
	{},
};
MODULE_DEVICE_TABLE(of, rt9426_match_table);

static struct i2c_driver rt9426_i2c_driver = {
	.driver = {
		.name = "rt9426",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rt9426_match_table),
		.pm = &rt9426_pm_ops,
	},
	.probe = rt9426_i2c_probe,
	.remove = rt9426_i2c_remove,
	.id_table = rt9426_i2c_id,
};

static int __init rt9426_init(void)
{
	return i2c_add_driver(&rt9426_i2c_driver);
}

static void __exit rt9426_exit(void)
{
	i2c_del_driver(&rt9426_i2c_driver);
}

#ifdef CONFIG_COUL_DRV
fs_initcall_sync(rt9426_init);
#else
device_initcall(rt9426_init);
#endif /* CONFIG_COUL_DRV */
module_exit(rt9426_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt9426 Fuel Gauge Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
