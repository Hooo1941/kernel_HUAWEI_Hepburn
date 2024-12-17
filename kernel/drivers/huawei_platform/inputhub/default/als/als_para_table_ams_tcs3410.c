

#include "als_para_table.h"

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

static tcs3410_als_para_table tcs3410_als_para_diff_tp_color_table[] = {
	{ GRL, V3, DEFAULT_TPLCD, OTHER,
		{512, // normalized gain
		18820, 0, -767, -14847, 100, // screen-off para: c/r/g/b/dgf * 1000
		1,                            // dgf / ratio
		5690, -4716, 2529, -5526, 100,
		100,                        // normalized atime
		1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20,
		12, 27450, 0, 8394, 23481, // ratio/(target/ration)c/r/g/b
		4000, 250
		}
	},
	{ TGR, V3, DEFAULT_TPLCD, BLACK,
		{486, // normalized gain
		-22, 15, 705, -215, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		-22, 15, 705, -215, 1,
		100,                        // normalized atime
		1, 30, 24472, 0, 0, -2165, 16183, 0, 0, 169, 1, 30,
		1, 16918, 8956, 4675, 3195, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ TGR, V3, DEFAULT_TPLCD, BLACK2,
		{486, // normalized gain
		-22, 15, 705, -215, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		-22, 15, 705, -215, 1,
		100,                        // normalized atime
		1, 30, 24472, 0, 0, -2165, 16183, 0, 0, 169, 1, 30,
		1, 16918, 8956, 4675, 3195, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ TGR, V3, DEFAULT_TPLCD, GRAY,
		{486, // normalized gain
		-22, 15, 705, -215, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		-22, 15, 705, -215, 1,
		100,                        // normalized atime
		1, 30, 24472, 0, 0, -2165, 16183, 0, 0, 169, 1, 30,
		1, 16918, 8956, 4675, 3195, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ TGR, V3, DEFAULT_TPLCD, BROWN,
		{486, // normalized gain
		-22, 15, 705, -215, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		-22, 15, 705, -215, 1,
		100,                        // normalized atime
		1, 30, 24472, 0, 0, -2165, 16183, 0, 0, 169, 1, 30,
		1, 16918, 8956, 4675, 3195, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ MRO, V3, DEFAULT_TPLCD, BLACK,
		{486, // normalized gain
		760, 49, -686, -1380, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		238, -199, 710, -479, 1,
		100,                        // normalized atime
		0, 30, 124, 0, 0, 6405, 20493, 0, 0, -1721, 0, 30,
		1, 4798, 2713, 1670, 1046, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ MRO, V3, DEFAULT_TPLCD, BLACK2,
		{486, // normalized gain
		760, 49, -686, -1380, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		238, -199, 710, -479, 1,
		100,                        // normalized atime
		0, 30, 124, 0, 0, 6405, 20493, 0, 0, -1721, 0, 30,
		1, 4798, 2713, 1670, 1046, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ BKY, V3, DEFAULT_TPLCD, BLACK,
		{486, // normalized gain
		-125, 209, 676, -104, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                       // dgf / ratio
		-240, 222, 1335, -443, 1,
		100,                        // normalized atime
		1, 30, 19905, 0, 0, -345, 14871, 0, 0, 497, 1, 30,
		1, 7716, 3034, 2412, 1301, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
	{ BKY, V3, DEFAULT_TPLCD, BLACK2,
		{486, // normalized gain
		-125, 209, 676, -104, 1, // screen-off para: c/r/g/b/dgf * 1000
		1,                       // dgf / ratio
		-240, 222, 1335, -443, 1,
		100,                        // normalized atime
		1, 30, 19905, 0, 0, -345, 14871, 0, 0, 497, 1, 30,
		1, 7716, 3034, 2412, 1301, // ratio/(target/ration)c/r/g/b
		5000, 200 }
	},
};

static tcs3410_als_para_table tcs3410_als1_para_diff_tp_color_table[] = {
	{ GRL, V3, DEFAULT_TPLCD, OTHER,
		{512, // normalized gain
		15789, 0, 7993, -15189, 100, // screen-off para: c/r/g/b/dgf * 1000
		1,                           // dgf / ratio
		5690, -4716, 2529, -5526, 100,
		100,                        // normalized atime
		1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20,
		12, 25021, 0, 7633, 20939, // ratio/(target/ration)c/r/g/b
		4000, 250
		}
	},
};

tcs3410_als_para_table *als_get_tcs3410_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tcs3410_als_para_diff_tp_color_table))
		return NULL;
	return &(tcs3410_als_para_diff_tp_color_table[id]);
}

tcs3410_als_para_table *als_get_tcs3410_first_table(void)
{
	return &(tcs3410_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_tcs3410_table_count(void)
{
	return ARRAY_SIZE(tcs3410_als_para_diff_tp_color_table);
}

tcs3410_als_para_table *als1_get_tcs3410_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(tcs3410_als1_para_diff_tp_color_table))
		return NULL;
	return &(tcs3410_als1_para_diff_tp_color_table[id]);
}

tcs3410_als_para_table *als1_get_tcs3410_first_table(void)
{
	return &(tcs3410_als1_para_diff_tp_color_table[0]);
}

uint32_t als1_get_tcs3410_table_count(void)
{
	return ARRAY_SIZE(tcs3410_als1_para_diff_tp_color_table);
}
