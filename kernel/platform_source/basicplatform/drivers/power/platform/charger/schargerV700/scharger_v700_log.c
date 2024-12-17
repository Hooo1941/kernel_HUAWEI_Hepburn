/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: sysfs process func for charger module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>

#include "../../../../mfd/scharger_v700/scharger_v700_adc.h"
#include "scharger_v700_buck.h"
#include "scharger_v700_dc.h"

#define REG_GLB_NUM (0xA0)
#define REG_GLB_BASE_ADDR (0x300)

#define REG_IRQ_NUM (0x31)
#define REG_IRQ_BASE_ADDR (0x800)

#define REG_ANA_NUM (0xBA)
#define REG_ANA_BASE_ADDR (0x900)

#define REG_PD_NUM (0x7F)
#define REG_PD_BASE_ADDR (0x0)

struct schargerlog_adc_data {
	int ibus;
	int vbus;
	int ibat_h;
	int ibat_l;
	int vusb;
	int vbat_h;
	int vbat_l;
	int tdie;
	int tsbat_h;
	int tsbat_l;
	int vpsw;
};

struct schargerlog_data {
	struct schargerlog_adc_data adc_data;
	enum usb_mode_state chg_mode;
};

static void show_reg_head(char *buf, size_t str_len)
{
	int ret = 0;
	int i;
	size_t len = 0;

	for (i = 0; i <= REG_PD_NUM; i++) {
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%-3x  ", REG_PD_BASE_ADDR + i);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_GLB_NUM; i++) {
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%x  ", REG_GLB_BASE_ADDR + i);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_IRQ_NUM; i++) {
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%x  ", REG_IRQ_BASE_ADDR + i);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_ANA_NUM; i++) {
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%x  ", REG_ANA_BASE_ADDR + i);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}
}

static ssize_t get_scharger_head(char *buf, int str_len)
{
	int ret;

	ret = snprintf_s(buf, str_len, str_len - 1, "%s",
		 "Mode  Ibus  Vbus  Ibat_h  Ibat_l  Vusb  Vbat_h  "
		 "Vbat_l  Tdie  TsBat_h  TsBat_l  Vpsw  ");
	if (ret < 0) {
		scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return 0;
	}

	show_reg_head(buf, str_len);

	return strlen(buf);
}

int scharger_get_register_head(char *buf, int size, void *dev_data)
{
	int str_len = size - 1;
	int err;

	err = memset_s(buf, size, 0, size);
	if (err != EOK)
		scharger_err("[%s]memset_s fail, err=%d\n", __func__, err);
	get_scharger_head(buf, str_len);

	return 0;
}

static void get_adc_data(struct schargerlog_adc_data *adc_data)
{
	adc_data->ibus = scharger_get_ibus();
	adc_data->vbus = scharger_get_vbus();
	adc_data->ibat_h = scharger_get_ibat_h();
	adc_data->ibat_l = scharger_get_ibat_l();
	adc_data->vusb = scharger_get_vusb();
	adc_data->vbat_h = scharger_get_vbat_h();
	adc_data->vbat_l = scharger_get_vbat_l();
	adc_data->tdie = scharger_get_tdie();
	adc_data->tsbat_h = scharger_get_tsbat_h();
	adc_data->tsbat_l = scharger_get_tsbat_l();
	adc_data->vpsw = scharger_get_vpsw();
}

static void show_chg_mode_schargerlog(char *buf, int str_len,
	enum usb_mode_state chg_mode)
{
	size_t len = strlen(buf);
	int ret;

	if (chg_mode == CHG_TYPE_SDP)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "SDP     ");
	else if (chg_mode == CHG_TYPE_CDP)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "CDP     ");
	else if (chg_mode == CHG_TYPE_UNKNOWN)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "UNKNOWN ");
	else if (chg_mode == CHG_TYPE_LVC)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "LVC     ");
	else if (chg_mode == CHG_TYPE_SC)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "SC      ");
	else if (chg_mode == CHG_TYPE_DCP)
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "DCP     ");
	else
		ret = snprintf_s(buf + len, str_len - len, str_len - len - 1, "NONE    ");
	if (ret <= 0)
		scharger_err("[%s]snprintf_s fail, %s\n", __func__, buf);
}

static void show_adc_schargerlog(char *buf, int str_len, struct schargerlog_adc_data *adc_data)
{
	size_t len = strlen(buf);
	int ret;

	ret = snprintf_s(buf + len, str_len - len, str_len -len - 1,
		"%-4d  %-4d  %-6d  %-6d  %-4d  %-6d  "
		"%-6d  %-4d  %-7d  %-7d  %-4d  ",
		 adc_data->ibus, adc_data->vbus, adc_data->ibat_h, adc_data->ibat_l,
		 adc_data->vusb, adc_data->vbat_h, adc_data->vbat_l, adc_data->tdie,
		 adc_data->tsbat_h, adc_data->tsbat_l, adc_data->vpsw);
	if (ret < 0) {
		scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
		return;
	}
}

static void show_reg_value_schargerlog(char *buf, int str_len, struct scharger_buck *di)
{
	unsigned int i;
	int ret = 0;
	size_t len = 0;
	unsigned int val;

	for (i = 0; i <= REG_PD_NUM; i++) {
		regmap_read(di->regmap, REG_PD_BASE_ADDR + i, &val);
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%-3x  ", val);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_GLB_NUM; i++) {
		regmap_read(di->regmap, REG_GLB_BASE_ADDR + i, &val);
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%-3x  ", val);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_IRQ_NUM; i++) {
		regmap_read(di->regmap, REG_IRQ_BASE_ADDR + i, &val);
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%-3x  ", val);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}

	for (i = 0; i <= REG_ANA_NUM; i++) {
		regmap_read(di->regmap, REG_ANA_BASE_ADDR + i, &val);
		len = strlen(buf);
		ret = snprintf_s(buf + len, str_len - len, str_len - len -1, "0x%-3x  ", val);
		if (ret < 0) {
			scharger_err("(%s)snprintf_s fail ret = %d\n", __func__, ret);
			return;
		}
	}
}

static enum usb_mode_state get_charge_mode(void)
{
	enum usb_mode_state chg_mode;
	enum chg_mode_state dc_chg_mode;

	chg_mode = get_buck_charge_mode();
	if (chg_mode != CHG_TYPE_NONE) {
		dc_chg_mode = dc_get_direct_charge_mode();
		if (dc_chg_mode == SC)
			chg_mode = CHG_TYPE_SC;
		else if (dc_chg_mode == LVC)
			chg_mode = CHG_TYPE_LVC;
	}
	return chg_mode;
}

static void _show_schargerlog(char *buf, int str_len, struct schargerlog_data *chglog_data)
{
	show_chg_mode_schargerlog(buf, str_len, chglog_data->chg_mode);
	show_adc_schargerlog(buf, str_len, &chglog_data->adc_data);
}

static ssize_t show_schargerlog(char *buf, int size, struct scharger_buck *di)
{
	struct schargerlog_data chglog_data = {0};
	int str_len = size - 1;

	get_adc_data(&chglog_data.adc_data);
	chglog_data.chg_mode = get_charge_mode();

	_show_schargerlog(buf, str_len, &chglog_data);

	show_reg_value_schargerlog(buf, str_len, di);

	return strlen(buf);
}

int scharger_dump_register(char *buf, int size, void *dev_data)
{
	int err;

	err = memset_s(buf, size, 0, size);
	if (err != EOK)
		scharger_err("[%s]memset_s fail, err=%d\n", __func__, err);
	show_schargerlog(buf, size, dev_data);

	return 0;
}
