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

static sip3642_als_para_table sip3642_als_para_diff_tp_color_table[] = {
	{ ADY, V3, BOE_TPLCD, OTHER,
		{ 64, 0, -11238, 19879, 2597, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8740, 10287, 15684, 11804, 4000, 250 }
	},
	{ ADY, V3, BOE_TPLCD_B12, OTHER,
		{ 64, 0, -4424, 16012, -2256, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8740, 10287, 15684, 11804, 4000, 250 }
	},
	{ ADY, V3, DEFAULT_TPLCD, OTHER,
		{ 64, 0, -11238, 19879, 2597, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8740, 10287, 15684, 11804, 4000, 250 }
	},
	{HBN, V3, BOE_TPLCD, OTHER,
		{ 64, 0, -11238, 19879, 2597, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8740, 10287, 15684, 11804, 4000, 250 }
	},
	{HBN, V3, DEFAULT_TPLCD, OTHER,
		{ 64, 0, -11238, 19879, 2597, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8740, 10287, 15684, 11804, 4000, 250 }
	},

};

sip3642_als_para_table *als_get_sip3642_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sip3642_als_para_diff_tp_color_table))
		return NULL;
	return &(sip3642_als_para_diff_tp_color_table[id]);
}

sip3642_als_para_table *als_get_sip3642_first_table(void)
{
	return &(sip3642_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_sip3642_table_count(void)
{
	return ARRAY_SIZE(sip3642_als_para_diff_tp_color_table);
}
