/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hw_pogopin_sw.c
 *
 * huawei pogopin driver
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
#include <platform_include/cee/linux/adc.h>
#include <huawei_platform/usb/hw_pogopin_sw.h>
#include <huawei_platform/log/hwlog_kernel.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_pogopin_sw
#define HW_POGO_SW_TTY_MAX_COUNT        50
HWLOG_REGIST();

static struct hw_pogopin_sw_info *g_hw_pogopin_sw_di;

enum hw_pogopin_sw_sysfs_type {
	HW_POGOPIN_SW_SYSFS_CHAGE_OPEN = 0,
	HW_POGOPIN_SW_SYSFS_BT_MAC_ADDR,
};

struct blocking_notifier_head g_hw_pogopin_sw_evt_nb;
BLOCKING_NOTIFIER_HEAD(g_hw_pogopin_sw_evt_nb);

struct hw_pogopin_sw_acc_kb_info g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_END] = {
	{ "DEVICENO", "0" },
	{ "DEVICESTATE", "UNKNOWN" },
	{ "DEVICEMAC", "FF:FF:FF:FF:FF:FF" },
	{ "DEVICEMODELID", "000000" },
	{ "DEVICESUBMODELID", "00" },
	{ "DEVICEVERSION", "00" },
	{ "DEVICEBUSINESS", "00" },
	{ "DEVICESLEADDRFLAG", "0" },
};

u8 g_msg_bt_mac_addr[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x57, 0xA8
};

static u8 g_msg_require_type[] = {
	0xFF, 0x8A, 0xA8
};

static void hw_pogopin_sw_tty_kclose(void);
int hw_pogopin_sw_adc_sampling(struct hw_pogopin_sw_info *di);
static irqreturn_t hw_pogopin_sw_int_handler(int irq, void *_di);
static int pogopin_detect_open(void);

static DEFINE_MUTEX(hw_pogopin_sw_tty_mutex);
static struct hw_pogopin_sw_info *hw_pogopin_sw_get_dev_info(void)
{
	if (!g_hw_pogopin_sw_di) {
		hwlog_info("g_hw_pogopin_sw_di is null\n");
		return NULL;
	}

	return g_hw_pogopin_sw_di;
}

static ssize_t hw_pogopin_sw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t hw_pogopin_sw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info hw_pogopin_sw_sysfs_tbl[] = {
	power_sysfs_attr_rw(hw_pogopin_sw, 0644, HW_POGOPIN_SW_SYSFS_CHAGE_OPEN, chage_open),
	power_sysfs_attr_rw(hw_pogopin_sw, 0644, HW_POGOPIN_SW_SYSFS_BT_MAC_ADDR, bt_mac_addr),
};

static struct attribute *hw_pogopin_sw_sysfs_attrs[ARRAY_SIZE(hw_pogopin_sw_sysfs_tbl) + 1];

static const struct attribute_group hw_pogopin_sw_sysfs_attr_group = {
	.attrs = hw_pogopin_sw_sysfs_attrs,
};

static void hw_pogopin_sw_vcc_ctrl(int enable)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di || !di->hw_pogopin_sw_vcc_en) {
		hwlog_info("hw_pogopin_sw_vcc_ctrl invalid di\n");
		return;
	}

	HWDUBAI_LOGE("DUBAI_TAG_POGOPIN_REVERSE_CHARGE", "status=%d type=%d", enable, ACC_POGO_DEV_NO_KB);
	gpio_set_value(di->hw_pogopin_sw_vcc_en, enable);
	hwlog_info("hw_pogopin_sw vcc : %d\n", enable);
}

static bool hw_pogopin_sw_need_drop_msg(const unsigned char *cp, int count)
{
	if (!cp)
		return false;

	// Filter out initial and final loopback data
	if (((cp[ACC_BT_MAC_CP_FRONT] == g_msg_bt_mac_addr[ACC_BT_MAC_CP_FRONT]) &&
		(cp[ACC_BT_MAC_CP_LATER] == g_msg_bt_mac_addr[ACC_BT_MAC_CP_LATER])) &&
		(count == HW_POGO_SW_CALL_INFO_LEN))
		return true;

	if (((cp[ACC_SUPPORT_SLE_CP_FRONT] == g_msg_require_type[ACC_SUPPORT_SLE_CP_FRONT]) &&
		(cp[ACC_SUPPORT_SLE_CP_LATER] == g_msg_require_type[ACC_SUPPORT_SLE_CP_LATER])) &&
		(count == ARRAY_SIZE(g_msg_require_type)))
		return true;

	return false;
}

static struct power_sysfs_attr_info *hw_pogopin_sw_sysfs_field_lookup(const char *name)
{
	int i;
	int limit = ARRAY_SIZE(hw_pogopin_sw_sysfs_tbl);
	int len;

	if (!name) {
		hwlog_err("name is null\n");
		return NULL;
	}

	len = strlen(name);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name, hw_pogopin_sw_sysfs_tbl[i].attr.attr.name, len))
			break;
	}

	if (i >= limit)
		return NULL;

	return &hw_pogopin_sw_sysfs_tbl[i];
}

static int hw_pogopin_sw_sysfs_create_group(struct device *dev)
{
	hwlog_info("%s: create begin:\n", __func__);
	power_sysfs_init_attrs(hw_pogopin_sw_sysfs_attrs,
		hw_pogopin_sw_sysfs_tbl, ARRAY_SIZE(hw_pogopin_sw_sysfs_tbl));

	power_sysfs_create_link_group("hw_power", "charger", "pogopin",
		dev, &hw_pogopin_sw_sysfs_attr_group);

	hwlog_info("[%s] end\n", __func__);

	return 0;
}

static void hw_pogopin_sw_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "pogopin",
		dev, &hw_pogopin_sw_sysfs_attr_group);
}

static ssize_t hw_pogopin_sw_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	int len = 0;

	info = hw_pogopin_sw_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("%s info or di is null\n", __func__);
		return -EINVAL;
	}

	if (!buf) {
		hwlog_err("buf is null\n");
		return -EINVAL;
	}

	switch (info->name) {
	case HW_POGOPIN_SW_SYSFS_CHAGE_OPEN:
		hwlog_info("%s: read succ\n", __func__);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", gpio_get_value(di->hw_pogopin_sw_vcc_en));
	default:
		hwlog_err("%s: NO THIS NODE:%d\n", __func__, info->name);
		break;
	}

	return len;
}

static int hw_pogopin_sw_ana_bt_mac(const char *buf, int len)
{
	int i = 0;
	int j = 0;
	long val = 0;
	char hw_pogopin_sw_mac_addr[ACC_BT_MAC_PI_LEN] = {0};

	for (i = 0; i < len; i++) {
		if (snprintf_s(hw_pogopin_sw_mac_addr, ACC_BT_MAC_PI_LEN, ACC_BT_MAC_PI_LEN - 1, "%c%c",
			buf[i], buf[i + 1]) < 0) {
			hwlog_err("snprintf_s hw_pogopin_sw_mac_addr fail\n");
			return -EINVAL;
		}
		if (kstrtol(hw_pogopin_sw_mac_addr, POWER_BASE_HEX, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		g_msg_bt_mac_addr[j] = val;
		j = j + 1;
		i = i + 1;
	}

	return 0;
}

static ssize_t hw_pogopin_sw_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	long val = 0;
	int len = 0;

	info = hw_pogopin_sw_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("%s info or di is null\n", __func__);
		return -EINVAL;
	}

	switch (info->name) {
	case HW_POGOPIN_SW_SYSFS_CHAGE_OPEN:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0) {
			hwlog_err("%s: val is not valid\n", __func__);
			return -EINVAL;
		}
		if (val == HIGH && (gpio_get_value(di->hw_pogopin_sw_vcc_en) != HIGH) &&
			!di->hw_pogopin_sw_working) {
			di->hw_pogopin_sw_open_flag = true;
			hw_pogopin_sw_int_handler(di->hw_pogopin_sw_int_irq, di);
		} else if (val == LOW) {
			hw_pogopin_sw_vcc_ctrl(val);
		} else {
			hwlog_err("%s: val is not HIGH/LOW\n", __func__);
		}
		break;
	case HW_POGOPIN_SW_SYSFS_BT_MAC_ADDR:
		len = strlen(buf);
		if (len != ACC_BT_MAC_LEN) {
			hwlog_err("%s: buf len error is %d\n", __func__, len);
			return -EINVAL;
		}
		if (hw_pogopin_sw_ana_bt_mac(buf, len))
			hwlog_err("%s: hw_pogopin_sw_ana_bt_mac error\n", __func__);
		break;
	default:
		hwlog_err("%s: NO THIS NODE:%d\n", __func__, info->name);
		break;
	}

	return count;
}

void hw_pogopin_sw_event_notify(enum hw_pogopin_sw_event event)
{
	hwlog_info("%s event:%d\n", __func__, event);
	blocking_notifier_call_chain(&g_hw_pogopin_sw_evt_nb, event, NULL);
}

void hw_pogopin_sw_event_notify_with_data(enum hw_pogopin_sw_event event, u32 data)
{
	hwlog_info("%s event:%d\n", __func__, event);
	blocking_notifier_call_chain(&g_hw_pogopin_sw_evt_nb, event, &data);
}

int hw_pogopin_sw_event_notifier_register(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_register(&g_hw_pogopin_sw_evt_nb, nb);
}

int hw_pogopin_sw_event_notifier_unregister(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_unregister(&g_hw_pogopin_sw_evt_nb, nb);
}

int hw_pogopin_sw_adc_sampling(struct hw_pogopin_sw_info *di)
{
	int i;
	int avgvalue;
	int vol_value;
	int sum = ADC_SAMPLING_SUM_DEFAULT;
	int retry = ADC_SAMPLING_RETRY_TIMES;
	int sample_cnt = ADC_SAMPLE_COUNT_DEFAULT;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!di) {
		hwlog_err("acc sampling invalid di\n");
		return GET_ADC_FAIL;
	}

	dev = &di->pdev->dev;
	np = dev->of_node;
	if (di->hw_pogopin_sw_adc_channel < 0) {
		do {
			if (of_property_read_u32(np, "hw_pogopin_sw_adc_channel", &di->hw_pogopin_sw_adc_channel)) {
				hwlog_info("hw_pogopin_sw_adc_channel get failed:%d\n", retry);
				di->hw_pogopin_sw_adc_channel = HW_POGO_SW_INVALID_CHANNEL;
				/* 10: get adc_channel failed, interval and retry */
				msleep(10);
			} else {
				hwlog_info("hw_pogopin_sw_adc channel is ready\n");
				break;
			}
		} while (retry-- > 0);
	}

	if (di->hw_pogopin_sw_adc_channel == HW_POGO_SW_INVALID_CHANNEL) {
		hwlog_err("hw_pogopin_sw_adc channel not exist\n");
		return GET_ADC_FAIL;
	}

	for (i = 0; i < VBATT_AVR_MAX_COUNT; i++) {
		vol_value = lpm_adc_get_value(di->hw_pogopin_sw_adc_channel);
		hwlog_info("%s, vol:%d\n", __func__, vol_value);
		udelay(SMAPLING_OPTIMIZE);

		/* 0: vol_value is negative */
		if (vol_value < 0) {
			hwlog_info("the value from adc is error\n");
			return GET_ADC_FAIL;
		}

		sum += vol_value;
		sample_cnt++;
	}

	avgvalue = sum / sample_cnt;
	hwlog_info("the average voltage of adc is %d\n", avgvalue);

	return avgvalue;
}

static void hw_pogopin_sw_timer_function(struct timer_list *data)
{
	hw_pogopin_sw_vcc_ctrl(LOW);
	hwlog_info("hw_pogopin_sw_timer_function disable vcc\n");
}

int hw_pogopin_sw_check_uart_status_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	int hw_pogopin_sw_gpio_value;

	if (!di) {
		hwlog_err("g_hw_pogopin_sw_di is null\n");
		return NOTIFY_OK;
	}

	switch (event) {
	case HW_POGOPIN_SW_UART_STATUS_OK:
		msleep(100);
		hw_pogopin_sw_tty_kclose();
		hwlog_info("enable_irq HW_POGOPIN_SW_UART_STATUS_OK\n");
		break;
	case HW_POGOPIN_SW_UART_STATUS_TIMEOUT:
	case HW_POGOPIN_SW_UART_STATUS_FAIL:
		hw_pogopin_sw_vcc_ctrl(LOW);
		hw_pogopin_sw_tty_kclose();
		hwlog_info("enable_irq HW_POGOPIN_SW_UART_STATUS_FAIL\n");
		break;
	case HW_POGOPIN_SW_ADC_STATUS_FAIL:
		hwlog_info("enable_irq HW_POGOPIN_SW_ADC_STATUS_FAIL\n");
		break;
	case HW_POGOPIN_PING_SUCC_SPARKLINK:
	case HW_POGOPIN_DISCONNECT_SPARKLINK:
		return NOTIFY_OK;
	default:
		break;
	}

	di->handshak_flag = false;
	di->hw_pogopin_sw_acc_info_num = 0;
	hw_pogopin_sw_gpio_value = gpio_get_value(di->hw_pogopin_sw_int_gpio);
	hwlog_info("notifier_call, int_gpio=%d\n", hw_pogopin_sw_gpio_value);

	if (di->hw_pogopin_sw_gpio_status != hw_pogopin_sw_gpio_value) {
		hwlog_info("di->hw_pogopin_sw_gpio_status != hw_pogopin_sw_gpio_value\n");
		schedule_work(&di->hw_pogopin_sw_work);
		return NOTIFY_OK;
	}
	enable_irq(di->hw_pogopin_sw_int_irq);
	di->hw_pogopin_sw_working = false;
	power_wakeup_unlock(di->wakelock, false);
	return NOTIFY_OK;
}

static struct tty_struct *pogopin_tty_kopen(const char *dev_name)
{
	struct tty_struct *tty = NULL;
	int ret;
	dev_t dev_no;

	hwlog_info("%s\n", __func__);

	ret = tty_dev_name_to_number(dev_name, &dev_no);
	if (ret != 0) {
		hwlog_info("can't found tty:%s ret=%d\n", dev_name, ret);
		return NULL;
	}

	/* open tty */
	tty = tty_kopen(dev_no);
	if (IS_ERR_OR_NULL(tty)) {
		hwlog_info("open tty %s failed ret=%d\n", dev_name, PTR_ERR_OR_ZERO(tty));
		return NULL;
	}

	if (tty->ops->open) {
		ret = tty->ops->open(tty, NULL);
	} else {
		hwlog_info("tty->ops->open is NULL\n");
		ret = -ENODEV;
	}

	if (ret) {
		tty_unlock(tty);
		return NULL;
	} else {
		return tty;
	}
}

static void pogopin_ktty_set_termios(struct tty_struct *tty, long baud_rate)
{
	struct ktermios ktermios;

	ktermios = tty->termios;
	/* close soft flowctrl */
	ktermios.c_iflag &= ~IXON;
	/* set uart cts/rts flowctrl */
	ktermios.c_cflag &= ~CRTSCTS;
	/* set csize */
	ktermios.c_cflag &= ~(CSIZE);
	ktermios.c_cflag |= CS8;
	/* set uart baudrate */
	ktermios.c_cflag &= ~CBAUD;
	ktermios.c_cflag |= BOTHER;
	tty_termios_encode_baud_rate(&ktermios, baud_rate, baud_rate);
	tty_set_termios(tty, &ktermios);
	hwlog_info("set baud_rate=%d, except=%d\n", (int)tty_termios_baud_rate(&tty->termios), (int)baud_rate);
}

static void hw_pogopin_sw_send_msg(unsigned char *buf, int len)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("send msg invalid di\n");
		return;
	}

	hwlog_info("pogopin start send msg to kb\n");
	if (di->hw_pogopin_sw_tty > 0) {
		mutex_lock(&hw_pogopin_sw_tty_mutex);
		di->hw_pogopin_sw_tty->ops->write(di->hw_pogopin_sw_tty, buf, len);
		mutex_unlock(&hw_pogopin_sw_tty_mutex);
	}
}

static int pogopin_detect_open(void)
{
	int ret;
	struct hw_pogopin_sw_ldisc_data *hw_pogopin_sw_ldisc_data;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di || di->hw_pogopin_sw_tty) {
		hwlog_err("pogopin_detect_open invalid di\n");
		return -EINVAL;
	}

	di->hw_pogopin_sw_tty = pogopin_tty_kopen(di->tty_name);
	if (!di->hw_pogopin_sw_tty) {
		hwlog_info("pogopin open tty error\n");
		return -EINVAL;
	}
	pogopin_ktty_set_termios(di->hw_pogopin_sw_tty, UART_DIRECT_SPEED);

	tty_unlock(di->hw_pogopin_sw_tty);

	/* set line ldisc */
	ret = tty_set_ldisc(di->hw_pogopin_sw_tty, N_HW_POGOPIN);
	if (ret != 0) {
		hwlog_info("%s failed to set ldisc on tty, ret:%d", __func__, ret);
		return -EINVAL;
	}

	hw_pogopin_sw_ldisc_data = kmalloc(sizeof(*hw_pogopin_sw_ldisc_data), GFP_KERNEL);
	if (!hw_pogopin_sw_ldisc_data)
		return -EINVAL;

	init_completion(&hw_pogopin_sw_ldisc_data->completion);
	hw_pogopin_sw_ldisc_data->buf_free = true;
	di->hw_pogopin_sw_tty->disc_data = hw_pogopin_sw_ldisc_data;
	hwlog_info("pogopin tty open ok\n");

	return 0;
}

static void hw_pogopin_sw_clear_acc_uevent_data(void)
{
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_NO].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%u", ACC_INFO_NO_LEN) < 0)
		hwlog_err("snprintf_s acc_info_no fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_STATE].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "UNKNOWN") < 0)
		hwlog_err("snprintf_s acc_info_state fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_MAC].value, ACC_VALUE_MAX_LEN,
		ACC_VALUE_MAX_LEN - 1, "%s", "FF:FF:FF:FF:FF:FF") < 0)
		hwlog_err("snprintf_s acc_info_mac fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_MODEL_ID].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "000000") < 0)
		hwlog_err("snprintf_s acc_info_model_id fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "00") < 0)
		hwlog_err("snprintf_s acc_info_submodel_id fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_VERSION].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "00") < 0)
		hwlog_err("snprintf_s acc_info_version fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_BUSINESS].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "00") < 0)
		hwlog_err("snprintf_s acc_info__business fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_SLE].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%s", "0") < 0)
		hwlog_err("snprintf_s acc_info_sle fail\n");
}

static void hw_pogopin_sw_send_acc_uevent_data(void)
{
	int i;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("acc info invalid di\n");
		return;
	}

	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_NO].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%u", 2) < 0)
		hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_NO fail\n");
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_STATE].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%u", di->hw_pogopin_sw_type) < 0)
		hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_STATE fail\n");
	for (i = 0; i < HW_POGOPIN_SW_ACC_INFO_END_FLAG; i++) {
		if (i == HW_POGOPIN_SW_ACC_INFO_MAC_FLAG) {
			if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_MAC].value, ACC_VALUE_MAX_LEN,
				ACC_VALUE_MAX_LEN - 1, "%02x:%02x:%02x:%02x:%02x:%02x", di->hw_pogopin_sw_acc_info[i + 5],
				di->hw_pogopin_sw_acc_info[i + 4], di->hw_pogopin_sw_acc_info[i + 3], di->hw_pogopin_sw_acc_info[i + 2],
				di->hw_pogopin_sw_acc_info[i + 1], di->hw_pogopin_sw_acc_info[i]) < 0)
				hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_MAC fail\n");
			i = i + 5; // skip 5 bytes
			continue;
		} else if (i == (HW_POGOPIN_SW_ACC_INFO_MODEL_ID_FLAG)) {
			if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_MODEL_ID].value,
				ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%02x%02x%02x", di->hw_pogopin_sw_acc_info[i],
				di->hw_pogopin_sw_acc_info[i + 1], di->hw_pogopin_sw_acc_info[i + 2]) < 0)
				hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_MODEL_ID fail\n");
			i = i + 2; // skip 2 bytes
			continue;
		} else if (i == (HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID_FLAG)) {
			if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID].value,
				ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%02x", di->hw_pogopin_sw_acc_info[i]) < 0)
				hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_SUBMODEL_ID fail\n");
		} else if (i == (HW_POGOPIN_SW_ACC_INFO_VERSION_FLAG)) {
			if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_VERSION].value,
				ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%02x", di->hw_pogopin_sw_acc_info[i]) < 0)
				hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_VERSION fail\n");
		} else if (i == (HW_POGOPIN_SW_ACC_INFO_BUSINESS_FLAG)) {
			if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_BUSINESS].value,
				ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%02x", di->hw_pogopin_sw_acc_info[i]) < 0)
				hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_BUSINESS fail\n");
		}
	}
	if (snprintf_s(g_hw_pogopin_sw_kb_acc_info[HW_POGOPIN_SW_ACC_INFO_SLE].value,
		ACC_VALUE_MAX_LEN, ACC_VALUE_MAX_LEN - 1, "%x", di->acc_sle_flag) < 0)
		hwlog_err("snprintf_s HW_POGOPIN_SW_ACC_INFO_SLE fail\n");
}

static bool hw_pogopin_sw_parse_acc_uevent_data(const unsigned char *g_autopairinfo, int count)
{
	int i;
	int data_len;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("acc info invalid di\n");
		return false;
	}

	i = di->handshak_offset;
	if ((count - i) <= 0) {
		hwlog_err("acc data len error");
		return false;
	}
	data_len = (count - i) > (ACC_VALUE_MAX_LEN - di->hw_pogopin_sw_acc_info_num) ?
		(ACC_VALUE_MAX_LEN - di->hw_pogopin_sw_acc_info_num) : (count - i);
	if (memcpy_s((di->hw_pogopin_sw_acc_info + di->hw_pogopin_sw_acc_info_num), ACC_VALUE_MAX_LEN,
		(g_autopairinfo + i), data_len) != EOK)
		hwlog_err("memcpy_s hw_pogopin_sw_acc_info fail\n");
	di->hw_pogopin_sw_acc_info_num += data_len;
	hwlog_info("hw_pogopin_sw_acc_info_num is %d\n", di->hw_pogopin_sw_acc_info_num);
	if (di->hw_pogopin_sw_acc_info_num >= ACC_INFO_LEN) {
		di->data_complete = true;
		di->handshak_flag = false;
		di->hw_pogopin_sw_acc_info_num = 0;
	} else {
		di->data_complete = false;
		return true;
	}

	hw_pogopin_sw_send_acc_uevent_data();
	hw_pogopin_sw_send_msg(g_msg_bt_mac_addr, ARRAY_SIZE(g_msg_bt_mac_addr));

	hwlog_info("accessory notify uevent end\n");
	return true;
}

static int hw_pogopin_sw_parse_require_type_msg(const unsigned char *cp, int count)
{
	int tx_type = 0;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("invalid di\n");
		return FALSE;
	}

	if (count != 2) {
		hwlog_err("invalid acc require info\n");
		return FALSE;
	}

	tx_type = (cp[0] << HW_POGO_SW_BITS_PER_BYTE) | cp[1];
	if (tx_type == HW_POGO_SW_TYPE_SLE) {
		hwlog_info("0x88AA require type succ\n");
		di->acc_sle_flag = di->support_sle ? 1 : 0;
		g_msg_require_type[0] = di->support_sle;
		hw_pogopin_sw_send_msg(g_msg_require_type, ARRAY_SIZE(g_msg_require_type));
		return TRUE;
	}

	hwlog_info("0x88AA require type fail\n");
	return FALSE;
}

static int hw_pogopin_sw_parse_handshake_msg(const unsigned char *cp, int count)
{
	int tx_id = 0;
	int i;
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("invalid di\n");
		return FALSE;
	}

	if (count < 0 || count > HW_POGO_SW_TTY_MAX_COUNT) {
		hwlog_err("invalid acc info\n");
		return FALSE;
	}

	if (di->handshak_flag) {
		hwlog_info("already handshak\n");
		di->handshak_offset = 0;
		return TRUE;
	}

	di->handshak_offset = 0;
	for (i = 0; i < count - 1; i++) {
		if (cp[i] == (HW_POGO_SW_HANDSHAKE_ID >> HW_POGO_SW_BITS_PER_BYTE)) {
			tx_id = (cp[i] << HW_POGO_SW_BITS_PER_BYTE) | cp[i + 1];
			if (tx_id == HW_POGO_SW_HANDSHAKE_ID) {
				di->handshak_offset = i + 2;
				hwlog_info("0x8866 handshake succ\n");
				di->handshak_flag = true;
				return TRUE;
			}
		}
	}

	hwlog_info("0x8866 handshake fail\n");
	return FALSE;
}

static void pogopin_tty_receive(struct tty_struct *tty,
	const unsigned char *cp, char *fp, int count)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	struct hw_pogopin_sw_ldisc_data *hw_pogopin_sw_ldisc_data = NULL;

	hwlog_info("%s pogopin_tty_receive in\n", __func__);

	if (!cp || !di || !di->hw_pogopin_sw_tty || !di->hw_pogopin_sw_tty->disc_data) {
		hwlog_err("invalid parameter\n");
		return;
	}
	hw_pogopin_sw_ldisc_data = di->hw_pogopin_sw_tty->disc_data;
	/* abandon tx called back msg */
	if (hw_pogopin_sw_need_drop_msg(cp, count)) {
		hwlog_info("%s abandon tx called back msg\n", __func__);
		return;
	}

	if (hw_pogopin_sw_parse_require_type_msg(cp, count)) {
		hwlog_info("%s recived type msg, return\n", __func__);
		return;
	}

	/* Make sure to parse acc msg after the handshake */
	if (!(hw_pogopin_sw_parse_handshake_msg(cp, count))) {
		hwlog_info("%s not handshake msg, return\n", __func__);
		return;
	}

	if (!hw_pogopin_sw_parse_acc_uevent_data(cp, count)) {
		hwlog_err("parse acc data error");
		return;
	}

	if (!di->data_complete)
		return;

	if (!hw_pogopin_sw_ldisc_data->buf_free) {
		hwlog_info("%s hw_pogopin_sw_ldisc_data->buf_free is null\n", __func__);
		return;
	}

	/* Make sure the consumer has read buf before we have seen
	 * buf_free == true and overwrite buf
	 */
	mb();

	hw_pogopin_sw_ldisc_data->buf_free = false;
	complete(&hw_pogopin_sw_ldisc_data->completion);
	return;
}

static void pogopin_detect_acc_info_work(struct work_struct *work)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	struct hw_pogopin_sw_ldisc_data *hw_pogopin_sw_ldisc_data = NULL;

	if (!di || !di->hw_pogopin_sw_vcc_en) {
		hwlog_info("pogopin_detect_acc_info_work invalid di\n");
		return;
	}

	hw_pogopin_sw_ldisc_data = di->hw_pogopin_sw_tty->disc_data;
	hwlog_info("[%s]enter to get acc info\n", __func__);

	if (wait_for_completion_timeout(&hw_pogopin_sw_ldisc_data->completion,
		msecs_to_jiffies(di->uart_timeout)) == 0) {
			hwlog_info("%s wait for completion time out\n", __func__);
			hw_pogopin_sw_event_notify(HW_POGOPIN_SW_UART_STATUS_TIMEOUT);
			return;
	}

	di->hw_pogopin_sw_type = CONNECTED;
	hw_pogopin_sw_report_acc_info(g_hw_pogopin_sw_kb_acc_info, di->hw_pogopin_sw_type);
	mb();
	hw_pogopin_sw_ldisc_data->buf_free = true;
	tty_schedule_flip(di->hw_pogopin_sw_tty->port);
	hw_pogopin_sw_event_notify(HW_POGOPIN_SW_UART_STATUS_OK);
}

static void pogopin_detect_check_work(struct work_struct *work)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	if (!di) {
		hwlog_err("invalid di\n");
		return;
	}

	hw_pogopin_sw_int_handler(di->hw_pogopin_sw_int_irq, di);
}

static int pogopin_tty_open(struct tty_struct *tty)
{
	hwlog_info("%s enter\n", __func__);

	mutex_lock(&hw_pogopin_sw_tty_mutex);
	/* don't do an wakeup for now */
	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	/* set mem already allocated */
	tty->receive_room = PUBLIC_BUF_MAX;
	/* Flush any pending characters in the driver and discipline. */
	tty_ldisc_flush(tty);
	tty_driver_flush_buffer(tty);
	mutex_unlock(&hw_pogopin_sw_tty_mutex);

	return 0;
}

static void hw_pogopin_sw_tty_kclose(void)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();

	hwlog_info("%s\n", __func__);

	/* close tty */
	if (!di || !di->hw_pogopin_sw_tty) {
		hwlog_info("di or tty is null, ignore\n");
		return;
	}

	tty_lock(di->hw_pogopin_sw_tty);
	if (di->hw_pogopin_sw_tty->ops->close) {
		di->hw_pogopin_sw_tty->ops->close(di->hw_pogopin_sw_tty, NULL);
	} else {
		hwlog_info("tty->ops->close is null\n");
	}
	tty_unlock(di->hw_pogopin_sw_tty);

	tty_kclose(di->hw_pogopin_sw_tty);
	di->hw_pogopin_sw_tty = NULL;
}

static void pogopin_tty_close(struct tty_struct *tty)
{
	hwlog_info("pogopin_tty_close");
}

static void pogopin_tty_wakeup(struct tty_struct *tty)
{
	hwlog_info("pogopin_tty_wakeup");
}

static void pogopin_tty_flush_buffer(struct tty_struct *tty)
{
	hwlog_info("pogopin_tty_flush_buffer");
}

static void pogopin_detect_id_irq_work(struct work_struct *work)
{
	struct hw_pogopin_sw_info *di = hw_pogopin_sw_get_dev_info();
	int hw_pogopin_sw_gpio_value;
	int avgvalue;

	if (!di)
		return;

	msleep(HW_POGO_SW_DETECT_DEBOUNCE_MSECS);
	hw_pogopin_sw_gpio_value = gpio_get_value(di->hw_pogopin_sw_int_gpio);
	hwlog_info("pogo detect gpio changed, int_gpio=%d\n", hw_pogopin_sw_gpio_value);

	if (!di->hw_pogopin_sw_open_flag) {
		if (di->hw_pogopin_sw_gpio_status == hw_pogopin_sw_gpio_value) {
			hwlog_info("di->hw_pogopin_sw_gpio_status == hw_pogopin_sw_gpio_value\n");
			goto exit;
		}
		di->hw_pogopin_sw_gpio_status = hw_pogopin_sw_gpio_value;
	}

	di->hw_pogopin_sw_open_flag = false;
	if (hw_pogopin_sw_gpio_value == HIGH) {
		if (di->hw_pogopin_sw_type == CONNECTED || di->hw_pogopin_sw_type == PING_SUCC) {
			di->hw_pogopin_sw_type = DISCONNECTED;
			hw_pogopin_sw_event_notify(HW_POGOPIN_DISCONNECT_SPARKLINK);
		}
		hw_pogopin_sw_vcc_ctrl(LOW);
		hw_pogopin_sw_report_acc_info(g_hw_pogopin_sw_kb_acc_info, di->hw_pogopin_sw_type);
		hw_pogopin_sw_clear_acc_uevent_data();
		hwlog_info("enable_irq hw_pogopin_sw_int_irq HIGH\n");
	} else {
		avgvalue = hw_pogopin_sw_adc_sampling(di);
		if (avgvalue >= KB_ONLINE_MIN_ADC_LIMIT && avgvalue <= KB_ONLINE_MAX_ADC_LIMIT) {
			hw_pogopin_sw_clear_acc_uevent_data();
			di->hw_pogopin_sw_type = PING_SUCC;
			hw_pogopin_sw_event_notify(HW_POGOPIN_PING_SUCC_SPARKLINK);
			di->acc_sle_flag = 0;
			hw_pogopin_sw_report_acc_info(g_hw_pogopin_sw_kb_acc_info, di->hw_pogopin_sw_type);
			if (!pogopin_detect_open()) {
				schedule_delayed_work(&di->detect_acc_info_work, 0);
				hw_pogopin_sw_vcc_ctrl(HIGH);
				mod_timer(&di->hw_pogopin_sw_timer, jiffies + msecs_to_jiffies(HW_POGO_SW_VCC_ENABLE_MSECS));
				return;
			}
		}
		hw_pogopin_sw_event_notify(HW_POGOPIN_SW_ADC_STATUS_FAIL);
		return;
	}

exit:
	enable_irq(di->hw_pogopin_sw_int_irq);
	di->hw_pogopin_sw_working = false;
	power_wakeup_unlock(di->wakelock, false);
}

static irqreturn_t hw_pogopin_sw_int_handler(int irq, void *_di)
{
	struct hw_pogopin_sw_info *di = _di;
	int hw_pogopin_sw_gpio_value;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	hwlog_info("[%s] enter\n", __func__);

	hw_pogopin_sw_gpio_value = gpio_get_value(di->hw_pogopin_sw_int_gpio);
	hwlog_info("[%s] pogo detect gpio changed, int_gpio=%d\n",
			__func__, hw_pogopin_sw_gpio_value);

	if (!di->hw_pogopin_sw_open_flag) {
		if (di->hw_pogopin_sw_gpio_status == hw_pogopin_sw_gpio_value) {
			hwlog_info("di->hw_pogopin_sw_gpio_status == hw_pogopin_sw_gpio_value\n");
			return IRQ_HANDLED;
		}
	}
	disable_irq_nosync(di->hw_pogopin_sw_int_irq);
	di->hw_pogopin_sw_working = true;
	power_wakeup_lock(di->wakelock, false);
	schedule_work(&di->hw_pogopin_sw_work);

	return IRQ_HANDLED;
}

static int hw_pogopin_sw_request_irq(struct hw_pogopin_sw_info *di)
{
	int ret;

	di->hw_pogopin_sw_int_irq = gpio_to_irq(di->hw_pogopin_sw_int_gpio);
	if (di->hw_pogopin_sw_int_irq < 0) {
		hwlog_err("gpio map to irq fail\n");
		return -EINVAL;
	}
	hwlog_info("hw_pogopin_sw_int_irq=%d\n", di->hw_pogopin_sw_int_irq);

	ret = request_irq(di->hw_pogopin_sw_int_irq, hw_pogopin_sw_int_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"huawei_pogopin_sw_int", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		di->hw_pogopin_sw_int_irq = -1;
	}

	return ret;
}

static int hw_pogopin_sw_set_gpio_direction_irq(struct hw_pogopin_sw_info *di)
{
	int ret;

	ret = gpio_direction_output(di->hw_pogopin_sw_vcc_en, 0);
	if (ret < 0) {
		hwlog_err("gpio set output fail\n");
		return ret;
	}

	ret = gpio_direction_input(di->hw_pogopin_sw_int_gpio);
	if (ret < 0) {
		hwlog_err("gpio set input fail\n");
		return ret;
	}

	return hw_pogopin_sw_request_irq(di);
}

static void hw_pogopin_sw_free_irqs(struct hw_pogopin_sw_info *di)
{
	if (!di)
		return;

	free_irq(di->hw_pogopin_sw_int_irq, di);
}

static int hw_pogopin_sw_parse_gpio_dts(struct hw_pogopin_sw_info *di,
	struct device_node *np)
{
	int ret = -EINVAL;

	di->hw_pogopin_sw_int_gpio = of_get_named_gpio(np, "huawei_pogopin_sw_int_gpio", 0);
	hwlog_info("hw_pogopin_sw_int_gpio=%d\n", di->hw_pogopin_sw_int_gpio);

	if (!gpio_is_valid(di->hw_pogopin_sw_int_gpio)) {
		hwlog_err("hw_pogopin_sw_int_gpio gpio is not valid\n");
		return ret;
	}

	di->hw_pogopin_sw_vcc_en = of_get_named_gpio(np, "huawei_pogopin_sw_vcc_en_gpio", 0);
	hwlog_info("hw_pogopin_sw_vcc_en=%d\n", di->hw_pogopin_sw_vcc_en);

	if (!gpio_is_valid(di->hw_pogopin_sw_vcc_en)) {
		hwlog_err("hw_pogopin_sw_vcc_en gpio is not valid\n");
		return ret;
	}

	return 0;
}

static void hw_pogopin_sw_free_common_gpio(struct hw_pogopin_sw_info *di)
{
	if (!di)
		return;

	gpio_free(di->hw_pogopin_sw_int_gpio);
}

static int hw_pogopin_sw_request_common_gpio(struct hw_pogopin_sw_info *di)
{
	int ret;

	ret = gpio_request(di->hw_pogopin_sw_vcc_en, "hw_pogopin_sw_vcc_en");
	if (ret) {
		hwlog_err("hw_pogopin_sw_vcc_en gpio request fail\n");
		goto hw_pogopin_sw_fail_vcc_en;
	}

	ret = gpio_request(di->hw_pogopin_sw_int_gpio, "huawei_pogopin_sw_int");
	if (ret) {
		hwlog_err("gpio request fail\n");
		goto hw_pogopin_sw_fail_int_gpio;
	}

	return 0;

hw_pogopin_sw_fail_int_gpio:
	gpio_free(di->hw_pogopin_sw_int_gpio);
hw_pogopin_sw_fail_vcc_en:
	gpio_free(di->hw_pogopin_sw_vcc_en);

	return ret;
}

static int hw_pogopin_sw_parse_and_request_gpios(struct hw_pogopin_sw_info *di,
	struct device_node *np)
{
	int ret;

	ret = hw_pogopin_sw_parse_gpio_dts(di, np);
	if (ret != 0)
		return ret;

	ret = hw_pogopin_sw_request_common_gpio(di);
	if (ret != 0)
		return ret;

	return 0;
}

static void hw_pogopin_sw_free_gpios(struct hw_pogopin_sw_info *di)
{
	if (!di)
		return;

	hw_pogopin_sw_free_common_gpio(di);
}

static int hw_pogopin_sw_parse_dts(struct hw_pogopin_sw_info *di, struct device_node *np)
{
	if (!di || !np)
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_sle", &di->support_sle, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"uart_timeout", &di->uart_timeout, ACC_TIMEOUT_MSECS);

	return 0;
}

static int hw_pogopin_sw_init_config(struct hw_pogopin_sw_info *di, struct device_node *np)
{
	int ret;

	ret = hw_pogopin_sw_parse_dts(di, np);
	if (ret)
		return ret;

	ret = hw_pogopin_sw_parse_and_request_gpios(di, np);
	if (ret)
		return ret;

	ret = hw_pogopin_sw_set_gpio_direction_irq(di);
	if (ret)
		goto fail_request_gpio;

	return 0;

fail_request_gpio:
	hw_pogopin_sw_free_gpios(di);
	return ret;
}

static struct tty_ldisc_ops g_pogopin_ldisc_ops = {
	.magic = TTY_LDISC_MAGIC,
	.name = "hw_pogopin_tty",
	.open = pogopin_tty_open,
	.close = pogopin_tty_close,
	.receive_buf = pogopin_tty_receive,
	.write_wakeup = pogopin_tty_wakeup,
	.flush_buffer = pogopin_tty_flush_buffer,
	.owner = THIS_MODULE
};
static int hw_pogopin_notify_and_sys_tty(struct device *dev, struct hw_pogopin_sw_info *di, struct device_node *np)
{
	int ret = 0;

	di->hw_pogopin_sw_check_uart_status_nb.notifier_call =
		hw_pogopin_sw_check_uart_status_notifier_call;
	ret = hw_pogopin_sw_event_notifier_register(&di->hw_pogopin_sw_check_uart_status_nb);
	if (ret < 0) {
		hwlog_err("pogopin uartstatus_notifier register failed\n");
		return 0;
	}

	ret = hw_pogopin_sw_sysfs_create_group(dev);
	if (ret)
		return 0;

	ret = of_property_read_string(np, "sw_tty_name", &di->tty_name);
	if (ret < 0) {
		hwlog_err("read tty name fail\n");
		return 0;
	}
	tty_register_ldisc(N_HW_POGOPIN, &g_pogopin_ldisc_ops);

	return 1;
}

static void hw_pogopin_sw_di_init(struct hw_pogopin_sw_info *di)
{
	if (!di)
		return;

	di->hw_pogopin_sw_type = HW_POGOPIN_SW_KB_UNKNOWN;
	di->hw_pogopin_sw_open_flag = false;
	di->hw_pogopin_sw_acc_info_num = 0;
	di->handshak_flag = false;
	di->hw_pogopin_sw_working = false;
	di->hw_pogopin_sw_tty = NULL;
	di->hw_pogopin_sw_adc_channel = HW_POGO_SW_INVALID_CHANNEL;
}

static int hw_pogopin_sw_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct hw_pogopin_sw_info *di = NULL;

	hwlog_info("[%s] enter\n", __func__);
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	platform_set_drvdata(pdev, di);
	g_hw_pogopin_sw_di = di;
	di->pdev = pdev;
	dev = &pdev->dev;
	np = dev->of_node;
	hw_pogopin_sw_di_init(di);

	di->hw_pogopin_sw_gpio_status = -1;
	(void)power_pinctrl_config(&(pdev->dev), "pinctrl-names", 1);

	di->wakelock = power_wakeup_source_register(&pdev->dev, "hw_pogopin_sw_charger_wakelock");
	if (!di->wakelock) {
		ret = -EINVAL;
		goto fail_register_wakeup_source;
	}

	INIT_WORK(&di->hw_pogopin_sw_work, pogopin_detect_id_irq_work);
	INIT_DELAYED_WORK(&di->detect_acc_info_work, pogopin_detect_acc_info_work);
	INIT_DELAYED_WORK(&di->detect_check_work, pogopin_detect_check_work);
	ret = hw_pogopin_sw_init_config(di, np);
	if (ret)
		goto fail_init_config;

	ret = hw_pogopin_notify_and_sys_tty(dev, di, np);
	if (!ret)
		goto fail_create_sysfs;

	timer_setup(&di->hw_pogopin_sw_timer, hw_pogopin_sw_timer_function, 0);
	hwlog_info("[%s] end\n", __func__);
	schedule_delayed_work(&di->detect_check_work, msecs_to_jiffies(HW_POGO_SW_DETECT_WORK_MSECS));
	return 0;

fail_create_sysfs:
	hw_pogopin_sw_free_irqs(di);
	hw_pogopin_sw_free_gpios(di);
fail_init_config:
	power_wakeup_source_unregister(di->wakelock);
fail_register_wakeup_source:
	hw_pogopin_sw_event_notifier_unregister(&di->hw_pogopin_sw_check_uart_status_nb);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hw_pogopin_sw_di = NULL;
	return ret;
}

static int hw_pogopin_sw_remove(struct platform_device *pdev)
{
	struct hw_pogopin_sw_info *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	hw_pogopin_sw_sysfs_remove_group(&pdev->dev);
	hw_pogopin_sw_free_irqs(di);
	hw_pogopin_sw_free_gpios(di);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_hw_pogopin_sw_di = NULL;
	del_timer(&di->hw_pogopin_sw_timer);
	return 0;
}

static const struct of_device_id hw_pogopin_sw_match_table[] = {
	{
		.compatible = "huawei,pogopin_sw",
		.data = NULL,
	},
	{},
};

static struct platform_driver hw_pogopin_sw_driver = {
	.probe = hw_pogopin_sw_probe,
	.remove = hw_pogopin_sw_remove,
	.driver = {
		.name = "huawei,pogopin_sw",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hw_pogopin_sw_match_table),
	},
};

static int __init hw_pogopin_sw_init(void)
{
	return platform_driver_register(&hw_pogopin_sw_driver);
}

static void __exit hw_pogopin_sw_exit(void)
{
	platform_driver_unregister(&hw_pogopin_sw_driver);
}

late_initcall(hw_pogopin_sw_init);
module_exit(hw_pogopin_sw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei hw_pogopin_sw module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
