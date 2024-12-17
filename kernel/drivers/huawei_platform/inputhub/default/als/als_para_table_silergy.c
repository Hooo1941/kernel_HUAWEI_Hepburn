/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table silergy source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_table_silergy.h"

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

als_para_normal_table syh399_als_para[] = {
	{ TEL, V3, TCL_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, TM_TPLCD, SYH399_PARA_SIZE,
	  { 2189, 24089, 185, 120, 300, 7433, 10000, 3357, 3796, 8450, 768, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, BOE_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, INX_TPLCD, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, BOE_TPLCD2, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ TEL, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ JSC, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  { 2055, 23010, 244, 120, 300, 7742, 10000, 2359, 4234, 9004, 804, 10000, 100, 600, 230, 25 }
	},

	{ LNA, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  { 1309, 11104, 700, 120, 250, 2000, 9899, 10459, 11087, 9509, 16560, 1882, 4000, 250, 0, 0}
	},

	{ BLK, V3, VISI_TPLCD, SYH399_PARA_SIZE,
	  {180, 230, 420, 2000, 1505, 1559, 1627, 1676, 16560, 1882, 4000, 250, 0, 0, 0, 0}
	},

	{ BLK, V3, CSOT_TPLCD, SYH399_PARA_SIZE,
	  {170, 230, 390, 2000, 1480, 1544, 1623, 1633, 16560, 1882, 4000, 250, 0, 0, 0, 0}
	},

	{ BLK, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  {210, 300, 440, 2000, 1335, 1406, 1482, 1536, 16560, 1882, 4000, 250, 0, 0, 0, 0}
	},

	{ ADA, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  {210, 300, 480, 2000, 1887, 1984, 2085, 2035, 9930, 1020, 4000, 250, 0, 0, 0, 0}
	},

	{ ADA, V3, CSOT_TPLCD, SYH399_PARA_SIZE,
	  {180, 250, 420, 2000, 2521, 2678, 2969, 3045, 9930, 1020, 4000, 250, 0, 0, 0, 0}
	},

	{ ADL, V3, CSOT_TPLCD, SYH399_PARA_SIZE,
	  {200, 280, 480, 2000, 1995, 2048, 2090, 2160, 9930, 1020, 4000, 250, 0, 0, 0, 0}
	},

	{ ADL, V3, BOE_TPLCD, SYH399_PARA_SIZE,
	  {170, 260, 430, 2000, 1939, 2013, 2121, 2000, 9930, 1020, 4000, 250, 0, 0, 0, 0}
	},

	{ ADL, V3, TS_PANEL_UNKNOWN, SYH399_PARA_SIZE,
	  {180, 250, 420, 2000, 2521, 2678, 2969, 3046, 9930, 1020, 4000, 250, 0, 0, 0, 0}
	},
};
static sy3133_als_para_table sy3133_als_para[] = {
	{ PCE, V3, DEFAULT_TPLCD, BLACK,
	{ 100, 120, 9113, 192, 7994, 15472, 1460, 807, 0, 120, 9113, 192,
		7994, 15472, 1460, 807, 0, 14752, 25094, 4040, 6436, 0, 100, 2048, 6, 0,
		0, 10000, 200 } },
	{ PCE, V3, DEFAULT_TPLCD, WHITE,
	{ 100, 120, 9113, 192, 7994, 15472, 1460, 807, 0, 120, 9113, 192,
		7994, 15472, 1460, 807, 0, 14752, 25094, 4040, 6436, 0, 100, 2048, 6, 0,
		0, 10000, 200 } },
	{ PCE, V3, TS_PANEL_VXN, BLACK,
	{ 100, 120, 9113, 192, 7994, 15500, 1400, 960, 0, 120, 9113, 192,
		7994, 17000, 1400, 960, 0, 12684, 21844, 3600, 6100, 0, 100, 2048, 6, 0,
		250, 10000, 200 } },
	{ PCE, V3, TS_PANEL_VXN, WHITE,
	{ 100, 120, 9113, 192, 7994, 15500, 1400, 960, 0, 120, 9113, 192,
		7994, 17000, 1400, 960, 0, 12684, 21844, 3600, 6100, 0, 100, 2048, 6, 0,
		250, 10000, 200 } },
	{ XYAO, V3, DEFAULT_TPLCD, BLACK,
	{ 100, 120, 9113, 192, 7994, 12472, 1260, 364, 0, 120, 9113, 192,
		7994, 12472, 1800, 364, 0, 23198, 25000, 8052, 8885, 15552, 50, 2048, 5, 0,
		300, 6000, 100 } },
	{ XYAO, V3, DEFAULT_TPLCD, WHITE,
	{ 100, 120, 9113, 192, 7994, 10800, 1050, 1300, 0, 120, 9113, 192,
		7994, 11500, 1100, 1300, 0, 11671, 21105, 5426, 4429, 0, 25, 2048, 4, 0,
		400, 6000, 100 } },
	{ XYAO, V3, TS_PANEL_VXN, BLACK,
	{ 100, 120, 9113, 192, 7994, 12472, 1260, 355, 0, 120, 9113, 192,
		7994, 12472, 1800, 355, 0, 23198, 25000, 8052, 8885, 15552, 50, 2048, 5, 0,
		300, 6000, 100 } },
	{ XYAO, VN1, DEFAULT_TPLCD, BLACK,
	{ 100, 120, 9113, 192, 7994, 12472, 1260, 364, 0, 120, 9113, 192,
		7994, 12472, 1800, 364, 0, 23198, 25000, 8052, 8885, 15552, 50, 2048, 5, 0,
		300, 6000, 100 } },
	{ XYAO, VN1, DEFAULT_TPLCD, WHITE,
	{ 100, 120, 9113, 192, 7994, 10800, 1050, 1300, 0, 120, 9113, 192,
		7994, 11500, 1100, 1300, 0, 11671, 21105, 5426, 4429, 0, 25, 2048, 4, 0,
		400, 6000, 100 } },
	{ XYAO, VN1, TS_PANEL_VXN, BLACK,
	{ 100, 120, 9113, 192, 7994, 12472, 1260, 355, 0, 120, 9113, 192,
		7994, 12472, 1800, 355, 0, 23198, 25000, 8052, 8885, 15552, 50, 2048, 5, 0,
		300, 6000, 100 } },
};

als_para_normal_table *als_get_syh399_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(syh399_als_para))
		return NULL;
	return &(syh399_als_para[id]);
}

als_para_normal_table *als_get_syh399_first_table(void)
{
	return &(syh399_als_para[0]);
}

uint32_t als_get_syh399_table_count(void)
{
	return ARRAY_SIZE(syh399_als_para);
}

sy3133_als_para_table *als_get_sy3133_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sy3133_als_para))
		return NULL;
	return &(sy3133_als_para[id]);
}

sy3133_als_para_table *als_get_sy3133_first_table(void)
{
	return &(sy3133_als_para[0]);
}

uint32_t als_get_sy3133_table_count(void)
{
	return ARRAY_SIZE(sy3133_als_para);
}
