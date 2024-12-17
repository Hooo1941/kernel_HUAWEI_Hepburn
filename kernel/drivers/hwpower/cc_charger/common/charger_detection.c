/*
 * charger_detection.c
 *
 * charger_detection module
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_icon.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/common_module/power_wakeup.h>
#include <chipset_common/hwpower/charger/charger_detection.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>

#define HWLOG_TAG charger_detection
HWLOG_REGIST();

#define MAX_EVENT_COUNT                     16
#define EVENT_QUEUE_UNIT                    MAX_EVENT_COUNT
#define CHECK_SUCC                          1
#define CHECK_FAIL                          0

struct charger_event_queue {
	enum power_supply_sinksource_type *event;
	unsigned int num_event;
	unsigned int max_event;
	unsigned int enpos, depos;
	unsigned int overlay, overlay_index;
};

struct charger_detection_info {
	struct device *dev;
	struct power_supply *detection_psy;
	struct work_struct event_work;
	spinlock_t event_spin_lock;
	enum power_supply_sinksource_type event;
	enum power_supply_sinksource_type last_event;
	struct charger_event_queue event_queue;
	int bc12_chg_type;
	struct charger_deteciton_ops *ops;
};

static struct charger_detection_info *g_detection_di;
/* X-axis show new event, Y-axis show current event,the result determine queue event work */
static int charge_event_check_map[POWER_SUPPLY_MAX_CHARGER_EVENT][POWER_SUPPLY_MAX_CHARGER_EVENT] = {
	{CHECK_FAIL, CHECK_SUCC, CHECK_FAIL, CHECK_SUCC, CHECK_SUCC, CHECK_SUCC},
	{CHECK_SUCC, CHECK_FAIL, CHECK_FAIL, CHECK_FAIL, CHECK_FAIL, CHECK_FAIL},
	{CHECK_FAIL, CHECK_SUCC, CHECK_FAIL, CHECK_SUCC, CHECK_SUCC, CHECK_SUCC},
	{CHECK_FAIL, CHECK_FAIL, CHECK_SUCC, CHECK_FAIL, CHECK_SUCC, CHECK_SUCC},
	{CHECK_FAIL, CHECK_SUCC, CHECK_SUCC, CHECK_SUCC, CHECK_FAIL, CHECK_SUCC},
	{CHECK_FAIL, CHECK_FAIL, CHECK_SUCC, CHECK_SUCC, CHECK_SUCC, CHECK_FAIL},
};

static const char *const charger_sourcesink_type_name[] = {
	[POWER_SUPPLY_START_SINK]          = "START_SINK",
	[POWER_SUPPLY_STOP_SINK]           = "STOP_SINK",
	[POWER_SUPPLY_START_SOURCE]        = "START_SOURCE",
	[POWER_SUPPLY_STOP_SOURCE]         = "STOP_SOURCE",
	[POWER_SUPPLY_START_SINK_WIRELESS] = "START_SINK_WIRELESS",
	[POWER_SUPPLY_STOP_SINK_WIRELESS]  = "STOP_SINK_WIRELESS",
	[POWER_SUPPLY_MAX_CHARGER_EVENT]   = "CHARGER_MAX_EVENT",
};

enum power_supply_sinksource_type charge_detecion_get_sinksource_type(void)
{
	if (!g_detection_di) {
		hwlog_err("g_detection_di is null\n");
		return POWER_SUPPLY_MAX_CHARGER_EVENT;
	}

	return g_detection_di->event;
}

const char *charger_detection_get_sourcesink_type_name(void)
{
	if (!g_detection_di) {
		hwlog_err("g_detection_di is null\n");
		return "null point err";
	}

	if ((g_detection_di->event < POWER_SUPPLY_START_SINK)
		|| (g_detection_di->event > POWER_SUPPLY_MAX_CHARGER_EVENT))
		return "illegal event";

	return charger_sourcesink_type_name[g_detection_di->event];
}

int charger_detecion_ops_register(struct charger_deteciton_ops *ops)
{
	if (!ops || !g_detection_di) {
		hwlog_err("ops or di is null\n");
		return -EPERM;
	}

	g_detection_di->ops = ops;
	hwlog_info("charge detection ops register ok\n");
	return 0;
}

void charger_detecion_ops_unregister(void)
{
	if (!g_detection_di) {
		hwlog_err("di is null\n");
		return;
	}

	g_detection_di->ops = NULL;
	hwlog_info("charge detection ops unregister ok\n");
}

static struct charger_deteciton_ops *charge_detecion_get_ops(void)
{
	return g_detection_di->ops;
}

static int charger_event_queue_create(struct charger_event_queue *event_queue, unsigned int count)
{
	count = (count >= MAX_EVENT_COUNT ? MAX_EVENT_COUNT : count);
	event_queue->max_event = count;
	event_queue->num_event = (count >= EVENT_QUEUE_UNIT ? EVENT_QUEUE_UNIT : count);

	event_queue->event = kzalloc(event_queue->num_event * sizeof(int), GFP_KERNEL);
	if (!event_queue->event) {
		hwlog_err("malloc event_queue failed\n");
		return -ENOMEM;
	}

	event_queue->enpos = 0;
	event_queue->depos = 0;
	event_queue->overlay = 0;
	event_queue->overlay_index = 0;

	return 0;
}

static void charger_event_queue_destroy(struct charger_event_queue *event_queue)
{
	kfree(event_queue->event);
	event_queue->event = NULL;
	event_queue->enpos = 0;
	event_queue->depos = 0;
	event_queue->num_event = 0;
	event_queue->max_event = 0;
	event_queue->overlay = 0;
	event_queue->overlay_index = 0;
}

static int charger_event_queue_isfull(struct charger_event_queue *event_queue)
{
	return (((event_queue->enpos + 1) % event_queue->num_event) == (event_queue->depos));
}

static int charger_event_queue_isempty(struct charger_event_queue *event_queue)
{
	return (event_queue->enpos == event_queue->depos);
}

static void charger_event_queue_set_overlay(struct charger_event_queue *event_queue)
{
	if (event_queue->overlay)
		return;
	event_queue->overlay = 1;
	event_queue->overlay_index = event_queue->enpos;
}

static void charger_event_queue_clear_overlay(struct charger_event_queue *event_queue)
{
	event_queue->overlay = 0;
	event_queue->overlay_index = 0;
}

static int charger_event_enqueue(struct charger_event_queue *event_queue,
	enum power_supply_sinksource_type event)
{
	/* no need verify argument, isfull will check it */
	if (charger_event_queue_isfull(event_queue)) {
		hwlog_err("event queue full\n");
		return -ENOSPC;
	}

	if (event_queue->overlay) {
		if (event_queue->overlay_index == event_queue->enpos)
			event_queue->enpos = ((event_queue->enpos + 1) % event_queue->num_event);

		if (charger_event_queue_isempty(event_queue)) {
			hwlog_info("queue is empty, just enqueue\n");
			event_queue->overlay_index = ((event_queue->overlay_index + 1) %
				event_queue->num_event);
			event_queue->enpos = ((event_queue->enpos + 1) % event_queue->num_event);
			event_queue->overlay = 0;
		}

		event_queue->event[event_queue->overlay_index] = event;
	} else {
		event_queue->event[event_queue->enpos] = event;
		event_queue->enpos = ((event_queue->enpos + 1) % event_queue->num_event);
	}

	return 0;
}

static enum power_supply_sinksource_type charger_event_dequeue(struct charger_event_queue *event_queue)
{
	enum power_supply_sinksource_type event;

	/* no need verify argument, isempty will check it */
	if (charger_event_queue_isempty(event_queue))
		return POWER_SUPPLY_MAX_CHARGER_EVENT;

	event = event_queue->event[event_queue->depos];
	event_queue->depos = ((event_queue->depos + 1) % event_queue->num_event);

	return event;
}

static int charger_event_check(struct charger_detection_info *di, enum power_supply_sinksource_type new_event)
{
	if ((new_event < POWER_SUPPLY_START_SINK) || (new_event >= POWER_SUPPLY_MAX_CHARGER_EVENT))
		return CHECK_FAIL;
	if (di->event == POWER_SUPPLY_MAX_CHARGER_EVENT)
		return CHECK_SUCC;
	return charge_event_check_map[new_event][di->event];
}

static void charger_detection_handle_start_sink_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_start_sink_event)
		return;

	c_ops->handle_start_sink_event();
}

static void charger_detection_handle_stop_sink_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_stop_sink_event)
		return;

	c_ops->handle_stop_sink_event();
}

static void charger_detection_handle_start_wireless_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_start_wireless_event)
		return;

	c_ops->handle_start_wireless_event();
}

static void charger_detection_handle_stop_wireless_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_stop_wireless_event)
		return;

	c_ops->handle_stop_wireless_event();
}

static void charger_detection_handle_start_source_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_start_source_event)
		return;

	c_ops->handle_start_source_event();
}

static void charger_detection_handle_stop_source_event(void)
{
	struct charger_deteciton_ops *c_ops = charge_detecion_get_ops();

	if (!c_ops || !c_ops->handle_stop_source_event)
		return;

	c_ops->handle_stop_source_event();
}

static void charger_handle_event(struct charger_detection_info *di, enum power_supply_sinksource_type event)
{
	switch (event) {
	case POWER_SUPPLY_START_SINK:
		charger_detection_handle_start_sink_event();
		break;
	case POWER_SUPPLY_START_SINK_WIRELESS:
		charger_detection_handle_start_wireless_event();
		break;
	case POWER_SUPPLY_STOP_SINK:
		charger_detection_handle_stop_sink_event();
		break;
	case POWER_SUPPLY_STOP_SINK_WIRELESS:
		charger_detection_handle_stop_wireless_event();
		break;
	case POWER_SUPPLY_START_SOURCE:
		charger_detection_handle_start_source_event();
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_INSERT, NULL);
		break;
	case POWER_SUPPLY_STOP_SOURCE:
		charger_detection_handle_stop_source_event();
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_REMOVE, NULL);
		break;
	default:
		hwlog_err("error event = %d\n", event);
		break;
	}
}

static void charger_event_work(struct work_struct *work)
{
	unsigned long flags;
	enum power_supply_sinksource_type event;
	struct charger_detection_info *di = container_of(work,
		struct charger_detection_info,
		event_work);

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	hwlog_info("%s+\n", __func__);

	if ((di->last_event == POWER_SUPPLY_START_SINK_WIRELESS) &&
		(di->event == POWER_SUPPLY_START_SINK))
		power_event_report_str_uevent("BMS_EVT=WLS2WIRED");

	while (!charger_event_queue_isempty(&di->event_queue)) {
		spin_lock_irqsave(&(di->event_spin_lock), flags);
		event = charger_event_dequeue(&di->event_queue);
		spin_unlock_irqrestore(&(di->event_spin_lock), flags);

		charger_handle_event(di, event);
	}

	spin_lock_irqsave(&(di->event_spin_lock), flags);
	charger_event_queue_clear_overlay(&di->event_queue);
	spin_unlock_irqrestore(&(di->event_spin_lock), flags);
	hwlog_info("%s-\n", __func__);
}

static void charger_source_sink_event(struct charger_detection_info *di, enum power_supply_sinksource_type event)
{
	unsigned long flags;

	if (power_platform_support_pmic_detection())
		return;

	if (!di->ops) {
		hwlog_err("ops null, do nothing\n");
		return;
	}

	if (!charger_event_check(di, event)) {
		hwlog_err("last event: [%s], event [%s] was rejected\n",
			charger_sourcesink_type_name[di->event],
			charger_sourcesink_type_name[event]);
		return;
	}

	spin_lock_irqsave(&(di->event_spin_lock), flags);

	if ((event == POWER_SUPPLY_START_SINK) ||
		(event == POWER_SUPPLY_START_SINK_WIRELESS))
		charge_set_charger_online(1);
	else
		charge_set_charger_online(0);
	di->last_event = di->event;
	di->event = event;
	hwlog_info("case = %s\n", charger_sourcesink_type_name[event]);

	if (!charger_event_enqueue(&di->event_queue, event)) {
		if (!queue_work(system_power_efficient_wq, &di->event_work))
			hwlog_err("schedule event_work wait = %d\n", event);
	} else {
		hwlog_err("can't enqueue event = %d\n", event);
	}

	if ((event == POWER_SUPPLY_STOP_SOURCE) || (event == POWER_SUPPLY_STOP_SINK) ||
		(event == POWER_SUPPLY_STOP_SINK_WIRELESS))
		charger_event_queue_set_overlay(&di->event_queue);

	spin_unlock_irqrestore(&(di->event_spin_lock), flags);
}

static int charger_detection_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	enum power_supply_sinksource_type event;
	struct charger_detection_info *di = g_detection_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	hwlog_info("prop = %d, val->intval = %d\n", psp, val->intval);

	switch (psp) {
	case POWER_SUPPLY_PROP_CHG_PLUGIN:
		hwlog_info("POWER_SUPPLY_PROP_CHG_PLUGIN\n");
		event = (enum power_supply_sinksource_type)val->intval;
		charger_source_sink_event(di, event);
		return 0;
	case POWER_SUPPLY_PROP_CHG_TYPE: /* bc12 result */
		return 0;
	default:
		return 0;
	}
}

static int charger_detection_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct charger_detection_info *di = g_detection_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -ENODEV;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_CHG_TYPE: /* bc12 result */
		val->intval = di->bc12_chg_type;
		break;
	default:
		return 0;
	}

	return 0;
}

static int charger_detection_property_is_writeable(struct power_supply *psy,
	enum power_supply_property prop)
{
	return 0;
}

static enum power_supply_property power_supply_charger_detection_props[] = {
	POWER_SUPPLY_PROP_CHG_PLUGIN,
	POWER_SUPPLY_PROP_CHG_TYPE,
};

static struct power_supply_desc g_charger_detection_psy_desc = {
	.name = "charger_detection",
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.properties = power_supply_charger_detection_props,
	.num_properties = ARRAY_SIZE(power_supply_charger_detection_props),
	.get_property = charger_detection_get_property,
	.set_property = charger_detection_set_property,
	.property_is_writeable = charger_detection_property_is_writeable,
};

static int charger_detection_init_psy(struct charger_detection_info *di)
{
	di->detection_psy = power_supply_register(di->dev,
		&g_charger_detection_psy_desc, NULL);
	if (IS_ERR(di->detection_psy))
		return -EPERM;

	return 0;
}

static int charger_detection_probe(struct platform_device *pdev)
{
	struct charger_detection_info *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->event = POWER_SUPPLY_MAX_CHARGER_EVENT;
	di->last_event = di->event;

	if (charger_detection_init_psy(di))
		goto fail_register_psy;

	if (charger_event_queue_create(&di->event_queue, MAX_EVENT_COUNT))
		goto fail_create_event_queue;
	spin_lock_init(&di->event_spin_lock);
	INIT_WORK(&di->event_work, charger_event_work);
	platform_set_drvdata(pdev, di);
	g_detection_di = di;

	hwlog_info("charger detection init ok\n");
	return 0;

fail_create_event_queue:
	charger_event_queue_destroy(&di->event_queue);
fail_register_psy:
	if (di->detection_psy)
		power_supply_unregister(di->detection_psy);
	g_detection_di = NULL;
	kfree(di);

	return -1;
}

static int charger_detection_remove(struct platform_device *pdev)
{
	struct charger_detection_info *di = g_detection_di;

	if (!di)
		return -ENODEV;

	if (di->detection_psy)
		power_supply_unregister(di->detection_psy);
	platform_set_drvdata(pdev, NULL);
	g_detection_di = NULL;
	kfree(di);

	return 0;
}

static const struct of_device_id charger_detection_match_table[] = {
	{
		.compatible = "huawei,charger_detection",
		.data = NULL,
	},
	{},
};

static struct platform_driver charger_detection_driver = {
	.probe = charger_detection_probe,
	.remove = charger_detection_remove,
	.driver = {
		.name = "huawei,charger_detection",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charger_detection_match_table),
	},
};

static int __init charger_detection_init(void)
{
	return platform_driver_register(&charger_detection_driver);
}

static void __exit charger_detection_exit(void)
{
	platform_driver_unregister(&charger_detection_driver);
}

module_init(charger_detection_init);
module_exit(charger_detection_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huawei charger module driver");
MODULE_AUTHOR("HUAWEI Inc");
