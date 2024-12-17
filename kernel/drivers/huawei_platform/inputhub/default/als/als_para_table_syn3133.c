/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tmd3702 source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_ams_tsl2540.h"

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

static syn3133_als_para_table syn3133_als_para_diff_tp_color_table[] = {
	{ ALTB, V3, VISI_TPLCD, OTHER,
	  { 512, 0, 328, 542, 0, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 18119,
	    9934, 17401, 1407, 4000, 250 }
	},
	{ ALTB, V3, BOE_TPLCD, OTHER,
	  { 512, 0, 83, 757, 0, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 12959,
	    6936, 10858, 684, 4000, 250 }
	}
};

syn3133_als_para_table *als_get_syn3133_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(syn3133_als_para_diff_tp_color_table))
		return NULL;
	return &(syn3133_als_para_diff_tp_color_table[id]);
}

syn3133_als_para_table *als_get_syn3133_first_table(void)
{
	return &(syn3133_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_syn3133_table_count(void)
{
	return ARRAY_SIZE(syn3133_als_para_diff_tp_color_table);
}
