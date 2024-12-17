/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2030. All rights reserved.
 * Description: coulometer hardware driver headfile
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
#include <platform_include/basicplatform/linux/spmi_platform.h>
#include <platform_include/basicplatform/linux/of_platform_spmi.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "../../coul_algo/coul_nv.h"
#include "../../coul_algo/coul_temp.h"
#include "../../coul_algo/coul_dts.h"
#include "hi6xxx_coul.h"

static struct pmu_coul_cali_params {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
};
static struct pmu_coul_device_info {
	struct device *dev;
	struct delayed_work cali_params_get_work;
	struct delayed_work read_temperature_delayed_work;
	u16 chip_proj;
	u16 chip_version;
	int r_pcb;
	int is_nv_read;
	int batt_index; /* battery index */
	char batt_name[BATT_NAME_SIZE_MAX];
	int batt_temp; /* temperature in degree * 10 */
	int batt_exist;
	struct coul_merge_drv_ops *ops;
	struct ss_coul_nv_info nv_info;
	struct pmu_coul_cali_params cali_params;
};

static struct pmu_coul_device_info *g_coul_hardware_dev;
static u64 g_last_eco_in;
static u64 g_last_eco_out;
static int g_r_coul_uohm = R_COUL_UOHM;

#define COUL_NV_NAME "HICOUL"
#define COUL_NV_NUM  316

#define COUL_NV_NAME2 "HICOUL2"
#define COUL_NV_NUM2  503

#define COUL_NV_NAME3 "HICOUL3"
#define COUL_NV_NUM3  506

#define READ_NV_MS               (5 * 1000)
#define READ_TEMPERATURE_MS      (5 * 1000)

#define VOL_CUR_BUF_DEPTH       3
#define ABNORMAL_BATT_TEMPERATURE_LOW   (-40)
#define ABNORMAL_BATT_TEMPERATURE_HIGH  80

static void coul_cali_params_get(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b)
{
	struct pmu_coul_device_info *di = g_coul_hardware_dev;
	struct pmu_coul_cali_params *params = NULL;

	if (di == NULL)
		return;

	params = &di->cali_params;

	/* The parameter may be is null */
	if (v_offset_a)
		*v_offset_a = params->v_offset_a;

	if (v_offset_b)
		*v_offset_b = params->v_offset_b;

	if (c_offset_a)
		*c_offset_a = params->c_offset_a;

	if (c_offset_b)
		*c_offset_b = params->c_offset_b;
}

/* NTC Table */
static int g_t_v_table[][2] = {
	{ -273, 4095 }, { -40, 3764 }, { -36, 3689 }, { -32, 3602 },
	{ -28, 3500 }, { -24, 3387 }, { -20, 3261 }, { -16, 3122 },
	{ -12, 2973 }, { -8, 2814 }, { -4, 2650 }, { 0, 2480 }, { 4, 2308 },
	{ 8, 2136 }, { 12, 1967 }, { 16, 1803 }, { 20, 1646 }, { 24, 1497 },
	{ 28, 1360 }, { 32, 1230 }, { 36, 1111 }, { 40, 1001 }, { 44, 903 },
	{ 48, 812 }, { 52, 729 }, { 56, 655 }, { 60, 590 }, { 64, 531 },
	{ 74, 406 }, { 84, 313 }, { 125, 110 }, { 0, 0 },
};
#define T_V_ARRAY_LENGTH        31 /* NTC Table length */

static int pmu_adc_to_temp_permille(int temp_volt)
{
	int i;
	int temprature = 0;

	if (temp_volt >= g_t_v_table[0][1])
		return g_t_v_table[0][0];
	if (temp_volt <= g_t_v_table[T_V_ARRAY_LENGTH - 1][1])
		return g_t_v_table[T_V_ARRAY_LENGTH - 1][0];
	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		if (temp_volt == g_t_v_table[i][1])
			return g_t_v_table[i][0];
		if (temp_volt > g_t_v_table[i][1])
			break;
	}
	if (i == 0)
		return g_t_v_table[0][0];
	if ((g_t_v_table[i][1] - g_t_v_table[i - 1][1]) != 0)
		temprature = g_t_v_table[i - 1][0] +
			(long)(temp_volt - g_t_v_table[i - 1][1]) *
			(g_t_v_table[i][0] - g_t_v_table[i - 1][0]) /
			(g_t_v_table[i][1] - g_t_v_table[i - 1][1]);
	return temprature;
}

/* Description: Get the temp of battery by soh */
static int pmu_coul_get_tbat(void)
{
	u16 tbat_code = 0;
	int tbat;

	/* get adc data */
	coul_regs_read(COUL_HARDWARE_SOH_TBAT_DATA_BASE, &tbat_code, 2); /* 2: regs_num */
	tbat = pmu_adc_to_temp_permille(tbat_code);
	coul_hardware_inf("%s code %d, tbat %d\n", __func__, tbat_code, tbat);
	return tbat;
}

static int pmu_coul_battery_retry_temp_permille(struct pmu_coul_device_info *di)
{
	if (di == NULL)
		goto error;

	return pmu_coul_get_tbat() * PERMILLAGE;

error:
	coul_hardware_err("%s error\n", __func__);
	return DEFAULT_TEMP * PERMILLAGE;
}

static int pmu_coul_battery_get_temperature_tenth_degree(struct pmu_coul_device_info *di)
{
	return pmu_coul_battery_retry_temp_permille(di) / PERCENT;
}

static int pmu_coul_core_get_battery_temperature(void *data)
{
	struct pmu_coul_device_info *di = data;

	if (di == NULL) {
		coul_hardware_err("error, di is NULL, return default temp\n");
		return DEFAULT_TEMP;
	}
	return (di->batt_temp / TENTH);
}

static void pmu_coul_update_battery_temperature(struct pmu_coul_device_info *di)
{
	int temp;

	if (di == NULL) {
		coul_core_err("%s, di is NULL\n", __func__);
		return;
	}
	temp = pmu_coul_battery_get_temperature_tenth_degree(di);
	di->batt_temp = temp;
}

static void pmu_read_temperature_work(struct work_struct *work)
{
	struct pmu_coul_device_info *di =
		container_of(work, struct pmu_coul_device_info,
			read_temperature_delayed_work.work);

	pmu_coul_update_battery_temperature(di);
	queue_delayed_work(system_power_efficient_wq, &di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));
}

/*
 * Description:  convert register value to uv
 * Input: reg_val:voltage reg value
 * Return: value of register in uV
 */
static int coul_hardware_convert_regval2uv(unsigned int reg_val)
{
	s64 temp;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;
	int val;
	int ret;

	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);
	/*
	 * uv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b
	 *     = code * 13 *a / 2^24 + b (a = 1000000, b = 0)
	 * 1bit = 0.77486 uV (Total 24bit)
	 */
	if (reg_val & COUL_BIT_MASK) {
		reg_val |= COUL_NEG_EXPAND;
		val = ((~reg_val) + 1) & (~COUL_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	temp = (s64)val * 13;
	temp = temp * (s64)(v_offset_a);
	temp = div_s64(temp, 16777216);
	temp += v_offset_b;

	ret = (int)temp;

	coul_hardware_dbg(
		"[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, ret);

	if (reg_val & COUL_BIT_MASK)
		return -ret;
	return ret;
}

/*
 * Description: convert register value to current(uA)
 * Remark:
 *  if Rsense in mohm, Current in mA
 *  if Rsense in uohm, Current in uA
 *  high bit = 1 is in, 0 is out
 */
static int coul_hardware_convert_regval2ua(unsigned int reg_val)
{
	int ret;
	s64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;
	int c_offset_b = DEFAULT_C_OFF_B;

	int val;
	/*
	 *  6x21V700:Current = code * / 2^(n-1) * 1.3 * (1000/10) * (1000/Rsense)
	 *                 = code *130000 / Rsense / 2^(n-1)
	 *  6x21V800:Current = code * / 2^(n-1) * 0.65 * (1000/10) * (1000/Rsense)
	 *                 = code *130000 / Rsense / 2^(n-1)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, &c_offset_b);
	if (reg_val & COUL_BIT_MASK) {
		reg_val |= COUL_NEG_EXPAND;
		val = ((~reg_val) + 1) & (~COUL_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	temp = (s64)val * 130000;
	temp = div_s64(temp * 1000, g_r_coul_uohm);
	temp = temp * 1000;
	temp = div_s64(temp, 8388608);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);
	ret += c_offset_b;
	ret = ret / 2;

	coul_hardware_dbg("[%s] h%d reg_val 0x%lx, ret %d\n", __func__, __LINE__,
		reg_val, ret);

	if (reg_val & COUL_BIT_MASK)
		return -ret;

	return ret;
}


static int coul_hardware_get_battery_voltage_uv(void)
{
	int uv;
	unsigned int regval = 0;

	coul_regs_read(COUL_HARDWARE_V_OUT, &regval, REG_NUM);
	uv = coul_hardware_convert_regval2uv(regval);

	coul_hardware_dbg("[%s] 0x%x, uv %d\n", __func__, regval, uv);
	return uv;
}

static int coul_get_battery_current_ua(void)
{
	int ua;
	unsigned int regval = 0;

	coul_regs_read(COUL_HARDWARE_CURRENT, &regval, REG_NUM);
	ua = coul_hardware_convert_regval2ua(regval);
	coul_hardware_dbg("[%s] 0x%x, ua %d\n", __func__, regval, ua);
	return ua;
}


/* Return: 0:success -1:fail */
static int coul_check_version(struct pmu_coul_device_info *di)
{
	int tryloop = 0;
	u8 ver[6] = {0}; /* 6:  num of  coul version name */

	do {
		coul_regs_read(COUL_HARDWARE_VERSION_ADDR, &ver, 6); /* 6: regs_num */
		coul_hardware_inf("do a dummy read, version is 0x%x\n", ver[0]);
		usleep_range(500, 510); /* 500: min delay 500us, 510: max delay 510us */
		/* 5: try times of get coul version */
		if ((tryloop++) > 5) {
			coul_hardware_err("version is not correct!\n");
			return -1;
		}
	} while ((ver[0] & 0xff) != COUL_HARDWARE);

	/* 12: left shift 12 bits, 8: left shift 8 bits, 4: left shift 4 bits */
	di->chip_proj = ((ver[0] & 0x0f) << 12) | ((ver[1] & 0x0f) << 8) |
		((ver[2] & 0x0f) << 4) | (ver[3] & 0x0f);
	di->chip_version = (ver[4] << 8) | ver[5];

	return 0;
}

/*
 * Description:  convert register value to current(uA)
 * Input:  reg_val:current reg value
 * Return: value of register in uA
 * Remark:
 *  6x21V700: Code = mA * 2^23 / 1.3 * 10 * Rsense / 1000 / 1000
 *  6x21V800&6x55V300: Code = mA * 2^23 / 0.65 * 10 * Rsense / 1000 / 1000
 *  high bit = 1 is in, 0 is out,  Rsense in mohm
 *  only for 6x21V700 & V800 & V300
 * 1000000: gain to ensure calculation accuracy
 */
static unsigned int coul_convert_ua2regval(int ua)
{
	unsigned int ret;
	s64 temp;
	int val = ua;
	int c_offset_a = DEFAULT_C_OFF_A;

	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);

	val = val * 2;

	temp = (s64)val * 1000000;
	if (c_offset_a != 0) {
		temp = div_s64(temp, c_offset_a);
	} else {
		coul_hardware_err("c_offset_a = 0, division by default value %d\n",
			DEFAULT_C_OFF_A);
		temp = div_s64(temp, DEFAULT_C_OFF_A);
	}

	temp = temp * 8388608;
	temp = temp * 100 / 1000 * (s64)g_r_coul_uohm;
	temp = div_s64(temp, 13);
	temp = div_s64(temp, 1000000000);

	ret = (unsigned int)temp;

	coul_hardware_dbg("[%s] ua 0x%lx, ret %d\n", __func__, ua, ret);

	return ret;
}

static void coul_eco_delay_enable(u8 enable)
{
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_ECO_CONFIG_ADDR);
	val = (val & ~(ECO_DELAY_EN_MASK)) | (enable << ECO_DELAY_EN_SHIFT);
	coul_reg_write(COUL_HARDWARE_ECO_CONFIG_ADDR, val);
}

static void coul_wait_comp_enable(u8 enable)
{
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_WAIT_COMP_ADDR);
	val = (val & ~(WAIT_COMP_EN_MASK)) | (enable << WAIT_COMP_EN_SHIFT);
	coul_reg_write(COUL_HARDWARE_WAIT_COMP_ADDR, val);
}

/* Description: config init */
static void coul_chip_init(void)
{
	/* unmask coul eco */
	udelay(110); /* 110: delay time, us */
#if defined(CONFIG_COUL_PMIC6X55V500) || defined(CONFIG_COUL_PMIC6X21V900)
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK, 0);
#else
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK_0, 0);
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK_1, 0);
#endif
	/* config coul Mode */
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, DEFAULT_COUL_CTRL_VAL);

	coul_wait_comp_enable(PMU_ENABLE);
}

/* Description: clear coul vol/current fifo value */
static void coul_clear_fifo(void)
{
	unsigned char reg_value;

	reg_value = coul_reg_read(COUL_HARDWARE_FIFO_CLEAR);
	coul_reg_write(COUL_HARDWARE_FIFO_CLEAR, (reg_value | COUL_FIFO_CLEAR));
}

/* Description: clear coul vol/current eco fifo value */
static void coul_clear_enable_eco_fifo(void)
{
	unsigned char reg_value;

	reg_value = coul_reg_read(COUL_HARDWARE_ECO_FIFO_CLEAR);
	coul_reg_write(COUL_HARDWARE_ECO_FIFO_CLEAR,
		(reg_value | ECO_FIFO_CLEAR | ECO_FIFO_EN));
}

/* Remark: coul eco follow pmu eco */
static void coul_enter_eco(void)
{
	unsigned char reg_val;
	u64 eco_in_reg = 0;
	u64 eco_out_reg = 0;

	coul_regs_read(COUL_HARDWARE_ECO_OUT_CLIN_REG_BASE, &eco_in_reg, CC_REG_NUM);
	coul_regs_read(
		COUL_HARDWARE_ECO_OUT_CLOUT_REG_BASE, &eco_out_reg, CC_REG_NUM);

	g_last_eco_in = eco_in_reg;
	g_last_eco_out = eco_out_reg;

	reg_val = ECO_COUL_CTRL_VAL;

	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, reg_val);
	coul_clear_fifo();

	coul_clear_enable_eco_fifo();

	coul_eco_delay_enable(PMU_ENABLE);
}

/* Remark: coul eco follow pmu eco */
static void coul_exit_eco(void)
{
	coul_clear_fifo();
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, DEFAULT_COUL_CTRL_VAL);

	coul_eco_delay_enable(PMU_DISABLE);
}

/*
 * Description:  wrrite reg by mask
 * Input: reg addr, mask bit, bit value
 */
static void reg_write_mask(u16 addr, u8 mask, u8 value)
{
	u8 data;

	data = coul_reg_read(addr);
	data = data & ~mask;
	data = data | value;
	coul_reg_write(addr, data);
}

#ifdef CONFIG_PM
/* suspend function, called when coul enter sleep */
static int pmu_coul_hardware_suspend(struct spmi_device *pdev, pm_message_t state)
{
	coul_hardware_inf("%s: +\n", __func__);

	coul_enter_eco();

	coul_hardware_inf("%s: -\n", __func__);

	return 0;
}

/* called when coul wakeup from deep sleep */
static int pmu_coul_hardware_resume(struct spmi_device *pdev)
{
	coul_hardware_inf("%s: +\n", __func__);

	coul_exit_eco();

	coul_hardware_inf("%s: -\n", __func__);

	return 0;
}
#endif

static void coul_set_cali_param(struct pmu_coul_cali_params *params)
{
	struct pmu_coul_device_info *di = g_coul_hardware_dev;

	if (di == NULL) {
		coul_hardware_err("[%s]di is null\n", __func__);
		return;
	}

	(void)memcpy_s(&di->cali_params,
		sizeof(struct pmu_coul_cali_params), params, sizeof(*params));

	coul_hardware_inf("%s batt %d, v_offset_a %d, v_offset_b %d, c_offset_a %d, c_offset_b %d\n",
		__func__, di->batt_index, params->v_offset_a, params->v_offset_b,
		params->c_offset_a, params->c_offset_b);
}

static void coul_set_default_cali_param(struct pmu_coul_device_info *di)
{
	di->cali_params.v_offset_a = DEFAULT_V_OFF_A;
	di->cali_params.v_offset_b = DEFAULT_V_OFF_B;
	di->cali_params.c_offset_a = DEFAULT_C_OFF_A;
	di->cali_params.c_offset_b = DEFAULT_C_OFF_B;
}

/* check whether coul is ok 0: success  1: fail */
static int coul_ic_check(void)
{
	u8 val;
	struct pmu_coul_device_info *di = g_coul_hardware_dev;

	if (!di) {
		coul_hardware_err("%s di is null.\n", __func__);
		return 1;
	}

	val = coul_reg_read(0);
	if (val == VERSION_VAL) {
		return 0;
	} else {
		coul_hardware_err("%s fail, 0x%x\n", __func__, val);
		return 1;
	}
}

 /* check wheather coul is ready 1: OK 0:not ready */
static int pmu_coul_is_ready(void *data)
{
	struct pmu_coul_device_info *di = (struct pmu_coul_device_info *)data;

	if (di != NULL)
		return 1;
	return 0;
}

/* check whether coul is ok 0: success  1: fail */
static int pmu_coul_device_check(void *data)
{
	return coul_ic_check();
}

static int pmu_coul_save_nv_info(struct pmu_coul_device_info *di)
{
	int ret;
	struct opt_nve_info_user nve;
	struct ss_coul_nv_info *pinfo = NULL;

	if (di == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return -1;
	}

	pinfo = &di->nv_info;

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if (di->batt_index == BATT_0) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME, sizeof(COUL_NV_NAME));
		nve.nv_number = COUL_NV_NUM;
	} else if (di->batt_index == BATT_1) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME2, sizeof(COUL_NV_NAME2));
		nve.nv_number = COUL_NV_NUM2;
	} else if (di->batt_index == BATT_2) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME3, sizeof(COUL_NV_NAME3));
		nve.nv_number = COUL_NV_NUM3;
	}

	nve.valid_size = (uint32_t)sizeof(*pinfo);
	nve.nv_operation = NV_WRITE;

	ret = memcpy_s(nve.nv_data, NVE_NV_DATA_SIZE, pinfo, sizeof(*pinfo));
	if (ret != EOK) {
		coul_hardware_err("memcpy_s failed, ret=%d\n", ret);
		return -1;
	}

	/* here save info in register */
	ret = nve_direct_access_interface(&nve);
	if (ret)
		coul_hardware_inf("save nv partion failed, ret=%d\n", ret);

	return ret;
}

static int pmu_coul_cali_info_set(struct pmu_coul_device_info *di, struct coul_batt_cali_info *info)
{
	if (di == NULL || info == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return -1;
	}

	if (info->charge_mode  == PARALLEL_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			di->nv_info.v_offset_a =
				di->cali_params.v_offset_a = info->a;
			di->nv_info.v_offset_b =
				di->cali_params.v_offset_b = info->b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			di->nv_info.c_offset_a =
				di->cali_params.c_offset_a = info->a;
			di->nv_info.c_offset_b =
				di->cali_params.c_offset_b = info->b;
		}
	}

	coul_set_cali_param(&di->cali_params);
	return pmu_coul_save_nv_info(di);
}

static int pmu_coul_set_cali_info(void *data, struct coul_batt_cali_info *info)
{
	struct pmu_coul_device_info *di = (struct pmu_coul_device_info *)data;

	if (di == NULL)
		return 0;

	return pmu_coul_cali_info_set(di, info);
}

static int pmu_coul_cali_info_get(struct pmu_coul_device_info *di, struct coul_batt_cali_info *info)
{
	if (di == NULL || info == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return -1;
	}

	if (info->charge_mode  == PARALLEL_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			info->a = di->cali_params.v_offset_a;
			info->b = di->cali_params.v_offset_b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			info->a = di->cali_params.c_offset_a;
			info->b = di->cali_params.c_offset_b;
		}
	}
	return 0;
}

static int pmu_coul_get_cali_info(void *data, struct coul_batt_cali_info *info)
{
	struct pmu_coul_device_info *di = (struct pmu_coul_device_info *)data;

	if (di == NULL)
		return -1;

	return pmu_coul_cali_info_get(di, info);
}

static void pmu_coul_core_get_battery_voltage_and_current(
	struct pmu_coul_device_info *di, int *ibat_ua, int *vbat_uv)
{
	int vol[VOL_CUR_BUF_DEPTH], cur[VOL_CUR_BUF_DEPTH];
	int i;

	if ((di == NULL) ||
		(ibat_ua == NULL) || (vbat_uv == NULL)) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return;
	}
	for (i = 0; i < VOL_CUR_BUF_DEPTH; i++) {
		vol[i] = coul_hardware_get_battery_voltage_uv();
		cur[i] = coul_get_battery_current_ua();
	}

	if ((vol[0] == vol[1]) && (cur[0] == cur[1])) {
		*ibat_ua = cur[0];
		*vbat_uv = vol[0];
	} else if ((vol[1] == vol[2]) && (cur[1] == cur[2])) {
		*ibat_ua = cur[1];
		*vbat_uv = vol[1];
	} else {
		*ibat_ua = cur[2];
		*vbat_uv = vol[2];
	}

	*vbat_uv += (*ibat_ua / PERMILLAGE) * (di->r_pcb / PERMILLAGE);
}

static int pmu_coul_get_battery_voltage_uv(void *data)
{
	struct pmu_coul_device_info *di = (struct pmu_coul_device_info *)data;
	int ibat_ua = 0;
	int vbat_uv = 0;

	if (di == NULL) {
		coul_hardware_inf("NULL point in %s\n", __func__);
		return -1;
	}

	pmu_coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);

	return vbat_uv;
}

static int pmu_coul_core_get_battery_current_ma(void *data)
{
	int cur;
	struct pmu_coul_device_info *di = data;

	if (di == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return 0;
	}

	cur = coul_get_battery_current_ua();
	return -cur / PERMILLAGE;
}

static int pmu_coul_core_get_battery_voltage_mv(void *data)
{
	int ibat_ua = 0;
	int vbat_uv = 0;
	struct pmu_coul_device_info *di = data;

	if (di == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return -1;
	}

	pmu_coul_core_get_battery_voltage_and_current(di, &ibat_ua, &vbat_uv);
	return vbat_uv / PERMILLAGE;
}

static int pmu_coul_core_is_battery_exist(void *data)
{
	int temp;
	struct pmu_coul_device_info *di = data;

	if (di == NULL) {
		coul_hardware_err("error, di is NULL, return default exist\n");
		return 1;
	}

#ifdef CONFIG_HLTHERM_RUNTEST
	di->batt_exist = 0;
	return 0;
#endif

	temp = pmu_coul_core_get_battery_temperature(di);
	if ((temp <= ABNORMAL_BATT_TEMPERATURE_LOW) ||
		(temp >= ABNORMAL_BATT_TEMPERATURE_HIGH))
		di->batt_exist = 0;
	else
		di->batt_exist = 1;

	return di->batt_exist;
}

static struct coul_merge_drv_ops g_coulometer_ops = {
	.is_coul_ready = pmu_coul_is_ready,
	.dev_check = pmu_coul_device_check,
	.is_battery_exist = pmu_coul_core_is_battery_exist,
	.battery_voltage = pmu_coul_core_get_battery_voltage_mv,
	.battery_voltage_uv = pmu_coul_get_battery_voltage_uv,
	.battery_current = pmu_coul_core_get_battery_current_ma,
	.battery_temperature = pmu_coul_core_get_battery_temperature,
	.coul_set_cali = pmu_coul_set_cali_info,
	.coul_get_cali = pmu_coul_get_cali_info,
};

static int pmu_is_nv_cali_param_valid(int offset_a, int offset_b)
{
	if (offset_a < OFFSET_A_MIN || offset_a > OFFSET_A_MAX)
		return 0;

	if (offset_b < OFFSET_B_MIN || offset_b > OFFSET_B_MAX)
		return 0;

	return 1;
}

static void pmu_coul_cali_param_init(struct pmu_coul_device_info *di)
{
	struct ss_coul_nv_info *pinfo = NULL;
	struct pmu_coul_cali_params *params = &di->cali_params;

	pinfo = &di->nv_info;

	if (pmu_is_nv_cali_param_valid(pinfo->v_offset_a, pinfo->v_offset_b)) {
		params->v_offset_a = pinfo->v_offset_a;
		params->v_offset_b = pinfo->v_offset_b;
	} else {
		params->v_offset_a = DEFAULT_V_OFF_A;
		params->v_offset_b = DEFAULT_V_OFF_B;
	}

	if (pmu_is_nv_cali_param_valid(pinfo->c_offset_a, pinfo->c_offset_b)) {
		params->c_offset_a = pinfo->c_offset_a;
		params->c_offset_b = pinfo->c_offset_b;
	} else {
		params->c_offset_a = DEFAULT_V_OFF_A;
		params->c_offset_b = DEFAULT_V_OFF_B;
	}

	coul_hardware_inf("[coul cali init]batt %d, v_offset_a %d, v_offset_b %d, c_offset_a %d, c_offset_b %d\n",
			di->batt_index, params->v_offset_a, params->v_offset_b,
			params->c_offset_a, params->c_offset_b);
	coul_set_cali_param(&di->cali_params);
}

static void pmu_coul_get_initial_params(struct work_struct *work)
{
	int ret;
	struct opt_nve_info_user nve;
	struct pmu_coul_device_info *di =
		container_of(work, struct pmu_coul_device_info,
			cali_params_get_work.work);

	if (di == NULL)
		coul_hardware_inf("NULL point in [%s]\n", __func__);

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if (di->batt_index == BATT_0) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME, sizeof(COUL_NV_NAME));
		nve.nv_number = COUL_NV_NUM;
	} else if (di->batt_index == BATT_1) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME2, sizeof(COUL_NV_NAME2));
		nve.nv_number = COUL_NV_NUM2;
	} else if (di->batt_index == BATT_2) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME3, sizeof(COUL_NV_NAME3));
		nve.nv_number = COUL_NV_NUM3;
	}

	nve.valid_size = (uint32_t)sizeof(di->nv_info);
	nve.nv_operation = NV_READ;
	ret = nve_direct_access_interface(&nve);
	if (ret) {
		coul_hardware_err("[%s]read nv failed, ret = %d\n",
				__func__, ret);
		queue_delayed_work(system_power_efficient_wq,
		&di->cali_params_get_work,
		round_jiffies_relative(msecs_to_jiffies(READ_NV_MS)));
	} else {
		memcpy_s(&di->nv_info, sizeof(di->nv_info), nve.nv_data, NVE_NV_DATA_SIZE);
	}

	pmu_coul_cali_param_init(di);
}

static void get_batt_index(struct pmu_coul_device_info *di,
	const struct device_node *np)
{
	const char *batt_name_str = NULL;
	int ret;

	ret = of_property_read_u32(np, "batt_index", (unsigned int *)&di->batt_index);
	if (ret)
		coul_hardware_err("%s batt_index error %d\n", __func__, ret);

	ret = of_property_read_string(np, "batt_name", &batt_name_str);
	if (ret)
		(void)strcpy_s(di->batt_name, BATT_NAME_SIZE_MAX, "unknow");
	else
		(void)strcpy_s(di->batt_name, BATT_NAME_SIZE_MAX, batt_name_str);

	coul_hardware_inf("%s batt_index %d, batt_name %s\n", __func__, di->batt_index, di->batt_name);
}

static void get_r_pcb_dts(struct pmu_coul_device_info *di,
	const struct device_node *np)
{
	int ret;
	unsigned int r_pcb = DEFAULT_RPCB;

	ret = of_property_read_u32(np, "r_pcb", &r_pcb);
	if (ret)
		coul_hardware_err("error:get r_pcb value failed\n");
	di->r_pcb = (int)r_pcb;
	coul_hardware_inf("dts:get r_pcb = %d uohm\n", di->r_pcb);
}

static void get_r_coul_uohm_dts(struct pmu_coul_device_info *di,
	const struct device_node *np)
{
	int ret;

	ret = of_property_read_u32(np, "r_coul_uohm", (u32 *)&g_r_coul_uohm);
	if (ret) {
		g_r_coul_uohm = R_COUL_UOHM;
		coul_hardware_err(
			"get r_coul_uohm fail, use default value 10000 uohm!\n");
	};
}

static void pmu_coul_work_queue(struct pmu_coul_device_info *di)
{
	INIT_DELAYED_WORK(&di->cali_params_get_work, pmu_coul_get_initial_params);
	INIT_DELAYED_WORK(&di->read_temperature_delayed_work, pmu_read_temperature_work);

	queue_delayed_work(system_power_efficient_wq,
		&di->cali_params_get_work,
		round_jiffies_relative(msecs_to_jiffies(READ_NV_MS)));
	queue_delayed_work(system_power_efficient_wq,
		&di->read_temperature_delayed_work,
		round_jiffies_relative(msecs_to_jiffies(READ_TEMPERATURE_MS)));
}

static int pmu_coul_hardware_probe(struct spmi_device *pdev)
{
	struct pmu_coul_device_info *di = NULL;
	struct device_node *np = NULL;
	int retval;

	di = (struct pmu_coul_device_info *)devm_kzalloc(
		     &pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		coul_hardware_err("%s failed to alloc di struct\n", __func__);
		return -1;
	}
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	if (np == NULL) {
		coul_hardware_err("%s np is null!\n", __func__);
		return -1;
	}

	spmi_set_devicedata(pdev, di);

	if (coul_check_version(di)) {
		retval = -EINVAL;
		goto failed_0;
	}

	coul_set_default_cali_param(di);

	/* config coul ctrl */
	coul_chip_init();

	g_coul_hardware_dev = di;
	coul_hardware_inf("coul hardware probe ok, chip %x, version is v%x !\n",
		di->chip_proj, di->chip_version);

	get_batt_index(di, np);
	get_r_pcb_dts(di, np);
	get_r_coul_uohm_dts(di, np);
	di->is_nv_read = NV_READ_SUCCESS;
	di->batt_temp = pmu_coul_battery_get_temperature_tenth_degree(di);
	di->ops = &g_coulometer_ops;
	di->batt_exist = 1;

	pmu_coul_work_queue(di);

	retval = coul_merge_drv_coul_ops_register(di->ops, di, di->batt_index, di->batt_name);
	if (retval) {
		coul_hardware_err("failed to register coul ops\n");
		goto failed_1;
	}
	return retval;

failed_1:
	di->ops = NULL;
	cancel_delayed_work(&di->cali_params_get_work);
	cancel_delayed_work(&di->read_temperature_delayed_work);
failed_0:
	spmi_set_devicedata(pdev, NULL);
	coul_hardware_err("coul hardware probe failed!\n");
	return retval;
}

static void pmu_coul_hardware_remove(struct spmi_device *pdev)
{
	struct pmu_coul_device_info *di = spmi_get_devicedata(pdev);

	if (di == NULL) {
		coul_hardware_err("[%s]di is null\n", __func__);
		return;
	}
	devm_kfree(&pdev->dev, di);
}

static const struct of_device_id coul_match_table[] = {
	{
		.compatible = "hisilicon,hi6xxx_coul_adc",
	},
	{
	},
};

static const struct spmi_device_id coul_spmi_id[] = {
	{
		"hi6xxx_coul_adc",
		0
	},
	{
	}
};

static struct spmi_driver coul_driver = {
	.probe = pmu_coul_hardware_probe,
	.remove = pmu_coul_hardware_remove,
#ifdef CONFIG_PM
	.suspend = pmu_coul_hardware_suspend,
	.resume = pmu_coul_hardware_resume,
#endif
	.driver = {
		.name = "hi6xxx_coul_adc",
		.owner = THIS_MODULE,
		.of_match_table = coul_match_table,
	},
	.id_table = coul_spmi_id,
};

int __init pmu_coul_hardware_init(void)
{
	return spmi_driver_register(&coul_driver);
}

void __exit pmu_coul_hardware_exit(void)
{
	spmi_driver_unregister(&coul_driver);
}

fs_initcall(pmu_coul_hardware_init);
module_exit(pmu_coul_hardware_exit);

MODULE_DESCRIPTION("pmu coul hardware driver");
MODULE_LICENSE("GPL");
