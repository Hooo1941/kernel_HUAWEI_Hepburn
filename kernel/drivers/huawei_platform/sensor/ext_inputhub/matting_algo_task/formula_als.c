// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023.
 * All rights reserved.
 * Description: ALS formula source file
 * Author: yangxiaopeng@huawei.com
 * Create: 2023-3-7
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <stdint.h>
#include "securec.h"
#include "formula_als.h"
#include "matting_algo_task.h"
#include <huawei_platform/log/hw_log.h>
#include "math_calculation.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

#define NEGATIVE_VALUE (-1)
#define POSITIVE_VALUE (1)

static void lcd_tls2585_do_div(uint64_t *n, uint64_t base)
{
	if (base != 0)
		*n = math_div64_32(*n, base, NULL);

	return;
}

static int32_t lcd_get_tsl2585_lux_value(struct sensor_hub_als_data_st *als_data)
{
	uint64_t lux;
	uint64_t ir_pho_var;
	uint64_t ir_pho_ratio;
	uint64_t cpl;
	uint64_t ir_data;
	int64_t lux_temp;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();
	struct coefficient_st coef = data_param_info->als_coef_u.tsl2585_coef_s.coef_c;

	ir_data = als_data->als_channel[1];
	if (als_data->als_again[0] != als_data->als_again[1]) {
		if (als_data->als_again[1] == 0) {
			hwlog_err("[lcd_get_tsl2**5_lux_value] als_again[1] illeague\n");
			return EC_FAILURE;
		}
		ir_data = math_div64_32(ir_data * ((uint64_t)als_data->als_again[0]), als_data->als_again[1], NULL);
	}

	if (als_data->als_channel[0] == 0) {
		hwlog_debug("[lcd_get_tsl2**5_lux_value] als_channel[0] is 0\n");
		als_data->als_lux = 0;
		return EC_SUCCESS;
	}

	ir_pho_ratio = ir_data * MILLI;
	ir_pho_var = als_data->als_channel[0];
	lcd_tls2585_do_div(&ir_pho_ratio, ir_pho_var);
	if (ir_pho_ratio < data_param_info->als_coef_u.tsl2585_coef_s.ir_pho_ratio1) {
		coef = data_param_info->als_coef_u.tsl2585_coef_s.coef_a;
	} else if (ir_pho_ratio > data_param_info->als_coef_u.tsl2585_coef_s.ir_pho_ratio2) {
		ir_data = math_div64_32(ir_data * data_param_info->als_coef_u.tsl2585_coef_s.ir_data_percent,
			data_param_info->als_coef_u.tsl2585_coef_s.multiples, NULL);
	}

	cpl = ((uint64_t)als_data->als_atime * als_data->als_again[0]) * MILLI;
	lux_temp = (als_data->als_channel[0] * coef.photopic) + (ir_data * coef.ir);
	lux_temp = lux_temp * coef.dgf;
	if (lux_temp > 0) {
		if (cpl == 0) {
			hwlog_err("[lcd_get_tsl2**5_lux_value] ALS_08 ComputeLux gain*time is 0\n");
			return EC_FAILURE;
		}
		lux = lux_temp;
		lcd_tls2585_do_div(&lux, cpl);
		lcd_tls2585_do_div(&lux, MILLI);
	} else {
		lux = 0;
	}
	als_data->als_lux = lux;

	return EC_SUCCESS;
}

static int32_t lcd_get_sip1225_lux_value(struct sensor_hub_als_data_st *als_data)
{
	uint32_t ratio;
	int32_t lux;
	uint32_t multiples;
	int32_t sign_flag;
	uint32_t ch0_data = als_data->als_channel[0];
	uint32_t ch1_data = als_data->als_channel[1];
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	if ((als_data->als_again[0] == 0) || (als_data->als_atime == 0)) {
		hwlog_err("[lcd_get_sip1**5_lux_value] again*time is 0\n");
		return EC_FAILURE;
	}

	if (ch1_data == 0) {
		hwlog_debug("[lcd_get_sip1**5_lux_value] ch1_data is 0\n");
		als_data->als_lux = 0;
		return EC_SUCCESS;
	}
	ratio = math_div64_32(ch0_data * ((uint64_t)data_param_info->als_coef_u.sip1225_coef_s.multiples),
		ch1_data, NULL);
	if (ratio < data_param_info->als_coef_u.sip1225_coef_s.ratio) {
		multiples = data_param_info->als_coef_u.sip1225_coef_s.coef_l_s.multiples;
		sign_flag = data_param_info->als_coef_u.sip1225_coef_s.coef_l_s.coef1 >= 0 ? POSITIVE_VALUE : NEGATIVE_VALUE;
		lux = math_div64_32((abs(data_param_info->als_coef_u.sip1225_coef_s.coef_l_s.coef1) * (uint64_t)ch0_data),
			multiples * als_data->als_again[0] * als_data->als_atime, NULL) * sign_flag;
		sign_flag = data_param_info->als_coef_u.sip1225_coef_s.coef_l_s.coef2 >= 0 ? POSITIVE_VALUE : NEGATIVE_VALUE;
		lux +=  math_div64_32((abs(data_param_info->als_coef_u.sip1225_coef_s.coef_l_s.coef2) * (uint64_t)ch1_data),
			multiples * als_data->als_again[0] * als_data->als_atime, NULL) * sign_flag;
	} else {
		multiples = data_param_info->als_coef_u.sip1225_coef_s.coef_h_s.multiples;
		sign_flag = data_param_info->als_coef_u.sip1225_coef_s.coef_h_s.coef1 >= 0 ? POSITIVE_VALUE : NEGATIVE_VALUE;
		lux = math_div64_32((abs(data_param_info->als_coef_u.sip1225_coef_s.coef_h_s.coef1) * (uint64_t)ch0_data),
			multiples * als_data->als_again[0] * als_data->als_atime, NULL) * sign_flag;
		sign_flag = data_param_info->als_coef_u.sip1225_coef_s.coef_h_s.coef2 >= 0 ? POSITIVE_VALUE : NEGATIVE_VALUE;
		lux +=  math_div64_32((abs(data_param_info->als_coef_u.sip1225_coef_s.coef_h_s.coef2) * (uint64_t)ch1_data),
			multiples * als_data->als_again[0] * als_data->als_atime, NULL) * sign_flag;
	}
	if (lux < 0)
		lux = 0;
	als_data->als_lux = lux;

	return EC_SUCCESS;
}

int32_t lcd_get_alx_lux_value(enum matting_algo_als_type_en als_type, struct sensor_hub_als_data_st *als_data)
{
	int32_t ret;
	struct alg_matting_data_param_info_st *data_param_info = matting_algo_get_data_param_info();

	if (als_data == NULL) {
		hwlog_err("[%s] als formula lux als_data is null, als_type=%d\n", __func__, als_type);
		return EC_FAILURE;
	}

	switch (als_type) {
	case ALS_TSL2585:
		ret = lcd_get_tsl2585_lux_value(als_data);
		break;
	case ALS_SIP1225:
		ret = lcd_get_sip1225_lux_value(als_data);
		break;
	default:
		ret = EC_FAILURE;
		hwlog_err("[%s] not support als_type=%d\n", __func__, als_type);
		break;
	}

	if (ret != EC_FAILURE)
		als_data->als_lux = als_data->als_lux * data_param_info->als_offset_s.als_offset / SENSORMGR_ALS_CALIB_COEF;

	return ret;
}