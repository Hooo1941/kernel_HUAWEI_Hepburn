/*
 * usbaudio_ctrl.c
 *
 * usbaudio ctrl.
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#include <linux/usb/audio-v3.h>
#endif
#include <linux/slab.h>

#include <sound/control.h>
#include <sound/core.h>
#include <sound/info.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <linux/platform_drivers/usb/chip_usb.h>
#ifdef CONFIG_HUAWEI_AUDIO
#include <huawei_platform/audio/usb_audio_power.h>
#include <huawei_platform/audio/usb_audio_power_v600.h>
#include <huawei_platform/audio/dig_hs.h>
#endif
#ifdef CONFIG_LOG_EXCEPTION
#include "huawei_platform/log/imonitor.h"
#endif
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#include "usbaudio.h"
#include "card.h"
#include "helper.h"
#include "format.h"
#include "clock.h"
#include "usbaudio_dsp_client.h"
#include "audio_log.h"
#include "usbaudio_dtsi_property.h"

#ifndef LOG_TAG
#define LOG_TAG "usbaudio"
#endif

#undef CREATE_TRACE_POINTS
#include <trace/hooks/hck_hook/hck_usb_audio.h>

#define DTS_USBAUDIO_DSP_NAME         "hisilicon,usbaudiodsp"
#ifdef CONFIG_HUAWEI_AUDIO
#define USB_HEADSET_PRENAME           "HUAWEI USB-C"
#define BBIITT_USB_HEADSET_PRENAME    "BBIITT USB-C"
#endif
#define USB_AUDIO_TYPEC_INFO_EVENT_ID 931001000
#define USBAUDIO_WAKE_UP_TIMEOUT      (5 * HZ)

struct usbaudio_dsp_client_ops {
	bool (*controller_switch)(struct usbaudio_dsp *pdata,
		struct usb_host_interface *ctrl_intf,
		int ctrlif, struct usbaudio_pcms *pcms);
	int (*disconnect)(struct snd_usb_audio *chip, unsigned int dsp_reset_flag);
	int (*set_pipeout_interface)(struct snd_usb_audio *chip,
		struct usbaudio_pcms *pcms, unsigned int running, unsigned int rate);
	int (*set_pipein_interface)(struct snd_usb_audio *chip,
		struct usbaudio_pcms *pcms, unsigned int running, unsigned int rate);
	bool (*controller_query)(struct snd_usb_audio *chip);
	bool (*send_usbaudioinfo2hifi)(struct snd_usb_audio *chip,
		struct usbaudio_pcms *pcms);
	void (*setinterface_complete)(unsigned int dir, unsigned int running,
		int ret, unsigned int rate);
	void (*nv_check_complete)(unsigned int usb_id, struct usb_device *usb_dev);
	int (*usb_resume)(void);
};

static struct usbaudio_dsp_client_ops g_client_ops = {
	.controller_switch = controller_switch,
	.disconnect = disconnect,
	.set_pipeout_interface = set_pipeout_interface,
	.set_pipein_interface = set_pipein_interface,
	.controller_query = controller_query,
	.setinterface_complete = setinterface_complete,
	.nv_check_complete = usbaudio_nv_check_complete,
	.usb_resume = usb_power_resume,
};

static struct usbaudio_dsp *g_usbaudio_hifi;

static void set_usbaudio_priv_data(struct usbaudio_dsp *pdata)
{
	g_usbaudio_hifi = pdata;
}

static struct usbaudio_dsp *get_usbaudio_priv_data(void)
{
	return g_usbaudio_hifi;
}

static void usbaudio_resume(void)
{
	unsigned long ret;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return;

	mutex_lock(&pdata->wakeup_mutex);
	if (pdata->wake_up) {
		mutex_unlock(&pdata->wakeup_mutex);
		return;
	}

	reinit_completion(&pdata->resume_complete);
	mutex_unlock(&pdata->wakeup_mutex);
	ret = wait_for_completion_timeout(&pdata->resume_complete,
		USBAUDIO_WAKE_UP_TIMEOUT);
	if (ret == 0)
		AUDIO_LOGE("usbaudio resume timeout");
}

static void usbaudio_ctrl_wake_up(bool wake_up)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return;

	mutex_lock(&pdata->wakeup_mutex);
	pdata->wake_up = wake_up;
	if (pdata->wake_up)
		complete(&pdata->resume_complete);
	mutex_unlock(&pdata->wakeup_mutex);
}

static bool usbaudio_ctrl_controller_switch(struct usb_device *dev, u32 usb_id,
	struct usb_host_interface *ctrl_intf, int ctrlif)
{
	bool ret = false;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return false;

	if (pdata->usb_dev != dev && pdata->usbaudio_connect_state == USBAUDIO_DSP)
		return false;

	mutex_lock(&pdata->connect_mutex);
	pdata->pipeout_running_flag = STOP_STREAM;
	pdata->pipein_running_flag = STOP_STREAM;
	pdata->usb_id = usb_id;
	pdata->usb_dev = dev;
	pdata->usbaudio_connect_state = USBAUDIO_AP;
	ret = pdata->ops->controller_switch(pdata,
		ctrl_intf, ctrlif, &pdata->pcms);
	mutex_unlock(&pdata->connect_mutex);

	return ret;
}

void usbaudio_ctrl_hifi_reset_inform(void)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (pdata && pdata->chip)
		pdata->dsp_reset_flag = USBAUDIO_DSP_ABNORMAL;
}

static int usbaudio_ctrl_disconnect(struct usb_device *dev)
{
	int ret;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata || !pdata->usb_dev)
		return 0;
	if (pdata->usb_dev != dev) {
		AUDIO_LOGI("disconnect ap usb");
		return 0;
	}

	mutex_lock(&pdata->connect_mutex);
	if (!pdata->chip) {
		ret = 0;
	} else {
		AUDIO_LOGI("disconnect hifi usb");
		pdata->usbaudio_connect_state = USBAUDIO_NONE;
		ret = pdata->ops->disconnect(pdata->chip, pdata->dsp_reset_flag);
		pdata->chip = NULL;
	}
	mutex_unlock(&pdata->connect_mutex);

	return ret;
}

void usbaudio_ctrl_nv_check(void)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();
	if (!pdata)
		return;

	mutex_lock(&pdata->connect_mutex);
	if (pdata->usbaudio_connect_state != USBAUDIO_AP)
		AUDIO_LOGE("usbaudio has been disconnected");
	else
		pdata->ops->nv_check_complete(pdata->usb_id, pdata->usb_dev);

	mutex_unlock(&pdata->connect_mutex);
}

int usbaudio_ctrl_usb_resume(void)
{
	int ret;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return 0;

	mutex_lock(&pdata->connect_mutex);
	if (!pdata->chip)
		ret = 0;
	else
		ret = pdata->ops->usb_resume();
	mutex_unlock(&pdata->connect_mutex);

	return ret;
}

int usbaudio_ctrl_set_pipeout_interface(unsigned int running, unsigned int rate)
{
	int ret;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return 0;

	mutex_lock(&pdata->connect_mutex);

	if (!pdata->chip) {
		mutex_unlock(&pdata->connect_mutex);
		return 0;
	}

	if (running == pdata->pipeout_running_flag && rate == pdata->dn_rate) {
		AUDIO_LOGE("return. running: %u rate %u", running, rate);
		ret = 0;
	} else {
		pdata->pipeout_running_flag = running;
		if (running == STOP_STREAM)
			usbaudio_resume();
		ret = pdata->ops->set_pipeout_interface(pdata->chip,
			&pdata->pcms, running, rate);
		pdata->dn_rate = rate;
	}

	pdata->ops->setinterface_complete(SNDRV_PCM_STREAM_PLAYBACK,
		running, ret, rate);

	mutex_unlock(&pdata->connect_mutex);

	return ret;
}

int usbaudio_ctrl_set_pipein_interface(unsigned int running,
	unsigned int rate, unsigned int period)
{
	int ret = 0;
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return 0;

	mutex_lock(&pdata->connect_mutex);

	if (!pdata->chip) {
		mutex_unlock(&pdata->connect_mutex);
		return 0;
	}

	if (running == pdata->pipein_running_flag && rate == pdata->up_rate &&
		period == pdata->up_period) {
		pdata->ops->setinterface_complete(SNDRV_PCM_STREAM_CAPTURE,
			running, ret, rate);
		mutex_unlock(&pdata->connect_mutex);
		return 0;
	}

	AUDIO_LOGI("running: %u rate: %u period: %u old running: %u old rate: %u old period: %u",
		running, rate, period,
		pdata->pipein_running_flag, pdata->up_rate, pdata->up_period);

	/* change period do not need to update interface */
	if (running != pdata->pipein_running_flag || rate != pdata->up_rate) {
		if (running == STOP_STREAM)
			usbaudio_resume();
		ret = pdata->ops->set_pipein_interface(pdata->chip,
			&pdata->pcms, running, rate);
	}
	pdata->pipein_running_flag = running;
	pdata->up_rate = rate;
	pdata->up_period = period;
	pdata->ops->setinterface_complete(SNDRV_PCM_STREAM_CAPTURE,
		running, ret, rate);

	mutex_unlock(&pdata->connect_mutex);

	return ret;
}

void usbaudio_ctrl_query_info(struct usbaudio_info *usbinfo)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!usbinfo)
		return;

	if (!pdata) {
		usbinfo->controller_location = ACPU_CONTROL;
		return;
	}

	mutex_lock(&pdata->connect_mutex);
	if (!pdata->chip) {
		usbinfo->controller_location = ACPU_CONTROL;
	} else {
		if (pdata->ops->controller_query(pdata->chip))
			usbinfo->controller_location = DSP_CONTROL;
		else
			usbinfo->controller_location = ACPU_CONTROL;
	}
	if (pdata->chip) {
		usbinfo->usbid = pdata->info.usbid;
		usbinfo->dnlink_channels = pdata->info.dnlink_channels;
		usbinfo->uplink_channels = pdata->info.uplink_channels;
		memcpy(usbinfo->name, pdata->info.name, sizeof(usbinfo->name));
		memcpy(usbinfo->dnlink_rate_table, pdata->info.dnlink_rate_table,
			sizeof(usbinfo->dnlink_rate_table));
		AUDIO_LOGI("usbid 0x%x, dnlink channels %u, uplink channels %u",
			usbinfo->usbid, usbinfo->dnlink_channels,
			usbinfo->uplink_channels);
		AUDIO_LOGI("usbname %s", usbinfo->name);
	}
	mutex_unlock(&pdata->connect_mutex);
}

#ifdef CONFIG_LOG_EXCEPTION
static int usbaudio_ctrl_typec_log_upload(const void *data)
{
	static unsigned int insert_times;
	const struct snd_usb_audio *info = NULL;
	struct imonitor_eventobj *obj = NULL;
	int ret;

	if (!data) {
		AUDIO_LOGE("headset info is null");
		return -EINVAL;
	}

	info = (const struct snd_usb_audio *)data;
	insert_times++;
	AUDIO_LOGI("typec headset bcddevice 0x%x, times %d",
		info->dev->descriptor.bcdDevice, insert_times);

	obj = imonitor_create_eventobj(USB_AUDIO_TYPEC_INFO_EVENT_ID);
	if (!obj) {
		AUDIO_LOGE("imonitor create eventobj error");
		return -ENOMEM;
	}

	imonitor_set_param_integer_v2(obj, "usbid", info->usb_id);
	imonitor_set_param_string_v2(obj, "IC", info->dev->manufacturer);
	imonitor_set_param_string_v2(obj, "module", info->card->shortname);
	imonitor_set_param_string_v2(obj, "isn", info->dev->serial);
	imonitor_set_param_integer_v2(obj, "times", insert_times);
	imonitor_set_param_integer_v2(obj, "ver",
		info->dev->descriptor.bcdDevice);
	ret = imonitor_send_event(obj);
	imonitor_destroy_eventobj(obj);

	return ret;
}
#endif

static void usbaudio_ctrl_set_chip(struct snd_usb_audio *chip)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata) {
		AUDIO_LOGE("priv data is null");
		return;
	}
	if (!chip) {
		AUDIO_LOGE("chip data is null");
		return;
	}
	if (chip->dev != pdata->usb_dev)
		return;
	mutex_lock(&pdata->connect_mutex);
	pdata->wake_up = true;
	if (chip->card && chip->dev->serial) {
#ifdef CONFIG_HUAWEI_DSM
		if (usb_using_hifi_usb(chip->dev))
			audio_dsm_report_info(AUDIO_CODEC, DSM_USBAUDIO_INFO,
				"usbid %x usbphy %s\n", chip->usb_id, "hifi");
		else
			audio_dsm_report_info(AUDIO_CODEC, DSM_USBAUDIO_INFO,
				"usbid %x usbphy %s\n", chip->usb_id, "arm");
#endif
		memcpy(pdata->info.name, chip->card->shortname,
			sizeof(chip->card->shortname));
		pdata->info.name[USBAUDIO_INFONAME_LEN - 1] = '\0';
	}

	pdata->chip = chip;
	pdata->dsp_reset_flag = USBAUDIO_DSP_NORMAL;
	pdata->info.dnlink_channels = pdata->pcms.fmts[0].channels;
	pdata->info.uplink_channels = pdata->pcms.fmts[1].channels;
	memcpy(pdata->info.dnlink_rate_table, pdata->pcms.fmts[0].rate_table,
		sizeof(pdata->info.dnlink_rate_table));
	pdata->info.usbid = chip->usb_id;
	send_usbaudioinfo2hifi(pdata->chip, &pdata->pcms);

#ifdef CONFIG_LOG_EXCEPTION
	if (usbaudio_ctrl_typec_log_upload(chip) < 0)
		AUDIO_LOGE("imonitor send eventobj error");
#endif

	if (chip->card) {
		AUDIO_LOGI("usb id is 0x%x, name is %s",
			chip->usb_id, chip->card->shortname);
#ifdef CONFIG_HUAWEI_AUDIO
		if (usb_using_hifi_usb(chip->dev) &&
			((strncmp(chip->card->shortname, USB_HEADSET_PRENAME,
				strlen(USB_HEADSET_PRENAME)) == 0) ||
			(strncmp(chip->card->shortname, BBIITT_USB_HEADSET_PRENAME,
				strlen(BBIITT_USB_HEADSET_PRENAME)) == 0))) {
			usb_audio_power_buckboost();
			usb_headset_plug_in();
			set_otg_switch_enable_v600();
			dig_hs_plug_in();
		}
#else
		if (usb_using_hifi_usb(chip->dev))
			AUDIO_LOGI("usb hs plug in");
#endif
	}

	mutex_unlock(&pdata->connect_mutex);
}

static int usb_audio_pre_reset(struct usb_interface *intf)
{
	return 0;
}

static int usb_audio_post_reset(struct usb_interface *intf)
{
	return 0;
}

static void usb_audio_connect(void *data, struct usb_interface *intf, bool *switch_result)
{
	struct usb_device *dev = NULL;
	struct usb_driver *drv = NULL;
	struct usb_host_interface *alts = NULL;
	int ifnum;
	u32 id;

	if (!intf || !switch_result) {
		AUDIO_LOGE("usb has null config");
		return;
	}

	drv = to_usb_driver(intf->dev.driver);
	if (!drv)
		return;

	drv->pre_reset = usb_audio_pre_reset;
	drv->post_reset = usb_audio_post_reset;

	dev = interface_to_usbdev(intf);
	alts = &intf->altsetting[0];
	if (!dev || !alts)
		return;
	ifnum = get_iface_desc(alts)->bInterfaceNumber;
	id = USB_ID(le16_to_cpu(dev->descriptor.idVendor),
		    le16_to_cpu(dev->descriptor.idProduct));

	check_customized_earphone_device(dev);
	AUDIO_LOGI("usb id is %x", id);
	*switch_result = usbaudio_ctrl_controller_switch(dev, id, alts, ifnum);
	if(*switch_result)
		AUDIO_LOGI("use hifi usb");
}

static void usb_audio_set_chip(void *data, struct snd_usb_audio *chip)
{
	if (!chip)
		return;

	usbaudio_ctrl_set_chip(chip);
}

static void usb_audio_check_auto_suspend(void *data, struct usb_device *dev)
{
	if (!dev)
		return;

	if(get_usbaudio_need_auto_suspend()) {
		AUDIO_LOGI("%s: set usb auto suspend\n",__FUNCTION__);
		usb_enable_autosuspend(dev);
	}
}

static void usb_audio_disconnect(void *data, struct usb_interface *intf)
{
	struct usb_device *dev;

	if (!intf)
		return;
	dev = interface_to_usbdev(intf);
	usbaudio_ctrl_disconnect(dev);
}

static void usb_audio_ctrl_wake_up(void *data, bool wake_up)
{
	usbaudio_ctrl_wake_up(wake_up);
}

static void usbaudio_hook_funcs_register(void)
{
	register_trace_hck_vh_usb_audio_connect(usb_audio_connect, NULL);
	register_trace_hck_vh_usb_audio_set_chip(usb_audio_set_chip, NULL);
	register_trace_hck_vh_usb_audio_check_auto_suspend(usb_audio_check_auto_suspend, NULL);
	register_trace_hck_vh_usb_audio_disconnect(usb_audio_disconnect, NULL);
	register_trace_hck_vh_usb_audio_ctrl_wake_up(usb_audio_ctrl_wake_up, NULL);
}

static int usbaudio_dsp_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct usbaudio_dsp *pdata = NULL;
	struct device_node *node = dev->of_node;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	ret = usbaudio_mailbox_init();
	if (ret)
		return ret;

	pdata->ops = &g_client_ops;
	pdata->usbaudio_connect_state = USBAUDIO_NONE;
	init_completion(&pdata->resume_complete);
	mutex_init(&pdata->connect_mutex);
	mutex_init(&pdata->wakeup_mutex);

	platform_set_drvdata(pdev, pdata);
	set_usbaudio_priv_data(pdata);

	usbaudio_hook_funcs_register();
	if (of_property_read_bool(node, "only_support_capture_microphone"))
		pdata->only_support_capture_microphone = true;
	else
		pdata->only_support_capture_microphone = false;
	return 0;
}

static int usbaudio_dsp_remove(struct platform_device *pdev)
{
	struct usbaudio_dsp *pdata = get_usbaudio_priv_data();

	if (!pdata)
		return 0;

	mutex_destroy(&pdata->connect_mutex);
	mutex_destroy(&pdata->wakeup_mutex);

	usbaudio_mailbox_deinit();
	set_usbaudio_priv_data(NULL);

	return 0;
}

static const struct of_device_id usbaudio_dsp_match_table[] = {
	{
		.compatible = DTS_USBAUDIO_DSP_NAME,
		.data = NULL,
	},
	{}
};

static struct platform_driver usbaudio_dsp_driver = {
	.driver = {
		.name = "usbaudio_dsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(usbaudio_dsp_match_table),
	},
	.probe = usbaudio_dsp_probe,
	.remove = usbaudio_dsp_remove,
};

static int __init usbaudio_dsp_init(void)
{
	return platform_driver_register(&usbaudio_dsp_driver);
}

static void __exit usbaudio_dsp_exit(void)
{
	platform_driver_unregister(&usbaudio_dsp_driver);
}

fs_initcall_sync(usbaudio_dsp_init);
module_exit(usbaudio_dsp_exit);

MODULE_DESCRIPTION("usbaudio dsp driver");
MODULE_LICENSE("GPL v2");
