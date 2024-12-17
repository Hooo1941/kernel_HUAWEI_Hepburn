// SPDX-License-Identifier: GPL-2.0
/*
 * reverse_charge_pd.c
 *
 * pd reverse charge driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#include <securec.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_debug.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/reverse_charge/reverse_charge_pd.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>

#define HWLOG_TAG reverse_charge_pd
HWLOG_REGIST();

#define OTG_CAP_MAX_SIZE                6
#define MONITRO_WORK_DELAY_TIME         5000
#define OTG_WORK_DELAY_TIME             300
#define OTG_DEFAULT_VOLTAGE             5000
#define OTG_DEFAULT_CURRENT             1500
#define TEMP_HIGH_DEFUALT_TH            45
#define OTG_CAP_BUFF_LEN                64
#define OTG_CAP_BUFF_MAX_LEN            256

enum reverse_charge_state {
	REVERSE_CHARGE_IDLE = 0,
	REVERSE_CHARGE_NORMAL,
	REVERSE_CHARGE_HIGH_VOL,
};

enum reverse_charge_src_cap {
	RC_CAP_TH = 0,
	RC_OUTPUT_VOL,
	RC_OUTPUT_CUR,
	RC_SRC_CAP_END
};

struct reverse_charge_otg_cap {
	u32 cap_th;
	u32 otg_vol;
	u32 otg_cur;
};

struct rchg_pd_dev {
	struct device *dev;
	struct delayed_work rchg_pd_otg_work;
	struct delayed_work rchg_pd_monitor_work;
	struct notifier_block connect_nb;
	struct mutex rchg_pd_lock;
	struct mutex plugged_lock;
	struct otg_cap current_cap;
	struct reverse_charge_otg_cap rchg_otg_cap[OTG_CAP_MAX_SIZE];
	int rchg_pd_state;
	bool wls_plugged_state;
	u32 temp_th;
};

static struct rchg_pd_ic_ops *g_rchg_pd_ic_ops;
static struct rchg_pd_dev *g_rchg_pd_dev;

static int rchg_pd_enable_ic(bool enable)
{
	struct rchg_pd_ic_ops *ops = g_rchg_pd_ic_ops;

	if (!ops || !ops->enable_reverse_charge)
		return -EPERM;

	return ops->enable_reverse_charge(enable, ops->dev_data);
}

int rchg_pd_ops_register(struct rchg_pd_ic_ops *ops)
{
	if (!ops || !ops->dev_name) {
		hwlog_err("%s ops or dev_name is null\n", __func__);
		return -EINVAL;
	}

	g_rchg_pd_ic_ops = ops;
	hwlog_info("%s register ok\n", __func__);
	return 0;
}

static int rchg_pd_9v_start(struct rchg_pd_dev *di)
{
	int ret;

	hwlog_info("%s +++\n", __func__);
	if (di->wls_plugged_state || wireless_tx_get_tx_open_flag()) {
		hwlog_info("%s wls trx is open, can not open 9v reverse charge\n", __func__);
		return -EPERM;
	}
	ret = rchg_pd_enable_ic(true);
	if (!ret) {
		di->rchg_pd_state = REVERSE_CHARGE_HIGH_VOL;
		schedule_delayed_work(&di->rchg_pd_monitor_work,
			msecs_to_jiffies(MONITRO_WORK_DELAY_TIME));
	}

	hwlog_info("%s --- ret=%d\n", __func__, ret);
	return ret;
}

static int rchg_pd_9v_stop(struct rchg_pd_dev *di)
{
	int ret;

	hwlog_info("%s +++\n", __func__);
	ret = rchg_pd_enable_ic(false);
	di->rchg_pd_state = REVERSE_CHARGE_NORMAL;
	cancel_delayed_work(&di->rchg_pd_monitor_work);
	hwlog_info("%s --- ret=%d\n", __func__, ret);
	return ret;
}

int rchg_pd_9v_enable(bool enable)
{
	int ret;
	struct rchg_pd_dev *di = g_rchg_pd_dev;

	if (!di)
		return -EPERM;

	mutex_lock(&di->rchg_pd_lock);
	if (enable) {
		if (di->rchg_pd_state == REVERSE_CHARGE_HIGH_VOL) {
			hwlog_info("already open 9v, not need open again\n");
			mutex_unlock(&di->rchg_pd_lock);
			return 0;
		}
		ret = rchg_pd_9v_start(di);
	} else {
		ret = rchg_pd_9v_stop(di);
	}
	mutex_unlock(&di->rchg_pd_lock);

	return ret;
}

static void rchg_pd_otg_start(struct rchg_pd_dev *di)
{
	hwlog_info("%s +++\n", __func__);
	if (di->rchg_pd_state == REVERSE_CHARGE_NORMAL) {
		hwlog_info("otg already start, ignore\n");
		return;
	}

	di->rchg_pd_state = REVERSE_CHARGE_NORMAL;
	schedule_delayed_work(&di->rchg_pd_otg_work, msecs_to_jiffies(OTG_WORK_DELAY_TIME));
	hwlog_info("%s ---\n", __func__);
}

static void rchg_pd_otg_stop(struct rchg_pd_dev *di)
{
	hwlog_info("%s +++\n", __func__);
	cancel_delayed_work(&di->rchg_pd_otg_work);
	if (di->rchg_pd_state == REVERSE_CHARGE_HIGH_VOL)
		rchg_pd_9v_enable(false);
	
	di->rchg_pd_state = REVERSE_CHARGE_IDLE;
	hwlog_info("%s ---\n", __func__);
}

void rchg_pd_otg_enable(bool enable)
{
	struct rchg_pd_dev *di = g_rchg_pd_dev;

	if (!di)
		return;

	if (enable)
		rchg_pd_otg_start(di);
	else
		rchg_pd_otg_stop(di);
}

static void rchg_pd_set_wls_plugged_state(bool state, struct rchg_pd_dev *di)
{
	mutex_lock(&di->plugged_lock);
	di->wls_plugged_state = state;
	mutex_unlock(&di->plugged_lock);
	hwlog_info("%s state=%d\n", __func__, state);
}

static int rchg_pd_connect_cb(struct notifier_block *nb, unsigned long event, void *data)
{
	struct otg_cap cap = { OTG_DEFAULT_VOLTAGE, OTG_DEFAULT_CURRENT };
	struct rchg_pd_dev *di = g_rchg_pd_dev;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_WIRELESS_CONNECT:
		rchg_pd_set_wls_plugged_state(true, di);
		if (di->rchg_pd_state == REVERSE_CHARGE_HIGH_VOL) {
			rchg_pd_9v_enable(false);
			power_event_bnc_notify(POWER_BNT_REVERSE_CHARGE,
				POWER_NE_REVERSE_CHARGE_CAP, &cap);
		}
		break;
	case POWER_NE_WIRELESS_DISCONNECT:
		rchg_pd_set_wls_plugged_state(false, di);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static bool rchg_pd_otg_cap_is_changed(struct otg_cap cap1, struct otg_cap cap2)
{
	return ((cap1.otg_vol != cap2.otg_vol) || (cap1.otg_cur != cap2.otg_cur));
}

static int rchg_pd_get_otg_cap(struct otg_cap *cap, struct rchg_pd_dev *di)
{
	int i;
	int temp = 0;
	int soc = 0;

	if (!cap || !di)
		return -EINVAL;

	if (di->wls_plugged_state || wireless_tx_get_tx_open_flag()) {
		hwlog_info("%s wireless trx is open, only support normal otg\n", __func__);
		cap->otg_vol = OTG_DEFAULT_VOLTAGE;
		cap->otg_cur = OTG_DEFAULT_CURRENT;
		return 0;
	}

	power_supply_get_int_prop(POWER_PLATFORM_BAT_PSY_NAME, POWER_SUPPLY_PROP_TEMP, &temp,
		POWER_SUPPLY_DEFAULT_TEMP / POWER_SUPPLY_BAT_TEMP_UNIT, POWER_SUPPLY_BAT_TEMP_UNIT);
	hwlog_info("%s temp=%d\n", __func__, temp);
	if (temp > di->temp_th) {
		cap->otg_vol = OTG_DEFAULT_VOLTAGE;
		cap->otg_cur = OTG_DEFAULT_CURRENT;
		return 0;
	}

	(void)power_supply_get_int_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_CAPACITY, &soc);
	hwlog_info("%s soc=%d\n", __func__, soc);
	for (i = 0; i < OTG_CAP_MAX_SIZE; i++) {
		if (soc >= di->rchg_otg_cap[i].cap_th) {
			cap->otg_vol = di->rchg_otg_cap[i].otg_vol;
			cap->otg_cur = di->rchg_otg_cap[i].otg_cur;
			break;
		}
	}

	hwlog_info("%s otg_vol=%d, otg_cur=%d\n", __func__, cap->otg_vol, cap->otg_cur);
	return 0;
}

static void rchg_pd_otg_work(struct work_struct *work)
{
	struct otg_cap cap = { OTG_DEFAULT_VOLTAGE, OTG_DEFAULT_CURRENT };
	struct rchg_pd_dev *di = container_of(work, struct rchg_pd_dev, rchg_pd_otg_work.work);

	if (!di)
		return;

	hwlog_info("%s +++\n", __func__);
	rchg_pd_get_otg_cap(&cap, di);
	if (cap.otg_vol == OTG_DEFAULT_VOLTAGE) {
		hwlog_info("%s not support 9v now, exit\n", __func__);
		return;
	}
	di->current_cap.otg_vol = cap.otg_vol;
	di->current_cap.otg_cur = cap.otg_cur;
	power_event_bnc_notify(POWER_BNT_REVERSE_CHARGE, POWER_NE_REVERSE_CHARGE_CAP, &cap);
	hwlog_info("%s ---\n", __func__);
}

static void rchg_pd_monitor_work(struct work_struct *work)
{
	struct otg_cap cap = { 0 };
	struct rchg_pd_dev *di = container_of(work, struct rchg_pd_dev, rchg_pd_monitor_work.work);

	if (!di) {
		hwlog_err("%s: di is null\n", __func__);
		return;
	}

	hwlog_info("%s +++\n", __func__);
	if (di->rchg_pd_state != REVERSE_CHARGE_HIGH_VOL) {
		hwlog_info("not in high vol reverse charge, exit\n");
		return;
	}

	rchg_pd_get_otg_cap(&cap, di);
	if (rchg_pd_otg_cap_is_changed(cap, di->current_cap)) {
		di->current_cap.otg_vol = cap.otg_vol;
		di->current_cap.otg_cur = cap.otg_cur;
		power_event_bnc_notify(POWER_BNT_REVERSE_CHARGE, POWER_NE_REVERSE_CHARGE_CAP, &cap);
	}

	schedule_delayed_work(&di->rchg_pd_monitor_work, msecs_to_jiffies(MONITRO_WORK_DELAY_TIME));
	hwlog_info("%s ---\n", __func__);
}

static int rchg_pd_parse_otg_cap(struct device_node *node, struct rchg_pd_dev *di)
{
	int i, len, row, col;
	u32 data;

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), node,
		"otg_cap", OTG_CAP_MAX_SIZE, RC_SRC_CAP_END);
	if (len <= 0) {
		hwlog_err("%s otg cap get failed\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		if (power_dts_read_u32_index(power_dts_tag(HWLOG_TAG),
			node, "otg_cap", i, &data))
			return -EINVAL;

		row = i / RC_SRC_CAP_END;
		col = i % RC_SRC_CAP_END;
		switch (col) {
		case RC_CAP_TH:
			di->rchg_otg_cap[row].cap_th = data;
			break;
		case RC_OUTPUT_VOL:
			di->rchg_otg_cap[row].otg_vol = data;
			break;
		case RC_OUTPUT_CUR:
			di->rchg_otg_cap[row].otg_cur = data;
			break;
		default:
			break;
		}
	}

	for (i = 0; i < len / RC_SRC_CAP_END; i++)
		hwlog_info("otg_cap[%d] %d %d %d\n", i, di->rchg_otg_cap[i].cap_th,
			di->rchg_otg_cap[i].otg_vol, di->rchg_otg_cap[i].otg_cur);

	return 0;
}

static int rchg_pd_parse_dts(struct device_node *np, struct rchg_pd_dev *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"temp_th", &di->temp_th, TEMP_HIGH_DEFUALT_TH);

	return rchg_pd_parse_otg_cap(np, di);
}

static ssize_t rchg_pd_dbg_temp_th_show(void *dev_data, char *buf, size_t size)
{
	struct rchg_pd_dev *di = (struct rchg_pd_dev *)dev_data;

	if (!di)
		return 0;

	return scnprintf(buf, size, "temp_th=%d\n", di->temp_th);
}

static ssize_t rchg_pd_dbg_temp_th_store(void *dev_data, const char *buf, size_t size)
{
	int val = 0;
	struct rchg_pd_dev *di = (struct rchg_pd_dev *)dev_data;

	if (!di)
		return -EINVAL;

	if (kstrtoint(buf, 0, &val)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	di->temp_th = val;
	hwlog_info("%s temp_th=%d\n", __func__, val);
	return size;
}

static ssize_t rchg_pd_dbg_otg_cap_show(void *dev_data, char *buf, size_t size)
{
	int i;
	char tmp_buf[OTG_CAP_BUFF_LEN] = { 0 };
	char out_buf[OTG_CAP_BUFF_MAX_LEN] = { 0 };
	struct rchg_pd_dev *di = (struct rchg_pd_dev *)dev_data;

	if (!di)
		return 0;

	for (i = 0; i < OTG_CAP_MAX_SIZE; i++) {
		memset_s(tmp_buf, OTG_CAP_BUFF_LEN, 0, OTG_CAP_BUFF_LEN);
		(void)snprintf_s(tmp_buf, OTG_CAP_BUFF_LEN, OTG_CAP_BUFF_LEN - 1, "%d %d %d\n",
			di->rchg_otg_cap[i].cap_th, di->rchg_otg_cap[i].otg_vol,
			di->rchg_otg_cap[i].otg_cur);
		if (strncat_s(out_buf, OTG_CAP_BUFF_MAX_LEN - 1, tmp_buf, strlen(tmp_buf)) != EOK)
			return -EINVAL;
	}

	return scnprintf(buf, size, "otg cap:\n%s\n", out_buf);
}

static ssize_t rchg_pd_dbg_otg_cap_store(void *dev_data, const char *buf, size_t size)
{
	int i;
	int index = 0;
	char *next = NULL;
	char *source = NULL;
	char buff[OTG_CAP_BUFF_MAX_LEN] = { 0 };
	unsigned int data[RC_SRC_CAP_END * OTG_CAP_MAX_SIZE] = { 0 };
	struct rchg_pd_dev *di = (struct rchg_pd_dev *)dev_data;

	if (!di)
		return -EINVAL;

	if (snprintf_s(buff, OTG_CAP_BUFF_MAX_LEN, OTG_CAP_BUFF_MAX_LEN - 1, "%s", buf) < 0)
		return -EINVAL;

	source = &buff[0];
	while ((next = strsep(&source, " "))) {
		hwlog_info("%s next=%s source=%s\n", __func__, next, source);
		if (sscanf_s(next, "%u", &data[index]) != 1) {
			hwlog_err("invalid parameters: %s\n", next);
			return -EINVAL;
		}
		if (++index >= RC_SRC_CAP_END * OTG_CAP_MAX_SIZE) {
			hwlog_info("index over size, break\n");
			break;
		}
		if (!source) {
			hwlog_info("tmp_buff is null, break\n");
			break;
		}
	}

	for (i = 0; i < OTG_CAP_MAX_SIZE; i++) {
		di->rchg_otg_cap[i].cap_th = data[i * RC_SRC_CAP_END + RC_CAP_TH];
		di->rchg_otg_cap[i].otg_vol = data[i * RC_SRC_CAP_END + RC_OUTPUT_VOL];
		di->rchg_otg_cap[i].otg_cur = data[i * RC_SRC_CAP_END + RC_OUTPUT_CUR];
		hwlog_info("otg_cap[%d] %d %d %d\n", i, di->rchg_otg_cap[i].cap_th,
			di->rchg_otg_cap[i].otg_vol, di->rchg_otg_cap[i].otg_cur);
	}

	return size;
}

static void rchg_pd_register_debugfs(struct rchg_pd_dev *di)
{
	power_dbg_ops_register("reverse_charge", "temp_th", di,
		rchg_pd_dbg_temp_th_show, rchg_pd_dbg_temp_th_store);
	power_dbg_ops_register("reverse_charge", "otg_cap", di,
		rchg_pd_dbg_otg_cap_show, rchg_pd_dbg_otg_cap_store);
}

static int reverse_charge_pd_probe(struct platform_device *pdev)
{
	int ret;
	struct rchg_pd_dev *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	ret = rchg_pd_parse_dts(np, di);
	if (ret) {
		hwlog_err("parse dts failed, ret=%d\n", ret);
		goto free_mem;
	}
	
	di->rchg_pd_state = REVERSE_CHARGE_IDLE;
	mutex_init(&di->rchg_pd_lock);
	mutex_init(&di->plugged_lock);
	INIT_DELAYED_WORK(&di->rchg_pd_otg_work, rchg_pd_otg_work);
	INIT_DELAYED_WORK(&di->rchg_pd_monitor_work, rchg_pd_monitor_work);

	di->connect_nb.notifier_call = rchg_pd_connect_cb;
	ret = power_event_bnc_register(POWER_BNT_CONNECT, &di->connect_nb);
	if (ret) {
		hwlog_err("register connect_nb failed, ret=%d\n", ret);
		goto free_mem1;
	}

	rchg_pd_register_debugfs(di);
	platform_set_drvdata(pdev, di);
	g_rchg_pd_dev = di;
	return 0;

free_mem1:
	mutex_destroy(&di->rchg_pd_lock);
	mutex_destroy(&di->plugged_lock);
free_mem:
	kfree(di);
	return ret;
}

static int reverse_charge_pd_remove(struct platform_device *pdev)
{
	struct rchg_pd_dev *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	cancel_delayed_work(&di->rchg_pd_otg_work);
	cancel_delayed_work(&di->rchg_pd_monitor_work);
	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->connect_nb);
	mutex_destroy(&di->rchg_pd_lock);
	mutex_destroy(&di->plugged_lock);
	kfree(di);
	g_rchg_pd_dev = NULL;
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id reverse_charge_pd_match_table[] = {
	{
		.compatible = "huawei,reverse_charge_pd",
		.data = NULL,
	},
	{},
};

static struct platform_driver reverse_charge_pd_driver = {
	.probe = reverse_charge_pd_probe,
	.remove = reverse_charge_pd_remove,
	.driver = {
		.name = "huawei,reverse_charge_pd",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(reverse_charge_pd_match_table),
	},
};

static int __init reverse_charge_pd_init(void)
{
	return platform_driver_register(&reverse_charge_pd_driver);
}

static void __exit reverse_charge_pd_exit(void)
{
	platform_driver_unregister(&reverse_charge_pd_driver);
}

module_init(reverse_charge_pd_init);
module_exit(reverse_charge_pd_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pd reverse charge driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
