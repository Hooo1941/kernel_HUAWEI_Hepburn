/*
 * huawei_pen_clk.c
 *
 * Used to provide a clock signal to the stylus.
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "huawei_thp.h"
#include "huawei_pen_clk.h"

static struct thp_pen_clk *pen_clk = NULL;
#define DEFAULT_CLK_FREQ 19200000
#define MS_TO_S 1000
#define MIN_CALIBRATION_TIME 60 /* 60s */

#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
static int thp_sts_clk_ctrl(uint32_t status)
{
	sts_clk_out_t clk_op;
	int rc;
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);

	thp_request_vote_of_polymeric_chip(cd);
	mutex_lock(&pen_clk->clk_mutex);
	clk_op.op_id = status;
	rc = sts_clk_out_sys(&clk_op);
	if (rc < 0) {
		thp_log_err("%s fail, rc = %d\n", __func__, rc);
		mutex_unlock(&pen_clk->clk_mutex);
		thp_release_vote_of_polymeric_chip(cd);
		return -EINVAL;
	}
	mutex_unlock(&pen_clk->clk_mutex);
	thp_release_vote_of_polymeric_chip(cd);
	if (status == STS_SUB_CMD_OPEN) {
		atomic_set(&pen_clk->status, CLK_ENABLE);
		thp_log_info("%s:enable clk success\n", __func__);
	} else {
		atomic_set(&pen_clk->status, CLK_DISABLE);
		thp_log_info("%s:disable clk success\n", __func__);
	}
	return 0;
}
#endif

int thp_pen_clk_enable(void)
{
	struct pinctrl_state *state = NULL;
	int rc;

	thp_log_info("%s:enter\n", __func__);
	if (!pen_clk) {
		thp_log_info("%s:pen_clk is null\n", __func__);
		return 0;
	}
	if (atomic_read(&pen_clk->status) != CLK_DISABLE) {
		thp_log_err("%s: invalid status\n", __func__);
		return 0;
	}
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	if (pen_clk->clk_source == CLK_FROM_POLYMERIC_CHIP)
		return thp_sts_clk_ctrl(STS_SUB_CMD_OPEN);
#endif
	state = pinctrl_lookup_state(pen_clk->clk_ctrl, "pwm_enable");
	if (IS_ERR(state)) {
		thp_log_err("%s: pinctrl lookup state fail\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&pen_clk->clk_mutex);
	rc = pinctrl_select_state(pen_clk->clk_ctrl, state);
	if (rc) {
		thp_log_err("%s: pinctrl select state fail\n");
		mutex_unlock(&pen_clk->clk_mutex);
		return rc;
	}

	rc = clk_set_rate(pen_clk->clk, (unsigned long)pen_clk->clk_freq);
	if (rc) {
		thp_log_err("%s: set clk rate to %u fail\n", __func__,
			pen_clk->clk_freq);
		mutex_unlock(&pen_clk->clk_mutex);
		return rc;
	}
	rc = clk_prepare_enable(pen_clk->clk);
	if (rc) {
		thp_log_err("%s: clk prepare and enable fail.\n", __func__);
		mutex_unlock(&pen_clk->clk_mutex);
		return rc;
	}
	mutex_unlock(&pen_clk->clk_mutex);
	atomic_set(&pen_clk->status, CLK_ENABLE);
	thp_log_info("%s:success\n", __func__);
	return 0;
}
EXPORT_SYMBOL(thp_pen_clk_enable);

int thp_pen_clk_disable(void)
{
	struct pinctrl_state *state = NULL;
	int rc;

	thp_log_info("%s:enter\n", __func__);

	if (!pen_clk) {
		thp_log_info("%s:pen_clk is null\n", __func__);
		return 0;
	}
	if (atomic_read(&pen_clk->status) != CLK_ENABLE) {
		thp_log_err("%s: invalid status\n", __func__);
		return 0;
	}
#ifdef CONFIG_CONTEXTHUB_IO_DIE_STS
	if (pen_clk->clk_source == CLK_FROM_POLYMERIC_CHIP)
		return thp_sts_clk_ctrl(STS_SUB_CMD_CLOSE);
#endif
	mutex_lock(&pen_clk->clk_mutex);
	state = pinctrl_lookup_state(pen_clk->clk_ctrl, "pwm_disable");
	if (IS_ERR(state)) {
		thp_log_err("%s: pinctrl lookup state fail.\n", __func__);
		mutex_unlock(&pen_clk->clk_mutex);
		return -EINVAL;
	}
	rc = pinctrl_select_state(pen_clk->clk_ctrl, state);
	if (rc) {
		thp_log_err("%s: pinctrl select state fail\n");
		mutex_unlock(&pen_clk->clk_mutex);
		return rc;
	}

	clk_disable_unprepare(pen_clk->clk);
	mutex_unlock(&pen_clk->clk_mutex);
	atomic_set(&pen_clk->status, CLK_DISABLE);
	thp_log_info("%s:success\n", __func__);
	return 0;
}
EXPORT_SYMBOL(thp_pen_clk_disable);

static int thp_pen_clk_init(struct platform_device *pdev, struct thp_pen_clk *clk_data)
{
	struct device_node *pen_clk_node = NULL;
	int rc;

	pen_clk_node = of_find_compatible_node(NULL, NULL, "huawei,pen_clk");
	if (!pen_clk_node) {
		thp_log_info("find_compatible_node huawei,pen_clk error\n");
		return -EINVAL;
	}
	rc = of_property_read_string(pen_clk_node, "clock-names",
		&clk_data->clk_name);
	thp_log_info("%s:clk_name = %s\n", __func__, clk_data->clk_name);

	rc = of_property_read_u32(pen_clk_node, "clock-freq",
		&clk_data->clk_freq);
	if (rc < 0)
		clk_data->clk_freq = DEFAULT_CLK_FREQ;
	thp_log_info("%s:clk_freq = %d\n", __func__, clk_data->clk_freq);

	rc = of_property_read_u32(pen_clk_node, "clock-source",
		&clk_data->clk_source);
	if (rc < 0)
		clk_data->clk_source = CLK_FROM_SOC;
	thp_log_info("%s:clk_source = %d\n", __func__, clk_data->clk_source);

	/* pen clrl clk init */
	if (clk_data->clk_source == CLK_FROM_SOC) {
		clk_data->clk = devm_clk_get(&pdev->dev, clk_data->clk_name);
		if (IS_ERR(clk_data->clk)) {
			thp_log_err("%s: get clk fail\n", __func__);
			return -EINVAL;
		}

		clk_data->clk_ctrl = devm_pinctrl_get(&pdev->dev);
		if (!clk_data->clk_ctrl) {
			thp_log_err("%s: get clk_ctrl failed\n", __func__);
			return -EINVAL;
		}
	}
	atomic_set(&clk_data->status, CLK_DISABLE);
	thp_log_info("%s:init pen clk success\n", __func__);
	return 0;
}

static int _pen_clk_calibration(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int rc;
	unsigned int flag;
	u32 i;

	rc = thp_pen_clk_enable();
	if (rc) {
		thp_log_err("%s: enable clk failed\n", __func__);
		return -EINVAL;
	}
	msleep(10); /* Wait for the clock signal to stabilize */
	for (i = 0; i <= cd->dual_thp; i++) {
		cd = thp_get_core_data(i);
		flag = (!cd || !cd->thp_dev || !cd->thp_dev->ops ||
			!cd->thp_dev->ops->clk_calibration);
		if (flag) {
			thp_log_err("%s:input is null\n", __func__);
			continue;
		}
		rc = cd->thp_dev->ops->clk_calibration(cd->thp_dev);
		if (rc) {
			thp_log_err("%s: enable clk failed\n", __func__);
			goto err;
		}
	}
	msleep(100); /* delay 100ms to wait clk calibration finish */
err:
	rc = thp_pen_clk_disable();
	if (rc) {
		thp_log_err("%s: disable clk failed\n", __func__);
		return -EINVAL;
	}
	thp_log_info("%s: out\n", __func__);
	return 0;
}

void pen_clk_calibration(void)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	int rc;
	unsigned int stylus3_status;

	thp_log_info("%s:enter\n", __func__);
	if (!cd || !pen_clk) {
		thp_log_err("%s:cd is null\n", __func__);
		return;
	}
	stylus3_status = thp_get_status(THP_STATUS_STYLUS3, cd->panel_id);
	if (!stylus3_status) {
		thp_log_err("%s:pen is not be connectted\n", __func__);
		return;
	}
	__pm_stay_awake(pen_clk->pen_wake_lock);
	rc = _pen_clk_calibration();
	start_pen_clk_timer();
	if (rc) {
		thp_log_err("%s:_pen_clk_calibration rc = %d\n", __func__, rc);
		__pm_relax(pen_clk->pen_wake_lock);
		goto err;
	}
	pen_clk->last_ts = ktime_get_boottime();
	thp_log_info("%s:clk calibration at %ld ms\n", __func__,
		ktime_to_ms(pen_clk->last_ts));
	__pm_relax(pen_clk->pen_wake_lock);
	return;
err:
	thp_log_info("%s:out\n", __func__);
}

static void pen_clk_calibration_work_fn(struct work_struct *work)
{
	thp_log_info("%s:enter\n", __func__);
	pen_clk_calibration();
}

void start_pen_clk_timer(void)
{
	unsigned int wakeup_ms = 300 * 1000; /* 5min */
	ktime_t now;
	ktime_t add;
	int rc;

	if (!pen_clk) {
		thp_log_info("%s:pen_clk is null\n", __func__);
		return;
	}
	now = ktime_get_boottime();
	add = ktime_set(wakeup_ms / MSEC_PER_SEC, (wakeup_ms % MSEC_PER_SEC) * NSEC_PER_MSEC);
	mutex_lock(&pen_clk->timer_mutex);
	rc = alarm_cancel(&pen_clk->alarm);
	alarm_start(&pen_clk->alarm, ktime_add(now, add));
	thp_log_info("%s:start new timer, rc = %d\n", __func__, rc);
	mutex_unlock(&pen_clk->timer_mutex);
}
EXPORT_SYMBOL(start_pen_clk_timer);

void stop_pen_clk_timer(void)
{
	int rc;

	if (!pen_clk) {
		thp_log_info("%s:pen_clk is null\n", __func__);
		return;
	}
	mutex_lock(&pen_clk->timer_mutex);
	rc = alarm_cancel(&pen_clk->alarm);
	mutex_unlock(&pen_clk->timer_mutex);
	thp_log_info("%s:alarm_cancel, rc = %d\n", __func__, rc);
}

DECLARE_WORK(pen_clk_calibration_work, pen_clk_calibration_work_fn);

static int pen_clk_resume(struct device *dev)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	unsigned int stylus3_status;
	long delta;
	bool flag = false;
	u32 i;

	for (i = 0; i <= cd->dual_thp; i++)
		flag = flag || !thp_get_core_data(i)->suspended;
	if (flag) {
		thp_log_err("%s: tp in resume state\n", __func__);
		return 0;
	}
	stylus3_status = thp_get_status(THP_STATUS_STYLUS3, cd->panel_id);
	if (!stylus3_status) {
		thp_log_info("%s:pen is not be enable,return\n", __func__);
		return 0;
	}

	pen_clk->cur_ts = ktime_get_boottime();
	thp_log_info("%s:at %ld ms\n", __func__, ktime_to_ms(pen_clk->cur_ts));
	/*
	 * The last calibration until now takes less than 1 minutes,
	 * then the calibration will not be performed during this wake-up
	 */
	delta = (ktime_to_ms(pen_clk->cur_ts) - ktime_to_ms(pen_clk->last_ts)) / MS_TO_S;
	if (delta > MIN_CALIBRATION_TIME) {
		thp_log_info("%s:wakeup pen_clk_calibration_work\n", __func__);
		schedule_work(&pen_clk_calibration_work);
	}
	thp_log_info("%s:out\n", __func__);
	return 0;
}

static int pen_clk_suspend(struct device *dev)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	unsigned int stylus3_status;

	if (!cd) {
		thp_log_err("%s:cd is null\n", __func__);
		return 0;
	}

	thp_log_info("%s:enter\n", __func__);
	stylus3_status = thp_get_status(THP_STATUS_STYLUS3, cd->panel_id);
	if (!stylus3_status) {
		thp_log_info("%s:pen is not be enable,return\n", __func__);
		return 0;
	}
	start_pen_clk_timer();
	thp_log_err("%s:out\n", __func__);
	return 0;
}

static enum alarmtimer_restart pen_clk_alarm_handle_timer(
	struct alarm *alarm, ktime_t now)
{
	struct thp_core_data *cd = thp_get_core_data(MAIN_TOUCH_PANEL);
	bool flag = false;
	u32 i;

	thp_log_info("%s:in\n", __func__);
	for (i = 0; i <= cd->dual_thp; i++)
		flag = flag || !thp_get_core_data(i)->suspended;
	if (flag) {
		thp_log_err("%s:tp work in resume status\n", __func__);
		return ALARMTIMER_NORESTART;
	}

	schedule_work(&pen_clk_calibration_work);
	thp_log_info("%s:pen clk timer, wakeup pen_clk_calibration_work\n", __func__);
	return ALARMTIMER_NORESTART;
}

static int pen_clk_probe(struct platform_device *pdev)
{
	int ret;
	struct thp_pen_clk *pen_clk_data = NULL;

	thp_log_info("%s:enter\n", __func__);
	pen_clk_data = devm_kzalloc(&pdev->dev, sizeof(*pen_clk_data), GFP_KERNEL);
	if (!pen_clk_data) {
		thp_log_info("%s:no memory\n", __func__);
		return -ENOMEM;
	}

	ret = thp_pen_clk_init(pdev, pen_clk_data);
	if (ret) {
		thp_log_err("%s: init clk source fail\n", __func__);
		goto err;
	}
	atomic_set(&pen_clk_data->status, CLK_UNINIT);
	mutex_init(&pen_clk_data->clk_mutex);
	mutex_init(&pen_clk_data->timer_mutex);

	pen_clk_data->last_ts = ktime_get_boottime();
	thp_log_info("%s:last_ts = %ld ms\n", __func__, ktime_to_ms(pen_clk_data->last_ts));
	pen_clk_data->pen_wake_lock = wakeup_source_register(&pdev->dev, "pen_wake_lock");
	if (!pen_clk_data->pen_wake_lock) {
		thp_log_err("%s: failed to init wakelock\n", __func__);
		goto err;
	}

	alarm_init(&pen_clk_data->alarm, ALARM_BOOTTIME, pen_clk_alarm_handle_timer);
	atomic_set(&pen_clk_data->status, CLK_DISABLE);
	pen_clk = pen_clk_data;
	thp_log_info("%s:success\n", __func__);
	return 0;
err:
	devm_kfree(&pdev->dev, pen_clk_data);
	pen_clk = NULL;
	return 0;
}

static const struct dev_pm_ops pen_clk_pm_ops = {
	.prepare = pen_clk_suspend,
	.resume = pen_clk_resume,
};

static const struct of_device_id pen_clk_match_table[] = {
	{
		.compatible = "huawei,pen_clk",
		.data = NULL,
	},
	{},
};

static struct platform_driver pen_clk_driver = {
	.probe = pen_clk_probe,
	.driver = {
		.name = "pen-pwm",
		.owner = THIS_MODULE,
		.pm = &pen_clk_pm_ops,
		.of_match_table = of_match_ptr(pen_clk_match_table),
	},
};

int pen_clk_init(void)
{
	thp_log_info("%s:in\n", __func__);
	return platform_driver_register(&pen_clk_driver);
}

void pen_clk_exit(void)
{
	thp_log_info("%s:in\n", __func__);
	platform_driver_unregister(&pen_clk_driver);
}
