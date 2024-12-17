/*
 * lcd_kit_core.h
 *
 * lcdkit core function for lcdkit head file
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifdef CONFIG_MODULE_KO_TP
#include <huawei_thp_api.h>

#ifdef CONFIG_LCD_KIT_DRIVER
/* ts kit ops */
struct ts_kit_ops *g_ts_kit_ops;
/* lcd kit ops register function */
int ts_kit_ops_register(struct ts_kit_ops *ops)
{
	if (!g_ts_kit_ops) {
		g_ts_kit_ops = ops;
		printk(KERN_INFO "[TS_KIT]%s: ops register suc!\n", __func__);
		return 0;
	}
	printk(KERN_ERR "[TS_KIT]%s: ops have been registered!\n", __func__);
	return -1;
}
EXPORT_SYMBOL(ts_kit_ops_register);

/* lcd kit ops unregister function */
int ts_kit_ops_unregister(struct ts_kit_ops *ops)
{
	if (g_ts_kit_ops == ops) {
		g_ts_kit_ops = NULL;
		printk(KERN_INFO "[TS_KIT]%s: ops unregister suc!\n", __func__);
		return 0;
	}
	printk(KERN_ERR "[TS_KIT]%s: ops unregister fail!\n", __func__);
	return -1;
}
EXPORT_SYMBOL(ts_kit_ops_unregister);

/* get lcd kit ops function */
struct ts_kit_ops *ts_kit_get_ops(void)
{
	return g_ts_kit_ops;
}
EXPORT_SYMBOL(ts_kit_get_ops);
#endif
#endif