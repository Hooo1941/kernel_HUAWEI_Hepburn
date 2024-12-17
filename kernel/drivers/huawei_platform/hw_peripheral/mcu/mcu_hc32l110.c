/*
 * mcu.c
 *
 * mcu driver
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
#include "mcu_hc32l110.h"
#include "mcu_hc32l110_i2c.h"
#include "mcu_hc32l110_fw.h"
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_firmware.h>
#include <chipset_common/hwpower/common_module/power_i2c.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <linux/timer.h>

#define HWLOG_TAG mcu_hc32l110
HWLOG_REGIST();

#define MCU_CMD_RTCTIME			0x03
#define MCU_CMD_SOC_STATUS		0x0A

#define MCU_FRAME_RTC_LEN		7
#define MCU_CUR_YEAR			2000
#define MCU_BASE_YEAR			1900

extern struct rtc_wkalrm poweroff_rtc_alarm;

static int mcu_hc32l110_decimal_to_hex(int decimal)
{
	return ((decimal / 10) << 4) | (decimal % 10);
}

static void mcu_hc32l110_rtc_time_dec2hex(struct rtc_time *tm)
{
	tm->tm_sec = mcu_hc32l110_decimal_to_hex(tm->tm_sec);
	tm->tm_min = mcu_hc32l110_decimal_to_hex(tm->tm_min);
	tm->tm_hour = mcu_hc32l110_decimal_to_hex(tm->tm_hour);
	tm->tm_wday = mcu_hc32l110_decimal_to_hex(tm->tm_wday);
	tm->tm_mday = mcu_hc32l110_decimal_to_hex(tm->tm_mday);
	tm->tm_mon = mcu_hc32l110_decimal_to_hex(tm->tm_mon + 1);
	tm->tm_year = mcu_hc32l110_decimal_to_hex(tm->tm_year + MCU_BASE_YEAR - MCU_CUR_YEAR);
}

static int mcu_reboot_notify_callback(struct notifier_block *this, unsigned long code, void *unused)
{
	struct rtc_time tm;
	struct rtc_wkalrm alrm;

	struct mcu_hc32l110_device_info *di = container_of(this, struct mcu_hc32l110_device_info, event_nb);
	if (!di)
		return NOTIFY_BAD;

	if (!rtc_read_time(di->rtc_dev, &tm)) {
		mcu_hc32l110_rtc_time_dec2hex(&tm);
		mcu_hc32l110_write_rtc_time(di, &tm);
	}

	if ((!di->is_usb_connected) && (code == SYS_POWER_OFF)) {
		if (!poweroff_rtc_alarm.enabled) {
			if (di->enter_std_proc) {
				mcu_hc32l110_write_byte(di, MCU_CMD_SOC_STATUS, 2);
			} else {
				mcu_hc32l110_write_byte(di, MCU_CMD_SOC_STATUS, 1);
			}
		}
	}
	return NOTIFY_DONE;
}

static int mcu_hc32l110_parse_dts(struct device_node *np, struct mcu_hc32l110_device_info *di)
{
	if (power_gpio_config_output(np, "gpio_reset", "hc32l110_gpio_reset", &di->gpio_reset, 0))
		goto reset_config_fail;

	if (power_gpio_config_output(np, "gpio_boot0", "hc32l110_gpio_boot0", &di->gpio_boot0, 0))
		goto enable_config_fail;

	return 0;

enable_config_fail:
	gpio_free(di->gpio_reset);
reset_config_fail:
	return -EINVAL;
}

static int __init has_wakealarm(struct device *dev, const void *data)
{
	struct rtc_device *candidate = to_rtc_device(dev);
	hwlog_info("RTCX: devname is %s!\n", dev_name(dev));
	if (!candidate->ops->read_alarm) {
		hwlog_info("RTCX: read_alarm not support!\n");
		return 0;
	}
	if (!device_may_wakeup(candidate->dev.parent)) {
		hwlog_info("RTCX: can_wakeup not support!\n");
		return 0;
	}
	hwlog_info("RTCX: has_wakealarm!\n");
	return 1;
}

static struct rtc_device *mcu_open_rtc_device(void)
{
	struct rtc_device *rtc = NULL;
	struct device *dev;

	dev = class_find_device(rtc_class, NULL, NULL, has_wakealarm);
	if (dev) {
		rtc = rtc_class_open(dev_name(dev));
		put_device(dev);
		hwlog_info("RTCX: dev ok(name:%s)!\n", dev_name(dev));
	}
	if (!rtc) {
		hwlog_info("RTCX: rtc_class_open failed!\n");
		return NULL;
	} else {
		hwlog_info("RTCX: rtc_class_open success!\n");
		return rtc;
	}
}

static int hc32l110_usb_event_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	hwlog_info("mcu receive usb event\n");
	struct mcu_hc32l110_device_info *di = container_of(nb, struct mcu_hc32l110_device_info, usb_event_nb);

	if (!di) {
		hwlog_err("mcu usb nb call err!");
		return NOTIFY_BAD;
	}

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		di->is_usb_connected = false;
		hwlog_info("mcu usb disconnected\n");
		break;
	case POWER_NE_USB_CONNECT:
		di->is_usb_connected = true;
		hwlog_info("mcu usb connected\n");
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int mcu_hc32l110_hex_to_decimal(u8 hex)
{
	return (hex >> 4) * 10 + (hex & 0x0F);
}

static void mcu_hc32l110_rtc_time_hex2dec(struct rtc_time *tm)
{
	tm->tm_sec = mcu_hc32l110_hex_to_decimal(tm->tm_sec);
	tm->tm_min = mcu_hc32l110_hex_to_decimal(tm->tm_min);
	tm->tm_hour = mcu_hc32l110_hex_to_decimal(tm->tm_hour);
	tm->tm_wday = mcu_hc32l110_hex_to_decimal(tm->tm_wday);
	tm->tm_mday = mcu_hc32l110_hex_to_decimal(tm->tm_mday);
	tm->tm_mon = mcu_hc32l110_hex_to_decimal(tm->tm_mon) - 1;
	tm->tm_year = MCU_CUR_YEAR + mcu_hc32l110_hex_to_decimal(tm->tm_year) - MCU_BASE_YEAR;
}

static void mcu_hc32l110_rtc_hctosys(struct rtc_device *rtc)
{
	int err;
	struct rtc_time tm;
	struct timespec64 tv64 = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	hwlog_err("start to hctosys sync mcu time \n");
	err = rtc_read_time(rtc, &tm);
	if (err) {
		hwlog_err("hctosys: unable to read the hardware clock, ret = %d\n", err);
		return;
	}

	hwlog_info("after: soc time: %02x, %02x, %02x, %02x, %02x, %02x, %02x\n", tm.tm_sec, tm.tm_min,
		tm.tm_hour, tm.tm_wday, tm.tm_mday, tm.tm_mon, tm.tm_year);

	tv64.tv_sec = rtc_tm_to_time64(&tm);

#if BITS_PER_LONG == 32
	if (tv64.tv_sec > INT_MAX) {
		err = -ERANGE;
		return;
	}
#endif

	err = do_settimeofday64(&tv64);
	if (err)
		hwlog_err("do_settimeofday64 err, ret = %d\n", err);

	hwlog_err("end to hctosys sync mcu time \n");
	return;
}

static int mcu_hc32l110_sync_time_to_soc(struct mcu_hc32l110_device_info *di)
{
	struct rtc_time tm;
	struct rtc_time soc_tm;

	di->rtc_dev = mcu_open_rtc_device();
	if (di->rtc_dev == NULL)
		return -EINVAL;

	if (mcu_hc32l110_read_rtc_time(di, &tm))
		return -EINVAL;

	if (rtc_read_time(di->rtc_dev, &soc_tm))
		hwlog_err("soc read rtc time err 1. %s:%d\n");

	mcu_hc32l110_rtc_time_hex2dec(&tm);
	if (rtc_set_time(di->rtc_dev, &tm))
		hwlog_err("mcu set rtc time err. %s:%d\n");

	mcu_hc32l110_rtc_hctosys(di->rtc_dev);

	hwlog_info("time info from mcu: %02x, %02x, %02x, %02x, %02x, %02x, %02x\n", tm.tm_sec, tm.tm_min,
		tm.tm_hour, tm.tm_wday, tm.tm_mday, tm.tm_mon, tm.tm_year);
	hwlog_info("before: soc time, first time: %02x, %02x, %02x, %02x, %02x, %02x, %02x\n", soc_tm.tm_sec,
		soc_tm.tm_min, soc_tm.tm_hour, soc_tm.tm_wday, soc_tm.tm_mday, soc_tm.tm_mon, soc_tm.tm_year);
	return 0;
}

static int mcu_hc32l110_hibernate_callback(struct notifier_block *nb, unsigned long event, void *ptr)
{
	struct mcu_hc32l110_device_info *di = container_of(nb, struct mcu_hc32l110_device_info, std_event_nb);

	if (!di) {
		hwlog_err("mcu hibernate nb callback error!");
		return NOTIFY_BAD;
	}

    switch (event) {
	case PM_HIBERNATION_PREPARE:
		di->enter_std_proc = true;
		break;
	case PM_POST_HIBERNATION:
		di->enter_std_proc = false;
		mcu_hc32l110_write_byte(di, MCU_CMD_SOC_STATUS, 0);
		break;
	default:
		break;
    }

	hwlog_info("mcu receive hibernate event %d", event);
    return NOTIFY_OK;
}

static int mcu_hc32l110_register_callback(struct mcu_hc32l110_device_info *di)
{
	di->event_nb.notifier_call = mcu_reboot_notify_callback;
	if (register_reboot_notifier(&di->event_nb)) {
		hwlog_err("register reboot notify err. %s:%d\n");
		return -EINVAL;
	}

	di->usb_event_nb.notifier_call = hc32l110_usb_event_notifier_call;
	if (power_event_bnc_register(POWER_BNT_CONNECT, &di->usb_event_nb)) {
		hwlog_err("register usb event err. %s:%d\n");
		goto mcu_free_reboot_nb;
	}

	di->std_event_nb.notifier_call = mcu_hc32l110_hibernate_callback;
	if (register_pm_notifier(&di->std_event_nb)) {
		hwlog_err("register std event err.\n");
		goto mcu_free_usb_nb;
	}

	return 0;

mcu_free_usb_nb:
	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->usb_event_nb);
mcu_free_reboot_nb:
	unregister_reboot_notifier(&di->event_nb);
	return -EINVAL;
}

static int mcu_hc32l110_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct mcu_hc32l110_device_info *di;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		hwlog_err("i2c_check_functionality error!\n", __func__, __LINE__);
		return -EINVAL;
	}

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		hwlog_err("mcu malloc err\n");
		return -ENOMEM;
	}

	di->dev = &client->dev;
	di->client = client;
	i2c_set_clientdata(client, di);
	di->is_low_power_mode = false;
	di->chip_already_init = 1;
	if (mcu_hc32l110_parse_dts(di->dev->of_node, di)) {
		hwlog_err("mcu prase dts err\n");
		goto fail_free_mem;
	}

	if (mcu_hc32l110_fw_get_dev_id(di)) {
		hwlog_err("get mcu dev id err, mcu may not in user mode\n");
		if (!mcu_hc32l110_fw_check_bootloader_mode(di)) {
			(void)mcu_hc32l110_update_check(di);
			return 0;
		} else {
			hwlog_err("mcu firmware is empty!");
			goto fail_free_gpio;
		}
	}

	if (mcu_hc32l110_register_callback(di))
		goto fail_free_gpio;

	if (mcu_hc32l110_sync_time_to_soc(di))
		goto fail_free_rtc;

	(void)mcu_hc32l110_update_check(di);

	hwlog_info("mcu probe done!\n");
	return 0;

fail_free_rtc:
	unregister_reboot_notifier(&di->event_nb);
	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->usb_event_nb);
	unregister_pm_notifier(&di->std_event_nb);
	if (di->rtc_dev != NULL)
		rtc_class_close(di->rtc_dev);
fail_free_gpio:
	gpio_free(di->gpio_boot0);
	gpio_free(di->gpio_reset);
fail_free_mem:
	devm_kfree(&client->dev, di);
	return -EPERM;
}

static int mcu_hc32l110_remove(struct i2c_client *client)
{
	struct mcu_hc32l110_device_info *di = i2c_get_clientdata(client);
	if (!di)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_CONNECT, &di->usb_event_nb);
	unregister_reboot_notifier(&di->event_nb);
	unregister_pm_notifier(&di->std_event_nb);
	rtc_class_close(di->rtc_dev);
	gpio_free(di->gpio_boot0);
	gpio_free(di->gpio_reset);
	devm_kfree(&client->dev, di);
	return 0;
}

static const struct i2c_device_id mcu_hc32l110_idtable[] = {
	{"huawei,mcu-hc32", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, mcu_hc32l110_idtable);

static const struct of_device_id mcu_hc32l110_of_match[] = {
	{.compatible = "huawei,mcu-hc32", .data = NULL},
	{},
};

static struct i2c_driver mcu_hc32l110_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "hw_mcu_hc32l110",
		.of_match_table = mcu_hc32l110_of_match,
	},
	.probe = mcu_hc32l110_probe,
	.remove	= mcu_hc32l110_remove,
	.id_table = mcu_hc32l110_idtable,
};

static int __init mcu_driver_init(void)
{
	hwlog_info("mcu driver init!\n");
	return i2c_add_driver(&mcu_hc32l110_driver);
}

static void __exit mcu_driver_exit(void)
{
	hwlog_info("mcu driver exit!\n");
	return i2c_del_driver(&mcu_hc32l110_driver);
}

late_initcall_sync(mcu_driver_init);
module_exit(mcu_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mcu hc32l110 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
