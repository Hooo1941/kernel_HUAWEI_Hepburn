 // SPDX-License-Identifier: GPL-2.0
 /*
  * afe_coul.c
  *
  * driver for AFE(analog front ending) coul gauge
  *
  * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/timer.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <securec.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/coul/coul_calibration.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_core.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "afe_coul.h"

#define HWLOG_TAG afe_coul
HWLOG_REGIST();

#define AFE_COUL_HALF				2
#define AFE_COUL_TENTH				10
#define AFE_COUL_HUNDRED			100
#define AFE_COUL_PERMILLAGE			1000
#define AFE_COUL_SEC_PER_HOUR			3600

typedef struct {
	int mode;
	int cmd;
} work_mode;

enum afe_work_mode {
	AFE_COUL_ACTIVE_MODE = 0,
	AFE_COUL_LOW_POWER_MODE,
	AFE_COUL_ULTRA_LOW_POWER_MODE,
	AFE_COUL_CALI_MODE,
	AFE_COUL_SHIPPING_MODE,
	AFE_COUL_SHUTDOWN_MODE,
};

static const work_mode mode_switch[] = {
	{ AFE_COUL_ACTIVE_MODE, AFE_COUL_ENTR_ACT_CMD },
	{ AFE_COUL_LOW_POWER_MODE, AFE_COUL_ENTR_LP_CMD },
	{ AFE_COUL_ULTRA_LOW_POWER_MODE, AFE_COUL_ENTR_ULP_CMD },
	{ AFE_COUL_CALI_MODE, AFE_COUL_ENTR_CALI_CMD },
	{ AFE_COUL_SHIPPING_MODE, AFE_COUL_ENTR_SP_CMD },
};

static struct afe_coul_device *g_afe_coul_dev = NULL;

/* ntc resistance table, temp range [-30, 80] celsius, unit 1 celsius */
static const int g_ntc_temp_tbl[] = {
	11790, 11180, 10600, 10060, 9551, 9069, 8615, 8186, 7781, 7399,
	7037, 6696, 6374, 6069, 5780, 5507, 5249, 5004, 4772, 4552,
	4344, 4146, 3959, 3781, 3613, 3453, 3300, 3156, 3019, 2888,
	2764, 2646, 2533, 2426, 2324, 2227, 2135, 2047, 1963, 1883,
	1806, 1734, 1664, 1598, 1535, 1474, 1417, 1362, 1309, 1259,
	1211, 1165, 1121, 1079, 1038, 1000, 963, 928, 894, 862,
	831, 801, 773, 745, 719, 694, 670, 647, 625, 603,
	583, 563, 544, 526, 508, 492, 475, 460, 445, 430,
	417, 403, 390, 378, 366, 354, 343, 333, 322, 312,
	303, 293, 285, 276, 268, 260, 252, 244, 237, 230,
	223, 217, 211, 205, 199, 193, 187, 182, 177, 172,
	167
};

static struct afe_coul_device *afe_coul_get_di(int coul_index)
{
	if (!g_afe_coul_dev)
		hwlog_err("%s chip not init\n", __func__);

	return g_afe_coul_dev;
}

static int afe_coul_read_block_data(unsigned char reg, unsigned char len, unsigned char *dst, int coul_index)
{
	unsigned char cmd[AFE_COUL_CMD_LEN];
	struct afe_coul_device *di = afe_coul_get_di(coul_index);

	if (!di || !dst) {
		hwlog_err("%s afe chip%d not init or dst is null\n", __func__, coul_index);
		return -ENODEV;
	}

	cmd[0] = reg;
	return power_i2c_read_block(di->client, cmd, sizeof(cmd), dst, len);
}

static int afe_coul_write_block_data(unsigned char reg, unsigned char len, unsigned char *src, int coul_index)
{
	int ret;
	unsigned char buf[AFE_COUL_CMD_LEN_MAX];
	struct afe_coul_device *di = afe_coul_get_di(coul_index);

	if (!di || !src) {
		hwlog_err("%s afe chip%d not init or src is null\n", __func__, coul_index);
		return -ENODEV;
	}

	if (len > AFE_COUL_CMD_LEN_MAX - 1) {
		hwlog_err("%s buf len overflow\n", __func__);
		return -ERANGE;
	}

	buf[0] = reg;
	ret = memcpy_s(&buf[1], AFE_COUL_CMD_LEN_MAX - 1, src, len);
	if (ret != EOK)
		return -ENOMEM;

	return power_i2c_write_block(di->client, buf, AFE_COUL_CMD_LEN + len);
}

static int afe_coul_write_byte(unsigned char reg, unsigned char value, int coul_index)
{
	struct afe_coul_device *di = afe_coul_get_di(coul_index);

	if (!di) {
		hwlog_err("%s afe chip%d not init\n", __func__, coul_index);
		return -ENODEV;
	}

	return power_i2c_u8_write_byte(di->client, reg, value);
}

static int afe_coul_read_byte(unsigned char reg, unsigned char *value, int coul_index)
{
	struct afe_coul_device *di = afe_coul_get_di(coul_index);

	if (!di) {
		hwlog_err("%s afe chip%d not init\n", __func__, coul_index);
		return -ENODEV;
	}

	return power_i2c_u8_read_byte(di->client, reg, value);
}

static int afe_coul_read_word(unsigned char reg, unsigned short *value, int coul_index)
{
	struct afe_coul_device *di = afe_coul_get_di(coul_index);

	if (!di) {
		hwlog_err("%s afe chip%d not init\n", __func__, coul_index);
		return -ENODEV;
	}
	/* 1: big endian slave chip */
	return power_i2c_u8_read_word(di->client, reg, value, 1);
}

static int afe_coul_write_word(unsigned char reg, unsigned short value, int coul_index)
{
	unsigned char buf[AFE_COUL_TWO_BYTES];

	buf[0] = value >> AFE_COUL_BITS_PER_BYTE;
	buf[1] = value;

	return afe_coul_write_block_data(reg, AFE_COUL_TWO_BYTES, buf, coul_index);
}

static int afe_coul_set_regval_bit_u16(unsigned char reg, unsigned short mask, bool flag, int batt)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(reg, &regval, batt);
	if (ret)
		return ret;

	if (flag)
		regval |= mask;
	else
		regval &= (~mask);

	return afe_coul_write_word(reg, regval, batt);
}

/* Voltage = (32000 + regval) * 62.5uV */
static int afe_coul_convert_regval2uv(int batt, unsigned int regval)
{
	return (regval + AFE_COUL_VOLT_REG_BASE) * AFE_COUL_VOLT_REG_UNIT / AFE_COUL_TENTH;
}

static unsigned int afe_coul_convert_uv2regval(int batt, int uv_val)
{
	return uv_val * AFE_COUL_TENTH / AFE_COUL_VOLT_REG_UNIT - AFE_COUL_VOLT_REG_BASE;
}

/*
 *    Current = (regval * unit) / Rsense
 * Current_ua = (regval * unit_uv * 1000) / Rsense_mohm
 *		      = (unit_uv * 1000) * regval / Rsense_mohm
 *		      = (30000 * 1000 / 32768) * regval /  Rsense_mohm
 *		      = 916 * regval / Rsense_mohm
 */
static int afe_coul_convert_regval2ua(int batt, unsigned int regval)
{
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di || (di->r_sense == 0)) {
		hwlog_err("%s coul%d chip not init or r_sense is zero\n", __func__, batt);
		return 0;
	}

	return -(AFE_COUL_UNIT_CUR_REG * (int64_t)((int)regval) * AFE_COUL_TENTH / di->r_sense);
}

static int afe_coul_convert_ocv_regval2ua(int batt, short regval)
{
	return afe_coul_convert_regval2ua(batt, regval);
}

static int afe_temp_table_search(int r)
{
	int record_temp, mid;
	int left = 0;
	int right = sizeof(g_ntc_temp_tbl) / sizeof(g_ntc_temp_tbl[0]) - 1;

	if (r > g_ntc_temp_tbl[0])
		return AFE_COUL_NTC_MIN_TEMP;
	else if (r < g_ntc_temp_tbl[right])
		return AFE_COUL_NTC_MAX_TEMP;

	while (left <= right) {
		mid = left + (right - left) / AFE_COUL_HALF;
		if (r == g_ntc_temp_tbl[mid])
			return (mid * AFE_COUL_TEMP_UNIT + AFE_COUL_NTC_MIN_TEMP);
		else if (r < g_ntc_temp_tbl[mid])
			left = mid + 1;
		else
			right = mid - 1;
	}

	if (g_ntc_temp_tbl[right] == g_ntc_temp_tbl[left]) {
		hwlog_err("ntc table search fail\n");
		return AFE_COUL_DEFAULT_TEMP;
	}
	/* linear interpolation: temp = left_temp + temp_unit * (r_left - r) / (r_left - r_right) */
	record_temp = AFE_COUL_NTC_MIN_TEMP + right * AFE_COUL_TEMP_UNIT + AFE_COUL_TEMP_UNIT *
		(g_ntc_temp_tbl[right] - r) / (g_ntc_temp_tbl[right] - g_ntc_temp_tbl[left]);
	return record_temp;
}

/*
 *   According to R_SERIES / R_NTC = V_SERIES / V_NTC and V_SERIES + V_NTC = 1800mV,
 *   R_NTC = V_NTC / (1800 - V_NTC) * R_SERIES
 *		   = regval * R_SERIES / (1800 - regval)
 */
static int afe_coul_convert_regval2temp(unsigned int regval)
{
	int tmp;

	if (AFE_COUL_TS_PULL_UP_VOLT == regval)
		tmp = AFE_COUL_DEFAULT_NTC_R;
	else
		tmp = regval * AFE_COUL_NTC_SERIES_R / (AFE_COUL_TS_PULL_UP_VOLT - regval);

	return afe_temp_table_search(tmp);
}

/* the regval using 2's-complment form to show negative current */
static int afe_coul_get_battery_cur_ua(int batt)
{
	int ua, ret;
	unsigned short regval = 0;

	ret = afe_coul_read_word(AFE_COUL_REG_CURRENT, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d afe_coul get battery current failed\n", __func__, batt);
		return 0;
	}

	/* u16->s16->int, ensure that the 2's-complement regval not change */
	ua = afe_coul_convert_regval2ua(batt, (short)regval);
	return ua;
}

static int afe_coul_get_battery_volt_uv(int batt)
{
	int uv, ret;
	unsigned short regval = 0;

	ret = afe_coul_read_word(AFE_COUL_REG_VOLTAGE, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d afe_coul get battery volt failed\n", __func__, batt);
		return 0;
	}
	uv = afe_coul_convert_regval2uv(batt, regval);
	return uv;
}

static int afe_coul_get_battery_temp(int batt)
{
	int temp_tenth, ret;
	unsigned short regval = 0;

	ret = afe_coul_read_word(AFE_COUL_REG_BAT_TEMP, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d afe_coul get battery temp failed\n", __func__, batt);
		return 0;
	}

	temp_tenth = afe_coul_convert_regval2temp(regval);
	return temp_tenth / AFE_COUL_TENTH;
}

static int afe_coul_get_battery_volt_from_fifo_uv(int batt, short fifo_order)
{
	int ret;
	unsigned char reg;
	unsigned short regval = 0;

	if (fifo_order >= AFE_COUL_NUM_FIFO)
		fifo_order = 0;

	reg = AFE_COUL_REG_VOLT_FIFO_BASE + fifo_order * AFE_COUL_TWO_BYTES;
	ret = afe_coul_read_word(reg, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d read fifo%d failed\n", __func__, batt, fifo_order);
		return AFE_COUL_VOLT_DEFAULT;
	}

	return afe_coul_convert_regval2uv(batt, regval);
}

static int afe_coul_get_battery_cur_from_fifo_ua(int batt, short fifo_order)
{
	int ret;
	unsigned char reg;
	unsigned short regval = 0;

	if (fifo_order >= AFE_COUL_NUM_FIFO)
		fifo_order = 0;

	reg = AFE_COUL_REG_CURR_FIFO_BASE + fifo_order * AFE_COUL_TWO_BYTES;
	ret = afe_coul_read_word(reg, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d read fifo%d failed\n", __func__, batt, fifo_order);
		return 0;
	}

	return afe_coul_convert_regval2ua(batt, (short)regval);
}

static void get_vc_by_cur(struct vcdata *vc, int cur)
{
	if (cur > vc->max_c)
		vc->max_c = cur;
	else if (cur < vc->min_c)
		vc->min_c = cur;
}

static void afe_coul_get_fifo_avg_data(int batt, struct vcdata *vc)
{
	int abnormal_value_cnt = 0;
	int vol_fifo[AFE_COUL_NUM_FIFO], cur_fifo[AFE_COUL_NUM_FIFO];
	int cur, vols, curs, i;

	if (!vc) {
		hwlog_err("%s vcdata point is NULL!\n", __func__);
		return ;
	}

	for (i = 0; i < AFE_COUL_NUM_FIFO; i++) {
		vol_fifo[i] = afe_coul_get_battery_volt_from_fifo_uv(batt, i) / AFE_COUL_PERMILLAGE;
		cur_fifo[i] = afe_coul_get_battery_cur_from_fifo_ua(batt, i) / AFE_COUL_PERMILLAGE;
	}

	if (vol_fifo[0] != AFE_COUL_VOLT_DEFAULT) {
		vols = vol_fifo[0];
		cur = cur_fifo[0];
	} else {
		vols = 0;
		cur = 0;
		abnormal_value_cnt++;
	}

	curs = cur;
	vc->max_c = cur;
	vc->min_c = cur;

	for (i = 1; i < AFE_COUL_NUM_FIFO; i++) {
		if (vol_fifo[i] != AFE_COUL_VOLT_DEFAULT) {
			vols += vol_fifo[i];
			cur = cur_fifo[i];
			curs += cur;
			get_vc_by_cur(vc, cur);
		} else {
			abnormal_value_cnt++;
		}
	}

	if (abnormal_value_cnt >= AFE_COUL_NUM_FIFO) {
		hwlog_err("%s coul%d no valid fifo data\n", __func__, batt);
		return;
	}

	vc->avg_v = vols / (AFE_COUL_NUM_FIFO - abnormal_value_cnt);
	vc->avg_c = curs / (AFE_COUL_NUM_FIFO - abnormal_value_cnt);

	hwlog_info("%s coul%d avg_v=%d, avg_c=%d, max_c=%d, min_c=%d\n",
		__func__, batt, vc->avg_v, vc->avg_c, vc->max_c, vc->min_c);
}

static int afe_coul_get_fifo_depth(void)
{
	return AFE_COUL_NUM_FIFO;
}

static inline unsigned int afe_coul_convert_cc_val_u32(unsigned char *buf)
{
	return (((unsigned int)buf[AFE_COUL_CC_REG0]) << AFE_COUL_THREE_BYTE_BITS) |
		   (((unsigned int)buf[AFE_COUL_CC_REG1]) << AFE_COUL_TWO_BYTE_BITS) |
		   (((unsigned int)buf[AFE_COUL_CC_REG2]) << AFE_COUL_BITS_PER_BYTE) |
		   ((unsigned int)buf[AFE_COUL_CC_REG3]);
}

static unsigned int afe_coul_get_battery_cc_in(int batt)
{
	int ret;
	unsigned char buf[AFE_COUL_NUM_CC_REG];

	ret = afe_coul_read_block_data(AFE_COUL_REG_CC_IN, AFE_COUL_NUM_CC_REG, buf, batt);
	if (ret) {
		hwlog_err("%s read battery cc in failed\n", __func__);
		return AFE_COUL_DEFAULT_CC;
	}

	return afe_coul_convert_cc_val_u32(buf);
}

static unsigned int afe_coul_get_battery_cc_out(int batt)
{
	int ret;
	unsigned char buf[AFE_COUL_NUM_CC_REG];

	ret = afe_coul_read_block_data(AFE_COUL_REG_CC_OUT, AFE_COUL_NUM_CC_REG, buf, batt);
	if (ret) {
		hwlog_err("%s read battery cc out failed\n", __func__);
		return AFE_COUL_DEFAULT_CC;
	}

	return afe_coul_convert_cc_val_u32(buf);
}

static int afe_coul_get_battery_cc_over_uah(int batt)
{
	int ret;
	unsigned char buf[AFE_COUL_NUM_CC_REG];

	ret = afe_coul_read_block_data(AFE_COUL_REG_CC_OVERFLOW, AFE_COUL_NUM_CC_REG, buf, batt);
	if (ret) {
		hwlog_err("%s read battery cc over failed\n", __func__);
		return 0;
	}

	return (int)afe_coul_convert_cc_val_u32(buf);
}

static void afe_coul_set_battery_cc_over(int batt, int cc_uah)
{
	int ret;
	unsigned char buf[AFE_COUL_NUM_CC_REG];

	buf[AFE_COUL_CC_REG0] = ((unsigned int)cc_uah) >> AFE_COUL_THREE_BYTE_BITS;
	buf[AFE_COUL_CC_REG1] = ((unsigned int)cc_uah) >> AFE_COUL_TWO_BYTE_BITS;
	buf[AFE_COUL_CC_REG2] = ((unsigned int)cc_uah) >> AFE_COUL_BITS_PER_BYTE;
	buf[AFE_COUL_CC_REG3] = ((unsigned int)cc_uah);

	ret = afe_coul_write_block_data(AFE_COUL_REG_CC_OVERFLOW, AFE_COUL_NUM_CC_REG, buf, batt);
	hwlog_info("%s coul%d set cc over reg %duah, ret=%d\n", __func__, batt, cc_uah, ret);
}

static s64 afe_coul_convert_regval2uah(int batt, u64 regval)
{
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di || (di->r_sense == 0)) {
		hwlog_err("%s chip%d not init or r_sense is zero\n", __func__, batt);
		return 0;
	}

	return (AFE_COUL_UNIT_CUR_REG * (long long)regval * AFE_COUL_TENTH / di->r_sense) /
		AFE_COUL_SEC_PER_HOUR;
}

/*
 * Description:  value of out_uah - in_uah + cc_over_uah recorded by afe
 * cc_over is the recorded value when cc overflow. It is clear when cc regs are clear.
 * Return: value of uah through coulomb
 */
static int afe_coul_calculate_cc_uah(int batt)
{
	int cc_uah, cc_over_uah;
	unsigned int cc_in_uah, cc_out_uah;
	unsigned int cc_in, cc_out;

	cc_in = afe_coul_get_battery_cc_in(batt);
	cc_out = afe_coul_get_battery_cc_out(batt);

	cc_in_uah = (unsigned int)afe_coul_convert_regval2uah(batt, cc_in);
	cc_out_uah = (unsigned int)afe_coul_convert_regval2uah(batt, cc_out);
	cc_over_uah = afe_coul_get_battery_cc_over_uah(batt);
	cc_uah = cc_out_uah - cc_in_uah + cc_over_uah;

	hwlog_info("%s coul%d cc_in=%u, cc_out=%u, cc_over=%d, cc_uah=%d\n",
		__func__, batt, cc_in_uah, cc_out_uah, cc_over_uah, cc_uah);
	return cc_uah;
}

static void afe_coul_set_low_volt_int(int volt_mv)
{
	int ret;
	unsigned char regval;

	regval = (volt_mv - AFE_COUL_LOW_VOLT_INT_B) / AFE_COUL_LOW_VOLT_INT_A;
	ret = afe_coul_write_byte(AFE_COUL_REG_UNDER_VOLT, regval, AFE_COUL_INDEX_AUX_COUL);
	hwlog_info("%s Set low volt int %dmV, ret=%d\n", __func__, volt_mv, ret);
}

static unsigned short afe_coul_get_int_flag(int batt)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(AFE_COUL_REG_INT_PIN_CLT, &regval, batt);
	if (ret) {
		hwlog_info("%s coul%d read int flag failed\n", __func__, batt);
		return 0;
	}

	hwlog_info("%s coul%d int flag reg0x%x", __func__, batt, regval);
	return regval;
}

static unsigned short afe_coul_get_chip_status(int batt)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(AFE_COUL_REG_CHIP_STATUS, &regval, batt);
	if (ret) {
		hwlog_info("%s coul%d read chip status failed\n", __func__, batt);
		return 0;
	}

	hwlog_info("%s coul%d afe chip status 0x%x", __func__, batt, regval);
	return regval;
}

static int afe_coul_is_battery_moved(int batt)
{
	int ret;
	unsigned short regval;
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di) {
		hwlog_err("%s chip%d not init\n", __func__, batt);
		return 0;
	}

	regval = afe_coul_get_chip_status(batt);
	if (regval & AFE_COUL_BSA_FLAG) {
		/* clear flag after read */
		regval &= (~AFE_COUL_BSA_FLAG);
		ret = afe_coul_write_word(AFE_COUL_REG_CHIP_STATUS, regval, batt);
		hwlog_info("%s coul%d batt has been moved, clear ret=%d\n", __func__, batt, ret);
		di->batt_moved_flag = 1;
	}

	return di->batt_moved_flag;
}

static void afe_coul_enable_volt_int(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG1, AFE_COUL_VOL_IRQ_EN, 1, batt);
	hwlog_info("%s coul%d enable volt int, ret=%d\n", __func__, batt, ret);
}

static void afe_coul_disable_volt_int(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG1, AFE_COUL_VOL_IRQ_EN, 0, batt);
	hwlog_info("%s coul%d disable volt int, ret=%d\n", __func__, batt, ret);
}

static void afe_coul_enable_battery_present_detect(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG1, AFE_COUL_BD_PRES_EN, 1, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
}

static void afe_coul_reset_cc_reg(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG3, AFE_COUL_CC_RESET, 1, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
}

static void afe_coul_reset_fifo(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG3, AFE_COUL_FIFO_RESET, 1, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
}

static int afe_coul_enter_shutdown_mode(int batt)
{
	unsigned char regval;
	int ret;

	ret = afe_coul_read_byte(AFE_COUL_REG_OPCG4, &regval, batt);
	regval |= AFE_COUL_ENTR_SD_CMD;
	ret += afe_coul_write_byte(AFE_COUL_REG_OPCG4, regval, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
	return ret;
}

static int afe_coul_select_work_mode(int batt, int mode)
{
	int ret;

	if (mode < AFE_COUL_SHUTDOWN_MODE)
		ret = afe_coul_write_word(AFE_COUL_REG_OPCG3, mode_switch[mode].cmd, batt);
	else
		ret = afe_coul_enter_shutdown_mode(batt);

	hwlog_info("%s coul%d mode=%d, ret=%d\n", __func__, batt, mode, ret);
	return ret;
}

static int afe_coul_get_use_saved_ocv_flag(int batt)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG1, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d get use_saved_ocv_flag failed\n", __func__, batt);
		return 0;
	}
	return regval & AFE_COUL_USE_SAVED_OCV_FLAG;
}

static int afe_coul_get_delta_rc_ignore_flag(int batt)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG1, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d get flag fail\n", __func__, batt);
		return 0;
	}

	if (regval & AFE_COUL_DRC_SCENE_FLAG) {
		regval &= ~AFE_COUL_DRC_SCENE_FLAG;
		ret = afe_coul_write_byte(AFE_COUL_REG_ALGO_FLAG1, regval, batt);
		hwlog_info("%s flag=1, clear ret=%d\n", __func__, ret);
		return 1;
	}
	return 0;
}

static int afe_coul_get_nv_read_flag(int batt)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG1, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d get nv read flag failed\n", __func__, batt);
		return 0;
	}
	return regval & AFE_COUL_NV_READ_FLAG;
}

static void afe_coul_set_nv_save_flag(int batt, int nv_flag)
{
	int ret;
	unsigned char regval = 0;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG1, &regval, batt);
	if (nv_flag == AFE_COUL_NV_SUCCESS)
		regval |= AFE_COUL_NV_SAVE_FLAG;
	else
		regval &= (~AFE_COUL_NV_SAVE_FLAG);

	ret += afe_coul_write_byte(AFE_COUL_REG_ALGO_FLAG1, regval, batt);
	hwlog_info("%s coul%d set nv read flag=%d, ret=%d\n", __func__, batt, nv_flag, ret);
}

static void afe_coul_save_ocv_level(int batt, unsigned char level)
{
	int ret;
	unsigned char regval = 0;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG2, &regval, batt);
	regval &= (~AFE_COUL_SAVE_OCV_LEVEL);
	regval |= ((level << AFE_COUL_OCV_LEVEL_SHIFT) & AFE_COUL_SAVE_OCV_LEVEL);
	ret += afe_coul_write_byte(AFE_COUL_REG_ALGO_FLAG2, regval, batt);
	hwlog_info("%s coul%d set ocv_level=%d, ret=%d\n", __func__, batt, level, ret);
}

static void afe_coul_get_ocv_level(int batt, unsigned char *level)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_ALGO_FLAG2, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d get ocv_level failed\n", __func__, batt);
		*level = AFE_COUL_OCV_LEVEL_MAX;
		return;
	}

	regval &= AFE_COUL_SAVE_OCV_LEVEL;
	*level = regval >> AFE_COUL_OCV_LEVEL_SHIFT;
	hwlog_info("%s coul%d ocv_level=%d\n", __func__, batt, *level);
}

/* OCV = [20000 + regval] * 100uV */
static int afe_coul_convert_ocv_regval2uv(int batt, short regval)
{
	if (regval & AFE_COUL_INVALID_TO_UPDATE_FCC)
		regval &= (~AFE_COUL_INVALID_TO_UPDATE_FCC);

	return (AFE_COUL_OCV_REG_BASE + regval) * AFE_COUL_OCV_REG_UNIT / AFE_COUL_TENTH;
}

static unsigned short afe_coul_ocv_convert_uv2regval(int ocv_uv)
{
	return (ocv_uv * AFE_COUL_TENTH / AFE_COUL_OCV_REG_UNIT - AFE_COUL_OCV_REG_BASE);
}

static int afe_coul_get_fcc_invalid_up_flag(int batt)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(AFE_COUL_REG_OCV, &regval, batt);
	if (ret) {
		hwlog_err("%s coul%d get fcc invalid up flag failed\n", __func__, batt);
		return 0;
	}

	return (regval & AFE_COUL_INVALID_TO_UPDATE_FCC);
}

static void afe_coul_save_ocv(int batt, int ocv, int invalid_fcc_up_flag)
{
	int ret;
	unsigned short regval = afe_coul_ocv_convert_uv2regval(ocv);

	if (invalid_fcc_up_flag)
		regval |= AFE_COUL_INVALID_TO_UPDATE_FCC;

	ret = afe_coul_write_word(AFE_COUL_REG_OCV, regval, batt);
	hwlog_info("%s coul%d saved_ocv_uv=%d, ret=%d\n", __func__, batt, ocv, ret);
}

static void afe_coul_clear_ocv(int batt)
{
	int ret;

	ret = afe_coul_write_word(AFE_COUL_REG_OCV, 0, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
}

static short afe_coul_get_ocv(int batt)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(AFE_COUL_REG_OCV, &regval, batt);
	hwlog_info("%s coul%d ocv_reg_val=0x%x, ret=%d\n", __func__, batt, regval, ret);

	return (short)regval;
}

static void afe_coul_save_ocv_temp_tenth(int batt, short ocv_temp)
{
	int ret;

	ret = afe_coul_write_word(AFE_COUL_REG_OCV_TEMP, ocv_temp, batt);
	hwlog_info("%s coul%d ocv_temp=%d, ret=%d\n", __func__, batt, ocv_temp, ret);
}

static void afe_coul_clear_ocv_temp(int batt)
{
	int ret;

	ret = afe_coul_write_word(AFE_COUL_REG_OCV_TEMP, 0, batt);
	hwlog_info("%s coul%d ret=%d\n", __func__, batt, ret);
}

static short afe_coul_get_ocv_temp(int batt)
{
	int ret;
	short temp = 0;

	ret = afe_coul_read_word(AFE_COUL_REG_OCV_TEMP, &temp, batt);
	hwlog_info("%s coul%d temp=%d, ret=%d\n", __func__, batt, temp, ret);
	return temp;
}

static void afe_coul_save_cc_uah(int batt, int cc_uah)
{
	afe_coul_reset_cc_reg(batt);
	afe_coul_set_battery_cc_over(batt, cc_uah);
}

static void afe_coul_clear_cc_register(int batt)
{
	struct afe_coul_device *di = NULL;

	di = afe_coul_get_di(batt);
	if (!di) {
		hwlog_err("%s coul_index%s not init\n", __func__, batt);
		return;
	}
	di->abs_cc_mah += afe_coul_calculate_cc_uah(batt);
	afe_coul_reset_cc_reg(batt);
	afe_coul_set_battery_cc_over(batt, 0);
}

static unsigned int afe_coul_get_last_coul_time(int batt)
{
	int ret;
	unsigned int last_time = 0;

	ret = afe_coul_read_block_data(AFE_COUL_REG_LAST_TIME, AFE_COUL_FOUR_BYTES, (unsigned char *)&last_time, batt);
	hwlog_info("%s last_time=%u, ret=%d\n", __func__, last_time, ret);
	return last_time;
}

static unsigned int afe_coul_get_coul_time(int batt)
{
	struct afe_coul_device *di = NULL;

	di = afe_coul_get_di(batt);
	if (!di) {
		hwlog_err("%s coul_index%s not init\n", __func__, batt);
		return (unsigned int)power_get_monotonic_boottime();
	}
	/* wait for power time init after resume */
	msleep(AFE_COUL_DELAY_10MS);
	return (unsigned int)power_get_monotonic_boottime() + di->coul_time_offset;
}

static void afe_coul_clear_coul_time(int batt)
{
	int ret, tmp;
	struct afe_coul_device *di = NULL;

	di = afe_coul_get_di(batt);
	if (!di) {
		hwlog_err("%s coul_index%s not init\n", __func__, batt);
		return;
	}
	tmp = 0;
	di->coul_time_offset = -afe_coul_get_coul_time(batt);
	ret = afe_coul_write_block_data(AFE_COUL_REG_LAST_TIME, AFE_COUL_FOUR_BYTES, (unsigned char *)&tmp, batt);
	hwlog_info("%s time_offset=%d, ret=%d\n", __func__, di->coul_time_offset, ret);
}

static void afe_coul_irq_enable(int batt)
{
	afe_coul_enable_volt_int(batt);
}

static void afe_coul_irq_disable(int batt)
{
	afe_coul_disable_volt_int(batt);
}

static void afe_coul_show_key_reg(void)
{
	int ret;
	unsigned short reg0, reg1;

	ret = afe_coul_read_word(AFE_COUL_REG_INT_FLAG, &reg0, AFE_COUL_INDEX_AUX_COUL);
	ret += afe_coul_read_word(AFE_COUL_REG_CHIP_STATUS, &reg1, AFE_COUL_INDEX_AUX_COUL);
	hwlog_info("%s int_reg=0x%x, status_reg=0x%x", __func__, reg0, reg1);
}

static void afe_coul_set_battery_move_magic(int batt, int move_flag)
{
	int ret;
	unsigned char val;

	if (move_flag)
		val = AFE_COUL_BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM;
	else
		val = AFE_COUL_BATTERY_MOVE_MAGIC_NUM;

	ret = afe_coul_write_byte(AFE_COUL_REG_BAT_MOVE, val, batt);
	hwlog_info("%s coul%d move_flag=%d, ret=%d\n", __func__, batt, val, ret);
}

static void afe_coul_save_last_soc(short soc)
{
	int ret;
	unsigned char regval;

	soc = (soc < 0) ? 0 : soc;
	regval = AFE_COUL_SAVE_LAST_SOC_FLAG |
		((unsigned char)soc & AFE_COUL_SAVE_LAST_SOC);

	ret = afe_coul_write_byte(AFE_COUL_REG_LAST_SOC, regval, AFE_COUL_INDEX_AUX_COUL);
	hwlog_info("%s soc=%d, ret=%d\n", __func__, soc, ret);
}

static void afe_coul_get_last_soc(short *soc)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_LAST_SOC, &regval, AFE_COUL_INDEX_AUX_COUL);
	*soc = (short)(regval & AFE_COUL_SAVE_LAST_SOC_FLAG);
	hwlog_info("%s soc=%d, ret=%d\n", __func__, *soc, ret);
}

static void afe_coul_get_last_soc_flag(bool *valid)
{
	int ret;
	unsigned char regval;

	ret = afe_coul_read_byte(AFE_COUL_REG_LAST_SOC, &regval, AFE_COUL_INDEX_AUX_COUL);
	*valid = regval & AFE_COUL_SAVE_LAST_SOC_FLAG;
	hwlog_info("%s last_soc_flag=%d, ret=%d\n", __func__, *valid, ret);
}

static void afe_coul_clear_last_soc_flag(void)
{
	int ret;

	ret = afe_coul_write_byte(AFE_COUL_REG_LAST_SOC, 0, AFE_COUL_INDEX_AUX_COUL);
	hwlog_info("%s clear flag, ret=%d\n", __func__, ret);
}

static int afe_coul_get_chip_temp_tenth(void)
{
	int ret;
	unsigned short regval;

	ret = afe_coul_read_word(AFE_COUL_REG_CHIP_TEMP, &regval, AFE_COUL_INDEX_AUX_COUL);
	regval = regval * AFE_COUL_TENTH / AFE_COUL_UNIT_CHIP_TEMP;
	hwlog_info("%s chip_temp=%d, ret=%d\n", __func__, regval, ret);
	return regval;
}

static void afe_coul_set_cc_max_threshold_mah(int batt, int max_mah)
{
	int ret;
	unsigned short regval = 0;
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di)
		return;

	ret = afe_coul_read_word(AFE_COUL_REG_OPCG2, &regval, batt);
	regval &= (~AFE_COUL_CC_MAX_MASK);
	regval |= ((unsigned short)(max_mah * di->r_sense / AFE_COUL_PERMILLAGE / AFE_COUL_TENTH) <<
		AFE_COUL_CC_MAX_BEGIN);
	ret += afe_coul_write_word(AFE_COUL_REG_OPCG2, regval, batt);
	hwlog_info("%s coul%d set_max_uah=%d, ret=%d\n", __func__, batt, max_mah, ret);
}

static void afe_coul_enable_cc_sampling(int batt)
{
	int ret;

	ret = afe_coul_set_regval_bit_u16(AFE_COUL_REG_OPCG1, AFE_COUL_CC_EN, 1, batt);
	hwlog_info("%s coul%d enable_cc_ret=%d\n", __func__, batt, ret);
}

static int afe_coul_dev_check(void)
{
	int ret;
	unsigned short regval = 0;

	ret = afe_coul_read_word(AFE_COUL_REG_VERSION, &regval, AFE_COUL_INDEX_AUX_COUL);
	if (ret) {
		hwlog_err("%s read chip id reg failed\n", __func__);
		return -ENOMEM;
	}

	if ((regval != AFE_COUL_ID_CELLWISE) && (regval != AFE_COUL_ID_SGM)) {
		hwlog_err("%s chipid=%d, not match\n", __func__, regval);
		return -ENODEV;
	}
	return 0;
}

static int afe_coul_get_abs_cc(int batt)
{
	struct afe_coul_device *di = NULL;

	di = afe_coul_get_di(batt);
	if (!di) {
		hwlog_info("%s coul%d chip not init\n", __func__, batt);
		return 0;
	}

	return di->abs_cc_mah;
}

/* not used for afe chip, but necessary for ops register */
static void afe_coul_cali_adc(int batt)
{
}

static int afe_coul_get_ate_a(void)
{
	return 0;
}

static int afe_coul_get_ate_b(void)
{
	return 0;
}

static short afe_coul_get_offset_current_mod(void)
{
	return 0;
}

static short afe_coul_get_offset_volt_mod(void)
{
	return 0;
}

static void afe_coul_set_offset_vol_mod(void)
{
}

static void afe_coul_enter_eco(int batt_index)
{
}

static void afe_coul_exit_eco(int batt_index)
{
}

static int afe_coul_calculate_eco_leak_uah(int batt)
{
	return 0;
}

static void afe_coul_cali_auto_off(void)
{
}

static void afe_coul_set_i_in_event_gate(int batt, int ma)
{
}

static void afe_coul_set_i_out_event_gate(int batt, int ma)
{
}

static int afe_coul_get_drained_battery_flag(int batt)
{
	return 0;
}

static void afe_coul_clear_drained_battery_flag(int batt)
{
}

static void afe_coul_set_bootocv_sample(int batt, unsigned char set_val)
{
}

static int afe_coul_set_cali_mode(int enable, int batt)
{
	int ret;

	if (enable)
		ret = afe_coul_select_work_mode(batt, AFE_COUL_CALI_MODE);
	else
		ret = afe_coul_select_work_mode(batt, AFE_COUL_LOW_POWER_MODE);

	return ret;
}

/* Regval = [(1- (gain_val / 1000000)) / (0.0625/2048)] + 2048
 *		  = 32678 * (1 - (gain_val / 1000000)) + 2048
 *		  = 34726 - 32678 * gain_val / 1000000
 */
static unsigned short afe_coul_convert_volt_gain_value(unsigned int val)
{
	return (AFE_COUL_VOLT_GAIN_OFFSET -
		(AFE_COUL_VOLT_GAIN_BASE * (uint64_t)val) / AFE_COUL_VOLT_GAIN_DEFAULT);
}

static int afe_coul_write_volt_gain(int batt, int gain_million)
{
	unsigned short regval;

	regval = afe_coul_convert_volt_gain_value(gain_million);
	return afe_coul_write_word(AFE_COUL_REG_VOLT_GAIN, regval, batt);
}

/* Regval = 4096 - (2048 * Cur_Gain) / 1000000 */
static unsigned short afe_coul_convert_cur_gain_value(unsigned int val)
{
	return (AFE_COUL_CURR_GAIN_OFFSET -
		((uint64_t)val * AFE_COUL_CURR_GAIN_BASE) / AFE_COUL_CURR_GAIN_DEFAULT);
}

static int afe_coul_write_cur_gain(int batt, int gain_million)
{
	unsigned short regval;

	regval = afe_coul_convert_cur_gain_value(gain_million);
	hwlog_info("%s regval=0x%4x\n", __func__, regval);
	return afe_coul_write_word(AFE_COUL_REG_CURR_GAIN, regval, batt);
}

/* Fac cali define: I_cali = k * I_coul + b
 * AFE reg define: I_cali = k' *  (I_coul - b')
 * thus, k = k' and b' = - b / k' = -b / k
 * According to Current_offset_Reg = Current_offset + 2048
 * reg_val = b' + 2048
 */
static unsigned short afe_coul_convert_cur_offset_value(int batt, int offset_ua, int gain_million)
{
	s64 c_offset_uv;
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di)
		return 0;

	if (!gain_million)
		gain_million = AFE_COUL_CURR_GAIN_DEFAULT;

	c_offset_uv = 0 - ((s64)offset_ua * di->r_sense * AFE_COUL_HUNDRED / gain_million);
	return AFE_COUL_COE_C_OFFSET + (c_offset_uv * AFE_COUL_CUR_REG_B / AFE_COUL_CUR_REG_A);
}

static int afe_coul_write_cur_offset(int batt, int offset_ua, int gain_million)
{
	unsigned short regval;

	regval = afe_coul_convert_cur_offset_value(batt, offset_ua, gain_million);
	hwlog_info("%s regval=0x%4x\n", __func__, regval);
	return afe_coul_write_word(AFE_COUL_REG_CURR_OFFSET, regval, batt);
}

static int afe_coul_set_volt_gain(unsigned int val, int batt)
{
	int ret = 0;

	if ((val > AFE_COUL_VOLT_GAIN_MAX) || (val < AFE_COUL_VOLT_GAIN_MIN)) {
		hwlog_err("%s val %d out of range\n", __func__, val);
		return ret;
	}

	ret = afe_coul_write_volt_gain(batt, val);
	hwlog_info("%s coul%d set volt_gain=%d, ret=%d\n", __func__, batt, val, ret);
	return ret;
}

static int afe_coul_set_curr_gain(unsigned int val, int batt)
{
	int ret = 0;

	if ((val > AFE_COUL_CURR_GAIN_MAX) || (val < AFE_COUL_CURR_GAIN_MIN)) {
		hwlog_err("%s val %d out of range\n", __func__, val);
		return ret;
	}

	ret = afe_coul_write_cur_gain(batt, val);
	hwlog_info("%s coul%d set cur_gain=%d, ret=%d\n", __func__, batt, val, ret);
	return ret;
}

static int afe_coul_set_curr_offset(struct coul_cali_params *params, int batt)
{
	int ret = 0;
	int offset = params->c_offset_b;
	struct afe_coul_device *di = afe_coul_get_di(batt);

	if (!di)
		return -ENODEV;

	/* limit current offset in [-200, 200]mA */
	if ((offset > AFE_COUL_CURR_OFFSET_MAX) || (offset < AFE_COUL_CURR_OFFSET_MIN)) {
		hwlog_err("%s offset_val %d out of range\n", __func__, offset);
		return ret;
	}

	if (di->force_offset_cali && (offset == 0)) {
		offset = di->c_offset_ua;
		hwlog_info("%s force offset cali, offset=%duA\n", __func__, offset);
	}

	ret = afe_coul_write_cur_offset(batt, offset, params->c_offset_a);
	hwlog_info("%s coul%d set cur_offset=%d, ret=%d\n", __func__, batt, offset, ret);
	return ret;
}

static void afe_coul_set_dev_cali_param(int batt, struct coul_cali_params *params)
{
	int ret;

	ret = afe_coul_set_cali_mode(1, batt);
	msleep(AFE_COUL_DELAY_500MS); /* wait for mode switch */
	ret += afe_coul_set_volt_gain(params->v_offset_a, batt);
	ret += afe_coul_set_curr_gain(params->c_offset_a, batt);
	ret += afe_coul_set_curr_offset(params, batt);
	ret += afe_coul_set_cali_mode(0, batt);
	hwlog_info("%s v_gain=%d, c_gain=%d, ret=%d\n", __func__, params->v_offset_a,
		params->c_offset_a, ret);
}

static struct coul_device_ops afe_coul_ops = {
	.calculate_cc_uah = afe_coul_calculate_cc_uah,
	.save_cc_uah = afe_coul_save_cc_uah,
	.convert_ocv_regval2ua = afe_coul_convert_ocv_regval2ua,
	.convert_ocv_regval2uv = afe_coul_convert_ocv_regval2uv,
	.is_battery_moved = afe_coul_is_battery_moved,
	.set_battery_moved_magic_num = afe_coul_set_battery_move_magic,
	.get_fifo_avg_data = afe_coul_get_fifo_avg_data,
	.get_fifo_depth = afe_coul_get_fifo_depth,
	.get_delta_rc_ignore_flag = afe_coul_get_delta_rc_ignore_flag,
	.get_nv_read_flag = afe_coul_get_nv_read_flag,
	.set_nv_save_flag = afe_coul_set_nv_save_flag,
	.get_use_saved_ocv_flag = afe_coul_get_use_saved_ocv_flag,
	.get_fcc_invalid_up_flag = afe_coul_get_fcc_invalid_up_flag,
	.set_low_low_int_val = afe_coul_set_low_volt_int,
	.save_ocv = afe_coul_save_ocv,
	.get_ocv = afe_coul_get_ocv,
	.clear_ocv = afe_coul_clear_ocv,
	.save_ocv_temp = afe_coul_save_ocv_temp_tenth,
	.get_ocv_temp = afe_coul_get_ocv_temp,
	.clear_ocv_temp = afe_coul_clear_ocv_temp,
	.get_abs_cc = afe_coul_get_abs_cc,
	.get_coul_time = afe_coul_get_coul_time,
	.clear_coul_time = afe_coul_clear_coul_time,
	.clear_cc_register = afe_coul_clear_cc_register,
	.cali_adc = afe_coul_cali_adc,
	.get_battery_voltage_uv = afe_coul_get_battery_volt_uv,
	.get_battery_current_ua = afe_coul_get_battery_cur_ua,
	.get_battery_vol_uv_from_fifo = afe_coul_get_battery_volt_from_fifo_uv,
	.get_battery_cur_ua_from_fifo = afe_coul_get_battery_cur_from_fifo_ua,
	.get_offset_current_mod = afe_coul_get_offset_current_mod,
	.get_offset_vol_mod = afe_coul_get_offset_volt_mod,
	.set_offset_vol_mod = afe_coul_set_offset_vol_mod,
	.get_ate_a = afe_coul_get_ate_a,
	.get_ate_b = afe_coul_get_ate_b,
	.irq_enable = afe_coul_irq_enable,
	.irq_disable = afe_coul_irq_disable,
	.show_key_reg = afe_coul_show_key_reg,
	.enter_eco = afe_coul_enter_eco,
	.exit_eco = afe_coul_exit_eco,
	.calculate_eco_leak_uah = afe_coul_calculate_eco_leak_uah,
	.save_last_soc = afe_coul_save_last_soc,
	.get_last_soc = afe_coul_get_last_soc,
	.clear_last_soc_flag = afe_coul_clear_last_soc_flag,
	.get_last_soc_flag = afe_coul_get_last_soc_flag,
	.cali_auto_off = afe_coul_cali_auto_off,
	.save_ocv_level = afe_coul_save_ocv_level,
	.get_ocv_level = afe_coul_get_ocv_level,
	.set_i_in_event_gate = afe_coul_set_i_in_event_gate,
	.set_i_out_event_gate = afe_coul_set_i_out_event_gate,
	.get_chip_temp = afe_coul_get_chip_temp_tenth,
	.get_bat_temp = afe_coul_get_battery_temp,
	.convert_regval2uv = afe_coul_convert_regval2uv,
	.convert_regval2ua = afe_coul_convert_regval2ua,
	.convert_regval2temp = afe_coul_convert_regval2temp,
	.convert_uv2regval = afe_coul_convert_uv2regval,
	.convert_regval2uah = afe_coul_convert_regval2uah,
	.get_drained_battery_flag = afe_coul_get_drained_battery_flag,
	.clear_drained_battery_flag = afe_coul_clear_drained_battery_flag,
	.set_bootocv_sample = afe_coul_set_bootocv_sample,
	.set_dev_cali_param = afe_coul_set_dev_cali_param,
	.coul_dev_check = afe_coul_dev_check,
};

static int afe_coul_ic_check(struct i2c_client *i2c)
{
	int ret;
	unsigned short regval;

	if (!i2c) {
		hwlog_err("%s p_i2c is null\n", __func__);
		return -ENOMEM;
	}
	/* 1: big endian slave chip */
	ret = power_i2c_u8_read_word(i2c, AFE_COUL_REG_VERSION, &regval, 1);
	if (ret || ((regval != AFE_COUL_ID_CELLWISE) && (regval != AFE_COUL_ID_SGM))) {
		hwlog_err("%s chipid 0x%x not match\n", __func__, regval);
		return -ENODEV;
	}
	return 0;
}

static void init_device_pointer(struct afe_coul_device *di, struct i2c_client *i2c)
{
	di->client = i2c;
	di->dev = &i2c->dev;
	i2c_set_clientdata(i2c, di);
	g_afe_coul_dev = di;
	di->abs_cc_mah = 0;
	di->batt_moved_flag = 0;
}

static void deinit_device_pointer(struct afe_coul_device *di)
{
	g_afe_coul_dev = NULL;
}

static int afe_coul_parse_dt(struct device *dev, struct afe_coul_device *di)
{
	struct device_node *np = dev->of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "r_sense",
		&di->r_sense, AFE_COUL_RESISTANCE_SENSE);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "coul_index",
		&di->coul_index, AFE_COUL_INDEX_AUX_COUL);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "force_offset_cali",
		&di->force_offset_cali, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "c_offset_ua",
		&di->c_offset_ua, 0);

	hwlog_info("%s r_sense=%d, coul_index=%d, force_cali=%d, c_offset=%d\n", __func__,
		di->r_sense, di->coul_index, di->force_offset_cali, di->c_offset_ua);
	return (di->r_sense == 0);
}

static void afe_coul_interrupt_work(struct work_struct *work)
{
	unsigned short flag;
	struct afe_coul_device *di = container_of(
		work, struct afe_coul_device, irq_work.work);

	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return;
	}

	flag = afe_coul_get_int_flag(di->coul_index);
	if (flag & AFE_COUL_VOLT_LO_IRQ) {
		hwlog_info("%s low batt volt irq\n", __func__);
		coul_core_notify_fault(di->coul_index, COUL_FAULT_LOW_VOL, NULL);
	}

	if (flag & AFE_COUL_CC_HI_IRQ) {
		hwlog_info("%s cc overflow irq\n", __func__);
		coul_core_notify_fault(di->coul_index, COUL_FAULT_CL_IN, NULL);
	}
}

static irqreturn_t afe_coul_interrupt(int irq, void *_di)
{
	struct afe_coul_device *di = _di;

	queue_delayed_work(di->afe_workqueue, &di->irq_work, msecs_to_jiffies(0));

	return IRQ_HANDLED;
}

static int afe_coul_irq_init(struct afe_coul_device *di, struct device_node *np)
{
	int ret;

	di->afe_workqueue = create_singlethread_workqueue("afe_coul_int_irq");
	INIT_DELAYED_WORK(&di->irq_work, afe_coul_interrupt_work);

	ret = power_gpio_config_interrupt(np, "gpio_int", "afe_coul_gpio_int",
		&di->int_gpio, &di->irq);
	if (ret)
		return ret;

	ret = request_irq(di->irq, afe_coul_interrupt,
		IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING, "afe_coul_int_irq", di);
	if (ret) {
		hwlog_err("%s gpio irq request failed\n", __func__);
		di->irq = -1;
		gpio_free(di->int_gpio);
		return ret;
	}

	afe_coul_set_cc_max_threshold_mah(di->coul_index, AFE_COUL_MAX_CC_MAH);
	afe_coul_irq_enable(di->coul_index);

	return ret;
}

static int afe_coul_chip_init(struct afe_coul_device *di)
{
	afe_coul_enable_battery_present_detect(di->coul_index);
	afe_coul_enable_cc_sampling(di->coul_index);
	di->coul_time_offset = afe_coul_get_last_coul_time(di->coul_index);
	return afe_coul_select_work_mode(di->coul_index, AFE_COUL_LOW_POWER_MODE);
}

static int afe_coul_bsp_init(struct afe_coul_device *di)
{
	int ret;

	ret = afe_coul_irq_init(di, di->dev->of_node);
	if (ret) {
		hwlog_err("AFE init irq fail\n");
		return ret;
	}

	ret = afe_coul_chip_init(di);
	if (ret)
		hwlog_err("AFE chip init fail\n");

	return ret;
}

static int afe_coul_hardware_algo_register(struct device_node *root)
{
	int ret;
	struct device_node *child = NULL;

	ret = -ENODEV;
	for_each_child_of_node(root, child)
		ret = coul_core_register(&afe_coul_ops, child);

	hwlog_info("%s coul_device_ops register ret=%d", __func__, ret);
	return ret;
}

static int afe_coul_ops_register(struct afe_coul_device *di)
{
	return afe_coul_hardware_algo_register(di->dev->of_node);
}

static void afe_coul_chip_init_fail_handle(struct afe_coul_device *di)
{
	afe_coul_irq_disable(di->coul_index);
	free_irq(di->irq, di);
	gpio_free(di->int_gpio);
	cancel_delayed_work(&di->irq_work);
}

static void afe_coul_save_coul_time_before_shutdown(int batt)
{
	int ret;
	unsigned int cur_time = afe_coul_get_coul_time(batt);

	ret = afe_coul_write_block_data(AFE_COUL_REG_LAST_TIME, AFE_COUL_FOUR_BYTES, (unsigned char *)&cur_time, batt);
	hwlog_info("%s coul_time=%u, ret=%d\n", __func__, cur_time, ret);
}

static int afe_coul_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct afe_coul_device *di = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	ret = afe_coul_ic_check(client);
	if (ret)
		goto chip_id_error;

	di = (struct afe_coul_device*)devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	init_device_pointer(di, client);
	ret = afe_coul_parse_dt(di->dev, di);
	if (ret) {
		hwlog_err("afe_coul parse dt fail\n");
		goto parse_dt_fail;
	}

	ret = afe_coul_bsp_init(di);
	if (ret) {
		hwlog_err("AFE init irq fail\n");
		goto chip_init_fail;
	}

	ret = afe_coul_ops_register(di);
	if (ret) {
		hwlog_err("ops register failed\n");
		goto ops_error;
	}

	hwlog_info("afe_coul probe ok\n");
	return 0;

ops_error:
	afe_coul_select_work_mode(di->coul_index, AFE_COUL_SHIPPING_MODE);
chip_init_fail:
	afe_coul_chip_init_fail_handle(di);
parse_dt_fail:
	deinit_device_pointer(di);
	devm_kfree(&client->dev, di);
chip_id_error:
	hwlog_err("afe_coul probe failed!\n");
	return ret;
}

static void afe_coul_i2c_shutdown(struct i2c_client *client)
{
	struct afe_coul_device *di = i2c_get_clientdata(client);

	if (!di)
		return;

	afe_coul_save_coul_time_before_shutdown(di->coul_index);
	afe_coul_select_work_mode(di->coul_index, AFE_COUL_SHIPPING_MODE);
	afe_coul_reset_fifo(di->coul_index);
	afe_coul_irq_disable(di->coul_index);
	free_irq(di->irq, di);
	gpio_free(di->int_gpio);
	cancel_delayed_work(&di->irq_work);
	g_afe_coul_dev = NULL;
	devm_kfree(&client->dev, di);
}

static const struct i2c_device_id afe_coul_i2c_id[] = {
	{ "afe_coul", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, afe_coul_i2c_id);

static const struct of_device_id afe_coul_match_table[] = {
	{ .compatible = "cellwise,afe_coul", },
	{},
}
MODULE_DEVICE_TABLE(of, afe_coul_match_table);

static struct i2c_driver afe_coul_i2c_driver = {
	.driver = {
		.name = "afe_coul",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(afe_coul_match_table),
	},
	.probe = afe_coul_i2c_probe,
	.shutdown = afe_coul_i2c_shutdown,
	.id_table = afe_coul_i2c_id,
};

static int __init afe_coul_init(void)
{
	return i2c_add_driver(&afe_coul_i2c_driver);
}

static void __exit afe_coul_exit(void)
{
	i2c_del_driver(&afe_coul_i2c_driver);
}

fs_initcall_sync(afe_coul_init);
module_exit(afe_coul_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("afe_coul AFE Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");