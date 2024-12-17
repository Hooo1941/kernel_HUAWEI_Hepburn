/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022.
 * All rights reserved.
 * Description: math calculation
 * Author: yangxiaopeng@huawei.com
 * Create: 2022-11-1
 */
#ifndef MATH_CALCULATION_H
#define MATH_CALCULATION_H

#include "div64.h"

uint64_t math_div64_32(uint64_t divisor, uint32_t dividend, uint32_t *remainder);

#endif