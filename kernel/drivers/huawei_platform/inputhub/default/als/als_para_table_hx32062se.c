/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table hx32062se source file
 * Author: huangjiaqing <huangjiaqing4@huawei.com>
 * Create: 2023-06-12
 */

#include "als_para_table_hx32062se.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_tp_color.h"
#include "contexthub_boot.h"
#include "contexthub_route.h"

static hx32062se_als_para_table hx32062se_als_para_diff_tp_color_table[] = {
	// Historical Parameters
	{ LEM, V3, DEFAULT_TPLCD, OTHER,
		{ 1520, 500, 0, 0, -853, 100, 0, 5690, -4716, 2529, -5526, 100,
		  0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 8750,
		  90, 18500, 0, 4000, 10 }
	},
};

hx32062se_als_para_table *als_get_hx32062se_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(hx32062se_als_para_diff_tp_color_table))
		return NULL;
	return &(hx32062se_als_para_diff_tp_color_table[id]);
}

hx32062se_als_para_table *als_get_hx32062se_first_table(void)
{
	return &(hx32062se_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_hx32062se_table_count(void)
{
	return ARRAY_SIZE(hx32062se_als_para_diff_tp_color_table);
}
