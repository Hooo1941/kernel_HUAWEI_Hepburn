// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: math calculation
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-1
 */

#include "math_calculation.h"
#include <huawei_platform/log/hw_log.h>


#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG matting_algo_task
HWLOG_REGIST();

uint64_t math_div64_32(uint64_t divisor, uint32_t dividend, uint32_t *remainder)
{
	uint64_t value = divisor;
	uint32_t mod;

	if (dividend == 0)
		return value;

	mod = do_div(value, dividend);
	hwlog_debug("[%s] quotient=%lld, remainder=%u\n", __func__, value, mod);

	if (remainder != NULL)
		*remainder = mod;

	return value;
}
