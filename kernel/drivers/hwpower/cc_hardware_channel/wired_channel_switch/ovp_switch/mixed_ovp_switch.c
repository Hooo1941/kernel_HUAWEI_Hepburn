// SPDX-License-Identifier: GPL-2.0
/*
 * mixed_ovp_switch.c
 *
 * mixed ovp switch to control wired channel
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/hardware_channel/power_sw.h>

#define HWLOG_TAG mixed_ovp_chsw
HWLOG_REGIST();

#define CHSW_NAME_LEN  15

struct movp_chsw_para {
	bool used;
	bool ready;
	const char *label;
	int restraint; /* A restrains B: if B-chsw on,  A must on */
	int idx;
};

struct movp_chsw_dev {
	struct device *dev;
	bool ready;
	struct movp_chsw_para *para;
};

struct movp_chsw_para g_movp_chsw_para[WIRED_CHANNEL_END];
struct movp_chsw_dev *g_movp_chsw_di;

static bool movp_chsw_ready(struct movp_chsw_dev *di)
{
	int i;

	if (di->ready)
		return true;

	for (i = 0; i < WIRED_CHANNEL_END; i++) {
		if  (!di->para[i].used || di->para[i].ready)
			continue;
		if (!power_sw_ready(di->para[i].idx))
			return false;
		di->para[i].ready = true;
	}

	di->ready = true;
	return true;
}

static int movp_chsw_get_wired_channel(int chsw_type)
{
	int i;
	struct movp_chsw_dev *di = g_movp_chsw_di;

	if (!di || !movp_chsw_ready(di)) {
		hwlog_err("get_wired_channel: di null or not ready\n");
		return WIRED_CHANNEL_RESTORE;
	}

	if ((chsw_type < WIRED_CHANNEL_BEGIN) || (chsw_type >= WIRED_CHANNEL_END))
		return WIRED_CHANNEL_RESTORE;

	if (chsw_type == WIRED_CHANNEL_ALL) {
		for (i = 0 ; i < WIRED_CHANNEL_ALL; i++) {
			if (!di->para[i].used)
				continue;
			if (power_sw_get_output(di->para[i].idx) == POWER_SW_ON)
				return WIRED_CHANNEL_RESTORE;
		}
		return WIRED_CHANNEL_CUTOFF;
	}

	if (!di->para[chsw_type].used)
		return WIRED_CHANNEL_CUTOFF;

	return power_sw_get_output(di->para[chsw_type].idx) == POWER_SW_ON ?
		WIRED_CHANNEL_RESTORE : WIRED_CHANNEL_CUTOFF;
}

static int movp_chsw_set_on(struct movp_chsw_dev *di, int chsw_type, int flag)
{
	int i;

	for (i = 0; i < WIRED_CHANNEL_END; i++) {
		if (!di->para[i].used || !di->para[i].ready)
			continue;
		if ((i != chsw_type) && (chsw_type != di->para[i].restraint))
			continue;
		if (power_sw_set_output(di->para[i].idx, POWER_SW_ON))
			return -EIO;
	}

	return 0;
}

static int movp_chsw_set_off(struct movp_chsw_dev *di, int chsw_type, int flag)
{
	int restraint = di->para[chsw_type].restraint;

	if (!di->para[chsw_type].used || !di->para[chsw_type].ready)
		return -EINVAL;

	if (restraint >= 0) {
		if (movp_chsw_get_wired_channel(restraint) == WIRED_CHANNEL_RESTORE) {
			hwlog_info("[set_off] %s off refused in case %s need it on\n",
				di->para[chsw_type].label, di->para[restraint].label);
			return 0;
		}
	}

	return power_sw_set_output(di->para[chsw_type].idx, POWER_SW_OFF);
}

static int movp_chsw_set(struct movp_chsw_dev *di, int chsw_type, int flag)
{
	if (flag == WIRED_CHANNEL_RESTORE)
		return movp_chsw_set_on(di, chsw_type, flag);
	else
		return movp_chsw_set_off(di, chsw_type, flag);
}

static int movp_chsw_set_wired_channel(int chsw_type, int flag)
{
	int i;
	struct movp_chsw_dev *di = g_movp_chsw_di;

	if (!di || !movp_chsw_ready(di)) {
		hwlog_err("set_wired_channel: di null or not ready\n");
		return -ENODEV;
	}

	if ((chsw_type < WIRED_CHANNEL_BEGIN) || (chsw_type >= WIRED_CHANNEL_END))
		return -EINVAL;

	if (chsw_type != WIRED_CHANNEL_ALL)
		return movp_chsw_set(di, chsw_type, flag);

	for (i = 0; i < WIRED_CHANNEL_ALL; i++)
		if (movp_chsw_set(di, i, flag))
			return -EIO;

	return 0;
}

static int movp_chsw_set_other_wired_channel(int chsw_type, int flag)
{
	int i;
	struct movp_chsw_dev *di = g_movp_chsw_di;

	if (!di || !movp_chsw_ready(di)) {
		hwlog_err("set_other_wired_channel: di null or not ready\n");
		return -ENODEV;
	}

	if ((chsw_type < WIRED_CHANNEL_BEGIN) || (chsw_type >= WIRED_CHANNEL_ALL))
		return 0;

	for (i = 0; i < WIRED_CHANNEL_ALL; i++) {
		if (!di->para[i].used)
			continue;
		if (di->para[chsw_type].idx == di->para[i].idx)
			continue;
		if (movp_chsw_set(di, i, flag))
			return -EIO;
	}

	return 0;
}

static struct wired_chsw_device_ops g_movp_chsw_ops = {
	.get_wired_channel = movp_chsw_get_wired_channel,
	.set_wired_channel = movp_chsw_set_wired_channel,
	.set_other_wired_channel = movp_chsw_set_other_wired_channel,
};

static int movp_chsw_label2idx_map(const char *label)
{
	unsigned int idx;
	static const char * const chsw_label_map[WIRED_CHANNEL_ALL] = {
		[WIRED_CHANNEL_BUCK] = "buck",
		[WIRED_CHANNEL_LVC] = "lvc",
		[WIRED_CHANNEL_LVC_MOS] = "lvc_mos",
		[WIRED_CHANNEL_SC] = "sc",
		[WIRED_CHANNEL_SC_AUX] = "sc_aux",
		[WIRED_CHANNEL_SC4] = "sc4",
	};

	for (idx = WIRED_CHANNEL_BEGIN; idx < WIRED_CHANNEL_ALL; idx++) {
		if (!strcmp(chsw_label_map[idx], label))
			break;
	}

	return idx;
}

static int movp_chsw_parse_cfg(struct device_node *np, struct movp_chsw_dev *di)
{
	int i;
	int chsw_cnt;
	int chsw_idx;
	int res_idx;
	const char *label = NULL;

	chsw_cnt = of_property_count_strings(np, "labels");
	if (chsw_cnt <= 0) {
		hwlog_err("init_resources: labels configured err\n");
		return -EINVAL;
	}

	for (i = 0; i < chsw_cnt; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "labels", i, &label))
			return -EINVAL;
		chsw_idx = movp_chsw_label2idx_map(label);
		if ((chsw_idx < 0) || (chsw_idx >= WIRED_CHANNEL_ALL))
			return -EINVAL;
		di->para[chsw_idx].used = true;
		di->para[chsw_idx].label = label;
		if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
			np, "indexs", i, &di->para[chsw_idx].idx))
			return -EINVAL;
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "restraints", i, &label))
			return -EINVAL;
		res_idx = movp_chsw_label2idx_map(label);
		if ((res_idx < 0) || (res_idx >= WIRED_CHANNEL_ALL))
			di->para[chsw_idx].restraint = -1;
		else
			di->para[chsw_idx].restraint = res_idx;
	}

	return 0;
}

static int mixed_ovp_chsw_probe(struct platform_device *pdev)
{
	int ret;
	struct movp_chsw_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	di->para = g_movp_chsw_para;

	ret = movp_chsw_parse_cfg(np, di);
	if (ret) {
		devm_kfree(&pdev->dev, di);
		return ret;
	}

	ret = wired_chsw_ops_register(&g_movp_chsw_ops);
	if (ret) {
		devm_kfree(&pdev->dev, di);
		return ret;
	}

	g_movp_chsw_di = di;
	platform_set_drvdata(pdev, di);
	hwlog_info("mixed_ovp_chsw probe ok\n");
	return 0;
}

static int mixed_ovp_chsw_remove(struct platform_device *pdev)
{
	struct movp_chsw_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	g_movp_chsw_di = NULL;
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id mixed_ovp_chsw_match_table[] = {
	{
		.compatible = "huawei,mixed_ovp_chsw",
		.data = NULL,
	},
	{},
};

static struct platform_driver mixed_ovp_chsw_driver = {
	.probe = mixed_ovp_chsw_probe,
	.remove = mixed_ovp_chsw_remove,
	.driver = {
		.name = "huawei,mixed_ovp_chsw",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mixed_ovp_chsw_match_table),
	},
};

static int __init mixed_ovp_chsw_init(void)
{
	return platform_driver_register(&mixed_ovp_chsw_driver);
}

static void __exit mixed_ovp_chsw_exit(void)
{
	platform_driver_unregister(&mixed_ovp_chsw_driver);
}

rootfs_initcall(mixed_ovp_chsw_init);
module_exit(mixed_ovp_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mixed ovp switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
