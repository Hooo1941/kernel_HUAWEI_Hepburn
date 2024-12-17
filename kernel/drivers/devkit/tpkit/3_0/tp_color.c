/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2050 Huawei Technologies Co., Ltd.
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
#include "tp_color.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <huawei_platform/log/hw_log.h>
#include "tpkit_platform_adapter.h"
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
#include "SCP_power_monitor.h"
#endif

#define HWLOG_TAG tp_color
HWLOG_REGIST();


char tp_color_buf[TP_COLOR_BUF_SIZE];
u8 cypress_ts_kit_color[TP_COLOR_SIZE];

static char g_tp_color_string[TP_COLOR_SIZE];

static struct work_struct nv_read_work;
static struct work_struct nv_write_work;
static struct completion nv_read_done;
static struct completion nv_write_done;

/* Ko Isolation Macro */
#ifdef CONFIG_MODULE_KO_TP

// 3_0\qcom_adapter\tpkit_platform_adapter.c
#ifdef CONFIG_HW_NVE
#include <linux/mtd/hw_nve_interface.h>
#endif
#define NV_NUMBER 16
#define VALID_SIZE 15

#define TS_LOG_INFO(x...) _hwlog_info(HWLOG_TAG, ##x)
#define TS_LOG_ERR(x...) _hwlog_err(HWLOG_TAG, ##x)
#define TS_LOG_DEBUG(x...) \
	do { \
		if (g_ts_kit_log_cfg) \
			_hwlog_info(HWLOG_TAG, ##x); \
	} while (0)

int read_tp_color_adapter(char *buf, int buf_size)
{
#ifdef CONFIG_HW_NVE
	int ret;
	struct hw_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hw_nve_direct_access(&user_info);
	if (ret) {
		TS_LOG_ERR("hw_nve_direct_access read error %d\n", ret);
		return ret;
	}
	/* buf_size value is NV_DATA_SIZE 104 */
	strncpy(buf, user_info.nv_data, buf_size - 1);
	TS_LOG_INFO("tp color from nv is %s\n", buf);
#endif
	return 0;
}

int write_tp_color_adapter(const char *buf)
{
#ifdef CONFIG_HW_NVE
	int ret;
	struct hw_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	strncpy(user_info.nv_data, buf, sizeof(user_info.nv_data) - 1);
	ret = hw_nve_direct_access(&user_info);
	if (ret) {
		TS_LOG_ERR("hw_nve_direct_access write error %d\n", ret);
		return ret;
	}
#endif
	return 0;
}

static int parse_tp_color_cmdline(void)
{
	char *pstr = NULL;
	char *dstr = NULL;
	uint32_t tp_color_len;

	hwlog_info("enter\n");
	pstr = strstr(saved_command_line, "TP_COLOR=");
	if (!pstr) {
		hwlog_err("No fastboot TP_COLOR info\n");
		return -EINVAL;
	}
	pstr += strlen("TP_COLOR=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		hwlog_err("No find the TP_COLOR end\n");
		return -EINVAL;
	}

	memset(tp_color_buf, 0, sizeof(tp_color_buf));

	tp_color_len = (unsigned long)(dstr - pstr);
	if (tp_color_len > sizeof(tp_color_buf)) {
		hwlog_info("tp_color_len is %d\n", tp_color_len);
		tp_color_len = sizeof(tp_color_buf);
	}
	memcpy(tp_color_buf, pstr, tp_color_len);

	return 0;
}

#else /* Ko Isolation Macro */

static int __init early_parse_tp_color_cmdline(char *arg)
{
	int len;

	memset(tp_color_buf, 0, sizeof(tp_color_buf));
	if (arg != NULL) {
		len = strlen(arg);
		if (len > sizeof(tp_color_buf))
			len = sizeof(tp_color_buf);
		memcpy(tp_color_buf, arg, len);
	} else {
		hwlog_info("%s: arg is NULL\n", __func__);
	}

	return 0;
}
early_param("TP_COLOR", early_parse_tp_color_cmdline);

#endif /* Ko Isolation Macro */

static int read_tp_color(void)
{
	int tp_color = 0;

	hwlog_info("tp color is %s\n", tp_color_buf);

	if (kstrtoint(tp_color_buf, 0, &tp_color) != 0)
		hwlog_err("kstrtol failed\n");

	return tp_color;
}

static int is_color_correct(u8 color)
{
	if ((color & TP_COLOR_MASK) ==
		((~(color >> TP_COLOR_OFFSET_4BIT)) & TP_COLOR_MASK))
		return true;
	else
		return false;
}

static int read_tp_color_from_nv(void)
{
	int ret;
	char nv_data[NV_DATA_SIZE] = {0};

	ret = read_tp_color_adapter(nv_data, NV_DATA_SIZE);
	if (ret < 0) {
		hwlog_err("%s error %d\n", __func__, ret);
		return ret;
	}

	if (strlen(nv_data) >= TP_COLOR_SIZE) {
		hwlog_err("%s: nv_data is illegal: nv_data_size = %d, tp_color_size = %d\n",
			__func__, strlen(nv_data), TP_COLOR_SIZE);
		return -EINVAL;
	}

	if ((!strncmp(nv_data, "white", strlen("white"))) ||
		(!strncmp(nv_data, "black", strlen("black"))) ||
		(!strncmp(nv_data, "silver", strlen("silver"))) ||
		(!strncmp(nv_data, "pink", strlen("pink"))) ||
		(!strncmp(nv_data, "red", strlen("red"))) ||
		(!strncmp(nv_data, "yellow", strlen("yellow"))) ||
		(!strncmp(nv_data, "blue", strlen("blue"))) ||
		(!strncmp(nv_data, "gold", strlen("gold"))) ||
		(!strncmp(nv_data, "gray", strlen("gray"))) ||
		(!strncmp(nv_data, "cafe", strlen("cafe"))) ||
		(!strncmp(nv_data, "pdblack", strlen("pdblack"))) ||
		(!strncmp(nv_data, "green", strlen("green"))) ||
		(!strncmp(nv_data, "pinkgold", strlen("pinkgold")))) {
		strncpy(g_tp_color_string, nv_data, strlen(nv_data));
		g_tp_color_string[strlen(nv_data)] = '\0';
		return 0;
	}
	hwlog_err("%s: read abnormal value\n", __func__);
	return -1;
}

struct color_type {
	u8 phone_color;
	u8 *color;
};

struct color_type tp_color_list [] = {
	{ .phone_color = WHITE, .color = "white" },
	{ .phone_color = BLACK, .color = "black" },
	{ .phone_color = PINK, .color = "pink" },
	{ .phone_color = PINKGOLD, .color = "pink" },
	{ .phone_color = RED, .color = "red" },
	{ .phone_color = YELLOW, .color = "yellow" },
	{ .phone_color = BLUE, .color = "blue" },
	{ .phone_color = GOLD, .color = "gold" },
	{ .phone_color = SILVER, .color = "silver" },
	{ .phone_color = GRAY, .color = "gray" },
	{ .phone_color = CAFE, .color = "cafe" },
	{ .phone_color = CAFE2, .color = "cafe" },
	{ .phone_color = BLACK2, .color = "pdblack" },
	{ .phone_color = GREEN, .color = "green" },
};

static int get_phone_color(u8 *phone_color)
{
	u8 lcd_id;
	lcd_id = read_tp_color();
	/* 0xff: an invalid tp_color value */
	if (lcd_id != 0xff)
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		*phone_color = cypress_ts_kit_color[0];
	} else if (is_color_correct(lcd_id)) {
		*phone_color = lcd_id;
	} else {
		hwlog_err("LCD/TP ID both error!\n");
		return -EINVAL;
	}
	return 0;
}

static void get_tp_color_string(u8 phone_color)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tp_color_list); i++) {
		if (phone_color == tp_color_list[i].phone_color) {
			strncpy(g_tp_color_string, tp_color_list[i].color, TP_COLOR_SIZE -1);
			break;
		}
	}
	if (i == ARRAY_SIZE(tp_color_list))
		strncpy(g_tp_color_string, "", TP_COLOR_SIZE -1);
}

static void write_tp_color_to_nv(void)
{
	u8 phone_color;

	memset(g_tp_color_string, 0, sizeof(g_tp_color_string));
	if (get_phone_color(&phone_color))
		return;
	hwlog_info("phone_color = %d\n", phone_color);
	get_tp_color_string(phone_color);
	schedule_work(&nv_write_work);
	if (!wait_for_completion_interruptible_timeout(&nv_write_done,
		msecs_to_jiffies(NV_WRITE_TIMEOUT)))
		hwlog_info("nv_write_work time out\n");
	hwlog_info("%s: %s\n", __func__, g_tp_color_string);
}

static void tp_nv_read_work_fn(struct work_struct *work)
{
	int ret;

	reinit_completion(&nv_write_done);
	ret = read_tp_color_from_nv();
	if (ret)
		strncpy(g_tp_color_string, "", sizeof(""));
	complete_all(&nv_read_done);
}

static void read_tp_color_from_system(void)
{
	reinit_completion(&nv_read_done);
	schedule_work(&nv_read_work);
	if (!wait_for_completion_interruptible_timeout(&nv_read_done,
		msecs_to_jiffies(NV_READ_TIMEOUT))) {
		snprintf(g_tp_color_string, TP_COLOR_SIZE, "%s", "");
		hwlog_info("nv_read_work time out\n");
	}
	hwlog_info("%s:tp color is %s\n", __func__, g_tp_color_string);
}

static ssize_t attr_get_tp_color_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	u8 lcd_id;
	u8 phone_color;

	lcd_id = read_tp_color();
	/* 0xff: an invalid tp_color value */
	if (lcd_id != 0xff)
		hwlog_info("lcd id is %u from read tp color\n", lcd_id);
	hwlog_info("%s: tp id=%x, lcd id=%x\n", __func__,
		cypress_ts_kit_color[0], lcd_id);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		phone_color = cypress_ts_kit_color[0];
	} else if (is_color_correct(lcd_id)) {
		phone_color = lcd_id;
	} else {
		read_tp_color_from_system();
		ret = snprintf(buf, TP_COLOR_SIZE, "%s", g_tp_color_string);
		hwlog_info("tp_color %s\n", buf);
		return ret;
	}
	switch (phone_color) {
	case WHITE:
	case WHITE_OLD:
		strncpy(buf, "white", sizeof("white"));
		break;
	case BLACK:
	case BLACK_OLD:
		strncpy(buf, "black", sizeof("black"));
		break;
	case PINK:
	case PINKGOLD:
		strncpy(buf, "pink", sizeof("pink"));
		break;
	case RED:
		strncpy(buf, "red", sizeof("red"));
		break;
	case YELLOW:
		strncpy(buf, "yellow", sizeof("yellow"));
		break;
	case BLUE:
		strncpy(buf, "blue", sizeof("blue"));
		break;
	case GOLD:
	case GOLD_OLD:
		strncpy(buf, "gold", sizeof("gold"));
		break;
	case SILVER:
		strncpy(buf, "silver", sizeof("silver"));
		break;
	case GRAY:
		strncpy(buf, "gray", sizeof("gray"));
		break;
	case CAFE:
	case CAFE2:
		strncpy(buf, "cafe", sizeof("cafe"));
		break;
	case BLACK2:
		strncpy(buf, "pdblack", sizeof("pdblack"));
		break;
	case GREEN:
		strncpy(buf, "green", sizeof("green"));
		break;
	default:
		strncpy(buf, "", sizeof(""));
		break;
	}
	hwlog_info("%s: TP color is %s\n", __func__, buf);
	return strlen(buf);
}

static void tp_nv_write_work_fn(struct work_struct *work)
{
	int ret;

	ret = write_tp_color_adapter(g_tp_color_string);
	if (ret < 0)
		hwlog_info("%s:write tp color failed\n", __func__);
	complete_all(&nv_write_done);
}

static ssize_t attr_set_tp_color_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;

	if (kstrtoul(buf, DEC_BASE_DATA, &val) != 0)
		hwlog_err("kstrtol failed\n");
	hwlog_info("[%s] val=%lu\n", __func__, val);
	if (val == 1)
		write_tp_color_to_nv();
	return count;
}

static struct device_attribute ts_kit_color_file =
__ATTR(ts_kit_color_info, 0664, attr_get_tp_color_info, attr_set_tp_color_info);

static struct platform_device huawei_ts_kit_color = {
	.name = "huawei_ts_kit_color",
	.id = -1,
};

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
static struct device_attribute tp_color_file = __ATTR(tp_color_info,
	0660, attr_get_tp_color_info, attr_set_tp_color_info);

static struct platform_device huawei_tp_color = {
	.name = "huawei_tp_color",
	.id = -1,
};
#endif

static int __init ts_color_info_init(void)
{
	int ret;
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
	uint8_t tp_color;
#endif

	hwlog_info("%s ++\n", __func__);

	INIT_WORK(&nv_read_work, tp_nv_read_work_fn);
	INIT_WORK(&nv_write_work, tp_nv_write_work_fn);
	init_completion(&nv_read_done);
	init_completion(&nv_write_done);
	ret = platform_device_register(&huawei_ts_kit_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret:%d.\n",
			__func__, ret);
		goto REGISTER_ERR;
	}
	if (device_create_file(&huawei_ts_kit_color.dev, &ts_kit_color_file)) {
		hwlog_err("%s:Unable to create interface\n", __func__);
		goto SYSFS_CREATE_FILE_ERR;
	}

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
	ret = platform_device_register(&huawei_tp_color);
	if (ret) {
		hwlog_err("%s: platform_device_register failed, ret: %d\n",
			__func__, ret);
		goto register_dev_err;
	}
	if (device_create_file(&huawei_tp_color.dev, &tp_color_file)) {
		hwlog_err("%s: Unable to create interface\n", __func__);
		goto sysfs_create_tpcolor_file_err;
	}
#endif
#ifdef CONFIG_SENSOR_PARAM_TO_SCP
	hwlog_info("%s:tp id=%x\n", __func__, cypress_ts_kit_color[0]);
	if (is_color_correct(cypress_ts_kit_color[0])) {
		tp_color = cypress_ts_kit_color[0];
		scp_power_monitor_notify(SENSOR_TP_COLOR, &tp_color);
	}
#endif

#ifdef CONFIG_MODULE_KO_TP
	parse_tp_color_cmdline();
#endif

	hwlog_info("%s --\n", __func__);
	return 0;

SYSFS_CREATE_FILE_ERR:
	platform_device_unregister(&huawei_ts_kit_color);
REGISTER_ERR:
	return ret;

#if (defined CONFIG_HUAWEI_THP_MTK) || \
	(defined CONFIG_HUAWEI_DEVKIT_MTK_3_0) || \
	(defined CONFIG_HUAWEI_THP_QCOM) || \
	(defined CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
sysfs_create_tpcolor_file_err:
	platform_device_unregister(&huawei_tp_color);
register_dev_err:
	return ret;
#endif
}

device_initcall(ts_color_info_init);
MODULE_DESCRIPTION("ts color info");
MODULE_AUTHOR("huawei driver group of k3");
MODULE_LICENSE("GPL");
