/*
 * discharge_boost.c
 *
 * discharge boost driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include "dischg_boost.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <huawei_platform/log/hw_log.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG dischg_boost
HWLOG_REGIST();

#define PSY_NAME                  "Battery"
#define VSYS_SWITCH_SCENE_NORMAL  0x1
#define VSYS_SWITCH_SCENE_BOOST   0x2
#define VSYS_SWITCH_SCENE_PENDING 0x4 // usually as a buffer zone bewteen normal and boost

typedef int (*switch_handler)(struct dischg_boost_dev *di);

struct switch_cond {
	int prev_scene;
	int next_scene;
};

struct switch_item {
	struct switch_cond cond;
	switch_handler handler;
};

static int dischg_boost_switch_to_boost(struct dischg_boost_dev *di)
{
	hwlog_info("switch_to_boost begin\n");
	power_wakeup_lock(di->wakelock, false);
	gpio_direction_output(di->dischg_cfg.normal_gpio, 0);
	power_usleep(di->dischg_cfg.switch_boost_delay_ms * DT_USLEEP_1MS);
	gpio_direction_output(di->dischg_cfg.boost_gpio, 1);
	power_wakeup_unlock(di->wakelock, false);
	hwlog_info("switch_to_boost end\n");
	return 0;
}

static int dischg_boost_switch_to_normal(struct dischg_boost_dev *di)
{
	hwlog_info("switch_to_normal begin\n");
	power_wakeup_lock(di->wakelock, false);
	gpio_direction_output(di->dischg_cfg.boost_gpio, 0);
	power_usleep(di->dischg_cfg.switch_normal_delay_ms * DT_USLEEP_1MS);
	gpio_direction_output(di->dischg_cfg.normal_gpio, 1);
	power_wakeup_unlock(di->wakelock, false);
	hwlog_info("switch_to_normal end\n");
	return 0;
}

static const struct switch_item handler_map[] = {
	{
		.cond = {
			.prev_scene = VSYS_SWITCH_SCENE_BOOST | VSYS_SWITCH_SCENE_PENDING,
			.next_scene = VSYS_SWITCH_SCENE_NORMAL,
		},
		.handler = dischg_boost_switch_to_normal,
	},
	{
		.cond = {
			.prev_scene = VSYS_SWITCH_SCENE_NORMAL | VSYS_SWITCH_SCENE_PENDING,
			.next_scene = VSYS_SWITCH_SCENE_BOOST,
		},
		.handler = dischg_boost_switch_to_boost,
	},
};

static inline int dischg_boost_get_scene_by_soc(
	const struct dischg_boost_config *cfg, int soc)
{
	if (soc <= cfg->switch_boost_soc)
		return VSYS_SWITCH_SCENE_BOOST;
	else if (soc > cfg->switch_normal_soc)
		return VSYS_SWITCH_SCENE_NORMAL;
	else
		return VSYS_SWITCH_SCENE_PENDING;
}

static inline bool dischg_boost_match_scene(int scene, int scenes)
{
	return !!(scene & scenes);
}

static inline bool dischg_boost_match_cond(const struct switch_cond *cur,
	const struct switch_cond *item)
{
	return dischg_boost_match_scene(cur->prev_scene, item->prev_scene) &&
		dischg_boost_match_scene(cur->next_scene, item->next_scene);
}

static inline switch_handler dischg_boost_get_switch_handler(const struct switch_cond *cur_cond)
{
	size_t i;
	size_t size = ARRAY_SIZE(handler_map);

	for (i = 0; i < size; ++i) {
		if (dischg_boost_match_cond(cur_cond, &handler_map[i].cond))
			return handler_map[i].handler;
	}
	return NULL;
}

static inline bool dischg_boost_has_switch_handler(const struct switch_cond *cur_cond)
{
	return !!dischg_boost_get_switch_handler(cur_cond);
}

#ifdef CONFIG_HUAWEI_POWER_DEBUG
static ssize_t dischg_boost_dbg_get_scene_store(void *dev_data,
	const char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;
	int soc = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	if (kstrtoint(buf, 0, &soc) < 0) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	hwlog_info("soc=%d, scene=%d\n", soc, dischg_boost_get_scene_by_soc(&di->dischg_cfg, soc));
	return size;
}

static ssize_t dischg_boost_dbg_exec_handler_store(void *dev_data,
	const char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;
	int prev_soc = 0;
	int next_soc = 0;
	struct switch_cond cond = { 0 };
	switch_handler handler = NULL;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	/* 2: two parameters */
	if (sscanf(buf, "%d %d", &prev_soc, &next_soc) != 2) {
		hwlog_err("buf does not contain two integers\n");
		return -EINVAL;
	}

	cond.prev_scene = dischg_boost_get_scene_by_soc(&di->dischg_cfg, prev_soc);
	cond.next_scene = dischg_boost_get_scene_by_soc(&di->dischg_cfg, next_soc);
	hwlog_info("prev_soc=%d, prev_scene=%d, next_soc=%d, next_scene=%d\n",
		prev_soc, cond.prev_scene, next_soc, cond.next_scene);

	handler = dischg_boost_get_switch_handler(&cond);
	if (!handler) {
		hwlog_info("handler is null\n");
		return -EINVAL;
	}
	handler(di);
	return size;
}

static ssize_t dischg_boost_dbg_dischg_soc_cfg_show(void *dev_data,
	char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	return scnprintf(buf, size, "boost_soc=%d, normal_soc=%d\n",
		di->dischg_cfg.switch_boost_soc,
		di->dischg_cfg.switch_normal_soc);
}

static ssize_t dischg_boost_dbg_dischg_soc_cfg_store(void *dev_data,
	const char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;
	int boost_soc = 0;
	int normal_soc = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	/* 2: two parameters */
	if (sscanf(buf, "%d %d", &boost_soc, &normal_soc) != 2) {
		hwlog_err("buf does not contain two integers\n");
		return -EINVAL;
	}

	di->dischg_cfg.switch_boost_soc = boost_soc;
	di->dischg_cfg.switch_normal_soc = normal_soc;
	hwlog_info("boost_soc=%d, normal_soc=%d\n", boost_soc, normal_soc);
	return size;
}

static ssize_t dischg_boost_dbg_switch_delay_ms_show(void *dev_data,
	char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	return scnprintf(buf, size, "boost_delay_ms=%d, normal_delay_ms=%d\n",
		di->dischg_cfg.switch_boost_delay_ms,
		di->dischg_cfg.switch_normal_delay_ms);
}

static ssize_t dischg_boost_dbg_switch_delay_ms_store(void *dev_data,
	const char *buf, size_t size)
{
	struct dischg_boost_dev *di = dev_data;
	int boost_delay_ms = 0;
	int normal_delay_ms = 0;

	if (!buf || !di) {
		hwlog_err("buf or di is null\n");
		return -EINVAL;
	}

	/* 2: two parameters */
	if (sscanf(buf, "%d %d", &boost_delay_ms, &normal_delay_ms) != 2) {
		hwlog_err("buf does not contain two integers\n");
		return -EINVAL;
	}

	di->dischg_cfg.switch_boost_delay_ms = boost_delay_ms;
	di->dischg_cfg.switch_normal_delay_ms = normal_delay_ms;
	hwlog_info("boost_delay_ms=%d, normal_delay_ms=%d\n",
		boost_delay_ms, normal_delay_ms);
	return size;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

static void dischg_boost_parse_discharge_dts(struct dischg_boost_dev *di)
{
	struct device_node *np = di->dev->of_node;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_boost_soc", &di->dischg_cfg.switch_boost_soc, 20); // 20: 20% soc
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_normal_soc", &di->dischg_cfg.switch_normal_soc, 25); // 25: 25% soc
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_boost_delay_ms", &di->dischg_cfg.switch_boost_delay_ms, 1); // 1: 1ms
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switch_normal_delay_ms", &di->dischg_cfg.switch_normal_delay_ms, 4); // 4: 4ms
}

static int dischg_boost_init_gpio(struct dischg_boost_dev *di)
{
	int ret = 0;

	ret += power_gpio_config_output(di->dev->of_node, "normal_gpio",
		"dischg_normal_gpio", &di->dischg_cfg.normal_gpio, 1);
	ret += power_gpio_config_output(di->dev->of_node, "boost_gpio",
		"dischg_boost_gpio", &di->dischg_cfg.boost_gpio, 0);

	return ret;
}

static inline int dischg_boost_get_current_scene(const struct dischg_boost_config *dischg_cfg)
{
	int cur_soc = 0;

	(void)power_supply_get_int_property_value(PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &cur_soc);

	return dischg_boost_get_scene_by_soc(dischg_cfg, cur_soc);
}

static void dischg_boost_work(struct work_struct *work)
{
	int current_scene;
	int previous_scene;
	struct dischg_boost_dev *di = NULL;
	switch_handler handler = NULL;
	struct switch_cond cur_cond = { 0 };

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct dischg_boost_dev, switch_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	previous_scene = atomic_read(&di->prev_scene);
	current_scene = dischg_boost_get_current_scene(&di->dischg_cfg);
	if (current_scene == previous_scene) {
		hwlog_err("scene:%d has not been changed\n", current_scene);
		return;
	}

	cur_cond.next_scene = current_scene;
	cur_cond.prev_scene = previous_scene;
	handler = dischg_boost_get_switch_handler(&cur_cond);
	if (!handler) {
		hwlog_err("handler is null\n");
		return;
	}

	hwlog_info("dischg_boost_work execute handler\n");
	handler(di);
	atomic_set(&di->prev_scene, current_scene);
}

static void dischg_boost_trigger_switch(struct dischg_boost_dev *di)
{
	int cur_scene;
	struct switch_cond cur_cond = { 0 };
	int prev_scene = atomic_read(&di->prev_scene);

	cur_scene = dischg_boost_get_current_scene(&di->dischg_cfg);
	if (prev_scene == cur_scene)
		return;

	cur_cond.next_scene = cur_scene;
	cur_cond.prev_scene = prev_scene;

	if (!dischg_boost_has_switch_handler(&cur_cond))
		return;

	if (work_busy(&di->switch_work)) {
		hwlog_info("switch work was just triggered, ignore\n");
		return;
	}

	hwlog_info("trigger switch work\n");
	schedule_work(&di->switch_work);
}

static int dischg_boost_psy_change_cb(
	struct notifier_block *nb, unsigned long event, void *data)
{
	struct dischg_boost_dev *di = NULL;
	struct power_supply *psy = data;

	if (!nb || !data) {
		hwlog_err("nb or data is null\n");
		return NOTIFY_OK;
	}

	di = container_of(nb, struct dischg_boost_dev, soc_change_nb);
	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	if (strcmp(psy->desc->name, PSY_NAME)) {
		hwlog_info("psy %s is not target, ignore\n", PSY_NAME);
		return NOTIFY_OK;
	}

	dischg_boost_trigger_switch(di);
	return NOTIFY_OK;
}

static inline void dischg_boost_register_power_debug(struct dischg_boost_dev *di)
{
#ifdef CONFIG_HUAWEI_POWER_DEBUG
	power_dbg_ops_register("dischg_boost", "get_scene", di,
		NULL,
		dischg_boost_dbg_get_scene_store);
	power_dbg_ops_register("dischg_boost", "exec_handler", di,
		NULL,
		dischg_boost_dbg_exec_handler_store);
	power_dbg_ops_register("dischg_boost", "dischg_soc_cfg", di,
		dischg_boost_dbg_dischg_soc_cfg_show,
		dischg_boost_dbg_dischg_soc_cfg_store);
	power_dbg_ops_register("dischg_boost", "switch_delay_ms", di,
		dischg_boost_dbg_switch_delay_ms_show,
		dischg_boost_dbg_switch_delay_ms_store);
#endif /* CONFIG_HUAWEI_POWER_DEBUG */
}

static int dischg_boost_probe(struct platform_device *pdev)
{
	int ret;
	struct dischg_boost_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	INIT_WORK(&di->switch_work, dischg_boost_work);
	dischg_boost_parse_discharge_dts(di);
	ret = dischg_boost_init_gpio(di);
	if (ret) {
		hwlog_err("init gpio failed, ret:%d\n", ret);
		goto free_mem;
	}

	di->soc_change_nb.notifier_call = dischg_boost_psy_change_cb;
	ret = power_supply_reg_notifier(&di->soc_change_nb);
	if (ret) {
		hwlog_err("register soc_change_nb failed, ret:%d\n", ret);
		goto free_mem;
	}
	dischg_boost_register_power_debug(di);
	di->wakelock = power_wakeup_source_register(di->dev, "dischg_boost_wakelock");
	// we assume that previous scene is NORMAL, and the gpios will be inited as NORMAL scene
	atomic_set(&di->prev_scene, VSYS_SWITCH_SCENE_NORMAL);
	dischg_boost_trigger_switch(di);

	platform_set_drvdata(pdev, di);
	return 0;

free_mem:
	devm_kfree(&pdev->dev, di);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int dischg_boost_remove(struct platform_device *pdev)
{
	struct dischg_boost_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di NULL\n");
		return -ENODEV;
	}
	power_supply_unreg_notifier(&di->soc_change_nb);
	return 0;
}

static void dischg_boost_shutdown(struct platform_device *pdev)
{
	struct dischg_boost_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di NULL\n");
		return;
	}
	power_supply_unreg_notifier(&di->soc_change_nb);
}

#ifdef CONFIG_PM
static int dischg_boost_resume(struct platform_device *pdev)
{
	struct dischg_boost_dev *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di NULL\n");
		return -ENODEV;
	}
	dischg_boost_trigger_switch(di);
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id dischg_boost_match_table[] = {
	{
		.compatible = "huawei,dischg_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver dischg_boost_driver = {
	.probe = dischg_boost_probe,
	.remove = dischg_boost_remove,
#ifdef CONFIG_PM
	.resume = dischg_boost_resume,
#endif /* CONFIG_PM */
	.shutdown = dischg_boost_shutdown,
	.driver = {
		.name = "huawei,dischg_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dischg_boost_match_table),
	},
};

static int __init dischg_boost_init(void)
{
	return platform_driver_register(&dischg_boost_driver);
}

static void __exit dischg_boost_exit(void)
{
	platform_driver_unregister(&dischg_boost_driver);
}

late_initcall(dischg_boost_init);
module_exit(dischg_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("discharge boost module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
