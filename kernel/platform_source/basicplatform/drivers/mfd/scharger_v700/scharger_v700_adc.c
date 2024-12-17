/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for schargerV700 adc
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

#include "scharger_v700_adc.h"
#include "scharger_v700.h"

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <securec.h>

enum adc_mode {
	SINGLE_MODE,
	LOOP_MODE,
	EIS_MODE,
};

struct scharger_adc {
	struct device *dev;
	struct regmap *regmap;
	enum adc_mode mode;
	struct wakeup_source *wake_lock;
	struct mutex adc_conv_lock;
	int batt_temp_compensation_en;
	unsigned int scharger_version;
};

struct scharger_adc *g_adc = NULL;
static atomic_t g_adc_get_val_flag = ATOMIC_INIT(0);

static unsigned short get_default_val(u32 ch)
{
	unsigned short val;

	switch (ch) {
	case CHG_ADC_CH_VBATL:
	case CHG_ADC_CH_VBATH:
		val = 0x1130; /* about 25 degrees */
		break;
	case CHG_ADC_CH_TDIE:
	case CHG_ADC_CH_TSBATL:
	case CHG_ADC_CH_TSBATH:
		val = 0x500; /* about 4400V */
		break;
	default:
		val = 0;
	}
	return val;
}

int get_adc_flag(void)
{
	return atomic_read(&g_adc_get_val_flag);
}

void set_adc_flag(unsigned int val)
{
	atomic_set(&g_adc_get_val_flag, val);
}

/* mode: single = 0, loop = 1 */
static void adc_loop_mode_set(struct scharger_adc *adc , u32 en)
{
	int ret;

	ret = regmap_update_bits
		(adc->regmap, REG_HKADC_CTRL1, SC_HKADC_SEQ_LOOP_MASK, en << SC_HKADC_SEQ_LOOP_SHIFT);
	if(ret)
		dev_err(adc->dev, "%s, en %d error %d\n", __func__, en, ret);
}

/* mode: single, loop */
static void adc_eis_set(struct scharger_adc *adc , int eis_en)
{
	int ret;

	ret = regmap_update_bits
		(adc->regmap, REG_EIS_EN, SC_ADC_EIS_SEL_MASK, eis_en);
	if(ret)
		dev_err(adc->dev, "%s, eis_en %d error %d\n", __func__, eis_en, ret);
}

static void adc_enable(struct scharger_adc *adc , u32 en)
{
	int ret;

	ret = regmap_update_bits
		(adc->regmap, REG_HKADC_EN, SC_HKADC_EN_MASK, en);
	if(ret)
		dev_err(adc->dev, "%s, eis_en %d error %d\n", __func__, en, ret);

	/* close part of adc circuit when adc disable */
	if (adc->scharger_version == CHIP_ID_V700)
		return;
	else
		regmap_update_bits
			(adc->regmap, REG_SCHG_LOGIC_CFG_REG_5, CLOSE_PART_ADC_CIRCUIT_MSK, en << CLOSE_PART_ADC_CIRCUIT_SHIFT);
}

static void adc_loop_channel_set(struct scharger_adc *adc, int en)
{
	int ret = 0;
	unsigned char val_l = 0;
	unsigned char val_h = 0;

	if(en) {
		/* select all channel */
		val_l = 0xff;
		val_h = 0x0f;
	}
	ret += regmap_write(adc->regmap, REG_HKADC_SEQ_CH_L, val_l);
	ret += regmap_write(adc->regmap, REG_HKADC_SEQ_CH_H, val_h);
	if(ret)
		dev_err(adc->dev, "%s, error %d\n", __func__, ret);
}


static void adc_channel_set(struct scharger_adc *adc , unsigned int ch)
{
	u8 val_l = 0x00;
	u8 val_h = 0x00;
	int ret = 0;

	if (ch < CHG_ADC_CH_L_MAX)
		val_l = (1 << ch);
	else
		val_h = (1 << (ch - CHG_ADC_CH_L_MAX));

	if (ch == CHG_ADC_CH_IBUS)
		val_l |= (1 << (CHG_ADC_CH_IBUS_REF));

	ret += regmap_write(adc->regmap, REG_HKADC_SEQ_CH_L, val_l);
	ret += regmap_write(adc->regmap, REG_HKADC_SEQ_CH_H, val_h);
	if(ret)
		dev_err(adc->dev, "%s, ch %d error %d\n", __func__, ch, ret);
}

static void adc_start(struct scharger_adc *adc)
{
	int ret;

	ret = regmap_update_bits
		(adc->regmap, REG_EIS_HKADC_START, SC_EIS_HKADC_START_MASK, 1);
	if(ret)
		dev_err(adc->dev, "%s, error %d\n", __func__, ret);
}

static void adc_read_request(struct scharger_adc *adc)
{
	int ret;

	ret = regmap_update_bits(adc->regmap, REG_HKADC_RD_SEQ, SC_HKADC_RD_REQ, 1);
	if(ret)
		dev_err(adc->dev, "%s error %d\n", __func__, ret);
}

static void adc_read_data(struct scharger_adc *adc, int ch, unsigned short *adc_data)
{
	int ret;

	ret = regmap_bulk_read(adc->regmap, REG_ADC_DATA_BASE + (ADC_DATA_LEN * ch),
		adc_data, ADC_DATA_LEN);
	pr_err("%s: ch=%d, adc_data=%u", __func__, ch, *adc_data);
	if(ret){
		dev_err(adc->dev, "%s error %d\n", __func__, ret);
		*adc_data = 0;
	}
}

static void adc_loop_mode_enable(struct scharger_adc * adc, u32 en)
{
	if(adc->mode == (LOOP_MODE ^ en)) {
		dev_err(adc->dev, "%s en %d, pre mode %d\n", __func__, en, adc->mode);
		adc_loop_mode_set(adc, en);
		adc_eis_set(adc, 0);
		adc_enable(adc, en);
		adc_loop_channel_set(adc, en);
		if(en) {
			adc_start(adc);
			adc->mode = LOOP_MODE;
		} else {
			adc->mode = SINGLE_MODE;
		}
	}
}

static int adc_complete(struct scharger_adc * adc)
{
	int ret;
	unsigned int val = 0;
	int time = 10;

	while(!(val & HKADC_DATA_VALID_MASK) && time){
		mdelay(1);
		ret = regmap_read(adc->regmap, REG_HKADC_DATA_VALID, &val);
		time--;
	}

	if(time == 0)
		return -1;
	return 0;
}

static int adc_get_val_single(struct scharger_adc * adc, u32 ch, unsigned short *data)
{
	int timeout;
	int ret = 0;

	adc_eis_set(adc, 0);
	adc_enable(adc, 1);
	adc_channel_set(adc, ch);
	adc_start(adc);

	timeout = adc_complete(adc);
	if(timeout) {
		*data = 0;
		ret = -EINVAL;
		goto out;
	}

	adc_read_data(adc, ch, data);
out:
	adc_enable(adc, 0);
	return ret;
}

static int scharger_get_adc_value(struct scharger_adc * adc, u32 ch, unsigned short *data)
{
	int ret = 0;

	if(!adc) {
		pr_err("%s adc is null.\n", __func__);
		*data = 0;
		return -EINVAL;
	}

	mutex_lock(&adc->adc_conv_lock);
	__pm_stay_awake(adc->wake_lock);
	if (get_adc_flag() == 1) {
		pr_err("%s eis is working, use default val.\n", __func__);
		*data = get_default_val(ch);
		__pm_relax(adc->wake_lock);
		mutex_unlock(&adc->adc_conv_lock);
		return ret;
	}
	set_adc_flag(1);
	if(adc->mode == LOOP_MODE) {
		adc_read_request(adc);
		adc_read_data(adc, ch, data);
	} else {
		/* single mode */
		ret = adc_get_val_single(adc, ch, data);
	}
	set_adc_flag(0);
	__pm_relax(adc->wake_lock);
	mutex_unlock(&adc->adc_conv_lock);
	return ret;
}

int scharger_get_sbu1_res(void)
{
	int val = 0;
	int ret = 0;

	if(!g_adc) {
		pr_err("%s adc is null.\n", __func__);
		return 0;
	}

	ret += regmap_write(g_adc->regmap, REG_SBU_DET_CFG, SBU1_10UA_DET);
	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_SBU, (unsigned short*)&val);
	ret += regmap_write(g_adc->regmap, REG_SBU_DET_CFG, SBU_DET_DIS);

	scharger_inf("[%s] %d\n", __func__, val);
	return val;
}

int scharger_get_sbu2_res(void)
{
	int val = 0;
	int ret = 0;

	if(!g_adc) {
		pr_err("%s adc is null.\n", __func__);
		return 0;
	}

	ret += regmap_write(g_adc->regmap, REG_SBU_DET_CFG, SBU2_10UA_DET);
	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_SBU, (unsigned short*)&val);
	ret += regmap_write(g_adc->regmap, REG_SBU_DET_CFG, SBU_DET_DIS);

	scharger_inf("[%s] %d\n", __func__, val);
	return val;
}

int scharger_get_dp_res(void)
{
	int val = 0;
	int ret = 0;

	if(!g_adc) {
		pr_err("%s adc is null.\n", __func__);
		return 0;
	}

	ret += regmap_write(g_adc->regmap, REG_DP_DET_CFG, DP_1UA_DET);
	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VDP, (unsigned short*)&val);
	ret += regmap_write(g_adc->regmap, REG_DP_DET_CFG, DP_DET_DIS);

	scharger_inf("[%s] %d\n", __func__, val);
	return val;
}

int scharger_get_ibat_h(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_IBATH, (unsigned short*)&val);
	return val;
}

int scharger_get_ibat_l(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_IBATL, (unsigned short*)&val);
	return val;
}

int scharger_get_vbat_h(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VBATH, (unsigned short*)&val);
	scharger_dbg("%s val %d\n", __func__, val);

	return val;
}

int scharger_get_vbat_l(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VBATL, (unsigned short*)&val);
	scharger_dbg("%s val %d\n", __func__, val);

	return val;
}

static int get_vbat_h(int *vbat)
{
	if (vbat == NULL) {
		pr_err("%s: input ibat is NULL\n", __func__);
		return -ENOMEM;
	}
	return scharger_get_adc_value(g_adc, CHG_ADC_CH_VBATH, (unsigned short*)vbat);
}

static int get_vbat_l(int *vbat)
{
	if (vbat == NULL) {
		pr_err("%s: input ibat is NULL\n", __func__);
		return -ENOMEM;
	}
	return scharger_get_adc_value(g_adc, CHG_ADC_CH_VBATL, (unsigned short*)vbat);
}

int scharger_get_vbus(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VBUS, (unsigned short*)&val);
	return val;
}

int scharger_adc_get_vbus(unsigned int *val)
{
	int ret;

	if (val == NULL) {
		pr_err("%s: input val is NULL\n", __func__);
		return -ENOMEM;
	}
	ret = scharger_get_adc_value(g_adc, CHG_ADC_CH_VBUS, (unsigned short*)val);

	scharger_dbg("%s val %d\n", __func__, *val);
	return ret;
}

int scharger_get_vpsw(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VPSW, (unsigned short*)&val);
	return val;
}

int scharger_get_vusb(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_VUSB, (unsigned short*)&val);
	return val;
}

int scharger_adc_get_vusb(int *val)
{
	int ret;

	if (val == NULL) {
		pr_err("%s: input val is NULL\n", __func__);
		return -ENOMEM;
	}
	ret = scharger_get_adc_value(g_adc, CHG_ADC_CH_VUSB, (unsigned short*)val);

	scharger_dbg("%s val %d\n", __func__, val);

	return ret;
}

/* NTC Table */
static int t_v_table[][2] = {
	{ -273, 4095}, { -40, 3764}, { -36, 3689}, { -32, 3602}, { -28, 3500},
	{ -24, 3387}, { -20, 3261}, { -16, 3122}, { -12, 2973}, { -8, 2814},
	{ -4, 2650}, { 0, 2480}, { 4, 2308}, { 8, 2136}, { 12, 1967},
	{ 16, 1803}, { 20, 1646}, { 24, 1497}, { 28, 1360}, { 32, 1230},
	{ 36, 1111}, { 40, 1001}, { 44, 903}, { 48, 812}, { 52, 729},
	{ 56, 655}, { 60, 590}, { 64, 531}, { 74, 406}, { 84, 313},
	{ 125, 110}, { 0, 0},
};

static int charger_adc_to_temp(int temp_volt)
{
	int temprature = 0;
	int i;

	if (temp_volt >= t_v_table[0][1])
		return t_v_table[0][0];

	if (temp_volt <= t_v_table[T_V_ARRAY_LENGTH - 1][1])
		return t_v_table[T_V_ARRAY_LENGTH - 1][0];

	/* else */
	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		if (temp_volt == t_v_table[i][1])
			return t_v_table[i][0];

		if (temp_volt > t_v_table[i][1])
			break;
	}
	if (i == 0)
		return t_v_table[0][0];

	if ((t_v_table[i][1] - t_v_table[i - 1][1]) != 0)
		temprature = t_v_table[i - 1][0] +
			(long)(temp_volt - t_v_table[i - 1][1]) *
				(t_v_table[i][0] - t_v_table[i - 1][0]) /
				(t_v_table[i][1] - t_v_table[i - 1][1]);

	return temprature;
}

static u32 battery_temp_compensation(struct scharger_adc *di,
	u32 adc_code)
{
	if (di->batt_temp_compensation_en == 0)
		goto no_compensation;

no_compensation:
	return adc_code;
}

static int _scharger_get_tsbat(u32 ch)
{
	int ret = 0;
	u32 adc_code = 0;
	u32 adc_code_new;
	int tbat;

	if (g_adc == NULL ) {
		scharger_err("[%s]\n", __func__);
		return 0;
	}

	ret = scharger_get_adc_value(g_adc, ch, (unsigned short*)&adc_code);
	if (ret) {
		scharger_err("[%s]get ts_bat fail,ret:%d\n", __func__, ret);
		return 0;
	}

	adc_code_new = battery_temp_compensation(g_adc, adc_code);
	tbat = charger_adc_to_temp(adc_code_new);
	scharger_inf("[%s]adc_code %d, adc_code_new %d, ts_bat %d\n",
			     __func__, adc_code, adc_code_new, tbat);
	return tbat;
}


int scharger_get_tsbat_h(void)
{
	int tsbat = 0;

	tsbat = _scharger_get_tsbat(CHG_ADC_CH_TSBATH);

	return tsbat;
}

int scharger_get_tsbat_l(void)
{
	int tsbat = 0;

	tsbat = _scharger_get_tsbat(CHG_ADC_CH_TSBATL);
	return tsbat;
}

int scharger_get_tdie(void)
{
	int val = 0;

	(void)scharger_get_adc_value(g_adc, CHG_ADC_CH_TDIE, (unsigned short*)&val);

	/*
	 * code val to actual val
	 * 2500: total range of temperature;
	 * 4096: 12 bits to record temperature, 2^12 = 4096;
	 * 1437277872: base value, for chip set
	 * 1000000: conversion scale
	 * 3406962: the reference value
	 */
	val = (2500 * val) / 4096;
	val = (int)(1437277872 - val * 1000000) / (3406962);

	return val;
}

int scharger_get_ibus(void)
{
	int val = 0;
	unsigned int ilimit = 0;
	int ret;

	ret = scharger_get_adc_value(g_adc, CHG_ADC_CH_IBUS, (unsigned short*)&val);
	if (ret) {
		scharger_err("[%s]get ibus fail,ret:%d\n", __func__, ret);
		return -1;
	}

	(void)regmap_read(g_adc->regmap, CHG_INPUT_SOURCE_REG, &ilimit);
	ilimit &= CHG_ILIMIT_MSK;
	ilimit >>= CHG_ILIMIT_SHIFT;
	if (ilimit < CHG_ILIMIT_600MA)
			val = val / CHG_IBUS_DIV;

	/* multiplied by 1045/1000 for IBUS ADC optimization precision */
	if (g_adc->scharger_version == CHIP_ID_V700)
		return val;
	else
		val = val * 1045 / 1000;

	return val;
}

static int get_ibus(int *ibus)
{
	if (ibus == NULL) {
		pr_err("%s: input ibus is NULL\n", __func__);
		return -ENOMEM;
	}

	return scharger_get_adc_value(g_adc, CHG_ADC_CH_IBUS, (unsigned short*)ibus);
}

static unsigned int get_device_version(struct scharger_adc *adc)
{
	unsigned int scharger_version = 0;

	if (adc == NULL) {
		pr_err("%s scharger is null\n", __func__);
		return -EINVAL;
	}

	/* byte length of version 0 is 2 */
	(void)regmap_bulk_read(adc->regmap, CHIP_VERSION_4,
				&scharger_version, 2);

	pr_info("%s, chip version is 0x%x\n", __func__, scharger_version);
	return scharger_version;
}

static int scharger_adc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct scharger_adc *adc = NULL;
	int ret = 0;

	adc = devm_kzalloc(dev, sizeof(*adc), GFP_KERNEL);
	if (!adc) {
		dev_err(dev, "[%s]adc is null.\n", __func__);
		return -ENOMEM;
	}

	adc->wake_lock = wakeup_source_register(dev, "scharger adc wakelock");
	mutex_init(&adc->adc_conv_lock);

	adc->dev = dev;
	adc->regmap = dev_get_regmap(dev->parent, NULL);
	if (!adc->regmap) {
		dev_err(dev, "Parent regmap unavailable.\n");
		return -ENXIO;
	}

	adc->scharger_version = get_device_version(adc);

	adc_loop_mode_enable(adc, 0);
	g_adc = adc;

	pr_err("[%s] succ\n", __func__);
	return ret;
}

static int scharger_adc_remove(struct platform_device *pdev)
{
	 g_adc = NULL;
	 return 0;
}

const static struct of_device_id scharger_v700_adc_of_match[] = {
	{
		.compatible = "hisilicon,scharger-v700-adc",
		.data = NULL,
	},
	{},
};

static struct platform_driver scharger_v700_adc_driver = {
	.driver = {
		.name	= "scharger_v700_adc",
		.owner  = THIS_MODULE,
		.of_match_table = scharger_v700_adc_of_match,
	},
	.probe	= scharger_adc_probe,
	.remove	= scharger_adc_remove,
};

static int __init scharger_adc_init(void)
{
	return platform_driver_register(&scharger_v700_adc_driver);
}

static void __exit scharger_adc_exit(void)
{
	platform_driver_unregister(&scharger_v700_adc_driver);
}

fs_initcall(scharger_adc_init);
module_exit(scharger_adc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("schargerV700 adc driver");
