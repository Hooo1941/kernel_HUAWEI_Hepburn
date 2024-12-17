

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

static sip2326_als_para_table sip2326_als_para_diff_tp_color_table[] = {
	{ GRL, V3, DEFAULT_TPLCD, OTHER,
		{2048, // normalized gain
		0, 7211, 20382, -8385, 1000,       // screen-off para1: c/r/g/b/dgf
		1,                                 // dgf / ratio
		-2031, 3476, 20137, -4718, 2000,   // screen-off para2(ir): c/r/g/b/dgf (unused)
		200,                               // normalized atime
		1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20,
		24, 13659, 2837, 2214, 1256, // ratio/(target/ration)c/r/g/b
		4000, 250
		}
	},
	{ TGR, V3, DEFAULT_TPLCD, BLACK,
		{2048, // normalized gain
		6, -22, 75, -1, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		6, -22, 75, -1, 1,
		100,                        // normalized atime
		1, 52, 13813, 0, 0, -2123, 7508, 0, 0, 952, 1, 52,
		6, 15155, 9448, 6125, 5496, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ TGR, V3, DEFAULT_TPLCD, BLACK2,
		{2048, // normalized gain
		6, -22, 75, -1, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		6, -22, 75, -1, 1,
		100,                        // normalized atime
		1, 52, 13813, 0, 0, -2123, 7508, 0, 0, 952, 1, 52,
		6, 15155, 9448, 6125, 5496, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ TGR, V3, DEFAULT_TPLCD, GRAY,
		{2048, // normalized gain
		6, -22, 75, -1, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		6, -22, 75, -1, 1,
		100,                        // normalized atime
		1, 52, 13813, 0, 0, -2123, 7508, 0, 0, 952, 1, 52,
		6, 15155, 9448, 6125, 5496, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ TGR, V3, DEFAULT_TPLCD, BROWN,
		{2048, // normalized gain
		6, -22, 75, -1, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		6, -22, 75, -1, 1,
		100,                        // normalized atime
		1, 52, 13813, 0, 0, -2123, 7508, 0, 0, 952, 1, 52,
		6, 15155, 9448, 6125, 5496, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ MRO, V3, DEFAULT_TPLCD, BLACK,
		{2048, // normalized gain
		7, 80, -26, -60, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		-4, 30, 18, 10, 1,
		100,                        // normalized atime
		1, 54, 9781, 0, 0, 0, 4681, 0, 0, 1800, 1, 54,
		6, 10562, 5986, 5182, 4585, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ MRO, V3, DEFAULT_TPLCD, BLACK2,
		{2048, // normalized gain
		7, 80, -26, -60, 1000, // screen-off para: c/r/g/b/dgf
		1000,                              // dgf / ratio
		-4, 30, 18, 10, 1,
		100,                        // normalized atime
		1, 54, 9781, 0, 0, 0, 4681, 0, 0, 1800, 1, 54,
		6, 10562, 5986, 5182, 4585, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ BKY, V3, DEFAULT_TPLCD, BLACK,
		{2048, // normalized gain
		-37, 36, 99, -10, 1000, // screen-off para: c/r/g/b/dgf
		1000,                  // dgf / ratio
		-14, 15, 64, -14, 1,
		100,                        // normalized atime
		1, 53, 8162, 0, 0, 1624, 6351, 0, 0, 1549, 1, 53,
		10, 6106, 3969, 3202, 2605, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
	{ BKY, V3, DEFAULT_TPLCD, BLACK2,
		{2048, // normalized gain
		-37, 36, 99, -10, 1000, // screen-off para: c/r/g/b/dgf
		1000,                  // dgf / ratio
		-14, 15, 64, -14, 1,
		100,                        // normalized atime
		1, 53, 8162, 0, 0, 1624, 6351, 0, 0, 1549, 1, 53,
		10, 6106, 3969, 3202, 2605, // ratio/(target/ratio)c/r/g/b
		5000, 200
		}
	},
};

static sip2326_als_para_table sip2326_als1_para_diff_tp_color_table[] = {
	{ GRL, V3, DEFAULT_TPLCD, OTHER,
		{2048, // normalized gain
		0, 5067, 22887, -8326, 1000,        // screen-off para1: c/r/g/b/dgf
		1,                                  // dgf / ratio
		-2673, 8652, 19568, -9322, 2000,    // screen-off para2(ir): c/r/g/b/dgf (unused)
		200,                                // normalized atime
		1, 20, -1000, 0, 0, 2776, 12000, 0, 0, -317, 1, 20,
		24, 11910, 2535, 1983, 1064, // ratio/(target/ration)c/r/g/b
		4000, 250
		}
	},
};

// als
sip2326_als_para_table *als_get_sip2326_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sip2326_als_para_diff_tp_color_table))
		return NULL;
	return &(sip2326_als_para_diff_tp_color_table[id]);
}

sip2326_als_para_table *als_get_sip2326_first_table(void)
{
	return &(sip2326_als_para_diff_tp_color_table[0]);
}

uint32_t als_get_sip2326_table_count(void)
{
	return ARRAY_SIZE(sip2326_als_para_diff_tp_color_table);
}

// als1
sip2326_als_para_table *als1_get_sip2326_table_by_id(uint32_t id)
{
	if (id >= ARRAY_SIZE(sip2326_als1_para_diff_tp_color_table))
		return NULL;
	return &(sip2326_als1_para_diff_tp_color_table[id]);
}

sip2326_als_para_table *als1_get_sip2326_first_table(void)
{
	return &(sip2326_als1_para_diff_tp_color_table[0]);
}

uint32_t als1_get_sip2326_table_count(void)
{
	return ARRAY_SIZE(sip2326_als1_para_diff_tp_color_table);
}
