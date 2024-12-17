// SPDX-License-Identifier: GPL-2.0
/*
 * cw2217.c
 *
 * driver for cw2217 battery fuel gauge
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sizes.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_devices_info.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/hardware_ic/ground_loop_compensate.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <chipset_common/hwmanufac/dev_detect/dev_detect.h>
#endif
#ifdef CONFIG_COUL_DRV
#include "platform_include/basicplatform/linux/power/platform/coul/coul_merge_drv.h"
#endif /* CONFIG_COUL_DRV */
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/battery/battery_adc_monitor.h>

#define HWLOG_TAG cw2217
HWLOG_REGIST();

#define CW2217_REG_CHIP_ID               0x00
/* history cycle count before reg reset */
#define CW2217_REG_CYCLE_BASE            0x01
#define CW2217_REG_VCELL_H               0x02
#define CW2217_REG_SOC_INT               0x04
#define CW2217_REG_TEMP                  0x06
#define CW2217_REG_MODE_CONFIG           0x08
#define CW2217_REG_GPIO_CONFIG           0x0A
#define CW2217_REG_SOC_ALERT             0x0B
#define CW2217_REG_CURRENT_H             0x0E
#define CW2217_REG_USER_CONF             0xA2
#define CW2217_REG_USER_CONF_EX          0xA3
#define CW2217_REG_CYCLE_H               0xA4
#define CW2217_REG_SOH                   0xA6
#define CW2217_REG_IC_STATE              0xA7
#define CW2217_REG_FW_VERSION            0xAB
#define CW2217_REG_BAT_PROFILE           0x10

#define CW2217_CONFIG_MODE_RESTART       0x30
#define CW2217_CONFIG_MODE_ACTIVE        0x00
#define CW2217_CONFIG_MODE_SLEEP         0xF0
#define CW2217_CONFIG_UPDATE_FLG         0x80
#define CW2217_IC_VCHIP_ID               0xA0
#define CW2217_IC_READY_MARK             0x0C
#define CW2217_GPIO_SOC_IRQ_VALUE        0x00

#define CW2217_COMPLEMENT_CODE_U16       0x8000
#define CW2217_QUEUE_DELAYED_WORK_TIME   5000
#define CW2217_QUEUE_START_WORK_TIME     50
#define CW2217_QUEUE_RESUME_WORK_TIME    20

#define CW2217_SLEEP_COUNT               50
#define CW2217_RETRY_COUNT               3
#define CW2217_VOL_MAGIC_PART1           5
#define CW2217_VOL_MAGIC_PART2           16
#define CW2217_UI_FULL                   100
#define CW2217_SOC_MAGIC_BASE            256
#define CW2217_SOC_MAGIC_100             100
#define CW2217_TEMP_MAGIC_PART1          10
#define CW2217_TEMP_MAGIC_PART2          2
#define CW2217_TEMP_MAGIC_PART3          400

#define CW2217_CYCLE_MAGIC               16
#define CW2217_FULL_CAPACITY             100
#define CW2217_TEMP_ABR_LOW              (-400)
#define CW2217_TEMP_ABR_HIGH             800
#define CW2217_NOT_ACTIVE                1
#define CW2217_PROFILE_NOT_READY         2
#define CW2217_PROFILE_NEED_UPDATE       3
#define CW2217_CAPACITY_TH               7
#define CW2217_SIZE_OF_PROFILE           80
#define CW2217_COEFFICIENT_DEFAULT_VAL   1000000
#define CW2217_RESISTANCE_DEFAULT_VAL    1000
#define CW2217_SOH_DEFAULT_VAL           100
#define CW2217_RATED_CAPACITY_VAL        4000
#define CW2217_RSENSE_CORRECT_UP         63
#define CW2217_RSENSE_CORRECT_DOWN       (-192)
#define CW2217_TBATICAL_MIN_A            820000
#define CW2217_TBATICAL_MAX_A            1050000
#define CW2217_LEN_U8_REG                1
#define CW2217_LEN_U16_DAT               2
#define CW2217_FLAG_UPDATE_SOC           90
#define CW2217_ERROR_UI_SOC              25
#define CW2217_ERROR_VOLTAGE             3900
#define CW2217_T_R_ARRAY_LEN             151
#define CW2217_SOC_MAPPING_ROW           10
#define CW2217_SOC_MAPPING_COL           3
#define CW2217_SOC_MAPPING_DEFAULT_K     10000
#define CW2217_SOC_MAPPING_DEFAULT_LEN   1
#define CW2217_BATT_NAME_LEN             32
#define CW2217_COLD_TEMP                 (-200)
#define CW2217_OVERHEAT_TEMP             600
#define CW2217_TEMP_UNKNOWN              (-400)
#define CW2217_RECORD_NUM                5
#define CW2217_MAX_SOC_DIFF              20
#define CW2217_DISCHARGE_THRESHOLD       (-10)
#define CW2217_VBAT_SOC_TABLE_ROW        11
#define CW2217_CYCLE_BASE_CONVERT        10
#define CW2217_MAX_U8_VALUE              255

struct cw2217_vat_soc_pair {
	int volt_min;
	int volt_max;
	int soc_u;
};

struct cw2217_cached_reg_data {
	u8 temp;
};

struct cw2217_dev {
	struct i2c_client *client;
	struct device *dev;
	struct workqueue_struct *cwfg_workqueue;
	struct delayed_work battery_delay_work;
	struct work_struct init_work;
	char batt_name[CW2217_BATT_NAME_LEN];
	u8 config_profile_info[CW2217_SIZE_OF_PROFILE];
	u32 soc_mapping[CW2217_SOC_MAPPING_ROW][CW2217_SOC_MAPPING_COL];
	u32 soc_mapping_len;
	int ocv_idx;
	u32 ic_role;
	int ir_comp_en;
	int compensation_r;
	int rated_capacity;
	int coefficient;
	int resistance;
	int chip_id;
	int voltage;
	int ic_soc_h;
	int ic_soc_l;
	int ui_soc;
	int ui_remainder;
	int temp;
	unsigned int cache_reg_temp_en;
	int curr;
	int cycle;
	int soh;
	int fw_version;
	bool print_error_flag;
	int ground_loop_comp_en;
	int en_soft_reset;
	int en_profile_update;
	int ibat_record[CW2217_RECORD_NUM];
	int vbat_record[CW2217_RECORD_NUM];
	int index;
	struct glc_temp_comp_data glc_data;
	struct cw2217_cached_reg_data cached_reg_data;
	struct cw2217_vat_soc_pair vbat_soc_pair[CW2217_VBAT_SOC_TABLE_ROW];
	u32 adc_sample_type;
};

static int g_cw2217_t_r_table[][2] = {
	{ -500, 366429 }, { -490, 344575 }, { -480, 324180 }, { -470, 305134 },
	{ -460, 287341 }, { -450, 270710 }, { -440, 255156 }, { -430, 240604 },
	{ -420, 226981 }, { -410, 214223 }, { -400, 202269 }, { -390, 191064 },
	{ -380, 180555 }, { -370, 170695 }, { -360, 161439 }, { -350, 152747 },
	{ -340, 144581 }, { -330, 136905 }, { -320, 129688 }, { -310, 122899 },
	{ -300, 116509 }, { -290, 110493 }, { -280, 104827 }, { -270, 99488 },
	{ -260, 94456 }, { -250, 89710 }, { -240, 85233 }, { -230, 81008 },
	{ -220, 77019 }, { -210, 73252 }, { -200, 69693 }, { -190, 66329 },
	{ -180, 63149 }, { -170, 60140 }, { -160, 57294 }, { -150, 54560 },
	{ -140, 52049 }, { -130, 49633 }, { -120, 47344 }, { -110, 45174 },
	{ -100, 43117 }, { -90, 41166 }, { -80, 39315 }, { -70, 37559 },
	{ -60, 35891 }, { -50, 34307 }, { -40, 32803 }, { -30, 31373 },
	{ -20, 30015 }, { -10, 28723 }, { 0, 27494 }, { 10, 26325 },
	{ 20, 25212 }, { 30, 24153 }, { 40, 23144 }, { 50, 22184 },
	{ 60, 21268 }, { 70, 20396 }, { 80, 19564 }, { 90, 18771 },
	{ 100, 18015 }, { 110, 17294 }, { 120, 16605 }, { 130, 15948 },
	{ 140, 15320 }, { 150, 14720 }, { 160, 14148 }, { 170, 13600 },
	{ 180, 13077 }, { 190, 12577 }, { 200, 12099 }, { 210, 11641 },
	{ 220, 11204 }, { 230, 10785 }, { 240, 10384 }, { 250, 10000 },
	{ 260, 9632 }, { 270, 9280 }, { 280, 8943 }, { 290, 8620 },
	{ 300, 8310 }, { 310, 8012 }, { 320, 7728 }, { 330, 7454 },
	{ 340, 7192 }, { 350, 6940 }, { 360, 6699 }, { 370, 6467 },
	{ 380, 6244 }, { 390, 6030 }, { 400, 5825 }, { 410, 5628 },
	{ 420, 5438 }, { 430, 5256 }, { 440, 5081 }, { 450, 4912 },
	{ 460, 4750 }, { 470, 4594 }, { 480, 4444 }, { 490, 4230 },
	{ 500, 4161 }, { 510, 4027 }, { 520, 3898 }, { 530, 3774 },
	{ 540, 3654 }, { 550, 3539 }, { 560, 3428 }, { 570, 3322 },
	{ 580, 3219 }, { 590, 3119 }, { 600, 3023 }, { 610, 2931 },
	{ 620, 2842 }, { 630, 2756 }, { 640, 2673 }, { 650, 2593 },
	{ 660, 2516 }, { 670, 2441 }, { 680, 2369 }, { 690, 2299 },
	{ 700, 2232 }, { 710, 2167 }, { 720, 2105 }, { 730, 2044 },
	{ 740, 1985 }, { 750, 1929 }, { 760, 1874 }, { 770, 1821 },
	{ 780, 1770 }, { 790, 1720 }, { 800, 1673 }, { 810, 1626 },
	{ 820, 1581 }, { 830, 1538 }, { 840, 1496 }, { 850, 1455 },
	{ 860, 1416 }, { 870, 1378 }, { 880, 1341 }, { 890, 1305 },
	{ 900, 1270 }, { 910, 1237 }, { 920, 1204 }, { 930, 1173 },
	{ 940, 1142 }, { 950, 1113 }, { 960, 1084 }, { 970, 1056 },
	{ 980, 1029 }, { 990, 1003 }, { 1000, 977 }, { 0, 0 },
};

enum {
	CW2217_IC_TYPE_MAIN = 0,
	CW2217_IC_TYPE_AUX,
	CW2217_IC_TYPE_MAX,
};

enum cw2217_soc_mapping_para {
	CW2217_SOC_MAPPING_VTERM = 0,
	CW2217_SOC_MAPPING_K,
	CW2217_SOC_MAPPING_B,
	CW2217_SOC_MAPPING_TOTAL,
};

enum cw2217_vat_soc_pair_info {
	CW2217_PAIR_VOLT_MIN = 0,
	CW2217_PAIR_VOLT_MAX,
	CW2217_PAIR_SOC_U,
	CW2217_PAIR_TOTAL,
};

static int cw2217_read_byte(struct i2c_client *client, u8 reg, u8 *value)
{
	if (!client) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(client, reg, value);
}

static int cw2217_write_byte(struct i2c_client *client, u8 reg, u8 value)
{
	if (!client) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(client, reg, value);
}

static int cw2217_read_word(struct i2c_client *client, u8 reg, u16 *value)
{
	u8 cmd[CW2217_LEN_U8_REG] = { 0 };
	u8 buf0[CW2217_LEN_U16_DAT] = { 0 };
	u8 buf1[CW2217_LEN_U16_DAT] = { 0 };

	cmd[0] = reg; /* cmd[0]: 8bit register */

	if (!client) {
		hwlog_err("chip not init\n");
		return -ENODEV;
	}

	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf0, sizeof(buf0)))
		return -EIO;
	usleep_range(500, 600); /* delay ranges from 500 us to 600 us */
	if (power_i2c_read_block(client, cmd, sizeof(cmd), buf1, sizeof(buf1)))
		return -EIO;

	if (buf0[0] != buf1[0]) /* high-8bit data not equal can make sure buf1 is right data */
		*value = (buf1[1] | (buf1[0] << 8)); /* buf1 is the right data */
	else
		*value = (buf0[1] | (buf0[0] << 8)); /* buf0 is the right data */

	return 0;
}

/* write profile function */
static int cw2217_write_profile(struct i2c_client *client, u8 size,
	struct cw2217_dev *di)
{
	int ret;
	int i;

	for (i = 0; i < size; i++) {
		ret = cw2217_write_byte(client, CW2217_REG_BAT_PROFILE + i,
			di->config_profile_info[i]);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static int cw2217_parse_ocv_table(struct device *dev, struct device_node *np,
	struct cw2217_dev *di)
{
	int i;
	int ret;
	u32 ocv_table_data[CW2217_SIZE_OF_PROFILE] = { 0 };

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"cw,fg_ocv_table",
		ocv_table_data, CW2217_SIZE_OF_PROFILE);
	if (ret < 0)
		return ret;

	for (i = 0; i < CW2217_SIZE_OF_PROFILE; i++)
		di->config_profile_info[i] = (u8)ocv_table_data[i];

	return 0;
}

static struct device_node *cw2217_get_child_node(struct device *dev)
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
			hwlog_info("childnode without batt_name property");
			continue;
		}
		if (!battery_name)
			continue;
		if (!default_node)
			default_node = child_node;
		hwlog_info("search battery data, battery_name: %s\n", battery_name);
		if (!batt_model_name || !strcmp(battery_name, batt_model_name))
			break;
	}

	if (!child_node) {
		if (default_node) {
			hwlog_info("cannt match childnode, use first\n");
			child_node = default_node;
		} else {
			hwlog_info("cannt find any childnode, use father\n");
			child_node = np;
		}
	}

	return child_node;
}

static void cw2217_parse_soc_mapping_table(struct device_node *np,
	struct cw2217_dev *di)
{
	int array_len;
	int i;
	int idata = 0;
	const char *string = NULL;
	int row;
	int col;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"soc_mapping_table", CW2217_SOC_MAPPING_ROW, CW2217_SOC_MAPPING_COL);
	if (array_len <= 0) {
		di->soc_mapping_len = CW2217_SOC_MAPPING_DEFAULT_LEN;
		di->soc_mapping[0][CW2217_SOC_MAPPING_K] = CW2217_SOC_MAPPING_DEFAULT_K;
		di->soc_mapping[0][CW2217_SOC_MAPPING_B] = 0;
		hwlog_err("failed to get soc_mapping_table\n");
		return;
	}
	di->soc_mapping_len = array_len / CW2217_SOC_MAPPING_COL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "soc_mapping_table", i, &string))
			return;

		if (kstrtoint(string, POWER_BASE_DEC, &idata))
			return;

		col = i % CW2217_SOC_MAPPING_TOTAL;
		row = i / CW2217_SOC_MAPPING_TOTAL;

		switch (col) {
		case CW2217_SOC_MAPPING_VTERM:
			di->soc_mapping[row][col] = idata;
			break;
		case CW2217_SOC_MAPPING_K:
			di->soc_mapping[row][col] = idata;
			break;
		case CW2217_SOC_MAPPING_B:
			di->soc_mapping[row][col] = idata;
			break;
		default:
			break;
		}

		hwlog_info("soc_mapping[%d][%d]=%d\n", row, col, idata);
	}
}

static void cw2217_parse_vbat_soc_pair_table(struct device_node *np,
	struct cw2217_dev *di)
{
	int row;
	int col;
	int len;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"vbat_soc_table", (int *)di->vbat_soc_pair, CW2217_VBAT_SOC_TABLE_ROW, CW2217_PAIR_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / CW2217_PAIR_TOTAL; row++)
		hwlog_info("vbat_soc_table[%d]=%d %d %d\n", row,
			di->vbat_soc_pair[row].volt_min,
			di->vbat_soc_pair[row].volt_max,
			di->vbat_soc_pair[row].soc_u);
}

static int cw2217_parse_dt(struct device *dev, struct cw2217_dev *di)
{
	int ret;
	struct device_node *np = dev->of_node;
	struct device_node *child_node = cw2217_get_child_node(dev);

	if (!child_node)
		return -ENOMEM;

	ret = cw2217_parse_ocv_table(dev, child_node, di);
	if (ret < 0)
		return ret;

	cw2217_parse_soc_mapping_table(child_node, di);
	cw2217_parse_vbat_soc_pair_table(np, di);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ic_role",
		&di->ic_role, CW2217_IC_TYPE_MAIN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"cw,resistance", (u32 *)&di->resistance, CW2217_RESISTANCE_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"cw,rated_capacity", (u32 *)&di->rated_capacity, CW2217_RATED_CAPACITY_VAL);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ground_loop_comp_en", (u32 *)&di->ground_loop_comp_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"en_soft_reset", (u32 *)&di->en_soft_reset, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_vpullup",
		(u32 *)&di->glc_data.vpullup, 1280); /* default pull-up voltage 1280 mV */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_rpullup",
		(u32 *)&di->glc_data.rpullup, 40000); /* default pull-up r 40 KOhm */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ground_loop_comp_rcomp",
		(u32 *)&di->glc_data.rcomp, 30); /* default compensate r 3.0 mOhm */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ir_comp_en",
		&di->ir_comp_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "compensation_r",
		&di->compensation_r, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "cache_reg_temp_en",
		&di->cache_reg_temp_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "en_profile_update",
		&di->en_profile_update, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "adc_sample_type",
		&di->adc_sample_type, 0);

	return 0;
}

/*
 * Active function
 * The CONFIG register is used for the host MCU to configure the fuel gauge IC. The default value is 0xF0,
 * SLEEP and RESTART bits are set. To power up the IC, the host MCU needs to write 0x30 to exit shutdown
 * mode, and then write 0x00 to restart the gauge to enter active mode. To reset the IC, the host MCU needs
 * to write 0xF0, 0x30 and 0x00 in sequence to this register to complete the restart procedure. The CW2217B
 * will reload relevant parameters and settings and restart SOC calculation. Note that the SOC may be a
 * different value after reset operation since it is a brand-new calculation based on the latest battery status.
 * CONFIG [3:0] is reserved. Don't do any operation with it.
 */
static int cw2217_enter_active_mode(struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = CW2217_CONFIG_MODE_RESTART;

	ret = cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG, reg_val);
	if (ret < 0)
		return ret;
	msleep(DT_MSLEEP_20MS);

	reg_val = CW2217_CONFIG_MODE_ACTIVE;
	ret = cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG, reg_val);
	if (ret < 0)
		return ret;
	msleep(DT_MSLEEP_10MS);

	return 0;
}

/*
 * Sleep function
 * The CONFIG register is used for the host MCU to configure the fuel gauge IC. The default value is 0xF0,
 * SLEEP and RESTART bits are set. To power up the IC, the host MCU needs to write 0x30 to exit shutdown
 * mode, and then write 0x00 to restart the gauge to enter active mode. To reset the IC, the host MCU needs
 * to write 0xF0, 0x30 and 0x00 in sequence to this register to complete the restart procedure. The CW2217B
 * will reload relevant parameters and settings and restart SOC calculation. Note that the SOC may be a
 * different value after reset operation since it is a brand-new calculation based on the latest battery status.
 * CONFIG [3:0] is reserved. Don't do any operation with it.
 */
static int cw2217_enter_sleep_mode(struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = CW2217_CONFIG_MODE_RESTART;

	ret = cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG, reg_val);
	if (ret < 0)
		return ret;
	msleep(DT_MSLEEP_20MS);

	reg_val = CW2217_CONFIG_MODE_SLEEP;
	ret = cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG, reg_val);
	if (ret < 0)
		return ret;
	msleep(DT_MSLEEP_10MS);

	return 0;
}

/*
 * The 0x00 register is an UNSIGNED 8bit read-only register. Its value is fixed to 0xA0 in shutdown
 * mode and active mode.
 */
static int cw2217_get_chip_id(int *value, struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = 0;
	int chip_id;

	ret = cw2217_read_byte(di->client, CW2217_REG_CHIP_ID, &reg_val);
	if (ret < 0)
		return ret;

	chip_id = reg_val; /* This value must be 0xA0 */
	hwlog_info("chip_id = %d\n", chip_id);
	*value = chip_id;

	return 0;
}

static int cw2217_update_chip_id(struct cw2217_dev *di)
{
	int ret;
	int chip_id = 0;

	ret = cw2217_get_chip_id(&chip_id, di);
	if (ret < 0)
		return ret;
	di->chip_id = chip_id;

	return 0;
}

static int cw2217_read_chip_id(struct cw2217_dev *di)
{
	int ret = cw2217_update_chip_id(di);
	if (ret < 0) {
		hwlog_err("iic read write error");
		return ret;
	}

	if (di->chip_id != CW2217_IC_VCHIP_ID) {
		hwlog_err("not cw2217B\n");
		return -EPERM;
	}

	return 0;
}

static int cw2217_check_chip(struct cw2217_dev *di)
{
	int retry = CW2217_RETRY_COUNT;
	int ret;

	do {
		ret = cw2217_read_chip_id(di);
		if (!ret)
			return 0;
	} while (retry-- > 0);
	if (ret)
		hwlog_err("chek chip fail\n");

	return ret;
}

/*
 * The VCELL register(0x02 0x03) is an UNSIGNED 14bit read-only register that updates the battery voltage continuously.
 * Battery voltage is measured between the VCELL pin and VSS pin, which is the ground reference. A 14bit
 * sigma-delta A/D converter is used and the voltage resolution is 312.5uV. (0.3125mV is *5/16)
 */
static int cw2217_get_voltage(int *value, struct cw2217_dev *di)
{
	int ret;
	u16 voltage = 0;

	ret = cw2217_read_word(di->client, CW2217_REG_VCELL_H, &voltage);
	if (ret < 0)
		return ret;

	voltage = voltage * CW2217_VOL_MAGIC_PART1 / CW2217_VOL_MAGIC_PART2;
	*value = voltage;

	return 0;
}

int cw2217_update_voltage(struct cw2217_dev *di)
{
	int ret;
	int voltage = 0;

	ret = cw2217_get_voltage(&voltage, di);
	if (ret < 0)
		return ret;
	di->voltage = voltage;

	return 0;
}

static void cw2217_calc_soc_mapping(struct cw2217_dev *di, int *ui_soc, int *remainder)
{
	int total_cap;

	total_cap = *ui_soc * CW2217_SOC_MAGIC_100 + *remainder;
	hwlog_info("before total_cap = %d\n", total_cap);
	total_cap = total_cap * CW2217_SOC_MAPPING_DEFAULT_K /
		di->soc_mapping[di->ocv_idx][CW2217_SOC_MAPPING_K] -
		di->soc_mapping[di->ocv_idx][CW2217_SOC_MAPPING_B];
	hwlog_info("after total_cap = %d k = %d\n",
		total_cap, di->soc_mapping[di->ocv_idx][CW2217_SOC_MAPPING_K]);
	*ui_soc = total_cap / CW2217_SOC_MAGIC_100;
	*remainder = total_cap % CW2217_SOC_MAGIC_100;
}

static void cw2217_basp_soc_mapping_strategy(struct cw2217_dev *di, int vterm)
{
	int row;

	if ((vterm <= 0) || (di->soc_mapping_len <= 0)) {
		di->ocv_idx = 0;
		return;
	}
	for (row = 0; row < di->soc_mapping_len; row++) {
		if (vterm <= di->soc_mapping[row][CW2217_SOC_MAPPING_VTERM]) {
			di->ocv_idx = row;
			hwlog_info("set ocv_idx = %d\n", di->ocv_idx);
			return;
		}
	}
	if (row >= di->soc_mapping_len)
		di->ocv_idx = di->soc_mapping_len - 1;
}

/*
 * The SOC register(0x04 0x05) is an UNSIGNED 16bit read-only register that indicates the SOC of the battery. The
 * SOC shows in % format, which means how much percent of the battery's total available capacity is
 * remaining in the battery now. The SOC can intrinsically adjust itself to cater to the change of battery status,
 * including load, temperature and aging etc.
 * The high byte(0x04) contains the SOC in 1% unit which can be directly used if this resolution is good
 * enough for the application. The low byte(0x05) provides more accurate fractional part of the SOC and its
 * LSB is (1/256) %.
 */
static int cw2217_get_capacity(int *value, struct cw2217_dev *di)
{
	int ret;
	u16 reg_val = 0;
	int soc_h;
	int soc_l;
	int ui_soc;
	int remainder;

	ret = cw2217_read_word(di->client, CW2217_REG_SOC_INT, &reg_val);
	if (ret < 0)
		return ret;
	soc_h = (reg_val & POWER_MASK_HIGH_BYTE) >> 8;
	soc_l = reg_val & POWER_MASK_BYTE;
	ui_soc = ((soc_h * CW2217_SOC_MAGIC_BASE + soc_l) * CW2217_SOC_MAGIC_100) /
		(CW2217_UI_FULL * CW2217_SOC_MAGIC_BASE);
	remainder = (((soc_h * CW2217_SOC_MAGIC_BASE + soc_l) * CW2217_SOC_MAGIC_100 *
		CW2217_SOC_MAGIC_100) / (CW2217_UI_FULL * CW2217_SOC_MAGIC_BASE)) % CW2217_SOC_MAGIC_100;
	if (ui_soc >= CW2217_SOC_MAGIC_100) {
		hwlog_info("UI_SOC = %d larger 100\n", ui_soc);
		ui_soc = CW2217_SOC_MAGIC_100;
	}
	di->ic_soc_h = soc_h;
	di->ic_soc_l = soc_l;

	cw2217_calc_soc_mapping(di, &ui_soc, &remainder);
	*value = ui_soc;
	di->ui_remainder = remainder;

	return 0;
}

static int cw2217_update_ui_soc(struct cw2217_dev *di)
{
	int ret;
	int ui_soc = 0;

	ret = cw2217_get_capacity(&ui_soc, di);
	if (ret < 0)
		return ret;
	di->ui_soc = ui_soc;

	return 0;
}

static int cw2217_write_reg_capacity(struct cw2217_dev *di, int capacity)
{
	int ret;
	u8 reg_val = (u8)capacity;

	ret = cw2217_write_byte(di->client, CW2217_REG_USER_CONF, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static int cw2217_read_reg_capacity(struct cw2217_dev *di)
{
	int ret;
	int ui_soc;
	u8 reg_val = 0;

	ret = cw2217_read_byte(di->client, CW2217_REG_USER_CONF, &reg_val);
	if (ret < 0)
		return ret;
	ui_soc = (int)reg_val;

	return ui_soc;
}

static int cw2217_write_reg_ocv_idx(struct cw2217_dev *di, int ocv_idx)
{
	int ret;
	u8 reg_val = (u8)ocv_idx;

	ret = cw2217_write_byte(di->client, CW2217_REG_USER_CONF_EX, reg_val);
	if (ret < 0)
		return ret;

	return 0;
}

static int cw2217_read_reg_ocv_idx(struct cw2217_dev *di)
{
	int ret;
	int ocv_idx;
	u8 reg_val = 0;

	ret = cw2217_read_byte(di->client, CW2217_REG_USER_CONF_EX, &reg_val);
	if (ret < 0)
		return ret;
	ocv_idx = (int)reg_val;

	return ocv_idx;
}

static int cw2217_get_current(int *value, struct cw2217_dev *di);

static inline u8 cw2217_get_reg_temp_cache(struct cw2217_dev *di)
{
	hwlog_info("role = %u, get cached reg temp:%d", di->ic_role, (int)di->cached_reg_data.temp);
	return di->cached_reg_data.temp;
}

static inline void cw2217_set_reg_temp_cache(struct cw2217_dev *di, u8 reg_temp)
{
	di->cached_reg_data.temp = reg_temp;
}

/*
 * The TEMP register is an UNSIGNED 8bit read only register.
 * It reports the real-time battery temperature
 * measured at TS pin. The scope is from -40 to 87.5 degrees Celsius,
 * LSB is 0.5 degree Celsius. TEMP(C) = - 40 + Value(0x06 Reg) / 2
 */
static int cw2217_get_temp(int *value, struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = 0;
	int temp;
	int fake_rntc;

	ret = cw2217_read_byte(di->client, CW2217_REG_TEMP, &reg_val);
	if (ret < 0)
		hwlog_info("get temp ret:%d fail", ret);

	if (di->cache_reg_temp_en) {
		if (ret < 0)
			reg_val = cw2217_get_reg_temp_cache(di);
		else
			cw2217_set_reg_temp_cache(di, reg_val);
	} else {
		if (ret < 0)
			return ret;
	}

	temp = (int)reg_val * CW2217_TEMP_MAGIC_PART1 / CW2217_TEMP_MAGIC_PART2 - CW2217_TEMP_MAGIC_PART3;

	if (di->ground_loop_comp_en) {
		cw2217_get_current(&di->glc_data.ibat, di); /* mA */
		fake_rntc = power_lookup_table_linear_trans_dichotomy(g_cw2217_t_r_table,
			CW2217_T_R_ARRAY_LEN, temp, 0); /* Ohm */
		di->glc_data.vadc = DIV_ROUND_CLOSEST(di->glc_data.vpullup * fake_rntc,
			fake_rntc + di->glc_data.rpullup); /* mV */
		hwlog_info("ibat%d, vadc%d, vpullup%d, rpullup%d, rcomp%d, temp%d\n", di->glc_data.ibat,
			di->glc_data.vadc, di->glc_data.vpullup, di->glc_data.rpullup, di->glc_data.rcomp, temp);
		temp = ground_loop_compensate_get_temp(&di->glc_data);
	}
	*value = temp;

	return 0;
}

static int cw2217_update_temp(struct cw2217_dev *di)
{
	int ret;
	int temp = 0;

	ret = cw2217_get_temp(&temp, di);
	if (ret < 0)
		return ret;
	di->temp = temp;

	return 0;
}

/* get complement code function, unsigned short must be U16 */
static long cw2217_get_complement_code(unsigned short raw_code)
{
	long complement_code;
	int dir;

	if ((raw_code & CW2217_COMPLEMENT_CODE_U16) != 0) {
		dir = -1;
		raw_code =  (~raw_code) + 1;
	} else {
		dir = 1;
	}
	complement_code = (long)raw_code * dir;

	return complement_code;
}

/*
 * CURRENT is a SIGNED 16bit register(0x0E 0x0F) that reports current A/D converter result of the voltage across the
 * current sense resistor, 10mohm typical. The result is stored as a two's complement value to show positive
 * and negative current. Voltages outside the minimum and maximum register values are reported as the
 * minimum or maximum value.
 * The register value should be divided by the sense resistance to convert to amperes. The value of the
 * sense resistor determines the resolution and the full-scale range of the current readings. The LSB of 0x0F
 * is (52.4/32768)uV.
 * The default value is 0x0000, stands for 0mA. 0x7FFF stands for the maximum charging current and 0x8001 stands for
 * the maximum discharging current.
 */
static int cw2217_get_current(int *value, struct cw2217_dev *di)
{
	int ret;
	s64 curr;
	u16 current_reg = 0;

	ret = cw2217_read_word(di->client, CW2217_REG_CURRENT_H, &current_reg);
	if (ret < 0)
		return ret;

	curr = cw2217_get_complement_code(current_reg);
	curr = curr * 1600 / di->resistance; /* I(A) = 0.0016(uV) * value / resistance(mR) */
	curr = curr * di->coefficient / CW2217_COEFFICIENT_DEFAULT_VAL;
	*value = curr;

	return 0;
}

static int cw2217_update_current(struct cw2217_dev *di)
{
	int ret;
	int curr = 0;

	ret = cw2217_get_current(&curr, di);
	if (ret < 0)
		return ret;
	di->curr = curr;

	return 0;
}

/*
 * CYCLECNT is an UNSIGNED 16bit register(0xA4 0xA5) that counts cycle life of the battery. The LSB of 0xA5 stands
 * for 1/16 cycle. This register will be clear after enters shutdown mode
 */
static int cw2217_get_cycle_count(int *value, struct cw2217_dev *di)
{
	int ret;
	u16 cycle = 0;
	u8 cycle_base = 0;

	ret = cw2217_read_word(di->client, CW2217_REG_CYCLE_H, &cycle);
	if (ret < 0)
		return ret;
	*value = cycle / CW2217_CYCLE_MAGIC;

	if (di->en_soft_reset || di->en_profile_update) {
		ret = cw2217_read_byte(di->client, CW2217_REG_CYCLE_BASE, &cycle_base);
		if (ret < 0)
			return ret;
		*value += cycle_base * CW2217_CYCLE_BASE_CONVERT;
	}

	return 0;
}

static int cw2217_update_cycle_count(struct cw2217_dev *di)
{
	int ret;
	int cycle_count = 0;

	ret = cw2217_get_cycle_count(&cycle_count, di);
	if (ret < 0)
		return ret;
	di->cycle = cycle_count;

	return 0;
}

/*
 * SOH (State of Health) is an UNSIGNED 8bit register(0xA6) that represents the level of battery aging by tracking
 * battery internal impedance increment. When the device enters active mode, this register refresh to 0x64
 * by default. Its range is 0x00 to 0x64, indicating 0 to 100%. This register will be clear after enters shutdown
 * mode.
 */
static int cw2217_get_soh(int *value, struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = 0;
	int soh;

	ret = cw2217_read_byte(di->client, CW2217_REG_SOH, &reg_val);
	if (ret < 0)
		return ret;

	soh = reg_val;
	*value = soh;

	return 0;
}

static int cw2217_update_soh(struct cw2217_dev *di)
{
	int ret;
	int soh = 0;

	ret = cw2217_get_soh(&soh, di);
	if (ret < 0)
		return ret;
	di->soh = soh;

	return 0;
}

/*
 * FW_VERSION register reports the firmware (FW) running in the chip. It is fixed to 0x00 when the chip is
 * in shutdown mode. When in active mode, Bit [7:6] are fixed to '01', which stand for the CW2217B and Bit
 * [5:0] stand for the FW version running in the chip. Note that the FW version is subject to update and contact
 * sales office for confirmation when necessary.
 */
static int cw2217_get_fw_version(int *value, struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = 0;
	int fw_version;

	ret = cw2217_read_byte(di->client, CW2217_REG_FW_VERSION, &reg_val);
	if (ret < 0)
		return ret;

	fw_version = reg_val;
	*value = fw_version;

	return 0;
}

static int cw2217_update_fw_version(struct cw2217_dev *di)
{
	int ret;
	int fw_version = 0;

	ret = cw2217_get_fw_version(&fw_version, di);
	if (ret < 0)
		return ret;
	di->fw_version = fw_version;

	return 0;
}

static int cw2217_update_data(struct cw2217_dev *di)
{
	int ret = 0;

	ret += cw2217_update_voltage(di);
	ret += cw2217_update_ui_soc(di);
	ret += cw2217_update_temp(di);
	ret += cw2217_update_current(di);
	ret += cw2217_update_cycle_count(di);
	ret += cw2217_update_soh(di);
	hwlog_info("vol = %d  current = %d cap = %d temp = %d\n",
		di->voltage, di->curr, di->ui_soc, di->temp);

	return ret;
}

static int cw2217_init_data(struct cw2217_dev *di)
{
	int ret = 0;

	ret += cw2217_update_chip_id(di);
	ret += cw2217_update_voltage(di);
	ret += cw2217_update_ui_soc(di);
	ret += cw2217_update_temp(di);
	ret += cw2217_update_current(di);
	ret += cw2217_update_cycle_count(di);
	ret += cw2217_update_soh(di);
	ret += cw2217_update_fw_version(di);
	hwlog_info("chip_id = %d vol = %d  cur = %d cap = %d temp = %d  fw_version = %d\n",
		di->chip_id, di->voltage, di->curr,
		di->ui_soc, di->temp, di->fw_version);

	return ret;
}

/* CW2217 update profile function, Often called during initialization */
static int cw2217_config_start_ic(struct cw2217_dev *di)
{
	int ret;
	u8 reg_val;
	int count = 0;

	ret = cw2217_enter_sleep_mode(di);
	if (ret < 0)
		return ret;

	/* update new battery info */
	ret = cw2217_write_profile(di->client, CW2217_SIZE_OF_PROFILE, di);
	if (ret < 0)
		return ret;

	/* set update flag and soc interrupt value */
	reg_val = CW2217_CONFIG_UPDATE_FLG | CW2217_GPIO_SOC_IRQ_VALUE;
	ret = cw2217_write_byte(di->client, CW2217_REG_SOC_ALERT, reg_val);
	if (ret < 0)
		return ret;

	/* close all interruptes */
	reg_val = 0;
	ret = cw2217_write_byte(di->client, CW2217_REG_GPIO_CONFIG, reg_val);
	if (ret < 0)
		return ret;

	ret = cw2217_enter_active_mode(di);
	if (ret < 0)
		return ret;

	while (true) {
		msleep(DT_MSLEEP_100MS);
		cw2217_read_byte(di->client, CW2217_REG_IC_STATE, &reg_val);
		if ((reg_val & CW2217_IC_READY_MARK) == CW2217_IC_READY_MARK)
			break;
		count++;
		if (count >= CW2217_SLEEP_COUNT) {
			cw2217_enter_sleep_mode(di);
			return -EPERM;
		}
	}

	return 0;
}

/*
 * Get the cw2217 running state
 * Determine whether the profile needs to be updated
 */
static int cw2217_get_state(struct cw2217_dev *di)
{
	int ret;
	u8 reg_val = 0;
	int i;
	int reg_profile;

	ret = cw2217_read_byte(di->client, CW2217_REG_MODE_CONFIG, &reg_val);
	if (ret < 0)
		return ret;
	if (reg_val != CW2217_CONFIG_MODE_ACTIVE)
		return CW2217_NOT_ACTIVE;

	ret = cw2217_read_byte(di->client, CW2217_REG_SOC_ALERT, &reg_val);
	if (ret < 0)
		return ret;
	if ((reg_val & CW2217_CONFIG_UPDATE_FLG) == 0x00)
		return CW2217_PROFILE_NOT_READY;

	if (power_cmdline_is_erecovery_mode() || power_cmdline_is_recovery_mode()) {
		hwlog_info("recovery_mode not update profile\n");
		return 0;
	}

	for (i = 0; i < CW2217_SIZE_OF_PROFILE; i++) {
		ret = cw2217_read_byte(di->client, (CW2217_REG_BAT_PROFILE + i),
			&reg_val);
		if (ret < 0)
			return ret;
		reg_profile = CW2217_REG_BAT_PROFILE + i;
		hwlog_info("0x%2x = 0x%2x\n", reg_profile, reg_val);
		if (di->config_profile_info[i] != reg_val)
			break;
	}
	if (i < CW2217_SIZE_OF_PROFILE)
		return CW2217_PROFILE_NEED_UPDATE;

	return 0;
}

static void cw2217_init_calibration_para(struct cw2217_dev *di)
{
	int curr_gain = 0;

	/* main battery gauge use aux calibration data for compatible */
	if (di->ic_role == CW2217_IC_TYPE_MAIN)
		coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &curr_gain);
	else
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_A, &curr_gain);

	if (curr_gain) {
		if (curr_gain < CW2217_TBATICAL_MIN_A)
			curr_gain = CW2217_TBATICAL_MIN_A;
		else if (curr_gain > CW2217_TBATICAL_MAX_A)
			curr_gain = CW2217_TBATICAL_MAX_A;
		di->coefficient = curr_gain;
	} else {
		di->coefficient = CW2217_COEFFICIENT_DEFAULT_VAL;
	}
	hwlog_info("coefficient = %u\n", di->coefficient);
}

static int cw2217_calibrate_rsense(int coefficient)
{
	int value;
	int round_mark = 500000; /* 500000: used for rounding */

	if (coefficient == CW2217_COEFFICIENT_DEFAULT_VAL) {
		hwlog_info("Calibration is not required\n");
		return -EPERM;
	}
	value = coefficient - CW2217_COEFFICIENT_DEFAULT_VAL;
	value = 1024 * value; /* multiply by 1024 to avoid floating point arithmetic */
	if (value < 0)
		round_mark = 0 - round_mark;

	value = (value + round_mark) / CW2217_COEFFICIENT_DEFAULT_VAL;
	if ((value > CW2217_RSENSE_CORRECT_UP) || (value < CW2217_RSENSE_CORRECT_DOWN) || (value == 0)) {
		hwlog_info("Calibration is error\n");
		return -EPERM;
	} else if (value < 0) {
		value = 256 + value; /* to get a positive number */
	}

	return value;
}

static int cw2217_calculate_crc_value(struct cw2217_dev *di)
{
	unsigned char crcvalue = 0x55; /* 0x55: crc odd check initial value */
	int i;
	int j;
	unsigned char temp_profile_one;

	for (i = 0; i < CW2217_SIZE_OF_PROFILE - 1; i++) {
		temp_profile_one = di->config_profile_info[i];
		crcvalue = crcvalue ^ temp_profile_one;
		for (j = 0; j < 8; j++) { /* 8: one byte */
			if ((crcvalue & 0x80)) {
				crcvalue <<= 1;
				crcvalue ^= 0x07;
			} else {
				crcvalue <<= 1;
			}
		}
	}

	return crcvalue;
}

static void cw2217_calibrate_profile(struct cw2217_dev *di)
{
	int ret;
	unsigned char byte7_value;
	unsigned char byte80_value;

	cw2217_init_calibration_para(di);

	ret = cw2217_calibrate_rsense(di->coefficient);
	if (ret < 0)
		return;
	byte7_value = ret;
	di->config_profile_info[6] = byte7_value; /* 6: the seventh element of the array needs to be updated */
	byte80_value = cw2217_calculate_crc_value(di);
	di->config_profile_info[79] = byte80_value; /* 79: the eighth element of the array needs to be updated */
	hwlog_info("calibration success\n");
}

/* init function, often called during initialization */
static int cw2217_config(struct cw2217_dev *di)
{
	int ret;
	int i;
	int idx;
	int ret_cycle;
	int update_cycle;
	bool need_update = false;

	idx = cw2217_read_reg_ocv_idx(di);
	if ((idx >= 0) && (idx < di->soc_mapping_len))
		di->ocv_idx = idx;
	else
		di->ocv_idx = 0;

	for (i = 0; i < CW2217_SIZE_OF_PROFILE; i++)
		hwlog_info("config_profile_info[%d] = 0x%x\n", i, di->config_profile_info[i]);
	cw2217_calibrate_profile(di);

	ret = cw2217_get_state(di);
	hwlog_info("role = %u, get_state = %d\n", di->ic_role, ret);
	if (ret < 0) {
		hwlog_err("iic read write error");
		return ret;
	}

	if (di->en_profile_update && (ret == CW2217_PROFILE_NEED_UPDATE)) {
		ret_cycle = cw2217_get_cycle_count(&update_cycle, di);
		if (ret_cycle != 0)
			return ret_cycle;
		need_update = true;
	}

	if (ret != 0) {
		ret = cw2217_config_start_ic(di);
		if (ret < 0)
			return ret;
	}

	if (need_update) {
		ret = cw2217_write_byte(di->client, CW2217_REG_CYCLE_BASE, update_cycle / CW2217_CYCLE_BASE_CONVERT);
		if (ret < 0)
			return ret;
	}

	hwlog_info("init success\n");
	return 0;
}

static void cw2217_capacity_error_detect(struct cw2217_dev *di)
{
	int i;
	int ret;
	u8 reg_val = 0;
	int reg_profile;

	if (di->ui_soc > CW2217_FLAG_UPDATE_SOC && !di->print_error_flag) {
		di->print_error_flag = true;
	} else if (di->ui_soc < CW2217_ERROR_UI_SOC && di->voltage > CW2217_ERROR_VOLTAGE &&
		di->print_error_flag) {
		for (i = 0; i < CW2217_SIZE_OF_PROFILE; i++) {
			ret = cw2217_read_byte(di->client, (CW2217_REG_BAT_PROFILE + i), &reg_val);
			if (ret < 0)
				return;
			reg_profile = CW2217_REG_BAT_PROFILE + i;
			hwlog_info("reg[0x%2x] = 0x%2x", reg_profile, reg_val);
		}
		di->print_error_flag = false;
	}
}

static int get_soc_u(int vbat, struct cw2217_dev *di)
{
	int i = 0;

	for (; i < CW2217_VBAT_SOC_TABLE_ROW; i++) {
		if ((vbat > di->vbat_soc_pair[i].volt_min) &&
			(vbat <= di->vbat_soc_pair[i].volt_max))
			return di->vbat_soc_pair[i].soc_u;
	}

	return -1;
}

static void cw2217_reg_reset(struct cw2217_dev *di)
{
	u8 reg_val = 0;
	int ret;

	cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG,
		CW2217_CONFIG_MODE_SLEEP);
	msleep(DT_MSLEEP_20MS);
	cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG,
		CW2217_CONFIG_MODE_RESTART);
	msleep(DT_MSLEEP_20MS);
	cw2217_write_byte(di->client, CW2217_REG_MODE_CONFIG,
		CW2217_CONFIG_MODE_ACTIVE);
	msleep(DT_MSLEEP_20MS);
	ret = cw2217_read_byte(di->client, CW2217_REG_MODE_CONFIG, &reg_val);
	if (ret < 0)
		hwlog_info("read 0x8 fail, reg_val=%d\n", reg_val);
	hwlog_info("%s done, read 0x8 success reg_val=%d\n", __func__, reg_val);
}

static void cw2217_soft_reset(struct cw2217_dev *di)
{
	int min_vbat;
	int soc_u;
	u8 need_reset = 0;
	int history_cycle = di->cycle / CW2217_CYCLE_BASE_CONVERT;
	char buf[POWER_DSM_BUF_SIZE_0128] = { 0 };

	if (di->curr < CW2217_DISCHARGE_THRESHOLD) {
		di->ibat_record[di->index] = di->curr;
		di->vbat_record[di->index] = di->voltage;
		di->index++;
	} else {
		di->index = 0;
	}
	if (di->index >= CW2217_RECORD_NUM) {
		min_vbat = power_get_min_value(di->vbat_record, CW2217_RECORD_NUM);
		soc_u = get_soc_u(min_vbat, di);
		if ((soc_u > 0) && ((soc_u - di->ui_soc) > CW2217_MAX_SOC_DIFF))
			need_reset = 1;
		di->index = 0;
	}
	if (need_reset ||
		((di->ui_soc <= CW2217_FLAG_UPDATE_SOC) && (charge_get_done_type() == CHARGE_DONE))) {
		cw2217_reg_reset(di);
		(void)cw2217_write_byte(di->client, CW2217_REG_CYCLE_BASE,
			history_cycle > CW2217_MAX_U8_VALUE ? CW2217_MAX_U8_VALUE : history_cycle);
		hwlog_info("%s done, need_reset=%d, cycle_total = %d\n", __func__, need_reset, di->cycle);
		snprintf(buf, POWER_DSM_BUF_SIZE_0128 - 1,
			"cw2217 soft reset: workaround %d\n", need_reset + 1);
		power_dsm_report_dmd(POWER_DSM_BATTERY, DSM_FUELGAGUE_SOFT_RESET, buf);
	}
}

static void cw2217_bat_work(struct work_struct *work)
{
	struct delayed_work *delay_work = NULL;
	struct cw2217_dev *di = NULL;

	delay_work = container_of(work, struct delayed_work, work);
	if (!delay_work)
		return;
	di = container_of(delay_work, struct cw2217_dev, battery_delay_work);
	if (!di)
		return;

	cw2217_update_data(di);
	cw2217_capacity_error_detect(di);

	if (di->en_soft_reset)
		cw2217_soft_reset(di);

	queue_delayed_work(di->cwfg_workqueue, &di->battery_delay_work,
		msecs_to_jiffies(CW2217_QUEUE_DELAYED_WORK_TIME));
}

static int cw2217_get_log_head(char *buffer, int size, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di || !buffer)
		return -EPERM;

	if (di->ic_role == CW2217_IC_TYPE_MAIN)
		snprintf(buffer, size,
			" soc    soc_h  soc_l  voltage  current  temp   cycle  soh  ");
	else
		snprintf(buffer, size,
			" soc1    soc_h1  soc_l1  voltage1  current1  temp1   cycle1  soh1  ");

	return 0;
}

static int cw2217_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di || !buffer)
		return -EPERM;

	if (di->ic_role == CW2217_IC_TYPE_MAIN)
		snprintf(buffer, size, "%-7d%-7d%-7d%-9d%-9d%-7d%-7d%-4d   ",
			di->ui_soc, di->ic_soc_h, di->ic_soc_l,
			di->voltage, di->curr, di->temp, di->cycle, di->soh);
	else
		snprintf(buffer, size, "%-8d%-8d%-8d%-10d%-10d%-8d%-8d%-7d   ",
			di->ui_soc, di->ic_soc_h, di->ic_soc_l,
			di->voltage, di->curr, di->temp, di->cycle, di->soh);

	return 0;
}

static struct power_log_ops cw2217_log_ops = {
	.dev_name = "cw2217",
	.dump_log_head = cw2217_get_log_head,
	.dump_log_content = cw2217_dump_log_data,
};

static struct power_log_ops cw2217_aux_log_ops = {
	.dev_name = "cw2217_aux",
	.dump_log_head = cw2217_get_log_head,
	.dump_log_content = cw2217_dump_log_data,
};

static int cw2217_is_ready(void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	return 1;
}

#ifdef CONFIG_HLTHERM_RUNTEST
static int cw2217_is_battery_exist(void *dev_data)
{
	return 0;
}
#else
static int cw2217_is_battery_exist(void *dev_data)
{
	int ret;
	int temp = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_temp(&temp, di);
	if (ret < 0)
		return ret;

	if ((temp <= CW2217_TEMP_ABR_LOW) || (temp >= CW2217_TEMP_ABR_HIGH))
		return 0;

	return 1; /* battery is existing */
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int cw2217_read_battery_soc(void *dev_data)
{
	int ret;
	int ui_soc = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_capacity(&ui_soc, di);
	if (ret < 0)
		return ret;

	return ui_soc;
}

static int cw2217_read_battery_vol(void *dev_data)
{
	int ret;
	int cur = 0;
	int voltage = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_voltage(&voltage, di);
	if (ret < 0)
		return ret;

	if (di->ir_comp_en) {
		ret = cw2217_get_current(&cur, di);
		if (ret < 0)
			return ret;
		voltage -= cur * di->compensation_r / POWER_UV_PER_MV;
		hwlog_info("vbatt_comp=%d, cur=%d, compr=%d\n",
			voltage, cur, di->compensation_r);
	}

	return voltage;
}

static int cw2217_read_battery_current(void *dev_data)
{
	int ret;
	int curr = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_current(&curr, di);
	if (ret < 0)
		return ret;

	return curr;
}

static int cw2217_read_battery_avg_current(void *dev_data)
{
	int ret;
	int avg_curr = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_current(&avg_curr, di);
	if (ret < 0)
		return ret;

	return avg_curr;
}

static int cw2217_read_battery_temperature(void *dev_data)
{
	int ret;
	int temp = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_temp(&temp, di);
	if (ret < 0)
		return ret;

	return temp;
}

static int cw2217_read_battery_fcc(void *dev_data)
{
	int ret;
	int soh = 0;
	struct cw2217_dev *di = dev_data;
	int fcc;

	if (!di)
		return 0;

	ret = cw2217_get_soh(&soh, di);
	if (ret < 0)
		return ret;
	fcc = di->rated_capacity * soh / CW2217_SOH_DEFAULT_VAL;

	return fcc * di->soc_mapping[di->ocv_idx][CW2217_SOC_MAPPING_K] / CW2217_SOC_MAPPING_DEFAULT_K;
}

static int cw2217_read_battery_cycle(void *dev_data)
{
	int ret;
	int cycle = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_cycle_count(&cycle, di);
	if (ret < 0)
		return ret;

	return cycle;
}

static int cw2217_set_battery_low_voltage(int val, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di)
		return -EPERM;

	return 0;
}

static int cw2217_set_last_capacity(int capacity, void *dev_data)
{
	struct cw2217_dev *di = dev_data;
	int ret;

	if (!di || (capacity > CW2217_FULL_CAPACITY) || (capacity < 0))
		return 0;

	ret = cw2217_write_reg_capacity(di, capacity);
	ret += cw2217_write_reg_ocv_idx(di, di->ocv_idx);

	return ret;
}

static int cw2217_get_last_capacity(void *dev_data)
{
	int ret;
	int last_cap = 0;
	int cap = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return last_cap;

	last_cap = cw2217_read_reg_capacity(di);
	ret = cw2217_get_capacity(&cap, di);
	if (ret < 0)
		return ret;
	hwlog_info("cap = %d, last_cap = %d, ocv_idx = %d\n",
		cap, last_cap, di->ocv_idx);

	if ((last_cap <= 0) || (cap <= 0))
		return cap;

	if (abs(last_cap - cap) >= CW2217_CAPACITY_TH)
		return cap;

	/* reset last capacity and ocv_idx */
	ret = cw2217_write_reg_capacity(di, 0);
	ret += cw2217_write_reg_ocv_idx(di, 0);
	if (ret < 0)
		return ret;

	return last_cap;
}

static int cw2217_read_battery_rm(void *dev_data)
{
	int ret;
	int fcc;
	int capacity = 0;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_get_capacity(&capacity, di);
	if (ret < 0)
		return ret;

	ret = cw2217_read_battery_fcc(di);
	if (ret < 0)
		return ret;
	fcc = ret;

	return capacity * fcc / CW2217_FULL_CAPACITY;
}

static int cw2217_read_battery_charge_counter(void *dev_data)
{
	int rm;
	int fcc;
	int ret;
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	ret = cw2217_read_battery_soc(di); /* to update the values of ui_soc and ui_remainder */
	if (ret < 0)
		return ret;
	fcc = cw2217_read_battery_fcc(di);
	if (fcc < 0)
		return fcc;
	rm = (di->ui_soc * CW2217_SOC_MAGIC_100 + di->ui_remainder) *
		fcc / (CW2217_FULL_CAPACITY * CW2217_SOC_MAGIC_100);

	return rm;
}

static int cw2217_set_vterm_dec(int vterm, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	hwlog_info("set vterm dec = %d\n", vterm);

	if (!di)
		return -EPERM;

	cw2217_basp_soc_mapping_strategy(di, vterm);

	return 0;
}

static const char *cw2217_get_coul_model(void *dev_data)
{
	return "cw22xx";
}

#ifdef CONFIG_COUL_DRV
static int cw2217_get_battery_voltage_uv(void *dev_data)
{
	return cw2217_read_battery_vol(dev_data) * 1000;
}

static int cw2217_get_battery_current(void *dev_data)
{
	return cw2217_read_battery_current(dev_data);
}

static int cw2217_get_battery_avg_current(void *dev_data)
{
	return cw2217_read_battery_avg_current(dev_data);
}

static int cw2217_get_battery_resistance(void *dev_data)
{
	return 0;
}

static int cw2217_get_battery_fcc_design(void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di)
		return 0;

	return di->rated_capacity;
}

static int cw2217_get_battery_temperature(void *dev_data)
{
	return cw2217_read_battery_temperature(dev_data) / 10;
}

static int cw2217_get_battery_health(void *dev_data)
{
	struct cw2217_dev *di = dev_data;
	int health;

	if (!di)
		return POWER_SUPPLY_HEALTH_UNKNOWN;

	if (cw2217_update_temp(di))
		return POWER_SUPPLY_HEALTH_UNKNOWN;

	if (di->temp == CW2217_TEMP_UNKNOWN) {
		health = POWER_SUPPLY_HEALTH_UNKNOWN;
	} else if (di->temp < CW2217_COLD_TEMP) {
		health = POWER_SUPPLY_HEALTH_COLD;
	} else if (di->temp > CW2217_OVERHEAT_TEMP) {
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	} else {
		health = POWER_SUPPLY_HEALTH_GOOD;
	}
	return health;
}

static int cw2217_get_battery_vbat_max(void *dev_data)
{
	struct cw2217_dev *di = dev_data;
	int health;
	if (!di)
		return POWER_SUPPLY_HEALTH_UNKNOWN;
	if (cw2217_update_temp(di))
		return POWER_SUPPLY_HEALTH_UNKNOWN;
	if (di->temp == CW2217_TEMP_UNKNOWN) {
		health = POWER_SUPPLY_HEALTH_UNKNOWN;
	} else if (di->temp < CW2217_COLD_TEMP) {
		health = POWER_SUPPLY_HEALTH_COLD;
	} else if (di->temp > CW2217_OVERHEAT_TEMP) {
		health = POWER_SUPPLY_HEALTH_OVERHEAT;
	} else {
		health = POWER_SUPPLY_HEALTH_GOOD;
	}
	return health;
}

static int cw2217_get_battery_cc(void *dev_data)
{
	return 0;
}

static int cw2217_update_basp_policy(void *dev_data, unsigned int level,
	unsigned int nondc_volt_dec)
{
	return cw2217_set_vterm_dec(nondc_volt_dec, dev_data);
}

static int cw2217_coul_set_cali(void *data, struct coul_batt_cali_info *info)
{
	struct coul_cali_coul_drv_ops *cali_ops = coul_cali_get_ops();
	int ret = 0;

	if (!cali_ops || !cali_ops->coul_set_mode || !cali_ops->coul_set_cali)
		return -EPERM;

	if (!data || !info)
		return -EPERM;

	if (info->batt_index == CW2217_IC_TYPE_MAIN)
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_AUX, cali_ops->dev_data);
	else
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_MAIN, cali_ops->dev_data);

	ret += cali_ops->coul_set_cali(cali_ops->dev_data, info);
	return ret;
}

static int cw2217_coul_get_cali(void *data, struct coul_batt_cali_info *info)
{
	struct coul_cali_coul_drv_ops *cali_ops = coul_cali_get_ops();
	int ret = 0;

	if (!cali_ops || !cali_ops->coul_set_mode || !cali_ops->coul_get_cali)
		return -EPERM;

	if (!data || !info)
		return -EPERM;

	if (info->batt_index == CW2217_IC_TYPE_MAIN)
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_AUX, cali_ops->dev_data);
	else
		ret += cali_ops->coul_set_mode(COUL_CALI_MODE_MAIN, cali_ops->dev_data);

	ret += cali_ops->coul_get_cali(cali_ops->dev_data, info);
	return ret;
}

static struct coul_merge_drv_ops cw2217_ops = {
	.is_battery_exist = cw2217_is_battery_exist,
	.battery_voltage = cw2217_read_battery_vol,
	.battery_voltage_uv = cw2217_get_battery_voltage_uv,
	.battery_current = cw2217_get_battery_current,
	.battery_resistance = cw2217_get_battery_resistance,
	.fifo_avg_current = cw2217_get_battery_avg_current,
	.battery_current_avg = cw2217_read_battery_avg_current,
	.battery_capacity = cw2217_read_battery_soc,
	.battery_rm = cw2217_read_battery_rm,
	.battery_temperature = cw2217_get_battery_temperature,
	.battery_fcc = cw2217_read_battery_fcc,
	.battery_fcc_design = cw2217_get_battery_fcc_design,
	.battery_health = cw2217_get_battery_health,
	.battery_vbat_max = NULL,
	.battery_cycle_count = cw2217_read_battery_cycle,
	.get_battery_limit_fcc = cw2217_read_battery_fcc,
	.battery_cc = cw2217_get_battery_cc,
	.get_qmax = cw2217_get_battery_fcc_design,
	.charger_event_rcv = NULL,
	.update_basp_policy = cw2217_update_basp_policy,
	.coul_set_cali = cw2217_coul_set_cali,
	.coul_get_cali = cw2217_coul_get_cali,
};
#else
static struct coul_interface_ops cw2217_ops = {
	.type_name = "main",
	.is_coul_ready = cw2217_is_ready,
	.is_battery_exist = cw2217_is_battery_exist,
	.get_battery_capacity = cw2217_read_battery_soc,
	.get_battery_voltage = cw2217_read_battery_vol,
	.get_battery_current = cw2217_read_battery_current,
	.get_battery_avg_current = cw2217_read_battery_avg_current,
	.get_battery_temperature = cw2217_read_battery_temperature,
	.get_battery_fcc = cw2217_read_battery_fcc,
	.get_battery_cycle = cw2217_read_battery_cycle,
	.set_battery_low_voltage = cw2217_set_battery_low_voltage,
	.set_battery_last_capacity = cw2217_set_last_capacity,
	.get_battery_last_capacity = cw2217_get_last_capacity,
	.get_battery_rm = cw2217_read_battery_rm,
	.get_battery_charge_counter = cw2217_read_battery_charge_counter,
	.set_vterm_dec = cw2217_set_vterm_dec,
	.get_coul_model = cw2217_get_coul_model,
};

static struct coul_interface_ops cw2217_aux_ops = {
	.type_name = "aux",
	.is_coul_ready = cw2217_is_ready,
	.is_battery_exist = cw2217_is_battery_exist,
	.get_battery_capacity = cw2217_read_battery_soc,
	.get_battery_voltage = cw2217_read_battery_vol,
	.get_battery_current = cw2217_read_battery_current,
	.get_battery_avg_current = cw2217_read_battery_avg_current,
	.get_battery_temperature = cw2217_read_battery_temperature,
	.get_battery_fcc = cw2217_read_battery_fcc,
	.get_battery_cycle = cw2217_read_battery_cycle,
	.set_battery_low_voltage = cw2217_set_battery_low_voltage,
	.set_battery_last_capacity = cw2217_set_last_capacity,
	.get_battery_last_capacity = cw2217_get_last_capacity,
	.get_battery_rm = cw2217_read_battery_rm,
	.get_battery_charge_counter = cw2217_read_battery_charge_counter,
	.set_vterm_dec = cw2217_set_vterm_dec,
	.get_coul_model = cw2217_get_coul_model,
};
#endif /* CONFIG_COUL_DRV */

static int cw2217_get_calibration_curr(int *val, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EPERM;
	}

	return cw2217_get_current(val, di);
}

static int cw2217_get_calibration_vol(int *val, void *dev_data)
{
	int ret;
	struct cw2217_dev *di = dev_data;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EPERM;
	}

	ret = cw2217_get_voltage(val, di);
	if (ret < 0)
		return ret;

	*val *= POWER_UV_PER_MV;
	return 0;
}

static int cw2217_set_current_gain(unsigned int val, void *dev_data)
{
	struct cw2217_dev *di = dev_data;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EPERM;
	}

	di->coefficient = val;
	return 0;
}

static int cw2217_set_voltage_gain(unsigned int val, void *dev_data)
{
	return 0;
}

static int cw2217_enable_cali_mode(int enable, void *dev_data)
{
	return 0;
}

static struct coul_cali_ops cw2217_cali_ops = {
	.dev_name = "aux",
	.get_current = cw2217_get_calibration_curr,
	.get_voltage = cw2217_get_calibration_vol,
	.set_current_gain = cw2217_set_current_gain,
	.set_voltage_gain = cw2217_set_voltage_gain,
	.set_cali_mode = cw2217_enable_cali_mode,
};

/* main battery gauge use aux calibration data for compatible */
static struct coul_cali_ops cw2217_aux_cali_ops = {
	.dev_name = "main",
	.get_current = cw2217_get_calibration_curr,
	.get_voltage = cw2217_get_calibration_vol,
	.set_current_gain = cw2217_set_current_gain,
	.set_voltage_gain = cw2217_set_voltage_gain,
	.set_cali_mode = cw2217_enable_cali_mode,
};

#ifdef CONFIG_COUL_DRV
static void cw2217_register_info(struct cw2217_dev *di)
{
	struct power_devices_info_data *power_dev_info = NULL;

	if (di->ic_role == CW2217_IC_TYPE_MAIN) {
		strncpy(di->batt_name, "battery_gauge", CW2217_BATT_NAME_LEN - 1);
		cw2217_log_ops.dev_data = (void *)di;
		power_log_ops_register(&cw2217_log_ops);
		cw2217_cali_ops.dev_data = (void *)di;
		coul_cali_ops_register(&cw2217_cali_ops);
	} else {
		strncpy(di->batt_name, "battery_gauge_aux", CW2217_BATT_NAME_LEN - 1);
		cw2217_aux_log_ops.dev_data = (void *)di;
		power_log_ops_register(&cw2217_aux_log_ops);
		cw2217_aux_cali_ops.dev_data = (void *)di;
		coul_cali_ops_register(&cw2217_aux_cali_ops);
	}

	if (di->adc_sample_type) {
		if (di->adc_sample_type & BIT(BATT_CURRENT))
			batt_adc_monitor_ops_register(di->ic_role, BATT_CURRENT, cw2217_read_battery_current, di);
	} else {
		coul_merge_drv_coul_ops_register(&cw2217_ops, di, di->ic_role, di->batt_name);
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->chip_id;
		power_dev_info->ver_id = 0;
	}
}
#else
static void cw2217_register_info(struct cw2217_dev *di)
{
	struct power_devices_info_data *power_dev_info = NULL;

	if (di->ic_role == CW2217_IC_TYPE_MAIN) {
		cw2217_log_ops.dev_data = (void *)di;
		power_log_ops_register(&cw2217_log_ops);
		cw2217_ops.dev_data = (void *)di;
		coul_interface_ops_register(&cw2217_ops);
		cw2217_cali_ops.dev_data = (void *)di;
		coul_cali_ops_register(&cw2217_cali_ops);
	} else {
		cw2217_aux_log_ops.dev_data = (void *)di;
		power_log_ops_register(&cw2217_aux_log_ops);
		cw2217_aux_ops.dev_data = (void *)di;
		coul_interface_ops_register(&cw2217_aux_ops);
		cw2217_aux_cali_ops.dev_data = (void *)di;
		coul_cali_ops_register(&cw2217_aux_cali_ops);
	}

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = di->chip_id;
		power_dev_info->ver_id = 0;
	}
}
#endif /* CONFIG_COUL_DRV */

static void cw2217_init_func(struct cw2217_dev *di)
{
	int ret;
	int loop = 0;

	if (!di) {
		hwlog_err("di NULL\n");
		return;
	}

	ret = cw2217_config(di);
	while ((loop++ < CW2217_RETRY_COUNT) && (ret != 0)) {
		msleep(DT_MSLEEP_200MS);
		ret = cw2217_config(di);
	}
	if (ret) {
		hwlog_err("init config fail\n");
		return;
	}

	ret = cw2217_init_data(di);
	if (ret) {
		hwlog_err("init data fail\n");
		return;
	}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	if (di->ic_role == CW2217_IC_TYPE_MAIN)
		set_hw_dev_flag(DEV_I2C_GAUGE_IC);
	else
		set_hw_dev_flag(DEV_I2C_GAUGE_IC_AUX);
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

	di->cwfg_workqueue = create_singlethread_workqueue("cwfg_gauge");
	INIT_DELAYED_WORK(&di->battery_delay_work, cw2217_bat_work);
	queue_delayed_work(di->cwfg_workqueue, &di->battery_delay_work,
		msecs_to_jiffies(CW2217_QUEUE_START_WORK_TIME));

	cw2217_register_info(di);
}

#ifndef CONFIG_COUL_DRV
static void cw2217_init_work_func(struct work_struct *work)
{
	struct cw2217_dev *di = container_of(work, struct cw2217_dev,
		init_work);
	cw2217_init_func(di);
}
#endif /* CONFIG_COUL_DRV */

static int cw2217_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cw2217_dev *di = NULL;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	ret = cw2217_parse_dt(&client->dev, di);
	if (ret) {
		hwlog_err("parse dts fail\n");
		return ret;
	}

	i2c_set_clientdata(client, di);
	di->dev = &client->dev;
	di->client = client;

	ret = cw2217_check_chip(di);
	if (ret)
		return ret;

#ifdef CONFIG_COUL_DRV
	cw2217_init_func(di);
#else
	INIT_WORK(&di->init_work, cw2217_init_work_func);
	queue_work(system_highpri_wq, &di->init_work);
#endif /* CONFIG_COUL_DRV */

	return 0;
}

static int cw2217_remove(struct i2c_client *client)
{
	return 0;
}

#ifdef CONFIG_PM
static int cw2217_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cw2217_dev *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;

	cancel_delayed_work_sync(&di->battery_delay_work);
	return 0;
}

static int cw2217_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cw2217_dev *di = NULL;

	if (!client)
		return 0;

	di = i2c_get_clientdata(client);
	if (!di)
		return 0;

	queue_delayed_work(di->cwfg_workqueue, &di->battery_delay_work,
		msecs_to_jiffies(CW2217_QUEUE_RESUME_WORK_TIME));
	return 0;
}

static const struct dev_pm_ops cw2217_pm_ops = {
	.suspend = cw2217_suspend,
	.resume = cw2217_resume,
};
#define CW2217_PM_OPS (&cw2217_pm_ops)
#else
#define CW2217_PM_OPS (NULL)
#endif /* CONFIG_PM */

static const struct i2c_device_id cw2217_id_table[] = {
	{ "cw2217", 0 },
	{}
};

static const struct of_device_id cw2217_match_table[] = {
	{ .compatible = "cellwise,cw2217", },
	{},
};

static struct i2c_driver cw2217_driver = {
	.probe = cw2217_probe,
	.remove = cw2217_remove,
	.id_table = cw2217_id_table,
	.driver = {
		.owner = THIS_MODULE,
		.name = "cw2217",
		.of_match_table = cw2217_match_table,
		.pm = CW2217_PM_OPS,
	},
};

static int __init cw2217_init(void)
{
	return i2c_add_driver(&cw2217_driver);
}

static void __exit cw2217_exit(void)
{
	i2c_del_driver(&cw2217_driver);
}

#ifdef CONFIG_COUL_DRV
fs_initcall_sync(cw2217_init);
#else
module_init(cw2217_init);
#endif /* CONFIG_COUL_DRV */
module_exit(cw2217_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("cw2217 Fuel Gauge Driver");
MODULE_LICENSE("GPL v2");
