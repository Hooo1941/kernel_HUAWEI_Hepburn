/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for schargerV700 batctl
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/mutex.h>
#include "bat_par_ctrl.h"
#include "scharger_v700_batctl.h"

struct scharger_batctl {
	struct device *dev;
	struct regmap *regmap;
	struct mutex ctrl_lock;
};

static int bat_cnct_mode = -1;
struct scharger_batctl *g_batctl = NULL;

int bat_cnct_ctrl(int mode)
{
	int ret = 0;
	struct scharger_batctl *batctl = g_batctl;

	pr_info("%s mode:%d\n", __func__, mode);

	if (batctl == NULL) {
		pr_err("%s scharger_batctl is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (mode != BAT_PARALLEL_MODE && mode != BAT_SERIES_MODE) {
		dev_err(batctl->dev, "%s: mode is invalid\n", __func__);
		return -1;
	}

	mutex_lock(&batctl->ctrl_lock);
	if (mode != bat_cnct_mode) {
		bat_cnct_mode = mode;
		if (mode == BAT_SERIES_MODE) {
			ret = bat_par_ctrl_set_switch_off(true);
			msleep(10); /* sleep 10ms */
			ret += regmap_update_bits(batctl->regmap,
				BAT_CNCT_SEL_REG, BAT_CNCT_SEL_MSK, (u8)mode);
		} else {
			ret = regmap_update_bits(batctl->regmap,
				BAT_CNCT_SEL_REG, BAT_CNCT_SEL_MSK, (u8)mode);
			if (ret) {
				mutex_unlock(&batctl->ctrl_lock);
				return ret;
			}
			msleep(10); /* sleep 10ms */
			ret += bat_par_ctrl_set_switch_off(false);
		}
	}
	mutex_unlock(&batctl->ctrl_lock);
	return ret;
}
int bat_cnct_ctrl_test(int mode)
{
	struct scharger_batctl *batctl = g_batctl;

	if (batctl == NULL) {
		pr_err("%s scharger_batctl is NULL!\n", __func__);
		return -ENOMEM;
	}

	if(mode == BAT_SERIES_MODE)
		regmap_update_bits(batctl->regmap, BAT_DET_REG,
				BATSW_DET_EN_MSK, 0);

	bat_cnct_ctrl(mode);

	if(mode == BAT_PARALLEL_MODE)
		regmap_update_bits(batctl->regmap, BAT_DET_REG,
				BATSW_DET_EN_MSK, BATSW_DET_EN_MSK);
	return 0;
}

int bat_cnct_mode_get(void)
{
	struct scharger_batctl *batctl = g_batctl;
	unsigned int val = 0;

	if (batctl == NULL) {
		pr_err("%s scharger_batctl is NULL!\n", __func__);
		return BAT_PARALLEL_MODE;
	}

	regmap_read(batctl->regmap, BAT_CNCT_SEL_REG, &val);

	return (val & BAT_CNCT_SEL_MSK);
}

static int bat_sw_ctrl(int enable)
{
	int ret;
	ret = regmap_update_bits(g_batctl->regmap, BAT_SW_EN_REG, BAT_SW_EN_MSK, !!enable);
	if (ret) {
		dev_err(g_batctl->dev, "bat_sw_ctrl fail\n");
		return -1;
	}
	return 0;
}

static int bat_cnct_init(void)
{
	int ret;
    /* default bat parallel */
	ret = bat_cnct_ctrl(BAT_PARALLEL_MODE);
	if (ret) {
		dev_err(g_batctl->dev, "%s: bat_cnt_ctrl fail\n", __func__);
		return -1;
	}
    /* default bat sw enable */
	ret = bat_sw_ctrl(BAT_SW_ENABLE);
	if (ret) {
		dev_err(g_batctl->dev, "%s: bat_sw_ctrl fail\n", __func__);
		return -1;
	}

	return 0;
}

static int scharger_batctl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct scharger_batctl *batctl = NULL;

	batctl = devm_kzalloc(dev, sizeof(*batctl), GFP_KERNEL);
	if (!batctl) {
		dev_err(dev, "[%s]batctl is null.\n", __func__);
		return -ENOMEM;
	}

	batctl->dev = dev;
	batctl->regmap = dev_get_regmap(dev->parent, NULL);
	if (!batctl->regmap) {
		dev_err(dev, "Parent regmap unavailable.\n");
		return -ENXIO;
	}

	mutex_init(&batctl->ctrl_lock);
	g_batctl = batctl;

	bat_cnct_init();
	pr_err("[%s] succ\n", __func__);
	return 0;
}

static int scharger_batctl_remove(struct platform_device *pdev)
{
	struct scharger_batctl *batctl = g_batctl;

	if (!batctl)
		return -ENODEV;

	mutex_destroy(&batctl->ctrl_lock);
	g_batctl = NULL;

	return 0;
}

const static struct of_device_id of_scharger_batctl_match_tbl[] = {
	{
		.compatible = "hisilicon,scharger-v700-batctl",
		.data = NULL,
	},
	{},
};

static struct platform_driver scharger_batctl_driver = {
	.driver = {
		.name	= "scharger_v700_batctl",
		.owner  = THIS_MODULE,
		.of_match_table = of_scharger_batctl_match_tbl,
	},
	.probe	= scharger_batctl_probe,
	.remove	= scharger_batctl_remove,
};

static int __init scharger_batctl_init(void)
{
	return platform_driver_register(&scharger_batctl_driver);
}

static void __exit scharger_batctl_exit(void)
{
	platform_driver_unregister(&scharger_batctl_driver);
}

fs_initcall(scharger_batctl_init);
module_exit(scharger_batctl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("schargerV700 batctl driver");
