/* SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023.
 * All rights reserved.
 * Description: head file of ALS formula
 * Author: yangxiaopeng@huawei.com
 * Create: 2023-3-7
 */

#ifndef FORMULA_ALS_H
#define FORMULA_ALS_H

#include "image_compositing_algo.h"

int32_t lcd_get_alx_lux_value(enum matting_algo_als_type_en als_type, struct sensor_hub_als_data_st *als_data);
#endif