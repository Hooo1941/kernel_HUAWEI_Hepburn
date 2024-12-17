/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sip3510 source file
 * Author: huangjiaqing <huangjiaqing4@huawei.com>
 * Create: 2023-06-12
 */

#include "als_para_table_sip3510.h"

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

static sip3510_als_para_table sip3510_als_para_diff_tp_color_table[] = {
	{ LEM, V3, DEFAULT_TPLCD, OTHER,
	  { 128, 500, 0, 0, -853, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 23776,
	    1000, 16176, 1000, 4000, 10}
	},
};

sip3510_als_para_table *als_get_sip3510_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sip3510_als_para_diff_tp_color_table))
		return NULL;
	return &(sip3510_als_para_diff_tp_color_table[id]);
}

sip3510_als_para_table *als_get_sip3510_first_table(void)
{
	return &(sip3510_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_sip3510_table_count(void)
{
	return ARRAY_SIZE(sip3510_als_para_diff_tp_color_table);
}
