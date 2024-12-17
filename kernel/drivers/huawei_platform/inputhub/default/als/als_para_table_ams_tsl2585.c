/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tsl2585 source file
 * Author: lisheng
 * Create: 2024-01-19
 */

#include "als_para_table_ams_tsl2585.h"

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

static als_para_normal_table tsl2585_als_para_diff_tp_color_table[] = {
	{ PSD, V3, VISI_TPLCD, TSL2585_PARA_SIZE,
	  {120, 220, 360, 2000, 3158, 3451, 3535, 3619, 63000, 5000, 4000, 250, 0, 0, 0, 0}
	},
	{ PSD, V3, BOE_TPLCD_B12, TSL2585_PARA_SIZE,
	  {120, 220, 350, 2000, 4957, 5613, 5545, 5600, 43601, 3679, 4000, 250, 0, 0, 0, 0}
	},
	{ PSD, V3, CSOT_TPLCD, TSL2585_PARA_SIZE,
	  {110, 250, 310, 2000, 5059, 5683, 5691, 5955, 43601, 3679, 4000, 250, 0, 0, 0, 0}
	},
	{ PSD, V3, TS_PANEL_UNKNOWN, TSL2585_PARA_SIZE,
	  {110, 250, 310, 2000, 5059, 5683, 5691, 5955, 43601, 3679, 4000, 250, 0, 0, 0, 0}
	},
};

als_para_normal_table *als_get_tsl2585_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tsl2585_als_para_diff_tp_color_table))
		return NULL;
	return &(tsl2585_als_para_diff_tp_color_table[id]);
}

als_para_normal_table *als_get_tsl2585_first_table(void)
{
	return &(tsl2585_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tsl2585_table_count(void)
{
	return ARRAY_SIZE(tsl2585_als_para_diff_tp_color_table);
}
