/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: nv operation for coul module
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
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include "coul_nv.h"

#define COUL_NV_NAME "HICOUL"
#define COUL_NV_NUM  316

#define COUL_NV_NAME2 "HICOUL2"
#define COUL_NV_NUM2  503

#define HW_COUL_NV_NAME   "HWCOUL"
#define HW_COUL_NV_NUM    392
#define NV_OPERATE_SUCC   0
#define NV_NOT_DEFINED    (-22)
#define PMU_NV_ADDR_CMDLINE_MAX_LEN 30
#define HEX               16
#define DECIMAL           10
#define INVALID_NV_FCC_VAL 100
#define BAT_CALIBRATION_NV_NAME    "BATCALI"
#define BAT_CALIBRATION_NV_NUM     317

/* hicoul 128 byte, acr/dcr/pdleak 3*128 byte */
#define RESERVED_MEM_FOR_PMU_COUL_ADDR_512 512
/* hicoul2 128 byte */
#define RESERVED_MEM_FOR_COUL_BATT_INFO_SIZE 128
/* battery low boot info 64 byte; battery high 64 byte */
#define RESERVED_MEM_FOR_COUL_BOOT_INFO_SIZE 64

enum nv_operation_type {
	NV_WRITE_TYPE = 0,
	NV_READ_TYPE,
};

struct coul_cali_nv_info {
	int v_offset_a;
	int v_offset_b;
	int c_offset_a;
	int c_offset_b;
	int c_chip_temp;
};

static unsigned long g_nv_info_addr;
/*
 * 0x34A10000~0x34A11000 is reserved for pmu coulomb,
 * we use these to transfer coul information from fastboot to kernel, we add charger info
 */
static struct ss_coul_nv_info g_batt_0_nv_info;
static struct ss_coul_nv_info g_batt_1_nv_info;
static struct batt_boot_info g_boot_info_batt_0;
static struct batt_boot_info g_boot_info_batt_1;

unsigned long coul_get_nv_info_addr(void)
{
	return g_nv_info_addr;
}

static int __init early_parse_pmu_nv_addr_cmdline(char *p)
{
	char buf[PMU_NV_ADDR_CMDLINE_MAX_LEN + 1] = {0};
	size_t len;
	int ret;

	if (p == NULL) {
		coul_core_err("NULL point in [%s]\n", __func__);
		return -1;
	}
	len = strlen(p);
	if (len > PMU_NV_ADDR_CMDLINE_MAX_LEN) {
		coul_core_err("pmu_nv_addr_cmdline length out of range\n");
		return -1;
	}

	ret = memcpy_s(buf, PMU_NV_ADDR_CMDLINE_MAX_LEN + 1, p, len + 1);
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return -1;
	}

	if (kstrtoul(buf, HEX, &g_nv_info_addr) < 0)
		coul_core_err("%s kstrtoul failed\n", __func__);
	return 0;
}

static int __init move_pmu_nv_info(void)
{
	unsigned long *vir_addr = NULL;
	unsigned long phy_addr;
	int ret;

	if (g_nv_info_addr == 0) {
		coul_core_err("%s g_nv_info_addr is error\n", __func__);
		return 0;
	}

	phy_addr = g_nv_info_addr;
	vir_addr = ioremap_wc(phy_addr, sizeof(g_batt_0_nv_info));
	if (vir_addr == NULL) {
		coul_core_err("nv add err\n");
		return 0;
	}
	ret = memcpy_s(&g_batt_0_nv_info, sizeof(g_batt_0_nv_info), vir_addr, sizeof(g_batt_0_nv_info));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return 0;
	}

	phy_addr += RESERVED_MEM_FOR_PMU_COUL_ADDR_512;
	vir_addr = ioremap_wc(phy_addr, sizeof(g_batt_1_nv_info));
	if (vir_addr == NULL) {
		coul_core_err("nv add err\n");
		return 0;
	}
	ret = memcpy_s(&g_batt_1_nv_info,  sizeof(g_batt_1_nv_info), vir_addr, sizeof(g_batt_1_nv_info));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return 0;
	}

	phy_addr += RESERVED_MEM_FOR_COUL_BATT_INFO_SIZE;
	vir_addr = ioremap_wc(phy_addr, sizeof(g_boot_info_batt_0));
	if (vir_addr == NULL) {
		coul_core_err("nv add err\n");
		return 0;
	}
	ret = memcpy_s(&g_boot_info_batt_0, sizeof(g_boot_info_batt_0), vir_addr, sizeof(g_boot_info_batt_0));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return 0;
	}

	phy_addr += RESERVED_MEM_FOR_COUL_BOOT_INFO_SIZE;
	vir_addr = ioremap_wc(phy_addr, sizeof(g_boot_info_batt_1));
	if (vir_addr == NULL) {
		coul_core_err("nv add err\n");
		return 0;
	}
	ret = memcpy_s(&g_boot_info_batt_1, sizeof(g_boot_info_batt_1), vir_addr, sizeof(g_boot_info_batt_1));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return 0;
	}

	return 0;
}

early_param("pmu_nv_addr", early_parse_pmu_nv_addr_cmdline);
arch_initcall(move_pmu_nv_info);

static int get_fcc_temp_info(struct smartstar_coul_device *di,
	const struct ss_coul_nv_info *pinfo, struct single_row_lut *preal_fcc_lut)
{
	int i;

	for (i = 0; i < MAX_TEMPS; i++) {
		if (pinfo->real_fcc[i] == 0)
			break;

		if (pinfo->real_fcc[i] < INVALID_NV_FCC_VAL) {
			coul_core_info("real fcc in nv is not currect\n");
			return 0;
		}

		preal_fcc_lut->x[i] = pinfo->temp[i];
		preal_fcc_lut->y[i] = pinfo->real_fcc[i];
	}

	if (i == 0) {
		coul_core_info("no real fcc data in nv\n");
		return 0;
	}

	preal_fcc_lut->cols = i;
	di->adjusted_fcc_temp_lut = preal_fcc_lut;
	coul_core_info("temp:real_fcc %d:%d %d:%d %d:%d %d:%d %d:%d %d:%d %d:%d\n",
		pinfo->temp[0], pinfo->real_fcc[0],
		pinfo->temp[1], pinfo->real_fcc[1],
		pinfo->temp[2], pinfo->real_fcc[2],
		pinfo->temp[3], pinfo->real_fcc[3],
		pinfo->temp[4], pinfo->real_fcc[4],
		pinfo->temp[5], pinfo->real_fcc[5],
		pinfo->temp[6], pinfo->real_fcc[6]);
	return 0;
}

static int is_nv_cali_param_valid(int offset_a, int offset_b)
{
	if (offset_a < OFFSET_A_MIN || offset_a > OFFSET_A_MAX)
		return 0;

	if (offset_b < OFFSET_B_MIN || offset_b > OFFSET_B_MAX)
		return 0;

	return 1;
}

static void coul_cali_param_init(struct smartstar_coul_device *di)
{
	struct ss_coul_nv_info *pinfo = NULL;
	struct coul_cali_params *params = &di->g_coul_cali_params;

	pinfo = &di->nv_info;

	if (is_nv_cali_param_valid(pinfo->v_offset_a, pinfo->v_offset_b)) {
		params->v_offset_a = pinfo->v_offset_a;
		params->v_offset_b = pinfo->v_offset_b;
	} else {
		params->v_offset_a = DEFAULT_V_OFF_A;
		params->v_offset_b = DEFAULT_V_OFF_B;
	}

	if (is_nv_cali_param_valid(pinfo->c_offset_a, pinfo->c_offset_b)) {
		params->c_offset_a = pinfo->c_offset_a;
		params->c_offset_b = pinfo->c_offset_b;
	} else {
		params->c_offset_a = DEFAULT_V_OFF_A;
		params->c_offset_b = DEFAULT_V_OFF_B;
	}

	if (is_nv_cali_param_valid(pinfo->v_offset_series_a, pinfo->v_offset_series_b)) {
		params->v_offset_series_a = pinfo->v_offset_series_a;
		params->v_offset_series_b = pinfo->v_offset_series_b;
	} else {
		params->v_offset_series_a = DEFAULT_V_OFF_A;
		params->v_offset_series_b = DEFAULT_V_OFF_B;
	}

	if (is_nv_cali_param_valid(pinfo->c_offset_series_a, pinfo->c_offset_series_b)) {
		params->c_offset_series_a = pinfo->c_offset_series_a;
		params->c_offset_series_b = pinfo->c_offset_series_b;
	} else {
		params->c_offset_series_a = DEFAULT_V_OFF_A;
		params->c_offset_series_b = DEFAULT_V_OFF_B;
	}

	coul_core_info("[coul cali init]batt %d, v_offset_a %d, v_offset_b %d, "
		"c_offset_a %d, c_offset_b %d, v_offset_series_a %d, v_offset_series_b %d "
		"c_offset_series_a %d, c_offset_series_b %d\n",
			di->batt_index, params->v_offset_a, params->v_offset_b,
			params->c_offset_a, params->c_offset_b, params->v_offset_series_a,
			params->v_offset_series_b, params->c_offset_series_a,
			params->c_offset_series_b);
	di->coul_dev_ops->set_dev_cali_param(di->batt_index, &di->g_coul_cali_params);
}

int coul_cali_info_set(struct smartstar_coul_device *di, struct coul_batt_cali_info *info)
{
	if (di == NULL ||info == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	if(info->charge_mode  == PARALLEL_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			di->nv_info.v_offset_a =
				di->g_coul_cali_params.v_offset_a = info->a;
			di->nv_info.v_offset_b =
				di->g_coul_cali_params.v_offset_b = info->b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			di->nv_info.c_offset_a =
				di->g_coul_cali_params.c_offset_a = info->a;
			di->nv_info.c_offset_b =
				di->g_coul_cali_params.c_offset_b = info->b;
		}
	} else if (info->charge_mode  == SERIES_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			di->nv_info.v_offset_series_a =
				di->g_coul_cali_params.v_offset_series_a = info->a;
			di->nv_info.v_offset_series_b =
				di->g_coul_cali_params.v_offset_series_b = info->b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			di->nv_info.c_offset_series_a =
				di->g_coul_cali_params.c_offset_series_a = info->a;
			di->nv_info.c_offset_series_b =
				di->g_coul_cali_params.c_offset_series_b = info->b;
		}
	}

	di->coul_dev_ops->set_dev_cali_param(di->batt_index, &di->g_coul_cali_params);
	return coul_save_nv_info(di);
}

int coul_cali_info_get(struct smartstar_coul_device *di, struct coul_batt_cali_info *info)
{
	if (di == NULL ||info == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	if(info->charge_mode  == PARALLEL_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			info->a = di->g_coul_cali_params.v_offset_a;
			info->b = di->g_coul_cali_params.v_offset_b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			info->a = di->g_coul_cali_params.c_offset_a;
			info->b = di->g_coul_cali_params.c_offset_b;
		}
	} else if (info->charge_mode  == SERIES_MODE) {
		if (info->data_type == BATT_DATA_VOLT_CALI) {
			info->a = di->g_coul_cali_params.v_offset_series_a;
			info->b = di->g_coul_cali_params.v_offset_series_b;
		} else if (info->data_type == BATT_DATA_CUR_CALI) {
			info->a = di->g_coul_cali_params.c_offset_series_a;
			info->b = di->g_coul_cali_params.c_offset_series_b;
		}
	}
	return 0;
}

/* get NV info from fastboot send */
int coul_get_initial_params(struct smartstar_coul_device *di)
{
	struct ss_coul_nv_info *pinfo = NULL;
	struct single_row_lut *preal_fcc_lut = NULL;

	if (di == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	pinfo = &di->nv_info;
	preal_fcc_lut = &di->adjusted_fcc_temp_lut_tbl1;

	if(di->batt_index == BATT_0) {
		(void)memcpy_s(&di->nv_info, sizeof(struct ss_coul_nv_info),
			&g_batt_0_nv_info, sizeof(g_batt_0_nv_info));
		(void)memcpy_s(&di->boot_info, sizeof(struct batt_boot_info),
			&g_boot_info_batt_0, sizeof(g_boot_info_batt_0));
	} else if(di->batt_index == BATT_1) {
		(void)memcpy_s(&di->nv_info, sizeof(struct ss_coul_nv_info),
			&g_batt_1_nv_info, sizeof(g_batt_1_nv_info));
		(void)memcpy_s(&di->boot_info, sizeof(struct batt_boot_info),
			&g_boot_info_batt_1, sizeof(g_boot_info_batt_1));
	} else {
		coul_core_err("[%s]batt_index is %d, not support\n", __func__, di->batt_index);
		return -1;
	}

	di->batt_chargecycles = (unsigned int)pinfo->charge_cycles;
	di->high_pre_qmax = pinfo->qmax;
	di->batt_limit_fcc = pinfo->limit_fcc;
	di->batt_report_full_fcc_real = pinfo->report_full_fcc_real;
	coul_core_info("%s,batt_index=%d,cycles=%d,v_a= %d,v_b=%d,c_a=%d,c_b=%d,"
		"limit_fcc=%d,report_full_fcc_real =%d,"
		"reg_c=%d, reg_v=%d,batt_id=%d\n",
		__func__, di->batt_index, pinfo->charge_cycles,
		pinfo->v_offset_a, pinfo->v_offset_b,
		pinfo->c_offset_a, pinfo->c_offset_b,
		pinfo->limit_fcc, pinfo->report_full_fcc_real,
		di->boot_info.calc_ocv_reg_c, di->boot_info.calc_ocv_reg_v,
		di->boot_info.hkadc_batt_id_voltage);

	coul_cali_param_init(di);

	return get_fcc_temp_info(di, pinfo, preal_fcc_lut);
}


static void save_backup_nv_info(struct smartstar_coul_device *di,
	struct ss_coul_nv_info *pinfo)
{
	int i;
	int refresh_fcc_success = 1;

	if ((di == NULL) || (pinfo == NULL))
		return;
	pinfo->qmax = (short)di->high_pre_qmax;
	pinfo->charge_cycles = (int)di->batt_chargecycles;
	pinfo->limit_fcc = di->batt_limit_fcc;
	pinfo->report_full_fcc_real = di->batt_report_full_fcc_real;

	if (di->adjusted_fcc_temp_lut != NULL) {
		for (i = 0; i < di->adjusted_fcc_temp_lut->cols; i++) {
			if (di->adjusted_fcc_temp_lut->y[i] < INVALID_NV_FCC_VAL) {
				refresh_fcc_success = 0;
				break;
			}
		}
		if (refresh_fcc_success) {
			for (i = 0; i < di->adjusted_fcc_temp_lut->cols; i++) {
				pinfo->temp[i] = di->adjusted_fcc_temp_lut->x[i];
				pinfo->real_fcc[i] =
					di->adjusted_fcc_temp_lut->y[i];
			}
		}
	} else {
		for (i = 0; i < MAX_TEMPS; i++) {
			pinfo->temp[i] = 0;
			pinfo->real_fcc[i] = 0;
		}
	}
}
int coul_save_nv_info(struct smartstar_coul_device *di)
{
	int ret;
	struct opt_nve_info_user nve;
	struct ss_coul_nv_info *pinfo = NULL;

	if (di == NULL) {
		coul_core_info("NULL point in [%s]\n", __func__);
		return -1;
	}

	__pm_stay_awake(di->power_wake_lock);
	pinfo = &di->nv_info;
	if (!di->is_nv_read) {
		/* udp do not print err log */
		if (di->is_board_type == BAT_BOARD_ASIC)
			coul_core_err("save nv before read, error\n");
		di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
		__pm_relax(di->power_wake_lock);
		return -1;
	}

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if(di->batt_index == BATT_0) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME, sizeof(COUL_NV_NAME));
		nve.nv_number = COUL_NV_NUM;
	} else {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME2, sizeof(COUL_NV_NAME2));
		nve.nv_number = COUL_NV_NUM2;
	}

	nve.valid_size = (uint32_t)sizeof(*pinfo);
	nve.nv_operation = NV_WRITE;

	save_backup_nv_info(di, pinfo);
	ret = memcpy_s(nve.nv_data, NVE_NV_DATA_SIZE, pinfo, sizeof(*pinfo));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		__pm_relax(di->power_wake_lock);
		return -1;
	}

	/* here save info in register */
	ret = nve_direct_access_interface(&nve);
	if (ret) {
		coul_core_info("save nv partion failed, ret=%d\n", ret);
		di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_FAIL);
	} else {
		di->coul_dev_ops->set_nv_save_flag(di->batt_index, NV_SAVE_SUCCESS);
	}

	coul_core_info("%s,cycles=%d,v_a= %d,v_b=%d,c_a=%d,c_b=%d,"
		"limit_fcc=%d,report_full_fcc_real =%d\n",
		__func__, pinfo->charge_cycles, pinfo->v_offset_a,
		pinfo->v_offset_b, pinfo->c_offset_a, pinfo->c_offset_b,
		pinfo->limit_fcc, pinfo->report_full_fcc_real);
	__pm_relax(di->power_wake_lock);
	return ret;
}

#ifdef CONFIG_DFX_DEBUG_FS
#define BATTERY_COUNT 2
static int g_basp_nv;
static int save_nv_info(int batt_index)
{
	int ret;
	struct opt_nve_info_user nve;
	struct ss_coul_nv_info g_batt_nv_info[BATTERY_COUNT] = {g_batt_0_nv_info , g_batt_1_nv_info};
	struct ss_coul_nv_info pinfo = g_batt_nv_info[batt_index];

	(void)memset_s(&nve, sizeof(nve), 0, sizeof(nve));
	if(batt_index == BATT_0) {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME, sizeof(COUL_NV_NAME));
		nve.nv_number = COUL_NV_NUM;
	} else {
		(void)strncpy_s(nve.nv_name,  NV_NAME_LENGTH, COUL_NV_NAME2, sizeof(COUL_NV_NAME2));
		nve.nv_number = COUL_NV_NUM2;
	}

	nve.valid_size = (uint32_t)sizeof(pinfo);
	nve.nv_operation = NV_WRITE;

	ret = memcpy_s(nve.nv_data, NVE_NV_DATA_SIZE, &pinfo, sizeof(pinfo));
	if (ret != EOK) {
		coul_core_err("memcpy_s failed, ret=%d\n", ret);
		return -1;
	}

	/* here save info in register */
	ret = nve_direct_access_interface(&nve);
	if (ret) {
		coul_core_info("save nv partion failed, ret=%d\n", ret);
	} else {
		coul_core_info("save nv partion success !\n");
	}

	return ret;
}


static int basp_nv_info_print(char *buffer, const struct ss_coul_nv_info nv_info)
{
	int i, ret;
	unsigned int len = strlen(buffer);

	for (i = 0; i < MAX_RECORDS_CNT; i++) {
		ret = snprintf_s(buffer + len, PAGE_SIZE - len,
			PAGE_SIZE - len - 1, "learned_fcc[%d]:%d\n",
			i, nv_info.real_fcc_record[i]);
		if (ret > 0)
			len = strlen(buffer);
	}

	ret = snprintf_s(buffer + len, PAGE_SIZE - len,
		PAGE_SIZE - len - 1, "chargecycles:%d\n",
		nv_info.charge_cycles);
	if (ret > 0)
		len = strlen(buffer);
	ret = snprintf_s(buffer + len, PAGE_SIZE - len,
		PAGE_SIZE - len - 1, "latest_record_index:%d\n",
		nv_info.latest_record_index);
	if (ret > 0)
		len = strlen(buffer);
	return len;
}

static bool basp_check_input_valid(const char *buffer)
{
	const char *c = NULL;

	coul_core_info(BASP_TAG"buffer:%s\n", buffer);

	c = buffer;
	while ((*c != '\n') && (*c != '\0')) {
		if (!(((*c >= '0') && (*c <= '9')) || (*c == ' '))) {
			coul_core_err(BASP_TAG"[%s], input invalid\n", __func__);
			return FALSE;
		}
		c++;
	}
	return TRUE;
}

static bool basp_nv_buff_ops(const char *buffer, int *need_save , int *batt_index)
{
#define MAX_TMP_BUF_LEN 10
	int ret, i;
	int indx = 0;
	long val = 0;
	const char *begin = NULL;
	const char *end = NULL;
	struct ss_coul_nv_info temp_batt_nv_info;
	struct ss_coul_nv_info* g_batt_nv_info[BATTERY_COUNT] = {&g_batt_0_nv_info , &g_batt_1_nv_info};

	char num[MAX_TMP_BUF_LEN] = {0};

	begin = buffer;
	while ((*begin != '\0') && (*begin != '\n')) {
		while (*begin == ' ')
			begin++;
		end = begin;
		while ((*end != ' ') && (*end != '\0') && (*end != '\n'))
			end++;
		if (end - begin >= MAX_TMP_BUF_LEN) {
			coul_core_err(BASP_TAG"[%s], input too big\n", __func__);
			return FALSE;
		}
		ret = memcpy_s(num, (end - begin), begin, (end - begin));
		if (ret != EOK) {
			coul_core_err("memcpy_s failed, ret=%d\n", ret);
			return -1;
		}
		if (kstrtol(num, DECIMAL, &val) < 0) {
			coul_core_err(BASP_TAG"[%s], num:%s, convert fail\n",
				__func__, num);
			break;
		}

		*need_save = 1;
		/* 0 ~ MAX_RECORDS_CNT-1 are fcc value,  MAX_RECORDS_CNT is chargecycles,  MAX_RECORDS_CNT+1 is batt_index */
		switch (indx) {
		case MAX_RECORDS_CNT:
			temp_batt_nv_info.charge_cycles = (unsigned int)val;
			indx++;
			break;
		case MAX_RECORDS_CNT + 1:
			*batt_index = (unsigned int)val;
			break;
		case MAX_RECORDS_CNT + 2:
			indx = 0;
			break;
		default:
			temp_batt_nv_info.real_fcc_record[indx++] = val;
			break;
		}
		(void)memset_s(num, sizeof(num), 0, sizeof(num));
		begin = end;
	}

	if(*batt_index >= BATTERY_COUNT) {
		coul_core_err(BASP_TAG"[%s], battIndex[%d] input is error\n", __func__, *batt_index);
		return FALSE;
	} else {
		g_batt_nv_info[*batt_index]->charge_cycles = temp_batt_nv_info.charge_cycles;
		for(i =0 ;i < MAX_RECORDS_CNT; i++)
			g_batt_nv_info[*batt_index]->real_fcc_record[i] = temp_batt_nv_info.real_fcc_record[i];
	}

	return TRUE;
#undef MAX_TMP_BUF_LEN
}

static int basp_nv_set(const char *buffer, const struct kernel_param *kp)
{
	int ret = 0;
	int bat_index = 0;
	int need_save = 0;
	const char *begin = NULL;

	if (basp_check_input_valid(buffer) == FALSE)
		goto func_end;

	begin = buffer;
	if (*begin == '\0') {
		coul_core_err(BASP_TAG"[%s], input empty\n", __func__);
		return 0;
	}

	if (basp_nv_buff_ops(buffer, &need_save ,&bat_index) == FALSE)
		goto func_end;

	if (need_save) {
		ret = save_nv_info(bat_index);
		if (ret)
			coul_core_err("[%s]save_batIndex[%d]_nv_info fail, ret=%d\n", __func__, bat_index ,ret);
	}

func_end:
	return strlen(buffer);
}

static int basp_nv_get(char *buffer, const struct kernel_param *kp)
{
	int index,ret;
	struct ss_coul_nv_info g_batt_nv_info[BATTERY_COUNT] = {g_batt_0_nv_info , g_batt_1_nv_info};
	for(index = BATT_0; index < BATTERY_COUNT ; index++) {
		coul_core_err(BASP_TAG"[%s], print BattIndex[%d]_nv_info\n", __func__, index);
		ret = basp_nv_info_print(buffer, g_batt_nv_info[index]);
	}

	return ret;
}

static struct kernel_param_ops g_basp_nv_ops = {
	.set = basp_nv_set,
	.get = basp_nv_get,
};

module_param_cb(basp_nv, &g_basp_nv_ops, &g_basp_nv, 0644);
#endif
