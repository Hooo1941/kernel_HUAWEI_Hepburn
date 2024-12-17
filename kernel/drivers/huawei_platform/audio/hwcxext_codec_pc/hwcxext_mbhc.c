/*
 * hwcxext_mbhc.c
 *
 * hwcxext mbhc driver
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/pm_wakeirq.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/input/matrix_keypad.h>
#include <sound/jack.h>

#include "hwcxext_codec_info.h"
#include "hwcxext_mbhc.h"
#include "hwcxext_log.h"

#define HWLOG_TAG hwcxext_mbhc
HWLOG_REGIST();

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define HS_LONG_BTN_PRESS_LIMIT_TIME 6000
#define HS_REPORT_TABLE_SIZE 0x08

#define HWCXEXT_MBHC_JACK_BUTTON_MASK (SND_JACK_BTN_0 | SND_JACK_BTN_1 | SND_JACK_BTN_2)

struct jack_key_to_type {
	enum snd_jack_types type;
	int key;
};

enum {
	MICB_DISABLE,
	MICB_ENABLE,
};

static void hwcxext_mbhc_plug_in_detect(
	struct hwcxext_mbhc_priv *mbhc_data, int pulg);

static int hwcxext_mbhc_cancel_long_btn_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;

	/* it can not use cancel_delayed_work_sync, otherwise lead deadlock */
	ret = cancel_delayed_work(&mbhc_data->long_press_btn_work);
	if (ret)
		hwlog_info("%s: cancel long btn work\n", __func__);

	return ret;
}

static int hwcxext_mbhc_check_headset_in(void *priv, int timeouts)
{
	struct hwcxext_mbhc_priv *mbhc_data = priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is null", __func__);
		return AUDIO_JACK_NONE;
	}

	if (mbhc_data->mbhc_cb->mbhc_check_headset_in)
		return mbhc_data->mbhc_cb->mbhc_check_headset_in(mbhc_data, timeouts);

	return AUDIO_JACK_NONE;
}

static void hwcxext_mbhc_micbias_control(struct hwcxext_mbhc_priv *mbhc_data,
	int req)
{
	int micbias_vote_cnt;

	if (mbhc_data->mbhc_cb->enable_micbias == NULL)
		return;

	mutex_lock(&mbhc_data->micbias_vote_mutex);
	micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
	hwlog_info("%s: req:%s, micbias_vote_cnt:%d\n",
		__func__, req == MICB_ENABLE ? "micbias enable" :
		"micbias disable", micbias_vote_cnt);
	if (req == MICB_ENABLE) {
		if (micbias_vote_cnt)
			goto mbhc_micbias_control_exit;

		hwlog_info("%s: enable micbias", __func__);
		atomic_inc(&mbhc_data->micbias_vote_cnt);
		mbhc_data->mbhc_cb->enable_micbias(mbhc_data);
	} else {
		atomic_dec(&mbhc_data->micbias_vote_cnt);
		micbias_vote_cnt = atomic_read(&mbhc_data->micbias_vote_cnt);
		hwlog_info("%s: disable micbias micbias_vote_cnt:%d",
			__func__, micbias_vote_cnt);
		if (micbias_vote_cnt < 0)
			atomic_set(&mbhc_data->micbias_vote_cnt, 0);
		mbhc_data->mbhc_cb->disable_micbias(mbhc_data);
	}

mbhc_micbias_control_exit:
	hwlog_info("%s: out, micbias_vote_cnt:%d\n", __func__,
		atomic_read(&mbhc_data->micbias_vote_cnt));
	mutex_unlock(&mbhc_data->micbias_vote_mutex);
}

static void hwcxext_mbhc_long_press_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data = container_of(work,
		struct hwcxext_mbhc_priv, long_press_btn_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is null\n", __func__);
		return;
	}

	__pm_stay_awake(mbhc_data->long_btn_wake_lock);
	IN_FUNCTION;
	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it not headset, ignore\n", __func__);
		goto long_press_btn_exit;
	}

	if (!mbhc_data->btn_report) {
		hwlog_warn("%s: btn_report:%d is not right\n", __func__,
			mbhc_data->btn_report);
		goto long_press_btn_exit;
	}

	mutex_lock(&mbhc_data->btn_mutex);
	mutex_lock(&mbhc_data->status_mutex);
	mbhc_data->btn_report = 0;
	snd_soc_jack_report(&mbhc_data->headset_jack,
		mbhc_data->btn_report, HWCXEXT_MBHC_JACK_BUTTON_MASK);
	hwlog_info("%s: long_press_btn, report 0\n", __func__);
	mutex_unlock(&mbhc_data->status_mutex);
	mutex_unlock(&mbhc_data->btn_mutex);

long_press_btn_exit:
	__pm_relax(mbhc_data->long_btn_wake_lock);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_long_press_btn_trigger(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	queue_delayed_work(mbhc_data->long_press_btn_wq,
		&mbhc_data->long_press_btn_work,
		msecs_to_jiffies(HS_LONG_BTN_PRESS_LIMIT_TIME));
}

static void hwcxext_mbhc_btn_handle(struct hwcxext_mbhc_priv *mbhc_data,
	int report, int status)
{
	const int report_table[HS_REPORT_TABLE_SIZE] = {
		0, SND_JACK_BTN_0, SND_JACK_BTN_1, SND_JACK_BTN_0,
		SND_JACK_BTN_2, SND_JACK_BTN_0, SND_JACK_BTN_1, SND_JACK_BTN_0,
	};
	mutex_lock(&mbhc_data->status_mutex);
	hwlog_info("%s: report is = %d\n", __func__, report);
	mbhc_data->btn_report = report_table[report & (HS_REPORT_TABLE_SIZE - 1)];
	hwlog_info("%s: btn_report is = %d\n", __func__, mbhc_data->btn_report);

	if (status & BTN_PRESS) {
		hwlog_info("%s: btn is pressed\n", __func__);
		snd_soc_jack_report(&mbhc_data->headset_jack,
			mbhc_data->btn_report, HWCXEXT_MBHC_JACK_BUTTON_MASK);
		if (status & BTN_RELEASED) {
			mutex_unlock(&mbhc_data->status_mutex);
			mdelay(50);
			mutex_lock(&mbhc_data->status_mutex);
		} else {
			hwcxext_mbhc_long_press_btn_trigger(mbhc_data);
		}
	}

	if (status & BTN_RELEASED) {
		hwcxext_mbhc_cancel_long_btn_work(mbhc_data);
		mbhc_data->btn_report = 0;
		snd_soc_jack_report(&mbhc_data->headset_jack,
			0, HWCXEXT_MBHC_JACK_BUTTON_MASK);
		hwlog_info("%s: btn is release\n", __func__);
	}
	mutex_unlock(&mbhc_data->status_mutex);
}

static void hwcxext_mbhc_btn_detect(struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	mutex_lock(&mbhc_data->btn_mutex);

	int pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data, 0);
	if (pulg_in != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: plug out happens\n", __func__);
		goto btn_detect_exit;
	}

	if (mbhc_data->hs_status != AUDIO_JACK_HEADSET) {
		hwlog_info("%s: it need further detect\n", __func__);
		msleep(20);
		hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
		hwcxext_mbhc_plug_in_detect(mbhc_data, pulg_in);
		goto btn_detect_exit;
	}

	if (mbhc_data->mbhc_cb->get_btn_type_recognize) {
		int report = 0;
		int status = 0;
		if (mbhc_data->mbhc_cb->get_btn_type_recognize(mbhc_data, &report, &status) == 0) {
			hwcxext_mbhc_btn_handle(mbhc_data, report, status);
		} else {
			hwlog_err("%s:get btn type error\n", __func__);
			pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data, 0);
			if (pulg_in != AUDIO_JACK_NONE && pulg_in != AUDIO_JACK_PLUG_OUT)
				hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
			hwcxext_mbhc_plug_in_detect(mbhc_data, pulg_in);
		}
	}

btn_detect_exit:
	mutex_unlock(&mbhc_data->btn_mutex);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data = container_of(work,
		struct hwcxext_mbhc_priv, btn_delay_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is null\n", __func__);
		return;
	}
	__pm_stay_awake(mbhc_data->btn_wake_lock);

	hwcxext_mbhc_btn_detect(mbhc_data);

	if (mbhc_data->mbhc_cb->clear_irq)
		mbhc_data->mbhc_cb->clear_irq(mbhc_data);

	__pm_relax(mbhc_data->btn_wake_lock);
	OUT_FUNCTION;
}

static void hwcxext_mbhc_jack_report(struct hwcxext_mbhc_priv *mbhc_data,
	enum audio_jack_states type)
{
#ifdef CONFIG_SWITCH
	enum audio_jack_states jack_status = type;
#endif
	int jack_report = 0;
	bool headset = true;
	bool rear = true;
	hwlog_info("%s: enter current_plug:%d new_plug:%d\n",
		__func__, mbhc_data->hs_status, type);
	if (mbhc_data->hs_status == type) {
		hwlog_info("%s: type already reported, exit\n", __func__);
		return;
	}

	mbhc_data->hs_status = type;
	switch (mbhc_data->hs_status) {
	case AUDIO_JACK_NONE:
	case AUDIO_JACK_PLUG_OUT:
		jack_report = 0;
		hwlog_info("%s : plug out\n", __func__);
		break;
	case AUDIO_JACK_HEADSET:
		jack_report = SND_JACK_HEADSET;
		snd_soc_jack_report(&mbhc_data->headset_jack, SND_JACK_MICROPHONE,
			SND_JACK_HEADSET);
		rear = false;
		hwlog_info("%s : 4-pole headset plug in\n", __func__);
		break;
	case AUDIO_JACK_HEADPHONE:
		jack_report = SND_JACK_HEADPHONE;
		rear = false;
		hwlog_info("%s : 3-pole headphone plug in\n", __func__);
		break;
	case AUDIO_JACK_LINEOUT:
		jack_report = SND_JACK_HEADPHONE;
		headset = false;
		hwlog_info("%s : lineout headphone plug in\n", __func__);
		break;
	case AUDIO_JACK_LINEIN:
		jack_report = SND_JACK_LINEIN;
		headset = false;
		hwlog_info("%s : linein plug in\n", __func__);
		break;
	case AUDIO_JACK_MIC:
		jack_report = SND_JACK_MICROPHONE;
		headset = false;
		hwlog_info("%s : mic plug in\n", __func__);
		break;
	default:
		hwlog_err("%s : error hs_status:%d\n", __func__,
			mbhc_data->hs_status);
		break;
	}
	/* report jack status */
	if (rear)
		snd_soc_jack_report(&mbhc_data->rear_jack, jack_report,
			SND_JACK_HEADSET | SND_JACK_LINEIN);
	if (headset)
		snd_soc_jack_report(&mbhc_data->headset_jack, jack_report,
			SND_JACK_HEADSET);

#ifdef CONFIG_SWITCH
	switch_set_state(&mbhc_data->sdev, jack_status);
#endif
}

static void hwcxext_report_hs_status(struct hwcxext_mbhc_priv *mbhc_data,
	enum audio_jack_states type)
{
	hwcxext_mbhc_jack_report(mbhc_data, type);
}

static void hwcxext_mbhc_plug_in_detect(
	struct hwcxext_mbhc_priv *mbhc_data, int pulg)
{
	IN_FUNCTION;
	bool pulg_state = true;
	hwcxext_mbhc_cancel_long_btn_work(mbhc_data);

	mutex_lock(&mbhc_data->plug_mutex);
	/* get voltage by read sar in mbhc */
	if (mbhc_data->mbhc_cb->get_hs_type_recognize == NULL) {
		hwlog_err("%s: get_hs_type_recognize is NULL\n", __func__);
		goto in_detect_for_hs_plguout_exit;
	}

	if (pulg == AUDIO_JACK_NONE || pulg == AUDIO_JACK_PLUG_OUT) {
		hwlog_info("%s: plug out happens\n", __func__);
		if (mbhc_data->hs_status != AUDIO_JACK_NONE)
			hwcxext_report_hs_status(mbhc_data, AUDIO_JACK_NONE);
		goto in_detect_for_hs_plguout_exit;
	}
	if (pulg == AUDIO_JACK_HEADSET) {
		mutex_lock(&mbhc_data->status_mutex);
		pulg = mbhc_data->mbhc_cb->get_hs_type_recognize(mbhc_data);
		mutex_unlock(&mbhc_data->status_mutex);
	}
	pulg_state = false;
	hwcxext_report_hs_status(mbhc_data, (enum audio_jack_states)pulg);
in_detect_for_hs_plguout_exit:
	mutex_unlock(&mbhc_data->plug_mutex);
	if (pulg_state)
		hwcxext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
	OUT_FUNCTION;
}

void hwcxext_mbhc_plug_out_detect(struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;
	hwcxext_mbhc_cancel_long_btn_work(mbhc_data);
	mutex_lock(&mbhc_data->plug_mutex);
	mutex_lock(&mbhc_data->status_mutex);
	if (mbhc_data->btn_report) {
		hwlog_info("%s: release of button press\n", __func__);
		snd_soc_jack_report(&mbhc_data->headset_jack,
			0, HWCXEXT_MBHC_JACK_BUTTON_MASK);
		mbhc_data->btn_report = 0;
	}
	mutex_unlock(&mbhc_data->status_mutex);
	hwcxext_report_hs_status(mbhc_data, AUDIO_JACK_NONE);
	mutex_unlock(&mbhc_data->plug_mutex);

	hwcxext_mbhc_micbias_control(mbhc_data, MICB_DISABLE);
}

/* work of headset insertion and removal recognize */
static void hwcxext_mbhc_plug_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data =
		container_of(work, struct hwcxext_mbhc_priv,
		hs_plug_work.work);
	IN_FUNCTION;
	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	if (mbhc_data->mbhc_cb->set_hs_detec_restart == NULL) {
		hwlog_err("%s: set hs detecre start is NULL\n", __func__);
		return;
	}

	if ((mbhc_data->init_state == false) && (mbhc_data->mbhc_cb->clear_irq)) {
		hwlog_warn("%s: init_state is false\n", __func__);
		mbhc_data->mbhc_cb->clear_irq(mbhc_data);
		return;
	}
	__pm_stay_awake(mbhc_data->plug_wake_lock);
	int pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data, 0);
	hwlog_info("%s: pulg_status = %d \n", __func__, pulg_in);
	if (pulg_in == AUDIO_JACK_NONE) {
		hwlog_err("%s: pulg_in is AUDIO_JACK_NONE error\n", __func__);
	} else if (pulg_in == AUDIO_JACK_HEADSET_NOUSE) {
		mbhc_data->need_reapeat = true;
		hwlog_info("%s: pulg_in is AUDIO_JACK_HEADSET_NOUSE\n", __func__);
	} else if (pulg_in == AUDIO_JACK_PLUG_OUT) {
		hwlog_info("%s: the headset is :plugout\n", __func__);
		/* Jack removed, or spurious IRQ */
		mbhc_data->need_reapeat = false;
		hwcxext_mbhc_plug_out_detect(mbhc_data);
	} else {
		if ((pulg_in == AUDIO_JACK_HEADSET) && (mbhc_data->need_reapeat)) {
			hwlog_info("%s: the headset is :plugin and need_reapeat\n", __func__);
			mbhc_data->mbhc_cb->set_hs_detec_restart(mbhc_data);
			mbhc_data->need_reapeat = false;
		}
		hwlog_info("%s: the headset is :plugin\n", __func__);
		hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
		/* Jack inserted, determine type */
		hwcxext_mbhc_plug_in_detect(mbhc_data, pulg_in);
	}

	if (mbhc_data->mbhc_cb->clear_irq)
		mbhc_data->mbhc_cb->clear_irq(mbhc_data);
	__pm_relax(mbhc_data->plug_wake_lock);
	OUT_FUNCTION;
}

/* work of the irq share with hs plugin or plugout and btn press & release */
static void hwcxext_mbhc_irq_plug_btn_work(struct work_struct *work)
{
	struct hwcxext_mbhc_priv *mbhc_data =
		container_of(work, struct hwcxext_mbhc_priv,
		irq_plug_btn_work.work);

	if (IS_ERR_OR_NULL(mbhc_data)) {
		hwlog_err("%s: mbhc_data is error\n", __func__);
		return;
	}

	__pm_wakeup_event(mbhc_data->plug_btn_wake_lock, 1000);
	IN_FUNCTION;
	int pulg_in = hwcxext_mbhc_check_headset_in(mbhc_data, 500);
	hwlog_info("%s: hs_status = %d, pulg_status = %d \n", __func__,
		mbhc_data->hs_status, pulg_in);
	if (pulg_in == AUDIO_JACK_NONE) {
		hwlog_err("%s: pulg_in is AUDIO_JACK_NONE error\n", __func__);
		return;
	}
	if ((mbhc_data->hs_status == AUDIO_JACK_HEADSET) &&
		(pulg_in == AUDIO_JACK_HEADSET)) {  /* btn irq */
		hwlog_info("%s: queue btn_delay_work btn\n", __func__);
		queue_delayed_work(mbhc_data->irq_btn_handle_wq,
			&mbhc_data->btn_delay_work, msecs_to_jiffies(10));
	} else {
		/* it no audio jack, it is plug in irq */
		/* it has audio jack, and hs is plugin */
		/* it is plug out irq */
		hwlog_info("%s: queue hs_plug_work plug in or out\n", __func__);
		queue_delayed_work(mbhc_data->irq_plug_handle_wq,
			&mbhc_data->hs_plug_work, msecs_to_jiffies(20));
	}
	OUT_FUNCTION;
}

static irqreturn_t hwcxext_mbhc_plug_btn_irq_thread(int irq, void *data)
{
	struct hwcxext_mbhc_priv *mbhc_data = data;

	__pm_wakeup_event(mbhc_data->plug_btn_irq_wake_lock, 1000);
	hwlog_info("%s: enter, irq = %d\n", __func__, irq);

	queue_delayed_work(mbhc_data->irq_plug_btn_wq,
		&mbhc_data->irq_plug_btn_work, msecs_to_jiffies(20));
	OUT_FUNCTION;
	return IRQ_HANDLED;
}

static int hwcxext_mbhc_get_plug_btn_irq_gpio(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *gpio_str = "plug_btn_irq_gpio";
	struct device *dev = mbhc_data->dev;

	mbhc_data->plug_btn_irq_gpio =
		of_get_named_gpio(dev->of_node, gpio_str, 0);
	if (mbhc_data->plug_btn_irq_gpio < 0) {
		hwlog_debug("%s: get_named_gpio:%s failed, %d\n", __func__,
			gpio_str, mbhc_data->plug_btn_irq_gpio);
		ret = of_property_read_u32(dev->of_node, gpio_str,
			(u32 *)&(mbhc_data->plug_btn_irq_gpio));
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			return -EFAULT;
		}
	}

	if (!gpio_is_valid(mbhc_data->plug_btn_irq_gpio)) {
		hwlog_err("%s: irq_handler gpio %d invalid\n", __func__,
			mbhc_data->plug_btn_irq_gpio);
		return -EFAULT;
	}

	ret = gpio_request(mbhc_data->plug_btn_irq_gpio, "hwcxext_plug_irq");
	if (ret < 0) {
		hwlog_err("%s: gpio_request ret %d invalid\n", __func__, ret);
		return -EFAULT;
	}

	ret = gpio_direction_input(mbhc_data->plug_btn_irq_gpio);
	if (ret < 0) {
		hwlog_err("%s set gpio input mode error:%d\n",
			__func__, ret);
		goto get_plug_btn_irq_gpio_err;
	}

	return 0;
get_plug_btn_irq_gpio_err:
	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	return ret;
}

static int hwcxext_mbhc_request_plug_btn_detect_irq(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	const char *irq_flags_str = "irq_flags";
	struct device *dev = mbhc_data->dev;
	unsigned int irqflags;

	ret = hwcxext_mbhc_get_plug_btn_irq_gpio(mbhc_data);
	if (ret < 0) {
		hwlog_err("%s: btn_irq_gpio failed ret= %d\n", __func__, ret);
		return ret;
	}

	mbhc_data->plug_btn_irq =
		gpio_to_irq((unsigned int)(mbhc_data->plug_btn_irq_gpio));
	hwlog_info("%s detect_irq_gpio: %d, irq: %d\n", __func__,
		mbhc_data->plug_btn_irq_gpio, mbhc_data->plug_btn_irq);

	ret = of_property_read_u32(dev->of_node, irq_flags_str, &irqflags);
	if (ret < 0) {
		hwlog_err("%s: irq_handler get irq_flags failed\n", __func__);
		goto request_plug_irq_err;
	}
	hwlog_info("%s irqflags: %d, ret = %d\n", __func__, irqflags, ret);

	ret = request_threaded_irq(mbhc_data->plug_btn_irq, NULL,
		hwcxext_mbhc_plug_btn_irq_thread,
		irqflags | IRQF_ONESHOT | IRQF_NO_SUSPEND,
		"hwcxext_mbhc_plug_detect", mbhc_data);
	if (ret) {
		hwlog_err("%s: Failed to request IRQ: %d\n", __func__, ret);
		goto request_plug_irq_err;
	}
	hwlog_info("%s request_threaded_irq ret = %d\n", __func__, ret);
	return 0;
request_plug_irq_err:
	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	hwlog_err("%s:btn_detect_irq Failed \n", __func__);
	return -EFAULT;
}

static void hwcxext_mbhc_free_plug_irq(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (!gpio_is_valid(mbhc_data->plug_btn_irq_gpio))
		return;

	gpio_free(mbhc_data->plug_btn_irq_gpio);
	mbhc_data->plug_btn_irq_gpio = -EINVAL;
	free_irq(mbhc_data->plug_btn_irq, mbhc_data);
}

void hwcxext_mbhc_detect_init_state(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int pulg = hwcxext_mbhc_check_headset_in(mbhc_data, 500);

	hwlog_info("%s: the headset is :%d \n", __func__, pulg);
	if (pulg == AUDIO_JACK_HEADSET_NOUSE) {
		hwlog_info("%s: HEADSET_NOUSE mdelay.\n", __func__);
		mdelay(300);
	}
	if ((pulg != AUDIO_JACK_NONE) &&
		(pulg != AUDIO_JACK_PLUG_OUT)) {
		hwcxext_mbhc_micbias_control(mbhc_data, MICB_ENABLE);
		hwcxext_mbhc_plug_in_detect(mbhc_data, pulg);
	}
	mbhc_data->init_state = true;
}

static struct snd_soc_jack_pin g_headset_jack_pins[] = {
	{
		.pin = "Headset Mic Jack",
		.mask = SND_JACK_MICROPHONE,
	},
	{
		.pin = "Headphone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
};

static struct snd_soc_jack_pin g_rear_jack_pins[] = {
	{
		.pin = "Rear Mic Jack",
		.mask = SND_JACK_MICROPHONE,
	},
	{
		.pin = "Rear HeadPhone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
	{
		.pin = "LineIn Jack",
		.mask = SND_JACK_LINEIN,
	},
};

static int hwcxext_mbhc_register_hs_jack_btn(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;
	struct jack_key_to_type key_type[] = {
		{ SND_JACK_BTN_0, KEY_PLAYPAUSE },
		{ SND_JACK_BTN_1, KEY_VOLUMEUP },
		{ SND_JACK_BTN_2, KEY_VOLUMEDOWN },
	};

	ret = snd_soc_card_jack_new(mbhc_data->component->card, "Headset Jack",
		SND_JACK_HEADSET,
		&mbhc_data->headset_jack, g_headset_jack_pins,
		ARRAY_SIZE(g_headset_jack_pins));
	if (ret) {
		hwlog_err("%s: Failed to create new headset jack\n", __func__);
		return -ENOMEM;
	}

	ret = snd_soc_card_jack_new(mbhc_data->component->card, "Rear Jack",
		(SND_JACK_HEADSET | SND_JACK_LINEIN),
		&mbhc_data->rear_jack, g_rear_jack_pins,
		ARRAY_SIZE(g_rear_jack_pins));
	if (ret) {
		hwlog_err("%s: Failed to create new rear jack\n", __func__);
		return -ENOMEM;
	}

	int i;
	for (i = 0; i < ARRAY_SIZE(key_type); i++) {
		ret = snd_jack_set_key(mbhc_data->headset_jack.jack,
			key_type[i].type, key_type[i].key);
		if (ret) {
			hwlog_err("%s:set code for btn%d, error num: %d", i, ret);
			return -ENOMEM;
		}
	}

	return 0;
}

static void hwcxext_mbhc_variables_init(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	mbhc_data->hs_status = AUDIO_JACK_NONE;
	atomic_set(&mbhc_data->micbias_vote_cnt, 0);
}

static void hwcxext_mbhc_mutex_init(struct hwcxext_mbhc_priv *mbhc_data)
{
	mbhc_data->plug_btn_wake_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-plug-btn");
	mbhc_data->plug_btn_irq_wake_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-plug-btn_irq");
	mbhc_data->plug_wake_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-plug");
	mbhc_data->btn_wake_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-btn");
	mbhc_data->long_btn_wake_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-long-btn");
	mbhc_data->plug_repeat_detect_lock = wakeup_source_register(NULL,
		"hwcxext-mbhc-repeat-detect");
	mutex_init(&mbhc_data->plug_mutex);
	mutex_init(&mbhc_data->status_mutex);
	mutex_init(&mbhc_data->btn_mutex);
	mutex_init(&mbhc_data->micbias_vote_mutex);
}

static int hwcxext_mbhc_variable_check(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	if (dev == NULL || mbhc_cb == NULL ||
		component == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static void hwcxext_mbhc_set_priv_data(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_pri, struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data, struct hwcxext_mbhc_cb *mbhc_cb)
{
	*mbhc_data = mbhc_pri;
	mbhc_pri->mbhc_cb = mbhc_cb;
	mbhc_pri->component = component;
	mbhc_pri->dev = dev;
}

static int hwcxext_mbhc_register_switch_dev(
	struct hwcxext_mbhc_priv *mbhc_pri)
{
	int ret;

#ifdef CONFIG_SWITCH
	mbhc_pri->sdev.name = "h2w";
	ret = switch_dev_register(&(mbhc_pri->sdev));
	if (ret) {
		hwlog_err("%s: switch_dev_register failed: %d\n",
			__func__, ret);
		return -ENOMEM;
	}
	hwlog_info("%s: switch_dev_register succ\n", __func__);
#endif
	return 0;
}

static void hwcxext_mbhc_remove_plug_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_plug_btn_wq) {
		cancel_delayed_work(&mbhc_data->irq_plug_btn_work);
		flush_workqueue(mbhc_data->irq_plug_btn_wq);
		destroy_workqueue(mbhc_data->irq_plug_btn_wq);
		mbhc_data->irq_plug_btn_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_plug_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_plug_handle_wq) {
		cancel_delayed_work(&mbhc_data->hs_plug_work);
		flush_workqueue(mbhc_data->irq_plug_handle_wq);
		destroy_workqueue(mbhc_data->irq_plug_handle_wq);
		mbhc_data->irq_plug_handle_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->irq_btn_handle_wq) {
		cancel_delayed_work(&mbhc_data->btn_delay_work);
		flush_workqueue(mbhc_data->irq_btn_handle_wq);
		destroy_workqueue(mbhc_data->irq_btn_handle_wq);
		mbhc_data->irq_btn_handle_wq = NULL;
	}
}

static void hwcxext_mbhc_remove_long_press_btn_workqueue(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	if (mbhc_data->long_press_btn_wq) {
		cancel_delayed_work(&mbhc_data->long_press_btn_work);
		flush_workqueue(mbhc_data->long_press_btn_wq);
		destroy_workqueue(mbhc_data->long_press_btn_wq);
		mbhc_data->long_press_btn_wq = NULL;
	}
}

static int hwcxext_mbhc_create_singlethread_workqueue(
	struct workqueue_struct **wq, const char *name)
{
	struct workqueue_struct *req_wq = create_singlethread_workqueue(name);

	if (!req_wq) {
		hwlog_err("%s : create %s wq failed", __func__, name);
		return -ENOMEM;
	}

	*wq = req_wq;
	return 0;
}

static int hwcxext_mbhc_create_delay_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	int ret;

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_plug_btn_wq), "hwcxext_mbhc_plug_btn_wq");
	if (ret < 0)
		goto create_delay_work_err1;
	INIT_DELAYED_WORK(&mbhc_data->irq_plug_btn_work,
		hwcxext_mbhc_irq_plug_btn_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_plug_handle_wq), "hwcxext_mbhc_plug_wq");
	if (ret < 0)
		goto create_delay_work_err2;
	INIT_DELAYED_WORK(&mbhc_data->hs_plug_work,
		hwcxext_mbhc_plug_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->irq_btn_handle_wq), "hwcxext_mbhc_btn_delay_wq");
	if (ret < 0)
		goto create_delay_work_err3;
	INIT_DELAYED_WORK(&mbhc_data->btn_delay_work, hwcxext_mbhc_btn_work);

	ret = hwcxext_mbhc_create_singlethread_workqueue(
		&(mbhc_data->long_press_btn_wq), "hwcxext_mbhc_long_press_btn_wq");
	if (ret < 0)
		goto create_delay_work_err4;
	INIT_DELAYED_WORK(&mbhc_data->long_press_btn_work,
		hwcxext_mbhc_long_press_btn_work);

	return 0;

create_delay_work_err5:
	hwcxext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
create_delay_work_err4:
	hwcxext_mbhc_remove_btn_workqueue(mbhc_data);
create_delay_work_err3:
	hwcxext_mbhc_remove_plug_workqueue(mbhc_data);
create_delay_work_err2:
	hwcxext_mbhc_remove_plug_btn_workqueue(mbhc_data);
create_delay_work_err1:
	return -ENOMEM;
}

static void hwcxext_mbhc_destroy_delay_work(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	hwcxext_mbhc_remove_plug_btn_workqueue(mbhc_data);
	hwcxext_mbhc_remove_btn_workqueue(mbhc_data);
	hwcxext_mbhc_remove_plug_workqueue(mbhc_data);
	hwcxext_mbhc_remove_long_press_btn_workqueue(mbhc_data);
}

static int hwcxext_mbhc_resource_init(
	struct hwcxext_mbhc_priv *mbhc_pri)
{
	int ret;

	ret = hwcxext_mbhc_create_delay_work(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s: resource_init suss = %d\n", __func__, ret);
		return ret;
	}

	ret = hwcxext_mbhc_request_plug_btn_detect_irq(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s request plug detect irq failed\n", __func__);
		ret = -EINVAL;
		goto hwcxext_mbhc_resource_init_err1;
	}

	ret = hwcxext_mbhc_register_hs_jack_btn(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s register hs jack failed\n", __func__);
		goto hwcxext_mbhc_resource_init_err2;
	}

	ret = hwcxext_mbhc_register_switch_dev(mbhc_pri);
	if (ret < 0) {
		goto hwcxext_mbhc_resource_init_err2;
		hwlog_err("%s hwcxext_mbhc_register_switch_dev failed\n", __func__);
	}

	return 0;
hwcxext_mbhc_resource_init_err2:
	hwcxext_mbhc_free_plug_irq(mbhc_pri);
hwcxext_mbhc_resource_init_err1:
	hwcxext_mbhc_destroy_delay_work(mbhc_pri);
	hwlog_err("%s resource_init failed = %d\n", __func__, ret);
	return ret;
}

int hwcxext_mbhc_init(struct device *dev,
	struct snd_soc_component *component,
	struct hwcxext_mbhc_priv **mbhc_data,
	struct hwcxext_mbhc_cb *mbhc_cb)
{
	int ret;
	struct hwcxext_mbhc_priv *mbhc_pri = NULL;

	IN_FUNCTION;
	if (hwcxext_mbhc_variable_check(dev, component, mbhc_data, mbhc_cb) < 0) {
		hwlog_err("%s: variable_check fail\n", __func__);
		return -EINVAL;
	}

	mbhc_pri = devm_kzalloc(dev,
			sizeof(struct hwcxext_mbhc_priv), GFP_KERNEL);
	if (mbhc_pri == NULL) {
		hwlog_err("%s: kzalloc fail\n", __func__);
		return -ENOMEM;
	}

	hwcxext_mbhc_set_priv_data(dev, mbhc_pri,
		component, mbhc_data, mbhc_cb);
	hwcxext_mbhc_variables_init(mbhc_pri);
	hwcxext_mbhc_mutex_init(mbhc_pri);
	mbhc_pri->init_state = false;
	ret = hwcxext_mbhc_resource_init(mbhc_pri);
	if (ret < 0) {
		hwlog_err("%s: resource_init fail ret = %d\n", __func__, ret);
		goto hwcxext_mbhc_init_err1;
	}
	hwlog_info("%s: resource_init suss = %d\n", __func__, ret);
	if (mbhc_pri->mbhc_cb->enable_jack_detect)
		mbhc_pri->mbhc_cb->enable_jack_detect(mbhc_pri);
	return 0;
hwcxext_mbhc_init_err1:
	devm_kfree(dev, mbhc_pri);
	return ret;
}
EXPORT_SYMBOL_GPL(hwcxext_mbhc_init);

static void hwcxext_mbhc_mutex_deinit(
	struct hwcxext_mbhc_priv *mbhc_data)
{
	wakeup_source_unregister(mbhc_data->plug_btn_wake_lock);
	wakeup_source_unregister(mbhc_data->plug_btn_irq_wake_lock);
	wakeup_source_unregister(mbhc_data->plug_wake_lock);
	wakeup_source_unregister(mbhc_data->btn_wake_lock);
	wakeup_source_unregister(mbhc_data->long_btn_wake_lock);
	wakeup_source_unregister(mbhc_data->plug_repeat_detect_lock);
	mutex_destroy(&mbhc_data->plug_mutex);
	mutex_destroy(&mbhc_data->status_mutex);
	mutex_destroy(&mbhc_data->btn_mutex);
	mutex_destroy(&mbhc_data->micbias_vote_mutex);
}

void hwcxext_mbhc_exit(struct device *dev,
	struct hwcxext_mbhc_priv *mbhc_data)
{
	IN_FUNCTION;

	if (dev == NULL || mbhc_data == NULL) {
		hwlog_err("%s: params is invaild\n", __func__);
		return;
	}

	hwcxext_mbhc_mutex_deinit(mbhc_data);
	hwcxext_mbhc_free_plug_irq(mbhc_data);
	hwcxext_mbhc_destroy_delay_work(mbhc_data);

#ifdef CONFIG_SWITCH
	switch_dev_unregister(&(mbhc_data->sdev));
#endif
	devm_kfree(dev, mbhc_data);
}
EXPORT_SYMBOL_GPL(hwcxext_mbhc_exit);

MODULE_DESCRIPTION("hwcxext mbhc");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
