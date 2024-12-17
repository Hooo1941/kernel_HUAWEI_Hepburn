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

static sip3623_als_para_table sip3623_als_para_diff_tp_color_table[] = {
	{ BARRY, V3, DEFAULT_TPLCD, OTHER,
	  { 128, -233, 5333, 24940, -2657, 2, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 21078,
	    7675, 5554, 412, 4000, 250 }
	},
	{ ALLEN, V3, DEFAULT_TPLCD, OTHER,
	  { 128, -112, 7367, 23548, -23162, 2, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 21078,
	    7675, 5554, 412, 4000, 250 }
	},
	{ ALTB, V3, DEFAULT_TPLCD, OTHER,
	  { 32, 0, 1584, 1709, -5643, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 26134,
	    3774, 2695, 318, 4000, 250 }
	},
	{ LEM, V3, BOE_TPLCD, OTHER,
	  { 1024, 0, 595, 899, -2259, 100, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 8, 28724,
	    3405, 2600, 156, 4000, 250 }
	},
	{ LEM, V3, VISI_TPLCD, OTHER,
	  { 1024, 0, -360, 7915, -6972, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 16, 25970,
	    3552, 3046, 191, 4000, 250 }
	},
	{ ADY, V3, BOE_TPLCD, OTHER,
	  { 128, 0, 1920, 3835, -6999, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 22322,
	    7259, 5617, 223, 4000, 250 }
	},
	{ ADY, V3, DEFAULT_TPLCD, OTHER,
	  { 128, 0, 1920, 3835, -6999, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 22322,
	    7259, 5617, 223, 4000, 250 }
	},
	{ HBN, V3, BOE_TPLCD, OTHER,
	  { 128, 0, 1285, 3088, -3937, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 28407,
	    9326, 7516, 371, 4000, 250 }
	},
	{ HBN, V3, DEFAULT_TPLCD, OTHER,
	  { 128, -61, 1568, 2775, 9028, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 1, 28407,
	    9326, 7516, 371, 4000, 250 }
	},
	{ LEM, V3, BOE_TPLCD_B12, OTHER,
	  { 1024, 0, -1464, 15501, -7882, 10, 0, 5690, -4716, 2529, -5526, 100,
	    0, 1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20, 8, 28795,
	    3828, 3271, 218, 4000, 250 }
	},

};

sip3623_als_para_table *als_get_sip3623_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sip3623_als_para_diff_tp_color_table))
		return NULL;
	return &(sip3623_als_para_diff_tp_color_table[id]);
}

sip3623_als_para_table *als_get_sip3623_first_table(void)
{
	return &(sip3623_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_sip3623_table_count(void)
{
	return ARRAY_SIZE(sip3623_als_para_diff_tp_color_table);
}
