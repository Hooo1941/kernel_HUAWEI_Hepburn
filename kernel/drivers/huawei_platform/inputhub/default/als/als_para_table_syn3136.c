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

static syn3136_als_para_table syn3136_als_para_diff_tp_color_table[] = {
	{ BARRY, V3, DEFAULT_TPLCD, OTHER,
	  { 32, 0, 228, 905, -853, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 26134,
	    3774, 2695, 318, 60000, 1}
	},
	{ ALLEN, V3, DEFAULT_TPLCD, OTHER,
	  { 32, 0, 228, 905, -853, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 0, 26134,
	    3774, 2695, 318, 60000, 1 }
	},
};

syn3136_als_para_table *als_get_syn3136_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(syn3136_als_para_diff_tp_color_table))
		return NULL;
	return &(syn3136_als_para_diff_tp_color_table[id]);
}

syn3136_als_para_table *als_get_syn3136_first_table(void)
{
	return &(syn3136_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_syn3136_table_count(void)
{
	return ARRAY_SIZE(syn3136_als_para_diff_tp_color_table);
}
